/* Device probe: answers the questions the UI layer depends on.
 *
 *   - which SDL video driver actually initialises (vendor build has "mali")
 *   - what surface size we really get
 *   - which SDL_Renderer backend is available
 *   - what the TRIMUI Player1 pad reports, and whether SDL_GameController
 *     recognises it without a custom mapping
 *
 * Draws a few test rectangles, then logs input for ~12s or until a quit.
 */
#include <SDL.h>
#include <stdio.h>

int main(void) {
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
