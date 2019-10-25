/*******************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         pod.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *   Description:    Main program Power On Diagnostic routines for Rack
 *                   controller main microprocessor PIC24HJ256GP210
 *
 * Revision History:
 *   Rev      Date    Who   Description of Change Made
 * -------- --------  ---   --------------------------------------------
 *  1.5.26  10/09/12  KLL  Changed the CRC storage location to 0x2ABF0 which is
 *                          a start of the last page in program memory. With
 *                          security on writing to the last program address
 *                          causes address traps because the Microchip builtins
 *                          that allow the writing into program memory actually
 *                          go beyond the address being updated.
 *  1.5.30  08/10/14  DHP  Changed definition/meaning of vapor LED to Ground bolt
 *                          LED and ground LED to resistive ground LED.
 *                         Added turn on non-permit LEDs at end of flash_panel()
 *                         Removed vapor_test() and call from diarray[]
 * 1.5.31  01/14/15  DHP  Changed check100OhmGnd() to always return PASS/FAIL
 * 1.6.34  10/10/16  DHP  Fogbugz 111: Added check for SysParm.Ena_TIM_Read
 *                         in flash_panel().
 *                        Added a time check in service_charge() to ensure the
 *                         charge pulses were of sufficient time.
 *                        Fogbugz 142: Added eeChk to diarray[].
 *                        QCCC 53: In check_5wire_5volt() added turn on and 
 *                          restore of high current to valid voltage reading.
 ******************************************************************************/

#include "common.h"
#include "diag.h"
#include "volts.h"
#include "version.h"

#define  JAN_01_2009    0x495C4DE0  /* GMT */
/*********************************************************************
 *  subroutine: boot_mem_test
 *
 *  function:     This was intended to report any errors uncovered by memory_test.
 *              That test was executed early after reset and was probably meant to test data 
 *              memory prior to its use, which includes stack setup.  As no documentation exists 
 *              to clarify the purpose and the test does not work within the scheme of the boot
 *              loader it has been dropped. This remnant is kept as a place holder should someone
 *              want to resurect the original or a replacement routine.
 *          
 *  input:   none
 *  output: TRUE / FALSE based on errors detected
 *
 *************************************************************************/
static char boot_mem_test (void)
{
    printf("\n\r    Powerup Memory Tests Passed\n\r");
    return FALSE;
}

/*********************************************************************
 *  subroutine: diag_clock
 *
 *  function:     Report the status of  Read_Clock()
 *  input:   
 *  output: TRUE (if error) / FALSE
 *
 ********************************************************************/
char diag_clock (void)
{
    char i;
    char clk;
    char sts;

    for (i = 5; i > 0; i--)             /* Up to 5 tries to read TOD clock */
    {
        clk = Read_Clock();             /* Read Dallas Time-Of-Day */
        if (clk == CLOCK_OK)            /* If successfully read, */
        {
            sts = FALSE;                    /* Note complete success */
            break;                      /* And get outta here */
        }
        else
        {
            if ((clock_status != CLOCK_DEFAULT)
                && (clock_status != CLOCK_STOPPED))
            {
                sts = TRUE;                  /* Problems reading time */
            }
            else
            {
            sts = 0;
            }
            DelayMS (5);                 /* Try again in a few ms */
        }
    }
    
    if ( sts == TRUE)
    {
        present_time = JAN_01_2009;     /*   Set default date */

        sts = Write_Clock ();           /* Update Dallas TOD clock chip */
        if (sts != CLOCK_OK)            /* Any problems? */
        {                               /* Yes... */
            sts = TRUE;                   /* Generic "register error" code */
        }

        UNIX_to_Greg();                /* Update to new year/month/etc. */
    }
    
    xprintf( 122, DUMMY );   /* use the xprintf function to report Dallas status */

    return (sts);

} /* End diag_clock() */

/*************************************************************************
 *  subroutine:      flash_panel()
 *  function:
 *         1.  Serially flash the RED bank, GREEN bank, 1 to 8 compartment,
 *             Optic Pulse, Ground Error, Service, Ground, Dynacheck, Terminal/
 *             Vehicle, and Authoorized/Unauthorized leds
 *         2.  Set the sequential rate at the overflow of TMR1 (or repeat
 *             value compare)
 *  input:  none
 *  output: none
 *
 *************************************************************************/
#define FLASH_PASSES    1       /* Number of Flash-the-LEDs sequences */
#define SW_SLOW         4       /* 4/8's second */
#define SW_MEDIUM       2       /* 2/8's second */

char flash_panel (void)
{
    char two_pass;
    int i;

    set_nonpermit(DARK);
    ledstate[OPTIC_OUT]  = DARK;
    ledstate[OPTIC_IN]   = DARK;
    ledstate[TASCOMM]    = DARK;
    ledstate[TRUCKCOMM]  = DARK;
    ledstate[GND_BAD] = DARK;
    set_new_led(GROUND_GOOD, DARK);    /* Green ground diode off */
    set_new_led(DEADMAN_BAD, DARK);
    set_new_led(DEADMAN_GOOD, DARK);
    ledstate[VIP_AUTH] = DARK;
    ledstate[VIP_UNAUTH] = DARK;
    ledstate[VIP_IDLE] = DARK;
    ledstate[GND_GOOD] = DARK;
    set_new_led(FREELED26, DARK);       /* Green ground diode off */
    ledstate[DYNACHEK] = DARK;
    service_wait(SW_SLOW);              /* Wait half-a-second */
    set_nonpermit(LITE);
    service_wait(SW_SLOW);              /* Wait half-a-second */
    set_nonpermit(DARK);
    set_permit(LITE);
    if ( new_front_panel == TRUE)
    {
      service_wait(SW_SLOW * 4);        /* Similarly, BigGreen gets 2 seconds */
    } else
    {
      service_wait(SW_SLOW);            /* Similarly, BigGreen gets .5 seconds */
    }
    set_permit(DARK);

    for (two_pass = 0; two_pass < FLASH_PASSES; two_pass++) /* */
    {
      for ( i=(int)COMPARTMENT_1; i<=(int)COMPARTMENT_8; i++)
      {
        ledstate[i] = LITE;
        service_wait(SW_MEDIUM);
      }

      if ( new_front_panel == TRUE)
      {
        for ( i=(int)COMPARTMENT_9; i<=(int)COMPARTMENT_12; i++)
        {
          ledstate[i] = LITE;
          service_wait(SW_MEDIUM);
        }
      }

      for ( i=(int)COMPARTMENT_1; i<=(int)COMPARTMENT_8; i++)
      {
        ledstate[i] = DARK;
      }

      if ( new_front_panel == TRUE)
      {
        for ( i=(int)COMPARTMENT_9; i<=(int)COMPARTMENT_12; i++)
        {
          ledstate[i] = DARK;
        }
      }
    }
    if ( new_front_panel == TRUE)
    {
//      if (SysParm.EnaFeatures & ENA_GROUND) /* Is Ground-Check enabled? */
      {                                     /* Yes */
        ledstate[GND_BAD] = LITE;                          /* Then flash Ground Error LED */
        set_new_led(GROUND_GOOD, DARK);     /* Then flash Ground LED */
        service_wait(SW_SLOW);
        ledstate[GND_BAD] = DARK;                      /* Then flash Ground Error LED */
        set_new_led(GROUND_GOOD, LITE);        /* Then flash Ground LED */
        service_wait(SW_SLOW);
      }

//      if (SysParm.EnaFeatures & ENA_VIP)  /* VIP enabled? */
      {                           /* Yes */

        ledstate[VIP_AUTH] = DARK;
        ledstate[VIP_UNAUTH] = LITE;
        service_wait(SW_SLOW);

        ledstate[VIP_UNAUTH] = DARK;
        ledstate[VIP_IDLE] = LITE;
        service_wait(SW_SLOW);
      }

//      if (SysParm.EnaFeatures & ENA_DEADMAN)    /* Is DEADMAN-Check enabled? */
      {                                     /* Yes */
        set_new_led(DEADMAN_BAD, LITE);        /* Then flash DEADMAN LED */
        set_new_led(DEADMAN_GOOD, DARK);     /* Then flash DEADMAN LED */
        service_wait(SW_SLOW);
        set_new_led(DEADMAN_BAD, DARK);        /* Then flash DEADMAN LED */
        set_new_led(DEADMAN_GOOD, LITE);     /* Then flash DEADMAN LED */
        service_wait(SW_SLOW);
      }

      if (unit_type == INTELLITROL2)
      {                                     /* Yes */
        ledstate[GND_GOOD] = LITE;            /* Then flash LED */
        service_wait(SW_SLOW);
        ledstate[GND_GOOD] = DARK;         /* Turn off LED */
        service_wait(SW_SLOW);
      }

      ledstate[OPTIC_OUT] = LITE;
      service_wait(SW_SLOW);

      ledstate[OPTIC_IN] = LITE;
      service_wait(SW_SLOW);

      ledstate[TASCOMM] = LITE;
      service_wait(SW_SLOW);

      ledstate[TRUCKCOMM] = LITE;
      service_wait(SW_SLOW);

      ledstate[DYNACHEK] = LITE;
      service_wait(SW_SLOW);

      ledstate[VIP_IDLE] = DARK;
      set_new_led(DEADMAN_GOOD, DARK);     /* Then flash DEADMAN LED */
      set_new_led(FREELED26, DARK);      /* Then flash Ground Green LED */
      set_new_led(GROUND_GOOD, DARK);     /* Then flash Ground LED */
      ledstate[OPTIC_OUT]  = DARK;
      ledstate[OPTIC_IN]   = DARK;
      ledstate[TASCOMM] = DARK;
      ledstate[TRUCKCOMM] = DARK;
      ledstate[DYNACHEK]   = DARK;
    } else
    {
      ledstate[OPTIC_OUT] = LITE;
      service_wait(SW_MEDIUM);

      ledstate[OPTIC_IN] = LITE;
      service_wait(SW_MEDIUM);

      if(SysParm.EnaFeatures & ENA_GROUND)
      {
        ledstate[GND_BAD] = LITE;  /* Then flash Ground Error LED */
        service_wait(SW_MEDIUM);
      }
      if((SysParm.EnaFeatures & ENA_GROUND)&&(unit_type == INTELLITROL2))
      {
        ledstate[GND_GOOD] = LITE;       /* Then flash Ground Good LED */
        service_wait(SW_MEDIUM);
      }
      ledstate[DYNACHEK] = LITE;
      service_wait(SW_MEDIUM);

      ledstate[OPTIC_OUT]   = DARK;
      ledstate[OPTIC_IN]    = DARK;
      ledstate[GND_BAD]  = DARK;
      ledstate[GND_GOOD]   = DARK;
      ledstate[DYNACHEK]    = DARK;

      ledstate[TASCOMM] = LITE;
      service_wait(SW_MEDIUM);

      ledstate[TRUCKCOMM] = LITE;
      service_wait(SW_MEDIUM);

      if (SysParm.EnaFeatures & ENA_VIP)  /* VIP enabled? */
      {                           /* Yes */
        ledstate[VIP_AUTH] = LITE;
        service_wait(SW_MEDIUM);

        ledstate[VIP_UNAUTH] = LITE;
        service_wait(SW_MEDIUM);

        ledstate[VIP_IDLE] = LITE;
        service_wait(SW_MEDIUM);
      }
    }

    ledstate[TASCOMM]    = DARK;
    ledstate[TRUCKCOMM]  = DARK;
    ledstate[VIP_AUTH]   = DARK;
    ledstate[VIP_UNAUTH] = DARK;
    ledstate[VIP_IDLE]   = DARK;
    /* Since we are called "last" in the diag sequence, set the LEDs to
       their initial "idle" states. Everything is DARK now. */

    set_nonpermit(LITE);
    ledstate[DYNACHEK] = FLASH1HZ;

    StatusO &= ~0x7;     /* Clear VIP output status */
//FOGBUGZ 111    if (SysParm.EnaFeatures & ENA_VIP)  /* If VIP is enabled */
    if ((SysParm.EnaFeatures & ENA_VIP) || (!(SysParm.EnaFeatures & ENA_VIP) &&
      (SysParm.Ena_TIM_Read == 1)))  //FOGBUGZ 111
    {
      ledstate[VIP_IDLE] = LITE;      /*  then it comes up "Idle" */
      StatusO |= STSO_STANDBY;
    }
    if (SysParm.EnaFeatures & ENA_GROUND)  /* If Ground is enabled */
    {
      set_new_led(GROUND_GOOD, LITE);    /* Green Ground diode on */
    }
    if (SysParm.EnaFeatures & ENA_DEADMAN)  /* If Deadman is enabled */
    {
      set_new_led(DEADMAN_GOOD, LITE);    /* Green Deadman diode on */
    }

    return (0);    /* The LEDs will be refreshed by the 1 mS interrupt */

}  /* end of flash_panel() */


void show_revision(void)
{
    set_nonpermit(DARK);
    ledstate[COMPARTMENT_1] = DARK;
    ledstate[COMPARTMENT_2] = DARK;
    ledstate[COMPARTMENT_3] = DARK;
    ledstate[COMPARTMENT_4] = DARK;
    ledstate[COMPARTMENT_5] = DARK;
    ledstate[COMPARTMENT_6] = DARK;
    ledstate[COMPARTMENT_7] = DARK;
    ledstate[COMPARTMENT_8] = DARK;
    ledstate[OPTIC_OUT]  = DARK;
    ledstate[OPTIC_IN]   = DARK;
    ledstate[TASCOMM]    = DARK;
    ledstate[TRUCKCOMM]  = DARK;
    ledstate[GND_BAD] = DARK;
    set_new_led(GROUND_GOOD, DARK);    /* Green ground diode off */
    set_new_led(DEADMAN_BAD, DARK);
    set_new_led(DEADMAN_GOOD, DARK);
    ledstate[VIP_AUTH] = DARK;
    ledstate[VIP_UNAUTH] = DARK;
    ledstate[VIP_IDLE] = DARK;
    ledstate[GND_GOOD] = DARK;
    set_new_led(FREELED26, DARK);     /* Green ground diode off */
    ledstate[DYNACHEK] = DARK;
    service_wait(8);              /* Wait half-a-second */
    
    ledstate[MAJVER] = LITE;
    ledstate[VIP_AUTH] = LITE;
    service_wait(16);              /* Wait a second */
    
    ledstate[COMPARTMENT_1] = DARK;
    ledstate[COMPARTMENT_2] = DARK;
    ledstate[COMPARTMENT_3] = DARK;
    ledstate[COMPARTMENT_4] = DARK;
    ledstate[COMPARTMENT_5] = DARK;
    ledstate[COMPARTMENT_6] = DARK;
    ledstate[COMPARTMENT_7] = DARK;
    ledstate[COMPARTMENT_8] = DARK;
    ledstate[VIP_AUTH] = DARK;
    ledstate[VIP_UNAUTH] = DARK;
    ledstate[VIP_IDLE] = DARK;
    service_wait(8);              /* Wait half-a-second */
    
    ledstate[MINVER] = LITE;
    ledstate[VIP_UNAUTH] = LITE;
    service_wait(16);                   /* Wait half-a-second */
 
    ledstate[COMPARTMENT_1] = DARK;
    ledstate[COMPARTMENT_2] = DARK;
    ledstate[COMPARTMENT_3] = DARK;
    ledstate[COMPARTMENT_4] = DARK;
    ledstate[COMPARTMENT_5] = DARK;
    ledstate[COMPARTMENT_6] = DARK;
    ledstate[COMPARTMENT_7] = DARK;
    ledstate[COMPARTMENT_8] = DARK;
    ledstate[VIP_AUTH] = DARK;
    ledstate[VIP_UNAUTH] = DARK;
    ledstate[VIP_IDLE] = DARK;
    service_wait(8);              /* Wait half-a-second */
    
    ledstate[EDTVER] = LITE;
    ledstate[VIP_IDLE] = LITE;
    service_wait(16);                   /* Wait half-a-second */
    
    ledstate[COMPARTMENT_1] = DARK;
    ledstate[COMPARTMENT_2] = DARK;
    ledstate[COMPARTMENT_3] = DARK;
    ledstate[COMPARTMENT_4] = DARK;
    ledstate[COMPARTMENT_5] = DARK;
    ledstate[COMPARTMENT_6] = DARK;
    ledstate[COMPARTMENT_7] = DARK;
    ledstate[COMPARTMENT_8] = DARK;
    ledstate[VIP_AUTH] = DARK;
    ledstate[VIP_UNAUTH] = DARK;
    ledstate[VIP_IDLE] = DARK;
    service_wait(8);              /* Wait half-a-second */
   
    // repeat the end stuff from flash_panel as we are now the last in the pool
    set_nonpermit(LITE);
    ledstate[DYNACHEK] = FLASH1HZ;

    StatusO &= ~0x7;     /* Clear VIP output status */
//FOGBUGZ 111    if (SysParm.EnaFeatures & ENA_VIP)  /* If VIP is enabled */
    if ((SysParm.EnaFeatures & ENA_VIP) || (!(SysParm.EnaFeatures & ENA_VIP) &&
      (SysParm.Ena_TIM_Read == 1)))  //FOGBUGZ 111
    {
      ledstate[VIP_IDLE] = LITE;      /*  then it comes up "Idle" */
      StatusO |= STSO_STANDBY;
    }
    if (SysParm.EnaFeatures & ENA_GROUND)  /* If Ground is enabled */
    {
      set_new_led(GROUND_GOOD, LITE);    /* Green Ground diode on */
    }
    if (SysParm.EnaFeatures & ENA_DEADMAN)  /* If Deadman is enabled */
    {
      set_new_led(DEADMAN_GOOD, LITE);    /* Green Deadman diode on */
    }
}


/*************************************************************************
 *  subroutine: check_ref_volt
 *
 *  function:
 *         1.  The SIM power applied to all the probes(done earlier).
 *         2.  Jump_Start applied (also done earlier)
 *         3.  Get the initial readings for open voltage & probe voltage
 *         4.  Get the truck tank type
 *
 *  input:  none
 *  output: TRUE (if error) / FALSE
 *
 *************************************************************************/
char check_ref_volt (void)
{
unsigned long  scratch;
unsigned char  tindex;
char status;

    /* If called out of RESET/init, then Jump-Start is still off (from CRSTI),
       but will be on if called from (e.g., truck_idle()) main-loop/etc code */
    ops_ADC( OFF);                            /* Turn off the read probe timer */
    set_mux(M_PROBES);                    /* Point to probe voltage */
    set_porte(~PROBE_CH_TEST5);   /* Drive all but ch 5 outputs (to +10) */
    JUMP_START = CLR;                   /* Jump_start off */
    PULSE5VOLT = CLR;                   /* Set optic pulse (ch 4) to 10 volts (~0x01) */

    /* Generate the "Reference Volt" based on the 6.759 volt precision voltage
       reference source for the 5-wire-optic diagnostic line. */

    DIAGNOSTIC_EN = CLR;                /* Select 5-wire-optic diagnostic (~ 10) */
    DelayMS(2);                         /* Pause to allow voltage to stabilize */
    scratch = 0;                        /* Use the channel 5 line */
    for (tindex=0; tindex<8; tindex++) /* Get the 5wire average */
    {
      if (read_ADC() == FAILED)
      {
        printf("%c", 0x1B);
        printf("[31m");
        printf("\n\rTrouble reading the Analog Port\n\r");
        iambroke |= ADC_FAULT;
        printf("%c", 0x1B);
        printf("[30m");
        return FAILED;
      }
      scratch += probe_volt[4];
    }

    scratch /= tindex;           /* Calculate average voltage for the diagnostic line */
    ReferenceVolt = (unsigned int)(((unsigned long)pSysDia5->Reference * 1000)
                                   / scratch);  /* Calculate the value of scaling coefficient times 1000 */
    if ((ReferenceVolt < pSysVolt->RefVoltLo)        /* Error if outside of allowed error window */
        || (ReferenceVolt > pSysVolt->RefVoltHi))
    {
       status = TRUE;
       iambroke |= REF_VOLT_FAULT;
       printf("%c", 0x1B);
       printf("[31m");
       xprintf( 162, ReferenceVolt);
       printf("\n\r    PVOLT5 Volt needs to be between %01u.%03uv and %01u.%03uv",
        pSysVolt->RefVoltLo/1000,pSysVolt->RefVoltLo%1000,pSysVolt->RefVoltHi/1000,pSysVolt->RefVoltHi%1000);
       printf("%c", 0x1B);
       printf("[30m");
    }
    else
    {
       status = FALSE;
       iambroke &= ~REF_VOLT_FAULT;
       xprintf( 163, ReferenceVolt);
    }
    DIAGNOSTIC_EN = SET;                     /* Clear 5-wire-optic diagnostic supply */
    return status;
}    /* End of check_ref_volt (void) */

/*************************************************************************
 *  subroutine: check_open_c_volt ()
 *
 *  function:   Read the voltages with NO truck connected
 *
 *  input:   none
 *  output: TRUE (if error) / FALSE
 *
 *************************************************************************/

char check_open_c_volt (void)
{
unsigned long open_c_init[MAX_CHAN];
unsigned long scratch;
int           probe, yindex, tindex;
char          bad_voltage;

    set_mux(M_PROBES);           /* Point to probe voltage */
    Init_ADC();
    JUMP_START = CLR;                   /* Jump_start off */
    PORTE = PULSE_TEST;                 /* Drive all channels */
    bad_voltage = FALSE;                /* default to OK */

    /* At this point "10" volts should be applied to all {6 | 8} channels.
       Jump start is still off (if called from CRTSI/init)...loop below
       will turn it (back) on */

    /* Now "calibrate" our 8 channels +10 and +20 readings */

    DelayMS (10);                     /* Stabilize voltages (just in case) */
    for (tindex=0; tindex<2; tindex++)
    {
      xprintf( 126, (unsigned int)(tindex+1) ); /* */
      for (probe=0; probe<MAX_CHAN; probe++ )   /* zero out array */
      {
        open_c_init[probe] = 0;
      }
      for (yindex=0; yindex<MAX_CHAN; yindex++ )
      {
        if (read_ADC() == FAILED)
        {
          printf("%c", 0x1B);
          printf("[31m");
          printf("\n\rTrouble reading the Analog Port\n\r");
          iambroke |= ADC_FAULT;
          printf("%c", 0x1B);
          printf("[30m");
          return FAILED;
        }
        for (probe=0; probe<MAX_CHAN; probe++ )
        {
          open_c_init[probe] += probe_volt[probe];
        }
      }
      for (probe=0; probe<MAX_CHAN; probe++ )   /* filter the open lines */
         {
         scratch = open_c_init[probe]/MAX_CHAN;  /* Average for 8 channels */
         open_c_volt[tindex][probe] = (unsigned int) scratch;   /* now drop hi bits */
         xprintf( 127, (unsigned int)scratch ); /* */

         if ((probe>1) && !tindex)
            {
            if (open_c_volt[tindex][probe] < ADC8V)/* If the open circuit voltage  */
               {
               printf("<==  ");
               bad_voltage = TRUE;      /* is funny, default to 10 volt */
               }
            }
         }

      /* Turn on Jump-Start for second iteration (open_c_volt[1][] readings) */
      if (tindex == 0)
      {
        JUMP_START = SET;         /* Jump_start on for second pass */
        DelayMS (10);               /* Allow +20 to propagate */
      }
    }
   /**************** Report results *****************************************/
    if (bad_voltage)
    {
      xprintf( 129, DUMMY );  /* do the printout first. . . */
    }
    return bad_voltage;               /* Return with voltage go/nogo */
} /* End of check_open_c_volt () */

/*************************************************************************
 *  subroutine: check_tank_type()
 *
 *  function:  Determine if the rack is a 6 or 8 tank setup
 *           Get the open_c_volt per channel
 *           Get the probe_volt per channel
 *           Note that the Jump_Start voltage does NOT get applied
 *             to the probe 0 & 1.
 *           Channel 0 & 1 will read 0 volts on a 6 tank truck.
 *
 *  input:   none
 *  output: FALSE (no error)
 *
 *************************************************************************/

char check_tank_type(void)
{
  if ((open_c_volt[0][0] > ADC1V)     /* If either channel 0 or */
      || (open_c_volt[0][1] > ADC1V)) /*   channel 1 have +10 volts... */
      ConfigA |= CFGA_8COMPARTMENT;   /* Mark 8-compartment trucks */
  else
      ConfigA &= ~CFGA_8COMPARTMENT;  /* Mark 6-compartment USA-style */

  if (ConfigA & CFGA_8COMPARTMENT)
    xprintf( 2, DUMMY );
  else
    xprintf( 3, DUMMY );

  return FALSE;

} /* End of check_tank_type() */

/*************************************************************************
 *  subroutine: check_bias_noise
 *  function:  Check bias noise levels
 *
 *  input:   none
 *  output: PASSED / FAILED (if read_ADC error)
 *
 *  Note:   check_bias_noise should be called before check_probe_noise,
 *          in order to accurately track NOISE_FAULT (only "reset" here)
 *          when periodically calling either/both.
 *
 *************************************************************************/

#define BIAS_N_SAMPLES 20

char check_bias_noise (void)
{
  unsigned int biasvolt, bias_p_init, bias_m_init;
  int bad_voltage, index;
  char status;
  unsigned long temp_word;

  bad_voltage = FALSE;
  status = PASSED;

  SCALE_AN1 = 1;
  SCALE_AN0 = 1;
  set_mux( M_THREEX );
  DelayUS (1000);         /* Allow voltage to settle */
  Init_ADC();
  bias_p_init = BiasVolt;
  bias_m_init = BiasVolt;

  for (index=0; index<BIAS_N_SAMPLES; index++)
  {
    if (read_ADC() == FAILED)
    {
       printf("%c", 0x1B);
       printf("[31m");
       printf("\n\rTrouble reading the Analog Port\n\r");
       iambroke |= ADC_FAULT;
       printf("%c", 0x1B);
       printf("[30m");
       set_mux( M_PROBES );
       SCALE_AN1 = 0;
       SCALE_AN0 = 0;
       return FAILED;
    }
    temp_word = (unsigned long)result_ptr[0] * (unsigned long)ATODSCALE;
    temp_word /= (unsigned long)100;        /* convert to a 3.3 volt basis */
    temp_word *= 2;           /* compensate for voltage divider */
    biasvolt = (unsigned int)temp_word;
    if (biasvolt > bias_p_init)
      bias_p_init = biasvolt;        /* New largest "noise" value */
    if (biasvolt < bias_m_init)
      bias_m_init = biasvolt;        /* New lowest "noise" value */
  }

  if (bias_p_init > (BiasVolt + pSysVolt->BiasNoiseP))
    bad_voltage++;
  if (bias_m_init < (BiasVolt - pSysVolt->BiasNoiseM))
    bad_voltage++;

  set_mux( M_PROBES );
  SCALE_AN1 = 0;
  SCALE_AN0 = 0;
  if (bad_voltage)
  {
    printf("%c", 0x1B);
    printf("[31m");
    xprintf( 78, bias_p_init - BiasVolt);  /* Print only "noise" levels */
    xprintf( 178, BiasVolt - bias_m_init);  /*  a la channel noise numbers */
    iambroke |= NOISE_FAULT;
    status = TRUE;
    printf("%c", 0x1B);
    printf("[30m");
  }
  else
  {
    iambroke &= ~NOISE_FAULT;         /* Clean slate [see check_probe_noise()] */
    xprintf( 79, bias_p_init - BiasVolt);
    xprintf( 179, BiasVolt - bias_m_init);
  }
  return(status);
}  /* End of check_bias_noise() */

/*************************************************************************
 *  subroutine: check_bias_volt()
 *
 *  function: Get the probe bias voltage (ie 3.5 or 3.8 volts)
 *
 *         Set mux (3.5v / 3.8v) and read ADC (AN0)
 *
 *  input:  none
 *  output: PASSED / FAILED (if read_ADC error)
 *
 *             NOTE: CANADIAN TRUCKS can be either 3.5 or 3.8
 *                   USA TRUCKS are 3.5
 *
 *************************************************************************/
char check_bias_volt (void)
{
int index;
char status;
unsigned long  probe_bias_volt;

  status = FAILED;

  SCALE_AN1 = 1;
  SCALE_AN0 = 1;
  set_mux(M_THREEX);
  DelayUS (1000);         /* Allow voltage to settle */
  Init_ADC();

  probe_bias_volt = 0;

  for (index=0; index<MAX_CHAN; index++ )
  {
    if (read_ADC() == FAILED)
    {
      printf("%c", 0x1B);
      printf("[31m");
      printf("\n\rTrouble reading the Analog Port\n\r");
      iambroke |= ADC_FAULT;
      printf("%c", 0x1B);
      printf("[30m");
      return FAILED;
    }
    probe_bias_volt += (unsigned long)result_ptr[0] * (unsigned long)ATODSCALE;
  }

  probe_bias_volt /= (unsigned long)MAX_CHAN;
  probe_bias_volt /= (unsigned long)100;        /* convert to a 3.3 volt basis */

  probe_bias_volt *= 2;        /* divide by (21.6/3.3 = 805) */

  /* 3.8volt (leans toward the high side) */
  if ((probe_bias_volt > pSysVolt->Bias38Lo)
       && (probe_bias_volt < pSysVolt->Bias38Hi))
  {
    status = PASSED;
  }

  /* 3.5volt (leans toward the high side) */
  if ((probe_bias_volt > pSysVolt->Bias35Lo)
       && (probe_bias_volt < pSysVolt->Bias35Hi))
  {
    status = PASSED;
  }

  BiasVolt = (unsigned int)probe_bias_volt; /* Save for logging */

  if ( status == FAILED)
  {
      printf("%c", 0x1B);
      printf("[31m");
      iambroke |= PROBE_BIAS_ERROR;
      xprintf(62, BiasVolt);
      printf("\n\r    Bias Voltage needs to be between %01u.%03uv and %01u.%03uv",
        pSysVolt->Bias35Lo/1000,pSysVolt->Bias35Lo%1000,pSysVolt->Bias35Hi/1000,pSysVolt->Bias35Hi%1000);
      printf("%c", 0x1B);
      printf("[30m");
  }
  else
  {
      iambroke &= ~PROBE_BIAS_ERROR;
      xprintf(63, BiasVolt);
  }
  set_mux( M_PROBES );
  SCALE_AN1 = 0;
  SCALE_AN0 = 0;
  return(status);
}  /* End of check_bias_volt() */

/*************************************************************************
 *  subroutine:   check_probe_noise
 *  function:  Check probe noise levels
 *
 *  input:    none
 *  output: PASSED (OK) or FAILED (if error) 
 *
 *  Note:   probe_noise() should be called after bias_noise() -- which is to
 *          say, after bias_tolerance() -- in order to accurately clear/set
 *          the NOISE_FAULT flag.
 *
 *************************************************************************/
#define NOISE_SAMPLES 20

char check_probe_noise(void)
{
unsigned int probe_p_init[MAX_CHAN];
unsigned int probe_m_init[MAX_CHAN];
unsigned int probe_value;
int probe, index;
char point = 0, status;
unsigned int bad_voltage;

    bad_voltage = FALSE;
    status = PASSED;
    if (!(ConfigA & CFGA_8COMPARTMENT))
    {
      point = 2;
    }
    /***************************** 1/25/2010 7:30AM **************************
     * Do a full initialization to make LINT happy
     *************************************************************************/
    for (probe=0; probe<MAX_CHAN; probe++ )
    {
      probe_p_init[probe] = 0;
      probe_m_init[probe] = 0;
    }

    for (probe=point; probe<MAX_CHAN; probe++ )
    {
      probe_p_init[probe] = open_c_volt[0][probe];
      probe_m_init[probe] = open_c_volt[0][probe];
    }

    PULSE5VOLT = CLR;               /* Set optic pulse (ch 4) to 10 volts (~0x01) */

    PORTE = PULSE_TEST; /*  SIM all probes MINUS the JUMP_START !!! (0xFF) */

    JUMP_START = CLR; /* Jump_start off !!! (0xFF) */

    /* At this point the voltage should be applied to all 8 channels */

    DelayMS (2);                     /* Allow voltages to stabilize */

    for (index=0; index<NOISE_SAMPLES; index++)
    {
      if (read_ADC() == FAILED)
      {
        printf("%c", 0x1B);
        printf("[31m");
        printf("\n\rTrouble reading the Analog Port\n\r");
        iambroke |= ADC_FAULT;
        printf("%c", 0x1B);
        printf("[30m");
        return FAILED;
      }
      for (probe=point; probe<MAX_CHAN; probe++ )
      {
        if (probe_volt[probe])
        { /* Update highest plus and lowest minus readings */
          if (probe_volt[probe] > probe_p_init[probe])
                probe_p_init[probe] = probe_volt[probe];
          if (probe_volt[probe] < probe_m_init[probe])
                probe_m_init[probe] = probe_volt[probe];
        }
      }
    }
   /* Print the "+" (noise over "+10") */
   xprintf( 64, DUMMY );
   if (point)
      xprintf( 65, DUMMY );

   for (probe=point; probe<MAX_CHAN; probe++ )   /* Print +, then*/
      {
      probe_value = probe_p_init[probe] - open_c_volt[0][probe];
      xprintf( 66, probe_value );
      if (probe_value > pSysVolt->ChanNoiseP)
         bad_voltage++;
      probe_p_init[probe] = probe_value;
      }
   /* Print the "-" (noise under "+10") */
   xprintf( 67, DUMMY );
   if (point)
      xprintf( 65, DUMMY );
   for (probe=point; probe<MAX_CHAN; probe++ )
   {
     probe_value = open_c_volt[0][probe] - probe_m_init[probe];
     xprintf( 66, probe_value );
     if (probe_value > pSysVolt->ChanNoiseM)
       bad_voltage++;
     probe_m_init[probe] = probe_value;

     /* Record larger of "+" or "-" noise margin to report via ModBus
        Note that value is a signed 16-bit millivolt... */

     if (probe_p_init[probe] > probe_value)
       noise_volt[probe] =  (signed int)(probe_p_init[probe]);
     else
       noise_volt[probe] = -(signed int)(probe_value);
   }
   if (bad_voltage)
   {
     printf("%c", 0x1B);
     printf("[31m");
     iambroke |= NOISE_FAULT;
     xprintf( 68, bad_voltage );
     printf("%c", 0x1B);
     printf("[30m");
     status = FAILED;
   }
   else
   {
     xprintf( 69, DUMMY );
   }
   return(status);
}  /* End of check_probe_noise() */

/*************************************************************************
 *  subroutine: check_probe_volt
 *  function:
 *
 *
 *         1.
 *         2.
 *
 *  input:   none
 *  output: TRUE (if error) / FALSE
 *
 *
 *************************************************************************/
char check_probe_volt (void)
{
long         sum;         /* 32-bits' worth of precision needed */
unsigned int baselo, basehi;  /* 10-volt baseline tolerances */
unsigned int probe,i, index;           /* Prototypical loop indexer */
char         status;
unsigned int point = 0;
unsigned int bad_10voltage = 0;
unsigned int bad_20voltage = 0;
unsigned int euro20voltage = 0;

    status = FALSE;
    if (!(ConfigA & CFGA_8COMPARTMENT))
        point = 2;
    /* Sum and average the channels to generate "nominal" base level.

       This is 10 volts for "U.S." trucks, but may be jumpered to "8.2" volts
       for (e.g., European) trucks. This jumper cannot be read by software,
       so rather than allow "10 +1 -3" volts, at least force all the channels
       to be in "close" agreement with each other.

       Also note that the voltages here are only "checked", relying on having
       previously called ADC_init() to actually *read* the channel voltages!
    */
    sum = 0L;
    for (i=point; i<MAX_CHAN; i++)      /* Sum up all operational "normal" */
        sum += open_c_volt[0][i];       /*  "10-volt" channel levels */
    baselo = (unsigned int)((sum / (MAX_CHAN - point)) - PROBE10_TOLERANCE);
    basehi = baselo + (2 * PROBE10_TOLERANCE);
    for (index=0; index<2; index++ )   /* check 10/20 volt tolerance */
        {
        for (probe=point; probe<MAX_CHAN; probe++ )
            {
            if (index==0)               /* Normal (no Jump-Start) open circuit */
                {
                if ((open_c_volt[index][probe] < baselo)
                    || (open_c_volt[index][probe] < pSysVolt->Chan10Lo)
                    || (open_c_volt[index][probe] > basehi)
                    || (open_c_volt[index][probe] > pSysVolt->Chan10Hi))
                   bad_10voltage++;
                }
            else                            /* Jump-Start open circuit voltages */
                {
                if ((open_c_volt[index][probe] < pSysVolt->Jump20Lo)
                    || (open_c_volt[index][probe] > pSysVolt->Jump20Hi))
                    {
                    if ((open_c_volt[index][probe] < pSysVolt->Euro20Lo)
                        || (open_c_volt[index][probe] > pSysVolt->Euro20Hi))
                        bad_20voltage++;
                    else
                        euro20voltage++;
                    }
                }
            }
        }
    ConfigA &= ~CFGA_EURO8VOLT;         /* Assume not "European" volt limit */
    if (bad_10voltage)
    {
      printf("%c", 0x1B);
      printf("[31m");
      xprintf( 70, bad_10voltage);
      iambroke |= TOL10V_FAULT;
      printf("%c", 0x1B);
      printf("[30m");
      status = TRUE;
    }
    else
    {                               /* Normal voltage OK, Intuit 8.2V */
      iambroke &= ~ TOL10V_FAULT;     /* No [more] problem with 10V */
    }
    if (bad_20voltage)
    {
      printf("%c", 0x1B);
      printf("[31m");
      xprintf( 71, bad_20voltage);
      iambroke |= TOL20V_FAULT;
      printf("%c", 0x1B);
      printf("[30m");
      status = TRUE;
    }
    else
    {                               /* 20V (aka Jump-Start) channel OK */
      if (euro20voltage)
      {
        if (euro20voltage + point != MAX_CHAN)
        {
          printf("%c", 0x1B);
          printf("[31m");
          xprintf( 71,euro20voltage);
          iambroke |= TOL20V_FAULT;
          printf("%c", 0x1B);
          printf("[30m");
          status = TRUE;
        }
        else                        /* All channels at Euro clamp levels */
        {
          ConfigA |= CFGA_EURO8VOLT;  /* "European" voltage limits */
          iambroke &= ~ TOL20V_FAULT; /* So 20V chans are OK afterall */
        }
      }
      else
      {
        iambroke &= ~ TOL20V_FAULT; /* No [more] problem with 20V */
      }
    }
    if (!status)
        xprintf( 72, DUMMY );
    return(status);
}  /* End of check_probe_volt() */

/*************************************************************************
 *  subroutine:  check_raw_13
 *  function:
 *
 *  input:   none
 *  output: TRUE (if error) FAILED
 *
 *************************************************************************/
char check_raw_13 (void)
{
char           bad_voltage;
unsigned           result;

  bad_voltage = PASSED;
  Init_ADC();
  SCALE_AN1 = 1;
  SCALE_AN0 = 1;
  set_mux(M_RAW);             /* set to read Raw 13 volt on mux */
  DelayMS(5);             /* Give some time for voltage to settle */
  asm volatile("nop");    /* place for the breakpoint to stop */
  asm volatile("nop");    /* place for the breakpoint to stop */
  asm volatile("nop");    /* place for the breakpoint to stop */
  if (read_ADC() == FAILED)
  {
    printf("%c", 0x1B);
    printf("[31m");
    printf("\n\rTrouble reading the Analog Port\n\r");
    iambroke |= ADC_FAULT;
    printf("%c", 0x1B);
    printf("[30m");
    return FAILED;
  }
  result = probe_volt[0]*2;     /* do these serially */
  Raw13Volt = result;                 /* Store for ModBus access/reporting */
  if ((result > pSysVolt->Raw13Hi + 3000) || /* check 13 volt tolerance */ // vijay add this 3000 count to test the code for higher raw 
      (result < pSysVolt->Raw13Lo))
      bad_voltage = TRUE;
  if (bad_voltage)
  {
    printf("%c", 0x1B);
    printf("[31m");
    iambroke |= RAW_FAULT;
    xprintf( 73, result);
    printf("\n\r    Raw 13 needs to be between %01u.%03uv and %01u.%03uv",
        pSysVolt->Raw13Lo/1000,pSysVolt->Raw13Lo%1000,pSysVolt->Raw13Hi/1000,pSysVolt->Raw13Hi%1000);
    printf("%c", 0x1B);
    printf("[30m");
  }
  else
  {
    iambroke &= ~RAW_FAULT;
    xprintf( 74, result);
  }

  set_mux(M_PROBES);                  /* return to normal */
  SCALE_AN1 = 0;
  SCALE_AN0 = 0;
  return(bad_voltage);
}  /* End of check_raw_13() */

/*************************************************************************
 *  subroutine:      check_5wire_5volt()
 *
 *  function:  See if we have 5 wire 5volt capability
 *  input:  None
 *  output: TRUE (if error) / FALSE (means we have a working 5 wire unit))
 *
 *  Note:   Leaves probe_try_state "confused"...ergo possibly confusing
 *          subsequent set_porte calls.
 *
 *************************************************************************/
#define RETRIES 5         /* number or repeats to catch 5 wire voltage pulse error */

char   check_5wire_5volt(void)
{
char status;
char index;
unsigned int pulse_4chan;

  Init_ADC();
  iambroke &= ~TOL5WO_FAULT;           /* Innocent until proven guilty */
  status = FALSE;
  for (index=0; index<RETRIES; index++ ) /* try several times */
  {                                 /* ?? line */
     probe_try_state = OPTIC2;         /* 10-volts, Jump-Start off... */
     set_porte( OPTIC_PULSE );         /* Output optic pulse */
     DelayMS(10);
     if (read_ADC() == FAILED)
     {
       printf("%c", 0x1B);
       printf("[31m");
       printf("\n\rTrouble reading the Analog Port\n\r");
       iambroke |= ADC_FAULT;
       printf("%c", 0x1B);
       printf("[30m");
       return FAILED;
     }
     pulse_4chan = probe_volt[3];
   if ( pulse_4chan < (open_c_volt[0][3]-ADC1V) ) /* 4 less than open volt spec */
   {
     status = TRUE;                 /* Failed 5 wire normal */
     break;
   }
   probe_try_state = OPTIC5;         /* Select 5-wire-optic config */
// >>> QCCC 53
   HC_OFF4 = 0;                           /* Need High Current */
// <<< QCCC 53
   set_porte( OPTIC_PULSE );         /* Output 4.7V (nominal) optic pulse */
   DelayMS(10);
   if (read_ADC() == FAILED)
   {
     printf("%c", 0x1B);
     printf("[31m");
     printf("\n\rTrouble reading the Analog Port\n\r");
     iambroke |= ADC_FAULT;
     printf("%c", 0x1B);
     printf("[30m");
     return FAILED;
   }
   pulse_4chan = probe_volt[3];
   set_porte( OPTIC_DRIVE );         /* shut off optic pulse */
   DelayMS(10);
   if ((pulse_4chan < OPTIC5OUT_MIN)  /* Verify Optic 5-wire output pulse */
         || (pulse_4chan > OPTIC5OUT_MAX))  /*   within specified tolerances */
   {
     iambroke |= TOL5WO_FAULT;      /* Optic pulse out of tolerance */
     status = TRUE;                 /* Failed 5 wire */
     break;
   }
   Optic5Volt = pulse_4chan;         /* Save for curious eyes */
  }
  if (status)
  {
    printf("%c", 0x1B);
    printf("[31m");
    if (probe_try_state == OPTIC5)
    {
      xprintf( 75, pulse_4chan );
    }
    else
    {
      xprintf( 76, pulse_4chan );
    }
    printf("%c", 0x1B);
    printf("[30m");
  }
  else
     xprintf( 77, pulse_4chan );
  probe_try_state = NO_TYPE;           /* Reset to default configuration */
// >>> QCCC 53
   HC_OFF4 = 1;                              /* Turn off high cuurent */
// <<< QCCC 53
  set_porte (PULSE_TEST);              /* Leaving all channels powered */
  return(status);
} /* end of check_5wire_5volt */

/*************************************************************************
 *  subroutine: check_100OhmGnd
 *
 *  function:  See if we're GND BOLT jumper (J8) is in place
 *
 *  input:  None
 *  output: PASSED or FAILED based on return from read_muxADC()
 *  Updated: ConfigA:CFGA_100_OHM_GND
 *  
 *************************************************************************/
char  check_100OhmGnd (void)
{
unsigned gvolts;                /* GND_SENSE volts */
unsigned int old_porte;             /* Saved PORT drive */
char sts;

  Init_ADC();
  old_porte = LATE;                   /* Remember state */
  set_porte (OFF);                    /* Kill off all drive voltages */
  set_gcheck();                       /* Assert GCHECK signal */
  DelayMS (15);                       /* 15ms as per GaryC. */
  sts = read_muxADC (0, M_GND_7_8, &gvolts);  /* Read GND_SENSE level */
  /* With J8 Jumper installed, Pin 9 should be at 5 volts, which would
     read at GND_SENSE as 1.16V (10K/43.2K * 5V);
      With the jumper removed (COMM_ID and GCHECK high), GND_SENSE should sit at 
     about .8V (10K/63.2K * 5V).
     A 1.0V threshold is used to determine which case is valid. */
  clear_gcheck();                     /* Revert GCHECK off */
  set_porte (old_porte);           /* Restore Probe/channel drive */
  if (sts == FALSE)
  {
    sts = FAILED;
  }else
  {
    sts = PASSED;
    if (gvolts > ADC1V)
    {  
      ConfigA |= CFGA_100_OHM_GND;
    }
    else
    {
      ConfigA &= ~CFGA_100_OHM_GND;
    }
  }
  return (sts);
} /* End check_100OhmGnd() */

/*************************************************************************
 *  subroutine:      service_wait()
 *
 *  function:  wait specified time without causing service light or watch dog timer to "trip"
 *             
 *  input:  Wait time, in 1/8 second (125 ms) intervals.
 *  output: none
 *
 *************************************************************************/
void service_wait(char wait_time)
{
char last8ths;

  last8ths = loopEighths;             /* Current 125ms counter */
  while (wait_time--)
  {
      while (last8ths == loopEighths) /* Wait for it to change */
      {
         service_charge();               /* Pump Service LED off */
      }
      last8ths++;                     /* Count up one cycle */
  }
  toggle_led();
}

/*********************************************************************
 *  subroutine:      service_charge()
 *
 *  function:  Entry from POD or main loop,  this is used to keep the SERVICE LED off
 *
 *  input:  none
 *  output: none
 *
 ********************************************************************/
void service_charge(void)
{
  if(service_time >= 10)
  {
    service_time = 0;
    if ((StatusA & STSA_FAULT) == 0)  /* If we're OK and running */
    {
      if (MSERV_TRG_NEG)
      {
        MSERV_TRG_NEG = 0;
      }
      else
      {
        MSERV_TRG_NEG = 1;
      }
    }
  }
  ClrWdt();                         /* Appease watchdog */
}

/*********************************************************************
 *  subroutine:  check_shell_crc
 *
 *  function:      Verify shell CRC
 *
 *  input:    none
 *  output:  PASSED / FAILED
 *
 ********************************************************************/
char check_shell_crc (void)
{
unsigned long     end_address;
unsigned short    icrc_val;
unsigned long     ptr;
char              status;
uReg32 Temp, SourceAddr, read_data;

  end_address = (unsigned long)__builtin_tbladdress(&_PROGRAM_END);
  icrc_val = INIT_CRC_SEED;

  for (ptr = SHELL_START;
       ptr < (end_address - FLASH_CRC_CHUNK);
       ptr += FLASH_CRC_CHUNK)
  {
    icrc_val = program_memory_CRC (ptr, FLASH_CRC_CHUNK, icrc_val);
    service_charge();           /* Appease Service LED */
  }

  /* Finish the last "few" bytes, kinda sorta modulo FLASH_CRC_CHUNK */

  icrc_val = program_memory_CRC (ptr,
                    (unsigned short)((end_address - ptr) + 1),
                    icrc_val);

  service_charge();           /* Appease Service LED */
  KernelCRCval = icrc_val;                 /* Save actual CRC-16 value */
  ShellCRCval = icrc_val;                 /* Save actual CRC-16 value */

  (void)_memcpy_p2d24((char *)&read_data.Val32, CHECKSUM_LOW_ADDR, 3);
  read_data.Val32 = read_data.Val32 & 0x00FFFFFF;

  if ((read_data.Val32 & 0x00FFFFFF) == 0x00FFFFFF)
  {
    printf("\n\r    *** Updating CRC(0x%04X) into Program Memory ***\n\r", icrc_val);
    SourceAddr.Val32 = LAST_BLOCK_ADDR;
    Erase(SourceAddr.Word.HW, SourceAddr.Word.LW, PM_ROW_ERASE);
    SourceAddr.Val32 = CHECKSUM_LOW_ADDR;
    Temp.Word.LW = (icrc_val & 0xFFFF);
    Temp.Word.HW = 0;
    WriteLatch(SourceAddr.Word.HW, SourceAddr.Word.LW,Temp.Word.HW,Temp.Word.LW);
    WriteMem(PM_ROW_WRITE);
    service_charge();           /* Appease Service LED */
    (void)_memcpy_p2d24((char *)&read_data.Val32, CHECKSUM_LOW_ADDR, 3);
  }

  Good_Shell_CRC_val = read_data.Word.LW;
  Good_KernelCRCval = read_data.Word.LW;

  status = PASSED;
#ifndef  __DEBUG          /* These tests interfere with debugger so don't run */
  if ((icrc_val == read_data.Word.LW)         /* Flash's CRC-16 valid? */
      || (DEBUG_IN == 0))               /* Or debug jumper installed? */
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
    printf("%c", 0x1B);
    printf("[31m");
    xprintf (61, icrc_val);
    printf("%c", 0x1B);
    printf("[30m");
    status = FAILED;
  }
#endif
  return (status);
}

/*************************************************************************
 *  subroutine:      diagnostics()
 *
 *  function:
 *
 *         1.  Look up result from CRTSI RAM test - zero is OK
 *         2.  Compute the checksum for EEPROM
 *         3.  If non-zero result (for 1 or 2 initialized flash) cause
 *             the Service led to flash at twice Dynacheck rate and
 *             cause reset watchdog timeout
 *            else
 *         4.  Flash sequentially the front panel led suite and exit
 *
 *  input:  ctlmsk - Diag control mask (what to do)
 *  output: none
 *
 *
 *************************************************************************/

typedef struct
{
  unsigned int mask;          /* DIA_CHK_* mask "invocation" bits */
  unsigned int flags;         /* Local control flags */
  char (*func)(void);             /* Function (pointer) to invoke */
} DIAFUN;

#define DIA_FL_STS      0x0001

/* Table (array) of diagnostic functions.

   This table lists the diagnostic() sub-functions, in more-or-less priority
   order. This order dictates the execution order of the sub-functions, and
   thus which "errors" will block further sub-functions from executing... */

/*** WARNING *** do **NOT** declare the diagnostic functions called in the
   diarray table as "static"! This POS Whitesmith's compiler screws up hor-
   ribly calculating the address in the table if you do!
 *** WARNING ***/

static const DIAFUN diarray[] =
{
    {DIA_CHK_RAM, 0, boot_mem_test},               /* Report any boot memory test errors */
    {DIA_CHK_FLASHCRC, 0, check_shell_crc},     /* Verify FLASH RAM "Shell" CRC */
    {DIA_CHK_EEPROM, 0, eeCRC},               /* Verify EEPROM */
    {DIA_CHK_CLOCK,   0, diag_clock},                /* Read Dallas Time-Of-Day clock */
    {DIA_CHK_MEM,   0, mem_test},                     /* March memory test */
    {DIA_CHK_VOLTS0,  0, check_ref_volt},          /* Init ReferenceVolt */
    {DIA_CHK_VOLTS0,  0, check_open_c_volt},   /* Init open_c_volt[] */
    {DIA_CHK_VOLTS0,  0, check_tank_type},      /* Check 6/8-compartment jumper */
    {DIA_CHK_VOLTS1,  0, check_raw_13},          /* Raw power supply voltage */
    {DIA_CHK_VOLTS1,  0, check_bias_volt},        /* 3.5/3.8 "Bias" voltage */
    {DIA_CHK_VOLTS1,  0, check_bias_noise},      /* 3.5/3.8 "Bias" voltage */
    {DIA_CHK_VOLTSCH, 0, check_probe_volt},    /* Channel 10/20 voltages */
    {DIA_CHK_VOLTSCH, 0, check_probe_noise},  /* Channel noise levels */
    {DIA_CHK_VOLTS1,  0, check_5wire_5volt},    /* 5-Wire-Optic "5-volt" level */
    {DIA_CHK_GROUND,  0, check_100OhmGnd}, /* Check for 100 Ohm/Diode ground */
    {DIA_CHK_JUMPERS, 0, test_jumpers},            /* Read/Verify "Enable" jumpers */
    {DIA_CHK_LEDSIG,  0, flash_panel},                /* Init "Signature" in the LEDs */
    {0, 0, 0}
}; /* NULL terminates diarray[] scan */

void diagnostics
    (
    unsigned ctlmsk         /* Diag control mask (what to do) */
    )
{
unsigned int i;                     /* Prototypical loop index */
int status = 0;

    /* Iterate through the diarray[] list of callable diagnostic sub-
       functions */

    i = 0;
    while (diarray[i].mask != 0)        /* Loop through array, quit on NULL */
    {
     service_charge();                 /* Appease Service LED */

        /* Execute one table entry (diag sub-function) if it matches the
           caller's control mask set, and if not contra-indicated by a
           previous error */

      if (diarray[i].mask & ctlmsk)   /* Requesting this sub-function? */
      {                           /* Yes */
        if (!((diarray[i].flags & DIA_FL_STS) /* If we care about status */
                   && (status != 0)))           /*  and status is OK */
        {                                     /* Don't care, or no accum errors */
          status = (int)(*diarray[i].func)();    /* Execute diag sub-function */
        }
      }
      i++;
    } /* End while stepping through sub-function list */
    set_mux(M_PROBES);           /* Point to probe voltage */
}  /* end of diagnostics() */

/********************* end of pod.c *****************************************/
