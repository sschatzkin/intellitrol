/*********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         VOLTS.h
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2008, 2014  Scully Signal Company
 *
 *   Description:    Event Log "Record" entry layouts
 *
  * Revision History:
 *   Rev      Date           Who   Description of Change Made
 *  --------  --------------  ---    --------------------------------------------
 *  1.5       09/23/08  KLL  Ported old Intellitrol code to the PIC24HJ256GP210 cpu
 *  1.5.30  08/10/14  DHP  Removed REF_DIFF_VOLTAGE
 *
 *********************************************************************************************/
#ifndef VOLTS_H
#define VOLTS_H

/*******************************6/12/2008 7:11AM******************************
 * The "channel" voltage levels are referenced to 21.6 volts limit
 *
 * With the 12-bit A/D conversion, each A/D "step" is thus 21.09 millivolts,
 * more or less.
 *
 *****************************************************************************/
/* The "channel" voltage levels are referenced to 21.6 volts limit

   With the 12-bit A/D conversion, each A/D "step" is thus 5.27 millivolts,
   more or less. */

#define ATODSCALE   80    /* 80.5 millivolts per step */

/* "Direct" voltage values -- read_ADC() converts A/D counts (and stores all
   results in probe_volt[]) directly into millivolts, assuming "Channel"
   voltages; read_muxADC() likewise directly returns millivolts (same "defi-
   nitions" below), but based on a 5-volt reference. */

#define ADC_05MV    50                  /* 50 MV */
#define ADC_06MV    60                  /* 60 MV */
#define ADC_07MV    75                  /* 75 MV */
#define ADC_10MV   100                  /* 0.10 (1/10) volt equivalent count */
#define ADC_15MV   150                  /* 0.15        volt equivalent count */
#define ADC_25MV   250                  /* 0.25 (1/4)  volt equivalent count */
#define ADC_33MV   333                  /* 0.33 (1/3)  volt equivalent count */
#define ADC_50MV   500                  /* 0.50 (1/2)  volt equivalent count */
#define ADC_66MV   666                  /* 0.66 (2/3)  volt equivalent count */
#define ADC_75MV   750                  /* 0.75 (3/4)  volt equivalent count */
#define ADC_90MV   900                  /* 0.90 (9/10) volt equivalent count */
#define ADC_927MV  927                  /* 0.927       volt equivalent count */
#define ADC1V     1000                  /* ~1 volt (47.407) equivalent count */
#define ADC1_09V  1090                  /* 1.090       volt equivalent count */

#define ADC1_25V  (ADC1V+ADC_25MV)      /* 1.25 volt equivilent count */
#define ADC1_50V  (ADC1V+ADC_50MV)      /* 1.50 volt equivilent count */
#define ADC1_75V  (ADC1V+ADC_75MV)      /* 1.75 volt equivilent count */
#define ADC2V     (2*ADC1V)             /* 2 volt equivalent count */
#define ADC3V     (3*ADC1V)             /* 3 volt equivalent count */
#define ADC4V     (4*ADC1V)             /* 4 volt equivalent count */
#define ADC4_9V   (ADC4V+ADC_90MV)      /* 4 volt equivalent count */
#define ADC5V     (5*ADC1V)             /* 5 volt equivalent count */
#define ADC5_25V  (ADC5V+ADC_25MV)      /* 5.25 volt equivalent count */
#define ADC5_50V  (ADC5V+ADC_50MV)      /* 5.50 volt equivalent count */
#define ADC6V     (6*ADC1V)             /* 6 volt equivalent count */
#define ADC7V     (7*ADC1V)             /* 7 volt equivalent count */
#define ADC8V     (8*ADC1V)             /* 8 volt equivalent count */
#define ADC9V     (9*ADC1V)             /* 9 volt equivalent count */
#define ADC10V    (10*ADC1V)            /* 10 volt equivalent count */
#define ADC12V    (12*ADC1V)            /* 12 volt equivalent count */
#define ADC15V    (15*ADC1V)            /* 15 volt equivalent count */
#define ADC20V    (20*ADC1V)            /* 20 volt equivalent count */

/* Raw 13 volt limits. These reference the raw power supply level based on
   incoming AC line levels.

   Raw-13 volt levels corresponding to input voltages

        Input       Raw13   Idle (5-min-idle diag, Big Red LED drawing power)
        ----        -----   -----
         95         10.9    10.4
        100/200     11.6    11.2
        105         12.4    11.9
        110/220     13.0    12.4
        115         13.6    13.1
        120         14.3    13.9
        125/250     15.0    14.6
        130         15.8    15.3
        135         16.5    15.8

   The original "Raw 13" limits were from G.Cadman 28-Sep-95, based on above
   empirical measurements vs manufacturing/etc. tolerances.  The HI was changed
   to ignore errors experienced in Scotland (see version.h rev 1.6.36. */

#define RAW13VOLT_LO 10000
#define RAW13VOLT_HI  30000

/* Reference Volt limits. The Reference Volt is calculated based on reading
   the 5-wire-optic precision diagnostic voltage. Limits +- 1.9% per G. Cadman
   5-Oct-95. [Kick the limits up to %-5% per Scott/FrankSki 21-Nov-95]
   DHP - per the above statement the limits should be 1000*1.05 and 1000*0.95
              resulting in values of 950 and 1050.  Unknown how these values came to be.
*/

#define REFVOLT_LO        750   /* Changed due to a few fault reports */
#define REFVOLT_HI       1100   /* with water on plug ref. volts @.845V */

/* Probe bias (comparator switch point) levels, and noise limits */

#define BIAS35_MIN       3400
#define BIAS35_MAX       3650     /* Changed 1-26-96 on edge of LM2951 Tol. +-2% */

#define BIAS38_MIN       3700
#define BIAS38_MAX       3950     /* Changed 1-26-96 on edge of LM2951 Tol. +- 2% */

#define BIAS_NOISE_P      100     /* Changed 1-26-96 per G.C. ground resistance factor */
#define BIAS_NOISE_M      100

/* Channel voltages minima/maxima, as per G. Cadman 2-Oct-95 - 4-Oct-95. */

/* 10 volt drive & noise levels, jump start off */

#define PROBE10_TOLERANCE   ADC_50MV
#define PROBE10_MIN      8500         /* Changed per G.Cadman Divider error +- 1.6% 1-25-96 */
#define PROBE10_MAX     10300
#define PROBE10_NOISE_P   350         /* Changed 1-26-96 to allow possible ground loop problems */
#define PROBE10_NOISE_M   350

/* Jump start on (No European jumper) */

#define PROBE20_MIN     16100         /* Changed per G Cadman Temp. Drift of 20 Volts Source 1-25-96 */
#define PROBE20_MAX     20600

/* Clamping levels (jump start on) with European jumpers (spec'ed as 9.8 -
   11.2 volts, plus "slop" factor for temperature/etc. variations, as per
   G. Cadman 24-Oct-95) */

#define PROBE_EUROMIN    9300
#define PROBE_EUROMAX   11300

/* Optic 5-wire output pulse levels, in millivolts. OPTIC5OUT is the nominal
   outgoing pulse level, with MIN and MAX respectively the minimum and maxi-
   mum acceptable levels. */

#define OPTIC5OUT       5000        /* Changed 1-26-96 to new Zener value ref. */
//#define OPTIC5OUT_MIN   4150        /* Changed per G Cadman error due to Divider error +- 1.6% 1-25-96 */
#define OPTIC5OUT_MIN   3900        /* Changed per K Langlais Verify the Z22 voltage meet EN13922 Spec 01-27-2010 */
#define OPTIC5OUT_MAX   5100

/* Optic 5-wire return "echo" pulse level specification. The return or "echo"
   pulse must be greater than the outgoing pulse to verify that channel 6 is
   not just shorted to channel 4. OPTIC5IN_MIN is the minimum acceptable re-
   turn pulse level, and defines the "leading" edge threshold for the echo
   pulse; OPTIC5IN_TRAIL defines the trailing edge voltage threshold of the
   echo pulse ("echo" pulse done when voltage drops below OPTIC5IN_TRAIL). */

#define OPTIC5IN_MIN             4900
#define OPTIC5IN_TRAIL           3000
#define OPTIC5_CHECK_FOR_PULSE   3500

/* Optic 5-wire diagnostic reference level */

//#define OPTIC5DIAG_REF  6759
#define OPTIC5DIAG_REF 6848

/* Optic 5-wire diagnostic "offset" level (probe's internal switching tran-
   sistor PN junction bias between 4.75k "diag" resistor and ground) */

#define OPTIC5DIAG_OFS    60

/* Optic 5-wire diagnostic "level" mapping (to identify which channel is wet,
   based on the fifth-wire "diag" line and the precision pull-down resistors
   mapped in parallel by the probes */

// Alternative Table - Offset threshold to match the ideal values
// optic5.c calc_tank() offset and slope calculation
/*#define OPTIC5DIAG_CH00 6840
#define OPTIC5DIAG_CH01 6630
#define OPTIC5DIAG_CH02 6380 
#define OPTIC5DIAG_CH03 6000
#define OPTIC5DIAG_CH04 5690
#define OPTIC5DIAG_CH05 5385
#define OPTIC5DIAG_CH06 5130
#define OPTIC5DIAG_CH07 4890 
#define OPTIC5DIAG_CH08 4660 
#define OPTIC5DIAG_CH09 4470 
#define OPTIC5DIAG_CH10 4270 
#define OPTIC5DIAG_CH11 4105
#define OPTIC5DIAG_CH12 3950
#define OPTIC5DIAG_CH13 3795 
#define OPTIC5DIAG_CH14 3675 
#define OPTIC5DIAG_CH15 3550
#define OPTIC5DIAG_CH16 3440*/ 

#define OPTIC5DIAG_CH01 6848    /* 0x1AC0 */
#define OPTIC5DIAG_CH02 6290    /* 0x1892 */
#define OPTIC5DIAG_CH03 5982    /* 0x175E */
#define OPTIC5DIAG_CH04 5656    /* 0x1618 */
#define OPTIC5DIAG_CH05 5365    /* 0x14F5 */
#define OPTIC5DIAG_CH06 5101    /* 0x13ED */
#define OPTIC5DIAG_CH07 4863    /* 0x12FF */
#define OPTIC5DIAG_CH08 4646    /* 0x1226 */
#define OPTIC5DIAG_CH09 4447    /* 0x115F */
#define OPTIC5DIAG_CH10 4265    /* 0x10A9 */
#define OPTIC5DIAG_CH11 4097    /* 0x1001 */
#define OPTIC5DIAG_CH12 3941    /* 0x0F65 */
#define OPTIC5DIAG_CH13 3798    /* 0x0ED6 */
#define OPTIC5DIAG_CH14 3664    /* 0x0E50 */
#define OPTIC5DIAG_CH15 3539    /* 0x0DD3 */
#define OPTIC5DIAG_CH16 3423    /* 0x0D5F */

/* Default probe detection threshold -- voltage drop below open-circuit level
   to declare something connected and go into acquire state Found that 8 volts
   seem to work for USA ("20V Jump Start") Version and .5 Volt for European
   Voltages ("10V Jumpers")*/

#define PROBE_SENS_JSV   1000
#define PROBE_SENS_EUROV  100

extern unsigned long lowVolt;

#endif        /* end of VOLTS_H */
