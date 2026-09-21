#!/usr/bin/env bash
# Build a link sysroot for the TrimUI Smart Pro.
#
# We link against the DEVICE's own SDL2 (/usr/trimui/lib), not a self-built one:
# that vendor build carries a custom "mali" EGL video driver (SDL_malivideo.c)
# that upstream SDL2 does not have, and the PowerVR stack on this device ships
# only libpvrNULL_WSEGL.so -- so an upstream KMSDRM build has nothing to draw on.
#
# Headers come from the matching SDL 2.30.8 release tarball.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SYS="$ROOT/sysroot/trimui"
SDL_VER="2.30.8"

DEVICE="${DEVICE:-192.168.1.211}"
DEVICE_USER="${DEVICE_USER:-spruce}"
DEVICE_PASS="${DEVICE_PASS:-happygaming}"
SSHOPTS=(-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=15)

mkdir -p "$SYS/lib" "$SYS/include"

# ---- device libraries ------------------------------------------------------
if [ -z "$(ls -A "$SYS/lib" 2>/dev/null)" ]; then
    echo "pulling libraries from $DEVICE_USER@$DEVICE ..."
    tmp="$(mktemp -d)"
    sshpass -p "$DEVICE_PASS" ssh "${SSHOPTS[@]}" "$DEVICE_USER@$DEVICE" \
        'tar -C / -chf - usr/trimui/lib/libSDL2-2.0.so.0 \
                        usr/trimui/lib/libSDL2_ttf-2.0.so.0 \
                        usr/trimui/lib/libSDL2_image-2.0.so.0 \
                        usr/trimui/lib/libSDL2_mixer-2.0.so.0 \
                        usr/lib/libfreetype.so.6 2>/dev/null' > "$tmp/dev.tar"
    tar -C "$tmp" -xf "$tmp/dev.tar"
    cp -a "$tmp"/usr/trimui/lib/* "$SYS/lib/"
    cp -a "$tmp"/usr/lib/* "$SYS/lib/" 2>/dev/null || true
    rm -rf "$tmp"

    # The linker resolves -lSDL2 through the unversioned soname.
    ln -sf libfreetype.so.6 "$SYS/lib/libfreetype.so"
    for l in SDL2-2.0 SDL2_ttf-2.0 SDL2_image-2.0 SDL2_mixer-2.0; do
        base="${l%%-*}"
        [ -e "$SYS/lib/lib$l.so.0" ] && ln -sf "lib$l.so.0" "$SYS/lib/lib$base.so"
    done
    ls -la "$SYS/lib"
else
    echo "device libraries already present in $SYS/lib"
fi

# ---- SDL2 headers ----------------------------------------------------------
if [ ! -f "$SYS/include/SDL2/SDL.h" ]; then
    echo "fetching SDL $SDL_VER headers ..."
    tmp="$(mktemp -d)"
    curl -fL --retry 3 -o "$tmp/sdl.tar.gz" \
        "https://github.com/libsdl-org/SDL/releases/download/release-$SDL_VER/SDL2-$SDL_VER.tar.gz"
    tar -C "$tmp" -xzf "$tmp/sdl.tar.gz"
    mkdir -p "$SYS/include/SDL2"
    cp "$tmp/SDL2-$SDL_VER/include/"*.h "$SYS/include/SDL2/"
    # SDL_config.h / SDL_revision.h are generated; the release tarball ships
    # usable defaults under include/ already, but SDL_config.h needs the
    # non-cmake fallback to exist.
    [ -f "$SYS/include/SDL2/SDL_config.h" ] || \
        cp "$tmp/SDL2-$SDL_VER/include/SDL_config.h.default" "$SYS/include/SDL2/SDL_config.h" 2>/dev/null || true
    rm -rf "$tmp"
else
    echo "SDL2 headers already present in $SYS/include/SDL2"
fi

echo "sysroot ready: $SYS"
