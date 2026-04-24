#include "drivers/uart_comm.h"

void short_delay(int loops) {
    for(int i = loops; i > 0; i--) {
        __asm("nop"); /* No Operation to prevent compiler optimization */
    }
}

int main(void) {
    /* Initialize Hardware */
    init_uart();

    int game_running = 0;
    int mock_counter = 0;

    uart_puts("Board Ready. Waiting for START ('S')...\n");

    while(1) {
        /* Poll UART for incoming commands */
        char rx_char = uart_getc_nonblocking();

        if (rx_char == 'S') {
            game_running = 1;
            mock_counter = 0;

            uart_puts("START! Green LED is ON.\n");
        }
        else if (rx_char == 'X') {
            game_running = 0;

            uart_puts("STOPPED! Green LED is OFF.\n");
        }

        if (game_running) {
        	short_delay(3000000);

            mock_counter++;
            if (mock_counter % 3 == 0) uart_puts("HIT:P\n");
            else if (mock_counter % 3 == 1) uart_puts("HIT:G\n");
            else uart_puts("HIT:M\n");
        }
    }
    return 0;
}
