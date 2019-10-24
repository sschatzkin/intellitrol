/*******************************************************************************
 *
 *  Project:      Rack Controller
 *  Module:       deadman.c
 *  Revision:     REV 1.6
 *  Author:       Dave Paquette
 *                   @Copyright 2017  Scully Signal Company
 *  Description:  Routines for handling the deadman switch.
 *
 * Revision History:
 *   Rev      Date    Who  Description of Change Made
 *  ------- --------  ---  --------------------------------------------
 *  1.6.35  02/01/17  DHP  Moved deadman_ops() function here from jumpers.c.
 ******************************************************************************/
#include "common.h"
#include "volts.h"
void deadman_init(void);
/*************************** local variables **********************************/
  static  char  old_deadman;      /* connect / previous deadman open status */
  static  unsigned int  DM_open_time  = 0; 
  static  unsigned int  DM_close_time = 0;
  static  unsigned int  DM_warn_close  = 0;
 
/*******************************************************************************
 *  name:       deadman_init()
 *  function:   Read the AN1 value of MUX 04 port (deadman switch) as a 0 or 1.
 *    Set the old_deadman variable based the open state. TRUE if switch is open
 *    or FALSE if closed.  A voltage > than 2 volts is seen as an open.
 *    Set the open, close and warn time variables to appropriate values based on
 *    the active or standard setting and the values of the parameter registers.
 *         
 *  input:  none
 *  output: none
 ******************************************************************************/
void deadman_init(void)
{
unsigned int save_timer_status;

  baddeadman = TRUE;
  save_timer_status = T3CON;
  /* turn off timer, ADC and DMA */
  ops_ADC(OFF);
  /* Read Deadman open/closed */
  (void) read_muxADC (1, M_DEADMAN, &deadman_voltage);
  old_deadman = (deadman_voltage > ADC2V); /* TRUE if deadman open */
  /* If timer was on at routine start turn it back on ***********************/
  if (save_timer_status & 0x8000)
  {
    ops_ADC(ON);
  }
  if(SysParm.DM_Active)
  {  //Close > (4*19), Warn < ((close + (4*15))
    if((SysParm.DM_Max_Close > 76)
            &&(SysParm.DM_Warn_Start < (SysParm.DM_Max_Close+60)))
    { 
      DM_warn_close = (SysParm.DM_Max_Close - SysParm.DM_Warn_Start);
    }else
    {
      DM_warn_close = (SysParm.DM_Max_Close - 20);
    }
    if(old_deadman)  // deadman currently open
    {
      DM_open_time  = SysParm.DM_Max_Open;
      DM_close_time = 0;
    }else                  // deadman currently closed
    {
      DM_open_time  = 0;
      DM_close_time  = SysParm.DM_Max_Close;
    }
  }
  else
  { //Standard deadman
    DM_open_time  = SysParm.DM_Max_Open;
    DM_close_time  = 0;
  }
}

/*******************************************************************************
 *  name:       deadman_ops()
 *  function:   Read the AN1 value of MUX 04 port (deadman switch) as a 0 or 1.
 *    If DM_Active is non-zero then the active deadman function is active.  If
 *    active, determine if the switch state is OK based on the max time values
 *    for the state. Return TRUE if time has expired or FALSE if OK.
 *    If DM_Active is zero return TRUE if switch is open beyond max allowed open         
 *    time or FALSE if closed. A voltage > than 2 volts is seen as an open.          
 *    The PERMIT and NONPERMIT bar LED state is set here to return to a normal 
 *    state after a warning or set to flash at different states as a warning
 *    prior to a timeout.  
 *
 *  input:  Last Reported Switch state
 *  output: TRUE  (deadman error - open or too long without activity)
 *          FASLE (deadman OK - within time parameters for current state)
 ******************************************************************************/
char deadman_ops(char past_deadman)
{
char new_deadman;
char status;
unsigned int save_timer_status;

  save_timer_status = T3CON;
  /* turn off timer, ADC and DMA */
  ops_ADC(OFF);
  /* Read Deadman open/closed */
  if (read_muxADC (1, M_DEADMAN, &deadman_voltage) == FALSE)
  {
    /* If timer was on at routine start turn it back on ***********************/
    if (save_timer_status & 0x8000)
    {
      ops_ADC(ON);
    }
    return past_deadman;
  }
  new_deadman = (deadman_voltage > ADC2V); /* TRUE if deadman open */
  if (new_deadman == old_deadman)         /* same state as before? */
  {
    status = new_deadman;
    /* We are now in a stable open or closed state - verify we are not here
    beyond the fault time. */
    if(status)
    {  // Deadman is now open, if longer than DM_max_open then BAD
      if(DM_open_time++ < SysParm.DM_Max_Open)
      {
        status = FALSE;           /* deadman is still OK */
        if((SysParm.DM_Max_Open - DM_open_time) < 20)
        {
          if (ledstate[PERMIT] != DARK) /* set fast flash */
          {
            ledstate[PERMIT] = FLASH4HZ;  /* set fast flash */
          }else
          /* Look for overfill bypass active */
          if (ledstate[NONPERMIT]  == FLASH1HZ75)
          {
            ledstate[NONPERMIT]  =  FLASH4HZ;
          } 
        }
      }
      else
      {
        status = TRUE;           /* deadman is NOT OK */
      }
    }
    else // Deadman is now closed, if beyond DM_warn tell them to take action
    if(SysParm.DM_Active)
    {
      {
        if(DM_close_time++ >= SysParm.DM_Max_Close)
        {
          status = TRUE;        /* deadman is NOT OK */
          DM_close_time--;      /* in case it is stuck (taped?) closed -prevent roll over */
        }else
        if ((DM_close_time > DM_warn_close) || (ledstate[NONPERMIT] ==  FLASH1HZ75))
        {
          if (ledstate[PERMIT] == LITE)
          {
            ledstate[PERMIT] = FLASH1HZ; /* set slow flash */
          }else
          if ((ledstate[PERMIT] == FLASH1HZ) || (ledstate[NONPERMIT] ==  FLASH1HZ75))
          { /* Check if within 10 seconds of fault? */
            if(((SysParm.DM_Max_Close - DM_close_time) < 40) &&
                    (ledstate[PERMIT] == FLASH1HZ))
            {
              ledstate[PERMIT] = FLASH2HZ;  /* set Medium flash */
            }
          }else
          if ((ledstate[PERMIT] == FLASH2HZ)  ||
                (ledstate[PERMIT] == FLASH1HZ75))
          {
            if(((SysParm.DM_Max_Close - DM_close_time) < 20) &&
                   (ledstate[PERMIT] != DARK))
            {
              ledstate[PERMIT] = FLASH4HZ;  /* set fast flash */
            }
          }else /* must be at fast flash or sensor bypass */
          {
          if ((ledstate[NONPERMIT] ==  FLASH1HZ75))
          {
            if((SysParm.DM_Max_Close - DM_close_time) < 20)
            {
              ledstate[NONPERMIT] = FLASH4HZ;  /* set fast flash */
            }
          }else
          if(DM_close_time >= SysParm.DM_Max_Close)
            {
              status = TRUE;           /* deadman is NOT OK */
            }
          }
        }
      }
    }
    /* standard deadman closed requires no action */
  }
  else                              /* deadman is in new state */
  {
    /* Deadman just changed from one state to another so set timers */
    status = past_deadman;
    if (new_deadman == FALSE)       /* Went from open to closed */
    {
      if (ledstate[PERMIT] != DARK)
      {
        ledstate[PERMIT] = LITE;    /* reset flash back to solid */
      }
      DM_open_time = 0;
      DM_close_time = 0;
      old_deadman = new_deadman;
    }
    else
    { /* Went from closed to open */
      if ((ledstate[PERMIT] == DARK) && (ledstate[NONPERMIT] != FLASH1HZ75))
      {
        status = TRUE;
      }
      else
      {
        old_deadman = new_deadman;
      } 
      DM_open_time = 0;
      DM_close_time = 0;
    }
  }
  /* If timer was on at routine start turn it back on *************************/
  if (save_timer_status & 0x8000)
  {
    ops_ADC(ON);
  }
  return status;
}  /* end of deadman_ops */

/******************************* end of deadman.c *****************************/

