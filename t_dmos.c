
#include "t_gicv2.h"
#include "t_io.h"
#include "t_exception.h"
#include "t_cfg.h"
#include "t_psci.h"
#include "t_spinlock.h"

extern void _t_stack_top();
extern void _t_stack_top_second();

// 前置声明
void t_second_entry();


void start_secondary_cpus()
{
    logger("core 0 thread info addr: %llx\n", (struct thread_into *)(void *)(_t_stack_top - T_STACK_SIZE));

    for (int i = 1; i < T_SMP_NUM; i++)
    {
        logger("\n");
        int result = hvc_call(PSCI_0_2_FN64_CPU_ON, i, (uint64_t)(void *)t_second_entry, (uint64_t)(_t_stack_top_second - T_STACK_SIZE * (i - 1)));
        if (result != 0)
        {
            logger("hvc_call failed!\n");
        }
        
        // 做一点休眠 保证第二个核 初始化完成
        for (int j = 0; j < 0xff; j++)
            for (int k = 0; k < 0xfffff; k++)
                ;

        logger("core %d thread info addr: %llx\n", i, (void *)(_t_stack_top_second - T_STACK_SIZE * i));
    }
}


volatile int t_inited_cpu_num = 0;
spinlock_t t_lock;
static int move = 0;

void t_main_entry()
{

    spin_lock(&t_lock);
    t_inited_cpu_num++;
    spin_unlock(&t_lock);

    logger_warn("core %d: t_main_entry called, inited cpu num: %d\n", t_get_current_cpu_id(), t_inited_cpu_num);
    while (t_inited_cpu_num != T_SMP_NUM) {
        // logger_info("core %d: waiting for other cpu to init...\n", t_get_current_cpu_id());
        // 这里不能使用wfi
        // __asm__ volatile("wfi" : : : "memory");
        for (int i = 0; i < 100000; i++) asm volatile("nop");
    }
        
    while(1) {
        // Test UART receive functionality
        if (uart_rx_available()) {
            char c;
            while (uart_getchar_nb(&c)) {
                logger_info("(cpu: %d) Received char: '%c' (0x%02x)\n",
                           t_get_current_cpu_id(), c, (unsigned char)c);
            }
        }

        // Small delay to avoid busy waiting
        for (int k = 0; k < 0x10000; k++) {
            asm volatile("nop");
        }
    }
    
}

// main.c
void t_kernel_main(void)
{
    io_init();
    spinlock_init(&t_lock);

    // 在这里可以添加你的内核代码
    logger_info("hello world, os version: %s\n", OS_VERSION);
    // t_run_printf_tests();

    // 初始化中断系统以支持UART中断
    exception_init();
    gic_init();

    // 启用中断
    enable_interrupts();

    logger_info("Interrupt system initialized for UART\n");

    start_secondary_cpus();
    
    t_main_entry();
    // can't reach here !
}


void t_second_kernel_main()
{
    logger_info("starting core: %d ...\n", t_get_current_cpu_id());

    for (int k = 0; k < 0xffff; k++)
            ;

    logger_info("core %d starting is done.\n\n", t_get_current_cpu_id());

    t_main_entry();
    // can't reach here !
}