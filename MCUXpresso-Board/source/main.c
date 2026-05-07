#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "drivers/uart_comm.h"
#include "drivers/proto.h"
#include "drivers/mma8451.h"
#include "drivers/timer_pit.h"
#include "app/dsp_filter.h"
#include "app/game_logic.h"
#include "app/config.h"             // GAME_MODE + wire_* routing macros

#define LINE_BUF_SIZE 64

static int s_accel_ok = 0;

static void handle_line(const char *s) {
    // SYN  ->  ACK:<board_ms>
    if (s[0] == 'S' && s[1] == 'Y' && s[2] == 'N' && s[3] == '\0') {
        /* ============================================================
         *  ACK:<board_ms>
         *  Sent in reply to a SYN command from the host.
         *  Required by host's clock_sync() / PeriodicSync.
         *  >>> Uncomment to enable. <<<
         * ============================================================ */
        // uint32_t t = millis();              // capture FIRST, before any TX
        // char buf[24];
        // snprintf(buf, sizeof(buf), "ACK:%lu\n", (unsigned long)t);
        // wire_puts(buf);
        return;
    }

    // B:<idx>:<dir>:<t_ms>  ->  enqueue beat (no message — game_logic emits H:/M:/BUSY:)
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
    /* ----------------------------------------------------------------
     *  INIT messages -- always enabled (visible at boot).
     * ---------------------------------------------------------------- */
    init_uart();                            // UART0 (always)
    uart_puts("INIT:UART_OK\n");
    uart_puts(WIRE_DESCRIPTION);            // tells you which mode this binary is

    wire_init();                            // proto_init() in DEBUG, no-op in GAME
    uart_puts("INIT:PROTO_OK\n");

    PIT_Init();
    uart_puts("INIT:PIT_OK\n");

    s_accel_ok = ACCEL_Init();
    uart_puts(s_accel_ok ? "INIT:ACCEL_OK\n" : "INIT:ACCEL_FAIL\n");

    GL_Init();

    /* ----------------------------------------------------------------
     *  BOOT marker -- always enabled.
     *  In DEBUG mode, also emits on the wire (UART2) so a BT host
     *  sees the reset.  In GAME mode the wire IS UART0, so a single
     *  emit covers both.
     * ---------------------------------------------------------------- */
    uart_puts("BOOT\n");
#if !GAME_MODE
    wire_puts("BOOT\n");
#endif

    char     line_buf[LINE_BUF_SIZE];
    uint8_t  line_len = 0;
    SRAWDATA accel_data;

    while (1) {
        // 1. Drain protocol-UART RX ring buffer; dispatch on '\n' / '\r'.
        char c;
        while ((c = wire_getc_nonblocking()) != 0) {
            if (c == '\n' || c == '\r') {
                if (line_len > 0) {
                    line_buf[line_len] = '\0';
                    handle_line(line_buf);
                    line_len = 0;
                }
            } else if (line_len < LINE_BUF_SIZE - 1) {
                line_buf[line_len++] = (uint8_t)c;
            } else {
                line_len = 0;                   // drop on overflow
                /* ============================================================
                 *  ERR:overflow
                 *  Surfaces RX line-buffer overflows on the wire.
                 *  Diagnostic only; safe to leave disabled.
                 *  >>> Uncomment to enable. <<<
                 * ============================================================ */
                // wire_puts("ERR:overflow\n");
            }
        }

        // 2. Read accel, run DSP, judge swings against queue head.
        if (s_accel_ok && ACCEL_getAccelDat(&accel_data) == 1) {
            /* ============================================================
             *  RAW:<x>:<y>:<z>
             *  Live MMA8451 12-bit signed X/Y/Z reading. Fires every
             *  accel read (~kHz) -- WILL SATURATE the link if left on
             *  raw. Throttle to every Nth read (or every M ms) before
             *  relying on it. Useful for plotting / DSP-tuning /
             *  hardware bring-up.
             *  >>> Uncomment to enable. <<<
             * ============================================================ */
            // {
            //     char rbuf[40];
            //     snprintf(rbuf, sizeof(rbuf), "RAW:%d:%d:%d\n",
            //              accel_data.x, accel_data.y, accel_data.z);
            //     wire_puts(rbuf);
            // }

            char dir = DSP_DetectSwing(&accel_data);
            if (dir != 0) {
                uint32_t t = millis();
                /* ============================================================
                 *  SWING:<dir>:<ms>
                 *  Accelerometer-derived swing event (one of U/D/L/R).
                 *  Fires on every detected swing, before the queue judges.
                 *  Useful for showcase visualization and tuning.
                 *  >>> Uncomment to enable. <<<
                 * ============================================================ */
                // {
                //     char swbuf[24];
                //     snprintf(swbuf, sizeof(swbuf), "SWING:%c:%lu\n",
                //              dir, (unsigned long)t);
                //     wire_puts(swbuf);
                // }
                GL_OnSwing(t, dir);
            }
        }

        // 3. Auto-expire stale beats (game_logic.c emits M:idx for each).
        GL_Tick(millis());
    }

    return 0;
}
