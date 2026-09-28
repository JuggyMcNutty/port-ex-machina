#!/usr/bin/env bash
# The recreation: deusex-launcher's main -- the original DeusEx.exe, as
# DeusEx -- with the engine linux-x86_64 builds, installed in the game's
# System folder beside DeusEx.exe and run from there.
#
#   scripts/recreation.sh build                  main into build/main/launcher, and the engine
#   scripts/recreation.sh install [<GameDir>]    into <GameDir>/System (gamefiles/ by default)
#   scripts/recreation.sh run [<args>]           gamefiles/System/DeusEx <args>, as a player starts it
#   scripts/recreation.sh check [<GameDir>]      main's tools/livecheck.py on a copy of the install,
#                                                on a private display (build/main/livecheck)
#   scripts/recreation.sh uninstall [<GameDir>]  what install put there, taken out again
#
# Installed: DeusEx and run-game.sh (main's own install), and the engine's
# SurrealEngine, libSurrealVideo.so and SurrealEngine.pk3. Installing touches
# none of the game's files; a run writes what the original's does
# (DeusEx.ini, User.ini, DeusEx.log, Running.ini, Detected.*). The engine is
# the clone's, which scripts/engine.sh check compares with the pin.
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

MAIN_DIR="$DX_LAUNCHER/main"
BUILD="$DX_ROOT/build/main/launcher"
ENGINE_BUILD="$DX_ROOT/build/linux-x86_64/engine"
ENGINE_FILES=(SurrealEngine libSurrealVideo.so SurrealEngine.pk3)
INSTALLED=(DeusEx run-game.sh "${ENGINE_FILES[@]}")

usage() { sed -n '2,17p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2; }

# The game's System folder, which must hold the original's DeusEx.exe.
system_dir() {
    local game="${1:-$DX_ROOT/gamefiles}"
    [ -f "$game/System/DeusEx.exe" ] || die "no Deus Ex in $game (no System/DeusEx.exe)"
    (cd "$game/System" && pwd)
}

cmd_build() {
    [ -e "$MAIN_DIR/.git" ] || die "no deusex-launcher/main -- scripts/launcher.sh fetch"
    cmake -S "$MAIN_DIR" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release >/dev/null
    cmake --build "$BUILD" -j"$(nproc)"
    "$DX_WORKSPACE/scripts/engine.sh" build linux-x86_64
}

cmd_install() {
    local sys f
    sys="$(system_dir "${1:-}")"
    [ -x "$BUILD/DeusEx" ] || die "no build of main -- scripts/recreation.sh build"
    [ -x "$ENGINE_BUILD/SurrealEngine" ] || die "no engine build -- scripts/recreation.sh build"
    cmake --install "$BUILD" --prefix "$sys" >/dev/null
    for f in "${ENGINE_FILES[@]}"; do
        [ -f "$ENGINE_BUILD/$f" ] || die "no $f in $ENGINE_BUILD"
        cp -p "$ENGINE_BUILD/$f" "$sys/$f"
    done
    say "installed in $sys: ${INSTALLED[*]}"
}

cmd_run() {
    local sys
    sys="$(system_dir)"
    [ -x "$sys/DeusEx" ] || die "not installed -- scripts/recreation.sh install"
    exec "$sys/DeusEx" "$@"
}

cmd_check() {
    local game="${1:-$DX_ROOT/gamefiles}"
    system_dir "$game" >/dev/null
    python3 "$MAIN_DIR/tools/livecheck.py" "$game" "$DX_ROOT/build/main/livecheck"
}

cmd_uninstall() {
    local sys f
    sys="$(system_dir "${1:-}")"
    for f in "${INSTALLED[@]}"; do
        rm -f "$sys/$f"
    done
    say "removed from $sys: ${INSTALLED[*]}"
}

cmd="${1:-}"; [ -n "$cmd" ] || usage; shift
case "$cmd" in
    build)     cmd_build ;;
    install)   cmd_install "$@" ;;
    run)       cmd_run "$@" ;;
    check)     cmd_check "$@" ;;
    uninstall) cmd_uninstall "$@" ;;
    -h|--help|help) usage ;;
    *)         die "unknown command '$cmd' (scripts/recreation.sh help)" ;;
esac
