/*********************************************************************************************
 *
 *   Project:        Intellitrol Rack Controller
 *
 *   Module:         STSBITS.H
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2008  Scully Signal Company
 *
 *   Description:    Global state "Status" bits set/cleared all over the place,
 *                   #included in many modules, and destined to be read by the
 *                   remote VIPER/TAS programs via ModBus.
 *
 * Revision History:
 *   Rev      Date           Who   Description of Change Made
 *  --------  --------------  ---    --------------------------------------------
 *  1.5       09/23/08  KLL  Ported old Intellitrol code to the PIC24HJ256GP210 cpu
 *  1.5.30  08/10/14  DHP  Removed STSA_VAPOR_FLOW
 *                                      Added STSTA_DIODE_GND, STSA_RESISTIVE_GND
 *
 *********************************************************************************************/
#ifndef STSBITS_H
#define STSBITS_H

/****************************************************************************
*
*   The "Config A" bits
*
* Config "A" is the collection of "hardware/jumpers" configuration flags. It
* is made up of the "enable/jumpers" byte (basically PORTGP) and the ConfigA
* byte to present to the TAS/VIPER program a 16-bit "hardware" configuration
* ModBus register.
*
* Config "B" parallels Config "A" as software-enable flags (and does not have
* all "A" flags, for instance, no 6/8-compartment software control).
*
****************************************************************************/

/* 6/8 Compartment mode -- 6 compartments ("USA") configuration if clear, 8
   compartments ("CANADA", and rest of world...) if set */

#define CFGA_8COMPARTMENT   0x01

/* European 8.2-volt jumper (intuited from channel voltages...) */

#define CFGA_EURO8VOLT      0x02

/* European/etc. "100-Ohm" ground (in lieu of "GroundBolt" diode) */

#define CFGA_100_OHM_GND      0x04

/* Free                     0x08 - 0x40 */

/* Copy of DEBUG jumper (which normally lives in the Status-A bits but
   is copied here so that it gets logged on a system reset. */

#define CFGA_DEBUG          0x80

/****************************************************************************
*
* The "Status A" bits (aka "Input Status" bits 0 - 15).
*
* StatusA is the general 16-bit status of the Intellitrol unit. These
* bits reflect the general/operational status "at a glance" of the Intelli-
* trol unit as a whole.
*
****************************************************************************/

/* Some sort of unit-detected problem exists; cannot run properly (e.g., will
   not permit, but running so that TAS/VIPER can interrogate status). If set,
   unit is non-permissive (but note: hotwired relays are, by definition, in a
   "PERMIT" state). */
#define STSA_FAULT          0x0001

/* Something is connected to the 10-wire bus */
#define STSA_TRK_PRESENT    0x0002

/* Communications established with something on the bus, TIM, Intellitrol,
   SculNet devices, etc. If set, at least one "Truck Serial Number" has been
   successfully read. */
#define STSA_TRK_TALK       0x0004

/* At least one Truck Serial Number is "Authorized" by EEPROM/TAS/etc. */
#define STSA_TRK_VALID      0x0008

/* Intellitrol has some variety of bypass condition in effect */
#define STSA_BYPASS         0x0010

/* Intellitrol is idle (non-permit, no truck, no fault, etc.) */
#define STSA_IDLE           0x0020

/* Intellitrol is permissive (may be bypassed) */
#define STSA_PERMIT         0x0040

/* Intellitrol non-permissive due to a bypassable condition. This maps onto
   the VIP's "UNAUTHORIZED" state. */
#define STSA_BYPASSABLE     0x0080

/* 'DEBUG' jumper - ??? */
#define STSA_DEBUG          0x0100

/* Channel 5 diag line resistance is higher than expected - calc_tank()*/
#define CH5_HIGH_RESISTANCE 0x0200

/* Free                     0x0400 */
/* Free                     0x0800 */

/* Deadman is closed */
#define STSA_DED_CLOSED     0x1000

/* Ground detection determined a good diode or resistive ground connection */
#define STSA_DIODE_GND      0x2000

#define STSA_RESISTIVE_GND  0x4000

#define STSA_INTELLICHECK   0x8000
/* ... end redo */

/****************************************************************************
*
*   The "Status B" bits (aka "Input Status" bits 16 - 31).
*
* StatusB is the exception 16-bit status of the Intellitrol unit. In
* general, Status B bits are "hard faults" which will cause STSA_FAULT
* to be set.
*
* The bits are numbered funny in order to map the 16-bit Status A/B/O/P
*"registers" onto the "Input/Output Status Bits" byte streams (in essence,
* byte-reversed integers.).
*
* The order of #define's below is "Input Status Bits" 16 - 31.
*
****************************************************************************/

/* FREE                     0x0001 */

/* Error reading/writing onboard EEPROM */

#define STSB_ERR_EEPROM     0x0002

/* Error with A/D converter(s) */

#define STSB_ADC_TIMEOUT    0x0004

/* ERPOM/FLASH 'Shell' error (bad Checksum/CRC) */

#define STSB_CRC_SHELL      0x0008

/* Problem with onboard (battery-backed) Dallas clock */

#define STSB_BAD_CLOCK      0x0010

#define STSB_BAD_CPU        0x0020

#define STSB_TRUCK          0x0040

/* EPROM/FLASH 'Kernel' error (bad Checksum/CRC) */

#define STSB_CRC_KERNEL     0x0080

/* Problem with voltage(s) -- input A/C too low/high, channel noise, etc. */

#define STSB_ERR_VOLTAGE    0x0100

/* Free                     0x0200 */

#define STSB_ERR_TIMDATA    0x0400

/* Free                     0x0800 */

#define STSB_GRND_FAULT     0x1000

/* Intellitrol in some "special" operation mode, and thus, while running,
   non-operational in the sense it will not permit due to the special cir-
   cumstances (e.g., ENA_ADD_1_KEY, etc.). */

#define STSB_SPECIAL        0x2000

/* Intellitrol in "forced non-permit" (aka "Shutdown") by TAS command */

#define STSB_SHUTDOWN       0x4000

/* Problem with relays -- shorted, won't close, etc.  Note if both relays
   are "shorted" the unit is "hotwired" and therefore "permissive" and thus
   unsafe; Flash BigRed to gain attention. */

#define STSB_BAD_RELAY      0x8000


/* StatusB flags which mean some sort of error condition is currently present
   that contraindicate going into acquire mode (i.e., unit will never leave
   its "Idle" state). */

#define STSB_NO_ACQUIRE     (STSB_ADC_TIMEOUT | STSB_TRUCK \
    | STSB_CRC_SHELL | STSB_CRC_KERNEL | STSB_BAD_CPU \
    | STSB_ERR_VOLTAGE)


/* StatusB flags which mean some sort of NEVER EVER PERMIT condition is in
   effect, may be deliberate (SHUTDOWN) or system problem (bad voltage).
   Other "problems" which do not affect the *SAFE* operation of the unit
   merely light the Fault LED, like problems with the clock or EEPROM (and
   presumably get the attention of any TAS/VIPER/passing human). */

#define STSB_NO_PERMIT      (STSB_ADC_TIMEOUT | STSB_TRUCK \
    | STSB_CRC_SHELL | STSB_CRC_KERNEL | STSB_BAD_CPU \
    | STSB_ERR_VOLTAGE \
    | STSB_SPECIAL | STSB_SHUTDOWN)

/****************************************************************************
*
*   The "Status O" bits (aka "Output Status" bits 0 - 15).
*
* StatusO
* The first three bits are VIP-defined...
*
* The bits are numbered funny in order to map the 16-bit Status A/B/O/P
*"registers" onto the "Input/Output Status Bits" byte streams (in essence,
* byte-reversed integers! -- See mbcRdIStatus() for example).
*
* The order of #define's below is "Output Status Bits" 0 - 15.
*
****************************************************************************/

/* Reserved/no use by Intellitrol */

#define STSO_STANDBY        0x0001
#define STSO_AUTHORIZED     0x0002
#define STSO_UNAUTHORIZED   0x0004
/* Reserved                 0x0008 */

/* Intellitrol is pulsing 5-Wire-Optic Out (Channel 4) */

#define STSO_5WIRE_PULSE    0x0030
#define STSO_5WIRE_PULSE1   0x0010
#define STSO_5WIRE_PULSE2   0x0020

/* Intellitrol received 5-Wire-Optic "Echo" (Channel 6) response to pulse */

#define STSO_5WIRE_ECHO     0x00C0
#define STSO_5WIRE_ECHO1    0x0040
#define STSO_5WIRE_ECHO2    0x0080

/* Intellitrol is communicating with vehicle (TIM, Intellitrol, etc.) */

#define STSO_COMM_VEHICLE   0x0300
#define STSO_COMM_VEHICLE1  0x0100
#define STSO_COMM_VEHICLE2  0x0200

/* Don't need "TAS" communications ... */

/* Reserved                 0x0400 - 0x8000 */

/****************************************************************************
*
*   The "Status P" bits (aka "Output Status" bits 16-31)
*
* StatusP is currently "undefined" and not used by Intellitrol; however
* it is VIP defined...as the "S-record" count from the last download.
*
****************************************************************************/

#endif      /* end of STSBITS_H */
/************************** end of STSBITS.H *******************************/

