#ifndef BLUETOOTH_HC06_H
#define BLUETOOTH_HC06_H

#include "uart_comm.h"

static inline void BT_Init(void) {
    init_uart();
}

static inline char BT_ReadCommand(void) {
    return uart_getc_nonblocking();
}

static inline void BT_SendResult(char *msg) {
    uart_puts(msg);
}

#endif
