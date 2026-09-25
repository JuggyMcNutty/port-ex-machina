#!/usr/bin/env bash
# The one entry point: fetch, build, stage and ship a port.
#
#   scripts/dx.sh ports                       list the ports
#   scripts/dx.sh deps    <port>              toolchains and sysroot into deps/
#   scripts/dx.sh build   <port> [launcher|engine]   default: both, if the port ships the engine
#   scripts/dx.sh stage   <port>              build/<port>/app: exactly what ships
#   scripts/dx.sh deploy  <port> [args]       build, stage, send the app to the device (port-specific)
#   scripts/dx.sh run     <port> [args]       run the staged app here (native ports)
#   scripts/dx.sh profile <port> [args]       frame-time profile on the device (port-specific)
#   scripts/dx.sh test                        host build + unit tests
#   scripts/dx.sh check                       docs paths, the engine pin, ABI, port files
#
# What each port provides is in docs/PORTING.md.
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

usage() { sed -n '2,14p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2; }

load_port() {
    PORT="${1:-}"
    [ -n "$PORT" ] || die "which port? ($(dx_ports | tr '\n' ' '))"
    PORT_DIR="$DX_ROOT/ports/$PORT"
    [ -f "$PORT_DIR/port.sh" ] || die "no port '$PORT' (have: $(dx_ports | tr '\n' ' '))"
    BUILD="$DX_ROOT/build/$PORT"
    APP="$BUILD/app"

    # Defaults a port.sh may override.
    PORT_DESC=""
    PORT_ENGINE=0
    port_deps()   { say "$PORT: nothing to fetch"; }
    port_stage()  { :; }
    port_deploy() { die "$PORT has no deploy step; its app is $APP"; }
    port_run()    { die "$PORT does not run on this machine; deploy it instead"; }
    port_profile() { die "$PORT has no profile step"; }
    # shellcheck source=/dev/null
    . "$PORT_DIR/port.sh"
}

has_preset() {
    grep -q "\"name\": \"$1\"" "$DX_ROOT/CMakePresets.json"
}

cmd_build() {
    local what="${1:-all}"
    case "$what" in
        all|launcher)
            has_preset "$PORT" || die "$PORT has no build preset -- see ports/$PORT/README.md"
            (cd "$DX_ROOT" && cmake --preset "$PORT" && cmake --build --preset "$PORT" -j"$(nproc)")
            ;;
    esac
    case "$what" in
        all|engine)
            if [ "$PORT_ENGINE" = 1 ]; then
                "$DX_ROOT/scripts/engine.sh" build "$PORT"
            elif [ "$what" = engine ]; then
                die "$PORT does not build the engine -- see ports/$PORT/README.md"
            fi
            ;;
        launcher) ;;
        *) die "build what? launcher, engine or nothing for both" ;;
    esac
}

# The app directory is laid out exactly as it ships: the binaries, the
# engine, ports/common/packaging and then the port's own packaging over it.
# launcher.ini belongs to whoever runs the app, so it is only created when
# missing; the packaged one is always there as launcher.ini.default.
cmd_stage() {
    [ -x "$BUILD/launcher/deusex-launcher" ] || die "no launcher build -- scripts/dx.sh build $PORT"
    mkdir -p "$APP"
    cmake --install "$BUILD/launcher" --prefix "$APP" >/dev/null

    local pkg; pkg="$(mktemp -d)"
    cp -a "$DX_ROOT/ports/common/packaging/." "$pkg/"
    [ -d "$PORT_DIR/packaging" ] && cp -a "$PORT_DIR/packaging/." "$pkg/"
    mv "$pkg/launcher.ini" "$pkg/launcher.ini.default"
    cp -a "$pkg/." "$APP/"
    rm -rf "$pkg"
    chmod +x "$APP"/*.sh
    local fresh=0
    if [ ! -s "$APP/launcher.ini" ]; then cp "$APP/launcher.ini.default" "$APP/launcher.ini"; fresh=1; fi

    if [ "$PORT_ENGINE" = 1 ]; then
        local e="$BUILD/engine"
        if [ -x "$e/SurrealEngine" ]; then
            cp "$e/SurrealEngine" "$APP/"
            local f
            for f in libSurrealVideo.so SurrealEngine.pk3; do
                [ -f "$e/$f" ] && cp "$e/$f" "$APP/"
            done
        else
            say "warning: no engine build in $e -- staged the launcher only (scripts/dx.sh build $PORT engine)"
        fi
    fi

    port_stage "$fresh"
    say "staged $APP"
}

cmd_test() {
    cd "$DX_ROOT"
    cmake --preset linux-x86_64 >/dev/null
    cmake --build --preset linux-x86_64 -j"$(nproc)"
    ctest --preset linux-x86_64
}

cmd_check() {
    local rc=0 p
    say "== docs"
    "$DX_ROOT/scripts/check-docs.sh" || rc=1

    say "== engine pin"
    if [ -d "$DX_ROOT/engine/SurrealEngine/.git" ]; then
        "$DX_ROOT/scripts/engine.sh" check || rc=1
    else
        say "skipped: no engine clone (scripts/engine.sh fetch)"
    fi

    say "== ports"
    for p in $(dx_ports); do
        local dir="$DX_ROOT/ports/$p" f missing=""
        for f in port.cmake port.sh README.md; do
            [ -f "$dir/$f" ] || missing="$missing $f"
        done
        if grep -q '^PORT_ENGINE=1' "$dir/port.sh" 2>/dev/null && [ ! -f "$dir/engine.cmake" ]; then
            missing="$missing engine.cmake"
        fi
        if [ -n "$missing" ]; then echo "FAIL ports/$p lacks$missing" >&2; rc=1; else echo "OK   ports/$p"; fi
    done

    say "== ABI of staged cross builds"
    for p in $(dx_ports); do
        local max bins=()
        max=$(sed -n 's/^[[:space:]]*set(DXL_PORT_GLIBC_MAX[[:space:]]\{1,\}\([0-9.]\{1,\}\))/\1/p' "$DX_ROOT/ports/$p/port.cmake" | head -n 1)
        [ -n "$max" ] || continue
        for f in deusex-launcher dxl-cli SurrealEngine; do
            [ -f "$DX_ROOT/build/$p/app/$f" ] && bins+=("$DX_ROOT/build/$p/app/$f")
        done
        [ ${#bins[@]} -gt 0 ] || continue
        "$DX_ROOT/scripts/check-abi.sh" --max "$max" "${bins[@]}" || rc=1
    done

    [ "$rc" = 0 ] && say "all checks passed" || say "checks FAILED"
    return "$rc"
}

cmd_ports() {
    local p
    for p in $(dx_ports); do
        ( load_port "$p"; printf '%-18s %s\n' "$p" "$PORT_DESC" )
    done
}

# A deploy is always of the current tree: build (incremental) and stage first,
# then the port's port_deploy. An engine with uncommitted changes -- the
# profiling hooks, most likely -- ships with a warning, not silently.
cmd_deploy() {
    cmd_build
    cmd_stage
    local eng="$DX_ROOT/engine/SurrealEngine"
    if [ "$PORT_ENGINE" = 1 ] && [ -d "$eng/.git" ] && ! git -C "$eng" diff --quiet HEAD; then
        say "warning: the engine has uncommitted changes (profiling hooks? scripts/engine.sh perf off) -- deploying them"
    fi
    port_deploy "$@"
}

cmd="${1:-}"; [ -n "$cmd" ] || usage; shift
case "$cmd" in
    ports)  cmd_ports ;;
    deps)   load_port "${1:-}"; shift || true; port_deps "$@" ;;
    build)  load_port "${1:-}"; shift || true; cmd_build "$@" ;;
    stage)  load_port "${1:-}"; cmd_stage ;;
    deploy) load_port "${1:-}"; shift || true; cmd_deploy "$@" ;;
    run)    load_port "${1:-}"; shift || true
            [ -x "$APP/deusex-launcher" ] || die "nothing staged -- scripts/dx.sh stage $PORT"
            port_run "$@" ;;
    profile) load_port "${1:-}"; shift || true; port_profile "$@" ;;
    test)   cmd_test ;;
    check)  cmd_check ;;
    -h|--help|help) usage ;;
    *)      die "unknown command '$cmd' (scripts/dx.sh help)" ;;
esac
