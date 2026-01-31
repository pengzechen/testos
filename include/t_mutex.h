#ifndef T_MUTEX_H
#define T_MUTEX_H

#include "t_types.h"
#include "t_list.h"
#include "t_spinlock.h"

// 前向声明
struct tcb_t;

typedef struct _mutex_t
{
    spinlock_t       lock;          // 保护 mutex 内部状态的自旋锁
    struct tcb_t    *owner;         // 当前持有锁的任务
    list_t           wait_queue;     // 等待此锁的任务队列
    uint32_t         locked;         // 锁状态：0=未锁定, 1=已锁定
} mutex_t;

/**
 * 初始化互斥锁
 */
void
mutex_init(mutex_t *mutex);

/**
 * 获取互斥锁
 * 如果锁已被占用，当前任务将被阻塞并加入等待队列
 */
void
mutex_lock(mutex_t *mutex);

/**
 * 尝试获取互斥锁（非阻塞）
 * @return 1 表示成功获取锁，0 表示锁已被占用
 */
int
mutex_trylock(mutex_t *mutex);

/**
 * 释放互斥锁
 * 唤醒等待队列中的第一个任务
 */
void
mutex_unlock(mutex_t *mutex);

/**
 * 检查互斥锁是否被当前任务持有
 * @return 1 表示当前任务持有该锁，0 表示不持有
 */
int
mutex_is_locked_by_current(mutex_t *mutex);

#endif  // T_MUTEX_H
