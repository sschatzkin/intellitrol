/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         init_timer.c
 *
 *   Revision:       REV 1.5
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

void Init_Timer1( void )
{
  /* declare temp variable for CPU IPL storing */
//  int current_cpu_ipl;

  /* ensure Timer 1 is in reset state */
  T1CON = 0;

  /* reset Timer 1 interrupt flag */
  IFS0bits.T1IF = 0;

  /* set Timer 1 interrupt priority level to 4 */
  IPC0bits.T1IP = 4;

  /* disable Timer 1 interrupt */
  IEC0bits.T1IE = 0;

  TMR1 = 0x00; // Clear timer count register

  /* set Timer 1 period register */
  PR1 = 0xFFFF;

  /******************************4/25/2008 7:42AM*****************************
   * Set Timer Input Clock Prescale Select bits to 1:1 prescale value
   ***************************************************************************/
  T1CONbits.TCKPS = NO_PRESCALE;

  /* select internal timer clock */
  T1CONbits.TCS = 0;

  /* restore CPU IPL value after executing unlock sequence */
//  RESTORE_CPU_IPL(current_cpu_ipl);

  /* enable Timer 1 and start the count */
  T1CONbits.TON = 1;
}

void Init_Timer2( void )
{
  /* ensure Timer 2 is in reset state */
  T2CON = 0;
  /* reset Timer 2 interrupt flag */
  IFS0bits.T2IF = 0;
  /* set Timer 2 interrupt priority level to 2 */
  IPC1bits.T2IP = 2;
  TMR2 = 0x00; // Clear timer count register
  /* set Timer 2 period register */
  PR2 = TIMER2_PERIOD;
  T2CONbits.TCKPS = NO_PRESCALE;
  /* select internal timer clock */
  T2CONbits.TCS = 0;
  /* enable Timer 2 interrupt */
  IEC0bits.T2IE = 1;
  /* enable Timer 2 and start the count */
  T2CONbits.TON = 1;
  loopEighths = 0;             /* Current 125ms counter */
}

void Init_Timer3( void )
{
  /* ensure Timer is in reset state */
  T3CON = 0;
  /* reset Timer 3 interrupt flag */
  IFS0bits.T3IF = 0;
  TMR3 = 0x00; // Clear timer count register
  /* set Timer 3 period register */
  PR3 = TIMER3_PERIOD;
  T3CONbits.TCKPS = NO_PRESCALE;
  /* select internal timer clock */
  T3CONbits.TCS = 0;
  /* set Timer 3 interrupt priority level to 4 */
  IPC2bits.T3IP = 4;
  /* enable Timer 3 interrupt */
  IEC0bits.T3IE = 1;
  /* enable Timer 3 and start the count */
  T3CONbits.TON = 0;
}

void Init_Timer4( void )
{
  /* declare temp variable for CPU IPL storing */
//  int current_cpu_ipl;

  /* ensure Timer 4 is in reset state */
  T4CON = 0;

  /* reset Timer 4 interrupt flag */
  IFS1bits.T4IF = 0;

  /* set Timer 4 interrupt priority level to 4 */
  IPC6bits.T4IP = 4;

  /* enable Timer 4 interrupt */
  IEC1bits.T4IE = 1;

  TMR4 = 0x00; // Clear timer count register

  /* set Timer 4 period register */
  PR4 = TIMER4_PERIOD;

  /******************************4/25/2008 7:42AM*****************************
   * Set Timer Input Clock Prescale Select bits to 1:1 prescale value
   ***************************************************************************/
  T4CONbits.TCKPS = NO_PRESCALE;

  /* select internal timer clock */
  T4CONbits.TCS = 0;
  /* enable Timer 4 and start the count */
  T4CONbits.TON = 1;
  set_porte( OPTIC_PULSE );  /* Output optic pulse of ~1500 us (0xDF) */
}



/******************************* 10/31/2008 6:20AM ***************************
 * Init 32bit timer pair 6 and 7
 *****************************************************************************/
void Init_32bit_Timer( void )
{
  T7CONbits.TON = 0;        // Stop any 16-bit Timer7 operation
  T6CONbits.TON = 0;        // Stop any 16/32-bit Timer7 operation
  T6CONbits.T32 = 1;        // Enable 32-bit Timer mode
  T6CONbits.TCS = 0;        // Select internal instruction cycle clock
  T6CONbits.TGATE = 0;      // Disable Gated Timer mode
  T6CONbits.TCKPS = 0b00;   // Select 1:1 Prescaler
  TMR7 = 0x00;              // Clear 32-bit Timer (msw)
  TMR6 = 0x00;              // Clear 32-bit Timer (lsw)
  PR7 = 0xFFFF;             // Load 32-bit period value (msw)
  PR6 = 0xFFFF;             // Load 32-bit period value (lsw)
  IPC12bits.T7IP = 0x02;    // Set Timer7 Interrupt Priority Level
  IFS3bits.T7IF = 0;        // Clear Timer7 Interrupt Flag
  IEC3bits.T7IE = 0;        // Disable Timer7 interrupt
  T6CONbits.TON = 1;        // Start 32-bit Timer
}


/*************************************************************************
*  subroutine:      read_time()
*
*  function:  Read the timer value and return it
*
*
*         1. Too simple right now - may be removed completely
*         2.
*         3.
*
*
*  input:  none
*  output: timer value
*
*
*************************************************************************/

unsigned long read_time()
{
    return( freetimer );

} /* end of read_time */

/*************************************************************************
*  subroutine:      read_realtime()
*
*  function:  Read the Timer 2 count value and return it
*
*
*         1.
*         2.
*         3.
*
*
*  input:  none
*  output: TMR2 timer value
*
*
*************************************************************************/

unsigned int read_realtime()
{
    return( TMR1/20 );    /* In Us */

} /* end of read_realtime */

/*************************************************************************
 *  subroutine: DeltaMsTimer
 *
 *  function:   Return difference in milliseconds
 *
 *              This routine returns the "delta" time in milliseconds be-
 *              tween the mstimer value passed as the argument and the
 *              current mstimer value. "Wrapping" of the 16-bit timer value
 *              is handled.
 *
 *  input:      "Previous" mstimer value
 *  output:     Difference in value between previous and current mstimer
 *
 *************************************************************************/

unsigned short DeltaMsTimer
    (
    unsigned short oldtime      /* Old ("previous") value of mstimer */
    )
{
    unsigned short curtime;     /* Current mstimer value (snapshot) */

    curtime = mstimer - oldtime;         /* Snapshot for calculations */

    if (!(curtime & 0x8000))            /* Time marching forward? */
        return (curtime);               /* Yes, return difference */
    else                                /* Time marching backwards? Snicker! */
        return ((unsigned short)0xFFFF + curtime + 1); /* Wrap it mod(16) */

} /* End DeltaMsTimer() */

/*************************************************************************
 *  subroutine: DeltaTCNT
 *
 *  function:   Return difference in TCNT cycles (approx 2us per count)
 *
 *              This routine returns the "delta" time in TCNT cycles be-
 *              tween the "reference" value passed as the argument and the
 *              current TCNT register value. "Wrapping" of the 16-bit timer
 *              value is handled.
 *
 *  input:      "Previous" TCNT value
 *  output:     Difference in value between previous and current TCNT
 *
 *************************************************************************/

UINT16 DeltaTCNT(unsigned short oldtime)  /* Old ("previous") value of TCNT */
{
    unsigned short curtime;     /* Current TCNT value (snapshot) */

    curtime = TMR1 - oldtime;         /* Snapshot for calculations */

    if (!(curtime & 0x8000))            /* Time marching forward? */
        return (curtime);               /* Yes, return difference */
    else                                /* Time marching backwards? Snicker! */
        return ((unsigned short)0xFFFF + curtime + 1); /* Wrap it mod(16) */

} /* End DeltaTCNT() */

/*************************************************************************
*  subroutine:      Debug_pulse()
*
*  function:  Pulse CS10 line at DEBUG connector
*
*  input:  unsigned int - is the time the pulse is held high in milli-seconds
*  output: none
*
*************************************************************************/

void debug_pulse(unsigned int x)
{
  printf("\n\rdebug_pulse: %u\n\r", x);
  LATGbits.LATG15 = 1;  /* Set SPDI1 a spare pin which is also on J10 pin 13 */
  DelayMS(x);
  LATGbits.LATG15 = 0;  /* Clear SPDI1 a spare pin */
} /* end of debug_pulse */

unsigned long read_32bit_realtime()
{
union
{
  struct {
    unsigned int low;
    unsigned int high;
  } temp;
  unsigned long lword;
} ul;

    ul.temp.low = TMR6;
    ul.temp.high = TMR7HLD;
    return  (ul.lword/(unsigned long)20 );    /* In Us */
} /* end of read_32bit_realtime */


