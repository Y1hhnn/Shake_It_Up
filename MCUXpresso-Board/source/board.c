#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_port.h"
#include "fsl_clock.h"

void BOARD_ConfigPins(void) {
    CLOCK_EnableClock(kCLOCK_PortA);
    PORT_SetPinMux(PORTA, 1U, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTA, 2U, kPORT_MuxAlt2);
}

void BOARD_InitHardware(void) {
    BOARD_ConfigPins();
    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE,
                    BOARD_UART_BAUDRATE,
                    BOARD_DEBUG_UART_TYPE,
                    BOARD_DEBUG_UART_CLK_FREQ);
}
