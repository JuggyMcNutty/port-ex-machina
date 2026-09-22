# TrimUI Smart Pro (spruceOS) -- sourced by scripts/dx.sh. See README.md.
PORT_DESC="TrimUI Smart Pro, spruceOS (aarch64, cross-built)"
PORT_ENGINE=1

# The device. The password is the stock firmware one; SSH is root.
DEVICE="${DEVICE:-192.168.1.211}"
DEVICE_USER="${DEVICE_USER:-spruce}"
DEVICE_PASS="${DEVICE_PASS:-happygaming}"
DEVICE_APPDIR="${DEVICE_APPDIR:-/mnt/SDCARD/App/DeusEx}"

# Two toolchains, both at or below the device's glibc 2.33: the launcher is
# C11 on GCC 9.3, the engine needs C++20 and GCC 10.3.
port_deps() {
    dx_fetch_bootlin aarch64--glibc--stable-2020.08-1        2.33
    dx_fetch_bootlin aarch64--glibc--bleeding-edge-2021.05-1 2.33
    "$PORT_DIR/fetch-sysroot.sh" "$@"
}

#   scripts/dx.sh deploy trimui-smartpro [--no-engine] [--run [-- launcher args]]
#
# Sends build/trimui-smartpro/app to the device. launcher.ini belongs to the
# device's owner (it holds the path to their game files): it is installed only
# when missing, from launcher.ini.default. exFAT has no permission bits worth
# trusting, so everything is tarred in and chmod'ed on arrival.
port_deploy() {
    local run=0 args=() excl=(--exclude=./launcher.ini --exclude=./home --exclude=./run-game.log)
    while [ $# -gt 0 ]; do
        case "$1" in
            --run)       run=1; shift ;;
            --no-engine) excl+=(--exclude=./SurrealEngine --exclude=./libSurrealVideo.so --exclude=./SurrealEngine.pk3); shift ;;
            --)          shift; args=("$@"); break ;;
            *)           die "unknown option: $1" ;;
        esac
    done
    local d="$DEVICE_APPDIR"
    say "deploying $APP to $DEVICE_USER@$DEVICE:$d ..."
    dx_ssh "mkdir -p '$d'"
    tar -C "$APP" "${excl[@]}" -cf - . | dx_ssh "tar -C '$d' -xf - && chmod +x '$d'/deusex-launcher '$d'/dxl-cli '$d'/*.sh && { [ ! -f '$d/SurrealEngine' ] || chmod +x '$d/SurrealEngine'; }"
    dx_ssh "[ -s '$d/launcher.ini' ] || cp '$d/launcher.ini.default' '$d/launcher.ini'"
    # MSAA must be off on the PowerVR GE8300 (speckle). The launcher writes
    # Settings.json before every launch and run-game.sh seeds it; this is the
    # last fallback, for an engine run by hand.
    dx_ssh "[ -s '$d/home/.config/SurrealEngine/Settings.json' ] || { mkdir -p '$d/home/.config/SurrealEngine'; cp '$d/engine-settings.json.default' '$d/home/.config/SurrealEngine/Settings.json'; }"
    say "done"
    if [ "$run" = 1 ]; then
        say "--- running ---"
        dx_ssh "cd '$d' && LD_LIBRARY_PATH=/usr/trimui/lib:/usr/lib:/lib ./deusex-launcher ${args[*]:-} 2>&1; echo \"exit=\$?\""
    fi
}
