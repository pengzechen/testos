// startup.S
.section .text
.global _start

.global _t_stack_top
.global _t_stack_top_second

.global t_second_entry

_start:

    msr daifset, #2   // 关闭所有中断

    adrp    x0, exception_vector_base
    add     x0, x0, :lo12:exception_vector_base
    // ldr x0, =exception_vector_base
    msr     vbar_el1, x0
    dsb     sy      
    isb        

    ldr x0, =_t_stack_top
    mov sp, x0

    // 调用 C 语言的 main 函数
    adrp x0, t_kernel_main
    add  x0, x0, :lo12:t_kernel_main
    blr  x0

    // 死循环，防止返回
    1:  b 1b


t_second_entry:
    // 查看当前异常等级
    //mrs x0, CurrentEL          // Read current exception level
    //lsr x0, x0, #2             // 右移两位
    
    // 设置栈指针
    mov sp, x0

    msr daifset, #2   // 关闭所有中断

    adrp    x0, exception_vector_base
    add     x0, x0, :lo12:exception_vector_base
    // ldr x0, =exception_vector_base
    msr     vbar_el1, x0
    dsb     sy     
    isb             

    adrp x0, t_second_kernel_main
    add  x0, x0, :lo12:t_second_kernel_main
    blr  x0

    b .

// 栈空间
.section .bss
.align 14

_t_stack_bottom:
    .skip 0x4000  // 4KB 的栈空间
_t_stack_top:


.align 14
_t_stack_bottom_second:
    .skip 0x4000 * 4 // 16KB 的栈空间
_t_stack_top_second: