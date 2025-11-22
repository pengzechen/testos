#include "t_types.h"
#include "t_spinlock.h"
#include "t_cfg.h"
#include "t_io.h"
#include "user.h"


static spinlock_irq_t print_lock = {0};

void
my_putchar(char c)
{
    // Fallback to direct write if not initialized
    volatile unsigned int *const UART0DR = (unsigned int *) UART_DR;
    *UART0DR                             = (unsigned int) c;
}

void
my_puts(const char *str)
{
    spin_lock_irqsave(&print_lock);
    while (*str) {
        my_putchar(*str++);
    }
    spin_unlock_irqrestore(&print_lock);
}

extern uint8_t __task_code_start[];
extern uint8_t __task_code_end[];

void
task_code_copy(void)
{
    size_t            total_size = __task_code_end - __task_code_start;
    size_t            num_tasks  = total_size / TEXT_SECTION_SIZE;
    volatile uint8_t *src        = __task_code_start;
    volatile uint8_t *dst        = (volatile uint8_t *) TASK_CODE_DST_BASE;

    for (size_t i = 0; i < num_tasks; ++i) {
        // 从源 .task_code section 复制每 task 4K
        for (size_t j = 0; j < TEXT_SECTION_SIZE; ++j)
            dst[i * TEXT_SECTION_SIZE + j] = src[i * TEXT_SECTION_SIZE + j];
    }
}

DECLARE_TASK(0)
{
    for (int i = 0; i < 10; ++i) {
        my_puts("Task 0 is running\n");
        sys_sleep(100);
    }
    sys_exit();
}

DECLARE_TASK(1)
{
    for (int i = 0; i < 20; ++i) {
        my_puts("Task 1 is running\n");
        sys_sleep(100);
    }
    sys_exit();
}

DECLARE_TASK(2)
{
    while (1) {
        my_puts("Task 2 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(3)
{
    while (1) {
        my_puts("Task 3 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(4)
{
    while (1) {
        my_puts("Task 4 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(5)
{
    while (1) {
        my_puts("Task 5 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(6)
{
    while (1) {
        my_puts("Task 6 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(7)
{
    while (1) {
        my_puts("Task 7 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(8)
{
    while (1) {
        my_puts("Task 8 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(9)
{
    while (1) {
        my_puts("Task 9 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(10)
{
    while (1) {
        my_puts("Task 10 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(11)
{
    while (1) {
        my_puts("Task 11 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(12)
{
    while (1) {
        my_puts("Task 12 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(13)
{
    while (1) {
        my_puts("Task 13 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(14)
{
    while (1) {
        my_puts("Task 14 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(15)
{
    while (1) {
        my_puts("Task 15 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(16)
{
    while (1) {
        my_puts("Task 16 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(17)
{
    while (1) {
        my_puts("Task 17 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(18)
{
    while (1) {
        my_puts("Task 18 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(19)
{
    while (1) {
        my_puts("Task 19 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASK(20)
{
    while (1) {
        my_puts("Task 20 is running\n");
        sys_sleep(1000);
    }
}

DECLARE_TASKINFO(0, 0x70000000, 0x50000000)
DECLARE_TASKINFO(1, 0x70001000, 0x50002000)
DECLARE_TASKINFO(2, 0x70002000, 0x50004000)
DECLARE_TASKINFO(3, 0x70003000, 0x50006000)
DECLARE_TASKINFO(4, 0x70004000, 0x50008000)
DECLARE_TASKINFO(5, 0x70005000, 0x5000A000)
DECLARE_TASKINFO(6, 0x70006000, 0x5000C000)
DECLARE_TASKINFO(7, 0x70007000, 0x5000E000)
DECLARE_TASKINFO(8, 0x70008000, 0x50010000)
DECLARE_TASKINFO(9, 0x70009000, 0x50012000)
DECLARE_TASKINFO(10, 0x7000A000, 0x50014000)
DECLARE_TASKINFO(11, 0x7000B000, 0x50016000)
DECLARE_TASKINFO(12, 0x7000C000, 0x50018000)
DECLARE_TASKINFO(13, 0x7000D000, 0x5001A000)
DECLARE_TASKINFO(14, 0x7000E000, 0x5001C000)
DECLARE_TASKINFO(15, 0x7000F000, 0x5001E000)
DECLARE_TASKINFO(16, 0x70010000, 0x50020000)
DECLARE_TASKINFO(17, 0x70011000, 0x50022000)
DECLARE_TASKINFO(18, 0x70012000, 0x50024000)
DECLARE_TASKINFO(19, 0x70013000, 0x50026000)
DECLARE_TASKINFO(20, 0x70014000, 0x50028000)