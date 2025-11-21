
#ifndef SPINLOCK_H
#define SPINLOCK_H

#include "t_types.h"

/*
 * 基本spinlock结构
 * 适用于不需要中断保护的场景
 */
typedef struct
{
    volatile uint64_t lock;
} spinlock_t;

/*
 * 带中断保护的spinlock结构
 * 在获取锁时会禁用中断，释放锁时恢复中断状态
 * 用于防止在持有锁期间被中断打断，避免死锁
 */
typedef struct
{
    volatile uint64_t      lock;       // 锁状态，0=未锁定，1=已锁定
    volatile uint32_t irq_flags;  // 保存的中断状态
} spinlock_irq_t;

/* 初始化宏 */
#define SPINLOCK_INIT                                                                              \
    {                                                                                              \
        .lock = 0                                                                                  \
    }
#define SPINLOCK_IRQ_INIT                                                                          \
    {                                                                                              \
        .lock = 0, .irq_flags = 0                                                                  \
    }

/* 基本spinlock函数 */
static inline void
spinlock_init(spinlock_t *lock)
{
    lock->lock = 0;
}

extern void
spin_lock(spinlock_t *lock);
extern int
spin_trylock(spinlock_t *lock);
extern void
spin_unlock(spinlock_t *lock);

/* 带中断保护的spinlock函数 */
static inline void
spinlock_irq_init(spinlock_irq_t *lock)
{
    lock->lock      = 0;
    lock->irq_flags = 0;
}

extern void
spin_lock_irqsave(spinlock_irq_t *lock);
extern int
spin_trylock_irqsave(spinlock_irq_t *lock);
extern void
spin_unlock_irqrestore(spinlock_irq_t *lock);

/*
 * 便利宏定义
 * 使用方法:
 *
 * spinlock_irq_t my_lock = SPINLOCK_IRQ_INIT;
 *
 * // 获取锁并禁用中断
 * spin_lock_irqsave(&my_lock);
 * // 临界区代码
 * // ...
 * // 释放锁并恢复中断
 * spin_unlock_irqrestore(&my_lock);
 *
 * 或者使用trylock:
 * if (spin_trylock_irqsave(&my_lock) == 0) {
 *     // 成功获取锁
 *     // 临界区代码
 *     // ...
 *     spin_unlock_irqrestore(&my_lock);
 * } else {
 *     // 获取锁失败
 * }
 */

#endif  // SPINLOCK_H