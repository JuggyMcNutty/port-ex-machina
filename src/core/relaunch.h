/* Process replacement: how the launcher hands over to the game.
 *
 * The launcher's last act is to exec the configured game command with the
 * command line it was given. exec rather than fork+wait, as the original
 * ended its own process: nothing of the launcher stays resident while the
 * game runs.
 *
 * The original also re-executed itself for safe mode with a flag string built
 * from eight checkboxes. Surreal Engine honours none of those flags, so that
 * path is gone; docs/DESIGN.md records why.
 */
#ifndef DXL_RELAUNCH_H
#define DXL_RELAUNCH_H

#include "core/common.h"

/* Replaces the current process. Only returns on failure. workdir may be NULL.
 *
 * exec is the POSIX analogue of the original's ShellExecute-then-exit, and
 * avoids a window where both processes are alive. */
int dxl_relaunch(const char *exe, const char *flags, const char *workdir, dxl_err *err);

/* Splits a flag string into an argv for exec. Caller frees with
 * dxl_argv_free. */
char **dxl_argv_build(const char *exe, const char *flags);
void   dxl_argv_free(char **argv);

#endif
