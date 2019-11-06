/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         evlog.h
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2008  Scully Signal Company
 *
 *   Description:    Event Log "Record" entry layouts
 *
 *   Revision History:
 *
 *****************************************************************************/
#ifndef EVLOG_H
#define EVLOG_H


/**************************************************************************
*
* Event Log Record entry layout (See ESQUARED.H for E2LOGREC)
*
*           +----------------+
*   0000/   |    Type        |
*           +----------------+
*   0001/   |   Subtype      |
*           +----------------+
*   0002/   |   Repeat       |
*   0003/   |    Mask        |
*           +----------------+
*   0004/   |   Record       |
*   0005/   |    Entry       |
*   0006/   |    Time        |
*   0007/   | (32-bit UCT)   |
*           +----------------+
*   0008/   |   Type-        |
*           ~    Specific    ~
*   001D/   |     Info       |
*           +----------------+
*   001E/   |    CRC-16      |
*           +----------------+
*
***************************************************************************/

/**************************************************************************
* Event Type definitions and related Type-Specific "Info" structures
***************************************************************************/


/***********************************************************************
   EEPROM Erase/Init/Format. The subcode is the EEV_* mask of which EEPROM
   block was erased/inited (with EEV_HOME meaning the entire EEPROM was
   "formatted").

   The info block is the same as the RESET block, for now (I'm open to
   other suggestions...)

   EVINIT events are "mergeable".
***********************************************************************/

#define EVINIT          0x01

typedef struct
{
    unsigned        HardwareVer;    /* Hardware version */
    unsigned        KernelVer;      /* Kernel version */
    unsigned        ShellVer;       /* Shell version */
    unsigned        Jumpers;        /* System jumpers (ConfigA) */
    unsigned        Config2;        /* Enables/etc. (ConfigB) */
    char            future[12];     /* Reserved for future expansion */
} EVI_INIT;


/***********************************************************************
   System Reset/Restart. The subcode is the reset type, as derived from the
   HC16's Reset Status Register.

  Bit 7
***********************************************************************/


#define EVRESET         0x02

typedef struct
{
    unsigned        HardwareVer;    /* Hardware version */
    unsigned        KernelVer;      /* Kernel version */
    unsigned        ShellVer;       /* Shell version */
    unsigned        Jumpers;        /* System jumpers (ConfigA) */
    unsigned        Config2;        /* Enables/etc. (ConfigB) */
    unsigned        EEPROM_Stat;
    char            future[10];     /* Reserved for future expansion */
} EVI_RESET;



/***********************************************************************
   Bypass Activity. The subcode is the 'bylevel' mask bit(s).

   Bypass events can be "merged".
***********************************************************************/

#define EVBYPASS        0x03

typedef struct
{
    char        Key[BYTESERIAL]; /* Bypass Key Serial Number */
    char        Truck[BYTESERIAL]; /* Truck Serial Number */
    char        future[22 - (2*BYTESERIAL)]; /* Reserved for future expansion */
} EVI_BYPASS;



/***********************************************************************
   System Hardware Error. The subcode is the type of hardware error fault.

   The info block depends on the subcode.

   Hardware Error events can be "repeated".
***********************************************************************/

#define EVHDWERR        0x04

/* System Hardware Error -- Kernel/Shell CRC-16 failure */

#define EVHDW_CRC       0x00

typedef struct
{
    unsigned        KernelGood;     /* Proper Kernel CRC-16 */
    unsigned        KernelReal;     /* Failed Kernel CRC-16 */
    unsigned        ShellGood;      /* Proper Shell CRC-16 */
    unsigned        ShellReal;      /* Failed Shell CRC-16 */
    char            future[14];     /* Reserved for future */
} EVI_HDW_CRC;


/* System Hardware Error -- Relay fault */

#define EVHDW_RELAY     0x01

typedef struct
{
    char        BackupSt;       /* Backup Relay state */
    char        MainSt;         /* Main Relay state */
    char        future[20];     /* Reserved for future */
} EVI_HDW_RELAY;

/* System Hardware Error -- EEPROM failure */

#define EVHDW_EEPROM    0x02

typedef struct
{
    unsigned        EE_status;      /* Offending status */
    unsigned        StatusA;        /* In case something else bad */
    unsigned        StatusB;        /* In case something else bad */
    char            future[16];     /* Reserved for future */
} EVI_HDW_EEPROM;


/***********************************************************************
   Voltage Error. The subcode is the mask of voltage errors from iambroke
   (the low 8 bits).

   The info block Ch1-Ch8 voltages actually depend on the type of error being
   logged. In priority order:

        TOL10V_FAULT        The "open-circuit" 10V readings
        TOL20V_FAULT        The "open-circuit" 20V "JumpStart" readings
        NOISE_FAULT         The "open-circuit" 10V noise ("variance") values

   Voltage Error events can be "repeated".
***********************************************************************/

#define EVOLTERR        0x05

typedef struct
{
    unsigned        Raw13;          /* "Raw 13V" value */
    unsigned        RefVolt;        /* One-volt "reference" */
    unsigned        BiasVolt;       /* Probe bias voltage */
    unsigned        Ch1;            /* Channel 1 { 10V | 20V | noise } voltage */
    unsigned        Ch2;            /* Channel 2 { 10V | 20V | noise } voltage */
    unsigned        Ch3;            /* Channel 3 { 10V | 20V | noise } voltage */
    unsigned        Ch4;            /* Channel 4 { 10V | 20V | noise } voltage */
    unsigned        Ch5;            /* Channel 5 { 10V | 20V | noise } voltage */
    unsigned        Ch6;            /* Channel 6 { 10V | 20V | noise } voltage */
    unsigned        Ch7;            /* Channel 7 { 10V | 20V | noise } voltage */
    unsigned        Ch8;            /* Channel 8 { 10V | 20V | noise } voltage */
} EVI_VOLTS;



/***********************************************************************
   Impact Sensor triggered. The subcode is currently ignored.

   Impact events can be "repeated".
***********************************************************************/

#define EVIMPACT        0x06

typedef struct
{
    unsigned char   Key[BYTESERIAL]; /* Bypass Key Serial Number */
    unsigned char   Truck[BYTESERIAL]; /* Truck Serial Number */
    unsigned char   future[22 - (2*BYTESERIAL)]; /* Reserved for future expansion */
} EVI_IMPACT;

#define EEOVERFILL      0x07

typedef struct                      /* Must be 22 bytes */
{
    unsigned char probe;              /* What kind of probe was wet */
    unsigned char probes_state[16];   /* Report probe state */
    unsigned char Truck[5];           /* Truck Serial Number */
} EEOVF_INFO;                         /* OVF OverFill */

#define EVIMAINTENANCE  0x08

typedef struct
{
    unsigned int    ch5_high_resistance;    /* Channel 5 resistance is higher than expected calc_tank() */
    char            future[20];             /* Reserved for future */
} EVI_MAINTENANCE;

#define EE_ERR_RESET    0x09

typedef struct
{
    unsigned char   Config;
    unsigned int    StatusA;
    unsigned int    StatusB;
    unsigned int    Iambroke;
    unsigned int    Iamsuffering;
    unsigned int    RefVolt;
    unsigned char   Ground;
    char            Main_state;
    char            Truck_state;
    char            Acquire_state;
    char            Probe_try_state;
    char            Five_wire_state;
    char            Two_wire_state;
    char            B_Relay_state;
    char            M_Relay_state;
    char            future[2];             /* Reserved for future */
} EE_ERR_RESET_INFO;

#define EEOVERFILL2  0x0A

typedef struct                      /* Must be 22 bytes */
{
    unsigned long active;
    unsigned char probes_state[8];  /* Report probe states */
    unsigned int  reason;           /* no-permit code */
    unsigned int  low_volts;
    unsigned int  high_volts;
    unsigned int  RefVolt;
    unsigned char probe;            /* What kind of probe was wet */
    char          future;           /* Reserved for future */
} EEOVF2_INFO;                      /* OVF OverFill */
/*********************  End of EVLOG.H  ***********************************/
#endif      /* end of EVLOG_H */
