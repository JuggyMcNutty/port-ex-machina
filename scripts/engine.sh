#!/usr/bin/env bash
# The engine: engine/SurrealEngine, a clone of VibeEngine, our fork of
# dpjudas/SurrealEngine. ENGINE-PIN.txt (repo, branch, commit) names the one
# commit this workspace builds.
#
#   scripts/engine.sh fetch                   clone the fork at the pin (if absent)
#   scripts/engine.sh build <port>            build/<port>/engine from the port's engine.cmake
#   scripts/engine.sh check                   the clone is at the pin, on its branch
#   scripts/engine.sh status                  the pin, the fork, and how far upstream has moved
#   scripts/engine.sh pin                     write the clone's HEAD to ENGINE-PIN.txt
#
# The fork's own tools -- the profiling hooks, merging upstream -- are in its
# vibe/tools, and how it is kept is its vibe/docs/ENGINE.md.
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

ENGINE_DIR="${ENGINE_DIR:-$DX_ROOT/engine/SurrealEngine}"
PIN_FILE="$DX_ROOT/ENGINE-PIN.txt"
UPSTREAM="https://github.com/dpjudas/SurrealEngine.git"

[ -f "$PIN_FILE" ] || die "no ENGINE-PIN.txt at the repository root"
pinned() { awk -v k="$1" '$1 == k { print $2 }' "$PIN_FILE"; }
FORK_URL="$(pinned repo)"
BRANCH="$(pinned branch)"

usage() { sed -n '2,11p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2; }
eng() { git -C "$ENGINE_DIR" "$@"; }

need_clone() { [ -d "$ENGINE_DIR/.git" ] || die "no engine clone -- scripts/engine.sh fetch"; }
ensure_upstream() { eng remote get-url upstream >/dev/null 2>&1 || eng remote add upstream "$UPSTREAM"; }

cmd_fetch() {
    if [ -d "$ENGINE_DIR/.git" ]; then
        say "engine clone present: $ENGINE_DIR"
        return 0
    fi
    mkdir -p "$(dirname "$ENGINE_DIR")"
    git clone --branch "$BRANCH" "$FORK_URL" "$ENGINE_DIR"
    ensure_upstream
    eng checkout -q -B "$BRANCH" "$(pinned commit)"
    eng config user.name JuggyMcNutty
    eng config user.email 11588877+JuggyMcNutty@users.noreply.github.com
    say "engine ready: $ENGINE_DIR ($BRANCH at $(eng rev-parse --short HEAD))"
}

# Cross builds need a zipdir that runs here: it packs the resource zip at
# build time.
host_zipdir() {
    local tools="$DX_ROOT/build/host-tools"
    if [ ! -x "$tools/zipdir" ]; then
        say "building a host zipdir ..."
        cmake -S "$ENGINE_DIR" -B "$tools/engine" -DCMAKE_BUILD_TYPE=Release >/dev/null
        cmake --build "$tools/engine" --target zipdir -j"$(nproc)" >/dev/null
        cp "$tools/engine/zipdir" "$tools/zipdir"
    fi
    printf '%s\n' "$tools/zipdir"
}

cmd_build() {
    local port="${1:-}"
    [ -n "$port" ] || die "which port?"
    # The port's files are on its launcher branch (scripts/launcher.sh fetch).
    local src="$DX_ROOT/launcher/$port"
    local pdir="$src/ports/$port"
    local cfg="$pdir/engine.cmake"
    [ -f "$pdir/port.sh" ] || die "no port '$port' in launcher/$port -- scripts/launcher.sh fetch"
    # port.sh decides whether the engine builds for this port on this machine.
    local ships; ships=$(PORT_ENGINE=0; . "$pdir/port.sh"; echo "$PORT_ENGINE")
    [ "$ships" = 1 ] && [ -f "$cfg" ] || die "port $port does not build the engine here -- see its ports/$port/README.md"
    [ -d "$ENGINE_DIR/.git" ] || die "no engine clone -- scripts/engine.sh fetch"
    local bdir="$DX_ROOT/build/$port/engine" extra=()
    if grep -q 'CMAKE_TOOLCHAIN_FILE' "$cfg"; then
        extra+=("-DZIPDIR_EXECUTABLE=$(host_zipdir)")
    fi
    cmake -S "$ENGINE_DIR" -B "$bdir" -C "$cfg" "${extra[@]}"
    cmake --build "$bdir" --target SurrealEngine -j"$(nproc)"
    local max
    max=$(sed -n 's/^[[:space:]]*set(DXL_PORT_GLIBC_MAX[[:space:]]\{1,\}\([0-9.]\{1,\}\))/\1/p' "$pdir/port.cmake" | head -n 1)
    if [ -n "$max" ]; then
        "$src/scripts/check-abi.sh" --max "$max" "$bdir/SurrealEngine" "$bdir/libSurrealVideo.so"
    fi
}

cmd_check() {
    need_clone
    local want head rc=0
    want="$(pinned commit)"
    eng cat-file -e "$want^{commit}" 2>/dev/null ||
        die "the pinned commit $want is not in the clone -- git -C engine/SurrealEngine fetch origin"
    head="$(eng rev-parse HEAD)"
    if [ "$head" = "$want" ]; then
        echo "OK   the clone is at the pin: $(eng log -1 --format='%h %s')"
    else
        echo "FAIL the clone is not at the pin" >&2
        echo "     pinned $(eng log -1 --format='%h %s' "$want")" >&2
        echo "     HEAD   $(eng log -1 --format='%h %s' "$head")" >&2
        if eng merge-base --is-ancestor "$want" "$head" 2>/dev/null; then
            echo "     HEAD is $(eng rev-list --count "$want..$head") commits past the pin -- scripts/engine.sh pin records it" >&2
        fi
        rc=1
    fi
    if [ "$(eng rev-parse --abbrev-ref HEAD)" != "$BRANCH" ]; then
        echo "FAIL the clone is not on $BRANCH" >&2
        rc=1
    fi
    local dirty
    dirty=$(eng status --porcelain --untracked-files=no)
    [ -z "$dirty" ] || say "note: uncommitted changes in the engine tree:"$'\n'"$dirty"
    return "$rc"
}

oneline() { eng log -1 --format='%h  %cs  %s' "$1"; }

cmd_status() {
    need_clone
    eng fetch --quiet origin
    local want
    want="$(pinned commit)"
    printf 'pinned    %s\n' "$(oneline "$want")"
    printf 'fork      %s  (origin/%s)\n' "$(oneline "origin/$BRANCH")" "$BRANCH"
    "$ENGINE_DIR/vibe/tools/upgrade.sh" status
    if [ "$(eng rev-parse HEAD)" = "$(eng rev-parse "$want^{commit}")" ]; then
        echo "the clone is at the pin"
    else
        echo "the clone is not at the pin -- scripts/engine.sh check"
    fi
}

# The pin moves only to a commit the fork repository has: a pin no one can
# fetch would break every other clone's cold start.
cmd_pin() {
    need_clone
    [ "$(eng rev-parse --abbrev-ref HEAD)" = "$BRANCH" ] || die "the engine clone is not on $BRANCH"
    [ -z "$(eng status --porcelain --untracked-files=no)" ] ||
        die "uncommitted changes in the engine tree -- commit them (or vibe/tools/perf/perf.sh off) first"
    ! eng grep -q 'TEMPORARY DEBUG TOOL' "$BRANCH" -- SurrealEngine 2>/dev/null ||
        die "TEMPORARY DEBUG TOOL hooks are committed on $BRANCH -- remove them before pinning"
    local head; head="$(eng rev-parse HEAD)"
    eng fetch --quiet origin
    eng merge-base --is-ancestor "$head" "origin/$BRANCH" 2>/dev/null ||
        die "HEAD is not on origin/$BRANCH -- push it first, or fetch cannot reproduce this pin"
    {
        printf '# The engine this workspace builds: scripts/engine.sh fetch clones it, check\n'
        printf '# verifies it, pin (after pushing a fork commit) moves it. The fork'\''s vibe/docs/ENGINE.md.\n'
        printf 'repo    %s\n' "$FORK_URL"
        printf 'branch  %s\n' "$BRANCH"
        printf 'commit  %s\n' "$head"
    } > "$PIN_FILE"
    say "pinned $BRANCH at $(eng log -1 --format='%h %s') -- commit ENGINE-PIN.txt with what depends on it"
}


cmd="${1:-}"; [ -n "$cmd" ] || usage; shift
case "$cmd" in
    fetch)  cmd_fetch ;;
    build)  cmd_build "$@" ;;
    check)  cmd_check ;;
    status) cmd_status ;;
    pin)    cmd_pin ;;
    upgrade) die "moved: engine/SurrealEngine/vibe/tools/upgrade.sh" ;;
    perf)   die "moved: engine/SurrealEngine/vibe/tools/perf/perf.sh" ;;
    export) die "retired: the fork is its own repository -- commit there, push, then scripts/engine.sh pin" ;;
    -h|--help|help) usage ;;
    *)      die "unknown command '$cmd' (scripts/engine.sh help)" ;;
esac
