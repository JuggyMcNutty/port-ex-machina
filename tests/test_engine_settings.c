#include "test.h"
#include "core/engine_settings.h"
#include "core/json.h"

#include <sys/stat.h>
#include <unistd.h>

static char dir[512];

static void setup(const char *tag) {
    snprintf(dir, sizeof dir, "/tmp/dxl-es-%s-%d", tag, (int)getpid());
    mkdir(dir, 0755);
}

static char *in_dir(const char *leaf) {
    static char p[4][640];
    static int n;
    char *out = p[n++ % 4];
    snprintf(out, 640, "%s/%s", dir, leaf);
    return out;
}

static void put(const char *leaf, const char *text) {
    FILE *f = fopen(in_dir(leaf), "wb");
    if (f) { fputs(text, f); fclose(f); }
}

static void teardown(void) {
    remove(in_dir("Settings.json"));
    remove(in_dir("default.json"));
    remove(in_dir("sub/deeper/Settings.json"));
    rmdir(in_dir("sub/deeper"));
    rmdir(in_dir("sub"));
    rmdir(dir);
}

/* The packaged default for the handheld. */
static const char *device_default =
    "{\"RenderDevice\": {\"Type\": \"Vulkan\", \"Antialias\": \"Off\", \"Light\": \"Normal\","
    " \"Gamma\": \"D3D9\", \"GammaCorrectScreenshots\": false, \"UseVSync\": false, \"Hdr\": false,"
    " \"HdrScale\": 128, \"Bloom\": false, \"BloomAmount\": 128, \"UseDebugLayer\": false},"
    " \"Games\": {\"SearchList\": [], \"LastSelected\": 0}}";

static void test_missing_file_seeds_from_default(void) {
    setup("missing");
    put("default.json", device_default);
    dxl_engine_settings *s = dxl_es_open(in_dir("Settings.json"), in_dir("default.json"));
    CHECK_INT(dxl_es_existed(s), 0);
    CHECK_INT(dxl_es_dirty(s), 1);
    CHECK_STR(dxl_es_choice(s, DXL_ES_RENDER_TYPE), "Vulkan");
    CHECK_STR(dxl_es_choice(s, DXL_ES_ANTIALIAS), "Off");
    CHECK_INT(dxl_es_bool(s, DXL_ES_VSYNC), 0);
    CHECK_INT(dxl_es_bool(s, DXL_ES_PAD_ENABLED), 1);
    CHECK_INT(dxl_es_was_present(s, DXL_ES_RENDER_TYPE), 0);
    dxl_es_free(s);
    teardown();
}

/* A file the engine cannot parse costs every setting -- MSAA4x included,
 * which is speckle on the PowerVR. It must be replaced, not trusted. */
static void test_corrupt_file_is_replaced(void) {
    setup("corrupt");
    put("default.json", device_default);
    put("Settings.json", "{\"RenderDevice\": {\"Type\": \"Vulkan\",");
    dxl_engine_settings *s = dxl_es_open(in_dir("Settings.json"), in_dir("default.json"));
    CHECK_INT(dxl_es_was_corrupt(s), 1);
    CHECK_STR(dxl_es_choice(s, DXL_ES_ANTIALIAS), "Off");
    dxl_err e;
    CHECK_INT(dxl_es_save(s, &e), 0);
    dxl_es_free(s);

    dxl_json *j = dxl_json_load(in_dir("Settings.json"), NULL);
    CHECK(j != NULL);
    CHECK_STR(dxl_json_get_string(dxl_json_get(j, "RenderDevice"), "Antialias", NULL), "Off");
    dxl_json_free(j);
    teardown();
}

/* The engine reads a missing member as empty/false/0 -- HdrScale 0 -- so a
 * save must carry every RenderDevice member even if the file lacked some. */
static void test_save_writes_every_field_and_keeps_games(void) {
    setup("full");
    put("Settings.json",
        "{\"RenderDevice\": {\"Type\": \"Vulkan\"}, \"Games\": {\"SearchList\": [\"/x\"], \"LastSelected\": 0}}");
    dxl_engine_settings *s = dxl_es_open(in_dir("Settings.json"), NULL);
    CHECK_INT(dxl_es_was_present(s, DXL_ES_RENDER_TYPE), 1);
    CHECK_INT(dxl_es_was_present(s, DXL_ES_HDR_SCALE), 0);
    dxl_err e;
    CHECK_INT(dxl_es_save(s, &e), 0);
    dxl_es_free(s);

    dxl_json *j = dxl_json_load(in_dir("Settings.json"), NULL);
    dxl_json *rd = dxl_json_get(j, "RenderDevice");
    for (int f = 0; f < DXL_ES_FIELD_COUNT; f++) {
        const dxl_es_info *in = dxl_es_describe((dxl_es_field)f);
        CHECK(dxl_json_get(dxl_json_get(j, in->section), in->key) != NULL);
    }
    CHECK_INT((int)dxl_json_get_number(rd, "HdrScale", 0), 128);
    CHECK(dxl_json_get(dxl_json_get(j, "Games"), "SearchList") != NULL);
    dxl_json_free(j);
    teardown();
}

static void test_setters_validate(void) {
    setup("validate");
    dxl_engine_settings *s = dxl_es_open(in_dir("Settings.json"), NULL);
    CHECK_INT(dxl_es_set_choice(s, DXL_ES_LIGHT, "OneX"), 0);
    CHECK_STR(dxl_es_choice(s, DXL_ES_LIGHT), "OneX");
    /* Not a value the engine knows: refused, previous value stands. */
    CHECK_INT(dxl_es_set_choice(s, DXL_ES_LIGHT, "Dazzling"), -1);
    CHECK_STR(dxl_es_choice(s, DXL_ES_LIGHT), "OneX");
    CHECK_INT(dxl_es_set_choice(s, DXL_ES_RENDER_TYPE, "OpenGLDrv.OpenGLRenderDevice"), -1);

    dxl_es_set_number(s, DXL_ES_PAD_DEADZONE, 9.0);
    CHECK(dxl_es_number(s, DXL_ES_PAD_DEADZONE) <= 0.5);
    dxl_es_set_number(s, DXL_ES_BLOOM_AMOUNT, 255);
    CHECK(dxl_es_number(s, DXL_ES_BLOOM_AMOUNT) <= 255);

    /* Stepping by 0.05 lands on the grid, not near it. */
    double v = 0.05;
    for (int i = 0; i < 5; i++) v += 0.05;
    dxl_es_set_number(s, DXL_ES_PAD_DEADZONE, v);
    CHECK(dxl_es_number(s, DXL_ES_PAD_DEADZONE) == 0.3);
    dxl_es_free(s);
    teardown();
}

static void test_reset_uses_packaged_default(void) {
    setup("reset");
    put("default.json", "{\"RenderDevice\": {\"UseVSync\": true, \"Light\": \"BrighterActors\"}}");
    dxl_engine_settings *s = dxl_es_open(in_dir("Settings.json"), in_dir("default.json"));
    dxl_es_set_bool(s, DXL_ES_VSYNC, 0);
    dxl_es_set_choice(s, DXL_ES_LIGHT, "Normal");
    dxl_es_set_choice(s, DXL_ES_ANTIALIAS, "MSAA4x");
    dxl_es_reset_section(s, "RenderDevice");
    CHECK_INT(dxl_es_bool(s, DXL_ES_VSYNC), 1);
    CHECK_STR(dxl_es_choice(s, DXL_ES_LIGHT), "BrighterActors");
    /* Absent from the packaged file: the built-in default, which is Off. */
    CHECK_STR(dxl_es_choice(s, DXL_ES_ANTIALIAS), "Off");
    dxl_es_free(s);
    teardown();
}

static void test_save_creates_the_directory(void) {
    setup("mkdir");
    dxl_engine_settings *s = dxl_es_open(in_dir("sub/deeper/Settings.json"), NULL);
    dxl_err e;
    CHECK_INT(dxl_es_save(s, &e), 0);
    CHECK(access(in_dir("sub/deeper/Settings.json"), F_OK) == 0);
    CHECK_INT(dxl_es_dirty(s), 0);
    dxl_es_free(s);
    teardown();
}

/* Opening and saving an untouched, complete file changes no value. */
static void test_clean_file_is_not_dirty(void) {
    setup("clean");
    dxl_engine_settings *s = dxl_es_open(in_dir("Settings.json"), NULL);
    dxl_err e;
    dxl_es_save(s, &e);
    dxl_es_free(s);
    s = dxl_es_open(in_dir("Settings.json"), NULL);
    CHECK_INT(dxl_es_dirty(s), 0);
    CHECK_INT(dxl_es_was_present(s, DXL_ES_PAD_LAYOUT), 1);
    dxl_es_set_bool(s, DXL_ES_VSYNC, dxl_es_bool(s, DXL_ES_VSYNC));
    CHECK_INT(dxl_es_dirty(s), 0);
    dxl_es_free(s);
    teardown();
}

TEST_MAIN_BEGIN
    RUN(test_missing_file_seeds_from_default);
    RUN(test_corrupt_file_is_replaced);
    RUN(test_save_writes_every_field_and_keeps_games);
    RUN(test_setters_validate);
    RUN(test_reset_uses_packaged_default);
    RUN(test_save_creates_the_directory);
    RUN(test_clean_file_is_not_dirty);
TEST_MAIN_END
