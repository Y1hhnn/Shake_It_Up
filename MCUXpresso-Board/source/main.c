#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "drivers/uart_comm.h"
#include "drivers/mma8451.h"
#include "drivers/timer_pit.h"
#include "app/dsp_filter.h"
#include "app/game_logic.h"

#define LINE_BUF_SIZE 64

static int s_accel_ok = 0;

static void handle_line(const char *s) {
    // SYN  -> ACK:<board_ms>
    if (s[0] == 'S' && s[1] == 'Y' && s[2] == 'N' && s[3] == '\0') {
        uint32_t t = millis();              // capture FIRST, before any TX work
        char buf[24];
        snprintf(buf, sizeof(buf), "ACK:%lu\n", (unsigned long)t);
        uart_puts(buf);
        return;
    }

    // B:<idx>:<dir>:<t_ms>  ->  enqueue beat
    if (s[0] == 'B' && s[1] == ':') {
        unsigned int  idx_u;
        char          dir;
        unsigned long t_ms;
        if (sscanf(s, "B:%u:%c:%lu", &idx_u, &dir, &t_ms) == 3 &&
            idx_u <= 0xFFFFU &&
            (dir == 'U' || dir == 'D' || dir == 'L' || dir == 'R')) {
            GL_EnqueueBeat((uint16_t)idx_u, dir, (uint32_t)t_ms);
        }
        return;
    }

    // Anything else: silently ignored.
}

int main(void) {
    init_uart();
    uart_puts("INIT:UART_OK\n");

    PIT_Init();
    uart_puts("INIT:PIT_OK\n");

    s_accel_ok = ACCEL_Init();
    uart_puts(s_accel_ok ? "INIT:ACCEL_OK\n" : "INIT:ACCEL_FAIL\n");

    GL_Init();
    uart_puts("BOOT\n");

    char    line_buf[LINE_BUF_SIZE];
    uint8_t line_len = 0;
    SRAWDATA accel_data;

    while (1) {
        // 1. Drain UART RX ring buffer; dispatch on '\n' / '\r'.
        char c;
        while ((c = uart_getc_nonblocking()) != 0) {
            if (c == '\n' || c == '\r') {
                if (line_len > 0) {
                    line_buf[line_len] = '\0';
                    handle_line(line_buf);
                    line_len = 0;
                }
            } else if (line_len < LINE_BUF_SIZE - 1) {
                line_buf[line_len++] = (uint8_t)c;
            } else {
                line_len = 0;                   // overflow: drop and warn
                uart_puts("ERR:overflow\n");
            }
        }

        // 2. Read accel, run DSP, judge swings against queue head.
        if (s_accel_ok && ACCEL_getAccelDat(&accel_data) == 1) {
            char dir = DSP_DetectSwing(&accel_data);
            if (dir != 0) {
                GL_OnSwing(millis(), dir);
            }
        }

        // 3. Auto-expire stale beats (emits M:idx for each).
        GL_Tick(millis());
    }

    return 0;
}
