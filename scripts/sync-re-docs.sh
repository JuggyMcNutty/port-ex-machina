#!/usr/bin/env bash
# Refresh port/docs/re/ from the canonical reverse-engineering spec.
#
# The port repo carries its own copy so it is portable off this machine, but a
# copy is a drift risk. This makes the sync one command and --check makes the
# drift visible without changing anything.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="${SRC:-$ROOT/..}"          # the game install, where agent.md and docs/ live
DEST="$ROOT/docs/re"

if [ ! -f "$SRC/agent.md" ] || [ ! -d "$SRC/docs" ]; then
    echo "no canonical spec at $SRC (expected agent.md and docs/)" >&2
    exit 1
fi

check=0
[ "${1:-}" = "--check" ] && check=1

drift=0
compare() {
    local a="$1" b="$2"
    if ! cmp -s "$a" "$b"; then
        echo "DRIFT: ${b#$ROOT/}"
        drift=1
        [ "$check" = 0 ] && cp "$a" "$b"
    fi
}

mkdir -p "$DEST/types"
compare "$SRC/agent.md" "$DEST/agent.md"
for f in "$SRC"/docs/*.md; do
    compare "$f" "$DEST/$(basename "$f")"
done
for f in "$SRC"/docs/types/*; do
    compare "$f" "$DEST/types/$(basename "$f")"
done

if [ "$drift" = 0 ]; then
    echo "port/docs/re is in sync with $SRC"
elif [ "$check" = 1 ]; then
    echo "run without --check to update" >&2
    exit 1
else
    echo "updated"
fi
