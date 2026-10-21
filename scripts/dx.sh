#!/usr/bin/env bash
# The one entry point: fetch, build, stage and ship a port.
#
#   scripts/dx.sh fetch                       the engine, the launcher's branches and the RE (if absent)
#   scripts/dx.sh ports                       list the ports
#   scripts/dx.sh deps    <port>              toolchains and sysroot into deps/
#   scripts/dx.sh build   <port> [launcher|engine]   default: both, if the port ships the engine
#   scripts/dx.sh stage   <port>              build/<port>/app: exactly what ships
#   scripts/dx.sh deploy  <port> [args]       build, stage, send the app to the device (port-specific)
#   scripts/dx.sh run     <port> [args]       run the staged app here (native ports)
#   scripts/dx.sh profile <port> [args]       frame-time profile on the device (port-specific)
#   scripts/dx.sh test    [<port>]            a port's host build + unit tests (default: linux-x86_64),
#                                             then its own (port_test: Windows' under Wine)
#   scripts/dx.sh check                       every repository's docs, the pins, port files, ABI, game builds
#
# A port is a branch of the launcher, checked out beside this repository in
# deusex-launcher/<port> (scripts/launcher.sh); what it provides is in
# docs/PORTING.md.
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"
export DX_WORKSPACE   # the Smart Pro's fetch-sysroot.sh loads lib/common.sh from it

RE_URL="https://github.com/JuggyMcNutty/dx-reverse-info.git"

usage() { sed -n '2,18p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2; }

load_port() {
    PORT="${1:-}"
    [ -n "$PORT" ] || die "which port? ($(dx_ports | tr '\n' ' '))"
    SRC="$DX_LAUNCHER/$PORT"
    # A port is pinned once its branch is pushed (scripts/launcher.sh pin);
    # before that, its worktree alone makes it one, with a warning.
    if ! dx_ports | grep -qx -- "$PORT"; then
        [ -e "$SRC/.git" ] || die "no port '$PORT' (have: $(dx_ports | tr '\n' ' '))"
        say "warning: $PORT is not pinned in LAUNCHER-PIN.txt -- its worktree's branch, unpushed"
    fi
    [ -e "$SRC/.git" ] || die "no deusex-launcher/$PORT -- scripts/launcher.sh fetch"
    PORT_DIR="$SRC/ports/$PORT"
    [ -f "$PORT_DIR/port.sh" ] || die "deusex-launcher/$PORT has no ports/$PORT/port.sh"
    BUILD="$DX_ROOT/build/$PORT"
    LAUNCHER_BUILD="$SRC/build/$PORT/launcher"   # the port's preset builds in its checkout
    APP="$BUILD/app"

    # Defaults a port.sh may override.
    PORT_DESC=""
    PORT_ENGINE=0
    PORT_LAUNCHER=deusex-launcher   # the launcher's program, which run starts
    PORT_ENGINE_FILES="DXEngine DXEngine.pk3"   # the engine's build that stage copies, its program first
    port_deps()   { say "$PORT: nothing to fetch"; }
    port_stage()  { :; }
    port_deploy() { die "$PORT has no deploy step; its app is $APP"; }
    port_run()    { die "$PORT does not run on this machine; deploy it instead"; }
    port_profile() { die "$PORT has no profile step"; }
    port_test()   { :; }
    # shellcheck source=/dev/null
    . "$PORT_DIR/port.sh"
}

has_preset() {
    grep -q "\"name\": \"$1\"" "$SRC/CMakePresets.json"
}

cmd_build() {
    local what="${1:-all}"
    case "$what" in
        all|launcher)
            has_preset "$PORT" || die "$PORT has no build preset -- see its ports/$PORT/README.md"
            (cd "$SRC" && cmake --preset "$PORT" && cmake --build --preset "$PORT" -j"$(nproc)")
            ;;
    esac
    case "$what" in
        all|engine)
            if [ "$PORT_ENGINE" = 1 ]; then
                "$DX_WORKSPACE/scripts/engine.sh" build "$PORT"
            elif [ "$what" = engine ]; then
                die "$PORT does not build the engine -- see its ports/$PORT/README.md"
            fi
            ;;
        launcher) ;;
        *) die "build what? launcher, engine or nothing for both" ;;
    esac
}

# The app directory is laid out exactly as it ships, from empty: the
# binaries and what the launcher's install rules add, the port's own
# packaging/ over them (a device's app files: its entry script, icon and
# hooks), and the engine's files (PORT_ENGINE_FILES: DXEngine with its
# DXEngine.pk3, unless the port names others).
cmd_stage() {
    [ -x "$LAUNCHER_BUILD/$PORT_LAUNCHER" ] || die "no launcher build -- scripts/dx.sh build $PORT"
    rm -rf "$APP"
    mkdir -p "$APP"
    cmake --install "$LAUNCHER_BUILD" --prefix "$APP" >/dev/null
    [ ! -d "$PORT_DIR/packaging" ] || cp -a "$PORT_DIR/packaging/." "$APP/"
    local sh
    for sh in "$APP"/*.sh; do
        [ ! -f "$sh" ] || chmod +x "$sh"
    done

    if [ "$PORT_ENGINE" = 1 ]; then
        local e="$BUILD/engine" f
        if [ -x "$e/${PORT_ENGINE_FILES%% *}" ]; then
            for f in $PORT_ENGINE_FILES; do
                [ ! -f "$e/$f" ] || cp "$e/$f" "$APP/"
            done
        else
            say "warning: no engine build in $e -- staged the launcher only (scripts/dx.sh build $PORT engine)"
        fi
    fi

    port_stage
    say "staged $APP"
}

# Every port branch builds for the host (the linux-x86_64 preset): its tests,
# a device profile's among them, run there. Then the port's own, where it has
# them (port_test): the Windows port's, cross-built, under Wine.
cmd_test() {
    local p="${1:-linux-x86_64}"
    local src="$DX_LAUNCHER/$p"
    [ -e "$src/.git" ] || die "no deusex-launcher/$p -- scripts/launcher.sh fetch"
    cd "$src"
    cmake --preset linux-x86_64 >/dev/null
    cmake --build --preset linux-x86_64 -j"$(nproc)"
    ctest --preset linux-x86_64
    [ ! -f "$src/ports/$p/port.sh" ] || ( load_port "$p"; port_test )
}

cmd_fetch() {
    "$DX_WORKSPACE/scripts/engine.sh" fetch
    "$DX_WORKSPACE/scripts/launcher.sh" fetch
    if [ -d "$DX_RE/.git" ]; then
        say "RE clone present: $DX_RE"
    else
        git clone "$RE_URL" "$DX_RE"
        git -C "$DX_RE" config user.name JuggyMcNutty
        git -C "$DX_RE" config user.email 11588877+JuggyMcNutty@users.noreply.github.com
    fi
}

cmd_check() {
    local rc=0 p
    say "== docs"
    "$DX_WORKSPACE/scripts/check-docs.sh" || rc=1

    say "== engine pin"
    if [ -d "$DX_ENGINE/.git" ]; then
        "$DX_WORKSPACE/scripts/engine.sh" check || rc=1
    else
        say "skipped: no engine clone (scripts/engine.sh fetch)"
    fi

    say "== launcher pins"
    if [ -d "$DX_LAUNCHER/main/.git" ]; then
        "$DX_WORKSPACE/scripts/launcher.sh" check || rc=1
    else
        say "skipped: no launcher clone (scripts/launcher.sh fetch)"
    fi

    say "== ports"
    for p in $(dx_ports); do
        local dir="$DX_LAUNCHER/$p/ports/$p" f missing=""
        [ -d "$DX_LAUNCHER/$p" ] || continue
        for f in port.cmake port.sh README.md; do
            [ -f "$dir/$f" ] || missing="$missing $f"
        done
        # PORT_ENGINE=1 anywhere: a port may set it only when built natively.
        if grep -q 'PORT_ENGINE=1' "$dir/port.sh" 2>/dev/null && [ ! -f "$dir/engine.cmake" ]; then
            missing="$missing engine.cmake"
        fi
        if [ -n "$missing" ]; then echo "FAIL ports/$p lacks$missing" >&2; rc=1; else echo "OK   ports/$p"; fi
    done

    # A Linux port's glibc ceiling, or a Windows port's floor and the DLLs its
    # programs import (dx_abi_args): the branch's scripts/check-abi.sh.
    say "== ABI of staged builds"
    for p in $(dx_ports); do
        local src="$DX_LAUNCHER/$p" app="$DX_ROOT/build/$p/app" abi bins=()
        [ -f "$src/ports/$p/port.cmake" ] || continue
        abi="$(dx_abi_args "$src/ports/$p/port.cmake")"
        [ -n "$abi" ] || continue
        for f in deusex-launcher dxl-cli DXEngine "$app"/*.exe "$app"/*.dll; do
            f="$app/${f##*/}"
            [ -f "$f" ] && bins+=("$f")
        done
        [ ${#bins[@]} -gt 0 ] || continue
        # shellcheck disable=SC2086
        "$src/scripts/check-abi.sh" $abi "${bins[@]}" || rc=1
    done

    # The builds of Deus Ex a launcher takes are the engine's: it plays only
    # a DeusEx.exe whose SHA1 its own list has.
    say "== the launchers' game builds against the engine's"
    local engine_db="$DX_ENGINE/DXEngine/UE1GameDatabase.h" want have
    if [ -f "$engine_db" ]; then
        want="$(grep -oE '"[0-9a-f]{40}"' "$engine_db" | tr -d '"' | sort)"
        for p in $(dx_ports); do
            local db="$DX_LAUNCHER/$p/src/core/gamedb.c"
            [ -f "$db" ] || continue
            have="$(grep -oE '"[0-9a-f]{40}"' "$db" | tr -d '"' | sort)"
            if [ "$have" = "$want" ]; then
                echo "OK   $p: src/core/gamedb.c"
            else
                echo "FAIL $p: src/core/gamedb.c's builds are not the engine's UE1GameDatabase.h's" >&2
                rc=1
            fi
        done
    else
        say "skipped: no engine clone (scripts/engine.sh fetch)"
    fi

    [ "$rc" = 0 ] && say "all checks passed" || say "checks FAILED"
    return "$rc"
}

cmd_ports() {
    local p
    for p in $(dx_ports); do
        if [ -e "$DX_LAUNCHER/$p/.git" ]; then
            ( load_port "$p"; printf '%-18s %s\n' "$p" "$PORT_DESC" )
        else
            printf '%-18s %s\n' "$p" "(not checked out -- scripts/launcher.sh fetch)"
        fi
    done
}

# A deploy is always of the current tree: build (incremental) and stage first,
# then the port's port_deploy. An engine with uncommitted changes -- the
# profiling hooks, most likely -- ships with a warning, not silently.
cmd_deploy() {
    cmd_build
    cmd_stage
    local eng="$DX_ENGINE"
    if [ "$PORT_ENGINE" = 1 ] && [ -d "$eng/.git" ] && ! git -C "$eng" diff --quiet HEAD -- . ':(exclude)docs' ':(exclude)tools'; then
        say "warning: the engine has uncommitted changes (profiling hooks? tools/perf/perf.sh off) -- deploying them"
    fi
    port_deploy "$@"
}

cmd="${1:-}"; [ -n "$cmd" ] || usage; shift
case "$cmd" in
    fetch)  cmd_fetch ;;
    ports)  cmd_ports ;;
    deps)   load_port "${1:-}"; shift || true; port_deps "$@" ;;
    build)  load_port "${1:-}"; shift || true; cmd_build "$@" ;;
    stage)  load_port "${1:-}"; cmd_stage ;;
    deploy) load_port "${1:-}"; shift || true; cmd_deploy "$@" ;;
    run)    load_port "${1:-}"; shift || true
            [ -x "$APP/$PORT_LAUNCHER" ] || die "nothing staged -- scripts/dx.sh stage $PORT"
            port_run "$@" ;;
    profile) load_port "${1:-}"; shift || true; port_profile "$@" ;;
    test)   cmd_test "$@" ;;
    check)  cmd_check ;;
    -h|--help|help) usage ;;
    *)      die "unknown command '$cmd' (scripts/dx.sh help)" ;;
esac
