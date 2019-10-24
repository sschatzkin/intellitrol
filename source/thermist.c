/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         thermist.c
 *
 *   Revision:       REV 1.5.27
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009,2010  Scully Signal Company
 *
 *  Description:  Main Rack controller main microprocessor PIC24HJ256GP210
 *                Thermister Probe handler
 *
 * Revision History:
 *   Rev      Date       Who   Description of Change Made
 * -------- ---------  ---      --------------------------------------------
 *  1.5.23  05/11/12  KLL  Moved thermistor_present() from two_wire_thermal() and
 *                          put it as part of the check_all_pulses() procedure.
 *                         Deleted reset_bypass() call from thermal_setup(),
 *                           this is now called from tuck_idle at connect time.
 *
 *****************************************************************************/
#include "common.h"
#include "volts.h"   /* A/D voltage definitions */
/*************************************************************************
 *  subroutine:      thermal_setup()
 *
 *  function:  Setup up the thermal probes for 6/8 compartment trucks
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/
void thermal_setup(void)
{
  // last_routine = 0x8B;
   set_main_state (ACTIVE);
   truck_state = THERMAL_TWO;
   acquire_state = IDLE_I;
   probe_try_state = THERMIS;

   jump_time = SEC20;      /* jump_time is 20 seconds */
   dry_timer  = 0;         /* dry probe operation only */

   two_wire_state  = NO_TEST_2W;

   badgndflag |= GND_INIT_TRIAL;

   two_wire_start();       /* set mux and power */

   xprintf( 26, DUMMY );

   if (ConfigA & CFGA_8COMPARTMENT)  /* Report the number of compartments the Truck should be */
   {
    number_of_Probes = 8;
   }
   else
   {
    number_of_Probes = 6;
   }
   display_probe();
   return;
} /* end of thermal_setup */

/*************************************************************************
 *  subroutine:      two_wire_thermal()
 *
 *  function: This is the final type of probe tested for.
 *            The thermal probes are presumed to be cold.  A warmup period
 *            is allowed for the oscillations to begin. Thermal presence
 *            is noted by a dropping probe voltage to approx. 7 volts.
 *            Oscillating probe signals are looked for on the appropriate
 *            pins.  If the inappropriate response is seen, opens, shorts,
 *            or wet is tested.  If we cannot determine that this is a thermal
 *            type probe, we will mark it as not found.
 *
 *  input:  none
 *  output: TRUE/FALSE
 *
 *************************************************************************/
char two_wire_thermal(void)
{
char  status;

  // last_routine = 0x8D;

  status = check_all_pulses(THERMIS);    /* See if theProbes are Oscillating */

  two_wire_state = PULSING_2W;

  return (status);

} /* end of two_wire_thermal */

/*************************************************************************
 *  subroutine:      thermistor_present()
 *
 *  function:
 *         1.  Read the probe voltage.  If most channels are falling
 *             presume that this is a thermistor setup.
 *
 *  input:  none
 *  output: TRUE/FALSE
 *
 *************************************************************************/
char thermistor_present(void)
{
char  status;
int   probe;
int   hi_count=0;
int   low_count=0;

  // last_routine = 0x8C;
  status = FALSE;

  for (probe=2; probe<MAX_CHAN; probe++ )   /* skip first 2 channels */
  {
    if ((probe_volt[probe]<ADC8V) && (probe_volt[probe]>ADC6V))
      hi_count++;
    if ((probe_volt[probe]<ADC4V) && (probe_volt[probe]>ADC3V))
      low_count++;
  }
  if ((hi_count > 1) || (low_count > 1))
  {
    status = TRUE;
  }
  return(status);

} /* end of thermistor_present */

/************************** end of THERMIST.C *******************************/
