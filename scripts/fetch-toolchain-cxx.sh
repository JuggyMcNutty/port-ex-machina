#!/usr/bin/env bash
# Fetch the C++20-capable aarch64 toolchain for the engine build.
#
# Same hard constraint as scripts/fetch-toolchain.sh: glibc must be <= 2.33.
# The launcher's GCC 9.3 toolchain cannot build C++20, so this is a second
# toolchain rather than a replacement.
#
#   Bootlin bleeding-edge 2021.05-1 -- GCC 10.3, glibc 2.33 (exact device match)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="$ROOT/toolchain/bleeding"
NAME="aarch64--glibc--bleeding-edge-2021.05-1"
URL="https://toolchains.bootlin.com/downloads/releases/toolchains/aarch64/tarballs/${NAME}.tar.bz2"
GXX="$DEST/$NAME/bin/aarch64-linux-g++"

if [ -x "$GXX" ]; then
    echo "already present: $DEST/$NAME"
    "$GXX" --version | head -1
    exit 0
fi

mkdir -p "$DEST"
echo "downloading $NAME ..."
curl -fL --retry 3 --progress-bar -o "$DEST/$NAME.tar.bz2" "$URL"
tar -C "$DEST" -xf "$DEST/$NAME.tar.bz2"
rm -f "$DEST/$NAME.tar.bz2"

"$GXX" --version | head -1
minor=$(grep -oE '__GLIBC_MINOR__[[:space:]]+[0-9]+' \
    "$DEST/$NAME/aarch64-buildroot-linux-gnu/sysroot/usr/include/features.h" \
    | grep -oE '[0-9]+$')
echo "toolchain glibc: 2.$minor"
[ "$minor" -le 33 ] || { echo "ERROR: glibc 2.$minor > device 2.33" >&2; exit 1; }
