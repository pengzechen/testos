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

    . = 0x80000000;
    __task_code_start = .;
    .task_code : ALIGN(4096)
    {
        KEEP(*(.task_code))
    }
    __task_code_end = .;

    .taskinfo_section 0x90000000 :  /* 可以选一个mapper区 */
    {
        KEEP(*(.taskinfo_section))
    }
}
