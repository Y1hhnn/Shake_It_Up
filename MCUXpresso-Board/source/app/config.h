#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "drivers/uart_comm.h"
#include "drivers/proto.h"

#define MODE_DEBUG 0
#define MODE_USB 1
#define MODE_BT 2

#ifndef BUILD_MODE
#define BUILD_MODE MODE_USB
#endif


#if BUILD_MODE == MODE_DEBUG
#define wire_init() proto_init()
#define wire_puts(s) proto_puts(s)
#define wire_getc_nonblocking() proto_getc_nonblocking()
#define WIRE_DESCRIPTION "MODE:DEBUG (USB on UART0 @ 115200, HC-06 on UART2 @ 38400)\n"

#elif BUILD_MODE == MODE_USB
#define wire_init() ((void)0) /* UART0 inited by init_uart() */
#define wire_puts(s) uart_puts(s)
#define wire_getc_nonblocking() uart_getc_nonblocking()
#define WIRE_DESCRIPTION "MODE:USB (game on UART0 via OpenSDA @ 115200)\n"

#elif BUILD_MODE == MODE_BT
#define wire_init() ((void)0) /* UART0 inited by init_uart() */
#define wire_puts(s) uart_puts(s)
#define wire_getc_nonblocking() uart_getc_nonblocking()
#define WIRE_DESCRIPTION "MODE:BT (HC-06 on UART0 @ 115200, J1[2]/J1[4])\n"

#else
#error "BUILD_MODE must be MODE_DEBUG, MODE_USB, or MODE_BT"
#endif

#endif
