#!/usr/bin/env bash
# Scripted runs of both engines -- the original game under Proton and the
# engine fork -- each driven by a console class of the DXCapture package
# (tools/dxcap), named in a private ini made from the game's own for every
# run: the game's inis are never touched. docs/DEVELOPMENT.md, scripted runs.
#
#   scripts/dxcap.sh setup                         build/dxcap: the SDK's UCC among links to the game's files
#   scripts/dxcap.sh compile                       tools/dxcap -> build/dxcap/System/DXCapture.u (UCC, under Proton)
#   scripts/dxcap.sh original <console> [<secs>]   the original, from its menu map (it takes no map to start)
#   scripts/dxcap.sh fork <console> <map> [<secs>] the fork's linux-x86_64 build, straight into <map>
#   scripts/dxcap.sh prove <map>                   the fork's proving run: shots at 20 s and 60 s, exit at 65 s
#
# A run's shots and log land in build/dxcap/runs/<engine>-<console>-<time>/.
# The original runs on the host through umu-run (from a container, through
# distrobox-host-exec); DXCAP_PREFIX (default ~/Games/umu/umu-default) and
# DXCAP_PROTON (default "Proton-CachyOS Latest") pick the Wine prefix and the
# Proton under ~/.local/share/Steam/compatibilitytools.d. DXCAP_AUDIO=1 gives
# the fork real audio; it is silent otherwise.
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

GAME="$DX_ROOT/gamefiles"
CAP="$DX_ROOT/build/dxcap"
SDK="$DX_ROOT/reference/ReleaseSDK1112f/System"
ENGINE_BIN="$DX_ROOT/build/linux-x86_64/engine/SurrealEngine"

usage() { sed -n '2,20p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2; }

[ -d "$GAME/System" ] || die "no game install at $GAME"

# Runs a command on the host, where Proton is.
on_host() {
    if command -v umu-run >/dev/null; then
        "$@"
    elif command -v distrobox-host-exec >/dev/null; then
        distrobox-host-exec "$@"
    else
        die "neither umu-run nor distrobox-host-exec is here"
    fi
}

# Wine's view of an absolute path: Z: is the root.
winpath() { printf 'Z:%s' "${1//\//\\}"; }

# make_ini <original|fork> <console> <out>: the game's DeusEx.ini with the
# console class, the package's path, and for the original a window of
# 1280x720 drawn through OpenGL -- the one renderer whose SHOT reads back
# under Proton.
make_ini() {
    local engine="$1" console="$2" out="$3" path
    if [ "$engine" = original ]; then
        path="$(winpath "$CAP/System")\\*.u"
    else
        path="$CAP/System/*.u"
    fi
    python3 - "$GAME/System/DeusEx.ini" "$out" "$engine" "$console" "$path" <<'EOF'
import re, sys
src, out, engine, console, path = sys.argv[1:]
s = open(src, encoding='latin1', newline='').read()
nl = '\r\n' if '\r\n' in s else '\n'
def put(key, value):
    global s
    s, n = re.subn(r'^%s=.*$' % re.escape(key), lambda m: '%s=%s' % (key, value), s, count=1, flags=re.M)
    if n != 1:
        sys.exit('no %s= in %s' % (key, src))
put('Console', 'DXCapture.' + console)
s, n = re.subn(r'^(Paths=\.\.\\System\\\*\.u)\r?$', lambda m: m.group(1) + nl + 'Paths=' + path, s, count=1, flags=re.M)
if n != 1:
    sys.exit('no Paths=..\\System\\*.u in ' + src)
if engine == 'original':
    put('WindowedViewportX', '1280')
    put('WindowedViewportY', '720')
    put('StartupFullscreen', 'False')
    put('GameRenderDevice', 'OpenGLDrv.OpenGLRenderDevice')
open(out, 'w', encoding='latin1', newline='').write(s)
EOF
}

cmd_setup() {
    [ -f "$SDK/UCC.exe" ] || die "no SDK at $SDK (reference/ReleaseSDK1112f)"
    mkdir -p "$CAP/System" "$CAP/DXCapture" "$CAP/runs"
    local f d
    for f in "$GAME"/System/*.u "$GAME"/System/*.dll "$GAME"/System/*.int; do
        ln -sfn "$f" "$CAP/System/$(basename "$f")"
    done
    # The SDK's UCC with the Core and Window it was built against.
    for f in UCC.exe Core.dll Window.dll; do
        rm -f "$CAP/System/$f"
        cp "$SDK/$f" "$CAP/System/$f"
    done
    for d in Textures Sounds Music Maps; do
        ln -sfn "$GAME/$d" "$CAP/$d"
    done
    ln -sfn "$DX_ROOT/tools/dxcap/Classes" "$CAP/DXCapture/Classes"
    # UCC wants both UCC.ini and DeusEx.ini, and the defaults beside them:
    # the game's, with the package to compile last.
    ln -sfn "$GAME/System/Default.ini" "$CAP/System/Default.ini"
    ln -sfn "$GAME/System/DefUser.ini" "$CAP/System/DefUser.ini"
    python3 - "$GAME/System/DeusEx.ini" "$CAP/System/UCC.ini" <<'EOF'
import re, sys
s = open(sys.argv[1], encoding='latin1', newline='').read()
nl = '\r\n' if '\r\n' in s else '\n'
s, n = re.subn(r'^(EditPackages=DeusEx)\r?$', lambda m: m.group(1) + nl + 'EditPackages=DXCapture', s, count=1, flags=re.M)
if n != 1:
    sys.exit('no EditPackages=DeusEx in ' + sys.argv[1])
open(sys.argv[2], 'w', encoding='latin1', newline='').write(s)
EOF
    cp "$CAP/System/UCC.ini" "$CAP/System/DeusEx.ini"
    cp "$GAME/System/User.ini" "$CAP/System/User.ini"

    # The original's run, on the host: started in a Wine desktop -- its
    # command line's first word is taken for the start URL, so the menu map
    # goes first -- waited for
    # until it exits (each console class ends its run with EXIT) or the time
    # runs out; then only its own process is killed -- the prefix may hold
    # IDA as well.
    cat > "$CAP/run-original.sh" <<EOF
#!/bin/sh
secs="\$1"; ini="\$2"; userini="\$3"
cd "$GAME/System" || exit 1
rm -f Running.ini
export WINEPREFIX="\${DXCAP_PREFIX:-\$HOME/Games/umu/umu-default}"
export GAMEID=umu-default
export PROTONPATH="\$HOME/.local/share/Steam/compatibilitytools.d/\${DXCAP_PROTON:-Proton-CachyOS Latest}"
[ -n "\$DISPLAY" ] || export DISPLAY=:0
[ -n "\$XAUTHORITY" ] || export XAUTHORITY="\$(ls /run/user/\$(id -u)/xauth_* 2>/dev/null | head -1)"
umu-run explorer /desktop=dxcap,1280x720 DeusEx.exe DX.dx "INI=\$ini" "USERINI=\$userini" > "$CAP/umu.log" 2>&1 &
i=0
while [ \$i -lt 60 ] && ! pgrep -f "^DeusEx.exe" > /dev/null; do sleep 1; i=\$((i+1)); done
i=0
while [ \$i -lt "\$secs" ] && pgrep -f "^DeusEx.exe" > /dev/null; do sleep 1; i=\$((i+1)); done
if pgrep -f "^DeusEx.exe" > /dev/null; then
    kill \$(pgrep -f "^DeusEx.exe")
    echo "killed after \$secs s"
else
    echo "exited after \$i s"
fi
sleep 3
EOF
    chmod +x "$CAP/run-original.sh"
    say "build/dxcap is ready -- scripts/dxcap.sh compile next"
}

cmd_compile() {
    [ -x "$CAP/run-original.sh" ] || die "scripts/dxcap.sh setup first"
    rm -f "$CAP/System/DXCapture.u"
    on_host sh -c "cd '$CAP/System' && WINEPREFIX=\"\${DXCAP_PREFIX:-\$HOME/Games/umu/umu-default}\" GAMEID=umu-default PROTONPATH=\"\$HOME/.local/share/Steam/compatibilitytools.d/\${DXCAP_PROTON:-Proton-CachyOS Latest}\" umu-run UCC.exe make > '$CAP/ucc.out' 2>&1" || true
    grep -E "Error|error\(s\)" "$CAP/System/UCC.log" >&2 || true
    [ -f "$CAP/System/DXCapture.u" ] || die "UCC made no DXCapture.u -- build/dxcap/System/UCC.log"
    say "compiled: build/dxcap/System/DXCapture.u"
}

# Shots the game writes into its System folder, but not ones that were
# already there before the run.
shots_before() { (cd "$GAME/System" && ls Shot*.bmp 2>/dev/null) || true; }

collect() {
    local dir="$1" before="$2" f
    mkdir -p "$dir"
    for f in $( (cd "$GAME/System" && ls Shot*.bmp 2>/dev/null) || true); do
        if ! printf '%s\n' "$before" | grep -qx "$f"; then
            mv "$GAME/System/$f" "$dir/$f"
        fi
    done
}

cmd_original() {
    local console="${1:?console class}" secs="${2:-120}"
    [ -f "$CAP/System/DXCapture.u" ] || die "scripts/dxcap.sh compile first"
    local ini="$CAP/System/Original.ini" userini="$CAP/System/OriginalUser.ini"
    local dir="$CAP/runs/original-$console-$(date +%H%M%S)"
    make_ini original "$console" "$ini"
    cp "$GAME/System/User.ini" "$userini"
    local before; before="$(shots_before)"
    on_host "$CAP/run-original.sh" "$secs" "$(winpath "$ini")" "$(winpath "$userini")"
    collect "$dir" "$before"
    cp "$GAME/System/DeusEx.log" "$dir/DeusEx.log" 2>/dev/null || true
    say "original: $dir"
}

cmd_fork() {
    local console="${1:?console class}" map="${2:?map}" secs="${3:-120}"
    [ -f "$CAP/System/DXCapture.u" ] || die "scripts/dxcap.sh compile first"
    [ -x "$ENGINE_BIN" ] || die "no engine build at $ENGINE_BIN"
    local ini="$CAP/System/Fork.ini" userini="$CAP/System/ForkUser.ini"
    local dir="$CAP/runs/fork-$console-$(date +%H%M%S)"
    make_ini fork "$console" "$ini"
    cp "$GAME/System/User.ini" "$userini"
    local before; before="$(shots_before)"
    mkdir -p "$dir"
    local rc=0
    (
        cd "$GAME"
        if [ "${DXCAP_AUDIO:-0}" != 1 ]; then
            printf '[general]\ndrivers = null\n' > "$CAP/alsoft-null.conf"
            export ALSOFT_CONF="$CAP/alsoft-null.conf"
        fi
        timeout -s KILL "$secs" "$ENGINE_BIN" --no-launcher "$GAME" --ini="$ini" --userini="$userini" --url="$map" > "$dir/engine.log" 2>&1
    ) || rc=$?
    collect "$dir" "$before"
    printf '%s\n' "$rc" > "$dir/exit"
    say "fork: $dir (exit $rc)"
    printf '%s\n' "$dir"
}

cmd_prove() {
    local map="${1:?map}" dir shots=0 f mean bad=0
    dir="$(cmd_fork ProveConsole "$map" 90 | tail -1)"
    [ "$(cat "$dir/exit")" = 0 ] || { say "the run did not exit cleanly: $dir/engine.log"; bad=1; }
    for f in "$dir"/Shot*.bmp; do
        [ -f "$f" ] || continue
        shots=$((shots + 1))
        mean="$(magick "$f" -format '%[fx:mean]' info:)"
        say "$(basename "$f"): mean brightness $mean"
        awk -v m="$mean" 'BEGIN { exit !(m < 0.01) }' && { say "  -- nearly black"; bad=1; }
    done
    [ "$shots" = 2 ] || { say "expected 2 shots, got $shots"; bad=1; }
    grep -E "Script error|SurrealEngine error|Unimplemented" "$dir/engine.log" | sort | uniq -c >&2 || true
    [ "$bad" = 0 ] || die "proving run failed: $dir"
    say "proving run clean: $dir"
}

cmd="${1:-help}"; shift || true
case "$cmd" in
    setup)    cmd_setup ;;
    compile)  cmd_compile ;;
    original) cmd_original "$@" ;;
    fork)     cmd_fork "$@" ;;
    prove)    cmd_prove "$@" ;;
    -h|--help|help) usage ;;
    *)        die "unknown command '$cmd' (scripts/dxcap.sh help)" ;;
esac
