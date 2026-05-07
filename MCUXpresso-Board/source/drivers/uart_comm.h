#ifndef UART_COMM_H
#define UART_COMM_H

#include <stdint.h>

/** Initializes the UART interface via the debug console.*/
void init_uart(void);

/** Transmits a single character over UART (Blocking). */
void uart_putc(char ch);

/** Transmits a null-terminated string over UART (Blocking) */
void uart_puts(char *ptr_str);

/** Receives a single character from UART (Non-blocking) */
char uart_getc_nonblocking(void);

#endif /* UART_COMM_H */
