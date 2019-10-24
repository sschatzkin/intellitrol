/*********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         sim.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *  Description:  LED routines for Rack controller for main microprocessor
 *                PIC24HJ256GP210. Also mux control and relay control routines.
 *
 * Revision History:
 *   Rev      Date           Who   Description of Change Made
 *  --------  --------------  ---    --------------------------------------------
 *  1.5     09/23/08  KLL  Ported old Intellitrol code to the PIC24HJ256GP210 cpu
 *  1.5.30  08/10/14  DHP  In EnaFeatures() added unit_type support
 *  1.6.32  03/30/15  DHP  In read_relays() changed relay_index>MAX_RELAY to >=
 *                         The relay updates based on relay_index were outside
 *                          the  arrays on relay_index = MAX_RELAY.
 *  1.6.34  10/10/16  DHP  Added increment of service_time to timer_heartbeat().
 *                         Correct display_probe() for counts greater than 8 and
 *                          changed indexing to match ledstate array indexing.
 **********************************************************************************************/
#include "common.h"
#define SW_MED_SLOW    3     /* 3/8's second */

/**************************************************************************
* ledtrans[] -- translate LED to MM5480 "pins"
*
* ledtrans[] is the LED driver translation array:
*
*   0       Don't care/idle bit, clock out a "0" to the MM5480
*   n       LED number 'n' (index into ledstate array)
*  -1       "Reset" indication (end of table, start over).
*
* The table is 128 (125...) entries long, resetting at 125, which yields a
* 125-millisecond cycle or 8Hz basic LED rate (yields 4Hz fast blink).
*
* The "excessive" length is to minimize compute cycles in timer_heartbeat()
* at the expense of static FLASHRAM code space. It has the interesting side
* effect of guaranteeing re-synchronization of the MM5480 on a periodic
* basis (which should normally not be needed, but allows you to un/plug the
* LED display board, and the system always resynch's within a fraction of a
* second. I don't know if this is really needed, but the old
* driver did this, so I maintained the concept...   -RDH 26-Jun-95
***************************************************************************/

static const signed char ledtrans[128] =
{   0,  1,  2,  3,  4,  5,  6,  7,      /* MM5480 Data bits   1 -   8 */
    0,  0,  0,  8,  9, 10, 11,  0,      /* MM5480 Data bits   9 -  16 */
    0,  0,  0, 12, 13, 14, 15, 16,      /* MM5480 Data bits  17 -  24 */
   17,  0, 18,  0,  0, 19, 20, 21,      /* MM5480 Data bits  25 -  32 */
   22, 23, 24, 25, 26, 27, 28,          /* MM5480 Data bits  33 -  35 */
                                0,      /*        Idle bits  36 -  40 */
    0,  0,  0,  0,  0,  0,  0,  0,      /*        Idle bits  41 -  48 */
    0,  0,  0,  0,  0,  0,  0,  0,      /*        Idle bits  49 -  56 */
    0,  0,  0,  0,  0,  0,  0,  0,      /*        Idle bits  57 -  64 */
    0,  0,  0,  0,  0,  0,  0,  0,      /*        Idle bits  65 -  72 */
    0,  0,  0,  0,  0,  0,  0,  0,      /*        Idle bits  73 -  80 */
    0,  0,  0,  0,  0,  0,  0,  0,      /*        Idle bits  81 -  88 */
    0,  0,  0,  0,  0,  0,  0,  0,      /*        Idle bits  89 -  96 */
    0,  0,  0,  0,  0,  0,  0,  0,      /*        Idle bits  97 - 104 */
    0,  0,  0,  0,  0,  0,  0,  0,      /*        Idle bits 105 - 112 */
    0,  0,  0,  0,  0,  0,  0,  0,      /*        Idle bits 113 - 120 */
    0,  0,  0,  0, -1, -1, -1, -1       /* "Reset" at 125 = 8Hz "LED cycle" */
    };

static char ledindex;           /* Current ledtrans LED/state index */
static char ledtick = 0;            /* Current LED cycle (125Ms) "tick" */

/***************************************************************************
 *      Purpose:        Initialize LED display
 *      inputs:
 *
 *      returns:        nothing
 *      side effects    none
 *
 *      Should be called before general-purpose 1-Ms timer enabled!
 *
 **************************************************************************/

void    init_led(void)
{
  memset (ledstate,DARK,sizeof(ledstate));    /* all leds off except */
  ledindex = 125;                     /* Trigger "reset" cycle on first int */
  global_count = 0;
  start_bit = 0;          /* Used to keep track when the heart beat is sending a start bit to the front panel */
}        /*  end of init_led()  */

/*************************************************************************
 *  subroutine:      timer_heartbeat()
 *
 *  function:
 *
 *         1.  Entry to this routine is from the T2Interrupt ISR
 *         2.  1 ms (MSec) timers 'freetimer' and 'mstimer' are updated
 *         3.  The fold over time for the PITM in PICR is set for 1ms
 *         4.  The level of interrupt is 6
 *         5.  LED display now driven by Ms timer...
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/
void timer_heartbeat(void)
{
static char toggle =0;
signed char ledarx;          /* LED number */
signed char ledval;          /* LED state for #ledarx LED */
char        ledbit;          /* LED logic level, more or less */
int array_index; /* Fix compiler problem that will not let a char be used to index an array */
volatile int delay_count;
    /* Drive the LED display; enter clock "low" phase, give signal a few
       usecs to settle (thus putting clock low "way up here" in the routine */
    LED_DISP_CLK = CLR;    /* LED serial "clock" low state */

   /* The Periodic Interrupt actually comes by at 1.125 (20MHz Fcy),
      for approximately 1.125 Millisecond interval; adjust the long-term
      millisecond counters appropriately - skip every 8th click */

   mstimer++;                 /* milliseconds... */
   freetimer++;               /* bump 1 ms counter */
   service_time++;          /* bump servicecounter */
   if (tank_state == T_DRY)
      dry_timer++;            /* bump 1 ms counter if dry */

   /* Drive the LED "data" line high to check for impact sensor being trig-
      gered. Since the line can take a few usecs to settle down (or up, as
      the case may be), put this line of code "well before" the read-back
      of the data line state. */

  LED_DISP_DAT = SET;    /* Assert LED data line high */

  toggle++;
  if ( toggle >= 4 )         /* Changed to >= to fix 50 HZ problem SJM 2-12-96 */
  {
    read_relays();          /* read the relay state @ 4ms rate */
    toggle = 0;
  }

  ledbit = 0;               /* Assume LED "OFF" */

    /* Check for "impact sensor" being triggered. This manifests itself as
       the LED data line being held low for approximately a second. This will
       of course "glitch" the LED data being clocked into the MM5480 chip,
       but so what -- if someone is beating on the poor 'troll, it should be
       allowed to cry out in pain and confusion . . .

       By now, the LED data line should have reliably asserted high, if it is
       ever gonna get there. */

#if 0  /* This has never been used on PIC CPU; unknown why - may be H/W issue */
  READ_IMPACT_SENSOR = 1;     /* Setup to read the Impact Sensor */
  if (IMPACT_SENSOR == 0)     /* Is LED data being held low? */
  {                           /* Yes - impact sensor triggered */
    hitCount++;               /* Note abuse for logging purposes */
  }
  READ_IMPACT_SENSOR = 0;     /* Put back to control the LEDs */
#endif

  array_index = ledindex++;
  ledarx = ledtrans[array_index];
  if (ledarx > 0)  /* Next LED bit/state */
  {                               /* Active LED entry */
    /* Dispatch on LED action (DARK, FLASH, etc.) */

    /* Fix compiler problem that will not let a char be used to index into an array */
    array_index = ledarx;
    ledval = ledstate[array_index];
    switch (ledval) /* State of this LED */
    {
      case LITE:                    /* LED is ON */
        ledbit++;                   /* Flag it logically so */
        break;

      case PULSE:                   /* LED is PULSED ON (one-shot) */
        ledbit++;                   /* Set LED data bit to logic high */
        /* Fix compiler problem that will not let a char be used to index into an array */
        array_index = ledarx;
        ledstate[array_index] = DARK;    /* Clear LED for further cycles */
        break;

      default:                      /* All other states are "blinks" */
                                    /* ("DARK" is blink never) */
        if (ledtick & ledval)       /* "ON" or "OFF" phase? */
          ledbit++;                 /* Blink "ON" */
                                    /* Else blink "OFF" */
    } /* End switch on ledval */
  }
  else
  {
    if (ledarx < 0)                 /* Negative bit/state value is "reset" */
    {                               /* Happens every 125 milliseconds */
      start_bit = 1;
      global_count = 0;
      loopEighths++;                /* Tell "Loop" level time is passing */
      ledtick++;                    /* Another "LED" cycle */
      ledindex = 0;                 /* Reset to initial scanning state */
      ledbit++;                     /* Drive "Start" data bit high to sync */
                                    /* Next cycle will be LED "0"... */
      Led_map_low_word_0 = 0;
      Led_map_low_word_1 = 0;
      Led_map_low_word_2 = 0;
    }
  }

  /* !!!!!!!!! PERMIT and NON-PERMIT LEDs are complemented !!!!!!!!! */
  if ((ledarx == (signed char)PERMIT) || (ledarx == (signed char)NONPERMIT))
    ledbit--;

  if (ledbit)                       /* And set the OUTPUT bit */
  {
    if ( start_bit == 1)
    {
      if ( global_count < 16)
      {
        Led_map_low_word_0 |= ((unsigned int)1 << global_count);
      } else
      if ( global_count < 32)
      {
        Led_map_low_word_1 |= ((unsigned int)1 << (global_count - 16));
      } else
      if ( global_count < 48)
      {
        Led_map_low_word_2 |= ((unsigned int)1 << (global_count - 32));
      } else
      {
        start_bit = 0;
      }
    }
    LED_DISP_DAT = SET;
  }
  else
  {
    LED_DISP_DAT = CLR;
  }

  /****************************** 8/15/2008 11:09AM **************************
   * Give some setup and hold time
   ***************************************************************************/
  for ( delay_count = 0; delay_count<5; delay_count++)
  {
  }
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  LED_DISP_CLK = SET;                 /* Lo-to-Hi clocks in data [LED] bit */
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  interrupt_count++;
                                        /* Lines left "static" till next Ms */
  if (ledbit)
  {
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
  }

  if (global_count == 48)            /* Intellitrol Will load whenever 36 clocks have been reached */
  {
    global_count++;
    start_bit = 0;
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    if (Led_map_low_word_1 == 0x8100)
    {
      asm volatile("nop");
      asm volatile("nop"); 
      asm volatile("nop"); 
    }
  }
  else
  {
    global_count++;
  }
} /* end of timer_heartbeat INTERRUPT */

/*************************************************************************
 *  subroutine:      set_mux()
 *
 *  function:
 *
 *         1.  Reset the MUX to a known state
 *
 *  input:  MUX enumeration value from file enum.h
 *  output: Previous MUX channel selection value
 *
 *************************************************************************/

void set_mux( SET_MUX mux_enum )
{
  MUX0 = ((int)mux_enum & 0x01);      /* set mux to the enumerated 0 to 7 value */
  MUX1 = (((int)mux_enum >> 1) & 0x01);
  MUX2 = (((int)mux_enum >> 2) & 0x01);
} /* end of set_mux() */

SET_MUX fetch_mux(void)
{
unsigned int mux = 0;

  mux |= (MUX0  & 0x01);
  mux |= (MUX1 << 1);
  mux |= (MUX2 << 2);
  return (SET_MUX)mux;
} /* end of fetch_mux */

/*************************************************************************
 *  subroutine:      set_porte()
 *
 *  function:
 *
 *
 *         1.  Reset the PORTE individual channels to a known state
 *         2.  JUMP_START (PORTF-0) is independently added or removed
 *         3.  The 5volt Optic5 pulse height is set via PORTB-14
 *         4.  OPTIC_PULSE, OPTIC_RTN, OPTIC_DIAG etc
 *
 *
 *  input:  Channel(s) to have active( others are set to 0)
 *  output: none
 *
 *
 *************************************************************************/

void  set_porte( UINT16 port_select )
{
    switch (probe_try_state)
        {
      case OPTIC5:                      /* 5-Wire-Optic configuration */

        PULSE5VOLT = SET;               /* Set optic pulse to 5 (4.7) volts */
        JUMP_START = CLR;               /* Jump-Start always off! */
        break;

      case OPTIC2:                      /* 2-Wire Optic probes */

        PULSE5VOLT = CLR;               /* Chan 4 normal (10V) drive */
        JUMP_START = CLR;               /* Jump-Start always off! */
        break;

      default:                          /* Whatever else comes down the pike... */
       PULSE5VOLT = CLR;               /* Chan 4 normal (10V) drive */

        if ((jump_time < read_time()) && (jump_time != 1)) /* Jump-Start? */
            jump_time = 0;              /* JUMP_START time out */
        if ((port_select != 0)          /* Disallow Jump-Start if all chan off */
            && ((jump_time == 1)        /* Jump-Start "force" on ? */
                || (jump_time > read_time()))) /* Or Jump-Start timer running? */
            JUMP_START = SET;           /* Enable Jump-Start's +20V */
        else
            JUMP_START = CLR;           /* Disable Jump-Start's +20V */

        } /* End switch on probe_try_state */

    PORTE = port_select;                /* Drive selected channels */

} /* end of set_porte */

/*********************************************************************
 *  subroutine:      read_relays()
 *
 *  function:
 *         1.  Read main and backup relay contacts for high or low.  When the relay is open a 
 *              sqaure wave with a period equal to the line voltage (50hz or 60hz) is seen.
 *         2.  Starting with a 0 byte, set a bit within the byte for each high reading for 8 reads,
 *              then move the resulting byte into an array and start over.  
 *  input:   none
 *  output: none
 *            Entries in main_array[] and bak_array[] are updated every 8 calls.
 *
 *********************************************************************/
void    read_relays(void)
{
static int relay_index = 0;
static unsigned char main_relay = 0;   /* we need only one byte each */
static unsigned char bak_relay = 0;
static unsigned char relay_bit = 0;

  if (relay_index >= MAX_RELAY)
  {
    main_relay = 0;
    bak_relay = 0;
    relay_index = 0;
  }
  if (++relay_bit>7)
  {
    relay_bit = 0;
    main_array[relay_index] = (char)main_relay;
    bak_array[relay_index] = (char)bak_relay;
    main_relay = 0;
    bak_relay = 0;
    if (++relay_index>MAX_RELAY)
      relay_index = 0;
  }
  if ( MMAIN_CONTACT )   /*  **B4** PORTA bit 0 */
  {
    main_relay |= (unsigned char)((unsigned int)1 << (unsigned int)relay_bit);
  }
  if ( MBACK_CONTACT )   /*  **B4** PORTA bit 1 */
  {
    bak_relay |= (unsigned char)((unsigned int)1 << (unsigned int)relay_bit);
  }
} /* end of read_relays */

void set_permit(char data)
{
  ledstate[PERMIT] = data;
 
}

void set_nonpermit(unsigned char data)
{
static unsigned char state = 0;

  ledstate[NONPERMIT] = (char)data;
  if ( state == data)
  {
    return;
  }
  if ( data == DARK)
  {
    asm volatile("nop");    /* place for the breakpoint to stop */
    asm volatile("nop");    /* place for the breakpoint to stop */
    asm volatile("nop");    /* place for the breakpoint to stop */
  }
  else
  {
    if (MAIN_ENABLE == SET)
    {
      asm volatile("nop");    /* place for the breakpoint to stop */
      asm volatile("nop");    /* place for the breakpoint to stop */
      asm volatile("nop");    /* place for the breakpoint to stop */
    }
  }
  state = data;
}

/******************************* 11/24/2008 6:26AM ***************************
 * Perform a byte swap because the last processor was big-endian and this
 * one is little-endian
 *****************************************************************************/
unsigned long long_swap(unsigned long original)
{
unsigned long swaped;

  swaped = (((original & 0x000000FFL) << 24) | ((original & 0x0000FF00L) << 8) |
           ((original & 0x00FF0000L) >> 8) | ((original & 0xFF000000L) >> 24));
  return swaped;
}

unsigned int int_swap(unsigned int original)
{
unsigned int swaped;

  swaped = (((original & 0x00FF) << 8) | ((original & 0xFF00) >> 8));
  return swaped;
}

/********************************************************************
 * This routine will display the number of 5-wire probes on the current truck or the 
 * 6 / 8 compartment setting for 2-wire trucks.  Intellitrol2 models will flash the count on 
 * front panel, all will report to the diagnostic screen if present.
 ********************************************************************/
void display_probe()
{
unsigned int i;

  if ((number_of_Probes > 0) && (number_of_Probes < 17)) /* Must be a IntelliCheck if 0 */
  {
//            printf("\n\r\n\r    *** This 5-Wire Truck has %u compartments ***\n\r", number_of_Probes);
    // >>> QCCC 53
//    xprintf( 170, DUMMY);    /* Tell the world how many probes the truck has */
    if(truck_state == OPTIC_FIVE)
    {
       printf("\n\r\n\r    *** This 5-Wire Truck has %u compartments ***\n\r", number_of_Probes);
       for(i=0;i<number_of_Probes;i++)
       {
         probe_type[i]=P_OPTIC5;
       }
    }
    else
    {
       printf("\n\r\n\r    *** This 2-Wire Truck has %u compartments ***\n\r", number_of_Probes);
    } 
    // <<< QCCC 53
         /* This is only at vehicle connection time and only for a dry 5-wire vehicle */ 
    if((unit_type == INTELLITROL2) && (truck_state == OPTIC_FIVE))
    {
unsigned int loop;
      if (SysParm.Five_Wire_Display != 0xFF)
        /* wait for SysParm.Five_Wire_Display seconds */
      {
        ledstate[NONPERMIT] = DARK;                   /* OFF */
        for ( loop=0; loop<SysParm.Five_Wire_Display; loop++)
        {
          if (( number_of_Probes > 8)  && (number_of_Probes <= 0x10))
          {
            for ( i=0; i<8; i++)
            {
              ledstate[(int)COMPARTMENT_1+i] = LITE;    /*solid on */
            }
            ledstate[PERMIT] = LITE;              /* Start a Flash sequence*/
            service_wait(SW_MED_SLOW);     /* wait 3/8 second */
            for ( i=0; i<8; i++)
            {
              ledstate[(int)COMPARTMENT_1+i] = DARK;
            }
            service_wait(SW_MED_SLOW);              /* wait 3/8 second */
            for ( i=0; i<(number_of_Probes-8); i++)
            {
              ledstate[(int)COMPARTMENT_1+i] = LITE;    /* excess off, leaving correct # on */
            }
            service_wait(SW_MED_SLOW);              /* wait 3/8 second */
          }
          else
          {
            for ( i=0; i<number_of_Probes; i++)
            {
              ledstate[(int)COMPARTMENT_1+i] = LITE;    /* solid on */
            }
            ledstate[PERMIT] = LITE;                  /* Start a Flash sequence */
            service_wait(SW_MED_SLOW*2);     /* wait 6/8 second */
          }
          for ( i=0; i<8; i++)
          {
            ledstate[(int)COMPARTMENT_1+i] = DARK;
          }
          ledstate[PERMIT] = DARK;                 /* Flash Off */
          service_wait(SW_MED_SLOW);          /* wait 3/8 second */
          modbus_execloop_process();            /* Serial (RS-485) Input */
        }
        /* Now turn off the LEDs that were blinking */
        for ( i=0; i<8; i++)
        {
          ledstate[(int)COMPARTMENT_1+i] = DARK;
        }
      }
      ledstate[PERMIT] = DARK;    /*solid off */
      ledstate[NONPERMIT] = LITE;                   /* OFF */
    }
  }
}

/******************************* 4/14/2009 6:54AM ****************************
 * EnaFeatures  --  Write "Features-Enable" Password
 *
 * Call is:
 *
 *      EnaFeatures ()
 *
 * EnaFeatures() decodes the "Features Password" string, validates it,
 * and sets the new "features" mask, if valid.
 *
 * Return value is the ModBus Exception code
 *
 *****************************************************************************/
MODBSTS EnaFeatures(unsigned char *psw)
{
unsigned char xx;                     /* Finagling byte */
unsigned int keysn;         /* Final 16-bit key */
unsigned char newena;                /* New enable features */
unsigned char newsftena;             /* New software enable features */
MODBSTS status;

    if (Read_Clock() != 0)         /* Fetch the intellitrol serial number */
    {
      return MB_READ_CLOCK_ERROR;
    }
    xx = psw[6];                      /* High-order CRC value byte 6 */
    asm volatile("nop");    /* place for the breakpoint to stop */
    asm volatile("nop");    /* place for the breakpoint to stop */
    asm volatile("nop");    /* place for the breakpoint to stop */
    psw[1] ^= xx;                     /* Un-obfuscate some bits */
    psw[3] ^= xx;
    psw[6] = xx;                      /* Stash Expected CRC-16 (high) */
    xx = psw[7];                      /* Low-order CRC value byte 7 */
    psw[2] ^= xx;
    psw[4] ^= xx;
    psw[7] = xx;                        /* Stash Expected CRC-16 (low) */
    xx = psw[0];                        /* Extract the random bits */
    keysn = modbus_CRC (psw, 6, INIT_CRC_SEED);
    if ((psw[6] != (unsigned char)(keysn >> 8))
        || (psw[7] != (unsigned char)(keysn)))
        return (MB_EXC_ILL_DATA);
    /* Now that we've un-XOR'ed a bunch of obfuscated bits (not especially
       "rigorous", mathematically speaking, but it makes for a pretty random-
       looking set of patterns...and is easy to undo here in the 68HC16),
       extract the useful info. Our unit serial number (low-16 bits) is in
       byte positions 1 & 2, while the new enable mask is in byte position 3.
       Byte position 5 is just quasi-random nonsense. */
    if (xx & 4)
        {                               /* Verify "addressed" to us */
        if ((psw[1] != clock_SN[4])     /* Serial Number match? */
            || (psw[2] != clock_SN[5])) /* ... */
            return (MB_EXC_ILL_DATA);   /* No, reject command */
        }
    else
        {                               /* Verify "address", byte-swapped */
        if ((psw[2] != clock_SN[4])     /* Serial Number match? */
            || (psw[1] != clock_SN[5])) /* ... */
            return (MB_EXC_ILL_DATA);   /* No, reject command */
        }
    newena = psw[3];                    /* New Enabled-Features mask */
    newsftena = psw[4];
    if (xx & 1)
        newena = (char)(~newena);
    newena |= (char)ENA_BYDEFAULT;      /* Always-On features */
    /* By default, automatically enable all enable'able features. The user
       will have to re-disable anything previously disabled... We call the
       mbfSetEnaFeatures() routine to try to keep the VIP (etc.) LEDs up to
       date.*/
    if (!(newena & ENA_VIP))            /* Is VIP to be disallowed? */
    {
      mbfClrEnaFeatures (ENA_VIP);     /* Yes -- Zap LEDs as well */
    }
    SysParm.EnaPassword = newena;       /* Remember new "master" mask */
    SysParm.EnaFeatures = 0;            /* Clear out everything */
    status = mbfSetEnaFeatures (enable_jumpers & newena); /* Set allowed */
    if ( status != MB_OK)
    {
      return status;
    }
    /* Force the new "enabled features" to be everything allowed under the
       features password, regardless of current hardware jumpers, so all
       the customer has to do is insert the jumper to "enable" the feature.
       This will "re-enable" anything previously disabled via VIPER/TAS
       software control! */
    SysParm.EnaFeatures    |= newena;      /* New "remembered active" */
    /* Enable 2 and 5 Wire by default,  Disable Ground Test Delay */
    SysParm.EnaSftFeatures = (ENA_INTELLITROL2 | ENA_2_WIRE | ENA_5_WIRE);
    SysParm.EnaSftFeatures &= newsftena;   /* Disabled Requested Feature */
    if(SysParm.EnaSftFeatures & ENA_INTELLITROL2)
    {
      unit_type = INTELLITROL2;
    }else       
    {
      unit_type = INTELLITROL_PIC;
    } 
     /* Write the new Enable-Features mask into EEPROM */
    return MB_OK;
}

void dummy_func(unsigned char *dummy)
{
  *dummy += 1;
  *dummy -= 1;
}

/******************************* 2/16/2010 6:07AM ****************************
 * This routine will turn on the leds that did not exist in the old front panel
 *****************************************************************************/
void set_new_led(LED_NAME led, char state)
{
  if (led < FREELED29)
  {
    if ( new_front_panel == TRUE)
    {
      ledstate[led] = state;
    }
  }
}

void set_compartment_led(int led, char state)
{
int led_tmp;

  if ((new_front_panel == TRUE) && (led > (int)COMPARTMENT_8))
  {
    if ( led <= 12)
    {
      led_tmp = led - ((int)COMPARTMENT_8 + 1);  /* Make sure Compartment 9 starts at 0 */
      led = led_tmp + (int)COMPARTMENT_9;
    }
  }
  ledstate[led] = state;
}
/**************************** end of sim.c **********************************/
