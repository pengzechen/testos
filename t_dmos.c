
#include "t_gicv2.h"
#include "t_io.h"
#include "t_exception.h"
#include "t_cfg.h"
#include "t_psci.h"
#include "t_spinlock.h"
#include "t_task.h"
#include "user.h"

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

volatile int t_inited_cpu_num = 0;
spinlock_t   t_lock;

const task_info_t *infos = (const task_info_t *) 0x90000000;

#define TEST_TASK_NUM (T_SMP_NUM + 1)

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

    int       cpu_id        = t_get_current_cpu_id();
    const int TASKS_PER_CPU = TEST_TASK_NUM;

    for (int i = 0; i < TASKS_PER_CPU; i++) {
        int           info_idx = cpu_id * TASKS_PER_CPU + i;
        struct tcb_t *task     = create_task();

        entry_t  entry     = infos[info_idx].entry;
        uint64_t el0_stack = infos[info_idx].el0_stack_addr;
        uint64_t el1_stack = infos[info_idx].el1_stack_addr;

        init_task(entry, task, el1_stack, el0_stack, 0);
        enque_task(task);  // 加入本核 ready 队列
    }

    extern void move_to_idle(struct tcb_t * idle_task);
    asm volatile("dmb ish" ::: "memory");
    move_to_idle(get_idle_task());

    logger_error("should not reach here!\n");
}

// main.c
void
t_kernel_main(void)
{
    // io_init();
    spinlock_init(&t_lock);

    // 在这里可以添加你的内核代码
    logger_info("hello world, os version: %s\n", OS_VERSION);
    // t_run_printf_tests();

    // 初始化中断系统以支持UART中断
    gic_init();

    timer_init();

    // 启用中断
    // enable_interrupts();

    logger_info("Interrupt system initialized for UART\n");

    init_scheduler();

    task_code_copy();

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

    // enable_interrupts();

    logger_info("core %d starting is done.\n\n", t_get_current_cpu_id());

    t_main_entry();
    // can't reach here !
}