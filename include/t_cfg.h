#ifndef __ACFG_H__
#define __ACFG_H__

#define CNTP_TIMER   30
#define CNTV_TIMER   27

#define TIMER  CNTP_TIMER

// UART interrupt number (SPI 1 = GIC interrupt 33)
#define UART_IRQ     33

#define GICD_BASE_ADDR  0x8000000
#define GICC_BASE_ADDR  0x8010000

// UART PL011 register definitions
#define UART_BASE_ADDR  0x9000000
#define UART_DR         (UART_BASE_ADDR + 0x000)  // Data Register
#define UART_RSR        (UART_BASE_ADDR + 0x004)  // Receive Status Register
#define UART_FR         (UART_BASE_ADDR + 0x018)  // Flag Register
#define UART_ILPR       (UART_BASE_ADDR + 0x020)  // IrDA Low-Power Counter Register
#define UART_IBRD       (UART_BASE_ADDR + 0x024)  // Integer Baud Rate Register
#define UART_FBRD       (UART_BASE_ADDR + 0x028)  // Fractional Baud Rate Register
#define UART_LCR_H      (UART_BASE_ADDR + 0x02C)  // Line Control Register
#define UART_CR         (UART_BASE_ADDR + 0x030)  // Control Register
#define UART_IFLS       (UART_BASE_ADDR + 0x034)  // Interrupt FIFO Level Select Register
#define UART_IMSC       (UART_BASE_ADDR + 0x038)  // Interrupt Mask Set/Clear Register
#define UART_RIS        (UART_BASE_ADDR + 0x03C)  // Raw Interrupt Status Register
#define UART_MIS        (UART_BASE_ADDR + 0x040)  // Masked Interrupt Status Register
#define UART_ICR        (UART_BASE_ADDR + 0x044)  // Interrupt Clear Register

// UART Flag Register bits
#define UART_FR_TXFF    (1 << 5)  // Transmit FIFO full
#define UART_FR_RXFE    (1 << 4)  // Receive FIFO empty
#define UART_FR_BUSY    (1 << 3)  // UART busy

// UART Interrupt bits
#define UART_INT_TX     (1 << 5)  // Transmit interrupt
#define UART_INT_RX     (1 << 4)  // Receive interrupt
#define UART_INT_RT     (1 << 6)  // Receive timeout interrupt

// UART Control Register bits
#define UART_CR_UARTEN  (1 << 0)  // UART enable
#define UART_CR_TXE     (1 << 8)  // Transmit enable
#define UART_CR_RXE     (1 << 9)  // Receive enable

#define OS_VERSION "0.1.0"

#define T_STACK_SIZE (1 << 14) // 16 K

// 没定义 T_SMP_NUM 就是单核
#ifndef T_SMP_NUM
#define T_SMP_NUM 1
#endif

// 没定义 GUEST_LABEL 就不是guest
#ifndef GUEST_LABEL
#define GUEST_LABEL "[not guest] "
#endif

#endif // __ACFG_H__