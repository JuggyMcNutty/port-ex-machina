#!/usr/bin/env bash
# GitHub's CI (docs/DEVELOPMENT.md#ci): the steps .github/workflows/ports.yml
# runs around scripts/dx.sh, each a command here so a run can be repeated on
# this machine.
#
#   scripts/ci.sh plan                       what a run builds, as GitHub output lines (CI_* below)
#   scripts/ci.sh link <port>                deps/ and gamefiles/ linked into deusex-launcher/<port>
#   scripts/ci.sh proton                     the pinned Proton where the Wine helpers look; prints its name
#   scripts/ci.sh warnings <log>             fails on a compiler or linker warning in our own code
#   scripts/ci.sh skips <log> [<test>]...    fails on a skipped test not named
#   scripts/ci.sh package <port>             a staged port as the release's file; prints its path
#   scripts/ci.sh notes                      the release's notes, markdown, for the workspace's HEAD
#
# plan reads the caller's event from CI_PORTS (pins: the workspace's pinned
# set; all: every pinned port; a port; main: the recreation), CI_REPO,
# CI_EVENT, CI_REF, CI_SHA and CI_BEFORE (a push's previous commit, a pull
# request's base).
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

OWNER_URL="https://github.com/JuggyMcNutty"

usage() { sed -n '2,17p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2; }

# What no build reads: the docs, and what is never built. Not the notices
# the apps carry (the engine's LICENSE.md, the launcher's THIRD-PARTY.md).
# Each repository's .github/workflows/build.yml filters its triggers the same
# way.
is_code() {   # <repository> <path>
    case "$1:$2" in
        DXEngine:LICENSE.md|deusex-launcher:THIRD-PARTY.md) return 0 ;;
        *.md|*:docs/*) return 1 ;;
        port-ex-machina:LICENSE|port-ex-machina:tools/probes/*|port-ex-machina:scripts/check-docs.sh) return 1 ;;
        DXEngine:tools/dxcap/*|DXEngine:tools/dxcap.sh|DXEngine:tools/perf/*) return 1 ;;
        DXEngine:tools/host-tools.sh|DXEngine:tools/natives_audit.py) return 1 ;;
    esac
    return 0
}

engine_pin() { awk '$1 == "commit" { print $2 }'; }
launcher_pin() { awk -v p="$1" '$1 == "port" && $2 == p { print $3 }'; }

# code_between <repository> <old> <new>: whether code changed between two of
# its commits. A blobless bare clone has the trees git diff needs.
code_between() {
    local repo="$1" old="$2" new="$3" dir="$CI_TMP/$1.git" f
    [ -d "$dir" ] || git clone -q --bare --filter=blob:none "$OWNER_URL/$repo.git" "$dir"
    if ! git -C "$dir" cat-file -e "$old^{commit}" 2>/dev/null; then
        say "$repo: $old is gone from it -- building"
        return 0
    fi
    git -C "$dir" cat-file -e "$new^{commit}" 2>/dev/null || die "$repo has no commit $new"
    while read -r f; do
        if is_code "$repo" "$f"; then say "$repo: code changed ($f)"; return 0; fi
    done < <(git -C "$dir" diff --no-renames --name-only "$old" "$new")
    say "$repo: docs only between ${old:0:7} and ${new:0:7}"
    return 1
}

# The workspace's push builds when code changed: its own, or the engine's or a
# port's between the commits its pins named before and name now.
pins_changed_code() {
    local before="${CI_BEFORE:-}" sha="$CI_SHA" f p old new
    if [ "$CI_EVENT" = workflow_dispatch ]; then
        say "run by hand -- building"
        return 0
    fi
    if [ -z "$before" ] || [ "$before" = 0000000000000000000000000000000000000000 ] ||
       ! git -C "$DX_WORKSPACE" cat-file -e "$before^{commit}" 2>/dev/null; then
        say "cannot tell what changed -- building"
        return 0
    fi
    while read -r f; do
        case "$f" in ENGINE-PIN.txt|LAUNCHER-PIN.txt) continue ;; esac
        if is_code port-ex-machina "$f"; then say "port-ex-machina: code changed ($f)"; return 0; fi
    done < <(git -C "$DX_WORKSPACE" diff --no-renames --name-only "$before" "$sha")
    old="$(git -C "$DX_WORKSPACE" show "$before:ENGINE-PIN.txt" | engine_pin)"
    new="$(engine_pin < "$DX_WORKSPACE/ENGINE-PIN.txt")"
    if [ "$old" != "$new" ] && code_between DXEngine "$old" "$new"; then return 0; fi
    for p in $(dx_ports); do
        old="$(git -C "$DX_WORKSPACE" show "$before:LAUNCHER-PIN.txt" | launcher_pin "$p")"
        new="$(launcher_pin "$p" < "$DX_WORKSPACE/LAUNCHER-PIN.txt")"
        [ "$old" != "$new" ] || continue
        if [ -z "$old" ]; then say "$p: newly pinned -- building"; return 0; fi
        if code_between deusex-launcher "$old" "$new"; then return 0; fi
    done
    return 1
}

cmd_plan() {
    local repo="${CI_REPO##*/}" ports="${CI_PORTS:?}" engine p list=() build=true release=false
    CI_TMP="$(mktemp -d)"; trap 'rm -rf "$CI_TMP"' EXIT
    engine="$(engine_pin < "$DX_WORKSPACE/ENGINE-PIN.txt")"
    [ "$repo" != DXEngine ] || engine="$CI_SHA"
    case "$ports" in
        pins|all) mapfile -t list < <(dx_ports) ;;
        main)     ;;
        *)        list=("$ports") ;;
    esac
    if [ "$ports" = pins ] && ! pins_changed_code; then
        build=false
        say "no code changed: nothing to build"
    fi
    if [ "$ports" = pins ] && [ "$build" = true ] && [ "$CI_REF" = refs/heads/main ] &&
       { [ "$CI_EVENT" = push ] || [ "$CI_EVENT" = workflow_dispatch ]; }; then
        release=true
    fi
    [ "$build" = true ] || list=()

    # Each port's launcher commit: the caller's own, or the pin.
    local matrix="" launcher
    for p in "${list[@]}"; do
        launcher="$(launcher_pin "$p" < "$DX_WORKSPACE/LAUNCHER-PIN.txt")"
        [ "$repo" != deusex-launcher ] || launcher="$CI_SHA"
        [ -n "$launcher" ] || die "$p is not pinned in LAUNCHER-PIN.txt"
        matrix+="${matrix:+,}{\"port\":\"$p\",\"launcher\":\"$launcher\"}"
    done
    printf 'workspace=%s\n' "$(git -C "$DX_WORKSPACE" rev-parse HEAD)"
    printf 'engine=%s\n' "$engine"
    printf 'ports=%s\n' "${#list[@]}"
    printf 'matrix={"include":[%s]}\n' "$matrix"
    printf 'recreation=%s\n' "$([ "$ports" = main ] && echo true || echo false)"
    printf 'main=%s\n' "$([ "$repo" = deusex-launcher ] && echo "$CI_SHA" || echo main)"
    printf 'release=%s\n' "$release"
}

cmd_link() {
    local p="${1:?which port?}"
    [ -e "$DX_LAUNCHER/$p/.git" ] || die "no deusex-launcher/$p"
    dx_link_workspace "$DX_LAUNCHER/$p"
}

# The Windows port's tests run under Proton's wine (lib/common.sh): on CI the
# build this machine's Steam has, CachyOS's release, pinned.
PROTON_NAME=proton-cachyos-11.0-20261005-slr-x86_64
PROTON_URL=https://github.com/CachyOS/proton-cachyos/releases/download/cachyos-11.0-20261005-slr/$PROTON_NAME.tar.xz
PROTON_SHA256=096bfe73b506d6565b04ecc45214197a4091818f16ed91f5324b4d2082d0a263

cmd_proton() {
    local dir="$HOME/.local/share/Steam/compatibilitytools.d/$PROTON_NAME" tar
    if [ ! -x "$dir/files/bin/wine" ]; then
        tar="$(dx_fetch_cached "$PROTON_NAME.tar.xz" "$PROTON_URL" "$PROTON_SHA256")"
        dx_unpack_into "$tar" "$dir" files/bin/wine
    fi
    say "Proton ready: $dir"
    printf '%s\n' "$PROTON_NAME"
}

# Third-party code is not ours to fix: the engine's Thirdparty/ and what its
# DXGPU carries (glslang, VMA, volk, Vulkan's headers), and the launcher's
# dependencies, built in its *-prefix/.
cmd_warnings() {
    local log="${1:?which log?}" hits
    [ -f "$log" ] || die "no log $log"
    hits="$(grep -E ': warning:' "$log" |
        grep -vE '/Thirdparty/|/DXGPU/src/(glslang|vk_mem_alloc|volk)/|/DXGPU/include/(vulkan|vk_video)/|-prefix/(src|tmp)/' |
        sort -u || true)"
    if [ -n "$hits" ]; then
        printf '%s\n' "$hits" | sed 's/^/::error::/'
        die "$(printf '%s\n' "$hits" | wc -l) warning(s) in our own code"
    fi
    say "no warnings in our own code"
}

# ctest passes a test that skips (exit 77): only the ones named may.
cmd_skips() {
    local log="${1:?which log?}" t name bad=0; shift
    [ -f "$log" ] || die "no log $log"
    while read -r name; do
        for t in "$@"; do [ "$t" != "$name" ] || continue 2; done
        echo "::error::$name was skipped"
        bad=1
    done < <(sed -n 's/.*Test *#[0-9]*: \([^ ]*\) .*\*\*\*Skipped.*/\1/p' "$log" | sort -u)
    [ "$bad" = 0 ] || die "tests were skipped that must run"
    say "no test skipped but: ${*:-none}"
}

# The release's file for a port, from its staged app: the Windows port's
# zip, which its staging makes; the Smart Pro's app folder as spruceOS's
# App/ takes it; any other's a tarball of DXLauncher/.
cmd_package() {
    local p="${1:?which port?}" b="$DX_ROOT/build/${1:-}" out tmp
    [ -d "$b/app" ] || die "nothing staged -- scripts/dx.sh stage $p"
    case "$p" in
        windows-x86_64)
            out="$b/DXLauncher-$p.zip"
            [ -f "$out" ] || die "no $out -- scripts/dx.sh stage $p"
            ;;
        trimui-smartpro)
            out="$b/DXLauncher-$p.zip"
            tmp="$(mktemp -d "$b/.zip.XXXXXX")"
            cp -a "$b/app" "$tmp/DeusEx"
            rm -f "$out"
            (cd "$tmp" && cmake -E tar cf "$out" --format=zip DeusEx)
            rm -rf "$tmp"
            ;;
        *)
            out="$b/DXLauncher-$p.tar.gz"
            tar -C "$b" -czf "$out" --transform 's,^app,DXLauncher,' app
            ;;
    esac
    printf '%s\n' "$out"
}

# The notes: what the release is built from and what changed since the
# previous one, by its tag.
cmd_notes() {
    local ws="$DX_WORKSPACE" sha prev p old new eng
    sha="$(git -C "$ws" rev-parse HEAD)"
    prev="$(git -C "$ws" describe --tags --abbrev=0 --match '20[0-9][0-9].[0-9][0-9].[0-9][0-9]-*' HEAD 2>/dev/null || true)"
    eng="$(engine_pin < "$ws/ENGINE-PIN.txt")"
    printf 'Every port of [Port Ex Machina](%s/port-ex-machina) built at the commits its pins name, from [`%s`](%s/port-ex-machina/commit/%s).\n\n' \
        "$OWNER_URL" "${sha:0:8}" "$OWNER_URL" "$sha"
    printf '**You supply your own game files. None are included.** Each file is named for its port: what each port is and how far along, [the ports](%s/port-ex-machina#engine-ports).\n\n' "$OWNER_URL"
    printf '### Built from\n\n| | Commit | Since the previous release |\n|---|---|---|\n'
    old="$( [ -z "$prev" ] || git -C "$ws" show "$prev:ENGINE-PIN.txt" | engine_pin)"
    printf '| DXEngine | [`%s`](%s/DXEngine/commit/%s) | %s |\n' "${eng:0:7}" "$OWNER_URL" "$eng" \
        "$(compare DXEngine "$old" "$eng")"
    for p in $(dx_ports); do
        new="$(launcher_pin "$p" < "$ws/LAUNCHER-PIN.txt")"
        old="$( [ -z "$prev" ] || git -C "$ws" show "$prev:LAUNCHER-PIN.txt" | launcher_pin "$p")"
        printf '| %s | [`%s`](%s/deusex-launcher/commit/%s) | %s |\n' "$p" "${new:0:7}" "$OWNER_URL" "$new" \
            "$(compare deusex-launcher "$old" "$new")"
    done
    printf '\n### What changed\n\n'
    if [ -z "$prev" ]; then
        printf 'The first release.\n'
    else
        git -C "$ws" log --no-merges --reverse --format="- %s ([\`%h\`]($OWNER_URL/port-ex-machina/commit/%H))" "$prev..HEAD"
    fi
}

compare() {   # <repository> <old> <new>
    if [ -z "$2" ]; then echo "--"
    elif [ "$2" = "$3" ]; then echo "unchanged"
    else printf '[%s...%s](%s/%s/compare/%s...%s)\n' "${2:0:7}" "${3:0:7}" "$OWNER_URL" "$1" "$2" "$3"
    fi
}

cmd="${1:-}"; [ -n "$cmd" ] || usage; shift
case "$cmd" in
    plan)     cmd_plan ;;
    link)     cmd_link "$@" ;;
    proton)   cmd_proton ;;
    warnings) cmd_warnings "$@" ;;
    skips)    cmd_skips "$@" ;;
    package)  cmd_package "$@" ;;
    notes)    cmd_notes ;;
    -h|--help|help) usage ;;
    *)        die "unknown command '$cmd' (scripts/ci.sh help)" ;;
esac
