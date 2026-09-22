#!/usr/bin/env bash
# Push a build to the device and optionally run it.
#
#   scripts/deploy.sh                 # sync the app dir
#   scripts/deploy.sh --run           # sync, then run and stream stdout
#   scripts/deploy.sh --run -- -safe  # ...passing arguments to the launcher
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${BUILD:-$ROOT/build-trimui}"
DEVICE="${DEVICE:-192.168.1.211}"
DEVICE_USER="${DEVICE_USER:-spruce}"
DEVICE_PASS="${DEVICE_PASS:-happygaming}"
APPDIR="${APPDIR:-/mnt/SDCARD/App/DeusEx}"
SSHOPTS=(-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=15)

run=0; args=()
while [ $# -gt 0 ]; do
    case "$1" in
        --run) run=1; shift ;;
        --)    shift; args=("$@"); break ;;
        *)     echo "unknown option: $1" >&2; exit 2 ;;
    esac
done

[ -x "$BUILD/deusex-launcher" ] || { echo "no build at $BUILD -- cmake --build it first" >&2; exit 1; }
"$ROOT/scripts/check-abi.sh" "$BUILD/deusex-launcher" "$BUILD/dxl-cli"

stage="$(mktemp -d)"; trap 'rm -rf "$stage"' EXIT
cp -a "$ROOT/packaging/trimui-smartpro/." "$stage/"

# launcher.ini belongs to whoever owns the device -- it holds the path to their
# game files. Ship it as a default and only install it if none is there.
mv "$stage/launcher.ini" "$stage/launcher.ini.default"
cp "$BUILD/deusex-launcher" "$stage/deusex-launcher"
cp "$BUILD/dxl-cli"          "$stage/dxl-cli"
[ -d "$ROOT/assets" ] && cp -a "$ROOT/assets" "$stage/assets"

echo "deploying to $DEVICE_USER@$DEVICE:$APPDIR ..."
sshpass -p "$DEVICE_PASS" ssh "${SSHOPTS[@]}" "$DEVICE_USER@$DEVICE" "mkdir -p '$APPDIR'"
# exFAT has no permission bits worth trusting -- tar in, then chmod on arrival.
tar -C "$stage" -cf - . | sshpass -p "$DEVICE_PASS" ssh "${SSHOPTS[@]}" \
    "$DEVICE_USER@$DEVICE" "tar -C '$APPDIR' -xf - && chmod +x '$APPDIR'/deusex-launcher '$APPDIR'/dxl-cli '$APPDIR'/*.sh"

sshpass -p "$DEVICE_PASS" ssh "${SSHOPTS[@]}" "$DEVICE_USER@$DEVICE" \
    "[ -s '$APPDIR/launcher.ini' ] || cp '$APPDIR/launcher.ini.default' '$APPDIR/launcher.ini'; true"

if [ "$run" = 1 ]; then
    echo "--- running ---"
    sshpass -p "$DEVICE_PASS" ssh "${SSHOPTS[@]}" "$DEVICE_USER@$DEVICE" \
        "cd '$APPDIR' && LD_LIBRARY_PATH=/usr/trimui/lib:/usr/lib:/lib ./deusex-launcher ${args[*]:-} 2>&1; echo \"exit=\$?\""
fi
