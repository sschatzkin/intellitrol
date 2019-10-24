/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         isr_DMA.c
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009  Scully Signal Company
 *
 *   Description:    DMA Interrupt routines for Rack controller, main
 *                   microprocessor PIC24HJ256GP210
 *
 *   Revision History:
 *
 *****************************************************************************/

#include "common.h"

extern unsigned int BufferA[];

/*******************************6/20/2008 3:02PM******************************
 * Function Name: ADCInterrupt
 * Description:   ADC Interrupt Handler
 * Inputs:        None
 * Returns:       None
 *****************************************************************************/
void __attribute__((__interrupt__, auto_psv)) _DMA0Interrupt( void )
{
  probe_result_flag = 0;    /* Clear  */

  result_ptr[0] = (BufferA[0] & 0xFFF);
  result_ptr[1] = (BufferA[1] & 0xFFF);
  result_ptr[2] = (BufferA[2] & 0xFFF);
  result_ptr[3] = (BufferA[3] & 0xFFF);
  result_ptr[4] = (BufferA[4] & 0xFFF);
  result_ptr[5] = (BufferA[5] & 0xFFF);
  result_ptr[6] = (BufferA[6] & 0xFFF);
  result_ptr[7] = (BufferA[7] & 0xFFF);

  dma_result_flag = 1;        /* Indicate the reading of the probe voltages is complete */

  DMA0CONbits.CHEN = 0;     /* Disable DMA0 */
  AD1CON1bits.ADON = 0;     /* turn off ADC module */
  IFS0bits.DMA0IF = 0;      /* Clear the DMA0 Interrupt Flag */
}

