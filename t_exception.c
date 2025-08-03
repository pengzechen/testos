
#include "t_types.h"
#include "t_exception.h"
#include "t_gicv2.h"
#include "t_io.h"
#include "t_cfg.h"

irq_handler_t g_handler_vec[512] = {0};
uint64_t print_flag = 0;

void irq_install(int vector, void (*h)(uint64_t *))
{
    g_handler_vec[vector] = h;
}

void handle_sync_exception(uint64_t *stack_pointer)
{
    trap_frame_t *el1_ctx = (trap_frame_t *)stack_pointer;

    int el1_esr = read_esr_el1();

    int ec = ((el1_esr >> 26) & 0b111111);

    logger("el1 esr: %x\n", el1_esr);
    logger("ec: %x\n", ec);

    logger("This is handle_sync_exception: \n");
    for (int i = 0; i < 31; i++)
    {
        uint64_t value = el1_ctx->r[i];
        logger("General-purpose register: %d, value: %x\n", i, value);
    }

    uint64_t elr_el1_value = el1_ctx->elr;
    uint64_t usp_value = el1_ctx->usp;
    uint64_t spsr_value = el1_ctx->spsr;

    logger("usp: %x, elr: %x, spsr: %x\n", usp_value, elr_el1_value, spsr_value);

    while (1)
        ;
}

void handle_irq_exception(uint64_t *stack_pointer)
{
    trap_frame_t *el1_ctx = (trap_frame_t *)stack_pointer;

    uint64_t x1_value = el1_ctx->r[1];
    uint64_t sp_el0_value = el1_ctx->usp;

    int iar = gic_read_iar();
    int vector = gic_iar_irqnr(iar);
    gic_write_eoir(iar);
    gic_write_dir(iar);
    
    g_handler_vec[vector]((uint64_t *)el1_ctx); // arg not use

}

void invalid_exception(uint64_t *stack_pointer, uint64_t kind, uint64_t source)
{
    trap_frame_t *el1_ctx = (trap_frame_t *)stack_pointer;

    uint64_t x2_value = el1_ctx->r[2];
}

void cntp_handler(uint64_t * one)
{
    asm volatile("msr cntp_tval_el0, %0" : : "r"(6250000));
    unsigned long mpidr;
    if (print_flag++ % 10 == 0)
    {
        __asm__ __volatile__("mrs %0, mpidr_el1" : "=r"(mpidr));
        logger("core: %d: irq %d. times: %d\n", (0xff & mpidr), TIMER, print_flag);
        if (print_flag >= 20000) {
            gic_disable_int(TIMER);
        }
    }
}

// SGI中断号定义（可选1~15，这里用1）
#define SGI_INT_ID 1

extern volatile int sgi_chain_stage;
extern volatile int sgi_chain_done;
extern void send_sgi_chain();

void sgi_handler(uint64_t *stack_pointer) {
    int cid = t_get_current_cpu_id();
    logger("SGI interrupt handled on core: %d\n", cid);
    if (sgi_chain_stage < T_SMP_NUM) {
        if (cid == sgi_chain_stage) {
            sgi_chain_stage++;
            send_sgi_chain();
        }
    } else if (cid == 0 && !sgi_chain_done) {
        // 最后一个核发回首核，结束
        sgi_chain_done = 1;
    }
}

void exception_init()
{
    irq_install(TIMER, cntp_handler);
    
    irq_install(SGI_INT_ID, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+1, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+2, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+3, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+4, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+5, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+6, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+7, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+8, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+9, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+10, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+11, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+12, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+13, sgi_handler); // 安装SGI中断处理器
    irq_install(SGI_INT_ID+14, sgi_handler); // 安装SGI中断处理器
}