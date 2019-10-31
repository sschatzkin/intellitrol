/*******************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         optic5.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *  Description:  Main program file filler for Rack controller main microprocessor
 *                PIC24HJ256GP210. 5 Wire Optic Probe Routines.
 *
 * Revision History:
 *   Rev      Date    Who   Description of Change Made
 * -------- --------  ---  --------------------------------------------
 *  1.5.23  06/26/12  KLL  Added five_wire_gone() routine to test that the
 *                          truck has left. There was a problem in previous
 *                          releases that the firmware would not know the
 *                          truck has left. It is called during the testing of
 *                          the diagnostic line.
 *                         In scully_probe() added unknown to the sensors after the
 *                          WET was identified and DRY to all the sensors before the
 *                          the WET one. This is for logging the dome out.
 *  1.5.30  08/10/14  DHP  Replaced GroundDiodePresent() with CheckGroundPresent()
 *          09/04/14       In scully_probe(), added else in front of statement
 *                          making probes_state = P_DRY to prevent changing the
 *                          P_WET after setting P_UNKOWN.
 *                          In check_diag(), changed tank_time from 1 sec to 2 -
 *                           tank_time was always expired and scully_probe() was
 *                           never called.
 *                          In active_5wire(), add set_porte(PULSE_TEST) when
 *                           changing state to DIAG; this was done in scully_probe()
 *                           but appears to need the extra time for probe_volt
 *                           to rise from the 0v it is at.
 *  1.5.31  01/14/15  DHP  Changed number_of_Probes in optic_5_setup() to calc_tank()-1 
 *                         Fix setting of probes_state[] on wet probe:
 *                          | check_diag() updated wet_tank on 1st call but the array 
 *                          | only when wet_tank = last_wet which can only happen on
 *                          | 2nd call.  Prior to the 2nd call, scully_probe() logged the 
 *                          | dome_out but with an array which still shows everything dry.
 *                          | Now array is updated on 1st call to check_diag()
 *  1.6.33  09/03/15  DHP  Changed check_diag() to always set a probe_state[] to 
 *                           P_WET. If diagnostic line changed then array may
 *                           have been left with all set to P_DRY.
 *  1.6.34  10/10/16  DHP  Fogbugz 108: IN active_5wire() added setting of flag
 *                          dry_once and call to dry_probes() when check_echo()
 *                          returns TRUE.
 *                         In optic_5_setup() added delay before calling 
 *                          try_five_wire() to ensure sensors have recovery time 
 *                          and shortened the delay following. Removed call to
 *                          unknown_probes() as this is handled elsewhere.
 *                         In check_diag() changed to always call scully_probe()
 *                          and to correctly set all 16 probe_state values.
 *                         In check_5wire_fault() added addition checks to 
 *                          correct intermittent misses with Intellichecks.
 *                         In calc_tank() added delay before pulsing to ensure
 *                          sensors have recovery time.
 *                         In scully_probe() added call to ops_ADC(ON) and
 *                          corresponding ops_ADC(OFF) and an extra unused
 *                          read to ensure correct results.
 *                         In active_5wire replaced call to five_truck_gone()
 *                          with call to check_truck_gone() which does the same
 *                          checks for TIM and ground plus voltage checks.
 *                         Deleted now unused five_truck_gone().
 *                         Deleted reset_bypass() call from optic_5_setup(),
 *                           this is now called from tuck_idle at connect time.
 *******************************************************************************/
#include "common.h"
#include "volts.h"

/****************************************************************************/
static   unsigned long  compute_time;
//static int report_flag;           /* used to report a WET once per load */
/****************************************************************************/

/*************************************************************************
 *  subroutine:      check_time()
 *
 *  function:   Test for the case of the optical pulse bit to be
 *              either set or cleared (ie, edge detect).
 *         1.   Called by check_echo & optic_5_pulse
 *         2.   Reads the ADC value from AN5 and compares it to > ADC4V
 *  input:  + or - edge to detect
 *  output: True/False
 *
 *************************************************************************/

char check_time( char edge )
{
char   status;

  status = FALSE;
  // last_routine = 0x34;
  if (read_ADC() == FAILED)
  {
    printf("Restart the Probe interrupts\n\r");
    Init_ADC();
    return status;
  }
  if (edge == '+')                 /* looking for a rising edge */
  {
    /* Require "echo" return pulse to be a higher voltage than we send out
       on Ch 4.
       With 8.2V jumper installed, return pulses are around 5.2V minimum
       observed in a 16-probe string, around 5.5V with just one optic probe
       in a "string" */
    if (probe_volt[5] > OPTIC5IN_MIN)      /* 4900 Mv */
    {
      status = TRUE;
    }
  }else
  if (edge == '-')                          /* looking for a falling edge */
  {
    if (probe_volt[5] < OPTIC5IN_TRAIL)    /* Use hysteresis, look for 3000 Mv */
    {
      status = TRUE;
    }
  }
  return( status );
}   /* end of check_time */

/*************************************************************************
 *  subroutine:      try_five_wire()
 *
 *  function:
 *            A pulse is delivered to the probe interface and a response
 *            is looked for.  If no response is seen, a FALSE is returned
 *  input:  none
 *  output: TRUE (return pulse seen), FALSE (no return pulse)
 *
 *************************************************************************/

char try_five_wire( void )
{
   char           status;

  // last_routine = 0x35;
   optic_5_pulse();
  // last_routine = 0x35;
   status = check_echo();
  // last_routine = 0x35;
   return(status);
} /* end of try_five_wire */

/*************************************************************************
 *  subroutine:      five_wire_optic()
 *
 *  function: This is the first type of probe tested for.
 *            A pulse is delivered to the probe interface and a response
 *            is looked for.  If no response is seen, opens, shorts, (or wet)
 *            is tested.  If we cannot determine that this is an optical 5
 *            type probe, we will move on to optical 2.
 *  input:  none
 *  output: TRUE (5-wire connection) / FALSE (Probably a 2-wire)
 *
 *************************************************************************/

#define DRY_5WIRE 3    /* note that the sampling ends at 20 for 1 sec */
#define WET_5WIRE 5

char five_wire_optic( void )
{
  char           status;

  status = FALSE;
  xprintf( 23, DUMMY );                  /* "   Try 5 wire Optic     " */
  if ( probe_time < read_time() )
  {
    probe_time = (read_time() + 30);  /* set for 30 ms repeat minimum */
    if ( optic5_state == NO5_TEST )
    {
       dry_pass_count = 0;    /* use dry pass counter to assure dry 5 wire */
       wet_pass_count = 0;    /* use wet pass counter to assure 5 wire */
    }
    optic5_state = PULSED;
    if (try_five_wire() != 0) /* setup five wire pulse */
    {
      dry_pass_count++;          /* see optical probe at least 3 times */
      if ( dry_pass_count >= DRY_5WIRE )
      {
        optic5_state = ECHOED;  /* we got 5 wire optics !!! */
        status = TRUE;
      }
    }
    else
    {
      dry_pass_count = 0;           /* don't let it fool you */
      if (check_5wire_fault(FALSE) != 0) /* see if a wet 5 wire */
      {
        wet_pass_count++;          /* see wet probe at least xx times */
        if ( wet_pass_count > WET_5WIRE ) /* in a sequence */
        {
            optic5_state = DIAG;
          status = TRUE;
        }
      }
      else
      {
        wet_pass_count = 0;        /* don't let it fool you */
      }
    }
  }
  // last_routine = 0x36;
  return(status);
} /* end of five_wire_optic */

/*************************************************************************
 *  subroutine:      active_5wire()
 *
 *  function:
 *
 *
 *         1.  Upon entry the truck type is determined but the status of
 *             the probe SYSTEM is indeterminate.
 *         2.  The tank_state is set for the specific probe analysis
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/
void active_5wire( void )
{
  // last_routine = 0x37;
  if ( probe_time < read_time() )
  {
     probe_time = (read_time() + 95);  /* set for appr  10.5 Hz repeat minimum */
                                     /* to comply with power requirements per new */
                                     /* European Spec.*/
     switch (optic5_state)
   {
      case NO5_TEST:
      case PULSED:
      case ECHOED:
      /* When wet will toggle between first state and DIAG */
         optic5_state = PULSED;
         optic_5_pulse();
  // last_routine = 0x37;
         if (check_echo() != 0)
         {
            optic5_state = ECHOED;
            tank_state = T_DRY;     /* It's Dry, set the state */
            dry_once = TRUE;         /* FogBugz 108 */
            jump_time  = 0;         /* reset the JUMP_START to 0 */
            if ( number_of_Probes == 0)  /* In case we are going from wet to dry */
            {
              number_of_Probes = 1;
            }
            dry_5W_probes();           /* initialize 5w probes*/
  // last_routine = 0x37;
            wet_pass_count = 0;     /* reset the filter */
         }
         else
         {
           dry_timer = 0;          /* reset for wet probe operation */
           wet_pass_count++;
           if (wet_pass_count > 2) /* allow for a few faults */
           {
             optic5_state = DIAG;
             if ( tank_state == T_DRY)
             {
               overfill_count = 0;
             }
             overfill_count++;
             tank_state = T_WET;  /* It's not pulsing, set it Wet */
           }
      /* WPW: reduce the cycle to handle multiple probe conflict */
           else if (wet_pass_count == 1)
           {
             probe_time -= 15;             //Joe was 10Msec changed to 15
           } 
           else if (wet_pass_count == 2)
           {
              probe_time -= 25;
           } 
         }
      break;
        case DIAG:
         optic5_state = NO5_TEST;
         if (check_diag() != 0)  /* determine probes Wet */
         {                          /* or truck Gone */
           wet_pass_count++;
           if (wet_pass_count > 8)    /* Allow ~2 seconds... */
           {
              if (check_truck_gone())   /* Test to see if truck has gone away */
              {
                xprintf( 47, 3 );       /* Force the next message to print */
                truck_state = DEPARTED;
              }
              else
              {
                 Nop();
              } 
           }
         }
  // last_routine = 0x37;
         if (tank_state == T_WET)            /* this will allow Civicon probes to settle */
         {
           probe_time = (read_time() + 500);  /* set for 500 ms repeat minimum */
         }
         break;
         default:
	     break;
     }
  }
} /* end of active_5wire */

/*************************************************************************
 *  subroutine:      optic_5_setup()
 *
 *  function:  Setup up the ADC to handle the 5 wire optic probes
 *
 *  input:  none
 *  output: none
  *
 *************************************************************************/
void optic_5_setup( void )
{
  unsigned int i;
  int probe_flag = FALSE;

  lowVolt = 9999;
  // last_routine = 0x38;
  ledstate[OPTIC_OUT] = DARK;   /* In case where updater has them flashing */ 
  ledstate[OPTIC_IN] = DARK;
  set_main_state (ACTIVE);
  truck_state = OPTIC_FIVE;
  acquire_state = IDLE_I;
  optic5_state = NO5_TEST;
  probe_try_state = OPTIC5;
  jump_time = 0;                 /* JUMP_START off always */
  dry_timer = 0;                 /* dry probe operation only */
  badgndflag |= GND_INIT_TRIAL;
  set_porte( OPTIC_DRIVE );     /* assure setup for 5 wire optic pulsing */
  set_mux( M_PROBES );           /* assure set mux to probes */
  ops_ADC( OFF );                /* shut off ADC timer interrupt (T3) */
  xprintf( 25, DUMMY );
  number_of_Probes = 0;
  /***************************** 12/5/2008 7:17AM **************************
   * Test how many probes on this truck might take a few tries
   *************************************************************************/
  for (i = 0; i < 5; i++)
  {
    DelayMS(30);                        /* ensure minimum period */
    if (try_five_wire() == TRUE)
    {
      probe_flag = TRUE;
      number_of_Probes = (unsigned int)(calc_tank() - 1);
      DelayMS(100);                     /* wait maximum period */
      service_charge();                 /* Appease watchdog */
    }
  }
  if (probe_flag == TRUE)         /* Return pulse seen; dry vehicle */
  {
      if (number_of_Probes < 1 || number_of_Probes > 16)
      {
          number_of_Probes = 1;        /* Might be a IntelliCheck or no diag line */
      }
      
      display_probe();
      dry_5W_probes();
  }
  else
  {
    /***************************** 7/22/2011 8:16AM **************************
     * Probes are wet
     *************************************************************************/
      number_of_Probes = (unsigned int)(calc_tank());
  }
  
  lowVolt = 9999;
} /* end of optic_5_setup */

/*************************************************************************
 *  subroutine:      optic_5_pulse()
 *
 *  function:  This simple call outputs an 1500 usec pulse to the pin 4
 *             5 wire optic probe.        ______
 *                                >______|      |__________>
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/

void optic_5_pulse( void )
{
unsigned long counter;
char edge = FALSE;

  // Check if pulse already Present, for the case where there are 2 Intellitrols
  // last_routine = 0x39;
  if (read_ADC() == FAILED)
  {
    Init_ADC();
    if (read_ADC() == FAILED)
    {
      printf("\n\r1: Trouble reading the Analog Port\n\r");
      return;
    }
  }
  // last_routine = 0x39;

  if (probe_volt[5] > OPTIC5_CHECK_FOR_PULSE)      /* 3500 Mv */
  {
     xprintf(28, DUMMY);  //"\n\r  Pulse Detected, Delaying          "
     DelayMS (15);
  }

  compute_time = (read_time() + (MSec*3));
  opt_return.rise_edge = 0;  /* clean out the structure dynamics */
  opt_return.fall_edge = 0;
  TMR7HLD = 0;
  TMR6 = 0;
  counter = read_32bit_realtime();
  // last_routine = 0x39;
  opt_return.base_count = counter;    /* get base count */
  Init_Timer4();          /* Setup 1.5ms counter and Output optic pulse of ~1500 us (0xDF) */
  // last_routine = 0x39;
  set_porte( OPTIC_PULSE );             /* Output optic pulse of ~1500 us  */
  while ( compute_time > read_time() )  /* ~3 Msec loop */
  {
    if (!edge)                    /* Will be set TRUE after positive edge detected */
    {
      if ( check_time( '+' ) == TRUE)    /* Do we see positive edge? */
      {
        edge = TRUE;              /* Yes, positive edge detected */
        opt_return.rise_edge = read_32bit_realtime();    /* Save time */
      }
    }
    else                          /* Now we are looking for negative edge */
    {
      if ( check_time( '-' ) == TRUE)    /* Do we see negative edge? */
         opt_return.fall_edge = read_32bit_realtime();    /* Yes, mark time */
      break;
    }
    if ( !T4CONbits.TON)          /* Did the 1500 uSec. timer interrupt happen ? */
    {
        /* The Timer 4 interrupt routine will shut off optic pulse */
       break;                     /* Yes, quit the loop */
    }
  }

  /*******************************************************************
   * In case the loop finishes before the interrupt
   *******************************************************************/
  IFS1bits.T4IF = 0;
  T4CONbits.TON = 0;      /* Turn off Timer 4 */
  set_porte( OPTIC_DRIVE );  /* shut off optic pulse */
  if (main_state != IDLE)
  {
     ledstate[OPTIC_OUT] = PULSE;      /* Tell Trucker we just PULSED */
     StatusO |= STSO_5WIRE_PULSE;      /* Tell TAS/VIPER we just PULSED */
  }
} /* end of optic_5_pulse */

/*************************************************************************
 *  subroutine:      check_echo()
 *
 *  function:   on a five wire optical probe system test that an echo is
 *              returned.  Check that this echo is within the limits
 *              prescribed.
 *
 *         1.   NOTE: There is at least a 200 usec latency from optic_pulse
 *                    return until the first level (or edge) can be seen...
 *         2.         The pulse width is 800usec
 *
 *  input:  none
 *  output: TRUE/FALSE
 *
 *************************************************************************/
char check_echo( void )
{
char           status;

  // last_routine = 0x3A;
  status = FALSE;
  compute_time = (read_time() + (MSec*15));
  while (compute_time > read_time())
  {
     if (opt_return.rise_edge == 0)
     {
      if (check_time( '+' ))
      {
         opt_return.rise_edge = read_32bit_realtime();
         break;                  /* done + edge, exit */
      }
     }
     else
      break;
  }
  while (compute_time > read_time())
  {
     if (opt_return.fall_edge == 0)
     {
      if (check_time( '-' ))
      {
         opt_return.fall_edge = read_32bit_realtime();
         break;                  /* all done, exit */
      }
     }
     else
      break;
  }
  if ( (opt_return.rise_edge == 0) ||
       (opt_return.fall_edge == 0) )
  {
     return(status);
  }
  /* is the rising edge > the base count */
  if (opt_return.rise_edge > (unsigned long)opt_return.base_count)
     opt_return.rtn_pulse = opt_return.rise_edge - (unsigned)opt_return.base_count;
  else
     opt_return.rtn_pulse = ((unsigned long)0xFFFFFFFF - (unsigned long)opt_return.base_count) + opt_return.rise_edge;

  /* is the falling edge > the rising edge */
  if (opt_return.fall_edge > opt_return.rise_edge)
     opt_return.pulse_width = opt_return.fall_edge - opt_return.rise_edge;
  else
     opt_return.pulse_width = ((unsigned long)0xFFFFFFFF - opt_return.rise_edge) + opt_return.fall_edge;

  if ( opt_return.rtn_pulse > 0 )
  {                                   /* pulse after out */
    if ( opt_return.rtn_pulse < (unsigned long)(RMSec*6) )         /* pulse before 6ms */
    {
      if ( opt_return.pulse_width < (unsigned long)(RMSec*12) )     /* return width < 12ms */
      {
        if ( opt_return.pulse_width > RUSec500 )                  /* return width > 500us */
        {
          status = TRUE;
        }
      }
    }
  }

  /* We have just received an "echo" aka "Optic In" pulse. Pulse the LED
     to indicate this. Since active 5-wire-optic cycles at 50ms intervals,
     and the LEDs at 125ms, this should result in a steady green optic
     return LED if operating "dry", and will take "several" missed pulses
     to extinguish the LED.

     HACK: Suppress indication (both unit LED and TAS/VIPER) if this is
     the idle loop's playing with the signals -- only flash the LED
     when a truck is connected (includes "Acquire" state). */

  if (main_state != IDLE)
  {
    ledstate[OPTIC_IN] = PULSE;    /* Tell Trucker we got a return pulse */
    StatusO |= STSO_5WIRE_ECHO1;   /* Tell TAS/VIPER ... */
  }
  return(status);
} /* end of check_echo */

/*************************************************************************
 *  subroutine:      check_diag()
 *
 *  function:
 *         1. Update probe_state[] every second; wait no longer than 5 seconds.
 *         2. -Always leave with 1 probe_state entry set to P_WET
 *         3. -If different compartment is wet, set previous to dry
 *         4. -Update flags and varibles for next check
 *         5. When not updating  probe_state[] check for truck gone via scully_probe()
 *
 *  input:  none
 *  output: True/False        TRUE == truck departed
 *
 *************************************************************************/
char check_diag( void )
{
//   static char       last_wet = 0;   /* force a second opinion always */
  char                 status;
  unsigned int    wet_tank;
  unsigned int    index;

  // last_routine = 0x3B;
//   status = TRUE;                   /* set default to gone */
  if ( (tank_time < read_time()) ||
      (tank_time > (read_time()+SEC5)) )        /* Over 5 Sec. since last time here */
  {
    tank_time = (read_time() + SEC1);            /* Set for 1 second timing */
    if ((wet_tank = calc_tank()) < (2*MAX_CHAN))          /* 2 x 8 = 16 */
    {
// >>> FogBugz108 !!! handle probe_state 0
      if (wet_tank > 1)
      { 
        for (index=1;index<wet_tank;index++)
        {
          probes_state[index-1] = P_DRY;               /* set lower probes DRY*/
        }
        probes_state[index-1] = P_WET;                 /* set wet probes WET */
        for (;index <(2*MAX_CHAN) ;index++)
        {
           probes_state[index] = P_UNKNOWN;  /* set higher probes UNKNOWN */
        }
      }
      else
      {
        probes_state[0] = P_WET;                 /* set wet probes WET */
        for (index=1; index <(2*MAX_CHAN); index++)
        {
           probes_state[index] = P_UNKNOWN;  /* set higher probes UNKNOWN */
        }
      } 
// <<< FogBugz108
//       }
//      last_wet = wet_tank;                     /* reset retrial */
    }
//    status = FALSE;                             /* set to still here */
    tank_time = read_time();                 /* Recheck next cycle */ 
  }
//   else
  {
    status = scully_probe();                     /* Has truck left ??? */
  }
  return(status);
} /* end of check_diag */

/*************************************************************************
 *  subroutine:      check_5wire_fault()
 *
 *  function:
 *
 *         1. See if we have a 5 wire truck with a wet tank
 *         2.  or a probe that is malfunctioning
 *         3.  or none of the above
 *             For a 5 wire unit
 *         4. Voltage on channel 3 (pulse out) will be pulled down at least??? 200mv.
 *         5. Non powered channel 5 (pulse in) will have at least mv.
 *         6. Voltage on channel 2 and 6 (open) stay at open_c_volt
 *              If the above is true:
 *         7.  Channel 4 (diag) at open channel voltage means Intellicheck
 *         8. If Intellicheck and channel 7 near open channel then Intellicheck 2
 *             1 Volt is about 1241 counts at 12 bit for 21.6 volt resolution
 *             allow 500mv ~ 24
 *
 *
 *  input:   TRUE/FALSE   (True means to do the two wire test)
 *  output: TRUE/FALSE   (TRUE with TRUE  input means we probably have a 2-wire unit)
 *                                    (TRUE with FALSE input means we probably have a 5-wire unit)
 *
 *************************************************************************/
#define SAMPLES 4         /* number or repeats to catch false 5 wire */

char check_5wire_fault(char do_two_wire)
{
char           status;
unsigned char  index, i;
unsigned long  sample[6];
unsigned long  open_3volt, open_7volt;
unsigned long  powr_8volt;
unsigned int open_3volt_good, open_7volt_good, powr_8volt_good;

  // last_routine = 0x3C;
  open_3volt   = 0;
  open_7volt   = 0;
  powr_8volt   = 0;
  status = FALSE;
  for (index=0;index<6; index++)
  {
    sample[index] = 0;
  }
  for (index=0; index<SAMPLES; index++ ) /* DSP filter the channel 3 open line */
  {                                      /* DSP filter the channel 8 ???? line */
    DelayMS(30);                          /* Wait minimun pulse period */
    optic_5_pulse();                     /* MAX draw ONLY while pulsing */
    if (read_ADC() == FAILED)
    {
      printf("\n\r2: Trouble reading the Analog Port\n\r");
      Init_ADC();
      return TRUE;        /* Since we can't read the voltage we call the probes wet */
    }
    // last_routine = 0x3B;
    if (do_two_wire)
    {
      if ( (probe_volt[2] < ADC8V) ||
           (probe_volt[6] < ADC8V) ||
           (probe_volt[7] < ADC6V) )
      {
        status = TRUE;
        xprintf( 119, DUMMY );
        optic_2_setup();
        // last_routine = 0x3B;
        tank_state = T_INIT;
        badvipflag = 0;  /* If truck connected have it look through the list again */
        return(status);             /* on level below 8 volts, NOT 5 wire */
      }
    }
    open_3volt += probe_volt[2];
    open_7volt += probe_volt[6];
    powr_8volt += probe_volt[7];
    {
      sample[0] += probe_volt[2];   /* Pin 3 */
      sample[1] += probe_volt[3];   /* Pin 4 */
      sample[2] += probe_volt[4];   /* Pin 5 */
      sample[3] += probe_volt[5];   /* Pin 6 */
      sample[4] += probe_volt[6];   /* Pin 7*/
      sample[5] += probe_volt[7];   /* Pin 8*/
    }
  }
  open_3volt /= (index);           /* OPEN circuit voltage */
  open_7volt /= (index);           /* OPEN circuit voltage */
  powr_8volt /= (index);           /* OPEN ??? circuit voltage */
  for (i=0; i<4; i++ )
  {
    sample[i] /= SAMPLES;
  }
  powr_8volt_good = (unsigned int)open_3volt - (2 * ADC_10MV);
  open_3volt_good = (open_c_volt[0][2] - ADC_50MV);
  open_7volt_good = (open_c_volt[0][6] - ADC_50MV);

/* Bring in tolerance (newer power supplies more more stable and consistent), */
/*  and one probe really is around 1/3 volt... */
  if ((powr_8volt < powr_8volt_good) /* 2 * Pin 8 pulled low */
      && (open_3volt > open_3volt_good)  /* Pin 3 high/rail */
      && (open_7volt > open_7volt_good)) /* Pin 7 high/rail */
  {
    status = TRUE;                         /* Likely 5 wire */
  }else
  if ((sample[0] > (open_c_volt[0][2] - ADC_50MV)) &&   /* Pin 3  high/rail */
      (sample[1] < (open_c_volt[0][3] - ADC_15MV)) &&   /* Pin 4 < high/rail */
      (sample[3] > ADC_50MV) && (sample[3] <  ADC2V) && /* Pin 6 low */
      (sample[4] > (open_c_volt[0][6] - ADC_50MV)))     /* Pin 7 high/rail */
  {
    status = TRUE;                         /* Likely 5 wire */
  }
  return(status);
} /* end of check_5wire_fault */

 /***********************************************************************\
 * 
 *  subroutine:      calc_tank()
 * 
 *  function
 *
 *         1.  5 wire optical w/68ohm leadin and 4.75kohm resistors 
 *         2.  Calculate the tank number found wet
 *         3.  First get a DSP sample of the open circuit Voltage 
 *         4.  Then a sample of the DIAG voltage (E)
 *         5.  New scheme ala Gary C. uses a special drive for the 5
 *             wire Diagnostic line when a probe is wet.  The open
 *             circuit value for the drive voltage (ReferenceVolt)
 *             is calabrated in the ADC code upon power up
 *  
 *  input:  none 
 *  output: the tank number that's wet
 * 
 \***********************************************************************/
#define TRIALS 10             /* number or repeats to do the averaging */

unsigned long lowVolt = 9999;

unsigned int calc_tank( void )
{
    unsigned long ch5_volt;
    unsigned long ch5_volt_oldTable;
    unsigned long index;
    unsigned int  tank_number;
    unsigned long voltList[17] = {6840, 6630, 6380, 6000, 5690, 5385, 5130, 4890, 4660, 4470, 4270, 4105, 3950, 3795, 3675, 3550, 3440};
    char          error_found = 0;
    
    StatusA &= ~CH5_HIGH_RESISTANCE;
    
    //printf("Using table number %d\n", pSysDia5->updatedADCTable);
    
    DelayMS(30);                              // ensure minimum period
    optic_5_pulse();                          // Pulse optic probe to get reading
    tank_number = 0;
    ch5_volt = 0;
    CH_TEST5 = 0;                             // Turn off Ch 5 (DIAG channel)
    DIAGNOSTIC_EN = 0;                        // Turn on precision DIAG voltage 
  
    if (read_ADC() == FAILED)
    {
        printf("\n\r3: Trouble reading the Analog Port\n\r");
        Init_ADC();
        return 18;                              // Since we can't read the voltage we call it a invalid probe
    }

    for (index = 0; index < TRIALS; index++ )     // Average TRIALS
    {
        if (read_ADC() == FAILED)
        {
            printf("\n\r4: Trouble reading the Analog Port\n\r");
            Init_ADC();
            return 18;                            // Since we can't read the voltage we call it a invalid probe
        } 
        else
        {
            ch5_volt += probe_volt[4];
            optic5_table[index] = probe_volt[4];
        }
    }
  
    CH_TEST5 = 1;
    DIAGNOSTIC_EN = 1;
    ch5_volt /= index;                        // 5-wire-optic diagnostic voltage
    
    // Add offset only for old table
    ch5_volt_oldTable = ch5_volt;
    ch5_volt_oldTable += (unsigned long)pSysDia5->PNOffset;
    ch5_volt_oldTable *= (unsigned long)ReferenceVolt;
    ch5_volt_oldTable /= (unsigned long)1000;
    
    ch5_volt *= (unsigned long)ReferenceVolt;
    ch5_volt /= (unsigned long)1000;
    
    // New Table
    if(pSysDia5->updatedADCTable == 1) 
    {
        if(ch5_volt < lowVolt)
        {
            lowVolt = ch5_volt;
        }
        
        //printf("LOW VOLTAGE: %d\n", (int)lowVolt);
        compare_volts = lowVolt;
        
        for (index = 0; index < 17; index++)
        {
            if (lowVolt <= voltList[index])
            {
                tank_number++;
            }
        }
        
        if(tank_number > 1 && tank_number < 16) {
            if (lowVolt > ((((voltList[tank_number - 1] - voltList[tank_number]) * 25) / 100) + voltList[tank_number])
                    || (lowVolt >= voltList[tank_number] && lowVolt < (voltList[tank_number] + (unsigned long)5))) {
                //printf("Please check sensor connection: %d\n", (int)((((voltList[tank_number - 1] - voltList[tank_number]) * 25) / 100) + voltList[tank_number]));
                StatusA |= CH5_HIGH_RESISTANCE;
                logmaintenanceerr();
            }
        }
        
        //printf("TABLE VALUE %d: %d\n", tank_number - 1, (int)voltList[tank_number - 1]);
        //printf("TABLE VALUE %d: %d\n\n", tank_number, (int)voltList[tank_number]);
    }
    
    // Old Table
    else {
        lowVolt = 9999;
        
        //printf("CHANNEL 5 VOLTAGE: %d\n", (int)(ch5_volt_oldTable));
        compare_volts = ch5_volt_oldTable;
        
        for (index = 0; index < 16; index++)
        {
            tank_number++;
            
            if (pSysDia5 != 0)
            {
                if (ch5_volt_oldTable > pSysDia5->WetVolts[index])
                {
                    error_found = 1;
                    break;
                }
            }
        }
        if (error_found == 0)
        {
            /***************************** 1/22/2010 2:52PM **************************
            * If no error found then it must be for sizing a truck that has 16
            * compartments. Because the result is subtracted by one we must add one to it
            *************************************************************************/
            tank_number++;     /* Because the result is subtracted by one we must add one to it */
        }
    }
    
    return(tank_number);
}


/*************************************************************************
 *  subroutine:      scully_probe()
 *
 *  function:
 *         1.  Scully probe MAY have diagnostic line
 *         2.  Scully probe WILL hold a powered OPTIC_RTN line LOW
 *         3.  Scully probe has slower (sic) rise time on power up
 *         4.  Voltage on channel 8 will rise to open_c_volt on truck leaving
 *         5.  Voltage on channel 2 & 3 stay at open_c_volt w/5 wire
 *
 *  input:  none
 *  output: True/False  (TRUE means the truck departed...)
 *
 **************************************************************************/
char scully_probe( void )
{
char status;
int i;

  // last_routine = 0x3D;
  status = TRUE;                /* set default to gone */
  if (check_5wire_fault(FALSE) != FALSE) /* see if a five wire still out there */
  {
    if (overfill_count==4)  /* Only log it once */
    {
      for ( i=0; i< 16; i++)          /* Copy int array into a character array */
      {
        if (probes_state[i] == P_WET)
        {
          int j = ++i;
          for ( ; j<16; j++)
          {
            probes_state[j] = P_UNKNOWN;      /* Make all probes after the wet one unknown */
          }
          break;
        }
        probes_state[i] = P_DRY;      /* Make all probes upto the wet one dry */
                                    /* all others are unknown */
      }
      log_dome_out();
    }
    status = FALSE;            /* not gone yet */
  }
  // last_routine = 0x3D;
  if ((groundiodestate & PRESENT_IDLE) != PRESENT_IDLE)
  {
    if (badgndflag==FALSE)
      status = FALSE;            /* not gone */
  }

  set_porte( PULSE_TEST );  /* Ensure all channels powered */
  // last_routine = 0x3D;
// >>> FogBugz 108
  ops_ADC( ON);                              /* Ensure automatic sensor ADC */
  if (wait_for_probes() == PASSED)  /* wait for and ignore 1st reading */
// <<< FogBugz 108
  {
    if (read_ADC() == FAILED)
    {
      printf("Restart the Probe interrupts\n\r");
      Init_ADC();
    }
  }else
  {
      Init_ADC();
  }
  // last_routine = 0x3D;
  if (probe_volt[5] < OPTIC5IN_MIN)
  {
    status = FALSE;            /* not gone */
  }
// >>> FogBugz 108
//  ops_ADC( OFF);                              /* turn off automatic sensor ADC */
// <<< FogBugz 108
  set_porte( OPTIC_DRIVE );  /* Restore 5-wire voltages  */

  // last_routine = 0x3D;
  return(status);
} /* end of scully_probe */

/************************* end of optic5.c **********************************/
