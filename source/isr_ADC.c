/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         isr_timer.c
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009  Scully Signal Company
 *
 *   Description:    Timer initialization routines for Rack controller, main
 *                   microprocessor PIC24HJ256GP210
 *
 *   Revision History:
 *
 *****************************************************************************/


#include "common.h"

/*******************************6/20/2008 3:02PM******************************
 * Function Name: ADCInterrupt
 * Description:   ADC Interrupt Handler
 * Inputs:        None
 * Returns:       None
 *****************************************************************************/
void __attribute__((__interrupt__, auto_psv)) _ADC1Interrupt( void )
{
int index;
unsigned int *adc_ptr;

  adc_ptr = (unsigned int *)ADC1BUF0;
  for ( index = 0; index<8; index++)
  {
    probe_volt[index] = *adc_ptr;
  }

	/* reset ADC interrupt flag */
	IFS0bits.AD1IF = 0;
}

