/* The renderer candidate list.
 *
 * The original enumerates Win32 3D devices, re-executing itself with
 * -testrendev= per candidate so a crashing driver only kills a child
 * (docs/re/wizard.md, Renderer page). There is no Win32 here and no engine to
 * detect against yet, so the candidates are declared in renderers.ini instead.
 *
 * What does NOT change is the output: the chosen class name is written to
 * [Engine.Engine] GameRenderDevice exactly as before. That key is the only
 * channel the wizard has to the engine, so keeping it honest matters more
 * than how the list was produced.
 *
 * Descriptions come from Startup.int [Descriptions] when the class is one the
 * game knows about, so the screen reads like the original where it can.
 */
#ifndef DXL_RENDERERS_H
#define DXL_RENDERERS_H

#include "core/common.h"
#include "core/strings.h"

typedef struct {
    char *class_name;    /* e.g. "OpenGLDrv.OpenGLRenderDevice" */
    char *label;         /* short name for the list */
    char *description;   /* prose for the detail pane */
    int   certified;     /* shown under "compatible" rather than "all" */
} dxl_renderer;

typedef struct {
    dxl_renderer *items;
    size_t        count;
} dxl_renderer_list;

/* Reads path; descriptions are filled from startup when absent. A missing or
 * empty file yields an empty list -- the caller decides what to do about it
 * rather than being handed a fabricated default. */
void dxl_renderers_load(dxl_renderer_list *out, const char *path,
                        const dxl_strings *startup);
void dxl_renderers_free(dxl_renderer_list *l);

/* Index of class_name, or -1. Case-insensitive, like every other class
 * comparison the engine makes. */
int dxl_renderers_find(const dxl_renderer_list *l, const char *class_name);

#endif
