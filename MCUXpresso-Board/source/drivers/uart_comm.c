#include "uart_comm.h"
#include <pin_mux.h>
#include <clock_config.h>
#include <board.h>
#include <MKL46Z4.h>
#include <fsl_debug_console.h>

void init_uart(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
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
