/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         write.c
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009  Scully Signal Company
 *
 *   Description:    The file contains write procedure. This replaces the write
 *                   library routine used by printf()
 *                   microprocessor PIC24HJ256GP210
 *
 *   Revision History:
 *
 *****************************************************************************/

#include "common.h"

// int __attribute__((__weak__, __section__(".libc")))
int __attribute__((, __section__(".libc")))
write(int handle, const void *buffer, unsigned int len)
{
unsigned int i;
volatile UxMODEBITS *umode = &U1MODEbits;
volatile UxSTABITS *ustatus = &U1STAbits;
volatile unsigned int *txreg = &U1TXREG;
volatile unsigned int *brg = &U1BRG;
unsigned char *temp_ptr = (unsigned char*)buffer;

  if (modbus_addr != 0)                   /* ASCII only if modbus address 0 */
    return (int)len;

  if ( len > 99)
  {
    asm volatile("nop");    /* place for the breakpoint to stop */
    asm volatile("nop");    /* place for the breakpoint to stop */
    asm volatile("nop");    /* place for the breakpoint to stop */
  }
  switch (handle)
  {
    case 0:
    case 1:
    case 2:
      if (__C30_UART != 1) {
        if ( TXEN == 0)
        {
          TXEN = 1;                       /* Enable transmit */
        }
        umode = (volatile UxMODEBITS *)(void*)&U2MODEbits;
        ustatus = (volatile UxSTABITS *)(void*)&U2STAbits;
        txreg = (volatile unsigned int *)&U2TXREG;
        brg = (volatile unsigned int *)&U2BRG;
      }
      if ((umode->UARTEN) == 0)
      {
        *brg = 0;
        umode->UARTEN = 1;
      }
      if ((ustatus->UTXEN) == 0)
      {
        ustatus->UTXEN = 1;
      }
      for (i = len; i; --i)
      {
        while ((ustatus->TRMT) ==0) {}
        *txreg = *temp_ptr++;
      }
      break;

    default: {
      break;
    }
  }
  while(U2STAbits.UTXBF == 1) {}
  DelayMS(1);
  TXEN = 0;
  flush_uart2();
  return (int)len;
}

