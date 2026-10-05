#!/usr/bin/env bash
# Docs drift guard for every repository of the workspace: each path a doc
# names must exist, and each link to a heading must find one -- in the doc's
# own repository, or in another's checkout here when the link goes there.
#
#   scripts/check-docs.sh [-v]
#
# The checkouts are the ones beside this repository
# (docs/DEVELOPMENT.md#the-repositories). The docs checked: this
# repository's; dx-reverse-info's; each branch of deusex-launcher checked out
# in deusex-launcher/<branch>/; and VibeEngine's own (its README and vibe/,
# not upstream's docs).
#
# In each doc, the `inline code` spans and the link targets that look like
# relative paths (they contain a "/") must resolve against, in order: the
# doc's own directory; its checkout's root, src/, ports/*/packaging/ and
# ports/*/; then the workspace's -- the folder the repositories share (so
# VibeEngine/..., gamefiles/... and reference/... resolve), this repository,
# the engine clone, dx-reverse-info, each launcher checkout and its src/, and
# gamefiles/ (the game install).
#
# A link to https://github.com/JuggyMcNutty/<repository>/(blob|tree)/<branch>/<path>
# must name a path that exists in that repository's checkout here:
# port-ex-machina at main is this one, dx-reverse-info at main and VibeEngine
# at deusex are their clones, and deusex-launcher at <branch> is
# deusex-launcher/<branch>/. A link to a VibeEngine commit must name one the
# clone has.
#
# A link with an #anchor to a markdown file (or to a heading of the doc itself)
# must name one of that file's headings, as GitHub makes their ids: lower case,
# punctuation dropped, spaces as "-", a repeated heading numbered -1, -2, ...
#
# The other checkouts, gamefiles/ and reference/ are not in this repository.
# When one is missing (a fresh clone), a path that resolves nowhere, or a link
# into a checkout that is absent, is counted as unverifiable rather than
# failed. The ALLOW list below names what exists only at run time, inside an
# archive, or on the device.
#
# AGENTS.md must exist: CLAUDE.md imports it into every agent session.
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"
ROOT="$DX_WORKSPACE"
verbose=0; [ "${1:-}" = "-v" ] && verbose=1
OWNER_URL="https://github.com/JuggyMcNutty"

# Paths that are correct but have nothing to resolve against here.
ALLOW=(
    '^home/'                          # the app dir's pinned HOME, created by run-game.sh at run time
    '^System/(Running\.ini|SE-DeusEx\.ini|SE-User\.ini|DeusExLauncher\.log)$'  # created at run time
    '^build/'                         # build output
    '^deps/'                          # fetched toolchains and sysroots
    '^(Window|Launch|Core|Engine|DeusEx|DeusExText|Extension|ConSys|WinDrv)/Inc/' # SDK headers inside reference/ReleaseSDK1112f/Headers/DxHeaders.zip
    '^spruce/'                        # spruceOS on the device (/mnt/SDCARD/spruce/...)
    '^usr/'                           # device filesystem, named without its leading slash in tar lists
    '^Joy[A-Za-z0-9]*/'               # binding names written as alternatives: JoyX/JoyY
    '^LookSensitivityX/Y$'            # the same, for two Settings.json members
)

ENGINE="$DX_ENGINE"
optional_missing=0
for r in "$ENGINE" "$DX_ROOT/gamefiles" "$DX_ROOT/reference" "$DX_RE" "$DX_LAUNCHER/main"; do
    [ -e "$r" ] || optional_missing=1
done

# The checkouts, as "dir repository branch".
checkouts=("$ROOT port-ex-machina main")
[ -d "$DX_RE/.git" ] && checkouts+=("$DX_RE dx-reverse-info main")
[ -d "$ENGINE/.git" ] && checkouts+=("$ENGINE VibeEngine deusex")
for d in "$DX_LAUNCHER"/*/; do
    d="${d%/}"
    [ -e "$d/.git" ] && checkouts+=("$d deusex-launcher $(basename "$d")")
done

# The local checkout of <repository> at <branch>, if there is one.
checkout_of() {
    local c
    for c in "${checkouts[@]}"; do
        set -- $c
        if [ "$2" = "$REPO_Q" ] && [ "$3" = "$REF_Q" ]; then printf '%s\n' "$1"; return 0; fi
    done
    return 1
}

# The workspace's roots, after a doc's own: names relative to a port (as
# packaging/ or target.c) resolve in any launcher checkout's ports.
workspace=("$DX_ROOT" "$ROOT" "$ENGINE" "$DX_RE" "$DX_ROOT/gamefiles")
for d in "$DX_LAUNCHER"/*/; do
    d="${d%/}"
    workspace+=("$d" "$d/src")
    for r in "$d"/ports/*/packaging "$d"/ports/*; do [ -d "$r" ] && workspace+=("$r"); done
done

# file:line:kind:token for every candidate, skipping fenced code blocks; kind
# is C for `code`, L for a link target.
extract() {
    awk '
        /^[[:space:]]*```/ { fence = !fence; next }
        fence { next }
        {
            line = $0
            while (match(line, /`[^`]+`/)) {
                print FILENAME ":" FNR ":C:" substr(line, RSTART + 1, RLENGTH - 2)
                line = substr(line, RSTART + RLENGTH)
            }
            line = $0
            while (match(line, /\]\([^)]+\)/)) {
                print FILENAME ":" FNR ":L:" substr(line, RSTART + 2, RLENGTH - 3)
                line = substr(line, RSTART + RLENGTH)
            }
        }' "$@"
}

# The heading ids GitHub gives one markdown file, one per line.
anchors_of() {
    awk '
        /^[[:space:]]*```/ { fence = !fence; next }
        fence { next }
        /^#+[ \t]/ {
            h = $0
            sub(/^#+[ \t]+/, "", h); sub(/[ \t]+$/, "", h)
            h = tolower(h)
            gsub(/[^a-z0-9 _-]/, "", h)
            gsub(/ /, "-", h)
            n = seen[h]++
            print (n ? h "-" n : h)
        }' "$1"
}

# Does <file> (a markdown file, or a directory's README) have heading <anchor>?
declare -A HEADINGS=()
has_heading() {
    local file="$1" anchor="$2"
    [[ "$file" == /* ]] || file="$PWD/$file"    # the cache spans checkouts
    [ -d "$file" ] && file="$file/README.md"
    case "$file" in *.md) ;; *) return 0 ;; esac
    [ -f "$file" ] || return 0              # a missing file is the path check's to report
    [ -n "${HEADINGS[$file]+x}" ] || HEADINGS[$file]="$(anchors_of "$file")"
    grep -qxF -- "$anchor" <<<"${HEADINGS[$file]}"
}

allowed() {
    local t="$1" re
    for re in "${ALLOW[@]}"; do [[ "$t" =~ $re ]] && return 0; done
    return 1
}

checked=0 anchors=0 urls=0 failed=0 unverifiable=0 ndocs=0
fail() { echo "$1" >&2; failed=$((failed + 1)); }

# A link into one of our repositories: 0 checked (and fine), 1 failed,
# 2 unverifiable (no checkout of it here), 3 not ours.
check_url() {
    local doc="$1" line="$2" url="$3" path="${3%%#*}" anchor=""
    [[ "$url" == *'#'* ]] && anchor="${url#*#}"
    [[ "$path" == "$OWNER_URL"/* ]] || return 3
    local rest="${path#"$OWNER_URL"/}" repo kind ref sub dir
    repo="${rest%%/*}"
    if [[ "$rest" =~ ^[^/]+/commit/([0-9a-f]+)$ ]]; then
        [ "$repo" = VibeEngine ] && [ -d "$ENGINE/.git" ] || return 2
        git -C "$ENGINE" cat-file -e "${BASH_REMATCH[1]}^{commit}" 2>/dev/null && return 0
        fail "NO COMMIT $doc:$line: $url"; return 1
    fi
    if [[ "$rest" =~ ^[^/]+/?$ ]]; then
        return 0                            # the repository itself
    fi
    [[ "$rest" =~ ^[^/]+/(blob|tree)/([^/]+)/(.+)$ ]] || { fail "BAD LINK $doc:$line: $url"; return 1; }
    ref="${BASH_REMATCH[2]}"; sub="${BASH_REMATCH[3]}"
    REPO_Q="$repo" REF_Q="$ref"
    dir="$(checkout_of)" || return 2
    if [ ! -e "$dir/$sub" ]; then
        fail "MISSING $doc:$line: $url"; return 1
    fi
    if [ -n "$anchor" ]; then
        anchors=$((anchors + 1))
        has_heading "$dir/$sub" "$anchor" || { fail "NO HEADING $doc:$line: $url"; return 1; }
    fi
    return 0
}

for c in "${checkouts[@]}"; do
    set -- $c
    co="$1" repo="$2"
    where="${co#"$DX_ROOT"/}"
    cd "$co"
    # A checkout git cannot read (a worktree whose link went stale when the
    # folders moved) fails here: skipped, its docs would pass unchecked.
    pathspec=('*.md'); [ "$repo" = VibeEngine ] && pathspec=(README.md 'vibe/*.md')
    if ! list="$(git ls-files --cached --others --exclude-standard "${pathspec[@]}" 2>&1)"; then
        fail "UNREADABLE $where: ${list%%$'\n'*} -- git -C deusex-launcher/main worktree repair, for a moved worktree"
        continue
    fi
    mapfile -t docs <<< "$list"
    [ -n "$list" ] || docs=()
    [ ${#docs[@]} -gt 0 ] || continue
    ndocs=$((ndocs + ${#docs[@]}))
    roots=("$co")
    [ -d "$co/src" ] && roots+=("$co/src")
    for r in "$co"/ports/*/packaging "$co"/ports/*; do [ -d "$r" ] && roots+=("$r"); done
    roots+=("${workspace[@]}")

    while IFS= read -r rec; do
        doc="${rec%%:*}"; rest="${rec#*:}"; line="${rest%%:*}"; rest="${rest#*:}"
        kind="${rest%%:*}"; tok="${rest#*:}"
        tok="${tok%% *}"                   # `path args` -> path; [x](path "title") -> path
        label="$where/$doc"
        if [ "$kind" = L ] && [[ "$tok" == https://* ]]; then
            rc=0; check_url "$label" "$line" "$tok" || rc=$?
            case "$rc" in
                0) urls=$((urls + 1)); [ "$verbose" = 1 ] && echo "ok   $label:$line $tok" ;;
                2) unverifiable=$((unverifiable + 1)); [ "$verbose" = 1 ] && echo "skip $label:$line $tok" ;;
            esac
            continue
        fi
        if [ "$kind" = L ] && [[ "$tok" == *'#'* ]] && ! [[ "$tok" =~ ^[a-z]+: ]]; then
            anchors=$((anchors + 1))
            p="${tok%%#*}"
            if [ -z "$p" ]; then f="$doc"; else f="$(dirname "$doc")/$p"; fi
            if has_heading "$f" "${tok#*#}"; then
                [ "$verbose" = 1 ] && echo "ok   $label:$line $tok"
            else
                fail "NO HEADING $label:$line: $tok"
            fi
        fi
        tok="${tok%%#*}"                   # link anchors
        # Relative paths only: must contain "/" and a letter, and nothing that
        # makes it a URL, a command, a placeholder, a glob or an address.
        [[ "$tok" == */* ]] || continue
        [[ "$tok" =~ [A-Za-z] ]] || continue
        [[ "$tok" =~ ^(/|~|-|\.\./\.\./\.\.) ]] && continue
        [[ "$tok" =~ [][\<\>\*\$\{\}=:@\\\|\(\)\'\",\;] ]] && continue
        [[ "$tok" =~ ^[A-Za-z0-9_.+-]+(/[A-Za-z0-9_.+-]*)+$ ]] || continue
        allowed "$tok" && continue
        checked=$((checked + 1))
        ok=0
        docdir="$(dirname "$doc")"
        for base in "$co/$docdir" "${roots[@]}"; do
            if [ -e "$base/$tok" ]; then ok=1; break; fi
        done
        if [ "$ok" = 1 ]; then
            [ "$verbose" = 1 ] && echo "ok   $label:$line $tok"
        elif [ "$optional_missing" = 1 ]; then
            unverifiable=$((unverifiable + 1))
            [ "$verbose" = 1 ] && echo "skip $label:$line $tok (a checkout, gamefiles/ or reference/ absent)"
        else
            fail "MISSING $label:$line: $tok"
        fi
    done < <(extract "${docs[@]}")
done

[ -f "$ROOT/AGENTS.md" ] || fail "MISSING AGENTS.md: CLAUDE.md imports it"

summary="docs: $checked paths, $anchors anchors and $urls links between repositories checked in $ndocs files, $failed missing"
[ "$unverifiable" = 0 ] || summary="$summary, $unverifiable unverifiable (a checkout, gamefiles/ or reference/ absent)"
echo "$summary"
[ "$failed" = 0 ]
