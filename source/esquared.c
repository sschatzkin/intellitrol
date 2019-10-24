 /********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         ESQUARED.C
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais  @Copyright 2009, 2014  Scully Signal Company
 *
 *   Description:    Main program EEPROM routines for Rack controller main
 *                   microprocessor PIC24HJ256GP210
 *
 * Revision History:
 *   Rev      Date   Who  Description of Change Made
 * -------- -------- ---  --------------------------------------------
 *  1.5     09/23/08  KLL  Ported old Intellitrol code to the PIC24HJ256GP210 CPU
 *  1.5.27  03/24/14  DHP  Increased Probe Short and Probe Power levels to
 *                          correctly identify connection of certain devices.
 *  1.5.30  08/10/14  DHP  Added init of SysParm.Ground_Reference to eeFormatSys()
 *  1.6.35  01/31/17  DHP  Removed from eeUpdateSys() the rewriting of the 
 *                          factory options. These should not change!
 *                         Added to eeUpdateSys() the new parameters for 
 *                           the Active Deadman
 *
 *********************************************************************************************/
#include "common.h"

static char eeFormatSys(void);

/****************************************************************************
*
* Low-level EEPROM driver/service code
*
*****************************************************************************/

/****************************************************************************
* eeMapPartition -- Map base address of EEPROM "partition" to physical
* address space
* Call is:
*
*   eeMapPartition (partid, *size, *base)
*
* Where:
*
*   "partid" is the partition identifier (EEP_*);
*
*   "size" is the pointer to the (int) size of the referenced partition;
*
*   "base" is the resultant EEPROM address/offset to reference the base
*    address (via eeMapAddr()) of the requested EEPROM partition or "block".
*
* eeMapPartition() returns an EE_Status value of 0 if successful, or
* error status if unable to access the partition (in particular, EE_INVALID
* if the partition is marked not valid in the Home block).
***************************************************************************/

char eeMapPartition
    (
    unsigned partid,                /* Logical partition ID */
    unsigned *size,                 /* Returned size of partition */
    unsigned *base                  /* Returned base "address" (offset) of partition */
    )
{
    unsigned len, ofs;              /* Local value holders */
    char sts = 0;                   /* Local status */


    /* Get access to the Home block */

  // last_routine = 0xD;
    if (EE_status & (EE_BADHOME | EE_FORMAT))
    {                                   /* Home block unavailable */
        *size = 0;                      /* No partition whatsoever */
        *base = 0;                      /* Invalid address */
        return ((char)EE_status);             /* Return with error status */
    }

    if ((home_block.pat1 != EEHOMEPAT1)
       || (home_block.pat2 != EEHOMEPAT2))
    {
      if (EEPROM_read(EEPROM_DEVICE, (unsigned long)HOME_BASE, (unsigned char *)&home_block, E2HOMESIZ) != 0)
      {
        *size = 0;                      /* Not valid */
        *base = 0;                      /* Not valid */
        return (EE_DATAERROR);          /* Return error status */
      }
    }

    /* Identify and extract partition info from the Home block */

    switch (partid)
    {
      case EEP_HOME:                    /* Asking for Home block */
        sts = 1;                        /* Always works */
        len = E2HOMESIZ;                /* Statically-defined size */
        ofs = 0x0000;                   /* Statically-defined address */
        break;

      case EEP_BOOT:                    /* Boot Block or Partition */
        sts = (char)(home_block.Valid & EEV_BOOTBLK); /* Extract validity bit */
        len = home_block.Bootlen;            /* Boot Block length */
        ofs = home_block.Bootptr;            /* Boot Block base offset */
        break;

      case EEP_CRASH:                   /* Crash Block or Partition */
        sts = (char)(home_block.Valid & EEV_CRASHBLK); /* Extract validity bit */
        len = home_block.Crashlen;           /* Crash Block length */
        ofs = home_block.Crashptr;           /* Crash Block base offset */
        break;

      case EEP_SYSNV:                   /* SysNV Block or Partition */
        sts = (char)(home_block.Valid & EEV_SYSBLK); /* Extract validity bit */
        len = home_block.SysNVlen;           /* SysNV Block length */
        ofs = home_block.SysNVptr;           /* SysNV Block base offset */
        break;

      case EEP_LOG:                     /* Log Block or Partition */
        sts = (char)(home_block.Valid & EEV_LOGBLK); /* Extract validity bit */
        len = home_block.Loglen;             /* Log Block length */
        ofs = home_block.Logptr;             /* Log Block base offset */
        break;

      case EEP_KEY:                     /* Key Block or Partition */
        sts = (char)(home_block.Valid & EEV_KEYBLK); /* Extract validity bit */
        len = home_block.Keylen;             /* Key Block length */
        ofs = home_block.Keyptr;             /* Key Block base offset */
        break;

      case EEP_TIM:                     /* TIM Block or Partition */
        sts = (char)(home_block.Valid & EEV_TIMBLK); /* Extract validity bit */
        len = home_block.TIMlen;             /* TIM Block length */
        ofs = home_block.TIMptr;             /* TIM Block base offset */
        break;

      default:
        *size = 0;                      /* Not valid */
        *base = 0;                      /* Not valid */
        return (EE_DATAERROR);          /* Return error status */

    } /* End switch on partition ID */

    *size = len;                        /* Return partition length */
    *base = ofs;                        /* EEPROM base offset to partition */

    if (sts)                            /* If partition marked Valid/sts>0/OK */
        return GOOD;                     /* Return successful status */
    else                                /* Problems with partition */
        return (EE_DATAERROR);          /* Return error status */

} /* End eeMapPartition() */

/****************************************************************************
* eeReadBlock -- Read block of data to EEPROM
*
* Call is:
*
*   eeReadBlock (offset, dptr, count)
*
* Where:
*
*   "offset" is the EEPROM relative offset;
*
*   "dptr" is the pointer to write the eeprom data into the the caller's
*           data buffer;
*
*   "count" is the count of bytes to be written into EEPROM;
*
* eeReadBlock() copies an arbitrary block (or string) of data bytes into
* the system EEPROM, batching the data into EEPROM "pages" for fastest
* write time.
*
* Note: eeReadBlock() is a synchronous routine -- it runs to full completion
*       before returning. Since EEPROM takes ("on average") 5ms to write
*       (10ms "worst case"), eeReadBlock can block for n * 10ms. The truck-
*       active loop mandates a 30ms nominal cycle time.
*
* On error, eeReadBlock() will return a non-zero EEStatus byte; on success,
* it will return 0.
****************************************************************************/
char eeReadBlock
    (
    unsigned int loc,           /* EEPROM-relative offset */
    unsigned char *dptr,                 /* Pointer to caller's data buffer */
    unsigned int block_count                  /* Count of data bytes to read */
    )
{
UINT16 i;
UINT32  reg_addr;
unsigned char status = 0;

  // last_routine = 0xE;
  reg_addr = loc;
  for ( i=0; i<block_count; i++, reg_addr++, dptr++)
  {
    if ((status = (unsigned char)EEPROM_read(MC24FC1025_DEVICE, reg_addr, (unsigned char *)dptr, 1)) != 0)
    {
      printf("EEPROM read failed!  Status = %u\n\r", (unsigned int)status);
      return (char)status;
    }
  }

  return (char)status;

} /* End eeReadBlock() */


/****************************************************************************
* eeWriteByte -- Write one physical EEPROM/NVRAM data byte
*
* Call is:
*
*   eeWriteByte (offset, data)
*
* Where:
*
*   "offset" is the EEPROM relative offset;
*
*   "data" the desired byte of data to be written into EEPROM.
*
* On error, eeWriteByte() will return a non-zero EEStatus byte; on success,
* it will return 0.
****************************************************************************/

char eeWriteByte
    (
    unsigned int loc,               /* EEPROM-relative offset */
    unsigned char datum             /* Caller's data byte */
    )
{

  // last_routine = 0xF;
  return (char)EEPROM_write(EEPROM_DEVICE, (unsigned long)loc, (unsigned char *)&datum, 1);

} /* End eeWriteByte() */

/****************************************************************************
* eeWriteBlock -- Write block of data to EEPROM
*
* Call is:
*
*   eeWriteBlock (offset, dptr, count)
*
* Where:
*
*   "offset" is the EEPROM relative offset;
*
*   "dptr" is the pointer to read the caller's data buffer;
*
*   "count" is the count of bytes to be written into EEPROM;
*
* eeWriteBlock() copies an arbitrary block (or string) of data bytes into
* the system EEPROM, batching the data into EEPROM "pages" for fastest
* write time. In essence, it is a "memcpy()" into EEPROM.
*
* Note: eeWriteBlock() is a synchronous routine -- it runs to full completion
*       before returning. Since EEPROM takes ("on average") 5ms to write
*       (10ms "worst case"), eeWriteBlock can block for n * 10ms. The truck-
*       active loop mandates a 30ms nominal cycle time. eeWriteBlock()
*       architecturally limits the block-size byte count to 256.
*       M.R. As the software architecture limits the block size to 64 byte max.
*       we are OK.
*
* On error, eeWriteBlock() will return a non-zero EEStatus byte; on success,
* it will return 0.
****************************************************************************/

char eeWriteBlock
    (
    unsigned int loc,             /* EEPROM-relative offset */
    const unsigned char *dptr,    /* Pointer to caller's data buffer */
    unsigned int block_count      /* Count of data bytes to write */
    )
{
UINT16 i, status = 0;
UINT32  reg_addr;

  // last_routine = 0x10;
  reg_addr = loc;
  for ( i=0; i<block_count; i++, reg_addr++, dptr++)
  {
    if ((status = EEPROM_write(MC24FC1025_DEVICE, reg_addr, (const unsigned char *)dptr, 1)) != 0)
    {
      printf("EEPROM write failed!  Status = %d\n\r", (int)status);
      return (char)status;
    }
    // last_routine = 0x10;
  }

  return (char)status;
} /* End eeWriteBlock() */

/****************************************************************************
* eeInit -- Initialize for EEPROM/NVRAM access
*
* Call is:
*
*   eeInit()
*
* eeInit() initializes the Intellitrol system to access the Non-Volatile
* system storage (EEPROM). If the EEPROM appears valid (formatted), then
* the system can actively use the EEPROM (read non-volatile configuration
* information, write system log entries, etc. and so forth).
*
* If the EEPROM appears to have failed (or never been initialized), then
* eeInit() sets the "can't use/needs formatting" flag and returns. If this
* flag is set, then a call to eeFormat must be executed to "format" and
* initialize the actual EEPROM storage.
*
* eeInit() and eeFormat() are split up to allow system startup to access
* NonVolatile storage as early as possible, yet defer the "hard work" of
* formatting/etc. until the system is up far enough that we can xprintf()
* what we are doing...
****************************************************************************/

void eeInit (void)
{
// E2HOMEBLK home;
unsigned long eeprom_addr;
unsigned char *temp_ptr;
unsigned crc;                         /* Temporary storage for Checksum */

  EE_valid = 0;                       /* No access */
  EE_status = 0;                      /* No status yet either */

  // last_routine = 0x11;
  eeprom_addr = HOME_BASE;
  temp_ptr = (unsigned char *)&home_block;
  if (eeReadBlock((unsigned int)eeprom_addr, temp_ptr, sizeof(E2HOMEBLK)) != 0)
  {
    printf("Error Reading EEPROM\n\r");
    EE_status |= EE_FORMAT;         /* EEPROM needs formatting */
    StatusB |= STSB_ERR_EEPROM;        /* Note errors reading EEPROM */
  }

  /* Verify the Home block integrity and software version */


  if ((home_block.pat1 != EEHOMEPAT1)      /* Likely-looking Home block? */
      || (home_block.pat2 != EEHOMEPAT2))  /* checking for Patterns: 0x17 or 0xE8 */
  {                               /* NO!!! patterns do not match */
    if (((home_block.pat1 == 0xFF)  || (home_block.pat1 == 0xF0))
      && (home_block.pat2 == 0xFF))     /* Likely unformatted? */
    {
      EE_status |= EE_NEW;             /* EEPROM needs formatting */
    }else
    {
      EE_status |= EE_FORMAT;         /* EEPROM needs formatting */
      StatusB |= STSB_ERR_EEPROM;     /* Note errors with EEPROM */
    }
  }  else       /* Looks like a Home block, verify the CRC before we trust it */
  {
    crc = modbus_CRC ((unsigned char *)&home_block,
                       sizeof(E2HOMEBLK) - 2,
                       INIT_CRC_SEED);  /* Home block CRC */
  // last_routine = 0x11;
    if (crc != home_block.CRC)               /* Home block data valid? */
    {                                   /* No! Can't use Home Block */
      EE_status |= EE_BADHOME;          /* Bad Home block */
      StatusB |= STSB_ERR_EEPROM;       /* Note errors with EEPROM */
    }
    /* And is it the latest (& 0xFF00) */
    else if((home_block.Version & EE_MAJVERMASK) != (EE_VERSION & EE_MAJVERMASK))
    {
      EE_status |= EE_BADHOME;    /* Bad Home block */
      StatusB |= STSB_ERR_EEPROM; /* Note errors with EEPROM */
    }
    else if((home_block.Version & EE_MINVERMASK) != (EE_VERSION & EE_MINVERMASK))
    {                               /* At this point we know that the major (01) */
                                    /* part of the Version Number passed the inspection */
      EE_status |= EE_UPDATE;       /* Wrong Rev. Number - try updating */
    }
    else    /* Valid Home block, set up system pointers to use it */
    {
      EE_valid = home_block.Valid;     /* Note valid blocks */
    }
  }

   /* Now try to setup the various System NonVolatile data from EEPROM */

  nvSysInit ();

  (void)nvLogInit();
} /* End of eeInit() */

/****************************************************************************
* eeHomePtr -- Return pointer to EEPROM "Home" block
*
* Call is:
*
*   eeHomePtr ()
*
* Returns 20-bit E2HOMEBLK pointer if Home block probably OK (CRC not verified),
*  or NULL if junk.
****************************************************************************/

E2HOMEBLK *eeHomePtr (void)
{
    E2HOMEBLK *eeprom_home=(E2HOMEBLK *)0;            /* Local Home block pointer */
//    char sts;                   /* Local status */

    /* Check access to the Home block */

  // last_routine = 0x12;
    if (EE_status & (EE_BADHOME | EE_FORMAT))
        {                               /* Home block unavailable */
        return ((E2HOMEBLK *)0);                     /* Return with no eeprom_home block */
        }

    /* Verify the Home block integrity */

    if ((home_block.pat1 != EEHOMEPAT1)      /* Likely-looking Home block? */
        || (home_block.pat2 != EEHOMEPAT2))  /* ... */
        {
        return (0);                     /* Bad Home block, return NULL */
        }

    /* We could reverify the CRC here, but why bother... */

    return (eeprom_home);                      /* Return pointer to Home block */

} /* End eeHomePtr() */

/****************************************************************************
* eeReport -- Report the EEPROM status
*
* Call is:
*
*   eeReport()
*
* eeReport() prints out the current status of the EEPROM if in "ASCII"
* mode.
****************************************************************************/

void eeReport (void)
{

  // last_routine = 0x13;
    if (EE_status == 0)
    {
        xprintf( 80, 0);    /* "The EEPROM seems OK; Home block CRC verified" */
        if ((EE_valid & EEV_ALL) == EEV_ALL)
            {
            xprintf( 81, 0);             /* "All blocks valid" */

            return;
            }

        /* Oops, one of the EEPROM blocks is marked invalid; complain, but
           not too strongly... */

        if (!(EE_valid & EEV_BOOTBLK))
            xprintf( 82, EEP_BOOT);      /* "EEPROM ___ block marked invalid" */
        if (!(EE_valid & EEV_CRASHBLK))
            xprintf( 82, EEP_CRASH);
        if (!(EE_valid & EEV_SYSBLK))
            xprintf( 82, EEP_SYSNV);
        if (!(EE_valid & EEV_LOGBLK))
            xprintf( 82, EEP_LOG);
        if (!(EE_valid & EEV_KEYBLK))
            xprintf( 82, EEP_KEY);
        if (!(EE_valid & EEV_TIMBLK))
            xprintf( 82, EEP_TIM);

    } else
    {

      /* Serious Problems with the EEPROM! */
      xprintf( 83, (unsigned int)(EE_status));      /* "Problem with EEPROM:" */
    }

} /* End of eeReport() */

/****************************************************************************
* eeUpdateHome -- Set new EE_VERSION into the EEPROM Home block
*
* Call is:
*
*   eeUpdateHome()
*
* eeUpdateHome() is specifically charged with "updating" the home block with 
* EE_VERSION and updating the CRC.
*
* Return is the EE status (mask of errors; 0 means success), with the system
* EE_status variable updated.
****************************************************************************/
char eeUpdateHome (void)
{
E2HOMEBLK home;
unsigned int i;
unsigned long eeprom_addr;
char *temp_ptr;
//    E2HOMEBLK *Pptr;
//    char    *ptr;               /* Handy dandy scratch byte pointer */
char     sts;          /* Local status */

  // last_routine = 0x14;
  eeprom_addr = HOME_BASE;
  temp_ptr = (char *)&home;
  for ( i=0; i<sizeof(E2HOMEBLK);i++)
  {
    if (EEPROM_read(EEPROM_DEVICE, eeprom_addr++, (unsigned char *)temp_ptr++, 1) != 0)
    {
        EE_status |= EE_FORMAT;         /* EEPROM needs formatting */
        StatusB |= STSB_ERR_EEPROM;        /* Note errors reading EEPROM */
        break;
    }
  }

  // last_routine = 0x14;
  home.Version = EE_VERSION;            /* New Version Number */

  home.CRC = modbus_CRC ((unsigned char *)&home,
                         sizeof(home) - 2,
                         INIT_CRC_SEED); /* New Home block CRC */

  /* Write out the newly-formatted Home block to the EEPROM */
  sts = eeBlockWrite (0x0000L, (unsigned char *)&home, sizeof(E2HOMEBLK));
  // last_routine = 0x14;
  if (sts)
  {
    xprintf( 87, (unsigned int)((unsigned char)sts));              /* Can't read/write home block */
    EE_status |= (sts | EE_BADHOME);  /* Latch error status */
    return ((char)EE_status);             /* Just give up */
  }
  StatusB &= ~STSB_ERR_EEPROM;            /* EEPROM is OK (for now, anyways) */
  return GOOD;
}    /* End of eeUpdateHome */

/****************************************************************************
* eeFormatHome -- Initialize the EEPROM Home block
*
* Call is:
*
*   eeFormatHome()
*
* eeFormatHome() is specifically charged with "formatting" the home block.
* As a side effect, kinda, sorta, it "erases" the entire EEPROM (since the
* home block is the root that points to all the other blocks, they must all
* be re-initialized; eeFormatHome() makes sure the EEPROM is good, or at
* least "erased" and left with no stale data).
*
* Return is the EE status (mask of errors; 0 means success), with the system
* EE_status variable updated.
****************************************************************************/

static char eeFormatHome (void)
{
//    E2HOMEBLK home;             /* Lotsa stack space! */

    unsigned eesize;            /* Size (KB) calculated for EEPROM */
    unsigned parsize;           /* Local partition size, helper */
    unsigned long offset;       /* Accumulated size/starting offset */
    unsigned char valid;        /* Accumulated home_block.Valid */

    char status;                /* Accumulated write/error status */
    char sts, stx;              /* Local status */

    xprintf( 84, 0);            /* "Formatting EEPROM" */

  // last_routine = 0x15;
    (void)Read_Clock();         /* Fetch the intellitrol serial number */

    /* Verify that we can read/write the Home block (first "several" bytes
       of EEPROM) */

  // last_routine = 0x15;
    sts = eeBlockFill (0x0000L, 0x00, E2HOMESIZ); /* Verify Home EEPROM */
  // last_routine = 0x15;
    if (sts)                            /* Any problems? */
        {                               /* Yes */
        xprintf( 85, (unsigned int)((unsigned char)sts)); /* Can't read/write home block */
        EE_status |= (sts | EE_BADHOME);  /* Latch error status */
        return ((char)EE_status);
        }
    sts = eeBlockFill (0x0000L, 0xFF, E2HOMESIZ); /* Verify Home EEPROM */
  // last_routine = 0x15;
    if (sts)                            /* Any problems? */
        {                               /* Yes */
        xprintf( 85, (unsigned int)(0xFF00 | (unsigned char)sts)); /* Can't read/write home block */
        EE_status |= (sts | EE_BADHOME);  /* Latch error status */
        return ((char)EE_status);
        }

    /* Dynamically size the EEPROM, see which chip we are */
    sts = eeWriteByte (0x0000, 0xF0); /* Mark relative offset 0 */
  // last_routine = 0x15;
    if (sts)                            /* Failure? */
        {                               /* Yes */
        xprintf( 85, (unsigned)(0xF000 | (unsigned char)sts)); /* Can't read/write home block */
        EE_status |= (sts | EE_BADHOME);  /* Latch error status */
        return ((char)EE_status);
        }

    eesize = 32;                    /* 256Kb == 32KB */

    xprintf( 86, eesize);

    /* Initialize "Home" block */

    memset ((char *)&home_block, 0xFF, sizeof(home_block)); /* Fill with "erased" bits */

    home_block.pat1 = EEHOMEPAT1;             /* Magic pattern byte 1 */
    home_block.pat2 = EEHOMEPAT2;             /* Magic pattern byte 2 */
    home_block.Version = EE_VERSION;          /* Mark our format/config */
    home_block.Serial[0] = 0;                 /* Only have 6 bytes */
    home_block.Serial[1] = 0;                 /*  of clock/serial number */
    memcpy (&home_block.Serial[2], clock_SN, 6); /* Fill in "Serial Number" */
    home_block.Time = present_time;           /* Time EEPROM/Home block formatted */
    home_block.EESize = eesize;               /* How big we are */

    /* Now "partition" the EEPROM amongst our customers. Each of the EEPROM
       partitions (aka "blocks") is verified to write both zeroes and ones,
       (except TIM, which is too big, and CRC-validates each entry anyways)
       leaving the partition 0xFF-filled (or "erased"). */

    valid = 0;                          /* Clean slate, no problems */
    status = 0;                         /* No accumulated errors yet */

    /* Boot block */

    offset = sizeof(E2HOMEBLK);         /* Starting after the Home block */
    parsize = E2BOOTSIZ;                /* Size of Boot block partition */

    home_block.Bootptr = (unsigned int)offset;              /* Offset to Boot block */
    home_block.Bootlen = parsize;             /* Size of this partition */
    status |= sts = eeBlockFill (offset, 0x00, parsize); /* Verify EEPROM space */
    status |= stx = eeBlockFill (offset, 0xFF, parsize); /* Verify EEPROM space */
    service_charge();             /* Keep Service LED off */
  // last_routine = 0x15;
    if (!(sts | stx))                   /* Any problems? */
        valid |= EEV_BOOTBLK;           /* No, Boot block is valid */
    else
        xprintf( 88, EEP_BOOT);

    /* Hardware "Crash" block */

    offset += parsize;                  /* Skip past Boot block */
    parsize = E2CRASHSIZ;               /* Size of Crash block partition */

    home_block.Crashptr = (unsigned int)offset;             /* Offset to Crash block */
    home_block.Crashlen = parsize;            /* Size of Crash block */
    status |= sts = eeBlockFill (offset, 0x00, parsize); /* Verify EEPROM */
  // last_routine = 0x15;
    status |= stx = eeBlockFill (offset, 0xFF, parsize); /* Verify EEPROM */
  // last_routine = 0x15;
    if (!(sts | stx))                   /* Any problems? */
        valid |= EEV_CRASHBLK;          /* No, Crash block is valid */
    else
        xprintf( 88, EEP_CRASH);
    service_charge();             /* Keep Service LED off */

    /* System NonVolatile parameters block */

    offset += parsize;                  /* Skip past Crash block */
    parsize = E2SYSSIZ;                 /* Size of System/NonVolatile block */

    home_block.SysNVptr = (unsigned int)offset;             /* Offset to System/NonVolatile block */
    home_block.SysNVlen = parsize;            /* Size of System NV block */
    status |= sts = eeBlockFill (offset, 0x00, parsize); /* Verify EEPROM space */
  // last_routine = 0x15;
    status |= stx = eeBlockFill (offset, 0xFF, parsize); /* Verify EEPROM space */
  // last_routine = 0x15;
    if (!(sts | stx))                   /* Any problems? */
        valid |= EEV_SYSBLK;            /* No, System/NV block is valid */
    else
        xprintf( 88, EEP_SYSNV);

    /* Event Log block */

    offset = LOG_BASE;                  /* Skip past SysNV block */
    parsize = E2LOGSIZ;             /* 32KB Size of "Log" block */

    home_block.Logptr = (unsigned int)offset;               /* Offset to "Log" block */
    home_block.Loglen = parsize;              /* Size of Log block */
    status |= sts = eeBlockFill (offset, 0x00, parsize); /* Verify EEPROM space */
  // last_routine = 0x15;
    status |= stx = eeBlockFill (offset, 0xFF, parsize); /* Verify EEPROM space */
  // last_routine = 0x15;
    if (!(sts | stx))                   /* Any problems? */
        valid |= EEV_LOGBLK;            /* No, Event-Log block is valid */
    else
        xprintf( 88, EEP_LOG);

    /* Bypass Key store */

    offset = KEY_BASE;                       /* Skip past Event Log block */
    parsize = E2KEYSIZ;             /* 32KB Size of "KEY" block */

    home_block.Keyptr = (unsigned int)offset;               /* Offset to "KEY" block */
    home_block.Keylen = parsize;              /* Size of KEY block */
    status |= sts = eeBlockFill (offset, 0x00, parsize); /* Verify EEPROM space */
  // last_routine = 0x15;
    status |= stx = eeBlockFill (offset, 0xFF, parsize); /* Verify EEPROM space */
  // last_routine = 0x15;
    if (!(sts | stx))                   /* Any problems? */
        valid |= EEV_KEYBLK;            /* No, Bypass Key block is valid */
    else
        xprintf( 88, EEP_KEY);

    /* Truck Identification Module store */

    offset += parsize;                  /* Skip past Bypass Key block */
    parsize = E2TIMSIZ;             /* 32KB Size of "TIM" block */

    home_block.TIMptr = (unsigned int)offset;               /* Offset to "TIM" block */
    home_block.TIMlen = parsize;              /* Size of TIM block */
    status |= sts = eeBlockFill (offset, 0xFF, parsize); /* Just "erase" TIM */
  // last_routine = 0x15;
    if (!(sts))                         /* Any problems? */
        valid |= EEV_TIMBLK;            /* No, TIM block is valid */
    else
        xprintf( 88, EEP_TIM);

    home_block.Valid = valid;                 /* Mark EEPROM validity in home block */
    home_block.Status = status;               /* Accumulated status */
    EE_status |= status;                /* Mark it in system status too */
    home_block.CRC = modbus_CRC ((unsigned char *)&home_block,
                           sizeof(home_block) - 2,
                           INIT_CRC_SEED); /* New Home block CRC */

    /* Write out the newly-formatted Home block to the EEPROM */

    sts = eeBlockWrite (0x0000L, (unsigned char *)&home_block, sizeof(home_block));
  // last_routine = 0x15;
    if (sts)
        {
        xprintf( 87, (unsigned int)((unsigned char)sts));              /* Can't read/write home block */
        EE_status |= (sts | EE_BADHOME);  /* Latch error status */
        return ((char)EE_status);             /* Just give up */
        }

    StatusB &= ~STSB_ERR_EEPROM;        /* EEPROM is OK (for now, anyways) */

    xprintf( 89, 0);                    /* "OK" */

    return (0);                         /* Successful completion */

} /* End of eeFormatHome() */

/**************************************************************************/

static char eeFormatBoot(void)
{
    return (0);

}  /* End of eeFormatBoot() */

/**************************************************************************/

static char eeFormatCrash(void)
{
    return (0);

}  /* End of eeFormatCrash() */

/**************************************************************************/

static char eeFormatSys(void)
{
  // last_routine = 0x16;
    return nvSysFormat();                      /* Do whatever */

}  /* End of eeFormatSys() */

/**************************************************************************/

char eeUpdateSys(void)
{
  // last_routine = 0x17;
  SysParm.ADCTmaxNV = 2900;            /* QCCC 53 Thermal 3.5 volt max swing */
  SysParm.Ground_Reference = GND_REF_DEFAULT;
  SysParm.EU_GND_REF = 0;
  SysParm.Five_Wire_Display = 5;
  SysParm.DM_Active = FALSE;          /* Active Deadman Enabled */
  SysParm.DM_Max_Open = DM_OPEN;     /* Active Deadman Max open time */
  SysParm.DM_Max_Close = DM_CLOSE;   /* Active Deadman Max close time */ 
  SysParm.DM_Warn_Start = DM_WARN;   /* Active Deadman Warning time */
  SysParm.Cert_Expiration_Mask = 0x00;   /* Active Deadman Warning time */
  SysParm.Unload_Max_Time_min = 240;   /* Active Deadman Warning time */
  SysParm.fuel_type_check_mask = 0x00;
  return nvSysParmUpdate();                        /* Write SysParm to EEPROM */
  // last_routine = 0x17;
}  /* End of eeUpdateSys() */

/**************************************************************************/

static char eeFormatLog(void)
{

//    nvLogInit();
    return nvLogInit();

}  /* End of eeFormatLog() */

/**************************************************************************/

static char eeFormatKey(void)
{
    return (0);

}  /* End of eeFormatKey() */

/**************************************************************************/

static char eeFormatTIM(void)
{
    return (0);

}  /* End of eeFormatTIM() */

/****************************************************************************
* eeFormat -- Initialize the EEPROM data structure
*
* Call is:
*
*   eeFormat()
*
* eeFormat() erases and initializes the system EEPROM for subsequent use
* as a NonVolatile storage medium.
*
* EEPROM is "partitioned" into several distinct areas for use by different
* Intellitrol subsystems (e.g., Authorized Truck Serial Numbers, Event
* Logging). eeFormat() initializes all these areas to a clean startup
* state (e.g, "empty").
*
* Specifically, the "Home" block (or "page" or "partition") is initialized,
* erasing the rest of the EEPROM.
*
* As of this writing (15-Jul-95, -RDH), the other EEPROM partitions are
* left "un-initialized" (erased == 0xFF'ed).
*
* eeInit() and eeFormat() are split up to allow system startup to access
* NonVolatile storage as early as possible, yet defer the "hard work" of
* formatting/etc. until the system is up far enough that we can xprintf()
* what we are doing...(implicitly after clock is read and system time is
* known) assuming I ever add in the implied xprintf calls...
*
* eeFormat() runs to completion in a blocking or synchronous fashion, which
* takes "a second or two".
*
* eeFormat() returns the cumulative EE_status results of the format opera-
* tion(s).
*
* eeInit() should be called afterwards.
****************************************************************************/

char eeFormat (void)
{
    char sts;                   /* Local status */

  // last_routine = 0x18;
    EE_status = 0;                      /* Re-Init EEPROM status */

    sts = eeFormatHome();               /* Master format, init Home block */

    if (sts == 0)                       /* If format successful */
        {
        sts |= eeFormatBoot();          /* Initialize Kernel/Boot block */

        sts |= eeFormatCrash();         /* Initialize Crash block */

        sts |= eeFormatSys();           /* Initialize System/NonVolatile */

        sts |= eeFormatLog();           /* Initialize Event Log block */

        sts |= eeFormatKey();           /* Initialize Bypass Key block */

        sts |= eeFormatTIM();           /* Initialize TIM block */
        }

    loginit ((char)0xFF);                     /* Event-Log a full "Format" op */

    return (sts);

} /* End of eeFormat() */

/*********************** end of ESQUARED.C **********************************/
