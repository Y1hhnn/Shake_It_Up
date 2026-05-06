#include "uart_comm.h"
#include <pin_mux.h>
#include <clock_config.h>
#include <board.h>
#include <MKL46Z4.h>
#include <fsl_debug_console.h>

#define UART_RX_BUF_SIZE 128U

static volatile uint8_t  uart_rx_buf[UART_RX_BUF_SIZE];
static volatile uint16_t uart_rx_head = 0;
static volatile uint16_t uart_rx_tail = 0;

void UART0_IRQHandler(void) {
    uint8_t s1 = UART0->S1;

    // Reading D clears RDRF and any of OR/NF/FE/PE that were set with this byte.
    if (s1 & (UART_S1_RDRF_MASK | UART_S1_OR_MASK |
              UART_S1_NF_MASK   | UART_S1_FE_MASK | UART_S1_PF_MASK)) {
        uint8_t c = UART0->D;
        if (s1 & UART_S1_RDRF_MASK) {
            uint16_t next = (uint16_t)((uart_rx_head + 1U) % UART_RX_BUF_SIZE);
            if (next != uart_rx_tail) {
                uart_rx_buf[uart_rx_head] = c;
                uart_rx_head = next;
            }
            // Ring buffer full -> byte dropped.
        }
    }
}

void init_uart(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    uart_rx_head = 0;
    uart_rx_tail = 0;

    UART0->C2 |= UART_C2_RIE_MASK;
    NVIC_EnableIRQ(UART0_IRQn);
}

void uart_putc(char ch) {
    while (!(UART0->S1 & UART_S1_TDRE_MASK));
    UART0->D = (uint8_t)ch;
}

void uart_puts(char *ptr_str) {
    while (*ptr_str) {
        if (*ptr_str == '\n') {
            uart_putc('\r');
        }
        uart_putc(*ptr_str++);
    }
}

char uart_getc_nonblocking(void) {
    if (uart_rx_head == uart_rx_tail) {
        return 0;
    }
    uint8_t c = uart_rx_buf[uart_rx_tail];
    uart_rx_tail = (uint16_t)((uart_rx_tail + 1U) % UART_RX_BUF_SIZE);
    return (char)c;
}
