#include "core/strings.h"
#include "core/ini.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct dxl_strings {
    dxl_ini *ini;
    char    *path;
    int      present;
    /* Unquoted values are cached so dxl_strings_get can return a stable
     * pointer without handing out a fresh allocation on every call. */
    char   **cache;
    size_t   cache_len, cache_cap;
};

dxl_strings *dxl_strings_load(const char *dir, const char *package) {
    dxl_strings *s = dxl_xmalloc(sizeof *s);
    memset(s, 0, sizeof *s);

    size_t n = strlen(dir) + strlen(package) + 8;
    s->path = dxl_xmalloc(n);
    snprintf(s->path, n, "%s/%s.int", dir, package);

    s->ini = dxl_ini_load(s->path, NULL);
    if (s->ini) {
        s->present = 1;
    } else {
        s->ini = dxl_ini_new();
    }
    return s;
}

void dxl_strings_free(dxl_strings *s) {
    if (!s) return;
    dxl_ini_free(s->ini);
    for (size_t i = 0; i < s->cache_len; i++) free(s->cache[i]);
    free(s->cache);
    free(s->path);
    free(s);
}

int dxl_strings_present(const dxl_strings *s) { return s ? s->present : 0; }
const char *dxl_strings_path(const dxl_strings *s) { return s ? s->path : ""; }

static const char *cache_put(dxl_strings *s, char *owned) {
    if (s->cache_len == s->cache_cap) {
        s->cache_cap = s->cache_cap ? s->cache_cap * 2 : 16;
        s->cache = dxl_xrealloc(s->cache, s->cache_cap * sizeof *s->cache);
    }
    s->cache[s->cache_len++] = owned;
    return owned;
}

const char *dxl_strings_get(const dxl_strings *s, const char *section,
                            const char *key, const char *fallback) {
    if (!s) return fallback;
    const char *v = dxl_ini_get(s->ini, section, key);
    if (!v) return fallback;

    size_t len = strlen(v);
    if (len >= 2 && v[0] == '"' && v[len - 1] == '"') {
        /* Cast away const: the cache is an implementation detail of an object
         * the caller owns, and it keeps the returned pointer stable. */
        dxl_strings *mut = (dxl_strings *)s;
        return cache_put(mut, dxl_xstrndup(v + 1, len - 2));
    }
    return v;
}
