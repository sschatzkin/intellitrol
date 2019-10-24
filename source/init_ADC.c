/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         init_ADC.c
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009  Scully Signal Company
 *
 *   Description:    ADC initialization routines for Rack controller, main
 *                   microprocessor PIC24HJ256GP210
 *
 *   Revision History:
 *
 *****************************************************************************/


#include "common.h"

/*******************************6/4/2008 10:19AM******************************
 * Function Name: Init_Probe_ADC
 * Description:   Initialize ADC module to sample the voltage from all 8
 *                probes AN0 - AN7 once
 * Inputs:        None
 * Returns:       None
 *****************************************************************************/
void Init_ADC(void)
{

  // last_routine = 0x69;
  probe_result_flag = 0;    /* Clear  */
  if ((AD1CON1 & 0xBFF4) == 0x24E4)
  {
    return;       /* No need to do an init if already been done */
  }
  else
  {
    AD1CON1bits.ADON = 0;     /* turn off ADC module */

    IFS0bits.AD1IF = 0;       /* reset ADC interrupt flag */

    /******************************6/3/2008 9:34AM******************************
     * Setup AN0 to AN7 as analog inputs
     ***************************************************************************/
    AD1PCFGL = 0xFF00;
    AD1CON1 = 0x0000;
    AD1CON2 = 0x0000;
    AD1CON3 = 0x0000;
    AD1CON4 = 0x0000;         /* Allocate 1 word per analog input */
    AD1CHS0 = 0x0000;
    AD1CON2bits.SMPI = 7;     /* Select 7 conversions before interrupt */
    AD1CON1bits.FORM = 0;     /* Select unsigned interger */
    AD1CON1bits.ADSIDL = 1;   /* Discontinue module operation when device enters Idle mode */
    AD1CON1bits.AD12B = 1;    /* Select 12-bit mode, 1-channel mode */
    AD1CON1bits.ASAM = 1;     /* Enable Automatic Sampling */
    AD1CON2bits.CSCNA = 1;    /* Enable Channel Scanning */
    AD1CSSL = 0x00FF;         /* Select AN0 - AN7 for input scan */
    AD1CON1bits.SSRC = 7;     /* Internal counter ends sampling and starts */
                              /* conversion (auto-convert) */
    AD1CON2bits.BUFM = 0;     /* Single 8-word result buffer */
    AD1CON2bits.ALTS = 0;     /* Always use MUX A input select */
    /******************************6/4/2008 10:57AM*****************************
     * Tcy is 50ns. TAD = 500 us. SAMC is 6.5us or 13 * TAD
     * Set sample time for 6 TAD for 6us
     ***************************************************************************/
    AD1CON3bits.ADRC = 0;     /* ADC Clock is derived from Systems Clock  */
    AD1CON3bits.SAMC = 13;    /* ADC sample time is 6.5us - SAMC = 13 * TAD */
    AD1CON3bits.ADCS = 10;    /* ADC Conversion Clock is 500ns - TAD = Tcy * 10 */

    Init_DMA0();              /* Setup DMA0 */
    /* disable this interrupt (ADC1) because DMA is enabled */
    IEC0bits.AD1IE = 0;
  }
}


