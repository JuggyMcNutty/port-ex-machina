#!/bin/sh
# Hands off from the launcher to the engine.
#
# The launcher has already settled configuration (Settings.json, the game's
# ini files, FirstRun) and created the crash sentinel, and it chdir's to
# GameDir before exec'ing this script. Everything here is about starting Surreal Engine and
# honouring the sentinel contract on the way out.

APPDIR="$(cd "$(dirname "$0")" && pwd)"
GAMEDIR="$(pwd)"
LOG="$APPDIR/run-game.log"

# APPDIR first: the engine links libSurrealVideo.so, which ships beside it.
export LD_LIBRARY_PATH="$APPDIR:/usr/trimui/lib:/usr/lib:/lib:$LD_LIBRARY_PATH"

# The engine reads Settings.json and writes SE-Log-LastRun.txt under
# $HOME/.config/SurrealEngine. Pin HOME to the SD card so both live somewhere
# the launcher can reach and readable over SSH, instead of wherever HOME
# points on boot.
export HOME="$APPDIR/home"
mkdir -p "$HOME/.config/SurrealEngine"

# The engine defaults to 4x MSAA when there is no Settings.json, and the
# PowerVR Rogue GE8300 produces garbage with it (partially covered pixels
# resolve to speckle). The launcher writes the file before every launch; this
# seed only matters when something runs the script without it.
if [ ! -s "$HOME/.config/SurrealEngine/Settings.json" ]; then
    cp "$APPDIR/engine-settings.json.default" "$HOME/.config/SurrealEngine/Settings.json"
fi

# The vendor SDL2 is the only display path here. Its "mali" video driver wires
# Vulkan surface creation to the PowerVR implementation, which is what lets the
# engine's Vulkan renderer present at 1280x720 via VK_KHR_display.
export SURREALWIDGETS_DISPLAY_BACKEND=SDL2

{
    echo "--- $(date '+%Y-%m-%d %H:%M:%S') ---"
    echo "gamedir: $GAMEDIR"
    echo "args:    $*"
} >> "$LOG"

if [ ! -x "$APPDIR/SurrealEngine" ]; then
    echo "SurrealEngine binary missing -- nothing to launch" >> "$LOG"
    exit 127
fi

# CPU mode. The spruceOS menu leaves the handheld in power-save -- two cores,
# conservative governor -- which costs the game about a third of its frame
# rate. Apply the launcher's CpuMode (launcher.ini) with spruceOS's own
# helpers, the ones its Ports launcher uses, and put the previous state back
# on the way out so the menu gets what it had.
#
# The helpers are sourced in subshells only: helperFunctions.sh exports its
# own LD_LIBRARY_PATH, which would hide libSurrealVideo.so from the engine.
CPU=/sys/devices/system/cpu
HELPERS=/mnt/SDCARD/spruce/scripts/helperFunctions.sh
cpu_mode=$(sed -n 's/^[[:space:]]*CpuMode[[:space:]]*=[[:space:]]*//p' "$APPDIR/launcher.ini" 2>/dev/null | tr -d '\r' | head -n 1)
cpu_saved=""
if [ -f "$HELPERS" ]; then
    cpu_saved="$(cat $CPU/online) $(cat $CPU/cpu0/cpufreq/scaling_governor) $(cat $CPU/cpu0/cpufreq/scaling_min_freq) $(cat $CPU/cpu0/cpufreq/scaling_max_freq)"
    (
        . "$HELPERS" >/dev/null 2>&1
        case "$cpu_mode" in
            Smart)     set_smart       >/dev/null 2>&1 ;;
            Overclock) set_overclock   >/dev/null 2>&1 ;;
            *)         set_performance >/dev/null 2>&1 ;;
        esac
    )
    echo "cpu mode: ${cpu_mode:-Performance} (online $(cat $CPU/online), $(cat $CPU/cpu0/cpufreq/scaling_governor) up to $(cat $CPU/cpu0/cpufreq/scaling_max_freq))" >> "$LOG"
fi

restore_cpu() {
    [ -n "$cpu_saved" ] || return 0
    ( . "$HELPERS" >/dev/null 2>&1; restore_cpu_state $cpu_saved )
    echo "cpu restored: online $(cat $CPU/online), $(cat $CPU/cpu0/cpufreq/scaling_governor)" >> "$LOG"
}

# Runs inside the helpers' subshell: online governor min max.
restore_cpu_state() {
    # "0,3" or "0-3" -> the digit string spruce's cores_online takes
    cores=""
    for part in $(echo "$1" | tr ',' ' '); do
        case "$part" in
            *-*) i=${part%-*}
                 while [ "$i" -le "${part#*-}" ]; do cores="$cores$i"; i=$((i + 1)); done ;;
            *)   cores="$cores$part" ;;
        esac
    done
    cores_online "$cores" >/dev/null 2>&1
    unlock_governor >/dev/null 2>&1
    echo "$2" > $CPU/cpu0/cpufreq/scaling_governor
    # min and max each must stay within the other: write min, max, then min
    # again so the order works whichever way the range is moving.
    echo "$3" > $CPU/cpu0/cpufreq/scaling_min_freq 2>/dev/null
    echo "$4" > $CPU/cpu0/cpufreq/scaling_max_freq
    echo "$3" > $CPU/cpu0/cpufreq/scaling_min_freq
    lock_governor >/dev/null 2>&1
}

"$APPDIR/SurrealEngine" --no-launcher "$GAMEDIR" "$@" >> "$LOG" 2>&1
status=$?
echo "engine exit: $status" >> "$LOG"
restore_cpu

# Set DXL_SIMULATE_CRASH=1 to leave the sentinel behind whatever the engine did,
# so the recovery path can be exercised without actually crashing anything.
if [ "$DXL_SIMULATE_CRASH" = "1" ]; then
    echo "DXL_SIMULATE_CRASH=1, leaving Running.ini in place" >> "$LOG"
    exit 1
fi

# The sentinel is the launcher's crash contract: clear it only on a clean exit,
# so an engine crash still produces the recovery screen on the next launch.
if [ "$status" -eq 0 ]; then
    rm -f "$GAMEDIR/System/Running.ini"
    echo "clean exit, removed Running.ini" >> "$LOG"
else
    echo "unclean exit, leaving Running.ini for recovery" >> "$LOG"
fi

exit $status
