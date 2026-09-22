#include "screens.h"
#include "core/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Every visible string comes from Startup.int when it is there, with a
 * fallback so a stripped install still reads sensibly. */
static const char *S(const dxl_session *s, const char *sect, const char *key,
                     const char *fallback) {
    return dxl_strings_get(s->app->startup, sect, key, fallback);
}
static const char *G(const dxl_session *s, const char *key, const char *fallback) {
    return S(s, "General", key, fallback);
}

#define SAFEMODE_SECT "IDDIALOG_ConfigPageSafeMode"
#define SAFEOPTS_SECT "IDDIALOG_ConfigPageSafeOptions"
#define RENDER_SECT   "IDDIALOG_ConfigPageRenderer"
#define DETAIL_SECT   "IDDIALOG_ConfigPageDetail"
#define FIRST_SECT    "IDDIALOG_ConfigPageFirstTime"

static void clamp_cursor(dxl_session *s, int count) {
    if (count <= 0) { s->cursor = 0; return; }
    if (s->cursor < 0) s->cursor = count - 1;
    if (s->cursor >= count) s->cursor = 0;
}

static void goto_screen(dxl_session *s, dxl_scr scr) {
    s->screen = scr;
    s->cursor = 0;
    s->notice[0] = '\0';
}

void dxl_session_start(dxl_session *s, dxl_app *app, dxl_ui *ui) {
    memset(s, 0, sizeof *s);
    s->app = app;
    s->ui  = ui;

    if (dxl_app_needs_install(app)) { s->screen = DXL_SCR_INSTALL; return; }

    switch (app->decision.screen) {
    case DXL_SCREEN_MAIN_SAFE:
    case DXL_SCREEN_MAIN_RECOVERY:
        s->screen = DXL_SCR_MAIN;
        break;
    case DXL_SCREEN_RENDERER_FIRST:
        s->screen = DXL_SCR_RENDERER;
        s->from_first_run = 1;
        break;
    case DXL_SCREEN_RENDERER_VIDEO:
        s->screen = DXL_SCR_RENDERER;
        break;
    case DXL_SCREEN_NONE:
    default:
        /* Nothing to ask: this is the ordinary case on a settled install. */
        s->screen = DXL_SCR_LAUNCH;
        break;
    }

    /* Start the renderer cursor on whatever is already configured, so
     * confirming without moving is a no-op rather than a silent change. */
    if (s->screen == DXL_SCR_RENDERER) {
        int at = dxl_renderers_find(&app->renderers,
                                    dxl_config_render_device(app->cfg));
        if (at >= 0) s->cursor = at;
    }
}

int dxl_session_finished(const dxl_session *s) {
    return s->screen == DXL_SCR_LAUNCH || s->screen == DXL_SCR_SAFE_EXEC ||
           s->screen == DXL_SCR_QUIT;
}

/* ---- install --------------------------------------------------------- */

static void install_draw(dxl_session *s) {
    dxl_ui *ui = s->ui;
    const dxl_metrics *m = dxl_ui_metrics(ui);
    int w = dxl_ui_width(ui) - 2 * m->pad;

    int y = dxl_ui_header(ui, "Deus Ex — game files not found");
    y = dxl_ui_paragraph(ui, m->pad, y, w,
        "Copy your own Deus Ex installation into the directory below, then "
        "choose Check again. The launcher needs these files; it does not "
        "include them.", DXL_TXT_BODY, DXL_PAL.text);
    y += m->line_gap / 2;

    dxl_ui_text(ui, m->pad, y, s->app->game_dir, DXL_TXT_BODY, DXL_PAL.accent);
    y += dxl_ui_line_height(ui, DXL_TXT_BODY) + m->line_gap / 2;

    for (int i = 0; i < s->app->install.item_count; i++) {
        const dxl_install_item *it = &s->app->install.items[i];
        char line[256];
        snprintf(line, sizeof line, "%s  %s", it->found ? "present" : "MISSING",
                 it->relative);
        dxl_ui_text(ui, m->pad, y, line, DXL_TXT_BODY,
                    it->found ? DXL_PAL.ok : DXL_PAL.bad);
        y += dxl_ui_line_height(ui, DXL_TXT_BODY);
    }
    y += m->line_gap / 2;

    static const char *items[] = { "Check again", "Exit" };
    for (int i = 0; i < 2; i++)
        dxl_ui_row(ui, y + i * m->item_height, items[i], NULL, s->cursor == i, 1);

    dxl_ui_footer(ui, "A  select      B / MENU  exit");
}

static void install_act(dxl_session *s, dxl_act a) {
    switch (a) {
    case DXL_ACT_UP:   s->cursor--; clamp_cursor(s, 2); break;
    case DXL_ACT_DOWN: s->cursor++; clamp_cursor(s, 2); break;
    case DXL_ACT_CONFIRM:
        if (s->cursor == 0) {
            dxl_install_free(&s->app->install);
            dxl_install_probe(s->app->game_dir, &s->app->install);
            dxl_log("install re-probed: %s",
                    s->app->install.ok ? "complete" : "still incomplete");
            if (s->app->install.ok) {
                /* Re-run the whole entry decision: with files present, a
                 * pristine install wants the first-run flow. */
                dxl_session_start(s, s->app, s->ui);
            }
        } else {
            goto_screen(s, DXL_SCR_QUIT);
        }
        break;
    case DXL_ACT_BACK:
    case DXL_ACT_QUIT: goto_screen(s, DXL_SCR_QUIT); break;
    default: break;
    }
}

/* ---- main menu (SafeMode / RecoveryMode) ----------------------------- */

enum { MAIN_RUN, MAIN_VIDEO, MAIN_SAFE, MAIN_WEB, MAIN_COUNT };

static void main_draw(dxl_session *s) {
    dxl_ui *ui = s->ui;
    const dxl_metrics *m = dxl_ui_metrics(ui);
    int w = dxl_ui_width(ui) - 2 * m->pad;
    int recovery = (s->app->decision.screen == DXL_SCREEN_MAIN_RECOVERY);

    const char *title = recovery ? G(s, "RecoveryMode", "Deus Ex Recovery Mode")
                                 : G(s, "SafeMode", "Deus Ex Safe Mode");
    int y = dxl_ui_header(ui, title);

    /* The original has two prompts and picks by entry path: the recovery text
     * explains that the last run did not shut down properly. */
    const char *prompt = recovery
        ? S(s, SAFEMODE_SECT, "IDC_SafeModePrompt",
            "The previous time Deus Ex was run, it was not shut down properly.")
        : S(s, SAFEMODE_SECT, "IDC_SafeModePrompt2",
            "Deus Ex safe mode options: if you are experiencing problems, you "
            "may use the options below for recovery.");
    y = dxl_ui_paragraph(ui, m->pad, y, w, prompt, DXL_TXT_BODY, DXL_PAL.text);
    y += m->line_gap / 2;

    const char *items[MAIN_COUNT] = {
        S(s, SAFEMODE_SECT, "IDC_Run",      "Run Deus Ex"),
        S(s, SAFEMODE_SECT, "IDC_Video",    "Change your 3D video device"),
        S(s, SAFEMODE_SECT, "IDC_SafeMode", "Run Deus Ex in safe mode - for troubleshooting"),
        S(s, SAFEMODE_SECT, "IDC_Web",      "Visit our Web site for troubleshooting tips"),
    };
    for (int i = 0; i < MAIN_COUNT; i++)
        dxl_ui_row(ui, y + i * m->item_height, items[i], NULL, s->cursor == i, 1);

    if (s->notice[0]) {
        int ny = y + MAIN_COUNT * m->item_height + m->line_gap / 2;
        dxl_ui_paragraph(ui, m->pad, ny, w, s->notice, DXL_TXT_BODY, DXL_PAL.warn);
    }
    dxl_ui_footer(ui, "A  select      B / MENU  exit without starting");
}

static void main_act(dxl_session *s, dxl_act a) {
    switch (a) {
    case DXL_ACT_UP:   s->cursor--; clamp_cursor(s, MAIN_COUNT); break;
    case DXL_ACT_DOWN: s->cursor++; clamp_cursor(s, MAIN_COUNT); break;
    case DXL_ACT_CONFIRM:
        switch (s->cursor) {
        case MAIN_RUN:   goto_screen(s, DXL_SCR_LAUNCH); break;
        case MAIN_VIDEO:
            s->came_from_menu = 1;
            goto_screen(s, DXL_SCR_RENDERER);
            {
                int at = dxl_renderers_find(&s->app->renderers,
                                            dxl_config_render_device(s->app->cfg));
                if (at >= 0) s->cursor = at;
            }
            break;
        case MAIN_SAFE:
            s->came_from_menu = 1;
            goto_screen(s, DXL_SCR_SAFEOPTIONS);
            break;
        case MAIN_WEB:
            /* There is no browser to hand off to, so show the address rather
             * than silently doing nothing (the original ShellExecutes it). */
            snprintf(s->notice, sizeof s->notice, "Troubleshooting: %s",
                     G(s, "WebPage", "http://www.deusex.com/"));
            break;
        }
        break;
    case DXL_ACT_BACK:
    case DXL_ACT_QUIT: goto_screen(s, DXL_SCR_QUIT); break;
    default: break;
    }
}

/* ---- renderer -------------------------------------------------------- */

static void renderer_draw(dxl_session *s) {
    dxl_ui *ui = s->ui;
    const dxl_metrics *m = dxl_ui_metrics(ui);
    int w = dxl_ui_width(ui) - 2 * m->pad;
    const dxl_renderer_list *rl = &s->app->renderers;

    const char *title = s->from_first_run
        ? G(s, "FirstTime", "Deus Ex First-Time Configuration")
        : G(s, "Video", "Deus Ex Video Configuration");
    int y = dxl_ui_header(ui, title);

    y = dxl_ui_paragraph(ui, m->pad, y, w,
        S(s, RENDER_SECT, "IDC_RenderPrompt",
          "Choose the renderer Deus Ex should use."),
        DXL_TXT_BODY, DXL_PAL.text);
    y += m->line_gap / 2;

    if (rl->count == 0) {
        dxl_ui_paragraph(ui, m->pad, y, w,
            "No renderers are declared in renderers.ini, so there is nothing "
            "to choose. The existing setting will be kept.",
            DXL_TXT_BODY, DXL_PAL.warn);
        dxl_ui_footer(ui, "A  continue      B  back");
        return;
    }

    for (size_t i = 0; i < rl->count; i++) {
        const char *badge = rl->items[i].certified ? NULL : "all devices";
        dxl_ui_row(ui, y + (int)i * m->item_height, rl->items[i].label, badge,
                   s->cursor == (int)i, 1);
    }
    y += (int)rl->count * m->item_height + m->line_gap / 2;

    /* The description pane is the Descriptions section of Startup.int -- the
     * game's own wording for each device, where it has one. */
    if (s->cursor >= 0 && s->cursor < (int)rl->count) {
        const dxl_renderer *r = &rl->items[s->cursor];
        dxl_ui_text(ui, m->pad, y, r->class_name, DXL_TXT_FOOTER, DXL_PAL.accent_dim);
        y += dxl_ui_line_height(ui, DXL_TXT_FOOTER) + m->line_gap / 4;
        dxl_ui_paragraph(ui, m->pad, y, w, r->description, DXL_TXT_BODY,
                         DXL_PAL.text_dim);
    }
    dxl_ui_footer(ui, s->came_from_menu ? "A  choose      B  back"
                                        : "A  choose      MENU  exit");
}

static void renderer_act(dxl_session *s, dxl_act a) {
    int count = (int)s->app->renderers.count;
    switch (a) {
    case DXL_ACT_UP:   s->cursor--; clamp_cursor(s, count); break;
    case DXL_ACT_DOWN: s->cursor++; clamp_cursor(s, count); break;
    case DXL_ACT_CONFIRM:
        if (count > 0 && s->cursor >= 0 && s->cursor < count)
            dxl_app_choose_renderer(s->app,
                s->app->renderers.items[s->cursor].class_name);
        goto_screen(s, DXL_SCR_DETAIL);
        break;
    case DXL_ACT_BACK:
        /* Back only means something if there is a menu behind us. In the
         * first-run flow this screen is the beginning. */
        if (s->came_from_menu) goto_screen(s, DXL_SCR_MAIN);
        break;
    case DXL_ACT_QUIT: goto_screen(s, DXL_SCR_QUIT); break;
    default: break;
    }
}

/* ---- detail ---------------------------------------------------------- */

enum { DET_SOUND, DET_SKINS, DET_WORLD, DET_RES, DET_CONTINUE, DET_COUNT };

static void detail_draw(dxl_session *s) {
    dxl_ui *ui = s->ui;
    const dxl_metrics *m = dxl_ui_metrics(ui);
    int w = dxl_ui_width(ui) - 2 * m->pad;
    const dxl_detail *d = &s->app->detail;

    int y = dxl_ui_header(ui, "Detail");
    y = dxl_ui_paragraph(ui, m->pad, y, w,
        S(s, DETAIL_SECT, "IDC_DetailPrompt",
          "Deus Ex has selected the following detail options to optimise "
          "performance."),
        DXL_TXT_BODY, DXL_PAL.text);
    y += m->line_gap / 2;

    /* Both halves of every pair are named in Startup.int [General]; the value
     * column shows whichever is in force. */
    struct { const char *label, *value; } rows[DET_COUNT] = {
        { "Sound quality",   d->low_sound ? G(s, "SoundLow",  "Low sound quality")
                                          : G(s, "SoundHigh", "High sound quality") },
        { "Player skins",    d->low_skins ? G(s, "SkinsLow",  "Medium detail player skin textures")
                                          : G(s, "SkinsHigh", "High detail player skins") },
        { "World textures",  d->low_world ? G(s, "WorldLow",  "Medium detail world textures")
                                          : G(s, "WorldHigh", "High detail textures") },
        { "Resolution",      d->low_res   ? G(s, "ResLow",    "Low video resolution")
                                          : G(s, "ResHigh",   "Standard video resolution") },
        { "Continue",        NULL },
    };
    for (int i = 0; i < DET_COUNT; i++)
        dxl_ui_row(ui, y + i * m->item_height, rows[i].label, rows[i].value,
                   s->cursor == i, 1);

    y += DET_COUNT * m->item_height + m->line_gap / 2;
    dxl_ui_paragraph(ui, m->pad, y, w,
        S(s, DETAIL_SECT, "IDC_DetailNote",
          "You may change these options from the game's Preferences window "
          "later, if you wish."),
        DXL_TXT_BODY, DXL_PAL.text_dim);

    dxl_ui_footer(ui, "left / right  change      A  continue      B  back");
}

static void detail_act(dxl_session *s, dxl_act a) {
    dxl_detail *d = &s->app->detail;
    int *flags[4] = { &d->low_sound, &d->low_skins, &d->low_world, &d->low_res };

    switch (a) {
    case DXL_ACT_UP:   s->cursor--; clamp_cursor(s, DET_COUNT); break;
    case DXL_ACT_DOWN: s->cursor++; clamp_cursor(s, DET_COUNT); break;
    case DXL_ACT_LEFT:
    case DXL_ACT_RIGHT:
        if (s->cursor >= 0 && s->cursor < 4) *flags[s->cursor] = !*flags[s->cursor];
        break;
    case DXL_ACT_CONFIRM:
        if (s->cursor >= 0 && s->cursor < 4) {
            *flags[s->cursor] = !*flags[s->cursor];
            break;
        }
        dxl_app_apply_detail(s->app);
        /* The greeting page is only honest on an actual first run. */
        goto_screen(s, s->from_first_run ? DXL_SCR_FIRSTRUN : DXL_SCR_LAUNCH);
        break;
    case DXL_ACT_BACK:  goto_screen(s, DXL_SCR_RENDERER); break;
    case DXL_ACT_QUIT:  goto_screen(s, DXL_SCR_QUIT); break;
    default: break;
    }
}

/* ---- safe options ---------------------------------------------------- */

enum { SO_NOSOUND, SO_NO3DSOUND, SO_NO3DVIDEO, SO_WINDOW, SO_RES,
       SO_RESETCONFIG, SO_NOPROCESSOR, SO_NOJOY, SO_RESTART, SO_COUNT };

static void safeopts_draw(dxl_session *s) {
    dxl_ui *ui = s->ui;
    const dxl_metrics *m = dxl_ui_metrics(ui);
    int w = dxl_ui_width(ui) - 2 * m->pad;
    dxl_safe_options *o = &s->app->safe;

    int y = dxl_ui_header(ui, G(s, "SafeMode", "Deus Ex Safe Mode"));
    y = dxl_ui_paragraph(ui, m->pad, y, w,
        S(s, SAFEOPTS_SECT, "IDC_SafeOptions",
          "Safe mode options, for diagnosing problems"),
        DXL_TXT_BODY, DXL_PAL.text);
    y += m->line_gap / 4;

    /* Order and labels are the original's, and all eight are live -- in the
     * shipped binary three of them were constructed but never read. */
    struct { const char *key, *fallback; int *flag; } boxes[8] = {
        { "IDC_NoSound",     "Disable all sound",                   &o->no_sound     },
        { "IDC_No3DSound",   "Disable 3D sound hardware",           &o->no_3d_sound  },
        { "IDC_No3DVideo",   "Disable 3D video hardware",           &o->no_3d_video  },
        { "IDC_Window",      "Run the game in a window",            &o->windowed     },
        { "IDC_Res",         "Run in standard 640x480 resolution",  &o->default_res  },
        { "IDC_ResetConfig", "Reset all configuration options to defaults",
                                                                    &o->reset_config },
        { "IDC_NoProcessor", "Disable Pentium III/3DNow processor extensions",
                                                                    &o->no_processor },
        { "IDC_NoJoy",       "Disable joystick support",            &o->no_joy       },
    };
    for (int i = 0; i < 8; i++)
        dxl_ui_row(ui, y + i * m->item_height,
                   S(s, SAFEOPTS_SECT, boxes[i].key, boxes[i].fallback),
                   *boxes[i].flag ? "on" : "off", s->cursor == i, 1);

    dxl_ui_row(ui, y + 8 * m->item_height, "Restart with these options", NULL,
               s->cursor == SO_RESTART, 1);

    /* Safe mode re-executes rather than applying in place, so showing the
     * exact command line is both honest and the fastest way to see that the
     * eight boxes really are independent now. */
    char *flags = dxl_safe_flags(o);
    int fy = y + SO_COUNT * m->item_height + m->line_gap / 2;
    char line[512];
    snprintf(line, sizeof line, "will restart with:  %s%s",
             flags[0] ? flags : "(no flags)",
             o->reset_config ? "   + delete DeusEx.ini" : "");
    dxl_ui_paragraph(ui, m->pad, fy, w, line, DXL_TXT_FOOTER, DXL_PAL.accent_dim);
    free(flags);

    dxl_ui_footer(ui, "A  toggle / restart      B  back");
}

static void safeopts_act(dxl_session *s, dxl_act a) {
    dxl_safe_options *o = &s->app->safe;
    int *flags[8] = { &o->no_sound, &o->no_3d_sound, &o->no_3d_video,
                      &o->windowed, &o->default_res, &o->reset_config,
                      &o->no_processor, &o->no_joy };
    switch (a) {
    case DXL_ACT_UP:   s->cursor--; clamp_cursor(s, SO_COUNT); break;
    case DXL_ACT_DOWN: s->cursor++; clamp_cursor(s, SO_COUNT); break;
    case DXL_ACT_LEFT:
    case DXL_ACT_RIGHT:
    case DXL_ACT_CONFIRM:
        if (s->cursor >= 0 && s->cursor < 8) {
            *flags[s->cursor] = !*flags[s->cursor];
        } else if (a == DXL_ACT_CONFIRM) {
            goto_screen(s, DXL_SCR_SAFE_EXEC);
        }
        break;
    case DXL_ACT_BACK:
        goto_screen(s, s->came_from_menu ? DXL_SCR_MAIN : DXL_SCR_QUIT);
        break;
    case DXL_ACT_QUIT: goto_screen(s, DXL_SCR_QUIT); break;
    default: break;
    }
}

/* ---- first-run greeting ---------------------------------------------- */

static void firstrun_draw(dxl_session *s) {
    dxl_ui *ui = s->ui;
    const dxl_metrics *m = dxl_ui_metrics(ui);
    int w = dxl_ui_width(ui) - 2 * m->pad;

    int y = dxl_ui_header(ui, G(s, "FirstTime", "Deus Ex First-Time Configuration"));
    y = dxl_ui_paragraph(ui, m->pad, y, w,
        S(s, FIRST_SECT, "IDC_Prompt",
          "Deus Ex is starting up for the first time."),
        DXL_TXT_BODY, DXL_PAL.text);
    y += m->line_gap;

    dxl_ui_row(ui, y, G(s, "Run", "Run!"), NULL, 1, 1);
    dxl_ui_footer(ui, "A  start the game      B  back");
}

static void firstrun_act(dxl_session *s, dxl_act a) {
    switch (a) {
    case DXL_ACT_CONFIRM: goto_screen(s, DXL_SCR_LAUNCH); break;
    case DXL_ACT_BACK:    goto_screen(s, DXL_SCR_DETAIL); break;
    case DXL_ACT_QUIT:    goto_screen(s, DXL_SCR_QUIT); break;
    default: break;
    }
}

/* ---- dispatch -------------------------------------------------------- */

int dxl_session_step(dxl_session *s) {
    if (dxl_session_finished(s)) return 1;

    dxl_act a;
    while ((a = dxl_ui_poll(s->ui)) != DXL_ACT_NONE) {
        switch (s->screen) {
        case DXL_SCR_INSTALL:     install_act (s, a); break;
        case DXL_SCR_MAIN:        main_act    (s, a); break;
        case DXL_SCR_RENDERER:    renderer_act(s, a); break;
        case DXL_SCR_DETAIL:      detail_act  (s, a); break;
        case DXL_SCR_SAFEOPTIONS: safeopts_act(s, a); break;
        case DXL_SCR_FIRSTRUN:    firstrun_act(s, a); break;
        default: break;
        }
        if (dxl_session_finished(s)) return 1;
    }

    dxl_ui_frame_begin(s->ui);
    switch (s->screen) {
    case DXL_SCR_INSTALL:     install_draw (s); break;
    case DXL_SCR_MAIN:        main_draw    (s); break;
    case DXL_SCR_RENDERER:    renderer_draw(s); break;
    case DXL_SCR_DETAIL:      detail_draw  (s); break;
    case DXL_SCR_SAFEOPTIONS: safeopts_draw(s); break;
    case DXL_SCR_FIRSTRUN:    firstrun_draw(s); break;
    default: break;
    }
    dxl_ui_frame_end(s->ui);
    return 0;
}
