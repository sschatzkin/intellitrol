/*********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         modfrc.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais  @Copyright 2009, 2014  Scully Signal Company
 *
 *   Description:    ModBus "Register" read and write routines
 *
 * Revision History:
 *   Rev      Date           Who   Description of Change Made
 *  --------  --------------  ---    --------------------------------------------
 *  1.5       09/23/08  KLL  Ported old Intellitrol code to the PIC24HJ256GP210 cpu
 *  1.5.30  08/10/14  DHP  Removed unused force case 5 and vapor cases 0xC, 0xD
 *
**********************************************************************************************/

#include "common.h"

#define MODBITOFF   0x0000
#define MODBITON    0xFF00
#define MODBITONE   0x0001

/*******************************************************************/
/*******************************************************************/

void mbfClrEnaFeatures
    (
    unsigned char bits                /* Enable-flags to clear */
    )
{
  if (SysParm.EnaFeatures & bits)     /* Any bits actually set? */
  {                               /* Yes */
    SysParm.EnaFeatures &= (unsigned char)(~bits); /* Clear requested bits */
    SysParm.Modbus_Ena_Features &= (unsigned char)(~bits);  /*  */
    modNVflag++;                    /* Flag EEPROM update needed */
  }

  /* If disabling VIP, also kill off the VIP-related LEDs */

  if (bits & ENA_VIP)
  {
    ledstate[VIP_AUTH] = DARK;
    ledstate[VIP_UNAUTH] = DARK;
    ledstate[VIP_IDLE] = DARK;
    StatusO &= ~0x7;     /* Clear VIP output status */
  }

} /* End mbfClrEnaFeatures() */

/*******************************************************************/
/*******************************************************************/

MODBSTS mbfSetEnaFeatures
    (
    unsigned char    bits                /* Enable-flags to set */
    )
{
unsigned char oldena = 0;                /* Previous incarnation */

  if ((SysParm.EnaFeatures & bits) != bits) /* Requested bits already set? */
  {                               /* No */
    if ((SysParm.EnaPassword & enable_jumpers & bits) != bits) /* Allowed? */
      return (MB_EXC_ILL_DATA);   /* No, error */

    oldena = SysParm.EnaFeatures;   /* Remember old ENA_VIP */

    SysParm.EnaFeatures |= bits;    /* Enable requested software features */
    SysParm.Modbus_Ena_Features |= bits;  /*  */

    modNVflag++;                    /* Note EEPROM update needed */
  }

  /* If enabling VIP, try to get the VIP-related LEDs right as well...
     (If VIP is *already* enabled, then leave it alone, 'cuz by definition
     it's already right!). Some effort is spent here trying to avoid
     cyling those BIG pumps needlessly. Probably overkill here, but hey,
     anything worth doing is worth doing to excess, right? */

  if ((bits & ENA_VIP)                /* Allowing VIP functionality */
       && ((oldena & ENA_VIP) == 0))  /*  where is wasn't allowed before? */
  {                               /* Yes */
      StatusO &= ~0x7;     /* Clear VIP output status */
      ledstate[VIP_AUTH] = DARK;      /* Assume not authorized */
      if (main_state == IDLE)         /* Any truck-like activity? */
      {                           /* No, easy case */
          ledstate[VIP_UNAUTH] = DARK;
          ledstate[VIP_IDLE] = LITE;
          StatusO |= STSO_STANDBY;
      }
      else if (StatusA & STSA_TRK_VALID) /* TIM previously authorized? */
          {                           /* Yes */
          /* This truck was previously authorized (truck gone clears the
             VALID bit, so we know it's still "the same truck"), so just
             keep it authorized, and make the LEDs reflect that. In par-
             ticular, don't force re-authorization, as that will force a
             non-permit/re-permit, which is hard on those BIG pumps! */
          ledstate[VIP_AUTH] = LITE;  /* Mark [re-]authorized */
          ledstate[VIP_UNAUTH] = DARK;
          ledstate[VIP_IDLE] = DARK;
          StatusO |= STSO_AUTHORIZED;
          }
      else if (StatusA & STSA_PERMIT) /* Otherwise Permissive? */
          {                           /* Yes */
          /* While not previously "Authorized" (ergo "Unauthorized" now),
             This truck was previously permitted, so fake an Authorization
             here to keep it permitting, and make the LEDs reflect this
             pseudo-"Authorized" condition. In particular, don't force
             non-permit/authorization/re-permit, which is hard on those
             BIG pumps! */
          StatusA |= STSA_TRK_VALID;  /* Valid (even though no TALK) */
          val_state = 1;              /* No need to authorize... */
          ledstate[VIP_AUTH] = LITE;  /* Mark [re-]authorized */
          ledstate[VIP_UNAUTH] = DARK;
          ledstate[VIP_IDLE] = DARK;
          StatusO |= STSO_AUTHORIZED;
          }
      else                            /* Some sort of truck-like activity */
          {                           /* Well, it can't be authorized yet */
          /* Note: if truck actively permitting, enabling VIP will nomi-
             nally force the unit non-permisive ("Unauthorized"), then re-
             permit "shortly" thereafter as the TIM of the connected truck
             is read and verified. However, the TIM is usually read and
             validated "so fast" that the unit never actually goes non-
             permissive (if TIM is in EEPROM and not TAS-validated)! The
             BIG pumps appreciate this little detail . . . */
          val_state = 0;              /* Re-validate, as needed */
          badvipflag = (unsigned int)BVF_INIT;/* In case actively permitting */
          ledstate[VIP_UNAUTH] = LITE;
          ledstate[VIP_IDLE] = DARK;
          StatusO |= STSO_UNAUTHORIZED;
          if ((print_once_msg & UN_AUTH) == 0)
          {
            print_once_msg |= UN_AUTH;
            printf("\n\rCan not locate ");
            Report_SN(&truck_SN[0]);
            printf(" in EEPROM\n\r");
          }
        }
  }

  return (MB_OK);

} /* End mbfSetEnaFeatures() */

/****************************************************************************
    Main routines -- mbxForce
****************************************************************************/

/*************************************************************************
* mbxForce -- Force-bits
*
* Call is:
*
*       mbxForce (fbit, fval)
*
* Where:
*
*       <fbit> is the "bit" to be forced (for all intents and purposes,
*       a "function index" or code);
*
*       <fval> is the "value" to which to force the bit (again, for all
*       intents and purposes here, an unsigned int arbitrary value).
*
* mbxForce() is the actual "bit forcer" for the ModBus "Force Bits" command.
* The "bit" is really just a function code, and the value passed is really
* just any ole value; a value of 0xFF00 is nominally "ON" or "TRUE" or "1"
* wrt the ModBus mindset.
*
* Return value is MB_OK (0) normally, or an exception value (~0) if there
* are any problems executing the requested function.
*
* Generally, MB_EXC_ILL_ADDR is unknown/illegal function, MB_EXC_ILL_DATA
* is wrong state, function not allowed due to something or other, etc.
*
*************************************************************************/

MODBSTS mbxForce
    (
    unsigned int fbit,          /* "Bit" or function code */
    unsigned int fval           /* Value/argument for bit/function code */
    )
{
MODBSTS sts = MB_OK;                /* Status holding value */

    /* Decode and dispatch on function code */

    switch (fbit)
    {
     case 0x0000:                      /* 0000 -- Forced Shutdown */
      if (fval == MODBITOFF)          /* Turning it off? */
          {                           /* Yes, always allowed */
          StatusB &= ~STSB_SHUTDOWN;  /* Clear "Shutdown" flag */
          return (MB_OK);             /* Success! */
          }

      if ((fval != MODBITON) && (fval != MODBITONE))
          return (MB_EXC_ILL_DATA);   /* Bad value */

/* Leave Bypass/et all alone (among other things, this clears out the */
/* VIP authorization, so an un-SHUTDOWN remains unauthorized...) */

      StatusB |= STSB_SHUTDOWN;       /* Latch the "Shutdown" bit */

      return (MB_OK);                 /* Success */

      case 0x0002:                      /* 0002 -- Recover (clear Shutdown) */
        if ((fval != MODBITON) && (fval != MODBITONE))
            return (MB_EXC_ILL_DATA);   /* Bad value */

        StatusB &= ~STSB_SHUTDOWN;      /* No longer shutdown */

        return (MB_OK);                 /* Up and running, probably */


      case 0x0003:                      /* 0003 -- Erase Truck ID list */
        if ((fval != MODBITON) && (fval != MODBITONE))
            return (MB_EXC_ILL_DATA);   /* Bad value */

        /* Since erasing the vehicle list is a time-consuming operation (in
           particular, takes more than 30 ms "active overfill" cycle interval),
           allow only if no truck actively connected. */

        if (main_state != IDLE)         /* Sittin' idle? */
            return (MB_EXC_BUSY);       /* No, tell 'im to try later */

        sts = (MODBSTS)nvTrkErase ();            /* Erase the Truck ID auth. list */
        loginit (EEV_TIMBLK);           /* Event-Log the erasure! */
        return (sts);

      case 0x0004:                      /* 0004 -- Erase Event Log */
        if ((fval != MODBITON) && (fval != MODBITONE))
            return (MB_EXC_ILL_DATA);   /* Bad value */

        /* Since erasing the Event Log is a time-consuming operation (in
           particular, takes more than 30 ms "active overfill" cycle interval),
           allow only if no truck actively connected. */

        if (main_state != IDLE)         /* Sittin' idle? */
        {
            return (MB_EXC_BUSY);       /* No, tell 'im to try later */
        }
        sts = (MODBSTS)nvLogErase ();            /* Erase the Event Log */
        loginit (EEV_LOGBLK);           /* Event-Log the erasure! */
        return (sts);

      case 0x0006:                      /* 0006 -- System ("hardware") reset */
        if ((fval != MODBITON) && (fval != MODBITONE))
            return (MB_EXC_ILL_DATA);   /* Bad value */

        secReset = 2;                   /* Reset system in a second or two */
        return (sts);                   /* Just time to send slave response */

      case 0x0008:                      /* 0008 -- Force Overfill Bypass */
        if (fval == MODBITOFF)          /* Turning it off? */
            {                           /* Yes, always allowed */
            clr_bypass (OVER_BYPASS);   /* Disable overfill bypass */
                                        /* Just leave Bypass_SN alone */
            return (MB_OK);             /* Success! */
            }

/******************************* 3/27/2009 8:07AM ****************************
 * KLL uncomment the below line of code for FlyingJ and define FlyingJ
 *****************************************************************************/
        return (MB_EXC_ILL_DATA);   /* Yes, reject command */
#ifdef FlyingJ
        if ((badoverfillflag)           /* Overfill bypass permissable? */
            && !(bystatus & BYS_DRY_NOOVFB)) /* (and not disallowed!) */
            {
            if ((fval != MODBITON)      /* Turning it on? */
                && (fval != MODBITONE)) /* Alternate implementation of "1" */
                return (MB_EXC_ILL_DATA); /* No, illegal "bit" value */
            if (bylevel & OVER_BYPASS)  /* Already on? */
                return (MB_OK);         /* Nothing to do */

            /* Turn on overfill bypass, leaving appropriate droppings */
            Reset_Bypass_SN (0xFF);     /* Mark Bypass-by-VIPER/ModBus */
            set_bypass (OVER_BYPASS);   /* Enable overfill bypass */
            }
        else
            {
            return (MB_EXC_ILL_DATA);   /* Overfill bypass rejected */
            }
        return (MB_OK);                 /* Overfill bypass successful */
#endif

      case 0x0009:                      /* 0009 -- Force Ground Bypass */
        if (fval == MODBITOFF)          /* Turning it off? */
            {                           /* Yes, always allowed */
            clr_bypass (GROUND_BYPASS); /* Disable ground-fault bypass */
                                        /* Just leave Bypass_SN alone */
            return (MB_OK);             /* Success! */
            }

        if (!(SysParm.EnaFeatures & ENA_GROUND)) /* Ground-check enabled? */
            return (MB_EXC_ILL_DATA);   /* No, reject command */

        if (bystatus & BYS_NONBYPASS)   /* Bypass disallowed? */
            return (MB_EXC_ILL_DATA);   /* Yes, reject command */

        if (badgndflag & GND_PROBLEMS)  /* Ground bypass permissable? */
            {
            if ((fval != MODBITON)      /* Turning it on? */
                && (fval != MODBITONE)) /* Alternate implementation of "1" */
                return (MB_EXC_ILL_DATA); /* No, illegal "bit" value */
            if (bylevel & GROUND_BYPASS)  /* Already on? */
                return (MB_OK);         /* Nothing to do */

            /* Turn on ground bypass, leaving appropriate droppings */

            set_bypass (GROUND_BYPASS); /* Enable ground bypass */
            Reset_Bypass_SN (0xFF);     /* Mark Bypass-by-VIPER/ModBus */
            }
        else
            {
            return (MB_EXC_ILL_DATA);   /* Ground bypass rejected */
            }

        return (MB_OK);                 /* Ground bypass successful */

      case 0x000A:                      /* 000A -- Enable Ground checking */
        if (fval == MODBITOFF)          /* Turning it off? */
        {                           /* Yes, always allowed */
          mbfClrEnaFeatures (ENA_GROUND);
          return (MB_OK);             /* Ground-check is disabled */
        }

        if (enable_jumpers & SysParm.EnaPassword & ENA_GROUND) /* Allowed? */
        {                           /* Yes, allowed to turn it on */
            if ((fval != MODBITON)      /* Turning it on? */
                && (fval != MODBITONE)) /* Alternate implementation of "1" */
                return (MB_EXC_ILL_DATA); /* No, illegal "bit" value */

            /* Enable Ground-Fault checking */

            if (mbfSetEnaFeatures (ENA_GROUND) != MB_OK)
            {
              return (MB_EXC_ILL_DATA);   /* Ground-check-enable rejected */
            }
        }
        else
        {
          return (MB_EXC_ILL_DATA);   /* Ground-check-enable rejected */
        }

        return (MB_OK);                 /* Ground-check is enabled */

      case 0x000F:                      /* 000F -- Enable DeadmanSwitch checking */
        if (fval == MODBITOFF)          /* Turning it off? */
            {                           /* Yes, always allowed */
            mbfClrEnaFeatures (ENA_DEADMAN);
            return (MB_OK);             /* Deadman-check is disabled */
            }

        if (SysParm.EnaPassword & enable_jumpers & ENA_DEADMAN) /* Allowed? */
        {                           /* Yes, allowed to turn it on */
            if ((fval != MODBITON)      /* Turning it on? */
                && (fval != MODBITONE)) /* Alternate implementation of "1" */
                return (MB_EXC_ILL_DATA); /* No, illegal "bit" value */

            /* Enable Deadman-Switch checking */

            if (mbfSetEnaFeatures (ENA_DEADMAN) != MB_OK)
            {
              return (MB_EXC_ILL_DATA);   /* Deadman-check-enable rejected */
            } else
            {
              ledstate[DEADMAN_BAD] = DARK;       /* Indicate Deadman enabled turn off bad */
              set_new_led(DEADMAN_GOOD, LITE);    /* Indicate Deadman enabled */
            }
        }
        else
            {
            return (MB_EXC_ILL_DATA);   /* Deadman-check-enable rejected */
            }

        return (MB_OK);                 /* Deadman-check is enabled */

      case 0x0012:                      /* 0012 -- Erase Bypass Key list */
        if ((fval != MODBITON) && (fval != MODBITONE))
            return (MB_EXC_ILL_DATA);   /* Bad value */

        sts = (MODBSTS)nvKeyErase ();            /* Erase the Truck ID auth. list */
        loginit (EEV_KEYBLK);           /* Event-Log the erasure! */
        return (sts);

      case 0x0013:                      /* 0013 -- Re-Format/Init all EEPROM */
        if ((fval != MODBITON) && (fval != MODBITONE))
            return (MB_EXC_ILL_DATA);   /* Bad value */

        /* Erasing the entire EEPROM is only slightly more time-consuming than
           the Truck ID list...and in particular, takes more than the 30 ms
           "active overfill" cycle interval), so allow the re-format operation
           only if no truck actively connected. */

        if (main_state != IDLE)         /* Sittin' idle? */
            return (MB_EXC_BUSY);       /* No, tell 'im to try later */

        sts = (MODBSTS)eeFormat();      /* Re-"Format" home block et al */
                                        /* eeFormat() event-logs this action */
        if (sts)                        /* If it failed, */
            return (MB_EXC_MEM_PAR_ERR); /* just blow it off */

        /* Given that we've just potentially invalidated almost everything we
           know (SysParm, SysVolts, etc., and so forth), just restart. */

        secReset = 1;                   /* Reset system soonest! */

        return (MB_OK);                 /* Done, for better or worse. */

      case 0x0015:                      /* 0015 -- Force VIP/Truck-ID Bypass */
        if (fval == MODBITOFF)          /* Turning it off? */
            {                           /* Yes, always allowed */
            clr_bypass (VIP_BYPASS);    /* Disable VIP bypass */
                                        /* Just leave Bypass_SN alone */
            return (MB_OK);             /* Success! */
            }

        if (!(SysParm.EnaFeatures & ENA_VIP)) /* VIP-check enabled? */
            return (MB_EXC_ILL_DATA);   /* No, reject command */

        if (bystatus & BYS_NONBYPASS)   /* Bypass disallowed? */
            return (MB_EXC_ILL_DATA);   /* Yes, reject command */

//        if ((badvipflag & 0xFF))                 /* VIP/Truck-ID bypass permissable? */
        if ((badvipflag & 0xFEFF))                 /* VIP/Truck-ID bypass permissable? */
            {
            if ((fval != MODBITON)      /* Turning it on? */
                && (fval != MODBITONE)) /* Alternate implementation of "1" */
                return (MB_EXC_ILL_DATA); /* No, illegal "bit" value */
            if (bylevel & VIP_BYPASS)   /* Already on? */
                return (MB_OK);         /* Nothing to do */

            /* Turn on VIP/Truck-ID bypass, leaving appropriate droppings */

            set_bypass (VIP_BYPASS);    /* Enable VIP/Truck-ID bypass */
            Reset_Bypass_SN (0xFF);     /* Mark Bypass-by-VIPER/ModBus */
            }
        else
            {
            return (MB_EXC_ILL_DATA);   /* VIP bypass rejected */
            }

        return (MB_OK);                 /* VIP bypass successful */


      case 0x0016:                      /* 0016 -- Enable VIP checking */
        if (fval == MODBITOFF)          /* Turning it off? */
            {                           /* Yes, always allowed */
            mbfClrEnaFeatures (ENA_VIP);
            badvipflag = 0;             /* In case trying to actively permit */
            return (MB_OK);             /* VIP-checking is disabled */
            }

        if (enable_jumpers & SysParm.EnaPassword & ENA_VIP) /* Allowed? */
        {                           /* Yes, allowed to turn it on */
            if ((fval != MODBITON)      /* Turning it on? */
                && (fval != MODBITONE)) /* Alternate implementation of "1" */
                return (MB_EXC_ILL_DATA); /* No, illegal "bit" value */

            /* Enable VIP/Truck-ID-Authorization checking */

            if (mbfSetEnaFeatures (ENA_VIP) != MB_OK)
            {
              return (MB_EXC_ILL_DATA);   /* VIP-check-enable rejected */
            }
        }
        else
            {
            return (MB_EXC_ILL_DATA);   /* VIP-check-enable rejected */
            }

        return (MB_OK);                 /* VIP-checking is enabled */


      default:                          /* Unknown Illegal */
        return (MB_EXC_ILL_ADDR);
     }

} /* End of mbxForce() */

/************************* End of MODFRC.C **********************************/
