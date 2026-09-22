/* The launcher session.
 *
 * Everything the launcher does, in the order InitEngine does it, with no UI
 * dependency. The frontend calls dxl_app_init, looks at app->decision to see
 * which screen (if any) to show, mutates the config through the core API as
 * the user makes choices, and finishes with dxl_app_commit + dxl_app_launch.
 *
 * Keeping this UI-free is what lets --dry-run exercise the entire contract
 * without a display, which is how the on-device behaviour gets verified
 * before any pixel is drawn.
 */
#ifndef DXL_APP_H
#define DXL_APP_H

#include "core/common.h"
#include "core/config.h"
#include "core/install.h"
#include "core/instance.h"
#include "core/policy.h"
#include "core/relaunch.h"
#include "core/renderers.h"
#include "core/sentinel.h"
#include "core/strings.h"

typedef struct {
    /* Where things are */
    char *app_dir;        /* the launcher's own directory */
    char *exe_path;       /* for the safe-mode re-exec */
    char *game_dir;       /* where the user put the game files */
    char *system_dir;     /* <game_dir>/System, case-resolved */
    char *package;        /* "DeusEx" -- names the ini and the log */
    char *game_command;   /* what to exec once configuration is settled */

    char *cmdline;        /* argv[1..] joined */

    dxl_config       *cfg;
    dxl_strings      *startup;     /* Startup.int */
    dxl_renderer_list renderers;
    dxl_install       install;
    dxl_sentinel      sentinel;
    dxl_instance     *instance;    /* NULL when another instance holds it */
    dxl_decision      decision;

    /* Filled by the frontend as the user chooses. */
    dxl_detail        detail;
    dxl_safe_options  safe;
} dxl_app;

/* Resolves paths, reads launcher.ini, loads config and strings, probes the
 * install, handles the single-instance question and computes the decision.
 * Returns 0 on success. */
int  dxl_app_init(dxl_app *app, int argc, char **argv, dxl_err *err);
void dxl_app_shutdown(dxl_app *app);

/* True when the game files are not (yet) where they should be. The frontend
 * shows the install screen and nothing else. */
int  dxl_app_needs_install(const dxl_app *app);

/* Writes GameRenderDevice. */
void dxl_app_choose_renderer(dxl_app *app, const char *class_name);
/* Writes the detail block for the current renderer. */
void dxl_app_apply_detail(dxl_app *app);

/* Post-wizard, pre-launch: create the sentinel, clamp FirstRun, save config.
 * Order matters and is the original's. */
int  dxl_app_commit(dxl_app *app, dxl_err *err);

/* Replaces this process with the game. Only returns on failure. */
int  dxl_app_launch(dxl_app *app, dxl_err *err);

/* Safe mode: assemble flags from app->safe, optionally reset the config, and
 * re-exec the launcher. Only returns on failure. */
int  dxl_app_safe_relaunch(dxl_app *app, dxl_err *err);

/* Prints the whole decision and the pending config changes without touching
 * anything. */
void dxl_app_dry_run(dxl_app *app);

#endif
