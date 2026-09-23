#include "test.h"
#include "core/strings.h"

/* Startup.int is the original launcher's string table (docs/re/). The real
 * file's strings are checked against the spec in test_gamefiles; this one is
 * a stand-in with the same sections and keys. */
static void test_reads_startup_int(void) {
    dxl_strings *s = dxl_strings_load(DXL_FIXTURES, "Startup");
    CHECK_INT(dxl_strings_present(s), 1);

    CHECK_STR(dxl_strings_get(s, "General", "Run", NULL), "Play");
    CHECK_STR(dxl_strings_get(s, "General", "SafeMode", NULL), "Safe mode");
    CHECK_STR(dxl_strings_get(s, "General", "WebPage", NULL), "https://example.invalid/");
    CHECK_STR(dxl_strings_get(s, "IDDIALOG_ConfigPageSafeMode", "IDC_Video", NULL),
              "Choose a 3D device");
    /* Two spaces inside a value are the value's, not collapsed. */
    CHECK_STR(dxl_strings_get(s, "Descriptions", "SoftDrv.SoftwareRenderDevice", NULL),
              "Software rendering.  Slow, but it always works.");
    /* An empty value is a present, empty string. */
    CHECK_STR(dxl_strings_get(s, "IDDIALOG_ConfigPageSafeOptions", "IDC_NoJoy", NULL), "");
    dxl_strings_free(s);
}

/* WorldHigh is quoted, as in the real file (its only quoted value). */
static void test_strips_quotes(void) {
    dxl_strings *s = dxl_strings_load(DXL_FIXTURES, "Startup");
    CHECK_STR(dxl_strings_get(s, "General", "WorldHigh", NULL), "High world textures");
    CHECK_STR(dxl_strings_get(s, "General", "WorldLow", NULL), "Medium world textures");
    dxl_strings_free(s);
}

static void test_renderer_descriptions(void) {
    dxl_strings *s = dxl_strings_load(DXL_FIXTURES, "Startup");
    const char *d = dxl_strings_get(s, "Descriptions", "OpenGLDrv.OpenGLRenderDevice", NULL);
    CHECK(d != NULL);
    CHECK(d && strstr(d, "OpenGL") != NULL);
    dxl_strings_free(s);
}

/* A stripped install must degrade, not crash: a missing .int yields fallbacks. */
static void test_missing_file_is_not_fatal(void) {
    dxl_strings *s = dxl_strings_load(DXL_FIXTURES, "NoSuchPackage");
    CHECK(s != NULL);
    CHECK_INT(dxl_strings_present(s), 0);
    CHECK_STR(dxl_strings_get(s, "General", "Run", "Run!"), "Run!");
    dxl_strings_free(s);
}

static void test_missing_key_uses_fallback(void) {
    dxl_strings *s = dxl_strings_load(DXL_FIXTURES, "Startup");
    CHECK_STR(dxl_strings_get(s, "General", "NoSuchKey", "fallback"), "fallback");
    CHECK(dxl_strings_get(s, "General", "NoSuchKey", NULL) == NULL);
    dxl_strings_free(s);
}

TEST_MAIN_BEGIN
    RUN(test_reads_startup_int);
    RUN(test_strips_quotes);
    RUN(test_renderer_descriptions);
    RUN(test_missing_file_is_not_fatal);
    RUN(test_missing_key_uses_fallback);
TEST_MAIN_END
