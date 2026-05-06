#ifndef BOARD_H
#define BOARD_H

#include "fsl_common.h"
#include "fsl_device_registers.h"

#ifndef kSerialPortUart
#define kSerialPortUart 0U
#endif

#define BOARD_DEBUG_UART_TYPE     kSerialPortUart
#define BOARD_DEBUG_UART_BASEADDR UART0
#define BOARD_DEBUG_UART_INSTANCE 0U
#define BOARD_DEBUG_UART_CLK_FREQ CLOCK_GetOsc0ErClkFreq()
#define BOARD_UART_BAUDRATE       115200

void BOARD_InitHardware(void);
void BOARD_ConfigPins(void);

#endif
