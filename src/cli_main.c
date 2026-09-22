/* Headless driver.
 *
 * The launcher's entire contract with no display attached: what would it
 * decide, and what would it write. This exists so the ini/flag/sentinel
 * behaviour can be verified on the device -- over SSH, before the SDL
 * frontend is in the picture -- and so a broken install can be diagnosed
 * without a screen.
 */
#include "app.h"
#include "core/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *argv0) {
    printf(
        "usage: %s [--dry-run | --safe-flags | --help] [launcher flags...]\n"
        "\n"
        "  --dry-run      print the decision and the config as it stands; write nothing\n"
        "  --safe-flags   print the flag string for the safe-mode boxes given below\n"
        "  --help         this text\n"
        "\n"
        "Launcher flags are passed through and parsed exactly as the original does\n"
        "(-safe, -firstrun, -changevideo, -log, -consolecommand=, -testrendev=, ...).\n"
        "\n"
        "safe-flags boxes: nosound no3dsound no3dvideo window res resetconfig\n"
        "                  noprocessor nojoy\n",
        argv0);
}

/* --safe-flags exists to make the eight-checkbox fix observable from a shell:
 * name boxes, see exactly the flags they produce. The original's bug was that
 * five of them produced the same thing. */
static int safe_flags_main(int argc, char **argv) {
    dxl_safe_options o = {0};
    for (int i = 2; i < argc; i++) {
        const char *a = argv[i];
        if      (!strcmp(a, "nosound"))     o.no_sound     = 1;
        else if (!strcmp(a, "no3dsound"))   o.no_3d_sound  = 1;
        else if (!strcmp(a, "no3dvideo"))   o.no_3d_video  = 1;
        else if (!strcmp(a, "window"))      o.windowed     = 1;
        else if (!strcmp(a, "res"))         o.default_res  = 1;
        else if (!strcmp(a, "resetconfig")) o.reset_config = 1;
        else if (!strcmp(a, "noprocessor")) o.no_processor = 1;
        else if (!strcmp(a, "nojoy"))       o.no_joy       = 1;
        else { fprintf(stderr, "unknown box: %s\n", a); return 2; }
    }
    char *f = dxl_safe_flags(&o);
    printf("%s\n", f);
    if (o.reset_config) printf("(and delete <Package>.ini)\n");
    free(f);
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "--help"))       { usage(argv[0]); return 0; }
    if (argc > 1 && !strcmp(argv[1], "--safe-flags")) return safe_flags_main(argc, argv);

    int dry = (argc > 1 && !strcmp(argv[1], "--dry-run"));
    if (!dry) { usage(argv[0]); return 2; }

    /* A dry run must not touch the install -- not even to write a log line
     * saying it did nothing. */
    dxl_log_set_echo(0);
    dxl_log_set_to_file(0);

    /* Hide our own option from the launcher's command line, so --dry-run does
     * not accidentally become part of what gets parsed or forwarded. */
    char *filtered[64];
    int n = 0;
    filtered[n++] = argv[0];
    for (int i = 2; i < argc && n < 63; i++) filtered[n++] = argv[i];
    filtered[n] = NULL;

    dxl_app app;
    dxl_err err;
    if (dxl_app_init(&app, n, filtered, &err) != 0) {
        fprintf(stderr, "init failed: %s\n", dxl_err_msg(&err));
        return 1;
    }
    dxl_app_dry_run(&app);
    dxl_app_shutdown(&app);
    return 0;
}
