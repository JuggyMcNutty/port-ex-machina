#!/usr/bin/env bash
# A Windows program run here: under the Proton wine and in the prefix that
# lib/common.sh's Wine helpers name (DX_PROTON, DX_WINEPREFIX), the prefix
# made on first use. The Windows port's tests run through it, CMake's
# emulator for them (CMAKE_CROSSCOMPILING_EMULATOR); its exit code is the
# program's.
#
#   scripts/wine.sh <program.exe> [args]
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

[ $# -ge 1 ] || die "usage: scripts/wine.sh <program.exe> [args]"
dx_wine_ready
dx_wine_run "$@"
