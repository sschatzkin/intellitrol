/*********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         specops.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *   Description:    Special Operations Modes processing
 *
 * Revision History:
 *   Rev      Date       Who   Description of Change Made
 *  -------- ---------        ---      --------------------------------------------
 *  1.5.30  08/10/14  DHP  Removed vapor check/ references
 *
 *********************************************************************************************/


#include "common.h"
// #include "ledlite.h"            /* LED definitions */


/****************************************************************************
* SpcLEDBlink -- Blink the LEDs in Special Ops mode
*
* Call is:
*
*   SpcLEDBlink ()
*
* SpcLEDBlink() will cause all LEDs below the 8 channel/probe state LEDs
* to blink. This is the "user interface" to tell the user that we are in
* "Special Operations" mode...
****************************************************************************/

void SpcLEDBlink(int compartment_flag)
{
LED_NAME i;

    /* Turn off the 8 channel/probe state LEDs */

    if ( compartment_flag)
    {
      for (i = COMPARTMENT_1; i <= COMPARTMENT_8; i++)
        ledstate[i] = FLASH2HZ;             /* Kill channel lights */
    } else
    {
      for (i = COMPARTMENT_1; i <= COMPARTMENT_8; i++)
        ledstate[i] = DARK;             /* Kill channel lights */
    }

    /* Turn on all "operational" LEDs below that to quick blink */

    if (new_front_panel == TRUE)
    {
      ledstate[OPTIC_OUT] = FLASH2HZ;         /* All ops LEDs to quick blink */
      ledstate[OPTIC_IN] = FLASH2HZ;          /* All ops LEDs to quick blink */
      ledstate[TRUCKCOMM] = FLASH2HZ;         /* All ops LEDs to quick blink */
      ledstate[TASCOMM] = FLASH2HZ;           /* All ops LEDs to quick blink */
      ledstate[GROUND_GOOD] = FLASH2HZ;       /* All ops LEDs to quick blink */
      ledstate[VIP_IDLE] = FLASH2HZ;          /* All ops LEDs to quick blink */
      ledstate[DEADMAN_GOOD] = FLASH2HZ;       /* All ops LEDs to quick blink */
    } else
    {
      for (i = OPTIC_OUT; i < NONPERMIT; i++) /* (Skip DynaCheck) */
      {
        if (!(SysParm.EnaFeatures & ENA_VIP)) /* VIP enabled? */
            {                           /* No. VIP disabled */
            if ((i == VIP_IDLE) || (i == VIP_AUTH) || (i == VIP_UNAUTH))
                continue;               /* Don't use VIP LEDs */
            }

        if (!(SysParm.EnaFeatures & ENA_GROUND)) /* Ground Check enabled ? */
        {                           /* No */
            if ((i == GND_BAD)||(i == GND_GOOD))           /* Skip Ground LED */
            {
                continue;                        /* Don't use Ground LED */
            } 
        }

        /* Useable LED, blink it */

        ledstate[i] = FLASH2HZ;         /* All ops LEDs to quick blink */
      }

    } /* End for all [rest of] LEDs */

} /* End of SpcLEDBlink() */

/****************************************************************************
* SpcLEDOff -- Turn off Special Ops blinkenLeds
*
* Call is:
*
*   SpcLEDOff ()
*
* spcLEDOff() turns off all the "Special Ops" LED blinking.
****************************************************************************/

void SpcLEDOff(void)
{
    LED_NAME i;

    /* Turn off everything ('cept for DynaCheck(tm) of course) */

    for (i = COMPARTMENT_1; i <= FREELED29; i++)
        ledstate[i] = DARK;             /* Kill all LEDs */

    asm volatile("nop");    /* place for the breakpoint to stop */
    asm volatile("nop");    /* place for the breakpoint to stop */
    asm volatile("nop");    /* place for the breakpoint to stop */
    ledstate[DYNACHEK] = FLASH2HZ;      /* *EXCEPT* for DynaCheck(tm) */

    return;

}   /* End of SpcLEDOff */

/****************************************************************************
* SpecialOps() -- top-level "Special Ops" handler
*
* Call is:
*
*       SpecialOps ()
*
* SpecialOps() is responsible for handling "loop level" special operations
*
* At the moment, this consists of setting the LEDs to blink...
*
* Someday RealSoonNow, this will handle the "manual" bypass key enter/erase
* Jumpers. Who knows what else it may grow to encompass!
*
****************************************************************************/

/* Interval (ms) between successive idle checks for Bypass Key */

#define KEYIDLINT 750

/* Interval (ms) between successive read/compares of Bypass Key */

#define KEYACTINT 250

void SpecialOps(void)
{
  static unsigned long spctime; /* General state timer, many uses */
  static unsigned char spckey[BYTESERIAL]; /* Holding copy of Bypass Key */
  static signed int spccnt;  /* Bypass Key counter */
  static signed int no_key_found;  /* Keep track of how many no key found */

  unsigned index;                 /* Key index */
  unsigned int csts;          /* For memcmp() and the compiler */
  int sts;                    /* Local status variable */

  set_porte (OFF);                    /* All channels off! */
                                      /* (also turns off Jump-Start) */
  if ((ledstate[GROUND_GOOD] == FLASH2HZ) && (ledstate[VIP_IDLE] == FLASH2HZ))
  {
    set_new_led(DEADMAN_GOOD, FLASH2HZ);
  } else
  {
    set_new_led(DEADMAN_GOOD, DARK);
  }

  /* Dispatch to handle the Special Operations modes:

      "ERASE KEY LIST" jumper in place -- erase the Bypass Key authori-
      zation list stored in NonVolatile memory.

      "ADD KEY S/N" jumper in place -- add a new Bypass Key number to
      the Bypass Key list stored in NonVolatile memory.
      */
// printf("SpecialOps_state: %d\n\r", SpecialOps_state);

  switch (SpecialOps_state)
  {

    case 0:                           /* Initial call */
      if (enable_jumpers & ENA_ERASE_KEYS)
          SpecialOps_state = 1;             /* Erase all keys init */
      else if (enable_jumpers & ENA_ADD_1_KEY)
      {
//          printf("\n\r*** Entering Special Operation Mode ***\n\r");
          SpecialOps_state = 10;            /* Add-a-Key init */
      }
      else
      {
          /* Hmmm...unknown Special Ops...Oh well, loop till reset */

//          ledstate[NONPERMIT] = FLASH4HZ; /* Someone is horribly confused! */
//          ledstate[PERMIT] = DARK;
        set_nonpermit(FLASH4HZ);
        set_permit(DARK);

        SpcLEDBlink (FALSE);             /* Special Ops signature */
      }
      break;

    /* Here for "ERASE KEY LIST" operation */

      case 1:                           /* Init for erase keys */
        SpcLEDOff ();                   /* Kill off all LEDs */
        spctime = (read_time() + 1000);   /* Pause for a second */
        SpecialOps_state++;
        break;

      case 2:
        if (spctime > read_time())      /* Time to do our thing? */
            break;                      /* Not yet */

        sts = nvKeyErase ();            /* Erase all keys */
        if (sts)                        /* Did the erase fail? */
        {
//            ledstate[NONPERMIT] = PULSE; /* Yes, flash BigRed */
          printf("\n\rErase bypass key memory Failed\n\r");
          set_nonpermit(PULSE);
        }
        else
        {
//            ledstate[PERMIT] = PULSE;   /* No, success, flash BigGreen */
          printf("\n\rFinished bypass key erase\n\r");
          set_permit(PULSE);
        }
        spctime = (read_time() + 250);    /* Wait 1/4 second */
        SpecialOps_state++;                   /* In next state */

        break;

      case 3:
        if (spctime > read_time())      /* Still waiting ? */
            break;                      /* Yup */

        /* This is the "ERASE done" state. Wait around for the user to reset
           the jumper(s) and RESET the system.

           Alternatively, we could just fall into the add-a-key state . . . */

        SpcLEDBlink (TRUE);               /* Blink LEDs aimlessly... */
        break;


    /* Here for "ADD KEY S/N" operation */

      case 10:                          /* Init for adding a key */
        SpcLEDBlink (FALSE);
        spctime = (read_time() + KEYIDLINT); /* Check once/second */
        SpecialOps_state++;
        no_key_found = 0;
        break;

      case 11:                          /* Wait for bypass key to appear */
        if (spctime > read_time())      /* Time to check again? */
            break;                      /* Nope */

        sts = Read_Bypass_SN();         /* Try to read Bypass Key */
        if (sts)                        /* Did we find a key? */
        {                               /* Yes */
          bystatus |= BYS_KEYPRESENT;   /* Note a key is present */
          SpecialOps_state++;               /* Advance to key-found state */
          SpcLEDOff();                  /* Tell user we see him/her */
          spctime += KEYACTINT;         /* Try again in a 1/4 second */
          memcpy (spckey, bypass_SN, BYTESERIAL); /* Holding copy of key */
          ledstate[COMPARTMENT_1] = LITE;      /* Indicate one-of-eight */
          spccnt = 0;                   /* First successful read */
        }
        else
        {
//          if (new_front_panel != TRUE)
//          {
          bystatus &= ~BYS_KEYPRESENT;  /* Bypass key is not present */
          spctime = (read_time() + KEYIDLINT); /* Check again in a second */
//          }
        }
        break;

      case 12:                            /* Key found, verify 7 times */
        if (spctime > read_time())        /* Time to check again? */
            break;                        /* Nope */

        sts = Read_Bypass_SN();           /* Read for the n'th time */
        if (sts == FALSE)                 /* Bypass key still there? */
        {                                 /* No. */
          if ((new_front_panel != TRUE) || (no_key_found > 10))
          {
            printf("Lost Bypass Key\n\r");
            no_key_found = 0;
            bystatus &= ~BYS_KEYPRESENT;  /* Bypass key is no longer readable */
            if (spccnt > 0)               /* First non-read? */
            {                             /* Yes */
              spccnt = (-spccnt);         /* Negate it as flag... */
              spctime += KEYACTINT;       /* Try again in a bit */
              break;                      /* Don't give up yet... */
            }
            else if (spccnt < 0)          /* Is this the n'th non-read */
            {                             /* Yes */
              ledstate[(signed int)COMPARTMENT_1 + (-spccnt)] = DARK; /* Back off one statelette */
              spccnt++;                   /* But don't give up yet */
              spctime += KEYACTINT;       /* Try again in a moment */
              break;
            }
            else                          /* Time to give up */
            {
              ledstate[COMPARTMENT_1] = DARK;      /* Last chance just got used up */
              SpecialOps_state = 10;      /* Reset to start of Add-A-Key cycle */
              break;                      /*   and give up */
            }
          } else
          {
            spctime += KEYACTINT;         /* Try again in a moment */
            no_key_found++;
            break;
          }
        }
        printf("Key found:  S/N: ");
        
        Report_SN (bypass_SN);                 /* Print serial number */
        bystatus |= BYS_KEYPRESENT;     /* We have a key readable [again] */
        if (spccnt < 0)                 /* Had problems before? */
        {                           /* Yes */
          spccnt = (-spccnt);         /* but we're feeling much better now */
          spctime += KEYACTINT;       /* Try again in a moment */
          break;
        }

        /* N'th successful read of a key, make sure same one! */

        csts = (unsigned)memcmp (spckey, bypass_SN, BYTESERIAL); /* Verify same Bypass Key */
        if (csts)                       /* If different */
        {                           /* Bypass Key changed! */
          SpecialOps_state = 11;            /* Reset our state to start */
          break;                      /*   and immediately try again */
        }

        spccnt++;                       /* Another successful read */
        ledstate[(signed int)COMPARTMENT_1 + spccnt] = LITE; /* Feedback to the user */
        if (spccnt == 7)                /* Eight successful reads? */
        {                           /* Yes */
          SpecialOps_state++;               /* New State: key acquired */
          spctime += 125;             /* Wait a LED cycle */
          
          ledstate[NONPERMIT] = DARK;
          ledstate[PERMIT] = LITE;
          service_wait(16);
          break;                      /*   and enter the key */
        }

        spctime += KEYACTINT;           /* Try again in a bit */
        break;                          /* Keep waiting for 8 reads */

      case 13:                          /* Successful Bypass Key acquisition */
        if (spctime > read_time())      /* Ready to enter key? */
            break;                      /* Not yet, wait a little longer */

        SpecialOps_state++;                   /* Next state: Wait for key to go away */
        spctime += 125;                 /* Minimum cycle time */

        for (sts = (signed int)COMPARTMENT_1; sts <= (signed int)COMPARTMENT_8; sts++) /* Annoy user to try and */
          ledstate[sts] = FLASH4HZ; /* Make him/her go away */
        /* Enter this [presumed new] Bypass Key in the NonVolatile store */

        sts = nvKeyFind (spckey, &index); /* See if key already authorized */
        if (sts == 0)                   /* If it is, */
        {                           /* (Key already authorized) */
//            ledstate[PERMIT] = PULSE;   /* Indicate success */
          printf("\n\rKey already stored in EEPROM\n\r");
          set_permit(PULSE);
          break;                      /* Wait for user to go away */
        }

        sts = nvKeyEmpty (&index);      /* Try to find an empty slot */
        if (sts)                        /* Any problems? */
        {                           /* Yes */
//            ledstate[NONPERMIT] = PULSE; /* Indicate failure */
          set_nonpermit(PULSE);
          break;
        }

        sts = nvKeyPut (spckey, index); /* Enter new key in empty slot */
        if (sts)                        /* Any problems? */
        {                           /* Yes */
//            ledstate[NONPERMIT] = PULSE; /* Indicate failure */
          set_nonpermit(PULSE);
          break;
        }

//        ledstate[PERMIT] = PULSE;       /* Success! */
        set_permit(PULSE);
        break;

      case 14:                          /* Wait for key to go away... */
        if (spctime > read_time())      /* Time to check again? */
            break;                      /* Nope */
        spctime += KEYACTINT;           /* Check again in a bit */

        sts = Read_Bypass_SN();         /* See if key readable */
        if (sts)                        /* Is it? */
        {                           /* Yes */
          for (sts = (signed int)COMPARTMENT_1; sts <= (signed int)COMPARTMENT_8; sts++) /* Annoy user to try and */
            ledstate[sts] = FLASH4HZ; /* Make him/her go away */
          break;                      /* Wait, and wait some more */
        }

        if (spccnt != 0)                /* First successful non-read? */
        {                           /* Yes */
          spccnt = 0;                 /* Require two successful non-reads... */
          break;
        }

        /* Bypass Key is gone, revert to idle and try again */

        bystatus &= ~BYS_KEYPRESENT;    /* Bypass key is not present */
        SpecialOps_state = 10;          /* Reset to idle/acquire-key state */
        break;                          /*   and that is that! */


    /* Shouldn't be any other states! */

      default:                          /* Unknown Special Ops state! */
//        printf("\n\rReset SpecialOps_state: %d to 0", SpecialOps_state);
        SpecialOps_state = 0;                 /* Back to ground zero */
        break;

  } /* End switch on SpecialOps_state */

} /* End SpecialOps() */

/************************* End of SPECOPS.C **********************************/
