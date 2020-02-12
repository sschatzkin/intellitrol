/**********************************************************************************************
 *
 *
 *  Project:          Rack Controller
 *
 *  Module:         adc.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *  Description:  Main program ADC routines for Rack controller main
 *                microprocessor PIC24HJ256GP210
 *
 *  Revision History:
 *  Revision   Date        Who   Description of Change Made
 *  --------   --------    ---   --------------------------------------------
 *  1.5.23  10/19/11  KLL  Removed return from convert_to_binary not needed
 *                         Changed SET_MUX to char to save space. Also removed
 *                          a thermistor test which is not needed
 *  1.5.31  01/14/15  DHP  Added delay in convert_to_binary()
 *  1.6.32  03/10/15  DHP  In wait_for_probes() changed while() wait to pre-decrement 
 *                          timeout to allow following if (timeout == 0) to catch timeout.
 *                         In ops_ADC() added braces to include turning off DMA in else
 *                         In  read_ADC(), enable DMA when wait_for_probes() passes
 *  1.6.34  07/08/16  DHP  QCCC 53: Major recode of convert_to_binary(). Added 
 *                          use of probe_type[] and based the setting of
 *                          test_volt on determined probe_type (once known)
 *                          rather than treating all sensors alike.  Previous
 *                          code typically handled optic sensors as thermistors.
 *                         In ops_ADC() added a turn off of ADC module to match
 *                           what was turned on.
 *                         In read_MUXADC() saved the state of Timer3 and replaced 
 *                           turn off of ADC to ops_ADC(OFF) to ensure related
 *                           background activities were also off and added 
 *                           conditional restore of ops ADC.
*********************************************************************************************/
#include "common.h"
#include "volts.h"
static void ADC_timedrive(void);

/*************************************************************************
 *  subroutine:      read_probes()
 *
 *  function:
 *  Is called as part of the T3 1 millisecond interrupt handler
 *
 *         1.  Read the probe voltages and convert to millivolts
 *         2.  This uses the T3 interrupt @1ms rate
 *         3.  The resultant value is converted to binary
 *             based on threshold values and hysteresis
 *  input:  none
 *  output: none
 *
 *
 *************************************************************************/

void read_probes(void)
{
int             probe;
unsigned long temp_word;

//  // last_routine = 0x110;
  if (dma_result_flag)             /* if ADC data ready */
  {
    for( probe=start_point; probe<MAX_CHAN; probe++ )    /* Ignore first two probes */
                                                        /* as needed */
    {  /* CONVERT TO MILLIVOLTS */
     temp_word = (unsigned long)result_ptr[probe] * (unsigned long)527;
     temp_word /= (unsigned long)100;
     probe_volt[probe] = (unsigned int) (temp_word);
    }
    convert_to_binary();          /* Only convert if successful reading */
    probe_result_flag = 1;
    ADC_timedrive();     /* Start the next 8 channel A/D scan */
  }
} /* end of read_probes */

/*******************************6/26/2008 1:40PM******************************
 * 1.  This routine sets up and reads the 8 channels of the ADC
 *     in a timed cyclic scan sequence of all 8 channels.
 * 2.  The ADC conversion (12bit) is started.
 *****************************************************************************/
void setup_probes()
{
//  // last_routine = (0x9000) | last_routine;
  dma_result_flag = 0;      /* Clear  */
  /****************************** 9/25/2008 5:55AM ***************************
   * clear out the analog receive buffer
   ***************************************************************************/
  IFS0bits.DMA0IF = 0;      /* Clear the DMA0 Interrupt Flag */
  DMA0CONbits.CHEN=1;       /* Enable DMA0 */
  AD1CON1bits.ADON = 1;     /* turn on ADC module */
}

/*************************************************************************
 *  subroutine:      wait_for_probes()
 *
 *  function:
 *  Is called by read_ADC() when T3 is running which indicates that DMA is active.
 *
 *         1.  Wait for probe_result_flag to be set by DMA ISR
 *         2.  Clear probe_result_flag and restart DMA if flag sets and return PASSED
 *         3.  If flag does not set return FAILED
 *  input:  none
 *  output: PASSED if probe_result_flag sets, FAILED if timeout waiting
 *
 *************************************************************************/
int wait_for_probes()
{
unsigned int timeout = 0x5000;
  // last_routine = 0x65;
  while ((probe_result_flag == 0) && (--timeout > 0))
  {
  }
  if ( timeout == 0)
  {
    /********** DEBUG STATES *****************/
     printf("*** ADC FAILED to do a conversion set in 1ms ***\n\r");
    /********** DEBUG STATES *****************/
    return FAILED;
  }
  probe_result_flag = 0;    /* Clear  */
  return PASSED;
}

/*************************************************************************
 *  subroutine:      convert_to_binary()
 *
 *  function:
 *         1. The test consists of serially evaluating the ADC channels and
 *             determining if a probe value is above or below the zero
 *             or one limit for the specific probe type
 *         2. A byte, probe_pulse, records any transitions of a sensor
 *         3. An array of bytes, probe_array[], records the 1 or 0 recorded for
 *             each type of probe (by its characteristics).
 *         4. Bump the probe index, if it overflows, reset to 0
 *  QCCC 53: Modified setting/use of test_volt to be specific to probe type.
 *           Added turn off of high current if probe type determined to be 
 *           optic.  Rev E PIC CPU hardware incorporates the supporting hardware
 *           for this; I/O has no effect with older revisions.
 *           Added high_3[] and high_8[]arrays to handle wet-dry-wet transitions. 
 *  input:  none
 *  output: none
 *
 *************************************************************************/
void convert_to_binary( void )
{
unsigned int probe_level;
unsigned int probe_therm = 0;
unsigned int index;
unsigned int test_volt;
unsigned int imsk = 0x00;

  probe_level = 0x00;
  for ( index=start_point; index<MAX_CHAN; index++ )
  {
    if((probe_try_state == THERMIS) && ((probe_type[index] ==P_NO_TYPE) ||
                                               (probe_type[index] ==P_THERMIS)))
    {
       test_volt = SysParm.ADCTmaxNV;   /* It's Thermistor type, use thermistor threshold */
    }
    else
    {
      test_volt = SysParm.ADCOmaxNV;   /* Or it's 2 wire optic value */
    }
    imsk = ((unsigned int)1 << index);   /* Walk 1 across a byte */
    if (probe_volt[index] > test_volt)   /* Test for high */
    {
      probe_pulse |= imsk;
    }
    else                                 /* Low */
    {
      probe_pulse &= ~imsk;
    }
    if ((probe_volt[index] > test_volt)    /* Above the High */
        && (old_probe_volt[index] <= (unsigned int)(test_volt - SysParm.ADCTHstNV)))
                                          /* Going UP and change > Hysteresis */
    {
      probe_level |= imsk;              /* 1 means Transition */
    /* This probe active and OK? */
      if (probes_state[index] == P_DRY) 
      {
        if (probe_volt[index] > ADC6V)
        {
         if (high_8[index]++ >= 3)  //This means 3 transitions from low to high
          {
            probe_therm |= imsk;     /* Mark mask as optic  */
            probe_type[index]=P_OPTIC2;
            high_3[index] = 0;
            high_8[index] = 3;  /* avoid roll over */
            if(dry_once == FALSE)
            {
              HighI_Off(index);
            }  
          }
        }
         else
         {
          if ( (probe_type[index] == P_NO_TYPE) || (probe_type[index] == P_THERMIS))
          {
            if(high_3[index]++ >= 4)  //This means 4 transitions from low to high
            {
              probe_type[index]=P_THERMIS;  //DHP ??? 
              high_3[index] = 4;  /* avoid roll over */
            } 
          }
         }
      }
       // High voltage and Not dry
    }
    else
    { // no transition to high
      if ((probe_volt[index] < (unsigned int)(test_volt - SysParm.ADCTHstNV))
           && (old_probe_volt[index] >= test_volt))
                                        /* Going DOWN and change > Hysteresis */
      {
        probe_level |= imsk;               /* 1 means Transition */
      }
    }
    old_probe_volt[index] = probe_volt[index]; /* Store for next binary check */
  }  /* End of for ( index=point; index<MAX_CHAN; index++ ) */
  act_therm_mask |= probe_therm;      /* Accumulate thermal probes */
  probe_array[probe_index++] = probe_level;    /* Set to accumulative mask */
  if (probe_index>=MAX_ARRAY)   /* Index the binary array */
  {
    probe_index = 0;           /* and wrap around */
  }
} /*  end of convert_to_binary */

/*************************************************************************
 *  subroutine:      ADC_timedrive()
 *
 *  function:
 *
 *         1.  This routine sets up and reads the 8 channels of the ADC
 *             in a timed cyclic scan sequence of all 8 channels.
 *         2.  The ADC conversion (12bit) is started.
 *
 *  input:  none
 *  output: none
 *
 *
 *************************************************************************/
static void ADC_timedrive()
{
//  // last_routine = 0x165;
  setup_probes();
} /* end of ADC_timedrive */



/*************************************************************************
 *  subroutine:      ops_ADC()
 *
 *  function:
 *
 *         1.  This routine turns ON or OFF the T3 interrupt and DMA
 *
 *  input:  ON(TRUE) or OFF(FALSE)
 *  output: none
 *
 *************************************************************************/
void ops_ADC( char int_on)
{
  // last_routine = 0x66;
  if ( int_on )
  {
    if (!T3CONbits.TON)      /* Only turn on the probes if needed */
    {
      Init_ADC();
      setup_probes();
    // last_routine = 0x66;
      T3CONbits.TON = 1;      /* Turn on Timer 3 */
    }
  }
  else
   {    
    T3CONbits.TON = 0;      /* Turn off Timer 3 */
    AD1CON1bits.ADON = 0;     /* turn off ADC module */
    DMA0CONbits.CHEN = 0; /* and turn off DMA */
   } 
} /* end of ops_ADC */

/*************************************************************************
 *  subroutine:      read_ADC()
 *
 *  function:
 *
 *         1.  This routine sets up and reads the 8 channels of the ADC
 *             one channel at a time into probe_volt[].
 *         2. The scan time is 100us per channel.
 *
 *  input:  none
 *  output: status PASSED/FAILED (probe_volt[] modified if PASSED)
 *
 *
 *************************************************************************/
char read_ADC()
{
unsigned int  index;
unsigned long temp_word;
int timeout = 2000;
int retry;

  // last_routine = 0x67;
  /***************************** 9/29/2008 9:38AM **************************
   * Only need to do this if timer 3 and DMA is not doing the probe_volt capture
   *************************************************************************/
  /* Is Timer 3 off */
  if (T3CONbits.TON == 0)  
  {
    for ( retry = 0; retry < 5; retry++)
    {
      Init_ADC();
      setup_probes();
      while ((dma_result_flag == 0) && (--timeout > 0))
      {
        DelayUS(1);
      }
      if ( timeout > 0)
      {
        for ( index=0; index< 8; index++)
        {
        /**************************** 9/2/2008 3:35PM **************************
         * The 527 used below comes from 21.6 / 4096.
         * The reference voltage is 3.3 volts but
         * we must scale it to 21.6 because the real voltages being monitored are
         * scaled from 21.6 volts. Since the DAC is 12 bits or 4096 possibilities
         * we divide 21.6 by 4096 and we have 5.27 milli-volts per division.
         * For better resolution we multiply the DAC result by 527 or 5.27 x 100.
         * Then divide the result by 100 to remove the 100.
         ***********************************************************************/
          temp_word = (unsigned long)result_ptr[index] * (unsigned long)527;
          temp_word /= (unsigned long)100;
          probe_volt[index] = (unsigned int) (temp_word);
        }
        return PASSED;
      }
      DelayMS(250);       /* Let things settle before retry */
    }
  }
  else
  { /* We get here when timer is already enabled; DMA should have completed a transfer,
          as DMA and timer may be out of sync -  wait for it if not yet completed */
    if (wait_for_probes() == PASSED)
    {
      DMA0CONbits.CHEN = 1;  /* reenable DMA for another scan */
      return PASSED;
    }
  // last_routine = 0x67;
  }
  iambroke |= ADC_FAULT;
  xprintf( 49, DUMMY);
  return FAILED;
} /* end of read_ADC */

/*************************************************************************
 *  subroutine:      read_muxADC()
 *
 *  function:
 *
 *         1.  This routine sets up and reads one MUX channel n
 *         2.  The ADC conversion is started and then a counted
 *             wait for the DONE bit in AD1CON
 *         NOTE: Ensure the MUX is returned
 *              to previous setup when done (via set_mux() )
 *         NOTE: The T3 interrupt is turned OFF in this routine
 *              but should probably be turned off before calling and turned
 *              back on AFTER doing whatever is needed to the probe_volt
 *              array.
 *
 *  input:  channel to read, MUX to read, pointer to result storage
 *  output: status TRUE if success, FALSE if error
 *
 *************************************************************************/
char read_muxADC
    (
    unsigned int channel,               /* analog signal to read */
    SET_MUX muxchan,               /* MUX channel to read (see enum.h) */
    unsigned int *retval            /* Pointer to return voltage read */
    )

{
unsigned long  volts = 0;
volatile int timeout;
char status;
SET_MUX old_mux;
int i;
int save_T3;

  save_T3 = T3CONbits.TON;    /* save T3 module ON/OFF status */
  ops_ADC(OFF);
  /* reset ADC interrupt flag */
  IFS0bits.AD1IF = 0;

  /* enable ADC interrupts, disable this interrupt if the DMA is enabled */
  IEC0bits.AD1IE = 0;

  DMA0CONbits.CHEN = 0;     /* Disable DMA0 */
  IFS0bits.DMA0IF = 0;      /* Clear the DMA0 Interrupt Flag */
  old_mux = fetch_mux();
  set_mux (muxchan);  /* Set MUX channel selection */

  AD1CHS0 = channel;        /* Select channel to sample */
  AD1PCFGL &= ~((unsigned int)1<<channel); /* select analog input pins */
  AD1CON1 = 0x04E0;         /* auto convert after end of sampling */
  AD1CSSL = 0;              /* no scanning required */
  AD1CON3bits.ADRC = 0;     /* ADC Clock is derived from Systems Clock  */
  AD1CON3bits.SAMC = 13;    /* ADC sample time is 6.5us */
  AD1CON3bits.ADCS = 10;    /* ADC Conversion Clock is 500ns */
  AD1CON2 = 0;              /* use MUXA, AVss and AVdd are used as Vref+/- */
  AD1CON1bits.ADON = 1;     /* turn on the ADC */
  for ( i=0; i<5; i++)
  {
    AD1CON1bits.SAMP = 1;   // start sampling, automatic conversion will follow
    timeout = 0x4000;       /* Protect my self if chip is bad */
    while ((!AD1CON1bits.DONE) && (timeout > 0))
    {
      timeout--;
    };  /* Wait for conversion to complete */
    if ( timeout <= 0)
    {
      iambroke |= ADC_FAULT;
      status = FALSE;          /* ERROR, Timed out */
      printf("Waiting for the Analog conversion timed out\n\r");
      set_mux (old_mux);              /* Restore MUX channel selection */
      return status;
    }
    /* CONVERT TO MILLIVOLTS */
    if ( i != 0)          /* Throw away the first reading */
    {
      volts += ADC1BUF0;            /* Sum up 4 readings */
      debug_tbl[i]=ADC1BUF0;
    }
  }
  volts /= (unsigned long)4;   /* calculate the average */
  volts *= (unsigned long)805;   /* Convert to millivolts */
  volts /= (unsigned long)1000;
  *retval = (unsigned int)(volts);    /* Return measured voltage */
  status = TRUE;
  set_mux (old_mux);              /* Restore MUX channel selection */
  Init_ADC();
  if (save_T3 != 0)
  {
    ops_ADC(ON);                  /* Turn ON T3, DMA, ADC modules */
  }
  return(status);
} /* end of read_muxADC() */

/**************************** end of adc.c **********************************/
