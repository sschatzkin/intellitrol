/****************************************************************************
 *
 *  Project:         Rack Controller
 *
 *  Module:        optic2.c
 *
 *   Revision:       REV 1.5.27
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *  Description:  Main Rack controller main microprocessor PIC24HJ256GP210
 *                Two wire Optical Probe handler
 *
 * Revision History:
 *   Rev      Date   Who   Description of Change Made
 * ------- --------  ---  --------------------------------------------
 * 1.5     09/23/08  KLL  Ported Motorola code to the PIC24HJ256GP210 CPU
 * 1.5.27  04/01/14  DHP  In optic_present() added check for sensors 4 and 6
 *                         only to be low before returning TRUE.  The routine
 *                         indicates to two_wire_optic() if 2W optic sensors may
 *                         be present and the code was getting caught in a loop
 *                         looking for dry 2W when an IntelliCheck with 5W output
 *                         was connected with No Permit (IntelliCheck AUX open).
 * 1.6.34  10/10/16  DHP  Fogbugz 108; In optic_present() added an initial
 *                         re-read of sensor voltages and retries before
 *                         committing to 2-wire sensor type.
 *                         Deleted reset_bypass() call from optic_2_setup(),
 *                           this is now called from tuck_idle at connect time.
****************************************************************************/

#include "common.h"
#include "volts.h"   /* A/D voltage definitions */

/****************************************************************************/

/*************************************************************************
 *  subroutine:      optic_2_setup()
 *
 *  function:  Setup up to handle the 2 wire optical probes in
 *             MPD mode for 6/8 compartment trucks
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/
void optic_2_setup(void)
{
  // last_routine = 0x88;
   set_main_state (ACTIVE);      /* Next time through the loop truck */
                                 /* will be active */
  // last_routine = 0x88;
   truck_state = OPTIC_TWO;
   acquire_state = IDLE_I;
   probe_try_state = OPTIC2;

   jump_time = 0;                /* JUMP_START off always */
   dry_timer = 0;                /* dry probe operation only */
   two_wire_state  = NO_TEST_2W; /* Initial state for active_two_wire */
                                 /* called from truck_active */
   badgndflag |= GND_INIT_TRIAL;

   two_wire_start();             /* set mux and power */

   xprintf( 24, DUMMY );

   if (ConfigA & CFGA_8COMPARTMENT)  /* Report the number of compartments the Truck should be */
   {
    number_of_Probes = 8;
   }
   else
   {
    number_of_Probes = 6;
   }
   display_probe();
  // last_routine = 0x88;
//FogBugz108   unknown_probes();
  // last_routine = 0x88;
} /* end of optic_2_setup */

/*************************************************************************
 *  subroutine:      two_wire_optic()
 *
 *  function: This is the 2nd type of probe tested for.
 *            Oscillating probe signals are looked for on the appropriate
 *            pins.  If no response is seen, opens, shorts, (or wet)
 *            is tested.  If we cannot determine that this is an optical 2
 *            type probe, we will move on to thermal (thermistor).
 *  function:
 *
 *  input:  none
 *  output: TRUE (pulse seen - probe detected) / FALSE (nothing found)
 *
 *************************************************************************/
char two_wire_optic(void)
{
char   status;

  // last_routine = 0x89;
   status = check_all_pulses(OPTIC2);   /* See if theProbes are Oscillating */
   if (status == FALSE)
   {
      status = optic_present();                                // look for optic voltage 
   }  
   two_wire_state = PULSING_2W;                   /* Check further on 2w */
   return(status);
} /* end of two_wire_optic */

/*************************************************************************
 *  subroutine:      optic_present()
 *
 *  function:
 *         1.  Read the probe voltage.  If at least 4 are high and 2 low then 
 *             presume that this is an optic setup.
 *
 *  input:  none
 *  output: TRUE if presumed to be optic else FALSE
 *
 *************************************************************************/
#define OPTIC_CNT 1           /* detection limit to see optic probes */

char optic_present(void)
{
char  status;
int   probe;
int   hi_count=0;
int   lo_count=0;
static int   try_count=0;

  // last_routine = 0x8A;
  status = FALSE;

  if (!JUMP_START)        /* We will be faked out if the +20 is on */
  {
    if (read_ADC() == FAILED) /* get a clean read */
    {
      return FALSE;
    }
    if (ConfigA & CFGA_8COMPARTMENT)
      probe = 0;                        /* 8-compartment configuration */
    else
      probe = 2;                        /* USA/6-compartment config */
    for (; probe<MAX_CHAN; probe++ )   /* skip first 2 channels */
    {
      if (probe_volt[probe]>ADC8V)
        hi_count++;
      if (probe_volt[probe]<ADC3V)
      {
        lo_count++;
      }
    }
    if ((lo_count >=1)  && (hi_count >= OPTIC_CNT ))
    {
// >>> FogBugz 108
      /* If a 5-wire then channel 6 will be low and maybe 4 so allow a few 5-wire checks
          before assuming a 2W if those are the lows */
      if (!(((lo_count == 1)  && ( probe_volt[5]<ADC3V)) || 
             ((lo_count == 2)   && ( probe_volt[5]<ADC3V)&& ( probe_volt[3]<ADC3V))))
 //     if (!((lo_count == 2)  && ( probe_volt[3]<ADC3V) && ( probe_volt[5]<ADC3V)))
      {
        status = TRUE;                  /* This is likely a 2W sensor */
        try_count = 0;
      }else
      {
         if(try_count++ > 3 )
         {
            status = TRUE;               /* This is likely a 2W sensor */
            try_count = 0;
         }else
         {
            probe_try_state = NO_TYPE;
         }  
// <<< FogBugz 108
      } 
    }
  }
  return(status);
} /* end of optic_present */

/************************* end of optic2.c **********************************/

