#!/usr/bin/env bash
# Docs drift guard: every repository path a doc names must exist.
#
#   scripts/check-docs.sh [-v]
#
# Reads every *.md in the repository, takes the `inline code` spans and the link targets
# that look like relative paths (they contain a "/"), and resolves each one
# against, in order:
#
#   the doc's own directory   links like re/wizard.md
#   the repository root       src/core/config.c, ports/trimui-smartpro/port.sh
#   src/                      core/config.c, ui/screens.c (how DESIGN.md names code)
#   engine/SurrealEngine/     SurrealEngine/GameApp.cpp (the fork, a separate clone)
#   gamefiles/                System/DeusEx.ini (the game install)
#   ports/*/packaging/        run-game.sh's neighbours: the app directory
#   ports/*/                  names relative to a port (packaging/, target.c)
#
# engine/, gamefiles/ and reference/ are not in the repository. When one is
# missing (a fresh clone), a path that resolves nowhere is counted as unverifiable
# rather than failed. The ALLOW list below names what exists only at run
# time, inside an archive, or on the device.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
verbose=0; [ "${1:-}" = "-v" ] && verbose=1

# Paths that are correct but have nothing to resolve against here.
ALLOW=(
    '^home/'                          # the app dir's pinned HOME, created by run-game.sh at run time
    '^System/(Running\.ini|SE-DeusEx\.ini|SE-User\.ini|DeusExLauncher\.log)$'  # created at run time
    '^build/'                         # build output
    '^deps/'                          # fetched toolchains and sysroots
    '^(Window|Launch|Core|Engine)/Inc/' # SDK headers inside reference/ReleaseSDK1112f/Headers/DxHeaders.zip
    '^spruce/'                        # spruceOS on the device (/mnt/SDCARD/spruce/...)
    '^usr/'                           # device filesystem, named without its leading slash in tar lists
    '^Joy[A-Za-z0-9]*/'               # binding names written as alternatives: JoyX/JoyY
    '^LookSensitivityX/Y$'            # the same, for two Settings.json members
)

roots=("$ROOT" "$ROOT/src")
optional_missing=0
for r in "$ROOT/engine/SurrealEngine" "$ROOT/gamefiles"; do
    if [ -d "$r" ]; then roots+=("$r"); else optional_missing=1; fi
done
[ -d "$ROOT/reference" ] || optional_missing=1
for r in "$ROOT"/ports/*/packaging; do roots+=("$r"); done
for r in "$ROOT"/ports/*; do roots+=("$r"); done

# file:line:token for every candidate, skipping fenced code blocks.
extract() {
    awk '
        /^[[:space:]]*```/ { fence = !fence; next }
        fence { next }
        {
            line = $0
            while (match(line, /`[^`]+`/)) {
                print FILENAME ":" FNR ":" substr(line, RSTART + 1, RLENGTH - 2)
                line = substr(line, RSTART + RLENGTH)
            }
            line = $0
            while (match(line, /\]\([^)]+\)/)) {
                print FILENAME ":" FNR ":" substr(line, RSTART + 2, RLENGTH - 3)
                line = substr(line, RSTART + RLENGTH)
            }
        }' "$@"
}

allowed() {
    local t="$1" re
    for re in "${ALLOW[@]}"; do [[ "$t" =~ $re ]] && return 0; done
    return 1
}

cd "$ROOT"
# Tracked docs, and new ones not yet added (anything not ignored).
mapfile -t docs < <(git ls-files --cached --others --exclude-standard '*.md')
[ ${#docs[@]} -gt 0 ] || { echo "no tracked docs?" >&2; exit 1; }

checked=0 failed=0 unverifiable=0
while IFS= read -r rec; do
    doc="${rec%%:*}"; rest="${rec#*:}"; line="${rest%%:*}"; tok="${rest#*:}"
    tok="${tok%%#*}"                       # link anchors
    tok="${tok%% *}"                       # `path args` -> path
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
    for base in "$ROOT/$docdir" "${roots[@]}"; do
        if [ -e "$base/$tok" ]; then ok=1; break; fi
    done
    if [ "$ok" = 1 ]; then
        [ "$verbose" = 1 ] && echo "ok   $doc:$line $tok"
    elif [ "$optional_missing" = 1 ]; then
        unverifiable=$((unverifiable + 1))
        [ "$verbose" = 1 ] && echo "skip $doc:$line $tok (engine/ or gamefiles/ absent)"
    else
        echo "MISSING $doc:$line: $tok" >&2
        failed=$((failed + 1))
    fi
done < <(extract "${docs[@]}")

summary="docs: $checked paths checked in ${#docs[@]} files, $failed missing"
[ "$unverifiable" = 0 ] || summary="$summary, $unverifiable unverifiable (engine/, gamefiles/ or reference/ absent)"
echo "$summary"
[ "$failed" = 0 ]
