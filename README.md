# gcOS

A gaming-focused operating-system prototype designed to be developed and tested safely in QEMU before touching real hardware.

## Current prototype

- GRUB Multiboot boot
- 32-bit freestanding C kernel
- VGA text display
- Keyboard polling
- Guest Session passcode creation
- Passcode confirmation
- Empty passcode option
- Guest Session welcome screen

The current passcode is memory-only. It is not persistent yet.

## Build on Debian/ChromeOS Linux

Install the tools:

    sudo apt update
    sudo apt install build-essential grub-pc-bin xorriso mtools qemu-system-x86

Clone the repository:

    git clone https://github.com/kdbroskii-droid/gcOS.git
    cd gcOS

Build:

    chmod +x build.sh
    ./build.sh

Run in QEMU:

    qemu-system-i386 -m 128M -cdrom build/gcos.iso

## Safety

gcOS is intended to be tested in QEMU first. Do not replace ChromeOS or write the ISO to a physical drive while the project is still experimental.
