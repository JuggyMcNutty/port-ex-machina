#!/usr/bin/env bash
# The launcher: deusex-launcher, our own repository. Its main branch recreates
# the original launcher; every other branch is a port. LAUNCHER-PIN.txt names
# the repository and, per port, the commit this workspace builds. The clone is
# deusex-launcher/main, beside this repository, with a worktree per port in
# deusex-launcher/<port> (on the branch of that name), each with deps/ and
# gamefiles/ linked in.
#
#   scripts/launcher.sh fetch                 clone it, and check out main and each pinned port (if absent)
#   scripts/launcher.sh check                 each port's worktree is at its pin, on its branch
#   scripts/launcher.sh status                the pins, and what each port's pin lacks of main
#   scripts/launcher.sh pin <port>|--all      write a worktree's HEAD to LAUNCHER-PIN.txt (pushed commits only)
#
# How the branches relate: docs/PORTING.md.
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

LAUNCHER_DIR="$DX_LAUNCHER"
PIN_FILE="$DX_WORKSPACE/LAUNCHER-PIN.txt"
MAIN="$LAUNCHER_DIR/main"

[ -f "$PIN_FILE" ] || die "no LAUNCHER-PIN.txt at the workspace root"
repo_url() { awk '$1 == "repo" { print $2 }' "$PIN_FILE"; }
pinned() { awk -v k="$1" '$1 == "port" && $2 == k { print $3 }' "$PIN_FILE"; }

usage() { sed -n '2,14p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2; }
need_clone() { [ -d "$MAIN/.git" ] || die "no launcher clone -- scripts/launcher.sh fetch"; }

# A port's toolchain files find deps/ beside its sources, and test_gamefiles
# finds gamefiles/ there: both are the ones beside the repositories, two levels
# up. Each checkout's build/ is its own (a device's host build and the
# desktop's would otherwise share one).
link_workspace() {
    ln -sfn ../../deps "$1/deps"
    ln -sfn ../../gamefiles "$1/gamefiles"
}

cmd_fetch() {
    if [ -d "$MAIN/.git" ]; then
        say "launcher clone present: $MAIN"
    else
        mkdir -p "$LAUNCHER_DIR"
        git clone --branch main "$(repo_url)" "$MAIN"
        git -C "$MAIN" config user.name JuggyMcNutty
        git -C "$MAIN" config user.email 11588877+JuggyMcNutty@users.noreply.github.com
        # Worktrees linked by relative paths, so the parent folder can move
        # (git 2.48 and later; an older git ignores it).
        git -C "$MAIN" config worktree.useRelativePaths true
    fi
    link_workspace "$MAIN"
    local p wt
    for p in $(dx_ports); do
        wt="$LAUNCHER_DIR/$p"
        if [ ! -e "$wt/.git" ]; then
            git -C "$MAIN" fetch --quiet origin
            git -C "$MAIN" worktree add -q -B "$p" "$wt" "$(pinned "$p")"
            git -C "$wt" branch -q --set-upstream-to "origin/$p"
            say "deusex-launcher/$p ready ($(git -C "$wt" rev-parse --short HEAD))"
        fi
        link_workspace "$wt"
    done
}

cmd_check() {
    need_clone
    local p wt want head rc=0 dirty
    for p in $(dx_ports); do
        wt="$LAUNCHER_DIR/$p"
        if [ ! -e "$wt/.git" ]; then
            echo "FAIL deusex-launcher/$p is missing -- scripts/launcher.sh fetch" >&2
            rc=1; continue
        fi
        want="$(pinned "$p")"
        if ! head="$(git -C "$wt" rev-parse HEAD 2>/dev/null)"; then
            echo "FAIL git cannot read deusex-launcher/$p -- moved? git -C deusex-launcher/main worktree repair" >&2
            rc=1; continue
        fi
        if [ "$head" = "$want" ]; then
            echo "OK   deusex-launcher/$p is at its pin: $(git -C "$wt" log -1 --format='%h %s')"
        else
            echo "FAIL deusex-launcher/$p is not at its pin" >&2
            echo "     pinned ${want:0:7}, HEAD $(git -C "$wt" log -1 --format='%h %s')" >&2
            if git -C "$wt" merge-base --is-ancestor "$want" "$head" 2>/dev/null; then
                echo "     HEAD is $(git -C "$wt" rev-list --count "$want..$head") commits past the pin -- scripts/launcher.sh pin $p records it" >&2
            fi
            rc=1
        fi
        if [ "$(git -C "$wt" rev-parse --abbrev-ref HEAD)" != "$p" ]; then
            echo "FAIL deusex-launcher/$p is not on its branch" >&2
            rc=1
        fi
        dirty=$(git -C "$wt" status --porcelain --untracked-files=no)
        [ -z "$dirty" ] || say "note: uncommitted changes in deusex-launcher/$p:"$'\n'"$dirty"
    done
    return "$rc"
}

# main is the working base: each port branch takes main's changes by merging
# main into it (docs/PORTING.md#the-branches), so a pin that lacks commits of
# main has a merge to take.
cmd_status() {
    need_clone
    git -C "$MAIN" fetch --quiet origin
    printf '%-16s %s\n' main "$(git -C "$MAIN" log -1 --format='%h  %cs  %s' origin/main)"
    local p want n
    for p in $(dx_ports); do
        want="$(pinned "$p")"
        printf '%-16s %s\n' "$p" "$(git -C "$MAIN" log -1 --format='%h  %cs  %s' "$want")"
        n=$(git -C "$MAIN" rev-list --count "$want..origin/main")
        [ "$n" = 0 ] || echo "                 lacks $n commit(s) of main -- merge main into $p"
    done
}

# The pin moves only to a commit the repository has: a pin no one can fetch
# would break every other clone's cold start.
cmd_pin() {
    local p="${1:-}"
    [ -n "$p" ] || die "pin which port? (or --all)"
    if [ "$p" = --all ]; then
        for p in $(dx_ports); do cmd_pin "$p"; done
        return
    fi
    local wt="$LAUNCHER_DIR/$p"
    [ -e "$wt/.git" ] || die "no deusex-launcher/$p -- scripts/launcher.sh fetch"
    [ "$(git -C "$wt" rev-parse --abbrev-ref HEAD)" = "$p" ] || die "deusex-launcher/$p is not on its branch"
    [ -z "$(git -C "$wt" status --porcelain --untracked-files=no)" ] ||
        die "uncommitted changes in deusex-launcher/$p -- commit them first"
    local head tmp
    head="$(git -C "$wt" rev-parse HEAD)"
    git -C "$wt" fetch --quiet origin
    git -C "$wt" merge-base --is-ancestor "$head" "origin/$p" 2>/dev/null ||
        die "deusex-launcher/$p's HEAD is not on origin/$p -- push it first, or fetch cannot reproduce this pin"
    tmp="$(mktemp)"
    awk -v k="$p" -v c="$head" '
        $1 == "port" && $2 == k { printf "port    %-16s %s\n", k, c; done = 1; next }
        { print }
        END { if (!done) printf "port    %-16s %s\n", k, c }' "$PIN_FILE" > "$tmp"
    mv "$tmp" "$PIN_FILE"
    say "pinned $p at $(git -C "$wt" log -1 --format='%h %s') -- commit LAUNCHER-PIN.txt with what depends on it"
}

cmd="${1:-}"; [ -n "$cmd" ] || usage; shift
case "$cmd" in
    fetch)  cmd_fetch ;;
    check)  cmd_check ;;
    status) cmd_status ;;
    pin)    cmd_pin "$@" ;;
    -h|--help|help) usage ;;
    *)      die "unknown command '$cmd' (scripts/launcher.sh help)" ;;
esac
