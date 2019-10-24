/*******************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         com_two.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais, Dave Paquette
 *
 *                   @Copyright 2009 2015  Scully Signal Company
 *
 *  Description:  Rack controller main microprocessor PIC24HJ256GP210
 *                Two wire Optic and Thermistor probes handling
 *
 * Revision History:
 *   Rev      Date       Who   Description of Change Made
 * ------  --------  ---  --------------------------------------------
 * 1.5.23  06/26/12  KLL  In active_two_wire() removed test code to try to
 *                          correctly identify 2 wire optical and thermistor
 *                          sensors. It did not  work consistently.
 *                        Initialize sensors 8 through 16 as unknown when
 *                          logging a dome_out condition
 *         05/11/12  KLL  Fixed a problem with check_all_pulses(). It now
 *                          reports the two wire probe type correctly. Also
 *                          The thermistor_present() call was moved to this
 *                          procedure from two_wire_thermal().
 *                        Changed the return in check_active_shorts() for debug.
 *                        Changed PROBE_TRY_STATE to char
 *                        Rewrote check_truck_gone to open up code space
 *                        Changed short_6to4_5to3() to support IntelliCheck 2 SHO
 * 1.5.27  10/22/12  KLL  Changed the force probe timeout from 19 seconds to 1 minute.
 *                        Added the time to the force printout for Modbus addr. 00
 *         03/24/14  DHP  Stop shorts tests after 10 failures.
 * 1.5.30  09/10/14  DHP  Added read_ADC() in two_wire_start() - a TB4 p9p10
 *                          short was showing active sensors.
 * 1.5.31  01/14/15  DHP  Changed bit_sum minimum from 2 to 1 in check_all oscillating()
 *                        Changed point init in check_shorts_opens()
 *                        Changed variable inits to be in-line versus via define
 * 1.6.32  03/10/15  DHP  In two_wire_start() removed read_ADC()
 *                        In PULSING_2W, set gone_pass_count to 0 if
 *                          check_truck_gone() returns FALSE indicating truck
 *                          still here. This eliminates the case where a truck
 *                          is seen as gone due to non consecutive TRUE returns
 *                          when someone is unable to get the plug connected.
 * 1.6.34  07/08/16  DHP  QCCC 53: Added HighI_On() and HighI_Off()
 *                          Made call to check_active_shorts() conditional
 *                            on sensor type being thermistor.
 *                        FogBugz 108 Changed check_truck_gone(void) to match
 *                            checks made in truck_idle() and added a check for
 *                            truck pulsing and an immediate return if so.
 *                          Added a re-acquire to active_two_wire() based
 *                            on having never been dry to handle a wet
 *                            connection of non-permitting 5-wire
 *                            Intellichecks switching to permit.
 *                          Added check_shorts_opens() call on wet 2-wire and 
 *                            changed open voltage level from rail-250 mv to
 *                            rail-200 mv
 *                          In short_6to4_5to3() changed sequence of setting 
 *                            PORTE and tuning on ops_ADC; All channels should be
 *                            powered prior to reading voltages.
 *                          Added check_2wire() to provide quick check of 2-wire
 *                            vehicle presence for truck_acquire().
 *                        FogBugz 127 Added check_highI_shorts() and call from 
 *                            active_two_wire() to detect a wet sensor shorted
 *                            to a dry.
 * NOTE: check_active_shorts() is called only for thermistors.  Dry 2-wire optics
 *       appear to drop about 2 volts from their high state after pvolt is
 *       removed but this takes about 10 ms. A lot of testing would be needed to
 *       verify we could find shorts without false positives.  Probably can
 *       be done but not worth the time required.
 ******************************************************************************/
#include "common.h"
#include "volts.h"   /* A/D voltage definitions */
unsigned char check_channels (void);
static void check_highI_shorts(void);

/*********************************************************************************************/

/* TIMING DEFINITIONS*/
/* Time in milliseconds after switching to two-wire state and before the
   initial channel/probe shorts test (essentially a "de-bounce" period).
   Unfortunately, on-boarder seems to need a coupla seconds sometimes!
   (see ONBOARD_GRACE below), so keep this "blind wait" small for
   everything else that works! */

#define N_CYCLES_250  250       /*CYCLE_TIME_2W_DEBOUNCE*/

/* Time in milliseconds between successive active_two_wire() short-tests
   before transitioning to active/permissive state. */

#define N_CYCLES_100      100   /* CYCLE_TIME_2W_TEST */

/* Time in milliseconds between successive active_two_wire() state checks
   for probe-is-pulsing wet/dry transitions. */

#define N_CYCLES_30       30    /* CYCLE_TIME_2W_ACTIVE */

/* Time in milliseconds between successive active_two_wire() state checks
   for truck disconnected after faulty (short/ground/etc.) detect. This
   state must be "slow enough" not to trick backup processor into thinking
   probe(s) oscillating due to StaticShortTest()'s diddling the channel
   drives... (apparently this is hopeless, backup is triggered anyways,
   just by StaticShortTest(). Sigh...) */

#define N_CYCLES_200      200   /* CYCLE_TIME_2W_FAIL */

/* Hysteresis (in milliseconds mod "Cycle" time) before going "dry" and
   permissive. Must see consecutive "dry" probe checks this long before
   switching to dry state and going permissive. */

#define N_CYCLES_12     12     /* DRY_HYSTERESIS(360 / 30) */

/* Hysteresis (in milliseconds mod "Cycle" time) before going "wet" and
   non-permissive. Must see consecutive "wet" probe checks this long
   before switching to wet truck state and shutting off the permit relay. */

#define N_CYCLES_5    5      /* WET_HYSTERESIS (150 / 30) */

/* Hysteresis (in milliseconds mod "Cycle" time) after going "wet" and
   before trying to re-establish Jump-Start's 20V in the case of mixed
   two-wire-optic (which start oscillating immediately) and thermistor
   probes. */

#define N_CYCLES_10   10        /* TRUCK_SWITCH (300 / 30) */

/* Hysteresis (in milliseconds mod "Cycle" time) before actually declaring
   a truck gone. This is a grace period to allow for flakey connections to
   "intermittently" come and go. Don't set this too high, since if we're in
   overfill bypass, the unit will be permissive for this entire period! */

// #define N_CYCLES_67   67    /* TRUCK_GONE_GRACE */

/* Grace period (in milliseconds) in which to allow a Fault-detected unit
   to "un-fault" itself. This is a ***HACK*** to allow the "onboarder"
   units to connect -- empirically it is observed (Ha!) that "it takes a
   while" (contact bounce? Cosmic Ray flux?) to properly ascertain that
   all contacts are shorted and thus it's really OK. What a Krock! */


/* Delay in milliseconds after two-wire thermistor goes active/permissive
   and before starting to check for active shorts (first start calling
   check_active_short()) Among other things, time to turn *OFF* jump
   start's 20 volts! ...and otherwise let things stabilize. */

#define N_CYCLES_1000 1000              /* ACT_SHORT_DELAY */

/* Interval in milliseconds between successive active shorts check
   (check_active_shorts() calls), per channel -- i.e., "n" channels
   times N_CYCLES_1000 is how often all channels will have been sampled. */

#define N_CYCLES_125 125                /* ACT_SHORT_INT */

#define N_SHORT_FAILS  10        /* Number of failures before giving up */

//>>> QCCC 53
void HighI_Off(unsigned int probe)
{
   switch (probe)
  {
     case 0:
       HC_OFF1 = 1; 
       break;
     case 1:
       HC_OFF2 = 1; 
       break;
     case 2:
       HC_OFF3 = 1; 
       break;
     case 3:
       HC_OFF4 = 1; 
       break;
     case 4:
       HC_OFF5 = 1; 
       break;
     case 5:
       HC_OFF6 = 1; 
       break;
     case 6:
       HC_OFF7 = 1; 
       break;
     case 7:
       HC_OFF8 = 1; 
       break;
     default:
       break;
   } 
}

void HighI_On (unsigned int probe)
{
  switch (probe)
  {
    case 0:
      HC_OFF1 = 0; 
      break;
    case 1:
      HC_OFF2 = 0; 
      break;
    case 2:
      HC_OFF3 = 0; 
      break;
    case 3:
      HC_OFF4 = 0; 
      break;
    case 4:
      HC_OFF5 = 0; 
      break;
    case 5:
      HC_OFF6 = 0; 
      break;
    case 6:
      HC_OFF7 = 0; 
      break;
    case 7:
      HC_OFF8 = 0; 
      break;
    default:
      break;
  } 
}
//<<< QCCC 53

/*********************************************************************************************
 *  subroutine:      two_wire_start()
 *
 *  function:   Set Up MUX and POWER for Optical and Thermistor Two Wire Probes
 *  Turn ON the Acquisition Interrupt (2 mS) if read_ADC() is OK,
 *  else iambroke will handle the ADC error
 *  Called from: optic_2_setup(), thermal_setup(), truck_acquire via
 *  which_probe_type()
 *
 *  input:  none
 *  output: none
 *
 **********************************************************************************************/

void two_wire_start(void)
{
  set_porte( PULSE_TEST );    /* assure setup for 2 wire pulsing */
  // last_routine = 0x8E;
  set_mux(M_PROBES);           /* assure mux set to probes */
  {
  // last_routine = 0x8E;
    probe_result_flag = 0;    /* Clear  */
    ops_ADC( ON );              /* enable 1ms interrupt */
  }
} /* end of two_wire_start */

/*************************************************************************
 *  subroutine:      active_two_wire()
 *  function:   Called from truck_active()
 *
 *         1.  Upon entry the truck type (Thermistor or Optic2W) is
 *             determined but the status of the probe SYSTEM is indeterminate.
 *         2.  Upon exit the tank_state is set as DRY, WET or DEPARTED per
 *             the result of specific probe analysis.
 *
 *  input: test_state as (No_type, Optic2, Optic5, Optic2-Thermis)
 *  two_wire_state: No_test_2W, ShortChk_2W, ShortFail_2W, Pulsing_2W
 *
 *  output: none
 *************************************************************************/
void active_two_wire( PROBE_TRY_STATE test_state)
{
static unsigned long act_short_time; /* Next active short check */
static char          no_swap;
static unsigned int  act_short_chan; /* Next channel for act short check */
unsigned int         status = 0;
int ch_index, dry_count;
static unsigned char short_fail_count;

  if ( probe_time < read_time() )    /* If it's time to work */
  {
    switch (two_wire_state)
    {
      case NO_TEST_2W:               /* Initialize two-wire active */
      /*  Zero counters */
         gon_pass_count = 0;        /*  - scan passes for probe time */
         dry_pass_count = 0;        /*  - scan passes for probe time */
         wet_pass_count = 0;        /*  - scan passes for probe time */
         short_fail_count = 0;       /*  - No test failures yet */ 
         two_wire_state = SHORTCHK_2W;        /*  Change flag to Checking for shorts/etc. prior to permit */
         probe_time = read_time() + N_CYCLES_250;    /*  Next run in 250 ms */
         break;

      case SHORTCHK_2W:
        if (test_state == OPTIC2)
        {
          xprintf( 45, DUMMY );
        }
        if (test_state == THERMIS)
        {
          xprintf( 46, DUMMY );
        }
        if ((status = StaticShortTest(TRUE)) != 0) /* Are all channels working */
        {                                  /* Here when Shorted/Grounded/Faulty channel */
          two_wire_state = SHORTFAIL_2W;  /* No more dealings with this */
                                          /* truck, but check for truck_gone */
        }
        else
        {                                  /* Here when passed for shorts, or crosstalk */
          xprintf( 42, status );
          two_wire_state = PULSING_2W;    /* Initialize for going active/permissive */
        }                                  /* next time around */
        probe_time = read_time() + N_CYCLES_100; /* next test in 100 cycle times */
        no_swap = FALSE;
        short_fail_count++;                /* count test failures */
        break;
                                  /* End two-wire short test */
      case SHORTFAIL_2W:
        tank_state = T_SHORT;       /* De-Permit */
        if (check_truck_gone())     /* Truck/whatever still attached? */
        {
          xprintf( 47, 1 );          /* Force the next message to print */
          truck_state = GONE_TWO;      /* No, outta here! */
        }
        if (short_fail_count < N_SHORT_FAILS)
        {
          status = StaticShortTest (TRUE); /* Truck connected, see if any more shorts */
          if (status == 0)            /*  state -- StaticShortTest() has */
                                      /*  changed its mind, it's OK now! */
                                      /*  as it returns TRUE (1) when shorted */
          {
            short_fail_count = 0;
            two_wire_state = SHORTCHK_2W;  /* So double-check one */
          }                                               /*  next time around */
          else
          {
            short_fail_count++;
          }
        }
        probe_time = read_time() + N_CYCLES_200;      /* 200 */
        break;

      case PULSING_2W:    /* Active - check wet/dry/gone */
                          /* Declare time to next wet/dry state check */
                          /* Accumulate probe time, don't keep resetting it, */
                          /* catch overlong cycles! */
        probe_time = (read_time() + N_CYCLES_30);                /* 30 */
        if(check_all_oscillating() != FALSE)    /* See if all probes DRY or WET */
        {                                         /* Here when all probes are DRY */
          wet_pass_count = 0;              /* Reset the filter */
          gon_pass_count = 0;              /* Reset Truck-Gone filter too */
          if (!(dry_pass_count & 0x80))    /* Only a byte, max at 127... */
          {
            dry_pass_count++;
          }
          else
          {
            dry_pass_count = 0;  /* Reset the count */
          } 
          if (dry_pass_count >= N_CYCLES_12) /* force several iterations */
          {
            /* Transition to Dry/permissive state ? */
            // >>> fogBugz 127
            if (dry_once == FALSE)
            {
              check_highI_shorts();
            }
            // <<< fogBugz 127
            dry_once = TRUE;       /* FogBugz 108 */
            if (dry_pass_count == N_CYCLES_12)
            {                      /* Re/Init active short checking */
              act_short_time = (read_time() + N_CYCLES_1000);
              act_short_chan = 8; /* Force new scan cycle */
              act_therm_mask = 0; /* Build new thermistor mask... */
            }
            tank_state = T_DRY;  /* It's Dry, set the state */
            jump_time  = 0;      /* reset the JUMP_START to 0 */
            dry_probes();        /* initialize all probes */
            no_swap = TRUE;      /* control of Thermistor bias switching */
            /* Check actively-permitting truck to make sure no probes
               have mysteriously shorted together or otherwise have
               flaked out on us. */
            if (read_time() > act_short_time) /* Has to be done with delay */
            {
              act_short_chan++;           /* Shorts-check the next channel */
              if (act_short_chan >= 8)    /* After 8 probes maximum */
                                          /* Start from the beginning */
              {
                if (ConfigA & CFGA_8COMPARTMENT)
                {
                    act_short_chan = 0; /* do all 8 channels */
                }
                else
                {
                    act_short_chan = 2; /* USA - only last 6 channels */
                }
              }
              /* Check one channel for Shorted while active */
              if(probe_type[act_short_chan]==P_THERMIS)  //QCCC 53
              {
                if (check_active_shorts ((unsigned int)1<<act_short_chan))
                {
                  asm volatile("nop");    /* place for the breakpoint to stop */
                  asm volatile("nop");    /* place for the breakpoint to stop */
                  asm volatile("nop");    /* place for the breakpoint to stop */
                }
              } 
              /* Check next channel "a little while later" */
              act_short_time = read_time() + N_CYCLES_125;
            }    /* End of if (read_time() > act_short_time) */
          }    /* End of if (dry_pass_count >= 12 MILLISECONDS) */
        }    /* End of if( status = check_all_oscillating() ) */
        else
        {                           /* check_all_oscillating() failed */
          dry_timer = 0;           /* reset for wet probe operation */
          dry_pass_count = 0;      /* reset the dry filter */
          if (!(wet_pass_count & 0x80)) /* It's only a byte */
          {
            wet_pass_count++;     /* Max value at 127 (> 3 seconds) */
          }
          if (wet_pass_count > N_CYCLES_5)   /* allow for a few faults */
          {
            if ( tank_state == T_DRY)
            {
              overfill_count = 0;  /* restart the count */
            }
            overfill_count++;
            tank_state = T_WET;  /* It's not oscillating, and it has not been for at least 5 cycles */
            if (overfill_count==20)  /* Only log it once */
            {
              dry_count = 0;
              for (ch_index=start_point; ch_index<MAX_CHAN; ch_index++)   /* all channels oscillating ??? */
              {
                if ( probes_state[ch_index] == P_DRY)
                {
                  dry_count++;
                }
              }
              if ( dry_count > 0 )
              {
              int i;
                for ( i=8; i<16; i++)   /* There are no probes after 8 in a two wire truck */
                {
                  probes_state[i] = P_UNKNOWN;      /* Best designation for no probe ? */
                }
                log_dome_out();               /* not gone, handles special cases */
              }
            }
          }
          if (check_truck_gone() !=FALSE)   /* determine if truck Gone */
          {                                  /* Give connection a second or two */
            if (gon_pass_count++ > 67)
            {                             /* Truck is truly gone */
             xprintf( 47, 2 );            /* Force the next message to print */
             truck_state = GONE_TWO;      /* We will double check if it is */
                                          /* not an Intellicheck playing games */
                                          /* in main_activity() */
            }
          }
          else
          {
            if (wet_pass_count > N_CYCLES_10)
            {
              check_shorts_opens();  /* This will reset probes_state and tank_state if open or short */
              /* May be mixed thermistor and optic probes -- optic
                  start oscillating immediately, but the thermal probes
                  might still need to warm up, so re-enable JUMP_START
                  to speed up the process... */
              wet_pass_count = (char)N_CYCLES_5; /* reset the filter */
              if ( (test_state == OPTIC2) && !no_swap )
              {
                xprintf( 34, DUMMY );     /* "\n\r\n\r    Switching Bias to Thermistor probe Value\n\r\n\r" */
                jump_time  = SEC20;        /* reset the JUMP_START to time out */
                probe_try_state = THERMIS; /* try a mixed 2 wire truck */
                // >>> FogBugz 108 */
                if((dry_once == FALSE) && (read_time() > cycle_timeout))
                {
                  cycle_timeout = (read_time() + SEC10);
                  set_main_state (ACQUIRE);         /* re-enter the ACQUIRE mode */
                  acquire_state =  IDLE_I;
                } 
                // <<< FogBugz 108 */
              }
            }
          }
        }    /* End of if check_all_oscillating */
        set_porte (PULSE_TEST);     /* Enforce Jump-Start/et-al status */
      break;
                              /* End two-wire active/pulsing */
      default:
        break;
    }    /* End of switch (two_wire_state) */
  }    /* End of if it's time to check */
} /* end of active_two_wire */

/*************************************************************************
 *  subroutine:      check_all_pulses()
 *
 *  function:
 *
 *         1. Check for the case of two wire optical & thermal oscillations
 *         2. If any oscillation is seen on (1,2),3,5,7,8 probes we then test
 *             for what type of probe we have.
 *         3. If the characteristics match the type of probe being tested
 *             return a TRUE state else a FALSE state.
 *         4. The test consists of serially reading the ADC channels and
 *             determining if an oscillation has occurred since last reading
 *             An array of bytes record the 1 or 0 recorded for each type
 *             of probe(its characteristics).
 *
 *  input:  probe_try_state
 *  output: TRUE (pulse seen - probe detected) / FALSE (nothing found)
 *
 *************************************************************************/

#define OPTIC_COUNT     8
#define THERMIS_COUNT   7

char   check_all_pulses( PROBE_TRY_STATE truck_probes )
{
unsigned char  count_osc, osc_base;
char           status;
unsigned int   index;
unsigned int   point;
unsigned int i, compart;
unsigned int volt_transition;

    status = FALSE;                  /* Default to no pulses */
    if (ConfigA & CFGA_8COMPARTMENT)    /* 6 or 8 compartment mode? */
    {
        compart = 0;                    /* 8-compartment configuration */
    }
    else
    {
        compart = 2;                    /* USA/6-compartment config */
    }
    if ( (two_wire_state==NO_TEST_2W) && ((truck_probes==THERMIS) ||
                                          (truck_probes==OPTIC2)) )
    {
       probe_index = 0;                 /* NOTE may conflict with interrupt's action */
       for (index=0; index<MAX_ARRAY; index++)
       {
          probe_array[index] = 0x00;    /* set to a 'cold' truck */
       }
       return(status);                  /* Initialization return */
    }

    osc_base = OPTIC_COUNT;        /* The number of times we need to see oscillation */
                                   /* on all channels is now set to 8 */
    if (truck_probes==THERMIS)     /* which probe type are we looking for */
    {
       osc_base = THERMIS_COUNT;   /* Now set to 7 */
       xprintf( 21, DUMMY );
    }
    else
       xprintf( 22, DUMMY );

    if ( probe_index > (MAX_ARRAY-2)) /* wait till processing almost done */
    {
       point = probe_index;
       for ( i=compart; i<MAX_CHAN; i++ )
       {
        /* NOTE we skip the first reading as it may be bogus */
         count_osc = 0;
         volt_transition = (1 << i);
         for (index=1; index<(point-2); index++) /* any oscillation at all ??? */
         {
            if ((probe_array[index] & volt_transition) == volt_transition )    /* Count transitions on all channels */
            {
               count_osc++;
               if (count_osc > osc_base)  /* force a filter on the oscillations */
               {                          /* Now at an ADC of 1ms sample */
                  status = TRUE;          /* Filter condition satisfied - we had */
                                          /* more than minimum transitions */
                  break;
               }
            }
         }
         if ( status == TRUE)
         {
          break;
         }
       }
       if (status == FALSE)  /* reset the probe state accordingly if status is FALSE */
       {
          two_wire_state = NO_TEST_2W;
          if (probe_try_state == THERMIS)     /* these are used in */
          {
             probe_try_state = NO_TYPE;       /* which_probe_type() */
          }
          if (probe_try_state == OPTIC2)      /* to control testing flow */
          {
             probe_try_state = THERMIS;
          }
       }
    }
    /***************************** 5/11/2012 10:36AM *************************
     * The Thermistors might not be pulsing but are warming up
     * There is a voltage drop. If we detect that drop then we know
     * there are Thermistor probes out there
     *************************************************************************/
    if ((status == FALSE) && (truck_probes==THERMIS))
    {
      status = thermistor_present();       /* look for multi voltage drops */
    }
    /***************************** 8/4/2010 3:30PM ***************************
     * Used to determined if we are in a endless
     * loop trying to figure out what type of probe is connected
     *************************************************************************/
    if (++probe_retry_count > 65000L)  /* If about minute has gone by */
    {
      xprintf( 128, 0x00);          /* Print <CR> */
      Print_Crnt_Time();
      xprintf( 145, 0x00);          /* Print out the timeout message */
      probe_try_state = THERMIS;    /* Default to 2 wire Thermister probes and see what happens */
      status = TRUE;
    }
    return(status);
} /* end of check_all_pulses */

/*************************************************************************
 *  subroutine:      check_all_oscillating()
 *
 *  function:
 *
 *         1. Check for the case of two wire optical & thermal oscillations
 *         2. Check that all channels are oscillating and the periods
 *            are between 10Hz and 200Hz (50ms to 5ms)
 *         3. If the characteristics match the type of probe being tested
 *             and all oscillate, return a TRUE state else a FALSE state.
 *         4. The test consists of serially reading the ADC channels and
 *             determining if an oscillation has occurred since last reading
 *             An array of bytes record the 1 or 0 recorded for each type
 *             of probe(its characteristics).
 *         5.  The testing is done over a band of channels from the last
 *             sample backward by SCAN_WIDTH channels (at 1 ms sample rate)
 *
 *  input:  none
 *  output: TRUE/FALSE
 *
 *************************************************************************/

#define SCAN_WIDTH      (unsigned int)143         /* number of binary 1ms samples to scan */
                                    /* for Maximum Array size = 200 */
#define MAX_OSC         120         /* if SCAN_WIDTH changes, so does this */
                                    /* right up to the probe index */

unsigned int check_all_oscillating( void )
{
unsigned int  status;
unsigned char  near_point;
unsigned char far_point;
unsigned int  index;
unsigned int  ch_index;
unsigned int  bit_sum;    /* INTEGER */
unsigned char bit_mask;
unsigned char bit_state;

  // last_routine = 0x91;
  status = TRUE;                   /* Set for GOOD probe */

  far_point = (unsigned char)probe_index;
  if (probe_index > SCAN_WIDTH)    /* The sample array fits in is past minimum */
     near_point = (unsigned char)(far_point - SCAN_WIDTH);    /* Set the window from */
                                                     /* current index back */
  else
     near_point = (unsigned char)(MAX_ARRAY + far_point - SCAN_WIDTH); /* assure we don't */
                                                              /* end up with a negative # */
                                                              /* by "rolling" */
  for (ch_index=start_point; ch_index<MAX_CHAN; ch_index++)   /* all channels oscillating ??? */
  {
    index = near_point;
    bit_mask = (unsigned char) ((unsigned int)1 << ch_index);   /* Walk 1 across the byte */
    bit_sum  = 0;

    while (index != far_point)
    {
      // last_routine = 0x91;
      bit_state = (unsigned char) (probe_array[index] & bit_mask);
      if (bit_state > 0)         /* 1 = transition */
        bit_sum += 1;
      index++;
      if (index >= MAX_ARRAY)    /* Roll Over at the end of Array */
        index = 0;
    }   /* End of while (index != far_point */

    // last_routine = 0x00;
    if (probes_state[ch_index] < P_FAULT) /* "Latch" onto any faults (10) */
    {
      if ( (bit_sum > 1) && (bit_sum < MAX_OSC) )
      {                           /* Probe appears to be oscillating */
        probes_state[ch_index] = P_DRY; /* Mark this one "Dry" (2) */
      }   /* End oscillating probe */
      else
      {
        if ((probes_state[ch_index] == P_DRY)            /* P_DRY = 2 */
                 || (probes_state[ch_index] == P_UNKNOWN))    /* P_UNKNOWN =0 */
        {                           /* Probe was DRY, now not oscillating */
                                    /* Or first time around and NOT yet OSCILLATING */
          /* High current maybe will cause a probe shorted to this one to show as bad */
          if (probes_state[ch_index] == P_DRY)
          { 
              for (index = start_point; index<MAX_CHAN; index++)
              {
                HighI_On(index);
              }  
          }
          probes_state[ch_index] = P_WET; /* Mark this probe "Wet" (1) */
        }
      }
     }    /* End of if (probes_state[ch_index] < P_Fault) */
  }    /* End of for (ch_index=point; ch_index<MAX_CHAN; ch_index++) */
          /* Do the next channel */
  for (index=start_point; index<MAX_CHAN; index++) /* all channels oscillating ??? */
  {
   if (probes_state[index] != P_DRY)             /* P-Dry (2) */
     {
       status   = FALSE;    /* If ANY probe is WET */
       break;               /* Set for NON PERMISSIVE */
     }
  }
  return(status);
} /* end of check_all_oscillating */

/*************************************************************************
 *  subroutine:      check_shorts_opens()
 *
 *  function:
 *
 *
 *         1.  Check for the case of two wire optical & thermal shorts
 *             or opens
 *         2.  Shorts turn on the probe led with a quick flash
 *         3.  Opens turn on the probe led with a slow flash
 *         4.  Thermal Cold or Hot leave the led dark
 *         5.  Values are in stdsym.h (at end)
 *
 *  __
 *    \                                               Vcc_
 *     \
 *      \              Thermistor Patterns                 COLD
 *       \
 *        \                                            8 _
 *         \
 *          \                                              WARM
 *           \
 *            \                                        7 _
 *             \                             ___________
 *              \      3.5/3.8              /
 *               \      ___     ___     ___/               WET/DRY
 *                \    |   |   |   |   |
 *                 \___|   |___|   |___|             2/3 _
 *                      1.2
 *                                                         SHORT
 *  input:  none  ____________________________________ 0 _________
 *  output: none
 *
 *
 *************************************************************************/

#define SHORT_VOLT      ((2*ADC1V)/3)        /* (2*1000)/3 = 667 */
#define OPEN_VOLT_DIFF  (ADC1V/4)            /* 1000/4 = 250 */
//#define OPEN_VOLT_DIFF  (ADC1V/5)            /* 1000/5 = 200: intermittent OPEN on dry*/
//#define COLD_VOLT       (ADC8V+(ADC1V/2))
//#define HOT_VOLT        (ADC7V+(ADC1V/2))

void check_shorts_opens( void )
{
  int   index;
  unsigned char  point;

  JUMP_START = CLR;                              /* Disable Jump-Start's +20V */
  service_charge();
  if (wait_for_probes() == PASSED)  /* wait for and ignore 1st reading */
  {
    if (read_ADC() == FAILED)
    {
      printf("Restart the Probe interrupts\n\r");
      Init_ADC();
    }
  }
    
  point = 0;       /* default is test first two probes */
// last_routine = 0x92;
  if (StatusA & STSA_DEBUG)           /* Is DEBUG jumper installed? */
     return;                          /* Yes, no shorts testing then */
  if (!(ConfigA & CFGA_8COMPARTMENT))
  {
    point = 2;       /* don't test first two probes */
  }
  else
  {
    point = 0;       /* test first two probes */
  }

  for (index=point; index<MAX_CHAN; index++) /* use present ADC values */
  {
    /* Some Optic Probes (e.g., Liquidometer Optic Dummy) will oscillate
       down to ground (less than 80 millivolts "average") . . . which
       would otherwise appear to be a short! If probe is currently marked
       as "Dry", then it is still oscillating, so skip it here -- only
       check probes after they have transited into a "Wet" (or worse)
       state.

       Note also that once a probe has been flagged shorted/etc., it is
       "permanently" out of service -- the truck will have to disconnect
       to clear the Short/Fault condition. */

   if (probes_state[index] != P_DRY) /* Probe/channel *not* osc'ing? (2) */
   {                                 /* Yes, Wet, Cold, Shorted, etc. */
      if ( probe_volt[index]<SHORT_VOLT )    /* 667 */
      {
         probes_state[index] = P_GROUND; /* Shorted to ground (11) */
         two_wire_state = SHORTFAIL_2W;  /* This truck is history! */
                                         /* DE-Permit, but check for truck_gone */
      }
      else  if ( probe_volt[index]>(open_c_volt[0][index]-OPEN_VOLT_DIFF) )    /* 250 mV */
      {    /* Here when voltage is less than 250 mV below open_c+volt level */
        probes_state[index] = P_OPEN;    /* P_Open (5) */
        tank_state = T_OPEN;
      }
// >>> QCCC 53; Unable to detect difference between wet optic and cold thermistor
#if 0
      else if (truck_state == THERMAL_TWO)
      {
        if ( probe_volt[index]>COLD_VOLT )
        {
          probes_state[index] = P_COLD;    /* P_Cold (3) */
          tank_state = T_COLD;
        }
        else if ( (probe_volt[index]<COLD_VOLT) &&
                (probe_volt[index]>HOT_VOLT) )
        {
          probes_state[index] = P_HOT;     /* P_Hot (4) */
          tank_state = T_HOT;
        }
      } /* End Thermistor probes check */
#endif
// <<< QCCC 53
    } /* End not-dry probe check */
  } /* End for loop over all channels */
} /* end of check_shorts_opens */

/*************************************************************************
*       Function Name:      check_active_shorts
*       Function Type:      enum boolean
*
*   Description
*               Like StaticShortTest(), but called while truck is active
*               permitting two-wire thermistor to check for intermittent
*               short (i.e., OK when truck first connected, but flaky wire/
*               whatever now shorts two probes/etc.). It Happens!
*
*               Here, the test is simpler, consisting of merely turning
*               off the selected channel momentarily ("a few MS") and
*               making sure the channel goes low/inactive (i.e., is not
*               shorted to and/or being fed by another channel). We must
*               be much more circumspect than StaticShortTest() since we
*               don't want backup processor to falsely start permitting!
*
*       Input:                  Mask of channels to "test"
*
*       Input Type:
*
*       Returns:                probes_state[] updated if needed
*
*************************************************************************/

/* Amount of time (milliseconds) to allow selected channel(s) to power down
   after turning off power. If channel is still powered after this interval,
   it is "shorted" to some power source. This interval must be kept fairly
   small (substantially less than 30 milliseconds). */

#define ACT_OFF_TIMEOUT     10

int check_active_shorts (unsigned int channel)
{
int status;
unsigned int  threshold;
unsigned short msdelay;
int           i;
char          point;
unsigned int  porte_save;
unsigned int  chan_mask;            /* Current PORTE/channel bit */

  status = 0;
  // last_routine = 0x93;
    if (StatusA & STSA_DEBUG)            /* Is DEBUG jumper in place? */
    {
       return 0;                           /* Yes, no shorts/etc. testing */
    }
    if (channel & act_therm_mask)        /* If this channel is optic probe */
    {   
       return 0;                           /*  don't mess with its power! */
    }
    if (ConfigA & CFGA_8COMPARTMENT)
    {
// DHP_DEBUG       point = 0;                          /* Rest of world is 8-channel */
       return 0;                           /* Yes, no shorts/etc. testing */
    }
    else
    {
       point = 2;                        /* USA is only 6-channel */
    }
/*******************************2/25/2008 2:53PM******************************
 * Removed in versiom 1.0A
 *   threshold = (2*ADC1V)/3;              *Thermistors go right to ground *
 * Changed threshold to 1.25v
 *****************************************************************************/
    threshold = ADC1_25V;                /* Thermistors go right to ground */
    ops_ADC (OFF);
    porte_save = LATE;                  /* Save current active channels */
    LATE &= (char)~channel;             /* Shut off selected channel(s) */

    /* Now loop, timing out the supposedly-disabled channel... */
    msdelay = mstimer;                   /* Current millisecond counter */
    while (DeltaMsTimer(msdelay) < ACT_OFF_TIMEOUT)    /* 10 Msec */
    {
       if (read_ADC() == FAILED) return 1;               /* Poll the channels */
       for (i=point; i<8; i++)
       {
          chan_mask = ((unsigned int)1 << i);    /* Next channel */
          if (channel & chan_mask)       /* Shorts-checking this channel? */
          {                              /* Yes */
            if (probe_volt[i] > threshold) /* Probe/channel pulled "high" ? */
              continue;                   /* Yes, keep waiting for low */
          }
          break;                         /* All (selected) channels OK... */
       }
    } /* End timeout on channel powering down */
    LATE = porte_save;                 /* Restore active channels */
   /* By now the selected channel (s) should have quieted down. The ADC
      is actively running/being polled by T3, so probe_volt[] will have
      the last "sampled" values (within 1 MS...), which should be "0" for
      the selected channels which we just shut off. */
    for (i=point; i<8; i++)
    {
       chan_mask = ((unsigned int)1 << i);       /* Next channel */
       if (channel & chan_mask)          /* Shorts-checking this channel? */
       {                                 /* Yes */
          if (probe_volt[i] > threshold) /* Probe/channel pulled "high" ? */
          {                              /* Something is feeding this channel */
             status = 1;            /* Indicate an error was uncovered */
             probes_state[i] = P_SHORT;  /* Latch the bad news (12) */
             two_wire_state = SHORTFAIL_2W;   /* Shut things down */
          }
       }
    }
    ops_ADC (ON);
    return status;
} /* End check_active_shorts() */

/*************************************************************************
 *  subroutine:      check_truck_gone()
 *
 *  function:
 *
 *         Check for the case of a truck that has left
 *         1.  Check that all used channels are returned to the open_c_volt
 *             or approximately there
 *         2.  Check for ground gone
 *         3.  Check for TIM gone
 *
 *  input:  none
 *  output: TRUE/FALSE  (TRUE means GONE)
 *
 *************************************************************************/
unsigned int check_truck_gone(void)
{
unsigned int status;
unsigned int start, i;
#define PROBE_GONE_BIAS   (ADC1V/2)

  if (truck_pulsing)
  {
     return FALSE;                         /* truck still here */
  }
  status = TRUE;                            /* truck gone */
  PORTE = PULSE_TEST;
  PULSE5VOLT = CLR;               /* Chan 4 normal (10V) drive */
  JUMP_START = CLR;               /* Jump-Start always off! */
  DelayMS(10);                              /* Allow voltages to stabilize */
/* >>> Fogbugz 108 check_channels() does the same check as in truck_idle();
    the difference between codes would sometimes call a truck gone that wasn't.
    Test with non-permitting 5-wire IntelliCheck */ 
  {
    if (read_ADC() == FAILED) return TRUE; /* Read 8 voltages in manually */
    if (ConfigA & CFGA_8COMPARTMENT)
    {
       start = 0;
    } else
    {
      start = 2;
    }
    for ( i=start; i<8; i++)
    {
      if ( probe_volt[i] < (open_c_volt[0][i] - PROBE_GONE_BIAS) ) /* same as open */
      {
         status = FALSE;                                        /* truck NOT gone */
      }
    }
  }
  if (status)                         /* No pulsing probes - anything else going on? */
  {  
     if (short_6to4_5to3() )   /*  3,5,8 & 4,6,7 */
     {
        status = FALSE;              /* not gone */
     }
     else
     {
      if (( (groundiodestate & PRESENT_IDLE) != PRESENT_IDLE)  && ( badgndflag == GND_OK ))
      {
          status = FALSE;              /* not gone, handles special cases */
      }else
      {
        if (TIM_state)                     /* Did we have a TIM? */
        {                                  /* Yes */
            if (Read_Truck_Presence() != FALSE) /* Do we still have TIM? */
            {
              status = FALSE;          /* Yes, still connected or unable to check */
              /* If the TIM "changes" this will NOT catch it */
            }
         }
       } 
     }
  }
  if (status)
  {
    status = check_channels ();
  }
  if(probe_try_state == OPTIC5)
  {
    set_porte(OPTIC_DRIVE);
  }
  return(status);
} /* end of check_truck_gone */

/*************************************************************************
 *  subroutine:      short_6to4_5to3()
 *
 *  function:
 *
 *         1.  Check for the case of a truck that has an OnBoarder or Intellicheck probe set
 *             with a Wet probe or open AUX. The probes will NEVER be seen.
 *         2.  Check that channels 3 & 8 are shorted AND 4,6, & 7 are shorted. Channel 5
 *              is shorted to 3 & 8 except at military sites so not checked.
 *         3.  If this is true, it is reasonable to presume that a smart probe
 *             is attached and that it is two wire.
 *         4. Apparently a "DRUM" device exist (or did) that had all channels shorted together
 *              so this pattern is also accepted.
 *         5.  Go Active with two wire optical.
 *
 *          Note: This routine is called from truck_idle() only once every 7+ seconds,  so 
 *                  a SHORT_COUNT of 2 seems appropriate.
 *  input:  none
 *  output: TRUE/FALSE  (TRUE means Smart probe two wire)
 *
 *************************************************************************/
#define  SHORT_COUNT 2

char short_6to4_5to3(void)
{
char           status;
unsigned long  temp_jump;
unsigned int temp_volt[8], i, temp_ops;
unsigned long  time_counter;
static char       short_all = 0, short_5to3 = 0, short_6to4 = 0;
//  last_routine = 0x95;
  service_charge();                    /* Appease watchdog */
  status = FALSE;                     /* default to NO smart probes */
  temp_jump = jump_time;
  jump_time = 0;                       /* shut off JUMP_START */
  temp_ops = T3CONbits.TON;
  ops_ADC(OFF);                   /* shut off T3 and DMA */
  for (i=0; i<8; i++)
  {
    temp_volt[i] = probe_volt[i];
  }
  time_counter = read_time()+MSec2;
  set_porte( OFF );                   /* Output to no probes */
  while (time_counter > read_time())  /* Wait here for time out */
    {;}
  time_counter = read_time()+MSec2;
  set_porte( PIN8 );            /* power up probe 8 */
  while (time_counter > read_time())  /* Wait here for time out */
    {;}
  if (read_ADC() == FAILED) return FALSE;
  // last_routine = 0x95;
  if ((probe_volt[2] > ADC8V) && (probe_volt[4] > ADC8V) && (probe_volt[7] > ADC8V))    /* read probes 3, 5, 8 */
  {
    short_5to3++;               /* 3, 5 and 8 seem to be shorted together */
  }else
  {
    short_5to3 = 0;
  } 
  if ((probe_volt[3] > ADC8V) && (probe_volt[5] > ADC8V) && (probe_volt[6] > ADC8V)) /* read probes 4, 6, 7 */
  {
    if (ConfigA & CFGA_8COMPARTMENT)
    {
      if ((probe_volt[0] > ADC8V) && (probe_volt[1] > ADC8V))   /* read probes 1 & 2 */
      {
        short_all++;            /* 8 compartments shorted! */
      } else 
      {
        short_all = 0;
      }
    }
    else
    {
      short_all++;            /* 6 compartments shorted! */
    }
  }
  time_counter = read_time()+MSec2;
  set_porte( OFF );                   /* Output to probes OFF */
  while (time_counter > read_time())  /* Wait */
  {;}
  time_counter = read_time()+MSec2;
  set_porte( PIN6 );                 /* Output to probe 6 */
  while (time_counter > read_time())
  {;}
  if (read_ADC() == FAILED) return FALSE;
  // last_routine = 0x95;
  if ((probe_volt[3] > ADC8V) && (probe_volt[5] > ADC8V) && (probe_volt[6] > ADC8V))  /* read probe 3, 5, 7 */
  {
    short_6to4++;
  }else 
  {
    short_6to4 = 0;
  } 
  if ( (short_6to4 >= SHORT_COUNT) &&  /* if we've seen the short pattern */
       (short_5to3 >= SHORT_COUNT) )       /* multiple times, then TRUE */
  {
    short_6to4 = SHORT_COUNT;
    short_5to3 = SHORT_COUNT;
    StatusA |= STSA_INTELLICHECK;
    status = TRUE;
  }else
  {
    StatusA &= ~STSA_INTELLICHECK;
    if (short_all>SHORT_COUNT)
    {
     short_all = SHORT_COUNT;
     status = TRUE;                               /* This could be "DRUM" - whatever that is */
    }
  }
  set_porte( PULSE_TEST );    /* return to normal two wire pulse */
  if (temp_ops != 0)
  {
    for (i=0; i<8; i++)
    {
      probe_volt[i] = temp_volt[i];
    }
    ops_ADC(ON);                   /* re-enable T3 and DMA */
  }
  jump_time = temp_jump;        /* return JUMP_START to original */
  service_charge();                    /* Appease watchdog */
  return(status);
} /* end of short_6to4_5to3 */

/*******************************************************************************
 *  subroutine:      check_2wire
 *
 *  function:
 *
 *    See if we have a 2 wire truck.
 *    If voltage on any non 5-wire channel is pulled down it must be a 2-wire.
 *
 ******************************************************************************/
unsigned char check_2wire(void)
{
unsigned char status;

  status = FALSE;
  JUMP_START = CLR;                              /* Disable Jump-Start's +20V */
  DelayMS(20);   
  if (read_ADC() == PASSED)
  {
#if 0
    if (((start_point == 0) && (probe_volt[0] < (open_c_volt[0][0] - ADC_25MV))) ||
         ((start_point == 0) && (probe_volt[1] < (open_c_volt[0][1] - ADC_25MV))) ||
         (probe_volt[2] < (open_c_volt[0][2] - ADC_25MV)) ||
         (probe_volt[6] < (open_c_volt[0][6] - ADC_25MV)) ||
         (probe_volt[7] < (open_c_volt[0][7] - ADC_25MV)))   
    {
      status = TRUE;
    }
    else
    {
      status = FALSE;
    }
#else
    if (start_point == 0)
    {
       if(probe_volt[0] < (open_c_volt[0][0] - ADC_25MV))
       {
          status |= 0x01;
        }
       if(probe_volt[1] < (open_c_volt[0][1] - ADC_25MV))
       {
          status |= 0x02;
        }
    } 
    if(probe_volt[2] < (open_c_volt[0][2] - ADC_25MV))
    {
          status |= 0x04;
    }  
    if(probe_volt[6] < (open_c_volt[0][6] - ADC_25MV))
    {
          status |= 0x40;
    }  
    if(probe_volt[7] <  ADC6V)
    {
          status |= 0x80;
    }  
#endif
  }
  return(status);
} /* end of check_2wire*/

static void check_highI_shorts(void)
{
  unsigned int index;

  for (index = 0; index <= MAX_CHAN; index++)
  {
    HighI_On(index);
  }
  for (index = 0; index < MAX_ARRAY; index++)  /* Clear the binary array */
  {
    probe_array[index] = 0;
  }
  DelayMS(50);
  (void) check_all_oscillating();  /* Check all probes still DRY */
  for (index = 0; index <= MAX_CHAN; index++)
  {
    if(probe_type[index] == P_OPTIC2)
    {
      HighI_Off(index);
    }
  }
}
/************************* end of com_two.c *********************************/

