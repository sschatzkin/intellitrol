/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         uart.h
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2008  Scully Signal Company
 *
 *   Description:    Settings for uart2
 *
 *   Revision History:
 *
 *****************************************************************************/
#ifndef UART_H
#define UART_H

/*****************************************************************************
 * DEFINITIONS
 *****************************************************************************/
// Baudrate
// #define BAUDRATE2   19200
#define BAUDRATE2   9600

// UART IOs
#define UART1_TX_TRIS   TRISFbits.TRISF3
#define UART1_RX_TRIS   TRISFbits.TRISF2
#define UART2_TX_TRIS   TRISFbits.TRISF5
#define UART2_RX_TRIS   TRISFbits.TRISF4

#define home_clr()      UART2PutChar(0x1B); \
                        UART2PutChar('['); \
                        UART2PutChar('2'); \
                        UART2PutChar('J');

#define home()          UART2PutChar(0x1B); \
                        UART2PutChar('['); \
                        UART2PutChar('1'); \
                        UART2PutChar(';'); \
                        UART2PutChar('1'); \
                        UART2PutChar('H');

/*****************************************************************************
 * EOF
 *****************************************************************************/

#endif  /* end of UART_H */
