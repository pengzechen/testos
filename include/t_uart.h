#ifndef __T_UART_H__
#define __T_UART_H__

#include "t_types.h"

// Initialize UART with interrupt support
void uart_init(void);

// Character output functions
void uart_putchar(char c);           // Blocking output
bool uart_putchar_nb(char c);        // Non-blocking output
void uart_putstr(const char *str);   // String output

// Character input functions  
bool uart_getchar_nb(char *c);       // Non-blocking input
bool uart_rx_available(void);        // Check if RX data available

// Buffer status
uint32_t uart_tx_buffer_usage(void); // Get TX buffer usage

// Interrupt handler (internal use)
void uart_interrupt_handler(uint64_t *stack_pointer);

#endif // __T_UART_H__
