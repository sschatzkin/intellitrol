/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         init_DMA.c
 *
 *   Revision:       REV 1.6
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2015  Scully Signal Company
 *
 *   Description:    DMA initialization routines for Rack controller, main
 *                   microprocessor PIC24HJ256GP210
 *
 *   Revision History:
 *   Rev      Date       Who   Description of Change Made
 * -------   --------- -----      --------------------------------------------
 * 1.6.03  03/10/15  DHP   In Init_DMA0() added set_mux(M_PROBES)
 *
 *****************************************************************************/
#include "common.h"

// Linker will allocate these buffers from the bottom of DMA RAM.
unsigned int BufferA[8] __attribute__((space(dma)));

/*******************************6/23/2008 11:43AM*****************************
 * DMA0 configuration
 * Direction: Read from peripheral address 0-x300 (ADC1BUF0) and write to DMA RAM
 * AMODE: Peripheral Indirect Addressing Mode
 * MODE: Continuous, Ping-Pong Mode
 * IRQ: ADC Interrupt
 *
 *****************************************************************************/
void Init_DMA0(void)
{
  DMA0CONbits.CHEN=0;         /* Disable DMA */
  IFS0bits.DMA0IF = 0;        /* Clear the DMA interrupt flag bit */
  /* set DMA0 interrupt priority level to 6 */
  IPC1bits.DMA0IP = 6;
  DMA0CONbits.AMODE = 0;      /* Configure DMA for Register Indirect with Post Increment mode */
  DMA0CONbits.MODE = 1;       /* Configure DMA for One-Shot, Ping-Pong mode disabled */
  DMA0PAD = (unsigned int) &ADC1BUF0;           /* Point DMA to ADC1BUF0 */
  DMA0CNT = 7;                /* 8 DMA request (8 buffers, each with 1 word) */
  DMA0REQ = 13;               /* Select ADC1 as DMA Request source */
  DMA0STA = (unsigned int)__builtin_dmaoffset(&BufferA[0]);
  IEC0bits.DMA0IE = 1;        /*Set the DMA interrupt enable bit  */
  set_mux(M_PROBES);           /* Point to probe voltage */}
