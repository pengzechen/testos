#include "t_mutex.h"
#include "t_task.h"
#include "t_io.h"
#include "t_spinlock.h"
#include "t_string.h"

void
mutex_init(mutex_t *mutex)
{
    if (!mutex) {
        logger_error("mutex_init: NULL mutex pointer\n");
        return;
    }

    spinlock_init(&mutex->lock);
    mutex->owner  = NULL;
    mutex->locked = 0;
    list_init(&mutex->wait_queue);
}

void
mutex_lock(mutex_t *mutex)
{
    if (!mutex) {
        logger_error("mutex_lock: NULL mutex pointer\n");
        return;
    }

    struct tcb_t *current = get_current_task();

    // 使用自旋锁保护 mutex 内部状态
    spin_lock(&mutex->lock);

    // 如果锁未被占用，直接获取
    if (mutex->locked == 0) {
        mutex->locked = 1;
        mutex->owner  = current;
        spin_unlock(&mutex->lock);
        return;
    }

    // 检查是否重复加锁（同一任务尝试再次获取已持有的锁）
    if (mutex->owner == current) {
        spin_unlock(&mutex->lock);
        logger_warn("mutex_lock: Task %d attempting to lock mutex it already owns\n",
                    current->task_id);
        return;
    }

    // 锁已被其他任务占用，将当前任务加入等待队列
    current->state = TASK_STATE_WAIT_IRQ;  // 使用 WAIT_IRQ 状态表示等待锁
    list_insert_last(&mutex->wait_queue, &current->run_node);

    spin_unlock(&mutex->lock);

    // 主动让出 CPU，切换到下一个任务
    // 注意：当前任务已在等待队列中，不应再进入 ready 队列
    struct tcb_t *next_task = deque_task();
    set_current_task(next_task);
    switch_context(current, next_task);

    // 当任务被唤醒后，会从这里继续执行
    // 此时锁已经被转移给当前任务
}

int
mutex_trylock(mutex_t *mutex)
{
    if (!mutex) {
        logger_error("mutex_trylock: NULL mutex pointer\n");
        return 0;
    }

    struct tcb_t *current = get_current_task();

    spin_lock(&mutex->lock);

    // 如果锁未被占用，直接获取
    if (mutex->locked == 0) {
        mutex->locked = 1;
        mutex->owner  = current;
        spin_unlock(&mutex->lock);
        return 1;
    }

    // 锁已被占用，返回失败
    spin_unlock(&mutex->lock);
    return 0;
}

void
mutex_unlock(mutex_t *mutex)
{
    if (!mutex) {
        logger_error("mutex_unlock: NULL mutex pointer\n");
        return;
    }

    struct tcb_t *current = get_current_task();

    spin_lock(&mutex->lock);

    // 检查是否是锁的持有者
    if (mutex->owner != current) {
        spin_unlock(&mutex->lock);
        logger_warn("mutex_unlock: Task %d attempting to unlock mutex owned by Task %d\n",
                    current->task_id,
                    mutex->owner ? mutex->owner->task_id : -1);
        return;
    }

    // 检查等待队列是否有任务
    if (list_count(&mutex->wait_queue) > 0) {
        // 唤醒等待队列中的第一个任务
        list_node_t  *node         = list_delete_first(&mutex->wait_queue);
        struct tcb_t *wakeup_task  = list_node_parent(node, struct tcb_t, run_node);

        // 将锁转移给被唤醒的任务
        mutex->owner = wakeup_task;
        // mutex->locked 保持为 1

        // 将被唤醒的任务加入到它所属 CPU 的 ready 队列
        // 注意：不要在这里设置状态，让 enque_task_to_cpu 来设置
        extern void enque_task_to_cpu(struct tcb_t *task, int cpu_id);
        enque_task_to_cpu(wakeup_task, wakeup_task->affinity);

        spin_unlock(&mutex->lock);
    } else {
        // 等待队列为空，释放锁
        mutex->locked = 0;
        mutex->owner  = NULL;
        spin_unlock(&mutex->lock);
    }
}

int
mutex_is_locked_by_current(mutex_t *mutex)
{
    if (!mutex) {
        return 0;
    }

    struct tcb_t *current = get_current_task();

    spin_lock(&mutex->lock);
    int result = (mutex->owner == current && mutex->locked == 1);
    spin_unlock(&mutex->lock);

    return result;
}
