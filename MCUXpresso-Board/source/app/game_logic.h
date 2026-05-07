#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdint.h>

#define BEAT_QUEUE_CAP    16U

#define PERFECT_WINDOW_MS 500U
#define GOOD_WINDOW_MS    1000U
#define MISS_WINDOW_MS    1500U

typedef struct {
    uint16_t idx;
    char     dir;
    uint32_t t_ms;
} Beat;

void GL_Init(void);

// Returns 1 on success, 0 if queue is full
int  GL_EnqueueBeat(uint16_t idx, char dir, uint32_t t_ms);

// Judge a swing against the head of the queue. Emit H:idx:grade:now.
void GL_OnSwing(uint32_t now, char detected_dir);

// Drain expired beats from the queue. Emits M:idx for each one auto-missed.
void GL_Tick(uint32_t now);

#endif
