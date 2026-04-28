#ifndef TIMER_PIT_H
#define TIMER_PIT_H

#include <stdint.h>

extern volatile uint32_t msTicks;

void PIT_Init(void);

uint32_t millis(void);

void PIT_ResetTimer(void);

#endif
