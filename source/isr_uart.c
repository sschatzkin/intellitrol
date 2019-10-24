/****************************************************************************
 *
 *   Project:        Rack Controller Intellitrol
 *
 *   Module:         isr_uart.c
 *
 *   Revision:       REV 1.01
 *
 *   Author:         Ken Langlais  @Copyright 2009  Scully Signal Company
 *
 *   Description:    Main program Modbus routines for Rack  controller main microprocessor
 *                   PIC24HJ256GP210.h
 *
 * Revision History:
 *   Rev      Date    Who   Description of Change Made
 * -------- --------  ---  --------------------------------------------
 *         04/22/08  KLL  Started porting old Intellitrol Modbus interrupt code
 *                          from the MC68HC16Y1 CPU on the original Intellitrol.
 *         09/24/10  KLL  Modified U2RX and U2TX to correct problem with lost
 *                          messages.
 *                         Cleared TXEN at end of message to prevent collisions
 *                          at host.
 *                         Accept new message in RX if not transmitting or
 *                          preparing to transmit.
 *                         Supporting changes were made in modbus.c and uart.c
 *         04/01/14  DHP  Removed lines of commented out code.
 * 1.6.34  10/10/16  DHP  Renamed communicate_flag to receive_bk_status.
 * 1.6.35  02/07/17  DHP  Added parameter & overflow checks in U1RXInterrupt
 ****************************************************************************/
#include "common.h"

/**************************************************************************
         M O D B U S _ Interrupt routines

   Modbus communications only receives or transmits. It is illegal to
   transmit and receive simultaneously.

   The Modbus communications status is set based on this status infomation
   to permit executive loop checking of the Mobus communication status.

   Data bytes read with errors are ignored.

**************************************************************************/

void __attribute__((__interrupt__, auto_psv)) _U2RXInterrupt( void )
{
unsigned char data;

  do
  {
   /* The following was added to help get rid of lost messages */
    if (DeltaMsTimer(modbus_rx_time) > modbus_eom_time)
    { // Should get here on first character of every message
      if (modbus_err)
      {
        asm volatile("nop");    /* place for the breakpoint to stop */
        asm volatile("nop");    /* place for the breakpoint to stop */
        asm volatile("nop");    /* place for the breakpoint to stop */
      }
      modbus_state   = (unsigned char)RECV;
      modbus_tx_len  = 0;
      modbus_tx_ptr  = modbus_tx_buff;
      for (modbus_rx_ptr = modbus_rx_buff; modbus_rx_ptr < (modbus_rx_buff + MODBUS_MAX_LEN); modbus_rx_ptr++)
        *modbus_rx_ptr = 0;
      modbus_rx_ptr  = modbus_rx_buff;
      modbus_rx_len = 0;
    }
    /* End of added code */
    if (modbus_state != (unsigned char)RECV)     /* If we are not in receive mode */
    {
    /* The following added to help get rid of lost messages */
      if (modbus_state == (unsigned char)READY)
      {

        modbus_state   = (unsigned char)RECV;
        modbus_tx_len  = 0;
        modbus_tx_ptr  = modbus_tx_buff;
        for (modbus_rx_ptr = modbus_rx_buff; modbus_rx_ptr < (modbus_rx_buff + MODBUS_MAX_LEN); modbus_rx_ptr++)
          *modbus_rx_ptr = 0;
        modbus_rx_ptr  = modbus_rx_buff;
        modbus_rx_len = 0;
      }
      else
      {
        asm volatile("nop");    /* place for the breakpoint to stop */
        asm volatile("nop");    /* place for the breakpoint to stop */
        asm volatile("nop");    /* place for the breakpoint to stop */
      }
    }
    /* End of added code */

    if ((U2STAbits.PERR) || (U2STAbits.FERR) || (U2STAbits.OERR) || (modbus_rx_len >= MODBUS_MAX_LEN))
    {

      data = (unsigned char)U2RXREG;             /* Clear out PERR  */
      U2STAbits.OERR = 0;       /* Clear the Overflow bit */
      modbus_state = (unsigned char)RESETMSG;
      modbus_Recv_err++;
    }
    else
    {
      data = (unsigned char)U2RXREG;             /* Read data in from Receive Data Reg */
      modbus_rx_len++;
      *modbus_rx_ptr++ = data;
    }
    modbus_rx_time  = mstimer;        /* Mark the time */
  }
  while(U2STAbits.URXDA == 1);
  IFS1bits.U2RXIF = 0;
}

void __attribute__((__interrupt__, auto_psv)) _U2TXInterrupt( void )
{

  if (modbus_state == (unsigned char)XMITMSG)
  {
    IFS1bits.U2TXIF = 0;
    if (modbus_tx_len > 0)
    {
      send_char++;
      modbus_tx_len--;

      U2TXREG = *(modbus_tx_ptr++);
      ledstate[TASCOMM] = PULSE;   /* Note TAS/VIPER comm activity */
    }
    else
    {
      while(U2STAbits.TRMT == 0) {} /* Wait for transmit shift register to empty */
      IEC1bits.U2TXIE = 0;
      modbus_state = (unsigned char)RESETMSG;
      /* DHP  The following Block added to prevent host collisions */
      {
        U2STAbits.UTXEN = 0;            /* To force Int when set in set_tx2_en() */
        IEC1bits.U2TXIE = 0;
        IFS1bits.U2TXIF = 0;
        TXEN = 0;                       /* Disable transmit at board level */
        flush_uart2();                  /* Remove any received characters */
        IFS1bits.U2RXIF = 0;
        IEC1bits.U2RXIE = 1;            /* Now ready for receive */
      }
      /* End of added code */
    }
  }
}

/******************************* 11/12/2009 6:06AM ***************************
 * This interrupt receives information from the backup processor
 * Packet must have the first byte the size and the second byte the CMD byte
 * The rest of the bytes must be data and the entire packet must not exceed
 * 20 bytes
 *****************************************************************************/

void __attribute__((__interrupt__, auto_psv)) _U1RXInterrupt( void )
{
    if ( U1STAbits.OERR)
    {
      U1STAbits.OERR = 0;       /* Clear the Overflow bit */
    }
  do
  {
    receive_data = U1RXREG;         /* Read data in from Uart Data Reg */
    communicate_RX_pkt[rx_size] = (unsigned char)receive_data;
    rx_size++;
    if (rx_size >= 20)
    {
      rx_size = 0;
      break;
    }
  } while(U1STAbits.URXDA == 1);  /* Make sure the receive UART is empty before leaving the Interrupt */

  if (rx_size >= communicate_RX_pkt[0])
  {
    rx_size = 0;
    receive_bk_status = TRUE;
    IEC0bits.U1RXIE = 0;
  }
  IFS0bits.U1RXIF = 0;      /* Clear the Interrupt */
}
/*****************************************************************************
 * EOF
 *****************************************************************************/


