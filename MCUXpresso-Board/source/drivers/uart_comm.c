#include "../board.h"
#include "uart_comm.h"
#include "fsl_device_registers.h"
#include "fsl_common.h"
#include <MKL46Z4.h>

void init_uart(void) {
	BOARD_InitHardware();
}

void uart_putc(char ch) {
    while(!(UART0->S1 & UART_S1_TDRE_MASK));
    UART0->D = (uint8_t)ch;
}

void uart_puts(char *ptr_str) {
    while(*ptr_str) {
        if(*ptr_str == '\n') {
            uart_putc('\r');
        }
        uart_putc(*ptr_str++);
    }
}

char uart_getc_nonblocking(void) {
    if(UART0->S1 & UART_S1_RDRF_MASK) {
        return UART0->D;
    }
    return 0;
}
