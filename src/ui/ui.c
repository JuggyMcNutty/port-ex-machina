#include "ui.h"

#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CACHE_MAX 96
#define REPEAT_DELAY_MS  320
#define REPEAT_RATE_MS    90
#define STICK_THRESHOLD 16000

typedef struct {
    char        *text;
    dxl_txt      style;
    SDL_Color    color;
    SDL_Texture *tex;
    int          w, h;
    unsigned     stamp;
} cache_entry;

struct dxl_ui {
    SDL_Window   *win;
    SDL_Renderer *ren;
    int           w, h;
    dxl_metrics   m;

    TTF_Font *f_title, *f_body, *f_item, *f_footer;
    int       headless;

    cache_entry cache[CACHE_MAX];
    unsigned    clock;

    SDL_GameController *pad;

    /* Auto-repeat state for whichever direction is currently held. */
    dxl_act  held;
    Uint32   held_since, last_repeat;
    /* Edge detection for the analog stick, which reports continuously. */
    int      stick_dir;
};

static TTF_Font *font_for(dxl_ui *ui, dxl_txt s) {
    switch (s) {
    case DXL_TXT_TITLE:  return ui->f_title;
    case DXL_TXT_BODY:   return ui->f_body;
    case DXL_TXT_ITEM:   return ui->f_item;
    case DXL_TXT_FOOTER: return ui->f_footer;
    }
    return ui->f_body;
}

dxl_ui *dxl_ui_init(dxl_err *err) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
        dxl_err_set(err, "SDL_Init: %s", SDL_GetError());
        return NULL;
    }
    dxl_ui *ui = dxl_xmalloc(sizeof *ui);
    memset(ui, 0, sizeof *ui);

    /* Fullscreen-desktop rather than a mode switch: the vendor "mali" driver
     * hands out one surface at the panel's native size, and asking it to
     * change modes gains nothing on a fixed screen. */
    ui->win = SDL_CreateWindow("Deus Ex", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, 1280, 720,
                               SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!ui->win) {
        dxl_err_set(err, "SDL_CreateWindow: %s", SDL_GetError());
        free(ui);
        SDL_Quit();
        return NULL;
    }
    ui->ren = SDL_CreateRenderer(ui->win, -1, SDL_RENDERER_ACCELERATED |
                                              SDL_RENDERER_PRESENTVSYNC);
    if (!ui->ren) ui->ren = SDL_CreateRenderer(ui->win, -1, 0);   /* software */
    if (!ui->ren) {
        dxl_err_set(err, "SDL_CreateRenderer: %s", SDL_GetError());
        SDL_DestroyWindow(ui->win);
        free(ui);
        SDL_Quit();
        return NULL;
    }
    SDL_GetRendererOutputSize(ui->ren, &ui->w, &ui->h);
    if (ui->w <= 0 || ui->h <= 0) { ui->w = 1280; ui->h = 720; }
    dxl_metrics_for(&ui->m, ui->h);
    SDL_ShowCursor(SDL_DISABLE);

    if (TTF_Init() == 0) {
        const char *path = dxl_theme_find_font();
        if (path) {
            ui->f_title  = TTF_OpenFont(path, ui->m.title_size);
            ui->f_body   = TTF_OpenFont(path, ui->m.body_size);
            ui->f_item   = TTF_OpenFont(path, ui->m.item_size);
            ui->f_footer = TTF_OpenFont(path, ui->m.footer_size);
        }
    }
    ui->headless = !(ui->f_title && ui->f_body && ui->f_item && ui->f_footer);

    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            ui->pad = SDL_GameControllerOpen(i);
            if (ui->pad) break;
        }
    }
    return ui;
}

void dxl_ui_quit(dxl_ui *ui) {
    if (!ui) return;
    for (int i = 0; i < CACHE_MAX; i++) {
        if (ui->cache[i].tex) SDL_DestroyTexture(ui->cache[i].tex);
        free(ui->cache[i].text);
    }
    if (ui->f_title)  TTF_CloseFont(ui->f_title);
    if (ui->f_body)   TTF_CloseFont(ui->f_body);
    if (ui->f_item)   TTF_CloseFont(ui->f_item);
    if (ui->f_footer) TTF_CloseFont(ui->f_footer);
    if (TTF_WasInit()) TTF_Quit();
    if (ui->pad) SDL_GameControllerClose(ui->pad);
    if (ui->ren) SDL_DestroyRenderer(ui->ren);
    if (ui->win) SDL_DestroyWindow(ui->win);
    free(ui);
    SDL_Quit();
}

int dxl_ui_width (const dxl_ui *ui) { return ui->w; }
int dxl_ui_height(const dxl_ui *ui) { return ui->h; }
const dxl_metrics *dxl_ui_metrics(const dxl_ui *ui) { return &ui->m; }
int dxl_ui_headless(const dxl_ui *ui) { return ui->headless; }

/* ---- text cache ------------------------------------------------------
 * Rendering every string through SDL_ttf each frame is the easy way to make
 * a 60fps menu drop frames on a Cortex-A53. Strings here are stable between
 * frames, so cache the textures and evict the least recently used. */

static cache_entry *cache_get(dxl_ui *ui, const char *s, dxl_txt style,
                              SDL_Color c) {
    cache_entry *lru = &ui->cache[0];
    for (int i = 0; i < CACHE_MAX; i++) {
        cache_entry *e = &ui->cache[i];
        if (e->tex && e->style == style && e->text && strcmp(e->text, s) == 0 &&
            e->color.r == c.r && e->color.g == c.g && e->color.b == c.b &&
            e->color.a == c.a) {
            e->stamp = ++ui->clock;
            return e;
        }
        if (!e->tex) { lru = e; break; }
        if (e->stamp < lru->stamp) lru = e;
    }

    TTF_Font *f = font_for(ui, style);
    if (!f) return NULL;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(f, s, c);
    if (!surf) return NULL;

    if (lru->tex) SDL_DestroyTexture(lru->tex);
    free(lru->text);

    lru->tex   = SDL_CreateTextureFromSurface(ui->ren, surf);
    lru->w     = surf->w;
    lru->h     = surf->h;
    lru->text  = dxl_xstrdup(s);
    lru->style = style;
    lru->color = c;
    lru->stamp = ++ui->clock;
    SDL_FreeSurface(surf);
    return lru->tex ? lru : NULL;
}

void dxl_ui_text(dxl_ui *ui, int x, int y, const char *s, dxl_txt style,
                 SDL_Color c) {
    if (!s || !*s || ui->headless) return;
    cache_entry *e = cache_get(ui, s, style, c);
    if (!e) return;
    SDL_Rect dst = { x, y, e->w, e->h };
    SDL_RenderCopy(ui->ren, e->tex, NULL, &dst);
}

int dxl_ui_text_width(dxl_ui *ui, const char *s, dxl_txt style) {
    if (!s || !*s || ui->headless) return 0;
    int w = 0, h = 0;
    TTF_Font *f = font_for(ui, style);
    if (f) TTF_SizeUTF8(f, s, &w, &h);
    return w;
}

int dxl_ui_line_height(dxl_ui *ui, dxl_txt style) {
    TTF_Font *f = font_for(ui, style);
    return f ? TTF_FontLineSkip(f) : ui->m.line_gap;
}

void dxl_ui_fill(dxl_ui *ui, SDL_Rect r, SDL_Color c) {
    SDL_SetRenderDrawColor(ui->ren, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(ui->ren, &r);
}

int dxl_ui_paragraph(dxl_ui *ui, int x, int y, int max_w, const char *s,
                     dxl_txt style, SDL_Color c) {
    if (!s || !*s) return y;
    int lh = dxl_ui_line_height(ui, style);
    if (ui->headless) return y + lh;

    char line[512];
    size_t len = 0;
    line[0] = '\0';

    const char *p = s;
    while (*p) {
        /* Take the next word, including the space that precedes it, so the
         * measurement matches what gets drawn. */
        const char *ws = p;
        while (*p && *p != ' ') p++;
        size_t wlen = (size_t)(p - ws);
        while (*p == ' ') p++;

        char candidate[512];
        int n = snprintf(candidate, sizeof candidate, "%s%s%.*s",
                         line, len ? " " : "", (int)wlen, ws);
        if (n < 0) break;

        if (len && dxl_ui_text_width(ui, candidate, style) > max_w) {
            dxl_ui_text(ui, x, y, line, style, c);
            y += lh;
            snprintf(line, sizeof line, "%.*s", (int)wlen, ws);
        } else {
            snprintf(line, sizeof line, "%s", candidate);
        }
        len = strlen(line);
    }
    if (len) { dxl_ui_text(ui, x, y, line, style, c); y += lh; }
    return y;
}

int dxl_ui_header(dxl_ui *ui, const char *title) {
    int x = ui->m.pad, y = ui->m.pad;
    dxl_ui_text(ui, x, y, title, DXL_TXT_TITLE, DXL_PAL.accent);
    y += dxl_ui_line_height(ui, DXL_TXT_TITLE) + ui->m.pad / 4;
    dxl_ui_fill(ui, (SDL_Rect){ x, y, ui->w - 2 * ui->m.pad, 2 }, DXL_PAL.rule);
    return y + ui->m.pad / 2;
}

void dxl_ui_footer(dxl_ui *ui, const char *hints) {
    int lh = dxl_ui_line_height(ui, DXL_TXT_FOOTER);
    int y  = ui->h - ui->m.pad / 2 - lh;
    dxl_ui_fill(ui, (SDL_Rect){ ui->m.pad, y - ui->m.pad / 4,
                                ui->w - 2 * ui->m.pad, 1 }, DXL_PAL.rule);
    dxl_ui_text(ui, ui->m.pad, y, hints, DXL_TXT_FOOTER, DXL_PAL.text_dim);
}

void dxl_ui_row(dxl_ui *ui, int y, const char *label, const char *value,
                int selected, int enabled) {
    int x = ui->m.pad, w = ui->w - 2 * ui->m.pad;
    int ih = ui->m.item_height;

    if (selected) {
        dxl_ui_fill(ui, (SDL_Rect){ x, y, w, ih }, DXL_PAL.panel_sel);
        dxl_ui_fill(ui, (SDL_Rect){ x, y, 4, ih }, DXL_PAL.accent);
    }
    SDL_Color c = !enabled ? DXL_PAL.text_dim
                : selected ? DXL_PAL.accent : DXL_PAL.text;

    int lh  = dxl_ui_line_height(ui, DXL_TXT_ITEM);
    int ty  = y + (ih - lh) / 2;
    dxl_ui_text(ui, x + ui->m.pad / 2, ty, label, DXL_TXT_ITEM, c);

    if (value && *value) {
        int vw = dxl_ui_text_width(ui, value, DXL_TXT_ITEM);
        dxl_ui_text(ui, x + w - ui->m.pad / 2 - vw, ty, value, DXL_TXT_ITEM,
                    selected ? DXL_PAL.text : DXL_PAL.text_dim);
    }
}

void dxl_ui_frame_begin(dxl_ui *ui) {
    SDL_SetRenderDrawColor(ui->ren, DXL_PAL.bg.r, DXL_PAL.bg.g, DXL_PAL.bg.b, 255);
    SDL_RenderClear(ui->ren);
}

void dxl_ui_frame_end(dxl_ui *ui) { SDL_RenderPresent(ui->ren); }

/* ---- input ----------------------------------------------------------- */

static dxl_act from_button(SDL_GameControllerButton b) {
    switch (b) {
    case SDL_CONTROLLER_BUTTON_DPAD_UP:    return DXL_ACT_UP;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  return DXL_ACT_DOWN;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  return DXL_ACT_LEFT;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return DXL_ACT_RIGHT;
    /* The pad reports as an Xbox 360 controller, so SDL's A/B are already the
     * physical A/B on this shell -- confirmed with tools/probe-sdl.c. */
    case SDL_CONTROLLER_BUTTON_A:          return DXL_ACT_CONFIRM;
    case SDL_CONTROLLER_BUTTON_B:          return DXL_ACT_BACK;
    case SDL_CONTROLLER_BUTTON_START:      return DXL_ACT_CONFIRM;
    case SDL_CONTROLLER_BUTTON_BACK:       return DXL_ACT_QUIT;   /* SELECT */
    case SDL_CONTROLLER_BUTTON_GUIDE:      return DXL_ACT_QUIT;   /* MENU   */
    default:                               return DXL_ACT_NONE;
    }
}

static dxl_act from_key(SDL_Keycode k) {
    switch (k) {
    case SDLK_UP:     case SDLK_w: return DXL_ACT_UP;
    case SDLK_DOWN:   case SDLK_s: return DXL_ACT_DOWN;
    case SDLK_LEFT:   case SDLK_a: return DXL_ACT_LEFT;
    case SDLK_RIGHT:  case SDLK_d: return DXL_ACT_RIGHT;
    case SDLK_RETURN: case SDLK_SPACE: case SDLK_z: return DXL_ACT_CONFIRM;
    case SDLK_BACKSPACE: case SDLK_x: return DXL_ACT_BACK;
    case SDLK_ESCAPE: case SDLK_q: return DXL_ACT_QUIT;
    default: return DXL_ACT_NONE;
    }
}

static int is_direction(dxl_act a) {
    return a == DXL_ACT_UP || a == DXL_ACT_DOWN ||
           a == DXL_ACT_LEFT || a == DXL_ACT_RIGHT;
}

dxl_act dxl_ui_poll(dxl_ui *ui) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            return DXL_ACT_QUIT;

        case SDL_CONTROLLERDEVICEADDED:
            if (!ui->pad) ui->pad = SDL_GameControllerOpen(e.cdevice.which);
            break;

        case SDL_CONTROLLERBUTTONDOWN: {
            dxl_act a = from_button((SDL_GameControllerButton)e.cbutton.button);
            if (is_direction(a)) {
                ui->held = a;
                ui->held_since = ui->last_repeat = SDL_GetTicks();
            }
            if (a != DXL_ACT_NONE) return a;
            break;
        }
        case SDL_CONTROLLERBUTTONUP: {
            dxl_act a = from_button((SDL_GameControllerButton)e.cbutton.button);
            if (a == ui->held) ui->held = DXL_ACT_NONE;
            break;
        }

        case SDL_CONTROLLERAXISMOTION: {
            /* Turn the continuous stick into discrete steps, one per crossing
             * of the threshold, so a held stick repeats at the same rate as
             * the d-pad instead of scrolling uncontrollably. */
            int vertical = (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY);
            int horizontal = (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX);
            if (!vertical && !horizontal) break;

            int v = e.caxis.value;
            int dir = (v > STICK_THRESHOLD) ? 1 : (v < -STICK_THRESHOLD) ? -1 : 0;
            int code = dir == 0 ? 0 : (vertical ? dir * 2 : dir);
            if (code == ui->stick_dir) break;
            ui->stick_dir = code;
            if (dir == 0) { ui->held = DXL_ACT_NONE; break; }

            dxl_act a = vertical ? (dir > 0 ? DXL_ACT_DOWN : DXL_ACT_UP)
                                 : (dir > 0 ? DXL_ACT_RIGHT : DXL_ACT_LEFT);
            ui->held = a;
            ui->held_since = ui->last_repeat = SDL_GetTicks();
            return a;
        }

        case SDL_KEYDOWN: {
            if (e.key.repeat) break;   /* we do our own repeat */
            dxl_act a = from_key(e.key.keysym.sym);
            if (is_direction(a)) {
                ui->held = a;
                ui->held_since = ui->last_repeat = SDL_GetTicks();
            }
            if (a != DXL_ACT_NONE) return a;
            break;
        }
        case SDL_KEYUP: {
            dxl_act a = from_key(e.key.keysym.sym);
            if (a == ui->held) ui->held = DXL_ACT_NONE;
            break;
        }
        default:
            break;
        }
    }

    if (ui->held != DXL_ACT_NONE) {
        Uint32 now = SDL_GetTicks();
        if (now - ui->held_since >= REPEAT_DELAY_MS &&
            now - ui->last_repeat >= REPEAT_RATE_MS) {
            ui->last_repeat = now;
            return ui->held;
        }
    }
    return DXL_ACT_NONE;
}
