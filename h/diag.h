/*********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         diag.h
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2008, 2014  Scully Signal Company
 *
 *   Description:    System "diagnostic" definitions
 *
  * Revision History:
 *   Rev      Date           Who   Description of Change Made
 *  --------  --------------  ---    --------------------------------------------
 *  1.5       09/23/08  KLL  Ported old Intellitrol code to the PIC24HJ256GP210 cpu
 *  1.5.31  08/10/14  DHP  Removed DIA_CHK_VAPOR define and use in DIA_CHK_INIT
 *                                      Renamed DIA_MEM_TESTS to DIA_CHK_MEM
 *********************************************************************************************/



/****************************************************************************
*
* Diagnostics() control mask definitions.
*
* These bits/flags control what the diagnostics() function tests.
*
****************************************************************************/

/* Check RAM -- future, more or less... */

#define DIA_CHK_RAM         0x0001


/* Check FLASH code space CRC */

#define DIA_CHK_FLASHCRC    0x0002

/******************************* 2/3/2010 2:51PM *****************************
 * Test the 16K Internal RAM
 *****************************************************************************/
#define DIA_CHK_MEM         0x0004

/* Check system clock */

#define DIA_CHK_CLOCK       0x0008

/* Check the "open-circuit" (aka "rail") voltages. As a side effect, check
   buncha other stuff as well, including set 6/8 compartment mode. Prolly
   otta be broken down into more than just "init_ADC()"! */

#define DIA_CHK_VOLTS0      0x0010


/* Check all the individual "point" voltages. This includes things like
   the "raw" input (measured as 13V), 5-wire-optic voltages, etc. */

#define DIA_CHK_VOLTS1      0x0020


/* Check all the channel/probe drive voltages. */

#define DIA_CHK_VOLTSCH     0x0040

/* Check the EEPROM structures */

#define DIA_CHK_EEPROM      0x0080

/* Check ground configuration */

#define DIA_CHK_GROUND      0x0100

/* Free                     0x0200 */
/* Free                     0x0400 */
/* Free                     0x0800 */

/* Flash the LEDs in the init signature */

#define DIA_CHK_LEDSIG      0x1000


/* Test/check enable jumpers et al */

#define DIA_CHK_JUMPERS     0x2000

/* Free                     0x4000 */
/* Free                     0x8000 */


/* Check pattern to do everything in reset/initialization */

#define DIA_CHK_INIT    (DIA_CHK_RAM | DIA_CHK_FLASHCRC | DIA_CHK_CLOCK | DIA_CHK_MEM \
    | DIA_CHK_VOLTS0 | DIA_CHK_VOLTS1 | DIA_CHK_VOLTSCH \
    | DIA_CHK_GROUND  | DIA_CHK_JUMPERS| DIA_CHK_LEDSIG)

/* Check pattern to use periodically in "idle" state, every few seconds */

#define DIA_CHK_IDLE   (DIA_CHK_MEM | DIA_CHK_EEPROM | DIA_CHK_VOLTS1 | DIA_CHK_VOLTSCH | DIA_CHK_GROUND)

/* Check pattern to do "some things" at "truck-gone" time
   Don't do the CRC as it takes too long and VIPER complains "No Response"... */

#define DIA_CHK_TRKGONE   (DIA_CHK_CLOCK  | DIA_CHK_MEM  \
    | DIA_CHK_VOLTS0 | DIA_CHK_VOLTS1 | DIA_CHK_VOLTSCH)

#define DIA_CHK_TRKFINI   (DIA_CHK_VOLTS0 | DIA_CHK_VOLTS1 | DIA_CHK_VOLTSCH)

/**************************** end of DIAG.H ********************************/
