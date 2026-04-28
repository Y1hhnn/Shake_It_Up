#include "fsl_device_registers.h"
#include "fsl_port.h"
#include <stdio.h>
#include "drivers/uart_comm.h"
#include "drivers/mma8451.h"
#include "drivers/timer_pit.h"
#include "app/dsp_filter.h"
#include "app/game_logic.h"

void short_delay(int loops)
{
    for (int i = loops; i > 0; i--)
    {
        __asm("nop"); /* No Operation to prevent compiler optimization */
    }
}

int main(void)
{
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;   // Start Port A Clock
    PORTA->PCR[1] = PORT_PCR_MUX(2);      // PTA1 -> ALT2 (UART0_RX)
    PORTA->PCR[2] = PORT_PCR_MUX(2);      // PTA2 -> ALT2 (UART0_TX)

    /* Initialize Hardware */
    init_uart();
    PIT_Init();
    uart_puts("System Init... Checking I2C Accel...\n");
    if (ACCEL_Init() == 1)
    {
        uart_puts("MMA8451Q Detected and Configured!\n");
    }
    else
    {
        uart_puts("ERROR: MMA8451Q Not Found!\n");
        while (1)
            ;
    }

    int game_running = 0;
    SRAWDATA accel_data;

    uart_puts("Board Ready. Waiting for START ('S')...\n");

    while (1)
    {
        /* Poll UART for incoming commands */
        char rx_char = uart_getc_nonblocking();

        if (rx_char == 'S')
        {
            PIT_ResetTimer();
        	game_running = 1;
            uart_puts("START!\n");
        }
        else if (rx_char == 'X')
        {
            game_running = 0;
            uart_puts("STOPPED!\n");
        }
        else if (rx_char == 'L' || rx_char == 'R' || rx_char == 'U' || rx_char == 'D')
        {
        	GL_SetTarget(rx_char, millis());
        }

        if (game_running)
        {
            if (ACCEL_getAccelDat(&accel_data) == 1)
            {
                char direction = DSP_DetectSwing(&accel_data);
                if (direction != 0)
                {
                	uint32_t current_time = millis();
                	char grade = GL_EvaluateSwing(current_time, direction);
                	char serial_msg[32];
                	snprintf(serial_msg, sizeof(serial_msg), "HIT:%c:%c:%lu\n", grade, direction, current_time);
                    uart_puts(serial_msg);
                    short_delay(1000000);
                }
            }
        }
    }
    return 0;
}
