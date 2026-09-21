/* Safe mode.
 *
 * The thing to understand, and the thing docs/re/wizard.md is emphatic about:
 * safe mode does NOT apply settings in process. It assembles a flag string,
 * optionally deletes the config, re-executes the launcher with those flags,
 * and ends the current process. The relaunch IS the mechanism.
 *
 * All eight checkboxes are wired here. In the shipped binary five of the eight
 * BM_GETCHECK sites read the same control (+0xB0), so ticking "Disable 3D
 * sound hardware" silently also applied -nohard, -noddraw and -defaultres,
 * while three boxes did nothing at all. That is a bug, not a contract, and
 * docs/DESIGN.md records the decision not to reproduce it.
 */
#ifndef DXL_RELAUNCH_H
#define DXL_RELAUNCH_H

#include "core/common.h"

/* Field order matches the construction order confirmed at 0x10911582, so this
 * struct reads in the same order as the screen. */
typedef struct {
    int no_sound;       /* IDC_NoSound      1108 -> -nosound              */
    int no_3d_sound;    /* IDC_No3DSound    1109 -> -no3dsound            */
    int no_3d_video;    /* IDC_No3dVideo    1110 -> -nohard               */
    int windowed;       /* IDC_Window       1112 -> -nohard -noddraw      */
    int default_res;    /* IDC_Res          1111 -> -defaultres           */
    int reset_config;   /* IDC_ResetConfig  1113 -> delete <Package>.ini  */
    int no_processor;   /* IDC_NoProcessor  1114 -> -nommx -nokni -nok6   */
    int no_joy;         /* IDC_NoJoy        1115 -> -nojoy                */
} dxl_safe_options;

/* Builds the flag string. -nohard is emitted once even when both the
 * "disable 3D video" and "windowed" boxes ask for it. Caller frees. */
char *dxl_safe_flags(const dxl_safe_options *o);

/* Deletes <system_dir>/<package>.ini for the reset checkbox. Returns 0 if the
 * file is gone afterwards, whether or not it existed. */
int dxl_safe_reset_config(const char *system_dir, const char *package);

/* Replaces the current process. Only returns on failure. workdir may be NULL.
 *
 * The original ShellExecutes a new process and ends this one; exec is the
 * closer analogue on POSIX and avoids a window where both are alive. */
int dxl_relaunch(const char *exe, const char *flags, const char *workdir, dxl_err *err);

/* Splits a flag string into an argv for exec. Caller frees with
 * dxl_argv_free. Included here because the safe-mode path and the game-launch
 * path both need it. */
char **dxl_argv_build(const char *exe, const char *flags);
void   dxl_argv_free(char **argv);

#endif
