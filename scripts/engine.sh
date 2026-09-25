#!/usr/bin/env bash
# The Surreal Engine fork: engine/SurrealEngine, a clone of our own repository,
# forked from dpjudas/SurrealEngine. ENGINE-PIN.txt (repo, branch, commit)
# names the one commit this repository builds.
#
#   scripts/engine.sh fetch                   clone the fork at the pin (if absent)
#   scripts/engine.sh build <port>            build/<port>/engine from ports/<port>/engine.cmake
#   scripts/engine.sh check                   the clone is at the pin, on its branch
#   scripts/engine.sh status                  the pin, the fork, and how far upstream has moved
#   scripts/engine.sh pin                     write the clone's HEAD to ENGINE-PIN.txt
#   scripts/engine.sh upgrade [<ref>]         merge upstream into the fork (default: its latest);
#                             --continue|--abort   after resolving conflicts, or to give up
#   scripts/engine.sh perf on|off|save        apply/revert scripts/perf-instrumentation.patch;
#                                             save writes it from the tree (after re-basing the hooks)
#
# The fork stays a fork: see docs/ENGINE.md before sharing any of it.
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

ENGINE_DIR="${ENGINE_DIR:-$DX_ROOT/engine/SurrealEngine}"
PIN_FILE="$DX_ROOT/ENGINE-PIN.txt"
UPSTREAM="https://github.com/dpjudas/SurrealEngine.git"
UPSTREAM_REF=upstream/master        # upstream's own branch, as the clone fetches it

[ -f "$PIN_FILE" ] || die "no ENGINE-PIN.txt at the repository root"
pinned() { awk -v k="$1" '$1 == k { print $2 }' "$PIN_FILE"; }
FORK_URL="$(pinned repo)"
BRANCH="$(pinned branch)"

usage() { sed -n '2,14p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2; }
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
commits() { if [ "$1" = 1 ]; then echo "1 commit"; else echo "$1 commits"; fi; }

cmd_status() {
    need_clone
    ensure_upstream
    eng fetch --quiet --force --tags upstream
    eng fetch --quiet origin
    local want mb n
    want="$(pinned commit)"
    printf 'pinned    %s\n' "$(oneline "$want")"
    printf 'fork      %s  (origin/%s)\n' "$(oneline "origin/$BRANCH")" "$BRANCH"
    printf 'upstream  %s  (%s)\n' "$(oneline "$UPSTREAM_REF")" "$UPSTREAM_REF"
    mb="$(eng merge-base "$UPSTREAM_REF" "$BRANCH")"
    n=$(eng rev-list --count "$mb..$UPSTREAM_REF")
    if [ "$n" = 0 ]; then
        echo "the fork has all of upstream"
    else
        echo "upstream is $(commits "$n") past the fork -- scripts/engine.sh upgrade merges them in"
    fi
    if [ "$(eng rev-parse HEAD)" = "$(eng rev-parse "$want^{commit}")" ]; then
        echo "the clone is at the pin"
    else
        echo "the clone is not at the pin -- scripts/engine.sh check"
    fi
    [ ! -f "$(eng rev-parse --absolute-git-dir)/MERGE_HEAD" ] ||
        echo "a merge is in progress -- scripts/engine.sh upgrade --continue or --abort"
}

# The pin moves only to a commit the fork repository has: a pin no one can
# fetch would break every other clone's cold start.
cmd_pin() {
    need_clone
    [ "$(eng rev-parse --abbrev-ref HEAD)" = "$BRANCH" ] || die "the engine clone is not on $BRANCH"
    [ -z "$(eng status --porcelain --untracked-files=no)" ] ||
        die "uncommitted changes in the engine tree -- commit them (or scripts/engine.sh perf off) first"
    ! eng grep -q 'TEMPORARY DEBUG TOOL' "$BRANCH" -- SurrealEngine 2>/dev/null ||
        die "TEMPORARY DEBUG TOOL hooks are committed on $BRANCH -- remove them before pinning"
    local head; head="$(eng rev-parse HEAD)"
    eng fetch --quiet origin
    eng merge-base --is-ancestor "$head" "origin/$BRANCH" 2>/dev/null ||
        die "HEAD is not on origin/$BRANCH -- push it first, or fetch cannot reproduce this pin"
    {
        printf '# The engine fork this repository builds: scripts/engine.sh fetch clones it,\n'
        printf '# check verifies it, pin (after pushing a fork commit) moves it. docs/ENGINE.md.\n'
        printf 'repo    %s\n' "$FORK_URL"
        printf 'branch  %s\n' "$BRANCH"
        printf 'commit  %s\n' "$head"
    } > "$PIN_FILE"
    say "pinned $BRANCH at $(eng log -1 --format='%h %s') -- commit ENGINE-PIN.txt with what depends on it"
}

cmd_upgrade() {
    need_clone
    ensure_upstream
    local g; g="$(eng rev-parse --absolute-git-dir)"
    case "${1:-}" in
        --continue)
            [ -f "$g/MERGE_HEAD" ] || die "no merge in progress"
            eng -c core.editor=true merge --continue ||
                die "still conflicts -- resolve them in $ENGINE_DIR (git add each file), then scripts/engine.sh upgrade --continue"
            upgrade_finish
            return ;;
        --abort)
            [ -f "$g/MERGE_HEAD" ] || die "no merge in progress"
            eng merge --abort
            say "upgrade abandoned: $BRANCH is as it was, the pin unchanged"
            return ;;
        -*) die "upgrade [<upstream ref>] | --continue | --abort" ;;
    esac
    [ ! -f "$g/MERGE_HEAD" ] || die "a merge is in progress -- scripts/engine.sh upgrade --continue or --abort"
    [ "$(eng rev-parse --abbrev-ref HEAD)" = "$BRANCH" ] || die "the engine clone is not on $BRANCH"
    [ -z "$(eng status --porcelain --untracked-files=no)" ] ||
        die "uncommitted changes in the engine tree (the profiling hooks? scripts/engine.sh perf off)"
    [ "$(eng rev-parse HEAD)" = "$(eng rev-parse "$(pinned commit)^{commit}")" ] ||
        die "the clone is not at the pin -- scripts/engine.sh check"

    eng fetch --quiet --force --tags upstream
    local ref="${1:-$UPSTREAM_REF}" target
    target="$(eng rev-parse --verify --quiet "$ref^{commit}")" || die "no such commit in the engine clone: $ref"
    if eng merge-base --is-ancestor "$target" HEAD; then
        say "the fork already has $ref ($(eng rev-parse --short "$target"))"
        return 0
    fi
    say "merging $(commits "$(eng rev-list --count "HEAD..$target")") of upstream into $BRANCH"
    if ! eng merge -m "Merge upstream SurrealEngine: $(eng log -1 --format=%s "$target")" "$target"; then
        say "upstream conflicts with the fork: resolve the files in $ENGINE_DIR (git add each one),"
        say "then scripts/engine.sh upgrade --continue -- or --abort to leave everything as it was"
        return 1
    fi
    upgrade_finish
}

upgrade_finish() {
    say "merged. Next:"
    say "  build and run linux-x86_64; check the Vulkan validation layer;"
    say "  scripts/engine.sh perf on (then perf save if the hooks moved); profile on the devices;"
    say "  push $BRANCH, then scripts/engine.sh pin, committed with docs/ENGINE.md brought up to date."
}

# The profiling hooks are a patch against the fork's head, so a fork commit
# that touches the same lines moves them. "on" falls back to a three-way
# merge; after one (and after resolving any conflict it leaves), "save"
# rewrites the patch from the tree so "off" and the next "on" apply cleanly.
cmd_perf() {
    local p="$DX_ROOT/scripts/perf-instrumentation.patch"
    case "${1:-}" in
        on)
            if eng apply "$p" 2>/dev/null; then
                say "profiling hooks applied -- never commit them (scripts/engine.sh perf off)"
                return 0
            fi
            # A three-way merge needs the index to match the tree: it applies
            # nothing over uncommitted changes to a file the hooks touch.
            local rc=0
            eng apply --3way "$p" || rc=$?
            eng reset -q
            local conflicts; conflicts="$(eng diff --name-only --diff-filter=U; eng grep -l '^<<<<<<< ' -- SurrealEngine 2>/dev/null || true)"
            [ -z "$conflicts" ] || die "the hooks conflict with the fork in: $(echo $conflicts) -- resolve, then scripts/engine.sh perf save"
            eng grep -q 'TEMPORARY DEBUG TOOL' -- SurrealEngine 2>/dev/null ||
                die "the hooks did not apply (git apply --3way: $rc) -- commit the engine's changes first, then perf on"
            say "profiling hooks applied by a three-way merge -- scripts/engine.sh perf save to re-base the patch"
            ;;
        off) eng apply -R "$p" && say "profiling hooks removed" ;;
        save)
            ! eng grep -q '^<<<<<<< \|^>>>>>>> ' -- SurrealEngine 2>/dev/null ||
                die "conflict markers in the engine tree -- resolve them before perf save"
            # New files (PerfLog.h) are untracked: record them for the diff only.
            local new; mapfile -t new < <(eng ls-files --others --exclude-standard -- SurrealEngine)
            [ ${#new[@]} -eq 0 ] || eng add -N -- "${new[@]}"
            eng diff > "$p"
            [ ${#new[@]} -eq 0 ] || eng reset -q -- "${new[@]}"
            grep -q '^+.*TEMPORARY DEBUG TOOL' "$p" || die "no TEMPORARY DEBUG TOOL lines in the tree -- are the hooks applied?"
            say "wrote scripts/perf-instrumentation.patch from the engine tree"
            ;;
        *)   die "perf on|off|save" ;;
    esac
}

cmd="${1:-}"; [ -n "$cmd" ] || usage; shift
case "$cmd" in
    fetch)  cmd_fetch ;;
    build)  cmd_build "$@" ;;
    check)  cmd_check ;;
    status) cmd_status ;;
    pin)    cmd_pin ;;
    upgrade) cmd_upgrade "$@" ;;
    export) die "retired: the fork is its own repository -- commit there, push, then scripts/engine.sh pin" ;;
    perf)   cmd_perf "$@" ;;
    -h|--help|help) usage ;;
    *)      die "unknown command '$cmd' (scripts/engine.sh help)" ;;
esac
