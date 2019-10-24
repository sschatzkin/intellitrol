/*********************************************************************************************
 *
 *  Project:     Intellitrol Rack Controller
 * 
 *  Module:      STDSYM.H  
 *
 *  Author:      Ken Langlais (KLL); edits by Dave Paquette (DHP)
 *               @Copyright 2009, 2024  Scully Signal Company
 *
 *  Description:  Global symbols, symbolic error codes and other stuff
 *                #included in every octopus module.
 *
 * Revision History:
 *   Rev      Date    Who   Description of Change Made
 * -------- --------  ---  --------------------------------------------
 *  1.5.27  04/11/14  DHP  Added EU_GND_REF to SysParmNV
 *  1.5.30  08/10/14  DHP  Added INTELLITROL_PIC and INTELLITROL2, GND_REF_LOW,
 *                          GND_REF_DEFAULT,  GRN_REF_EU, GND_REF_HIGH
 *                          Removed VaporTimeOut, Ref_Diff_Voltage
 *  1.6.34  08/05/16  DHP  Corrected variable type for free in SysParmNV to allow for correct
 *                          CRC checking in nvSysInit()
 *  1.6.35  02/07/17  DHP  Added SysParmNV entries for Active deadman
 *********************************************************************************************/
#ifndef STDSYM_H
#define STDSYM_H

#define INTELLITROL_PIC     3       // Standard PIC CPU with form function of Intellitrol
#define INTELLITROL2        4       // Intellitrol with modified front panel and functions

#define SET     1
#define CLR     0

typedef int bool;

#define TRUE    1
#define FALSE   0

#define BAD     1
#define FAILED  1
#define PASSED  0
#define GOOD    0

#define ON      1
#define OFF     0

#define SUCCESS 0
#define FAILURE -1

#define TEN     0                           /* Index to 10 volt open_c_volt */
#define TWENTY  1                        /* ditto for 20 volt */

#define NOMINAL10   10700               /* Millivolts */
#define NOMINAL20   20000               /* Millivolts */

#define EOS     0                           /* ASCII file termination */
#define CR      '\n'
#define DUMMY   0

/* Initial "Seed" value for CRC-16 calculations (see modbus.c) */

#define INIT_CRC_SEED   0xFFFF

typedef char byte;
typedef unsigned int word;
typedef unsigned short UINT16;
typedef unsigned long UINT32;
typedef unsigned char UINT8;
typedef unsigned long unixtime;         /* seconds since 1 Jan 1970 */
typedef unsigned long adctime;          /* A/D time. One tick = 3 msec */

typedef unsigned short volatile USV;
typedef char volatile UCV;
typedef int flagbits;                   /* flag word with the following bypass bits in it */

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/*  Touch Rite status codes     */

#define BUSY        -1
#define NOTBUSY     0


#define DEADCLOCK         0x7FFFFFFFL /* Time returned if Dallas clk inop*/
#define BYTESERIAL        6           /* length of Dallas serial num  */
#define COMPARTMENT_SIZE  4           /* number of bytes that store a compartment volume */

/*
 *  Symbolic Error Codes Error. Codes 1-8 (ILL_FUNC thru MEM_PAR_ERR
 * taken from PI-MBUS-300 Rev E Page 96-97.  These are modbus error codes
 * and can be placed in Modbus  exception messages as is. Other error
 * codes can be returned by functions, but will confuse a Modbus master if
 * placed in an exception message.
*/

enum    err
{
    OK,             /* 000/0x00 - OK - no error */
    COMMFAIL ,      /*   1/0x01 - Serial port error[s] overun etc  */
    DWNOPRESENCE ,  /*   2/0x02 - No presence pulse on Dallas rite */
    DWSTUCKBIT ,    /*   3/0x03 - Stuck bits inside Dallas chip on rite*/
    DWWRONGCHIP ,   /*   4/0x04 - Attempt to write DS1990 or DS1991 ram*/
    DWFAIL,         /*   5/0x05 - Dallas general write failure     */
    DRNOPRESENCE,   /*   6/0x06 - No presence pulse on Dallas read */
    DROMCRC,        /*   7/0x07 - Dallas ROM crc won't check out on read*/
    DRSEARCH,       /*   8/0x08 - Can't sort out Dallas multichip net  */
    DRWRONGCHIP,    /*   9/0x09 - Attempt to read DS1990 or DS1991 ram */
    DRAMCRC,        /*  10/0x0A - Dallas RAM crc won't check out on read*/
    DRFAIL,         /*  11/0x0B - Dallas general read failure      */
    INVALID,        /*  12/0x0C - Truck Validation failure     */
    NOTSRECORD,     /*  13/0x0D - Bootloader failed to see leading S   */
    SRECTYPE,       /*  14/0x0E - Bootloader Srecord not types 0,1,9   */
    SRECKSUM,       /*  15/0x0F - Bootloader Srecord checksum failure  */
    SRECORDBAD,     /*  16/0x10 - Bootloader general failure       */
    LISTFULL,       /*  17/0x11 - Serial number list filled up     */
    BADHEXCHAR,     /*  18/0x12 - Hex input contained non-hex char */
    HEXTOOBIG,      /*  19/0x13 - Hex input with too many chars    */
    HEXNULSTRING,   /*  20/0x14 - Received a null string where hex expected*/
    FILENOTFND,     /*  21/0x15 - Touch chip file not found        */
    TCHIPFORMAT,    /*  22/0x16 - Tchip not formatted. No "AA"     */
    TCHIPFULL,      /*  23/0x17 - Tchip filled up. No room for data    */
    OUTTAFCBS,      /*  24/0x18 - No free fcb for tchip file operation */
    DGENTIMEOUT,    /*  25/0x19 - Timeout wait for Dallas Chips    */
    DROMTIMEOUT,    /*  26/0x1A - Timeout Dallas rom operation     */
    DRAMTIMEOUT,    /*  27/0x1B - Timeout Dallas ram operation     */
    FILENOTOPEN,    /*  28/0x1C - Read/Write attempt without opening file*/
    BADTERMNO,      /*  29/0x1D - Oil Terminal number not 0-9999   */
    TCHIPLENBYT,    /*  30/0x1E - Tchip bad length byte (over 29)  */
    TCHIPWRITELOCK, /*  31/0x1F - Tchip not writeable (software or hardware) */
    TCHIPEOF,       /*  32/0x20 - EOF reading Tchip file           */
    DSBADCHAR,      /*  33/0x21 - Bad character in DateStamp file  */
    DSEXPIRED,      /*  34/0x22 - DateStamp expired                */
} ;

#define USA_TRUCK   6               /* Max compartments in American trucks */
#define CAN_TRUCK   8               /* Max compartments in Europe/Canada   */

/* MAX defines */

#define MAX_ARRAY       (unsigned int)200         /* Maximum binary ADC samples input */
#define MAX_RELAY       7           /* Maximum binary relay samples */
#define MAXCHIP         4           /* Max Dallas chips on truck    */
#define MAX_CHAN_NEW    12          /* Maximum display probe channels  */
#define MAX_CHAN        8           /* Maximum channels of input */
#define COMPART_MAX CAN_TRUCK       /* Max thermal probes   */
#define TMR1_OPT        55000       /* optimal TCNT value for 5 wire */
#define TMR1_MAX        0xFFFF      /* max TCNT value */


#define  SHELL_START    0x00000

   /* ***** NOTE Real Timer Ticks divides TMR1 by 20 which make the return count */
            /* in microseconds */
#define  USEC      20           /* Number of TMR ticks to equal 1us */
#define  RUSEC     1            /* Microsecs per Real Timer Ticks (approx) */
#define  RUSec2    2*RUSEC      /* Real Timer Ticks per 2 microsec (approx) */
#define  RUSec10   10*RUSEC     /* Real Timer Ticks per 10 microsec (approx) */
#define  RUSec100  100*RUSEC    /* Real Timer Ticks per 100 microsec (approx) */
#define  RUSec150  150*RUSEC    /* Real Timer Ticks per 150 microsec (approx) */
#define  RUSec200  200*RUSEC    /* Real Timer Ticks per 200 microsec (approx) */
#define  RUSec300  300*RUSEC    /* Real Timer Ticks per 300 microsec (approx) */
#define  RUSec500  500*RUSEC    /* Real Timer Ticks per 500 microsec (approx) */
#define  RMSec     1000*RUSEC   /* Real Timer Ticks per 1 millisec (approx) */

   /* *****
 NOTE these values are based on a 1 ms 'freetimer' count ***** */

#define  MSec     1           /* Timer Counts per millisec */
#define  MSec2    2           /* Timer Counts per 2  millisec */
#define  MSec4    4           /* Timer Counts per 4  millisec */
#define  MSec5    5           /* Timer Counts per 5  millisec */
#define  MSec10   10          /* Timer Counts per 10 millisec */
#define  MSec20   20          /* Timer Counts per 20 millisec */
#define  MSec100  100         /* Timer Counts per 100 millisec */
#define  MSec500  500         /* Timer Counts per 500 millisec */
#define  MSec750  750         /* Timer Counts per 750 millisec */
#define  SEC1     1000        /* Timer Counts per 1 second */
#define  SEC2     2000        /* Timer Counts per 2 seconds */
#define  SEC3     3000        /* Timer Counts per 3 seconds */
#define  SEC4     4000        /* Timer Counts per 4 seconds */
#define  SEC5     5000        /* Timer Counts per 5 seconds */
#define  SEC10    10000       /* Timer Counts per 10 seconds   */
#define  SEC15    15000       /* Timer Counts per 15 seconds   */
#define  SEC20    20000       /* Timer Counts per 20 seconds   */
#define  SEC30    30000       /* Timer Counts per 30 seconds   */
#define  SEC45    45000       /* Timer Counts per 45 seconds   */
#define  MIN1     60000       /* Timer Counts per 1  Minutes */
#define  SEC100   100000      /* Timer Counts per 100 seconds */
#define  SEC120   120000      /* Timer Counts per 120 seconds */
#define  MIN3     180000      /* Timer Counts per 3  Minutes */
#define  MIN5     300000      /* Timer Counts per 5  Minutes */
#define  MIN10    600000      /* Timer Counts per 10  Minutes */
#define  MIN15    900000      /* Timer Counts per 15 Minutes */
#define  MIN30    1800000     /* Timer Counts per 30 Minutes */
#define  HOUR     3600000     /* Timer Counts per 60 Minutes */


/*
      M O D B U S   C O M M   D E F I N E S
*/

#define RX_ERR             0x0F
#define MODBUS_MAX_LEN     80


/* System EEPROM format/version, used to trap mixing software versions
   and EEPROM formats!

   In (byte)Major(byte)Minor format:

	Major version different, then EEPROM not usable by this code;

	Minor version different, then EEPROM usable, but some "minor"
	features/values/etc. must be checked for explicitly by software
	before allowed to be used.
  EE_VERSION updated to 116 to add resistive ground variables */

#define EE_VERSION          0x0120
#define EE_MAJVERMASK       0xFF00
#define EE_MINVERMASK       0x00FF

/**************************************************************************
* System "Non-Volatile" information structures.
*
* The EEPROM has a "System Information" block, which is in turn comprised
* of "a bunch" of smaller-and-individually-CRC'ed data structures.
*
* Generally, each block is max of 64 bytes for both ease and modularity of
* CRC'ing and checksumming (i.e., 28C256 can write a 64-byte "page" in one
* operation, typically 5 ms duration).
**************************************************************************/

/* System "NonVolatile data structure" struct definition */

typedef struct
{
  unsigned int    ModBusRespWait;      /* 0x020100 Milliseconds turnaround delay for ModBus */
  unsigned int    TASWait;             /* 0x020102 Seconds to wait for TAS/VIP control */
  unsigned int    BypassTimeOut;       /* 0x020104 Seconds active before Bypass Timeout */
  unsigned int    Terminal;            /* 0x020106 Terminal ID number (999 = wildcard) */
  unsigned char   ConfigB;             /* 0x020108 Future "ConfigB" bits */
  unsigned char   EnaFeatures;         /* 0x020109 Enabled software Features */
  unsigned char   EnaPassword;         /* 0x02010A Enabled-Features "password" */
  unsigned char   VIPMode;             /* 0x02010B Remember the VIP Mode JGS Rev 1.8 */
  unsigned int    ADCTmaxNV;           /* 0x02010C Thermal 3.8 volt max swing */
  unsigned int    ADCTHstNV;           /* 0x02010E Hysteresis value */
  unsigned int    ADCOmaxNV;           /* 0x020110 Optical 4.5 volt min(of max) swing */
  unsigned char   Ena_INTL_ShortNV;    /* 0x020112 Enable Shorts Test Flag */
  unsigned char   Ena_Debug_Func_1;    /* 0x020113 */
  unsigned char   Ena_Debug_Func_2;    /* 0x020114 */
  unsigned char   Ena_Debug_Func_3;    /* 0x020115 */
  unsigned char   Ena_Debug_Func_4;    /* 0x020116 */
  unsigned char   Ena_Debug_Fail_1;    /* 0x020117 */
  unsigned char   Ena_Debug_Fail_2;    /* 0x020118 */
  unsigned char   Ena_Debug_Fail_3;    /* 0x020119 */
  unsigned char   Ena_Debug_Fail_4;    /* 0x02011A */
  unsigned char   EnaSftFeatures;      /* 0x02011B Software Features - Enable 5 wire mode etc..0x02011A */
  unsigned char   EnaFaultDelay;       /* 0x02011C Fault Delay ON/OFF */
  unsigned char   Ena_TIM_Read;        /* 0x02011D WPW: Enable TIM read internally */
  unsigned char   Ena_GND_Display;     /* 0x02011E Display GND OK during truck_acquire */
  unsigned char   Modbus_Ena_Features; /* 0x02011F Remember features enable/disable by modbus commands */
  unsigned int    Ground_Reference;    /* 0x020220 Level for resistive ground checking, with jumpstart (ECR 2857) */
  unsigned int    EU_GND_REF;          /* 0x020222 Level for resistive ground checking without jumpstart*/
  unsigned int    Five_Wire_Display;   /* 0x020224 Display time for 5-wire compartment count */
  unsigned int    DM_Active;           /* 0x020226 Active Deadman Enabled */
  unsigned int    DM_Max_Open;         /* 0x020228 Active Deadman Max open time */
  unsigned int    DM_Max_Close;        /* 0x02022A Active Deadman Max close time */ 
  unsigned int    DM_Warn_Start;       /* 0x02022C Active Deadman Warning time */
  unsigned char   Cert_Expiration_Mask; /* 0x02022E supertim cert expiration mask */
  unsigned int    Unload_Max_Time_min;  /* 0x02022F Allowable time for unload since load */
  unsigned char   EnaSftFeatures2;
  unsigned char   fuel_type_check_mask;
  unsigned char   default_fuel_type[3];
  unsigned char   free[10];             /* Fogbugz 131 0x020231 Round up to 64 bytes total  (23E - current) */
  unsigned int    CRC;                  /* 0x02023E G.P. Parameter block CRC */
} SysParmNV;

#define DM_OPEN     (1*4)       /* Active Deadman Max open time */
#define DM_CLOSE    (120*4)     /* Active Deadman Max close time */ 
#define DM_WARN     (15*4)      /* Active Deadman Warning time */

/* System "DateStamp" data structure */

/* DateStamp "name", 4 chars for filename, 1 for extension */

#define DS_NAMMAX   6

/* DateStamp "password", 8 chars, plus count byte */

#define DS_PSWMAX   10

typedef struct
{
    char            name[DS_NAMMAX];/* DateStamp "company name" */
    char            psw[DS_PSWMAX]; /* DateStamp "password" */
    char            free[22 - DS_NAMMAX - DS_PSWMAX]; /* 24 bytes total */
    unsigned        CRC;            /* DateStamp block CRC */
} DateStampNV;



/* System 5-wire-optic Diagnostic voltage table (used to determine which
   5-wire-optic probe is "wet" based on how many 4.75K diag resistors are
   conducting [dry probe "ON", wet probe "OFF"]). */

typedef struct
{
    unsigned        Reference;          /* "6.759" volt reference level */
    unsigned        PNOffset;           /* Transistor PN-junction bias/offset */
    unsigned        WetVolts[16];       /* 16-probe "wet" level */
    unsigned int    updatedADCTable;    /* Switch to use updated ADC table for probe counting in calc_tank, 1 = new table, 0 = old table */
                                        /* DateStamp + SysDia5 = 64 bytes total! */
    unsigned        CRC;                /* SysDia5 block CRC */
} SysDia5NV;



/* System "Voltages" block. These voltages are general unit values, min and
   max limits, etc. */

typedef struct
{
    unsigned        Raw13Lo;        /* Minimum "Raw 13 Volt" level */
    unsigned        Raw13Hi;        /* Maximum "Raw 13 Volt" level */
    unsigned        RefVoltLo;      /* Minimum "1 Volt Reference" level */
    unsigned        RefVoltHi;      /* Maximum "1 Volt Reference" level */
    unsigned        Chan10Lo;       /* Minimum "10 Volt" channel drive level */
    unsigned        Chan10Hi;       /* Maximum "10 Volt" channel drive level */
    unsigned        ChanNoiseP;     /* Maximum "Channel Noise Plus" (positive) */
    unsigned        ChanNoiseM;     /* Maximum "Channel Noise Minus" (negative) */
    unsigned        Jump20Lo;       /* Minimum "20 Volt Jump Start" level */
    unsigned        Jump20Hi;       /* Maximum "20 Volt Jump Start" level */
    unsigned        Euro20Lo;       /* Minimum "Euro-Jumper" clamping level */
    unsigned        Euro20Hi;       /* Maximum "Euro-Jumper" clamping level */
    unsigned        Bias35Lo;       /* Minimum "3.5 Volt Bias" level */
    unsigned        Bias35Hi;       /* Maximum "3.5 Volt Bias" level */
    unsigned        Bias38Lo;       /* Minimum "3.8 Volt Bias" level */
    unsigned        Bias38Hi;       /* Maximum "3.8 Volt Bias" level */
    unsigned        BiasNoiseP;     /* Maximum "Bias Noise Plus" (positive) */
    unsigned        BiasNoiseM;     /* Maximum "Bias Noise Minus" (negative) */
    unsigned        OpticOutLo;     /* Minimum "5-Wire Optic" drive level */
    unsigned        OpticOutHi;     /* Maximum "5-Wire Optic" drive level */
    unsigned        OpticInLo;      /* Minimum "5-Wire Optic" response level */
    unsigned        OpticInHi;      /* Maximum "5-Wire Optic" response level */
    unsigned        ProbeSensJSV;   /* Probe detection threshold (millivolts) */
    unsigned        ProbeSensEuroV; /* Ditto, "European" voltages jumper */
    unsigned        free[14];       /* Round up to 64 bytes total */
    unsigned        CRC;            /* CRC-16 validity check */
} SysVoltNV;

/* System "Settings" block ("Static", manufacturing, not customer/end-user
   changeable. These are in EEPROM to facilitate production changes, not to
   necessarily tune a unit for a customer's site). Customer-settable para-
   meters are in SysParm above.*/

typedef struct
{
    unsigned        free[62];       /* Round up to 64 byes total */
    unsigned        CRC;            /* CRC-16 validity check */
} SysSet1NV;

#define GND_REF_DEFAULT 0x3A0   // Fogbugz 134 Near 2.2k ohms 
#define GND_REF_LOW     0x00A   // Near 10 ohms 
#define GND_REF_EU      0x21C   // Near 500 ohms 
#define GND_REF_HIGH    0x44C   // Near 10k ohms

/******************************* 3/16/2009 3:48PM ****************************
 * Structure STATREG and union _SPI_EEPROMStatus_
 *
 * Overview: Provide a bits and byte access to SPI_EEPROM status value.
 *****************************************************************************/
struct  STATREG{
_Bool    BSY:1;            /* Erase or Write In Progress */
_Bool    WEL:1;            /* Write Enable Latch */
_Bool    BP0:1;            /* Block Protect Bite */
_Bool    BP1:1;            /*  */
_Bool    BP2:1;            /*  */
_Bool    TB:1;             /* TOP/BOTTOM Write Protect */
_Bool    RESERVED:1;
_Bool    SRP:1;            /* Status Register Protect */
};

typedef union EEPROMStatus
{
  struct  STATREG Bits;
  char    Char;
} _SPI_EEPROMStatus_;

typedef union tuReg32
{
  unsigned long Val32;

  struct
  {
    unsigned short LW;
    unsigned short HW;
  } Word;

  unsigned char Val[4];
} uReg32;


#endif    /* end of STDSYM_H */
/************************** end of stdsym **********************************/
