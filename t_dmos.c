
#include "t_gicv2.h"
#include "t_io.h"
#include "t_exception.h"
#include "t_cfg.h"
#include "t_psci.h"
#include "t_spinlock.h"

void test_mem_maped()
{
    volatile unsigned int *const MEM_ADDR = (unsigned int *)0x70000000;
    *MEM_ADDR = 0x7777;
}

void test_mem_no_maped()
{
    volatile unsigned int *const MEM_ADDR = (unsigned int *)0x80000000;
    *MEM_ADDR = 0x8888;
}

void test_mem_no_maped2()
{
    volatile unsigned int *const MEM_ADDR = (unsigned int *)0x50000000;
    *MEM_ADDR = 0x8888;
}




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

volatile int sgi_chain_stage = 0;
volatile int sgi_chain_done = 0;

void send_sgi_chain() {
    int cid = t_get_current_cpu_id();
    int next = (cid + 1) % T_SMP_NUM;
    int sgi_id = (cid % 15) + 1; // SGI号1~15循环
    if (!sgi_chain_done) {
        logger_info("Send SGI %d to core %d\n", sgi_id, next);
        gic_ipi_send_single(sgi_id, next);
    }
}

void t_main_entry()
{
    spin_lock(&t_lock);
    t_inited_cpu_num++;
    spin_unlock(&t_lock);

    logger_warn("t_main_entry called, inited cpu num: %d\n", t_inited_cpu_num);
    while (t_inited_cpu_num != T_SMP_NUM) {
        for (int i = 0; i < 100000; i++) asm volatile("nop");
    }

    enable_interrupts();

    // 首核发起链式SGI
    if (t_get_current_cpu_id() == 0) {
        sgi_chain_stage = 1;
        send_sgi_chain();
    }

    // 等待链式SGI完成
    while (!sgi_chain_done) {
        for (int i = 0; i < 100000; i++) asm volatile("nop");
    }

    asm volatile("msr cntv_ctl_el0, %0" : : "r"(1));
    // logger_info("SGI chain test finished!\n");
}

// main.c
void t_kernel_main(void)
{
    io_init();
    spinlock_init(&t_lock);

    // 在这里可以添加你的内核代码
    logger_info("hello world, os version: %s\n", OS_VERSION);
    // t_run_printf_tests();

    // 中断控制器先不考虑
    exception_init();
    gic_init();
    
    asm volatile("msr cntv_tval_el0, %0" : : "r"(100000));
    // asm volatile("msr cntv_ctl_el0, %0" : : "r"(1));
    // gic_enable_int(TIMER);

    start_secondary_cpus();
    
    t_main_entry();
    // can't reach here !
}


void t_second_kernel_main()
{
    logger_info("starting core:...\n");

    for (int k = 0; k < 0xffff; k++)
            ;
    gicc_init();

    asm volatile("msr cntv_tval_el0, %0" : : "r"(100000));
    // asm volatile("msr cntv_ctl_el0, %0" : : "r"(1));
    // gic_enable_int(TIMER);

    logger_info("starting is done.\n\n");

    t_main_entry();
    // can't reach here !
}