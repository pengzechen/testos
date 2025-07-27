#ifndef __IO_H__
#define __IO_H__

#include "t_types.h"
#include <stdarg.h>

static inline uint8_t read8(const volatile void *addr) {
    return *(const volatile uint8_t *)addr;
}

static inline void write8(uint8_t value, volatile void *addr) {
    *(volatile uint8_t *)addr = value;
}

static inline uint16_t read16(const volatile void *addr) {
    return *(const volatile uint16_t *)addr;
}

static inline void write16(uint16_t value, volatile void *addr) {
    *(volatile uint16_t *)addr = value;
}


static inline uint32_t read32(const volatile void *addr) {
    return *(const volatile uint32_t *)addr;
}

static inline void write32(uint32_t value, volatile void *addr) {
    *(volatile uint32_t *)addr = value;
}

static inline uint64_t read64(const volatile void *addr) {
    return *(const volatile uint64_t *)addr;
}

static inline void write64(uint64_t value, volatile void *addr) {
    *(volatile uint64_t *)addr = value;
}


void io_init();

extern void uart_putstr(const char *str);

/*  printf 函数库  */
extern int my_vprintf(const char *fmt, va_list va);
extern int my_snprintf(char *buf, int size, const char *fmt, ...);
extern int my_vsnprintf(char *buf, int size, const char *fmt, va_list va);

extern int logger(const char *fmt, ...);
extern int logger_info(const char *fmt, ...);
extern int logger_warn(const char *fmt, ...);
extern int logger_error(const char *fmt, ...);

extern void t_run_printf_tests();

#endif // __IO_H__