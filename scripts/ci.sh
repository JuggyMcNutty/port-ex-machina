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
#   scripts/ci.sh failed <log>               the tests that failed, as GitHub annotations
#   scripts/ci.sh package <port>             a staged port as the releases' files: DX-, DXEngine-, DXLauncher-<port>
#   scripts/ci.sh publish <dir>              the releases, of the files in <dir> (DX_CI_DRY=1: say what, publish nothing)
#
# plan reads the caller's event from CI_PORTS (pins: the workspace's pinned
# set; all: every pinned port; a port; main: the recreation), CI_REPO,
# CI_EVENT, CI_REF, CI_SHA and CI_BEFORE (a push's previous commit, a pull
# request's base). The repositories are public: plan and publish read them
# over HTTPS, as RELEASES_TOKEN's owner where it is set, or on this machine
# as DX_OWNER_GIT names them (git@github.com:JuggyMcNutty, over SSH).
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

OWNER_URL="https://github.com/JuggyMcNutty"
OWNER_GIT="${DX_OWNER_GIT:-$OWNER_URL}"

usage() { sed -n '2,20p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2; }

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

# Git to one of the owner's repositories: as RELEASES_TOKEN's owner, when it
# is set.
owner_git() {
    if [ -n "${RELEASES_TOKEN:-}" ]; then
        git -c "http.https://github.com/.extraheader=AUTHORIZATION: basic $(printf 'x-access-token:%s' "$RELEASES_TOKEN" | base64 | tr -d '\n')" "$@"
    else
        git "$@"
    fi
}

engine_pin() { awk '$1 == "commit" { print $2 }'; }
launcher_pin() { awk -v p="$1" '$1 == "port" && $2 == p { print $3 }'; }

# code_between <repository> <old> <new>: whether code changed between two of
# its commits. A blobless bare clone has the trees git diff needs.
code_between() {
    local repo="$1" old="$2" new="$3" dir="$CI_TMP/$1.git" f
    [ -d "$dir" ] || owner_git clone -q --bare --filter=blob:none "$OWNER_GIT/$repo.git" "$dir"
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
# port's between the commits its pins named before and name now. Before is a
# push's last release, not its previous commit: GitHub keeps one pending run a
# group, cancelling the older, and a failed run releases nothing, so a later
# push of docs alone would leave their code unbuilt. A pull request's base.
pins_changed_code() {
    local before="${CI_BEFORE:-}" sha="$CI_SHA" f p old new
    if [ "$CI_EVENT" = workflow_dispatch ]; then
        say "run by hand -- building"
        return 0
    fi
    if [ "$CI_EVENT" = push ]; then
        before="$(last_release port-ex-machina '20')"
        [ -n "$before" ] || { say "no release yet -- building"; return 0; }
        say "against the last release, ${before:0:7}"
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

# Each failed test named on the run's page without its log opened: ctest's
# line for it, an error annotation.
cmd_failed() {
    local log="${1:?which log?}"
    [ -f "$log" ] || die "no log $log"
    grep -E 'Test +#[0-9]+: .*\*\*\*(Failed|Timeout|Exception)' "$log" | sed 's/^ */::error::/' || true
}

# A port's files for the releases, from its staged app, beside it in
# build/<port>/: DX-<port>, the app as it ships, launcher and engine;
# DXEngine-<port>, the engine alone, its files as staging copies them
# (PORT_ENGINE_FILES), its notices and on Windows its runtime's and OpenAL
# Soft's; DXLauncher-<port>, the app but the engine. A zip for the Windows
# port and the Smart Pro (whose app folder is DeusEx, as spruceOS's App/
# takes it), else a tarball; DXLauncher's folder, DXEngine's, or DeusEx.
cmd_package() {
    local p="${1:?which port?}" b="$DX_ROOT/build/${1:-}" tmp f files ext dir
    [ -d "$b/app" ] || die "nothing staged -- scripts/dx.sh stage $p"
    # shellcheck source=/dev/null
    files="$(PORT_ENGINE_FILES="DXEngine DXEngine.pk3"; . "$DX_LAUNCHER/$p/ports/$p/port.sh"; echo "$PORT_ENGINE_FILES")"
    for f in $files DXEngine-LICENSE.md; do
        [ -f "$b/app/$f" ] || die "the staged app lacks $f -- scripts/dx.sh build $p, then stage"
    done
    case "$p" in windows-x86_64|trimui-smartpro) ext=zip ;; *) ext=tar.gz ;; esac
    case "$p" in trimui-smartpro) dir=DeusEx ;; *) dir=DXLauncher ;; esac
    tmp="$(mktemp -d "$b/.package.XXXXXX")"
    mkdir -p "$tmp/app" "$tmp/engine/DXEngine" "$tmp/launcher"
    cp -a "$b/app" "$tmp/app/$dir"
    cp -a "$b/app" "$tmp/launcher/$dir"
    for f in $files DXEngine-LICENSE.md; do
        cp -a "$b/app/$f" "$tmp/engine/DXEngine/"
        rm -f "$tmp/launcher/$dir/$f"
    done
    if [ -d "$b/app/licenses" ]; then
        cp -a "$b/app/licenses" "$tmp/engine/DXEngine/"
        rm -f "$tmp/launcher/$dir/licenses/OpenAL-Soft-COPYING.txt"
    fi
    archive "$tmp/app" "$dir" "$b/DX-$p.$ext"
    archive "$tmp/engine" DXEngine "$b/DXEngine-$p.$ext"
    archive "$tmp/launcher" "$dir" "$b/DXLauncher-$p.$ext"
    rm -rf "$tmp"
    printf '%s\n' "$b/DX-$p.$ext" "$b/DXEngine-$p.$ext" "$b/DXLauncher-$p.$ext"
}

archive() {   # <parent folder> <its folder to pack> <file>
    rm -f "$3"
    case "$3" in
        *.zip) (cd "$1" && cmake -E tar cf "$3" --format=zip "$2") ;;
        *)     tar -C "$1" -czf "$3" "$2" ;;
    esac
}

# The releases (docs/DEVELOPMENT.md#ci), of a run's files in <dir>: this
# repository's, every port's app (DX-*), tagged <date>-<sha8>; DXEngine's,
# the engine alone (DXEngine-*), tagged <date>-<sha8> on its commit, when its
# code changed since its last release or its build did since this
# repository's last (engine_build_changed), a rebuild of one commit the same
# day <date>-<sha8>.2 and on (free_tag); and deusex-launcher's, one per port,
# its launcher alone (DXLauncher-<port>.*), tagged <port>-<date>-<sha8> on
# its branch's commit, when that branch's code changed since its last. The
# two others need RELEASES_TOKEN, a token that may write to them; GH_TOKEN is
# this repository's own. Each is tried, a failure failing the whole at the end.
cmd_publish() {
    local dist="${1:?which folder?}" ws="$DX_WORKSPACE" sha date eng p pin last prevws tag rc=0
    [ -d "$dist" ] || die "no folder $dist"
    CI_TMP="$(mktemp -d)"; trap 'rm -rf "$CI_TMP"' EXIT
    sha="$(git -C "$ws" rev-parse HEAD)"
    date="$(date -u +%Y.%m.%d)"
    eng="$(engine_pin < "$ws/ENGINE-PIN.txt")"
    prevws="$(last_release port-ex-machina '20')"   # before this run's own
    notes_workspace "$sha" > "$CI_TMP/notes-ws.md"
    release "${GITHUB_REPOSITORY:-JuggyMcNutty/port-ex-machina}" "$date-${sha:0:8}" "$sha" "$CI_TMP/notes-ws.md" \
        "$dist"/DX-* || rc=1
    if [ -z "${RELEASES_TOKEN:-}" ] && [ -z "${DX_CI_DRY:-}" ]; then
        echo "::warning::no RELEASES_TOKEN: neither DXEngine's release nor deusex-launcher's published"
        return "$rc"
    fi
    last="$(last_release DXEngine '20')"
    if [ -z "$last" ] || code_between DXEngine "$last" "$eng" || engine_build_changed "$prevws" "$sha"; then
        notes_engine "$last" "$eng" "$sha" > "$CI_TMP/notes-engine.md"
        tag="$(free_tag DXEngine "$date-${eng:0:8}")"
        release JuggyMcNutty/DXEngine "$tag" "$eng" "$CI_TMP/notes-engine.md" "$dist"/DXEngine-* || rc=1
    fi
    for p in $(dx_ports); do
        pin="$(launcher_pin "$p" < "$ws/LAUNCHER-PIN.txt")"
        last="$(last_release deusex-launcher "$p-20")"
        if [ -z "$last" ] || code_between deusex-launcher "$last" "$pin"; then
            notes_launcher "$p" "$last" "$pin" "$sha" > "$CI_TMP/notes-$p.md"
            release JuggyMcNutty/deusex-launcher "$p-$date-${pin:0:8}" "$pin" "$CI_TMP/notes-$p.md" \
                "$dist"/DXLauncher-"$p".* || rc=1
        fi
    done
    return "$rc"
}

# engine_build_changed <workspace commit> <workspace commit>: whether what the
# engine's binaries are built with, besides its code, changed between two of
# this repository's commits: its build scripts or CI, or a pinned port
# branch's engine.cmake, toolchain, port.sh or cmake/deps.lock (SDL3's
# version among them). The first empty or unknown: changed.
engine_build_changed() {
    local old="$1" new="$2" f p a b dir="$CI_TMP/deusex-launcher.git"
    [ -n "$old" ] && git -C "$DX_WORKSPACE" cat-file -e "$old^{commit}" 2>/dev/null || return 0
    while read -r f; do
        case "$f" in
            scripts/engine.sh|scripts/dx.sh|scripts/lib/*|.github/workflows/ports.yml)
                say "the engine's build: $f changed"; return 0 ;;
        esac
    done < <(git -C "$DX_WORKSPACE" diff --no-renames --name-only "$old" "$new")
    for p in $(dx_ports); do
        a="$(git -C "$DX_WORKSPACE" show "$old:LAUNCHER-PIN.txt" | launcher_pin "$p")"
        b="$(git -C "$DX_WORKSPACE" show "$new:LAUNCHER-PIN.txt" | launcher_pin "$p")"
        [ "$a" != "$b" ] || continue
        [ -n "$a" ] || { say "the engine's build: $p newly pinned"; return 0; }
        [ -d "$dir" ] || owner_git clone -q --bare --filter=blob:none "$OWNER_GIT/deusex-launcher.git" "$dir"
        while read -r f; do
            case "$f" in
                "ports/$p/engine.cmake"|"ports/$p/"toolchain*.cmake|"ports/$p/port.sh"|cmake/deps.lock)
                    say "the engine's build: $p's $f changed"; return 0 ;;
            esac
        done < <(git -C "$dir" diff --no-renames --name-only "$a" "$b")
    done
    return 1
}

# free_tag <repository> <tag>: the tag, or the first of <tag>.2, .3, ... the
# repository has not, for a rebuild of one commit the same day.
free_tag() {
    local tags t="$2" n=2
    tags="$(owner_git ls-remote --tags --refs "$OWNER_GIT/$1.git" | sed 's|.*refs/tags/||')"
    while grep -qxF "$t" <<< "$tags"; do t="$2.$n"; n=$((n + 1)); done
    printf '%s\n' "$t"
}

# last_release <repository> <tag's start>: the commit of its newest release
# so tagged, or nothing. The tags start with their date, and of the latest
# day's the newest is the one whose commit is the latest (a day's sha8s sort
# anyhow: a day of three releases took its second, made the third's tag
# again and failed), read in the bare clone code_between keeps.
last_release() {
    local dir="$CI_TMP/$1.git" tag sha day latest="" best="" bestwhen=0 when
    [ -d "$dir" ] || owner_git clone -q --bare --filter=blob:none "$OWNER_GIT/$1.git" "$dir"
    while read -r tag sha; do
        day="${tag%-*}"
        if [[ "$day" > "$latest" ]]; then
            latest="$day"
            best=""
        fi
        [ "$day" = "$latest" ] || continue
        when="$(git -C "$dir" log -1 --format=%ct "$sha" 2>/dev/null || echo 0)"
        if [ -z "$best" ] || [ "$when" -gt "$bestwhen" ]; then
            best="$sha"
            bestwhen="$when"
        fi
    done < <(owner_git ls-remote --tags --refs "$OWNER_GIT/$1.git" |
             awk -v t="refs/tags/$2" 'index($2, t) == 1 { print $2, $1 }' | sort)
    printf '%s\n' "$best"
}

# release <owner/repository> <tag> <commit> <notes file> <file>...
release() {
    local repo="$1" tag="$2" target="$3" notes="$4" token; shift 4
    [ -e "${1:-}" ] || { echo "::error::$repo: no files to publish"; return 1; }
    if [ -n "${DX_CI_DRY:-}" ]; then
        say "would publish $repo $tag at ${target:0:8}: ${*##*/}"
        sed 's/^/    | /' "$notes" >&2
        return 0
    fi
    token="${RELEASES_TOKEN:-}"
    [ "${repo##*/}" != port-ex-machina ] || token="${GH_TOKEN:-}"
    if ! GH_TOKEN="$token" gh release create "$tag" "$@" --repo "$repo" --target "$target" \
            --title "$tag" --notes-file "$notes" 2> "$CI_TMP/gh.err"; then
        echo "::error::$repo $tag: $(tr '\n' ' ' < "$CI_TMP/gh.err")"
        return 1
    fi
    echo "::notice::$repo $tag: ${*##*/}"
}

# The notes. Each names what it is built from, links what changed since the
# release before it, and says where the others are.
notes_workspace() {   # <commit>
    local ws="$DX_WORKSPACE" sha="$1" prev p old new eng
    prev="$(git -C "$ws" describe --tags --abbrev=0 --match '20[0-9][0-9].[0-9][0-9].[0-9][0-9]-*' HEAD 2>/dev/null || true)"
    eng="$(engine_pin < "$ws/ENGINE-PIN.txt")"
    printf 'Every port of [Port Ex Machina](%s/port-ex-machina), launcher and engine together, built at the commits its pins name, from [`%s`](%s/port-ex-machina/commit/%s). The engine alone is in [DXEngine'\''s releases](%s/DXEngine/releases), each port'\''s launcher alone in [deusex-launcher'\''s](%s/deusex-launcher/releases).\n\n' \
        "$OWNER_URL" "${sha:0:8}" "$OWNER_URL" "$sha" "$OWNER_URL" "$OWNER_URL"
    game_files
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

notes_engine() {   # <last released commit or nothing> <commit> <workspace commit>
    printf 'DXEngine alone, at [`%s`](%s/DXEngine/commit/%s), for every port, each built as its launcher branch configures it at the commits [Port Ex Machina](%s/port-ex-machina) pins ([`%s`](%s/port-ex-machina/commit/%s)). With the launcher, each port'\''s app is in [Port Ex Machina'\''s releases](%s/port-ex-machina/releases).\n\n' \
        "${2:0:8}" "$OWNER_URL" "$2" "$OWNER_URL" "${3:0:8}" "$OWNER_URL" "$3" "$OWNER_URL"
    game_files
    changes DXEngine "$1" "$2"
}

notes_launcher() {   # <port> <last released commit or nothing> <commit> <workspace commit>
    printf 'DXLauncher for %s alone, at [`%s`](%s/deusex-launcher/commit/%s), the head of branch [`%s`](%s/deusex-launcher/tree/%s) that [Port Ex Machina](%s/port-ex-machina) pins ([`%s`](%s/port-ex-machina/commit/%s)). The engine is in [DXEngine'\''s releases](%s/DXEngine/releases); both together, in [Port Ex Machina'\''s](%s/port-ex-machina/releases).\n\n' \
        "$1" "${3:0:8}" "$OWNER_URL" "$3" "$1" "$OWNER_URL" "$1" "$OWNER_URL" "${4:0:8}" "$OWNER_URL" "$4" "$OWNER_URL" "$OWNER_URL"
    game_files
    changes deusex-launcher "$2" "$3"
}

game_files() {
    printf '**You supply your own game files. None are included.** What each port is and how far along: [the ports](%s/port-ex-machina#engine-ports).\n\n' "$OWNER_URL"
}

# What changed in a repository between two commits: the comparison, and its
# commits' subjects, cut short (each says it all at the link), fifty at most.
changes() {   # <repository> <old or nothing> <new>
    local repo="$1" old="$2" new="$3" dir="$CI_TMP/$1.git" n
    printf '### What changed\n\n'
    if [ -z "$old" ]; then
        printf 'The first release.\n'
        return 0
    fi
    [ -d "$dir" ] || owner_git clone -q --bare --filter=blob:none "$OWNER_GIT/$repo.git" "$dir"
    printf '%s\n\n' "$(compare "$repo" "$old" "$new")"
    n="$(git -C "$dir" rev-list --no-merges --count "$old..$new")"
    git -C "$dir" log --no-merges --reverse -n 50 --format='%h%x09%H%x09%s' "$old..$new" |
        awk -F '\t' -v url="$OWNER_URL/$repo/commit/" '{
            s = $3; if (length(s) > 200) s = substr(s, 1, 200) "..."
            printf "- %s ([`%s`](%s%s))\n", s, $1, url, $2 }'
    [ "$n" -le 50 ] || printf '\nAnd %s more: the comparison has them all.\n' "$((n - 50))"
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
    failed)   cmd_failed "$@" ;;
    package)  cmd_package "$@" ;;
    publish)  cmd_publish "$@" ;;
    -h|--help|help) usage ;;
    *)        die "unknown command '$cmd' (scripts/ci.sh help)" ;;
esac
