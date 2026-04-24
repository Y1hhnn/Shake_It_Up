/**
 * @file    uart_comm.h
 * @brief   UART Communication Driver for FRDM-KL46Z
 * @details Handles hardware initialization and non-blocking/blocking
 * serial communication for the "Shake It Up!" project.
 */

#ifndef UART_COMM_H
#define UART_COMM_H

#include <stdint.h>

/**
 * @brief Initializes the UART interface via the debug console.
 * @details Configures board pins, clocks, and the debug console to enable
 * serial communication (typically 115200, 8N1).
 */
void init_uart(void);

/**
 * @brief Transmits a single character over UART (Blocking).
 * @param ch The character to be transmitted.
 * @details This function waits until the Transmit Data Register Empty (TDRE)
 * flag is set before sending the character.
 */
void uart_putc(char ch);

/**
 * @brief Transmits a null-terminated string over UART (Blocking).
 * @param ptr_str Pointer to the string to be transmitted.
 * @details Automatically converts newline characters ('\n') to carriage
 * returns ('\r') followed by newlines.
 */
void uart_puts(char *ptr_str);

/**
 * @brief Receives a single character from UART (Non-blocking).
 * @return The received character if available, otherwise returns 0 (null char).
 * @details Checks the Receive Data Register Full (RDRF) flag. If data is
 * available, it reads and returns it immediately without blocking
 * the CPU execution.
 */
char uart_getc_nonblocking(void);

#endif /* UART_COMM_H */
