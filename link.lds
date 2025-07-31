/* link.ld */
ENTRY(_start)

SECTIONS
{
    /* 设置内核加载地址  */
    . = __LOAD_ADDR__;

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }
}
