ENTRY(_start)

SECTIONS
{
    . = 0x1000;

    .text : ALIGN(4)
    {
        KEEP(*(.multiboot))
        *(.text)
    }

    .rodata : ALIGN(4)
    {
        *(.rodata*)
    }

    .data : ALIGN(4)
    {
        *(.data)
    }

    .bss : ALIGN(4)
    {
        *(COMMON)
        *(.bss)
    }
}
