/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         delay.c
 *
 *   Revision:       REV 1.5.27
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *   Description:    Main program enumerated list for Rack controller main
 *                   microprocessor PIC24HJ256GP210
 *
 * Revision History:
 *   Rev      Date       Who   Description of Change Made
 * --------  -------------  -----   --------------------------------------------
 * 1.5.31  01/14/15  DHP  Changed DElayUS() to not clear TMR0
 *****************************************************************************/

#include "common.h"

/*
DelayUS() was coded to use TMR1, setting it to 0 on each entry and waiting for it to exceed
an expected count while counting down a timeout value (0x1000 or about 10MS).  If the 
timeout reached zero a message was printed.  The Timer3 interrupt calls read_probes() which
in processing also uses DelayUS() causing a timeout which causes a printf() which also uses
DelayUS(). So everything got confused and eventually the stack overflowed and the PIC got
reset.  The current implementation is not great but works better.
*/
void DelayUS (unsigned int usperiod)
{
unsigned int delay_cnt, start, end;

  if (usperiod !=0)
  {
    delay_cnt = usperiod * 20;  /* 20 is the number of counts per microsecond */
    start = (TMR1);
    end = (start + delay_cnt);
    if (end > start)
    {
      while ( TMR1 < end)
      {
      }
    }else
    {
      while (TMR1 > start)
      { 
      }
      while (TMR1 <= end)
      {
      }
    }
  }
}
void DelayMS (unsigned int msperiod)
{
unsigned int i;

//  PORTAbits.RA4 ^= 1;
  for ( i=0; i<msperiod; i++)
  {
    DelayUS(1000);
  }
//  PORTAbits.RA4 ^= 1;
}


