#ifndef __ACFG_H__
#define __ACFG_H__

#define CNTP_TIMER   30
#define CNTV_TIMER   27

#define TIMER  CNTV_TIMER

#define GICD_BASE_ADDR  0x8000000
#define GICC_BASE_ADDR  0x8010000

#define OS_VERSION "0.1.0"

#define T_STACK_SIZE (1 << 14) // 16 K

#ifndef T_SMP_NUM
#define T_SMP_NUM 2
#endif

#endif // __ACFG_H__