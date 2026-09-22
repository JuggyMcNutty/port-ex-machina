#!/bin/sh
# Hands off from the launcher to the engine.
#
# The launcher has already settled configuration (renderer, detail block,
# FirstRun) and created the crash sentinel, and it chdir's to GameDir before
# exec'ing this script. Everything here is about starting Surreal Engine and
# honouring the sentinel contract on the way out.

APPDIR="$(cd "$(dirname "$0")" && pwd)"
GAMEDIR="$(pwd)"
LOG="$APPDIR/run-game.log"

# APPDIR first: the engine links libSurrealVideo.so, which ships beside it.
export LD_LIBRARY_PATH="$APPDIR:/usr/trimui/lib:/usr/lib:/lib:$LD_LIBRARY_PATH"

# The engine writes Settings.json and SE-Log-LastRun.txt under
# $HOME/.config/SurrealEngine. Pin HOME to the SD card so both land somewhere
# writable and readable over SSH, instead of wherever HOME points on boot.
export HOME="$APPDIR/home"
mkdir -p "$HOME/.config/SurrealEngine"

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

"$APPDIR/SurrealEngine" --no-launcher "$GAMEDIR" "$@" >> "$LOG" 2>&1
status=$?
echo "engine exit: $status" >> "$LOG"

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
