/*********************************************************************************************
 *
 *  Project:	Intellitrol Rack Controller
 *
 *  Module:      Ledlite.h
 *
 *  Revision:    REV 1.5
 *
 *
 *  Author:  Ken Langlais
 *   @Copyright 2008, 2014  Scully Signal Company
 *
 *  Description: Led Display driver function prototype, declarations
 *              of the led state array, led functions (FLASH,DARK etc)
 *              and led names (COMPARTMENT_1,PERMIT etc)
 *
 * Revision History:
 *   Rev      Date           Who   Description of Change Made
 *  --------  --------------  ---    --------------------------------------------
 *  1.5       09/23/08  KLL  Ported old Intellitrol code to the PIC24HJ256GP210 cpu
 *  1.6.32  03/14/15  DHP  Replaced VAPOR_BAD, GROUND_BAD with GND_GOOD,
 *                                        GND_BAD, VAPOR_GOOD with FREELED26
 *
 *********************************************************************************************/
#ifndef LEDLITE_H
#define LEDLITE_H

/* Number of bits/LEDS to bang ("0" slot unused) -- i.e, number of defined
   "LED"s known/driveable. Currently, only 21 are in use.

   Note:  the MM5480 LED drive chip needs 35 "bits".  Not all these bits
          are connected to anything. (See ledtrans[] in SIM.C) */
void    init_led(void);                 /* One time initialization      */


/**************************************************************************
* led_action -- What each LED can do
*
* "DARK" is a special case of blinking; Blinking (FLASHxxx) is defined as
* LED is ON while FLASHxxx mask 'AND' LED cycle clock (see timer_heartbeat()
* routine); LITE is ON, and PULSE is a one-shot (the LED will pulse ON for
* one "cycle" (125Ms) and then automatically reset its state to DARK).
***************************************************************************/
//enum led_action
//{
//  DARK = 0,                   /* LED off [***MUST BE 0***] */
//  FLASH4HZ   = 0x01,          /* Flash  4Hz, 50% duty cycle */
//  FLASH2HZ   = 0x02,          /* Flash  2Hz, 50% duty cycle */
//  FLASH2HZ75 = 0x03,          /* Flash  2Hz, 75% duty cycle */
//  FLASH1HZ   = 0x04,          /* Flash  1Hz, 50% duty cycle */
//  FLASH1HZ75 = 0x06,          /* Flash  1Hz, 75% duty cycle */
//  FLASH1HZ87 = 0x07,          /* Flash  1Hz, 87% duty cycle */
//  FLASH_5HZ  = 0x08,          /* Flash .5Hz, 50% duty cycle */
//  FLASH_5HZ75= 0x0C,          /* Flash .5Hz, 75% duty cycle */
//  LITE       = 0x40,          /* LED on */
//  PULSE      = 0x41           /* Pulse once */
//};

#define DARK          0         /* LED off [***MUST BE 0***] */
#define FLASH4HZ      0x01      /* Flash  4Hz; 50% duty cycle */
#define FLASH2HZ      0x02      /* Flash  2Hz; 50% duty cycle */
#define FLASH2HZ75    0x03      /* Flash  2Hz; 75% duty cycle */
#define FLASH1HZ      0x04      /* Flash  1Hz; 50% duty cycle */
#define FLASH1HZ75    0x06      /* Flash  1Hz; 75% duty cycle */
#define FLASH1HZ87    0x07      /* Flash  1Hz; 87% duty cycle */
#define FLASH_5HZ     0x08      /* Flash .5Hz; 50% duty cycle */
#define FLASH_5HZ75   0x0C      /* Flash .5Hz; 75% duty cycle */
#define LITE          0x40      /* LED on */
#define PULSE         0x41      /* Pulse once */

/**************************************************************************
* led_name --  Which LED to do it to
*
*   Note:  The LED order is mapped into the MM5480 Led driver chip
*          by ledtrans[], see timer_heartbeat() routine.
*
***************************************************************************/

typedef enum
{
    COMPARTMENT_1=1,            /*  1 [Red] Channel/Probe #1 Status */
    COMPARTMENT_2,              /*  2 [Red] Channel/Probe #2 Status */
    COMPARTMENT_3,              /*  3 [Red] Channel/Probe #3 Status */
    COMPARTMENT_4,              /*  4 [Red] Channel/Probe #4 Status */
    COMPARTMENT_5,              /*  5 [Red] Channel/Probe #5 Status */
    COMPARTMENT_6,              /*  6 [Red] Channel/Probe #6 Status */
    COMPARTMENT_7,              /*  7 [Red] Channel/Probe #7 Status */
    COMPARTMENT_8,              /*  8 [Red] Channel/Probe #8 Status */
    DYNACHEK,                      /*  9 [Yel] DynaCheck(tm) "All-Is-Well" */
    OPTIC_OUT,                      /* 10 [Green] 5-Wire-Optic actively sending */
    OPTIC_IN,                          /* 11 [Green] 5-Wire-Optic actively responding */
    GND_GOOD,                      /* 12 [Green] */
    GND_BAD,                         /* 13 [Red] Ground Status on Intellitrol or [Green] Resistive Ground on Intellitrol2 */
    TRUCKCOMM,                  /* 14 [Yel] Communications with truck/TIM/etc. */
    TASCOMM,                    /* 15 [Yel] Communications with TAS (VIPER, etc.) */
    VIP_IDLE,                   /* 16 [Yel] Truck ID idle no activity */
    VIP_AUTH,                   /* 17 [Grn] Truck ID Authorized */
    VIP_UNAUTH,                 /* 18 [Red] Truck ID UnAuthorized */
    NONPERMIT,                  /* 19 [Red] Unit is not permitting */
    PERMIT,                     /* 20 [Grn] Unit is permitting */
    COMPARTMENT_9,              /* 21 [Red] Channel/Probe #9 Status */
    COMPARTMENT_10,             /* 22 [Red] Channel/Probe #10 Status */
    COMPARTMENT_11,             /* 23 [Red] Channel/Probe #11 Status */
    COMPARTMENT_12,             /* 24 [Red] Channel/Probe #12 Status */
    GROUND_GOOD,                /* 25 [GREEN] Good Ground Status */
    FREELED26,                  /* 26 [GREEN] */
    DEADMAN_GOOD,               /* 27 [Green] Good Deadman Status */
    DEADMAN_BAD,                /* 28 [Red] Bad Deadman Status */
    FREELED29
} LED_NAME;
#define NLEDBIT    FREELED29+1
#define GROUND_BAD      GND_BAD
#define GND_RESISTIVE   GND_BAD
#define GND_DIODE          GND_GOOD

/*      Write desired LED action into the proper element of ledstate
 *      for example, turn on the PERMIT led like this:
 *      ledstate[PERMIT] = LITE;
 *      Or flash the DynaCheck led like this
 *      ledstate[DYNACHEK] = FLASH50;
 */
#endif                          /* end of LEDLITE_H */
