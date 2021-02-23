OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(_start32)

SECTIONS
{
    . = 0x100000;
    _boot_start = .;
    .mboot :    { *(.text.mboot*)    KEEP(*(.text.mboot))     }
    .text :     { *(.text*)     KEEP(*(.text))      }
    .rodata :   { *(.rodata*)   KEEP(*(.rodata))    }
    .data :     { *(.data*)     KEEP(*(.data))      }
    _bss_start = .;
    .bss :      { *(.bss*)      KEEP(*(.bss))       }
    _bss_end = .;
    .cdi :      { *(.cdi*)      KEEP(*(.cdi))       }
    /DISCARD/ : { *(.eh_frame*);                    }
    _boot_end = .;
}
