# Shared by scripts/dx.sh, scripts/engine.sh and the ports' port.sh files.
# Sourced, never run; expects bash with set -euo pipefail.

DX_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
DX_DEPS="$DX_ROOT/deps"

say() { printf '%s\n' "$*" >&2; }
die() { printf 'error: %s\n' "$*" >&2; exit 1; }

# Every directory under ports/ except common/ is a port.
dx_ports() {
    local d
    for d in "$DX_ROOT"/ports/*/; do
        d="$(basename "$d")"
        [ "$d" = common ] || printf '%s\n' "$d"
    done
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

    mapfile -t changed < <(printf '%s\n' "$manifest" | dx_ssh "mkdir -p '$dst' && sh -c '$differs' sh '$dst'")
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
# dx_fetch_bootlin <name> <glibc-max>, e.g. aarch64--glibc--stable-2020.08-1 2.33
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

# ---- Debian packages --------------------------------------------------------
# dx_fetch_debs <dest> <suite> <arch> <package>...
#
# Downloads each package (resolved through the suite's Packages index, so no
# version numbers are hardcoded) and unpacks its data into <dest>, the way
# dpkg would lay it out. Downloads are cached in deps/cache/.
dx_fetch_debs() {
    local dest="$1" suite="$2" arch="$3"; shift 3
    local mirror="https://deb.debian.org/debian"
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
