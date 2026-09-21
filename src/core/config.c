#include "core/config.h"
#include "core/paths.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct dxl_config {
    dxl_ini *ini;
    char    *path;
    int      existed;
};

dxl_config *dxl_config_open(const char *system_dir, const char *package) {
    dxl_config *c = dxl_xmalloc(sizeof *c);
    memset(c, 0, sizeof *c);

    size_t n = strlen(system_dir) + strlen(package) + 8;
    c->path = dxl_xmalloc(n);
    snprintf(c->path, n, "%s/%s.ini", system_dir, package);

    c->ini = dxl_ini_load(c->path, NULL);
    if (c->ini) {
        c->existed = 1;
    } else {
        /* Try the shipped-case variant before giving up: the ini is named
         * after the package, and installs vary in case. */
        char *found = dxl_path_resolve_ci(system_dir, dxl_path_basename(c->path));
        if (found) {
            c->ini = dxl_ini_load(found, NULL);
            if (c->ini) { free(c->path); c->path = found; c->existed = 1; }
            else free(found);
        }
    }
    if (!c->ini) c->ini = dxl_ini_new();
    return c;
}

void dxl_config_free(dxl_config *c) {
    if (!c) return;
    dxl_ini_free(c->ini);
    free(c->path);
    free(c);
}

int dxl_config_dirty(const dxl_config *c) { return dxl_ini_dirty(c->ini); }
const char *dxl_config_path(const dxl_config *c) { return c->path; }
dxl_ini *dxl_config_ini(dxl_config *c) { return c->ini; }

int dxl_config_save(dxl_config *c, dxl_err *err) {
    if (!dxl_ini_dirty(c->ini) && c->existed) return 0;
    return dxl_ini_save(c->ini, c->path, err);
}

int dxl_config_first_run(const dxl_config *c) {
    return dxl_ini_get_int(c->ini, "FirstRun", "FirstRun", 0);
}

void dxl_config_clamp_first_run(dxl_config *c) {
    if (dxl_config_first_run(c) < DXL_FIRSTRUN_CURRENT)
        dxl_ini_set_int(c->ini, "FirstRun", "FirstRun", DXL_FIRSTRUN_CURRENT);
}

const char *dxl_config_render_device(const dxl_config *c) {
    return dxl_ini_get(c->ini, "Engine.Engine", "GameRenderDevice");
}

void dxl_config_set_render_device(dxl_config *c, const char *class_name) {
    dxl_ini_set(c->ini, "Engine.Engine", "GameRenderDevice", class_name);
}

const char *dxl_config_cd_path(const dxl_config *c) {
    return dxl_ini_get(c->ini, "Engine.Engine", "CdPath");
}

const char *dxl_config_game_engine(const dxl_config *c) {
    return dxl_ini_get(c->ini, "Engine.Engine", "GameEngine");
}

int dxl_config_desc_flags(const dxl_config *c, const char *render_class) {
    return dxl_ini_get_int(c->ini, render_class, "DescFlags", 0);
}

void dxl_config_set_desc_flags(dxl_config *c, const char *render_class, int flags) {
    dxl_ini_set_int(c->ini, render_class, "DescFlags", flags);
}

const char *dxl_config_description(const dxl_config *c, const char *render_class) {
    return dxl_ini_get(c->ini, render_class, "Description");
}

void dxl_detail_defaults(dxl_detail *d, int weak_machine) {
    d->low_sound = weak_machine;
    d->low_skins = weak_machine;
    d->low_world = weak_machine;
    d->low_res   = weak_machine;
}

void dxl_config_apply_detail(dxl_config *c, const dxl_detail *d,
                             const char *render_class) {
    dxl_ini *ini = c->ini;

    /* MinDesiredFrameRate. The original writes it on two paths -- software
     * renderer or a sub-~280MHz CPU (0x1090ED2C), and Direct3D (0x1090ED9E) --
     * and BOTH push the same literal "1". (docs/re/ini-keys.md implied a
     * per-renderer value; verified in the disassembly that there is only one.)
     * The shipped default is "1.0", so this write is a real change. */
    if (render_class &&
        (dxl_stricmp(render_class, "SoftDrv.SoftwareRenderDevice") == 0 ||
         dxl_stricmp(render_class, "D3DDrv.D3DRenderDevice") == 0))
        dxl_ini_set(ini, "WinDrv.WindowsClient", "MinDesiredFrameRate", "1");

    /* Audio. The original only ever writes the downgrade set; leaving the
     * shipped high-quality defaults alone is what "high" means. */
    if (d->low_sound) {
        dxl_ini_set_bool(ini, "Galaxy.GalaxyAudioSubsystem", "UseReverb",  0);
        dxl_ini_set     (ini, "Galaxy.GalaxyAudioSubsystem", "OutputRate", "11025Hz");
        dxl_ini_set_bool(ini, "Galaxy.GalaxyAudioSubsystem", "UseSpatial", 0);
        dxl_ini_set_bool(ini, "Galaxy.GalaxyAudioSubsystem", "UseFilter",  0);
    }
    dxl_ini_set_bool(ini, "Galaxy.GalaxyAudioSubsystem", "LowSoundQuality", d->low_sound);

    dxl_ini_set(ini, "WinDrv.WindowsClient", "SkinDetail",
                d->low_skins ? "Medium" : "High");
    dxl_ini_set(ini, "WinDrv.WindowsClient", "TextureDetail",
                d->low_world ? "Medium" : "High");

    if (d->low_res) {
        dxl_ini_set_int(ini, "WinDrv.WindowsClient", "WindowedViewportX",   640);
        dxl_ini_set_int(ini, "WinDrv.WindowsClient", "WindowedViewportY",   480);
        dxl_ini_set_int(ini, "WinDrv.WindowsClient", "WindowedColorBits",    16);
        dxl_ini_set_int(ini, "WinDrv.WindowsClient", "FullscreenViewportX", 640);
        dxl_ini_set_int(ini, "WinDrv.WindowsClient", "FullscreenViewportY", 480);
        dxl_ini_set_int(ini, "WinDrv.WindowsClient", "FullscreenColorBits",  16);
    }
}
