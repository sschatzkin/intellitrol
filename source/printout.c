/*********************************************************************************************
 *
 *  Project:   Rack Controller
 *
 *  Module:       printout.C
 *
 *  Revision:     REV 1.5
 *
 *  Author:       Ken Langlais  @Copyright 2010, 2014  Scully Signal Company
 *
 *  Description:  Main program printout routines using printf()
 *
 *
 *  Revision History:
 *  Revision   Date        Who  Description of Change Made
 *  --------   --------    ---  --------------------------------------------
 *  1.5.23     10/19/11    KLL  Changed print of gnd_retry value from hex to decimal
 *             05/02/12    KLL  Added more message flags to xprintf 47
 *                              Added set pass message flag after case 36, 37, 38, and
 *                                39 which are error messages
 *                              Added message 144 so if service light, the debug terminal
 *                                will display the contents of the error registers that should
 *                                indicate why the service led is on.
 *  1.5.27     10/16/12    KLL  Added message 145 to report the timeout when trying to figure out the probe.
 *                              If the Intellitrol is reset during the printing out of an error message the
 *                              screen will remain red. Added turning the the screen to black in the startup
 *                              message.
 *  1.5.30  08/10/14  DHP  Changed text as needed to reflect changes in ground type
 *                                        and unit type display
 *  1.6.35  02/07/17  DHP  Updated case 136 to indicate deadman fault rather than only open
 **********************************************************************************************/


#include "common.h"
#include "version.h"    /* Shell version information */

//lint -esym(843, __Compatible)

extern char b_date[];

/************************** Dallas Constants ********************************/

static const char state_string0[] = "Idle   ";
static const char state_string1[] = "Acquire";
static const char state_string2[] = "Active ";
static const char state_string3[] = "Gone   ";
static const char state_string4[] = "Finish ";

static const char * const state_string[] = {state_string0,
                                     state_string1,
                                     state_string2,
                                     state_string3,
                                     state_string4};

static const char serial_message0[] = "DS2401";
static const char serial_message1[] = "Invalid message number";

static const char clock_message0[] = "OK";
static const char clock_message1[] = "Chip not present";
static const char clock_message2[] = "CRC8 read error";
static const char clock_message3[] = "DS2401 Not present";
static const char clock_message4[] = "Time out of range";
static const char clock_message5[] = "Clock not running";
static const char clock_message6[] = "Clock unreadable";
static const char clock_message7[] = "Clock Default 1970";

static const char * const clock_message[] = {clock_message0,    /* lint -esym (843) 0 */
                                      clock_message1,    /* 1 */
                                      clock_message2,    /* 2 */
                                      clock_message3,    /* 3 */
                                      clock_message4,    /* 4 */
                                      clock_message5,    /* 5 */
                                      clock_message6,    /* 6 */
                                      clock_message7,    /* 7 */
                                      serial_message0,   /* 8 */
                                      serial_message1};  /* 9 */

static const char bypass_message0[] = "Key Present";
static const char bypass_message1[] = "Key Rejected";
static const char bypass_message2[] = "Overfill";
static const char bypass_message3[] = "Ground";
static const char bypass_message4[] = "VIP";

static const char * const bypass_message[] = {bypass_message0,  /* 0 */
                                       bypass_message1,  /* 1 */
                                       bypass_message2,  /* 2 */
                                       bypass_message3,  /* 3 */
                                       bypass_message4,  /* 4 */
                                       serial_message1}; /* 5 */

static const char eeb_message0[] = "";
static const char eeb_message1[] = "Home";
static const char eeb_message2[] = "Boot";
static const char eeb_message3[] = "Crash";
static const char eeb_message4[] = "System Parameters";
static const char eeb_message5[] = "Event Log";
static const char eeb_message6[] = "Bypass Key";
static const char eeb_message7[] = "Truck ID";

static const char * const eeb_message[] = {eeb_message0,
                                    eeb_message1,
                                    eeb_message2,
                                    eeb_message3,
                                    eeb_message4,
                                    eeb_message5,
                                    eeb_message6,
                                    eeb_message7};

/****************************************************************************/
/****************************************************************************/

void ee83sts(unsigned int eests)
{
    if (eests & EE_DATAERROR)
    {
        printf ("Can't read and/or write; ");
    }else 
//FogBugz 131    if (eests & EE_WRITETIMEOUT)
    if (eests & EE_NEW)  //FogBugz 131
    {
        printf ("New EEPROM being Formatted");
    }else //look at EE_FORMAT only if not new
    if (eests & EE_FORMAT)
    {
        printf ("Needs to be Formatted; ");
    }else 
    if (eests & EE_BADHOME)
    {
        printf ("Home block CRC failure (needs formatting?); ");
    }else
    if (eests & (EE_ACTIVE | EE_BUSY))
    {
        printf ("Active/Busy; ");    /* Shouldn't happen! */
    }else
// >>> FogBugz 142
    if (eests & EE_CRC)
    {
        printf ("EEPROM CRC failure");
    }else
    {
        printf ("status = %x",eests);
    }
//<<< FogBugz 142
 } /* End ee83sts() */

/****************************************************************************/

/*************************************************************************
 *  subroutine:      xprintf()
 *
 *  function:
 *         1. A callable routine that prints the message called
 *         2. A message is only repeated once
 *         3. A table of messages is included here in the switch
 *
 *  input: byte message_number, unsigned int parameter1
 *  output: none
 *
 *
 *************************************************************************/

void   xprintf( unsigned int message_number, unsigned int parameter1 )
{
static unsigned int main_message = 0;     /* bit fields determining output */
static unsigned int ack_message = 0;
static unsigned int act_message = 0;
static unsigned int toggle = 0;           /* toggle prevents overwrite of a last message */
static unsigned int print_once = 0;
unsigned int bit_mask = 0;
static int majver, minver, edtver;

   if (modbus_addr != 0)                   /* ASCII only if modbus address 0 */
      return;

   if (message_number==11)
   {
      asm volatile("nop");    /* These NOPs are needed for the  */
      asm volatile("nop");    /* write cycle to complete */
      asm volatile("nop");    /*  */
   }

   if (message_number<16)
      {
      bit_mask = ((unsigned int)1 << message_number);
      }
    else if (message_number<32)
      {
      bit_mask = (unsigned int)1 << (message_number-16);
      }
    else if (message_number<48)
      {
      bit_mask = (unsigned int)1 << (message_number-32);
      }
    else if (message_number<64)
      {
      bit_mask = (unsigned int)1 << (message_number-48);
      }

    if ( message_number==0 )
      {
      main_message = 0x800F;
      act_message |= 0x00F0;
      }
    if ( (message_number<16) &&
        (main_message & bit_mask) )
    {
      main_message &= ~((unsigned int)1 << message_number);
      switch (message_number)
        /********** DEBUG STATES *****************/
        {
        case 0:
           majver = (updater_rev >>12);
           minver = (updater_rev >>8) & 0xF;
           edtver = (updater_rev  & 0x00FF);
           printf("%c", 0x1B);     /* Make sure the below message is printed black */
           printf("[30m");
           printf("\n\r*  Intellitrol");
           if(unit_type == INTELLITROL2)
           {
             printf("2");
           }else
           {
             printf(" PIC CPU");
           }
           printf(" Rack Controller Rev %X.%d.%d  %s, Updater Rev %X.%d.%d\n\r",
                  MAJVER, MINVER, EDTVER, b_date, (majver), (minver), (edtver));
           printf("*  Copyright 2010, 2014 Scully Signal Company. Scully is a registered\n\r");
           printf("*  trademark of Scully Signal Company. All Rights Reserved\n\r");
           printf("*  Specifications are subject to change without notice.\n\r");
           break;
        case 1:
           printf("\n\r    *** The tank state DISAGREED on 6 or 8 compartment");
           break;
        case 2:
           printf("\n\r    8 compartment truck configuration");
           break;
        case 3:
           printf("\n\r    6 compartment truck configuration");
           break;
        case 4:
           printf("\n\r     The MAIN relay seems BROKEN\n\r");
           toggle = TRUE;
           ack_message |= 0x0001;
           break;
        case 5:
           printf("\n\r     The MAIN relay appears SHORTED\n\r");
           toggle = TRUE;
           ack_message |= 0x0001;
           break;
        case 6:
           printf("\n\r     The BACKUP relay seems BROKEN\n\r");
           toggle = TRUE;
           ack_message |= 0x0002;
           break;
        case 7:
           printf("\n\r     The BACKUP relay appears SHORTED\n\r");
           toggle = TRUE;
           ack_message |= 0x0002;
           break;
        case 8:
           printf("\n\r    Ground Detection is not enabled");
           toggle = TRUE;
           main_message &= ~(0x1E00);
           break;
        case 9:
           printf("\n\r    The Ground Bolt Test shows a SHORT");
           toggle = TRUE;
           main_message |= 0x1D00;
           break;
        case 10:
           if (StatusA & STSA_RESISTIVE_GND)
           {
              printf("\n\r    A Resistive Ground is present");
           } else
           {
             if (StatusA & STSA_DIODE_GND)
             {
                printf("\n\r    A Ground Bolt is present");
             }else 
             {
                printf("\n\r    Grounding type unknown");
             }
           }
             toggle = TRUE;
             main_message |= 0x1B00;
             break;
          case 11:
             if (ConfigA & CFGA_100_OHM_GND)
             {
                printf("\n\r    A Ground is not present\n\r");
             } else
             {
                printf("\n\r    A Ground Bolt is not present\n\r");
             }
             toggle = TRUE;
             main_message |= 0x1700;
             break;
          case 12:
             if (ConfigA & CFGA_100_OHM_GND)
             {
               printf("\n\r    A Ground fault has been detected");
             } else
             {
               printf("\n\r    A Ground Bolt fault has been detected");
             }
             toggle = TRUE;
             main_message |= 0x0F00;
             break;
          case 13:
             printf("\n\r     The HOT WIRED BYPASS key is removed");
             main_message |= 0x4000;
             ack_message |= 0x0003;
             break;
          case 14:
             printf("\n\r     We show a HOT WIRED BYPASS key");
             main_message |= 0x2000;
             ack_message |= 0x0003;
             break;
          case 15:
             printf("\n\r\n\r    ");
             Print_Crnt_Time();
             printf("  MAIN Idle state");
             main_message = 0x1FF0;
             ack_message = 0x0FFC;
             print_once = 0x00FF;
             break;
          default:
             break;
          }
    }

    if ( (message_number>=16) && (message_number<32) &&
                        (ack_message & bit_mask) )
    {
      ack_message &= ~((unsigned int)1 << (message_number-16));
      switch (message_number)
        {
         case 16:
            printf("\n\r    The MAIN relay appears NORMAL");
            toggle = TRUE;
            main_message |= 0x0030;
            break;
         case 17:
            printf("\n\r    The BACKUP relay appears NORMAL");
            toggle = TRUE;
            main_message |= 0x00C0;
            break;
         case 18:
            printf("\n\r    The relays appear HARD WIRED");
            ack_message |= 0x0008;
            break;
         case 19:
            printf("\n\r    The relays appear operational\n\r");
            toggle = TRUE;
            main_message |= 0x00F0;
            ack_message |= 0x0004;
            break;
         case 20:
            printf("\n\r\n     ");
            Print_Crnt_Time();
            printf("  MAIN Acquire state\n\r");
            main_message = 0x0000;
            ack_message &= ~(0x000F);
            ack_message |= 0x0FE0;
            toggle = FALSE;
            break;
         case 21:
            printf("\r\033[K   Try 2 wire Thermistor");
            ack_message |= 0x00C0;
            break;
         case 22:
            printf("\r\033[K   Try 2 wire Optic     ");
            ack_message |= 0x00A0;
            break;
         case 23:
            printf("\r\033[K   Try 5 wire Optic     ");
            ack_message |= 0x0060;
            break;
         case 24:
            printf("\r\033[K   Going Active 2 wire Optic");
            main_message = 0x0EF0;
            ack_message = 0x0008;
            act_message = 0x200E;
            break;
         case 25:
            printf("\r\033[K   Going Active 5 wire Optic");
            main_message = 0x0EF0;
            ack_message = 0x0008;
            act_message = 0x100E;
            break;
         case 26:
            printf("\r\033[K   Going Active 2 wire Thermistor");
            main_message = 0x0EF0;
            ack_message = 0x0008;
            act_message = 0x400E;
            break;
         case 27:
            printf("\n\r     ");
            Print_Crnt_Time();
            printf("  MAIN Acquire -> Idle - No Truck Found          ");
            main_message = 0xFFF0;
            ack_message = 0x000F;
            act_message = 0x00F0;
            break;
         case 28:    //Added by Joe for debug purposes
            printf("\n\r  Pulse Detected, Delaying          ");
            break;
         case 29:
           break;
         case 30:
           break;
         case 31:
            break;
        default:
            break;
        }
    }

   if ( message_number == 47)
   {
     main_message = 0xFFF0;         /* 15 - 00 messages unmask 15 - 04 */
     ack_message = 0x080F;          /* 31 - 16 messages unmask 27 and 19 - 16 */
     act_message = 0x00F8;          /* 47 - 32 messages unmask 39 - 35 */
   }

   if ( (message_number>=32) && (message_number<48) )
   {
      if (act_message & bit_mask)
         {
         act_message &= ~((unsigned int)1 << (message_number-32));
         switch (message_number)
            {
            case 32:
                  printf("\n\r    MAIN Active - All Compartments are Dry\n\r");
                  break;
            case 33:
                  break;
            case 34:
                  printf("\n\r\n\r    Switching Bias to Thermistor probe Value\n\r\n\r");
                  break;
            case 35:
                  printf("\n\r\n\r     ");
                  Print_Crnt_Time();
                  printf("  MAIN Active -> Idle - Truck Gone");
                  main_message = 0xFFF0;
                  ack_message = 0x000F;
                  act_message = 0x00F0;
                  break;
            case 36:
               printf("\n\r    Always shorted channels (hex 8-1) 0x%02X\n\r", (unsigned char)parameter1);
               act_message |= 0x0080;  /* Allow pass message to come back in */
               break;
            case 37:
               printf("\n\r    Always unpowered channels (hex 8-1) 0x%02X\n\r", (unsigned char)parameter1);
               act_message |= 0x0080;  /* Allow pass message to come back in */
               break;
            case 38:
               printf("\n\r    Always powered channels (hex 8-1) 0x%02X\n\r", (unsigned char)parameter1);
               act_message |= 0x0080;  /* Allow pass message to come back in */
               break;
            case 39:
               printf("\n\r    All channels appear normal\n\r");
               act_message |= 0x0070;  /* Put error messages 36, 37, 38 back in */
               break;
            case 40:
               printf("\n\r    Probes shorted to probes/ground (hex 8-1) 0x%02X\n\r", (unsigned char)parameter1);
               act_message |= 0x0200;
               break;
            case 41:
               printf("\n\r    Probes shorted to ground (hex 8-1) 0x%02X\n\r", (unsigned char)parameter1);
               act_message |= 0x0200;
               break;
            case 42:
               printf("\n\r    Probes are now normal\n\r");
               act_message |= 0x0100;
               break;
            case 43:
               break;
         case 44:
            printf("\n\r\n\r     ");
            Print_Crnt_Time();
            printf("  MAIN Active state 5 wire optic\n\r");
            ack_message = 0x0000;
            act_message = 0x010F;
            break;
         case 45:
            printf("\n\r\n\r     ");
            Print_Crnt_Time();
            printf("  MAIN Active state 2 wire optic\n\r");
            ack_message = 0x0000;
            act_message = 0x010F;
            break;
         case 46:
            printf("\n\r\n\r     ");
            Print_Crnt_Time();
            printf("  MAIN Active state 2 wire thermistor\n\r");
            ack_message = 0x0000;
            act_message = 0x010F;
            break;
         case 47:
            break;
            default:
               break;
            }
         }
   }
   if (((message_number>=48) && (message_number<100))
         || ((message_number >= 156) && (message_number < 180)))
    {
      switch (message_number)
         {
         case 48:
            printf("\n\r The Bypass time has EXPIRED...\n\r ");
            printf("\n\r Please Remove the Truck...\n\r");
            break;
         case 49:
            printf("\n\r  *** ADC FAILED to do a conversion with in 1ms");
            break;
         case 50:
            printf("\n\r  *** We have a failure of truck_state going Active");
            break;
         case 51:
            printf("\n\r  *** We have a state failure of truck_state");
            break;
         case 52:
            printf("%02u/", parameter1);
            break;
         case 53:
            printf("%02u:", parameter1);
            break;
         case 54:
            printf("%02u ", parameter1);
            break;
         case 55:
            printf ("%02X", (unsigned char)parameter1);      /* Dallas SN */
            break;
         case 56:
            if ( parameter1 < sizeof(state_string)/sizeof(state_string[0]))
            {
              printf("\n\r  Key Authourized, MAIN state %s", state_string[parameter1]);
              toggle = TRUE;
            }
            break;
         case 57:
            if ( parameter1 < sizeof(bypass_message)/sizeof(bypass_message[0]))
            {
              printf("\n\r  bypass level: %s", bypass_message[parameter1]);
            }
            break;
         case 58:
            printf("  S/N: ");
            break;
         case 59:
            if ( parameter1 < sizeof(clock_message)/sizeof(clock_message[0]))
            {
              printf("Clock status: %s", clock_message[parameter1]);
            }
            break;
         case 156:
            printf("\n\rBypass Key detected:");
            break;
         case 157:
            if ( parameter1 < sizeof(clock_message)/sizeof(clock_message[0]))
            {
              printf("\n\r  ?\? Can't read Dallas chip %s\n\r", clock_message[parameter1]);
            }
            break;
         case 158:
            printf("\n\r  ?\? Wrong Dallas family code 0x%02X     \n\r", parameter1);
            break;
         case 159:
            printf("\n\r  family code %u       \n\r", parameter1);
            break;
         case 60:
            printf("\n\r    Shell  CRC-16 Verified: %04X", parameter1);
            break;
         case 160:
            printf("\n\r    Kernel CRC-16 Verified: %04X", parameter1);
            break;
         case 61:
            printf("\n\r    Shell  CRC-16 Failure:  %04X", parameter1);
            break;
         case 161:
            printf("\n\r    Kernel CRC-16 Failure:  %04X", parameter1);
            break;
         case 62:
            printf("\n\r    The Bias Voltage %01u.%03uv is in error", parameter1/1000, parameter1%1000);
            break;
         case 63:
            printf("\n\r    The Bias Voltage %01u.%03uv is OK", parameter1/1000, parameter1%1000);
            break;
         case 162:
            printf("\n\r    The PVOLT5 Test Volt %01u.%03uv is in error", parameter1/1000, parameter1%1000);
            break;
         case 163:
            printf("\n\r    The PVOLT5 Test Volt %01u.%03uv is OK", parameter1/1000, parameter1%1000);
            break;
         case 64:
            printf("\n\r     10 volt channel noise \n\r   + ");
            break;
         case 65:
            printf("00000 00000  ");
            break;
         case 66:
            printf("%05u  ", parameter1 );
            break;
         case 67:
            printf("\n\r   - ");
            break;
         case 68:
            printf("\n\r    %u of the 10 volt Probe Noise Voltage(s) out of RANGE +/- 350mv", parameter1);
            break;
         case 69:
            printf("\n\r    The Probe Noise Voltage is OK ");
            break;
         case 70:
            printf("\n\r    %u Probe Voltage(s) out of RANGE 8/10 volt Tolerance of +/- 1 volt", parameter1);
            break;
         case 71:
            printf("\n\r    %u Probe Voltage(s) out of RANGE 20 volt Tolerance of +/- 2 volt", parameter1);
            break;
         case 72:
            printf("\n\r    The Probe Voltage Tolerance is OK ");
            break;
         case 73:
            printf("\n\r    The Raw 13 Voltage Tolerance is out of RANGE, %01u.%03uv", parameter1/1000, parameter1%1000);
            break;
         case 74:
            printf("\n\r    The Raw 13 Voltage %01u.%03uv is OK ", parameter1/1000, parameter1%1000);
            break;
         case 75:
            printf("\n\r    The 5 VOLT Optic Pulse is out of RANGE %01u.%03uv ", parameter1/1000, parameter1%1000);
            break;
         case 76:
            printf("\n\r    The Optic Pulse was stuck low %01u.%03u ", parameter1/1000, parameter1%1000);
            break;
         case 77:
            printf("\n\r    The 5 VOLT Optic Pulse is OK %01u.%03uv ", parameter1/1000, parameter1%1000);
            break;
         case 78:
            printf("\n\r    The Bias Voltage noise + %01u.%03uv, ", parameter1/1000, parameter1%1000);
            break;
         case 170:
            printf("\n\r\n\r    *** This Truck has %u compartments ***\n\r", number_of_Probes);
            break;
         case 178:
            printf("- %01u.%03u is in ERROR ", parameter1/1000, parameter1%1000);
            break;
         case 79:
            printf("\n\r    The Bias Voltage noise +%05u, ", parameter1);
            break;
         case 179:
            printf("- %01u.%03u is OK ", parameter1/1000, parameter1%1000);
            break;
         case 80:
            printf("\n\r    The EEPROM seems OK; Home block CRC verified; ");
            break;
         case 81:
            printf("all blocks valid ");
            break;
         case 82:
            if ( parameter1 < sizeof(eeb_message)/sizeof(eeb_message[0]))
            {
              printf("\n\r        EEPROM \"%s\" block marked invalid ",
                   eeb_message[parameter1]);
            }
            break;
         case 83:
            printf("\n\r    Problem with EEPROM: ");
            ee83sts(parameter1);
            break;
         case 84:
            printf("\n\r    Formatting EEPROM: ");
            break;
         case 85:
            printf("\n\rCan't read/write Home block: 0x%4.4X ",
                   parameter1);
            break;
         case 86:
            switch (parameter1)
              {
            case 0:
              printf("\n\rCan't detect 28C256/28C513 chip; ");
              break;
            case 32:
              printf("\n\r24FC1025 (128KB) detected; ");
              break;
            case 64:
              printf("\n\r28C513 (64KB) detected; ");
              break;
            default:
              printf("\n\rUnknown EEPROM chip; ");
              }
            break;
         case 87:
            printf("\n\rCan't write new Home block: 0x%4.4X ",
                   parameter1);
            break;
         case 88:
            if ( parameter1 < sizeof(eeb_message)/sizeof(eeb_message[0]))
            {
             printf("\n\r%s block invalid; ", eeb_message[parameter1]);
            }
            break;
         case 89:
            printf("\n\rformatted OK, Home block written ");
            break;
         case 90:
            printf("\n\rIWCOL write collision: 0x%04X",parameter1);
            break;
         case 91:
            printf("\n\rBCL write collision: 0x%04X",parameter1);
            break;
         case 92:
            printf("\n\rTransmit Buffer Full time out: 0x%04X",parameter1);
            break;
         case 93:
            printf("\n\rACK/NACK time out: 0x%04X",parameter1);
            break;
         case 94:
            printf("\n\rMaster Transmit time out: 0x%04X",parameter1);
            break;
         case 95:
            printf("\n\rStop condition time out: 0x%04X",parameter1);
            break;
         case 96:
            printf("\n\rReceive Data time out: 0x%04X",parameter1);
            break;
         case 97:
            printf("\n\rI2C %u Reset successful",parameter1);
            break;
         case 98:
            printf("\n\rI2C %u Reset failed",parameter1);
            break;
         case 99:
            printf("\n\rI2C %u",parameter1);
            break;

         default:
            break;
         }
    }

   if (((message_number >= 100) && (message_number < 110))
        || ((message_number >= 200) && (message_number < 207)))
   {
      switch (message_number)
      {
         case 100:
               printf(" Installed ");
               break;
         case 101:
               printf(" Open      ");
               break;
         case 102:
               printf("    TB3 CON  ");
               break;
         case 103:
               printf("    EN_VIP   ");
               break;
         case 104:
               printf("    EN_GND   ");
               break;
         case 105:
               printf("    EN_ADD   ");
               break;
         case 106:
               printf("    EN_ERS   ");
               break;
         case 107:
               printf("    EN_DMN   ");
               break;
         case 108:
               printf(" Enabled ");
               break;
         case 109:
               printf(" Disabled");
               break;
         case 200:
               printf("\n\r    8-CHANNEL");
               break;
         case 201:
               printf("\n\r    J21 - J23");
               break;
         case 202:
               printf("\n\r    GND BOLT ");
               break;
         default:
               break;
      }
   }

   if ( ((message_number>=110) && (message_number<120))
         || ((message_number >= 210) && (message_number<220)))
   {
      switch (message_number)
         {
         case 110:
            if (toggle)
               printf("\n\r    ");
            else
               printf("\r    ");
            toggle = FALSE;
            break;
         case 111:
 // QCCC  53         printf("+++   +++   ");
            printf("+++   "); /* QCCC 53 */
            break;
         case 112:
            printf("UNK  ");
            break;
         case 113:
            printf("WET  ");
            break;
         case 114:
            printf("Dry  ");
            break;
         case 115:
            printf("Open ");
            break;
         case 116:
            printf("Grnd ");
            break;
         case 117:
            printf("Cold ");
            break;
         case 118:
            printf("Warm ");
            break;
         case 210:
            printf(" badgndflag: %02X\n", parameter1);
            break;
         case 216:
            printf("Shrt ");
            break;
         case 217:
            printf("Flt  ");
            break;
         case 119:
            printf("\n\r Oscillation detected, Switching to Two wire Optic");
            break;
         default:
            break;
         }
   }
   if ( (message_number>=120) && (message_number<=135) )
   {
      switch (message_number)          /* POW printouts */
         {
         case 120:
            printf("\n\r    *** We have a TPURAM error");
            break;
         case 121:
            printf("\n\r    TPURAM tests out OK");
            break;
         case 122:
            printf("\n\r    ");
            report_clock();
            printf("     ");
            Print_Crnt_Time();
            printf("\n\r");
            break;
         case 123:
            printf("\n\r    Operating under UNchecksummed flash -> %04x", parameter1);
            break;
         case 124:
            printf("\n\r    The %04x Check Sum has been written to flash", parameter1);
            break;
         case 125:
            printf("\n\r    The %04x Check Sum failed flash write", parameter1);
            break;
         case 126:
            printf("\n\r    %u0 volt channels\n\r    ", parameter1);
            break;
         case 127:
            printf("%05u  ", parameter1);
            break;
         case 128:
            printf("\n\r");
            break;
         case 129:
            printf("\n\r\n\r    *** Either a truck is present, or the ADC has failed ***\n\r");
            break;
         case 130:
            printf(" gnd_retry: %d\n", (int)parameter1);
            // printf("\n\r    We are Emulating with RAM -> %04x", parameter1);
            break;
         case 131:
            printf("%c", 0x1B);     /* Lets make sure the printing is black */
            printf("[30m");
            printf("\n\r\n\r     ");
            Print_Crnt_Time ();
            printf("\n\r    Starting 5-Minute-Idle Self-Check/Diagnostics");
            break;
         case 132:
            printf("\n\r    Device 0x%02X on I2C bus ONE read error", parameter1);
            break;
         case 133:
            printf("\n\r    Device 0x%02X on I2C bus TWO read error", parameter1);
            break;
         case 134:
            printf("\n\r    Device 0x%02X on I2C bus ONE write error", parameter1);
            break;
         case 135:
            printf("\n\r    Device 0x%02X on I2C bus TWO write error", parameter1);
            break;
         default:
            break;
         }
   }
   if ( (message_number>=136) && (message_number<=145) )
   {
      bit_mask = (unsigned int)1 << (message_number-136);
      switch (message_number)          /* POW printouts */
      {
        case 136:
          if (print_once & bit_mask)
          {
            printf("\n\r     * Deadman switch fault (open or inactive): %umv\n\r", parameter1);
            print_once &= ~0x0001;
            print_once |= 0x0002;
          }
          break;
        case 137:
          if (print_once & bit_mask)
          {
            printf("\n\r     * Deadman switch closed: %umv\n\r", parameter1);
            print_once &= ~0x0002;
            print_once |= 0x0001;
          }
          break;
        case 140:
          if (print_once & bit_mask)
          {
            printf("\n\r     Backup Relay Open\n\r");
            print_once &= ~0x0010;
            print_once |= 0x0020;
          }
          break;
        case 141:
          if (print_once & bit_mask)
          {
            printf("\n\r     Backup Relay Closed\n\r");
            print_once &= ~0x0020;
            print_once |= 0x00010;
          }
          break;
        case 142:
          if (print_once & bit_mask)
          {
            printf("\n\r     Super TIM does not contain valid Builder Information\n\r");
            print_once &= ~0x0040;
          }
          break;
        case 143:
          printf("%c", 0x1B);
          printf("7");
          printf("%c", 0x1B);
          printf("[1;78H%c", (char)parameter1);
          printf("%c", 0x1B);
          printf("8");
          break;
        case 144:
          printf("\n\rError Reset will occur");
          printf(" StatusA: 0x%04X\n\r", StatusA);
          printf(" StatusB: 0x%04X\n\r", StatusB);
          printf(" iambroke: 0x%04X\n\r", iambroke);
          printf(" iamsuffering: 0x%04X\n\r", iamsuffering);
          printf(" ReferenceVol: 0x%04X\n\r", ReferenceVolt);
          printf(" badgndflag: 0x%04X\n\r", badgndflag);
          printf(" main_state: 0x%02X\n\r", main_state);
          printf(" truck_state: 0x%02X\n\r", truck_state);
          printf(" acquire_state: 0x%02X\n\r", acquire_state);
          printf(" probe_try_state: 0x%02X\n\r", probe_try_state);
          printf(" optic5_state: 0x%02X\n\r", optic5_state);
          printf(" two_wire_state: 0x%02X\n\r", two_wire_state);
          printf(" BackRelaySt: 0x%02X\n\r", BackRelaySt);
          printf(" MainRelaySt: 0x%02X\n\r", MainRelaySt);
          break;
        case 145:
           printf("\nTimeout trying to determine probe type\nDefault to Thermister probe\n");
           break;
        default:
          break;
      }
   }

} /* end of xprintf */


/********************** end of printout.c ***********************************/
