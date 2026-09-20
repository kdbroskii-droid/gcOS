# xoriesOS

xoriesOS is a lightweight Debian-based desktop operating system designed to stay below a 2 GB ISO target.

Included:
- XFCE desktop
- Xorg display server
- XFCE Terminal
- Google Chrome Stable
- NetworkManager + Wi-Fi support
- Bluetooth + Blueman
- PipeWire audio
- File manager and removable-storage support
- Debian Linux kernel
- Common firmware for Wi-Fi, Bluetooth, audio and graphics
- sudo and basic system utilities

The build is done in GitHub Actions so a low-memory Chromebook does not have to build the ISO locally.

## Build

Push changes under linux/ or manually run the GitHub Actions workflow.

The workflow fails if the resulting ISO is larger than 2 GiB.

## Test

After downloading the artifact:

qemu-system-x86_64 -m 1024 -smp 2 -cdrom xoriesOS.iso

This is an amd64 build.
