/* Localisation: the engine's .int files.
 *
 * System/Startup.int is the launcher's own string table -- it names every
 * wizard page and every control on it. Nothing user-visible is hardcoded here;
 * if a string is missing the caller supplies a fallback, so a stripped install
 * degrades instead of crashing.
 *
 * .int files are ini files, so this is a thin layer over dxl_ini: package
 * naming, plus the quoting convention (WorldHigh="High detail textures").
 */
#ifndef DXL_STRINGS_H
#define DXL_STRINGS_H

#include "core/common.h"

typedef struct dxl_strings dxl_strings;

/* Loads <dir>/<package>.int. Returns an empty, usable object if the file is
 * absent -- a missing localisation file is not a reason to fail to start. */
dxl_strings *dxl_strings_load(const char *dir, const char *package);
void         dxl_strings_free(dxl_strings *s);

/* Was a file actually found? Callers use this to warn, not to abort. */
int          dxl_strings_present(const dxl_strings *s);
const char  *dxl_strings_path(const dxl_strings *s);

/* Surrounding double quotes are stripped, as the engine does. Returns
 * fallback when the key is absent. */
const char *dxl_strings_get(const dxl_strings *s, const char *section,
                            const char *key, const char *fallback);

#endif
