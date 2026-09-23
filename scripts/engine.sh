#!/usr/bin/env bash
# The Surreal Engine fork: engine/SurrealEngine, branch deusex-handheld, which
# is upstream at engine-patches/UPSTREAM-BASE.txt plus one commit per patch.
#
#   scripts/engine.sh fetch                   clone upstream and apply engine-patches/ (if absent)
#   scripts/engine.sh build <port>            build/<port>/engine from ports/<port>/engine.cmake
#   scripts/engine.sh check                   the fork's commits are exactly engine-patches/*.patch
#   scripts/engine.sh export <commit> <NNNN-name>   write a fork commit to engine-patches/
#   scripts/engine.sh perf on|off             apply/revert engine-patches/optional/perf-instrumentation.patch
#
# The fork stays a fork: see engine-patches/README.md before sharing any of it.
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

ENGINE_DIR="${ENGINE_DIR:-$DX_ROOT/engine/SurrealEngine}"
PATCHES="$DX_ROOT/engine-patches"
UPSTREAM="https://github.com/dpjudas/SurrealEngine.git"
BRANCH=deusex-handheld

usage() { sed -n '2,11p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2; }
base() { awk 'NR == 1 { print $1 }' "$PATCHES/UPSTREAM-BASE.txt"; }
eng() { git -C "$ENGINE_DIR" "$@"; }

cmd_fetch() {
    if [ -d "$ENGINE_DIR/.git" ]; then
        say "engine clone present: $ENGINE_DIR"
        return 0
    fi
    mkdir -p "$(dirname "$ENGINE_DIR")"
    git clone "$UPSTREAM" "$ENGINE_DIR"
    eng checkout -b "$BRANCH" "$(base)"
    # Each fork commit was committed by its author at its author date, so
    # applying a patch with that identity and date reproduces the commit id
    # the patch records -- on any machine, whatever its git identity.
    local p from
    for p in "$PATCHES"/[0-9][0-9][0-9][0-9]-*.patch; do
        from=$(sed -n 's/^From: //p' "$p" | head -n 1)
        GIT_COMMITTER_NAME="${from% <*}" GIT_COMMITTER_EMAIL="$(printf '%s' "$from" | sed 's/.*<\(.*\)>.*/\1/')" \
            eng am --committer-date-is-author-date "$p"
    done
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
    local cfg="$DX_ROOT/ports/$port/engine.cmake"
    [ -f "$DX_ROOT/ports/$port/port.sh" ] || die "no port '$port'"
    # port.sh decides whether the engine builds for this port on this machine.
    local ships; ships=$(PORT_ENGINE=0; . "$DX_ROOT/ports/$port/port.sh"; echo "$PORT_ENGINE")
    [ "$ships" = 1 ] && [ -f "$cfg" ] || die "port $port does not build the engine here -- see ports/$port/README.md"
    [ -d "$ENGINE_DIR/.git" ] || die "no engine clone -- scripts/engine.sh fetch"
    local bdir="$DX_ROOT/build/$port/engine" extra=()
    if grep -q 'CMAKE_TOOLCHAIN_FILE' "$cfg"; then
        extra+=("-DZIPDIR_EXECUTABLE=$(host_zipdir)")
    fi
    cmake -S "$ENGINE_DIR" -B "$bdir" -C "$cfg" "${extra[@]}"
    cmake --build "$bdir" --target SurrealEngine -j"$(nproc)"
    local max
    max=$(sed -n 's/^[[:space:]]*set(DXL_PORT_GLIBC_MAX[[:space:]]\{1,\}\([0-9.]\{1,\}\))/\1/p' "$DX_ROOT/ports/$port/port.cmake" | head -n 1)
    if [ -n "$max" ]; then
        "$DX_ROOT/scripts/check-abi.sh" --max "$max" "$bdir/SurrealEngine" "$bdir/libSurrealVideo.so"
    fi
}

# A patch file is its commit's format-patch output. Compared without the
# first line (the commit id), the "index" lines (git abbreviates blob ids
# longer as a repository grows, so the same diff prints differently in a
# fuller clone) and the git version signature at the end.
normalize() { sed -e '1d' -e '/^index [0-9a-f]*\.\.[0-9a-f]*/d' -e '/^-- $/,$d'; }

cmd_check() {
    [ -d "$ENGINE_DIR/.git" ] || die "no engine clone -- scripts/engine.sh fetch"
    local b; b="$(base)"
    eng cat-file -e "$b^{commit}" 2>/dev/null || die "upstream base $b is not in the clone"
    mapfile -t commits < <(eng rev-list --reverse "$b..$BRANCH")
    mapfile -t files < <(ls "$PATCHES"/[0-9][0-9][0-9][0-9]-*.patch)
    local rc=0 i
    if [ "${#commits[@]}" != "${#files[@]}" ]; then
        echo "FAIL $BRANCH has ${#commits[@]} commits over $b, engine-patches/ has ${#files[@]} patches" >&2
        rc=1
    fi
    for i in "${!files[@]}"; do
        local f="${files[$i]}" c="${commits[$i]:-}" name
        name="$(basename "$f")"
        if [ -z "$c" ]; then echo "FAIL $name: no matching commit" >&2; rc=1; continue; fi
        if diff -q <(eng format-patch -1 "$c" --stdout | normalize) <(normalize < "$f") >/dev/null; then
            echo "OK   $name = $(eng log -1 --format='%h %s' "$c")"
        else
            echo "FAIL $name differs from $(eng log -1 --format=%h "$c") -- scripts/engine.sh export $(eng log -1 --format=%h "$c") ${name%.patch}" >&2
            rc=1
        fi
    done
    local dirty
    dirty=$(eng status --porcelain --untracked-files=no)
    [ -z "$dirty" ] || say "note: uncommitted changes in the engine tree (not in any patch):"$'\n'"$dirty"
    return "$rc"
}

cmd_export() {
    local c="${1:-}" name="${2:-}"
    [ -n "$c" ] && [ -n "$name" ] || die "usage: scripts/engine.sh export <commit> <NNNN-name>"
    eng format-patch -1 "$c" --stdout > "$PATCHES/${name%.patch}.patch"
    say "wrote engine-patches/${name%.patch}.patch"
}

cmd_perf() {
    local p="$PATCHES/optional/perf-instrumentation.patch"
    case "${1:-}" in
        on)  eng apply "$p" && say "profiling hooks applied -- never commit them (scripts/engine.sh perf off)" ;;
        off) eng apply -R "$p" && say "profiling hooks removed" ;;
        *)   die "perf on|off" ;;
    esac
}

cmd="${1:-}"; [ -n "$cmd" ] || usage; shift
case "$cmd" in
    fetch)  cmd_fetch ;;
    build)  cmd_build "$@" ;;
    check)  cmd_check ;;
    export) cmd_export "$@" ;;
    perf)   cmd_perf "$@" ;;
    -h|--help|help) usage ;;
    *)      die "unknown command '$cmd' (scripts/engine.sh help)" ;;
esac
