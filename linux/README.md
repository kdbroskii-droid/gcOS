# gcOS Linux Edition

The original gcOS kernel prototype proved the boot/input concept. The next stage is a real Linux-based gcOS userspace so we can have real storage, networking, USB/input devices, graphics, applications, browsers, and normal processes.

## Architecture

- Linux kernel for hardware support
- Debian base for packages and drivers
- Lightweight desktop for the first graphical gcOS build
- Chromium for web access and Google account OAuth
- PipeWire for audio
- NetworkManager for networking
- Mesa/Xorg graphics stack
- GameMode for game performance integration

Google account support starts as normal secure OAuth in Chromium. gcOS should never ask the user for their Google password directly.

## Build

From Debian/ChromeOS Linux:

    sudo apt update
    sudo apt install live-build debootstrap xorriso squashfs-tools qemu-system-x86

Then:

    cd ~/gcos/linux
    chmod +x build-gcos.sh
    ./build-gcos.sh

The ISO will be created in the linux/build-output directory.

Run it in QEMU:

    qemu-system-x86_64 -m 2048 -smp 2 -cdrom build-output/gcos-linux.iso

This edition is separate from the tiny kernel prototype. Nothing writes to the Chromebook's ChromeOS installation.
