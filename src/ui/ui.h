/* SDL2 frontend: window, input and the drawing primitives the screens need.
 *
 * Deliberately small. Six screens, all of them a title, some prose and a list
 * of choices, so the widget vocabulary is a label, a wrapped paragraph, a
 * menu row and a footer hint. There is no layout engine and no retained
 * widget tree; each screen draws itself every frame from the app state.
 */
#ifndef DXL_UI_H
#define DXL_UI_H

#include "core/common.h"
#include "theme.h"

#include <SDL.h>

typedef enum {
    DXL_ACT_NONE,
    DXL_ACT_UP, DXL_ACT_DOWN, DXL_ACT_LEFT, DXL_ACT_RIGHT,
    DXL_ACT_CONFIRM, DXL_ACT_BACK,
    DXL_ACT_QUIT
} dxl_act;

typedef enum {
    DXL_TXT_TITLE,
    DXL_TXT_BODY,
    DXL_TXT_ITEM,
    DXL_TXT_FOOTER
} dxl_txt;

typedef struct dxl_ui dxl_ui;

dxl_ui *dxl_ui_init(dxl_err *err);
void    dxl_ui_quit(dxl_ui *ui);

int  dxl_ui_width (const dxl_ui *ui);
int  dxl_ui_height(const dxl_ui *ui);
const dxl_metrics *dxl_ui_metrics(const dxl_ui *ui);

/* Drains SDL's queue and returns the next abstract action, or DXL_ACT_NONE.
 * Auto-repeat on the d-pad and stick is handled here so the screens do not
 * each reinvent it. */
dxl_act dxl_ui_poll(dxl_ui *ui);

void dxl_ui_frame_begin(dxl_ui *ui);
void dxl_ui_frame_end(dxl_ui *ui);

void dxl_ui_fill(dxl_ui *ui, SDL_Rect r, SDL_Color c);
void dxl_ui_text(dxl_ui *ui, int x, int y, const char *s, dxl_txt style, SDL_Color c);
int  dxl_ui_text_width(dxl_ui *ui, const char *s, dxl_txt style);
int  dxl_ui_line_height(dxl_ui *ui, dxl_txt style);

/* Word-wraps s into max_w, drawing from (x,y). Returns the y below the last
 * line, so callers can stack blocks without measuring twice. */
int  dxl_ui_paragraph(dxl_ui *ui, int x, int y, int max_w, const char *s,
                      dxl_txt style, SDL_Color c);

/* The standard furniture: amber title with a rule under it, and a footer of
 * button hints. Returns the y where content may start. */
int  dxl_ui_header(dxl_ui *ui, const char *title);
void dxl_ui_footer(dxl_ui *ui, const char *hints);

/* One menu row. selected draws the amber bar; value is the right-aligned
 * secondary text (a toggle state, a class name) and may be NULL. */
void dxl_ui_row(dxl_ui *ui, int y, const char *label, const char *value,
                int selected, int enabled);

/* True when no font could be loaded; the caller falls back to stderr rather
 * than drawing an empty screen. */
int  dxl_ui_headless(const dxl_ui *ui);

#endif
