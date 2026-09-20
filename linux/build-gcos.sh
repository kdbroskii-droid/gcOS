#!/bin/sh
set -eu

cd "$(dirname "$0")"

mkdir -p build-output

if ! command -v lb >/dev/null 2>&1; then
    echo "live-build is not installed."
    echo "Run: sudo apt install live-build debootstrap xorriso squashfs-tools qemu-system-x86"
    exit 1
fi

rm -rf config binary chroot cache .build
./auto/config

lb build

rm -f build-output/gcos-linux.iso
mv ./*.hybrid.iso build-output/gcos-linux.iso

echo
echo "gcOS Linux ISO built:"
echo "  build-output/gcos-linux.iso"
echo
echo "Run:"
echo "  qemu-system-x86_64 -m 2048 -smp 2 -cdrom build-output/gcos-linux.iso"
