#include "test.h"
#include "core/config.h"
#include "core/ini.h"

#include <unistd.h>
#include <sys/stat.h>

/* Copies the shipped DeusEx.ini into a scratch directory so the tests can
 * write to it. Returns the directory; caller removes it. */
static char *scratch_install(const char *tag) {
    static char dir[512];
    snprintf(dir, sizeof dir, "/tmp/dxl-test-%s-%d", tag, (int)getpid());
    mkdir(dir, 0755);

    char *src = fixture("DeusEx.ini");
    size_t len = 0;
    char *data = slurp(src, &len);
    free(src);

    char dst[640];
    snprintf(dst, sizeof dst, "%s/DeusEx.ini", dir);
    FILE *f = fopen(dst, "wb");
    if (f) { fwrite(data, 1, len, f); fclose(f); }
    free(data);
    return dir;
}

static void scrub(const char *dir) {
    char p[640];
    snprintf(p, sizeof p, "%s/DeusEx.ini", dir); remove(p);
    snprintf(p, sizeof p, "%s/Running.ini", dir); remove(p);
    rmdir(dir);
}

static void test_reads_the_gates(void) {
    char *dir = scratch_install("gates");
    dxl_config *c = dxl_config_open(dir, "DeusEx");

    CHECK_INT(dxl_config_first_run(c), 0);
    CHECK_STR(dxl_config_render_device(c), "GlideDrv.GlideRenderDevice");
    CHECK_STR(dxl_config_cd_path(c), "..\\");
    CHECK_STR(dxl_config_game_engine(c), "DeusEx.DeusExGameEngine");

    dxl_config_free(c);
    scrub(dir);
}

/* FirstRun only ever rises. A user who has an install newer than this build
 * must not be dragged backwards into the first-run wizard. */
static void test_first_run_clamps_up_only(void) {
    char *dir = scratch_install("clamp");
    dxl_config *c = dxl_config_open(dir, "DeusEx");

    dxl_config_clamp_first_run(c);
    CHECK_INT(dxl_config_first_run(c), DXL_FIRSTRUN_CURRENT);
    CHECK_INT(dxl_config_first_run(c), 1100);
    dxl_config_free(c);

    /* Already at or above the ceiling: untouched, and not even dirtied. */
    c = dxl_config_open(dir, "DeusEx");
    dxl_ini_set_int(dxl_config_ini(c), "FirstRun", "FirstRun", 2000);
    dxl_config_clamp_first_run(c);
    CHECK_INT(dxl_config_first_run(c), 2000);
    dxl_config_free(c);

    scrub(dir);
}

/* The renderer choice is the ONLY thing the wizard communicates to the engine
 * about video. docs/re/porting-notes.md lists it as load-bearing. */
static void test_render_device_write_persists(void) {
    char *dir = scratch_install("render");

    dxl_config *c = dxl_config_open(dir, "DeusEx");
    dxl_config_set_render_device(c, "OpenGLDrv.OpenGLRenderDevice");
    dxl_config_clamp_first_run(c);
    CHECK_INT(dxl_config_dirty(c), 1);
    dxl_err e;
    CHECK_INT(dxl_config_save(c, &e), 0);
    dxl_config_free(c);

    c = dxl_config_open(dir, "DeusEx");
    CHECK_STR(dxl_config_render_device(c), "OpenGLDrv.OpenGLRenderDevice");
    CHECK_INT(dxl_config_first_run(c), 1100);
    dxl_config_free(c);

    scrub(dir);
}

/* Saving a config nothing touched must not rewrite the file at all. */
static void test_untouched_config_is_byte_identical_after_save(void) {
    char *dir = scratch_install("notouch");
    char path[640];
    snprintf(path, sizeof path, "%s/DeusEx.ini", dir);

    size_t before_len = 0;
    char *before = slurp(path, &before_len);

    dxl_config *c = dxl_config_open(dir, "DeusEx");
    CHECK_INT(dxl_config_dirty(c), 0);
    dxl_config_save(c, NULL);
    dxl_config_free(c);

    size_t after_len = 0;
    char *after = slurp(path, &after_len);
    CHECK_INT(after_len, before_len);
    CHECK(before && after && after_len == before_len &&
          memcmp(before, after, before_len) == 0);

    free(before); free(after);
    scrub(dir);
}

/* The golden test: after the detail screen, the ini must carry the block
 * docs/re/ini-keys.md "Detail auto-configuration" lists. */
static void test_detail_block_low(void) {
    char *dir = scratch_install("detail-low");
    dxl_config *c = dxl_config_open(dir, "DeusEx");
    dxl_ini *ini = dxl_config_ini(c);

    dxl_detail d;
    dxl_detail_defaults(&d, 1);            /* weak machine -> everything low */
    dxl_config_apply_detail(c, &d, "D3DDrv.D3DRenderDevice");

    CHECK_STR(dxl_ini_get(ini, "WinDrv.WindowsClient", "MinDesiredFrameRate"), "1");
    CHECK_STR(dxl_ini_get(ini, "Galaxy.GalaxyAudioSubsystem", "UseReverb"),  "False");
    CHECK_STR(dxl_ini_get(ini, "Galaxy.GalaxyAudioSubsystem", "OutputRate"), "11025Hz");
    CHECK_STR(dxl_ini_get(ini, "Galaxy.GalaxyAudioSubsystem", "UseSpatial"), "False");
    CHECK_STR(dxl_ini_get(ini, "Galaxy.GalaxyAudioSubsystem", "UseFilter"),  "False");
    CHECK_STR(dxl_ini_get(ini, "Galaxy.GalaxyAudioSubsystem", "LowSoundQuality"), "True");
    CHECK_STR(dxl_ini_get(ini, "WinDrv.WindowsClient", "SkinDetail"),    "Medium");
    CHECK_STR(dxl_ini_get(ini, "WinDrv.WindowsClient", "TextureDetail"), "Medium");
    CHECK_STR(dxl_ini_get(ini, "WinDrv.WindowsClient", "WindowedViewportX"),   "640");
    CHECK_STR(dxl_ini_get(ini, "WinDrv.WindowsClient", "WindowedViewportY"),   "480");
    CHECK_STR(dxl_ini_get(ini, "WinDrv.WindowsClient", "WindowedColorBits"),    "16");
    CHECK_STR(dxl_ini_get(ini, "WinDrv.WindowsClient", "FullscreenViewportX"), "640");
    CHECK_STR(dxl_ini_get(ini, "WinDrv.WindowsClient", "FullscreenViewportY"), "480");
    CHECK_STR(dxl_ini_get(ini, "WinDrv.WindowsClient", "FullscreenColorBits"),  "16");

    dxl_config_free(c);
    scrub(dir);
}

/* High detail leaves the shipped defaults where they are. The original only
 * ever writes the downgrade set. */
static void test_detail_block_high_leaves_defaults(void) {
    char *dir = scratch_install("detail-high");
    dxl_config *c = dxl_config_open(dir, "DeusEx");
    dxl_ini *ini = dxl_config_ini(c);

    dxl_detail d;
    dxl_detail_defaults(&d, 0);
    dxl_config_apply_detail(c, &d, "OpenGLDrv.OpenGLRenderDevice");

    /* Shipped values, untouched. */
    CHECK_STR(dxl_ini_get(ini, "Galaxy.GalaxyAudioSubsystem", "UseReverb"),  "True");
    CHECK_STR(dxl_ini_get(ini, "Galaxy.GalaxyAudioSubsystem", "OutputRate"), "44100Hz");
    CHECK_STR(dxl_ini_get(ini, "WinDrv.WindowsClient", "SkinDetail"),    "High");
    CHECK_STR(dxl_ini_get(ini, "WinDrv.WindowsClient", "TextureDetail"), "High");
    CHECK_STR(dxl_ini_get(ini, "Galaxy.GalaxyAudioSubsystem", "LowSoundQuality"), "False");

    /* OpenGL is neither SoftDrv nor D3DDrv, so the frame-rate override does
     * not apply and the shipped 1.0 stands. */
    CHECK_STR(dxl_ini_get(ini, "WinDrv.WindowsClient", "MinDesiredFrameRate"), "1.0");

    dxl_config_free(c);
    scrub(dir);
}

/* Verified in the disassembly: both call sites push the literal "1", and the
 * write happens for the software renderer as well as Direct3D. */
static void test_min_frame_rate_rule(void) {
    char *dir = scratch_install("mfr");
    dxl_detail d;
    dxl_detail_defaults(&d, 0);

    const char *writes[] = { "SoftDrv.SoftwareRenderDevice", "D3DDrv.D3DRenderDevice" };
    for (size_t i = 0; i < 2; i++) {
        dxl_config *c = dxl_config_open(dir, "DeusEx");
        dxl_config_apply_detail(c, &d, writes[i]);
        CHECK_STR(dxl_ini_get(dxl_config_ini(c), "WinDrv.WindowsClient",
                              "MinDesiredFrameRate"), "1");
        dxl_config_free(c);
    }
    const char *leaves[] = { "OpenGLDrv.OpenGLRenderDevice", "GlideDrv.GlideRenderDevice" };
    for (size_t i = 0; i < 2; i++) {
        dxl_config *c = dxl_config_open(dir, "DeusEx");
        dxl_config_apply_detail(c, &d, leaves[i]);
        CHECK_STR(dxl_ini_get(dxl_config_ini(c), "WinDrv.WindowsClient",
                              "MinDesiredFrameRate"), "1.0");
        dxl_config_free(c);
    }
    scrub(dir);
}

/* Reproduces what the live Proton run actually wrote into the ini
 * (docs/re/live-verification.md): DescFlags and Description are runtime values
 * that no shipped file carries, written by detection and read back by the
 * wizard. */
static void test_desc_flags_round_trip(void) {
    char *dir = scratch_install("desc");
    dxl_config *c = dxl_config_open(dir, "DeusEx");

    CHECK_INT(dxl_config_desc_flags(c, "D3DDrv.D3DRenderDevice"), 0);
    CHECK(dxl_config_description(c, "D3DDrv.D3DRenderDevice") == NULL);

    dxl_config_set_desc_flags(c, "D3DDrv.D3DRenderDevice", 1);
    dxl_ini_set(dxl_config_ini(c), "D3DDrv.D3DRenderDevice", "Description",
                "ATI Radeon HD 5600 Series");
    dxl_err e;
    CHECK_INT(dxl_config_save(c, &e), 0);
    dxl_config_free(c);

    c = dxl_config_open(dir, "DeusEx");
    CHECK_INT(dxl_config_desc_flags(c, "D3DDrv.D3DRenderDevice"), 1);
    CHECK_STR(dxl_config_description(c, "D3DDrv.D3DRenderDevice"),
              "ATI Radeon HD 5600 Series");
    dxl_config_free(c);

    scrub(dir);
}

/* A fresh install with no ini must still be usable: the wizard writes one. */
static void test_missing_ini_is_not_fatal(void) {
    char dir[512];
    snprintf(dir, sizeof dir, "/tmp/dxl-test-noini-%d", (int)getpid());
    mkdir(dir, 0755);

    dxl_config *c = dxl_config_open(dir, "DeusEx");
    CHECK(c != NULL);
    CHECK_INT(dxl_config_first_run(c), 0);         /* absent reads as 0 */
    dxl_config_set_render_device(c, "OpenGLDrv.OpenGLRenderDevice");
    dxl_err e;
    CHECK_INT(dxl_config_save(c, &e), 0);
    dxl_config_free(c);

    c = dxl_config_open(dir, "DeusEx");
    CHECK_STR(dxl_config_render_device(c), "OpenGLDrv.OpenGLRenderDevice");
    dxl_config_free(c);

    scrub(dir);
}

TEST_MAIN_BEGIN
    RUN(test_reads_the_gates);
    RUN(test_first_run_clamps_up_only);
    RUN(test_render_device_write_persists);
    RUN(test_untouched_config_is_byte_identical_after_save);
    RUN(test_detail_block_low);
    RUN(test_detail_block_high_leaves_defaults);
    RUN(test_min_frame_rate_rule);
    RUN(test_desc_flags_round_trip);
    RUN(test_missing_ini_is_not_fatal);
TEST_MAIN_END
