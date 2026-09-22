/* Device probe: answers the questions the UI layer depends on.
 *
 *   - which SDL video driver actually initialises (vendor build has "mali")
 *   - what surface size we really get
 *   - which SDL_Renderer backend is available
 *   - what the TRIMUI Player1 pad reports, and whether SDL_GameController
 *     recognises it without a custom mapping
 *
 * Draws a few test rectangles, then logs input for ~12s or until a quit.
 *
 * --pad [seconds] instead records every controller event (default 30s) and
 * ends with a summary: each button seen, and for each axis its range and how
 * many distinct values it reported. That summary is what tells an analog
 * trigger (thousands of values) from a digital one wired as an axis (two).
 */
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int seen, min, max, distinct;
    unsigned char hit[65536 / 8];   /* one bit per possible value */
} axis_stat;

static void axis_note(axis_stat *a, int v) {
    unsigned idx = (unsigned)(v + 32768);
    if (!a->seen) { a->min = a->max = v; a->seen = 1; }
    if (v < a->min) a->min = v;
    if (v > a->max) a->max = v;
    if (!(a->hit[idx / 8] & (1u << (idx % 8)))) {
        a->hit[idx / 8] |= (unsigned char)(1u << (idx % 8));
        a->distinct++;
    }
}

static int pad_mode(int seconds) {
    /* No video: the pad is read straight from evdev, and leaving the display
     * alone means this can run while the spruceOS menu is paused on screen. */
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("SDL_Init FAILED: %s\n", SDL_GetError());
        return 1;
    }

    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        printf("joystick [%d] \"%s\" gamecontroller=%s\n", i,
               SDL_JoystickNameForIndex(i), SDL_IsGameController(i) ? "yes" : "NO");
        if (SDL_IsGameController(i)) {
            SDL_GameController *gc = SDL_GameControllerOpen(i);
            char *m = gc ? SDL_GameControllerMapping(gc) : NULL;
            if (m) { printf("  mapping: %s\n", m); SDL_free(m); }
        } else {
            SDL_JoystickOpen(i);
        }
    }
    printf("recording for %ds -- press every button, sweep both sticks and "
           "both triggers slowly\n", seconds);
    fflush(stdout);

    static axis_stat caxes[SDL_CONTROLLER_AXIS_MAX];
    static axis_stat jaxes[16];
    int cbtn[SDL_CONTROLLER_BUTTON_MAX] = {0};
    int jbtn[64] = {0};
    int hats = 0;

    Uint32 start = SDL_GetTicks();
    while (SDL_GetTicks() - start < (Uint32)seconds * 1000) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                printf("%6u  controller %-13s %s\n", e.cbutton.timestamp,
                       SDL_GameControllerGetStringForButton(e.cbutton.button),
                       e.type == SDL_CONTROLLERBUTTONDOWN ? "down" : "up");
                if (e.cbutton.button < SDL_CONTROLLER_BUTTON_MAX)
                    cbtn[e.cbutton.button]++;
                break;
            case SDL_CONTROLLERAXISMOTION:
                if (e.caxis.axis < SDL_CONTROLLER_AXIS_MAX) {
                    axis_stat *a = &caxes[e.caxis.axis];
                    int before = a->distinct;
                    axis_note(a, e.caxis.value);
                    /* Log sparsely: a stick sweep reports hundreds of values. */
                    if (a->distinct != before && (a->distinct < 4 || a->distinct % 64 == 0))
                        printf("%6u  controller axis %-13s %d\n", e.caxis.timestamp,
                               SDL_GameControllerGetStringForAxis(e.caxis.axis),
                               e.caxis.value);
                }
                break;
            case SDL_JOYBUTTONDOWN:
                printf("%6u  raw button %d down\n", e.jbutton.timestamp, e.jbutton.button);
                if (e.jbutton.button < 64) jbtn[e.jbutton.button]++;
                break;
            case SDL_JOYAXISMOTION:
                if (e.jaxis.axis < 16) axis_note(&jaxes[e.jaxis.axis], e.jaxis.value);
                break;
            case SDL_JOYHATMOTION:
                printf("%6u  raw hat %d value %d\n", e.jhat.timestamp, e.jhat.hat, e.jhat.value);
                hats++;
                break;
            }
            fflush(stdout);
        }
        SDL_Delay(5);
    }

    printf("\n--- summary ---\ncontroller buttons seen:");
    for (int b = 0; b < SDL_CONTROLLER_BUTTON_MAX; b++)
        if (cbtn[b]) printf(" %s", SDL_GameControllerGetStringForButton(b));
    printf("\nraw buttons seen:");
    for (int b = 0; b < 64; b++) if (jbtn[b]) printf(" %d", b);
    printf("\nhat events: %d\n", hats);
    for (int a = 0; a < SDL_CONTROLLER_AXIS_MAX; a++)
        if (caxes[a].seen)
            printf("controller axis %-13s min %6d max %6d distinct %5d\n",
                   SDL_GameControllerGetStringForAxis(a), caxes[a].min,
                   caxes[a].max, caxes[a].distinct);
    for (int a = 0; a < 16; a++)
        if (jaxes[a].seen)
            printf("raw axis %-2d min %6d max %6d distinct %5d\n", a,
                   jaxes[a].min, jaxes[a].max, jaxes[a].distinct);
    SDL_Quit();
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--pad") == 0)
        return pad_mode(argc > 2 ? atoi(argv[2]) : 30);

    printf("compiled against SDL %d.%d.%d\n",
           SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
    SDL_version rt; SDL_GetVersion(&rt);
    printf("linked against   SDL %d.%d.%d\n", rt.major, rt.minor, rt.patch);

    int nvd = SDL_GetNumVideoDrivers();
    printf("video drivers (%d):", nvd);
    for (int i = 0; i < nvd; i++) printf(" %s", SDL_GetVideoDriver(i));
    printf("\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("SDL_Init FAILED: %s\n", SDL_GetError());
        return 1;
    }
    printf("chosen video driver: %s\n", SDL_GetCurrentVideoDriver());

    SDL_DisplayMode dm;
    if (SDL_GetCurrentDisplayMode(0, &dm) == 0)
        printf("display mode: %dx%d @%dHz fmt=%s\n",
               dm.w, dm.h, dm.refresh_rate, SDL_GetPixelFormatName(dm.format));

    int nrd = SDL_GetNumRenderDrivers();
    printf("render drivers (%d):", nrd);
    for (int i = 0; i < nrd; i++) {
        SDL_RendererInfo ri;
        if (SDL_GetRenderDriverInfo(i, &ri) == 0) printf(" %s", ri.name);
    }
    printf("\n");

    SDL_Window *w = SDL_CreateWindow("dxl probe", SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, 1280, 720,
                                     SDL_WINDOW_FULLSCREEN);
    if (!w) { printf("CreateWindow FAILED: %s\n", SDL_GetError()); SDL_Quit(); return 1; }
    int ww, wh; SDL_GetWindowSize(w, &ww, &wh);
    printf("window size: %dx%d\n", ww, wh);

    SDL_Renderer *r = SDL_CreateRenderer(w, -1, 0);
    if (!r) { printf("CreateRenderer FAILED: %s\n", SDL_GetError()); SDL_Quit(); return 1; }
    SDL_RendererInfo ri; SDL_GetRendererInfo(r, &ri);
    printf("renderer in use: %s (max tex %dx%d, flags 0x%x)\n",
           ri.name, ri.max_texture_width, ri.max_texture_height, ri.flags);
    int ow, oh; SDL_GetRendererOutputSize(r, &ow, &oh);
    printf("renderer output: %dx%d\n", ow, oh);

    int nj = SDL_NumJoysticks();
    printf("joysticks: %d\n", nj);
    for (int i = 0; i < nj; i++) {
        SDL_Joystick *j = SDL_JoystickOpen(i);
        if (!j) continue;
        char guid[64];
        SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(j), guid, sizeof guid);
        printf("  [%d] \"%s\" guid=%s axes=%d buttons=%d hats=%d gamecontroller=%s\n",
               i, SDL_JoystickName(j), guid, SDL_JoystickNumAxes(j),
               SDL_JoystickNumButtons(j), SDL_JoystickNumHats(j),
               SDL_IsGameController(i) ? "yes" : "NO");
        if (SDL_IsGameController(i)) {
            char *m = SDL_GameControllerMappingForGUID(SDL_JoystickGetGUID(j));
            if (m) { printf("      mapping: %s\n", m); SDL_free(m); }
            SDL_GameControllerOpen(i);
        }
    }
    fflush(stdout);

    const SDL_Color bars[] = {{200,40,40,255},{40,200,40,255},{40,40,200,255},{230,230,230,255}};
    Uint32 start = SDL_GetTicks();
    int frame = 0, running = 1;
    while (running && SDL_GetTicks() - start < 12000) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT: running = 0; break;
            case SDL_JOYBUTTONDOWN:
                printf("JOYBUTTONDOWN  which=%d button=%d\n", e.jbutton.which, e.jbutton.button);
                fflush(stdout); break;
            case SDL_JOYHATMOTION:
                printf("JOYHATMOTION   hat=%d value=%d\n", e.jhat.hat, e.jhat.value);
                fflush(stdout); break;
            case SDL_JOYAXISMOTION:
                if (e.jaxis.value > 16000 || e.jaxis.value < -16000) {
                    printf("JOYAXISMOTION  axis=%d value=%d\n", e.jaxis.axis, e.jaxis.value);
                    fflush(stdout);
                }
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                printf("CONTROLLERBUTTONDOWN %s\n",
                       SDL_GameControllerGetStringForButton(e.cbutton.button));
                fflush(stdout); break;
            case SDL_KEYDOWN:
                printf("KEYDOWN        %s\n", SDL_GetKeyName(e.key.keysym.sym));
                fflush(stdout);
                if (e.key.keysym.sym == SDLK_ESCAPE) running = 0;
                break;
            }
        }
        SDL_SetRenderDrawColor(r, 18, 18, 24, 255);
        SDL_RenderClear(r);
        for (int i = 0; i < 4; i++) {
            SDL_Rect q = { 40 + i * (ow - 80) / 4, oh / 3, (ow - 80) / 4 - 20, oh / 3 };
            SDL_SetRenderDrawColor(r, bars[i].r, bars[i].g, bars[i].b, 255);
            SDL_RenderFillRect(r, &q);
        }
        /* a moving bar, so a frozen frame is distinguishable from a live one */
        SDL_Rect m = { (frame * 6) % ow, 20, 60, 20 };
        SDL_SetRenderDrawColor(r, 255, 200, 0, 255);
        SDL_RenderFillRect(r, &m);
        SDL_RenderPresent(r);
        frame++;
    }
    printf("rendered %d frames in %ums\n", frame, SDL_GetTicks() - start);
    SDL_DestroyRenderer(r); SDL_DestroyWindow(w); SDL_Quit();
    return 0;
}
