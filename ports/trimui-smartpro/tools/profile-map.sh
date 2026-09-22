#!/bin/sh
# Frame-time profile of one map, on the handheld.
#
# Needs an engine built with engine-patches/optional/perf-instrumentation.patch
# applied (the hooks are temporary and never committed -- see the patch header
# comments). From the repository root:
#
#   scripts/engine.sh perf on
#   scripts/dx.sh build trimui-smartpro engine && scripts/dx.sh stage trimui-smartpro
#   scripts/dx.sh deploy trimui-smartpro
#   ...profile...
#   scripts/engine.sh perf off     (then rebuild, stage and deploy again)
#
# Then copy this script to the device and run it over SSH:
#
#   profile-map.sh [seconds] [label] [perf|none] [turn] [map]
#
#   seconds  how long to run (default 60)
#   label    names the log: /tmp/dxl-test/perf-<label>.log
#   perf     switch spruceOS to performance mode first, restore power-save after
#   turn     aBaseX to turn the player on the spot (3000 = keyboard turn speed),
#            so an unattended run still brings new surfaces into view
#   map      default 01_NYC_UNATCOIsland.dx (Liberty Island). The engine only
#            takes --url=<map>; "-u <map>" silently loads the default map.
#
# The spruceOS menu is paused while the engine runs (two programs drawing to
# one framebuffer fight) and resumed on exit. The engine ignores SIGTERM, so
# it is stopped with SIGKILL; that leaves Running.ini like any crash.
SECS=${1:-60}; LABEL=${2:-run}; MODE=${3:-none}; TURN=${4:-0}; MAP=${5:-01_NYC_UNATCOIsland.dx}
OUT=/tmp/dxl-test/perf-$LABEL.log
mkdir -p /tmp/dxl-test
M=$(pidof MainUI)
[ -n "$M" ] && kill -STOP $M
restore() { [ -n "$M" ] && kill -CONT $M; }
trap restore EXIT INT TERM HUP
if [ "$MODE" = "perf" ]; then
    ( . /mnt/SDCARD/spruce/scripts/helperFunctions.sh >/dev/null 2>&1; set_performance >/dev/null 2>&1 )
fi
C=/sys/devices/system/cpu
echo "cpu: online=$(cat $C/online) gov=$(cat $C/cpu0/cpufreq/scaling_governor) max=$(cat $C/cpu0/cpufreq/scaling_max_freq) map=$MAP" > $OUT
cd /mnt/SDCARD/Roms/PORTS/DeusEx || exit 1
export HOME=/mnt/SDCARD/App/DeusEx/home
export LD_LIBRARY_PATH=/mnt/SDCARD/App/DeusEx:/usr/trimui/lib:/usr/lib:/lib
export SURREALWIDGETS_DISPLAY_BACKEND=SDL2 SURREAL_PERF_LOG=1 SURREAL_PERF_TURN=$TURN
setsid /mnt/SDCARD/App/DeusEx/SurrealEngine --no-launcher /mnt/SDCARD/Roms/PORTS/DeusEx "--url=$MAP" </dev/null >>$OUT 2>&1 &
PID=$!
i=0
while [ $i -lt "$SECS" ]; do
    sleep 5; i=$((i + 5))
    echo "t=$i cpu: $(cat $C/cpu0/cpufreq/scaling_cur_freq) load: $(cat /proc/loadavg)" >> $OUT
done
kill -9 $PID 2>/dev/null
sleep 1
echo "engines left: $(pidof SurrealEngine | wc -w)" >> $OUT
if [ "$MODE" = "perf" ]; then
    ( . /mnt/SDCARD/spruce/scripts/helperFunctions.sh >/dev/null 2>&1; set_powersave >/dev/null 2>&1 )
fi
grep -E "^cpu:|^perf:|engines left" $OUT
