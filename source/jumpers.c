/*********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         jumpers.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014 Scully Signal Company
 *
 *   Description:    Main program routines for Jumpered operations of the
 *                   Rack controller main microprocessor PIC24HJ256GP210
 *
 * Revision History:
 *   Rev      Date        Who   Description of Change Made
 *  --------  --------   ----    --------------------------------------------
 *  1.5.23  12/09/11  KLL  In read_jumpers() added break between case 4 and 5
 *                                      because with out it it will cause the VIP to be
 *                                      enabled even if the factory did not enable this feature.
 *                                     Added some {} for readability.
 *                                     Added the printing of the jumpers vs. software enable to
 *                                      the debug terminal
 *  1.5.28  05/13/14  DHP  Changed fetch_jumpers() to return value indicating all 
 *                                       jumpers out if error on reading and set jumper changed
 *                                       error flag in iambroke.
 *                                      Shortened IO Expander reset time and wait after reset.
 *                                      Changed read_jumper() to skip read if main not idle.
 *  1.5.30  08/10/14  DHP  Restructured the print out of jumpers and enables and
 *                                       changed text as appropriate to reflect the Intellitrol2 usage.
 *  1.5.31  01/14/15  DHP  Added else statement in read_jumpers() to disable deadman 
 *                                        when jumper is out
 *                                      Removed k_poke_dog(), ClrWdt() is used in its place
 *  1.6.35  02/01/17  DHP  Moved deadman_ops() function to deadman.c
*********************************************************************************************/

#include "common.h"
/****************************************************************************/
static const EXP_DEF Expander_init_buf[] =
{
  {0x05, 0xB4},
  {0x00, 0xFF},                 /* Configure Port A as input to read enable jumpers */
  {0x01, 0xFF},                 /* GPIO register bit will reflect the same logic state of the input pin */
  {0x02, 0},                    /* Disable GPIO input pin for interrupt-on-change event */
  {0x03, 0},
  {0x04, 0},
  {0x06, 0},                    /* Pull-up disabled */
  {0x15, 0xB4},
  {0x10, 0xFF},                 /* WRPROT input */
  {0x11, 0xFF},                 /* GPIO register bit will reflect the same logic state of the input pin */
  {0x12, 0},                    /* Disable GPIO input pin for interrupt-on-change event */
  {0x13, 0},
  {0x14, 0},
  {0x16, 0},                    /* Pull-up disabled */
};

/******************************* 7/30/2008 11:18AM ***************************
 * This procedure initializes the MCP23017 which allows the software to read the
 * J5 jumper block
 *****************************************************************************/
int init_I_O_Expander()
{
unsigned int i;
UINT8 data, address;

  PEXT_RST = 0;                             /* Reset the chip */
  DelayUS(4);
  PEXT_RST = 1;                             /* Remove reset from the chip */
  DelayUS(2);

  /****************************** 7/28/2008 4:13PM ***************************
   * Make sure BANK bit is set to 1: 8 bit mode
   ***************************************************************************/
  data=0x80;
  if (I2C2_write(MCP23017_DEVICE, 0x0A , &data,1))
  {
    xprintf( 135, MCP23017_DEVICE);
    return FAILED;
  }

  /****************************** 9/10/2008 8:43AM ***************************
   * Complement the jumper read so active low will read a high(one).
   ***************************************************************************/
  data = 0xFF;            /*  */
  if (I2C2_write(MCP23017_DEVICE, 0x01 , &data,1))
  {
    xprintf( 135, MCP23017_DEVICE);
    return FAILED;
  }
  for ( i=0; i< sizeof(Expander_init_buf)/sizeof(Expander_init_buf[0]); i++)
  {
    data = Expander_init_buf[i].data;
    address = Expander_init_buf[i].reg ;
    if (I2C2_write(MCP23017_DEVICE, address, &data, 1))
    {
      xprintf( 135, MCP23017_DEVICE);
      return FAILED;
    }
    address = 0x15;
    if (I2C2_read(MCP23017_DEVICE, address, &data, 1))
    {
      xprintf( 133, MCP23017_DEVICE); // "Device 0x4A on I2C bus TWO read error"

      return FAILED;
    }
  }
  for ( i=0; i< sizeof(Expander_init_buf)/sizeof(Expander_init_buf[0]); i++)
  {
    address = Expander_init_buf[i].reg ;
    if (I2C2_read(MCP23017_DEVICE, address, &data, 1))
    {
      xprintf( 133, MCP23017_DEVICE); // "Device 0x4A on I2C bus TWO read error"
      return FAILED;
    }
    if (data != Expander_init_buf[i].data)
    {
      return FAILED;
    }
  }
  return PASSED;
}

/*************************************************************************
 *  subroutine:      test_jumpers()
 *
 *  function:
 *
 *         1.  Entry is from either POD or main loop
 *         2.  This is used to display the status of the enable/disable
 *             jumpers
 *
 *  input:  none
 *  output: none
 *
 *************************************************************************/

char test_jumpers(void)
{
unsigned int  i, status;

  if (DEBUG_IN)
    printf("\n\r    DEBUG Disabled");
  else
    printf("\n\r    *** WARNING DEBUG ENABLED ***");

  printf("\n\n\r  Features    Jumpers    Factory  Software");
  printf("\n\r  --------    -------    -------  --------");
  read_jumpers(0);                     /* Initialize/set up enable jumpers */
  for (i=1; i<7; i++)                  /* Show what's active */
  {
    printf("\n\r");
    status = ((unsigned int)enable_jumpers & ((unsigned int)1<<i));
    xprintf( ( 101+i), DUMMY );
    if (status)
    {
      xprintf( 100, DUMMY );
    }
    else
    {
      xprintf( 101, DUMMY );
    }
    status = ((unsigned int)SysParm.EnaPassword & ((unsigned int)1<<i));
    if (status)
    {
      xprintf( 108, DUMMY );
    }
    else
    {
      xprintf(  109 , DUMMY);
    }
    status = ((unsigned int)SysParm.EnaFeatures & ((unsigned int)1<<i));
    if (status)
    {/* use printout file structure */
      xprintf(  108, DUMMY );
    }
    else
    {
      xprintf( 109, DUMMY );
    }
  }
  for (i=0; i<3; i++)
  {
    status = ((unsigned int)ConfigA & ((unsigned int)1<<i));
    xprintf( ( 200+i), DUMMY );
    if (status)
    {/* use printout file structure */
      xprintf( 100, DUMMY );
    }
    else
    {
      xprintf( 101, DUMMY );
    }
  }
  if (unit_type == INTELLITROL2)
  {
    if ((enable_jumpers & ENA_TRUCK_HERE))
    {
      printf("\n\n\r    TB3 5/6 is GROUND Proving");
    }else
    {
      printf("\n\n\r    TB3 5/6 is TRUCK HERE");
    }  
  }else
  {
    if ((enable_jumpers & ENA_TRUCK_HERE))
    {
      printf("\n\n\r    TB3 5/6 is TRUCK HERE");
    }else
    {
      if (SysParm.EnaFeatures & ENA_TRUCK_HERE)
      {
         printf("\n\n\r    TB3 5/6 is GROUND Proving");
      }
    }
  }
  printf("\n\r");
  return (0);
}

/*************************************************************************
 *  subroutine:  fetch_jumpers()
 *
 *  function:      read the jumpers and return the result.
 *
 *  input:          none
 *  output:        returns jumper setting or 0 if error on read
 *
 *************************************************************************/
unsigned char fetch_jumpers()
{
unsigned char byte_data;
unsigned char new_jumpers = 0;

  if (I2C2_read(JUMPER_MUX, 9 ,&byte_data,1))
  {
    xprintf( 133, JUMPER_MUX);
    iambroke |= JUMPER_CHANGE;  /* force a reset on next cycle, return all jumpers off for now */
  }
  else
  {
  /***************************** 9/9/2008 3:18PM ****************************
   * Translate new jumper settings to old jumper settings
   **************************************************************************/
  if(byte_data & ENA_TRUCK_HERE_NEW) new_jumpers |= ENA_TRUCK_HERE;
  if(byte_data & ENA_DEADMAN_NEW) new_jumpers |= ENA_DEADMAN;
  if(byte_data & ENA_VAPOR_FLOW_NEW) new_jumpers |= ENA_VAPOR_FLOW;
  if(byte_data & ENA_VIP_NEW) new_jumpers |= ENA_VIP;
  if(byte_data & ENA_GROUND_NEW) new_jumpers |= ENA_GROUND;
  if(byte_data & ENA_ADD_1_KEY_NEW) new_jumpers |= ENA_ADD_1_KEY;
  if(byte_data & ENA_ERASE_KEYS_NEW) new_jumpers |= ENA_ERASE_KEYS;
  }
  return new_jumpers;
}

/*************************************************************************
 *  subroutine:      read_jumpers()
 *
 *  function:   read the jumpers and place the result in the
 *              global variable enable_jumpers
 *
 *  input:  flag    if 0, initial/reset, just read and set
 *                  if 1, verify against previous
 *  output: none     sets the global byte enable_jumpers
 *
 *************************************************************************/
void read_jumpers(char flag)
{
unsigned char  new_jumpers;

  /* Read the DEBUG jumper. This jumper is read dynamically and allowed to
     change on the fly; in theory any other jumper that changes value while
     Intellitrol unit is running is cause to "FAULT" and go non-permissive. */

  // last_routine = 0x5E;
  /* Get the jumpers only if idle or first time read */
  if (main_state != IDLE)
  {
    return;
  }
  new_jumpers = fetch_jumpers(); 
 // last_routine = 0x5E;
  if (DEBUG_IN == 1) /* DEBUG jumper in place (0 = jumper)? */
  {                                 /* No */
    StatusA &= ~STSA_DEBUG;         /* Note DEBUG jumper "OFF" */
    ConfigA &= ~CFGA_DEBUG;         /* Note in Config-A also */
  }
  else                              /* DEBUG jumper in place (0 = jumper)? */
  {                                 /* Yes */
    StatusA |= STSA_DEBUG;          /* Note DEBUG jumper "ON" */
    if (!(ConfigA & CFGA_DEBUG))    /* If wasn't already set */
    {                               /* It wasn't! */
      if (flag)                     /* First time, or changing? */
      {
        printf("\n\rDebug Jumper Change Error\n\r");
        printf("DEBUG_IN: 0x%X   ConfigA: 0x%X\n\r", DEBUG_IN, ConfigA);
        secReset = 1;               /* Change, force reset to "log" it */
      }
    }
    ConfigA |= CFGA_DEBUG;          /* Note in Config-A also */
  }
  /* Read the primary feature "enable" jumpers (block J5) */

  if (flag == 0)                    /* "First" (== "Reset") time? */
  {                                 /* Yes, initialize enable jumpers */
    enable_jumpers = new_jumpers;
    /***************************** 7/21/2011 10:17AM *************************
     * Check if Modbus Enable Features has ever been initialize?
     *************************************************************************/
    if ((SysParm.Modbus_Ena_Features & 0x01) == 0)
    {
      SysParm.Modbus_Ena_Features = 0xFF;  /* No it has not, so initialize it */
      (void)nvSysParmUpdate();  /*  */
    }
    SysParm.EnaFeatures = new_jumpers;
    SysParm.EnaFeatures &= SysParm.EnaPassword;
    SysParm.EnaFeatures &= SysParm.Modbus_Ena_Features;

    if (enable_jumpers & ENA_VIP)   /* If the VIP jumper is set, Disable the Exxon Reg 7B setting*/
    {
      switch(SysParm.VIPMode)
      {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
          SysParm.VIPMode = 0;
          break;
        case 5:
          SysParm.EnaFeatures |= ENA_VIP; /* Turn on normal VIP ops       */
          SysParm.TASWait = 0;
          break;
        default:
          SysParm.VIPMode = 0;
      }
    }
    else
    {
      SysParm.VIPMode = 0;
    }
    modbusVIPmode = SysParm.VIPMode;
    if (new_jumpers & (ENA_ADD_1_KEY | ENA_ERASE_KEYS))
    {
      StatusB |= STSB_SPECIAL;    /* Special Op mode */
    }
  }
  else                                /* "First" (== "Reset") time? */
  {                               /* No, verify jumpers haven't changed */
    if (enable_jumpers != new_jumpers)
    {
      iambroke |= JUMPER_CHANGE;   /* Jumpers changed! Must reset system! */
    }
  }
  if (enable_jumpers & ENA_DEADMAN)  /* If the Deadman jumper is set, make sure the deadman feature is enabled */
  {
    SysParm.EnaFeatures |= ENA_DEADMAN;  /* Make sure deadman feature is enabled */
  }else
  {
    SysParm.EnaFeatures &= ~ENA_DEADMAN; /* Make sure deadman feature is disabled */
  }
    /* 8-Compartment, 8.2-Volt, and 100-Ohm Ground are intuited elsewhere... */
} /* End of read_jumpers() */

/*************************************************************************
 *  subroutine: volts_jumper
 *
 *  function:   Convert voltage from resistor network/jumper block into
 *              "jumper" (index)
 *
 *  input:  Voltage reading from Jumper block
 *  output: Jumper index number (zero-based 0 - 9)
 *
 *************************************************************************/

#define  JTOLERANCE  50

static const unsigned int JumperReading[10] =
{ 2900, 2600, 2300, 2000, 1700, 1400, 1100, 800,  500,  200 };

unsigned char volts_jumper
    (
    unsigned int volts                  /* Voltage reading to be converted */
    )
{
int i;
  // last_routine = 0x5F;

    volts += JTOLERANCE;                /* Why not just offset the table??? */
    /* Search the table for voltage range */
    for (i=0; i<10; i++)
    {
      if (volts > JumperReading[i])
        break;
    }
    return (unsigned char)i;
} /* End volts_jumper() */
