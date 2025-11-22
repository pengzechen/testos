
#ifndef USER_H
#define USER_H

#include "t_task.h"
#include "t_types.h"

// 定义任务元数据结构
typedef struct {
    entry_t   entry;           // 入口（实际跳转用）
    uint64_t  el0_stack_addr;  // EL0 stack
    uint64_t  el1_stack_addr;  // EL1 stack
} task_info_t;

#define TASK_CODE_DST_BASE 0x80000000
#define TEXT_SECTION_SIZE 4096

// 声明所有任务，放task_sec section
#define DECLARE_TASK(n) \
__attribute__((section(".task_code"), aligned(TEXT_SECTION_SIZE))) \
void test_task##n(void)

// 声明 taskinfo，放 .taskinfo_section
#define DECLARE_TASKINFO(n, el0_stack, el1_stack) \
__attribute__((section(".taskinfo_section"))) \
const task_info_t task##n##_info = { \
    (entry_t)((uint64_t)TASK_CODE_DST_BASE + n * TEXT_SECTION_SIZE), \
    el0_stack, el1_stack \
};

void
task_code_copy(void);

#endif  // USER_H