/*******************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         permit.c
 *
 *   Revision:       REV 1.6
 *
 *   Author:         Ken Langlais  @Copyright 2009, 2014  Scully Signal Company
 *
 *   Description:    Main program routines for Jumpered operations of the
 *                   Rack controller main microprocessor PIC24HJ256GP210
 *
 * Revision History:
 *   Rev      Date   Who  Description of Change Made
 * -------- -------- ---  --------------------------------------------
 * 1.5.23   04/19/12 KLL  Added StatusB error check to the reasons
 *                         not to allow permit.
 * 1.5.27   03/24/14 KLL  Remove cause of secondary, unwanted pulse in
 *                                       ground output.
 * 1.5.30  08/10/14  DHP  Removed all vapor checks
 * 1.6.32  03/30/15  DHP  For main_relay_state() and bak_relay_state() changed 
 *                         comparing array entries to next entry to comparing
 *                         to 0, the value when relay is closed (permitting).
 *                         For main_relay_state() added setting the shorted flag
 *                         if relay closed and main state idle.
 * 1.6.34  10/10/16  DHP  In permit_relay() added setting of the permit and non
 *                         permit LED states when forcing the non permit relay.
 *                        Added a call to service_charge() prior to checking
 *                         the relay states.  Service LED wasn't coming on but
 *                         the time was seen as excessive using an oscilloscope.
 *                        Fogbugz 141: Added check of iambroke and iamsuffering
 *                         to prevent permit when not well.
 * 1.6.35  02/07/17  DHP  In permit_relay() set Permit LED to LITE only if DARK
 *                         to not override the deadman warning.
 *         03/17/17       In permit_bypass() added code to call check_bk() to
 *                         inform backup processor of OK to permit status.
 ******************************************************************************/
#include "common.h"
#include "evlog.h"      /* Event Logging definitions */
#include "volts.h"
extern void check_bk(void);

/*************************************************************************
*  subroutine:  main_charge()
*
*  function:    This routine fires the output connection to the
*               charge pump for the permit relay.
*  input:  none
*  output: none
*
*************************************************************************/
void main_charge(void)
{
char status;
  // last_routine = 0x2C;
  if ( drive_time < read_time() )
  {
    status = permit_bypass();        /* can we allow permit */

    if ( status == TRUE )            /* Can we? */
    {                                /* Yes - toggle the MAIN_CHARGE bit */
      if ( no_relay_flag == FALSE)
      {
        if(MAIN_CHARGE)              /* LATA13, Output 1 is HIGH? */
        {
          MAIN_CHARGE = 0;           /* Yes, turn it LOW */
        }
        else                         /* Is it LOW */
        {
          MAIN_CHARGE = 1;           /* Toggle to HIGH */
        }
      }
      drive_time = (read_time() + 20);   /* guarantee a max 50Hz frequency */
    }
    else
    {                                /* No */
      MAIN_CHARGE = 0;               /* Force charge pump off */
    }
  }
} /* end of main_charge */

/*************************************************************************
*  subroutine:      init_permit_relay
*
*  function:
*
*
*         Sets or Clears the Main Relay depending on the input argument which
*         is either ON or OFF
*         Sets the proper LED's ON or OFF depending on state of functions
*
*         Maintains STSA_PERMIT according to current state (main_charge()
*         now keys off of STSA_PERMIT being set).
*
*  input:
*  output: none
*
*
*************************************************************************/
static unsigned long main_relay_time;
static unsigned long bypass_message;
static char hold_turn_on;
static char bak_relay_ops;
static char main_relay_ops;

void init_permit_relay (void)
{
  // last_routine = 0x2D;
    hold_turn_on = FALSE;
    bak_relay_ops = FALSE;
    main_relay_ops = FALSE;
    main_relay_time = read_time()+SEC1;
    relay_turn_on = 0;
    bypass_message = 0;
    /* At this point, we should ABSOLUTELY NOT be permitting! */
    MAIN_ENABLE = CLR;              /* Just to make double sure! */
}   /* End init_permit_relay() */
/*************************************************************************/

/*************************************************************************/
void permit_relay (void)
{
  char status, relay_status;

  // last_routine = 0x2E;
  if ((main_state != ACTIVE) || (iambroke) || ((StatusA & STSA_FAULT) != 0) || (StatusB))            /* If we're not "ACTIVE"ly truckin' */
  {                                     /* (We're not) */
    MAIN_ENABLE = CLR;             /* Then force main relay non-permit */
//>>> FogBugz 141
    set_nonpermit(LITE); 
    set_permit(DARK);
//<<< FogBugz 141
  }
  else
  if (relay_turn_on < read_time()) /* Actively trucking, holding back? */
  {                                     /* No, check for main relay action */
    if (((bypass_time + (unsigned long)(SysParm.BypassTimeOut * 1000L))
          < read_time())
        && (StatusA & STSA_BYPASS))
    {
      bylevel = 0;                    /* Clear all pending bypass activity */
      bystatus |= BYS_NONBYPASS;      /* Prohibit any further bypass act */
      if ( bypass_message < read_time() )
      {
        xprintf( 48, DUMMY );
        bypass_message = (read_time() + MIN1);
      }
    }
    status = permit_bypass();          /* can we allow permit */
    /*********************** relay permit state **********************/
    if (main_relay_time < read_time())
    {
      service_charge();                     /* DHP - excessive time */
      relay_status = main_relay_state (status);    /* What should it be ? */
      /* Get appropriate Main relay state:
       *   If status = TRUE and the main relay is NOT pulled in a TRUE will occur
       *   If status = FALSE and the main relay is pulled in, a TRUE will occur */
      if (relay_status == FALSE)
      {
        main_relay_ops = FALSE;       /* the main relay is open */
      }
      else
      {
        main_relay_ops = TRUE;        /* the main relay is closed */
      }
      relay_status = bak_relay_state (status); /* Get Backup relay state */
      if ( relay_status == FALSE )
      {
        bak_relay_ops = FALSE;        /* the backup relay is open */
      }
      else
      {
        bak_relay_ops = TRUE;         /* the backup relay is closed */
      }
      main_relay_time = (read_time() + SEC1);
    }
    /*********************** end relay permit led ******************/
    /*********************** do permit ***********************************/
    if (StatusB & STSB_SPECIAL)       /* In "Special Operations" mode? */
    {                                 /* Yes */
       /* Here if we are in "Special Operations" mode. This means that we
          are ignoring trucks and the like, and in particular, will never
          permit! The "meaning" of the PERMIT and NONPERMIT LEDs is dependent
          on the funny mode, so we leave them alone here. The relay
          states are monitored, since we are "operational" vis-a-vis the
          ModBus and TAS/VIPER communications, so we want the state of the
          relays/etc. available -- we just won't permit... */

       MAIN_ENABLE = CLR;         /* Main relay forced off always! */
    }
    else
    {
      if (status)                 /* Normal operations, set relay/LEDs */
      {
        if ( (bak_relay_ops == TRUE) &&   /* the backup relay closed */
            (main_relay_ops == TRUE) )   /* the main relay closed */
        {
          if (StatusA & STSA_BYPASS)  /* if o.f. bypass active, set led's appropriately */
          {
            /*********************** bypassed ******************************/
            if ( bylevel & OVER_BYPASS )
            {
              set_nonpermit(FLASH1HZ75);
              set_permit(DARK);
            }
            else
            {
              set_nonpermit(DARK);
              if (ledstate[PERMIT] != FLASH4HZ)
                {
                  set_permit(FLASH1HZ75);
                } 
            } /*********************** end bypassed **************************/
          }
          else
          {
            set_nonpermit(DARK);
            if(ledstate[PERMIT] == DARK)
            {
              set_permit(LITE);
            }
          }
        }
        if ( (bak_relay_ops == FALSE) ||   /* the backup relay open */
            (main_relay_ops == FALSE) )   /* the main relay open */
        {
          if ((main_relay_ops == TRUE) && (bak_relay_ops == FALSE))
          {
            xprintf( 140, DUMMY);
          }
          set_nonpermit(LITE);
          set_permit(DARK);
        }
        if ((main_relay_ops == TRUE) && (bak_relay_ops == TRUE))        /* the backup relay closed */
        { 
          xprintf( 141, DUMMY);
        } 
        /*********************** end relay permit led ******************/
        StatusA |= STSA_PERMIT;        /* Intellitrol wants to permit */
        MAIN_ENABLE = SET;             /*  enable the main relay */
        hold_turn_on = TRUE;           /* toggle setup for chatter anti-lockup */
      }
      else               /* NON_PERMIT - NOTE this is really /MAIN_RELAY */
      {
        /*********************** no permit *******************************/
        set_permit(DARK);
        set_nonpermit(LITE);
        StatusA &= ~STSA_PERMIT;       /* Intellitrol now *NOT* permissive */
        MAIN_ENABLE = CLR;             /* disable the main relay */
       if (hold_turn_on)
        {
          relay_turn_on = (read_time() + (unsigned long)((unsigned long)(unsigned char)delay_time * (unsigned long)SEC1));/* prevent chatter */
          hold_turn_on = FALSE;
          main_relay_time = 0;
        } /*********************** end no permit ****************************/
      }
    }
  } /*********************** end do permit *******************************/
}   /* end of permit_relay */

/*************************************************************************
*  subroutine:      permit_bypass
*
*  function:
*
*         Set the status to TRUE/FALSE depending on the level
*         of allowed operation or bypass allowed
*
*         Maintains setting of STSA_BYPASSABLE bit (meaning that we
*         cannot go permissive due to a bypassable condition).
*
*  Note:
*
*         Any "non-permissive" reasoning implemented here should some-
*         how be represented in the "Non-Permissive Reasons" register
*         (MODREG.C mbrNonPermitReg() logic). This is to allow the TAS
*         (e.g., VIPER-II, etc.) to readily display on the TAS screen
*         a quick and concise "state" of the Intellitrol.
*
*  input:   none
*  output:  TRUE (OK to PERMIT)/FALSE(Not OK)
*************************************************************************/
char permit_bypass(void)
{
   char status;                 /* temporary status value */
#define NOWAY       0           /* 0 == No Permit, No Bypass allowed */
#define NOTNOW      1           /* 1 == No Permit,    Bypass allowed */
#define GOFORIT     2           /* 2 ==    Permit */

  status = GOFORIT;                   /* Permissive by default, shut him */
                                      /*   off ONLY if he fails any test */
  // >>> Fogbugz 141
  if (iambroke || iamsuffering)   
  {
    status = NOWAY;                   /* No permit if broke or suffering */
  }
  else
  {
    // <<< Fogbugz 141
    if (tank_state == T_DRY)          /* Cover the Dry condition */
    {
      lowVolt = 9999;                 // Used for calc_tank() probe compartment count
        
      badoverfillflag = FALSE;        /* Not in bypassable overfill */
      bystatus &= ~BYS_WAIT_OVFB;     /* Clear overfill must-wait flag */
      if ( dry_timer > SEC30 )        /* If was "Consistently Dry" */
      {
        bystatus |= BYS_DRY_NOOVFB;   /* Then mark Was-Dry-Can't-Bypass */
        dry_timer = SEC45;                        /* ensure no roll over */
      }else
      if (( dry_timer < SEC2 ) && ( probe_try_state == THERMIS))
      {
         status = NOTNOW;             /* Let thermistors settle a bit longer */
      }
    }
    else
    {
      if (tank_state == T_INIT)       /* cover the Idle condition */
      {
        status = NOWAY;               /* Can't bypass "idle" */
        badoverfillflag = FALSE;      /* Not in a bypassable overfill */
        bystatus &= ~BYS_WAIT_OVFB;   /* So no point in waiting for it */
      }
      else                             /* Wet/fault/etc.  condition */
      {
        if (bystatus & BYS_DRY_NOOVFB)/* Once dry, never bypass! */
        {
          status = NOWAY;             /* Can't bypass Dry-to-Wet case */
          bylevel &= ~OVER_BYPASS;    /* (no more overfill bypassing) */
          bystatus &= ~BYS_WAIT_OVFB; /* (no more waiting either) */
          badoverfillflag = FALSE;    /* Not in a bypassable overfill */
        }
        else                          /* Not "Dry/No Bypass" */
        {                             /*  so candidate for overfill bypass */
          if (truck_state == THERMAL_TWO) /* Thermistor-based probe(s)? */
          {                           /* Yes */
            if ((unsigned long)MIN1 < read_time())     /* Wait a minute for them to warm up */
            {
              badoverfillflag = TRUE; /* Long enough, bypassable */
            }
            else
            {
              bystatus |= BYS_WAIT_OVFB; /* Set must-wait overfill bypass */
            }
          }
          else                             /* Thermistor-based probe(s)? */
          {                                /* No, other (optic, etc.) probes */
            if ((unsigned long)SEC5 < read_time())       /* Wait a bit for them to settle */
            {
              badoverfillflag = TRUE;    /* Long enough, bypassable */
            }
            else
            {
              bystatus |= BYS_WAIT_OVFB; /* Set must-wait overfill bypass */
            }
          }
        }
        if (badoverfillflag)              /* Bypassable overfill/fault cond? */
        {                                 /* Yes */
          bystatus &= ~BYS_WAIT_OVFB;    /* Clear overfill must-wait flag */
          if ((!(bylevel & OVER_BYPASS)) /* Want to bypass overfill protect? */
               && (status))               /*  and not locked out? */
          {
            status = NOTNOW;            /* Bypassable non-permissive */
          }
        }
        else
        {
          status = NOWAY;                /* Non-bypassable non-permissive */
        }
      }
    }
    
    /* Check all other non-permit/bypass states for OK */
    if (StatusB & STSB_NO_PERMIT)        /* Forced non-permit? */
    {                                    /* Yes */
      status = NOWAY;                   /* Non-bypassable non-permissive */
    }
    if (secReset)                        /* System reset pending/emminent? */
      {                                 /* Yes */
      status = NOWAY;                   /* Non-bypassable non-permissive */
      }
    if ((badvipflag & 0xFEFF))                      /* Unauthorized truck ID ? */
    {                                    /* Yes */
      if ((!(bylevel & VIP_BYPASS))     /* VIP (truck ID) bypass set? */
          && (status))                  /*  and not locked out? */
      {
        status = NOTNOW;               /* No -- Bypassable non-permissive */
      }
    }
    if (badgndflag & GND_PROBLEMS)       /* Bad ground? */
    {                                    /* Yes */
      if ((!(bylevel & GROUND_BYPASS))  /* Want to bypass Ground detect? */
          && (status))                  /*  and not locked out? */
      {
        status = NOTNOW;               /* No -- Bypassable non-permissive */
      }
    }
    
    /* Check for DEADMAN enabled */
    if ((status == GOFORIT) && (SysParm.EnaFeatures & ENA_DEADMAN) )
    {
      if (baddeadman)                  /* Deadman-switch preventing? */
      {                                /* Yes */
        status = NOWAY;                /* Non-bypassable non-permissive */
      }else
      {
         check_bk();                   /* let backup know permit is OK */
      }   
    }
  }
   /* Now update STSA_BYPASSABLE to reflect the existence of a bypassable
      non-permissive state (clear if permit or non-bypassable). */
  switch (status)
  {
    case NOWAY:                       /* Non-bypassable non-permissive */
      StatusA &= ~STSA_BYPASSABLE;    /* Clear bypassable state */
      return (FALSE);                 /* Return non-permissive */

    case NOTNOW:                      /* Bypassable non-permissive */
      StatusA |= STSA_BYPASSABLE;     /* Yes, indicate bypassability */
      return (FALSE);                 /* But return non-permissive */

    case GOFORIT:                     /* Permissive */
      StatusA &= ~STSA_BYPASSABLE;    /* NOTHING to bypass */
      return (TRUE);                  /* Return permissive */

    default:
      break;
      /* Default case fall through */
  } /* End switch on status */
  printf("\n\rShould never get here!\n\r");
  return(FALSE);                        /* Should never get here! */
} /* end of permit_bypass */

/*************************************************************************
 *  subroutine:      clr_bypass()
 *
 *  function:
 *
 *          Disables bypass for the specified condition.
 *
 *          It is the caller's responsibility to ascertain whether or not
 *          bypass *should* be disabled/makes sense to disable. In particu-
 *          lar, setting bypass_SN, logging bypass events, etc. are all
 *          duties of the caller.
 *
 *  input:  bylevel mask of bypass conditions to disable.
 *  output: none
 *
 *
 *************************************************************************/

void clr_bypass
    (
    unsigned char bits                   /* Mask of 'bylevel' conditions to *NOT* bypass */
    )
{
    /* Disable bypass conditions for selected bypass "levels" */

  // last_routine = 0x30;
    bylevel &= ~bits;          /* Disable requested condition(s) */
    if (bylevel & (VIP_BYPASS | GROUND_BYPASS | OVER_BYPASS))
    {                                  /* Cleared last bypass enable */
       StatusA &= ~STSA_BYPASS;        /* Unit is no longer bypassed */
       Reset_Bypass_SN (0x00);         /* Clear bypass serial number array */

        /* This is the only way -- short of disconnecting the truck -- to
           reset the bypass timer, and can only be done via TAS/VIPER */
  // last_routine = 0x30;

       bypass_time = 0;                /* New bypass gets new timer */
    }
}    /* End clr_bypass() */

/*************************************************************************
 *  subroutine:      set_bypass()
 *
 *  function:
 *
 *          Enables bypass for the specified condition.
 *
 *          It is the caller's responsibility to ascertain whether or not
 *          bypass *should* be enabled/makes sense to enable. In particular,
 *          setting bypass_SN, logging bypass events, etc. are all
 *          duties of the caller.
 *  Called by: mbxForce() on force command bypass or read_bypass() on key bypass.
 *  input:  bylevel mask of bypass conditions to enable.
 *  output: none
 *
 *
 *************************************************************************/
void set_bypass
    (
    unsigned char bits                   /* Mask of 'bylevel' conditions to bypass */
    )
{
  EVI_BYPASS info;            /* Bypass Event Log info block */
  // last_routine = 0x31;
  if (bystatus & BYS_NONBYPASS)       /* Bypass activity prohibited? */
  {
    return;                         /* Yes */
  }
  /* Enable bypass conditions for selected bypass "levels" */
  bylevel |= bits;                    /* Enable requested condition(s) */
  StatusA |= STSA_BYPASS;             /* Note (latch) in main status too */
  /* Make sure the bypass timer (limit) is running, but don't re-set it
     again - i.e., don't "extend" the timer, limit it to time from first
     set, not last (i.e., "now") set. */
  if (bypass_time == 0)               /* If timer not yet enabled */
  {
    bypass_time = read_time();        /* Init timer */
  }
  memcpy (info.Key, bypass_SN, BYTESERIAL);     /* Copy bypass key id */
  memcpy (info.Truck, (unsigned char *)&truck_SN[0], BYTESERIAL); /* Copy Truck id */
  memset (info.future, 0xFF, sizeof(info.future));
  // last_routine = 0x31;
  if (TIM_state)
  {/* Got a truck serial number? */
    nvLogMerge (EVBYPASS, (char)bits, (char *)&info, (5 * 60));
  }
  else
  {/* No, can't distinguish trucks... */
    nvLogPut (EVBYPASS, (char)bits, (char *)&info); /* Log Bypass event */
  }
  // last_routine = 0x31;
  return;
}    /* End set_bypass() */

/*************************************************************************
 *  subroutine:      reset_bypass()
 *
 *  function:
 *
 *         1.  Reset the bypass flags to a known state
 *         2.  Set ground time variable
 *         3.
 *
 *
 *  input:  none
 *  output: none
 *
 *
 *************************************************************************/
void   reset_bypass( void )
{
  Permit_OK = FALSE;
  bystatus = 0;                         /* Clear all bypass status/flags */
  bylevel = 0;
  bypass_time = 0;
  badgndflag = GND_INIT_TRIAL;
  ground_time = read_time();            /* reset timer */
  if ((SysParm.EnaFeatures & ENA_VIP)   /* Running w/VIP functionality? */
       && (main_state != IDLE))         /*  and with truck activity? */
  {                                     /* Yes */
    badvipflag = (char)BVF_INIT;        /* Init VIP flags */
    if (SysParm.TASWait)                /* Requiring TAS intervention? */
    {                                   /* Yes */
      badvipflag |= BVF_TASDELAY;       /* Give TAS first refusal */
      StatusO &= ~0x7;                  /* Clear VIP output status */
      ledstate[VIP_AUTH] = DARK;
      ledstate[VIP_IDLE] = FLASH1HZ87;  /* Indicate truck found and waiting...*/
      StatusO |= STSO_STANDBY;
    }
  }
  else if (!(SysParm.EnaFeatures & ENA_VIP))    /* VIP disabled */
  {
    badvipflag = 0;                     /* Reset the problem flag */
  }                                     /* and do not bother with TAS */
  badoverfillflag = 0;
  Reset_Bypass_SN (0x00);               /* Clear bypass serial number array */
  StatusA &= ~STSA_BYPASS;              /* No Bypass activity */
  return;
} /* end of reset_bypass */

/*************************************************************************
*  subroutine:      main_relay_state
*
*  function:
*            Update MainRelaySt based on input of main permit state and monitored state of main
*             relay contacts.  Return TRUE/FALSE depending on the state of the relay contacts.
*
*  input:   operation== TRUE  (Main in Permit state; relay closed)
*                           == FALSE (Main in Non-permit state; relay open)
*  output:  TRUE   (main relay closed - Permit state)
*              FALSE (main relay open - Non-permit state)
*           Updates MainRelaySt "State" variable
*
*  input:   operation== TRUE  (Main in Permit state; relay closed)
*                           == FALSE (Main in Non-permit state; relay open)
*  output:  TRUE   (main relay closed - Permit state)
*              FALSE (main relay open - Non-permit state)
*           Updates MainRelaySt "State" variable
*
*************************************************************************/
#define MIN_OSC 4

char  main_relay_state( char operation )
{
   static char faultm = 0;          /* Hysteresis counter */

   unsigned char index;           /* try for several oscillations before faulting */
   char        open_mcount;     /* main relay status >1 OPEN */
   char        status;          /* temporary status value */

  // last_routine = 0x33;
  if ( no_relay_flag == TRUE)
  {
    if ( operation == TRUE)
    {
      MainRelaySt |= RELAY_CLOSED;
    }
    else
    {
      MainRelaySt &= ~(RELAY_CLOSED | RELAY_SHORTED | RELAY_NOTSURE);
    }
    return operation;
  }

  open_mcount = 0;

  for (index=0; index<(MAX_RELAY-1); index++)    /* " < 7 " */
  {
     if ( main_array[index] != 0 )
     {
        open_mcount++;             /* if not 0, we're oscillating: and main relay appears OPEN */
     }                             /* NOTE: we are looking at half rectified AC line frequecy here */
  }

  /* Update Main relay state (reportable via ModBus) */
  if (open_mcount >= MIN_OSC)          /* Main relay OPEN ? 4 US, 5 Europe */
  {                                    /* Yes */
     MainRelaySt &= ~(RELAY_CLOSED | RELAY_SHORTED | RELAY_NOTSURE);
     status = FALSE;                   /* Return status to caller */
     if (operation)                    /* Caller want relay OPEN? */
     {                                 /* No! */
        if (faultm++ > 5)              /* A touch of hysteresis... */
        {
           MainRelaySt |= RELAY_BROKEN;   /* Relay appears to be broken */
           xprintf( 4, DUMMY );
        }
     }
     else
     {                                 /* Caller wants relay OPEN */
        faultm = 0;                    /* Reset hysteresis counter */
        xprintf( 16, DUMMY );
     }
  }
  else if (open_mcount < 2)            /* Main relay CLOSED ?  */
  {                                    /* Yes */
     MainRelaySt &= ~(RELAY_BROKEN | RELAY_NOTSURE);    /* Clear Flag */
     MainRelaySt |= RELAY_CLOSED;                       /* and Set Flag */
     status = TRUE;                    /* Return status to caller */
     if (!operation)                   /* Caller want relay CLOSED? */
     {                                 /* No! */
        if ((main_state == IDLE) || (faultm++ > 5)) /* A touch of hysteresis...  */
        {
           MainRelaySt |= RELAY_SHORTED;  /* Relay appears shorted */
           xprintf( 5, DUMMY );
        }
     }
     else
     {                                 /* Caller wants relay CLOSED */
        faultm = 0;                    /* Reset hysteresis counter */
        xprintf( 16, DUMMY );
     }
  }
  else
  {
     status = 0;                       /* Unknown/in transition */
     MainRelaySt |= RELAY_NOTSURE;     /* Flag in transition, and leave */
                                       /*  rest of flags alone for now */
  }
  return(status);
}    /* end of main_relay_state */

/*************************************************************************
*  subroutine:      bak_relay_state
*
*  function:
*            Update BackRelaySt based on input of main permit state and monitored state of backup
*             relay contacts.  Return TRUE/FALSE depending on the state of the relay contacts.
*
*  input:   operation== TRUE  (Main in Permit state; relay closed)
*                           == FALSE (Main in Non-permit state; relay open)
*  output:  TRUE   (Backup relay closed - Permit state)
*              FALSE (Backup relay open - Non-permit state)
*           Updates BackRelaySt "State" variable
*
*************************************************************************/
char  bak_relay_state( char operation )
{
   unsigned char index;            /* try several oscillations before faulting */
   char          open_bcount;      /* backup relay status >1 OPEN */
   char          status;           /* temporary status value */
   static char   faultb = 0;       /* fault count before report */
  // last_routine = 0x34;
  if ( no_relay_flag == TRUE)
  {
    if ( operation == TRUE)
    {
      BackRelaySt |= RELAY_CLOSED;
    }
    else
    {
      BackRelaySt &= ~(RELAY_CLOSED | RELAY_SHORTED | RELAY_NOTSURE);
    }
    return operation;
  }
  open_bcount = 0;
  for (index=0; index<(MAX_RELAY-1); index++)
  {
     if ( bak_array[index] != 0 )
     {
        open_bcount++;             /* if not 0, we're oscillating; backup relay appears OPEN */
     }
  }
  /* Update Backup relay state (reportable via ModBus) */
  if (open_bcount >= MIN_OSC)          /* Backup relay OPEN ? */
  {                                    /* Yes */
     BackRelaySt &= ~(RELAY_CLOSED | RELAY_SHORTED | RELAY_NOTSURE);
                                       /* Should "Shorted" be cleared ??? */
     status = FALSE;                   /* Return status to caller */
     if (operation)                    /* Caller want relay OPEN? */
     {                                 /* No! */
        if (faultb++ > 10)             /* A [larger] touch of hysteresis ... (10 SJM) */
        {
           BackRelaySt |= RELAY_BROKEN; /* Backup/relay appears broken */
           xprintf( 6, DUMMY );
        }
     }
     else
     {                              /* Caller wants relay OPEN */
        faultb = 0;                 /* Reset hysteresis counter */
        xprintf( 17, DUMMY );
     }
  }
  else if (open_bcount < 2)         /* Backup relay CLOSED ? (10 SJM) */
  {                                 /* Yes */
     BackRelaySt &= ~(RELAY_BROKEN | RELAY_NOTSURE);
     BackRelaySt |= RELAY_CLOSED;
     status = TRUE;                    /* Return status to caller */
     if (!operation)                   /* Caller want relay CLOSED? */
     {                                 /* No! */
        if (main_state == IDLE)
        {                                /* If main is idle, what is backup?? */
           BackRelaySt |= RELAY_SHORTED; /* Backup appears shorted */
           xprintf( 7, DUMMY );
        }
     }
     else
     {                              /* Caller wants relay CLOSED */
        xprintf( 17, DUMMY );
     }
  }
  else
  {
     status = FALSE;                       /* Unknown/in transition */
     BackRelaySt |= RELAY_NOTSURE;     /* Flag in transition, and leave */
                                       /*  rest of flags alone for now */
  }
  return(status);
} /* end of bak_relay_state */

/**************************** end of permit.c *******************************/

