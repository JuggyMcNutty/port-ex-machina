#include "test.h"
#include "core/strings.h"

/* Startup.int is the launcher's own string table; docs/re/agent.md calls it
 * one of the two cheat codes for this whole effort. These assertions double as
 * a check that the shipped file still says what the spec says it says. */
static void test_reads_startup_int(void) {
    dxl_strings *s = dxl_strings_load(DXL_FIXTURES, "Startup");
    CHECK_INT(dxl_strings_present(s), 1);

    CHECK_STR(dxl_strings_get(s, "General", "Run", NULL), "Run!");
    CHECK_STR(dxl_strings_get(s, "General", "SafeMode", NULL), "Deus Ex Safe Mode");
    CHECK_STR(dxl_strings_get(s, "General", "RecoveryMode", NULL), "Deus Ex Recovery Mode");
    CHECK_STR(dxl_strings_get(s, "General", "FirstTime", NULL),
              "Deus Ex First-Time Configuration");
    CHECK_STR(dxl_strings_get(s, "General", "WebPage", NULL), "http://www.deusex.com/");

    /* The four SafeMode buttons, in the order docs/re/wizard.md lists them. */
    CHECK_STR(dxl_strings_get(s, "IDDIALOG_ConfigPageSafeMode", "IDC_Run", NULL),
              "Run Deus Ex");
    CHECK_STR(dxl_strings_get(s, "IDDIALOG_ConfigPageSafeMode", "IDC_Video", NULL),
              "Change your 3D video device");

    /* All eight safe-mode checkbox labels exist -- including the three the
     * original never reads. We wire them; the strings were always there. */
    const char *boxes[] = { "IDC_NoSound", "IDC_No3DSound", "IDC_No3DVideo",
                            "IDC_Window", "IDC_Res", "IDC_ResetConfig",
                            "IDC_NoProcessor" };
    for (size_t i = 0; i < sizeof boxes / sizeof *boxes; i++)
        CHECK(dxl_strings_get(s, "IDDIALOG_ConfigPageSafeOptions", boxes[i], NULL) != NULL);

    dxl_strings_free(s);
}

/* WorldHigh="High detail textures" -- the only quoted value in the file. */
static void test_strips_quotes(void) {
    dxl_strings *s = dxl_strings_load(DXL_FIXTURES, "Startup");
    CHECK_STR(dxl_strings_get(s, "General", "WorldHigh", NULL), "High detail textures");
    CHECK_STR(dxl_strings_get(s, "General", "WorldLow", NULL), "Medium detail world textures");
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
