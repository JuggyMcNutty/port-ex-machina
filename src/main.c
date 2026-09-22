/* Entry point.
 *
 * Mirrors LaunchWinMain's shape (docs/re/launch-flow.md): decide first, and
 * only bring up a display if there is actually something to ask. On a settled
 * install the launcher shows nothing at all and execs straight into the game
 * -- which is what the original does, and what anyone launching from the
 * spruceOS menu expects.
 */
#include "app.h"
#include "core/log.h"
#include "ui/screens.h"
#include "ui/ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The handoff timeout the original uses for its WM_COPYDATA
 * (SendMessageTimeout, 30 000 ms). Nothing should take that long; the limit
 * exists so a wedged primary cannot hang the messenger forever. */
#define HANDOFF_TIMEOUT_MS 30000

static int fail(const char *what, const dxl_err *e) {
    fprintf(stderr, "deusex-launcher: %s: %s\n", what, dxl_err_msg(e));
    dxl_log("fatal: %s: %s", what, dxl_err_msg(e));
    return 1;
}

int main(int argc, char **argv) {
    dxl_app app;
    dxl_err err;

    if (dxl_app_init(&app, argc, argv, &err) != 0)
        return fail("startup", &err);

    int rc = 0;

    switch (app.decision.action) {

    /* A second launch is only a messenger: hand the command line to the live
     * instance and exit without starting anything. */
    case DXL_ACTION_FORWARD:
        if (dxl_instance_forward(app.game_dir, app.cmdline,
                                 HANDOFF_TIMEOUT_MS, &err) != 0) {
            dxl_log("handoff failed: %s", dxl_err_msg(&err));
            /* The original gives up and exits rather than starting a second
             * copy; a failed handoff is not a reason to run two games. */
            rc = 1;
        } else {
            dxl_log("forwarded to running instance: \"%s\"", app.cmdline);
        }
        goto done;

    /* Both of these exit without ever launching. Neither is reachable from
     * the UI; they exist because the flags are part of the contract. */
    case DXL_ACTION_CONSOLE_CMD:
        dxl_log("console command requested: %s", app.decision.console_command);
        printf("%s\n", app.decision.console_command);
        fprintf(stderr, "deusex-launcher: -consolecommand needs a running "
                        "engine; nothing to run it against yet.\n");
        goto done;

    case DXL_ACTION_TEST_RENDEV:
        /* The original loads the render-device class, writes Detected.ini and
         * exits. Without an engine there is nothing to probe, so record the
         * request rather than fabricating a result. */
        dxl_log("render device probe requested: %s", app.decision.test_rendev);
        fprintf(stderr, "deusex-launcher: -testrendev needs a render device to "
                        "load; none is available yet.\n");
        goto done;

    case DXL_ACTION_SCREEN:
    case DXL_ACTION_LAUNCH:
        break;
    }

    /* Only now is a display needed -- and only if there is a question. */
    if (app.decision.screen != DXL_SCREEN_NONE || dxl_app_needs_install(&app)) {
        dxl_ui *ui = dxl_ui_init(&err);
        if (!ui) {
            /* No display is not automatically fatal: a complete install with
             * nothing to ask can still start. A missing install cannot. */
            if (dxl_app_needs_install(&app)) {
                fprintf(stderr, "deusex-launcher: game files not found under %s\n",
                        app.game_dir);
                for (int i = 0; i < app.install.item_count; i++)
                    if (!app.install.items[i].found)
                        fprintf(stderr, "  missing: %s\n",
                                app.install.items[i].relative);
                rc = fail("no display", &err);
                goto done;
            }
            dxl_log("no display (%s); continuing without the wizard",
                    dxl_err_msg(&err));
        } else {
            if (dxl_ui_headless(ui))
                dxl_log("warning: no usable font found; screens will be blank");

            dxl_session s;
            dxl_session_start(&s, &app, ui);
            while (!dxl_session_step(&s)) { /* one frame per iteration */ }

            dxl_scr end = s.screen;
            dxl_ui_quit(ui);

            if (end == DXL_SCR_QUIT) {
                /* Cancelling the wizard aborts startup, and leaves no
                 * sentinel behind: DoModal returning 0 means do not launch. */
                dxl_log("cancelled; not launching");
                goto done;
            }
            if (end == DXL_SCR_SAFE_EXEC) {
                if (dxl_app_safe_relaunch(&app, &err) != 0)
                    rc = fail("safe-mode relaunch", &err);
                goto done;
            }
        }
    }

    /* Sentinel, FirstRun clamp, config write -- in that order. */
    if (dxl_app_commit(&app, &err) != 0) { rc = fail("commit", &err); goto done; }

    if (dxl_app_launch(&app, &err) != 0) {
        /* exec failed, so we are still here and own the sentinel. Clear it:
         * the game never started, and a stale marker would greet the next
         * launch with a recovery screen for a crash that never happened. */
        dxl_sentinel_remove(&app.sentinel);
        rc = fail("launch", &err);
    }

done:
    dxl_app_shutdown(&app);
    return rc;
}
