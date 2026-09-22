#!/usr/bin/env bash
# Fetch the aarch64 cross toolchain.
#
# HARD CONSTRAINT: the toolchain's glibc must be <= 2.33, the version on the
# TrimUI Smart Pro (/lib/ld-2.33.so). glibc 2.34 merged libpthread/libdl into
# libc and re-versioned the startup symbols, so ANY toolchain built against
# >= 2.34 emits __libc_start_main@GLIBC_2.34 from crt1.o and the binary will
# not load on this device -- regardless of what our own code calls.
#
# Measured, not assumed:
#   Arch aarch64-linux-gnu-gcc        glibc 2.44  -> rejected
#   ARM GNU 13.3.rel1                 glibc 2.38  -> rejected (emits GLIBC_2.34)
#   Bootlin stable-2020.08-1 (gcc 10) glibc 2.31  -> USED
#
# scripts/check-abi.sh enforces this on every build artifact.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="$ROOT/toolchain"
NAME="aarch64--glibc--stable-2020.08-1"
URL="https://toolchains.bootlin.com/downloads/releases/toolchains/aarch64/tarballs/${NAME}.tar.bz2"
GCC="$DEST/$NAME/bin/aarch64-linux-gcc"

if [ -x "$GCC" ]; then
    echo "toolchain already present: $DEST/$NAME"
    "$GCC" --version | head -1
    exit 0
fi

mkdir -p "$DEST"
echo "downloading $NAME ..."
curl -fL --retry 3 --progress-bar -o "$DEST/$NAME.tar.bz2" "$URL"
echo "extracting ..."
tar -C "$DEST" -xf "$DEST/$NAME.tar.bz2"
rm -f "$DEST/$NAME.tar.bz2"

"$GCC" --version | head -1
minor=$(grep -oE '__GLIBC_MINOR__[[:space:]]+[0-9]+' \
    "$DEST/$NAME/aarch64-buildroot-linux-gnu/sysroot/usr/include/features.h" \
    | grep -oE '[0-9]+$')
echo "toolchain glibc: 2.$minor"
if [ "$minor" -gt 33 ]; then
    echo "ERROR: toolchain glibc 2.$minor > device glibc 2.33" >&2
    exit 1
fi
echo "toolchain ready: $DEST/$NAME"
