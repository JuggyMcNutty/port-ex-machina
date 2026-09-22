#!/bin/sh
# Stand-in for the game.
#
# Nothing on this device can execute the x86 Windows Core.dll / Engine.dll /
# DeusEx.dll yet: there is no box64, no wine, no native UE1 engine. So the
# launcher's final exec lands here.
#
# This is not a placeholder to be embarrassed about -- it is what makes the
# rest observable. The launcher's whole contract (renderer choice, detail
# block, FirstRun clamp, the crash sentinel, the safe-mode flag string)
# happens BEFORE this point, and this script records exactly what the launcher
# decided to hand over.
#
# Replacing it with a real runtime is a one-line change in launcher.ini.

LOG="${0%/*}/run-game.log"
GAMEDIR="$(pwd)"

{
    echo "--- $(date '+%Y-%m-%d %H:%M:%S') ---"
    echo "cwd:  $GAMEDIR"
    echo "argc: $#"
    i=1
    for a in "$@"; do
        echo "argv[$i]: $a"
        i=$((i + 1))
    done
    [ $# -eq 0 ] && echo "(no arguments)"
} >> "$LOG"

# The game owns the crash sentinel from here: it is deleted on a clean exit,
# and survives a crash so the next launch offers recovery. Set DXL_STUB_CRASH=1
# to leave it behind and exercise that path.
SENTINEL="$GAMEDIR/System/Running.ini"
if [ "$DXL_STUB_CRASH" = "1" ]; then
    echo "exit: simulating a crash, leaving $SENTINEL in place" >> "$LOG"
    exit 1
fi

if [ -f "$SENTINEL" ]; then
    rm -f "$SENTINEL"
    echo "exit: clean, removed Running.ini" >> "$LOG"
else
    echo "exit: clean, no Running.ini found" >> "$LOG"
fi
exit 0
