#include <stdint.h>
#include <stdio.h>
#include "game_logic.h"
#include "config.h"                 // wire_puts -> UART0 or UART2 per GAME_MODE

static Beat    q[BEAT_QUEUE_CAP];
static uint8_t q_head  = 0;
static uint8_t q_count = 0;

static uint32_t abs_diff(uint32_t a, uint32_t b) {
    return (a >= b) ? (a - b) : (b - a);
}

static Beat *q_peek_head(void) {
    return &q[q_head];
}

static void q_pop_head(void) {
    q_head = (uint8_t)((q_head + 1U) % BEAT_QUEUE_CAP);
    q_count--;
}

static int q_push(uint16_t idx, char dir, uint32_t t_ms) {
    if (q_count >= BEAT_QUEUE_CAP) return 0;
    uint8_t tail = (uint8_t)((q_head + q_count) % BEAT_QUEUE_CAP);
    q[tail].idx  = idx;
    q[tail].dir  = dir;
    q[tail].t_ms = t_ms;
    q_count++;
    return 1;
}

static void emit_hit(uint16_t idx, char grade, uint32_t t) {
    /* ============================================================
     *  H:<idx>:<grade>:<actual_ms>
     *  Final game determination -- Perfect / Good / Miss for a
     *  swing that landed within MISS_WINDOW_MS of the queue head.
     *  Required by host's parse_v2_line to score hits.
     *  >>> Uncomment to enable. <<<
     * ============================================================ */
    // char buf[40];
    // snprintf(buf, sizeof(buf), "H:%u:%c:%lu\n",
    //          (unsigned)idx, grade, (unsigned long)t);
    // wire_puts(buf);
    (void)idx; (void)grade; (void)t;        // silence unused-arg warnings
}

static void emit_miss(uint16_t idx) {
    /* ============================================================
     *  M:<idx>
     *  Auto-miss: a queued beat expired (now > t_ms + MISS_WINDOW)
     *  with no swing arriving in time.
     *  Required by host to know which beats timed out.
     *  >>> Uncomment to enable. <<<
     * ============================================================ */
    // char buf[24];
    // snprintf(buf, sizeof(buf), "M:%u\n", (unsigned)idx);
    // wire_puts(buf);
    (void)idx;
}

static void emit_busy(uint16_t idx) {
    /* ============================================================
     *  BUSY:<idx>
     *  Queue is full; host's beat with this idx was dropped.
     *  Diagnostic; safe to leave disabled if you trust your
     *  scheduler to respect BEAT_QUEUE_CAP.
     *  >>> Uncomment to enable. <<<
     * ============================================================ */
    // char buf[24];
    // snprintf(buf, sizeof(buf), "BUSY:%u\n", (unsigned)idx);
    // wire_puts(buf);
    (void)idx;
}

void GL_Init(void) {
    q_head  = 0;
    q_count = 0;
}

int GL_EnqueueBeat(uint16_t idx, char dir, uint32_t t_ms) {
    if (!q_push(idx, dir, t_ms)) {
        emit_busy(idx);
        return 0;
    }
    return 1;
}

void GL_OnSwing(uint32_t now, char detected_dir) {
    if (q_count == 0) return;                       // stray: queue empty

    Beat *h = q_peek_head();

    // Stray-early: head's window has not opened yet.
    if (h->t_ms > now && (h->t_ms - now) > GOOD_WINDOW_MS) {
        return;
    }

    uint32_t dt = abs_diff(now, h->t_ms);

    char grade;
    if (detected_dir != h->dir) {
        grade = 'M';                                // wrong direction
    } else if (dt <= PERFECT_WINDOW_MS) {
        grade = 'P';
    } else if (dt <= GOOD_WINDOW_MS) {
        grade = 'G';
    } else if (dt <= MISS_WINDOW_MS) {
        grade = 'M';                                // correct dir but too late
    } else {
        // Past MISS_WINDOW; let GL_Tick auto-miss this one.
        return;
    }

    emit_hit(h->idx, grade, now);
    q_pop_head();
}

void GL_Tick(uint32_t now) {
    while (q_count > 0) {
        Beat *h = q_peek_head();
        // Expired only if now is strictly past head time + miss window.
        if (now <= h->t_ms + MISS_WINDOW_MS) break;
        emit_miss(h->idx);
        q_pop_head();
    }
}
