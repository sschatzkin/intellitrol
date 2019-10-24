/*********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         modreg.c
 *
 *   Revision:       REV 1.6
 *
 *   Author:         Ken Langlais (KLL); edits by Dave Paquette (DHP)
 *                   @Copyright 2009, 2017  Scully Signal Company
 *
 *   Description:    ModBus "Register" read and write routines
 *
 * Revision History:
 *   Rev      Date     Who   Description of Change Made
 *  ------  --------  ---  --------------------------------------------
 *  1.5     09/23/08  KLL  Ported old Intellitrol code to the PIC24HJ256GP210
 *  1.5.30  08/10/14  DHP  Removed badvaporflag check from mbrNonPermitReg()
 *                         Changed mbrRdReg case 12 return from 3 to unit_type
 *                         Removed mbrRdReg cases 36, 37, 130, 131
 *                         Replaced mbrWrtReg case 7C code with
 *                           set_ground_reference()
 *                         Changed mbrWrtReg case 11B to 11e to match read
 *                         Removed mbrWrtReg cases 130, 131 
 *  1.5.32  03/25/15  DHP  Added code to register 0x2D read to report 0 for
 *                           no truck or 0xFF for non-dry
 *  1.6.35  02/03/17  DHP  Added code for new Active deadman registers 80-83
 *                         Replaced REG_RES_VAL with an appropriate error code;
 *                           changes a good return to the appropriate error code.
*******************************************************************************/

#include "common.h"
#include "version.h"

/*************************************************************************
* mbrGetEESize -- Return EEPROM size
**************************************************************************/

static unsigned mbrGetEESize (void)
{
    E2HOMEBLK *hptr;            /* Pointer to Home block */

    hptr = eeHomePtr();         /* Retrieve pointer to Home block */
    if (hptr == 0)              /* OK? */
        return (0);             /* No EEPROM (for practical purposes) */

    return (hptr->EESize);      /* Return Home block's recorded size */

} /* End mbrGetEESize() */

/*************************************************************************
* mbrNonPermitReg -- Calculate 16-bit "Non-Permissive Reasons" Register
*
* Call is:
*
*       mbrNonPermitReg ()
*
* Returns calculated set of flags as to why the Intellitrol is non-permissive
*
* While intended primarily as an indication of why the Intellitrol is not
* permissive, VIPER-II also tends to use it as a "Ground LED" state . . .
* so make it a little more defensive that Intellitrol itself!
*
* Really should take the time out to define the 16 bits as (e.g.) NPR_xxx
* defines, rather than just stealing the bit values from other flags...
**************************************************************************/

static unsigned mbrNonPermitReg (void)
{
    unsigned hval;                  /* Non-Permit flags */

    /* This value is built up "on the fly" by snooping the rest
       of the Intellitrol's state... */

    hval = 0;                           /* Initialize mask */

    if (bystatus & BYS_NONBYPASS)       /* Wait before probe/overfill bypass */
        hval |= (unsigned)(BYS_NONBYPASS << 8);  /* Yes, overrides all other */

    if (!((tank_state == T_INIT)        /* If "Truck" in not IDLE */
          || (tank_state == T_DRY)))    /*   and is not Dry */
        {                               /* Then it's in overfill condition */
        if (!(bylevel & OVER_BYPASS))   /* If not already bypassed, */
            hval |= OVER_BYPASS;        /* Non-Permit due to probe/overfill */
        if (bystatus & BYS_WAIT_OVFB)   /* Wait before probe/overfill bypass? */
            hval |= (unsigned)(BYS_WAIT_OVFB << 8);  /* Yes, transient locked out */
        if (bystatus & BYS_DRY_NOOVFB)  /* Dry-once, can't bypass overfill? */
            hval |= (unsigned)(BYS_DRY_NOOVFB << 8);  /* Yes, indicate locked out */
        }

    if ((badgndflag & GND_PROBLEMS)     /* Ground ? */
        && (StatusA & STSA_TRK_PRESENT) /* ... but only if truck connected */
        && (!(bylevel & GROUND_BYPASS))) /*   and if not already bypassed */
        {                               /* Yes */
        hval |= GROUND_BYPASS;          /* Non-Permit due to Ground Fault */
        }

    if (((badvipflag & 0xFEFF))                    /* Truck ID? */
        && (!(bylevel & VIP_BYPASS)))   /*   and not already bypassed? */
        {                               /* Yes */
        hval |= VIP_BYPASS;             /* Non-Permit due to Unauthorized */
        }

    if (baddeadman)                     /* Deadman switch? */
        {                               /* Yes */
        hval |= DEADMAN_bypass;         /* Non-Permit due to Deadman open */
        }

    if (StatusB & STSB_SHUTDOWN)        /* Software non-permit "Shutdown"? */
        {                               /* Yes */
        hval |= 0x8000;                 /* Non-Permit due to Shutdown state */
        }

    if (StatusB & STSB_NO_PERMIT)       /* Unit in hard "Fault" state? */
        {                               /* Yes */
        hval |= 0x4000;                 /* Non-Permit due to hard fault */
        }

    if (StatusB & STSB_SPECIAL)         /* Unit in "Special" mode? */
        {                               /* Yes */
        hval |= 0x2000;                 /* Non-Permit due to Special Ops mode */
        }

    return (hval);                      /* Return 16-bit Non-Permit flags */

    } /* End mbrNonPermitReg() */

/*************************************************************************
* mbrVIPModeWr -- Set "VIP Mode Control" register
*
* Call is:
*
*       mbrVIPModeWr ()
*
* This routine implements the semantics of the VIP Mode Control register.
*
**************************************************************************/

static MODBSTS mbrVIPModeWr
    (
    unsigned    mode                /* VIP Mode */
    )
{
MODBSTS sts;

    if (((SysParm.EnaPassword & ENA_VIP) == 0)  /* Allowed to use VIP? */
        || ((enable_jumpers & ENA_VIP) == 0)) /* Hardware enabled for VIP? */
        return (MB_EXC_ILL_ADDR);       /* VIP not enabled/enable'able */

    switch (mode)                       /* Dispatch on mode control */
        {
      case 0:                           /* 00 -- "Normal" operation */
        SysParm.EnaFeatures |= ENA_VIP; /* Turn on normal VIP ops */
        SysParm.VIPMode = 0;      /* Remember the Mode JGS Rev 1.8 */
        modbusVIPmode = 0;
        SysParm.TASWait = 0;            /* Disable Wait for TAS */
        (void)nvSysParmUpdate();        /* Force EEPROM update */
		   return (MB_OK);

      case 1:                           /* 01 -- "Bypass" operation */
        if ((SysParm.EnaFeatures & ENA_VIP)  /* VIP turned on? */
            && (main_state == IDLE))    /*  and Truck activity? */
            return (MB_EXC_ILL_DATA);   /* No - Nothing to bypass! */
        modbusVIPmode = 1;
        sts = (MODBSTS)mbxForce(0x0015, 1);     /* Otherwise "force" a bypass */
       return (sts);


      case 2:                           /* 02 -- "Unauthorize" operation */
         if ((SysParm.EnaFeatures & ENA_VIP)  /* VIP turned on? */
            && (main_state == IDLE))    /*  and Truck activity? */
            return (MB_EXC_ILL_DATA);   /* No - Not Applicable */
         modbusVIPmode = 2;
		   StatusA &= ~STSA_TRK_VALID;     /* This TID unauthorized */
         badvipflag |= (BVF_TASDENY | BVF_UNAUTH); /* This TID unauthorized */
         badvipflag &= ~BVF_TASDELAY;    /* No longer waiting for TAS */
         clr_bypass (OVER_BYPASS);       /* Defeat VIP bypass too! */
         return (MB_OK);

      case 3:                           /* 03 -- "Authorize" operation */
         if ((SysParm.EnaFeatures & ENA_VIP)  /* VIP turned on? */
            && (main_state == IDLE))    /*  and Truck activity? */
            return (MB_EXC_ILL_DATA);   /* No - Not Applicable */
         modbusVIPmode = 3;
         StatusA |= STSA_TRK_VALID;      /* This TID authorized */
         badvipflag = 0;                 /* No bypass needed */
         return (MB_OK);


      case 4:                           /* 04 -- "Perm. Auth" operation */
         /* It's not real clear what this means vis-a-vis the Intellitrol.
            Just turn off "VIP" function (same effect...I guess). */
         SysParm.EnaFeatures &= ~ENA_VIP; /* No VIP == always "Authorize" */
         SysParm.VIPMode = 4;
         modbusVIPmode = 4;
         (void)nvSysParmUpdate();       /* Force EEPROM update */
         return (MB_OK);

      case 5:                           /* 05 -- "Read VIP Only JGS Rev 1.8 */
         if (SysParm.EnaFeatures & ENA_VIP)  /* VIP turned on? */
         {
           SysParm.VIPMode = 5;      /* VIP Read Only */
           modbusVIPmode = 5;
           (void)nvSysParmUpdate();       /* Force EEPROM update */
           return (MB_OK);
         }
         return (MB_EXC_ILL_DATA); /* No - Not Applicable */

      default:
         return (MB_EXC_ILL_DATA);       /* Illegal mode control */

    } /* End switch on VIP mode control */

} /* End mbrVIPModeWr() */

/****************************************************************************
    Main routines -- mbrRdReg and mbrWrReg
****************************************************************************/

/*************************************************************************
* mbrRdReg -- Read 16-bit "ModBus Register"
*
* Call is:
*
*       mbrRdReg (regno, value)
*
* Where:
*
*       <regno> is the Register Number to be read;
*
*       <value> is the pointer to where the resultant 16-bit value should
*       be returned.
*
* mbrRdReg() is responsible for figuring out what is meant by a ModBus
* 16-bit "Holding Register", and returning the appropriate 16-bit data
* value associated with "reading" that register. The "register value"
* may be an immediate constant, a pointer to an static or dynamic memory
* location, or a function call that will generate the value on the fly.
*
* Return value is MB_OK (0) normally, or an exception value (~0) if there
* are any problems reading the register:
*
*   MB_EXC_ILL_ADDR if attempting to read an undefined register (note that
*                   "Reserved" registers are "defined" and always return 0)
*   MB_EXC_FAULT    if some sort of processing error arises
*   <anything>      "Exception" return from a register-function; the error
*                   status is merely propagated back to the caller.
*
*************************************************************************/

MODBSTS mbrRdReg
    (
    unsigned int regno,         /* 16-bit ModBus "Register" number */
    unsigned int *value         /* Pointer to return 16-bit register data */
    )
{
  char *ptr;                  /* Scratch pointer */
  unsigned hval;              /* Holding value (less code generated) */
  unsigned hxxx;              /* Extra value (scratch word) */
  unsigned int group;        /* Register mod 0x100 */
  unsigned int block;                 /* Register group mod 0x10 */
  unsigned int gbreg;                 /* Register within group/block */
  MODBSTS sts;                /* Status holding */
  unsigned int read_addr;
  unsigned char read_buffer[10];
  unsigned int div_res;
  unsigned int mod_res;
  unsigned int base_addr;
  unsigned int offset;
  unsigned int wtmp;

  group = (unsigned int)(regno >> 8);         /* Extract Register group */
  block = (unsigned int)(regno & 0x00F0);     /* Extract "block" within group */
  gbreg = (unsigned int)(regno & 0x000F);     /* Extract Register within block */

  sts = MB_OK;                        /* Assume OK until fails */
  switch (group)                      /* Dispatch based on Register Group */
  {
    case 0x00:                        /* 0x000 VIP/Configuration registers */
    {
    /* ================================ */
    /* === Register block 00 - 0FF  === */
    /* ================================ */

      /* Dispatch based on block within VIP/Config group */

      switch (block)
      {
        case 0x00:                    /* 000-00F -- VIP standard registers */
          switch (gbreg)
              {
            case 0x0:                 /* 000 -- VIP Year */
              UNIX_to_Greg ();        /* Update year/month/day/hour/min */
                                      /* Ergo "time" only accurate if read
                                         as a block starting with "year" */
              hval = year;            /* Return Year */
              break;

            case 0x1:                 /* 001 -- VIP Month */
              hval = month;           /* Return Month; see "Year" above. */
              break;

            case 0x2:                 /* 002 -- VIP Day */
              hval = day;             /* Return Day; see "Year" above. */
              break;

            case 0x3:                 /* 003 -- VIP Hour */
              hval = hour;            /* Return Hour; see "Year" above. */
              break;

            case 0x4:                 /* 004 -- VIP Minute */
              hval = minute;          /* Return Minute; see "Year" above. */
                                      /* For "Second" resolution, use
                                         Registers 100 & 101 */
              break;

            case 0x5:                 /* 005 -- Shell version */
              hval = (unsigned int)SHELLVER;
              break;

            case 0x6:                 /* 006 -- VIP Serial Number */
            case 0x7:                 /* 007 -- Latest Log pointer */
              hval = 0xFFFF;          /* VIP-style inaccessible! */
              break;

            case 0x8:                 /* 008 -- VIP TAS Delay */
              hval = SysParm.TASWait; /* Active NonVolatile copy */
              break;

            case 0x9:                 /* 009 -- VIP Bypass Time-Out */
              hval = SysParm.BypassTimeOut; /* Return seconds */
              break;

            case 0xA:                 /* 00A -- VIP Terminal number */
              hval = SysParm.Terminal; /* Return terminal number */
              break;

            case 0xB:                 /* 00B -- VIP ModBus Turnaround Delay */
              hval = SysParm.ModBusRespWait;
              break;

            case 0xC:                 /* 00C -- Shell Checksum/CRC */
              hval = ShellCRCval;     /* Calculated Shell CRC-16 */
              break;

            case 0xE:         /* 00E -- Read the VIP Mode Control */
              hval = modbusVIPmode;
              break;

            case 0xF:                 /* 00F -- Kernel version */
              hval = SHELLVER;
              break;

            default:                  /* Can't get here on good code */
              hval = MB_EXC_FAULT;
              break;
              } /* End switch on gbreg for group 000 block 00 */

          break;

        case 0x10:                    /* 010-01F -- Standard registers */
          switch (gbreg)
              {
            case 0x02:                /* 012 -- Scully type/model */
              hval = unit_type;       /* INTELLITROL_PIC is type "3",
                                         INTELLITROL2 = "4"*/
              break;

            default:                  /* Others are an error */
              return(MB_EXC_ILL_ADDR);
              break;
              } /* End switch on gbreg for group 000 block 10 */

          break;

        case 0x20:                    /* 020-02F -- Static config */
          switch (gbreg)
              {
            case 0x0:                 /* 020 -- Unit Serial Number (1/4) */
              hval = 0;               /* Bits 63:48 zero */
              break;

            case 0x1:                 /* 021 -- Unit Serial Number (2/4) */
              hval = (unsigned)(((unsigned)clock_SN[0] << 8)
                      | (unsigned)clock_SN[1]);
              break;

            case 0x2:                 /* 022 -- Unit Serial Number (3/4) */
              hval = (unsigned)(((unsigned)clock_SN[2] << 8)
                      | (unsigned)clock_SN[3]);
              break;

            case 0x3:                 /* 023 -- Unit Serial Number (4/4) */
              hval = (unsigned)(((unsigned)clock_SN[4] << 8)
                      | (unsigned)clock_SN[5]);
              break;

            case 0x4:                 /* 024 -- Hardware Revision Level */
              hval = SHELLVER;    /* Get Hardware rev level */
              break;

            case 0x5:                 /* 025 -- System Configuration "A" */
              hval = (unsigned)(((unsigned)ConfigA << 8)
                      | (unsigned)enable_jumpers);
              break;

            case 0x6:                 /* 026 -- System Configuration "B" */
              hval = (unsigned)(((unsigned int)SysParm.ConfigB << 8)
                      | (unsigned int)SysParm.EnaFeatures);
              break;

            case 0x7:                 /* 027 -- System Configuration "C" */
              hval = (unsigned)((unsigned)ConfigC << 8);
              break;

            case 0x8:                 /* 028 -- System Configuration "D" */
              hval = 0;               /* Yet more future */
              break;

            case 0x9:                 /* 029 -- RAM size (in KB) */
              hval = 2;               /* ***ATTENTION*** */
              break;

            case 0xA:                 /* 02A -- FLASHRAM size (in KB) */
              hval = 128;             /* ***ATTENTION*** */
              break;

            case 0xB:                 /* 02B -- EEPROM size (in KB) */
              hval = mbrGetEESize();  /* Retrieve size from Home block */
              break;

            case 0xC:                 /* 02C -- Max ModBus message size */
              hval = MODBUS_MAX_LEN;  /*  (Includes addr/header/CRC) */
              break;

            case 0xD:                 /* 02D -- number of 5 wire probes */
              if (main_state  != ACTIVE)
              {
                 hval = 0;                    /*  Report 0 for no truck */
              }
              else
              {
                 if (tank_state == T_DRY)
                 {
                   hval = number_of_Probes;  /*  found for the current truck */
                 }
                 else 
                 {
                   hval = 0xFF;
                 }
              }
              break;

            case 0xE:                 /* 02E -- Enable Modbus Features */
              hval = SysParm.Modbus_Ena_Features;  /*  */
              break;

            default:                  /* Others are an error */
              return(MB_EXC_ILL_ADDR);
              break;
              } /* End switch on gbreg for group 000 block 20*/
          break;

        case 0x30:                    /* 030-03F -- Voltage/references */
          switch (gbreg)
              {
            case 0x0:                 /* 030 -- Reference volt */
              hval = ReferenceVolt;   /* Should be 1.000 (1000mv) */
              break;

            case 0x1:                 /* 031 -- Raw Power Supply (13 V) */
              hval = Raw13Volt;       /* 13V for 110/220 AC input */
              break;

            case 0x2:                 /* 032 -- Probe Bias Voltage*/
              hval = BiasVolt;        /* Millivolts */
              break;

            case 0x3:                 /* 033 -- 5-Wire Optic Pulse */
              hval = Optic5Volt;      /* Millivolts */
              break;

           /* Note well: The channels reported are the actual hardware
               channels, which may or may not directly translate into
               "Probes" -- if "6" compartment (USA) jumper, then channels
               "0" and "1" are 'undefined', and channels "2" to "7" are
               probes "1" to "6", with probes "7" and "8" undefined;
               if "8" compartment trucks, then channels "0" to "7" are
               probes "1" to "8"; unless of course you're talking about
               "5-wire optic" in which case the rules are different...
               (channels "not applicable", probes calculated via the
               channel 5 "diagnostic" line) */

            case 0x8:                 /* 038 -- Channel 0 Noise Voltage */
              hval = (unsigned int)(noise_volt[0]); /* +- Millivolts */
              break;

            case 0x9:                 /* 039 -- Channel 1 Noise Voltage */
              hval = (unsigned int)(noise_volt[1]); /* +- Millivolts */
              break;

            case 0xA:                 /* 03A -- Channel 2 Noise Voltage */
              hval = (unsigned int)(noise_volt[2]); /* +- Millivolts */
              break;

            case 0xB:                 /* 03B -- Channel 3 Noise Voltage */
              hval = (unsigned int)(noise_volt[3]); /* +- Millivolts */
              break;

            case 0xC:                 /* 03C -- Channel 4 Noise Voltage */
              hval = (unsigned int)(noise_volt[4]); /* +- Millivolts */
              break;

            case 0xD:                 /* 03D -- Channel 5 Noise Voltage */
              hval = (unsigned int)(noise_volt[5]); /* +- Millivolts */
              break;

            case 0xE:                 /* 03E -- Channel 6 Noise Voltage */
              hval = (unsigned int)(noise_volt[6]); /* +- Millivolts */
              break;

            case 0xF:                 /* 03F -- Channel 7 Noise Voltage */
              hval = (unsigned int)(noise_volt[7]); /* +- Millivolts */
              break;

            default:                  /* Others are an error */
              return(MB_EXC_ILL_ADDR);
              break;
              } /* End switch on gbreg for group 000 block 30 */
          break;

        case 0x40:                    /* 040-04F -- 10/20-Volt Rail levels */
          switch (gbreg)
              {
            case 0x0:                 /* 040 -- Channel 0 10-Volt Rail */
              hval = open_c_volt[0][0]; /* Millivolts */
              break;

            case 0x1:                 /* 041 -- Channel 1 10-Volt Rail */
              hval = open_c_volt[0][1]; /* Millivolts */
              break;

            case 0x2:                 /* 042 -- Channel 2 10-Volt Rail */
              hval = open_c_volt[0][2]; /* Millivolts */
              break;

            case 0x3:                 /* 043 -- Channel 3 10-Volt Rail */
              hval = open_c_volt[0][3]; /* Millivolts */
              break;

            case 0x4:                 /* 044 -- Channel 4 10-Volt Rail */
              hval = open_c_volt[0][4]; /* Millivolts */
              break;

            case 0x5:                 /* 045 -- Channel 5 10-Volt Rail */
              hval = open_c_volt[0][5]; /* Millivolts */
              break;

            case 0x6:                 /* 046 -- Channel 6 10-Volt Rail */
              hval = open_c_volt[0][6]; /* Millivolts */
              break;

            case 0x7:                 /* 047 -- Channel 7 10-Volt Rail */
              hval = open_c_volt[0][7]; /* Millivolts */
              break;

            case 0x8:                 /* 048 -- Channel 0 20-Volt Rail */
              hval = open_c_volt[1][0];   /* Millivolts */
              break;

            case 0x9:                 /* 049 -- Channel 1 20-Volt Rail */
              hval = open_c_volt[1][1];   /* Millivolts */
              break;

            case 0xA:                 /* 04A -- Channel 2 20-Volt Rail */
              hval = open_c_volt[1][2];   /* Millivolts */
              break;

            case 0xB:                 /* 04B -- Channel 3 20-Volt Rail */
              hval = open_c_volt[1][3];   /* Millivolts */
              break;

            case 0xC:                 /* 04C -- Channel 4 20-Volt Rail */
              hval = open_c_volt[1][4];   /* Millivolts */
              break;

            case 0xD:                 /* 04D -- Channel 5 20-Volt Rail */
              hval = open_c_volt[1][5];   /* Millivolts */
              break;

            case 0xE:                 /* 04E -- Channel 6 20-Volt Rail */
              hval = open_c_volt[1][6];   /* Millivolts */
              break;

            case 0xF:                 /* 04F -- Channel 7 20-Volt Rail */
              hval = open_c_volt[1][7];   /* Millivolts */
              break;

            default:                  /* Getting here is bad code */
              hval = MB_EXC_FAULT;
              break;
              } /* End switch on gbreg for group 000 block 40 */
          break;

        case 0x50:                    /* 050-05F -- Even more voltages! */
          switch (gbreg)
              {
            case 0x0:                 /* 050 -- Channel 0 Current Voltage */
              hval = probe_volt[0];   /* Millivolts */
              break;

            case 0x1:                 /* 051 -- Channel 1 Current Voltage */
              hval = probe_volt[1];   /* Millivolts */
              break;

            case 0x2:                 /* 052 -- Channel 2 Current Voltage */
              hval = probe_volt[2];   /* Millivolts */
              break;

            case 0x3:                 /* 053 -- Channel 3 Current Voltage */
              hval = probe_volt[3];   /* Millivolts */
              break;

            case 0x4:                 /* 054 -- Channel 4 Current Voltage */
              hval = probe_volt[4];   /* Millivolts */
              break;

            case 0x5:                 /* 055 -- Channel 5 Current Voltage */
              hval = probe_volt[5];   /* Millivolts */
              break;

            case 0x6:                 /* 056 -- Channel 6 Current Voltage */
              hval = probe_volt[6];   /* Millivolts */
              break;

            case 0x7:                 /* 057 -- Channel 7 Current Voltage */
              hval = probe_volt[7];   /* Millivolts */
              break;

            default:                  /* Others are an error */
              return(MB_EXC_ILL_ADDR);
              break;
              } /* End switch on gbreg for group 000 block 50 */
          break;

        case 0x60:                    /* 060-06F -- Assorted States/etc. */
          switch (gbreg)
              {
            case 0x0:                 /* 060 -- Read Clock Status */
              hval = (unsigned)(unsigned char)clock_status;    /* Last clock access status */
              break;

            case 0x1:                 /* 061 -- Relay Status */
              hval = (unsigned)(((unsigned)(unsigned char)BackRelaySt << 8)
                      | (unsigned)(unsigned char)MainRelaySt);
              break;

            case 0x2:                 /* 062 -- EEPROM status */
              hval = (unsigned)(((unsigned)(unsigned char)EE_valid << 8)
                      | (unsigned)(unsigned char)EE_status);
              break;

            case 0x4:                 /* 064 -- "Acquire" state */
              hval = (unsigned)acquire_state;
              break;

            case 0x5:                 /* 065 -- "Probe-Try" state */
              hval = (unsigned)probe_try_state;
              break;

            case 0x6:                 /* 066 -- Optic 5-wire state */
              hval = (unsigned)optic5_state;
              break;

            case 0x7:                 /* 067 -- Optic/Thermal 2-wire state */
              hval = (unsigned)two_wire_state;
              break;

            case 0x8:                 /* 068 -- VIP status (inc DateStamp) */
              hval = badvipflag;
              break;

            case 0x9:                 /* 069 -- Service "A" flags */
              hval = iambroke;        /* Voltage error details, etc. */
              break;

            case 0xA:                 /* 06A -- Service "B" flags */
              hval = iamsuffering;    /* Hard-wired relays etc. */
              break;

            case 0xB:                 /* 06B -- VIP status (inc DateStamp) */
              hval = fuel_type_fails;
              break;
              
            case 0xC:                 /* 06B -- VIP status (inc DateStamp) */
              hval = (unsigned)(((unsigned)(unsigned char)badvipdscode << 8)
                      | ((unsigned)(unsigned char)badvipflag & 0xFF));
              break;

            case 0xD:                 /* 06D -- Ground status */
              hval = (unsigned)(unsigned char)badgndflag;
              break;

            case 0xE:                 /* 06C -- VIP status (inc DateStamp) */
              hval = badvipflag;
              break;

            case 0xF:                 /* 06C -- VIP status (inc DateStamp) */
              hval = cert_ds_fails;
              break;
              
            default:                  /* Others are an error */
              return(MB_EXC_ILL_ADDR);
              break;
              } /* End switch on gbreg for group 000 block 60 */
          break;

        case 0x70:                    /* 070-07F -- Settable config/parm */
          switch (gbreg)
          {
            case 0x00:                 /* 70 -- Shorts test active */
              hval = (unsigned)SysParm.Ena_INTL_ShortNV;
            break;
            case 0x01:                 /* 71 -- Debug Pulse on Code */
              hval = (unsigned)(unsigned char)SysParm.Ena_Debug_Func_1;
            break;
            case 0x02:                 /* 72 -- Debug Pulse on Code */
              hval = (unsigned)(unsigned char)SysParm.Ena_Debug_Func_2;
            break;
            case 0x03:                 /* 73 -- Debug Pulse on Code */
              hval = (unsigned)(unsigned char)SysParm.Ena_Debug_Func_3;
            break;
            case 0x04:                 /* 74 -- Debug Pulse on Code */
              hval = (unsigned)(unsigned char)SysParm.Ena_Debug_Func_4;
            break;
            case 0x05:                 /* 75 -- Debug Pulse on Failed Code */
              hval = (unsigned)(unsigned char)SysParm.Ena_Debug_Fail_1;
            break;
            case 0x06:                 /* 76 -- Debug Pulse on Failed Code */
              hval = (unsigned)(unsigned char)SysParm.Ena_Debug_Fail_2;
            break;
            case 0x07:                 /* 77 -- Debug Pulse on Failed Code */
              hval = (unsigned)(unsigned char)SysParm.Ena_Debug_Fail_3;
            break;
            case 0x08:                 /* 78 -- Debug Pulse on Failed Code */
              hval = (unsigned)(unsigned char)SysParm.Ena_Debug_Fail_4;
            break;
            case 0x09:                 /* 79 -- Software Feature Enable Code */
              hval = (unsigned)(unsigned char)SysParm.EnaSftFeatures;
            break;
            case 0x0A:                 /* 7A -- Software Feature Enable Code */
              if ( SysParm.EnaSftFeatures & ENA_GRND_DELAY)
              {
                hval = 1;
              } else
              {
                hval = 0;
              }
            break;
            case 0x0B:                 /* 7B -- TIM read enable WPW */
                hval = (unsigned)(unsigned char)SysParm.Ena_TIM_Read;
            break;
            case 0x0C:                 /* 7C -- Resistive Ground reference value */
                hval = (unsigned)(unsigned char)SysParm.EU_GND_REF;
            break;
            case 0x0E:                 /* 7E -- TIM read enable WPW */
                hval = (unsigned)(unsigned char)SysParm.Ena_GND_Display;
            break;
            case 0x0F:                 /* 7F -- 5-Wire Compartment Count Display Time */
                hval = (unsigned)(unsigned char)SysParm.Five_Wire_Display;
            break;

          default:                  /* Others are an error */
              return(MB_EXC_ILL_ADDR);
              break;
          } /* End switch on gbreg for group 000 block 70 */
          break;

        case 0x80:                    /* 080-08F -- Settable config/parm */
          switch (gbreg)
          {
            case 0x00:                /* 80 -- Active Deadman Enabled */
              hval = SysParm.DM_Active;
            break;

            case 0x01:                /* 81 -- Active Deadman Max open time */
              hval = (SysParm.DM_Max_Open >> 2);
            break;

            case 0x02:                /* 82 -- Active Deadman Max close time */
              hval = (SysParm.DM_Max_Close >> 2);
            break;

            case 0x03:                /* 83 -- Active Deadman Warning time */
              hval = (SysParm.DM_Warn_Start >> 2);
            break;

            case 0x04:                 /* 84 -- Software Feature Enable Unload Term */
              if(SysParm.EnaSftFeatures & ENA_UNLOAD_TERM)
              {
                  hval = 0xFF;
              }
              else
              {
                  hval = 0;
              }
            break;
              
            case 0x05:                /* 86 -- Supertim max unload time */
              hval = (SysParm.Unload_Max_Time_min);
            break;

            case 0x06:                /* 86 -- Supertim cert date enable mask */
              hval = (SysParm.Cert_Expiration_Mask);
            break;

            case 0x07:                 /* 87 -- Software Feature Enable compartment count check */
              if(SysParm.EnaSftFeatures & ENA_CPT_COUNT)
              {
                  hval = 0xFF;
              }
              else
              {
                  hval = 0;
              }
            break;
              
            case 0x08:                /* 88 -- Supertim fuel type check mask */
              hval = (SysParm.fuel_type_check_mask);
            break;

            case 0x09:                 /* 89 -- Software Feature Enable2 auto write fuel type flag */
              if(SysParm.EnaSftFeatures2 & ENA_AUTO_FUEL_TYPE_WRITE)
              {
                  hval = 0xFF;
              }
              else
              {
                  hval = 0;
              }
            break;
              
            case 0x0A:                /* 8A -- Supertim default fuel type msb and middle byte */
              wtmp = (unsigned int)(SysParm.default_fuel_type[0] << 8);
              wtmp |= (unsigned int)(SysParm.default_fuel_type[1]);
              hval = wtmp;
            break;

            case 0x0B:                /* 8B -- Supertim default fuel type lsb */
              wtmp = 0;
              wtmp |= (unsigned int)(SysParm.default_fuel_type[2] << 8);
              hval = wtmp;
            break;
          
          default:                  /* Others are an error */
              return(MB_EXC_ILL_ADDR);
              break;
          } /* End switch on gbreg for group 000 block 80 */
          break;

        case 0xA0:                    /* 0A0-0BF -- EEPROM/NV partition info */
          switch (gbreg)
          {
            case 0x0:                 /* 0A0 -- EEPROM base address, hi */
              hval = 0; /* High-order 16 bits */
              break;

            case 0x1:                 /* 0A1 -- EEPROM base address, lo */
              ptr = (char *)eeHomePtr(); /* Base address of Home block */
              hval = (unsigned int)ptr; /* Low-order 16 bits */
              break;

            case 0x2:                 /* 0A2 -- Home block size */
              sts = (MODBSTS)eeMapPartition (EEP_HOME, &hval, &hxxx);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0x3:                 /* 0A3 -- Home block offset */
              sts = (MODBSTS)eeMapPartition (EEP_HOME, &hxxx, &hval);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0x4:                 /* 0A4 -- Boot block size */
              sts = (MODBSTS)eeMapPartition (EEP_BOOT, &hval, &hxxx);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0x5:                 /* 0A5 -- Boot block offset */
              sts = (MODBSTS)eeMapPartition (EEP_BOOT, &hxxx, &hval);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0x6:                 /* 0A6 -- Crash block size */
              sts = (MODBSTS)eeMapPartition (EEP_CRASH, &hval, &hxxx);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0x7:                 /* 0A7 -- Crash block offset */
              sts = (MODBSTS)eeMapPartition (EEP_CRASH, &hxxx, &hval);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0x8:                 /* 0A8 -- System block size */
              sts = (MODBSTS)eeMapPartition (EEP_SYSNV, &hval, &hxxx);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0x9:                 /* 0A9 -- System block offset */
              sts = (MODBSTS)eeMapPartition (EEP_SYSNV, &hxxx, &hval);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0xA:                 /* 0AA -- Event log block size */
              sts = (MODBSTS)eeMapPartition (EEP_LOG, &hval, &hxxx);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0xB:                 /* 0AB -- Event log block offset */
              sts = (MODBSTS)eeMapPartition (EEP_LOG, &hxxx, &hval);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0xC:                 /* 0AC -- Bypass key block size */
              sts = (MODBSTS)eeMapPartition (EEP_KEY, &hval, &hxxx);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0xD:                 /* 0AD -- Bypass key block offset */
              sts = (MODBSTS)eeMapPartition (EEP_KEY, &hxxx, &hval);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0xE:                 /* 0AE -- Truck ID block size */
              sts = (MODBSTS)eeMapPartition (EEP_TIM, &hval, &hxxx);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            case 0xF:                 /* 0AF -- Truck ID block offset */
              sts = (MODBSTS)eeMapPartition (EEP_TIM, &hxxx, &hval);
              if (sts)
                  return (MB_EXC_MEM_PAR_ERR); /* Generic EEPROM error */
              break;

            default:                  /* getting here is a code error */
              hval = MB_EXC_FAULT;
              break;
          } /* End switch on gbreg for group 000 block A0 */
          break;

        case 0xB0:                    /* 0B0-0BF -- More EEPROM/NV stuff */
          switch (gbreg)
          {
            case 0 :
            default:                  /* Others are an error */
              hval = MB_EXC_ILL_ADDR;
              break;
          } /* End switch on gbreg for group 000 block B0 */
          break;

        case 0xE0:                    /* 0E0-0EF -- Debug event/error/etc. */
          switch (gbreg)
          {
            case 0x0:                 /* 0E0 -- Soft COMM_ID ground errs */
              hval = 0;
              break;

            case 0x1:                 /* 0E1 -- Hard COMM_ID ground errs */
              hval = 0;
              break;

            default:                  /* Others are an error */
              hval = MB_EXC_ILL_ADDR;
              break;
          } /* End switch on gbreg for group 000 block E0 */
          break;

        case 0xF0:                    /* 0F0-0FF -- More error/event/etc. */
          switch (gbreg)
          {
            case 0 :
            default:                  /* all are errors */
              hval = MB_EXC_ILL_ADDR;
              break;
          } /* End switch on gbreg for group 000 block F0 */
          break;

        default:
          return (MB_EXC_ILL_ADDR);
      }                                 /* End switch on Register Block */
    }
    break;                              /* End 0x000 Register Group */

  case 0x01:                        /* 0x100 Main-line dynamic state registers */
  {
  /* ================================ */
  /* === Register block 100 - 1FF === */
  /* ================================ */

  /* Dispatch based on block within "Main-line" dynamic/state group */

    switch (block)
    {
      case 0x00:                    /* 100-10F -- Dynamic unit state/info */
        switch (gbreg)
            {
          case 0x0:                 /* 100 -- Date/Time (Unix format) */
            hval = (unsigned)(present_time >> 16); /* High-order 16 bits */
            break;

          case 0x1:                 /* 101 -- Date/Time (Unix format) */
            hval = (unsigned)(present_time & 0xFFFF); /* Low-order 16 bits */
            break;

          case 0x2:                 /* 102 -- Current Event Elapsed Time */
            hval = (unsigned)(freetimer >> 16);  /* High-order 16 bits */
            break;

          case 0x3:                 /* 103 -- Current Event Elapsed Time */
            hval = (unsigned)(freetimer & 0xFFFF);  /* Low-order 16 bits */
            break;

          case 0x4:                 /* 104 -- Status "A" flags */
            hval = StatusA;         /* AKA Input Status 15 - 0 */
            if (hval == 0)          /* Should never be "Unknown" */
                hval = STSA_IDLE;   /* Nifty place to put a breakpoint...*/
            break;

          case 0x5:                 /* 105 -- Status "B" flags */
            hval = StatusB;         /* AKA Input Status 31 - 16 */
            break;

          case 0x6:                 /* 106 -- Status "O" flags */
            hval = StatusO;         /* AKA Output Status 15 - 0 */
            break;

          case 0x7:                 /* 107 -- Status "P" flags */
            hval = StatusP;         /* AKA Output Status 31 - 16 */
            break;

          case 0x8:                 /* 108 -- Main Intellitrol state */
            hval = (unsigned)main_state;      /* "IDLE", etc. */
            break;

          case 0x9:                 /* 109 -- Truck [Type] state */
            hval = (unsigned)truck_state;     /* "5-Wire", etc. */
            break;

          case 0xA:                 /* 10A -- Truck Serial Number  1/3 */
                                    /* ***ATTENTION*** */
            hval = (unsigned)(((unsigned)truck_SN[0] << 8)
                    | (unsigned)truck_SN[1]);
            break;

          case 0xB:                 /* 10B -- Truck Serial Number  2/3 */
           hval = (unsigned)(((unsigned)truck_SN[2] << 8)
                   | (unsigned)truck_SN[3]);
            break;

          case 0xC:                 /* 10C -- Truck Serial Number  3/3 */
            hval = (unsigned)(((unsigned)truck_SN[4] << 8)
                    | (unsigned)truck_SN[5]);
            break;

          /* Offset the "Probes" state so that VIPER/TAS always see
             "Probe 1" (AKA tank/compartment 1) as the first reported
             Probe State value...for non-5-wire-optic USA 6-compartment
             truck shift probes_state[] "up by two" (i.e., ignore
             probes "1" and "2" AKA channels "0" and "1") */

          case 0xD:                 /* 10D -- Probes 1 & 2 State */
            if ((ConfigA & CFGA_8COMPARTMENT) /* US/6-compartment truck? */
                || (truck_state == OPTIC_FIVE)) /* 5wire-Optic overrides */
                hval = (unsigned)(((unsigned)probes_state[0] << 8)
                        | (unsigned)probes_state[1]);
            else
                hval = (unsigned)(((unsigned)probes_state[2] << 8)
                        | (unsigned)probes_state[3]);
            break;

          case 0xE:                 /* 10E -- Probes 3 & 4 State */
            if ((ConfigA & CFGA_8COMPARTMENT) /* US/6-compartment truck? */
                || (truck_state == OPTIC_FIVE)) /* 5wire-Optic overrides */
                hval = (unsigned)(((unsigned)probes_state[2] << 8)
                        | (unsigned)probes_state[3]);
            else
                hval = (unsigned)(((unsigned)probes_state[4] << 8)
                        | (unsigned)probes_state[5]);
            break;

          case 0xF:                 /* 10F -- Probes 5 & 6 State */
            if ((ConfigA & CFGA_8COMPARTMENT) /* US/6-compartment truck? */
                || (truck_state == OPTIC_FIVE)) /* 5wire-Optic overrides */
                hval = (unsigned)(((unsigned)probes_state[4] << 8)
                        | (unsigned)probes_state[5]);
            else
                hval = (unsigned)(((unsigned)probes_state[6] << 8)
                        | (unsigned)probes_state[7]);
            break;

          default:                  /* Can't get here */
            return (MB_EXC_FAULT);
            } /* End switch on gbreg for group 100 block 00 */

        break;

      case 0x10:                    /* 110-11F -- More unit state info */
        switch (gbreg)
            {
          case 0x0:                 /* 110 -- Probes 7 & 8 State */
            if ((ConfigA & CFGA_8COMPARTMENT) /* US/6-compartment truck? */
                || (truck_state == OPTIC_FIVE)) /* 5wire-Optic overrides */
                hval = (unsigned)(((unsigned)probes_state[6] << 8)
                        | (unsigned)probes_state[7]);
            else
                hval = (unsigned)(/*lint -e{835, 641} */(P_UNKNOWN << 8)
                        | P_UNKNOWN);
            break;

          case 0x1:                 /* 111 -- Probes 9 & 10 State */
            if ((ConfigA & CFGA_8COMPARTMENT) /* US/6-compartment truck? */
                || (truck_state == OPTIC_FIVE)) /* 5wire-Optic overrides */
                hval = (unsigned)(((unsigned)probes_state[8] << 8)
                        | (unsigned)probes_state[9]);
            else
                hval = (unsigned)(/*lint -e{835, 641} */(P_UNKNOWN << 8)
                        | P_UNKNOWN);
            break;

          case 0x2:                 /* 112 -- Probes 11 & 12 State */
            if ((ConfigA & CFGA_8COMPARTMENT) /* US/6-compartment truck? */
                || (truck_state == OPTIC_FIVE)) /* 5wire-Optic overrides */
                hval = (unsigned)(((unsigned)probes_state[10] << 8)
                        | (unsigned)probes_state[11]);
            else
                hval = (unsigned)(/*lint -e{835, 641} */(P_UNKNOWN << 8
                        ) | P_UNKNOWN);
            break;

          case 0x3:                 /* 113 -- Probes 13 & 14 State */
            if ((ConfigA & CFGA_8COMPARTMENT) /* US/6-compartment truck? */
                || (truck_state == OPTIC_FIVE)) /* 5wire-Optic overrides */
                hval = (unsigned)(((unsigned)probes_state[12] << 8)
                        | (unsigned)probes_state[13]);
            else
                  hval = (unsigned)(/*lint -e{835, 641} */(P_UNKNOWN << 8)
                          | P_UNKNOWN);
            break;

          case 0x4:                 /* 114 -- Probes 15 & 16 State */
            if ((ConfigA & CFGA_8COMPARTMENT) /* US/6-compartment truck? */
                || (truck_state == OPTIC_FIVE)) /* 5wire-Optic overrides */
                hval = (unsigned)(((unsigned)probes_state[14] << 8)
                        | (unsigned)probes_state[15]);
            else
                hval = (unsigned)(/*lint -e{835, 641} */(P_UNKNOWN << 8)
                        | P_UNKNOWN);
            break;

          case 0x5:                 /* 115 -- Bypass State */
            hval = (unsigned)(((unsigned)(unsigned char)bystatus << 8)
                    | (unsigned)(unsigned char)bylevel);
            break;

          case 0x6:                 /* 116 -- Bypass Serial Number */
            hval = (unsigned)(((unsigned)bypass_SN[0] << 8)
                    | (unsigned)bypass_SN[1]); /* 1/3 */
            break;

          case 0x7:                 /* 117 -- Bypass Serial Number */
            hval = (unsigned)(((unsigned)bypass_SN[2] << 8)
                    | (unsigned)bypass_SN[3]); /* 2/3 */
            break;

          case 0x8:                 /* 118 -- Bypass Serial Number */
            hval = (unsigned)(((unsigned)bypass_SN[4] << 8)
                    | (unsigned)bypass_SN[5]); /* 3/3 */
            break;

          case 0x9:                 /* 119 -- Bypass Time (elapsed) */
            if (bypass_time)
                hval = (unsigned)((read_time()-bypass_time)/1000);
            else
                hval = 0;
            break;

          case 0xA:                 /* 11A -- Non-permissive reasons */
            hval = mbrNonPermitReg (); /* Go figure them out... */
            break;

          case 0xB:                 /* 11B -- Latest Log pointer (nee 007) */
            if (evMax)              /* If Event Log accessible */
                if (evIndex)        /* Return "first free" */
                    hval = evIndex - 1; /* Backed up one entry ... */
                else                /* (time to backwards- */
                    hval = evMax;   /*  wrap the pointer) */
            else
                hval = 0xFFFF;      /* Event Log inaccessible */
            break;

          case 0xC:
            hval = (unsigned)SysParm.ADCOmaxNV;   /* 11C - Optic 2 wire Threshold */
            break;

          case 0xD:
            hval = (unsigned)SysParm.ADCTHstNV;   /* 11D - Hysteresis */
            break;

          case 0xE:
            hval = (unsigned)SysParm.ADCTmaxNV;   /* 11E - Thermistor Threshold */
            break;

          default:                  /* Others */
            return(MB_EXC_ILL_ADDR);
            break;
            } /* End switch on gbreg for group 100 block 10 */

        break;
      case 0x20:                    /* 120-12F -- More unit state info */
        switch (gbreg)
        {
          case 0x0:                 /* 120 - Number of truck compartments */
            if ((number_of_Compartments < 1)
                    || (number_of_Compartments > 16))
            {
              hval = 0xFFFF;  /* Not configured */
            }
            else
            {
              hval = number_of_Compartments;
            }
          break;

          case 0x1:                 /* 121 - Stop logging dome out events */
              hval = (unsigned)disable_domeout_logging;  /*  */
          break;

          case 0x2:                 /* 122 -  */
              hval = (unsigned int)TIM_info_logged;  /*  */
          break;
          
          case 0x3:                 /* 123 -  */
              hval = (unsigned)TIM_size;  /*  */
          break;
          
          case 0x4:                 /* 124 -  */
             hval = (unsigned)S_TIM_code;  /*  */
          break;
          
          case 0x5:                 /* 125 -  */
              hval = (unsigned int)log_data_state;  /*  */
          break;
          
          case 0x6:                 /* 126 -  */
              hval = (unsigned int)compare_volts;  /* Channel 5 diag line voltage from calc_tank() */
          break;
          
          default:                  /* Others */
            return(MB_EXC_ILL_ADDR);
            break;
        }
       break;

      default:
      return (MB_EXC_ILL_ADDR);
    } /* End switch on Register Block for Group 0x100 */
  }
    break;                          /* End 0x100 Register Group */

    /* ================================ */
    /* === Register block 200 - 2FF === */
    /* ================================ */

    case 0x02:                        /* 200-2FF On-Board Intellicheck/etc. */
      return (MB_EXC_ILL_ADDR);

    /* ================================ */
    /* === Register block 300 - ??? === */
    /* ================================ */

// SME add 300 and 400 for super TIM      
// Probably less space and typing to do an if else for each chunk of contigous TIM memory
    case 0x03:
    case 0x04:
        if(( regno >= 0x300) && (regno <= 0x331) )
        {   // 
            read_addr = ((regno - 0x300) * 2 ) + 0x200;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);
        }
        else if( (regno >= 0x332) && (regno <= 0x341))
        {
            read_addr = ((regno - 0x332) * 2 ) + 0x26E;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);            
        } 
        else if( (regno >= 0x342) && (regno <= 0x34D))
        {
            read_addr = ((regno - 0x342) * 2 ) + 0x300;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);            
        } 
        else if( (regno >= 0x34E) && (regno <= 0x359))
        {
            read_addr = ((regno - 0x34E) * 2 ) + 0x322;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);            
        } 
        else if( (regno >= 0x35A) && (regno <= 0x365))
        {
            read_addr = ((regno - 0x35A) * 2 ) + 0x344;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);            
        } 
        else if( (regno >= 0x366) && (regno <= 0x371))
        {
            read_addr = ((regno - 0x366) * 2 ) + 0x366;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);            
        } 
        else if( (regno >= 0x372) && (regno <= 0x37D))
        {
            read_addr = ((regno - 0x372) * 2 ) + 0x388;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);            
        } 
        else if( (regno >= 0x37E) && (regno <= 0x37F))
        {
            read_addr = ((regno - 0x37E) * 2 ) + 0x400;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);            
        } 
        else if( regno == 0x380 )
        {
            read_addr = 0x40B;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 1)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)(read_buffer[0]);
        } 
        else if( (regno >= 0x381) && (regno <= 0x383))
        {
            read_addr = ((regno - 0x381) * 2 ) + 0x40C;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);            
        } 
        else if( regno == 0x384 )
        {
            read_addr = 0x478;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 1)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)(read_buffer[0]);
        } 
        else if( (regno >= 0x385) && (regno <= 0x38E))
        {
            read_addr = ((regno - 0x385) * 2 ) + 0x4A2;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);            
        } 
        else if( regno == 0x38F )
        {
            read_addr = 0x4CA;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 1)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)(read_buffer[0]);
        } 
        else if( regno == 0x390 )
        {
            read_addr = 0x4D0;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 1)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)(read_buffer[0]);
        } 
        else if( (regno >= 0x391) && (regno <= 0x3B0))
        {
            mod_res = regno % 2;
            if( mod_res == 1)
            {
                read_addr = (((regno - 0x391) / 2 ) * 3) + 0x4D1;
                if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 1)) != MB_OK)
                {
                    return (sts);
                }   
                hval = 0;
                hval = (unsigned int)read_buffer[0];            
            }
            else
            {
                read_addr = (((regno - 0x392) / 2 ) * 3) + 0x4D2;
                if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
                {
                    return (sts);
                }   
                hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);            
            }
        } 
        else if( (regno >= 0x3B1) && (regno <= 0x400))
        {
            offset = regno - 0x3B1;
            div_res = offset / 5;
            mod_res = offset % 5;
            base_addr = 0x5FF - (div_res * 2);
            if( mod_res > 1 )
            {
                base_addr -= 1;
            }
            read_addr = (offset * 2 ) + base_addr;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);
            if((mod_res == 0) || (mod_res == 2))
            {
                hval &= 0x00FF;
            }
        } 
        else if( (regno >= 0x401) && (regno <= 0x41E))
        {
            read_addr = ((regno - 0x401) * 2 ) + 0x687;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);            
        } 
        else if( regno == 0x41F )
        {
            read_addr = 0x6C3;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 1)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)(read_buffer[0]);
        } 
        else if( regno == 0x420 )
        {
            read_addr = 0x477;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 1)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)(read_buffer[0]);
        } 
        else if( regno == 0x421 )
        {
            read_addr = 0x501;
            if ((sts = (MODBSTS)tim_block_read(read_buffer, read_addr, 2)) != MB_OK)
            {
                return (sts);
            }   
            hval = (unsigned int)((read_buffer[0] << 8 ) | read_buffer[1]);            
        } 
        else
        {
            return (MB_EXC_ILL_ADDR);
        }
        break;
        
    default:                          /* Rest undefined, illegal */
      return (MB_EXC_ILL_ADDR);
  } /* End switch on Register Group */

  /* If any problems... */

  *value = hval;                      /* Return Register Contents */

  return (MB_OK);

} /* End mbrRdReg() */

/*************************************************************************
* mbrWrReg -- Write 16-bit "ModBus Register"
*
* Call is:
*
*       mbrWrReg (regno, value)
*
* Where:
*
*       <regno> is the Register Number to be read;
*
*       <value> is the pointer to where the desired 16-bit value is cur-
*       rently residing (i.e., points to new value to be written to the
*       specified "Register").
*
* mbrWrReg() is responsible for figuring out what is meant by a ModBus
* 16-bit "Holding Register", and assigning ("writing", "setting", etc.)
* a new value to that register, if appropriate. The "register value" has
* no semantic meaning to mbrWrReg(), which merely "stores" the data for
* other routines to utilize later. If more than straight storage is needed
* (e.g., an immediate action is required), then the "Register" must be
* implemented as a function call.
*
* Return value is MB_OK (0) normally, or an exception value (~0) if there
* are any problems writing the register:
*
*   MB_EXC_ILL_ADDR if attempting to write an undefined register (note that
*                   "Reserved" registers are "defined" and always return 0)
*   MB_EXC_ILL_DATA if attempting to write to a read only register, or possibly
*                   the data itself is out of range (return value from
*                   a register function, for example)
*   MB_EXC_FAULT    if some sort of processing error arises
*   <anything>      "Exception" return from a register-function; the error
*                   status is merely propagated back to the caller.
*
*************************************************************************/

MODBSTS mbrWrReg
    (
    unsigned int regno,         /* 16-bit ModBus "Register" number */
    unsigned int *value         /* 16-bit "Register" data */
    )
{
    static unsigned wtmp;           /* Temp "holding" buffer */
    unsigned char tmp_byte;
    unsigned char tmp_buf[5];
    unsigned int addr;
    MODBSTS sts;                /* Status holding */

    /* Unlike "Read Registers" above, the "Write" register set is very
       sparsely populated... just do a master switch/case pass */

    sts = MB_OK;                        /* Assume the best */

    switch (regno)                      /* Dispatch on register-to-write */
        {
      case 0x008:                       /* 008 -- Wait-for-TAS delay */
        if (*value > 60)                /* Legal range is 0 to 60 seconds */
            *value = 60;                /* Trim to max we support */
        SysParm.TASWait = *value;       /* Set TAS delay cycle */
        modNVflag++;                    /* Request EEPROM update */
        break;

      case 0x009:                       /* 009 -- Bypass timeout */
/*RDH   Allow max possible values (18 hours) as per FrankSki 2-Nov-95 */
        SysParm.BypassTimeOut = *value; /* Set Bypass timeout limit */
        modNVflag++;                    /* Request EEPROM update */
        break;

      case 0x00A:                       /* 00A -- Terminal number */
        SysParm.Terminal = *value;      /* Set new terminal number */
        modNVflag++;                    /* Request EEPROM update */
        break;

      case 0x00B:                        /* 00B -- ModBus Min Response Time */
        if (*value > 1001)               /* Within reason? */
            *value = 1000;               /* No, trim it */
        SysParm.ModBusRespWait = *value; /* Set rx->tx turnaround delay */
        modNVflag++;                     /* Request EEPROM update */
        break;

      case 0x00E:                       /* 00E -- VIP "Mode" control */
        sts =  mbrVIPModeWr (*value);   /* Handle Mode set command   */
        break;


      case 0x70:                        /* 0070 - System Software Configuration */
                                        /* Enable/Disable Shorts Test */
        if ((*value == 0) || (*value == 1))   /* Within reason? */
        {
            SysParm.Ena_INTL_ShortNV = (unsigned char) *value;
            modNVflag++;                 /* Request EEPROM update */
        }
        break;

      case 0x71:                        /* 0071 - System Software Configuration */
        if (*value <= 0xFF)             /* Within reason? */
        {
            SysParm.Ena_Debug_Func_1 = (unsigned char) *value;
            modNVflag++;                 /* Request EEPROM update */
        }
        break;
      case 0x72:                        /* 0072 - System Software Configuration */
        if (*value <= 0xFF)             /* Within reason? */
        {
            SysParm.Ena_Debug_Func_2 = (unsigned char) *value;
            modNVflag++;                 /* Request EEPROM update */
        }
        break;
      case 0x73:                        /* 0073 - System Software Configuration */
        if (*value <= 0xFF)             /* Within reason? */
        {
            SysParm.Ena_Debug_Func_3 = (unsigned char) *value;
            modNVflag++;                 /* Request EEPROM update */
        }
        break;
      case 0x74:                        /* 0074 - System Software Configuration */
        if (*value <= 0xFF)             /* Within reason? */
        {
            SysParm.Ena_Debug_Func_4 = (unsigned char) *value;
            modNVflag++;                 /* Request EEPROM update */
        }
        break;
      case 0x75:                        /* 0075 - System Software Configuration */
        if (*value <= 0xFF)             /* Within reason? */
        {
            SysParm.Ena_Debug_Fail_1 = (unsigned char) *value;
            modNVflag++;                 /* Request EEPROM update */
        }
        break;
      case 0x76:                        /* 0076 - System Software Configuration */
        if (*value <= 0xFF)             /* Within reason? */
        {
            SysParm.Ena_Debug_Fail_2 = (unsigned char) *value;
            modNVflag++;                 /* Request EEPROM update */
        }
        break;
      case 0x77:                        /* 0077 - System Software Configuration */
        if (*value <= 0xFF)             /* Within reason? */
        {
            SysParm.Ena_Debug_Fail_3 = (unsigned char) *value;
            modNVflag++;                 /* Request EEPROM update */
        }
        break;
      case 0x78:                        /* 0078 - System Software Configuration */
        if (*value <= 0xFF)             /* Within reason? */
        {
            SysParm.Ena_Debug_Fail_4 = (unsigned char) *value;
            modNVflag++;                 /* Request EEPROM update */
        }
        break;
      case 0x79:                        /* 0079 - System Software Configuration */
        SysParm.EnaSftFeatures = (unsigned char) *value;
        modNVflag++;                    /* Request EEPROM update */
        break;

      case 0x7A:                         /* 7A -- Software Feature Enable Code */
                                         /* Enable/Disable Fault Delay */
        if (*value == 0)                 /* Disable requested */
        {
           SysParm.EnaSftFeatures =
                   (unsigned char) (SysParm.EnaSftFeatures & ~ENA_GRND_DELAY);
        }
        if (*value > 0)                  /* Enable requested */
        {
           SysParm.EnaSftFeatures =
                   (unsigned char) (SysParm.EnaSftFeatures | ENA_GRND_DELAY);
        }
        modNVflag++;                    /* Request EEPROM update */
        break;

      case 0x07B:						      /* WPW: new register for read TIM */
        if (*value == 0)
        {
          SysParm.Ena_TIM_Read = (unsigned char)*value;  /* enable or disable */
          (void)nvSysParmUpdate();

          if (((SysParm.EnaPassword & ENA_VIP) == ENA_VIP)
                  && ((StatusA & STSA_TRK_PRESENT) == 0)) /* Allowed to use VIP? */
          {
            ledstate[VIP_IDLE] = DARK;
            ledstate[VIP_AUTH] = DARK;
            ledstate[VIP_UNAUTH] = DARK;
            StatusO = 0;
          }
        }
        else
        {
          /**************************** 7/22/2011 2:35PM *************************
           * The Intellitrol must be purchased with VIP. To use this command
           ***********************************************************************/
          if ((SysParm.EnaPassword & ENA_VIP) == 0) /* Must have purchased VIP */
          {
            return (MB_EXC_ILL_ADDR);                       /* VIP not enabled */
          }
          if (*value == 1)
          {
            SysParm.Ena_TIM_Read = (unsigned char)*value;  /* enable or disable */
            (void)nvSysParmUpdate();
            ledstate[VIP_IDLE] = LITE;
            ledstate[VIP_AUTH] = DARK;
            ledstate[VIP_UNAUTH] = DARK;
            StatusO |= STSO_STANDBY;
          } else
          {
            return (MB_EXC_ILL_DATA); /* No, illegal "bit" value */
          }
        }
       break;

     case 0x07C:                  /* Set resistive ground reference */
       SysParm.EU_GND_REF = *value;
       set_ground_reference();
       (void)nvSysParmUpdate();       /* Force EEPROM update */
       break;

     case 0x07E:                  /* Enable/disable ground indication */
       if ((*value == 0) || (*value == 1))
       {
          SysParm.Ena_GND_Display = (unsigned char)*value;  /* enable or disable */
          (void)nvSysParmUpdate();
       }
       break;

     case 0x07F:                  /* 5-Wire Compartment count display time */
       if (((*value >0) && (*value <= 30)) || (*value == 0xFF))
       {
          SysParm.Five_Wire_Display = (unsigned char)*value;  /* Time in seconds */
          (void)nvSysParmUpdate();
       }
       else
       {
          return (MB_EXC_ILL_DATA); /* No, illegal "bit" value */
       }
       break;

      case 0x80:                /* 80 -- Active Deadman Enabled */
        if (*value != 0)
        {
          SysParm.DM_Active = 0xFF;
        }else
        {
          SysParm.DM_Active = 0;
        }
        (void)nvSysParmUpdate();
      break;

      case 0x81:                /* 81 -- Active Deadman Max open time */
        if ((*value >= 1) && (*value <= 30))
        {
          SysParm.DM_Max_Open = (*value << 2);
          (void)nvSysParmUpdate();
        }
        else
        {
          return (MB_EXC_ILL_DATA); /* reject bad values */
        }
      break;

      case 0x82:                /* 82 -- Active Deadman Max close time */
       if ((*value >= 10) && (*value <= 600))
        {
          SysParm.DM_Max_Close = (*value << 2);
          (void)nvSysParmUpdate();
        }
        else
        {
          return (MB_EXC_ILL_DATA); /* reject bad values */
        }
      break;

      case 0x83:                /* 83 -- Active Deadman Warning time */
       if ((*value >= 10) && (*value <= 60))
        {
          SysParm.DM_Warn_Start = (*value << 2);
          (void)nvSysParmUpdate();
        }
        else
        {
          return (MB_EXC_ILL_DATA); /* reject bad values */
        }
      break;
              
      case 0x84:                         /* 84 -- Software Feature Enable Code */
                                         /* Enable/Disable Unload Terminal  */
        if (*value == 0)                 /* Disable requested */
        {
           SysParm.EnaSftFeatures =
                   (unsigned char) (SysParm.EnaSftFeatures & ~ENA_UNLOAD_TERM);
        }
        if (*value > 0)                  /* Enable requested */
        {
           SysParm.EnaSftFeatures =
                   (unsigned char) (SysParm.EnaSftFeatures | ENA_UNLOAD_TERM);
        }
//        (void)nvSysParmUpdate();       /* Force EEPROM update */
        modNVflag++;                    /* Request EEPROM update */
        break;

      case 0x85:                /* 85 -- Unload max time */
        SysParm.Unload_Max_Time_min = *value;
        (void)nvSysParmUpdate();
      break;
              
      case 0x86:                /* 86 -- Cert DS enable mask */
       if ((*value >= 0) && (*value < 32))
        {
          SysParm.Cert_Expiration_Mask = (unsigned char)(*value);
          (void)nvSysParmUpdate();
        }
        else
        {
          return (MB_EXC_ILL_DATA); /* reject bad values */
        }
      break;
              
      case 0x87:                         /* 84 -- Software Feature Enable Code */
                                         /* Enable/Disable Unload Terminal  */
        if (*value == 0)                 /* Disable requested */
        {
           SysParm.EnaSftFeatures =
                   (unsigned char) (SysParm.EnaSftFeatures & ~ENA_CPT_COUNT);
        }
        if (*value > 0)                  /* Enable requested */
        {
           SysParm.EnaSftFeatures =
                   (unsigned char) (SysParm.EnaSftFeatures | ENA_CPT_COUNT);
        }
//        (void)nvSysParmUpdate();       /* Force EEPROM update */
        modNVflag++;                    /* Request EEPROM update */
        break;

      case 0x88:                /* 88 -- fuel type check mask */
       if ((*value >= 0) && (*value < 256))
        {
          SysParm.fuel_type_check_mask = (unsigned char)(*value);
          (void)nvSysParmUpdate();
        }
        else
        {
          return (MB_EXC_ILL_DATA); /* reject bad values */
        }
      break;
              
      case 0x89:                         /* 89 -- Software Feature Enable Code */
                                         /* Enable/Disable auto fuel type write  */
        if (*value == 0)                 /* Disable requested */
        {
           SysParm.EnaSftFeatures2 =
                   (unsigned char) (SysParm.EnaSftFeatures2 & ~ENA_AUTO_FUEL_TYPE_WRITE);
        }
        if (*value > 0)                  /* Enable requested */
        {
           SysParm.EnaSftFeatures2 =
                   (unsigned char) (SysParm.EnaSftFeatures2 | ENA_AUTO_FUEL_TYPE_WRITE);
        }
//        (void)nvSysParmUpdate();       /* Force EEPROM update */
        modNVflag++;                    /* Request EEPROM update */
        break;

      case 0x8A:                /* 88 -- default fuel type msb and middle byte */
        wtmp = *value;                  /* Just hold on to high order half... */
        tmp_byte = (unsigned char)(wtmp >> 8);
        SysParm.default_fuel_type[0] = tmp_byte;
        tmp_byte = (unsigned char)wtmp;
        SysParm.default_fuel_type[1] = tmp_byte;
        (void)nvSysParmUpdate();
      break;
              
      case 0x8B:                /* 88 -- default fuel type lsb */
        wtmp = *value;                  /* Just hold on to high order half... */
        tmp_byte = (unsigned char)(wtmp >> 8);
        SysParm.default_fuel_type[2] = tmp_byte;
        (void)nvSysParmUpdate();
      break;
          
      case 0x100:                       /* 100 -- High-order system Time-Of-Day */
        /* Wait for second half to do the actual write as an atomic operation.
           Just "stash" the high half for now; assume low half follows 
           immediately after. */
        wtmp = *value;                  /* Just hold on to high order half... */
        break;

      case 0x101:                       /* 101 -- Low-order system Time-Of-Day */
        /* Low-order TOD must immediately follow high-order half; this triggers
           the full 32-bit "write" operation to the Dallas clock chip. */

        present_time = wtmp;                     /* Set new TOD, Cumbersome, */
        present_time *= 0x10000;                 /*  but at least */
        present_time += *value;                  /*   the compiler likes it */

        sts = (MODBSTS)Write_Clock ();           /* Update Dallas TOD clock chip */
        if (sts != CLOCK_OK)            /* Any problems? */
            {                           /* Yes... */
            sts = MB_EXC_MEM_PAR_ERR;   /* Generic "register error" code */
            break;                      /* And that's that. */
            }

        /* Should we do a Read_Clock() here??? */

        UNIX_to_Greg ();                /* Update to new year/month/etc. */
        break;

      case 0x11C:                       /* 11C -- ADCOmaxNV Optical 2 Wire Probe */
                                        /* Positive Threshold WRITE ONLY*/
        if ((*value > 5001) || (*value < 3999))    /* Within reason? */
            *value = 4375;              /* No, trim it */
        SysParm.ADCOmaxNV = *value;
        modNVflag++;                    /* Request EEPROM update */
        break;

      case 0x11D:                       /* 11D -- ADCTHstNV Hysteresis */
        if ((*value > 801) || (*value < 599))   /* Within reason? */
            *value = 700;               /* No, trim it */
        SysParm.ADCTHstNV = *value;
        modNVflag++;                    /* Request EEPROM update */
        break;

      case 0x11E:                       /* 11E -- ADCTmaxNV Thermistor Probe */
        if ((*value > 4001) || (*value < 2800))   /* Within reason? */
            *value = 3675;              /* No, trim it */
        SysParm.ADCTmaxNV = *value;
        modNVflag++;                    /* Request EEPROM update */
        break;

      case 0x121:                       /* 121 - Stop logging dome out events KLL */
        disable_domeout_logging = (int)*value;  /*  */
        break;


        
      case 0x3B1:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x600,1);
        break;
        
      case 0x3B2:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x601,2);
        break;
        
      case 0x3B3:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x603,1);
        break;
                
      case 0x3B4:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x604,2);
        break;
        
      case 0x3B5:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x606,2);
        break;
 
      case 0x3B6:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x608,1);
        break;
        
      case 0x3B7:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x609,2);
        break;
        
      case 0x3B8:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x60B,1);
        break;
                
      case 0x3B9:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x60C,2);
        break;
        
      case 0x3BA:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x60E,2);
        break;
 
      case 0x3BB:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x610,1);
        break;
        
      case 0x3BC:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x611,2);
        break;
        
      case 0x3BD:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x613,1);
        break;
                
      case 0x3BE:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x614,2);
        break;
        
      case 0x3BF:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x616,2);
        break;
 
      case 0x3C0:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x618,1);
        break;
        
      case 0x3C1:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x619,2);
        break;
        
      case 0x3C2:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x61B,1);
        break;
                
      case 0x3C3:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x61C,2);
        break;
        
      case 0x3C4:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x61E,2);
        break;
 
      case 0x3C5:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x620,1);
        break;
        
      case 0x3C6:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x621,2);
        break;
        
      case 0x3C7:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x623,1);
        break;
                
      case 0x3C8:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x624,2);
        break;
        
      case 0x3C9:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x626,2);
        break;
 
      case 0x3CA:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x628,1);
        break;
        
      case 0x3CB:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x629,2);
        break;
        
      case 0x3CC:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x62B,1);
        break;
                
      case 0x3CD:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x62C,2);
        break;
        
      case 0x3CE:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x62E,2);
        break;
 
      case 0x3CF:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x630,1);
        break;
        
      case 0x3D0:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x631,2);
        break;
        
      case 0x3D1:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x633,1);
        break;
                
      case 0x3D2:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x634,2);
        break;
        
      case 0x3D3:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x636,2);
        break;
 
      case 0x3D4:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x638,1);
        break;
        
      case 0x3D5:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x639,2);
        break;
        
      case 0x3D6:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x63B,1);
        break;
                
      case 0x3D7:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x63C,2);
        break;
        
      case 0x3D8:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x63E,2);
        break;
 
        
      case 0x3D9:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x640,1);
        break;
        
      case 0x3DA:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x641,2);
        break;
        
      case 0x3DB:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x643,1);
        break;
                
      case 0x3DC:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x644,2);
        break;
        
      case 0x3DD:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x646,2);
        break;
 
      case 0x3DE:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x648,1);
        break;
        
      case 0x3DF:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x649,2);
        break;
        
      case 0x3E0:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x64B,1);
        break;
                
      case 0x3E1:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x64C,2);
        break;
        
      case 0x3E2:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x64E,2);
        break;
 
      case 0x3E3:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x650,1);
        break;
        
      case 0x3E4:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x651,2);
        break;
        
      case 0x3E5:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x653,1);
        break;
                
      case 0x3E6:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x654,2);
        break;
        
      case 0x3E7:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x656,2);
        break;
 
      case 0x3E8:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x658,1);
        break;
        
      case 0x3E9:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x659,2);
        break;
        
      case 0x3EA:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x65B,1);
        break;
                
      case 0x3EB:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x65C,2);
        break;
        
      case 0x3EC:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x65E,2);
        break;
 
      case 0x3ED:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x660,1);
        break;
        
      case 0x3EE:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x661,2);
        break;
        
      case 0x3EF:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x663,1);
        break;
                
      case 0x3F0:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x664,2);
        break;
        
      case 0x3F1:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x666,2);
        break;
 
      case 0x3F2:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x668,1);
        break;
        
      case 0x3F3:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x669,2);
        break;
        
      case 0x3F4:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x66B,1);
        break;
                
      case 0x3F5:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x66C,2);
        break;
        
      case 0x3F6:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x66E,2);
        break;
 
      case 0x3F7:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x670,1);
        break;
        
      case 0x3F8:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x671,2);
        break;
        
      case 0x3F9:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x673,1);
        break;
                
      case 0x3FA:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x674,2);
        break;
        
      case 0x3FB:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x676,2);
        break;
 
      case 0x3FC:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x678,1);
        break;
        
      case 0x3FD:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x679,2);
        break;
        
      case 0x3FE:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x67B,1);
        break;
                
      case 0x3FF:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x67C,2);
        break;
        
      case 0x400:                       
        wtmp = *value;  /*  */
        tmp_buf[0] = wtmp >> 8;
        tmp_buf[1] = wtmp;
        sts = tim_block_write(tmp_buf,0x67E,2);
        break;
        
      case 0x41F:                       
        tmp_byte = (unsigned char)*value;  /*  */
        sts = tim_block_write(&tmp_byte,0x6C3,1);
        break;
        
      default:
        if( (regno >= 0x401) && (regno <=0x41E) )
        {
            addr = ((regno - 0x401) * 2) + 0x687;
            wtmp = *value;  /*  */
            tmp_buf[0] = wtmp >> 8;
            tmp_buf[1] = wtmp;
            sts = tim_block_write(tmp_buf,addr,2);
        }
        if( (regno >= 0x300) && (regno <= 0x421) )
        {
            sts = MB_READ_ONLY_VALUE;
        }
        else
        {
            sts = MB_EXC_ILL_ADDR;          /* Bad "address" to write */
        }
        break;

        } /* End master switch/case on register number */

    return (sts);                       /* Propagate success/failure */

} /* End mbrWrReg() */

/************************* End of MODREG.C **********************************/

