/*******************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         trukstat.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *  Description:  Functions called from Main loop for Rack controller main
 *                microprocessor - PIC24HJ256GP210. Truck status processing routines.
 *
 * Revision History:
 *   Rev      Date    Who   Description of Change Made
 * -------  --------  ---  --------------------------------------------
 *  1.5.23  05/02/12  KLL  Remove the idle check. So is_truck_pulsing() is 
 *                          called all the time in main_activity() ACQUIRE state.
 *                         In idle state replaced CheckGroundPresent() call to 
 *                           test_gnd_idle(). Added gnd_retry = 0 when leaving
 *                           idle state.
 *          05/02/12  KLL  Added call xprintf 47 to clear print flags to let some
 *                          masked out messages to print
 *                         In GONE case added back the call to AllShortTest()
 *  1.5.26  10/09/12  KLL  Changed the CRC storage location to 0x2ABF0 which is 
 *                          a start of the last page in program memory. With 
 *                          security on writing to the last program address 
 *                          causes address traps because the Microchip built ins
 *                          that allow the writing into program memory actually
 *                          go beyond the address being updating.
 *  1.5.27  03/24/14  DHP  Reduce deadman switch response check from 1.1s to 0.2s.
 *                         Fixed "reset" problem with resistive ground, seen in EU.
 *                         Set TIM variables to default values in truck_gone()
 *                         Removed lines of commented out code
 *  1.5.30  08/10/14  DHP  Removed from truck_idle() vapor probe check and unused 
 *                          cases and renumbered remaining cases
 *                         Changed badgdflag and LED handling to reflect auto 
 *                          detect and use of vapor LED as ground bolt LED
 *                         Changed code to handle Intellitrol2 defaults
 *                         Removed all code that handled vapor
 *                         In truck_idle() with no truck detected, added VIP 
 *                          enabled prior to Read_Truck_Presence()
 *  1.6.34  08/05/16  DHP  FogBugz 108: In which_probe_type() changed acquire
 *                          time from 1.5 seconds to 5 seconds for 5-wire to
 *                          allow for consistent detection of wet 5-wire connection.
 *                          In truck_idle() added cycle_timeout and dry_once
 *                          flag and call to unknown_probes().
 *                          Reversed sense of badgndflag check in ground_detect().
 *                         In which_probe_type() added probe_time variable for
 *                          2-wire and moved setting of acquire_start to only
 *                          occur for 5-wire.  Added setting of compartment LED
 *                          on likely non-permitting 5-wire in OPTIC5 case.
 *                         In main_activity() changed setting of truck_state
 *                          from THERMAL_TWO to OPTIC_TWO in GONE case. Added an
 *                          else clause to force cycling of checks for 5-wire.
 *                         Added check_channels() to aid in determining if truck
 *                          has disconnected.
 *                         FogBugz 136: To aid in debug added messages in 
 *                          truck_idle() to indicate what flagged a connection.
 *  1.6.35  02/01/17  DHP  FogBugz 146: Implement an active deadman function.
 *                          In truck_idle() removed deadman checks when no truck
 *                           detected and added call to deadman_init() when truck
 *                           was detected.
 *                          In truck_active() separated the diagnostic terminal
 *                          update and deadman check so that the deadman is
 *                          checked every 250ms and the update is 5 seconds.
 *                         In truck_idle() replaced Permit_OK = FALSE with
 *                          reset_bypass() which had been called from the sensor
 *                          setup routines.  This was causing a conflict (race
 *                          condition when attempting to bypass a faulty 2-wire.
 *           03/17/17      In truck_active() removed handling of deadman changes
 *                          and ensuing flagging for backup processor.  This is
 *                          now handled in permit_bypass().
 ******************************************************************************/
#include "common.h"
#include "volts.h"   /* A/D voltage definitions */
#include "diag.h"    /* diagnostics() definitions */
#include "tim_utl.h"

static const char TRUCK_TESTER_SERIAL_NO[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
static void truck_here(char truck);
static unsigned long acquire_start;
void deadman_init(void);

//static unsigned char TIM_info_logged = 0;

/****************************************************************************/

/******************************* 9/10/2009 10:53AM ***************************
 * This routine will test if the number of compartments stored in a Super TIM
 * are the same as the number of probes found in the truck
 *****************************************************************************/
#ifdef SUPERTIM
static int compartment_check()
{
unsigned int index;

  if ((number_of_Compartments > 0) && (number_of_Compartments < 17) &&
                            ((number_of_Probes > 0) && (number_of_Probes < 17)))
  {
    if (number_of_Compartments != number_of_Probes)
    {
      /* Is this first time through the five wire loop */
      if ((tank_state == T_DRY) || (truck_state == THERMAL_TWO) ||
      (truck_state == OPTIC_TWO))
      {
        if ( truck_first_time )
        {
          truck_first_time = FALSE;
          printf("\n\rTruck compartments(%u) do not agree with Sensor count(%u)\n\r",
          number_of_Compartments, number_of_Probes);
          compartment_time = mstimer;
        }
        if (DeltaMsTimer(compartment_time) < SEC2)  /* Make sure it is at LEAST 1 sec. */
        {
          ledstate[OPTIC_OUT] = LITE;
          ledstate[OPTIC_IN] = LITE;
          for ( index=0; index < number_of_Compartments; index++)
          {
            probes_state[index] = P_DRY;  /* Start again */
          }
          for ( index=0; index < number_of_Probes; index++)
          {
            probes_state[index] = P_SHORT;  /* Indicate probe problem */
          }
        } else
        {
          if (DeltaMsTimer(compartment_time) < SEC4)  /* Make sure it is at LEAST 1 sec. */
          {
            ledstate[OPTIC_OUT] = DARK;
            ledstate[OPTIC_IN] = DARK;
            for ( index=0; index < number_of_Probes; index++)
            {
              probes_state[index] = P_DRY;  /* Start again */
            }
            for ( index=0; index < number_of_Compartments; index++)
            {
              probes_state[index] = P_SHORT;  /* Indicate probe problem */
            }
          } else
          {
            compartment_time = mstimer;
          }
        }
      }
      StatusB |= STSB_TRUCK;  /* This truck is not valid */
      return TRUE;
    }
    else
    {
      if ( truck_first_time )
      {
        truck_first_time = FALSE;
        printf("\n\rTruck compartments(%u) AGREE with Sensor count(%u)\n\r",
        number_of_Compartments, number_of_Probes);
      }
    }
  }
  return FALSE;
}
#endif
/********************************************************************
 *
 *  FILENAME:       truck_acquire_TIM()
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:    Read Truck Identification Module (DALLAS)
 *
 *  RETURNS:
 *
 ********************************************************************/

static void truck_acquire_TIM (void)
{
  char sts =0;
  char retry = 0;

  // last_routine = 0x3F;
  if (!(StatusA & STSA_TRK_TALK))     /* Negotiated yet? */
  {                                   /* No */
  /* In theory, we can only get here (read the TIM) while not permissive,
     so we don't have any "time/responsiveness" issues to worry
     about. */

    clear_gcheck ();                /* Test Code to turn off g_check so TIM can be read */
    TIM_state = 0;                  /* Just to make sure */
    while ((!sts) && retry++ < 5)
    {
      sts = Read_Truck_SN ();         /* Try to read the Truck's TIM */
  // last_routine = 0x3F;
      if ( sts)
      {
        break;
      }
    }
    if (sts)                        /* Was a TIM successfully read? */
    {                               /* Yes */
      TIM_state = 1;               /* We have a TIM on the truck */
      StatusA |= STSA_TRK_TALK;    /* Comm with truck established */
    }
  }
} /* End truck_acquire_TIM() */

/*************************************************************************
 *  subroutine:     truck_idle()
 *
 *  function:       Check for truck being connected to Intellitrol.
 *                  Determine the type of the probes
 *
 *  input:  none
 *  output: none
 *
 *
 *************************************************************************/

static void ground_detect(void);
static void truck_validate (void);

/* CRC the Kernel and Shell 2KB at a shot */

static void truck_idle(void)
{  /* Repeat until the voltage drops to hint at a truck arrival */

static unsigned idlediag = 0;     /* IDLE diag/etc. timer */
static unsigned idlediagsec = 0;  /* IDLE diag/etc. timer */
static unsigned long icrc_ptr;    /* IDLE diag/CRC pointer */
static unsigned icrc_val;         /* IDLE diag/CRC value holder */

#ifndef __DEBUG            /* These tests interfere with REAL ICE so don't run */
uReg32 read_data;
unsigned long end_address;
#endif
                                  /* F0 Ground Diode HW TRUE */
int index;
char status;
char bypass_state;
char point;
int tindex;                       /* counter */
// >>> FogBugz 136
char printed;

  printed = FALSE;
// >>> FogBugz 136
  // last_routine = 0x40;
  if ((SysParm.Ena_Debug_Func_1 == 0x10) || (SysParm.Ena_Debug_Func_2 == 0x10)
        || (SysParm.Ena_Debug_Func_3 == 0x10) || (SysParm.Ena_Debug_Func_4 == 0x10))
      debug_pulse(0x10);
  ops_ADC( OFF );    /* Shut OFF the 1 mS interrupt (T3) */
  // last_routine = 0x40;

  no_relay_flag = FALSE;
  probe_retry_count = 0;
  badvipflag &= ~BVF_DONE;
  number_of_Probes = 0;  /* No truck no 5 Wire Probes */
  number_of_Compartments = 0;
  truck_first_time = TRUE;
  print_once_msg &= ~UN_AUTH;   /* Clear all entries in idle loop */
  TIM_fault_logged = 0;
  TIM_info_logged = 0;
  log_data_state = 0;
//  bad_compartment_count = 0;
  StatusB &= ~STSB_TRUCK;  /* Clear truck valid */
  /* If we have any voltage-related problems, try reinitializing (one might
     almost call it "re-calibrating") and see if the problem has gone away.

     Note 1:  The truck *MUST* be disconnected for this to work (connected
      truck will drag down our "open circuit reference" and fail the
      diagnostic checks).

     Note 2:  The STSB_ERR_VOLTAGE flag will not clear until the next time
      through top-level loop service (check_status() gets called). */

  if (StatusB & STSB_ERR_VOLTAGE)     /* Any problems with channels/etc. */
  {                                   /* Yes, see if they've cleared up */
      /* Institute a small lag time here. We don't want to toggle the
         lines more than once a second or so -- don't want to trick
         the backup processor to think it's got "good oscillations"! */

     if (DeltaMsTimer(idlediag) > SEC1)  /* Make sure it is at LEAST 1 sec. */
                                         /* since last time here */
     {                                   /* Yes - time to check now */
       idlediag = mstimer;               /* Reset "timer" */
       diagnostics ((DIA_CHK_VOLTS0      /* Re-check rail/etc levels */
                     | DIA_CHK_VOLTS1    /* see POD.C */
                     | DIA_CHK_VOLTSCH  /* Rest of the voltages */
                     | DIA_CHK_GROUND)); /* Re-check ground */
       if (iambroke == 0)
       {
         print_once_msg &= ~QUEDED_RESET_MSG;  /* Reset Message Flag */
       }
     }

      /* Don't even think of doing truck-related stuff with bad voltages! */
     return;   /* Re-Check and try again a bit later */
  }

  /* NOTE: this is a fail-safe for main loop */
  jump_time = 1;                      /* Ensure +20 "Jump-Start" timer active */
  probe_try_state = NO_TYPE;          /* No 5-wire/etc. shenanigans */

  status = JUMP_START;        /* JUMP_START currently enabled ? */
  set_porte (PULSE_TEST);             /* Drive channels (inc. JUMP_START) */
  // last_routine = 0x40;
  if (!status)                        /* If first time here, (JUMP_START on) */
  {
     DelayUS(1000);                  /*  allow JUMP_START's +20 to ramp up */
  }
  status = FALSE;                     /* Assume no truck - will be reset later */
                                      /* if truck connects */
  xprintf( 15, DUMMY );

  /* All channels now have +20 always... */

  if (ConfigA & CFGA_8COMPARTMENT)
  {
     point = 0;
  }
  else
  {
     point = 2;
  }
  if (read_ADC() == FAILED)              /* see what the probes are doing now */
  {
    return ;
  }
  if ( start_idle)
  {
    start_idle = 0;     /* Only print the probe voltage once during idle */
  }

  // last_routine = 0x40;

  /* Check all channels for a likely-looking voltage drop. If any channel(s)
     seems to have activity of some sort, assume a truck has just connected
     and try to figure out what it is.

     Oscillating thermistors and/or 2-wire optics are trivial, channels will
     drop to a couple of volts. Even really cold thermistors will eventually
     warm up and trigger the voltage-drop-detect below.

     Generally, 5-wire optic probes (real probes) will drag channel 8 down
     (power-draw), and/or drag channel 6 (optic-return) down, so they are
     easy to detect (without even oscillating!).

     The SD-O Optic 5-wire dummy however has no appreciable power draw when
     connected, and by itself will not trigger us into acquire based solely
     on a voltage drop. Similarly, a "wet onboarder" has no statistically-
     significant voltage drop to detect. In fact, the measured channel 
     voltages are often *HIGHER* with a wet onboarder (or SD-O dummy) unit 
     connected!!! This is because on power-up when the initial "rail" voltages
     are measured, "the diodes are cold"; later when "the diodes are warmed
     up", the voltages easily rise by 50-100 millivolts or more, giving the
     apparent contradiction of voltages rising as you plug in an onboarder.*/

  for (index=point; index<MAX_CHAN; index++)
  {
     if (ConfigA & CFGA_EURO8VOLT)                      /* "European 10 Volt" jumper?? */
     {                                                  /* Yes, limited voltages... */
        if ((probe_volt[index] > ADC1V)                 /* If current channel NOT grounded */
              && ((probe_volt[index] + ADC1V)           /* but it does show */
                   < open_c_volt[1][index]))            /* a 250mV voltage "drop" */
        {
           status = 1;                               /* Assume a truck has connected */
// >>> FogBugz 136
           if ( printed == FALSE)
           {
              printf("Connection detected; channel %x voltage: %d, open voltage: %d \n\r",
                        index+1, (int)probe_volt[index],  (int)open_c_volt[1][index]);
              printed = TRUE;
           }
// >>> FogBugz 136
           badgndflag &= 0xFE;                          /* Clear Ground Fault so _acquire */
                                                        /* will not trip (via main loop) */
                                                        /* Ground LED */
        }
     }
     else                                               /* IF, "US" 20 Volt Jump Start Present */
     {
        if ((probe_volt[index] > ADC1V)                 /* If current channel NOT grounded */
              && ((probe_volt[index] + ADC4V)           /* but it does show */
                  < open_c_volt[1][index]))             /* a 4 volt voltage "drop" */
        {
           status = 2;                               /* Assume a truck has connected */
// >>> FogBugz 136
           if ( printed == FALSE)
           {
              printf("Connection detected; channel (1-8) %x voltage: %d, open voltage: %d \n\r",
                        index+1, (int)probe_volt[index], (int)open_c_volt[1][index]);
              printed = TRUE;
           }
// <<< FogBugz 136
           badgndflag &= 0xFE;                          /* Clear Ground Fault so _acquire */
                                                        /* will not trip (via main loop) */
                                                        /* Ground LED */
        }
     }
  }    /* End of for (index=point */
  if (!status)                                  /* If truck is not detected, yet */
  {
#if 0
     if (SysParm.EnaFeatures & ENA_DEADMAN)   /* Is DEADMAN enabled ? */
     {                                        /* Yes, Analyze */
       baddeadman = deadman_ops(baddeadman);
       if (baddeadman == OPEN)
       {
         StatusA &= ~STSA_DED_CLOSED;
         set_new_led(DEADMAN_BAD, LITE);      /* Then flash DEADMAN LED */
         set_new_led(DEADMAN_GOOD, DARK);     /* Then flash DEADMAN LED */
         if ( deadman_voltage > ADC2V)
         {
            xprintf( 136, deadman_voltage);      /* Deadman open */
         }
       }
       else
       {
         StatusA |= STSA_DED_CLOSED;
         set_new_led(DEADMAN_BAD, DARK);      /* Then flash DEADMAN LED */
         set_new_led(DEADMAN_GOOD, LITE);     /* Then flash DEADMAN LED */
         if ( deadman_voltage < ADC2V)
         {
            xprintf( 137, deadman_voltage);      /* Deadman closed */
         }
       }
     }
     else
     {                                      /* No, default to everything is fine */
       baddeadman = FALSE;
     }
#endif
     if ((SysParm.Ena_Debug_Func_1 == 0x11) || (SysParm.Ena_Debug_Func_2 == 0x11)
          || (SysParm.Ena_Debug_Func_3 == 0x11) || (SysParm.Ena_Debug_Func_4 == 0x11))
        debug_pulse(0x11);

     if (SysParm.EnaFeatures & ENA_VIP)   /* Do we care about VIP/authorization? */
     {                                    /* Yes */
       if (Read_Truck_Presence() == 1)            /* And look for an active TIM */
                                                /* the Reset/Presence pulse ONLY */
       {
          // printf("\n\r***** 03 ******\n\r");
         status = 3;
// >>> FogBugz 136
         if ( printed == FALSE)
         {
            printf("Connection detected; 1-wire device (TIM) found \n\r");
            printed = TRUE;
         }
// <<< FogBugz 136
         badgndflag &= 0xFE;                       /* Clear Ground Fault so _acquire */
                                                  /* will not trip (via main loop) */
                                                  /* Ground LED */
        }
      }
     // last_routine = 0x40;
     if (!status)
     {
        if ((SysParm.Ena_Debug_Func_1 == 0x12) || (SysParm.Ena_Debug_Func_2 == 0x12)
             || (SysParm.Ena_Debug_Func_3 == 0x12) || (SysParm.Ena_Debug_Func_4 == 0x12))
        {
          debug_pulse(0x12);
        }
        badgndflag = test_gnd_idle();   /* Test if we can see ground bolt */
        // last_routine = 0x40;
        if (badgndflag == GND_OK)                  /* If "Good" (00) ground, */
        {
          // printf("\n\r***** 04 ******\n\r");
          status = 4;                          /* Assume a truck has connected */
// >>> FogBugz 136
          if ( printed == FALSE)
          {
              printf("Connection detected; Pin 9 ground found \n\r");
              printed = TRUE;
           }
// <<< FogBugz 136
        }
     }
  }

  /* Once-a-Second IDLE stuff */
  if ((status == 0)                                              /* If no truck... */
      && (DeltaMsTimer(idlediag) > SEC1))                  /* and it's been "awhile" */
  {
     idlediagsec++;

    /* Periodically, run "special" checks in truckless IDLE loop. Some
    special checks are run every time (TIM detect, for example), while
    others (checking for shorts) are "spread out" (temporally-speaking)
    to both minimize their confusing the backup uP, as well as to minimize 
    the window between last "truck detect" test and having a truck
    connect which fails the special test (e.g., rail voltage check, for
    which a "failure" puts us in "fault" mode and we won't permit!)

    Notes: This segment is now (23-Jun-95) consistently running at about
       257 - 261 Ms, with the short_6to4_5to3() routine being 220 of
       that; try_five_wire() running about 17, and StaticShortTest()
       running 16 - 17 Ms. (Don't have new timings for diagnostics()
       additions... -RDH 31-Jul-95)

               This is still *way* too long for "real time"  usage, but should
       be substantially under the ModBus timeout response time (there
       was a bug wherein this loop was taking 1350Ms!!!). As such,
       this code should be "sub-stated" -- in particular, this involves
       'sub-sub-stating" short_6to4_5to3() (and StaticShortTest(), as
       it *could* take substantial elapsed times) in order to keep the
       main IDLE loop down to "a dozen or so" Ms cycle time . . . -RDH
       */

    /* Perform the more time-intensive "IDLE" diagnostics, one per cycle
    to minimize the window-of-opportunity for a truck to connect and
    falsely fail the self-test/diagnostics */
    {
       switch (sub_state)             /* One "action" per diag cycle */
       {
          case 0:                     /* Check for OnBoarder short pattern */
             if (short_6to4_5to3() )   /*  3,5,8 & 4,6,7 */
             {
                // printf("\n\r***** 05 ******\n\r");
                status = 5;        /* Assume a truck is detected */
// >>> FogBugz 136
                 if ( printed == FALSE)
                 {
                    printf("Connection detected; IntelliCheck 2-wire short pattern found \n\r");
                    printed = TRUE;
                 }
// <<< FogBugz 136
                badgndflag &= 0xFE;  /* Clear Fault so _acquire will not trip Ground LED (via main loop) */
             }
             else
             {
               sub_state++;             /* Advance for next cycle if no short pattern */
             } 
             // last_routine = 0x40;
          break;

          case 1:                     /* Check for shorted channels */

             if (StaticShortTest(FALSE) != 0) /* Are all channels non-shorted */
             {
                status = FALSE;
             }
             // last_routine = 0x40;
             sub_state++;             /* Advance to next test in next cycle */
          break;

          case 2:                        /* Periodic "Self-Check" diagnostics */
       /* Run periodic "diagnostic scan" in IDLE loop. Since status is NULL
       (no truck connected), if the diags find a problem, it will be
       flagged by check_status() before next call here (truck_idle()) --
       we won't try to go into truck_acquire(). */

             if (idlediagsec < 300)      /* Time for "5 minute" diags? */
                sub_state = 6;              /* Not yet time for 5-min diags */
             else                                /* It's time to run 5-min self-test diags */
                {
                   idlediagsec = 0;         /* Reset Diag trigger */
                   xprintf (131, 0);         /* Announce our intentions */
                   sub_state++;             /* Step through 5-minute IDLE self-check */
                 }
          break;

          case 3:                        /* 5-minute IDLE Volts/etc. check */
       /* If a truck should connect just exactly *NOW*, the voltage diags
       fail (truck/probes will drag down the channel voltages). Since
       it is extremely obnoxious to "fault" the unit when a truck connects,
       run the diags much less frequently than shorts-checking, etc. */

             diagnostics (DIA_CHK_IDLE);/* Run periodic diagnostics */
             // last_routine = 0x40;
             if (iambroke & VOLTS_FAULT)   /* Any voltage problems detected? */
                logvolterr();              /* Yes, Log it in Event Log */
             sub_state++;                  /* Advance to next test in next cycle */
          break;

          case 4:                          /* Build IDLE ShellCRC-16 value */
             icrc_ptr = SHELL_START;       /* Init CRC-16 pointer */
             icrc_val = INIT_CRC_SEED;     /* Init CRC-16 value */
             sub_state++;                  /* Advance to next test in next cycle */
          break;

          case 5:                        /* Build IDLE Shell CRC-16 value */
       /* We run 2KB chunks which takes us about a minute to accumulate a full Shell CRC-16 value.

          *** Developers Take Note ***

           Remember folks, putting a breakpoint anywhere in the
           code will cause the following-calculated CRC-16 to
           change! If you want to leave long-term breakpoints in-
           stalled, make sure the DEBUG jumper is in place.
          */

#ifndef  __DEBUG          /* These tests interfere with debugger so don't run */
              end_address = __builtin_tbladdress(&_PROGRAM_END);
              if (icrc_ptr < (end_address - 0x1000))      /* Still lots to do? */
              {                                       /* Yes */
                 icrc_val = program_memory_CRC ((unsigned long)icrc_ptr, 0x1000, icrc_val);
                 icrc_ptr += 0x1000;      /* Advance pointer for next chunk */
              }
              else                        /* Last chunk of Shell to check */
              {
                 icrc_val = program_memory_CRC (icrc_ptr,
                                  (unsigned)((end_address - icrc_ptr) + 1),
                                              icrc_val);   /* Calculate final CRC-16 */
                 ShellCRCval = icrc_val;                   /* Save last "Real" Shell CRC-16 */

                 (void)_memcpy_p2d24((char *)&read_data.Val32, CHECKSUM_LOW_ADDR, 3);

                 Good_Shell_CRC_val = read_data.Word.LW;
                 if ((icrc_val == read_data.Word.LW)           /* Shell's CRC-16 valid? */
                            || (DEBUG_IN == 0)) /* Or DEBUG jumper in ? */
                 {                                         /* Yes */
                    StatusB &= ~STSB_CRC_SHELL;            /* Clear the error flag */
                    xprintf (60, icrc_val);
                 }
                 else                                      /* Invalid CRC Value  */
                 {
                    if ((StatusB & STSB_CRC_SHELL) == 0)   /* First time? */
                    {                                      /* Yes, note it */
                       logcrcerr ();                       /* Event-Log this error */
                       StatusB |= STSB_CRC_SHELL;          /* Flag it in "error" reg */
                    }
                    xprintf (61, icrc_val);
                 }
                sub_state++;            /* Advance to next test in next cycle */
              }
#else
             diagnostics (DIA_CHK_FLASHCRC);
             sub_state++;            /* Advance to next test in next cycle */
#endif
           break;
           case 6:                       /* Check relay states */
              (void)bak_relay_state (FALSE);    /* Check the backup relay is OPEN */
              // last_routine = 0x40;
              (void)main_relay_state (FALSE);   /* Check the main relay is OPEN */
              // last_routine = 0x40;
              /************************ 4/14/2009 2:16PM *********************
               * Test
               ***************************************************************/
              if ((BackRelaySt & RELAY_SHORTED)    /* Don't allow permit if hot wired */
                        && (MainRelaySt & RELAY_SHORTED))    /* (i.e., both relays "shorted") */
              {
                 iamsuffering |= HARD_WIRE_FAULT;
                 xprintf( 18,DUMMY );
                 set_nonpermit(FLASH4HZ);
                 status = FALSE;
                 logrelayerr ();          /* Log horrible awfulness */
              }
              else
              {
                 iamsuffering &= ~HARD_WIRE_FAULT;
                 set_nonpermit(LITE);
                 // last_routine = 0x40;
                 if (!(BackRelaySt & (RELAY_SHORTED | RELAY_BROKEN))
                                 || (MainRelaySt & (RELAY_SHORTED | RELAY_BROKEN)))
                 {
                    StatusB &= ~STSB_BAD_RELAY;    /* All relays appear OK */
                    xprintf( 19, DUMMY );    /* operational relays */
                 }
                 sub_state++;                /* Advance for next test in next cycle */
              }
           break;

           case 7:                       /* Periodically update our TOD clock */
              (void)Read_Clock ();              /* Read Dallas TOD clock */
              // last_routine = 0x40;
              UNIX_to_Greg ();            /* Convert time to standard */
              // last_routine = 0x40;
              sub_state = 0;              /* Reset diagnostic counter */
           break;

           default:
              sub_state = 0;
           break;
        } /* End switch on sub_state */
        if (!status)  /*keep looking only if nothing already found */
        {
           probe_try_state = OPTIC5;    /* Check for an active Dry OnBoarder 5 wire optic socket */
           set_porte(OPTIC_DRIVE);      /* 5 wire optic pulse */
           // last_routine = 0x40;
           DelayMS (20);                /* Time for Channel 4/6 to settle */
           if((status |= try_five_wire()) == TRUE )        /* questioned above */
           {
              badgndflag &= 0xFE;  /* Clear Fault so _acquire will not trip Ground LED (via main loop) */
           }
           set_porte(PULSE_TEST);        /* Drive channels (inc. JUMP_START) */
           // last_routine = 0x40;
           probe_try_state = NO_TYPE;    /* No 5-wire/etc. shenanigans */
        }
    }   /* End if !status sub-stated IDLE diag select */

    /* necessary for IDLE tank flash sequence */
    for (index=point,tindex=0; index<(MAX_CHAN); index++,tindex++)
    {
        if (probes_state[index] >= P_FAULT) /* Fault/Ground/Short? */
        {
           ledstate[LedAddr[tindex]] = FLASH2HZ;    /* 2 Hz flash */
        }
        else if (probes_state[index] == P_OPEN)
        {
           ledstate[LedAddr[tindex]] = FLASH_5HZ;    /* 5 Hz flash */
        }
        else if ( probes_state[index] == P_WET )
        {
           ledstate[LedAddr[tindex]] = LITE;         /* State ON */
        }
        else
        {
           ledstate[LedAddr[tindex]] = DARK;         /* State OFF*/
        }
    }
    /* Or we have an active bypass key that's NOT hot wired */
    if ((bypass_state = bypass_operation()) != 0)
    {
      if( (StatusA & STSA_FAULT) ) // if in system fault do a reset
      {
          secReset = 1;           /* Force immediate board-level RESET */
//        for(;;);  // let the watchdog reset us         
      }
      if (bypass_state == 2)    /* good bypass key */
      {
        // printf("\n\r***** 07 ******\n\r");
        status = TRUE;
// >>> FogBugz 136
        if ( printed == FALSE)
        {
          printf("Connection not detected; bypass operation started \n\r");
        }
// <<< FogBugz 136
        badgndflag &= 0xFE;  /* Clear Fault so _acquire will not trip Ground LED (via main loop) */
      }
    }
     
    /* Check "IDLE" Ground Bolt/100-Ohm (shouldn't be either!) */
    clear_gcheck ();
    ground_detect();
    idlediag = mstimer;               /* Set for next timer check */
  } /* End periodic IDLE "diagnostic" code */

  /************************* truck here **********************************/

  /* If "status" is TRUE (which is to say non-zero), then something has
     "permuted" one of the channel/probes lines. Assume a truck has connected
     and go into "ACQUIRE" mode to identify the type of truck/
     probes and deal with them accordingly.

     If we have any "severe" errors (we're in a "Fault" condition, with the
     Service LED flashing), then we know that permit_bypass() will *NOT* let
     the unit go permissive, so we can safely "go through all the steps" to
     see if the fault condition clears up (in particular, "Broken" relays
     must try to permit to see if they are no longer broken...). HOWEVER, a
     small class of problems are intercepted here -- if we've got bad voltages
     we can deadlock ourselves (low channel voltage can stick us in acquire/
     wet states forever since we use voltages back to the rail as an indication 
     that the truck has disconnected; if the supply is sagging then
     we never detect the truck as "gone"). "Shorts" are not considered as
     faults (could easily be truck just connected). */

  if((status != 0)               /* We see a truck !!!! */
        && ((StatusB & STSB_NO_ACQUIRE) == 0) /* Don't even think about it */
         && ((StatusA & STSA_FAULT) == 0))   /* KLL 04-14-2009 Don't recognize the truck if we are broke */
  {
    xprintf( 47, DUMMY );          /* Start printing out messages that might have been masked */
    xprintf( 20, DUMMY );             /* ACQUIRE state */
    set_main_state (ACQUIRE);         /* enter the ACQUIRE mode */
    // last_routine = 0x40;
    deadman_init();
    reset_bypass();
    scully_flag = FALSE;           /* default the truck does not have Scully Equipment */
    gnd_retry = 0;
    probe_pulse_old = 0;
    probe_pulse = 0;
    truck_pulsing = FALSE;
    idlediagsec = 0;        /* Clear IDLE 5-minute diag timer */
    freetimer  = 1;         /* reset the timer value */
    tank_time = 1;          /* Setup 5 wire timing */
    drive_time = 1;         /* assure charge pump reset */
    relay_turn_on = 1;      /* reset relay turn on time */
    jump_time  = 1;         /* assure the JUMP_START stays on THROUGH ACQUIRE */
    pulse_timeout = (read_time() + 5000L);  /* Must see pulses with in 5 seconds */
    for (index = 0;index < MAX_CHAN;++index)        /* Preset for positive edge */
    {
     old_probe_volt[index] = SysParm.ADCTHstNV;   /*(SysParm.ADCTmaxNV + SysParm.ADCOmaxNV)/2 - SysParm.ADCTHstNV;*/
    }
    init_permit_relay();              /* Init permit/bypass/etc. */
    /*************************** 2/5/2009 10:06AM ************************
    * Intellitrol must figure the type of probes the truck has within
    * 2 minutes or the Intellitrol will go back to IDLE mode
    *********************************************************************/
    truck_timeout = (read_time() + 120000L);  /*  */
    cycle_timeout = (read_time() + SEC10);     /* FogBugz 108 */
    dry_once = FALSE;                          /* FogBugz 108 */

    /* If TAS/VIPER disable/re=enable VIP while trucks connect/disconnect,
    valid_state/etc. can confuse (refuse-to-"re"-authorize) permit, so
    just always initialize them regardless of SysParm.EnaFeatures
    and ENA_VIP being set *at this moment*.  */

    val_state = 0;                    /* No validation yet */
    badvipdscode = 0;                 /* No DateStamp info either */
    StatusA &= ~(STSA_TRK_TALK | STSA_TRK_VALID); /* Just to make sure... */

    /* 0x00 = No truck; 0xFF = "something" there, just don't know what
    yet (and if ENA_VIP turned off, don't care either). */

    Reset_Truck_SN (0xFF);            /* Obliterate truck serial number */
    // last_routine = 0x40;
    truck_here(TRUE);
    // last_routine = 0x40;
    overfill_count = 0;
    unknown_probes();
    for(index=0; index<8; index++)
    {
      high_3[index]  = 0;
      high_8[index]  = 0;
    } 
  }               /* End of we see a truck !!!! */
} /* end of truck_idle() */

/*************************************************************************
 *  subroutine:     set_main_state
 *
 *  function:       Set the "main_state" state variable;
 *                  Clear "sub-state" variable on state transition
 *                  Keep StatusA up-to-date vis-a-vis main_state.
 *
 *  input:          New "state" (enum main_state values)
 *  output: none
 *
 *
 *************************************************************************/

void set_main_state (MAIN_STATE newstate)
{
  main_state = newstate;          /* Set new "main" state */
  sub_state = 0;                  /* State change clears "sub" state */

  // last_routine = 0x41;
  if (newstate == IDLE)
  {
    StatusA |= STSA_IDLE;           /* We're now "IDLE" */
  }
  else
  {
    if (StatusA & STSA_TRK_VALID)
    {
      StatusA &= ~STSA_IDLE;
      StatusA |= STSA_TRK_VALID;
    }                            /* VIP is a separate test */
    else
    {
      StatusA &= ~STSA_IDLE;      /* We're now "Busy" */
    }
  }

}   /* End of set_main_state() */

/*************************************************************************
 *  subroutine:      ground_detect()
 *
 *  function:   Detect if a Ground is present in the IDLE state
 *              and flash the service and Ground LED if found..
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/
static void ground_detect(void)
{
static unsigned long   diode_timeout=0;    /* 1 hour timeout for Ground found */

  // last_routine = 0x42;
  if ( (diode_timeout > (read_time() + (unsigned long)HOUR)) ||
          (diode_timeout < (unsigned long)HOUR) )
     diode_timeout = (read_time() + (unsigned long)HOUR);

  if (!modbus_addr) /* if modbus address is zero, report ASCII */
  {
     if (badgndflag & GND_NO_TEST)    /* 0x10 */
     {
        xprintf( 8, DUMMY );          /* test not enabled */
     }
     if (badgndflag & GND_SHORTED)    /* 0x20 */
     {
        xprintf( 9, DUMMY );          /* a ground diode line shorted */
     }
     if (badgndflag & GND_HARD_FAULT) /* 0x40 */
     {
        xprintf( 12, DUMMY );       /* a ground diode hardware fault */
     }
     if (badgndflag & GND_FAIL)     /* 0x01 */
     {
        xprintf( 11, DUMMY );       /* A Ground Bolt is not present */
     }
  }

  if (SysParm.EnaFeatures & ENA_GROUND) /* Is Ground-Check enabled? */
  {
    if ((badgndflag & GND_FAIL)       /* Ground not found */
      || (badgndflag & GND_NO_TEST) /* test not enabled */
         || (badgndflag & GND_NO_TRIAL))  /* test not done yet */
    {                                 /* Yes -- desired "IDLE" state */

      ledstate[GND_BAD]  = DARK;                /* no ground error */
      ledstate[GND_GOOD]  = DARK;             /* ground  not there */
      set_new_led(GROUND_GOOD, LITE);    /* Green ground diode on */
      groundiodestate = 0;
    }
    else if (badgndflag != GND_OK)         /* Anything else (like Ground) */
    {                                 /*  is an "IDLE" fault condition */
      groundiodestate |= PRESENT_IDLE;
      ledstate[GND_BAD] = FLASH1HZ;         /* Problems with Ground-Detect */
      ledstate[GND_GOOD] = DARK;              /* Problems with Ground-Detect */
      set_new_led(GROUND_GOOD, DARK);    /* Problems with Ground-Detect */
    }
  } else
  {
    ledstate[GND_BAD]  = DARK;                  /* no ground error */
    ledstate[GND_GOOD]  = DARK;               /* ground  not there */
    set_new_led(GROUND_GOOD, DARK);    /* Green ground off */
  }
} /* end of ground_detect */

/*******************************************************************************
 *  subroutine:      bypass_operation()
 *
 *  function:   Detect if a bypass key is present in the IDLE state
 *              and do the following:
 *
 *         1.   First time seen bypass into 5 wire optic acquire
 *              for OnBoarder
 *         2.   If continuously seen for more than 2 minutes,
 *              HARD WIRED and flash service and don't bypass
 *         3.   If gone away reset conditions
 *
 *  input:  none
 *  output: state  0==NO BYPASS 1=BYPASS 2=OK_to_bypass 3=HARDWIRED
 *
 ******************************************************************************/
char bypass_operation(void)
{
  static  unsigned long  bypass_timeout;   /* 1 minute after hard wire timeout */
  static  unsigned long  bypass_detect = 0;    /* 1 minute till hard wire timeout */
  static  char  bypass_toggle = 0;
  static  char  bypass_permit = 0;
  char          state;

  state = FALSE;
  if ( read_bypass(FALSE) )     /* should there be a bypass key applied */
  {                             /* BUT we do not want the bypass state to change */
    state = 1;
    if (bypass_detect == 0)        /* If 0 (bypass_detect = 0) */
    {
      bypass_detect = (read_time() + MIN1);    /* then we set timeout to 1 minute */
    }
    if (bypass_detect < read_time())
    {                           /* Here when bypass_detect timed OUT */
      bypass_timeout = (read_time() + MIN1);
      bypass_permit = FALSE;
      /* Flag as "Hot-Wired", prohibit further bypass actions */
      bystatus |= (BYS_HOTWIRED | BYS_NONBYPASS);
      xprintf( 14, DUMMY );
    }
    else if (bypass_toggle)
    {                           /* Here when still have time (bypass_detect) */
      bypass_timeout = 0;
      bypass_toggle = FALSE;
      bypass_permit = TRUE;
    }
  }
  else
  {
    bypass_toggle = TRUE;
    if (bypass_timeout < read_time())
    {                           /* Here when run out of time */
      state = 0;
      bypass_detect = 0;
      if (bypass_permit)
      {
        state = 2;
      }
      bypass_permit = FALSE;
      /* Bypass not "Hot-Wired", allow future bypass activity */
      bystatus &= ~(BYS_HOTWIRED | BYS_NONBYPASS);
    }
    else
    {
     state = 3;
    }
  }
  return(state);
} /* end of bypass_operation */

/*************************************************************************
 *  subroutine:      truck_acquire()
 *
 *  function:  This state performs the task of determining the status
 *             of an attached truck.  The routine is multi-state in that
 *             after an initial probe type is determined, the tank state
 *             is configured with either wet, dry, open, or short.
 *             Note that in some cases it is not possible to determine
 *             the difference between 'wet' or 'open'.  Either case will
 *             NOT allow a permit to occur.
 *
 *             This routine will NOT set a tank_state to 'dry'.....
 *             This operation is performed in the ACQUIRE state ONLY....
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/
static void truck_acquire(void)
{
   if ((SysParm.Ena_Debug_Func_1 == 0x20) || (SysParm.Ena_Debug_Func_2 == 0x20)
        || (SysParm.Ena_Debug_Func_3 == 0x20) || (SysParm.Ena_Debug_Func_4 == 0x20))
      debug_pulse(0x20);
  // last_routine = 0x44;
   badgndflag = CheckGroundPresent(TRUE);       /* Test whenever allowed (1/Second) */
   if ((badgndflag == GND_OK) && (SysParm.Ena_GND_Display == 1))
   {
      if ( ledstate[PERMIT] != FLASH_5HZ)
      {
        set_nonpermit(DARK);
        set_permit(FLASH_5HZ);         /* Indicate good ground */
      }
   } else
   {
    if ((badgndflag & GND_NO_TEST) != GND_NO_TEST)
    {
         ledstate[NONPERMIT] = LITE;
         set_permit(DARK);         /* Indicate bad ground */
    }
   }
   switch( acquire_state )
   {
        case IDLE_I:              /* If here (0), we have to determine what type */
                                  /* of probe is being used. The acquire_state */
          which_probe_type();     /* must be set by type of probe. This routine */
                                  /* is cyclic and cannot exceed 20 ms  */
  // last_routine = 0x44;
        break;                    /* (continue the processing on the next pass) */
        case OPTIC_5:             /* Here we have a determined 5 wire optical */
                                  /* probe */
          if(SysParm.EnaSftFeatures & ENA_5_WIRE)
              optic_5_setup();       /* Initialize the variables and set main_state to active */
  // last_routine = 0x44;
        break;
        case OPTIC_2:             /* Here we are looking for a 2 wire probe */
         optic_2_setup();       /* Initialize the variables and set main_state to active */
 // last_routine = 0x44;
        break;
        case THERMAL:             /* When here, we have a hot Thermistor probe */
                                  /* The truck_state is set */
          thermal_setup();       /* Continue in this routine till  */
  // last_routine = 0x44;
        break;                    /* oscillating (or busted) */
        case GONE_NOW:            /* truck has disconnected and probes are gone */
                                  /* The truck_state is set to show a state of UNKNOWN */
          truck_inactive(); 
  // last_routine = 0x44;
        break;
        default:                  /* we should NEVER get here */
           acquire_state = GONE_NOW; /* But if we do... */
        break;
   } /* End switch on acquire_state */

   if (SysParm.EnaFeatures & ENA_VIP)   /* Do we care about VIP/authorization? */
   {                                    /* Yes */

        if (badvipflag == BVF_INIT)       /* Initial Value */
        {
            badvipflag = 0x00;                   /* Clear for following tests */
           if (SysParm.TASWait)              /* Requiring TAS intervention? */
           {                                 /* Yes */
                 badvipflag |= BVF_TASDELAY;    /* Give TAS first refusal */
                 ledstate[VIP_AUTH] = DARK;
                 ledstate[VIP_IDLE] = FLASH1HZ87;   /* Indicate waiting... */
           }
        }
        truck_acquire_TIM ();             /* Try for TIM as needed */
  // last_routine = 0x44;
        truck_validate ();                /* Try to authorize/validate vehicle */
                                          /* The flags set here will cause the LEDs */
                                          /* for VIP to show correct state */
  // last_routine = 0x44;
    }

} /* end of truck_acquire */

/*******************************************************************************
 *  subroutine:      truck_active()
 *
 *  function:  This is the heart of the fail-safe system.  The permit relay
 *             is ONLY turned on if the tank_state is T_DRY...
 *
 *             The bypass flag is a separate route with override functions.
 *
 *  input:  none
 *  output: none
 *  NOTE: The wait no longer than 5 seconds on the timer checks is a carry over
 *    from earlier code. It apparently is needed for the initial setting of the
 *    timers.  Perhaps a state machine here or global variables would be better? 
 *               
 ******************************************************************************/
static void truck_active(void)
{
static  unsigned long   DM_timer = 0;     /* timer for DEADMAN detection */
static  unsigned long   TERM_timer = 0;   /* timer for terminal update */
static  unsigned long   TIM_timer = 0;    /* timer for TIM detection */
static  unsigned long   log_time=0;
unsigned int fault_num;
unsigned char log_stat;
unsigned int i;
  if ((SysParm.Ena_Debug_Func_1 == 0x30) || (SysParm.Ena_Debug_Func_2 == 0x30)
        || (SysParm.Ena_Debug_Func_3 == 0x30) || (SysParm.Ena_Debug_Func_4 == 0x30))
  {
    debug_pulse(0x30);
  }
  badgndflag = CheckGroundPresent(TRUE);       /* Test whenever allowed (1/Second) */
  if ( (TERM_timer < read_time()) ||        /* Do this once every active_timeout period */
         (TERM_timer > (read_time()+SEC5)) ) /* but do not wait longer than 5 seconds */
  {    /* start of do once per active period */
    if (!modbus_addr) /* if Modbus address is zero, report ASCII */
    {
      if (badgndflag & GND_NO_TEST)
      {
        xprintf( 8, DUMMY );          /* test not enabled */
      }
      if (badgndflag & GND_SHORTED)
      {
        xprintf( 9, DUMMY );          /* a ground diode line shorted */
      }
      if (badgndflag & GND_HARD_FAULT)
      {
        xprintf( 12, DUMMY );         /* a ground diode hardware fault */
      }
      if (badgndflag & GND_FAIL)
      {
        xprintf( 11, DUMMY );         /* a ground not found */
        groundiodestate |= PRESENT_ACTIVE;
      }
      if (badgndflag==GND_OK)
      {
        xprintf( 10, DUMMY );         /* a ground present */
      }
      TERM_timer = (read_time() + 5000); /* Reset the timer */
    }    /* End of if MODBUS address = 0 */
  }
  if ((DM_timer < read_time()) ||        /* Do this once every active_timeout period */
         (DM_timer > (read_time()+SEC5))) /* but do not wait longer than 5 seconds */
  {
    if (SysParm.EnaFeatures & ENA_DEADMAN) /* Is DEADMAN enabled ? */
    {                                      /* Yes, Analyze */
      baddeadman = deadman_ops(baddeadman);
      if (baddeadman == TRUE)
      {
        StatusA &= ~STSA_DED_CLOSED;
        xprintf( 136, deadman_voltage);  /* Deadman non-Permit */
        if ((StatusA & STSA_BYPASS))
        {
          Permit_OK = FALSE;
        }
      }
      else
      {
        StatusA |= STSA_DED_CLOSED;
        xprintf( 137, deadman_voltage);  /* Deadman permit */
        if ((StatusA & STSA_BYPASS))
        {
          Permit_OK = TRUE;
        }
      }
    }
    else
    {
      baddeadman = FALSE;                /* No, default to everything is fine */
    }
    DM_timer = (read_time() + 250);      /* Reset the timer */
  }
  if ((TIM_timer < read_time()) ||        /* Execute first time through and 1/3 sec. */
         (TIM_timer > (read_time()+SEC5))) /* but do not wait longer than 5 seconds */
  {    /* start of do once per TIM period */
    if ((SysParm.EnaFeatures & ENA_VIP) && (SysParm.VIPMode == 5))
    {                                      /* Mode 5 we authorize whether or not a TIM exists  */
      if (!(StatusA & STSA_TRK_VALID))    /* This truck need validation? */
      {                                   /* Yes */
        clear_gcheck ();
        DelayUS (70);                    /* Hold the GCHECK pin high */
        val_state = 0;                   /* Try Fresh */
        truck_acquire_TIM();             /* Try for TIM as needed */
        truck_validate();                /* Try to authorize/validate vehicle */
      }
      else
      {
        if (!(StatusA & STSA_TRK_TALK))/* This is for the case where there is no TIM */
        {                                   /* or we do not have a good read of the serial number */
          truck_acquire_TIM();
          truck_validate();
        }
      }
    }
    else
    {
      if (SysParm.EnaFeatures & ENA_VIP)
      /* Do we care about VIP/authorization? */
      {                                    /* Yes */
        clear_gcheck ();
        DelayUS (70);                     /* Hold the GCHECK pin high */
        if ((!(StatusA & STSA_TRK_VALID)) /*|| (SysParm.EnaSftFeatures & ENA_CPT_COUNT)*/ )  /* This truck need validation? */
        {                                 /* Yes */
          val_state = 0;                 /* Try Fresh */
          truck_acquire_TIM ();          /* Try for TIM as needed */
          truck_validate ();             /* Try to authorize/validate vehicle */
        }
        else
        {
          if ((SysParm.Ena_Debug_Func_1 == 0x36) || (SysParm.Ena_Debug_Func_2 == 0x36)
               || (SysParm.Ena_Debug_Func_3 == 0x36) || (SysParm.Ena_Debug_Func_4 == 0x36))
          {
            debug_pulse(0x36);
          }
          if (Read_Truck_Presence() == 0)
          {
            BadVipCntr ++;                    /* when failed */
            if ((SysParm.Ena_Debug_Fail_1 == 0x34) || (SysParm.Ena_Debug_Fail_2 == 0x34)
              || (SysParm.Ena_Debug_Fail_3 == 0x34) || (SysParm.Ena_Debug_Fail_4 == 0x34))
            {
              debug_pulse(0x34);
            }
            if (BadVipCntr > 10)
            {
              StatusA &= ~(STSA_TRK_TALK | STSA_TRK_VALID); /* Just to make sure... */
              badvipflag |= BVF_TIMABSENT;      /* No TIM, set appropriate status bit */
              StatusA &= ~STSA_TRK_VALID;    /* Truck not authorized! Somebody */
                                             /* tried to fool the unit? */
              BadVipCntr = 0;
            }
          }
          else
          {
            BadVipCntr = 0;
          }
        }
      }
      /* WPW
       * Added this section to read TIM without ENA
       * Here the truck_active is called repeatedly
       * as long as the truck is there.
       * If the TIM is not read before, it reads TIM here
       * The STSA_TRK_VALID will be set based on read result
       * the serial number is saved in truck_SN
       */
      else
      {
        if (SysParm.Ena_TIM_Read == 1)
        {
          if (!(StatusA & STSA_TRK_VALID))
          {
            truck_acquire_TIM ();             /* Try for TIM as needed */
          /* If read is OK, Set the valid flag automatically, no check needed */
            StatusO &= ~0x7;     /* Clear VIP output status */
            if (StatusA & STSA_TRK_TALK)
            {
              StatusA |= STSA_TRK_VALID;
              ledstate[VIP_IDLE] = DARK;
              ledstate[VIP_AUTH] = LITE;
              ledstate[VIP_UNAUTH] = DARK;
              StatusO |= STSO_AUTHORIZED;
            }
            else
            {
              ledstate[VIP_IDLE] = DARK;
              ledstate[VIP_AUTH] = DARK;
              ledstate[VIP_UNAUTH] = LITE;
              StatusO |= STSO_UNAUTHORIZED;
            }
            badvipflag = 0;
            BadVipCntr = 0;
          }
          else
          {
            if (Read_Truck_Presence() == 0)      /* And look for an active TIM */
            {                                    /* Read_Truck_Presence() returns 0 */
              BadVipCntr++;
              if (BadVipCntr > 15)
              {
                StatusA &= ~(STSA_TRK_TALK | STSA_TRK_VALID); /* Just to make sure... */
                BadVipCntr = 0;
                StatusA &= ~STSA_TRK_VALID;
              }
            }
            else
            {
              StatusO &= ~0x7;     /* Clear VIP output status */
              ledstate[VIP_IDLE] = DARK;
              ledstate[VIP_AUTH] = LITE;
              ledstate[VIP_UNAUTH] = DARK;
              StatusO |= STSO_AUTHORIZED;
            }
          }
        }
      }
    }

    if( TIM_size == DS28EC20_SIZE) // Super TIM
    {
        if((StatusA & STSA_PERMIT) == STSA_PERMIT )
        {
            if(  TIM_info_logged == 0 )
            {
                if(  (active_comm & (INTELLI | GROUNDIODE)) == 0  )
                {
                    log_stat = TIM_log_info();
                    if( log_stat == 0 )
                    {
                        TIM_info_logged = 1;
                        log_time = (read_time() + MIN1); /* Reset the timer for 1/3 second */
                    }
                }
            }
        
            if(log_time > read_time() )
            {
                if(  (active_comm & (INTELLI | GROUNDIODE)) == 0  )
                {
                    log_date_and_time(log_time_address);
                    log_time = (read_time() + MIN1); /* Reset the timer for 1/3 second */  
                }
            }
        
        }   
        if( TIM_fault_logged == 0 )
        {
            if( ( ((badgndflag & GND_PROBLEMS) != 0) && ((badgndflag & GND_INIT_TRIAL) == 0) ) || (tank_state == T_WET) )
            {
                fault_num = 0;
                if( badgndflag & GND_PROBLEMS )
                {
                    fault_num = 25;
                }
                else
                {                        
                    for(i = 0; i < 16; i++)
                    {
                        if((probes_state[i] != P_UNKNOWN) && (probes_state[i] != P_DRY))
                        {
                            fault_num = i + 1;
                            break;
                        }
                    }
                }
                if( fault_num != 0 )
                {
                    if(  (active_comm & (INTELLI | GROUNDIODE)) == 0  )
                    {
                        TIM_log_fault(fault_num);
                        TIM_fault_logged = 1;
                    }
                }
            }
        }
    }
    
    TIM_timer = (read_time() + 330); /* Reset the timer for 1/3 second */
  }    /* end of "once per TIM period" */

  switch( truck_state )       /* This is time independent, done every iteration */
  {
    case UNKNOWN:             /* If here, we have a MAJOR problem */
      if (!(StatusA & STSA_BYPASS))
       xprintf( 51, DUMMY );
    break;

    case THERMAL_TWO:           /* If here, we have a THERMAL truck */
      xprintf( 46, DUMMY );    /* as determined in thermal_setup */
      if ( read_time() > SEC30 )
      {
        (void)read_bypass(TRUE);    /* Read any bypass chip */
      }
      if ((SysParm.Ena_Debug_Func_1 == 0x31) || (SysParm.Ena_Debug_Func_2 == 0x31)
          || (SysParm.Ena_Debug_Func_3 == 0x31) || (SysParm.Ena_Debug_Func_4 == 0x31))
      {
        debug_pulse(0x31);
      }
      active_two_wire(THERMIS);
      break;

      case OPTIC_TWO:           /* If here, we have a 2 wire optic truck */
        xprintf( 45, DUMMY );  /* as determined in optic_2_setup */
        (void)read_bypass(TRUE);     /* Read any bypass chip */
        if ((SysParm.Ena_Debug_Func_1 == 0x32) || (SysParm.Ena_Debug_Func_2 == 0x32)
           || (SysParm.Ena_Debug_Func_3 == 0x32) || (SysParm.Ena_Debug_Func_4 == 0x32))
        {
          debug_pulse(0x32);
        }
//       (void)compartment_check();   /* Validate sensor count with SuperTIM */
        active_two_wire(OPTIC2);
        break;

      case OPTIC_FIVE:          /* If here, we have a 5 wire optic truck */
      {
        if(!(SysParm.EnaSftFeatures & ENA_5_WIRE))    /* If 5 Wire mode disabled */
        {
          break;
        }
        xprintf( 44, DUMMY );  /* as determined in optic_5_setup */
        (void)read_bypass(TRUE);     /* Read any bypass chip */
        if ((SysParm.Ena_Debug_Func_1 == 0x33) || (SysParm.Ena_Debug_Func_2 == 0x33)
           || (SysParm.Ena_Debug_Func_3 == 0x33) || (SysParm.Ena_Debug_Func_4 == 0x33))
        {
          debug_pulse(0x33);
        }
        /*************************** 5/11/2009 6:52AM **************************
        * The compartment count can be faked out by a wet probe. So test for
        * return pulse. If the pulse does not return the last probe is wet. If
        * it does return all probes are functioning and valid compartment count
        ***********************************************************************/
//        check_compartment_count();
//       (void)compartment_check();
        active_5wire();
      }
      break;

    case DEPARTED:            /* If here, we had a wet timeout on a 5 wire connection */
      if ((SysParm.Ena_Debug_Func_1 == 0x34) || (SysParm.Ena_Debug_Func_2 == 0x34)
          || (SysParm.Ena_Debug_Func_3 == 0x34) || (SysParm.Ena_Debug_Func_4 == 0x34))
      {
        debug_pulse(0x34);
      }
      set_main_state (GONE);
      break;

    case GONE_TWO:            /* If here, we had a wet timeout on a 2 wire connection */
      if ((SysParm.Ena_Debug_Func_1 == 0x35) || (SysParm.Ena_Debug_Func_2 == 0x35)
          || (SysParm.Ena_Debug_Func_3 == 0x35) || (SysParm.Ena_Debug_Func_4 == 0x35))
      {
        debug_pulse(0x35);
      }
      set_main_state (GONE);
      break;

    default:                      /* we should NEVER get here */
      xprintf( 47, 4 );          /* Force the next message to print */
      truck_state = DEPARTED;    /* But if we do... might as well */
      xprintf( 50, DUMMY );      /* get stuck doing gone routine */
      break;
  }
} /* end of truck_active */

/*************************************************************************
 *  subroutine:      truck_gone()
 *
 *  function:
 *
 *         1.  Reset the main_state to IDLE
 *         2.  Reset the tank_state to T_INIT
 *         3.  Reset the acquire_state to IDLE_I
 *         4.  Reset the probe_try_state to NO_TYPE
 *         5.  Reset the truck_state to UNKNOWN
 *         6.  Reset the bypass flag to FALSE
 *         7.  Reset the open_c_volt[]
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/

static unsigned long gone_time;

void    truck_gone(void)
{
unsigned int i;

  // last_routine = 0x46;
  if ((SysParm.Ena_Debug_Func_1 == 0x40) || (SysParm.Ena_Debug_Func_2 == 0x40)
      || (SysParm.Ena_Debug_Func_3 == 0x40) || (SysParm.Ena_Debug_Func_4 == 0x40))
     debug_pulse(0x40);
  gone_time = read_time();            /* Mark departure time */

  if (main_state == GONE)
     xprintf( 35, DUMMY );
  else
     xprintf( 27, DUMMY );

  set_main_state (FINI);

  acquire_state   = IDLE_I;
  probe_try_state = NO_TYPE;
  tank_state      = T_INIT;
  truck_state     = UNKNOWN;
  TIM_state       = 0;
  groundiodestate = 0;
  if(SysParm.EnaSftFeatures & ENA_5_WIRE)    /* If 5 Wire mode enabled */
  {
     optic5_state    = NO5_TEST;
  }
  two_wire_state  = NO_TEST_2W;
  jump_time        = 1;  /* assure proper setup for JUMP_START on */
  ground_time = (read_time() + SEC1);  /* reset timer */
  relay_turn_on    = 0;  /* reset relay turn on time */
  if(SysParm.EnaSftFeatures & ENA_5_WIRE)    /* If 5 Wire mode enabled */
  {
     optic5_timeout   = 0;  /* reset optic5 trial */
  }
  unknown_probes();      /* reset the probe states */

  for (i = 0; i < (int)NLEDBIT; i++)       /* Turn off all LEDs */
     if (i != (unsigned int)DYNACHEK)               /* Except leave "DynaCheck(tm)" alone */
     {
        ledstate[i] = DARK;           /* (The Comm LEDs "PULSE" on) */
     } 
  set_nonpermit(LITE);

  /***************************** 12/30/2008 1:18PM *************************
   * Clear the truck TIM configuration that was saved when the truck first
   * read the TIM when the truck went active.
   *************************************************************************/
  for ( i=0; i<sizeof(Truck_TIM_Configuration); i++)
  {
    Truck_TIM_Configuration[i] = 0;
  }

  number_of_Compartments = 0;
  TIM_size = DEFAULT_SIZE;
  TIM_scratchpad_size = DEFAULT_SCRATCHPAD_SIZE;
  S_TIM_code = FALSE;

  if (SysParm.EnaFeatures & ENA_VIP)  /* Do we care about VIP/authorization? */
  {
    /* See if there is a valid TIM module still connected */

    truck_acquire_TIM ();             /* Try for TIM as needed */
  // last_routine = 0x46;
    truck_validate ();                /* Try to authorize/validate vehicle */
  // last_routine = 0x46;
    if (!(StatusA & STSA_TRK_VALID))  /* No valid TIM out there */
    {
      ledstate[VIP_UNAUTH] = LITE;      /* "IDLE LED" set in "fini" below */
    }

  }
	/*
	 * WPW: Added the following for Exxon TIM read
	 */
  else
  {
    if (SysParm.Ena_TIM_Read == 1)
    {
      truck_acquire_TIM ();             /* Try for TIM as needed */
  // last_routine = 0x46;
		/*
		 * If the read is OK
		 * Set the valid flag automatically, no check needed
		 */
      if (!(StatusA & STSA_TRK_TALK))
      {
        if ((print_once_msg & UN_AUTH) == 0)
        {
          print_once_msg |= UN_AUTH;
          printf("\n\rCan not locate ");
          Report_SN((unsigned char *)&truck_SN[0]);
          printf(" in EEPROM\n\r");
        }
        ledstate[VIP_UNAUTH] = LITE;   /* set UnAuth LED on */
      }
    }
  }

  init_permit_relay();                /* Init ("clear") permit/bypass/etc. */
  // last_routine = 0x46;

  set_porte( PULSE_TEST );            /* assure proper setup for JUMP_START */
  // last_routine = 0x46;

    /* No longer permitting, bypassing, etc. */

  reset_bypass();
  badgndflag |= GND_INIT_TRIAL;
  StatusA &= ~(STSA_PERMIT | STSA_BYPASS | STSA_BYPASSABLE);

  /* Rest of state cleared (and set to "IDLE" in truck_fini() below) */

  xprintf( 128, DUMMY );              /* "\n\r" correction to repeat operation */

  diagnostics (DIA_CHK_TRKGONE);   /* Run "Truck-GONE" diagnostics  */
                                      /* The results will be used in truck_fini */
  // last_routine = 0x46;
  /* Say "Good Bye" to the nice trucker */
  for (i = (unsigned int)COMPARTMENT_1; i < ((unsigned int)COMPARTMENT_1+MAX_CHAN); i++)
  {
     ledstate[i] = LITE;              /* Turn on channel LEDs */
  }
  if ( new_front_panel == TRUE)
  {
    for ( i=(int)COMPARTMENT_9; i<=(int)COMPARTMENT_12; i++)
    {
      ledstate[i] = LITE;
    }
  }
  modbusVIPmode = SysParm.VIPMode;

} /* End of truck_gone() */

/*************************************************************************
 *  subroutine: truck_fini()
 *
 *  function:   Verify truck gone (voltages etc. OK) after truck_gone()
 *              and before returning to IDLE state.
 *
 *              truck_gone() turns all the channel LEDs on to indicate
 *              the gone state; if the truck is still connected
 *              or something is dragging down any of the channels, then
 *              we loop here flashing the channel LEDs. After "a minute",
 *              give up and go IDLE anyways.
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/

void   truck_fini(void)
{
  // last_routine = 0x47;
  if ((SysParm.Ena_Debug_Func_1 == 0x50) || (SysParm.Ena_Debug_Func_2 == 0x50)
      || (SysParm.Ena_Debug_Func_3 == 0x50) || (SysParm.Ena_Debug_Func_4 == 0x50))
     debug_pulse(0x50);

  if ((iambroke & (TOL10V_FAULT | TOL20V_FAULT | NOISE_FAULT | REF_VOLT_FAULT))
         && (gone_time < (read_time() + MIN1)))
  {
  /* Apparently, our disconnected truck is still dangling and drag-
     ging our channel voltages down. Wait here until the offending
     truck disconnects. report_tank_state() will change truck_gone()'s
     channel LEDs from ON to fast blink... */

     diagnostics (DIA_CHK_TRKFINI);  /* Rerun some diagnostics */
                                        /* That will fail the check_ref_volt */
                                        /* test in case INTELLICHEK is connected */
  // last_routine = 0x47;
  }
  else
  {
  /* Apparently the truck truly has disconnected, as our "open" circuit
     channel voltages (etc.) are "normal". We can safely enter
     IDLE state now without falsely faulting on a still-dangling
     truck.

     Alternatively, we've timed out waiting for the blighter, so just
     fault the unit. (Might even be a real fault, who knows?) */

    clear_gcheck ();                /* Test Code to turn off g_check so TIM can be read */
  // last_routine = 0x47;
    probe_leds_off ();              /* Shut off the channel LEDs now */
  // last_routine = 0x47;

    if (SysParm.EnaFeatures & ENA_DEADMAN) /* Is DEADMAN enabled ? */
    {                                      /* Yes, Analyze */
      baddeadman = deadman_ops(baddeadman);
  // last_routine = 0x45;
      if (baddeadman)
      {
        set_new_led(DEADMAN_BAD, LITE);        /* Then flash DEADMAN LED */
        set_new_led(DEADMAN_GOOD, DARK);     /* Then flash DEADMAN LED */
        xprintf( 136, deadman_voltage);  /* Deadman open */
      }
      else
      {
        set_new_led(DEADMAN_BAD, DARK);        /* Then flash DEADMAN LED */
        set_new_led(DEADMAN_GOOD, LITE);     /* Then flash DEADMAN LED */
        xprintf( 137, deadman_voltage);  /* Deadman closed */
      }
    }
     // WPW: added the TIM_Read checking
    if (SysParm.EnaFeatures & ENA_VIP || SysParm.Ena_TIM_Read == 1) /* Do we care about VIP/authorization? */
    {
      StatusO &= ~0x7;     /* Clear VIP output status */
      ledstate[VIP_IDLE] = LITE;  /* Yes, then VIP is IDLE */
      ledstate[VIP_AUTH] = DARK;
      ledstate[VIP_UNAUTH] = DARK;
      StatusO |= STSO_STANDBY;
    }
    Reset_Truck_SN (0x00);          /* Clear truck serial number array */
  // last_routine = 0x47;
    truck_here(FALSE);
  // last_routine = 0x47;
    set_main_state (IDLE);          /* Enter IDLE state */
  // last_routine = 0x47;

  }
  modbusVIPmode = SysParm.VIPMode;  /* Remember the Mode JGS Rev 1.8 */
} /* End truck_fini(); */

/*************************************************************************
 *  subroutine:      main_activity()
 *
 *  function:
 *
 *
 *         1.  Determine the Activity of the test points to see a truck
 *             a. Probe activity (ie voltage drop on lines)
 *             b. TIM
 *             c. Bypass single shot
 *             d. Ground Bolt present (hard-wired)
 *             e. Bypass chip hard-wired
 *         2.  Both Ground bolt and TIM use the TIM dallas channel and
 *             are interrupt derived from a channel monitor.
 *
 *  input:  none
 *  output: none
 *
 *
 *************************************************************************/

void main_activity(void)
{
unsigned char chtemp, index;
  // last_routine = 0x48;
  switch (main_state)
  {
    case IDLE:                /* wait in this main state until the voltage */
              /* drops to hint at a truck arrival */
       truck_idle();
  // last_routine = 0x48;
    break;

    case ACQUIRE:             /* determine the truck_state state and set   */
    {
      truck_acquire();
      is_truck_pulsing();
    }
    break;

    case ACTIVE:              /* Just repeat the watching of the probes */
                              /* till one turns wet, then no-permit, */
    {
       truck_active();        /* main_state will be GONE if no truck */
      is_truck_pulsing();
     }
    break;

    case GONE:                /* this state waits till the truck is gone */
    {
    /* Re-check rail/etc levels & Rest of the voltages */
      diagnostics ((DIA_CHK_CLOCK | DIA_CHK_VOLTS0 | DIA_CHK_VOLTSCH));
  // last_routine = 0x48;
      if (iambroke)                 /* Let's try for ANY fault condition */
      {                             /* Suspect Intellicheck truck */
        chtemp = AllShortTest();   /* Intellicheck shows up as all channels */
        if (chtemp)                /* shorted together when thermistor */
        {                          /* socket is used */
          if ((truck_state == OPTIC_FIVE) || (truck_state == DEPARTED))
          {
            if(SysParm.EnaSftFeatures & ENA_5_WIRE)
            {
              truck_state = OPTIC_FIVE;
            }
            else
            {
               truck_state =OPTIC_TWO;
            }
          }
          set_main_state (ACTIVE);         /* need to keep it WET until it REALLY  DISCONNECTS */
          iambroke = 0;           /* Clear Voltage Error Flags */
          break;
        }
// >>> FogBugz 108
        else
        {
          set_main_state (ACQUIRE);     /* We may have just connected to a non-permitting 5-wire IntelliCheck */
          acquire_state     = IDLE_I;
          probe_try_state = NO_TYPE;
          tank_state          = T_INIT;
          truck_state         = UNKNOWN;
          unknown_probes();                            /* reset the probe states */
          iambroke = 0;           /* Clear Voltage Error Flags */
          for (index = 0; index < 8; index++)       /* Turn off all compartment LEDs */
          {
               ledstate[(signed int)COMPARTMENT_1 + (index)]= DARK;
          } 
        } 
// <<<FogBugz 108
        service_charge();      /* "Pump" Service LED */
        iambroke = 0;          /* Clear Voltage Error Flags */
        break;
      }
      truck_gone();           /* resets the truck_state and tank_state */
  // last_routine = 0x48;
      start_idle = 1;         /* Print probe voltage during idle mode */
      break;                     /* main_state to IDLE */
    }

    case FINI:                 /* The truck has been declared gone; verify */
                                 /* voltages/etc. OK before re-entering IDLE */
    {
      truck_fini();           /* Transit to IDLE if voltages/etc OK */
  // last_routine = 0x48;
      break;
    }

    default:                   /* we should NEVER get here */
    {
      set_main_state (GONE);  /* Empirically, however, I observe that we */
                                 /* do in fact get here, and it's a good */
                                 /* idea in general to handle such cases */
  // last_routine = 0x48;
      break;
    } 
  } /* End switch on main_state */

} /* End of main_activity() */

/*************************************************************************
 *  subroutine:      which_probe_type()
 *
 *  function:  This routine is the main analyzer of the probe function
 *             for a newly attached truck.  The probe source voltage has
 *             been seen to decrease from the established Vcc.  This decrease
 *             has caused a change of state and an attempt to acquire the
 *             probe type will now be made.
 *
 *             The probe voltage are recaptured and anaylized to determine 
 *             if 2-wire sensors are connected; if so then skip the 5-wire.
 *
 *             Optical 5 wire probes are checked first. A pulse is sent
 *             out and the echo is searched for during the next 5ms.
 *
 *             Optical 2 wire probes are checked next.  Oscillations from
 *             this type of probe should begin within 20-40ms.
 *
 *             Shorts and opens are tested for each of these optical probe
 *             types, and if no appearances fit the optical profile,
 *             then we move on to THERMAL probe types as a cold probe.
 *
 *             THERMAL probes are checked for next. ( A delay of up to 2
 *             minutes may occur before oscillation occurs).
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/

void which_probe_type(void)
{
static char   one_try;
unsigned int  result;

  // last_routine = 0x49;
  if(StatusA & STSA_INTELLICHECK)
  {
     probe_try_state = OPTIC2;
     probe_time     = read_time();                                      /* FogBugz 108 */
     one_try = TRUE;
  }
  if( (probe_try_state == NO_TYPE) )
  {
    /* Verify 5-Wire is enabled and 2-wire is not connected */
    if(((SysParm.EnaSftFeatures & ENA_5_WIRE) !=0) && (check_2wire() == FALSE))
    {          
      optic5_state    = NO5_TEST;
      probe_try_state = OPTIC5;
      acquire_start   = (read_time()+(SEC2+MSec500));                     /* Used for 5-wire */
    }
    else
    {
      two_wire_state  = NO_TEST_2W;
      probe_try_state = OPTIC2;
    } 
    probe_time     = read_time(); 
    one_try           = TRUE;
  }
  if(read_time() > truck_timeout)               /* exceeded 2 minutes */
  {
     acquire_state   = GONE_NOW;         /* Timed Out and we do not know why */
     probe_try_state = NO_TYPE;          /* Restart from truck_idle */
  }
  else
  {
    switch( probe_try_state )         /* treat Optics & Thermistor similarly */
    {
      case OPTIC5:                    /* The first to try */
      if (enable_jumpers & ENA_VAPOR_FLOW)
      {
  // >>> QCCC 53
        HighI_On(3);                               /* High current on Pulse out */
        HighI_On(4);                               /* High current on Diagnostic Line */
  // <<< QCCC 53
      }
      if(!(SysParm.EnaSftFeatures & ENA_5_WIRE))    /* If 5 Wire mode disabled */
      {
        probe_try_state = OPTIC2;            /* Try for a two wire optic probe */
        break;
      }
      ops_ADC( OFF );
  // last_routine = 0x49;
      set_porte( OPTIC_DRIVE );    /* assure setup for 5 wire optic pulsing */
  // last_routine = 0x49;
      DelayMS (20);                /* Time for Channel 4/6 to settle */
      if( acquire_start > read_time())    /*First seconds of acquire? */
      {
        if(five_wire_optic() != 0)
        {
          acquire_state = OPTIC_5;       /* set acquire state to 5 wire optic */
        }
      }
      else
      {
        if (bypass_operation() == 2)  /* allow a wet 5 wire OnBoarder to bypass active */
        {
          acquire_state = OPTIC_5;    /* set acquire state to 5 wire optic */
        }
        else
        {
          if((result = (calc_tank()-1)) > (2*MAX_CHAN))
          {
            result = 0;
          }
          probes_state[result] = P_WET; /* show likely probe as wet */
          probe_try_state = OPTIC2;        /* Try for a two wire optic probe */
        }
      }
      break;

      case OPTIC2:                     /* Remember that in check_all_pulses */
// >>> QCCC 53
      if (enable_jumpers & ENA_VAPOR_FLOW)
      {
        HighI_Off(3);                              /* High current off Pulse out */
        HighI_Off(4);                              /* High current off Diagnostic Line */
      }
// <<< QCCC 53
      two_wire_start();             /* we set the try_state - assure setup right */
      if(two_wire_optic() != 0)
      {
// last_routine = 0x49;
        acquire_state = OPTIC_2; /* We might have a 2W */
      }
      else
      {
          if ( one_try )                    /* OPTIC2 one trial short testing */
          {
            one_try = FALSE;
            if (short_6to4_5to3())         /* smart probe two wire ? */
            {
              acquire_state   = OPTIC_2;  /* yes */
            }
// last_routine = 0x49;
          }
      }
      break;

      case THERMIS:
        two_wire_start();       /* assure setup right */
// last_routine = 0x49;
        if(two_wire_thermal() != 0)
        {
          if (acquire_state == IDLE_I)
          {
            acquire_state = THERMAL;
          } 
        }
// last_routine = 0x49;
        break;

      default:
        probe_try_state = NO_TYPE;
        acquire_state   = GONE_NOW;    /* Bomb outta here! */
        break;
    }
  }
} /* end of which_probe_type */

/*************************************************************************
 *  subroutine:      truck_inactive()
 *
 *  function:
 *
 *
 *         1.  Reset the main_state to IDLE
 *         2.  Reset the tank_state to T_INIT
 *         3.  Reset the acquire_state to IDLE_I
 *         4.  Reset the probe_try_state to NO_TYPE
 *         5.  Reset the truck_state to UNKNOWN
 *         6.  Reset the bypass flag to FALSE
 *
 *
 *  input:  none
 *  output: none
 *
 *
 *************************************************************************/

void truck_inactive(void)
{

  // last_routine = 0x4A;
  truck_gone();

} /* end of truck_inactive */

/*************************************************************************
 *  subroutine:      truck_here()
 *
 *  function:
 *         1.  Set/Reset the PortD pin 12 for truck here
 *         2.  Set/Reset StatusA to reflect presence/absence of truck
 *
 *  input:  True/False
 *  output: none
 *
 *************************************************************************/

static void   truck_here(char truck)
{
  // last_routine = 0x4B;
  if (truck)
  {
    StatusA |= STSA_TRK_PRESENT;       /* We've got 'something' connected */

    /* Check for "Truck Here" stuff (locking gate, etc.) */

    if (((unit_type == INTELLITROL2) && !(enable_jumpers & ENA_TRUCK_HERE)) || 
         ((unit_type == INTELLITROL_PIC) && (enable_jumpers & ENA_TRUCK_HERE)))
    {
       TB3P5P6 = 0;     /* ACTIVE Low logic (0x02) */
    }
  }
  else
  {    /* Clear out truck-stuff */
    StatusA &= ~(STSA_TRK_PRESENT | STSA_TRK_TALK | STSA_TRK_VALID
                | STSA_BYPASSABLE | STSA_DIODE_GND | STSA_RESISTIVE_GND);

    /* Clear TB3p5p6 "Truck Here"or "Good Ground" */
      TB3P5P6 = 1;
  }
} /* end of truck_here */

extern const unsigned char DOW_CRC_tab[];
/*******************************************************************************
 * subroutine: truck_validate
 * function:   Validate the Vehicle ID ("TIM")
 * If TAS wait time has not elapsed, return
 * If TAS has denied the vehicle, return
 * If a TIM has not responded and VIP mode is not 5, return
 * If TIM S/N is all zeros, Clear STSA_TRK_VALID and return
 * If TIM S/N is all F and VIP mode is 5, Set STSA_TRK_VALID and return
 * If TIM S/N is all F and VIP mode is not 5, Clear STSA_TRK_VALID and return
 * If VIP mode is 5, Set STSA_TRK_VALID and return
 * If already validated, return
 * >>> If SperTIM connected call read_TIM_Go_NoGo_info()
 * Call nvTrkFind() to see if S/N in list, if yes set STSA_TRK_VALID and return
 * >>> If not found and SuperTIM look for TESTER or alternative S/N, 
 *    if found set STSA_TRK_VALID and return
 * If not yet validated and date stamp mode enabled, look for valid date stamp
 * If valid date stamp set STSA_TRK_VALID and return
 * If invalid date stamp set as unauthorized
 * If error on date stamp read, clear validate flag to set up for a retry
 * 
 *  input:      None
 *  output:     None
 *
 ******************************************************************************/
static void truck_validate (void)
{
int index;         /* Index of matching TIM */
char dsname[DS_NAMMAX];     /* DateStamp "Company ID" name string 6 characters*/
unsigned char dspsw[DS_PSWMAX];      /* DateStamp "Password" string (decoded) 10 characters */
int i;
char sts;
unsigned char temp_array[BYTESERIAL];

  if (badvipflag & BVF_TASDELAY)      /* Waiting for TAS? */
  {                                   /* Yes */
  /* See if we've waited long enough for TAS to exert its VIP "right of
     first refusal" */
    if (read_time() > (unsigned long)(SysParm.TASWait * 1000L))
    {
      badvipflag &= ~BVF_TASDELAY;    /* Yes, wait no more */
    }else
    {
      return;
    }
  }
  if (badvipflag & BVF_TASDENY)       /* TAS denied authorization? */
  {
    return;                          /* Yes, so be it */
  }
    /* TAS computer is out of the picture (either we've timed out waiting
       for response, or TAS delay is not enabled and we're just gonna look
       in our local NonVolatile Truck-ID list anyways). */
    /* Only validate a "real" Truck ID; 0x000000000000 is the empty Serial
       number, and 0xFFFFFFFFFFFF is the "dud" Serial Number, so if any TIM
       actually tries to return either of those numbers, punt it! (This also
       traps the case where we can't read the TIM...) */
  if ((SysParm.VIPMode != 5) && (TIM_state == 0))                 /* If no TIM, */
  {
    StatusA &= ~STSA_TRK_VALID;       /* Truck not authorized! */
    badvipflag |= BVF_TIMABSENT;      /* No Dallas/IO problems (0x08) */
    return;                           /*  we can't do much more here */
  }
  for ( i=0; i<BYTESERIAL; i++)
  {
    temp_array[i] = 0;
  }
  index = memcmp ((const void*)&truck_SN[0], (const void*)&temp_array[0], BYTESERIAL);
  if (index == 0)
  {
    StatusA &= ~STSA_TRK_VALID;  /* Truck not authorized! */
    return;
  }
  for ( i=0; i<BYTESERIAL; i++)
  {
    temp_array[i] = 0xFF;
  }
  index = memcmp ((const void*)&truck_SN[0], (const void*)&temp_array[0], BYTESERIAL);
  if (index == 0)
  {
    if (SysParm.VIPMode == 5) /* JGS Rev 1.8 */
    {
      StatusA |= STSA_TRK_VALID;          /* VIP Read Only, Truck is Authorized! */
      badvipflag = 0;                     /* No need to keep problem flags */
      return;
    }
    StatusA &= ~STSA_TRK_VALID;  /* Truck not authorized! */
    return;
  }
  if (SysParm.VIPMode == 5)	/* JGS Rev 1.8 */
  {
    StatusA |= STSA_TRK_VALID;          /* VIP Read Only, Truck is Authorized! */
    badvipflag = 0;                     /* No need to keep problem flags */
    return;
  }
	/* Once done, we don't need to [re]validate a truck until such time as
       the NonVolatile Truck ID list changes... */
  if (val_state)
  {
    return;
  }
  val_state++;                        /* No need to do again */
                                      /*  (unless TAS/VIPER changes the
                                          list on us...) */
  
  
  if (S_TIM_code)                     /* is the TIM a Super TIM? */
  {
    read_TIM_Go_NoGo_info();
  }
  
  if( (SysParm.EnaSftFeatures & ENA_UNLOAD_TERM) && (SysParm.Unload_Max_Time_min != 0) )
  {
      sts = check_unload_time();
      if( sts )
      {
        StatusA &= ~STSA_TRK_VALID;       /* Truck not authorized! */
//        badvipflag |= BVF_DSNOAUTH;      /* No Dallas/IO problems (0x08) */
        badvipflag |= BVF_UNLOAD_EXP;      /* No Dallas/IO problems (0x08) */
        return;                           /*  we can't do much more here */
          
      }
  }
  
  if( (SysParm.EnaSftFeatures & ENA_UNLOAD_TERM) && (SysParm.fuel_type_check_mask != 0) )
  {
      sts = check_fuel_type();
      if( sts )
      {
        StatusA &= ~STSA_TRK_VALID;       /* Truck not authorized! */
        badvipflag |= BVF_FUEL_TYPE;      /* wrong fuel type (0x0800) */
        return;                           /*  we can't do much more here */
          
      }
  }
  
  if( (SysParm.EnaSftFeatures & ENA_CPT_COUNT) != 0  )
  {
    if( truck_state == UNKNOWN )
    {
        StatusA &= ~STSA_TRK_VALID;       /* Can't authorize yet, don't know if we're 5 wire */
        badvipflag |= BVF_CPT_COUNT; /* DateStamp says No! */
        return;          
    }
      
    if( truck_state == OPTIC_FIVE )
    {
      sts = check_compartment_count();
      if(sts)
      {
//          bad_compartment_count = 1;
          StatusA &= ~STSA_TRK_VALID;       /* Truck not authorized! */
          badvipflag &= ~BVF_DSERROR; /* No Dallas/IO problems */
          badvipflag |= BVF_CPT_COUNT; /* DateStamp says No! */
          return;
      }
      else
      {
          badvipflag &= ~BVF_CPT_COUNT; /* DateStamp says No! */
      }
    }
    else
    {
        badvipflag &= ~BVF_CPT_COUNT; /* DateStamp says No! */       
    }
  }
  if( SysParm.Cert_Expiration_Mask != 0)
  {
    sts = superTIM_ds_validate();
    if (sts == (int)DSEXPIRED )  /* or an invalid terminal */
    {
      /* DateStamp authoritatively rejects this truck, so his
         only hope is a VIP-Bypass operation */
      badvipflag &= ~BVF_DSERROR; /* No Dallas/IO problems */
      badvipflag |= BVF_DSNOAUTH; /* DateStamp says No! */
      badvipdscode = sts; /* Detailed rejection code */
      return;
    }
    else
    {
      if (sts)         /* If can't access TIM/file */
      {
         /* May be "random" I/O error, try again later and maybe
            it'll work the next time round */
        badvipflag |= BVF_DSERROR; /* Errors accessing DateStamp */
        badvipdscode = sts; /* Detailed error code */
        val_state = 0;      /* Enable retry later */
        return;
      }
      else
      {
        badvipflag &= ~BVF_DSERROR; /* No error now */
        badvipdscode = 0;   /* This truck authorized */
      }
    }
            
  }
  
  sts = nvTrkFind (&truck_SN[0], (word *)&index); /* Check local list of good guys */
  // last_routine = 0x4C;
  if (sts)                            /* Is this TIM a known good guy? */
  {                                   /* No */
    unsigned char *mem_ptr;
    unsigned char temp_area[ALT_TIM_ID_SIZE];

    mem_ptr = temp_area;
    if (S_TIM_code)                   /* is the TIM a Super TIM? */
    {
      /******************************************************************************************
       * Check to see if Trucker Serial number is valid
       * DHP: Need to validate the SuperTim memory ONCE, then if not valid skip trying 
       *  to make it so. Also need to determine why so many 1-wire commands fail!
       ******************************************************************************************/
      if (fetch_serial_number(TEST_TIM, mem_ptr) == MB_OK)
      {
        badvipflag &= ~BVF_DONE;  /*  */
        if (memcmp((const void*)TRUCK_TESTER_SERIAL_NO, (const void*)mem_ptr, ALT_TIM_ID_SIZE) == 0)
        {
          printf("    Found Scully Tester ID\n\r");
          StatusA |= STSA_TRK_VALID;          /* Truck is Authorized! */
          badvipflag = BVF_DONE;              /* No need to keep problem flags */
          return;
        }
      }
      /**************************** 6/23/2009 10:16AM ************************
       * Since we did not find the laser etched TIM number and there is a
       * SuperTIM fetch the Alternate serial number.
       ***********************************************************************/
      if (fetch_serial_number(ALT_TIM, mem_ptr) == MB_OK)
      {
        /*************************** 6/23/2009 10:15AM ***********************
         * Lets try again and see if the Alternative TIM serial number
         * is somewhere in the VIP list
         *********************************************************************/
        badvipflag &= ~BVF_DONE;  /*  */
        if (nvTrkFind ((void *)mem_ptr, (void *)&index) == 0) /* Check local list of good guys */
        {
          printf("    Authorizing Alternative TIM ID\n\r");
          StatusA |= STSA_TRK_VALID;          /* Truck is Authorized! */
          badvipflag = BVF_DONE;              /* No need to keep problem flags */
          return;
        }
      }
    }
    /* TIM not in our local good-guys list; check the TIM for a valid
       "DateStamp" that intrinsically authorizes the truck */
    if (!(StatusA & STSA_TRK_VALID)) /* Already validated? */
    {                                /* No */
      if (active_comm & (INTELLI | GROUNDIODE)) /* COMM_ID aka TXA/RXA in use? */
      {                             /* Yes */
         val_state = 0;             /* Enable retry */
         return;                    /*  and try again later */
      }
      else
      {   /* Dallas communication available to us */
          /* If we have both a "Company ID" and a "Terminal Password",
             then we can operate in "Date Stamp" mode */
        active_comm |= TIM;       /* COMM_ID used for TIM "file access" */
        if ((pDateStamp)
            && (pDateStamp->name[0] != 0)
              && (pDateStamp->psw[0] != 0))
        {
            /* DateStamp-checking is enabled for this Intellitrol. */
          for (i = 0; i < DS_NAMMAX; i++) /* Decode file name */
          {
            dsname[i] = (char)(pDateStamp->name[i] ^ DOW_CRC_tab[i]);
          }
          for (i = 0; i < DS_PSWMAX; i++) /* Decode password */
          {
            dspsw[i] = (char)(pDateStamp->psw[i] ^ DOW_CRC_tab[i]);
          }
          sts = dsTruckValidate (COMM_ID, dsname, dspsw);
          if ((sts == (int)FILENOTFND)      /* If no DateStamp file */
            || (sts == (int)DSEXPIRED)    /* or DateStamp expired */
            || (sts == (int)INVALID))  /* or an invalid terminal */
          {
            /* DateStamp authoritatively rejects this truck, so his
               only hope is a VIP-Bypass operation */
            badvipflag &= ~BVF_DSERROR; /* No Dallas/IO problems */
            badvipflag |= BVF_DSNOAUTH; /* DateStamp says No! */
            badvipdscode = sts; /* Detailed rejection code */
          }
          else
          {
            if (sts)         /* If can't access TIM/file */
            {
               /* May be "random" I/O error, try again later and maybe
                  it'll work the next time round */
              badvipflag |= BVF_DSERROR; /* Errors accessing DateStamp */
              badvipdscode = sts; /* Detailed error code */
              val_state = 0;      /* Enable retry later */
            }
            else
            {
              badvipflag &= ~BVF_DSERROR; /* No error now */
              badvipdscode = 0;   /* This truck authorized */
            }
          }
        }
        
        active_comm &= ~TIM;      /* Mark COMM_ID now free again */
      }
    }
    if (sts)                        /* Still unauthorized? */
    {                               /* Yes... */
        /* Clear the Truck-Authorized flag (TAS/VIPER may have just "black-
           listed" the current/active truck...note also that you cannot
           blacklist a "DateStamp" truck!) */
      if (!((SysParm.TASWait != 0)      /* Don't unauthorize if TAS in control */
         && (StatusA & STSA_TRK_VALID)))  /* and already authorized */
      {
        StatusA &= ~STSA_TRK_VALID;  /* Truck not authorized! */
        badvipflag |= BVF_UNAUTH;    /* Flag unauthorized (for bypass) */
      }
      return;
    }
  }
  StatusA |= STSA_TRK_VALID;          /* Truck is Authorized! */
  badvipflag = 0;                     /* No need to keep problem flags */
  return;

} /* End truck_validate() */

/***************** previous end of trukstat ********************************/
/*********************************************************************
 *  subroutine: check_channels
 *
 *  function:   Compare current sensor channel voltages to open connector voltages and
 *                  report if any are seen as being pulled down which indicates a likely connection.
 *
 *  input:       None
 *  output:     TRUE (All voltages at open connector levels) / FALSE  (low voltage seen)
 *
 *********************************************************************/
unsigned char check_channels (void)
{  
int index;
unsigned int    save_porte;
unsigned char status, ops_on;
unsigned char save_JUMP;
unsigned char save_pulse5;

  save_porte   = LATE;                           /* save the state */
  save_pulse5 =PULSE5VOLT;              /* save the state */
  save_JUMP = JUMP_START;            /* JUMP_START currently enabled ? */
  if (T3CONbits.TON)                           /* Turn off T3/ADC if needed */
  {
    ops_on = TRUE;
    ops_ADC( OFF );                  /* Shut OFF the 1 mS interrupt (T3) */
  }
  else
  {
    ops_on = FALSE;
  }
  JUMP_START = SET;              /* Enable Jump-Start's +20V */
  PORTE = PULSE_TEST;          /* Drive selected channels */
  PULSE5VOLT = CLR;              /* Chan 4 normal (10V) drive */
  DelayUS(1000);                         /*  allow ramp up time */
  (void) read_ADC();                    /* discard what may be a stale read */
  if (read_ADC() == FAILED)      /* see what the probes are doing now */
  {
    return FALSE;
  }
  /* Check all channels for a likely-looking voltage drop. If any channel(s)
     seems to have activity of some sort, assume a truck still connected. */
  status = TRUE;                                           /* Assume rail voltages */
  for (index=start_point; index<MAX_CHAN; index++)
  {
     if (ConfigA & CFGA_EURO8VOLT)      /* "European 10 Volt" jumper?? */
     {                                                               /* Yes, limited voltages... */
        if ((probe_volt[index] > ADC1V)           /* If current channel NOT grounded */
              && ((probe_volt[index] + ADC1V) /* but it does show */
                   < open_c_volt[1][index]))            /* a 250mV voltage "drop" */
        {
           status = FALSE;                                  /* channel not at rail */
        }
     }
     else                                                          /* IF, "US" 20 Volt Jump Start Present */
     {
        if ((probe_volt[index] > ADC1V)           /* If current channel NOT grounded */
              && ((probe_volt[index] + ADC4V) /* but it does show */
                  < open_c_volt[1][index]))            /* a 4 volt voltage "drop" */
        {
           status = FALSE;                                  /* channel not at rail */
        }
     }
  }
  if (!save_JUMP)
  {
    JUMP_START = CLR;                              /* Disable Jump-Start's +20V */
  }  
  LATE = save_porte;
  if(ops_on)
  {
     ops_ADC( ON );                                      /* Turn ON the 1 mS interrupt (T3) */
  }
  PULSE5VOLT = save_pulse5;
  return status;
}

/************************* end of trukstat **********************************/
