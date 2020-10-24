OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start64)

SECTIONS
{
    . = 0xffff800001000000;
    _kernel_start = .;
    .text.init :{ *(.text.init*)    KEEP(*(.text.init)) }
    .text :     { *(.text*)         KEEP(*(.text))      }
    .rodata :   { *(.rodata*)       KEEP(*(.rodata))    }
    .data :     { *(.data*)         KEEP(*(.data))      }
    _bss_start = .;
    .bss :      { *(.bss*)          KEEP(*(.bss))       }
    _bss_end = .;
    .cdi :      { *(.cdi*)          KEEP(*(.cdi))       }
    /DISCARD/ : { *(.eh_frame*);                       }
    . = ALIGN(4096);
    _kernel_end = .;
}
