
#include "t_task.h"
#include "t_cfg.h"
#include "t_exception.h"
#include "t_list.h"
#include "t_string.h"
#include "t_io.h"

#define IDLE_STACK_SIZE     0x1000UL
#define TASK_EL1_STACK_SIZE 0x2000UL
#define TASK_EL0_STACK_SIZE 0x2000UL

#define TASK_MAX_NUM 64
static struct tcb_t task_poll[TASK_MAX_NUM];
static int32_t      task_count = 0;
static uint8_t      idle_task_stack[T_SMP_NUM][IDLE_STACK_SIZE]
    __attribute__((aligned(4096)));


// Per cpu
static struct tcb_t      *current_task[T_SMP_NUM] = {0};
static struct scheduler_t sched[T_SMP_NUM]        = {0};

extern void
el0_task_entry();

void
idle_task_entry(void)
{
    logger_info("enter idle task\n");
    while (1) {
        asm volatile("wfi");
    }
}

void
init_scheduler(void)
{
    for (int i = 0; i < T_SMP_NUM; i++) {
        list_init(&sched[i].ready_queue);
        list_init(&sched[i].sleep_queue);

        // 初始化 idle 任务
        struct tcb_t *idle_task = &sched[i].idle_task;
        memset(idle_task, 0, sizeof(struct tcb_t));

        idle_task->task_id = -(i + 1);  // idle 任务 ID 为 -1
        idle_task->state   = TASK_STATE_READY;
        idle_task->ctx.x30 = (uint64_t) el0_task_entry;

        uint64_t idle_task_stack_addr =
            (uint64_t) (void *) &idle_task_stack[i][IDLE_STACK_SIZE];
        uint64_t idle_sp_position = idle_task_stack_addr - sizeof(trap_frame_t);
        idle_task->ctx.x29        = idle_sp_position;
        idle_task->ctx.sp_elx     = idle_sp_position;
        idle_task->sp             = idle_task_stack_addr;

        // 在栈上设置初始的 trap_frame_t
        trap_frame_t *tf = (trap_frame_t *) idle_sp_position;
        memset(tf, 0, sizeof(trap_frame_t));
        tf->elr = (uint64_t) idle_task_entry;  // 设置任务入口地址
        tf->spsr = 0b101;  // 设置初始的 SPSR（根据需要设置标志位）
        tf->usp = 0;       // 设置用户栈指针
        memcpy((void *) idle_sp_position, tf, sizeof(trap_frame_t));

        current_task[i] = idle_task;
    }
}

struct tcb_t *
get_idle_task(void)
{
    int cpu_id = t_get_current_cpu_id();
    return &sched[cpu_id].idle_task;
}

struct tcb_t *
get_current_task(void)
{
    int cpu_id = t_get_current_cpu_id();
    return current_task[cpu_id];
}

void
set_current_task(struct tcb_t *task)
{
    int cpu_id           = t_get_current_cpu_id();
    current_task[cpu_id] = task;
}

struct tcb_t *
create_task(void)
{
    if (task_count >= TASK_MAX_NUM) {
        return NULL;
    }

    struct tcb_t *new_task = &task_poll[task_count++];
    memset(new_task, 0, sizeof(struct tcb_t));
    new_task->task_id = task_count - 1;

    return new_task;
}

void
init_task(entry_t       entry,
          struct tcb_t *task,
          uint64_t      el1_stack_addr,
          uint64_t      el0_stack_addr,
          uint32_t      affinity)
{
    task->sp       = el1_stack_addr;
    task->affinity = affinity;

    memset(&task->ctx, 0, sizeof(contex_t));
    uint64_t el1_sp_position =
        el1_stack_addr + TASK_EL1_STACK_SIZE - sizeof(trap_frame_t);
    task->ctx.x29    = el1_sp_position;
    task->ctx.sp_elx = el1_sp_position;

    task->ctx.x30 = (uint64_t) el0_task_entry;  // 设置返回地址为任务入口地址


    // 在栈上设置初始的 trap_frame_t
    trap_frame_t *tf = (trap_frame_t *) el1_sp_position;
    memset(tf, 0, sizeof(trap_frame_t));
    tf->elr  = (uint64_t) entry;  // 设置任务入口地址
    tf->spsr = 0b101;  // 设置初始的 SPSR（根据需要设置标志位）
    tf->usp  = el0_stack_addr + TASK_EL0_STACK_SIZE;  // 设置用户栈指针
    memcpy((void *) el1_sp_position, tf, sizeof(trap_frame_t));

    task->state = TASK_STATE_CREATE;
}

// Per cpu
// 入队之后设置为 READY 状态
void
enque_task(struct tcb_t *task)
{
    int cpu_id = t_get_current_cpu_id();
    // logger_info("Enque task %d to cpu %d ready queue\n", task->task_id, cpu_id);
    list_insert_last(&sched[cpu_id].ready_queue, &task->run_node);
    task->state = TASK_STATE_READY;
}

// Per cpu
// 出队之后设置为 RUNNING 状态
struct tcb_t *
deque_task(void)
{
    int cpu_id = t_get_current_cpu_id();
    if (list_count(&sched[cpu_id].ready_queue) == 0) {
        return &sched[cpu_id].idle_task;
    }

    list_node_t  *node = list_delete_first(&sched[cpu_id].ready_queue);
    struct tcb_t *task = list_node_parent(node, struct tcb_t, run_node);
    task->state        = TASK_STATE_RUNNING;
    // logger_info("Deque task %d from cpu %d ready queue\n",
    //             task->task_id,
    //             cpu_id);
    return task;
}

void
yield_cpu(void)
{
    struct tcb_t *current = get_current_task();
    if (current != &sched[t_get_current_cpu_id()].idle_task)
        enque_task(current);

    struct tcb_t *next_task = deque_task();
    set_current_task(next_task);
    switch_context(current, next_task);
}

void
switch_to_task(struct tcb_t *next_task)
{
    struct tcb_t *current = get_current_task();
    set_current_task(next_task);
    switch_context(current, next_task);
}

void
schedule(void)
{
    struct tcb_t *current   = get_current_task();
    struct tcb_t *next_task = deque_task();
    
    if (current != &sched[t_get_current_cpu_id()].idle_task) {
        enque_task(current);
    }

    logger_info("Scheduling from task %d to task %d\n",
                current->task_id,
                next_task->task_id);
    logger_info("Ready queue count: %d\n",
                list_count(&sched[t_get_current_cpu_id()].ready_queue));

    if (current->task_id != next_task->task_id) {
        set_current_task(next_task);
        switch_context(current, next_task);
    }
}