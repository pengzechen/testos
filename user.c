#include "t_types.h"
#include "t_spinlock.h"
#include "t_cfg.h"
#include "t_io.h"
#include "t_mutex.h"
#include "user.h"

static spinlock_irq_t print_lock = {0};

// 全局共享变量 - 对比测试
static mutex_t test_mutex          = {0};
static int     protected_counter   = 0;  // 受mutex保护
static int     unprotected_counter = 0;  // 不受保护
static int     protected_complex   = 0;  // 受保护的复杂值
static int     unprotected_complex = 0;  // 不受保护的复杂值

void
my_putchar(char c)
{
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
mutex_test_init(void)
{
    mutex_init(&test_mutex);
    protected_counter   = 0;
    unprotected_counter = 0;
    protected_complex   = 0;
    unprotected_complex = 0;
}

void
task_code_copy(void)
{
    size_t            total_size = __task_code_end - __task_code_start;
    size_t            num_tasks  = total_size / TEXT_SECTION_SIZE;
    volatile uint8_t *src        = __task_code_start;
    volatile uint8_t *dst        = (volatile uint8_t *) TASK_CODE_DST_BASE;
    for (size_t i = 0; i < num_tasks; ++i) {
        for (size_t j = 0; j < TEXT_SECTION_SIZE; ++j)
            dst[i * TEXT_SECTION_SIZE + j] = src[i * TEXT_SECTION_SIZE + j];
    }
}

DECLARE_TASK(0)
{
    my_puts("[Task 0] Starting\n");
    for (int i = 0; i < 100; ++i) {
        // 受保护操作
        sys_mutex_lock(&test_mutex);
        int old_p = protected_counter;
        sys_sleep(5);
        protected_counter = old_p + 1;
        protected_complex += i;
        sys_sleep(5);
        sys_mutex_unlock(&test_mutex);

        // 未保护操作 - 读取
        int old_u = unprotected_counter;
        int old_c = unprotected_complex;
        sys_sleep(20);  // 增加延迟让其他任务有机会读取相同的值
        // 写回
        unprotected_counter = old_u + 1;
        sys_sleep(20);  // 增加延迟让其他任务有机会覆盖
        unprotected_complex = old_c + i;
    }
    my_puts("[Task 0] Done\n");
    sys_exit();
}

DECLARE_TASK(1)
{
    my_puts("[Task 1] Starting\n");
    for (int i = 0; i < 100; ++i) {
        // 受保护操作
        sys_mutex_lock(&test_mutex);
        int old_p = protected_counter;
        sys_sleep(5);
        protected_counter = old_p + 1;
        protected_complex += (i * 2);
        sys_sleep(5);
        sys_mutex_unlock(&test_mutex);

        // 未保护操作 - 读取
        int old_u = unprotected_counter;
        int old_c = unprotected_complex;
        sys_sleep(20);  // 增加延迟让其他任务有机会读取相同的值
        // 写回
        unprotected_counter = old_u + 1;
        sys_sleep(20);  // 增加延迟让其他任务有机会覆盖
        unprotected_complex = old_c + (i * 2);
    }
    my_puts("[Task 1] Done\n");
    sys_exit();
}

DECLARE_TASK(2)
{
    my_puts("[Task 2] Starting\n");
    for (int i = 0; i < 100; ++i) {
        // 受保护操作
        sys_mutex_lock(&test_mutex);
        int old_p = protected_counter;
        sys_sleep(5);
        protected_counter = old_p + 1;
        protected_complex += (i * 3);
        sys_sleep(5);
        sys_mutex_unlock(&test_mutex);

        // 未保护操作 - 读取
        int old_u = unprotected_counter;
        int old_c = unprotected_complex;
        sys_sleep(20);  // 增加延迟让其他任务有机会读取相同的值
        // 写回
        unprotected_counter = old_u + 1;
        sys_sleep(20);  // 增加延迟让其他任务有机会覆盖
        unprotected_complex = old_c + (i * 3);
    }

    sys_sleep(500);

    my_puts("\n========== MUTEX vs NO-MUTEX COMPARISON ==========\n");
    sys_mutex_lock(&test_mutex);
    int fp = protected_counter;
    int fc = protected_complex;
    sys_mutex_unlock(&test_mutex);
    int fu  = unprotected_counter;
    int fuc = unprotected_complex;

    my_puts("\nWITH MUTEX:\n");
    my_puts("  Counter: ");
    char b1[20];
    int  idx = 0;
    if (fp == 0) {
        b1[idx++] = '0';
    } else {
        int v = fp, d = 0;
        while (v > 0) {
            v /= 10;
            d++;
        }
        for (int j = d - 1; j >= 0; j--) {
            b1[j] = '0' + (fp % 10);
            fp /= 10;
        }
        idx = d;
    }
    b1[idx++] = '\n';
    b1[idx]   = '\0';
    my_puts(b1);

    my_puts("  Complex: ");
    char b2[20];
    idx = 0;
    if (fc == 0) {
        b2[idx++] = '0';
    } else {
        int v = fc, d = 0;
        while (v > 0) {
            v /= 10;
            d++;
        }
        for (int j = d - 1; j >= 0; j--) {
            b2[j] = '0' + (fc % 10);
            fc /= 10;
        }
        idx = d;
    }
    b2[idx++] = '\n';
    b2[idx]   = '\0';
    my_puts(b2);

    my_puts("\nWITHOUT MUTEX:\n");
    my_puts("  Counter: ");
    char b3[20];
    idx = 0;
    if (fu == 0) {
        b3[idx++] = '0';
    } else {
        int v = fu, d = 0;
        while (v > 0) {
            v /= 10;
            d++;
        }
        for (int j = d - 1; j >= 0; j--) {
            b3[j] = '0' + (fu % 10);
            fu /= 10;
        }
        idx = d;
    }
    b3[idx++] = '\n';
    b3[idx]   = '\0';
    my_puts(b3);

    my_puts("  Complex: ");
    char b4[20];
    idx = 0;
    if (fuc == 0) {
        b4[idx++] = '0';
    } else {
        int v = fuc, d = 0;
        while (v > 0) {
            v /= 10;
            d++;
        }
        for (int j = d - 1; j >= 0; j--) {
            b4[j] = '0' + (fuc % 10);
            fuc /= 10;
        }
        idx = d;
    }
    b4[idx++] = '\n';
    b4[idx]   = '\0';
    my_puts(b4);

    sys_mutex_lock(&test_mutex);
    fp = protected_counter;
    fc = protected_complex;
    sys_mutex_unlock(&test_mutex);
    fu  = unprotected_counter;
    fuc = unprotected_complex;

    my_puts("\nVERIFICATION:\n");
    if (fp == 60 && fc == 1140) {
        my_puts("  MUTEX PROTECTED: CORRECT!\n");
    } else {
        my_puts("  MUTEX PROTECTED: WRONG!\n");
    }

    if (fu != 60 || fuc != 1140) {
        my_puts("  UNPROTECTED: WRONG! Data race detected!\n");
        my_puts("  >>> Mutex IS necessary! <<<\n");
    } else {
        my_puts("  UNPROTECTED: Correct (got lucky)\n");
    }
    my_puts("==================================================\n\n");
    sys_exit();
}

DECLARE_TASK(3)
{
    while (1) {
        my_puts("Task 3\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(4)
{
    while (1) {
        my_puts("Task 4\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(5)
{
    while (1) {
        my_puts("Task 5\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(6)
{
    while (1) {
        my_puts("Task 6\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(7)
{
    while (1) {
        my_puts("Task 7\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(8)
{
    while (1) {
        my_puts("Task 8\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(9)
{
    while (1) {
        my_puts("Task 9\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(10)
{
    while (1) {
        my_puts("Task 10\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(11)
{
    while (1) {
        my_puts("Task 11\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(12)
{
    while (1) {
        my_puts("Task 12\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(13)
{
    while (1) {
        my_puts("Task 13\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(14)
{
    while (1) {
        my_puts("Task 14\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(15)
{
    while (1) {
        my_puts("Task 15\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(16)
{
    while (1) {
        my_puts("Task 16\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(17)
{
    while (1) {
        my_puts("Task 17\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(18)
{
    while (1) {
        my_puts("Task 18\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(19)
{
    while (1) {
        my_puts("Task 19\n");
        sys_sleep(1000);
    }
}
DECLARE_TASK(20)
{
    while (1) {
        my_puts("Task 20\n");
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
