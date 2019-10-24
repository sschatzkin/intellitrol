/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         shorts.c
 *
 *   Revision:       REV 1.5.27
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *   Description:    This file contains routines which test the 8 probe
 *                   lines for shorts between lines.  The StaticShortTest
 *   applies a walking 0's and walking 1's test.  The DynamicShortTest measures
 *   pulse widths on each of the lines
 *
 *   Channels are numbered from 0 to 7 rather than 1 to 8 as they are
 *   referenced in the drawings.
 *
 * Revision History:
 *   Rev      Date       Who   Description of Change Made
 * -------- ---------  ---      --------------------------------------------
 * 1.5.23   10/19/11  KLL  In StaticShortTest() added the saving of the current
 *                                        probe voltages and restore them after the test as well as
 *                                        clearing out probe_array[]. It was discovered
 *                                        this test was faking out the truck state machine to think the probes were
 *                                        pulsing when they may not.
 *                                       If this test passes set the probes_state to unknown to clear out any
 *                                        previous errors that the test uncovered because the errors have gone away.
 *                                      Removed grnd_flag because the results were never used
 *             04/19/12  KLL  Added message at end of StaticShortTest routine
 *             05/02/12  KLL  Added call xprintf 47 to clear print flags so messages will
 *                                        flow again
 *             05/07/12  KLL  Added a pass condition if SHO Intellicheck is detected
 *                        Removed grnd_flag
 * 1.5.24  07/17/12  KLL  Removed the code that would clear all errors when the shorts test passed.
 *                                       It was discovered that when Thermistors are shorted the shorts test will
 *                                        pass over time. This fakes out the firmware to think that the short has
 *                                        gone away.
 *                                       Previous versions of firmware would determine if the Intellitrol is connected
 *                                        to an Intellicheck by looking for all pins shorted together. That is because
 *                                        the wiring pattern defined in the Intellicheck installation manual when or'd
 *                                        together has all the sensors shorted. This release will actually look for the
 *                                        correct pattern to determine if an Intellicheck is connected. This way a truck
 *                                        that has all the sensors shorted together will be detected.
 *                                       Removed all unused variables from the shorts test.
 * 1.5.25   09/07/12  KLL  Save and restore the probe_pulse_old and probe_pulse variables to prevent the
 *                                        truck going away routine from thinking the truck is still connected when it is
 *                                        not.
 * 1.5.27   03/24/14 DHP  Corrected handling of 1st time shorts test failure
 *                                        so permit is allowed if later tests are successful.
 *                                       Increased Probe Short and Probe Power levels to
 *                                        correctly identify connection of certain devices.
 *
 *****************************************************************************/


#include "common.h"
#include "volts.h"

/****************************************************************************/

//static unsigned char shorts[8];         /* Channel-to-Channel shorts */


//#define  FIRST_CHANNEL  0
#define  LAST_CHANNEL   8

/* "Open circuit short" level -- in millivolts. This is the level that will
   be considered an inter-channel short if no truck is connected to the
   channel (it's floating "open"). 2000 mVolt */

//#define  O_SHORTLVL         2000    /*ADC2V*/

/* "Probe short" level -- in millivolts. This is the level above which an
   unpowered channel will be considered shorted to another channel/power
   source. This level must be below normal two-wire probe oscillating range,
   yet high enough not to take "forever" for optic/etc. circuitry to
   power-down (decay internal capacitor/energy storage). Value of 3100
   determined by testing and evaluation of non-SIL Intellitrol v1.0.27 */

#define  P_SHORTLVL         3100    /* 3.1 volts */

/* Grounded level -- in millivolts. Voltages below this point are con-
   sidered "shorted to ground" as opposed to shorted to another channel
   or signal/power source; unpowered channels above this level are con-
   sidered "shorted to channel" (i.e., getting power from some source).
   Empiric observation: for "Dummy" modules this value is around .45 to .54 volts
   on "unpowered" channels... 750 mVolts */

#define P_GRNDLVL           ADC_75MV

/* Zero level -- in millivolts. Voltages at or below this point at indis-
   tinguishable from "0" volts. 100 mVolts */

#define P_ZEROLVL           ADC_10MV

/* Probe level -- in millivolts. Powered channel with probe must have at
   least this level to be considered "OK" -- anything less and something
   must be "shorted to this channel" and dragging the voltage down. This
   level must be well below normal thermistor (etc.) oscillating range.
   (Note that the Liquidometer Optic Dummy however oscillates from rail to
   about 0.85 volts...) 1100 mVolts */

// #define P_POWERLVL          1100    /* (ADC1V + ADC_10V) 1000 + 100 */

/******************************* 7/17/2012 11:35AM ***************************
 * This value is used to determined when a thermistor sensor is high. It is
 * the same voltage used to determine when the thermistor sensor is high when
 * testing for pulsing. The thermistor voltage is chosen because the optic
 * voltage is higher. Tested this with a Liquidometer Optic Dummy with a
 * thermistor and they worked.
 *****************************************************************************/
#define P_POWERLVL          SysParm.ADCTmaxNV    /* Thermistor threshold */

/* Open Powered level -- in millivolts. Open channel (no probe) must have
   at least this level to be considered "OK". (This is distinct from the
   self-test/diagnostic threshold, which is much less forgiving) 4000 mVolts*/

#define O_POWERLVL          4000    /*ADC4V = 4*ADC1V = 4*1000 */

/* Time (in microseconds) duration of powered-pulse (before read_ADC() time)
   for measuring "inter-channel shorts" (appearance of power on "unpowered"
   channel). */

//#define POWER_UP_TIME         100
#define POWER_UP_TIME         500

/* Maximum wait time (in milliseconds) for an individual channel to power-up
   (e.g., rise above "threshold" once power is applied). */

#define POWER_UP_TIMEOUT      300

/* Time (in milliseconds) that a powered-down channel must stay "down" before
   we believe it.

   SP-TO probes behave as follows:

   +10   --+  +--+
           |  |  |<== Normal 1.4 <=> 10 volt oscillations ~20-40Hz
           |  |  |
           |  |  |  |== Enter StaticShortsTest(), turn off JumpStart & 10V
           |  |  |
           |  |  |
           |  |  |     ++ <== Apply POWER_UP_TIME +10V power pulse, hits 1.4-10V
           |  |  |     ||  <== Sample all channels, turn off all power
   +1.4    +--+  +--+  ||    _  <== SP-TO probe output rises ~5-10ms later,
                    |  ||   | --___   slowly decays thereafter (~100-200ms)
   +1.0             =+ |=+  |      ----____
                     | | |  |              ------_________
                     | | |  |
   +0.5              +-+ +--+

*/

#define HOLD_DOWN_TIME      20

/* Timeout (in milliseconds) for all channels to "decay" to "ground" (either
   O_SHORTLVL or P_SHORTLVL above) after channel turned off, on initial entry
   where channels have been powered for a long time (and in particular, with
   JUMPSTART's +20v levels) */

#define ALL_DECAY_TIMEOUT   300

/* Timeout (in milliseconds) for individual channel to decay to ground level
   after being briefly pulsed in main shorts loop */

#define CHN_DECAY_TIMEOUT   300

/*************************************************************************
*       Function Name:      StaticShortTest
*       Function Type:      enum boolean
*
*   Description
*           Checks for shorts by writing a walking 1's or walking 0's pattern
*           to PORTE lines which are used to drive the probe voltages to
*           either 0 volts or 10.6 volts
*
*       Input:                TRUE to test w/truck connected;
*                               FALSE if no truck - all channels open ("Idle" state)
*
*       Returns:            TRUE when short observed; FALSE otherwise.
*                               Arrays probes_state[] is updated to show any shorts or opens,
*                                          probe_array[] is cleared
*
*   Note well:  Calling StaticShortTest() causes power transitions on all
*               the channels, which can fool the backup processor into
*               thinking it sees "oscillations" and permitting, if this
*               routine is called "too often".
*
*************************************************************************/

unsigned int StaticShortTest(bool probe_test)
{
unsigned long delay, htime;
unsigned int  threshold;
unsigned int  i, imsk;
unsigned int  j, jmsk;
unsigned int  nonzero;
unsigned int  point;
unsigned int  shorts_flag = 0;      /* Unpowered chans shorted to power */
unsigned int  open_flag = 0;        /* Unpowered or Grounded channels */
unsigned int  powered_flag = 0;     /* Always-powered channels */
unsigned int  portf_save;
unsigned int  porte_save;
unsigned int  probe_pulse_old_save = probe_pulse_old;
unsigned int  probe_pulse_save = probe_pulse;
unsigned int  save_probes[COMPART_MAX];
unsigned int  intellicheck_low = FALSE;
unsigned int  intellicheck_high = FALSE;
unsigned int  sho_low = FALSE;
unsigned int  sho_high = FALSE;
int timeout;

  if ((StatusA & STSA_DEBUG) || (SysParm.Ena_INTL_ShortNV == 0))
                                       /* Debug jumper installed? */
    return (0);                       /* Yes, return "successfully" always */

  memcpy(save_probes,  probe_volt, COMPART_MAX);
  portf_save = LATF;                 /* Read and save the state of ports */
  porte_save = LATE;
  if (probe_test)
  {                      /* Real probe or open channel test */
    threshold = P_POWERLVL;           /* Truck/Probe connected (1100mV) */
  }
  else
  {
    threshold = O_POWERLVL;           /* Idle, open circuit 4000 mV */
  }
  if (ConfigA & CFGA_8COMPARTMENT)
    point = 0;                        /* 8-compartment configuration */
  else
    point = 2;                        /* USA/6-compartment config */
  /* If truck not connected (idle) Turn OFF A/D interrupts T3 and DMA0 */
  if (!probe_test)                 
  {
    ops_ADC(OFF); 
  }
  JUMP_START = 0;            /* turn off jump start to limit return (0x40) */
  PULSE5VOLT = 0;            /* set optic pulse to 10 volts (1111 1110) */
  LATE &= 0xFF00;            /* All channel drivers OFF */
  /* Jump_start disabled, all channels turned off. Loop till all channels
     drop to "ground" level, or we time out and declare a channel "Always
     Powered" (i.e., shorted to some power source). */
  delay = (read_time() + ALL_DECAY_TIMEOUT); /* Timeout value */
  htime = (read_time() + HOLD_DOWN_TIME);    /* Min hold-down time */

  while ((read_time() < delay)         /* Timeout eventually */
         && (read_time() < htime))     /* Hold down at least this long */
  {
    if (read_ADC() == FAILED) return FAILED; /* Read all channels' voltages */
    for (i=0; i<LAST_CHANNEL; i++)    /* Loop for all channels (8) */
    {
      if (probe_volt[i] > P_GRNDLVL) /* Channel "decayed" to "0" volts? */
        break;                      /* No, break out and loop again */
    }
    if (i != LAST_CHANNEL)            /* All channels dropped? */
      htime = (read_time() + HOLD_DOWN_TIME); /* No, reset hold-down timer */
  }
  if (!probe_test)
  {                                    /* only when Idle state */
    if (read_ADC() == FAILED) return FAILED; /* One last time, for the record */
    for( i=0; i<LAST_CHANNEL; i++ )   /* Check all channels for power */
    {
      if( probe_volt[i] > P_GRNDLVL) /* Un-powered-channel getting power? */
      {                              /* Yes */
        powered_flag |= ((unsigned)1 << i);   /* Mark always-powered channel(s)  */
      }
    }
  }    /* End of if (!probe_test) */
  /* Look for probes shorted to each other (or ground) by powering each
     channel individually and making sure that first each channel powers
     up (if "idle" and no power, then "Un-powered", otherwise "Grounded"
     if a truck/probe connected) and that no other channel is powered up
     ("Shorted") by the powered channel.

     This test basically looks for any single channel that seems to pro-
     vide power/signal to another channel (e.g., a "wire short"). */
  for (i=point; i<LAST_CHANNEL; i++)
  {
    service_charge();       /* DHP DEBUG */
    imsk = ((unsigned)1 << i);            /* Select a channel to turn ON */
    LATE |= imsk;                /* Power-on ("pulse") one channel */
     /* Delay a small bit to allow Intellicheck to come up; normal 2-wire
        optic don't seem to care (they power-up "instantly"); thermistors
        take forever (relatively speaking) to do anything... */
    DelayUS (POWER_UP_TIME);          /* Bit of time to initially stabilize */
     /* Now loop waiting for channel to "power-up". Generally this will
        happen instantly. Liquidometer optic dummy units however sometimes
        need "a bit" to properly come up (empirical observation...) */
    timeout = 0;
    delay = (read_time() + POWER_UP_TIMEOUT); /* Timeout eventually */
    while (read_time() < delay)
    {
      if (read_ADC() == FAILED) return FAILED; /* Read the voltages */
      if (probe_volt[i] > threshold) /* Desired channel come alive yet? */
      {
        timeout = 1;
        break;                      /* Yes, do it for real now */
      }
    }
    LATE &= 0xFF00;                   /* End power "pulse" */
     /* Make sure that the selected channel voltage rose to a "good"
        level. If not, then either the internal driver is open-circuit
        or externally shorted to ground (or maybe two thermistors are
        shorted together, which can drag voltage down to 1.1 or so...) */
    if ((probe_volt[i] <= threshold) || (timeout == 0))  /* "Good" voltage level? */
    {                                 /* No, channel voltage too low */
      open_flag |= imsk;             /* "Trouble" on this channel */
    }
     /* Check for inter-channel shorts (i.e., powering "current" channel
        provides power/signal to another channel) */
    for (j = point; j < LAST_CHANNEL; j++)
    {
      jmsk = ((unsigned)1 << j);               /* Channel.Probe to check */
      if (probe_volt[j] > P_SHORTLVL) /* This channel "powered" ? */
      {
        if (i != j)                  /* Is this channel the currently/powered? */
        {                            /* No, inter-channel short */
          shorts_flag |= (imsk | jmsk); /* Note shorted channels */
        }
      }
    }
    delay = read_time() + CHN_DECAY_TIMEOUT; /* Channel decay timeout limit */
    htime = read_time() + HOLD_DOWN_TIME; /* Min Hold down time */
    while ((read_time() < delay)      /* Timeout eventually */
            && (read_time() < htime))  /* Hold channel down this long */
    {
      if (read_ADC() == FAILED)
      {
        return FAILED; /* Re-read all channels */
      }
      nonzero = 0;                   /* Hope they all are at "zero" */
      for (j = point; j < LAST_CHANNEL; j++)
      {
         if (probe_volt[i] > P_ZEROLVL) /* Any channel above "zero"? */
            nonzero++;                  /* Yes, can't bypass htime */
         if (probe_volt[i] > P_GRNDLVL) /* Any channel still "powered"? */
            break;                      /* Yes, keep waiting */
      }
      if (nonzero == 0)              /* If all channels at "zero" */
         htime--;                    /* Then back off htime timer... */
                                     /* (still ensures "several" samples */
      if (j != LAST_CHANNEL)         /* All channels "un-powered"? */
         htime = (read_time() + HOLD_DOWN_TIME); /* Reset down-timer and try again */
    }
    if (point == 0)                /* 8 compartment */
    {
      if (i==0)                    /* sensors 1, 3, 5, and 8 shorted together */
      {
        if ( shorts_flag == 0x95)
        {
          intellicheck_low = TRUE;
        }else
        {
          if (shorts_flag == 0x85)   /* sensors 1, 3 and 8 shorted together */
          {
            sho_low = TRUE;
          }
        }
      }
      if ( i==1)
      {
        /**************************** 7/18/2012 11:13AM ************************
         * sensors 1, 3, 5, and 8 shorted together and
         * Sensors 2, 4, 6, and 7 shorted together equals 0xFF or Intellicheck
         ***********************************************************************/
        if (shorts_flag == 0xFF)  /* Sensors 2, 4, 6, and 7 shorted together */
        {
          intellicheck_high = TRUE;
        }else
        /**************************** 7/18/2012 11:13AM ************************
         * sensors 1, 3, and 8 shorted together and
         * Sensors 2, 4, 6, and 7 shorted together equals 0xEF or Intellicheck SHO
         ***********************************************************************/
        {
          if (shorts_flag == 0xEF)  /* Sensors 2, 4, 6, and 7 shorted together */
          {
            sho_high = TRUE;
          }
        }
      }
    } else                          /* 6 compartment */
    {
      if (i==2)
      {
        if (shorts_flag == 0x94)  /* sensors 3, 5, and 8 shorted together */
        {
          intellicheck_low = TRUE;
        }else
        {
          if (shorts_flag == 0x84)  /* sensors 3 and 8 shorted together */
          {
            sho_low = TRUE;
          }
        }
      }
      if (i==3)
      {

        /**************************** 7/18/2012 11:13AM ************************
         * sensors 3, 5, and 8 shorted together and
         * Sensors 4, 6, and 7 shorted together equals 0xFC or Intellicheck
         ***********************************************************************/
        if (shorts_flag == 0xFC) /* Sensors 4, 6, and 7 shorted together */
        {
          intellicheck_high = TRUE;
        }else
        /**************************** 7/18/2012 11:13AM ************************
         * sensors 3, and 8 shorted together and
         * Sensors 4, 6, and 7 shorted together equals 0xEC or SHO Intellicheck
         ***********************************************************************/
        {  
          if (shorts_flag == 0xEC)  /* Sensors 4, 6, and 7 shorted together */
          {
            sho_high = TRUE;
          }
        }
      }
    }
  } /* End check all channels for inter-channel short */

  /****************************** 7/18/2012 3:10PM ***************************
   * Now check to see if there is an Intellicheck or SHO attached
   ***************************************************************************/
  if ((intellicheck_low == TRUE) && (intellicheck_high == TRUE))
  {
    StatusA |= STSA_INTELLICHECK;
    shorts_flag = 0;             /* We have onboarder or Intellicheck connected */
  } else
  {
    StatusA &= ~STSA_INTELLICHECK;
  }
  if ((sho_low == TRUE) && (sho_high == TRUE))
  {
    shorts_flag = 0;             /* We have SHO Intellicheck connected */
  }

  /* Restore "normal" Channel drives now to allow time for them to stabilize
      before final reading at end of this routine. */
  LATE = porte_save;                   /* Restore port status */
  LATF = portf_save;
  if (probe_test)                      /* Truck connected or just idle? */
  {                                    /* Truck connected */
     /* "Print" out status info here rather than caller (active_two_wire)
        since here we have details ("Short/Ground") which are glossed
        over by the time active_two_wire() tries to report problems... */
    if (shorts_flag)                  /* If any inter-probe shorts */
    {                                 /* Show combined shorts & grounds */
       xprintf( 40, (shorts_flag | open_flag));
    }
    else if (open_flag)               /* Any probes grounded? */
    {                                 /* No "Short"s, just show "Grnd"s */
       xprintf( 42, open_flag);
    }
  }
  else
  {                                    /* Idle state, no truck connected */
    if (powered_flag)                 /* Any probes that won't turn off? */
    {
      for (i=point; i<8; i++)
      {
        if ( (powered_flag&((unsigned)1<<i)) )
        {
          probes_state[i] = P_WET;   /* LED will stay ON */
        }
      }
      xprintf( 38, powered_flag );     /* Always powered channels... */
    }
    if (open_flag)
    {
      xprintf( 37, open_flag );
    }
  }
  /* If "Idle" state, reset and check for fresh shorts/grounds; for a
     truck connected state, accumulate shorts/grounds (i.e., once a real
     truck/probe/sensor is shorted, "latch" it -- the truck *MUST* disconnect
     before we will permit again). */
  if ((probe_test && !shorts_flag) || !probe_test)
  {
    for (i=point; i<8; i++)
    {
      if (probes_state[i] >= P_FAULT)
      {
        probes_state[i] = P_UNKNOWN; /* Reset for "Idle" test */
      }
    }
  }
  shorts_flag &= ~powered_flag;        /* remove any channels always on */
  if ( shorts_flag | open_flag )       /* If any shorts/grounds */
  {
    if (!probe_test)
    {                                 /* only Idle state */
      xprintf( 36, shorts_flag );    /* Always shorted channels */
    }
    for (i=point; i<8; i++)
    {                           /* Sort out the state of the individual probes */
      imsk = (char)(1 << i);
      if (shorts_flag & imsk)
      {
        probes_state[i] = P_SHORT;
      }
      else if (open_flag & imsk)
      {
        probes_state[i] = P_GROUND;      /* LED will flash */
      }
    }
  }
  /* Now Re/Pre-load the probe_volt[] array with all-channels-driven
     levels so that active_two_wire() calls to check_truck_gone() (or
     anyone else who wants to read the probe_volt[] levels) will see
     good solid open circuit voltages if the truck is truly gone. */
  memcpy(probe_volt,  save_probes, COMPART_MAX);
  for ( i=0; i<MAX_ARRAY ;i++)
  {
    probe_array[i] = 0;         /* so this test will not fake out probes pulsing */
  }

  probe_pulse_old = probe_pulse_old_save;
  probe_pulse = probe_pulse_save;
  return (shorts_flag | open_flag);    /*  return the combined short/open status */
} /* End StaticShortTest() */

/*************************************************************************
*       Function Name:      AllShortTest
*       Function Type:      char
*
*   Description
*           Checks for Intellicheck wiring
*
*       Input:         NONE
*       Input Type:
*
*       Returns:       0 when no onboarder detected, 1 (TRUE) when
*                      onboarder detected
*
*************************************************************************/

unsigned char AllShortTest(void)
{
unsigned long  delay, htime;
unsigned int msdelay;
unsigned int imsk;
unsigned int jmsk;
unsigned int i, j;
unsigned int temp_one;
unsigned int nonzero;
unsigned int point;
unsigned int shorts_flag = 0;      /* Unpowered chans shorted to power */
unsigned int porte_save;
unsigned int portf_save;

   if ((StatusA & STSA_DEBUG) || (SysParm.Ena_INTL_ShortNV == 0))
                                        /* Debug jumper installed? */
      return (1);                       /* Yes, return "successfully" always */
   porte_save = LATE;                   /* Read and save the state of ports */
   portf_save = LATF;
   if (ConfigA & CFGA_8COMPARTMENT)
      point = 0;                        /* 8-compartment configuration */
   else
      point = 2;                        /* USA/6-compartment config */
   JUMP_START = 0;                      /* turn off jump start to limit return */
   PULSE5VOLT = 0;                      /* set optic pulse to 10 volts (1111 1110) */
   LATE = 0;                            /* All channel drivers OFF */
   for (i=point; i<LAST_CHANNEL; i++)
   {
      imsk = ((unsigned)1 << i);            /* Select a channel to turn ON */
      LATE = imsk;                /* Power-on ("pulse") one channel */
      DelayUS (POWER_UP_TIME);
      service_charge();       /* DHP DEBUG */
      /* Now loop waiting for channel to "power-up". Generally this will
         happen instantly. Liquidometer optic dummy units however sometimes
         need "a bit" to properly come up (empirical observation...) */
      msdelay = mstimer;                /* Mark current time */
      while (DeltaMsTimer(msdelay) < POWER_UP_TIMEOUT) /* Timeout eventually */
      {
         if (read_ADC() == FAILED) return FAILED; /* Read the voltages */
         if (probe_volt[i] > P_POWERLVL) /* Desired channel come alive yet? */
            break;                      /* Yes, do it for real now */
      }
      LATE = 0;                         /* End power "pulse" */
      /* Check for inter-channel shorts */
      for (j = point; j < LAST_CHANNEL; j++)
      {
         jmsk = ((unsigned)1 << j);
         if (probe_volt[j] > P_SHORTLVL) /* This channel "powered" ? */
         {
            if (i != j)                 /* Is this chan the cur/pwr chan? */
            {                           /* No, inter-channel short */
               shorts_flag |= jmsk;     /* Note shorted channels */
            }
         }
      }
      delay = (read_time() + CHN_DECAY_TIMEOUT); /* Channel decay timeout limit */
      htime = (read_time() + HOLD_DOWN_TIME);    /* Min Hold down time */
      while ((read_time() < delay)             /* Timeout eventually */
             && (read_time() < htime))         /* Hold channel down this long */
      {
         if (read_ADC() == FAILED) return FAILED; /* Re-read all channels */
         nonzero = 0;                   /* Hope they all are at "zero" */
         for (j = point; j < LAST_CHANNEL; j++)
         {
            if (probe_volt[i] > P_ZEROLVL) /* Any channel above "zero"? */
               nonzero++;                  /* Yes, can't bypass htime */
            if (probe_volt[i] > P_GRNDLVL) /* Any channel still "powered"? */
               break;                      /* Yes, keep waiting */
         }
         if (nonzero == 0)              /* If all channels at "zero" */
            htime--;                    /* Then backoff htime timer... */
                                        /* (still ensures "several" samples */
         if (j != LAST_CHANNEL)         /* All channels checked? */
            htime = (read_time() + HOLD_DOWN_TIME); /* No - reset down-timer */
      }
   } /* End check all channels for inter-channel short */
   /* Restore "normal" Channel drives now to allow time for them to sta-
      bilize before final reading at end of this routine. */
   LATE = porte_save;             /* Restore port status */
   LATF = portf_save;
   temp_one = 0;                     /* Init for count of "one" bits */
   for (i=point; i<8; i++)           /* Sum up only shorts (no grounds) */
   {
      if (shorts_flag & ((unsigned)1 << i))
      {
         temp_one++;
      }
   }
   if ( (temp_one+point) >= 8 )    /* do we have 6/8 compartment onboarder */
   {                               /* (point = 0 or 2 */
                                   /* if the sum of shorts >= 8 we have an onboarder */
      shorts_flag = 1;             /* We have onboarder or dummies connected */
   }
   else
   {
      shorts_flag = 0;             /* discard test results - we are not realy looking */
                                   /* for "random" shorts here */
   }
   if (read_ADC() == FAILED) return FAILED;
   return ((unsigned char)shorts_flag);    /*  return short status */
} /* End AllShortTest() */

/***********End of SHORTS.C**********/
