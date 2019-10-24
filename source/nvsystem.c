/*******************************************************************************
 *
 *   Project:        Rack Controller Intellitrol
 *
 *   Module:         NVSYSTEM.C
 *
 *   Revision:       REV 1.00
 *
 *   Author:         Ken Langlais  @Copyright 2009,2014  Scully Signal Company
 *
 *   Description:    Main program Non-Volatile storage (EEPROM) routines for
 *                   handling "System" and "Event Log" data.
 *
 * Revision History:
 *   Rev      Date   Who  Description of Change Made
 * -------- -------- ---  --------------------------------------------
 * 1.5.00  04/22/08  KLL  Started porting old Intellitrol modbus code from
 *                                     the MC68HC16Y1 cpu on the original Intellitrol
 * 1.5.02  12/09/11  KLL  Fixed an indexing problem in nvLogGet(), changed
 *                                      char to unsigned int
 * 1.5.27  03/24/14  DHP  Increased Probe Short and Probe Power levels to
 *                                       correctly identify connection of certain devices.
 * 1.5.30  08/10/14  DHP  Removed VaporTimeOut init
 *                        Changed SysParm.Ground_Reference init to use 
 *                        GND_REF_DEFAULT rather than 0x3A0
 * 1.6.34  08/05/16  DHP  FogBugz 131: Added EE_CRC error flag in nvSysInit().
 *                        In nvSysInit() changed copy of default voltage data copy 
 *                         to EEPROM to be on EEPROM error rather than no error.
 *                        Removed setting of system pointer to default; either 
 *                         everything is OK and matches EEPROM or error.
 *                        Added nvSysSet1Update() as this block needs a CRC.
 *                        Restored SysParm.ADCTmaxNV to original value.
 * 1.6.34  08/05/16  DHP  Added to eeUpdateSys() the new parameters for the
 *                          Active Deadman. Added in nvSysInit() a check to 
 *                          ensure Active Deadman parameters were in place and
 *                          add them if not so that the max open time on standard 
 *                          deadman was not zero.
 ******************************************************************************/

#include "common.h"
#include "volts.h"      /* SysVolts stuff */


/****************************************************************************
*
* Non-Volatile System Parameters Partition services
*
****************************************************************************/

/* Default System 5-wire-optic diagnostic voltage levels block */

static const SysDia5NV SysDia5Default =
{
  OPTIC5DIAG_REF,               /* 5-wire-optic diagnostic reference */
  OPTIC5DIAG_OFS,               /* 5-wire-optic PN-junction bias/offset */
  {                             /* 5-wire-optic "wet" probe levels */
    OPTIC5DIAG_CH01,            /* Tank/Compartment # 1 */
    OPTIC5DIAG_CH02,            /* Tank/Compartment # 2 */
    OPTIC5DIAG_CH03,            /* Tank/Compartment # 3 */
    OPTIC5DIAG_CH04,            /* Tank/Compartment # 4 */
    OPTIC5DIAG_CH05,            /* Tank/Compartment # 5 */
    OPTIC5DIAG_CH06,            /* Tank/Compartment # 6 */
    OPTIC5DIAG_CH07,            /* Tank/Compartment # 7 */
    OPTIC5DIAG_CH08,            /* Tank/Compartment # 8 */
    OPTIC5DIAG_CH09,            /* Tank/Compartment # 9 */
    OPTIC5DIAG_CH10,            /* Tank/Compartment # 10 */
    OPTIC5DIAG_CH11,            /* Tank/Compartment # 11 */
    OPTIC5DIAG_CH12,            /* Tank/Compartment # 12 */
    OPTIC5DIAG_CH13,            /* Tank/Compartment # 13 */
    OPTIC5DIAG_CH14,            /* Tank/Compartment # 14 */
    OPTIC5DIAG_CH15,            /* Tank/Compartment # 15 */
    OPTIC5DIAG_CH16             /* Tank/Compartment # 16 */
  },
  1,                            /* Switch to use updated ADC table for probe counting in calc_tank, 1 = new table, 0 = old table */
  0                             /* CRC-16 slot */
  };

/* Default System Voltages block */

static const SysVoltNV SysVoltDefault =
{
  RAW13VOLT_LO,   RAW13VOLT_HI,       /* Min, Max Raw 13 volt levels */
  REFVOLT_LO,     REFVOLT_HI,         /* Min, Max "Reference" levels */
  PROBE10_MIN,    PROBE10_MAX,        /* Min, Max 10 volt channel levels */
  PROBE10_NOISE_P,PROBE10_NOISE_M,    /* Min, Max channel noise levels */
  PROBE20_MIN,    PROBE20_MAX,        /* Min, Max 20 volt jumpstart */
  PROBE_EUROMIN,  PROBE_EUROMAX,      /* Min, Max "euro" jumper jumpstart */
  BIAS35_MIN,     BIAS35_MAX,         /* Min, Max 3.5 volt bias levels */
  BIAS38_MIN,     BIAS38_MAX,         /* Min, Max 3,8 volt bias levels */
  BIAS_NOISE_P,   BIAS_NOISE_M,       /* Min, Max bias noise levels */
  OPTIC5OUT_MIN,  OPTIC5OUT_MAX,      /* Min, Max 5-wire optic drive levels */
  OPTIC5IN_MIN,   OPTIC5IN_TRAIL,     /* Min, cutoff 5-wire optic response */
  PROBE_SENS_JSV, PROBE_SENS_EUROV,   /* Probe Detect Voltage */
  {0},                                /* Reserved, MBZ */
  0                                   /* CRC-16 slot */
};

/* Default System Settings block */

static const SysSet1NV SysSet1Default =
{
  {0},
  0                               /* CRC-16 slot */
};

/****************************************************************************
* nvSysFormat -- "Format/Initialize" EEPROM's System NonVolatile block
*
* Call is:
*
*   nvSysFormat ()
*
* On successful return, The "System" partition in EEPROM has been initialized.
****************************************************************************/
char nvSysFormat (void)
{
char sts;

  /* Set up default NonVolatile parameter values */

  // last_routine = 0x1F;
  nvSysParmDefaults();
  // last_routine = 0x1F;
  sts = nvSysParmUpdate();            /* Write SysParm to EEPROM */
                                      /* When FIRST power Up */
                                      /* with UNFormatted EEPROM - will */
                                      /* set it with default values */
  if ( sts)
  {
    return sts;
  }
  // last_routine = 0x1F;
  sts = nvSysDia5Update(1);
  if ( sts)
  {
    return sts;
  }
  // last_routine = 0x1F;
  sts = nvSysVoltUpdate();
  if ( sts)
  {
    return sts;
  }
  sts = nvSysSet1Update();
  if ( sts)
  {
    return sts;
  }

DateStampNV dsblk;
int i;

  for (i=0; i < DS_NAMMAX; i++)
  {
    dsblk.name[i] = 0;
  }
  for (i=0; i < DS_PSWMAX; i++)
  {
    dsblk.psw[i] = 0;
  }
  for (i=0; i < ((22 - DS_NAMMAX) - DS_PSWMAX); i++)
  {
    dsblk.free[i] = 0;
  }
  sts = nvSysDSUpdate(&dsblk);
  return sts;
} /* End nvSysFormat() */

/****************************************************************************
* nvSysInit -- Initialize SysParm/etc block from EEPROM copy
*
* Call is:
*
*   nvSysInit ()
*
* On successful return, the "System NonVolatile" parameters have been setup
* for use by the Intellitrol:
*
*   the "SysParm" struct in RAM has been filled in from EEPROM (or defaulted
*   if EEPROM dead, data bad (CRC error), etc.;
*
*   the DateStamp values, if any, are available via pDateStamp (pDateStamp
*   may be NULL if DateStamp is not active -- verify pointer before using it!);
*
*   system settings are available via the pSysDia5, pSysVolt and pSysSet1
*   pointers.
****************************************************************************/
void nvSysInit (void)
{
E2SYSBLK *sysptr;           /* EEPROM's System NonVolatile area */
SysParmNV *Pptr;
DateStampNV *Dptr;
SysDia5NV *D5ptr;
SysVoltNV *Vptr;
SysSet1NV *S1ptr;
unsigned base;                  /* Base offset of System-NV partition */
unsigned size;                  /* Size (bytes) of System-NV partition */
unsigned crc = 0xFFFF;
char sts;
unsigned char *temp_ptr;
unsigned int i;
unsigned int eeprom_addr;
unsigned long temp_word;

  /* Locate and "map" the EEPROM's "System NonVolatile" parameters partition */
  // last_routine = 0x20;
  temp_ptr = (unsigned char *)&SysNonV;
  sysptr = (E2SYSBLK *)&SysNonV;
  for ( i=0; i<sizeof(E2SYSBLK);i++)
  {
    *temp_ptr++=0;
  }
  sts = eeMapPartition (EEP_SYSNV, &size, &base);   /* Upon return base = */
                                                    /* absolute address of SYSNV */
  // last_routine = 0x20;
  if (sts)
  {
    StatusB |= STSB_ERR_EEPROM;        /* Note errors with EEPROM */
    sysptr = 0;
  } else
  {
    eeprom_addr = base;
    sysptr = (E2SYSBLK *)&SysNonV;
    if ( eeReadBlock(eeprom_addr, (unsigned char *)&SysNonV, sizeof(E2SYSBLK)))
    {
      EE_status |= EE_FORMAT;         /* EEPROM needs formatting */
      StatusB |= STSB_ERR_EEPROM;        /* Note errors reading EEPROM */
    }
  }
  // last_routine = 0x20;
  if ( sysptr)
  {
    Pptr = &sysptr->ParmBlock;      /* Point to EEPROM SysParm block */
  } else
  {
    Pptr = 0;
  }
  if (Pptr)
  {
    crc = modbus_CRC ((unsigned char *)Pptr,     /* Check the block CRC */
                         sizeof(SysParmNV) - 2,
                         INIT_CRC_SEED);
  }
//>>> FogBugz 131  if (Pptr && (crc == Pptr->CRC))     /* EE's SysParm block look good? */
  if ((Pptr && (crc == Pptr->CRC)) || 
     (Pptr && (home_block.Version & EE_MINVERMASK) == 0x15)) //<<< FogBugz 131
  {  /* copy it to RAM */
    memcpy (&SysParm, (char *)Pptr, sizeof(SysParmNV));
    if(SysParm.DM_Max_Open == 0)  /* ensure new active deadman variable are included */
    {  // put in defaults if not already there
      SysParm.DM_Max_Open = DM_OPEN;
      SysParm.DM_Max_Close = DM_CLOSE;
      SysParm.DM_Warn_Start = DM_WARN;
      (void)nvSysParmUpdate();
    }
  }
  else                              /* EE's SysParm block look good? */
  {                                 /* No */
    EE_status |= EE_CRC;            /* FogBugz 131 EEPROM CRC error */
    nvSysParmDefaults ();           /* Fake out default values */
  }                                 /* And by the way - no reason to update */
                                    /* the EEPROM */
  /* Init the System DateStamp pointer */
  /* Map "address" of EEPROM-resident DateStamp block */
  if (sysptr)                         /* EEPROM looking good? */
  {
    Dptr = &sysptr->DateStampBlock; /* Point to EEPROM DateStamp block */
  }
  else
  {
    Dptr = 0;
  }
  if (Dptr)                           /* If can access DateStamp block */
  {
    crc = modbus_CRC ((unsigned char *)Dptr,
                      sizeof(DateStampNV) - 2,
                      INIT_CRC_SEED);
  }
  if (Dptr && (crc == Dptr->CRC))     /* EE's DateStamp block look good? */
  {                                   /* Yes */
    pDateStamp = Dptr;                 /* Set system access pointer */
  }
  else                                /* EE's DateStamp block look good? */
  {                                   /* No */
    pDateStamp = 0;                   /* No access to DateStamp! */
  }
  /* Init the System 5-wire-optic diagnostic voltages pointer */
  /* Map "address" of EEPROM-resident SysDia5NV block */
  if (sysptr)                         /* EEPROM cooperating */
  {
    D5ptr = &sysptr->Dia5Block;     /* Grab the EEPROM copy */
    crc = modbus_CRC ((unsigned char *)D5ptr,
                      sizeof(SysDia5NV) - 2,
                      INIT_CRC_SEED);
    if ( crc != D5ptr->CRC)
    {
      D5ptr = 0;
      EE_status |= EE_CRC;            /* FogBugz 141 EEPROM CRC error */      
    }
    else
    {
      pSysDia5 = D5ptr;     /* Set system pointer to Default */
    }
  }
  else
  {
    D5ptr = 0;
  }
  if (D5ptr == 0)                     /* If cannot access SysDia5 block */
  {
    D5ptr = &SysNonV.Dia5Block;       /* Point to SysDia5 block */

    /***************************** 11/6/2008 11:22AM *************************
     * Setup to load defaults to EEPROM
     *************************************************************************/
    memcpy((char *)D5ptr, (char *)&SysDia5Default, sizeof(SysDia5Default));
    crc = modbus_CRC ((unsigned char *)D5ptr,
                      sizeof(SysDia5NV) - 2,
                      INIT_CRC_SEED);
    D5ptr->CRC = crc;
    pSysDia5 = D5ptr;     /* Set system pointer to Default */
    temp_word = (unsigned long)SysDia5Adr + (unsigned long)base;
    (void)eeBlockWrite(temp_word, (unsigned char *)pSysDia5, sizeof(SysDia5NV));
  // last_routine = 0x20;
  }
  /* Init the System voltages pointer */
  /* Map "address" of EEPROM-resident SysVoltNV block */
  if (sysptr)                         /* EEPROM cooperating */
  {
    Vptr = &sysptr->VoltBlock;     /* Grab the EEPROM copy */
    crc = modbus_CRC ((unsigned char *)Vptr,
                      sizeof(SysVoltNV) - 2,
                      INIT_CRC_SEED);
    if ( crc != Vptr->CRC)
    {
      Vptr = 0;
      EE_status |= EE_CRC;            /* FogBugz 141 EEPROM CRC error */      
    }
    else
    {
      pSysVolt = Vptr;     /* Set system pointer to Default */
    }
  }
  else
  {
     Vptr = 0;
  }
  if (Vptr == 0)                     /* If cannot access SysDia5 block */
  {
    Vptr = &SysNonV.VoltBlock;       /* Point to SysDia5 block */

    /***************************** 11/6/2008 11:22AM *************************
     * Setup to load defaults to EEPROM
     *************************************************************************/
    memcpy((char *)Vptr, (char *)&SysVoltDefault, sizeof(SysVoltDefault));
    crc = modbus_CRC ((unsigned char *)Vptr,
                      sizeof(SysVoltNV) - 2,
                      INIT_CRC_SEED);
    Vptr->CRC = crc;
    pSysVolt = Vptr;     /* Set system pointer to Default */
    temp_word = (unsigned long)SysVoltAdr + (unsigned long)base;
    (void)eeBlockWrite(temp_word, (unsigned char *)pSysVolt, sizeof(SysVoltNV));
  // last_routine = 0x20;
  }
  /* Init the System assorted settings pointer */
  /* Map "address" of EEPROM-resident SysSet1NV block */
  if (sysptr)
  {/* EEPROM cooperating */
    S1ptr = &sysptr->Set1Block;     /* Point to EEPROM's Settings block */
  }
  else
  {
    S1ptr = 0;
  }
  if (S1ptr)                          /* If can access SysSet1 block */
  {
    crc = modbus_CRC ((unsigned char *)S1ptr,
                      sizeof(SysSet1NV) - 2,
                      INIT_CRC_SEED);
  }
  if (S1ptr && (crc == S1ptr->CRC))   /* EE's SysSet1 block look good? */
  {                                   /* Yes */
    pSysSet1 = S1ptr;                 /* Set system pointer to EEPROM */
  }
  else 
  {
    (void) nvSysSet1Update();
// for now CRC of unused EEPROM is not done - Needs to be added for overall check      
//      EE_status |= EE_CRC;            /* FogBugz 141 EEPROM CRC error */      
  }
} /* End nvSysInit() */

/****************************************************************************
* nvSysDia5Update -- Update 5-wire-optic diag tank table in EEPROM
*
* Call is:
*
*   nvSysDia5Update
*
* On successful return, the "SysParm" block in the "System" partition has
* been updated. On failure, who knows what has happened...
****************************************************************************/
char nvSysDia5Update (unsigned int updatedADCTable)
{
unsigned int crc;
char sts;

  memcpy((char *)&SysNonV.Dia5Block, (char *)&SysDia5Default, sizeof(SysDia5Default));
  SysNonV.Dia5Block.updatedADCTable = updatedADCTable;
  crc = modbus_CRC ((unsigned char *)&SysNonV.Dia5Block,
                    sizeof(SysDia5NV) - 2,
                    INIT_CRC_SEED);
  SysNonV.Dia5Block.CRC = crc;
  sts = eeBlockWrite((unsigned long)SysDia5Adr + 0x100, (unsigned char *)&SysNonV.Dia5Block.Reference, sizeof(SysDia5Default));
 // last_routine = 0x21;
  return (sts);                       /* Propagate success/failure */
} /* End nvSysDia5Update() */

/****************************************************************************
* nvSysVoltUpdate -- Update System voltages/parameters in EEPROM
*
* Call is:
*
*   nvSysVoltUpdate
*
* On successful return, the "SysParm" block in the "System" partition has
* been updated. On failure, who knows what has happened...
****************************************************************************/
char nvSysVoltUpdate (void)
{
unsigned int crc;
char sts;

  /***************************** 11/6/2008 11:22AM *************************
   * Setup to load defaults to eeprom
   *************************************************************************/
  memcpy((char *)&SysNonV.VoltBlock, (char *)&SysVoltDefault, sizeof(SysVoltDefault));
  crc = modbus_CRC ((unsigned char *)&SysNonV.VoltBlock,
                    sizeof(SysVoltNV) - 2,
                    INIT_CRC_SEED);
  SysNonV.VoltBlock.CRC = crc;
  sts = eeBlockWrite((unsigned long)SysVoltAdr + 0x100, (unsigned char *)&SysNonV.VoltBlock.Raw13Lo, sizeof(SysVoltDefault));
  return (sts);                       /* Propagate success/failure */
} /* End nvSysVoltUpdate() */

/****************************************************************************
* nvSysParmUpdate -- Update "SysParm" block in EEPROM
*
* Call is:
*
*   nvSysParmUpdate
*
* On successful return, the "SysParm" block in the "System" partition has
* been updated. On failure, who knows what has happened...
****************************************************************************/
char nvSysParmUpdate (void)
{
  unsigned int base;                  /* Base offset of System-NV partition */
  unsigned int size;                  /* Size (bytes) of System-NV partition */
  unsigned int crc;
  char sts;

  sts = eeMapPartition (EEP_SYSNV, &size, &base);
  if (sts)
  {
    return (sts);
  }
  crc = modbus_CRC ((unsigned char *)&SysParm, sizeof(SysParmNV) - 2, INIT_CRC_SEED);
  SysParm.CRC = crc;
  sts = eeBlockWrite ((unsigned long)base, (unsigned char *)&SysParm, sizeof(SysParmNV));
  return (sts);                       /* Propagate success/failure */
} /* End nvSysParmUpdate() */

/****************************************************************************
* nvSysSet1Update -- Update System block in EEPROM
*
* Call is:
*
*   nvSysSet1Update
*
* On successful return, the "Set1" block in the "System" partition has
* been updated.
****************************************************************************/
char nvSysSet1Update (void)
{
  unsigned long temp_word;
  unsigned int base;                  /* Base offset of System-NV partition */
  unsigned int size;                  /* Size (bytes) of System-NV partition */
  unsigned int crc;
  char sts;

  sts = eeMapPartition (EEP_SYSNV, &size, &base);   /* Upon return base = */
                                                    /* absolute address of SYSNV */
  memcpy((char *)&SysNonV.Set1Block, (char *)&SysSet1Default, sizeof(SysSet1Default));
  crc = modbus_CRC ((unsigned char *)&SysNonV.Set1Block,
                      sizeof(SysSet1NV) - 2,
                      INIT_CRC_SEED);
  SysNonV.Set1Block.CRC = crc;
  temp_word = (unsigned long)Set1BlockAdr + (unsigned long)base;
  sts |= eeBlockWrite(temp_word, (unsigned char *)&SysNonV.Set1Block, sizeof(SysSet1NV));
  return (sts);                       /* Propagate success/failure */
} /* End nvSysSet1Update() */

/***********************************************************************/
void nvSysParmDefaults (void)
{
  // last_routine = 0x23;
  memset (&SysParm, 0x00, sizeof(SysParmNV));    /* Fill with 0's */
  /* And set appropriate values */
  SysParm.ModBusRespWait = 100;       /* ModBus Response delay 100 ms */
  SysParm.BypassTimeOut = 60 * 60;    /* Bypass Timeout 60 minutes */
  SysParm.EnaFeatures = (char)(enable_jumpers & SysParm.EnaPassword);
  SysParm.EnaPassword = (char)ENA_BYDEFAULT; /* No unbundled options */
  SysParm.Modbus_Ena_Features = 0xFF; /* All Modbus enable features are default on */
  SysParm.ADCTmaxNV   = 2900;         /* QCCC 53 */
  SysParm.ADCTHstNV   = 700;          /* Hysteresis value */
  SysParm.ADCOmaxNV   = 4375;         /* Optical 4.5 volt min(of max) swing */
  SysParm.Ena_INTL_ShortNV = 1;       /* Enable Shorts Tests */
  /* Enable 2 and 5 Wire by default,  Disable Ground Test Delay */
  SysParm.EnaSftFeatures = (ENA_2_WIRE | ENA_5_WIRE);
  SysParm.VIPMode = 0;
  modbusVIPmode = 0;
  SysParm.EU_GND_REF = 0;
  SysParm.Five_Wire_Display = 5;
  SysParm.Ground_Reference = GND_REF_DEFAULT;
  SysParm.DM_Active = FALSE;          /* Active Deadman Enabled */
  SysParm.DM_Max_Open = DM_OPEN;     /* Active Deadman Max open time */
  SysParm.DM_Max_Close = DM_CLOSE;   /* Active Deadman Max close time */ 
  SysParm.DM_Warn_Start = DM_WARN;   /* Active Deadman Warning time */
} /* End nvSysParmDefaults() */

/****************************************************************************
* nvSysDSUpdate -- Update "DateStamp" block in EEPROM
*
* Call is:
*
*   nvSysDSUpdate
*
* On successful return, the "DateStamp" block in the "System" partition has
* been updated. On failure, who knows what has happened...
****************************************************************************/
char nvSysDSUpdate
    (
    DateStampNV *dsptr          /* Pointer to new DateStamp block */
    )
{
  unsigned base;                  /* Base offset of System-NV partition */
  unsigned size;                  /* Size (bytes) of System-NV partition */
  unsigned crc;
  char sts;

  sts = eeMapPartition (EEP_SYSNV, &size, &base);
  if (sts)
      return (sts);
  /* Update the DateStamp block */
  crc = modbus_CRC ((unsigned char *)dsptr,
                    sizeof(DateStampNV) - 2,
                    INIT_CRC_SEED);
  dsptr->CRC = crc;
  sts = eeBlockWrite ((unsigned long)base + sizeof(SysParmNV),
                      (unsigned char *)dsptr,
                      sizeof(DateStampNV));
  if (!pDateStamp)
  {
    nvSysInit();
  }
  return (sts);                       /* Propagate success/failure */
} /* End nvSysDSUpdate() */

/****************************************************************************
* nvSysWrBlock -- Write special parameters block to EEPROM
*
* Call is:
*
*   nvSysWrBlock (etype, bcnt, buf)
*
* Where:
*
*   "etype" is the E2SYS_* block type;
*
*   "bcnt" is the byte count of the data to write;
*
*   "buf" is the pointer to the data buffer to be written to EEPROM.
*
* On successful return, the specified block has been written to the System
* NonVolatile parameters partition.
****************************************************************************/
char nvSysWrBlock
    (
    char etype,                 /* E2SYS_* block type */
    unsigned char bcnt,                  /* Byte count of data */
    char *buf                   /* Address of buffer to write */
    )
{
  unsigned base;                  /* Base offset of System-NV partition */
  unsigned size;                  /* Size (bytes) of System-NV partition */
  unsigned crc;

  unsigned nvoffset;              /* Offset from partition base */
  unsigned int bcntmax;               /* Max/legal size of data block */
  char sts;

// last_routine = 0x25;
  sts = eeMapPartition (EEP_SYSNV, &size, &base);
  if (sts)
      return (sts);

  /* Determine which Sys block to overwrite */

  switch (etype)
  {
    case E2SYS_DIA5:                  /* 5-wire-optic diag/config levels */
      nvoffset = sizeof(SysParmNV) + sizeof(DateStampNV);
      bcntmax = sizeof(SysDia5NV);
      break;

    case E2SYS_VOLT:                  /* System voltage minima/maxima/etc. */
      nvoffset = sizeof(SysParmNV) + sizeof(DateStampNV) + sizeof(SysDia5NV);
      bcntmax = sizeof(SysVoltNV);
      break;

    case E2SYS_SET1:                  /* System config settings */
      nvoffset = sizeof(SysParmNV) + sizeof(DateStampNV) + sizeof(SysDia5NV)
                 + sizeof(SysVoltNV);
      bcntmax = sizeof(SysDia5NV);
      break;

    case E2SYS_PARM:                  /* System general parameters -- error */
    case E2SYS_DSTAMP:                /* DateStamp parameters -- error */
    default:                          /* Unknown special block type */
      return (EE_DATAERROR);          /* Error */
  } /* End of switch (etype) */
  if (bcnt != bcntmax - 2)
  {
    return (EE_DATAERROR);          /* Wrong size */
  }
  /* Update the EEPROM block */
  crc = modbus_CRC ((unsigned char *)buf, bcnt, INIT_CRC_SEED); /* Update CRC */
  buf[bcnt] = (char)(crc >> 8);       /* Stuff the CRC-16 into the buffer */
  buf[bcnt+1] = (char)(crc & 0x00FF); /*  at the end of the "structure" */
  sts = eeBlockWrite ((unsigned long)((unsigned long)base+(unsigned long)nvoffset), (unsigned char *)buf, bcntmax);
  return (sts);                       /* Propagate success/failure */
} /* End nvSysWrBlock() */

/****************************************************************************
*
* Non-Volatile Event Log partition services
*
****************************************************************************/

/****************************************************************************
* nvLogInit -- Initialize Event Logging
*
* Call is:
*
*   nvLogInit ()
*
* On successful return, Event Logging is set up and ready to go, if EEPROM is willing.
*
* The log supports 1024 possible entries.
*
****************************************************************************/
char nvLogInit (void)
{
unsigned long logptr;       /* Pointer to Event Log record */
E2LOGREC log_store;         /* Pointer to Event Log record */
unixtime hitime;            /* Highest time */
unsigned int base;          /* Base offset of Event Log partition */
unsigned int crc;
unsigned long byte_swap;
unsigned int hindex, i;
char sts;

  /*****************************5/21/2008 11:56AM***************************
   * Default in case EE's Event Log block does not look good
   *************************************************************************/
  evMax = 0;                      /* Not allowed to index */
  evIndex = 0;                    /* Starting index if we could write */
  sts = eeMapPartition (EEP_LOG, &evMax, &base);
  /* Map "address" of EEPROM-resident Event Log block */
  if (sts == 0)               /* If OK so far...*/
  /* If can access Event Log block */
  {                                 /* Yes */
    logptr = LOG_BASE;
    evMax /= sizeof(E2LOGREC);       /* Convert size into count of entries */
    hitime = 0;                     /* No times read yet */
    hindex = 0x3FF;                 /* No index yet matched... */
    /* Scan all possible Event Log entries, looking for the "last" (most
       recent -- or "highest" -- date) entry written. */
    for (i = 0; i < evMax; i++)
    {
      if (EEPROM_read(EEPROM_DEVICE, logptr, (unsigned char*)&log_store, sizeof(E2LOGREC)) != 0)
      {
        break;
      }
      byte_swap = long_swap(log_store.Time);  /* Current UCT date/time  */

      if ((byte_swap != 0xFFFFFFFF)  /* Ignore empty slots */
           && (byte_swap > hitime))  /* Check date/time */
      {                       /* Later date/time */
        crc = modbus_CRC (((unsigned char *)&log_store) + E2LOGCRCOFS,
                          (sizeof(E2LOGREC) - E2LOGCRCOFS) - 2,
                          INIT_CRC_SEED);
        if (log_store.CRC == crc) /* Is this a valid entry? */
        {                   /* Yes */
          hitime = byte_swap;
          hindex = i;         /* Remember index of latest */
        }
      }
      logptr+=sizeof(E2LOGREC);   /* Advance to next Event Log entry */
    }
    /* Setup volatile parameters based on Non-Volatile EEPROM */
    hindex++;                       /* Point to "next" free slot */
    if (hindex >= evMax)      /* Off end of Event Log block? */
    {
      hindex = 0;                   /* Yes, wrap back to entry zero */
    }
    evIndex = hindex;               /* Save pointer to first free entry */
    /* Question: what if "hitime" is later than "present_time" ??? Should we
       automatically set the system clock forward? */
  }
  evLastRead = evIndex;             /* Assume TAS/VIPER up to date */
  return (sts);                     /* Return likely success/fail status */
} /* End nvLogInit() */

/****************************************************************************
* nvLogErase -- Erase NonVolatile Event Log store
*
* Call is:
*   nvLogErase ()
*
* On success, nvLogErase returns zero, having overwritten the entire
* NonVolatile Event Log store with an "erased" pattern (all 0xFF bytes).
* On error, an EE_status error value is returned.
*
* Note: nvLogErase() uses eeBlockFill() to "erase" the Event Log Non-
*       Volatile store. As such, it "blocks" until completion.
****************************************************************************/
char nvLogErase ()
{
  unsigned base;                  /* Base offset of Event Log partition */
  unsigned size;                  /* Size (bytes) of Event Log partition */
  char sts;

  sts = eeMapPartition (EEP_LOG, &size, &base);
  if (sts)
  {
    return (sts);
  } 
  /* Block-fill Event Log NonVolatile store with 0xFF ("erase it"). */
  sts = eeBlockFill ((unsigned long)base, 0xFF, size);
  if ( sts)
  {
    return (sts);                   /* Propagate success/failure */
  }
  sts = nvLogInit();
  return (sts);                   /* Propagate success/failure */
} /* End nvLogErase() */

/****************************************************************************
* nvLogGet -- Read/Retrieve an Event Log entry from EEPROM
*
* Call is:
*
*   nvLogGet (index, eptr)
*
* Where:
*
*   "index" is the index (zero-based) of the Event Log entry to read;
*
*   "eptr" is the pointer to the Event Log type-specific Info data block.
*
* nvLogGet() finds and returns one Event Log entry from EEPROM.
*
* The system Event Log is a circular buffer of Event Log Records stored in
* EEPROM, and readable via TAS/VIPER ModBus access. The buffer is fairly
* small, typically 32 entries (1KB of EEPROM space).
*
* On return, the event was retrieved (written) into the specified "eptr" RAM
* buffer, and a zero ("OK") return value is returned.
*
* On error, a non-zero failure status is returned.
****************************************************************************/
char nvLogGet
    (
    unsigned char *eptr,               /* Event type-specific data (32 bytes) */
    unsigned int index                /* Event Log entry index */
    )
{
  unsigned int logptr;            /* EEPROM byte pointer */
  unsigned base;                  /* Base offset of Event Log partition */
  unsigned size;                  /* Size (bytes) of Event Log partition */
  char sts;

  if (evMax == 0)                     /* Event Logging disabled? */
  {
    return (-1);                    /* Yes */
  }
  sts = eeMapPartition (EEP_LOG, &size, &base);
  if (sts)
  {
      return (sts);                   /* No EEPROM access */
  }
  logptr = LOG_BASE + (index * sizeof(E2LOGREC));
  sts = eeReadBlock(logptr, eptr, sizeof(E2LOGREC));
  return sts;                         /* Success return */
} /* End nvLogGet() */

/****************************************************************************
* nvLogPut -- Write an Event Log entry to EEPROM
*
* Call is:
*
*   nvLogPut (etyp, esub, eptr)
*
* Where:
*
*   "etyp" is the Event Log type code;
*
*   "esub" is the Event Log type-specific subcode;
*
*   "eptr" is the pointer to the Event Log type-specific Info data block.
*
* nvLogPut() is the general-purpose event logger routine (see also nvLogMerge()
* and nvLogRepeat() for variations on the theme). It accepts the event type
* and subtype (essentially a 16-bit overall type field) and a pointer to a
* buffer containing the event-specific information to be logged with the
* event (here treated as a blind array of data bytes; the caller is free to
* supply any sort of data).
*
* The system Event Log is a circular buffer of Event Log Records stored in
* EEPROM, and readable via TAS/VIPER ModBus access. The buffer has 1000 entries.
*
* On return, the event was logged (written) into EEPROM and can be read back
* by TAS/VIPER, EEPROM willing. nvLog() has no error return status -- if the
* write fails for any reason, the event is ignored.
****************************************************************************/
void nvLogPut
    (
    char    etyp,               /* Event type code */
    char    esub,               /* Event subtype code */
    const char   *eptr          /* Event type-specific data (22 bytes) */
    )
{
E2LOGREC event;             /* Local event-log-entry buffer */
unsigned base;              /* Base offset of Event Log partition */
unsigned size;              /* Size (bytes) of Event Log partition */
char sts;
unsigned char *data_ptr;
unsigned long byte_swap;

  if (evMax == 0)                     /* Event Logging disabled? */
  {
    return;                         /* Yes */
  }
  sts = eeMapPartition (EEP_LOG, &size, &base);
  if (sts)
  {
    return;                           /* No EEPROM access */
  }
  /* Build the event log record entry */
  event.Type = etyp;                  /* Event type code */
  event.Subtype = esub;               /* Type-specific subcode */
  event.RepMask = 0xFFFF;             /* Repeat Mask == no repeats [yet] */
  // event.Time = present_time;          /* Current UCT date/time */
  data_ptr = (unsigned char *)&event.Time;  /* Fetch location of  */
  /***************************** 11/21/2008 8:59AM *************************
   * Perform a byte swap
   *************************************************************************/
  byte_swap = long_swap(present_time);  /* Current UCT date/time  */
  memcpy (data_ptr, (unsigned char *)&byte_swap, sizeof(long));

  memcpy (event.Info, eptr, sizeof(event.Info));
  event.CRC = modbus_CRC ((unsigned char *)&event+E2LOGCRCOFS,
                          (sizeof(E2LOGREC) - E2LOGCRCOFS) - 2,
                          INIT_CRC_SEED);
  /* Write the event log record into EEPROM */
  sts = eeBlockWrite ((unsigned long)((unsigned long)base + (unsigned long)((unsigned long)sizeof(E2LOGREC) * (unsigned long)evIndex)),
                      (const unsigned char *)&event,
                      sizeof(E2LOGREC));
  if (sts)                            /* Problems writing EEPROM? */
  {
      return;                         /* Yes, punt */
  }
  evIndex++;                          /* Advance Event Log "first free" */
  if (evIndex == evMax)               /* Hit "end" of circular buffer? */
  {
    evIndex = 0;                    /* Yes, wrap to start of buffer */
  } 
} /* End nvLogPut() */

/****************************************************************************
* nvLogMerge -- Write an Event Log entry to EEPROM, allowing Merging
*
* Call is:
*
*   nvLogMerge (etyp, esub, eptr, deltat)
*
* Where:
*
*   "etyp" is the Event Log type code;
*
*   "esub" is the Event Log type-specific subcode;
*
*   "eptr" is the pointer to the Event Log type-specific Info data block;
*
*   "deltat" is the time difference or window to allow merging this Event
*   Log entry with the preceding entry.
*
* nvLogMerge() acts like nvLogPut() insofar as it logs an event into the
* EEPROM Event Log. However, if possible, nvLogMerge will merge this event
* with the "immediately preceding" event if at all possible.
*
* To be "Merged", the immediately preceding Event Log entry must be the same
* type ("etyp" must match), the info blocks must be the same, and the time
* of the preceding entry must be within "deltat" seconds of present time. If
* all of these constraints are met, *AND* TAS/VIPER has not yet read the
* preceding entry (as indicated by evLastRead), then the current "esub" field
* is simply "OR"ed into the preceding event record (note that the CRC does
* not include the type or subtype fields).
*
* On return, the event was logged (written) into EEPROM and can be read back
* by TAS/VIPER, EEPROM willing. nvLogMerge() has no error return status -- if
* the write fails for any reason, the event is ignored. No indication is given
* as to whether a new entry was created, or the new event was merged with a
* previous event.
****************************************************************************/
void nvLogMerge
    (
    char    etyp,               /* Event type code */
    char    esub,               /* Event subtype code */
    const char   *eptr,         /* Event type-specific data (22 bytes) */
    unsigned    deltat          /* Time limit on Merging with previous */
    )
{
E2LOGREC last_log;              /* Temporary location to store last log entry */
E2LOGREC *logptr;               /* Pointer to EEPROM Event Log entry */
unsigned base;                  /* Base offset of Event Log partition */
unsigned size;                  /* Size (bytes) of Event Log partition */
unsigned int lasti, orbits, sts;
unsigned int eeprom_addr;
unsigned long current_time;

  // last_routine = 0x2A;
  if (evMax == 0)                     /* Event Logging disabled? */
  {
      return;                         /* Yes */
  }
  sts = (unsigned int)(unsigned char)eeMapPartition (EEP_LOG, &size, &base);
  if (sts)
  {
    return;                         /* No EEPROM access */
  }
  lasti = evIndex;
  if (lasti == 0)             /* First free entry */
  {
    lasti = evMax;                  /* Wrap back */
  }
  lasti--;                            /* Last entry written */
  eeprom_addr = LOG_BASE + (lasti * sizeof(E2LOGREC));
  if (eeReadBlock(eeprom_addr, (unsigned char *)&last_log, sizeof(E2LOGREC)) != 0)
  {
    EE_status |= EE_FORMAT;         /* EEPROM needs formatting */
    StatusB |= STSB_ERR_EEPROM;        /* Note errors reading EEPROM */
  }
  logptr = (E2LOGREC *)&last_log;     /* Point to stored last record */
  current_time = long_swap(logptr->Time);  /* Current UCT date/time  */
  if ((lasti != evLastRead)           /* If TAS/VIPER hasn't read this yet, */
      && (logptr->Type == etyp)       /*   the types match, */
      && (memcmp(logptr->Info,        /*   the info is unchanged, */
                 eptr,
                 sizeof(logptr->Info)) == 0)
    && (current_time  + deltat > present_time)) /* AND it was fairly recently */
  {                               /* Merge this entry */
    orbits = (char)(logptr->Subtype | esub); /* "Merge" Subtype fields */
    (void)eeWriteByte (base + (lasti * sizeof(E2LOGREC)) + 1,
                       (unsigned char)orbits);
  }
  else
  {                               /* Must write new entry */
    nvLogPut (etyp, esub, eptr);    /* Log this event */
  }
} /* End nvLogMerge() */

/****************************************************************************
* nvLogRepeat -- Write an Event Log entry to EEPROM, allowing Repeating
*
* Call is:
*
*   nvLogRepeat (etyp, esub, eptr, deltat)
*
* Where:
*
*   "etyp" is the Event Log type code;
*
*   "esub" is the Event Log type-specific subcode;
*
*   "eptr" is the pointer to the Event Log type-specific Info data block;
*
*   "deltat" is the time difference or window to allow merging this Event
*   Log entry with the preceding entry.
*
* nvLogRepeat() acts like nvLogPut() insofar as it logs an event into the
* EEPROM Event Log. However, if possible, nvLogRepeat will Repeat this event
* with any "recently preceding" event if at all possible.
*
* To be "Repeatd", a preceding Event Log entry must be the same type ("etyp"
* must match), as well as the same subtype ("esub" must match), and the time
* of the preceding entry must be within "deltat" seconds of present time. If
* all of these constraints are met, *AND* TAS/ VIPER has not yet read the
* preceding entry (as indicated by evLastRead), then the entry is "Repeated".
*
* The "Info" block for the repeated record is ignored.
*
* The "Repeat mask" for the repeated record is "incremented", which means the
* lowest-order 'one' bit is cleared, until all 16 bits have been cleared, at
* which point the event is ignored, yielding a "repeat count" of "1 to 16"
* and "17-or-more" events (a sliding ones=>zeroes mask is used to evenly
* "wear" the EEPROM, by keeping the total one=>zero bit transistions roughly
* the same for all bytes).
*
* On return, the event was logged (written) into EEPROM and can be read back
* by TAS/VIPER, EEPROM willing. nvLogRepeat() has no error return status --
* if the write fails for any reason, the event is ignored. No indication is
* given as to whether a new entry was created, or the new event was Repeated
* with a previous event.
****************************************************************************/
void nvLogRepeat
    (
    char    etyp,               /* Event type code */
    char    esub,               /* Event subtype code */
    const char   *eptr,         /* Event type-specific data (22 bytes) */
    unsigned long deltat        /* Time limit on Repeating previous */
    )
{
E2LOGREC *logptr;               /* Pointer to EEPROM Event Log entry */
E2LOGREC log_store;             /* Pointer to EEPROM Event Log entry */
unsigned base;                  /* Base offset of Event Log partition */
unsigned size;                  /* Size (bytes) of Event Log partition */
unsigned int i, lasti, orbits;
unsigned int eeprom_addr;
unsigned long current_time;
union
{
  unsigned int temp_word;
  unsigned char data[2];
} byte_swap;

  // last_routine = 0x2B;
  if (evMax == 0)                     /* Event Logging disabled? */
  {
    return;                         /* Yes */
  }
  if (eeMapPartition (EEP_LOG, &size, &base) != 0) return;
  lasti = evIndex;                    /* First Free entry slot */
  /* Loop over the preceding "n" (8) entries in the Event Log, looking for
     a "match" to the new entry; if found, just "Repeat" that entry. */
  for (i = 8; i > 0; i--)             /* Check back 8 entries' worth */
  {
    if (lasti == 0)                 /* If at head of buffer */
    {
      lasti = evMax;              /* Wrap back to tail */
    }
    lasti--;                        /* Previous entry written */
    if (lasti == evLastRead)        /* If TAS/VIPER has read this one, */
    {
      i = 0;                      /* Terminate the search, and write */
      break;                      /*  this entry as a new one */
    }
    eeprom_addr = LOG_BASE + (lasti * sizeof(E2LOGREC));
    if (eeReadBlock(eeprom_addr, (unsigned char *)&log_store, sizeof(E2LOGREC)) != 0)
    {
      EE_status |= EE_FORMAT;     /* EEPROM needs formatting */
      StatusB |= STSB_ERR_EEPROM; /* Note errors reading EEPROM */
    }
    logptr = (E2LOGREC *)&log_store;     /* Point to stored last record */
    current_time = long_swap(logptr->Time);  /* Current UCT date/time  */
    if ((logptr->Type == etyp)      /* If the types match, */
        && (logptr->Subtype == esub) /*  and the subtypes match */
        && ((current_time  + deltat) > present_time)) /* AND it was fairly recently */
    {                           /* Repeat this entry */
      /* "Increment" the Repeat Count (aka Repeat Mask) */
      byte_swap.temp_word = logptr->RepMask;
      if ( byte_swap.data[1] == 0)
      {
        byte_swap.data[0] <<= 1;
      } else
      {
        byte_swap.data[1] <<= 1;
      }
      orbits = (unsigned char)(byte_swap.data[0]);
      asm volatile("nop");    /* place for the breakpoint to stop */
      asm volatile("nop");    /* place for the breakpoint to stop */
      asm volatile("nop");    /* place for the breakpoint to stop */
      if (eeWriteByte (base + (lasti * sizeof(E2LOGREC)) + 2,
                             (unsigned char)orbits))
      {
        return; /* Rewrite repeat mask low byte */
      }
      orbits = (unsigned char)(byte_swap.data[1]);
      asm volatile("nop");    /* place for the breakpoint to stop */
      asm volatile("nop");    /* place for the breakpoint to stop */
      asm volatile("nop");    /* place for the breakpoint to stop */
      if (eeWriteByte (base + (lasti * sizeof(E2LOGREC)) + 3,
                             (unsigned char)orbits))
      {
        return; /* Rewrite repeat mask low byte */
      }
      break;                      /* Done searching, entry repeated */
    } /* End if one record repeatable */
  } /* End 'for' loop on finding repeatable entry */
  /* If couldn't repeat any of the "n" preceding Event Log entries, then
     write a brand new Event Log entry instead. */
  if (i == 0)                         /* No repeats? */
  {                                   /* Must write new entry */
      nvLogPut (etyp, esub, eptr);    /* Log this event */
  }
} /* End nvLogRepeat() */

/*********************** end of NVSYSTEM.C **********************************/
