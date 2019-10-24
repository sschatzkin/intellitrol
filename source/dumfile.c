/*******************************************************************************
 *
 *  Project:   Rack Controller
 *
 *  Module:       dumfile.C
 *
 *  Revision:     REV 1.5
 *
 *  Author:       Ken Langlais  @Copyright 2009, 2014  Scully Signal Company
 *
 *  Description:   Handles truck state display LEDs and variable initialization
 *
 *  Revision History:
 *
 * Revision   Date   Who  Description of Change Made
 * ------  --------  ---  ------------------------------------------------------
 * 1.5.23  10/19/11  KLL  init_variables() added gnd_retry, iambroke and 
 *                         iamsuffering with initial value 0.
 *         04/19/12  KLL  In init_variables() added toggle global variable and
 *                         initialized it to false.
 *                        Changed the deadman identification LEDs when a truck 
 *                         TIM can't be read from stand by to blinking unauthorized.
 * 1.5.27  10/22/12  KLL  Changed print statements 111 and 112 to only print
 *                         when the main_state is ACTIVE when they make sense.
 * 1.5.30  07/28/14  DHP  Added code to indicate which type of ground was detected.
 *                        Removed code related to vapor
 * 1.5.31  01/05/15  DHP  Removed init of unused "touch chip" variables
 * 1.6.32  04/02/15  DHP  Changed Intellitrol2 ground display to show ground 
 *                         error and good ground.
 * 1.6.34  07/08/16  DHP  QC Customer Complaint (QCCC) 53: Added probe_type[]
 *                           initialization and turn on of high current.
 *                         In unknown_probes() added secondary method of turning
 *                           off highI based on vapor jumper.
 *                        FogBugz 137: Added dry_5W_probes(), called by
 *                          optic_5_setup() and active_5W(), to correctly handle
 *                          probes_state[].
 *                        Changed dry_probes() to only change state on
 *                          number_of_Probes not max.
 *                        Changed report_tank_state() to use switch rather than 
 *                          long sequence of if else statements and incorporated 
 *                          use of new probe_type array in decisions.
  *****************************************************************************/
#include "common.h"
/*************************************************************************
 *  subroutine:      init_variables()
 *
 *  function:
 *               Set all global variables to known, i.e. default, values
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/

void   init_variables(void)
{
   unsigned char index;

   // last_routine = 0x83;
   gnd_retry        = 0;
   iambroke         = 0;
   iamsuffering     = 0;
   new_front_panel  = FALSE;            /* Default to old Front Panel */
   secReset         = 0;                /* Don't want any random RESETs */
   main_state       = IDLE;
   SpecialOps_state = 0;
   acquire_state    = IDLE_I;
   probe_try_state  = NO_TYPE;
   tank_state       = T_INIT;
   truck_state      = UNKNOWN;
   berr             = FALSE;
   groundiodestate  = 0;
   BadGndCntr       = 0;
   GoodGndCntr      = 0;
   BadVipCntr       = 0;
   enable_jumpers   = 0;
   StatusA          = STSA_IDLE;        /* If nothing else, we're idle... */
   StatusB          = 0;
   StatusO          = 0;
   StatusP          = 0;
   freetimer        = 1;
   drive_time       = 1;
   probe_time       = 1;
   ground_time      = SEC5;
   jump_time        = 0;                /* stays on forever if 1 */
   optic5_timeout   = 0;
   relay_turn_on    = 0;
   scully_flag      = FALSE;

   probe_index      = 0;                /* reset the ADC array index */
   active_comm      = 0;                /* reset the comm monitor */

   delay_time = 1;                      /* relay anti-chatter default time seconds */
                                        /* delay time default 1 seconds */
   unknown_probes();                    /* reset the probe states to unknown */

   /* ModBus/Communications line control/parameters */

   modbus_addr = get_modbus_addr();     /* Read Address for printf/ModBus */
  // last_routine = 0x83;
   modbus_baud = get_modbus_baud();     /* Read baud rate for printf/ModBus */
  // last_routine = 0x83;
   modbus_csize = get_modbus_csize();   /* Read character size */
  // last_routine = 0x83;
   modbus_parity = get_modbus_parity(); /* Read character parity */
  // last_routine = 0x83;
   modbus_Recv_err = 0;
   modbusVIPmode = 0;
   modbus_PrepMsg_err = 0;
   modbus_DecodeMsg_err = 0;
   for (index = 0;index < MAX_CHAN;++index)        /* Preset for positive edge */
   {
      old_probe_volt[index] = SysParm.ADCTHstNV;   /*(SysParm.ADCTmaxNV + SysParm.ADCOmaxNV)/2 - SysParm.ADCTHstNV;*/
   }
   disable_domeout_logging = FALSE;
} /* end of init_variables */

/*************************************************************************
 *  subroutine:      report_tank_state()
 *
 *  function:   Drive most of the LEDs (in particular, the "tank state" or
 *              probes LEDs.
 *
 *              The Optic In/Out and Communications Terminal/Truck LEDs
 *              asynchronously drive themselves via actual activity.
 *
 *              The Permit and Non-Permit LEDs are likewise driven asyn-
 *              chronously (see permit_relay() et al).
 *
 *  input:  none
 *  output: Lotsa photons.
 *
 *
 *************************************************************************/

void report_tank_state(void)
{
static unsigned long print_time = 0;
char           print_flag;
char           point=0;
int            index, tindex, all_states;  /* counter */
unsigned char  t1, t2, t3;
int max_display_channel;

  // last_routine = 0x84;
  if (StatusB & STSB_SPECIAL)          /* In Special Operations mode? */
  {
    return;                            /* Yes, leave LEDs alone */
  }
  if (main_state == IDLE)
  {
    return;                            /* Don't do tank UNLESS Active */
  }
  if ((badgndflag & GND_PROBLEMS)&&(SysParm.EnaFeatures & ENA_GROUND))       /* Problems with ground? */
  {
    set_new_led(GROUND_GOOD, DARK);      /* Green ground diode off */
    if (bylevel & GROUND_BYPASS)
    {
      ledstate[GND_GOOD] = DARK;             /* OFF if we're bypassed */
      ledstate[GND_BAD] = FLASH1HZ;        /* flash if we're bypassed */
    }else
    {
      ledstate[GND_GOOD] = DARK;          /* no ground */
      ledstate[GND_BAD] = LITE;                /* ground error */
    }   
  }
  else
  {
    ledstate[GND_BAD]  = DARK;              /* no ground error */
    if ((unit_type == INTELLITROL2)&&(SysParm.EnaFeatures & ENA_GROUND))
    {
      ledstate[GND_GOOD]  = LITE;          /* Good ground on */
    }
  }
  set_new_led(GROUND_GOOD, LITE);     /* Green ground diode on */
  /* Set VIP LED; only one (or "none" if the feature is disabled)is ever on */
  t1 = DARK;                           /* VIP Idle LED */
  t2 = DARK;                           /* VIP Authorized LED */
  t3 = DARK;                           /* VIP Unauthorized LED */
  if (SysParm.EnaFeatures & ENA_VIP)   /* Is VIP-functionality enabled? */
  {                                    /* Yes */
    t1 = LITE;                         /* Start with yellow quiescent state */
    if (badvipflag && (bylevel & VIP_BYPASS)) /* Bypassed unauthorized TIM? */
    {
      t2 = FLASH1HZ;                   /* Flash to indicate bypass */
      t1 = DARK;
    }
    else if (StatusA & STSA_TRK_VALID)   /* Otherwise valid/authorized TIM? */
    {                                    /* Yes */
      if (!(StatusA & STSA_BYPASSABLE))  /* Make sure it is not in bypassable fault */
      {
        if ((SysParm.VIPMode == 5) && (!(StatusA & STSA_TRK_TALK))) /* Mode 5 authorize No TIM exists  */
        {
          t2 = DARK;
          t1 = DARK;
          t3 = FLASH1HZ;
        }
        else
        {
          t2 = LITE;                      /* Green for Go */
          t1 = DARK;
        }
      }
      else if (badoverfillflag)
      {
        t3 = DARK;
        t1 = DARK;
        if (badvipflag == 0)
        {
          t2 = LITE;
        }
      }
      else if (badgndflag & GND_PROBLEMS)
      {
        t3 = DARK;
        t1 = DARK;
        if (badvipflag == 0)
        {
          t2 = LITE;
        }
      }
      else if (!((badvipflag & 0xFEFF) == 0))
      
      {
        t3 = LITE;                      /* And RED for Failure */
        t1 = DARK;
        t2 = DARK;
      }
    }
    else if (StatusA & STSA_TRK_PRESENT)   /* Is a truck present? */
    {                                      /* Yes */
      if (badvipflag & BVF_TASDELAY)      /* Waiting for TAS to make up its mind? */
      {
        t1 = FLASH1HZ87;                 /* Yes, "throb" the Yellow LED */
        t2 = DARK;
      }
      else if (badvipflag & BVF_TIMABSENT)  /* TIM could not be read - probably missing */
      {
        t1 = DARK;                            /*  */
        t2 = DARK;
        t3 = FLASH_5HZ;                   /* Indicate the TIM can't be read */
        if ((print_once_msg & UN_AUTH) == 0)
        {
          printf("\n\r   *** Unable to read the TIM ***\n\r");
          print_once_msg |= UN_AUTH;
        }
      }
      else if (!((badvipflag & 0xFEFF) == 0))             /* Anything else wrong?*/
      {
        t3 = LITE;                           /* Yes, just plain unauthorized truck */
        t1 = DARK;
        t2 = DARK;
      }
    }
  } /* End if ENA_VIP */
  /* WPW: Change for LED control by TIM_READ */
//FOGBUGZ 111	if ((SysParm.EnaFeatures & ENA_VIP) || SysParm.Ena_TIM_Read == 0)
  if ((SysParm.EnaFeatures & ENA_VIP) || (!(SysParm.EnaFeatures & ENA_VIP) &&
      (SysParm.Ena_TIM_Read == 1)))  //FOGBUGZ 111
	{
    StatusO &= ~0x7;     /* Clear VIP output status */
    ledstate[VIP_IDLE] = (char)t1;                 /* Set Yellow "Idle" VIP LED */
    if ( t1 == LITE)
    {
      print_once_msg &= ~UN_AUTH;
      StatusO |= STSO_STANDBY;
    }
    ledstate[VIP_AUTH] = (char)t2;                 /* Set Green "Authorized" VIP LED */
    if ( t2 == LITE)
    {
      print_once_msg &= ~UN_AUTH;
      StatusO |= STSO_AUTHORIZED;
    }
    ledstate[VIP_UNAUTH] = (char)t3;               /* Set Red "Unauthorized" VIP LED */
    if ( t3 == LITE)
    {
      if ((print_once_msg & UN_AUTH) == 0)
      {
        print_once_msg |= UN_AUTH;
        printf("\n\rCan not Verify TIM ");
        Report_SN(&truck_SN[0]);
      }
      StatusO |= STSO_UNAUTHORIZED;
    }
  }
  if ( print_time > (read_time() + (unsigned long)SEC5) )   /* assure not restarting */
  {
    print_time = read_time();
  }
  if ((print_time < read_time())       /* reduce update to 2 second rate */
      && (modbus_addr == 0))           /* ASCII update only if address 0 */
  {
    print_flag = 1;                   /* ASCII screen update on this pass */
    xprintf( 110, DUMMY );            /* Clear the probe status print line */
    print_time = (read_time() + SEC2);  /* Next screen update in coupla secs */
  }
  else
  {
    print_flag = 0;                   /* No ASCII screen update */
  }
  all_states = TRUE;                   /* FALSE if any probe not P_DRY */
  t3 = FALSE;                          /* Hi-8-Wet flag */
  if ( (!(ConfigA & CFGA_8COMPARTMENT)) && (probe_try_state != OPTIC5) )
  {
    point = 2;                        /* don't test first two probes */
    if ( main_state == ACTIVE)  /* Don't print status until in ACTIVE state */
    {
      if (print_flag)                   /* ASCII update cycle? */
      {
        xprintf( 111, DUMMY );
        xprintf( 111, DUMMY );  /* Rev 134 */
      }
    }
  }
  /****************************** 3/5/2010 6:35AM ****************************
   * To display the probe status the new front panel has 12 leds
   * the old front panel has 8 leds
   ***************************************************************************/
  if (new_front_panel == TRUE)
  {
    max_display_channel = MAX_CHAN_NEW;
  }
  else
  {
    max_display_channel = MAX_CHAN;
  }
  /* If the truck has just disconnected and we're in the Gone/Fini state,
     light up all the channel LEDs. In essence, we should only get here
     if we get stuck in Fini state due to problems with channel voltages.
     Gone will set all the channel LEDs on, this will transition them to
     flash if there's a problem (essentially saying "Go Away!"). */
  if (main_state == FINI)
  {
    for (index = 0; index < max_display_channel; index++)
    {
      set_compartment_led((int)COMPARTMENT_1 + index, FLASH4HZ);  /* Wet channel, turn on LED */
    }
  }
  if (truck_state == OPTIC_FIVE)
  {
    max_display_channel = (2*MAX_CHAN);
  }
  for (index=point,tindex=0; index<max_display_channel; index++,tindex++)
  {
    switch (probes_state[index])
    {
      case P_UNKNOWN:
      {
        all_states = FALSE;            /* At least one probe/channel not dry */
        if (index < MAX_CHAN)
        {
          set_compartment_led((int)COMPARTMENT_1 + tindex, DARK);  /* Possible 5-wire beyond wet */
        }
        if ( main_state == ACTIVE)  /* Don't print status until in ACTIVE state */
        {
          if (print_flag)                /* ASCII update cycle? */
          {
            xprintf( 112, DUMMY );
          }
        }
      }
      break;

      case P_WET:
      {
        all_states = FALSE;            /* At least one probe/channel not dry */
        if (index < MAX_CHAN)
        {
          set_compartment_led((int)COMPARTMENT_1 + tindex, LITE);  /* Wet channel, turn on LED */
          if (print_flag)             /* ASCII update cycle? */
          {
            xprintf( 113, DUMMY ); /* "WET   " */
          }
        }
        else
        {
           set_compartment_led((int)COMPARTMENT_1 + (tindex & 7), LITE);  /* Wet channel, turn on LED */
           set_compartment_led((int)COMPARTMENT_1 + 7, LITE);                  /*  and compartment 8 LED */
           t3 = TRUE;                  /* Optic-5 > 8 wet */
         }
      }
      break;

      case P_DRY:
      {
        if (((index < MAX_CHAN-1)          /* If channel 1..7 dry, and */
             && (probes_state[index+MAX_CHAN] != P_WET)) /* not 9..15 wet */
            )                      /* Or channel 8 dry and 9-15 all dry */
        {                                  /* Channels 8 to 15 all dry! */
          set_compartment_led((int)COMPARTMENT_1+tindex , DARK); /* Turn off channel 8 LED */
        }else
        if ((index ==MAX_CHAN-1)     /* If channel 8 dry, and */
             && !t3)                                  /* channel 8 dry and 9-15 all dry */
        { 
          set_compartment_led((int)COMPARTMENT_1+tindex , DARK); /* Turn off channel 8 LED */
        }else
        if (probes_state[index+MAX_CHAN] == P_WET)   /* 9..15 wet */
        {
             t3 = TRUE;
        } 
        if (print_flag)                    /* ASCII update cycle? */
        {
          // Kingsbury           xprintf( 114, DUMMY );
          if (probe_type[index] == P_OPTIC2)
          {
            printf("DryO ");
          }
          else   if (probe_type[index] == P_THERMIS)
          {
            printf("DryT ");
          }
          else
          {
            printf("Dry   ");
          }
        } 
      }
      break;

      case P_COLD:
      {
        all_states = FALSE;            /* At least one probe/channel not dry */
        set_compartment_led((int)COMPARTMENT_1+tindex , LITE);
        if (print_flag)                /* ASCII update cycle? */
        {
          xprintf( 117, DUMMY );
        }
      }
      break;

      case P_HOT:
      {
        all_states = FALSE;            /* At least one probe/channel not dry */
        set_compartment_led((int)COMPARTMENT_1+tindex , LITE);
        if (print_flag)                /* ASCII update cycle? */
        {
          xprintf( 118, DUMMY );
        }
      }
      break;

      case P_OPEN:
      {
        all_states = FALSE;                  /* At least one probe/channel not dry */
        set_compartment_led((int)COMPARTMENT_1+tindex , FLASH_5HZ); /* Flash LED */
        if (print_flag)                      /* ASCII update cycle? */
        {
          xprintf( 115, DUMMY );
        }
      }
      break;

      case P_FAULT:
      {
        all_states = FALSE;                 /* At least one probe/channel not dry */
        set_compartment_led((int)COMPARTMENT_1+tindex , FLASH1HZ); /* Flash LED */
        if (print_flag)                     /* ASCII update cycle? */
        {
          xprintf( 217, DUMMY );
        }
      }
      break;

      case P_GROUND:
      {
        all_states = FALSE;                 /* At least one probe/channel not dry */
        set_compartment_led((int)COMPARTMENT_1+tindex , FLASH2HZ); /* Flash LED */
        if (print_flag)                     /* ASCII update cycle? */
        {
          xprintf( 116, DUMMY );
        }
      }
      break;

      case P_SHORT:
      {
        all_states = FALSE;                 /* At least one probe/channel not dry */
        set_compartment_led((int)COMPARTMENT_1+tindex , FLASH2HZ); /* Flash LED */
        if (print_flag)                     /* ASCII update cycle? */
        {
          xprintf( 216, DUMMY );
        }
      }
      break;
      default:
      break;
    }
  }
  /* An alternative method of doing the above is in alternate.c. The 5-wire wet to dry to 
      a different wet needs a fix. */
  if (all_states && print_flag)
  {
    /********** DEBUG STATES *****************/
    xprintf( 32, DUMMY );
    /********** DEBUG STATES *****************/
  }
} /* end of report_tank_state() */

/*************************************************************************
 *  subroutine:      dry_probes()
 *
 *  function:
 *
 *         1.  Reset the 2-wire  probe types to P_DRY if not Shorted/Grounded
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/
void      dry_probes()
{
  unsigned char  index;

  for( index=start_point; index<(MAX_CHAN); index++ )       /* Dry the probes */
  {
    if (probes_state[index] < P_FAULT)
    {
      probes_state[index] = P_DRY;
    }
  }
  for(; index<(2*MAX_CHAN); index++ )   /* No probes */
  {
      probes_state[index] = P_UNKNOWN;
  }
  return;
} /* end of dry_probes */

// >>> QCCC 53, 58, FogBugz 137 
/*************************************************************************
 *  subroutine:      dry_5W_probes()
 *
 *  function:
 *          Routine added to initialize the probes_state and probe_type arrays to correctly indicate
 *          the types and statuses of 5-wire probes for reporting purposes.
 *         1.  Reset the probe types to P_DRY up to number_of_Probes then set to P_NONE
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/
void      dry_5W_probes(void)
{
  unsigned char  index;

  for( index=0; index<number_of_Probes; index++ )  /* Dry the probes */
  {
      probe_type[index] = P_OPTIC5;
      probes_state[index] = P_DRY;
  }
  for( ; index<2*MAX_CHAN; index++ )  /* Dry the probes */
  {
     probes_state[index] = P_UNKNOWN;
     probe_type[index] = P_NONE;
  }
  return;
} /* end of dry_5W_probes */
// <<< QCCC 53, 58, FogBugz 137  

/*************************************************************************
 *  subroutine:      unknown_probes()
 *
 *  function:
 *
 *         1.  Reset the probe states to P_UNKNOWN
 *         2.  Set high current control ON to allow thermistor sensors to pulse
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/
void    unknown_probes(void)
{
   unsigned char  index;
  // last_routine = 0x86;
// >>> QCCC 53
   for( index=0; index<(MAX_CHAN); index++ )   /* initialize them for now */
   {
      probes_state[index]  = P_UNKNOWN;
      probe_type[index]    = P_NO_TYPE;
// >>> QCCC 53
      if ((enable_jumpers & ENA_VAPOR_FLOW) == 0)
      {
        HighI_On(index);                              /* High current on */
      }
      else
      {
        HighI_Off(index);                             /* High current off */
      }
// <<< QCCC 53
    }
   for( ; index<(2*MAX_CHAN); index++ )          /* continue with 8-15 */
   {
// <<< QCCC 53
      probes_state[index]  = P_UNKNOWN;
   }
} /* end of unknown_probes */

/*************************************************************************
 *  subroutine:      probe_leds_off()
 *
 *  function:
 *
 *         1.  Shut off the probe leds
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/
void    probe_leds_off(void)
{
int index, max_display_channel;

  // last_routine = 0x87;
  if (new_front_panel == TRUE)
    max_display_channel = MAX_CHAN_NEW;
  else
    max_display_channel = MAX_CHAN;

  for( index=0; index<max_display_channel; index++ )   /* shut them off for now */
  {
    set_compartment_led((int)COMPARTMENT_1 + index, DARK);  /* Wet channel, turn on LED */
  }
  return;
} /* end of probe_leds_off */

/************************* end of dumfile.c **********************************/
