#include "test.h"
#include "core/relaunch.h"

/* The shipped binary reads only five distinct controls across its eight
 * BM_GETCHECK sites -- five of them read +0xB0, IDC_No3DSound. So ticking
 * "Disable 3D sound hardware" silently also applied -nohard, -noddraw and
 * -defaultres, and three checkboxes did nothing at all.
 *
 * docs/DESIGN.md records the decision to wire all eight. These tests are the
 * regression that keeps it wired: each box produces its own flags and NOTHING
 * else. If someone ever "restores fidelity" here, this fails loudly. */

static char *flags_for(dxl_safe_options o) { return dxl_safe_flags(&o); }

static void test_nothing_ticked_is_empty(void) {
    dxl_safe_options o = {0};
    char *f = dxl_safe_flags(&o);
    CHECK_STR(f, "");
    free(f);
}

static void test_each_box_is_independent(void) {
    struct { dxl_safe_options o; const char *want; const char *name; } cases[] = {
        { { .no_sound     = 1 }, "-nosound",              "NoSound"      },
        { { .no_3d_sound  = 1 }, "-no3dsound",            "No3DSound"    },
        { { .no_3d_video  = 1 }, "-nohard",               "No3DVideo"    },
        { { .windowed     = 1 }, "-nohard -noddraw",      "Window"       },
        { { .default_res  = 1 }, "-defaultres",           "Res"          },
        { { .no_processor = 1 }, "-nommx -nokni -nok6",   "NoProcessor"  },
        { { .no_joy       = 1 }, "-nojoy",                "NoJoy"        },
    };
    for (size_t i = 0; i < sizeof cases / sizeof *cases; i++) {
        char *f = dxl_safe_flags(&cases[i].o);
        CHECK_STR(f, cases[i].want);
        free(f);
    }
}

/* The exact shape of the original bug, asserted as absent: ticking only
 * "Disable 3D sound hardware" must not drag in three other flags. */
static void test_no3dsound_does_not_leak(void) {
    char *f = flags_for((dxl_safe_options){ .no_3d_sound = 1 });
    CHECK_STR(f, "-no3dsound");
    CHECK(strstr(f, "-nohard") == NULL);
    CHECK(strstr(f, "-noddraw") == NULL);
    CHECK(strstr(f, "-defaultres") == NULL);
    free(f);
}

/* The three dead boxes must actually do something now. */
static void test_the_three_dead_boxes_work(void) {
    char *f;
    f = flags_for((dxl_safe_options){ .no_3d_video = 1 });
    CHECK(strstr(f, "-nohard") != NULL); free(f);

    f = flags_for((dxl_safe_options){ .windowed = 1 });
    CHECK(strstr(f, "-noddraw") != NULL); free(f);

    f = flags_for((dxl_safe_options){ .default_res = 1 });
    CHECK(strstr(f, "-defaultres") != NULL); free(f);
}

/* Boxes 3 and 4 both want -nohard. Emitting it twice would be harmless to the
 * engine but is noise in a log someone has to read. */
static void test_nohard_emitted_once(void) {
    char *f = flags_for((dxl_safe_options){ .no_3d_video = 1, .windowed = 1 });
    CHECK_STR(f, "-nohard -noddraw");
    const char *first = strstr(f, "-nohard");
    CHECK(first && strstr(first + 1, "-nohard") == NULL);
    free(f);
}

static void test_all_boxes_together(void) {
    char *f = flags_for((dxl_safe_options){
        .no_sound = 1, .no_3d_sound = 1, .no_3d_video = 1, .windowed = 1,
        .default_res = 1, .no_processor = 1, .no_joy = 1 });
    CHECK_STR(f, "-nosound -no3dsound -nohard -noddraw -defaultres "
                 "-nommx -nokni -nok6 -nojoy");
    free(f);
}

/* reset_config is not a flag -- it deletes the ini. It must contribute
 * nothing to the command line. */
static void test_reset_config_is_not_a_flag(void) {
    char *f = flags_for((dxl_safe_options){ .reset_config = 1 });
    CHECK_STR(f, "");
    free(f);
}

/* --- argv construction for the exec --- */

static void test_argv_build(void) {
    char **a = dxl_argv_build("/app/deusex-launcher", "-nosound -nojoy");
    CHECK_STR(a[0], "/app/deusex-launcher");
    CHECK_STR(a[1], "-nosound");
    CHECK_STR(a[2], "-nojoy");
    CHECK(a[3] == NULL);
    dxl_argv_free(a);
}

static void test_argv_build_empty_flags(void) {
    char **a = dxl_argv_build("/app/x", "");
    CHECK_STR(a[0], "/app/x");
    CHECK(a[1] == NULL);
    dxl_argv_free(a);

    a = dxl_argv_build("/app/x", NULL);
    CHECK_STR(a[0], "/app/x");
    CHECK(a[1] == NULL);
    dxl_argv_free(a);
}

/* A game directory with a space in it is normal on an SD card. */
static void test_argv_build_quoted(void) {
    char **a = dxl_argv_build("/app/x", "-map=\"Deus Ex.dx\" -nosound");
    CHECK_STR(a[1], "-map=Deus Ex.dx");
    CHECK_STR(a[2], "-nosound");
    CHECK(a[3] == NULL);
    dxl_argv_free(a);
}

/* Round trip: the flags a box produces must parse back to the same flags,
 * because the relaunched process re-reads them with the same parser. */
static void test_flags_round_trip_through_argv(void) {
    char *f = flags_for((dxl_safe_options){ .no_processor = 1, .no_joy = 1 });
    char **a = dxl_argv_build("x", f);
    CHECK_STR(a[1], "-nommx");
    CHECK_STR(a[2], "-nokni");
    CHECK_STR(a[3], "-nok6");
    CHECK_STR(a[4], "-nojoy");
    CHECK(a[5] == NULL);
    dxl_argv_free(a);
    free(f);
}

TEST_MAIN_BEGIN
    RUN(test_nothing_ticked_is_empty);
    RUN(test_each_box_is_independent);
    RUN(test_no3dsound_does_not_leak);
    RUN(test_the_three_dead_boxes_work);
    RUN(test_nohard_emitted_once);
    RUN(test_all_boxes_together);
    RUN(test_reset_config_is_not_a_flag);
    RUN(test_argv_build);
    RUN(test_argv_build_empty_flags);
    RUN(test_argv_build_quoted);
    RUN(test_flags_round_trip_through_argv);
TEST_MAIN_END
