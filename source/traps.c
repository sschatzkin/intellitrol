/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         rino.c
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009  Scully Signal Company
 *
 *   Description:    Procedures to handle system error interrupts for the
 *                   main microprocessor PIC24HJ256GP210
 *
 *   Revision History:
 *
 *****************************************************************************/


#include "common.h"

void __attribute__((__interrupt__, auto_psv, auto_psv)) _OscillatorFail(void);
void __attribute__((__interrupt__, auto_psv, auto_psv)) _AddressError(void);
void __attribute__((__interrupt__, auto_psv, auto_psv)) _StackError(void);
void __attribute__((__interrupt__, auto_psv, auto_psv)) _MathError(void);
void __attribute__((__interrupt__, auto_psv, auto_psv)) _DMACError(void);
void __attribute__((__interrupt__, auto_psv, auto_psv)) _AltOscillatorFail(void);
void __attribute__((__interrupt__, auto_psv, auto_psv)) _AltAddressError(void);
void __attribute__((__interrupt__, auto_psv, auto_psv)) _AltStackError(void);
void __attribute__((__interrupt__, auto_psv, auto_psv)) _AltMathError(void);
void __attribute__((__interrupt__, auto_psv, auto_psv)) _AltDMACError(void);

/* Primary Exception Vector handlers:
   These routines are used if INTCON2bits.ALTIVT = 0.
   All trap service routines in this file simply ensure that device
   continuously executes code within the trap service routine. Users
   may modify the basic framework provided here to suit to the needs
   of their application. */

void __attribute__((__interrupt__, auto_psv)) _OscillatorFail(void)
{
        // last_routine = 0x1000;
        INTCON1bits.OSCFAIL = 0;        //Clear the trap flag
        if ( berr == TRUE)
        {
/* >>> DHP DEBUG */
          printf("\n\r    *** Oscillator Error occurred\n\r");
          DelayMS(10);
/* <<< DHP DEBUG */
          LATGbits.LATG13 = 0;  /* Been here before so reset */
        }
        berr = TRUE;
}

void __attribute__((__interrupt__, auto_psv)) _AddressError(void)
{
        // last_routine = 0x1001;
        INTCON1bits.ADDRERR = 0;        //Clear the trap flag
        if ( berr == TRUE)
        {
/* >>> DHP DEBUG */
          printf("\n\r    *** Address Error occurred\n\r");
          DelayMS(10);
/* <<< DHP DEBUG */
          LATGbits.LATG13 = 0;  /* Been here before so reset */
        }
        berr = TRUE;
}

void __attribute__((__interrupt__, auto_psv)) _StackError(void)
{
        // last_routine = 0x1002;
        INTCON1bits.STKERR = 0;         //Clear the trap flag
        if ( berr == TRUE)
        {
/* >>> DHP DEBUG */
          printf("\n\r    *** Stack Error occurred\n\r");
          DelayMS(10);
/* <<< DHP DEBUG */
          LATGbits.LATG13 = 0;  /* Been here before so reset */
        }
        berr = TRUE;
}

void __attribute__((__interrupt__, auto_psv)) _MathError(void)
{
        // last_routine = 0x1003;
        INTCON1bits.MATHERR = 0;        //Clear the trap flag
        if ( berr == TRUE)
        {
/* >>> DHP DEBUG */
          printf("\n\r    *** Math Error occurred\n\r");
          DelayMS(10);
/* <<< DHP DEBUG */
          LATGbits.LATG13 = 0;  /* Been here before so reset */
        }
        berr = TRUE;
}


void __attribute__((__interrupt__, auto_psv)) _DMACError(void)
{
        // last_routine = 0x1004;
		/* reset status bits, real app should check which ones */
		DMACS0 = 0;

        INTCON1bits.DMACERR = 0;        //Clear the trap flag
        if ( berr == TRUE)
        {
          LATGbits.LATG13 = 0;  /* Been here before so reset */
        }
        berr = TRUE;
}


/* Alternate Exception Vector handlers:
   These routines are used if INTCON2bits.ALTIVT = 1.
   All trap service routines in this file simply ensure that device
   continuously executes code within the trap service routine. Users
   may modify the basic framework provided here to suit to the needs
   of their application. */

void __attribute__((__interrupt__, auto_psv)) _AltOscillatorFail(void)
{
        // last_routine = 0x1005;
        INTCON1bits.OSCFAIL = 0;
        if ( berr == TRUE)
        {
          LATGbits.LATG13 = 0;  /* Been here before so reset */
        }
        berr = TRUE;
}

void __attribute__((__interrupt__, auto_psv)) _AltAddressError(void)
{
        // last_routine = 0x1006;
        INTCON1bits.ADDRERR = 0;
        if ( berr == TRUE)
        {
          LATGbits.LATG13 = 0;  /* Been here before so reset */
        }
        berr = TRUE;
}

void __attribute__((__interrupt__, auto_psv)) _AltStackError(void)
{
        // last_routine = 0x1007;
        INTCON1bits.STKERR = 0;
        if ( berr == TRUE)
        {
          LATGbits.LATG13 = 0;  /* Been here before so reset */
        }
        berr = TRUE;
}

void __attribute__((__interrupt__, auto_psv)) _AltMathError(void)
{
        // last_routine = 0x1008;
        INTCON1bits.MATHERR = 0;
        if ( berr == TRUE)
        {
          LATGbits.LATG13 = 0;  /* Been here before so reset */
        }
        berr = TRUE;
}


void __attribute__((__interrupt__, auto_psv)) _AltDMACError(void)
{
        // last_routine = 0x1009;
		/* reset status bits, real app should check which ones */
		DMACS0 = 0;

        INTCON1bits.DMACERR = 0;        //Clear the trap flag
        if ( berr == TRUE)
        {
          LATGbits.LATG13 = 0;  /* Been here before so reset */
        }
        berr = TRUE;
}
