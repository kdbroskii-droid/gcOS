#!/bin/sh
set -eu

cd "$(dirname "$0")"

mkdir -p build/isodir/boot/grub

rm -f build/boot.o build/kernel.o build/gcos.elf build/gcos.iso

as --32 src/boot.s -o build/boot.o
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c src/kernel.c -o build/kernel.o
ld -m elf_i386 -T linker.ld -o build/gcos.elf build/boot.o build/kernel.o

if command -v grub-file >/dev/null 2>&1; then
    grub-file --is-x86-multiboot build/gcos.elf
fi

cp build/gcos.elf build/isodir/boot/gcos.elf

printf '%s\n' 'menuentry "gcOS" {' '    multiboot /boot/gcos.elf' '    boot' '}' > build/isodir/boot/grub/grub.cfg

grub-mkrescue -o build/gcos.iso build/isodir

echo
echo "Built: build/gcos.iso"
echo "Run it with:"
echo "qemu-system-i386 -m 128M -cdrom build/gcos.iso"
