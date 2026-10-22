#!/usr/bin/env bash
# Packages onto a GitHub runner, without hanging the job: CI's only apt
# (docs/DEVELOPMENT.md#ci).
#
#   scripts/ci-apt-install.sh <package>...
#
# The runner's mirror list (/etc/apt/apt-mirrors.txt) puts Azure's Ubuntu
# mirror first, and apt moves to the next one only on a hard error, never on
# a slow one; its own timeout is for inactivity, which a mirror sending a
# few bytes at a time never trips. So each attempt is bounded by the clock,
# and a retry drops the Azure mirror for the canonical archive. The first
# attempt keeps it: from a runner it is by far the fastest.
set -euo pipefail

[ "$#" -gt 0 ] || { echo "usage: $0 <package>..." >&2; exit 2; }

MIRRORS=/etc/apt/apt-mirrors.txt
ATTEMPTS=3
UPDATE_TIMEOUT=240    # seconds; a healthy mirror takes a few
INSTALL_TIMEOUT=600

for attempt in $(seq 1 "$ATTEMPTS"); do
    if [ "$attempt" -gt 1 ] && [ -f "$MIRRORS" ] && grep -q azure.archive.ubuntu.com "$MIRRORS"; then
        echo "dropping azure.archive.ubuntu.com from the mirror list"
        sudo sed -i '/azure\.archive\.ubuntu\.com/d' "$MIRRORS"
    fi
    if timeout "$UPDATE_TIMEOUT" sudo apt-get update -o Acquire::Retries=3 &&
       timeout "$INSTALL_TIMEOUT" sudo DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
           -o Acquire::Retries=3 -o Acquire::http::Timeout=30 "$@"; then
        exit 0
    fi
    echo "::warning::apt attempt $attempt of $ATTEMPTS stalled or failed"
    sleep 10
done

echo "::error::could not install after $ATTEMPTS attempts: $*"
exit 1
