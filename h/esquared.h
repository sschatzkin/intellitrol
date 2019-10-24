/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         esquared.h
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2008  Scully Signal Company
 *
 *   Description:    Non-Volatile (EEPROM) definitions and layout.
 *
 *   Revision History:
 *
 *****************************************************************************/
#ifndef ESQUARED_H
#define ESQUARED_H

/* Number of Event Log entries to maintain in EEPROM */

//#define E2LOGCNT    32
#define E2LOGCNT    1024

/* Number of Bypass Key entries to maintain in EEPROM */

#define E2KEYCNT    32

/* Minimum number of Truck ID Module entries to maintain in EEPROM. 5000
   fits nicely (well, tightly) into a 32KB (28C256) EEPROM. */

#define E2TIMCNT    5000

/* Protected writes are a three-step initialization sequence followed by the
   actual data write(s): Relative address 5555<==AA; 2AAA<==55; 5555<==A0.*/

#define WRENADR1    0x5555
#define WRENDAT1    0xAA

#define WRENADR2    0x2AAA
#define WRENDAT2    0x55

#define WRENADR3    0x5555
#define WRENDAT3    0xA0

/**************************************************************************
*
* EEPROM layout:
*
*           +----------------+      "Home" block layout
*   0000/   |    0x17        |
*   0001/   |    0xE8        |
*   0002/   | version/format |
*           ~                ~
*   003E/   | Home-Block     |
*   003F/   |  CRC-16        |
*           +----------------+      "Boot" block
*   0040/   | Boot info      |
*           ~                ~
*   007F/   |                |
*           +----------------+      "Crash/Reset" block
*   0080/   | Crash info     |
*           ~                ~
*   00FF/   |                |
*           +----------------+      System Non-Volatile storage
*   0100/   | NonVolatile    |
*           ~  System        ~
*   03FF/   |   Info         |
*           +----------------+      Customer/System Error log
*   0400/   | Error Log      |
*           ~  32 x 32       ~
*   07FF/   |                |
*           +----------------+      Bypass Key Registry
*   0800/   | Bypass Key     |
*           ~  Registry      ~
*   08FF/   |   32 x 8       |
*           +----------------+      Truck Key Registry
*   0900/   | Truck Key      |
*           ~  Registry      ~
*   7E2F/   |  5000 x 6      |
*           +----------------+      Factory Settings
*   7E30/   |                |
*           ~                ~
*   7FFF/   |                |
*           +----------------+
*
***************************************************************************/

/* EEPROM Partition ID types */

#define EEP_HOME        0x01
#define EEP_BOOT        0x02
#define EEP_CRASH       0x03
#define EEP_SYSNV       0x04
#define EEP_LOG         0x05
#define EEP_KEY         0x06



#define EEP_TIM         0x07
#define EEP_FACT        0x08

/* EEPROM "Home" block format. This is the "root" of all access to the
   various "non-volatile partitions" used by the Intellitrol. */

typedef struct
{
    unsigned char pat1;         /* Pattern: 0x17 */
    unsigned char pat2;         /* Pattern: 0xE8 */
    unsigned    Version;        /* Version/format number */
    unsigned char Valid;          /* Home/EEPROM validity flags */
    char        Status;         /* Home/EEProm status */
    char        Serial[8];      /* Unit Serial number (Dallas clock) */
    unixtime    Time;           /* Unit EEPROM Home init time (4 byte unsigned long) */
    unsigned    EESize;         /* Size (in KB) of EEPROM */
    unsigned    Bootlen;        /* Length of "boot" block */
    unsigned    Bootptr;        /* Start of "boot" block */
    unsigned    Crashlen;       /* Length of "Crash" block */
    unsigned    Crashptr;       /* Start of "Crash" block */
    unsigned    SysNVlen;       /* Length of "NonVolatile" system block */
    unsigned    SysNVptr;       /* Start of "NonVolatile" system block */
    unsigned    Loglen;         /* Length of system log block */
    unsigned    Logptr;         /* Start of system log block */
    unsigned    Keylen;         /* Length of Bypass Key block */
    unsigned    Keyptr;         /* Start of Bypass Key block */
    unsigned    TIMlen;         /* Length of Authorized-TIM block */
    unsigned    TIMptr;         /* Start of Authorized-TIM block */
    char        free[18];       /* Free/Available Home block storage */
    unsigned        CRC;            /* Home block CRC */
    } E2HOMEBLK;

/* Home block magic pattern bytes (used to id/verify it's a home block) */

#define EEHOMEPAT1      0xE8
#define EEHOMEPAT2      0x17

/* Home block Validity bits -- Flag is set if valid to use block */

#define EEV_HOMEBLK     0x01
#define EEV_BOOTBLK     0x02
#define EEV_CRASHBLK    0x04
#define EEV_SYSBLK      0x08
#define EEV_LOGBLK      0x10
#define EEV_KEYBLK      0x20
#define EEV_TIMBLK      0x40

#define EEV_ALL (EEV_BOOTBLK | EEV_CRASHBLK | EEV_SYSBLK | EEV_LOGBLK \
                 | EEV_KEYBLK | EEV_TIMBLK)

/* EEPROM "Boot" block format. This block is reserved for bootstrap/kernel
   usage; as such it ain't defined yet! */

typedef struct
{
    char        free[62];       /* Whatever we decide upon */
    unsigned        CRC;            /* Boot block CRC */
    } E2BOOTBLK;


/* EEPROM "Crash" block format. The Crash block can be used to save software
   crash information (e.g., registers/stack/etc.) for debugging purposes. */

typedef struct
{
    char        free[126];      /* Whatever we decide upon */
    unsigned        CRC;            /* Crash block CRC */
    } E2CRASHBLK;


/* EEPROM "System NonVolatile" block format. It consists mostly of smaller
   "sub-blocks", which can be individually CRC'ed */

#define E2SYS_PARM      0x00
#define E2SYS_DSTAMP    0x01
#define E2SYS_DIA5      0x02
#define E2SYS_VOLT      0x03
#define E2SYS_SET1      0x04

typedef struct
{
    SysParmNV   ParmBlock;      /* 0x000 General system parameters */
    DateStampNV DateStampBlock; /* 0x040 "DateStamp" mode parameters */
    SysDia5NV   Dia5Block;      /* 0x058 5-wire-optic diag tank table */
    SysVoltNV   VoltBlock;      /* 0x080 System voltages/parameters */
    SysSet1NV   Set1Block;      /* 0x0C0 System settings/limits/etc. */
                                /* 0x100 */
    char        free[768 - sizeof(SysParmNV)    /* 0 - 03F */
                         - sizeof(DateStampNV)  /* 40 - 57 */
                         - sizeof(SysDia5NV)    /* 58 - 7F*/
                         - sizeof(SysVoltNV)    /* 80 - C0 */
                         - sizeof(SysSet1NV)]; /* C0 - Whatever else ... */
    } E2SYSBLK;

#define SysParmAdr      0
#define DateStampAdr    sizeof(SysParmNV)
#define SysDia5Adr      DateStampAdr + sizeof(DateStampNV)
#define SysVoltAdr      SysDia5Adr + sizeof(SysDia5NV)
#define Set1BlockAdr    SysVoltAdr + sizeof(SysVoltNV)

/* EEPROM "Log" block format. The Log block is where system log events are
   recorded for later perusal by TAS/VIPER/etc. The Log block consists of a
   bunch of E2LOGREC Log records.

   The CRC is calculated on the last portion of the E2LOGREC records, skipping
   the Type, SubType, and RepMask fields to allow them to change after initial
   entry logging (see nvLogMerge() and nvLogRepeat()). */

typedef struct
{
    char        Type;           /* Log entry type */
    char        Subtype;        /* Type-specific additional info (byte *1*) */
    unsigned    RepMask;        /* Repeat mask (must be bytes *2* and *3*) */
    unixtime    Time;           /* Date/Time (UCT) of original entry */
    char        Info[22];       /* Type- & Subtype-specific log entry data */
    unsigned    CRC;            /* Log entry CRC */
    } E2LOGREC;


#define E2LOGCRCOFS 4           /* byte + byte + word fields not CRC'ed */

//typedef struct
//{
//    E2LOGREC    Entry[E2LOGCNT]; /* "Buncha" Log records */
//    } E2LOGBLK;

/* EEPROM "Key" block format. The Key block stores authorized bypass key
   serial numbers, and as such is composed of a bunch of E2KEYBLK Key
   records. */

typedef struct
{
    unsigned char   Key[BYTESERIAL]; /* Bypass/Dallas key serial number */
    unsigned        CRC;            /* Bypass key CRC */
    } E2KEYREC;


/* EEPROM "TIM" block format. The TIM block stores authorized truck serial
   numbers, and as such is composed of a bunch of E2TIMREC TIM records. */

/*  *KROCK ALERT* */
/*  In order to shoehorn 5000 serial numbers (marketing decision) into a */
/*  lowly 32K EEPROM, we only have 6 bytes per TIM, so we steal the leading */
/*  byte ("always a zero, anyways") and stuff the Dallas-style CRC-8 in its */
/*  place... */

typedef struct
{
    char        Serial[BYTESERIAL]; /* TIM serial number (6) */
//    unsigned        CRC;             /* TIM serial number CRC */
    } E2TIMREC;

/*
HOME_BASE:  0x0
BOOT_BASE:  0x40
CRASH_BASE: 0x80
SYS_BASE:   0x100
LOG_BASE:   0x8000
KEY_BASE:   0x800
TIM_BASE:   0x900
Size of E2HOMESIZ : 0x40 64
Size of E2BOOTSIZ : 0x40 64
Size of E2CRASHSIZ: 0x80 128
Size of E2SYSSIZ  : 0x300 768
Size of E2LOGSIZ  : 0x400 1024
Size of E2KEYSIZ  : 0x100 256
Size of E2TIMSIZ  : 0x7530 30000
*/

/* Base offset and size of "Home" block in EEPROM */

#define HOME_BASE   0x0000
#define E2HOMESIZ   (sizeof(E2HOMEBLK))

/* Size of "Boot" block in EEPROM */

#define BOOT_BASE   (HOME_BASE + E2HOMESIZ)
#define E2BOOTSIZ   (sizeof(E2BOOTBLK))

/* Size of "Crash" block in EEPROM */

#define CRASH_BASE  (BOOT_BASE + E2BOOTSIZ)
#define E2CRASHSIZ  (sizeof(E2CRASHBLK))

/* Size of "System Non-Volatile" block in EEPROM */

#define SYS_BASE    (CRASH_BASE + E2CRASHSIZ)
#define E2SYSSIZ    (sizeof(E2SYSBLK))

/* Size of event "Log" block in EEPROM */

//#define LOG_BASE    (SYS_BASE + E2SYSSIZ)
#define LOG_BASE    (0x8000)
#define E2LOGSIZ    (E2LOGCNT * sizeof(E2LOGREC))

/* Size of "Bypass Key" block in EEPROM */

// #define KEY_BASE    (LOG_BASE + E2LOGSIZ)
#define KEY_BASE    (0x800)
#define E2KEYSIZ    (E2KEYCNT * sizeof(E2KEYREC))

/* Size of "Truck Serial Number" block in EEPROM */

#define TIM_BASE    (KEY_BASE + E2KEYSIZ)
#define E2TIMSIZ    (E2TIMCNT * sizeof(E2TIMREC))


extern  E2HOMEBLK *eeHomePtr(void); /* Return Home block pointer */

/******************  End of ESQUARED.H  ***********************************/
#endif        /* end of ESQUARED_H */
