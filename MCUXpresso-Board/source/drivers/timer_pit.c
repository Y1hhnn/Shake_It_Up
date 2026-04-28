#include "MKL46Z4.h"
#include "timer_pit.h"

volatile uint32_t msTicks = 0;

void PIT_Init(void) {
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;

    PIT->MCR = PIT_MCR_FRZ_MASK;

    PIT->CHANNEL[0].LDVAL = 23999;

    PIT->CHANNEL[0].TCTRL |= (PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK);

    NVIC_EnableIRQ(PIT_IRQn);
}

void PIT_IRQHandler(void) {
    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;

    msTicks++;
}

uint32_t millis(void) {
    return msTicks;
}

void PIT_ResetTimer(void) {
    msTicks = 0;
}
