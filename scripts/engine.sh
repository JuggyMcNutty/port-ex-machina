#!/usr/bin/env bash
# The Surreal Engine fork: engine/SurrealEngine, branch deusex-handheld, which
# is upstream at engine-patches/UPSTREAM-BASE.txt plus one commit per patch.
#
#   scripts/engine.sh fetch                   clone upstream and apply engine-patches/ (if absent)
#   scripts/engine.sh build <port>            build/<port>/engine from ports/<port>/engine.cmake
#   scripts/engine.sh check                   the fork's commits are exactly engine-patches/*.patch
#   scripts/engine.sh status                  the pin, and how far upstream has moved past it
#   scripts/engine.sh upgrade [<ref>]         move the pin to an upstream commit (default: its latest);
#                             --continue|--abort   after resolving conflicts, or to give up
#   scripts/engine.sh export <commit> <NNNN-name>   write a fork commit to engine-patches/
#   scripts/engine.sh perf on|off|save        apply/revert engine-patches/optional/perf-instrumentation.patch;
#                                             save writes it from the tree (after re-basing the hooks)
#
# The fork stays a fork: see docs/ENGINE.md before sharing any of it.
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

ENGINE_DIR="${ENGINE_DIR:-$DX_ROOT/engine/SurrealEngine}"
PATCHES="$DX_ROOT/engine-patches"
UPSTREAM="https://github.com/dpjudas/SurrealEngine.git"
UPSTREAM_REF=origin/master          # upstream's own branch, as the clone fetches it
BRANCH=deusex-handheld

usage() { sed -n '2,14p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2; }
base() { awk 'NR == 1 { print $1 }' "$PATCHES/UPSTREAM-BASE.txt"; }
eng() { git -C "$ENGINE_DIR" "$@"; }

need_clone() { [ -d "$ENGINE_DIR/.git" ] || die "no engine clone -- scripts/engine.sh fetch"; }

# Applies engine-patches/ in order onto the current branch. Each fork commit
# was committed by its author at its author date, so applying a patch with
# that identity and date reproduces the commit id the patch records -- on any
# machine, whatever its git identity.
apply_patches() {
    local p from
    for p in "$PATCHES"/[0-9][0-9][0-9][0-9]-*.patch; do
        from=$(sed -n 's/^From: //p' "$p" | head -n 1)
        GIT_COMMITTER_NAME="${from% <*}" GIT_COMMITTER_EMAIL="$(printf '%s' "$from" | sed 's/.*<\(.*\)>.*/\1/')" \
            eng am --quiet --committer-date-is-author-date "$p"
    done
}

cmd_fetch() {
    if [ -d "$ENGINE_DIR/.git" ]; then
        say "engine clone present: $ENGINE_DIR"
        return 0
    fi
    mkdir -p "$(dirname "$ENGINE_DIR")"
    git clone "$UPSTREAM" "$ENGINE_DIR"
    eng checkout -b "$BRANCH" "$(base)"
    apply_patches
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
    need_clone
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

# ---- the pin, and moving it ------------------------------------------------
# The engine follows upstream only when someone asks: "upgrade" rebases the
# fork onto the upstream commit it is given, writes each rebased commit back
# to its own patch file, moves UPSTREAM-BASE.txt, and rebuilds the branch from
# the patches as "fetch" would, so the commit ids stay reproducible.

oneline() { eng log -1 --format='%h  %cs  %s' "$1"; }
commits() { if [ "$1" = 1 ]; then echo "1 commit"; else echo "$1 commits"; fi; }

cmd_status() {
    need_clone
    eng fetch --quiet --force --tags origin
    local b n; b="$(base)"
    printf 'pinned    %s\n' "$(oneline "$b")"
    printf 'upstream  %s  (%s)\n' "$(oneline "$UPSTREAM_REF")" "$UPSTREAM_REF"
    n=$(eng rev-list --count "$b..$UPSTREAM_REF")
    if [ "$n" = 0 ]; then
        echo "the pin is upstream's latest"
    else
        echo "upstream is $(commits "$n") past the pin -- scripts/engine.sh upgrade takes them in"
    fi
    if cmd_check >/dev/null 2>&1; then
        echo "the fork matches engine-patches/ ($(eng rev-list --count "$b..$BRANCH") patches)"
    else
        echo "the fork does not match engine-patches/ -- scripts/engine.sh check"
    fi
    [ ! -f "$(upgrade_state)" ] || echo "an upgrade is in progress -- scripts/engine.sh upgrade --continue or --abort"
}

upgrade_state() { printf '%s/dx-upgrade\n' "$(eng rev-parse --absolute-git-dir)"; }
rebasing() {
    local g; g="$(eng rev-parse --absolute-git-dir)"
    [ -d "$g/rebase-merge" ] || [ -d "$g/rebase-apply" ]
}

# The subject a patch file records, unfolded, without its [PATCH] tag.
patch_subject() {
    awk '/^Subject: / { s = substr($0, 10); sub(/^\[PATCH[^]]*\] /, "", s)
                        while ((getline l) > 0 && l ~ /^[ \t]/) s = s l
                        print s; exit }' "$1"
}

cmd_upgrade() {
    need_clone
    local state; state="$(upgrade_state)"
    case "${1:-}" in
        --continue)
            [ -f "$state" ] || die "no upgrade in progress"
            if rebasing; then
                eng -c core.editor=true rebase --continue ||
                    die "still conflicts -- resolve them in $ENGINE_DIR, then scripts/engine.sh upgrade --continue"
            fi
            upgrade_finish "$(cat "$state")"
            return ;;
        --abort)
            [ -f "$state" ] || die "no upgrade in progress"
            if rebasing; then eng rebase --abort; fi
            rm -f "$state"
            say "upgrade abandoned: $BRANCH is as it was, the pin unchanged"
            return ;;
        -*) die "upgrade [<upstream ref>] | --continue | --abort" ;;
    esac
    [ ! -f "$state" ] || die "an upgrade is in progress -- scripts/engine.sh upgrade --continue or --abort"
    [ "$(eng rev-parse --abbrev-ref HEAD)" = "$BRANCH" ] || die "the engine clone is not on $BRANCH"
    [ -z "$(eng status --porcelain --untracked-files=no)" ] ||
        die "uncommitted changes in the engine tree (the profiling hooks? scripts/engine.sh perf off)"
    cmd_check >/dev/null || die "the fork does not match engine-patches/ -- scripts/engine.sh check"

    local ref="${1:-$UPSTREAM_REF}" b target
    b="$(eng rev-parse "$(base)^{commit}")"
    eng fetch --quiet --force --tags origin
    target="$(eng rev-parse --verify --quiet "$ref^{commit}")" || die "no such commit in the engine clone: $ref"
    if [ "$target" = "$b" ]; then
        say "already pinned at $ref ($(eng rev-parse --short "$b"))"
        return 0
    fi
    eng merge-base --is-ancestor "$b" "$target" ||
        say "note: $ref does not descend from the pin -- upstream's history was rewritten, or it is older"
    say "upgrading over $(commits "$(eng rev-list --count "$b..$target")") of upstream, $(eng rev-parse --short "$b") -> $(eng rev-parse --short "$target")"
    printf '%s\n' "$target" > "$state"
    if ! eng rebase --onto "$target" "$b" "$BRANCH"; then
        say "the patches conflict with upstream: resolve them in $ENGINE_DIR (git add each file),"
        say "then scripts/engine.sh upgrade --continue -- or --abort to leave everything as it was"
        return 1
    fi
    upgrade_finish "$target"
}

upgrade_finish() {
    local target="$1" state; state="$(upgrade_state)"
    [ "$(eng rev-parse --abbrev-ref HEAD)" = "$BRANCH" ] && eng merge-base --is-ancestor "$target" HEAD ||
        die "$BRANCH is not on $(eng rev-parse --short "$target") yet -- finish the rebase in $ENGINE_DIR"
    local commits files keep=() dropped=() f i=0
    mapfile -t commits < <(eng rev-list --reverse "$target..$BRANCH")
    mapfile -t files < <(ls "$PATCHES"/[0-9][0-9][0-9][0-9]-*.patch)
    # A patch the rebase has no commit for became empty: upstream has it now.
    for f in "${files[@]}"; do
        if [ "$i" -lt "${#commits[@]}" ] && [ "$(eng log -1 --format=%s "${commits[$i]}")" = "$(patch_subject "$f")" ]; then
            keep+=("$f"); i=$((i + 1))
        else
            dropped+=("$f")
        fi
    done
    [ "$i" = "${#commits[@]}" ] ||
        die "$BRANCH's commits over the new base do not line up with engine-patches/ -- sort it out by hand, then upgrade --continue"
    for i in "${!commits[@]}"; do eng format-patch -1 "${commits[$i]}" --stdout > "${keep[$i]}"; done
    for f in "${dropped[@]}"; do
        rm -f -- "$f"
        say "dropped $(basename "$f"): upstream's new commits make it empty"
    done
    printf '%s %s\n' "$target" "$(eng log -1 --format=%s "$target")" > "$PATCHES/UPSTREAM-BASE.txt"
    # Rebuilt as fetch builds it, so the commit ids are reproducible, then
    # exported once more so the patch files record those ids.
    eng checkout --quiet -B "$BRANCH" "$target"
    apply_patches
    mapfile -t commits < <(eng rev-list --reverse "$target..$BRANCH")
    for i in "${!commits[@]}"; do eng format-patch -1 "${commits[$i]}" --stdout > "${keep[$i]}"; done
    rm -f "$state"
    cmd_check >/dev/null
    say "pinned at $(eng rev-parse --short "$target"); ${#keep[@]} patches re-exported, ${#dropped[@]} dropped. Next:"
    say "  build and run linux-x86_64; scripts/engine.sh perf on (then perf save if the hooks moved);"
    say "  profile on the devices; commit engine-patches/ with docs/ENGINE.md brought up to date."
}

cmd_export() {
    local c="${1:-}" name="${2:-}"
    [ -n "$c" ] && [ -n "$name" ] || die "usage: scripts/engine.sh export <commit> <NNNN-name>"
    eng format-patch -1 "$c" --stdout > "$PATCHES/${name%.patch}.patch"
    say "wrote engine-patches/${name%.patch}.patch"
}

# The profiling hooks are a patch against the fork's head, so a fork commit
# that touches the same lines moves them. "on" falls back to a three-way
# merge; after one (and after resolving any conflict it leaves), "save"
# rewrites the patch from the tree so "off" and the next "on" apply cleanly.
cmd_perf() {
    local p="$PATCHES/optional/perf-instrumentation.patch"
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
            say "wrote engine-patches/optional/perf-instrumentation.patch from the engine tree"
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
    upgrade) cmd_upgrade "$@" ;;
    export) cmd_export "$@" ;;
    perf)   cmd_perf "$@" ;;
    -h|--help|help) usage ;;
    *)      die "unknown command '$cmd' (scripts/engine.sh help)" ;;
esac
