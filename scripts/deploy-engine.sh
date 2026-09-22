#!/usr/bin/env bash
# Push the cross-built Surreal Engine to the device, next to the launcher.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ENGINE="${ENGINE:-$ROOT/../engine/SurrealEngine/build-trimui}"
DEVICE="${DEVICE:-192.168.1.211}"
DEVICE_USER="${DEVICE_USER:-spruce}"
DEVICE_PASS="${DEVICE_PASS:-happygaming}"
APPDIR="${APPDIR:-/mnt/SDCARD/App/DeusEx}"
SSHOPTS=(-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=15)

[ -x "$ENGINE/SurrealEngine" ] || { echo "no engine build at $ENGINE" >&2; exit 1; }
"$ROOT/scripts/check-abi.sh" "$ENGINE/SurrealEngine"

stage="$(mktemp -d)"; trap 'rm -rf "$stage"' EXIT
cp "$ENGINE/SurrealEngine" "$stage/"
# Shipped alongside the binary by the engine's own build.
for f in libSurrealVideo.so; do
    [ -f "$ENGINE/$f" ] && cp "$ENGINE/$f" "$stage/"
done
# The resource zip the engine loads at runtime, and any bundle directories.
for f in SurrealEngine.pk3; do
    [ -f "$ENGINE/$f" ] && cp "$ENGINE/$f" "$stage/"
done
for d in Resources Assets; do
    [ -d "$ENGINE/$d" ] && cp -a "$ENGINE/$d" "$stage/"
done

echo "deploying engine to $DEVICE_USER@$DEVICE:$APPDIR ..."
tar -C "$stage" -cf - . | sshpass -p "$DEVICE_PASS" ssh "${SSHOPTS[@]}" \
    "$DEVICE_USER@$DEVICE" "tar -C '$APPDIR' -xf - && chmod +x '$APPDIR/SurrealEngine'"
echo "done"
