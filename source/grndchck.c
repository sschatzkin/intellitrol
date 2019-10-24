/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         grndchck.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *  Description:  GroundDiodeCheck function.  Used to detect the presense of
 *                a truck ground diode.
 *
 * Revision History:
 *   Rev     Date    Who   Description of Change Made
 * ------  --------  ---  --------------------------------------------
 * 1.5.23  10/19/11  KLL  Removed test_gnd_idle() which was already commented
 *                         out.
 *         04/19/12  KLL  Fixed some coding issues.
 *                        Added test_gnd_idle() routine. This is called during the
 *                         idle state and allows 5 good tests of ground before it
 *                         indicates there is a good ground and probably a truck
 *                         is attached.
 * 1.5.30  07/10/14  DHP  Autodetect ground as resisitive or diode if "gnd bolt"
 *                         jumper is open.  Update new Status A bits to show
 *                         which, if any, was found
 *                        Created Update_TB3p5p6() from code which was part of
 *                         diode check and called from there and resistive check.
 *                        Decreased GNDTIME value to avoid falsely detecting field 
 *                         analyzer unit as good Ground Bolt
 *                        Renamed GroundDiodePresent() to CheckGroundPreset()
 *                        Created set_ground_reference() from code in modreg.c
 * 1.5.31  01/14/15  DHP  Removed setting of READ_COMM_ID_BIT
 * 1.5.34  08/05/16  DHP  FogBugz 129: Added clear of GND_SHORTED in diode check
 *                         of CheckGroundPresent() when short removed.
 *                        Deleted unneeded debug printf statements.
 ********************************************************************************************/

#include "common.h"

#define GNDTIME     125              /* Microseconds to discharge cap */

static void Update_TB3p5p6(unsigned char status);

/********************************************************************************************
 *
 *  Routine:      CheckGroundPresent
 *
 *  Description:  Handle ground checking (Determine if a ground "bolt" diode
 *                is present (USA configuration), or if ground path is less than
 *                 selected ground resistive level (European configuration).
 *
 *                NOTE: the delay between tests is handled INTERNALLY
 *                      due to charge requirement flow-thru
 *
 *
 * No TIM          |----- 5ms -------|--10/50us--|
 *             ____                   ________       _____________
 *     COMM_ID     |_________________|    |||||_____|
 *
 *                                    __________________
 *     GCHECK  ______________________|                  |_________
 *
 *                                   |----100us max-----|
 *
 *
 * TIM present     |----- 5ms -------|------300us---|--10/50us--|
 *             ____                   ___         ___________      ________
 *     COMM_ID     |_________________|   |_______||||     ||||____|
 *
 *                                                   __________________
 *     GCHECK ______________________________________|                  |___
 *
 *                                                  |----100us max-----|
 *
 * European resistive ground is much simpler...just read voltage at GND_SENSE
 * and verify less than selected level (~10, ~500, ~2.5k or ~10k Ohms).
 *
 *  Return:
 *     GND_OK          0x00   Ground diode present (or < selected resistive value)
 *     GND_FAIL        0x01   Ground diode not present (or bad ground)
 *     GND_INIT_TRIAL  0x08   Initial Condition, first time here
 *                             set in truck_gone() and sensor setup routines )
 *     GND_NO_TEST     0x10   Test not enabled
 *     GND_SHORTED     0x20   Short on line detected (G-Bolt only)
 *     GND_HARD_FAULT  0x40   Ground-bolt-detect circuit dead
 *     GND_NO_TRIAL    0x80   Test not performed, i.e. try again later
 *
 *  Notes:     COMM_ID and RX_COMM_ID are connected together on the CPU board.
 *             Be sure not to drive RX_COMM_ID when attempting to read COMM_ID.
 *
 ********************************************************************************************/
static unsigned char  diodedetect = FALSE;

unsigned char CheckGroundPresent(char truckhere)
{
unsigned int startime, stoptime,breakoutime, timeout;
unsigned int dischargeticks;  /* Measure cap dischrge time */
unsigned int dischargetime;   /* Measure cap dischrge time */
unsigned int  Ena_GRND_fault_delay = 0;
unsigned char  status;
unsigned long fetch_time;
unsigned int save_iec0;

  if (!(SysParm.EnaFeatures & ENA_GROUND)) /* Is Ground Check enabled ? */
  {                                                                         /* No, return with proper result */
    return(GND_NO_TEST);
  }
  if (SysParm.EnaSftFeatures & ENA_GRND_DELAY)
  {  
    Ena_GRND_fault_delay = 6;
  }
  fetch_time = read_time();
  status = badgndflag;      /* use last value of bad ground flag */
  /* Execute first time through and every 1/3 sec; reset timeout if > 5 seconds */
  if ((ground_time < fetch_time) || (ground_time > (fetch_time+SEC5)) )
  {
     /* NOTE: testing for active serial communications */
    if (active_comm & (TIM | INTELLI)) /* Is xmitter/receiver active? ( 2 | 4) */
    {
      if (truckhere)               /* When called from Idle truckhere = 0 (FALSE) */
        return(status);           /* If so, don't start charging */
      else
        return(GND_NO_TEST);      /* When in Idle (No truck) and VIP present */
    }
    /*For resistive ground testing, versus diode testing, just need to measure actual
        voltage (or lack thereof) on Pin 9 which we are current-limited-driving to "+5".
        If Pin 9 "grounded" properly, then we should see 0 volts at GND_SENSE; allow
        "grounding" to be up to 10ohms, 500 ohms, 1.3k ohms or 10k ohms based on 
        Modbus register setting of EU_GND_REF (register 0x7C).
        We will not look for resistive ground if a TIM is detected, VIP is enabled, diode ground
        checking is in progress or it has already determined this vehicle has a diode or the 
        the Intellitrol is jumpered for ground bolt/ball checking only.
        We set g_check to turn on current sources.  Any call to read the TIM needs to disable
        the current sources - clear g_check (see trukstat.c) */ 
    if (!diodedetect)
    {
      if (((active_comm & GROUNDIODE) == 0)&&(ConfigA & CFGA_100_OHM_GND))
      {
          ground_time = read_time() + 330;
          ops_ADC(OFF);
          set_gcheck ();                  /* set g_check high for resistive check */
          COMM_ID_BIT = 1;     /* with COMM_ID & GCHECK no path to ground except pin 9 */
          DelayMS (5);                   /* delay 5 Millisec's to allow settling time */
          if((unsigned char)read_muxADC (0, M_GND_7_8, &stoptime)) /* Read GND_SENSE */
          {
            if (truckhere)
            {
              ops_ADC(ON);
            }  

            if (stoptime <= SysParm.Ground_Reference)   /* Good ground? */
            {
              BadGndCntr = 0;
              gnd_retry++;
              if(gnd_retry > 3)
              {
                 gnd_retry = 3;
                 badgndflag &= ~(GND_FAIL_D | GND_FAIL_R | GND_INIT_TRIAL);
                 StatusA &= ~STSA_DIODE_GND;
                 StatusA |= STSA_RESISTIVE_GND;
                 status = GND_OK;
              }
              Update_TB3p5p6(status);
              return (status);
            }
            else                                  /* No, excessive resistance; bad ground or diode */
            {
               gnd_retry = 0;
               status |= (GND_FAIL_R | GND_FAIL);        /* Yes */
               // with truckhere allow a bad ground for a short time
               if (truckhere && !(badgndflag & GND_INIT_TRIAL))
               {
                  BadGndCntr++;
                  if (BadGndCntr < Ena_GRND_fault_delay)
                  {
                     return (GND_OK);
                  }
               }
            }
          }else
          {
             return (status);
          }
       // If we didn't return then no resistive ground was found,
       // look for diode ground.
       // let diode check increment BadGndCntr
       } /* End "resistive" ground check */
     } 
    /***************************** 10/24/2008 5:19AM *******************
     * Start of diode ground check
     *****************************************************************/
    if((unit_type == INTELLITROL2) || ((ConfigA & CFGA_100_OHM_GND) == 0))
    {
      if ((active_comm & GROUNDIODE) == 0)
      {
        active_comm |= GROUNDIODE;
        clear_gcheck();
        COMM_ID_BIT = 1;
        // wait for COMM_ID to show being set
        timeout = 1000;
        while ((( READ_COMM_ID_BIT) == 0) && (timeout > 0))
        {
          DelayUS(1);
          timeout--;
        }
        if (timeout == 0)
        {
           /* The COMM_ID should be high */
           active_comm &= ~GROUNDIODE;
           diodedetect = FALSE;
           status = GND_SHORTED;
           Update_TB3p5p6(status);
        }else
        {
          status &= ~GND_SHORTED;    /* FogBugz 129 */
          ODCDbits.ODCD0 = 0;             /* Remove the weak pullup */
          COMM_ID_BIT = 0;                 /* Set Low to charge the cap */
          TRISD |= RX_COMM_ID;        /* Make COMM_ID input */
          TRISD &= ~COMM_ID;           /* Make COMM_ID output */
          ground_time = read_time() + 6;  /* Takes 6ms to charge the cap */
        }
        return status;
      }

      /* test to see if 6ms has past*/
      fetch_time = read_time();
      if ( fetch_time < ground_time)
      {
          return status;      /* Cap has not finishing charging */
      }
      ground_time = read_time() + 330;  /* reset timer */

      /***************************** 10/24/2008 5:26AM ************************
       * Now it is time to test for the ground bolt
       * Disable interrupts
       ************************************************************************/
      save_iec0 = IEC0;
      IEC0 = 0;                   /* Disable heart beat and DMA interrupt */
      TRISD |= COMM_ID | RX_COMM_ID;      /* Set to input */

      /* NOTE: testing for cap charge time; do not change order of below instructions */
      /* ========================================================== */
      TMR1 = 0;
      set_gcheck();                                             /* Set gcheck high */
      startime = TMR1;                                      /* Start your stopwatch */
      breakoutime = TMR1 + (200 * USEC);    /* Setup ground check timeout for 200us */
      while (READ_COMM_ID_BIT == 0)
      {
      /* Wait for COMM_ID to pull up; but don't get stuck */
        if ( TMR1>breakoutime )
          break;
      }
      stoptime = TMR1;                                     /* Stop your stop watch */
      /* ========================================================== */
      clear_gcheck();                                         /* Drive GCHECK low */
      ODCDbits.ODCD0 = 1;
      COMM_ID_BIT = 1;
      TRISD &= ~COMM_ID;
      active_comm &= ~GROUNDIODE;       /* clear Ground Test active */

      /****************************** 9/11/2008 10:35AM **************************
       * restore interrupts
       ***************************************************************************/
      IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */

      dischargeticks = stoptime - startime;  /* Compute zapcap discharge time */
      dischargetime = dischargeticks / USEC;   /* and convert from tick to usec */

      if (dischargetime > GNDTIME)           /* 125 us */
      {
        if (truckhere && !(badgndflag & GND_INIT_TRIAL))
        {
          BadGndCntr++;
          if (BadGndCntr >= Ena_GRND_fault_delay)
          {
             gnd_retry = 0;
             status = (GND_FAIL_D | GND_FAIL);             /* Diode didn't discharge zapcap */
          }
          else
          {
            gnd_retry++;
            diodedetect = TRUE;
            if(gnd_retry > 3)
            {
               gnd_retry = 3;
               badgndflag &= ~(GND_FAIL_D | GND_FAIL_R | GND_INIT_TRIAL);
               StatusA &= ~STSA_RESISTIVE_GND;
               StatusA |= STSA_DIODE_GND;
               status = GND_OK;
            }
          }
        }
        else
        {
           diodedetect = FALSE;
           status |= (GND_FAIL_D | GND_FAIL);
        }
      }
      else
      {
        BadGndCntr = 0;
        if(badgndflag & GND_INIT_TRIAL)
        {
          badgndflag &= ~GND_INIT_TRIAL;
        }
        diodedetect = TRUE;
        gnd_retry++;
        if(gnd_retry > 3)
        {
          gnd_retry = 3;
          badgndflag &= ~(GND_FAIL_D | GND_FAIL_R | GND_INIT_TRIAL | GND_FAIL);
          status = GND_OK;
//          if (ConfigA & CFGA_100_OHM_GND)      /* Re-defined as auto type detect */
          {                                  /* Yes */
            StatusA &= ~STSA_RESISTIVE_GND;
            StatusA |= STSA_DIODE_GND;
          }
        }
      } 
    }    /* End of do this every 1/3 second */
  }
  Update_TB3p5p6(status);
  return(status);
} /* End of CheckGroundPresent() */

/****************************** 8/15/2011 11:07AM ********************
 * Update_TB3p5p6
 * TB3p5p6 signal may is defined to indicate either good ground or truck present
 * based on J5:ENA_TRUCK_HERE jumper.  The jumper in sets the default which is:
 * Intellitrol2 default is good ground
 * Intellitrol_PIC is Truck Present
 * 
 *******************************************************************/
static void Update_TB3p5p6(unsigned char status)
{
  if ((unit_type == INTELLITROL2) && (enable_jumpers & ENA_TRUCK_HERE))
  {
    if (status == GND_OK)
    {
      TB3P5P6 = 0;     /* ACTIVE Low logic */
    } else
    {
      TB3P5P6 = 1;
    }
  }else // jumper indicates TB3 is Truck Present - handle that elsewhere
  {
    if ((unit_type == INTELLITROL_PIC) && !(enable_jumpers & ENA_TRUCK_HERE))
    {
      if (status == GND_OK)
      {
        TB3P5P6 = 0;     /* ACTIVE Low logic */
      } else
      {
        TB3P5P6 = 1;
      }
    }
  }
}

/****************************************************************************
 *
 ****************************************************************************/
void set_gcheck(void)
{
  TRISD &= ~GCHECK_BIT;  /* Set the Ground Check bit to output */
  GCHECK = SET;
}

/****************************************************************************
 *
 ****************************************************************************/
void clear_gcheck(void)
{
  TRISD &= ~GCHECK_BIT;  /* Set the Ground Check bit to output */
  GCHECK = CLR;
}

/******************************* 4/17/2012 9:27AM ***************************************
 * test_gnd_idle
 * Called from truck_idle(), this waits for 5 good ground checks before updating
 * badgndflag status to GND_OK.  This is because sometimes the ground check routine, 
 * CheckGroundPresent(), will give a false positive when the truck state is idle.
 ********************************************************************************************/

unsigned char test_gnd_idle()
{
unsigned char badgndflag_save;

  badgndflag_save = badgndflag;

  badgndflag = CheckGroundPresent(FALSE);   /* Test if we can see ground */

  if (badgndflag == GND_OK)
  {
    if (gnd_retry < 3)
    {
      badgndflag = badgndflag_save;
    }
  }
  return badgndflag;
}
/****************************************************************************
 * set_ground_reference()
 *   Set Ground_Reference to correct value based on SysParm.EU_GND_REF.
 *   EU_GND_REF must have already been set up and SysParm initialized from NV memory.
 ****************************************************************************/
void set_ground_reference(void)
{
  switch(SysParm.EU_GND_REF)
  {
    case (0):
      SysParm.Ground_Reference = GND_REF_DEFAULT;  /* Near 2k ohms */
      break;
    case (1):
      SysParm.Ground_Reference = GND_REF_LOW;          /* Near 10 ohms */
      break;
    case (2):
      SysParm.Ground_Reference = GND_REF_EU;              /* Near 500 ohms */  
      break;
    case (3):
      SysParm.Ground_Reference = GND_REF_HIGH;          /* Near 10k ohms */
      break;
    default:
      SysParm.EU_GND_REF = 0;
      SysParm.Ground_Reference = GND_REF_DEFAULT;  /* Near 2k ohms */
      break;
  }
}
/****************** end of grndchck ****************************************/

