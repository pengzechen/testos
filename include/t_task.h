#ifndef T_TASK_H
#define T_TASK_H

#include "t_types.h"
#include "t_list.h"

typedef enum _task_state_t
{
    TASK_STATE_CREATE = 1,  // 刚分配完 TCB，还没进入任何队列
    TASK_STATE_READY,       // 已进入 ready 队列，等待调度
    TASK_STATE_RUNNING,
    TASK_STATE_WAITING,   // 睡眠状态（sleep tick 到期后可转 READY）
    TASK_STATE_WAIT_IRQ,  // 等待中断
    TASK_STATE_EXIT,
} task_state_t;

typedef struct _contex_t
{
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t x29;        // Stack Frame Pointer
    uint64_t x30;        // Link register (the address to return)
    uint64_t tpidr_elx;  // "Thread ID" Register
    uint64_t sp_elx;
} contex_t;

struct tcb_t
{
    contex_t ctx;

    uint64_t sp;  // 记录栈地址，将来释放使用

    task_state_t state;
    uint32_t     affinity;

    int32_t task_id;  // 任务ID
    int32_t remaining_ticks;

    int32_t  sleep_ticks;
    uint32_t reserved;


    list_node_t run_node;    // 运行相关结点
    list_node_t sleep_node;  // 睡眠队列 not used
    list_node_t all_node;    // 所有队列结点
};

struct scheduler_t
{
    list_t ready_queue;
    list_t sleep_queue;

    struct tcb_t idle_task;
};

extern void switch_context(struct tcb_t *, struct tcb_t *);
typedef void (*entry_t)(void);


void
init_scheduler(void);


struct tcb_t *
create_task(void);

void
init_task(entry_t       entry,
          struct tcb_t *task,
          uint64_t      el1_stack_addr,
          uint64_t      el0_stack_addr,
          uint32_t      affinity);

void
enque_task(struct tcb_t *task);

void
enque_task_to_cpu(struct tcb_t *task, int cpu_id);

struct tcb_t *
get_idle_task(void);

struct tcb_t *
get_current_task(void);

void
set_current_task(struct tcb_t *task);

struct tcb_t *
deque_task(void);

void
schedule(void);

void
handle_timer_tick(void);


// ================= 下面是系统调用接口声明 =================

void
sys_exit(void);

void
sys_sleep(uint32_t ticks);

void
sys_yield(void);

// Mutex 系统调用
struct _mutex_t;  // 前向声明
void
sys_mutex_lock(struct _mutex_t *mutex);

void
sys_mutex_unlock(struct _mutex_t *mutex);

int
sys_mutex_trylock(struct _mutex_t *mutex);

#endif /* T_TASK_H */