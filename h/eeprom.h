/***********************************************************
*	FILE: eeprom.h
*  Project:   Rack Controller
*
*  Module:       eeprom.h
*
*  Revision:     REV 1.05
*
*
*  Author:       M.Rivin  @Copyright 1993  Scully Signal Company
*
* Constant definitions to be read from the EEPROM
*
*
*
***********************************************************/
#ifndef EEPROM_H
#define EEPROM_H

/* Two Wire Probes*/
/* TIMING DEFINITIONS*/
/* Time in milliseconds after switching to two-wire state and before the
   initial channel/probe shorts test (essentially a "de-bounce" period).
   Unfortunately, on-boarder seems to need a coupla seconds sometimes!
   (see ONBOARD_GRACE below), so keep this "blind wait" small for
   everything else that works! */

#define N_CYCLES_250  250       /*CYCLE_TIME_2W_DEBOUNCE ulong */

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

#define N_CYCLES_67   67    /* TRUCK_GONE_GRACE */

/* Grace period (in milliseconds) in which to allow a Fault-detected unit
   to "un-fault" itself. This is a ***HACK*** to allow the "onboarder"
   units to connect -- empirically it is observed (Ha!) that "it takes a
   while" (contact bounce? Cosmic Ray flux?) to properly ascertain that
   all contacts are shorted and thus it's really OK. What a Krock! */

#define N_CYCLES_20250    20250         /* ONBOARD_GRACE */

/* Delay in milliseconds after two-wire thermistor goes active/permissive
   and before starting to check for active shorts (first start calling
   check_active_short()) Among other things, time to turn *OFF* jump
   start's 20 volts! ...and otherwise let things stabilize. */

#define N_CYCLES_1000 1000              /* ACT_SHORT_DELAY */

/* Interval in milliseconds between successive active shorts check
   (check_active_shorts() calls), per channel -- i.e., "n" channels
   times N_CYCLES_1000 is how often all channels will have been sampled. */

#define N_CYCLES_125 125                /* ACT_SHORT_INT */



/* 5 Wire Probes */

#define WET_COUNT 2

#define SAMPLES 5         /* number or repeats to catch false 5 wire */

#endif          /* end of EEPROM_H */
