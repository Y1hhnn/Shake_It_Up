/**
 * @file    proto.h
 * @brief   Protocol UART (UART2 on PTD2/PTD3 -> HC-06 Bluetooth).
 *
 * Mirrors uart_comm's API but on UART2. UART0 remains the debug bus
 * (USB via OpenSDA); this one carries the game protocol over Bluetooth.
 */

#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>

void proto_init(void);                  // 115200 8N1, RX-IRQ ring buffered
void proto_putc(char ch);
void proto_puts(char *ptr_str);
char proto_getc_nonblocking(void);

#endif
