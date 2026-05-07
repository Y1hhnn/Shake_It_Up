#include "proto.h"
#include "MKL46Z4.h"
#include "fsl_clock.h"

#define PROTO_RX_BUF_SIZE 128U
#define PROTO_BAUDRATE    38400U

static volatile uint8_t  proto_rx_buf[PROTO_RX_BUF_SIZE];
static volatile uint16_t proto_rx_head = 0;
static volatile uint16_t proto_rx_tail = 0;

void UART2_IRQHandler(void) {
    uint8_t s1 = UART2->S1;

    if (s1 & (UART_S1_RDRF_MASK | UART_S1_OR_MASK |
              UART_S1_NF_MASK   | UART_S1_FE_MASK | UART_S1_PF_MASK)) {
        uint8_t c = UART2->D;                       
        if (s1 & UART_S1_RDRF_MASK) {
            uint16_t next = (uint16_t)((proto_rx_head + 1U) % PROTO_RX_BUF_SIZE);
            if (next != proto_rx_tail) {
                proto_rx_buf[proto_rx_head] = c;
                proto_rx_head = next;
            }
            // Ring buffer full -> byte dropped.
        }
    }
}

void proto_init(void) {
    // 1. Gate clocks for PORTE and UART2.
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
    SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;

    // 2. Pin mux: PTE17 = UART2_RX (ALT3, J3[7]), PTE16 = UART2_TX (ALT3, J3[9]).
    PORTE->PCR[17] = PORT_PCR_MUX(3);
    PORTE->PCR[16] = PORT_PCR_MUX(3);

    // 3. Disable TX/RX while reprogramming baud.
    UART2->C2 &= (uint8_t)~(UART_C2_TE_MASK | UART_C2_RE_MASK);

    // 4. Baud
    uint32_t bus_hz = CLOCK_GetBusClkFreq();
    uint32_t sbr    = (bus_hz + (8U * PROTO_BAUDRATE)) / (16U * PROTO_BAUDRATE);
    if (sbr == 0U) sbr = 1U;
    if (sbr > 0x1FFFU) sbr = 0x1FFFU;

    UART2->BDH = (uint8_t)((UART2->BDH & ~UART_BDH_SBR_MASK)
                           | UART_BDH_SBR((uint8_t)(sbr >> 8)));
    UART2->BDL = (uint8_t)(sbr & 0xFFU);

    UART2->C1 = 0;

    proto_rx_head = 0;
    proto_rx_tail = 0;

    // 7. Enable RX interrupt, then enable TX/RX, then NVIC line.
    UART2->C2 |= UART_C2_RIE_MASK;
    UART2->C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK);
    NVIC_EnableIRQ(UART2_IRQn);
}

void proto_putc(char ch) {
    while (!(UART2->S1 & UART_S1_TDRE_MASK));
    UART2->D = (uint8_t)ch;
}

void proto_puts(char *ptr_str) {
    while (*ptr_str) {
        if (*ptr_str == '\n') {
            proto_putc('\r');
        }
        proto_putc(*ptr_str++);
    }
}

char proto_getc_nonblocking(void) {
    if (proto_rx_head == proto_rx_tail) {
        return 0;
    }
    uint8_t c = proto_rx_buf[proto_rx_tail];
    proto_rx_tail = (uint16_t)((proto_rx_tail + 1U) % PROTO_RX_BUF_SIZE);
    return (char)c;
}
