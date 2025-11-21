
#include "t_gicv2.h"
#include "t_io.h"
#include "t_exception.h"
#include "t_cfg.h"
#include "t_psci.h"
#include "t_spinlock.h"
#include "t_task.h"

extern void
_t_stack_top();
extern void
_t_stack_top_second();
void
t_second_entry();


void
start_secondary_cpus()
{
    logger("core 0 thread info addr: %llx\n",
           (struct thread_into *) (void *) (_t_stack_top - T_STACK_SIZE));

    for (int i = 1; i < T_SMP_NUM; i++) {
        logger("\n");
        int result =
            hvc_call(PSCI_0_2_FN64_CPU_ON,
                     i,
                     (uint64_t) (void *) t_second_entry,
                     (uint64_t) (_t_stack_top_second - T_STACK_SIZE * (i - 1)));
        if (result != 0) {
            logger_error("hvc_call failed!\n");
        }

        logger("core %d thread info addr: %llx\n",
               i,
               (void *) (_t_stack_top_second - T_STACK_SIZE * i));
    }
}

uint64_t el1_stack = (uint64_t) 0x50000000;
uint64_t el0_stack = (uint64_t) 0x60000000;

volatile int t_inited_cpu_num = 0;
spinlock_t   t_lock;
static int   move = 0;

void
test_task1()
{
    while (1) {
        logger_info("Task 1 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task2()
{
    while (1) {
        logger_info("Task 2 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task3()
{
    while (1) {
        logger_info("Task 3 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task4()
{
    while (1) {
        logger_info("Task 4 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task5()
{
    while (1) {
        logger_info("Task 5 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task6()
{
    while (1) {
        logger_info("Task 6 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task7()
{
    while (1) {
        logger_info("Task 7 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task8()
{
    while (1) {
        logger_info("Task 8 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task9()
{
    while (1) {
        logger_info("Task 9 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task10()
{
    while (1) {
        logger_info("Task 10 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task11()
{
    while (1) {
        logger_info("Task 11 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task12()
{
    while (1) {
        logger_info("Task 12 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task13()
{
    while (1) {
        logger_info("Task 13 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task14()
{
    while (1) {
        logger_info("Task 14 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task15()
{
    while (1) {
        logger_info("Task 15 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task16()
{
    while (1) {
        logger_info("Task 16 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task17()
{
    while (1) {
        logger_info("Task 17 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task18()
{
    while (1) {
        logger_info("Task 18 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task19()
{
    while (1) {
        logger_info("Task 19 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

void
test_task20()
{
    while (1) {
        logger_info("Task 20 is running\n");
        for (int i = 0; i < 0x105000; i++) {
            asm volatile("nop");
        }
    }
}

static entry_t test_tasks[] = {
    test_task1,  test_task2,  test_task3,  test_task4,  test_task5,
    test_task6,  test_task7,  test_task8,  test_task9,  test_task10,
    test_task11, test_task12, test_task13, test_task14, test_task15,
    test_task16, test_task17, test_task18, test_task19, test_task20};

void
t_main_entry()
{
    spin_lock(&t_lock);
    t_inited_cpu_num++;
    spin_unlock(&t_lock);

    logger_warn("core %d: t_main_entry called, inited cpu num: %d\n",
                t_get_current_cpu_id(),
                t_inited_cpu_num);
    while (t_inited_cpu_num != T_SMP_NUM) {
        for (int i = 0; i < 100000; i++)
            asm volatile("nop");
    }

    for (int i = 0; i < T_SMP_NUM + 1; i++) {
        struct tcb_t *task = create_task();
        init_task(
            (entry_t) test_tasks[t_get_current_cpu_id() * (T_SMP_NUM + 1) + i],
            task,
            (uint64_t) (void *) (el1_stack + task->task_id * T_STACK_SIZE),
            (uint64_t) (void *) (el0_stack + task->task_id * T_STACK_SIZE),
            0);
        enque_task(task);
    }

    extern void move_to_idle(struct tcb_t * idle_task);
    move_to_idle(get_idle_task());

    logger_error("should not reach here!\n");
}

// main.c
void
t_kernel_main(void)
{
    io_init();
    spinlock_init(&t_lock);

    // 在这里可以添加你的内核代码
    logger_info("hello world, os version: %s\n", OS_VERSION);
    // t_run_printf_tests();

    // 初始化中断系统以支持UART中断
    gic_init();

    timer_init();

    // 启用中断
    enable_interrupts();

    logger_info("Interrupt system initialized for UART\n");

    init_scheduler();

    start_secondary_cpus();

    t_main_entry();
    // can't reach here !
}


void
t_second_kernel_main()
{
    logger_info("starting core: %d ...\n", t_get_current_cpu_id());

    gicc_init();

    timer_init();

    enable_interrupts();

    logger_info("core %d starting is done.\n\n", t_get_current_cpu_id());

    t_main_entry();
    // can't reach here !
}