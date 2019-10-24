/*********************************************************************************************
 *
 *  Project:      Rack Controller
 *
 *  Module:       DALLAS.C
 *
 *  Revision:     REV 1.5
 *
 *  Author:       Ken Langlais  @Copyright 2009,2014  Scully Signal Company
 *
 *  Description:  Routines for interfacing to the Dallas bypass and clock
 *                Touch Memory chips.
 *
 * Revision History:
 *   Rev      Date        Who  Description of Change Made
 * --------   --------        ---   --------------------------------------------
 * 1.5.23  05/02/12  KLL  Check_Truck_SN() always reports the truck has left.
 *                                      Changed the default from FALSE to TRUE. So if all tests
 *                                        fail the truck is reported as still connected.
 *                             KLL  Renamed Dallas_Bit() to Dallas_Bit_Read() and added:
 *                                         Dallas_Bit_Write(),reset_iButton(), Read_Intellitrol_SN,
 *                                         read_TIM_compartment_info(),test_for_new_front_panel(),
 *                                         and read_TIM_Go_NoGo_info()
 *             03/25/14 DHP   Removed references to DS28CM00 as this chip is not used.
 *                                      Added Dallas reset to Read_Dallas_SN() to help avoid intermittent failures.
 *                                      Removed lines of commented out code.
 * 1.5.30  07/29/14  DHP  Qualified call to read_TIM_compartment_info() on SuperTIM
 *            08/10/14           Removed badvaporflag check from read_bypass()
 * 1.5.31  01/14/15  DHP  Removed setting of READ_COMM_ID_BIT
 * 1.6.34  08/08/16  DHP  Cleanup: commented unneeded (duplicated) printf
 *                        FogBugz 143: Added interrupt protection in Dallas_Byte()
 *
 *********************************************************************************************/

#include "common.h"
#include "tim_utl.h"

/********************************* Defines **********************************/

#define  JAN_01_1970    0x00000000        /* UNIX time constants */
#define  JAN_01_1994    0x2D24BD00

/******************************** Constants *********************************/

static const unsigned char month_days[] = {31, 28, 31, 30, 31, 30,
                                           31, 31, 30, 31, 30, 31};

static unsigned char save_bypass_key[10];

/******************************* Subroutines ********************************/

/******************************* 12/29/2008 3:22PM ***************************
 * This routine will reset the iButton chip with retries
 *****************************************************************************/
int reset_iButton(unsigned char port)
{
int timeout = 3;

  // last_routine = 0x6A;

  while (timeout)
  {
    if (Dallas_Reset(port) != 0) break;
    timeout--;
  }
  if ( timeout <= 0)
  {
    return FAILED;
  }
  return GOOD;
}

/*******************************************************************************
 *
 *  Subroutine:   read_bypass()
 *
 *  Function:     Read and validate a bypass key.
 *
 *       1.  If in time window (1/2 second elapsed) Read_Bypass_SN()
 *           Return status if called with FALSE parameter
 *       2.  Validate key if found, then set the bypass flag and bypass level
 *
 *  Input:        Action - TRUE/FALSE (Read bypass chip)
 *  Output:       Result - TRUE/FALSE
 *
 ******************************************************************************/
char read_bypass (char action)
{
  static unsigned long bypass_delay = 0; /* Bypass timer */
  static unsigned long bypass_wait;      /* Bypass timer wait */
  static char          bypass_toggle = 0;/* Bypass True/False toggle */
  unsigned int         index;            /* Key index in NV authorization */
  char                 bypass_found;     /* TRUE if bypass key present */
  unsigned int         level;            /* Level of bypass detected */

  if (bypass_delay > (read_time() + SEC1)) /* This will prevent non-init */
  {
    bypass_delay = read_time();
    bypass_wait = read_time();
  }
  bypass_found = FALSE;
  if (bypass_delay < read_time())
  {                                  /* We have waited long enough */
    if ((bypass_found = Read_Bypass_SN()) != 0)   /* Valid Dallas key detected? */
    {                                /* Yes - CRC-validated bypass key */
      if ( action == FALSE)          /* We are idle and there is a bypass key */
      {
        return TRUE;                 /* Might be HOT wired */
      }
      bystatus |= BYS_KEYPRESENT;   /* Bypass key present (0x10) */
      if ((bypass_found = nvKeyFind(bypass_SN, &index)) == 0) /* Also auth'ed in Intellitrol list? */
      {                              /* Yes - Authorized lowest level Bypass */
        level = 0;                   /* Initialize bypass level */
        if ((bypass_wait < read_time()) && (bypass_toggle == 0))
        {                            /* Wait expired and no toggle */
          if (!(bystatus & BYS_NONBYPASS) /* If bypassing not prohibited */
                && (action == TRUE) )/* Bypass allowed and requested by caller function */
                                     /* from truck_active */
          {                          /* Try to bypass something... */
            /* Look for something to bypass. Start with the "lowest" possible
               priority or importance condition and work "upwards",
               allowing only *one* bypass action per presentation of a
               bypass key. */
            /* Truck ID/Authorization is lowest priority (never an "unsafe"
               or "dangerous" condition). */
            if ((badvipflag & ~BVF_TASDELAY) /* Bypassable VIP condition? */
                 && !(bylevel & VIP_BYPASS)) /*  and not already bypassed? */
            {                        /* Yes */
              set_bypass (VIP_BYPASS);
              level = 4;
            }
            /* Ground protection comes in next */
            else if ((badgndflag&GND_PROBLEMS) /* Bypassable Ground Fault? */
                      && !(bylevel & GROUND_BYPASS)) /* not already byp'ed? */
            {                        /* Yes */
              set_bypass (GROUND_BYPASS);
              level = 3;
            }
            /* Overfill protection has highest ("last") priority */
            else if ((badoverfillflag) /* Bypassable Overfill condition? */
                && !(bylevel & OVER_BYPASS))  /* (and not already) */
            {                        /* Yes */
              if (!(bystatus & BYS_DRY_NOOVFB)) /* ...and not prohibited */
              {                      /* Bypass overfill protection */
                set_bypass (OVER_BYPASS);
                level = 2;
              }
            }
          }
          bypass_wait = read_time() + SEC1;
        }    /* End of if ((bypass_wait < read_time()) */
      }
      else
      {                              /* Here when the key is NOT authorized */
        level = 1;                   /* Unauthorized bypass key */
      }
      if ((bypass_toggle == 0)       /* First report */
           || (bypass_toggle == 4))  /*  or still hanging out */
      {                              /* Repeats every 256 seconds . . . */
        report_bypass (level);       /* Debugging/ASCII info */
      }
      bypass_toggle++;               /* One op per key presentation */
    }  /* End Bypass Key seen in Read_Bypass_SN() */
    else
    {
      bystatus &= ~BYS_KEYPRESENT;    /* Bypass key is not present */
      bypass_toggle = 0;              /* Reset for "wait expired and no toggle, */
                                      /* i.e. next time through */
    }
    bypass_delay = (read_time() + MSec500);    /* Set to repeat in 1/2 Second */
  }    /* End of if (bypass_delay < read_time()) */
       /* At this point the bypass_found is determined by Read_Bypass_SN() */
  return (bypass_found);                 /* Return status */
} /* end of read_bypass() */

/****************************************************************************
 *
 *  Subroutine:   Read_Intellitrol_SN()
 *
 *  Function:     This routine reads the serial number from the Dallas 1-wire
 *                in the Intellitrol.
 *
 *       1.  Read the 64 bits (8 bytes).
 *       2.  Calculate the CRC to validate the data read.
 *       3.  If the CRC is 0, the data has been read properly, return TRUE.
 *       4.  If the CRC is not 0, the data is not correct, return FALSE.
 *
 *  Input:        None
 *  Output:       TRUE if the Dallas serial number read OK; otherwise, FALSE
 *                The serial number is returned in the array: touchbuf[]
 *
 ****************************************************************************/
char Read_Intellitrol_SN (void)
{
char status;
int read_retry;

   // last_routine = 0x100;

  for ( read_retry = 0; read_retry<5; read_retry++)
  {
    if ((status = (char)reset_iButton (INTELLITROL_SN)) == 0)  /* Issue reset pulse */
    {
      if ((status = Read_Dallas_SN (INTELLITROL_SN)) != 0)     /* Read serial number */
        return status;
    }
  }
  printf("\n\rError %X Reading the Intellitrol Serial Number DS2401\n\r", status);
  return status;
}  /* end of Read_Intellitrol_SN() */

/****************************************************************************
 *
 *  Subroutine:   Report_SN()
 *
 *  Function:     Prints the specified serial number.
 *
 *       1. Report serial number.
 *
 *  Input:        serial_number - Pointer to serial number to print.
 *  Output:       None
 *
 ****************************************************************************/
void Report_SN (const unsigned char *serial_number)
{
int i;

   // last_routine = 0x6D;
   for (i = 0; i < BYTESERIAL; ++i)     /* Print serial number */
   {
     xprintf( 55, (unsigned int)*serial_number++);
   }
}

/****************************************************************************
 *
 *  Subroutine:   report_bypass()
 *
 *  Function:     Prints the bypass status.
 *
 *       1. Report bypass status.
 *
 *  Input:        bypass_level - Level of bypass detected.
 *  Output:       None
 *
 ****************************************************************************/

void report_bypass (unsigned int bypass_level)
{
   // last_routine = 0x6E;

   xprintf( 56, (unsigned int)main_state );
   xprintf( 57, bypass_level );
   xprintf( 110, DUMMY );

}  /* end of report_bypass() */

/****************************************************************************
 *
 *  Subroutine:   Dallas_Reset()
 *
 *  Function:     Issues a reset pulse to the dallas port specified, returning
 *                 TRUE if valid presence pulse detected.
 *
 *       1. Issue a 500 uS reset pulse to the port specified.
 *       2. Wait for the presence pulse.
 *       3. If the presence pulse starts too soon, return FALSE.
 *       4. If the presence pulse doesn't start in time, return FALSE.
 *       5. If the presence pulse is too short, return FALSE.
 *       6. If the presence pulse is too long, return FALSE.
 *       7. Otherwise, return TRUE.
 *
 *  Input:          port - Port designation bit mask of PORTD to send bit/read
 *                bit from.
 *                    NOTE:  Only the (one) bit corresponding to the port
 *                can be set; all others must be 0.
 *  Output:       TRUE if proper presence pulse returned from Result of
 *                reading sent bit.  If the bit sent was a 0, then the return-
 *                ed bit will always be 0 (FALSE).  If a 1 was send, then a 0
 *                (FALSE) will be returned if the Dallas chip holds the line
 *                low in response; otherwise, a 1 (TRUE) will be returned.
 *
 ****************************************************************************/
unsigned char Dallas_Reset(unsigned char port)
{
char result=0;
unsigned int start, end;

  // last_routine = 0x6F;
  if(port == COMM_ID) COMM_ID_BIT = 0; // Drives DQ low
  if(port == READ_BYPASS) BYPASS_BIT = 0; // Drives DQ low
  if(port == INTELLITROL_SN)
  {
    TRISB &= ~INTELLITROL_SN_INPUT;      /* Set to output */
    SERIAL_BIT = 0; // Drives DQ low
  }
  DelayUS(480);
  if(port == COMM_ID) COMM_ID_BIT = 1; // Drives DQ
  if(port == READ_BYPASS) BYPASS_BIT = 1; // Drives DQ
  if(port == INTELLITROL_SN)
  {
    TRISB &= ~INTELLITROL_SN_INPUT;      /* Set to output */
    SERIAL_BIT = 1; // Drives DQ high
  }
//  DelayUS(70);
  if(port == COMM_ID)
  {
    DelayUS(70); 
    result = (unsigned char)(~(PORTD >> 1) & 1); // Sample for presence pulse from slave
  }
  if(port == READ_BYPASS)
  {
    start = (TMR1);
    end = (start + 1400);  /* 1400 is number of counts for 70us */
    if (end > start)
    {
      while (( TMR1 < end)  && (result == 0))
      {
          result = (unsigned char)(~(PORTD >> 3) & 1); // Sample for presence pulse from slave
      }
    }else
    {
      while ((TMR1 > start)  && (result == 0))
      { 
         result = (unsigned char)(~(PORTD >> 3) & 1); // Sample for presence pulse from slave
      }
      if (result ==0)
      {
        while ((TMR1 <= end)  && (result == 0))
        {
          result = (unsigned char)(~(PORTD >> 3) & 1); // Sample for presence pulse from slave
        }
      }
    }
//    result = (unsigned char)(~(PORTD >> 3) & 1); // Sample for presence pulse from slave
  }
  if(port == INTELLITROL_SN)
  {
    DelayUS(70); 
    TRISB |= INTELLITROL_SN_INPUT;      /* Set to input */
    result = (unsigned char)(~(PORTB >> 15) & 1); // Sample for presence pulse from slave
  }
  DelayUS(410); // Complete the reset sequence recovery
  return (unsigned char)result; // Return sample presence pulse result
}

/****************************************************************************
 *
 *  Subroutine:   Dallas_Bit_Read()
 *
 *  Function:     Sends a bit to the Dallas port specified and returns the
 *                bit read.
 *
 *       1. Wait for Tslot to send the next bit.
 *       2. Sends the bit specified to the port specified.
 *       3. Returns the value of the bit.
 *
 *  Input:        bit - TRUE if sending a 1, or FALSE if sending a 0.
 *                port - Port designation bit mask of PORTMC to send bit/read
 *                       bit from.
 *                       NOTE:  Only the (one) bit corresponding to the port
 *                              can be set; all others must be 0.
 *  Output:       Result of reading sent bit.  If the bit sent was a 0, then
 *                the returned bit will always be 0 (FALSE).  If a 1 was
 *                send, then a 0 (FALSE) will be returned if the Dallas chip
 *                holds the line low in responce; otherwise, a 1 (TRUE) will
 *                be returned.
 *
 ****************************************************************************/

char Dallas_Bit_Read (unsigned char port)
{
int result = 0;

  if(port == COMM_ID) COMM_ID_BIT = 0; // Drives DQ low
  if(port == READ_BYPASS) BYPASS_BIT = 0; // Drives DQ low
  if(port == INTELLITROL_SN)
  {
    TRISB &= ~INTELLITROL_SN_INPUT;      /* Set to output */
    SERIAL_BIT = 0; // Drives DQ low
  }
  DelayUS(6);
  if(port == COMM_ID) COMM_ID_BIT = 1; // Drives DQ low
  if(port == READ_BYPASS) BYPASS_BIT = 1; // Drives DQ low
  if(port == INTELLITROL_SN)
  {
    TRISB &= ~INTELLITROL_SN_INPUT;      /* Set to output */
    SERIAL_BIT = 1; // Drives DQ low
  }
  DelayUS(9);
  if(port == COMM_ID) result = (PORTD >> 1) & 1; // Sample for presence pulse from slave
  if(port == READ_BYPASS) result = (PORTD >> 3) & 1; // Sample for presence pulse from slave
  if(port == INTELLITROL_SN)
  {
    TRISB |= INTELLITROL_SN_INPUT;      /* Set to input */
    result = (char)((PORTB >> 15) & 1); // Sample for presence pulse from slave
  }
  DelayUS(55); // Complete the time slot and 10us recovery
  return ((char)result);
}

char Dallas_Bit_Write (char bit, unsigned char port)
{
  if (bit)
  {
    // Write '1' bit
    if(port == COMM_ID) COMM_ID_BIT = 0;          // Drives DQ low
    if(port == READ_BYPASS) BYPASS_BIT = 0;  // Drives DQ low
    if(port == INTELLITROL_SN)
    {
      TRISB &= ~INTELLITROL_SN_INPUT;      /* Set to output */
      SERIAL_BIT = 0; // Drives DQ low
    }
    DelayUS(6);
    if(port == COMM_ID) COMM_ID_BIT = 1;          // Releases the bus
    if(port == READ_BYPASS) BYPASS_BIT = 1;  // Releases the bus
    if(port == INTELLITROL_SN)
    {
      TRISB &= ~INTELLITROL_SN_INPUT;      /* Set to output */
      SERIAL_BIT = 1; // Drives DQ low
    }
    DelayUS(64);                // Complete the time slot and 10us recovery
  }
  else
  {
    // Write '0' bit
    if(port == COMM_ID) COMM_ID_BIT = 0;          // Drives DQ low
    if(port == READ_BYPASS) BYPASS_BIT = 0;  // Drives DQ low
    if(port == INTELLITROL_SN)
    {
      TRISB &= ~INTELLITROL_SN_INPUT;      /* Set to output */
      SERIAL_BIT = 0; // Drives DQ low
    }
    DelayUS(60);
    if(port == COMM_ID) COMM_ID_BIT = 1;          // Releases the bus
    if(port == READ_BYPASS) BYPASS_BIT = 1;  // Releases the bus
    if(port == INTELLITROL_SN)
    {
      TRISB &= ~INTELLITROL_SN_INPUT;      /* Set to output */
      SERIAL_BIT = 1; // Drives DQ low
    }
    DelayUS(10);
  }
  return bit;
}

/****************************************************************************
 *
 *  Subroutine:   Dallas_Byte()
 *
 *  Function:     Sends a byte to the Dallas port specified, and returns the result.
 *
 *       1. Send each byte specified to the port specified a bit at a time
 *          (LSB first) and accumulate the returned byte.
 *       2. Returns the value of the accumulated byte returned.
 *
 *  Input:     data - Byte to be sent.
 *                port - Port designation bit mask of PORT to send byte/
 *                       read bit from.
 *                       NOTE:  Only the (one) bit corresponding to the port
 *                              can be set; all others must be 0.
 *  Output:       Result of reading sent byte.
 *
 ****************************************************************************/
unsigned char Dallas_Byte(unsigned char data, unsigned char port)
{
int loop;
unsigned int result = 0;
unsigned int save_iec0;

  if (port == COMM_ID)
  {                 /* Talking on "truck" channel */
    ledstate[TRUCKCOMM] = (int)PULSE;      /* Yup... */
  }

  save_iec0 = IEC0;
  IEC0 = 0;                   /* Disable heart beat and DMA interrupt */
  for (loop = 0; loop < 8; loop++)
  {
    // shift the result to get it ready for the next bit
    result >>= 1;
    // If sending a '1' then read a bit else write a '0'
    if (data & 0x01)
    {
      if (Dallas_Bit_Read(port))
      {
        result |= 0x80;
      }
    }
    else
      (void)Dallas_Bit_Write(0, port);
    // shift the data byte for the next bit
    data >>= 1;
  }
  IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */
  return (unsigned char)result;
}

/****************************************************************************
 *
 *  Subroutine:   report_clock()
 *
 *  Function:     Report status of reading the Dallas clock.
 *
 *  Input:        clock_status - Clock read status.
 *  Output:       None
 *
 ****************************************************************************/
void report_clock (void)
{
   // last_routine = 0x73;
   if ( clock_status > 8)
   {
    clock_status = 8;
   }
   xprintf( 59, (unsigned char)clock_status );

   if (clock_status == CLOCK_OK)          /* Check clock error */
   {
      xprintf( 58, DUMMY );
      Report_SN (clock_SN);               /* Print serial number */
   }

}  /* end of report_clock() */

/****************************************************************************
 *
 *  Dallas One-Wire ("DOW") CRC table
 *
 ***************************************************************************/

const unsigned char DOW_CRC_tab [256] =
{
   0,  94, 188, 226,  97,  63, 221, 131, 194, 156, 126,  32, 163, 253,  31,  65,
 157, 195,  33, 127, 252, 162,  64,  30,  95,   1, 227, 189,  62,  96, 130, 220,
  35, 125, 159, 193,  66,  28, 254, 160, 225, 191,  93,   3, 128, 222,  60,  98,
 190, 224,   2,  92, 223, 129,  99,  61, 124,  34, 192, 158,  29,  67, 161, 255,
  70,  24, 250, 164,  39, 121, 155, 197, 132, 218,  56, 102, 229, 187,  89,   7,
 219, 133, 103,  57, 186, 228,   6,  88,  25,  71, 165, 251, 120,  38, 196, 154,
 101,  59, 217, 135,   4,  90, 184, 230, 167, 249,  27,  69, 198, 152, 122,  36,
 248, 166,  68,  26, 153, 199,  37, 123,  58, 100, 134, 216,  91,   5, 231, 185,
 140, 210,  48, 110, 237, 179,  81,  15,  78,  16, 242, 172,  47, 113, 147, 205,
  17,  79, 173, 243, 112,  46, 204, 146, 211, 141, 111,  49, 178, 236,  14,  80,
 175, 241,  19,  77, 206, 144, 114,  44, 109,  51, 209, 143,  12,  82, 176, 238,
  50, 108, 142, 208,  83,  13, 239, 177, 240, 174,  76,  18, 145, 207,  45, 115,
 202, 148, 118,  40, 171, 245,  23,  73,   8,  86, 180, 234, 105,  55, 213, 139,
  87,   9, 235, 181,  54, 104, 138, 212, 149, 203,  41, 119, 244, 170,  72,  22,
 233, 183,  85,  11, 136, 214,  52, 106,  43, 117, 151, 201,  74,  20, 246, 168,
 116,  42, 200, 150,  21,  75, 169, 247, 182, 232,  10,  84, 215, 137, 107,  53
   };

/****************************************************************************
 *
 *  Subroutine:   Dallas_CRC8()
 *
 *  Function:     Returns the 8 bit Dallas "One-Wire" CRC of the specified
 *                byte array.
 *
 *  Input:        buf  -  Address of first byte of byte array
 *                len  -  Count of bytes in array.
 *
 *  Output:       Calculated Dallas "One-Wire" CRC-8.
 *
 *  Note:         The Dallas DS-19xx chips calculate the CRC from "last"
 *                byte to "first" byte (end of buffer to start of buffer).
 *
 *                Also, the length is just a byte, so only small buffers
 *                or strings can be Dallas_CRC8()'ed.
 *
 *  Finally:      The Dallas CRC-8 algorithm is such that if the last byte
 *                read/CRC'ed is the actual CRC-8 of all preceding bytes,
 *                then the resultant CRC-8 value will be "0". This is in
 *                fact the on-chip format of the 8-byte Dallas "header" --
 *                the family code (1 byte), serial number (6 bytes), and
 *                CRC-8 (1 byte).
 *
 ****************************************************************************/

UINT8 Dallas_CRC8
    (
    UINT8 *buf,                 /* Address of byte data buffer */
    UINT8   len                  /* Count of bytes to check */
    )
{
    UINT8 crc;                   /* Running CRC */
    UINT8 *ptr;                  /* Running buffer pointer */
unsigned char temp;

    // last_routine = 0x74;
    crc = 0;                    /* Initial "seed" value for CRC-8 */

    ptr = buf + len - 1;        /* Start with last ("LSB") data byte */

    while (len-- > 0)		/* Calculate CRC for buffer */
    {                       /* Walk the buffer backwards... */
      temp = *ptr--;
      crc = DOW_CRC_tab[(crc ^ temp)];

    }

    return (crc);

} /* End Dallas_CRC8() */

/*************************************************************************
 *  subroutine:     Reset_Bypass_SN()
 *
 *  function:
 *
 *      "Resets" the bypass key serial number array, either to all zeros
 *      (no bypass in effect) or to all ones (bypass by VIPER/software/
 *      ModBus control).
 *
 *************************************************************************/
void Reset_Bypass_SN
    (
    unsigned char val                    /* 00 or FF */
    )
{
    unsigned char i;

    // last_routine = 0x76;
    for (i = 0; i < BYTESERIAL; i++)
        bypass_SN[i] = val;

} /* End Reset_Bypass_SN() */

/*************************************************************************
 *  subroutine:     Reset_Truck_SN()
 *
 *  function:
 *
 *      "Resets" the truck serial number array, either to all zeros (no
 *      truck present) or to all ones (truck -- or something -- is present
 *      but we haven't ID'ed it yet).
 *************************************************************************/
void Reset_Truck_SN(unsigned char val)                    /* 00 or FF */
{
int i;

    // last_routine = 0x77;
  for (i = 0; i < BYTESERIAL; i++)
  {
    truck_SN[i] = val;
  }

} /* End Reset_Truck_SN() */


/****************************************************************************
 *
 *  Subroutine: Read_Dallas_SN()
 *
 *  Function:    This routine reads the serial number from a dallas 1-wire chip from
 *                the specefied port.
 *
 *       1.  Reset the 1-wire and return FALSE if presence pulse not seen.
 *       2.  Send a 0x33 READ_ROM command; return FALSE if error.
 *       3.  Read 8 bytes (family, S/N & CRC8) fom 1-wire into touchbuf[] and 
 *            if reading bypass key also into save_bypass_key[].
 *       4.  Calculate the CRC to validate the data read, return FALSE if error.
 *       5.  If reading a TIM, determine from family code if possible ithe 1-wire device
 *            and set family code, memory size, scratch pad size and SuperTIM flag.
 *
 *  Input:         None
 *  Output:      TRUE if the dallas serial number read OK; otherwise, FALSE
 *        If the serial number is read OK, it is returned in the array touchbuf[]
 *
 ****************************************************************************/

char Read_Dallas_SN (unsigned char port)

{
unsigned int i, j;                        /* Real-time counter for Tslot */
unsigned char crc8;
unsigned char temp;

   if (Dallas_Reset(port) != TRUE)
   {
      return (FALSE);
   } 
   // last_routine = 0x78;
   if (Dallas_Byte (0x33, port) != 0x33)  /* Send READ_ROM command */
   {
      return (FALSE);                     /* Command not sent properly */
   } 
   for (i = 8, j=0; i > 0; --i, j++)                /* Read family, S/N & CRC8 */
   {
      temp = Dallas_Byte (0xFF, port);
      touchbuf[i-1] = temp;
      if ( port == READ_BYPASS) 
      {
        save_bypass_key[j] = temp;
      }
   }

  crc8 = Dallas_CRC8 (touchbuf, 8);        /* Verify valid data read */
  if (crc8 != 0)
    {
       return (FALSE);             /* Unsuccessful read attempt */
    }  
  if ( port == COMM_ID)
  {
    S_TIM_code = FALSE;
    family_code = touchbuf[7];  /* Store away the current truck TIM family code */
    switch (family_code)
	{
		case DS2401:
         TIM_size = DS2401_SIZE;
         TIM_scratchpad_size = DS2401_SCRATCHPAD_SIZE;
        break;
		case DS1992:
		case DS1993:
         TIM_size = DS1992_SIZE;
         TIM_scratchpad_size = DS1992_SCRATCHPAD_SIZE;
        break;
		case DS1996:
          TIM_size = DS1996_SIZE;
          TIM_scratchpad_size = DS1996_SCRATCHPAD_SIZE;
          S_TIM_code = TRUE;
        break;
 		case DS2431:
         TIM_size = DS2431_SIZE;
         TIM_scratchpad_size = DS2431_SCRATCHPAD_SIZE;
        break;
		case DS2433:
          TIM_size = DS2433_SIZE;
          TIM_scratchpad_size = DS2433_SCRATCHPAD_SIZE;
        break;
		case DS28EC20:
          TIM_size = DS28EC20_SIZE;
          TIM_scratchpad_size = DS28EC20_SCRATCHPAD_SIZE;
          S_TIM_code = TRUE;
        break;

        default:
          TIM_size = DEFAULT_SIZE;
          TIM_scratchpad_size = DEFAULT_SCRATCHPAD_SIZE;
          printf("\n\rBlock Read CMD: Family code not known: 0x%02X\n\r", touchbuf[7]);
        break;
    }   
  }

  return (TRUE);                                     /* Return successful read */

}  /* end of Read_Dallas_SN() */


/****************************************************************************
 *
 *  Subroutine:   Read_Bypass_SN()
 *
 *  Function:     Read the bypass key serial number.
 *
 *       1.  This routine reads a bypass key.  It assumes that the key is
 *           already present (issues RESET pulse, returns false if no
 *           PRESENCE pulse follows)
 *       2.  Send a 0x33 READ_ROM command a bit at a time (LSB first).
 *       3.  Read the next 64 bits (8 bytes) a bit at a time (LSB/LSB first).
 *       4.  Check the family code, returning FALSE if not correct (1990 = 1).
 *       5.  Calculate the CRC to validate the data read.
 *       6.  If the CRC is 0, the data has been read properly, transfer the
 *           serial number read, and return TRUE.
 *       7.  If the CRC is not 0, the data is not correct, and return FALSE.
 *
 *  Input:   None
 *  Output:  TRUE if bypass serial number read OK; otherwise, FALSE
 *           The serial number is returned in the array: bypass_SN[]
 *
 *           If in ASCII line mode, issue messages to update user/engineer...
 *
 ****************************************************************************/

char Read_Bypass_SN (void)
{
unsigned int i;                        /* Real-time counter for Tslot */
char status;

  // last_routine = 0x79;
  if (Dallas_Reset (READ_BYPASS) == 0)       /* Issue reset pulse */
  {
    return (FALSE);
  }
  for ( i=0; i<5; i++)
  {
   if ((status = Read_Dallas_SN (READ_BYPASS)) != 0)
   {
     break;
   } else
   {
     if (Dallas_Reset (READ_BYPASS) == 0)       /* Issue reset pulse */
     {
       return (FALSE);
     }
   }
  }

  if ((save_bypass_key[0] == 0xFF) || (save_bypass_key[7] == 0xFF))
  {
    return (FALSE);
  }

  if ( !status)
  {
    return (FALSE);
  }

  xprintf( 156, DUMMY);                  /* "Bypass Key detected:" */

  for (i = 0; i < BYTESERIAL; i++)       /* Copy bypass serial number */
  {
    bypass_SN[i] = touchbuf[i+1];
  }
  if (touchbuf[7] != 0x01)               /* Check for DS1990 family code */
  {
    xprintf( 158, touchbuf[7]);         /* Complain to whomever will listen */
    xprintf( 58, DUMMY);
    Report_SN (bypass_SN);                 /* Print serial number */
    Reset_Bypass_SN(0);
    return (FALSE);
  }
  xprintf( 58, DUMMY);
  Report_SN (bypass_SN);                 /* Print serial number */
  xprintf( 159, touchbuf[7]);            /* Cap off with family code */
  if (badoverfillflag)
  {
    printf ("  OK to Bypass\n\r"); //DHP debug
  }else
  {
    printf ("  Not allowed to Bypass now \n\r"); //DHP debug
  }
  return (TRUE);                         /* Bypass S/N read OK */
}  /* end of Read_Bypass_SN() */


/****************************************************************************
 *
 *  Subroutine:   Read_Truck_Presence()
 *  Function:      Determine if truck is present
 *
 *       1.  This routine checks that the communication line is available, returns 02 if not
 *       2.  Attempts up to 5 tries at seeing a presence pulse on the communication line
 *       3.  Sets the vehicle LED to PULSE if a presence pulse is seen
 *       4.  Returns a TRUE or FALSE
 *
 *  Input:   None
 *  Output:  TRUE      (0x01) if valid presence pulse response
 *               FALSE    (0x00) if presence pulse response is invalid or absent
 *               Not Read (0x02) if communication line (pin 9) not available
 *           
 ****************************************************************************/
unsigned char Read_Truck_Presence (void)
{
unsigned char sts = 0, retry = 0;

  // last_routine = 0x7A;
  if (active_comm & (INTELLI | GROUNDIODE)) /* COMM_ID aka TXA/RXA in use? (4|8) */
     return (2);                            /* Yes, try again later */

  while ( retry++ < 5)
  {
   active_comm |= TIM;                  /* Mark COMM_ID in use by us! (2) */
   if ((sts = Dallas_Reset(COMM_ID)) == 1)     /* Issue reset pulse */
   {
     break;
   }
  }
  active_comm &= ~TIM;                 /* COMM_ID line now free */
  if (sts)                             /* Did we find a truck? */
  {
    ledstate[TRUCKCOMM] = (unsigned int)PULSE;       /* Yup... */
  }
  return (sts);                        /* Return Presence/Absence */
}  /* end of Read_Truck_Presence() */

/*********************************************************************
 *
 *  Subroutine:   Read_Truck_SN()
 *  Subroutine:   Check_Truck_SN()
 *
 *  Function:     Read the truck serial number.
 *
 *  1.  This routine checks that the communication line is available,
 *      returns FALSE if not
 *  2.  This routine issues a reset pulse and ensures a valid return pulse;
 *      if not BVF_TIMABSENT is set in badvipflag and a FALSE is returned.
 *  3.  An attempt to read the 1-wire serial number is made.  If unsuccessful
 *      BVF_TIMCRC is set in badvipflag and a FALSE is returned.
 *  4.  If successful error flags are cleared and S/N is copied to truck_SN[]. 
 *  5.  If 1-wire family code shows a SuperTIM, the TIM compartment info is read.
 *  6.  A TRUE is returned indicating a serial number was successfully read.
 *
 *  Input:   None
 *  Output:  TRUE  (0x01) if serial number read from 1-wire device
 *           FALSE (0x00) if presence pulse response is invalid or absent or
 *              an error occurred on the read or the communication line (pin 9)
 *              not available.
 ********************************************************************/
char Read_Truck_SN (void)
{
unsigned int i;                        /* Real-time counter for Tslot */
char sts;
  if (active_comm & (INTELLI | GROUNDIODE)) /* COMM_ID aka TXA/RXA in use? */
  {                                           /* Might be by Ground Test */
    return (FALSE);                   /* Yes, try again later !!!! */
  }
  active_comm |= TIM;                  /* Mark COMM_ID in use by us! (2) */

  if (!READ_COMM_ID_BIT)    /* Fix if ground is disabled */
  {
    COMM_ID_BIT = 1;
    ODCD |= 0x5;
    DelayUS(480);
  }
  if (Dallas_Reset (COMM_ID))          /* Issue reset pulse */
  {
    badvipflag &= ~BVF_TIMABSENT;     /* Clear TIM absent bit (8) */
    if (Read_Dallas_SN (COMM_ID))     /* Read serial number */
    {
      badvipflag &= ~BVF_TIMCRC;       /* Good CRC (and data comm) - clear error bit (2) */
      badvipflag &= ~BVF_TIMFAMILY;    /* Good Dallas family - clear error bit (4) */
      /* Copy Vehicle ID (serial number - 6 char's) */
      for (i = 0; i < BYTESERIAL; i++) /* Copy Vehicle ID (serial number - 6 char's) */
      {
        truck_SN[i] = touchbuf[i+1];
      }                           /* Authorization handled by caller */
      printf("\n\r     Truck TIM ID: ");
      Report_SN ((unsigned char *)&touchbuf[1]);

      if (S_TIM_code)                     /* is the TIM a Super TIM? */
      {
        if (read_TIM_compartment_info())
        {
          // printf(" FAILED\n\r");  /* Place for error code */
        }
      }
      sts = TRUE;                 /* Successfully read TIM */
    }
    else
    {                                 /* Can't read "Serial Number" */
      badvipflag |= BVF_TIMCRC;      /* CRC (or data comm) error bit set */
      sts = FALSE;
    }
  }
  else
  {                                    /* No RESET => Presence Pulse */
    badvipflag |= BVF_TIMABSENT;      /* No TIM, set appropriate status bit */
    sts = FALSE;                      /* Set return status = failed */
  }

  active_comm &= ~TIM;                 /* COMM_ID line free now */

  return (sts);

}  /* end of Read_Truck_SN() */

/*************************************************************************/
/*************************************************************************/

static unsigned short TIMtime;  /* Periodic TIM check time [self-init] */

/* Call only if you're willing to spend the time to read the TIM again! */
char Check_Truck_SN
    (
    char slowflag               /* TRUE (non-zero) to read and verify TIM
                                   FALSE (zero) to just check for Presence */
    )
{
unsigned int i;
char sts;

   /* Default return value is "FALSE" unless we can truly verify that the
      proper truck is connected (or just TIM present if slow flag == 0). Since
      a ground test in progress prohibits our testing the TIM, it is the caller's
      problem to deal with "false" FALSEs. This is necessary for example,
      for check_truck_gone()! */

   // last_routine = 0x7C;
   sts = TRUE;

   if (active_comm & (INTELLI | GROUNDIODE)) /* COMM_ID aka TXA/RXA in use? */
      return (sts);                     /* Yes, try again later */

   if (DeltaMsTimer (TIMtime) < SEC1)   /* Been awhile? */
      return (sts);                     /* No, let TRUCKCOMM LED go out. */

   active_comm |= TIM;                  /* Mark COMM_ID in use by us! */

   TIMtime = mstimer;                   /* Reset for "next" time */

   if (Dallas_Reset (COMM_ID))          /* Issue reset pulse */
   {                                 /* TIM/Dallas Presence pulse detected */
      if (slowflag)                     /* Want to verify the TIM too? */
      {
         if (Read_Dallas_SN (COMM_ID))     /* Read serial number */
         {
            for (i = 0; i < BYTESERIAL; i++) /* Copy Vehicle ID (serial number) */
            {
               if (truck_SN[i] != touchbuf[i+1])
                   sts = FALSE;            /* Serial Number changed! */
            }
         }
         else
         {                              /* Can't read "Serial Number" */
            sts = FALSE;
         }
      } /* End want to check TIM (slow flag set) */
   } /* End Presence pulse detected */
   else
   {                                 /* No RESET => Presence Pulse */
      sts = FALSE;                      /* No TIM */
   }

   active_comm &= ~TIM;                 /* COMM_ID line free now */

  return (sts);                        /* Return Good/Bad "still connected" */

}  /* end of Check_Truck_SN() */

/****************************************************************************
 *
 *  Subroutine:   Read_Clock()
 *
 *  Function:     This routine reads the time from the on-board Dallas clock.
 *
 *       1.  Issue a reset pulse and wait for a presence pulse.
 *       2.  If no valid presence pulse is returned, return FALSE.
 *       3.  Send a 0x33 READ_ROM command a bit at a time (LSB first), and
 *           return FALSE if not sent properly.
 *       4.  Read the next 64 bits (8 bytes) a bit at a time (LSB first).
 *       5.  Calculate the CRC to validate the data read.
 *       6.  If the CRC is not 0, the data is not correct, and return FALSE.
 *       7.  Check the family code, returning FALSE if not correct (1994 = 4).
 *       8.  Send a 0xF0 READ_MEMORY command a bit at a time (LSB first), and
 *           return FALSE if not sent properly..
 *       9.  Send a 0x02 (LSB) and a 0x02 (MSB) a bit at a time to read the
 *           time from address 0x0202H, and return FALSE if not sent properly.
 *      10.  Read the fraction counter byte a bit at a time (LSB first).
 *      11.  Read the UNIX date (next 4 bytes) a bit at a time (LSB first).
 *
 *      12.  Issue a reset pulse and wait for a presence pulse.
 *      13.  If no valid presence pulse is returned, return FALSE.
 *      14.  Send a 0xCC SKIP_ROM command a bit at a time (LSB first), and
 *           return FALSE if not sent properly.
 *      15.  Repeat steps 8 through 11.
 *      16.  If the UNIX date from the second reading is not equal to
 *           the first reading, or is greater than the first reading by more
 *           than one, then the date is unreadable because the line is noisy,
 *           and return FALSE.
 *      17.  If the fraction counters are equal, the clock is not running,
 *           return FALSE.
 *      18.  If the date is January 1, 1970 at midnight, the clock is set to
 *           the factory default and is not initialized, return FALSE.
 *      19.  Compare the date for validity (1994 >= date < 2050) and return
 *           FALSE if not valid.
 *      20.  Otherwise, the clock is running, transfer the clock serial
 *           number, transfer the UNIX date, and return TRUE.
 *
 *  Input:   None
 *  Output:  0 if clock present & time read OK; otherwise, clock_status
 *           clock_status - Set to the status of reading the clock:
 *              CLOCK_OK, CLOCK_ABSENT, CLOCK_CRC, CLOCK_FAMILY,
 *              CLOCK_RANGE, CLOCK_STOPPED, CLOCK_NOISY, CLOCK_DEFAULT.
 *           The serial number is returned in the array:
 *              clock_SN[]
 *           The time is returned in the global:
 *              present_time
 *
 ****************************************************************************/

char Read_Clock (void)
{
UINT8 data;
unsigned int i;
unsigned long second_time;

  // last_routine = 0x7D;
  for ( i=0; i<sizeof(clock_SN); i++)  /* Reset serial number */
  {
    clock_SN[i] = 0;
  }
  if (!Read_Intellitrol_SN ())      /* Read serial number */
    return (clock_status = CLOCK_CRC);

  if (touchbuf[7] != DS2401_SERIAL_ID)        /* Check for correct family code */
  {
    xprintf( 158, touchbuf[7]);               /* Cap off message with error text */
    return (clock_status = SERIAL_FAMILY);
  }
  for (i = 0; i < BYTESERIAL; ++i)       /* Copy Intellitrol serial number */
    clock_SN[i] = touchbuf[i+1];

  second_time = 0;                       /* Get second measured time */
  if (I2C1_read(DS1371_DEVICE, 8, &data, 1))  /* Fetch the status */
  {
    xprintf( 157, 8);               /* Cap off message with error text */
    return (clock_status = CLOCK_NOISY);
  }

  if ( !(data & 0x80) )     /* Test OSF see the oscillator is running */
  {
    /*****************************6/11/2008 9:44AM*****************************
     * oscillator either is stopped or was stopped for some period and can be
     * used to judge the validity of the timekeeping data
     **************************************************************************/
    data = 0x00;          /* Start the oscillator by clearing the OSF bit*/
    if (I2C1_read(DS1371_DEVICE, 8, &data, 1))  /* Fetch the status */
    {
      xprintf( 132, DS1371_DEVICE); /* Cap off message with error text */
      return (clock_status = CLOCK_NOISY);
    }
    if ( !(data & 0x80) )     /* Test OSF see the oscillator is running */
    {
      return (clock_status = CLOCK_STOPPED);
    }
  }

  for (i = 4; i > 0; i--)
  {
     second_time <<= 8;
     (void)I2C1_read(DS1371_DEVICE, (unsigned char)(i-1), &data, 1);  /* Fetch the time count */
     second_time |= data;
  }

  present_time = second_time;                     /* Return time */

  if ((second_time == JAN_01_1970))      /* Clock default time? */
  {
    return (clock_status = CLOCK_DEFAULT);
  }

  if (second_time < JAN_01_1994)           /* Clock in range? */
  {
     return (clock_status = CLOCK_RANGE);
  }
  return (clock_status = CLOCK_OK);      /* Clock read OK */

}  /* end of Read_Clock() */


/****************************************************************************
 *
 *  Subroutine:   Write_Clock()
 *
 *  Function:     This routine writes the time to the on-board Dallas clock
 *                and starts the clock running, as well as defaulting all of
 *                the other DS2404 features.  It is recomended to make a call
 *                to Read_Clock to verify the clock is set properly and run-
 *                ning after setting the clock with Write_Clock ().
 *

 *
 *  Input:        None
 *  Output:       0 if clock present & time read OK; otherwise, clock_status
 *                clock_status - Set to the status of reading the clock:
 *                   CLOCK_OK, CLOCK_ABSENT, CLOCK_CRC, CLOCK_FAMILY,
 *                   CLOCK_RANGE, CLOCK_STOPPED, CLOCK_NOISY
 *                The time is returned in the global:
 *                   present_time
 *
 ****************************************************************************/

char Write_Clock (void)
{
unsigned char i;                                 /* Index variable */
UINT8 Temp, data_verify;                 /* Byte to send */
unsigned long UNIX_Time, Temp_Time;     /* Time to set */

   // last_routine = 0x7E;
   Temp_Time = UNIX_Time = present_time;           /* Get time to send */
   for (i = 0; i < 4; ++i)
   {
      Temp = (char)(Temp_Time & 0x000000FF);
      if (I2C1_write(RTC_DEVICE, i, &Temp, 1))  /* Write the time on byte at a time */
      {
        xprintf( 134, RTC_DEVICE);               /* Cap off message with error text */
        return (clock_status = CLOCK_NOISY);
      }
      Temp_Time >>= 8;                    /* Get next byte */
   }

   Temp_Time = UNIX_Time;                 /* Get time to verify */
   for (i = 0; i < 4; ++i)
   {
      Temp = (char)(Temp_Time & 0x000000FF);
      if (I2C1_read(RTC_DEVICE, i, &data_verify, 1))  /* Write the time one byte at a time */
      {
        xprintf( 157, 8);               /* Cap off message with error text */
        return (clock_status = CLOCK_NOISY);
      }
      if ( data_verify != Temp)
      {
        return (clock_status = CLOCK_NOISY);
      }
      Temp_Time >>= 8;                    /* Get next byte */
   }

   return (clock_status = CLOCK_OK);      /* 31. Successful write */

}  /* end of Write_Clock() */

/****************************************************************************
 *
 *  Subroutine:   UNIX_to_Greg()
 *
 *  Function:     Convert a UNIX date into a Gregorian date.
 *
 *       1.  This routine reads the UNIX date (number of seconds since
 *           January 1, 1970) and converts it to a Gregorian date and time.
 *
 *  Input:        None
 *                The UNIX time is converted from the global:
 *                   present_time
 *  Output:       None
 *                The Gregorian time is returned in the globals:
 *                   month, day, year, hour, minute
 *
 ****************************************************************************/

void UNIX_to_Greg (void)
{
   unsigned long seconds, temp;
   unsigned char not_leap_year, days_in_month;

   // last_routine = 0x7F;
   seconds = present_time;                         /* Get the current time */

   second = seconds % 60;                 /* Save seconds */
   seconds /= 60;                         /* Throw away seconds */

   minute = (char)(seconds % 60);         /* Calculate minute */
   seconds /= 60;

   hour = (char)(seconds % 24);           /* Calculate hour */
   seconds /= 24;

   temp = seconds / 1461;                 /* Calculate year (takes care of leap years too) */
   temp *= 4;
   year = (unsigned int)(temp + 1970);
   seconds %= 1461;
   if (seconds >= 365)
   {
      ++year;
      if ((seconds -= 365) >= 365)
      {
         ++year;
         if ((seconds -= 365) >= 366)
         {
            ++year;
            seconds -= 366;
         }
      }
   }

   month = 1;                             /* Calculate month */
   ++seconds;
   /* The year 2000 is an anti-anti-leap year, and thus behaves as a normal
      leap year . . . -RDH */
   not_leap_year = (unsigned char)(year % 4);
   int loop = 1;
   do
   {
      days_in_month = month_days[month - 1];
      if ((month == 2) && !not_leap_year)
         ++days_in_month;
      if (seconds <= days_in_month)
         break;
      ++month;
      seconds -= days_in_month;
   } while (loop == 1);

   day = (unsigned char)seconds;                   /* Calculate day */

}  /* end of UNIX_to_Greg() */

/****************************************************************************
 *
 *  Subroutine:   Greg_to_UNIX()
 *
 *  Function:     Convert a Gregorian date into a UNIX date.
 *
 *       1.  This routine reads the Gregorian date and converts it to a UNIX
 *           date (number of seconds since January 1, 1970).
 *
 *  Input:  Year, Month, Day, Hour, Minute
 *
 *  Output: 32-bit "unixtime", or 0 if error
 *
 ****************************************************************************/

unixtime Greg_to_UNIX
    (
    unsigned p_year,                      /* Full year (e.g., "1995") */
    unsigned p_month,                     /* Month within year (1 = January, etc.) */
    unsigned p_day,                       /* Day within month */
    unsigned p_hour,                      /* Hour within day */
    unsigned p_minute                     /* Minute within hour */
    )
{
   unsigned long seconds;
   unsigned int years;
   unsigned int i;

   // last_routine = 0x80;
   if (p_year < 1992)                     /* Check range */
      return (0);

   years = p_year - 1970;                 /* Days/Years */
   seconds = ((long)years * 365) + (((long)years + 1) / 4);

   if ((p_month < 1) || (p_month > 12))   /* Check range */
      return (0);

   for (i = 1; i < p_month; ++i)
      seconds += month_days[i - 1];
   /***************************** 3/7/2011 7:10AM ****************************
    * years is based from 1970 which is two years from leap year so add 2
    * before testing for leap year.
    **************************************************************************/
   if ((!((years + 2) % 4)) && (p_month > 2))  /*  */
      ++seconds;

   if ((p_day < 1) ||
       (p_day > month_days[p_month - 1]))   /* Check range */
   {
      /***************************** 3/7/2011 7:10AM *************************
       * years is based from 1970 which is two years from leap year so add 2
       * before testing for leap year.
       ***********************************************************************/
      if (!((((years + 2) % 4) == 0) && (p_month == 2) && (p_day == 29)))
          return (0);
   }

   seconds += p_day - 1;                  /* Days */

   if (p_hour >= 24)                      /* Check range */
      return (0);

   seconds = (seconds * 24) + p_hour;     /* Hours */

   if (p_minute >= 60)                    /* Check range */
      return (0);

   seconds = (seconds * 60) + p_minute;   /* Minutes */

   seconds = seconds * 60;          /* Seconds */

   return (seconds);

}  /* end of Greg_to_UNIX */

/****************************************************************************
 *
 *  Subroutine:   Print_Crnt_Time()
 *
 *  Function:     Print current date and time.
 *
 *       1.  This routine reads prints the current time in mm/dd/yyyy hh:mm.
 *
 *  Input:        None
 *                The UNIX time is printed from the global:
 *                   present_time
 *  Output:       None
 *
 ****************************************************************************/

void Print_Crnt_Time (void)
{
  // last_routine = 0x81;
  (void)Read_Clock();
  UNIX_to_Greg ();                       /* Convert time */

  xprintf( 52, (unsigned char)month );
  xprintf( 52, (unsigned char)day );
  xprintf( 54, (unsigned int)year );
  xprintf( 53, (unsigned char)hour );
  xprintf( 53, (unsigned char)minute );
  xprintf( 54, (unsigned char)second );
}
/******************************* 12/30/2008 7:28AM ***************************
 * This routine will fetch the truck configuration previously stored in the
 * Truck's TIM.
 *****************************************************************************/
int read_TIM_compartment_info()
{
unsigned char mem_ptr[10];
unsigned int i;
//unsigned int temp_word;

  // last_routine = 0x82;
  clear_gcheck();                   /* Drive GCHECK low */
  ODCD = 0x5;
  TRISD &= ~COMM_ID;      /* Set to output */
  COMM_ID_BIT = 1;

  for ( i=0; i<sizeof(Truck_TIM_Configuration); i++)
  {
    Truck_TIM_Configuration[i] = 0;
  }

  if ( S_TIM_code )       /* is the TIM a Super TIM? */
  {
    return GOOD;
  }
  
  if (tim_block_read(mem_ptr, INTELLICHECK_TYPE_ADDR, INTELLICHECK_TYPE_SIZE) != MB_OK)
  {
    return FAILED;
  }

  if (mem_ptr[0])
  {
    printf("\n\r     Truck has an IntelliCheck\n\r");
    number_of_Probes = 0xAA;
    return GOOD;
  }
  
  if (tim_block_read(Truck_TIM_Configuration, NUMBER_OF_COMPARTMENTS_ADDR, 1) != MB_OK)
  {
    return FAILED;
  }
  number_of_Compartments = (unsigned int)Truck_TIM_Configuration[0];  /* number of compartments stored in the TIM */
  
  return GOOD;
}

/******************************* 2/16/2010 6:32AM ****************************
 * This routine will send a 0xAA to the bypass one wire interface. If the answer
 * is 0x55 it is the new front panel else it is the old front panel
 *****************************************************************************/
void test_for_new_front_panel(void)
{
unsigned char present_byte;
unsigned int save_iec0;

  save_iec0 = IEC0;
  IEC0 = 0;                   /* Disable heart beat and DMA interrupt */

  new_front_panel = FALSE;

  if (reset_iButton(READ_BYPASS) != GOOD)
  {
    IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */
    return;
  }

  if (Dallas_Byte (0xAA, READ_BYPASS) != 0xAA)  /* Send READ_ROM command */
  {
    IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */
    return;                           /* Command not sent properly */
  }
  present_byte = Dallas_Byte (0xFF, READ_BYPASS);
  if ( present_byte == 0x55)
  {
    new_front_panel = TRUE;
    printf("%c", 0x1B);
    printf("[32m");
    printf("\n\r    Detected New Front Panel\n\r");
    printf("%c", 0x1B);
    printf("[30m");
  }

  IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */
}  /* test_for_new_front_panel */


/******************************* 12/30/2008 7:28AM ***************************
 * read_TIM_Go_NoGo_info
 * This routine will fetch the Super TIM flags
 * Set up hardware for TIM access
 * Read SuperTIM data valid flag using tim_block_read(), if not valid return
 * Read SuperTIM NO_RELAY flag and set no_relay_flag accordingly
 * Read SuperTIM Scully Equipment flag and set scully_flag accordingly
 * 
 *****************************************************************************/
void read_TIM_Go_NoGo_info()
{
unsigned char temp_byte;

  // last_routine = 0x82;
  clear_gcheck();                   /* Drive GCHECK low */
  ODCD = 0x5;
  TRISD &= ~COMM_ID;      /* Set to output */
  COMM_ID_BIT = 1;

  /**************************** 8/23/2011 3:10PM *************************
   * Now check if we need to disable the relays when we permit
   ***********************************************************************/
/*
  if ( temp_byte == 0x33)
  {
    no_relay_flag = TRUE;
    printf("\n\r     *** ENTERING SPECIAL TEST MODE. RELAYS ARE DISABLED. ***\n\r\n\r");
  } else
  {
    no_relay_flag = FALSE;
  }
*/
    no_relay_flag = FALSE;
  
  /****************************** 10/20/2011 1:28PM **************************
   * Now check if Scully Truck
   ***************************************************************************/
  if (tim_block_read((unsigned char*)&temp_byte, SCULLY_SENSORS_ADDR, SCULLY_SENSORS_SIZE) != MB_OK)
  {
    return;
  }

  if ( temp_byte == 0x33)
  {
    if(scully_flag == FALSE)
    {
      printf("\n\r     *** Truck is outfitted with all Scully Eguipment. ***\n\r\n\r");
      scully_flag = TRUE;
    }
  } else
  {
    scully_flag = FALSE;
  }
}
/***************************** End of DALLAS.C ******************************/
