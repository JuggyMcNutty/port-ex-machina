#define _GNU_SOURCE
#include "app.h"

#include "core/cmdline.h"
#include "core/ini.h"
#include "core/log.h"
#include "core/paths.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* The launcher's own settings, kept separate from the game's config so that
 * resetting the game ini (safe-mode checkbox 6) cannot lose the path to the
 * game files. */
#define LAUNCHER_INI "launcher.ini"
#define RENDERERS_INI "renderers.ini"

static char *dup_or(const char *v, const char *fallback) {
    return dxl_xstrdup(v && *v ? v : fallback);
}

int dxl_app_init(dxl_app *app, int argc, char **argv, dxl_err *err) {
    memset(app, 0, sizeof *app);

    app->exe_path = dxl_path_self();
    if (!app->exe_path) app->exe_path = dxl_xstrdup(argv[0]);
    app->app_dir  = dxl_path_dirname(app->exe_path);
    app->cmdline  = dxl_cmdline_join(argc, argv);

    /* --- launcher.ini ------------------------------------------------- */
    char *lpath = dxl_path_join(app->app_dir, LAUNCHER_INI);
    dxl_ini *l = dxl_ini_load(lpath, NULL);
    free(lpath);

    app->package      = dup_or(l ? dxl_ini_get(l, "Launcher", "Package") : NULL, "DeusEx");
    app->game_command = dup_or(l ? dxl_ini_get(l, "Launcher", "GameCommand") : NULL,
                               "./run-game.sh");
    {
        const char *gd = l ? dxl_ini_get(l, "Launcher", "GameDir") : NULL;
        if (gd && *gd) {
            char *n = dxl_path_from_ini(gd);
            app->game_dir = n;
        } else {
            /* Default to a "game" directory beside the launcher, which is
             * what the packaging ships and what the install screen tells the
             * user to fill. */
            app->game_dir = dxl_path_join(app->app_dir, "game");
        }
    }
    if (l) dxl_ini_free(l);

    /* --- install ------------------------------------------------------ */
    dxl_install_probe(app->game_dir, &app->install);
    app->system_dir = dxl_xstrdup(app->install.system_dir);

    /* Log next to the game's own, where anyone debugging would look. */
    char *logp = dxl_path_join(app->system_dir, "DeusExLauncher.log");
    if (dxl_path_is_dir(app->system_dir)) dxl_log_open(logp);
    free(logp);
    dxl_log("launcher starting: exe=%s", app->exe_path);
    dxl_log("game dir: %s (%s)", app->game_dir,
            app->install.ok ? "complete" : "INCOMPLETE");

    /* --- config and strings ------------------------------------------- */
    app->cfg     = dxl_config_open(app->system_dir, app->package);
    app->startup = dxl_strings_load(app->system_dir, "Startup");
    if (!dxl_strings_present(app->startup))
        dxl_log("note: %s not found; using built-in fallback strings",
                dxl_strings_path(app->startup));

    char *rpath = dxl_path_join(app->app_dir, RENDERERS_INI);
    dxl_renderers_load(&app->renderers, rpath, app->startup);
    free(rpath);
    dxl_log("renderer candidates: %zu", app->renderers.count);

    dxl_sentinel_init(&app->sentinel, app->system_dir);

    /* --- single instance ---------------------------------------------- */
    int other = dxl_instance_other_running(app->game_dir);

    /* --- the decision -------------------------------------------------- */
    dxl_policy_input in = {
        .cmdline            = app->cmdline,
        .first_run          = dxl_config_first_run(app->cfg),
        .other_instance     = other,
        .running_ini_exists = dxl_sentinel_exists(&app->sentinel),
        .is_client          = !dxl_cmd_param(app->cmdline, "server"),
    };
    dxl_policy_decide(&in, &app->decision);
    dxl_log("decision: action=%s screen=%s firstrun=%d other=%d sentinel=%d",
            dxl_action_name(app->decision.action),
            dxl_screen_name(app->decision.screen),
            app->decision.effective_first_run, other, in.running_ini_exists);

    /* Hold the lock for the rest of the session unless we are the messenger. */
    if (app->decision.action != DXL_ACTION_FORWARD)
        app->instance = dxl_instance_acquire(app->game_dir);

    /* Detail defaults. The original picks low when the machine lacks MMX or
     * has 64 MB or less (0x1090EDA1); the handheld equivalent is simply
     * whether there is room to breathe. */
    long pages = sysconf(_SC_PHYS_PAGES), psize = sysconf(_SC_PAGESIZE);
    long long mem = (pages > 0 && psize > 0) ? (long long)pages * psize : 0;
    dxl_detail_defaults(&app->detail, mem > 0 && mem <= 256LL * 1024 * 1024);

    (void)err;
    return 0;
}

void dxl_app_shutdown(dxl_app *app) {
    if (!app) return;
    if (app->instance) dxl_instance_release(app->instance);
    dxl_renderers_free(&app->renderers);
    dxl_install_free(&app->install);
    dxl_sentinel_free(&app->sentinel);
    dxl_strings_free(app->startup);
    dxl_config_free(app->cfg);
    free(app->app_dir); free(app->exe_path); free(app->game_dir);
    free(app->system_dir); free(app->package); free(app->game_command);
    free(app->cmdline);
    dxl_log_close();
    memset(app, 0, sizeof *app);
}

int dxl_app_needs_install(const dxl_app *app) { return !app->install.ok; }

void dxl_app_choose_renderer(dxl_app *app, const char *class_name) {
    if (!class_name || !*class_name) return;
    dxl_config_set_render_device(app->cfg, class_name);
    dxl_log("renderer: %s", class_name);
}

void dxl_app_apply_detail(dxl_app *app) {
    const char *rd = dxl_config_render_device(app->cfg);
    dxl_config_apply_detail(app->cfg, &app->detail, rd);
    dxl_log("detail: sound=%s skins=%s world=%s res=%s",
            app->detail.low_sound ? "low" : "high",
            app->detail.low_skins ? "low" : "high",
            app->detail.low_world ? "low" : "high",
            app->detail.low_res   ? "low" : "high");
}

int dxl_app_commit(dxl_app *app, dxl_err *err) {
    /* Order is the original's (docs/re/launch-flow.md section 5, steps 9-11):
     * sentinel first, then the FirstRun clamp, then the config write. */
    if (dxl_sentinel_create(&app->sentinel, err) != 0) {
        /* Not fatal: losing crash detection is worse than not starting? No --
         * the opposite. Log it and continue; a game that will not start
         * because a marker file failed is the worse outcome. */
        dxl_log("warning: %s", dxl_err_msg(err));
    }
    dxl_config_clamp_first_run(app->cfg);

    if (dxl_config_dirty(app->cfg) && dxl_config_save(app->cfg, err) != 0) {
        dxl_log("error: %s", dxl_err_msg(err));
        return -1;
    }
    /* CdPath is almost always "..\", which resolves back to the game dir; the
     * check is kept for installs that still point at a disc. */
    if (!dxl_install_cd_ok(app->game_dir, dxl_config_cd_path(app->cfg)))
        dxl_log("warning: CdPath check failed for '%s'",
                dxl_config_cd_path(app->cfg) ? dxl_config_cd_path(app->cfg) : "");
    return 0;
}

/* The game command is resolved relative to the launcher directory when it is
 * not absolute, so launcher.ini can say "./run-game.sh". */
static char *resolve_command(const dxl_app *app) {
    if (app->game_command[0] == '/') return dxl_xstrdup(app->game_command);
    const char *c = app->game_command;
    if (c[0] == '.' && c[1] == '/') c += 2;
    return dxl_path_join(app->app_dir, c);
}

int dxl_app_launch(dxl_app *app, dxl_err *err) {
    char *cmd = resolve_command(app);
    dxl_log("launching: %s %s", cmd, app->cmdline);

    /* The sentinel must survive into the game: it is deleted on clean exit by
     * whoever owns the run. Release the lock first so the game -- or a future
     * native engine -- can take it. */
    if (app->instance) { dxl_instance_release(app->instance); app->instance = NULL; }
    dxl_log_close();

    int rc = dxl_relaunch(cmd, app->cmdline, app->game_dir, err);
    free(cmd);
    return rc;   /* only here on failure */
}

int dxl_app_safe_relaunch(dxl_app *app, dxl_err *err) {
    if (app->safe.reset_config) {
        dxl_safe_reset_config(app->system_dir, app->package);
        dxl_log("safe mode: deleted %s.ini", app->package);
    }
    char *flags = dxl_safe_flags(&app->safe);
    dxl_log("safe mode: re-exec with '%s'", flags);

    if (app->instance) { dxl_instance_release(app->instance); app->instance = NULL; }
    dxl_log_close();

    int rc = dxl_relaunch(app->exe_path, flags, app->app_dir, err);
    free(flags);
    return rc;
}

void dxl_app_dry_run(dxl_app *app) {
    const dxl_decision *d = &app->decision;

    printf("launcher\n");
    printf("  exe            %s\n", app->exe_path);
    printf("  app dir        %s\n", app->app_dir);
    printf("  game dir       %s\n", app->game_dir);
    printf("  system dir     %s\n", app->system_dir);
    printf("  package        %s\n", app->package);
    printf("  game command   %s\n", app->game_command);
    printf("  command line   \"%s\"\n", app->cmdline);

    printf("\ninstall (%s)\n", app->install.ok ? "complete" : "INCOMPLETE");
    for (int i = 0; i < app->install.item_count; i++) {
        const dxl_install_item *it = &app->install.items[i];
        printf("  [%s] %-24s %s\n", it->found ? "ok" : "--",
               it->relative, it->found ? it->resolved : it->label);
    }

    printf("\ndecision\n");
    printf("  action         %s\n", dxl_action_name(d->action));
    printf("  screen         %s\n", dxl_screen_name(d->screen));
    printf("  caption key    %s\n", d->caption_key ? d->caption_key : "-");
    printf("  first run      %d (effective)\n", d->effective_first_run);
    printf("  migrate saves  %s\n", d->migrate_saves ? "yes" : "no");
    printf("  splash         %s\n", d->show_splash ? "shown" : "suppressed");
    printf("  skip handoff   %s\n", d->skip_handoff ? "yes" : "no");
    if (d->console_command[0]) printf("  console cmd    %s\n", d->console_command);
    if (d->test_rendev[0])     printf("  test rendev    %s\n", d->test_rendev);
    if (d->exec_file[0])       printf("  exec file      %s\n", d->exec_file);

    printf("\nconfig (%s)\n", dxl_config_path(app->cfg));
    printf("  FirstRun            %d\n", dxl_config_first_run(app->cfg));
    printf("  GameRenderDevice    %s\n",
           dxl_config_render_device(app->cfg) ? dxl_config_render_device(app->cfg) : "-");
    printf("  GameEngine          %s\n",
           dxl_config_game_engine(app->cfg) ? dxl_config_game_engine(app->cfg) : "-");
    printf("  CdPath              %s\n",
           dxl_config_cd_path(app->cfg) ? dxl_config_cd_path(app->cfg) : "-");
    printf("  Running.ini         %s\n",
           dxl_sentinel_exists(&app->sentinel) ? "present (crash pending)" : "absent");

    printf("\nrenderer candidates (%zu)\n", app->renderers.count);
    for (size_t i = 0; i < app->renderers.count; i++)
        printf("  %-32s %s%s\n", app->renderers.items[i].class_name,
               app->renderers.items[i].label,
               app->renderers.items[i].certified ? "" : "  (all-devices only)");

    printf("\nnothing was written.\n");
}
