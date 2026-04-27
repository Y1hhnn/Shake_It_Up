#include "drivers/uart_comm.h"
#include "drivers/mma8451.h"
#include "app/dsp_filter.h"
#include <stdio.h>

void short_delay(int loops)
{
    for (int i = loops; i > 0; i--)
    {
        __asm("nop"); /* No Operation to prevent compiler optimization */
    }
}

int main(void)
{
    /* Initialize Hardware */
    init_uart();

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
            game_running = 1;
            uart_puts("START!\n");
        }
        else if (rx_char == 'X')
        {
            game_running = 0;
            uart_puts("STOPPED!\n");
        }

        if (game_running)
        {
            if (ACCEL_getAccelDat(&accel_data) == 1)
            {
                char direction = DSP_DetectSwing(&accel_data);
                if (direction != 0)
                {
                    char serial_msg[16];
                    snprintf(serial_msg, sizeof(serial_msg), "HIT:P:%c\n", direction);
                    uart_puts(serial_msg);
                }
            }
        }
    }
    return 0;
}
