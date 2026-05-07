#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "drivers/uart_comm.h"
#include "drivers/proto.h"

/* =========================================================================
 * Build-mode selector
 *
 *   GAME_MODE = 0   DEBUG MODE  (default)
 *     - UART0  -> OpenSDA -> USB                      (115200 8N1)
 *     - UART2  -> HC-06   -> Bluetooth                (38400 8N1)
 *                            PTE16 (J3[9]) = TX
 *                            PTE17 (J3[7]) = RX
 *     - HC-06 baud:  AT+BAUD6 (38400)
 *     - Both buses are live simultaneously: USB shows INIT diagnostics,
 *       BT carries the game protocol.
 *
 *   GAME_MODE = 1   GAME MODE
 *     - UART0  -> HC-06   -> Bluetooth                (115200 8N1)
 *                            PTA2  (J1[4]) = TX
 *                            PTA1  (J1[2]) = RX
 *     - UART2  -> not used
 *     - HC-06 baud:  AT+BAUD8 (115200)
 *     - HC-06 shares UART0 lines with the OpenSDA bridge -- USB-serial
 *       and BT cannot both be active simultaneously. Either keep USB
 *       unplugged (battery-powered demo), or plug in for power-only
 *       and accept that the OpenSDA chip will fight HC-06 on the bus.
 *
 * To switch modes, change the value below and rebuild + reflash.
 * Or override from the project preprocessor flags with -DGAME_MODE=1.
 * ========================================================================= */

#ifndef GAME_MODE
#define GAME_MODE 0
#endif

/* The "wire" is whichever UART carries the game protocol in this build.
 * main.c and game_logic.c only call wire_*; the routing happens here. */

#if GAME_MODE
  #define wire_init()              ((void)0)              /* UART0 inited by init_uart() */
  #define wire_puts(s)             uart_puts(s)
  #define wire_getc_nonblocking()  uart_getc_nonblocking()
  #define WIRE_DESCRIPTION         "MODE:GAME (HC-06 on UART0 @ 115200)\n"
#else
  #define wire_init()              proto_init()
  #define wire_puts(s)             proto_puts(s)
  #define wire_getc_nonblocking()  proto_getc_nonblocking()
  #define WIRE_DESCRIPTION         "MODE:DEBUG (USB on UART0, HC-06 on UART2 @ 38400)\n"
#endif

#endif
