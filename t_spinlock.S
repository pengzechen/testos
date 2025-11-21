

/*
忙等待：获取锁的处理器会不断地检查锁的状态，直到锁被释放。
低开销：由于不涉及上下文切换，t_spinlock 比一般的锁（如互斥锁）开销更低。
短时间锁定：适用于锁定时间较短的场景。如果锁定时间较长，自旋等待会浪费 CPU 资源。
 */
.global spin_lock
.global spin_unlock
.global spin_trylock


spin_lock:
    mov w1, #1                // w1 = 1 (表示锁定)
1:  ldaxr w2, [x0]            // 原子加载锁的状态到 w2，带有 Acquire 语义
    cbnz w2, 1b               // 如果锁已被持有，继续自旋
    stlxr w2, w1, [x0]        // 尝试原子存储 1 到锁变量，带有 Release 语义
    cbnz w2, 1b               // 如果存储失败（锁被其他处理器获取），继续自旋
    dmb ish                   // 内存屏障，确保锁定操作完成
    ret

/*
 * when t_spinlock already taken, r1 will read 1 which is same with the
 * 'strex' failure return value, no extra convertion needed.
 */
spin_trylock:
    mov w1, #1                // w1 = 1 (表示锁定)
    ldaxr w2, [x0]            // 原子加载锁的状态到 w2，带有 Acquire 语义
    cbnz w2, 2f               // 如果锁已被持有，跳到标签2
    stlxr w2, w1, [x0]        // 尝试原子存储 1 到锁变量，带有 Release 语义
    cbnz w2, 2f               // 如果存储失败（锁被其他处理器获取），跳到标签2
    dmb ish                   // 内存屏障，确保锁定操作完成
    mov w0, #0                // 返回 0 表示获取锁成功
    ret
2:  mov w0, #1                // 返回 1 表示获取锁失败
    ret


spin_unlock:
    dmb ish                   // 内存屏障，确保之前的内存操作完成
    mov w1, #0
    stlr w1, [x0]             // 原子存储 0 到锁变量，带有 Release 语义
    ret



/*
 * Spinlock with IRQ disable/enable
 * 在获取锁时禁用中断，释放锁时恢复中断状态
 * 这样可以防止在持有锁期间被中断打断，避免死锁
 */
.global spin_lock_irqsave
.global spin_unlock_irqrestore
.global spin_trylock_irqsave

/*
 * spin_lock_irqsave - 获取锁并保存中断状态
 * 参数: x0 = spinlock_irq_t *lock
 * 返回: 无
 *
 * spinlock_irq_t 结构:
 * offset 0: lock (int)
 * offset 4: irq_flags (uint32_t)
 */
spin_lock_irqsave:
    // 保存当前的DAIF状态
    mrs x2, daif              // 读取当前DAIF寄存器
    str w2, [x0, #4]          // 保存到lock->irq_flags

    // 禁用IRQ中断
    msr daifset, #2           // 设置IRQ mask位

    // 获取spinlock
    mov w1, #1                // w1 = 1 (表示锁定)
1:  ldaxr w2, [x0]            // 原子加载锁的状态到 w2，带有 Acquire 语义
    cbnz w2, 1b               // 如果锁已被持有，继续自旋
    stlxr w2, w1, [x0]        // 尝试原子存储 1 到锁变量，带有 Release 语义
    cbnz w2, 1b               // 如果存储失败（锁被其他处理器获取），继续自旋
    dmb ish                   // 内存屏障，确保锁定操作完成
    ret

/*
 * spin_trylock_irqsave - 尝试获取锁并保存中断状态
 * 参数: x0 = spinlock_irq_t *lock
 * 返回: w0 = 0 成功, 1 失败
 */
spin_trylock_irqsave:
    // 保存当前的DAIF状态
    mrs x2, daif              // 读取当前DAIF寄存器
    str w2, [x0, #4]          // 保存到lock->irq_flags

    // 禁用IRQ中断
    msr daifset, #2           // 设置IRQ mask位

    // 尝试获取spinlock
    mov w1, #1                // w1 = 1 (表示锁定)
    ldaxr w2, [x0]            // 原子加载锁的状态到 w2，带有 Acquire 语义
    cbnz w2, 2f               // 如果锁已被持有，跳到失败处理
    stlxr w2, w1, [x0]        // 尝试原子存储 1 到锁变量，带有 Release 语义
    cbnz w2, 2f               // 如果存储失败（锁被其他处理器获取），跳到失败处理
    dmb ish                   // 内存屏障，确保锁定操作完成
    mov w0, #0                // 返回 0 表示获取锁成功
    ret
2:  // 获取锁失败，恢复中断状态
    ldr w2, [x0, #4]          // 加载保存的irq_flags
    msr daif, x2              // 恢复DAIF寄存器
    mov w0, #1                // 返回 1 表示获取锁失败
    ret

/*
 * spin_unlock_irqrestore - 释放锁并恢复中断状态
 * 参数: x0 = spinlock_irq_t *lock
 * 返回: 无
 */
spin_unlock_irqrestore:
    dmb ish                   // 内存屏障，确保之前的内存操作完成
    mov w1, #0
    stlr w1, [x0]             // 原子存储 0 到锁变量，带有 Release 语义

    // 恢复中断状态
    ldr w1, [x0, #4]          // 加载保存的irq_flags
    msr daif, x1              // 恢复DAIF寄存器
    ret