/* Renders every screen to a .bmp, so layout can be reviewed without a
 * handheld in reach. Uses SDL's offscreen/dummy driver, so it runs headless.
 *
 * This is a development tool, not part of the launcher: it fabricates the
 * session state each screen needs and drives the same draw code the real
 * frontend does, which is the point -- a screenshot of different code would
 * prove nothing.
 */
#include "app.h"
#include "ui/screens.h"
#include "ui/ui.h"
#include "core/log.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* dxl_ui keeps its internals private, so grab the renderer back out of SDL
 * rather than widening the header for a tool. */
static SDL_Renderer *renderer_of_current_window(void) {
    for (int id = 1; id < 64; id++) {
        SDL_Window *w = SDL_GetWindowFromID((Uint32)id);
        if (w) {
            SDL_Renderer *r = SDL_GetRenderer(w);
            if (r) return r;
        }
    }
    return NULL;
}

static void save(SDL_Renderer *r, const char *dir, const char *name) {
    if (!r) return;
    int w, h;
    SDL_GetRendererOutputSize(r, &w, &h);
    SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32,
                                                    SDL_PIXELFORMAT_RGB888);
    if (!s) return;
    /* The read-back happens after a present, so on a double-buffered driver
     * whatever the buffer holds is the driver's business. Start from black so
     * an unwritten row reads as background rather than as garbage. */
    SDL_FillRect(s, NULL, SDL_MapRGB(s->format, 10, 10, 12));
    if (SDL_RenderReadPixels(r, NULL, SDL_PIXELFORMAT_RGB888,
                             s->pixels, s->pitch) == 0) {
        char path[512];
        snprintf(path, sizeof path, "%s/%s.bmp", dir, name);
        SDL_SaveBMP(s, path);
        printf("  %s (%dx%d)\n", path, w, h);
    } else {
        printf("  read-back failed for %s: %s\n", name, SDL_GetError());
    }
    SDL_FreeSurface(s);
}

int main(int argc, char **argv) {
    const char *outdir = (argc > 1) ? argv[1] : ".";

    /* Offscreen first; dummy cannot read pixels back but is a useful
     * smoke test if offscreen is missing. */
    if (!SDL_getenv("SDL_VIDEODRIVER")) SDL_setenv("SDL_VIDEODRIVER", "offscreen", 1);

    dxl_log_set_echo(0);
    dxl_log_set_to_file(0);

    dxl_app app;
    dxl_err err;
    if (dxl_app_init(&app, argc > 2 ? argc - 1 : 1, argv, &err) != 0) {
        fprintf(stderr, "app init: %s\n", dxl_err_msg(&err));
        return 1;
    }
    dxl_ui *ui = dxl_ui_init(&err);
    if (!ui) { fprintf(stderr, "ui init: %s\n", dxl_err_msg(&err)); return 1; }
    if (dxl_ui_headless(ui)) fprintf(stderr, "warning: no font; shots will be blank\n");

    SDL_Renderer *r = renderer_of_current_window();
    printf("rendering to %s\n", outdir);

    dxl_session s;
    dxl_session_start(&s, &app, ui);

    struct { dxl_scr scr; const char *name; int cursor; } shots[] = {
        { DXL_SCR_INSTALL,     "01-install",          0 },
        { DXL_SCR_MAIN,        "02-main",             0 },
        { DXL_SCR_RENDERER,    "03-renderer",         0 },
        { DXL_SCR_DETAIL,      "04-detail",           0 },
        { DXL_SCR_SAFEOPTIONS, "05-safeoptions",      2 },
        { DXL_SCR_FIRSTRUN,    "06-firstrun",         0 },
    };

    for (size_t i = 0; i < sizeof shots / sizeof *shots; i++) {
        s.screen = shots[i].scr;
        s.cursor = shots[i].cursor;
        s.came_from_menu = 1;
        s.from_first_run = 1;
        if (shots[i].scr == DXL_SCR_SAFEOPTIONS) {
            /* Tick the box the original's bug made indistinguishable, so the
             * shot shows the flag line for exactly one choice. */
            memset(&app.safe, 0, sizeof app.safe);
            app.safe.no_3d_video = 1;
        }
        if (shots[i].scr == DXL_SCR_MAIN)
            app.decision.screen = DXL_SCREEN_MAIN_RECOVERY;

        /* Two frames through the real draw path: with double buffering the
         * buffer we read back is the one presented a frame earlier. */
        dxl_session_step(&s);
        dxl_session_step(&s);
        save(r, outdir, shots[i].name);
    }

    dxl_ui_quit(ui);
    dxl_app_shutdown(&app);
    return 0;
}
