/*********************************************************************************************
 *
 *   Project:        Rack Controller Intellitrol
 *
 *   Module:         ENUM.H
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *   Description:    Main program Modbus enumerated list for Rack  controller main
 *                   microprocessor PIC24HJ256GP210.h
 *
 * Revision History:
 *   Rev      Date       Who   Description of Change Made
 * -------- ---------  ---      --------------------------------------------
 *              04/22/08  KLL  Started porting old Intellitrol enumerated list from the 
 *                                         MC68HC16Y1 cpu on the original Intellitrol
 * 1.5.31  08/10/14  DHP  Removed M_VAPORFLOW, VAPOR_FLOW, VAPOR_REF
 *
 *********************************************************************************************/
#ifndef ENUM_H
#define ENUM_H


/****************************************************************************/


typedef enum
{
    IDLE,       /* Waiting for something to happen */
    ACQUIRE,    /* Decide which probe type is present */
    ACTIVE,     /* Stay and allow permit while wiggling */
    GONE,       /* Truck gone, clean up and return to idle */
    FINI        /* Post-Gone, make sure cleaned up before idle */
} MAIN_STATE;

/****************************************************************************/

typedef enum
{
    IDLE_I,     /* Waiting for something to happen */
    OPTIC_5,    /* 5 wire optical probe truck */
    OPTIC_2,    /* 2 wire optical probe truck */
    THERMAL,    /* Thermistor (hot/cold) probe truck */
    GONE_NOW    /* Truck gone, clean up and return to idle */
} ACQUIRE_STATE;

/****************************************************************************/

typedef enum
{
    NO_TYPE,    /* No test in progress */
    OPTIC5,     /* 5 wire optical probe truck testing */
    OPTIC2,     /* 2 wire optical probe truck testing */
    THERMIS     /* Thermal (thermistor) probe truck testing */
} PROBE_TRY_STATE;

/****************************************************************************/

typedef enum
{
    NO5_TEST,      /* No test in progress */
    PULSED,        /* Pulse emitted, awaiting return */
    ECHOED,        /* Echo received */
    DIAG           /* Diagnostic Green wire present */
} OPTIC5_STATE;

typedef enum
{
    NO_TEST_2W,    /* No test in progress */
    SHORTCHK_2W,   /* Checking for shorts/etc. prior to permit */
    SHORTFAIL_2W,  /* Failed shorts test, wait for truck to disconnect */
    PULSING_2W     /* Pulses detected, determine how many */
} TWO_WIRE_STATE;

/****************************************************************************/

typedef enum
{
    UNKNOWN,       /* A truck Unknown Probes */
    THERMAL_TWO,   /* Thermal probe type is present */
    OPTIC_TWO,     /* 2 wire optical probe type is present */
    OPTIC_FIVE,    /* 5 wire optical probe type is present */
    DEPARTED,       /* Truck gone, clean up and return to idle */
    GONE_TWO
} TRUCK_STATE;

/****************************************************************************/

typedef enum
{               /* NOTE: states parallel probes_state */
    T_INIT,     /* 00 - Initial unknown status */
    T_WET,      /* 01 - Truck's wet -- DON'T permit */
    T_DRY,      /* 02 - Truck's dry -- OK to permit */
    T_COLD,     /* 03 - Cold Thermister probe */
    T_HOT,      /* 04 - Hot Thermister probe */
    T_OPEN,     /* 05 - Bad probe on truck open */
    T_X6,       /* 06 - Unused */
    T_X7,       /* 07 - Unused */
    T_X8,       /* 08 - Unused */
    T_X9,       /* 09 - Unused */
    T_FAULT,    /* 10 - Random/Bizarre/Unspecified fault (7vt oscs, etc.) */
    T_GROUND,   /* 11 - Probe/Channel shorted to ground */
    T_SHORT     /* 12 - Probe/Channel shorted to another probe/channel */
} TANK_STATE;

/****************************************************************************/

typedef enum
{               /* NOTE: states parallel tank_state */
    P_UNKNOWN,  /* 00 - Initial unknown status */
    P_WET,      /* 01 - Truck probe wet */
    P_DRY,      /* 02 - Truck probe dry */
    P_COLD,     /* 03 - Cold thermistor probe */
    P_HOT,      /* 04 - Hot thermistor probe */
    P_OPEN,     /* 05 - Bad probe on truck open */
    P_X6,       /* 06 - Unused */
    P_X7,       /* 07 - Unused */
    P_X8,       /* 08 - Unused */
    P_X9,       /* 09 - Unused */
    P_FAULT,    /* 10 - Random/Bizarre/Unspecified fault (7vt oscs, etc.) */
    P_GROUND,   /* 11 - Probe/Channel shorted to ground */
    P_SHORT     /* 12 - Probe/Channel shorted to another probe/channel */
} PROBES_STATE;

/****************************************************************************/

/****************************************************************************/

typedef enum
{                                       /*           ---- ADC Channel 1 ----  |  ---- ADC Channel 2 ---- */
    M_PROBES,                /* 00  PVOLT1 (Probe/Channel 1)   PVOLT2 (Probe/Channel 2) */
    M_UNUSED,               /* 01  Flow rate Thermistor               Reference Thermistor */
    M_RAW,                      /* 02  Raw 13.5 volt input                 Pad 8 */
    M_THREEX,                /* 03  3.5/3.8 bias voltage                 Divide 3.5 volt */
    M_DEADMAN,           /* 04  Pad 37                                    MDEADMAN */
    M_GND_7_8,              /* 05  GND_SENSE                         7/8 bit jumper */
    M_PARITY,                 /* 06  Parity jumper                           Baud rate jumper */
    M_ADDR                     /* 07  Address 1 jumper                    Address 10 jumper */
} SET_MUX;

/****************************************************************************/

typedef struct
{
    unsigned long  base_count;    /* expanded start time readout */
    unsigned long  rise_edge;     /* readout for rising edge */
    unsigned long  fall_edge;     /* readout for falling edge */
    unsigned long  rtn_pulse;     /* us return time */
    unsigned long  pulse_width;   /* us pulse width */
} OPT_PULSE;

/****************************************************************************/


/****************************************************************************/
typedef enum
{
    BRESERVED0, /* 00  Reserved (no parity) */
    BRESERVED1, /* 01  Reserved (odd parity) */
    BRESERVED2, /* 02  Reserved (even parity) */
    B01200,     /* 03   1200 */
    B02400,     /* 04   2400 */
    B04800,     /* 05   4800 */
    B09600,     /* 06   9600 */
    B19200,     /* 07  19200 */
    B38400,     /* 08  38400 */
    BRESERVED4, /* 09  Reserved (7-bit character) */
    BRESERVED5  /* 10  Reserved (8-bit character) */
} BAUD_RATE;
/****************************************************************************/

/****************************************************************************/
typedef enum
{
    NO_PARITY,       /* default to NO parity */
    ODD_PARITY,      /*  */
    EVEN_PARITY      /*  */
} PARITY;

/****************************************************************************/

/****************************************************************************/
typedef enum
{
   READY,                   // 0
   RECV,                      // 1
   PREPMSG,              // 2
   DECODEMSG,         // 3
   XMITPREP,              // 4
   XMITMSG,              // 5
   RESETMSG              // 6
} MODBUS_STAT;

/****************************************************************************/
// >>> QCCC 53
typedef enum
{
    P_NO_TYPE,    /* Unknown */
    P_OPTIC5,        /* 5 wire optical probe */
    P_OPTIC2,        /* 2 wire optical probe */
    P_THERMIS,     /* Thermistor probe */
    P_NONE            /* Possible 5-wire channels with no sensor */ 
} PROBE_TYPE;
// <<< QCCC 53
#endif        /* end of ENUM_H */
/*************************** end of enum ************************************/

