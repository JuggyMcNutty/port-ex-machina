#include "core/renderers.h"
#include "core/ini.h"

#include <stdlib.h>
#include <string.h>

void dxl_renderers_load(dxl_renderer_list *out, const char *path,
                        const dxl_strings *startup) {
    memset(out, 0, sizeof *out);

    dxl_ini *ini = dxl_ini_load(path, NULL);
    if (!ini) return;

    size_t n = dxl_ini_get_all(ini, "Renderers", "Device", NULL, 0);
    if (n == 0) { dxl_ini_free(ini); return; }

    const char **names = dxl_xmalloc(n * sizeof *names);
    dxl_ini_get_all(ini, "Renderers", "Device", names, n);

    out->items = dxl_xmalloc(n * sizeof *out->items);
    for (size_t i = 0; i < n; i++) {
        const char *cls = names[i];
        if (!cls || !*cls) continue;

        dxl_renderer *r = &out->items[out->count++];
        memset(r, 0, sizeof *r);
        r->class_name = dxl_xstrdup(cls);

        const char *label = dxl_ini_get(ini, cls, "Label");
        r->label = dxl_xstrdup(label ? label : cls);
        r->certified = dxl_ini_get_bool(ini, cls, "Certified", 1);

        /* Prefer the engine's own wording; fall back to anything the
         * renderers file supplies, then to nothing. */
        const char *desc = dxl_strings_get(startup, "Descriptions", cls, NULL);
        if (!desc) desc = dxl_ini_get(ini, cls, "Description");
        r->description = dxl_xstrdup(desc ? desc : "");
    }

    free(names);
    dxl_ini_free(ini);
}

void dxl_renderers_free(dxl_renderer_list *l) {
    if (!l) return;
    for (size_t i = 0; i < l->count; i++) {
        free(l->items[i].class_name);
        free(l->items[i].label);
        free(l->items[i].description);
    }
    free(l->items);
    memset(l, 0, sizeof *l);
}

int dxl_renderers_find(const dxl_renderer_list *l, const char *class_name) {
    if (!class_name) return -1;
    for (size_t i = 0; i < l->count; i++)
        if (dxl_stricmp(l->items[i].class_name, class_name) == 0) return (int)i;
    return -1;
}
