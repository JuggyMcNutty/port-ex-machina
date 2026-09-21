/* The engine's configuration contract.
 *
 * Typed accessors over the game ini for exactly the keys docs/re/ini-keys.md
 * says the launcher reads and writes. Nothing else in this port touches the
 * ini directly, so this header is the complete list of what a launch can
 * change in someone's config.
 */
#ifndef DXL_CONFIG_H
#define DXL_CONFIG_H

#include "core/common.h"
#include "core/ini.h"

/* [FirstRun] FirstRun is an engine-version integer, not a boolean.
 * docs/re/wizard.md "The FirstRun version gates". 1100 was confirmed live to
 * be the engine version itself ("Init: Version: 1100"). */
#define DXL_FIRSTRUN_MIGRATE_BELOW 220
#define DXL_FIRSTRUN_WIZARD_BELOW  400
#define DXL_FIRSTRUN_CURRENT       1100

typedef struct dxl_config dxl_config;

/* Opens <system_dir>/<package>.ini. A missing file yields an empty config
 * rather than an error: the wizard can still run and will write one. */
dxl_config *dxl_config_open(const char *system_dir, const char *package);
void        dxl_config_free(dxl_config *c);

/* Writes only if something actually changed. */
int         dxl_config_save(dxl_config *c, dxl_err *err);
int         dxl_config_dirty(const dxl_config *c);
const char *dxl_config_path(const dxl_config *c);
dxl_ini    *dxl_config_ini(dxl_config *c);

int  dxl_config_first_run(const dxl_config *c);
/* Clamps up to DXL_FIRSTRUN_CURRENT, never down -- the original only ever
 * raises this value (0x1090B997). */
void dxl_config_clamp_first_run(dxl_config *c);

const char *dxl_config_render_device(const dxl_config *c);
void        dxl_config_set_render_device(dxl_config *c, const char *class_name);
const char *dxl_config_cd_path(const dxl_config *c);
const char *dxl_config_game_engine(const dxl_config *c);

/* [<render class>] DescFlags -- the output channel from device detection back
 * into the wizard. docs/re/ini-keys.md. */
int  dxl_config_desc_flags(const dxl_config *c, const char *render_class);
void dxl_config_set_desc_flags(dxl_config *c, const char *render_class, int flags);
const char *dxl_config_description(const dxl_config *c, const char *render_class);

/* The four detail choices. The names mirror the Startup.int [General] label
 * pairs the original Detail page shows: SoundLow/SoundHigh, SkinsLow/SkinsHigh,
 * WorldLow/WorldHigh, ResLow/ResHigh. */
typedef struct {
    int low_sound;   /* SoundLow  vs SoundHigh  */
    int low_skins;   /* SkinsLow  vs SkinsHigh  */
    int low_world;   /* WorldLow  vs WorldHigh  */
    int low_res;     /* ResLow    vs ResHigh    */
} dxl_detail;

/* Writes the whole detail block from docs/re/ini-keys.md "Detail
 * auto-configuration". render_class selects the MinDesiredFrameRate rule. */
void dxl_config_apply_detail(dxl_config *c, const dxl_detail *d,
                             const char *render_class);

/* The original's own defaults, as a starting point for the Detail screen:
 * low sound when the machine lacks MMX or has <= 64 MB (0x1090EDA1). Here the
 * equivalent test is made by the caller and passed in. */
void dxl_detail_defaults(dxl_detail *d, int weak_machine);

#endif
