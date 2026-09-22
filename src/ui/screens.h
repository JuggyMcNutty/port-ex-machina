/* The screens, and the navigation between them.
 *
 * Six screens standing in for the original's six wizard pages
 * (docs/re/wizard.md "Page graph"). The decisions are the same; the layout is
 * not, because a mouse-driven Win32 wizard is the wrong shape for a d-pad and
 * a 1280x720 panel held in two hands.
 *
 * Divergences from the page graph, all deliberate:
 *   - Driver (2022) is folded into the renderer screen. It existed only to
 *     show the detected Direct3D card name, which has no meaning here.
 *   - FirstTime (2019) is shown only when the entry really was a first run.
 *     The original reaches it from Detail unconditionally, so -changevideo
 *     greeted you with "Deus Ex is starting up for the first time".
 *   - The Web button cannot open a browser, so it shows the URL instead of
 *     pretending to.
 *   - An install screen is new; see docs/DESIGN.md.
 */
#ifndef DXL_SCREENS_H
#define DXL_SCREENS_H

#include "app.h"
#include "ui.h"

typedef enum {
    DXL_SCR_INSTALL,
    DXL_SCR_MAIN,
    DXL_SCR_RENDERER,
    DXL_SCR_DETAIL,
    DXL_SCR_SAFEOPTIONS,
    DXL_SCR_FIRSTRUN,
    /* terminal states */
    DXL_SCR_LAUNCH,     /* commit, then exec the game */
    DXL_SCR_SAFE_EXEC,  /* re-exec the launcher with safe flags */
    DXL_SCR_QUIT        /* leave without launching */
} dxl_scr;

typedef struct {
    dxl_app *app;
    dxl_ui  *ui;

    dxl_scr screen;
    int     cursor;

    /* Whether this session arrived through the first-run gate rather than
     * -changevideo or the menu. Decides if the greeting page appears and
     * whether Back can leave the renderer screen. */
    int from_first_run;
    int came_from_menu;

    char notice[320];
} dxl_session;

void dxl_session_start(dxl_session *s, dxl_app *app, dxl_ui *ui);
/* One frame. Returns 1 once a terminal state is reached. */
int  dxl_session_step(dxl_session *s);
/* True if the session ended in a state that still needs no UI. */
int  dxl_session_finished(const dxl_session *s);

#endif
