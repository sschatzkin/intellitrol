/**********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         MODBUS.C
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais  @Copyright 2009, 2014  Scully Signal Company
 *
 *   Description:    Main program Modbus routines for Rack controller main microprocessor
 *                   PIC24HJ256GP210
 *
 * Revision History:
 *   Rev      Date       Who   Description of Change Made
 * -------- ---------  ---      --------------------------------------------
 *              09/24/10  KLL  Modified to correct problem with lost messages
 *                                        - To prevent collisions at host, clear of TXEN moved to ISR.
 *                                       Code at case RECV should never execute
 *                                        - To accept new message sooner, the value of modbus_state
 *                                        changed in several places.
 *                                        - Supporting changes made in isr_uart.c
 *
 * 1.5.27   03/25/14  DHP  Deleted commented out code, corrected comments
 * 1.5.31  01/14/15  DHP  Added increment of bufptr in program_memory_CRC()
 *
 *********************************************************************************************/
#include "common.h"

/*************************************************************************
 *  subroutine:      get_modbus_addr()
 *
 *  function:
 *
 *
 *         1.  Entry is from either POD or main loop
 *         2.  This is used to display the address of the modbus
 *
 *  input:  none
 *  output: none
 *************************************************************************/
unsigned char get_modbus_addr(void)
{
unsigned volts;
unsigned addr = 0x01;     /* 0 for debug */
char sts;

    /* Read the two ModBus Address jumpers, which are taps into a pair of
       resistor-networks (voltage dividers), and convert into the two-digit
       ModBus Address. If we can't accurately determine the jumper, then
       default to "unused" address at top of address space (don't want to
       just use address "0" (ASCII debug output) as that would "jam" any
       working ModBus network to which we might be connected!) */

    sts = read_muxADC (1, M_ADDR, &volts); /* Read "Tens" address jumper */
    if (sts)
        {
        addr = 10 * volts_jumper (volts); /* "Convert to decimal" */
        sts = read_muxADC (0, M_ADDR, &volts); /* Read "Ones" address */
        if (sts)
            {
            addr += volts_jumper(volts); /* "Convert to decimal" */
            }
        else
            {
            addr = 254;                 /* Error, default to unused */
            }
        }
    else
        {
        addr = 254;                     /* Error, default to unused */
        }

    return ((unsigned char)addr);                      /* Return resultant ModBus Address */

}  /* end of get_modbus_addr() */

/*************************************************************************
 *  subroutine:      get_modbus_baud()
 *
 *  function:
 *
 *
 *         1.  Entry is from either POD or main loop
 *         2.  This is used to read the ModBus/Communications baud rate
 *
 *  input:  none
 *  output: none
 *************************************************************************/

unsigned char    get_modbus_baud(void)
{
    unsigned volts;
    unsigned baud;
    char sts;

    sts = read_muxADC (1, M_PARITY, &volts); /* Read "Baud Rate" jumper */
    if (!sts)
        {
        return(6);        /* Default 9600 */
        }

    baud = (unsigned int)volts_jumper (volts);        /* Convert to integer/raw-index */

    /* The ModBus/Comm parity/baud/size jumpers share a single 0-9 jumper
       block:

           0/1/2        None/Odd/Even parity
           3/4/5/6/7    1200/2400/4800/9600/19200 baud rate
           8/9          7/8 data bits

       For baud rate, just return the raw index into 10-word table */

    return ((unsigned char)baud);                      /* Return resultant baud index */

}  /* end of get_modbus_baud() */

/*************************************************************************
 *  subroutine:      get_modbus_parity()
 *
 *  function:
 *
 *
 *         1.  Entry is from either POD or main loop
 *         2.  This is used to read the ModBus/Communications Parity
 *
 *  input:  none
 *  output: none
 *************************************************************************/

unsigned char get_modbus_parity(void)
{
    unsigned volts;
    unsigned parity;
    char sts;

    sts = read_muxADC (0, M_PARITY, &volts); /* Read "Parity" jumper */
    if (!sts)
        {
        return 0;         /* No parity */
        }

    parity = (unsigned)volts_jumper (volts);      /* Convert to integer/index */

    /* The ModBus/Comm parity/baud/size jumpers share a single 0-9 jumper
       block:

           0/1/2        None/Odd/Even parity
           3/4/5/6/7    1200/2400/4800/9600/19200 baud rate
           8/9          7/8 data bits

       For parity, just return the raw index into 10-word table, limited to
       the first "few". */

    if (parity > 2)                     /* "Parity" reasonable? */
        parity = 0;                     /* No, override with default */

    return ((unsigned char)parity);                    /* Return resultant parity index */

}  /* end of get_modbus_parity() */

/*************************************************************************
 *  subroutine:      get_modbus_csize()
 *
 *  function:
 *
 *
 *         1.  Entry is from either POD or main loop
 *         2.  This is used to read the ModBus/Communications Char Size
 *
 *  input:  none
 *  output: none
 *************************************************************************/

unsigned char get_modbus_csize(void)
{
    unsigned volts;
    unsigned csize;
    char sts;

    sts = read_muxADC (1, M_GND_7_8, &volts); /* Read "7/8" jumper */
    if (!sts)
        {
        volts = 0;
        }

    csize = (unsigned)volts_jumper (volts);       /* Convert to integer/index */

    /* The ModBus/Comm parity/baud/size jumpers share a single 0-9 jumper
       block:

           0/1/2        None/Odd/Even parity
           3/4/5/6/7    1200/2400/4800/9600/19200 baud rate
           8/9          7/8 data bits

       For character size, force 8-bit characters. */

    if (csize != 9)                     /* Char size not "8-bit"? */
        csize = 9;                      /* Yes, override with "8-bit" */

    csize -= 8;                         /* 0 = 7-bit, 1 => 8-bit */
    return ((unsigned char)csize);                     /* Return resultant char size index */

}  /* end of get_modbus_csize() */


/*************************************************************************
            M O D B U S _ I N I T ( )
**************************************************************************/

static const char eomtimtbl[10] =
{
     0,                         /* 00  Reserved (no parity) */
     0,                         /* 01  Reserved (odd parity) */
     0,                         /* 02  Reserved (even parity) */
    29,                         /* 03   1200 baud */
    15,                         /* 04   2400 baud */
     7,                         /* 05   4800 baud */
     4,                         /* 06   9600 baud */
     3,                         /* 07  19200 baud */
//   2,                         /* 08  38400 baud */
     0,                         /* 08  Reserved (7-bit char) */
     0                          /* 09  Reserved (8-bit char) */
    };

void modbus_init(void)
{
   unsigned char * ptr;

   modbus_state   = (unsigned char)READY;

   modbus_err     = FALSE;

   modbus_rx_len  = 0;
   modbus_rx_ptr  = modbus_rx_buff;
   modbus_rx_time = mstimer;

   modbus_tx_len  = 0;
   modbus_tx_ptr  = modbus_tx_buff;

   modbus_eom_time = (unsigned short)((unsigned char)eomtimtbl[modbus_baud]);

   for (ptr = modbus_tx_buff; ptr < (modbus_tx_buff + MODBUS_MAX_LEN); ptr++)
      *ptr = 0;
   for (ptr = modbus_rx_buff; ptr < (modbus_rx_buff + MODBUS_MAX_LEN); ptr++)
      *ptr = 0;
}    /* End of modbus_init */

/**************************************************************************
    M O D B U S _ E X E C L O O P _ P R O C E S S ( )
**************************************************************************/

void modbus_execloop_process(void)
{
static unsigned short    delta_time;
unsigned short    crc_val;
unsigned short    crc_len;
unsigned char     *crc_ptr;
unsigned char     stat;
unsigned int i;

  // last_routine = 0x61;
    switch (modbus_state)
        {
      case READY:                       /* Looking for Idle line sync */
        /* Looking for 3.5 characters "idle" time window on ModBus to indi-
           cate that we're at the "end" of a message, and thus that the next
           byte we see is potentially the start of the next message, that
           might be addressed to us. */

        delta_time = DeltaMsTimer (modbus_rx_time);
  // last_routine = 0x61;

        if (delta_time > modbus_eom_time)
        {
            modbus_state = (unsigned char)RECV;
        }
        break;


      case RECV:                        /* Receiving chars (set in )*/
        if ( TXEN == 1)
        {
          IEC1bits.U2TXIE = 0;
          IFS1bits.U2TXIF = 0;
          while(U2STAbits.TRMT == 0)   /* Wait for transmit shift register to empty */
          { }
          DelayMS(1);
          clear_tx2en();
          TXEN = 0;                       /* Disable transmit */
  // last_routine = 0x61;
          flush_uart2();                  /* Remove any characters that were transmited */
          IFS1bits.U2RXIF = 0;
          IEC1bits.U2RXIE = 1;
        }

        if (modbus_rx_len == 0)         /* Actively receiving? */
        {                           /* No */
          modbus_rx_time = mstimer;   /* Re-Sync EOM timer */
          break;
        }

        send_char |= 0xA000;

        /* Actively receiving characters. If 3.5 character times have elapsed
           since last character, then we have EOM and need to process the
           just-received message */


        delta_time = DeltaMsTimer (modbus_rx_time);
  // last_routine = 0x61;

        if (delta_time > modbus_eom_time)
        {                           /* End-Of-Message! */
          send_char |= 0x0A00;
          if ((modbus_rx_buff[0] == modbus_addr)  /* Address to us? */
              || ((unsigned char)modbus_rx_buff[0] == 128))      /* Broadcast to us all? */
          {
            for ( i=0; i< sizeof(modbus_tx_buff); i++)
            {
              modbus_tx_buff[i] = 0;
            }
            crc_len = (char) (modbus_rx_len - 2);  /*DC005*/
            crc_val = modbus_CRC( modbus_rx_buff, crc_len, INIT_CRC_SEED);
  // last_routine = 0x61;

            crc_ptr = modbus_rx_ptr;

            if ((*--crc_ptr == (unsigned char)(crc_val >> 8))        /* CRC High Byte */
                  && (*--crc_ptr == (unsigned char)(crc_val & 0xFF)))  /* CRC Low Byte */
            {                   /* Good CRC. Process this message. */
              ledstate[TASCOMM] = PULSE; /* Indicate TAS/VIPER talkin' at us */
              {
                int index;
                save_last_recv_len = modbus_rx_len;
                for ( index=0; index<modbus_rx_len; index++)
                {
                  save_last_recv[index] = modbus_rx_buff[index];
                }

                save_last_recv[index] = 0x1A;  /* ^Z end of command */
              }
              stat = modbus_decode(modbus_rx_len,
                                         modbus_rx_buff,
                                         &modbus_tx_len,
                                         modbus_tx_buff);
              if (stat)
              {
                if (stat == MB_NO_RESPONSE)
                { /* No slave response/action is requested */
                  modbus_state = (unsigned char)RESETMSG;    /* Immediately transit back to receive */
                  break;
                }
                else
                {
                  modbus_tx_len = 3;
                  modbus_tx_buff[1] |= 0x80;  /* Set bit 7 to indicate exception */
                  modbus_tx_buff[2] = (unsigned char) stat;    /* And report what is the problem */
                }
              }
              if (modbus_tx_len > 0)
              {
                crc_val = modbus_CRC (modbus_tx_buff,
                                      modbus_tx_len,
                                      INIT_CRC_SEED);
                crc_ptr = (modbus_tx_buff + modbus_tx_len);
                *crc_ptr++ = (unsigned char)(crc_val & 0xFF);
                *crc_ptr   = (unsigned char)(crc_val >> 8);

                modbus_tx_len += 2;
                modbus_tx_ptr = modbus_tx_buff;

                delta_time = DeltaMsTimer (modbus_rx_time);
  // last_routine = 0x61;
                if (delta_time > SysParm.ModBusRespWait)
                {       /* OK to start response */
                  modbus_state = (unsigned char)XMITMSG;
                  set_tx2_en ();    /* Trigger xmit ISR */
                }
                else
                {       /* Must wait for line turnaround */
                  modbus_state = (unsigned char)XMITPREP; /* Wait first */
                  modbus_tx_time = modbus_rx_time;
                }
              }
              else
              {
                modbus_DecodeMsg_err++;
                modbus_state = (unsigned char)RESETMSG;
              }
            }
            else
            {                   /* CRC error, bad message, ignore */
              modbus_state = (unsigned char)RESETMSG;
            }
          }
          else
          {                       /* Not addressed to us */
            modbus_PrepMsg_err++;
            modbus_state = (unsigned char)RESETMSG;
          }
        }
        break;

      case XMITPREP:
      {
        int index;
        save_last_xmit_len = modbus_tx_len;
        for ( index=0; index<modbus_tx_len; index++)
        {
          save_last_xmit[index] = modbus_tx_buff[index];
        }
        save_last_xmit[index++] = 0x1A;  /* ^Z end of command */
        save_last_xmit[index] = 0x00;    /* 00 end of command */

        delta_time = DeltaMsTimer (modbus_tx_time);
  // last_routine = 0x61;
        if (delta_time > SysParm.ModBusRespWait)
        {
          send_char |= 0x50;
          modbus_state = (unsigned char)XMITMSG;
          set_tx2_en();
        }
      }
        break;

      case XMITMSG:
      {
        int index;
        for ( index=0; index<modbus_tx_len; index++)
        {
          while (U2STAbits.UTXBF == 1) ;    /* Wait for room for at least one character */
          U2TXREG = *(modbus_tx_ptr++);
        }
        while(U2STAbits.TRMT == 0) {} /* Wait for transmit shift register to empty */
        modbus_state = (unsigned char)RESETMSG;
        /* DHP  The following Block added to prevent host collisions */
        {
          U2STAbits.UTXEN = 0;            /* To force Int when set in set_tx2_en() */
          IEC1bits.U2TXIE = 0;
          IFS1bits.U2TXIF = 0;
          TXEN = 0;                       /* Disable transmit at board level */
          flush_uart2();                  /* Remove any received characters */
          IFS1bits.U2RXIF = 0;
          IEC1bits.U2RXIE = 1;            /* Now ready for receive */
        }
      }
        break;

      case RESETMSG:
          if ( U2STAbits.OERR == 1)
          {
            U2STAbits.OERR = 0;       /* Clear the Overflow bit */
          }

          modbus_init();
  // last_routine = 0x61;
        break;

      default:
        break;
        } /* End switch on modbus_state */

} /* End modbus_execloop_process() */


static const unsigned char hi_crc_table[] = {
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
   };

static const unsigned char lo_crc_table[] = {
   0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,
   0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
   0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
   0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
   0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
   0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
   0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,
   0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
   0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
   0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
   0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE,
   0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
   0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,
   0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
   0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
   0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
   0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62,
   0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
   0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,
   0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
   0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
   0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
   0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76,
   0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
   0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
   0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
   0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
   0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
   0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A,
   0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
   0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,
   0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
   };



unsigned short modbus_CRC( const unsigned char *bufptr,  /* Starting address */
                           unsigned short buflen,  /* CRC Segment length */
                           unsigned short seed) {  /* Initial CRC seed Value */

   unsigned char index;
   unsigned char crc_hi;
   unsigned char crc_lo;

  // last_routine = 0x62;
   crc_hi = (unsigned char) (seed >> 8);
   crc_lo = (unsigned char) (seed);

   while (buflen--) {
      index = (unsigned char) (crc_lo ^ *bufptr++);
      crc_lo = (unsigned char) (crc_hi ^ hi_crc_table[index]);
      crc_hi = lo_crc_table[index];
      }

   return((unsigned short) ((unsigned short)crc_hi << 8) | (unsigned short)crc_lo);

}    /* End of modbus_CRC */

unsigned short program_memory_CRC( unsigned long bufptr,   /* Starting address */
                                   unsigned short buflen, /* CRC Segment length */
                                   unsigned short seed)   /* Initial CRC seed Value */
{
uReg32 read_data;
unsigned char index;
unsigned char crc_hi;
unsigned char crc_lo;

  // last_routine = 0x62;
  crc_hi = (unsigned char) (seed >> 8);
  crc_lo = (unsigned char) (seed);

  while (buflen--)
  {
    (void)_memcpy_p2d24((char *)&read_data.Val32, bufptr, 1);
    bufptr++;
    index = (unsigned char) (crc_lo ^ read_data.Val[0]);
    crc_lo = (unsigned char) (crc_hi ^ hi_crc_table[index]);
    crc_hi = lo_crc_table[index];
  }

  return((unsigned short) ((unsigned short)crc_hi << 8) | (unsigned short)crc_lo);

}    /* End of modbus_CRC */


