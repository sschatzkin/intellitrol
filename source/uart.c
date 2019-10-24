/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         uart.c
 *
 *   Revision:       REV 1.5.27
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009,2016  Scully Signal Company
 *
 *   Description:    Dallas "Touch Data" File-structure operations
 *
 * Revision History:
 *   Rev     Date    Who  Description of Change Made
 *  ------ --------  ---  --------------------------------------------
 *         09/24/10  KLL  Modified uart2_init to correct problem with lost
 *                          messages.
 *                         - Cleared TXEN to prevent collisions at host
 *                         - Enabled receiver
 *         10/09_12  KLL  Added transmitter enable to the UART2PutChar routine
 * 1.6.34  10/10/16  DHP  Renamed communicate_flag to receive_bk_status.
 *****************************************************************************/

#include "common.h"
#include "uart.h"

/*****************************************************************************
 * U2BRG register value and baudrate mistake calculation
 *****************************************************************************/
#define BAUDRATEREG2 (((FCY/16)/BAUDRATE2)-1)
#define SCBR1200     (((FCY/16)/1200)-1)
#define SCBR2400     (((FCY/16)/2400)-1)
#define SCBR4800     (((FCY/16)/4800)-1)
#define SCBR9600     (((FCY/16)/9600)-1)
#define SCBR19200    (((FCY/16)/19200)-1)

#if BAUDRATEREG2 > 0xFF
#error Cannot set up UART2 for the SYSCLK and BAUDRATE.\
 Correct values in main.h and uart.h files.
#endif

static const UINT16 baudtbl[] =
{
    0,                          /* 00  Reserved (no parity) */
    0,                          /* 01  Reserved (odd parity) */
    0,                          /* 02  Reserved (even parity) */
    SCBR1200,                   /* 03   1200 baud */
    SCBR2400,                   /* 04   2400 baud */
    SCBR4800,                   /* 05   4800 baud */
    SCBR9600,                   /* 06   9600 baud */
    SCBR19200,                  /* 07  19200 baud */
    0,                          /* 08  Reserved (7-bit char) */
    0                           /* 09  Reserved (8-bit char) */
};

static const unsigned bit8tbl[] =
{
    0,              /* 000 8-bit   no parity */
    2,              /* 004 8-bit  odd parity */
    1               /* 002 8-bit even parity */
};

/*****************************************************************************
 * Function: UART2Init
 *
 * Precondition: None.
 *
 * Overview: Setup UART2 module.
 *
 * Input: None.
 *
 * Output: None.
 *
 *****************************************************************************/
static UINT16 baud_rate_temp;

void UART2Init()
{
    // Set directions of UART IOs
  UART2_TX_TRIS = 0;
  UART2_RX_TRIS = 1;
  IEC1bits.U2TXIE = 0;      /* Disable Transmit interrupt */
  IEC1bits.U2RXIE = 0;      /* Disable Receiver Interrupt */
  baud_rate_temp = baudtbl[modbus_baud];
  U2BRG = baud_rate_temp;       /* ModBus/Comm baud rate */
  U2MODE = 0;
  U2STA = 0;

  U2MODEbits.PDSEL= bit8tbl[modbus_parity];  /* Set up parity none-odd-even */
  U2MODEbits.RTSMD = 1;     /* UxRTS in Simplex mode */

  /* UxTX, UxRX and BCLK pins are enabled and used; UxCTS pin controlled by port latches */
  U2MODEbits.UEN = 0;
  U2STAbits.URXISEL = 0;    /* Interrupt on every character received */
  IPC7bits.U2RXIP = 5;      /* Interrupt Priority 5 */
  IFS1bits.U2TXIF = 0;
  IFS1bits.U2RXIF = 0;
  IEC1bits.U2RXIE = 1;
  /****************************** 9/23/2008 6:38AM ***************************
   * Interrupt when the last character is shifted out of the Transmit Shift
   * Register; all transmit operations are completed
   ***************************************************************************/
  U2STAbits.UTXISEL0 = 1;
  U2MODEbits.UARTEN = 1;
  U2STAbits.UTXEN = 1;

  TXEN = 0;                       /* Disable transmit */
}

/*******************************4/23/2008 11:36AM*****************************
 * Disable the MODBUS uart transmitter. Nothing to send right now
 *****************************************************************************/
void clear_tx2en(void)
{
  IEC1bits.U2TXIE = 0;
  U2STAbits.UTXEN = 0;
  IFS1bits.U2TXIF = 0;
}

/*******************************4/23/2008 11:37AM*****************************
 * Enable the MODBUS interrupt. Need to answer the call.
 * MODBUS protocol dictates only answer when spoken to
 *****************************************************************************/
void set_tx2_en(void)
{
  TXEN = 1;                       /* Enable transmit */
  IFS1bits.U2RXIF = 0;
  IEC1bits.U2RXIE = 0;
  IFS1bits.U2TXIF = 0;
  U2STAbits.UTXEN = 1;
}


void  UART2PutChar(char Ch)
{
  if (modbus_addr == 0)                   /* ASCII only if modbus address 0 */
  {
      // wait for empty buffer
    if ( TXEN == 0)
    {
      flush_uart2();
      TXEN = 1;                       /* Enable transmit */
    }
    U2STAbits.UTXEN = 1;
    while(U2STAbits.UTXBF == 1) {}
    U2TXREG = (unsigned int)(unsigned char)Ch;
  }

}

void flush_uart2()
{
volatile unsigned int key;
int timeout = 0x7FFF;

  while ((U2STAbits.URXDA == 1) && (timeout--))
  {
    key = U2RXREG;        /* Flushing any errors */
  }
  U2STAbits.OERR = 0;                     /* Clear overflow condition */
/*lint -e{438, 550}*/ 
}

/******************************* 11/12/2009 5:53AM ***************************
 * These routines are used to communicate to the backup processor
 * The communication packet format: The first byte is the size of the packet
 * being received and the second byte is the CMD byte. The rest of the bytes
 * must be data and the entire packet must not exceed 20 bytes
 *****************************************************************************/
void UART1Init()
{
    // Set directions of UART IOs
  rx_size = 0;
  UART1_TX_TRIS = 0;
  UART1_RX_TRIS = 1;
  IEC0bits.U1TXIE = 0;      /* Disable Transmit interrupt */
  IEC0bits.U1RXIE = 0;      /* Disable Receiver Interrupt */
  U1BRG = SCBR9600;       /* ModBus/Comm baud rate */
  U1MODE = 0;
  U1STA = 0;

  U1MODEbits.PDSEL = 0;     /* Set up parity none-odd-even */
  U1MODEbits.RTSMD = 1;     /* UxRTS in Simplex mode */

  /* UxTX, UxRX and BCLK pins are enabled and used; UxCTS pin controlled by port latches */
  U1STAbits.URXISEL = 0;    /* Interrupt on every character received */
  IPC2bits.U1RXIP = 5;      /* Interrupt Priority 5 */
  IFS0bits.U1TXIF = 0;
  IFS0bits.U1RXIF = 0;
  /****************************** 9/23/2008 6:38AM ***************************
   * Interrupt when the last character is shifted out of the Transmit Shift
   * Register; all transmit operations are completed
   ***************************************************************************/
  U1STAbits.UTXISEL0 = 1;
  U1MODEbits.UARTEN = 1;
  U1STAbits.UTXEN = 1;
}

void flush_uart1()
{
volatile unsigned int key;
int timeout = 0x7FFF;

  while ((U1STAbits.URXDA == 1) && (timeout--))
  {
    key = U1RXREG;
  }
  U1STAbits.OERR = 0;                     /* Clear overflow condition */
/*lint -e{438,550}*/ 
}

void UART1PutChar(const unsigned char Ch)
{

  while(U1STAbits.UTXBF == 1) {}  /* Wait if busy */
  U1TXREG = (unsigned int)Ch;
}

/**********************************************************************************************
 * send_backup_pkt()
 *   This routine will send a data packet to the backup processor
 *   Clear the receive data packet array  (communicate_RX_pkt; filled by _U1RXInterrupt)
 *   Set receive_bk_status = FALSE (Set TRUE by _U1RXInterrupt)
 *   Set rx_size = 0  (Incremented by _U1RXInterrupt on each data byte received)
 *   Set receive_data = 0 (Just in case garbage was received since last good message)
 **********************************************************************************************/
void send_backup_pkt(const unsigned char *pkt_ptr)
{
unsigned int i;

  for ( i=0; i<sizeof(communicate_RX_pkt); i++)
  {
    communicate_RX_pkt[i]=0;
  }
  receive_bk_status = FALSE;
  flush_uart1();
  rx_size = 0;
  receive_data = 0;
  IFS0bits.U1RXIF = 0;
  IEC0bits.U1RXIE = 1;

  for ( i=0; i<pkt_ptr[0]; i++)
  {
    UART1PutChar(pkt_ptr[i]);
      /* Give transmitter a little time */
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
  }
}

/*****************************************************************************
 * EOF
 *****************************************************************************/

