# Shared by scripts/dx.sh, engine.sh, launcher.sh, recreation.sh, wine.sh,
# check-docs.sh and ci.sh; by the ports' port.sh files (on their launcher
# branches, sourced by dx.sh, engine.sh and ci.sh) and the Smart Pro's
# fetch-sysroot.sh; and by the engine's tools/host-tools.sh. Sourced, never run; expects bash with
# set -euo pipefail.

# The layout (docs/DEVELOPMENT.md#the-repositories): each repository in its
# own folder of one parent, DX_ROOT, beside what none of them owns.
DX_WORKSPACE="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"   # this repository
DX_ROOT="$(dirname "$DX_WORKSPACE")"
DX_ENGINE="$DX_ROOT/DXEngine"
DX_LAUNCHER="$DX_ROOT/deusex-launcher"     # main/, and a worktree per port
DX_RE="$DX_ROOT/dx-reverse-info"
DX_DEPS="$DX_ROOT/deps"

say() { printf '%s\n' "$*" >&2; }
die() { printf 'error: %s\n' "$*" >&2; exit 1; }

# The ports are the ones LAUNCHER-PIN.txt pins: each a branch of the launcher.
dx_ports() {
    awk '$1 == "port" { print $2 }' "$DX_WORKSPACE/LAUNCHER-PIN.txt"
}

# dx_link_workspace <launcher checkout>
#
# A port's toolchain files find deps/ beside its sources, and test_gamefiles
# finds gamefiles/ there: both are the ones beside the repositories, two levels
# up. Each checkout's build/ is its own (a device's host build and the
# desktop's would otherwise share one).
dx_link_workspace() {
    ln -sfn ../../deps "$1/deps"
    ln -sfn ../../gamefiles "$1/gamefiles"
}

# ---- devices over SSH -------------------------------------------------------
# A port sets DEVICE, DEVICE_USER, DEVICE_PASS (defaults in its port.sh, all
# overridable from the environment). sshpass -e reads the password from
# SSHPASS, so it never appears on a command line.
DX_SSHOPTS=(-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o LogLevel=ERROR -o ConnectTimeout=15)

dx_ssh() {
    command -v sshpass >/dev/null || die "sshpass is needed to reach the device"
    SSHPASS="$DEVICE_PASS" sshpass -e ssh "${DX_SSHOPTS[@]}" "$DEVICE_USER@$DEVICE" "$@"
}

# dx_ssh_sync <local dir> <device dir> [./path to leave alone]...
#
# Makes the device's copy of a staged app match it, and proves it did. Only
# files whose checksum differs are sent. The device's copies of those files are
# kept first in <device dir>/.prev-<device date-time>, so any deploy can be
# rolled back by copying one back. Afterwards every file is checksummed on the
# device against the staged one. Nothing on the device is deleted, and the
# paths to leave alone (the owner's own files) are never sent.
dx_ssh_sync() {
    local src="$1" dst="$2"; shift 2
    local f s files=() changed=()
    while IFS= read -r -d '' f; do
        for s in "$@"; do [ "$f" = "$s" ] && continue 2; done
        files+=("$f")
    done < <(cd "$src" && find . -type f -print0 | sort -z)
    [ ${#files[@]} -gt 0 ] || die "nothing to send in $src"
    local manifest; manifest="$(cd "$src" && md5sum "${files[@]}")"

    # busybox sh on the far side; the scripts use no single quotes.
    local differs='cd "$1" && while read -r sum f; do
        [ -f "$f" ] && [ "$(md5sum "$f" | cut -d" " -f1)" = "$sum" ] || echo "$f"
    done'
    local keep='cd "$1" && p=.prev-$(date +%Y%m%d-%H%M%S) && n=0 && while read -r f; do
        [ -f "$f" ] || continue
        mkdir -p "$p/$(dirname "$f")" && cp -p "$f" "$p/$f" && n=$((n + 1))
    done; if [ $n -gt 0 ]; then echo "$p"; fi'

    # Not mapfile < <(...): that hides ssh's exit status, and an unreachable
    # device would read as one that already has everything.
    local out
    out="$(printf '%s\n' "$manifest" | dx_ssh "mkdir -p '$dst' && sh -c '$differs' sh '$dst'")" ||
        die "cannot reach $DEVICE_USER@$DEVICE (asleep?) -- nothing was sent"
    [ -z "$out" ] || mapfile -t changed <<< "$out"
    if [ ${#changed[@]} -eq 0 ]; then
        say "the device already has this build (${#files[@]} files)"
    else
        local prev
        prev="$(printf '%s\n' "${changed[@]}" | dx_ssh "sh -c '$keep' sh '$dst'")"
        (cd "$src" && tar -cf - "${changed[@]}") | dx_ssh "tar -C '$dst' -xf -"
        say "sent ${#changed[@]} of ${#files[@]} files: ${changed[*]#./}"
        [ -z "$prev" ] || say "the device's previous copies: $dst/$prev"
    fi

    local sums; sums="$(printf '%s\n' "${files[@]}" | dx_ssh "cd '$dst' && xargs md5sum")"
    if [ "$sums" != "$manifest" ]; then
        diff <(printf '%s\n' "$manifest") <(printf '%s\n' "$sums") >&2 || true
        die "the device's copy does not match $src"
    fi
    say "verified ${#files[@]} files on the device"
}

# ---- Bootlin toolchains -----------------------------------------------------
# dx_fetch_bootlin <name> <glibc-max>: the toolchain as Bootlin names it, and the device's glibc
#
# Into deps/toolchains/<name>. Refuses a toolchain whose own glibc is newer than
# the ceiling: glibc 2.34 re-versioned the startup symbols, so anything linked
# against a newer one emits __libc_start_main@GLIBC_2.34 from crt1.o and will
# not load on an older device, whatever our own code calls.
dx_fetch_bootlin() {
    local name="$1" max="$2"
    local arch="${name%%--*}"
    local dest="$DX_DEPS/toolchains"
    local url="https://toolchains.bootlin.com/downloads/releases/toolchains/$arch/tarballs/$name.tar.bz2"
    local gcc="$dest/$name/bin/$arch-linux-gcc"

    if [ -x "$gcc" ]; then
        say "toolchain present: $name ($("$gcc" -dumpversion))"
    else
        mkdir -p "$dest"
        say "downloading $name ..."
        curl -fL --retry 3 --progress-bar -o "$dest/$name.tar.bz2" "$url"
        tar -C "$dest" -xf "$dest/$name.tar.bz2"
        rm -f "$dest/$name.tar.bz2"
    fi

    local minor
    minor=$(grep -oE '__GLIBC_MINOR__[[:space:]]+[0-9]+' \
        "$dest/$name/$arch-buildroot-linux-gnu/sysroot/usr/include/features.h" | grep -oE '[0-9]+$')
    [ "$minor" -le "${max#2.}" ] || die "$name has glibc 2.$minor, above the ceiling $max"
    say "  glibc 2.$minor (ceiling $max)"
}

# ---- pinned tarballs --------------------------------------------------------
# dx_fetch_cached <file> <url> <sha256>
#
# Into deps/cache/<file>, downloaded again unless it is the pinned one; the
# path printed.
dx_fetch_cached() {
    local file="$1" url="$2" sha="$3"
    local tar="$DX_DEPS/cache/$file"
    mkdir -p "$DX_DEPS/cache"
    if [ ! -f "$tar" ] || ! echo "$sha  $tar" | sha256sum -c --status; then
        say "downloading $file ..."
        curl -fL --retry 3 --progress-bar -o "$tar.part" "$url"
        mv "$tar.part" "$tar"
    fi
    echo "$sha  $tar" | sha256sum -c --status || die "$tar is not the pinned one ($sha)"
    printf '%s\n' "$tar"
}

# dx_unpack_into <tarball> <dir> <path>
#
# The tarball's one top folder becomes <dir>, replacing it: unpacked beside it
# first (tar takes any compression), and refused unless it holds <path>.
dx_unpack_into() {
    local tar="$1" dir="$2" need="$3"
    mkdir -p "$(dirname "$dir")"
    local tmp; tmp="$(mktemp -d "$(dirname "$dir")/.unpack.XXXXXX")"
    tar -C "$tmp" -xf "$tar"
    local top; top="$(ls "$tmp")"
    if [ -z "$top" ] || [ "$(printf '%s\n' "$top" | wc -l)" != 1 ] || [ ! -e "$tmp/$top/$need" ]; then
        rm -rf "$tmp"
        die "$tar holds no one folder with $need"
    fi
    rm -rf "$dir"
    mv "$tmp/$top" "$dir"
    rmdir "$tmp"
}

# ---- buildroot SDKs ---------------------------------------------------------
# dx_fetch_buildroot <name> <url> <sha256> <glibc-max>
#
# A Buildroot SDK -- a toolchain and its sysroot, as Godot's
# (github.com/godotengine/buildroot) -- into deps/toolchains/<name>: the
# tarball checked against its pinned SHA256, unpacked, relocated to where it
# is (relocate-sdk.sh, once), and refused when its own glibc is above the
# ceiling, as dx_fetch_bootlin refuses one.
dx_fetch_buildroot() {
    local name="$1" url="$2" sha="$3" max="$4"
    local dir="$DX_DEPS/toolchains/$name"

    if [ -x "$dir/relocate-sdk.sh" ] && [ -f "$dir/.dx-relocated" ]; then
        say "toolchain present: $name"
    else
        local tar; tar="$(dx_fetch_cached "$name.tar.bz2" "$url" "$sha")"
        dx_unpack_into "$tar" "$dir" relocate-sdk.sh
        (cd "$dir" && ./relocate-sdk.sh >/dev/null) || die "$name would not relocate"
        touch "$dir/.dx-relocated"
    fi

    local features minor
    features="$(ls "$dir"/*/sysroot/usr/include/features.h | head -n 1)"
    minor=$(grep -oE 'define[[:space:]]+__GLIBC_MINOR__[[:space:]]+[0-9]+' "$features" | grep -oE '[0-9]+$')
    [ "$minor" -le "${max#2.}" ] || die "$name has glibc 2.$minor, above the ceiling $max"
    say "  glibc 2.$minor (ceiling $max)"
}

# ---- llvm-mingw -------------------------------------------------------------
# dx_fetch_llvm_mingw <tag> <sha256>
#
# llvm-mingw (github.com/mstorsjo/llvm-mingw): Clang, LLD and mingw-w64 on the
# UCRT, a cross toolchain for Windows that runs here, into
# deps/toolchains/llvm-mingw-<tag>-ucrt-ubuntu-22.04-x86_64. Its release is
# checked against the SHA256 the port pins, then a C and a C++ program built
# with it statically must import none of its own DLLs (libc++, libunwind,
# winpthread): its programs need Windows' alone. No glibc to hold it to: it
# is a tool of this machine's, linking nothing of it into its programs.
dx_fetch_llvm_mingw() {
    local tag="$1" sha="$2"
    local name="llvm-mingw-$tag-ucrt-ubuntu-22.04-x86_64"
    local dir="$DX_DEPS/toolchains/$name"
    if [ -f "$dir/.dx-ready" ]; then
        say "toolchain present: $name"
        return 0
    fi
    local tar
    tar="$(dx_fetch_cached "$name.tar.xz" "https://github.com/mstorsjo/llvm-mingw/releases/download/$tag/$name.tar.xz" "$sha")"
    dx_unpack_into "$tar" "$dir" bin/x86_64-w64-mingw32-clang
    local tmp; tmp="$(mktemp -d)"
    printf '#include <stdio.h>\nint main(void) { puts("c"); return 0; }\n' > "$tmp/c.c"
    printf '%s\n' '#include <filesystem>' '#include <thread>' \
        'int main() { std::thread t([] {}); t.join(); return std::filesystem::exists(".") ? 0 : 1; }' > "$tmp/cxx.cpp"
    "$dir/bin/x86_64-w64-mingw32-clang" -static -o "$tmp/c.exe" "$tmp/c.c" || die "$name builds no C program"
    "$dir/bin/x86_64-w64-mingw32-clang++" -std=c++20 -static -o "$tmp/cxx.exe" "$tmp/cxx.cpp" ||
        die "$name builds no C++ program"
    local imports
    imports="$("$dir/bin/llvm-readobj" --coff-imports "$tmp/c.exe" "$tmp/cxx.exe")" || die "$name builds no PE program"
    rm -rf "$tmp"
    if printf '%s\n' "$imports" | grep -qiE 'Name: (libc\+\+|libunwind|libwinpthread)'; then
        die "$name links its own runtime DLLs into a static program"
    fi
    touch "$dir/.dx-ready"
    say "toolchain ready: $name ($("$dir/bin/x86_64-w64-mingw32-clang" --version | head -n 1))"
}

# ---- Wine -------------------------------------------------------------------
# The Windows port's programs and tests run here under a Proton build's own
# wine, as the engine's harness runs the original (DXEngine's tools/dxcap.sh):
# DX_PROTON names the build in Steam's compatibility tools (default
# "Proton-CachyOS Latest"), and its prefix is build/windows-x86_64/prefix
# (DX_WINEPREFIX another), made on first use.
dx_wine() {
    local w="$HOME/.local/share/Steam/compatibilitytools.d/${DX_PROTON:-Proton-CachyOS Latest}/files/bin/wine"
    [ -x "$w" ] || die "no Proton wine at $w (DX_PROTON names a build in ~/.local/share/Steam/compatibilitytools.d)"
    printf '%s\n' "$w"
}

dx_wine_prefix() { printf '%s\n' "${DX_WINEPREFIX:-$DX_ROOT/build/windows-x86_64/prefix}"; }

# dx_wine_ready: the prefix, made if missing -- by wineboot without Mono or
# Gecko, which nothing here needs, and with no crash dialog -- and given
# vkd3d's libraries, which Proton's wined3d, under its d3d11 and dxgi, imports
# and a bare wineboot's prefix lacks.
dx_wine_ready() {
    local wine prefix
    wine="$(dx_wine)"
    prefix="$(dx_wine_prefix)"
    if [ ! -f "$prefix/system.reg" ]; then
        say "making the Wine prefix $prefix ($("$wine" --version 2>/dev/null))"
        mkdir -p "$(dirname "$prefix")"
        env -u WAYLAND_DISPLAY WINEPREFIX="$prefix" WINEDEBUG=-all \
            WINEDLLOVERRIDES="winemenubuilder.exe=d;mscoree,mshtml=" "$wine" wineboot -i >/dev/null 2>&1 || true
        env WINEPREFIX="$prefix" "${wine%/wine}/wineserver" -w || true
        [ -f "$prefix/system.reg" ] || die "wineboot made no prefix at $prefix"
        env WINEPREFIX="$prefix" WINEDEBUG=-all "$wine" reg add 'HKCU\Software\Wine\WineDbg' \
            /v ShowCrashDialog /t REG_DWORD /d 0 /f >/dev/null 2>&1 || true
        env WINEPREFIX="$prefix" "${wine%/wine}/wineserver" -w || true
    fi
    local vkd3d="${wine%/bin/wine}/lib/vkd3d" arch dir f
    for arch in x86_64:system32 i386:syswow64; do
        dir="$prefix/drive_c/windows/${arch#*:}"
        for f in "$vkd3d/${arch%%:*}-windows"/libvkd3d*.dll; do
            [ -f "$f" ] || die "no vkd3d libraries in $vkd3d"
            [ -f "$dir/${f##*/}" ] || cp "$f" "$dir/"
        done
    done
}

# dx_wine_run <program> [args]: in the prefix, with nothing reaching the
# desktop's menus or file associations (winemenubuilder off), on its X display.
dx_wine_run() {
    local overrides="winemenubuilder.exe=d"
    [ -z "${WINEDLLOVERRIDES:-}" ] || overrides="$overrides;$WINEDLLOVERRIDES"
    env -u WAYLAND_DISPLAY WINEPREFIX="$(dx_wine_prefix)" WINEDEBUG="${WINEDEBUG:--all}" \
        WINEDLLOVERRIDES="$overrides" "$(dx_wine)" "$@"
}

# dx_winpath <absolute path>: Wine's name for it, Z: being the root.
dx_winpath() { printf 'Z:%s' "${1//\//\\}"; }

# dx_wine_dxvk: Proton's DXVK -- D3D11 on Vulkan -- in the prefix beside
# Wine's own d3d11 and dxgi, which stay the default: a run takes DXVK's with
# WINEDLLOVERRIDES="d3d11,dxgi=n,b".
dx_wine_dxvk() {
    local wine dxvk dir f
    wine="$(dx_wine)"
    dxvk="${wine%/bin/wine}/lib/wine/dxvk/x86_64-windows"
    dir="$(dx_wine_prefix)/drive_c/windows/system32"
    for f in d3d11.dll dxgi.dll; do
        [ -f "$dxvk/$f" ] || die "no DXVK $f in $dxvk"
        cmp -s "$dxvk/$f" "$dir/$f" || cp "$dxvk/$f" "$dir/$f"
    done
}

# dx_abi_args <port.cmake>: what the branch's scripts/check-abi.sh takes for
# the port -- --max <glibc> for a ceiling (DXL_PORT_GLIBC_MAX), --windows
# <version> for Windows' floor (DXL_PORT_WINDOWS_MIN) -- or nothing.
dx_abi_args() {
    local v
    v=$(sed -n 's/^[[:space:]]*set(DXL_PORT_GLIBC_MAX[[:space:]]\{1,\}\([0-9.]\{1,\}\))/\1/p' "$1" | head -n 1)
    if [ -n "$v" ]; then
        printf -- '--max %s\n' "$v"
        return 0
    fi
    v=$(sed -n 's/^[[:space:]]*set(DXL_PORT_WINDOWS_MIN[[:space:]]\{1,\}\([0-9.]\{1,\}\))/\1/p' "$1" | head -n 1)
    [ -z "$v" ] || printf -- '--windows %s\n' "$v"
}

# ---- Debian packages --------------------------------------------------------
# dx_fetch_debs <dest> <suite> <arch> <package>...
#
# Downloads each package (resolved through the suite's Packages index, so no
# version numbers are hardcoded) and unpacks its data into <dest>, the way
# dpkg would lay it out. Downloads are cached in deps/cache/. A suite gone to
# Debian's archive (buster) is fetched from there.
dx_fetch_debs() {
    local dest="$1" suite="$2" arch="$3"; shift 3
    local mirror="https://deb.debian.org/debian"
    case "$suite" in buster|stretch|jessie) mirror="https://archive.debian.org/debian" ;; esac
    local cache="$DX_DEPS/cache/debian-$suite-$arch"
    mkdir -p "$cache" "$dest"
    if [ ! -s "$cache/Packages" ]; then
        say "fetching the $suite/$arch package index ..."
        curl -fsSL --retry 3 "$mirror/dists/$suite/main/binary-$arch/Packages.xz" | xz -d > "$cache/Packages"
    fi
    local pkg file deb tmp
    for pkg in "$@"; do
        file=$(awk -v p="$pkg" '$1 == "Package:" { hit = ($2 == p) } hit && $1 == "Filename:" { print $2; exit }' "$cache/Packages")
        [ -n "$file" ] || die "no package $pkg in $suite/$arch"
        deb="$cache/$(basename "$file")"
        if [ ! -s "$deb" ]; then
            say "  $pkg <- $(basename "$file")"
            curl -fsSL --retry 3 -o "$deb" "$mirror/$file"
        fi
        tmp="$(mktemp -d)"
        (cd "$tmp" && ar x "$deb")
        tar -C "$dest" -xf "$tmp"/data.tar.*
        rm -rf "$tmp"
    done
}

# ---- Arch Linux packages (pinned headers, host tools) -----------------------
# dx_fetch_arch_pkg <dest> <name> <version> <arch> [path in the package]...
#   e.g. dx_fetch_arch_pkg "$tmp" vulkan-headers 1:1.4.357.0-1 any
#
# Unpacks one exact package version from archive.archlinux.org, which keeps
# every version ever published -- so a header set pinned by version comes back
# byte for byte. Only the paths given are unpacked (usr/include by default).
# Downloads are cached in deps/cache/.
dx_fetch_arch_pkg() {
    local dest="$1" name="$2" ver="$3" arch="$4"; shift 4
    local file="$name-$ver-$arch.pkg.tar.zst"
    local cache="$DX_DEPS/cache/archlinux"
    mkdir -p "$cache" "$dest"
    if [ ! -s "$cache/$file" ]; then
        say "  $name $ver"
        curl -fsSL --retry 3 -o "$cache/$file" \
            "https://archive.archlinux.org/packages/${name:0:1}/$name/${file//:/%3A}"
    fi
    tar -C "$dest" --zstd -xf "$cache/$file" "${@:-usr/include}"
}

# ---- a launcher branch's vendored sources ------------------------------------
# dx_fetch_src <deps.lock> [name]...
#
# The sources a launcher branch's cmake/deps.lock pins, each by version and
# SHA256, into deps/cache/src, where its vendored build reads them with no
# network: every one, or the names given. One already there with the pinned
# hash is kept.
dx_fetch_src() {
    local lock="$1"; shift
    local dest="$DX_DEPS/cache/src" name ver file sum url w want
    mkdir -p "$dest"
    while read -r name ver file sum url; do
        case "$name" in ''|'#'*) continue ;; esac
        if [ $# -gt 0 ]; then
            want=0
            for w in "$@"; do [ "$w" != "$name" ] || want=1; done
            [ "$want" = 1 ] || continue
        fi
        if [ -s "$dest/$file" ] && echo "$sum  $dest/$file" | sha256sum -c --status; then
            say "source present: $file"
            continue
        fi
        say "downloading $file ..."
        curl -fL --retry 3 --progress-bar -o "$dest/$file.part" "$url"
        if ! echo "$sum  $dest/$file.part" | sha256sum -c --status; then
            rm -f "$dest/$file.part"
            die "$file is not the one $lock pins (SHA256 $sum)"
        fi
        mv "$dest/$file.part" "$dest/$file"
    done < "$lock"
}
