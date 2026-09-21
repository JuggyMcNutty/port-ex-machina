#!/usr/bin/env bash
# Fail if a binary references glibc symbol versions newer than the device's 2.33.
#
# This is the guard for the one mistake that silently produces an unrunnable
# binary: building with a toolchain whose glibc is newer than the target's.
# glibc >= 2.34 re-versioned __libc_start_main and folded in libpthread, so a
# single GLIBC_2.34 reference is enough to make ld.so refuse the executable.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OBJDUMP="${OBJDUMP:-$ROOT/toolchain/aarch64--glibc--stable-2020.08-1/bin/aarch64-linux-objdump}"
MAX_MINOR=33

[ $# -ge 1 ] || { echo "usage: $0 <binary> [binary...]" >&2; exit 2; }

rc=0
for bin in "$@"; do
    [ -f "$bin" ] || { echo "MISSING $bin" >&2; rc=1; continue; }
    bad=$("$OBJDUMP" -T "$bin" 2>/dev/null \
        | grep -oE 'GLIBC_2\.[0-9]+' | sort -u \
        | awk -F. -v max="$MAX_MINOR" '$3 > max')
    if [ -n "$bad" ]; then
        echo "FAIL $bin references $(echo "$bad" | tr '\n' ' ')(device has 2.$MAX_MINOR)" >&2
        rc=1
    else
        have=$("$OBJDUMP" -T "$bin" 2>/dev/null | grep -oE 'GLIBC_2\.[0-9]+' | sort -u | tr '\n' ' ')
        echo "OK   $bin [${have:-no glibc refs}]"
    fi
done
exit $rc
