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
 *   Description:    Timer interrupt routines for Rack controller, main
 *                   microprocessor PIC24HJ256GP210
 *
 *   Revision History:
 *
 *****************************************************************************/

#include "common.h"

/*******************************6/17/2008 6:06AM******************************
 * Function Name: _T2Interrupt
 * Description:   Timer2 Interrupt Handler. Interrupt every 1ms
 * Inputs:        None
 * Returns:       None
 *****************************************************************************/
void __attribute__((__interrupt__, auto_psv)) _T2Interrupt( void )
{
   timer_heartbeat();
  /* reset Timer 2 interrupt flag */
  IFS0bits.T2IF = 0;
}

/*******************************6/17/2008 6:06AM******************************
 * Function Name: _T3Interrupt
 * Description:   Timer3 Interrupt Handler. Interrupt every 1ms
 * Inputs:        None
 * Returns:       None
 *****************************************************************************/
void __attribute__((__interrupt__, auto_psv)) _T3Interrupt( void )
{
  /* reset Timer 3 interrupt flag */
  IFS0bits.T3IF = 0;
  read_probes();                     /* read A/D convert into probe_volt */
}

/*******************************6/17/2008 6:06AM******************************
 * Function Name: _T4Interrupt
 * Description:   Timer4 Interrupt Handler. Interrupt every 1.5ms
 * Inputs:        None
 * Returns:       None
 *****************************************************************************/
void __attribute__((__interrupt__, auto_psv)) _T4Interrupt( void )
{
  /* reset Timer 4 interrupt flag */
  IFS1bits.T4IF = 0;
  T4CONbits.TON = 0;      /* Turn off Timer 4 */
  set_porte( OPTIC_DRIVE );  /* shut off optic pulse */
  /* We have just transmitted an "Optic Pulse". Reflect this detail in
     the OPTIC_OUT LED. Since active 5-wire optic cycles at 50ms intervals,
     and the LEDs cycle at 125ms, this will result in "steady on" while
     actively running 5-wire probes.
     DIAG: Suppress indication (both unit LED and TAS/VIPER) if this is
     the idle loop's playing with the signals -- only "flash the LED"
     when a truck is connected (includes "Acquire" state). */
}

/*******************************6/17/2008 6:06AM******************************
 * Function Name: _T5Interrupt
 * Description:   Timer5 Interrupt Handler. Interrupt every 500us
 * Inputs:        None
 * Returns:       None
 *****************************************************************************/
void __attribute__((__interrupt__, auto_psv)) _T5Interrupt( void )
{
  /* reset Timer 5 interrupt flag */
  IFS1bits.T5IF = 0;
}

