#include "test.h"
#include "core/bindings.h"
#include "core/ini.h"
#include "core/strings.h"

#include <unistd.h>

/* The checks tests/fixtures can only stand in for, run against a real install
 * in gamefiles/System: the byte-identical round trip, the values docs/re/
 * describes, and the classic pad layout. The game's files are not part of this
 * repository, so without an install there this test is skipped (exit 77). */

#ifndef DXL_GAMEFILES
#define DXL_GAMEFILES "gamefiles/System"
#endif

static char *game_file(const char *name) {
    size_t n = strlen(DXL_GAMEFILES) + strlen(name) + 2;
    char *p = malloc(n);
    snprintf(p, n, "%s/%s", DXL_GAMEFILES, name);
    return p;
}

/* DeusEx.ini and User.ini only exist once the game has run; the defaults it
 * builds them from always do. */
static void test_roundtrip(void) {
    const char *names[] = { "Default.ini", "DefUser.ini", "DeusEx.ini", "User.ini" };
    for (size_t i = 0; i < sizeof names / sizeof *names; i++) {
        char *path = game_file(names[i]);
        size_t orig_len = 0;
        char *orig = slurp(path, &orig_len);
        if (!orig && i >= 2) { free(path); continue; }
        CHECK(orig != NULL);
        dxl_ini *ini = orig ? dxl_ini_load(path, NULL) : NULL;
        if (ini) {
            size_t out_len = 0;
            char *out = dxl_ini_render(ini, &out_len);
            CHECK_INT(out_len, orig_len);
            CHECK(out_len == orig_len && memcmp(out, orig, orig_len) == 0);
            free(out);
            dxl_ini_free(ini);
        }
        free(orig);
        free(path);
    }
}

static void test_default_ini_matches_the_spec(void) {
    char *path = game_file("Default.ini");
    dxl_ini *ini = dxl_ini_load(path, NULL);
    free(path);
    CHECK(ini != NULL);
    if (!ini) return;

    /* docs/re/wizard.md: a pristine install ships FirstRun=0, so the
     * first-time flow always runs. */
    CHECK_INT(dxl_ini_get_int(ini, "FirstRun", "FirstRun", -1), 0);
    /* docs/re/launch-flow.md section 8: CdPath=..\ makes the CD check pass. */
    CHECK_STR(dxl_ini_get(ini, "Engine.Engine", "CdPath"), "..\\");
    CHECK_STR(dxl_ini_get(ini, "Engine.Engine", "GameEngine"), "DeusEx.DeusExGameEngine");
    CHECK_STR(dxl_ini_get(ini, "Engine.Engine", "GameRenderDevice"),
              "GlideDrv.GlideRenderDevice");
    CHECK_INT(dxl_ini_get_all(ini, "Core.System", "Paths", NULL, 0), 5);

    /* docs/re/ini-keys.md: DescFlags and Description are runtime values that
     * appear in no shipped ini. If these ever start existing, the detection
     * story in the docs is wrong. */
    CHECK(dxl_ini_get(ini, "D3DDrv.D3DRenderDevice", "Description") == NULL);
    CHECK(dxl_ini_get(ini, "D3DDrv.D3DRenderDevice", "DescFlags") == NULL);
    dxl_ini_free(ini);
}

/* The classic preset in src/core/bindings.c is meant to be exactly what
 * DefUser.ini ships. */
static void test_defuser_is_classic(void) {
    char *path = game_file("DefUser.ini");
    dxl_ini *ini = dxl_ini_load(path, NULL);
    free(path);
    CHECK(ini != NULL);
    if (!ini) return;
    const dxl_pad_preset *p = dxl_bindings_detect(ini);
    CHECK(p != NULL);
    if (p) CHECK_STR(p->id, "classic");
    dxl_ini_free(ini);
}

/* Startup.int is the original launcher's string table: the file still says
 * what docs/re/wizard.md says it says. */
static void test_startup_int_matches_the_spec(void) {
    dxl_strings *s = dxl_strings_load(DXL_GAMEFILES, "Startup");
    CHECK_INT(dxl_strings_present(s), 1);

    CHECK_STR(dxl_strings_get(s, "General", "Run", NULL), "Run!");
    CHECK_STR(dxl_strings_get(s, "General", "SafeMode", NULL), "Deus Ex Safe Mode");
    CHECK_STR(dxl_strings_get(s, "General", "RecoveryMode", NULL), "Deus Ex Recovery Mode");
    CHECK_STR(dxl_strings_get(s, "General", "FirstTime", NULL),
              "Deus Ex First-Time Configuration");
    /* WorldHigh is the file's only quoted value. */
    CHECK_STR(dxl_strings_get(s, "General", "WorldHigh", NULL), "High detail textures");

    /* The SafeMode buttons, as docs/re/wizard.md lists them. */
    CHECK_STR(dxl_strings_get(s, "IDDIALOG_ConfigPageSafeMode", "IDC_Run", NULL),
              "Run Deus Ex");
    CHECK_STR(dxl_strings_get(s, "IDDIALOG_ConfigPageSafeMode", "IDC_Video", NULL),
              "Change your 3D video device");

    /* All the safe-mode checkbox labels exist -- including the three the
     * original never reads. We wire them; the strings were always there. */
    const char *boxes[] = { "IDC_NoSound", "IDC_No3DSound", "IDC_No3DVideo",
                            "IDC_Window", "IDC_Res", "IDC_ResetConfig",
                            "IDC_NoProcessor" };
    for (size_t i = 0; i < sizeof boxes / sizeof *boxes; i++)
        CHECK(dxl_strings_get(s, "IDDIALOG_ConfigPageSafeOptions", boxes[i], NULL) != NULL);

    const char *d = dxl_strings_get(s, "Descriptions", "OpenGLDrv.OpenGLRenderDevice", NULL);
    CHECK(d && strstr(d, "OpenGL") != NULL);
    dxl_strings_free(s);
}

TEST_MAIN_BEGIN
    if (access(DXL_GAMEFILES "/Default.ini", R_OK) != 0) {
        printf("skipped: no Deus Ex install in %s\n", DXL_GAMEFILES);
        return 77;
    }
    RUN(test_roundtrip);
    RUN(test_default_ini_matches_the_spec);
    RUN(test_defuser_is_classic);
    RUN(test_startup_int_matches_the_spec);
TEST_MAIN_END
