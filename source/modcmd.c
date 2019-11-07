/****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   module:         modcmd.c
 *
 *   Revision:       REV 1.5.27
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *   Description:    Higher-level ModBus "Command" processing (Low-level
 *                   message/network code is in modbus.c).
 *
 * Revision History:
 *   Rev      Date       Who   Description of Change Made
 * -------- ---------  ---      --------------------------------------------
 *              04/22/08 KLL  Ported old Intellitrol modbus code from the
 *                                       MC68HC16Y1 cpu on the original Intellitrol
 * 1.5.23   12/09/11 KLL  Added fuel type read and write commands for
 *                                        the Super TIM
 * 1.5.27   03/25/14 DHP  Deleted commented out code, corrected comments.
 *                                       Added code in mbcRdTruckIDs() to reject zero a count
 *                                        command and read current truck with count other than 1.
 *
 ****************************************************************************/


#include "common.h"
#include "tim_utl.h"

extern const unsigned char DOW_CRC_tab[];

/****************************************************************************
    DATA (local)
****************************************************************************/

char modNVflag;                               /* nvSysParmUpdate() needed */

static unsigned char *getptr;            /* Pointer to next input byte to parse */
static unsigned char *putptr;            /* Poitner to next output byte to write */

static unsigned char  getcnt;             /* Current input buffer count (bytes left) */
static unsigned char  putcnt;             /* Current output buffer byte count */

static void mbcInitDSblock(DateStampNV *dsptr);          /* Pointer to "scratch" DateStamp block */
static MODBSTS mbcRdTrCompt (void);

/****************************************************************************
    Subroutines -- mbcGetXxx and mbcPutXxx ModBus message byte manipulation
****************************************************************************/

/*************************************************************************
* mbcGetByte -- Read and return next input command message byte
*
* Call is:
*
*       mbcGetByte (rval)
*
* Where:
*
*       <rval> is the pointer (byte *) to where mbcGetByte should return
*       the resultant value.
*
* mbcGetByte returns the next input message byte from the current ModBus
* input command buffer, or an error if no more bytes are available. It is
* assumed that modbus_decode() has initialized the getcnt/ptr pair with
* appropriate values.
*
* Return value is MB_OK (0) normally, or MB_EXC_FAULT (~0) if there are no
* more inputs bytes left to read. I.e., failure is indicated by a non-zero
* return value.
*
*************************************************************************/

static MODBSTS mbcGetByte
    (
    unsigned char *rval                          /* Pointer to return message byte */
    )
{
    if (getcnt-- > 0)
        {
        *rval = (unsigned char)(*getptr++);      /* Extract and return ModBus msg byte */
        return (MB_OK);
        }
    else
        {
        return (MB_EXC_FAULT);          /* Asking for non-ex byte */
        }

} /* End of mbcGetByte() */

/*************************************************************************
* mbcGetInt -- Read and return next input command message "Int"eger
*
* Call is:
*
*       mbcGetInt (rval)
*
* Where:
*
*       <rval> is the pointer (byte *) to where mbGetByte should return
*       the resultant value.
*
* mbcGetInt returns the next two input message bytes from the current ModBus
* input command buffer, formatted as a "NetworkByteOrder" (that's TCPese for
* "Big Endian" or high-order-byte-first) 16-bit integer.
*
* It is assumed that modbus_decode() has initialized the getcnt/ptr pair
* with appropriate values.
*
* Return value is MB_OK (0) normally, or MB_EXC_FAULT (~0) if there are no
* more inputs bytes left to read. I.e., failure is indicated by a non-zero
* return value.
*
*************************************************************************/

static MODBSTS mbcGetInt
    (
    unsigned int *rval          /* Pointer to return message byte */
    )
{
    MODBSTS sts;                        /* Local status */
    unsigned char hold1, hold2;                  /* int-building bytes */


    sts = mbcGetByte (&hold1);          /* Extract high-order byte */
    if (sts)
        return (sts);                   /* Something bad... */

    sts = mbcGetByte (&hold2);          /* Extract low-order byte */
    if (sts)
        return (sts);                   /* Died in middle of integer */

    *rval = (int)(((unsigned int)hold1 << 8) | (unsigned int)hold2);    /* Return resultant "int" */

    return (MB_OK);

} /* End of mbcGetByte() */

/*************************************************************************
* mbcPutByte -- Store next ModBus output/response byte for transmission
*
* Call is:
*
*       mbcPutByte (data)
*
* Where:
*
*       <data> is the new output byte.
*
* mbcPutByte() is the low-level (of the high-level message handling routines)
* byte-stuffer for building output/response ModBus messages. Call with the
* desired output byte.
*
* It is assumed that modbus_decode() has initialized the putcnt/ptr pair with
* appropriate values.
*
* Return value is MB_OK (0) normally, or MB_EXC_FAULT (~0) if there is no
* more room left in the output buffer (attempt to build/transmit a ModBus
* message larger than MODBUS_MAX_LEN-2). I.e., failure is indicated by a
* non-zero return value.
*
*************************************************************************/

static MODBSTS mbcPutByte
    (
    unsigned char datum               /* New output byte */
    )
{
    save_last_xmit[putcnt] = datum;
    save_last_xmit[putcnt+1] = 0x1A;  /* ^Z for end of response command */
    save_last_xmit_len = ++putcnt;    /* Save byte count */
    if (putcnt > MODBUS_MAX_LEN-2)
    {
      return (MB_EXC_FAULT);          /* No room left in output buffer */
    }
    else
    {
      *putptr++ = datum;              /* Stuff byte in output buffer */
      return (MB_OK);
    }
} /* End of mbcPutByte() */

/*************************************************************************
* mbcPutInt -- Format and output one 16-bit Integer value, network byte order
*
* Call is:
*
*       mbcPutInt (datum)
*
* Where:
*
*       <datum> is the 16-bit integer value to be byte formatted into the ModBus
*       output buffer.
*
* mbcPutInt() takes a 16-bit integer value, formats in in "NetworkByteOrder"
* (that's TCPese for "Big Endian" or high-order-byte-first), and stuffs the
* resultant data bytes into the ModBus output buffer.
*
* It is assumed that Modbus_decode() has initialized the putcnt/ptr pair
* with appropriate values.
*
* Return value is MB_OK (0) normally, or MB_EXC_FAULT (~0) if there is no
* more room in the output buffer (i.e., attempting to build/send a ModBus
* message larger than MODBUS_MAX_LEN-2 bytes).
*
*************************************************************************/

static MODBSTS mbcPutInt
    (
    unsigned int datum                           /* 16-bit value to be sent */
    )
{
    MODBSTS sts;                        /* Local status */

    sts = mbcPutByte ((unsigned char)(datum>>8)); /* Extract high-order byte */
    if (sts)
        return (sts);                   /* Something bad... */

    sts = mbcPutByte ((unsigned char)(datum));   /* Extract low-order byte */
    if (sts)
        return (sts);                   /* Died in middle of integer */

    return (MB_OK);

} /* End of mbcPutByte() */

/*************************************************************************
* mbcPutNString -- Output counted "string" to ModBus buffer
*
* Call is:
*
*       mbcPutNString (cnt, buf)
*
* Where:
*
*       <cnt> is the count of bytes in the string;
*
*       <buf> is the pointer to the "string" of bytes to be sent.
*
* mbcPutNString() copies a counted string (consecutive bytes in memory)
* into the current ModBus output buffer. No "formatting" of the string
* is performed, it is just copied verbatim.
*
* It is assumed that Modbus_decode() has initialized the putcnt/ptr pair
* with appropriate values.
*
* Return value is MB_OK (0) normally, or MB_EXC_FAULT (~0) if there is no
* more room in the output buffer (i.e., attempting to build/send a ModBus
* message larger than MODBUS_MAX_LEN-2 bytes).
*
*************************************************************************/

static MODBSTS mbcPutNString
    (
    char  cnt,                          /* Count of bytes in string */
    const unsigned char *ptr                           /* Pointer to string of bytes */
    )
{
    MODBSTS sts;                        /* Local status */

    while (cnt--)
        {
        sts = mbcPutByte (*ptr++); /* Extract next "string" byte */
        if (sts)
            return (sts);               /* Something bad... */
        }

    return (MB_OK);

} /* End of mbcPutNString() */

/*************************************************************************
* mbcInitDSblock -- Initialize scratch DateStamp block for editing
*
* Call is:
*
*       mbcInitDSblock (dsptr)
*
* Where:
*
*       <dsptr> is the pointer to a DateStamp block
*
* mbcInitDSblock initializes a [presumably] scratch DateStamp block with
* the current active DateStamp parameters, if any, or 0's if none.
*************************************************************************/

static void mbcInitDSblock
    (
    DateStampNV *dsptr          /* Pointer to "scratch" DateStamp block */
    )
{
    if (pDateStamp)
        memcpy ((char *)dsptr, (char *)pDateStamp, sizeof(DateStampNV));
    else
        memset ((char *)dsptr, 0x00, sizeof(DateStampNV));

} /* End mbcInitDSblock() */

/****************************************************************************
    Subroutines --  message parsing and action routines
****************************************************************************/

/****************************************************************************

All ModBus command-processing routines are responsible for verifying both the
syntax and semantics of the command (the CRC has been verified already).

All ModBus command-processing routines are responsible for generating some
sort of reply as well. This reply must be "immediate" (in order to "free up"
the ModBus (master control computer) to do other messages to other units).
For those messages that require extended time for processing (such as any-
thing doing with writing/erasing Flash/EEPROM), the immediate response should
be a simple "ACK" acknowledging the correct receipt of the command; the actual
extended command processing itself must be done "externally" to the
ModBus processing (via complicated state machines, etc.).

Upon return from any of these mbc* command-processing routines, the response
("Transmit") message buffer is assumed setup and will be sent auto-
magically to the ModBus master (unless it was a broadcast message, but we
don't concern ourselves here with such trivia), based on the return status
code. If "Success", the formatted message is sent; if any sort of error in
the message was detected ("unimplemented", "illegal argument", etc.), then
the return status reflects the exception condition and an Exception Response
message is transmitted instead.

*****************************************************************************/

/*************************************************************************
* mbcRdOStatus  --  Function 0x01: Read Output Status Bits
*
* Call is:
*
*      mbcRdOStatus ()
*
* mbcRdOStatus() reads and returns the unit "Output Status" bit array.
*
* NOTE:
*
*   As far as the Intellitrol is concerned, there is no "Output Status"
*   per se; rather this routine returns the "Status O/P" registers. This
*   routine exists solely to maintain some compatibility with VIP-era
*   control programs (e.g., VIPER I). As such, you can only read modulo
*   full 16-bit "registers" (e.g., requesting bits "7-19" will result in
*   an "Illegal Data Address" Exception response).
*
* Return value is the ModBus Exception code ("Success" meaning that the
* olen/orsp buffer is filled in and ready to be transmitted to the ModBus
* master).
*
*************************************************************************/

static MODBSTS mbcRdOStatus ()
{
    unsigned int bfirst;        /* First bit of status desired */
    unsigned int bcount;        /* Count of bits desired */
    char barray[4];             /* Transmuted StatusO/P */
    MODBSTS sts;                /* Message status */

    sts = mbcGetInt (&bfirst);          /* Extract starting bit position */
    if (sts)
        return (sts);                   /* Error */

    sts = mbcGetInt (&bcount);          /* Extract count of bits */
    if (sts)
        return (sts);                   /* Error */

    if (getcnt)                         /* Anything left? */
        {
        return (MB_EXC_ILL_FUNC);       /* Too many message bytes */
        }

    /* Verify that this is an "address"/count that we handle. Only legal
       cases are reading byte 0 (8@0), word 0 (16@0), word 1 (16@16),
       and words 0&1 (32@0). */

    if ((bfirst != 0) && (bfirst != 16))
        return (MB_EXC_ILL_ADDR);       /* Reject not byte 0/word 0/word 1 */

    if ((bcount != 8) && (bcount != 16) && (bcount != 32))
    {
        return (MB_EXC_ILL_DATA);       /* Reject not 1/2/4 bytes */
    }

    bfirst >>= 3;                       /* Modulo StatusA/B byte number */
    bcount >>= 3;                       /* Dealing in bytes from now on */

    if ((bfirst + bcount) > 4)
        return (MB_EXC_ILL_DATA);       /* Too many bytes/off end */

    sts = mbcPutByte ((unsigned char)bcount);          /* Count of bytes we're gonna send */
    if (sts)
        return (sts);                   /* Error??? */

    barray[0] = (char)(StatusO);        /* Byte */
    barray[1] = (char)(StatusO >> 8);   /*  Swap */
    barray[2] = (char)(StatusO);        /*   Stupid */
    barray[3] = (char)(StatusO >> 8);   /*    OSBits */

    sts = mbcPutNString ((char)bcount, (const unsigned char *)&barray[bfirst]);
    if (sts)
        return (sts);

    return (MB_OK);                     /* Success */

} /* End of mbcRdOStatus() */

/*************************************************************************
* mbcRdIStatus  --  Function 0x02: Read Input Status Bits
*
* Call is:
*
*      mbcRdIStatus ()
*
* mbcRdIStatus() reads and returns the unit "Input Status" bit array.
*
* NOTE:
*
*   As far as the Intellitrol is concerned, there is no "Input Status"
*   per se; rather this routine returns the "Status A/B" registers. This
*   routine exists solely to maintain some compatibility with VIP-era
*   control programs (e.g., VIPER I). As such, you can only read modulo
*   full 16-bit "registers" (e.g., requesting bits "7-19" will result in
*   an "Illegal Data Address" Exception response).
*
* Return value is the ModBus Exception code ("Success" meaning that the
* olen/orsp buffer is filled in and ready to be transmitted to the ModBus
* master).
*
*************************************************************************/

static MODBSTS mbcRdIStatus (void)
{
    unsigned int bfirst;        /* First bit of status desired */
    unsigned int bcount;        /* Count of bits desired */
    char barray[4];             /* Transmuted StatusA/B */
    MODBSTS sts;                /* Message status */

    sts = mbcGetInt (&bfirst);          /* Extract starting bit position */
    if (sts)
        return (sts);                   /* Error */

    sts = mbcGetInt (&bcount);          /* Extract count of bits */
    if (sts)
        return (sts);                   /* Error */

    if (getcnt)                         /* Anything left? */
        {
        return (MB_EXC_ILL_FUNC);       /* Too many message bytes */
        }

    /* Verify that this is an "address"/count that we handle. Only legal
       cases are reading byte 0 (8@0), word 0 (16@0), word 1 (16@16),
       and words 0&1 (32@0). */

    if ((bfirst != 0) && (bfirst != 16))
        return (MB_EXC_ILL_ADDR);       /* Reject not byte 0/word 0/word 1 */

    if ((bcount != 8) && (bcount != 16) && (bcount != 32))
        return (MB_EXC_ILL_DATA);       /* Reject not 1/2/4 bytes */

    bfirst >>= 3;                       /* Modulo StatusA/B byte number */
    bcount >>= 3;                       /* Dealing in bytes from now on */

    if ((bfirst + bcount) > 4)
        return (MB_EXC_ILL_DATA);       /* Too many bytes/off end */

    sts = mbcPutByte ((unsigned char)bcount);          /* Count of bytes we're gonna send */
    if (sts)
        return (sts);                   /* Error??? */

    barray[0] = (char)(StatusA);        /* Byte */
    barray[1] = (char)(StatusA >> 8);   /*  Swap */
    barray[2] = (char)(StatusB);        /*   Stupid */
    barray[3] = (char)(StatusB >> 8);   /*    ISBits */

    sts = mbcPutNString ((char)bcount, (const unsigned char *)&barray[bfirst]);
    if (sts)
        return (sts);

    return (MB_OK);                     /* Success */

} /* End of mbcRdIStatus() */

/*************************************************************************
* mbcRdRegs  --  Function 0x03: Read 16-bit Holding Registers
*
* Call is:
*
*      mbcRdRegs ()
*
* mbcRdRegs() reads and returns one or more "Registers" (which is to say,
* assorted 16-bit "datums") maintained and/or hallucinated (generated 
* dynamically) by the Intellitrol unit.
*
* Return value is the ModBus Exception code ("Success" meaning that the
* olen/orsp buffer is filled in and ready to be transmitted to the ModBus
* master).
*
*************************************************************************/

static MODBSTS mbcRdRegs (void)
{
    unsigned int rcnt;          /* Count of consecutive Registers to read */
    unsigned int rnum;          /* Register number to read */
    unsigned int rval;          /* Register contents or "value" */
    MODBSTS sts;                /* Status holding */

    /* First thing we see is the first "Holding Register" number to be
       read and returned */

    sts = mbcGetInt (&rnum);            /* Extract register number */
    if (sts)
        return (sts);                   /* Error ? */

    /* Second (and last!) item in the message is the count of consecutive
       "Registers" to be read as one operation */

    sts = mbcGetInt (&rcnt);            /* Extract count of registers */
    if (sts)
        return (sts);

    /* Message should now be empty */

    if (getcnt)
        return (MB_EXC_ILL_FUNC);       /* Malformed message */

    /* Make sure we can fit requested number of 16-bit registers into a
       response message buffer */

    if (rcnt > ((MODBUS_MAX_DATA - 1) / 2))
    {
      return (MB_EXC_ILL_DATA);       /* Malformed message */
    }
    /* Looks good, respond with "byte-count" response (twice count of registers)
       and "string" of 16-bit datums */

    sts = mbcPutByte ((unsigned char)(rcnt << 1));       /* Two bytes per "Register" */
    if (sts)
        return (sts);                   /* Error writing ? */

    for (; rcnt > 0; rcnt--, rnum++)
        {
        sts = mbrRdReg (rnum, &rval);   /* Read the n'th "Register" */
        if (sts)
            return (sts);

        sts = mbcPutInt (rval);         /* Write "Register" value */
        if (sts)                        /*  into response buffer */
            return (sts);
        }

    return (MB_OK);

} /* End of mbcRdRegs() */

/*************************************************************************
* mbcForceBit  --  Function 0x05: Force Bit (Action/Function)
*
* Call is:
*
*      mbcForceBit ()
*
* mbcForceBit() is used to "activate" or carry out a specific unit
* action. All defined actions take "a long time" to execute; this routine
* simply ACKs a good message and "queues" (or otherwise instigates) the
* desired action.
*
* Return value is the ModBus Exception code ("Acknowledge" meaning that
* the command has been accepted and will be acted upon imminently).
*
*************************************************************************/

static MODBSTS mbcForceBit (void)
{
    unsigned int fbit;          /* "Bit" (aka function) to "Force" */
    unsigned int fval;          /* "Bit" value */
    MODBSTS sts;                /* Status holding */

    /* Extract the "Bit" aka function code */

    sts = mbcGetInt (&fbit);            /* Extract function code */
    if (sts)
        return (sts);

    /* Second (and last!) item is the "value" to force into the "bit" */

    sts = mbcGetInt (&fval);            /* Extract function argument */
    if (sts)
        return (sts);

    /* Message should now be empty */

    if (getcnt)
        return (MB_EXC_ILL_FUNC);       /* Malformed message */

    /* Off to another source file */

    sts = mbxForce (fbit, fval);        /* Do the real work */
    if (sts)
        return (sts);                   /* Propagate error/exception */

    /* Whatever it was, it seems to have worked. "Echo" back the original
       function/value pair as successful completion of the command */

    sts = mbcPutInt (fbit);             /* Return bit/function code */
    if (sts)
        return (sts);

    sts = mbcPutInt (fval);             /* Echo back bit/function value */
    if (sts)
        return (sts);

    return (MB_OK);                     /* Return successfully */

} /* End of mbcForceBit() */

/*************************************************************************
* mbcWrOneReg  --  Function 0x06: Write Single 16-bit Holding Register
*
* Call is:
*
*      mbcWrOneReg ()
*
* mbcWrOneReg() writes ("pre-sets") one "Register" (which is to say,
* 16-bit "datum") maintained by the Intellitrol unit.
*
* Return value is the ModBus Exception code ("Success" meaning that the
* olen/orsp buffer is filled in and ready to be transmitted to the ModBus
* master).
*
*************************************************************************/

static MODBSTS mbcWrOneReg (void)
{
    unsigned int rnum;          /* Register number to write */
    unsigned int rval;          /* Register contents or "value" */
    MODBSTS sts;                /* Status holding */

    /* First thing we see is the "Holding Register" number to be written */

    sts = mbcGetInt (&rnum);            /* Extract register number */
    if (sts)
        return (sts);

    sts = mbcPutInt (rnum);             /* Echo back the register number */
    if (sts)
        return (sts);

    /* Second item in the message is the "Register" value */

    sts = mbcGetInt (&rval);            /* Extract new "Register" value */
    if (sts)
        return (sts);

    /* Message should now be empty */

    if (getcnt)
        return (MB_EXC_ILL_FUNC);       /* Malformed message */

    sts = mbrWrReg (rnum, &rval);       /* Write specified "Register" */
    if (sts)
        return (sts);

    /* We defer the "echo" of the value until after the mbrWrReg() call.
       This allows mbrWrREg() to "return" the "actual" value, should it in
       some way differ . . . dunno if this will ever be of use . . . */

    sts = mbcPutInt (rval);             /* Echo back the register contents */
    if (sts)
        return (sts);

    /* If we get to here, then we've successfully written the requested
       "Register".  Message should now be empty */

    if (getcnt)
        return (MB_EXC_ILL_FUNC);       /* Malformed message */

    return (MB_OK);                     /* Register successfully written */

} /* End of mbcWrOneReg() */

/*************************************************************************
* mbcWrRegs  --  Function 0x10: Write 16-bit Holding Registers
*
* Call is:
*
*      mbcWrRegs ()
*
* mbcWrRegs() writes ("pre-sets") one or more "Registers" (which is to say,
* assorted 16-bit "datums") maintained by the Intellitrol unit.
*
* Return value is the ModBus Exception code ("Success" meaning that the
* olen/orsp buffer is filled in and ready to be transmitted to the ModBus
* master).
*
*************************************************************************/

static MODBSTS mbcWrRegs (void)
{
    unsigned int rcnt;          /* Count of consecutive Registers to write */
    unsigned int rnum;          /* Register number to write */
    unsigned int rval;          /* Register contents or "value" */
    unsigned char rbyte;                 /* Stupid silly byte */
    MODBSTS sts;                /* Status holding */

    /* First thing we see is the first "Holding Register" number to be
       written. */

    sts = mbcGetInt (&rnum);            /* Extract register number */
    if (sts)
        return (sts);                   /* Error ? */

    sts = mbcPutInt (rnum);             /* Echo back the register number */
    if (sts)
        return (sts);

    /* Second item in the message is the count of consecutive "Registers"
       to be written as one operation. */

    sts = mbcGetInt (&rcnt);            /* Extract count of registers */
    if (sts)
        return (sts);

    sts = mbcPutInt (rcnt);             /* Echo back the register count */
    if (sts)
        return (sts);

    /* Make sure the requested number of registers fits into the input
       message buffer. */

    if (rcnt > ((MODBUS_MAX_DATA - 1) / 2))
        return (MB_EXC_ILL_DATA);       /* Malformed message */

    /* Third thing in message is byte count of following registers. This
       is always 2 * rcnt above, which is pretty gosh darned stupid, IMO. */

    sts = mbcGetByte (&rbyte);          /* Extract stupid byte count */
    if (sts)
        return (sts);

    if ((rcnt << 1) != (int)(rbyte))
        return (MB_EXC_ILL_DATA);       /* Don't like this data format */

    if ((rcnt << 1) != getcnt)
        return (MB_EXC_ILL_DATA);       /* Don't like this data format */

    /* Loop through the rest of the message, writing the "16-bit holding
       registers" one at a time. */

    for (; rcnt > 0; rcnt--, rnum++)
        {
        sts = mbcGetInt (&rval);        /* Extract next register value */
        if (sts)
            return (sts);

        sts = mbrWrReg (rnum, &rval);   /* Write the n'th "Register" */
        if (sts)
            return (sts);
        }

    /* If we get to here, then we've successfully written all the requested
       "Registers". */

    return (MB_OK);                     /* All registers successfully written */

} /* End of mbcWrRegs() */

/*************************************************************************
* mbcWrOneTruckID  --  Function 0x41: Write Single Truck ID to EEPROM
*
* Call is:
*
*      mbcWrOneTruckID ()
*
* mbcWrOneTruckID() writes ("records") one Truck Identification code or
* Serial Number to the Intellitrol/VIP Authorization List (in EEPROM).
*
* Return value is the ModBus Exception code ("Acknowledge" meaning that
* the command has been accepted and will be acted upon imminently; and
* "Memory Error" meaning that the NonVolatile routines failed in some
* fashion).
*
* Note: This operation is an "atomic" operation and runs to full completion
*       taking (in theory) 10 ms worst case . . . (see eeWrite routines)
*
*************************************************************************/

static MODBSTS mbcWrOneTruckID (void)
{
  unsigned char *ptr;                  /* Scratch pointer */
  unsigned int index;         /* Index of Truck ID to write */
  MODBSTS sts;                /* Status holding */

  /* Extract the TIM index to overwrite */
  sts = mbcGetInt (&index);           /* Extract NV-store index */
  if (sts)
      return (sts);

  /* The Truck ID follows, as a 6-digit "string" */

  ptr = getptr;                       /* Input buffer address */

  /* Message should have "6" bytes (one 48-bit serial number) left */

  if (getcnt != BYTESERIAL)
      return (MB_EXC_ILL_FUNC);       /* Malformed message */

  /* Write the specified Truck ID into EEPROM at the specified index */

  sts = (MODBSTS)nvTrkPut (ptr, index);        /* Write EEPROM */
  if (sts)                            /* Errors? */
  {                               /* Yes */
    return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */
                                      /* E.g., index out of range, etc. */
  }

  val_state = 0;                      /* Re-Authorize active truck as needed */

  /* The EEPROM write seems to have worked. "Echo" back the original
     index/serial-number pair as successful completion of the command */

  sts = mbcPutInt (index);            /* Return index code */
  if (sts)
      return (sts);

  sts = mbcPutNString (BYTESERIAL, (unsigned char *)ptr); /* Return Truck ID */
  if (sts)
      return (sts);

  badvipflag &= ~BVF_DONE;  /* If truck connected have it look through the list again */
  return (MB_OK);                     /* Return successfully */

} /* End of mbcWrOneTruckID() */

/*************************************************************************
* mbcRdOneTruckID  --  Function 0x42: Read Single Truck ID from EEPROM
*
* Call is:
*
*      mbcRdOneTruckID ()
*
* mbcRdOneTruckID() reads one Truck Identification code or Serial Number
* from the Intellitrol/VIP Authorization List (in EEPROM).
*
* Return value is the ModBus Exception code ("Success" meaning that the
* olen/orsp buffer is filled in and ready to be transmitted to the ModBus
* master).
*
*************************************************************************/

static MODBSTS mbcRdOneTruckID (void)
{
    unsigned char *ptr;                  /* Scratch pointer */
    unsigned int index;         /* Truck ID "index" */
    MODBSTS sts;                /* Local status */

    sts = mbcGetInt (&index);           /* Extract Truck ID index */
    if (sts)
        return (sts);                   /* Propagate error */

    /* Message should now be empty */

    if (getcnt)
        return (MB_EXC_ILL_FUNC);       /* Malformed message */

    sts = mbcPutInt (index);            /* "Echo" back truck ID index */
    if (sts)
        return (sts);

    ptr = putptr;                       /* Save current output pointer */

    sts = mbcPutNString (BYTESERIAL,    /* "Reserve" room in output message */
                         (unsigned char *)&truck_SN[0]);  /* Whatever's there... */
    if (sts)
        return (sts);

/* HACK */
    if (index == 0xFFFF)                /* Asking for "current truck" ? */
    {                               /* Yes */
      return (MB_OK);                 /* How convenient... */
    }
/* HACK Off */

    sts = (MODBSTS)nvTrkGet (ptr, index);        /* Retrieve Truck ID from NonVolatile */
    if (sts)                            /* Errors? */
    {                               /* Yes */
      return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */
                                        /* E.g., index out of range, etc. */
    }

    return (MB_OK);

} /* End of mbcRdOneTruckID() */

/*************************************************************************
* mbcRdOneTruckID  --  Function 0x42: Read Single Truck ID from EEPROM
*
* Call is:
*
*      mbcRdOneTruckID ()
*
* mbcRdOneTruckID() reads one Truck Identification code or Serial Number
* from the Intellitrol/VIP Authorization List (in EEPROM).
*
* Return value is the ModBus Exception code ("Success" meaning that the
* olen/orsp buffer is filled in and ready to be transmitted to the ModBus
* master).
*
*************************************************************************/

static MODBSTS mbcInsertTruckID (void)
{
  unsigned char *ptr;                  /* Scratch pointer */
  unsigned int index;         /* Index of Truck ID to write */
  MODBSTS sts;                /* Status holding */


  /* The Truck ID follows, as a 6-digit "string" */

  ptr = getptr;                       /* Input buffer address */

  /* Message should have "6" bytes (one 48-bit serial number) left */

  if (getcnt != BYTESERIAL)
      return (MB_EXC_ILL_FUNC);       /* Malformed message */

  sts = (MODBSTS)nvTrkFind (ptr,&index);        /* Retrieve Truck ID from NonVolatile */
  if (sts == 0)                            /* already exists */
  {                               /* Yes */
      sts = mbcPutInt (index); /* Return index */
      if (sts)
        return (sts);


      sts = mbcPutNString (BYTESERIAL, (unsigned char *)ptr); /* Return Truck ID */
      if (sts)
        return (sts);

    return (MB_OK);    /* Needs fleshing out */
                                        /* E.g., index out of range, etc. */
  }
   
  
  sts = (MODBSTS)nvTrkEmpty (&index);        /* Retrieve Truck ID from NonVolatile */
  if (sts)                            /* Errors? */
  {                               /* Yes */
    return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */
                                        /* E.g., index out of range, etc. */
  }
  
  /* Write the specified Truck ID into EEPROM at the empty index */

  sts = (MODBSTS)nvTrkPut (ptr, index);        /* Write EEPROM */
  if (sts)                            /* Errors? */
  {                               /* Yes */
    return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */                                      /* E.g., index out of range, etc. */
  }

  val_state = 0;                      /* Re-Authorize active truck as needed */

  /* The EEPROM write seems to have worked. "Echo" back the original
     index/serial-number pair as successful completion of the command */

  sts = mbcPutInt (index); /* Return index */
  if (sts)
      return (sts);


  sts = mbcPutNString (BYTESERIAL, (unsigned char *)ptr); /* Return Truck ID */
  if (sts)
      return (sts);

  badvipflag &= ~BVF_DONE;  /* If truck connected have it look through the list again */
  return (MB_OK);                     /* Return successfully */

} /* End of mbcInsertTruckID() */



static MODBSTS mbcRemoveTruckID (void)
{
  unsigned char *ptr;                  /* Scratch pointer */
  unsigned int index;         /* Index of Truck ID to write */
  MODBSTS sts;                /* Status holding */


  /* The Truck ID follows, as a 6-digit "string" */

  ptr = getptr;                       /* Input buffer address */

  /* Message should have "6" bytes (one 48-bit serial number) left */

  if (getcnt != BYTESERIAL)
      return (MB_EXC_ILL_FUNC);       /* Malformed message */

  sts = (MODBSTS)nvTrkFind (ptr,&index);        /* Retrieve Truck ID from NonVolatile */
  if (sts)                            /* Errors? */
  {          /* Yes */
    index = 0xFFFF;
    sts = mbcPutInt (index); /* Return index */
    if (sts)
      return (sts);

    return (MB_OK);    /* Needs fleshing out */
                                        /* E.g., index out of range, etc. */
  }
  
  /* Write the specified Truck ID into EEPROM at the empty index */

  sts = (MODBSTS)nvTrkDelete (index);        /* Write EEPROM */
  if (sts)                            /* Errors? */
  {                               /* Yes */
    return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */                                      /* E.g., index out of range, etc. */
  }

  val_state = 0;                      /* Re-Authorize active truck as needed */

  /* The EEPROM write seems to have worked.  */
  sts = mbcPutInt (index); /* Return index */
  if (sts)
      return (sts);



  badvipflag &= ~BVF_DONE;  /* If truck connected have it look through the list again */
  return (MB_OK);                     /* Return successfully */

} /* End of mbcInsertTruckID() */



/*************************************************************************
* mbcRdVIPLog  --  Function 0x43: Read Single Log Element
*
* Call is:
*
*      mbcRdVIPLog ()
*
* mbcRdVIPLog() reads one entry from the on-board "VIP" event log.
* This functionality is not supported on the Intellitrol (see mbcRdTrlLog)
*************************************************************************/

static MODBSTS mbcRdVIPLog (void)
{
    return (MB_EXC_ILL_FUNC);   /* Unimplemented (ergo illegal) function */

} /* End of mbcRdVIPLog() */



/*************************************************************************
* mbcWrPassword  --  Function 0x44: Write "DateStamp" Password
*
* Call is:
*
*      mbcWrPassword ()
*
* mbcWrPassword() writes the "dDateStamp" encryption password to the
* DateStamp block and updates the EEPROM copy.
*
* See also DateStamp "Company ID", and Terminal Number register.
*
* Return value is the ModBus Exception code ("Acknowledge" meaning that
* the command has been accepted and will be acted upon imminently).
*
*************************************************************************/

static MODBSTS mbcWrPassword (void)
{
DateStampNV dsblk;          /* Scratch DateStamp block */
unsigned char *ptr;                  /* Scratch pointer */
unsigned char cnt;                 /* Byte cnt for password */
unsigned char i;            /* Local loop cnter */
MODBSTS sts;                /* Status holding */

    mbcInitDSblock (&dsblk);    /* Fill in current DateStamp stuff */

    /* The "password" follows, as an n-byte non-null-terminated string */

    ptr = getptr;                       /* Input buffer address */

    cnt = *ptr;                       /* First byte is cnt...*/

    /* Message should have exactly one password, and that password should
       fit the DateStamp block */

    if ((cnt+1 != getcnt)             /* Right message size? */
        || (cnt > DS_PSWMAX-1))       /* Not too big? */
        return (MB_EXC_ILL_DATA);       /* Malformed message */

    /* Copy the password into the DateStamp block, pseudo-encrypting it on
       G.P.'s (copying junk in the buffer after the password helps obfuscate
       the name/password block in EEPROM! This is a feature, not a bug.)

       Note: This depends on a leading "0" ^'ing into a 0 for the first
             byte position! */

    for (i = 0; i < DS_PSWMAX; i++)
        dsblk.psw[i] = (char)((char)ptr[i] ^ (char)DOW_CRC_tab[i]);

    /* Write the new DateStamp block into current block */
    for ( i=0; i<DS_NAMMAX; i++ )
    {
      pDateStamp->psw[i] = dsblk.psw[i];
    }

    /* Write the new DateStamp block into EEPROM */

    sts = (MODBSTS)nvSysDSUpdate (&dsblk);       /* Write to EEPROM */
    if (sts)                            /* Errors? */
    {                               /* Yes */
      return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */
    }

    /* The EEPROM write seems to have worked. "Echo" back the password
       as successful completion of the command */

    sts = (MODBSTS)mbcPutNString ((char)cnt+1, (const unsigned char *)ptr); /* Return password */
    if (sts)
        return (sts);

    return (MB_OK);                     /* Return successfully */

} /* End of mbcWrPassword() */

/*************************************************************************
* mbcWrCompanyID  --  Function 0x45: Write Company ID
*
* Call is:
*
*      mbcWrCompanyID ()
*
* mbcWrCompanyID() writes the DateStamp company name into the DateStamp
* internal database for Truck authorization.
*
* The "name" field is 4 bytes of ASCII name. (The "extension" is forced
* to "D" [for "D"ate? "D"ata?] by the DateStamp verification code.)
*
* Return value is the ModBus Exception code ("Acknowledge" meaning that
* the command has been accepted and will be acted upon imminently).
*
*************************************************************************/

static MODBSTS mbcWrCompanyID (void)
{
DateStampNV dsblk;          /* Scratch DateStamp block */
unsigned char *ptr;                  /* Scratch pointer */
unsigned char cnt;                 /* Byte cnt for name */
unsigned char i;            /* Local loop cnter */
MODBSTS sts;                /* Status holding */

    mbcInitDSblock (&dsblk);    /* Fill in current DateStamp stuff */

    /* The "ID" (name) follows, as an n-byte non-null-terminated string */

    ptr = getptr;                       /* Input buffer address */

    cnt = getcnt;                     /* Size is what ever is left */

    if ((cnt < 1)                     /* Must have at least one character! */
        || (cnt > DS_NAMMAX))         /* Not too big? */
        return (MB_EXC_ILL_DATA);       /* Malformed message */

    /* Copy the ID/name into the DateStamp block, pseudo-encrypting it on
       G.P.'s.  We're a little more flexible perhaps than the spec intends
       on what we'll accept as input name -- spec says exactly 4 characters,
       (implied left-justified and blank-filled). */

    for (i = 0; i < cnt; i++)
        dsblk.name[i] = (char)((char)ptr[i] ^ (char)DOW_CRC_tab[i]);

    /* The TAS/VIPER system has nominally sent us a four-character left-
       justifed and blank-filled "name". The fifth character is the Dallas
       file "extension". We'll allow TAS/VIPER to pass less than 4 characters
       and blank-fill ourself, or even allow the fifth extension character
       to be specified...or we default it to "D". */

    switch (cnt)
    {
      case 1:                           /* Need 3 blanks and a "D" */
        dsblk.name[1] = (char)(' ' ^ DOW_CRC_tab[1]); /* Blank-fill */
        //lint -fallthrough
      case 2:                                         /* Need 2 blanks and a "D" */
        dsblk.name[2] = (char)(' ' ^ DOW_CRC_tab[2]); /*  to four */
        //lint -fallthrough
      case 3:                           /* Need 1 blank and a "D" */
        dsblk.name[3] = (char)(' ' ^ DOW_CRC_tab[3]); /*  char name */
        //lint -fallthrough
      case 4:                           /* Full name, need just a "D" */
        dsblk.name[4] = (char)('D' ^ DOW_CRC_tab[4]); /* "D" extension */
        //lint -fallthrough
      case 5:                           /* Don't really need this at all */
        dsblk.name[5] = (char)DOW_CRC_tab[5]; /* Null... */
        //lint -fallthrough
      default:
        break;
    }

    /* Write the new DateStamp block into current block */
    for ( i=0; i<DS_NAMMAX; i++ )
    {
      pDateStamp->name[i] = dsblk.name[i];
    }

    /* Write the new DateStamp block into EEPROM */
    sts = (MODBSTS)nvSysDSUpdate (&dsblk);       /* Write to EEPROM */
    if (sts)                            /* Errors? */
        {                               /* Yes */
        return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */
        }

    /* The EEPROM write seems to have worked. "Echo" back the name
       as successful completion of the command */

    sts = (MODBSTS)mbcPutNString ((char)cnt, (const unsigned char *)ptr);   /* Return name */
    if (sts)
        return (sts);

    return (MB_OK);                     /* Return successfully */

} /* End of mbcWrCompanyID() */

/*************************************************************************
* mbcWrTruckIDs  --  Function 0x46: Write one or more Truck IDs to EEPROM
*
* Call is:
*
*      mbcWrTruckIDs ()
*
* mbcWrTruckIDs() writes ("records") one or more Truck Identification
* codes or Serial Numbers to the Intellitrol/VIP Authorization List (in
* EEPROM).
*
* Return value is the ModBus Exception code ("Acknowledge" meaning that
* the command has been accepted and will be acted upon imminently; and
* "Memory Error" meaning that the NonVolatile routines failed in some
* fashion).
*
* Note: This operation is an "atomic" operation and runs to full completion
*       taking (in theory) 10 ms worst case . . . (see eeWrite routines)
*
*************************************************************************/

static MODBSTS mbcWrTruckIDs (void)
{
    unsigned char *ptr;                  /* Scratch pointer */
    unsigned int index;         /* Index of first Truck ID to write */
    unsigned int cnt;         /* Count of Truck IDs to write */
    MODBSTS sts;                /* Status holding */

    /* Extract the TIM index to overwrite */

    sts = mbcGetInt (&index);           /* Extract NV-store index */
    if (sts)
        return (sts);

    /* Extract the count of TIMs to write */

    sts = mbcGetInt (&cnt);           /* Get TIM count */
    if (sts)
        return (sts);

    /* Message should have "6 * n" bytes left */

    if ((cnt * BYTESERIAL) != getcnt)
        return (MB_EXC_ILL_FUNC);       /* Malformed message */

    /* The Truck ID(s) follows, as one or more 6-digit "string"s */

    ptr = getptr;                       /* Input buffer address */

    /* Write the specified Truck ID(s) into EEPROM at the specified index */

    sts = (MODBSTS)nvTrkPutMany (ptr, index, (unsigned char)cnt); /* Write EEPROM */
    if (sts)                            /* Errors? */
    {                                 /* Yes */
      return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */
                                      /* E.g., index out of range, etc. */
    }

    val_state = 0;                      /* Re-Authorize active truck as needed */

    /* The EEPROM write seems to have worked. "Echo" back the original
       index/cnt pair as successful completion of the command */

    sts = mbcPutInt (index);            /* Return index code */
    if (sts)
        return (sts);

    sts = mbcPutInt (cnt);            /* Return Truck ID cnt */
    if (sts)
        return (sts);

    badvipflag &= ~BVF_DONE;  /* If truck connected have it look through the list again */
    return (MB_OK);                     /* Return successfully */

} /* End of mbcWrTruckIDs() */

/*************************************************************************
* mbcRdTruckIDs  --  Function 0x47: Read Truck IDs from EEPROM
*
* Call is:
*
*      mbcRdTruckIDs ()
*
* mbcRdTruckIDs() reads one or more Truck Identification code or Serial
* Numbers from the Intellitrol/VIP Authorization List (in EEPROM).
*
* Return value is the ModBus Exception code ("Success" meaning that the
* olen/orsp buffer is filled in and ready to be transmitted to the ModBus
* master).
*
*************************************************************************/

static MODBSTS mbcRdTruckIDs (void)
{
    unsigned char *ptr;                  /* Scratch pointer */
    unsigned int index;         /* Truck ID "index" */
    unsigned int cnt;         /* Truck ID cnt */
    MODBSTS sts;                /* Local status */

    /* Handle Truck ID index */

    sts = mbcGetInt (&index);           /* Extract Truck ID index */
    if (sts)
        return (sts);                   /* Propagate error */

    sts = mbcPutInt (index);            /* "Echo" back truck ID index */
    if (sts)
        return (sts);

    /* Handle Truck ID cnt */

    sts = mbcGetInt(&cnt);            /* Extract cnt of Truck IDs */
    if (sts)
        return (sts);

    /* Message should now be empty */

    if (getcnt)
        return (MB_EXC_ILL_FUNC);       /* Malformed message */

    sts = mbcPutInt (cnt);            /* "Echo" back truck ID cnt */
    if (sts)
        return (sts);
    if (cnt==0)
        return (MB_EXC_ILL_DATA);       /* Count of zero not allowed */

    ptr = putptr;                       /* Save current output pointer */

    /* "Reserve" room in the response message for the Truck ID strings.
       The funny (word)((byte)*(byte)) casting is a trivial optimization.
       In the particularly special case of reading Truck ID 0xFFFF (the
       "Current Truck Register"), since we copied what was in truck_SN[0]
       (and whatever happened to be lying in memory after it), it works.
       If the TAS/VIPER asked for more than one cnt for 0xFFFF, then
       they deserve whatever they get... */

    sts = (MODBSTS)mbcPutNString ((char)(cnt * BYTESERIAL),
                         (unsigned char *)&truck_SN[0]);  /* Whatever's there... */
    if (sts)
        return (sts);

    if (index == 0xFFFF)                       /* Asking for "current truck" ? */
    {                                                      /* Yes */
      if (cnt!=1)
      {
        return (MB_EXC_ILL_DATA); /* Count of zero not allowed */
      }else
      { 
         return (MB_OK);                      /* Return what we have for S/N */
      }
    }                                
    /* Now extract the requested Truck IDs from their NonVolatile storage
       and dump them into the above-reserved response message "buffer". */

    sts = (MODBSTS)nvTrkGetMany (ptr, index, (unsigned char)cnt); /* Retrieve Truck ID(s) */
    if (sts)                            /* Errors? */
        {                               /* Yes */
        return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */
                                        /* E.g., index out of range, etc. */
        }

    return (MB_OK);                     /* If here, then success */

} /* End of mbcRdTruckIDs() */

/*************************************************************************
* mbcVrTruckIDs  --  Function 0x4A: Verify (CRC) Truck IDs from EEPROM
*
* Call is:
*
*      mbcVrTruckIDs ()
*
* mbcRdTruckIDs() "verifies" one or more Truck Identification code or Serial
* Numbers from the Intellitrol/VIP Authorization List (in EEPROM). This
* verification takes the form of calculating the (ModBus-style) CRC-16 of
* the "logical consecutive array of" Truck IDs.
*
* Return value is the ModBus Exception code ("Success" meaning that the
* olen/orsp buffer is filled in and ready to be transmitted to the ModBus
* master).
*
*************************************************************************/

static MODBSTS mbcVrTruckIDs (void)
{
    unsigned int index;         /* Truck ID "index" */
    unsigned int cnt;         /* Truck ID cnt */
    unsigned int crc;           /* Verification "CRC" code */
    MODBSTS sts;                /* Local status */

    /* Handle Truck ID index */

    sts = mbcGetInt (&index);           /* Extract Truck ID index */
    if (sts)
        return (sts);                   /* Propagate error */

    sts = mbcPutInt (index);            /* "Echo" back truck ID index */
    if (sts)
        return (sts);

    /* Handle Truck ID cnt */

    sts = mbcGetInt(&cnt);            /* Extract cnt of Truck IDs */
    if (sts)
        return (sts);

    /* Message should now be empty */

    if (getcnt)
        return (MB_EXC_ILL_FUNC);       /* Malformed message */

    sts = mbcPutInt (cnt);            /* "Echo" back truck ID cnt */
    if (sts)
        return (sts);

    /* This takes about 240 microseconds/TIM, call it 25 milliseconds per
       hundred-cnt. Since Wet/Dry detection wants to run on a 30ms cycle,
       severely restrict what we'll accept from TAS/VIPER! */

    if (cnt > 100)                    /* Arbitrarily limit "slice" size */
        return (MB_EXC_ILL_DATA);       /* Error */

    /* Verify (CRC) the "array" of TIMs */

    sts = (MODBSTS)nvTrkVrMany ((word *)&crc, index, cnt); /* Verify Truck ID(s) */
    if (sts)                            /* Errors? */
        {                               /* Yes */
        return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */
                                        /* E.g., index out of range, etc. */
        }

    sts = mbcPutInt (crc);              /* Return requested CRC code */
    if (sts)
        return (sts);

    return (MB_OK);                     /* If here, then success */

} /* End of mbcVrTruckIDs() */

/*************************************************************************
* mbcRdTrlLog  --  Function 0x49: Read Single Log Element
*
* Call is:
*
*      mbcRdTrlLog ()
*
* mbcRdTrlLog() reads one entry from the on-board internal Intellitrol
* Event Log.
*
* Return value is the ModBus Exception code ("Success" meaning that the
* olen/orsp buffer is filled in and ready to be transmitted to the ModBus
* master).
*
*************************************************************************/

static MODBSTS mbcRdTrlLog (void)
{
    unsigned char *ptr;         /* Scratch pointer */
    unsigned int index;         /* Intellitrol Log "index" */
    MODBSTS sts;                /* Local status */

    /* Extract Event Log index desired */

    sts = mbcGetInt (&index);           /* Extract Event Log index */
    if (sts)
    {
        return (sts);                   /* Propagate error */
    }
    sts = mbcPutInt (index);            /* "Echo" back the index */
    if (sts)
    {    
        return (sts);
    } 
    /* Message should now be empty */
    if (getcnt)
    {
        return (MB_EXC_ILL_FUNC);       /* Malformed message */
    } 
    if (index>= evMax)  
    {
        return (MB_EXC_ILL_ADDR);       /* Event Number out of range */
    } 
    sts = mbcPutInt (sizeof(E2LOGREC)); /* Return byte count following */
    if (sts)
        return (sts);

    ptr = putptr;                       /* Save current output pointer */

    /* "Reserve" room in the response message for the event log entry. */

    sts = mbcPutNString (sizeof(E2LOGREC),
                         (unsigned char *)&truck_SN[0]);  /* Whatever's there... */
    if (sts)
    {
        return (sts);
    }
    /* Now extract the requested Event Log element from NonVolatile storage
       and dump it into the above-reserved response message "buffer". */

    sts = (MODBSTS)nvLogGet (ptr, index);        /* Retrieve Event Log entry */
    if (sts)                            /* Errors? */
        {                               /* Yes */
        return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */
                                        /* E.g., index out of range, etc. */
        }

    evLastRead = index;                   /* Remember "read"/expired entries */

    return (MB_OK);                     /* If here, then success */

} /* End of mbcRdTrlLog() */

/*************************************************************************
* mbcWrKeys  --  Function 0x4B: Write one or more Bypass Keys to EEPROM
*
* Call is:
*
*      mbcWrKeys ()
*
* mbcWrKeys() writes ("records") one or more Bypass Key codes or Serial
* Numbers to the NonVolatile Bypass Key Authorization list.
*
* Return value is the ModBus Exception code ("Acknowledge" meaning that
* the command has been accepted and will be acted upon imminently; and
* "Memory Error" meaning that the NonVolatile routines failed in some
* fashion).
*
* Note: This operation is an "atomic" operation and runs to full completion
*       taking (in theory) 10 ms worst case . . . (see eeWrite routines)
*
*************************************************************************/

static MODBSTS mbcWrKeys (void)
{
    unsigned char *ptr;                  /* Scratch pointer */
    unsigned int index;         /* Index of first Bypass Key to write */
    unsigned int cnt;         /* cnt of Bypass Keys to write */
    MODBSTS sts;                /* Status holding */

    /* Extract the Key index to overwrite */

    sts = mbcGetInt (&index);           /* Extract NV-store index */
    if (sts)
        return (sts);

    /* Extract the count of Keys to write */

    sts = mbcGetInt (&cnt);           /* Get item cnt */
    if (sts)
        return (sts);

    if ((cnt * BYTESERIAL) != getcnt)
        return (MB_EXC_ILL_FUNC);       /* Malformed message */

    /* The Bypass Key(s) follows, as one or more 6-digit "string"s */

    ptr = getptr;                       /* Input buffer address */

    /* Write the specified Bypass Key(s) into EEPROM at the specified index */

    sts = (MODBSTS)nvKeyPutMany (ptr, index, (unsigned char)cnt); /* Write EEPROM */
    if (sts)                            /* Errors? */
    {                               /* Yes */
      return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */
                                        /* E.g., index out of range, etc. */
    }

    /* The EEPROM write seems to have worked. "Echo" back the original
       index/count pair as successful completion of the command */

    sts = mbcPutInt (index);            /* Return index code */
    if (sts)
        return (sts);

    sts = mbcPutInt (cnt);            /* Return Bypass Key cnt */
    if (sts)
        return (sts);

    return (MB_OK);                     /* Return successfully */

} /* End of mbcWrKeys() */

/*************************************************************************
* mbcRdKeys  --  Function 0x4C: Read Bypass Keys from EEPROM
*
* Call is:
*
*      mbcRdKeys ()
*
* mbcRdKeys() reads one or more Bypass Keys or Serial Numbers from the
* Intellitrol's Bypass Key Authorization List (in EEPROM).
*
* Return value is the ModBus Exception code ("Success" meaning that the
* olen/orsp buffer is filled in and ready to be transmitted to the ModBus
* master).
*
*************************************************************************/

static MODBSTS mbcRdKeys (void)
{
    unsigned char *ptr;                  /* Scratch pointer */
    unsigned int index;         /* Bypass Key "index" */
    unsigned int cnt;         /* Bypass Key cnt */
    MODBSTS sts;                /* Local status */

    /* Handle Bypass Key index */

    sts = mbcGetInt (&index);           /* Extract Bypass Key index */
    if (sts)
        return (sts);                   /* Propagate error */

    sts = mbcPutInt (index);            /* "Echo" back Bypass Key index */
    if (sts)
        return (sts);

    /* Handle Bypass Key cnt */

    sts = mbcGetInt(&cnt);            /* Extract cnt of Bypass Keys */
    if (sts)
        return (sts);

    /* Message should now be empty */

    if (getcnt)
        return (MB_EXC_ILL_FUNC);       /* Malformed message */

    sts = mbcPutInt (cnt);            /* "Echo" back Bypass Key cnt */
    if (sts)
        return (sts);

    ptr = putptr;                       /* Save current output pointer */

    /* "Reserve" room in the response message for the Bypass Key strings.
       The funny (word)((char)*(char)) casting is a trivial optimization. */

    sts = (MODBSTS)mbcPutNString ((char)(cnt * BYTESERIAL),
                         (unsigned char *)&truck_SN[0]);  /* Whatever's there... */
    if (sts)
        return (sts);

    /* Now extract the requested Bypass Keys from their NonVolatile storage
       and dump them into the above-reserved response message "buffer". */

    sts = (MODBSTS)nvKeyGetMany (ptr, index, (unsigned char)cnt); /* Retrieve Bypass Key(s) */
    if (sts)                            /* Errors? */
        {                               /* Yes */
        return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */
                                        /* E.g., index out of range, etc. */
        }

    return (MB_OK);                     /* If here, then success */

} /* End of mbcRdKeys() */

/*************************************************************************
* mbcWrEnaFeatures  --  Function 0x4D: Write "Features-Enable" Password
*
* Call is:
*
*      mbcWrEnaFeatures ()
*
* mbcWrEnaFeatures() decodes the "Features Password" string, validates it,
* and sets the new "features" mask, if valid.
*
* Return value is the ModBus Exception code
*
*************************************************************************/

static MODBSTS mbcWrEnaFeatures (void)
{
    unsigned char psw[8];                /* Decoding password block */
    unsigned char cnt;                 /* Byte cnt for password */
    unsigned char xx;                    /* Finagling byte */
    unsigned char i;                     /* Local loop cnter */
    MODBSTS sts;                /* Status holding */

    /* The "password" follows, as an n-byte non-null-terminated string */

    sts = mbcGetByte (&cnt);  /* Extract password byte cnt */
    if (sts)
        return (sts);
    /* Message should have exactly one password, and that password should
       fit the message */
    if ((cnt != getcnt)               /* Right message size? */
        || (cnt != 8))                /* Not too big? */
        return (MB_EXC_ILL_DATA);       /* Malformed message */
    for (i = 0; i < 8; i++)
    {
      sts = mbcGetByte (&xx);
      if (sts)
        return (sts);
      psw[i] = xx;
    }
#if 0 /* Needs verification; option updates via updater module not currently supported */
    if (!OPTBDPRES)            /* Jump is option board not present */
    {
      uReg32 ProgramMemory;
      unsigned long gen_ptr;
      unsigned char temp_byte;

      SPIMPolInit();          /* Initialize the SPI bus */
      SPI_EEPROMInit();       /* Initialize the SPI eeprom interface */

      ProgramMemory.Val32 = SPI_EEPROMReadDeviceID();  /* Fetch SPI memory ID */
      if ((ProgramMemory.Val[2] < 0x13) || (ProgramMemory.Val[2] > 0x14))
      {
        return (MB_SPI_FAMILY_ERR);      /* Needs fleshing out */
      }
      /**************************** 5/4/2009 7:59AM **************************
       * Store away the
       ***********************************************************************/
      gen_ptr = 0x40008L;
      for ( i=0; i<8; i++)
      {
        if (SPI_EEPROMWriteByte(gen_ptr++, psw[i]))
        {
          return (MB_SPI_WRITE_ERR);      /* Needs fleshing out */
        }
      }
      gen_ptr = 0x40008L;
      for ( i=0; i<8; i++)
      {
        if (SPI_EEPROMReadByte(gen_ptr++, (unsigned char *)&temp_byte, 1))
        {
          return (MB_SPI_READ_ERR);      /* Needs fleshing out */
        }
        if ( temp_byte != psw[i])
        {
          return (MB_EXC_MEM_PAR_ERR);      /* Needs fleshing out */
        }
      }
    }
    else
#endif
    {
      if ((sts = EnaFeatures(psw)) != MB_OK)
      {
        return (sts);
      }
      /* Write the new Enable-Features mask into EEPROM */
      sts = (MODBSTS)nvSysParmUpdate ();           /* Write to EEPROM */
      if (sts)                            /* Errors? */
      {                                   /* Yes */
        return (MB_EXC_MEM_PAR_ERR);      /* Needs fleshing out */
      }
     /* The EEPROM write seems to have worked. "Echo" back the password
       (well, just the count) as successful completion of the command */
      sts = mbcPutByte (cnt);           /* Positive acknowledgment */
      if (sts)
        return (sts);
    }
    return (MB_OK);                     /* Return successfully */
} /* End of mbcWrEnaFeatures() */

/*************************************************************************
* mbcRdEEBlock  --  Function 0x4E: Read special EEPROM block
*
* Call is:
*
*      mbcRdEEBlock ()
*
* mbcRdEEBlock() reads one of the System NonVolatile parameters blocks
* from EEPROM.
*
* This code actually reads the "current active" pointer values, so that
* if the EEPROM-resident copy is unreadable, what will be returned is
* the FLASHRAM-resident "default, static" copy.
*
* The return message is in a format suitable for "editing in place" and
* resending as a WRITE_EE_BLOCK message. In other words, TAS/VIPER can
* READ_EE_BLOCK, change a value in situ (directly in the message buffer
* for example), then WRITE_EE_BLOCK the message back to us. Note however
* that if the caller asks for a smaller block size than the actual block,
* then the subsequent write *will* fail.
*
* Return value is the ModBus Exception code ("Success" meaning that the
* olen/orsp buffer is filled in and ready to be transmitted to the ModBus
* master).
*
*************************************************************************/

static MODBSTS mbcRdEEBlock (void)
{
    unsigned char *ptr;                  /* Scratch pointer */
    unsigned char etype;                 /* E2SYS_* block type */
    unsigned char bcnt, bcntmax;         /* Byte count */
    MODBSTS sts;                /* Local status */

    /* Extract "special block type" code */

    sts = mbcGetByte (&etype);          /* Extract block type code */
    if (sts)
        return (sts);                   /* Propagate error */

    sts = mbcPutByte (etype);           /* "Echo" back block type code */
    if (sts)
        return (sts);

    /* Get byte count */

    sts = mbcGetByte((unsigned char *)&bcnt);            /* Extract data byte count */
    if (sts)
        return (sts);

    /* Message should now be empty */

    if (getcnt)
        return (MB_EXC_ILL_FUNC);       /* Malformed message */

    /* Determine which Sys block to retrieve */

    switch (etype)
        {
      case E2SYS_PARM:                  /* System general parameters */
        ptr = (unsigned char *)&SysParm;
        bcntmax = (char)sizeof(SysParmNV);
        break;

      case E2SYS_DSTAMP:                /* DateStamp parameters */
        ptr = (unsigned char *)pDateStamp;
        bcntmax = (char)sizeof(DateStampNV);
        break;

      case E2SYS_DIA5:                  /* 5-wire-optic diag/config levels */
        ptr = (unsigned char *)pSysDia5;
        bcntmax = (char)sizeof(SysDia5NV);
        break;

      case E2SYS_VOLT:                  /* System voltage minima/maxima/etc. */
        ptr = (unsigned char *)pSysVolt;
        bcntmax = (char)sizeof(SysVoltNV);
        break;

      case E2SYS_SET1:                  /* System config settings */
        ptr = (unsigned char *)pSysSet1;
        bcntmax = (char)sizeof(SysSet1NV);
        break;

      default:                          /* Unknown special block type */
        return (MB_EXC_ILL_ADDR);
        }

    bcntmax -= 2;                       /* Discount CRC-16 word */
    if (bcnt > bcntmax)                 /* Only read what is allowed... */
        bcnt = bcntmax;

    sts = mbcPutByte (bcnt);            /* "Echo" back returnable byte count */
    if (sts)
        return (sts);

    /* Extract and stuff requested data into response buffer */

    sts = (MODBSTS)mbcPutNString ((char)bcnt, ptr);    /* Whatever's there... */
    if (sts)
        return (sts);

    return (MB_OK);                     /* If here, then success */

} /* End of mbcRdEEBlock() */

/*************************************************************************
* mbcWrEEBlock  --  Function 0x4F: Write Special EEPROM block to EEPROM
*
* Call is:
*
*      mbcWrEEBlock ()
*
* mbcWrEEBlock() writes one of the special System NonVolatile parameters
* blocks directly to EEPROM. Caller is responsible for formatting the block
* with reasonable data...
*
* Return value is the ModBus Exception code.
*
* Note: This operation is an "atomic" operation and runs to full completion
*       taking (in theory) 10 ms worst case . . . (see eeWrite routines)
*
*************************************************************************/

static MODBSTS mbcWrEEBlock (void)
{
    unsigned char etype;                 /* E2SYS_* special block type */
    unsigned char bcnt;                  /* Byte count */
    MODBSTS sts;                /* Status holding */

    /* Extract "special block type" code */

    sts = mbcGetByte (&etype);          /* Extract block type code */
    if (sts)
        return (sts);                   /* Propagate error */

    /* Get byte count */

    sts = mbcGetByte((unsigned char *)&bcnt);            /* Extract data byte count */
    if (sts)
        return (sts);

    /* Message should now be just data bytes */

    if (bcnt != getcnt)
        return (MB_EXC_ILL_FUNC);       /* Malformed message */

    /* Write the specified parameter block into EEPROM, using the ModBus
       input buffer as an "in-situ" struct block (nvSysWrBlock will write
       a CRC-16 at the bcnt:bcnt+1 bytes -- i.e., at the end of the mes-
       sage buffer where the ModBus CRC currently sits). This saves us
       from having to dedicate yet another 64 bytes of RAM (or stack)
       space for "temporary holding" copies... */
    /* NOTE: at this time E2SYS_PARM - System general parameters and */
    /* E2SYS_DSTAMP - DateStamp parameters block codes will cause an error M.R. 06/06/97 */
    sts = (MODBSTS)nvSysWrBlock ((char)etype, bcnt, (char *)getptr); /* Write EEPROM */
    if (sts)                            /* Errors? */
        {                               /* Yes */
        return (MB_EXC_MEM_PAR_ERR);    /* Needs fleshing out */
                                        /* E.g., index out of range, etc. */
        }

    /* The EEPROM write seems to have worked. "Echo" back the original
       block type and data byte count as successful completion of the
       command */

    sts = mbcPutByte (etype);           /* Return block type */
    if (sts)
        return (sts);

    sts = mbcPutByte (bcnt);            /* Return data byte count */
    if (sts)
        return (sts);

    return (MB_OK);                     /* Return successfully */

} /* End of mbcWrEEBlock() */

/******************************* 6/22/2009 8:20AM ****************************
 * readTIMarea(unsigned int start, unsigned int end)
 * This routine will fetch a contents of a section of T.I.M. memory and send it
 * up the Modbus. start is the beginning of the memory area and end is the end
 * of the memory section. An error will be reported if the read location is
 * outside the memory area.
 *****************************************************************************/
static MODBSTS readTIMarea(unsigned int start, unsigned int end)
{
MODBSTS sts;                          /* Status holding */
unsigned int i, address, size;
unsigned char *mem_ptr;
unsigned char temp_area[75];

  mem_ptr = temp_area;
  sts = mbcGetInt (&address); /* Extract the section of memory the user wants */
  if (sts)
    return (sts);                     /* Propagate error */

  sts = mbcGetByte ((unsigned char *)&size); /* Extract the size of memory the user wants */
  if (sts)
    return (sts);                     /* Propagate error */

  sts = mbcPutByte ((unsigned char)size);           /* "Echo" back size */
  if (sts)
    return (sts);

  /****************************** 6/22/2009 9:33AM ***************************
   * Make sure the requested memory area falls within the proper area
   * if the size is greater than 70 bytes it is an error due to our modbus
   * length limitations
   ***************************************************************************/
  if ((address < start) || ((address+size) > end) || (size > 70))
  {
    return (MB_EXC_MEM_PAR_ERR);
  }

  /****************************** 6/22/2009 9:38AM ***************************
   * Now lets fetch the contents of the area
   ***************************************************************************/
  if ((sts = (MODBSTS)tim_block_read(mem_ptr, address, size)) != MB_OK)
  {
     return (sts);
  }

  for ( i=0; i<size; i++)
  {
      sts = mbcPutByte (mem_ptr[i]);           /* Send back the indication for invalid entry */
      if (sts)
      {
        return (MB_EXC_MEM_PAR_ERR);
      }
  }

  return (MB_OK);                     /* Return successfully */
}

/******************************* 6/15/2009 2:21PM ****************************
 * Write a section of the Super TIM memory
 *****************************************************************************/
static MODBSTS writeTIMarea(unsigned int start, unsigned int end)
{
MODBSTS sts;                          /* Status holding */
unsigned int i, address;
unsigned char size;
unsigned char *mem_ptr;
unsigned char temp_area[75];

  mem_ptr = temp_area;

  sts = mbcGetInt (&address); /* Extract the section of memory the user wants */
  if (sts)
    return (sts);                     /* Propagate error */

  sts = mbcPutInt (address);           /* "Echo" back address */
  if (sts)
    return (sts);

  sts = mbcGetByte (&size); /* Extract the section of memory the user wants */
  if (sts)
    return (sts);                     /* Propagate error */

  sts = mbcPutByte (size);           /* "Echo" back size */
  if (sts)
    return (sts);

  /****************************** 6/22/2009 9:33AM ***************************
   * Make sure the requested memory area falls within the proper area
   * Size greater than 70 bytes is an error due to our modbus
   * lenght limitations
   ***************************************************************************/
  if ((address < start) || ((address+size) > end) || (size > 70))
  {
    return (MB_EXC_TIM_MEM_AREA_ERR);
  }
  for ( i=0; i<size; i++)
  {
      mem_ptr[i] = 0;
  }

  /****************************** 6/22/2009 9:49AM ***************************
   * Put the write data into the modbus response buffer
   ***************************************************************************/
  for ( i=0; i<size; i++)
  {
      sts = mbcGetByte (&mem_ptr[i]); /* Fetch the byte to be written */
      if (sts)
      {
        return (MB_EXC_MEM_PAR_ERR);
      }
  }

  sts = (MODBSTS)tim_block_write(mem_ptr, address, size);

  return (sts);                     /* Return successfully */
}

/******************************* 6/22/2009 1:46PM ****************************
 * Fetch the TIM Serial Number
 * input 2 is Test Tim, 3 is Alternative TIM
 * If the vaild byte is not 0x33 them report 0x0B - TIM number is not valid
 *****************************************************************************/
static MODBSTS read_tim(unsigned char tim_type)
{
MODBSTS sts;                          /* Status holding */
unsigned int i;
unsigned char *mem_ptr;
unsigned char temp_area[8];

  mem_ptr = temp_area;
  if ((sts = (MODBSTS)fetch_serial_number(tim_type, mem_ptr)) != MB_OK)
  {
    return (sts);
  }

  sts = mbcPutByte (6);           /* Report string length always 6 */

  for ( i=0; i<6; i++)
  {
      sts = mbcPutByte (mem_ptr[i]);           /* Send back the indication for invalid entry */
      if (sts)
      {
        return (sts);
      }
  }
  return (sts);                     /* Return successfully */
}

/******************************* 6/22/2009 1:46PM ****************************
 * Write a new TIM Serial Number
 * Input 2 is Test Tim, 3 is Alternative TIM
 * If the number is 6 byte then the valid byte is set to 0x33
 *****************************************************************************/
#ifdef NO_DEF
static MODBSTS write_tim(unsigned char tim_type)
{
MODBSTS sts;                          /* Status holding */
unsigned int valid_address, tim_address, i;
unsigned char size;
unsigned char *mem_ptr;
unsigned char temp_area[75];

  mem_ptr = temp_area;

  switch ( tim_type)
  {
// SME figure out
//    case TEST_TIM :
//      valid_address = VALID_TRUCKTEST_TIM_ADR;
//      tim_address = TRUCK_TEST_TIM_NUMBER_ADR;
//      break;
    case ALT_TIM :
      valid_address = ALT_TIM_ID_VALID_SIZE;
      tim_address = ALT_TIM_ID_VALID_ADDR;
      break;
    default:
      return MB_EXC_TIM_CMD_ERR;
  }

  sts = mbcGetByte (&size); /* Extract the section of memory the user wants */
  if (sts)
    return (sts);                     /* Propagate error */

  sts = mbcPutByte (size);           /* "Echo" back compartment number */
  if (sts)
    return (sts);
  if ( size != 6)
  {
    return MB_EXC_TIM_MEM_AREA_ERR;
  }

  /****************************** 6/22/2009 1:34PM ***************************
   * Test if TIM serial number is valid
   ***************************************************************************/
  for ( i=0; i<size; i++)
  {
      sts = mbcGetByte (&mem_ptr[i]);           /* Send back the indication for invalid entry */
      if (sts)
      {
        return (sts);
      }
  }

  if ((sts = (MODBSTS)tim_block_write(mem_ptr, tim_address, size)) != MB_OK)
  {
    return (sts);
  }

  /****************************** 6/22/2009 1:58PM ***************************
   * Set this TIM serial address to valid
   ***************************************************************************/
  mem_ptr[0] = TIM_VALID;
  sts = (MODBSTS)tim_block_write(mem_ptr, valid_address, 1);

  return (sts);                     /* Return successfully */
}
#endif
/******************************* 6/15/2009 2:21PM ****************************
 * Fetch a section of the Super TIM memory and send it up the Modbus
 *****************************************************************************/
static MODBSTS mbcRdTrBuilderInfo(void)
{
unsigned char builder_section, size, cnt;
MODBSTS sts;                          /* Status holding */
unsigned int i, address;
int ascii_flag = FALSE;
unsigned char *mem_ptr;
unsigned char temp_area[80];

  mem_ptr = temp_area;

  sts = mbcGetByte (&builder_section); /* Extract the section of memory the user wants */
  if (sts)
    return (sts);                     /* Propagate error */

  sts = mbcPutByte (builder_section);           /* "Echo" back compartment number */
  if (sts)
    return (sts);

  switch ( builder_section)
  {
    case CARRIER_NAME_CASE :                  /* 0x01  */
      address = CARRIER_NAME_ADDR;
      size = CARRIER_NAME_SIZE;
      break;
    case CARRIER_ADDRESS_CASE :                  /* 0x02  */
      address = CARRIER_ADDRESS_ADDR;
      size = CARRIER_ADDRESS_SIZE;
      break;
    case CONTRACT_NUMBER_CASE :                  /* 0x03  */
      address = CONTRACT_NUMBER_ADDR;
      size = CONTRACT_NUMBER_SIZE;
      break;
    case OPPERATING_SERVICE_CASE :                  /* 0x04  */
      address = OPPERATING_SERVICE_ADDR;
      size = OPPERATING_SERVICE_SIZE;
      break;
    case DRIVER_ID_CASE :                  /* 0x05  */
      address = DRIVER_ID_ADDR;
      size = DRIVER_ID_SIZE;
      break;
    case ALLOWABLE_VOL_CPT1_CASE :                  /* 0x06  */
      address = ALLOWABLE_VOL_CPT1_ADDR;
      size = ALLOWABLE_VOL_CPT1_SIZE;
      break;
    case ALLOWABLE_VOL_CPT2_CASE :                  /* 0x07  */
      address = ALLOWABLE_VOL_CPT2_ADDR;
      size = ALLOWABLE_VOL_CPT2_SIZE;
      break;
    case ALLOWABLE_VOL_CPT3_CASE :                  /* 0x08  */
      address = ALLOWABLE_VOL_CPT3_ADDR;
      size = ALLOWABLE_VOL_CPT3_SIZE;
      break;
    case ALLOWABLE_VOL_CPT4_CASE :                  /* 0x09  */
      address = ALLOWABLE_VOL_CPT4_ADDR;
      size = ALLOWABLE_VOL_CPT4_SIZE;
      break;
    case ALLOWABLE_VOL_CPT5_CASE :                  /* 0x0A  */
      address = ALLOWABLE_VOL_CPT5_ADDR;
      size = ALLOWABLE_VOL_CPT5_SIZE;
      break;
    case ALLOWABLE_VOL_CPT6_CASE :                  /* 0x0B  */
      address = ALLOWABLE_VOL_CPT6_ADDR;
      size = ALLOWABLE_VOL_CPT6_SIZE;
      break;
    case ALLOWABLE_VOL_CPT7_CASE :                  /* 0x0C  */
      address = ALLOWABLE_VOL_CPT7_ADDR;
      size = ALLOWABLE_VOL_CPT7_SIZE;
      break;
    case ALLOWABLE_VOL_CPT8_CASE :                  /* 0x0D  */
      address = ALLOWABLE_VOL_CPT8_ADDR;
      size = ALLOWABLE_VOL_CPT8_SIZE;
      break;
    case ALLOWABLE_VOL_CPT9_CASE :                  /* 0x0E  */
      address = ALLOWABLE_VOL_CPT9_ADDR;
      size = ALLOWABLE_VOL_CPT9_SIZE;
      break;
    case ALLOWABLE_VOL_CPT10_CASE :                  /* 0x0F  */
      address = ALLOWABLE_VOL_CPT10_ADDR;
      size = ALLOWABLE_VOL_CPT10_SIZE;
      break;
    case ALLOWABLE_VOL_CPT11_CASE :                  /* 0x10  */
      address = ALLOWABLE_VOL_CPT11_ADDR;
      size = ALLOWABLE_VOL_CPT11_SIZE;
      break;
    case ALLOWABLE_VOL_CPT12_CASE :                  /* 0x11  */
      address = ALLOWABLE_VOL_CPT12_ADDR;
      size = ALLOWABLE_VOL_CPT12_SIZE;
      break;
    case ALLOWABLE_VOL_CPT13_CASE :                  /* 0x12  */
      address = ALLOWABLE_VOL_CPT13_ADDR;
      size = ALLOWABLE_VOL_CPT13_SIZE;
      break;
    case ALLOWABLE_VOL_CPT14_CASE :                  /* 0x13  */
      address = ALLOWABLE_VOL_CPT14_ADDR;
      size = ALLOWABLE_VOL_CPT14_SIZE;
      break;
    case ALLOWABLE_VOL_CPT15_CASE :                  /* 0x14  */
      address = ALLOWABLE_VOL_CPT15_ADDR;
      size = ALLOWABLE_VOL_CPT15_SIZE;
      break;
    case ALLOWABLE_VOL_CPT16_CASE :                  /* 0x15  */
      address = ALLOWABLE_VOL_CPT16_ADDR;
      size = ALLOWABLE_VOL_CPT16_SIZE;
      break;
    case VAP_TIGHT_CERT_TYPE_CASE :                  /* 0x16  */
      address = VAP_TIGHT_CERT_TYPE_ADDR;
      size = VAP_TIGHT_CERT_TYPE_SIZE;
      break;
    case VAP_TIGHT_CERT_DATE_CASE :                  /* 0x17  */
      address = VAP_TIGHT_CERT_DATE_ADDR;
      size = VAP_TIGHT_CERT_DATE_SIZE;
      break;
    case VAP_TIGHT_CERT_NUMBER_CASE :                  /* 0x18  */
      address = VAP_TIGHT_CERT_NUMBER_ADDR;
      size = VAP_TIGHT_CERT_NUMBER_SIZE;
      break;
    case SAFE_PASS_CERT_TYPE_CASE :                  /* 0x19  */
      address = SAFE_PASS_CERT_TYPE_ADDR;
      size = SAFE_PASS_CERT_TYPE_SIZE;
      break;
    case SAFE_PASS_CERT_DATE_CASE :                  /* 0x1A  */
      address = SAFE_PASS_CERT_DATE_ADDR;
      size = SAFE_PASS_CERT_DATE_SIZE;
      break;
    case SAFE_PASS_CERT_NUMBER_CASE :                  /* 0x1B  */
      address = SAFE_PASS_CERT_NUMBER_ADDR;
      size = SAFE_PASS_CERT_NUMBER_SIZE;
      break;
    case CERT_3_TYPE_CASE :                  /* 0x1C  */
      address = CERT_3_TYPE_ADDR;
      size = CERT_3_TYPE_SIZE;
      break;
    case CERT_3_DATE_CASE :                  /* 0x11D  */
      address = CERT_3_DATE_ADDR;
      size = CERT_3_DATE_SIZE;
      break;
    case CERT_3_NUMBER_CASE :                  
      address = CERT_3_NUMBER_ADDR;
      size = CERT_3_NUMBER_SIZE;
      break;
    case CERT_4_TYPE_CASE :              
      address = CERT_4_TYPE_ADDR;
      size = CERT_4_TYPE_SIZE;
      break;
    case CERT_4_DATE_CASE :                  
      address = CERT_4_DATE_ADDR;
      size = CERT_4_DATE_SIZE;
      break;
    case CERT_4_NUMBER_CASE :            
      address = CERT_4_NUMBER_ADDR;
      size = CERT_4_NUMBER_SIZE;
      break;
    case CERT_5_TYPE_CASE :            
      address = CERT_5_TYPE_ADDR;
      size = CERT_5_TYPE_SIZE;
      break;
    case CERT_5_DATE_CASE :             
      address = CERT_5_DATE_ADDR;
      size = CERT_5_DATE_SIZE;
      break;
    case CERT_5_NUMBER_CASE :            
      address = CERT_5_NUMBER_ADDR;
      size = CERT_5_NUMBER_SIZE;
      break;
    case TABLE_VALID_CASE :           
      address = TABLE_VALID_ADDR;
      size = TABLE_VALID_SIZE;
      break;
    case TABLE_REVISION_CASE :            
      address = TABLE_REVISION_ADDR;
      size = TABLE_REVISION_SIZE;
      break;
//    case VALID_TRUCKTEST_TIM_CASE :      
//      address = VALID_TRUCKTEST_TIM_ADR;
//      size = VALID_TRUCKTEST_TIM_SIZE;
//      break;
//    case TRUCK_TEST_TIM_NUMBER_CASE :         /* 4  */
//      return read_tim(TEST_TIM);
    case ALT_TIM_ID_VALID_CASE :            
      address = ALT_TIM_ID_VALID_ADDR;
      size = ALT_TIM_ID_VALID_SIZE;
      break;
    case ALT_TIM_ID_CASE :             
      return read_tim(ALT_TIM);
    case NUMBER_OF_COMPARTMENTS_CASE :          
      address = NUMBER_OF_COMPARTMENTS_ADDR;
      size = NUMBER_OF_COMPARTMENTS_SIZE;
      break;
    case COMPARTMENT_VOLUME_UNITS_CASE :         
      address = COMPARTMENT_VOLUME_UNITS_ADDR;
      size = COMPARTMENT_VOLUME_UNITS_SIZE;
      break;
    case TRAILER_ID_NUMBER_CASE :            
      address = TRAILER_ID_NUMBER_ADDR;
      size = TRAILER_ID_NUMBER_SIZE;
      break;
    case COMPARTMENT_CONFIG_CASE :             
      address = COMPARTMENT_CONFIG_ADDR;
      size = COMPARTMENT_CONFIG_SIZE;
      break;
    case VAPOR_INTERLOCK_TYPE_CASE :        
      address = VAPOR_INTERLOCK_TYPE_ADDR;
      size = VAPOR_INTERLOCK_TYPE_SIZE;
      break;
    case CPT1_TYPES_ALLOWED_CASE :             
      ascii_flag = TRUE;
      address = CPT1_TYPES_ALLOWED_ADDR;
      size = CPT1_TYPES_ALLOWED_SIZE;
      break;
    case CPT2_TYPES_ALLOWED_CASE :             
      ascii_flag = TRUE;
      address = CPT2_TYPES_ALLOWED_ADDR;
      size = CPT2_TYPES_ALLOWED_SIZE;
      break;
    case CPT3_TYPES_ALLOWED_CASE :             
      ascii_flag = TRUE;
      address = CPT3_TYPES_ALLOWED_ADDR;
      size = CPT3_TYPES_ALLOWED_SIZE;
      break;
    case CPT4_TYPES_ALLOWED_CASE :          
      ascii_flag = TRUE;
      address = CPT4_TYPES_ALLOWED_ADDR;
      size = CPT4_TYPES_ALLOWED_SIZE;
      break;
    case CPT5_TYPES_ALLOWED_CASE :               
      ascii_flag = TRUE;
      address = CPT5_TYPES_ALLOWED_ADDR;
      size = CPT5_TYPES_ALLOWED_SIZE;
      break;
    case CPT6_TYPES_ALLOWED_CASE :                
      ascii_flag = TRUE;
      address = CPT6_TYPES_ALLOWED_ADDR;
      size = CPT6_TYPES_ALLOWED_SIZE;
      break;
    case CPT7_TYPES_ALLOWED_CASE :             
      ascii_flag = TRUE;
      address = CPT7_TYPES_ALLOWED_ADDR;
      size = CPT7_TYPES_ALLOWED_SIZE;
      break;
    case CPT8_TYPES_ALLOWED_CASE :             
      ascii_flag = TRUE;
      address = CPT8_TYPES_ALLOWED_ADDR;
      size = CPT8_TYPES_ALLOWED_SIZE;
      break;
    case CPT9_TYPES_ALLOWED_CASE :              
      ascii_flag = TRUE;
      address = CPT9_TYPES_ALLOWED_ADDR;
      size = CPT9_TYPES_ALLOWED_SIZE;
      break;
    case CPT10_TYPES_ALLOWED_CASE :           
      ascii_flag = TRUE;
      address = CPT10_TYPES_ALLOWED_ADDR;
      size = CPT10_TYPES_ALLOWED_SIZE;
      break;
    case CPT11_TYPES_ALLOWED_CASE :             
      ascii_flag = TRUE;
      address = CPT11_TYPES_ALLOWED_ADDR;
      size = CPT11_TYPES_ALLOWED_SIZE;
      break;
    case CPT12_TYPES_ALLOWED_CASE :             
      ascii_flag = TRUE;
      address = CPT12_TYPES_ALLOWED_ADDR;
      size = CPT12_TYPES_ALLOWED_SIZE;
      break;
    case CPT13_TYPES_ALLOWED_CASE :          
      ascii_flag = TRUE;
      address = CPT13_TYPES_ALLOWED_ADDR;
      size = CPT13_TYPES_ALLOWED_SIZE;
      break;
    case CPT14_TYPES_ALLOWED_CASE :           
      ascii_flag = TRUE;
      address = CPT14_TYPES_ALLOWED_ADDR;
      size = CPT14_TYPES_ALLOWED_SIZE;
      break;
    case CPT15_TYPES_ALLOWED_CASE :          
      ascii_flag = TRUE;
      address = CPT15_TYPES_ALLOWED_ADDR;
      size = CPT15_TYPES_ALLOWED_SIZE;
      break;
    case CPT16_TYPES_ALLOWED_CASE :               
      ascii_flag = TRUE;
      address = CPT16_TYPES_ALLOWED_ADDR;
      size = CPT16_TYPES_ALLOWED_SIZE;
      break;
    case MAX_LOADING_TEMP_CASE :           
      address = MAX_LOADING_TEMP_ADDR;
      size = MAX_LOADING_TEMP_SIZE;
      break;
    case TEMPERATURE_UNITS_CASE :             
      address = TEMPERATURE_UNITS_ADDR;
      size = TEMPERATURE_UNITS_SIZE;
      break;
      
    case CPT1_TYPE_LOADED_CASE :            
      ascii_flag = TRUE;
      address = CPT1_TYPE_LOADED_ADDR;
      size = CPT1_TYPE_LOADED_SIZE;
      break;
    case CPT1_BATCH_ID_LOADED_CASE :            
      address = CPT1_BATCH_ID_LOADED_ADDR;
      size = CPT1_BATCH_ID_LOADED_SIZE;
      break;
    case CPT1_VOLUME_LOADED_CASE :               
      address = CPT1_VOLUME_LOADED_ADDR;
      size = CPT1_VOLUME_LOADED_SIZE;
      break;
    case CPT2_TYPE_LOADED_CASE :            
      ascii_flag = TRUE;
      address = CPT2_TYPE_LOADED_ADDR;
      size = CPT2_TYPE_LOADED_SIZE;
      break;
    case CPT2_BATCH_ID_LOADED_CASE :               
      address = CPT2_BATCH_ID_LOADED_ADDR;
      size = CPT2_BATCH_ID_LOADED_SIZE;
      break;
    case CPT2_VOLUME_LOADED_CASE :           
      address = CPT2_VOLUME_LOADED_ADDR;
      size = CPT2_VOLUME_LOADED_SIZE;
      break;
    case CPT3_TYPE_LOADED_CASE :                
      ascii_flag = TRUE;
      address = CPT3_TYPE_LOADED_ADDR;
      size = CPT3_TYPE_LOADED_SIZE;
      break;
    case CPT3_BATCH_ID_LOADED_CASE :             
      address = CPT3_BATCH_ID_LOADED_ADDR;
      size = CPT3_BATCH_ID_LOADED_SIZE;
      break;
    case CPT3_VOLUME_LOADED_CASE :           
      address = CPT3_VOLUME_LOADED_ADDR;
      size = CPT3_VOLUME_LOADED_SIZE;
      break;
    case CPT4_TYPE_LOADED_CASE :              
      ascii_flag = TRUE;
      address = CPT4_TYPE_LOADED_ADDR;
      size = CPT4_TYPE_LOADED_SIZE;
      break;
    case CPT4_BATCH_ID_LOADED_CASE :               
      address = CPT4_BATCH_ID_LOADED_ADDR;
      size = CPT4_BATCH_ID_LOADED_SIZE;
      break;
    case CPT4_VOLUME_LOADED_CASE :            
      address = CPT4_VOLUME_LOADED_ADDR;
      size = CPT4_VOLUME_LOADED_SIZE;
      break;
    case CPT5_TYPE_LOADED_CASE :              
      ascii_flag = TRUE;
      address = CPT5_TYPE_LOADED_ADDR;
      size = CPT5_TYPE_LOADED_SIZE;
      break;
    case CPT5_BATCH_ID_LOADED_CASE :                 
      address = CPT5_BATCH_ID_LOADED_ADDR;
      size = CPT5_BATCH_ID_LOADED_SIZE;
      break;
    case CPT5_VOLUME_LOADED_CASE :             
      address = CPT5_VOLUME_LOADED_ADDR;
      size = CPT5_VOLUME_LOADED_SIZE;
      break;
    case CPT6_TYPE_LOADED_CASE :         
      ascii_flag = TRUE;
      address = CPT6_TYPE_LOADED_ADDR;
      size = CPT6_TYPE_LOADED_SIZE;
      break;
    case CPT6_BATCH_ID_LOADED_CASE :              
      address = CPT6_BATCH_ID_LOADED_ADDR;
      size = CPT6_BATCH_ID_LOADED_SIZE;
      break;
    case CPT6_VOLUME_LOADED_CASE :           
      address = CPT6_VOLUME_LOADED_ADDR;
      size = CPT6_VOLUME_LOADED_SIZE;
      break;
    case CPT7_TYPE_LOADED_CASE :             
      ascii_flag = TRUE;
      address = CPT7_TYPE_LOADED_ADDR;
      size = CPT7_TYPE_LOADED_SIZE;
      break;
    case CPT7_BATCH_ID_LOADED_CASE :          
      address = CPT7_BATCH_ID_LOADED_ADDR;
      size = CPT7_BATCH_ID_LOADED_SIZE;
      break;
    case CPT7_VOLUME_LOADED_CASE :              
      address = CPT7_VOLUME_LOADED_ADDR;
      size = CPT7_VOLUME_LOADED_SIZE;
      break;
    case CPT8_TYPE_LOADED_CASE :             
      ascii_flag = TRUE;
      address = CPT8_TYPE_LOADED_ADDR;
      size = CPT8_TYPE_LOADED_SIZE;
      break;
    case CPT8_BATCH_ID_LOADED_CASE :           
      address = CPT8_BATCH_ID_LOADED_ADDR;
      size = CPT8_BATCH_ID_LOADED_SIZE;
      break;
    case CPT8_VOLUME_LOADED_CASE :           
      address = CPT8_VOLUME_LOADED_ADDR;
      size = CPT8_VOLUME_LOADED_SIZE;
      break;
    case CPT9_TYPE_LOADED_CASE :              
      ascii_flag = TRUE;
      address = CPT9_TYPE_LOADED_ADDR;
      size = CPT9_TYPE_LOADED_SIZE;
      break;
    case CPT9_BATCH_ID_LOADED_CASE :             
      address = CPT9_BATCH_ID_LOADED_ADDR;
      size = CPT9_BATCH_ID_LOADED_SIZE;
      break;
    case CPT9_VOLUME_LOADED_CASE :            
      address = CPT9_VOLUME_LOADED_ADDR;
      size = CPT9_VOLUME_LOADED_SIZE;
      break;
    case CPT10_TYPE_LOADED_CASE :            
      ascii_flag = TRUE;
      address = CPT10_TYPE_LOADED_ADDR;
      size = CPT10_TYPE_LOADED_SIZE;
      break;
    case CPT10_BATCH_ID_LOADED_CASE :                
      address = CPT10_BATCH_ID_LOADED_ADDR;
      size = CPT10_BATCH_ID_LOADED_SIZE;
      break;
    case CPT10_VOLUME_LOADED_CASE :            
      address = CPT10_VOLUME_LOADED_ADDR;
      size = CPT10_VOLUME_LOADED_SIZE;
      break;
    case CPT11_TYPE_LOADED_CASE :                 
      ascii_flag = TRUE;
      address = CPT11_TYPE_LOADED_ADDR;
      size = CPT11_TYPE_LOADED_SIZE;
      break;
    case CPT11_BATCH_ID_LOADED_CASE :              
      address = CPT11_BATCH_ID_LOADED_ADDR;
      size = CPT11_BATCH_ID_LOADED_SIZE;
      break;
    case CPT11_VOLUME_LOADED_CASE :              
      address = CPT11_VOLUME_LOADED_ADDR;
      size = CPT11_VOLUME_LOADED_SIZE;
      break;
    case CPT12_TYPE_LOADED_CASE :             
      ascii_flag = TRUE;
      address = CPT12_TYPE_LOADED_ADDR;
      size = CPT12_TYPE_LOADED_SIZE;
      break;
    case CPT12_BATCH_ID_LOADED_CASE :              
      address = CPT12_BATCH_ID_LOADED_ADDR;
      size = CPT12_BATCH_ID_LOADED_SIZE;
      break;
    case CPT12_VOLUME_LOADED_CASE :          
      address = CPT12_VOLUME_LOADED_ADDR;
      size = CPT12_VOLUME_LOADED_SIZE;
      break;
    case CPT13_TYPE_LOADED_CASE :              
      ascii_flag = TRUE;
      address = CPT13_TYPE_LOADED_ADDR;
      size = CPT13_TYPE_LOADED_SIZE;
      break;
    case CPT13_BATCH_ID_LOADED_CASE :               
      address = CPT13_BATCH_ID_LOADED_ADDR;
      size = CPT13_BATCH_ID_LOADED_SIZE;
      break;
    case CPT13_VOLUME_LOADED_CASE :           
      address = CPT13_VOLUME_LOADED_ADDR;
      size = CPT13_VOLUME_LOADED_SIZE;
      break;
    case CPT14_TYPE_LOADED_CASE :                
      ascii_flag = TRUE;
      address = CPT14_TYPE_LOADED_ADDR;
      size = CPT14_TYPE_LOADED_SIZE;
      break;
    case CPT14_BATCH_ID_LOADED_CASE :                 
      address = CPT14_BATCH_ID_LOADED_ADDR;
      size = CPT14_BATCH_ID_LOADED_SIZE;
      break;
    case CPT14_VOLUME_LOADED_CASE :            
      address = CPT14_VOLUME_LOADED_ADDR;
      size = CPT14_VOLUME_LOADED_SIZE;
      break;
    case CPT15_TYPE_LOADED_CASE :            
      ascii_flag = TRUE;
      address = CPT15_TYPE_LOADED_ADDR;
      size = CPT15_TYPE_LOADED_SIZE;
      break;
    case CPT15_BATCH_ID_LOADED_CASE :            
      address = CPT15_BATCH_ID_LOADED_ADDR;
      size = CPT15_BATCH_ID_LOADED_SIZE;
      break;
    case CPT15_VOLUME_LOADED_CASE :            
      address = CPT15_VOLUME_LOADED_ADDR;
      size = CPT15_VOLUME_LOADED_SIZE;
      break;
    case CPT16_TYPE_LOADED_CASE :             
      ascii_flag = TRUE;
      address = CPT16_TYPE_LOADED_ADDR;
      size = CPT16_TYPE_LOADED_SIZE;
      break;
    case CPT16_BATCH_ID_LOADED_CASE :        
      address = CPT16_BATCH_ID_LOADED_ADDR;
      size = CPT16_BATCH_ID_LOADED_SIZE;
      break;
    case CPT16_VOLUME_LOADED_CASE :          
      address = CPT16_VOLUME_LOADED_ADDR;
      size = CPT16_VOLUME_LOADED_SIZE;
      break;
     case TERMINAL_NAME_CASE :               
      ascii_flag = TRUE;
      address = TERMINAL_NAME_ADDR;
      size = TERMINAL_NAME_SIZE;
      break;
     case TERMINAL_ADDRESS_CASE :            
      ascii_flag = TRUE;
      address = TERMINAL_ADDRESS_ADDR;
      size = TERMINAL_ADDRESS_SIZE;
      break;
     case TERMINAL_GANTRY_NUMBER_CASE :               
      address = TERMINAL_GANTRY_NUMBER_ADDR;
      size = TERMINAL_GANTRY_NUMBER_SIZE;
      break;
     
      
      
     case FAULT_LOG1_CASE :              
      address = FAULT_LOG1_ADDR;
      size = FAULT_LOG1_SIZE;
      break;
     case FAULT_LOG2_CASE :             
      address = FAULT_LOG2_ADDR;
      size = FAULT_LOG2_SIZE;
      break;
     case FAULT_LOG3_CASE :               
      address = FAULT_LOG3_ADDR;
      size = FAULT_LOG3_SIZE;
      break;
     case FAULT_LOG4_CASE :              
      address = FAULT_LOG4_ADDR;
      size = FAULT_LOG4_SIZE;
      break;
     case FAULT_LOG5_CASE :               
      address = FAULT_LOG5_ADDR;
      size = FAULT_LOG5_SIZE;
      break;
     case SERVICE_CENTER_NAME_CASE :             
      address = SERVICE_CENTER_NAME_ADDR;
      size = SERVICE_CENTER_NAME_SIZE;
      break;
     case SERVICE_CENTER_ADDRESS_CASE :                  
      address = SERVICE_CENTER_ADDRESS_ADDR;
      size = SERVICE_CENTER_ADDRESS_SIZE;
      break;
    case BUILDER_NAME_CASE :                  
      ascii_flag = TRUE;
      address = BUILDER_NAME_ADDR;
      size = BUILDER_NAME_SIZE;
      break;
    case BUILDER_ADDRESS_CASE :           
      ascii_flag = TRUE;
      address = BUILDER_ADDRESS_ADDR;
      size = BUILDER_ADDRESS_SIZE;
      break;
    case TRUCK_SERIAL_NUMBER_CASE :       
      ascii_flag = TRUE;
      address = TRUCK_SERIAL_NUMBER_ADDR;
      size = TRUCK_SERIAL_NUMBER_SIZE;
      break;
    case TRUCK_VIN_CASE :                    
      ascii_flag = TRUE;
      address = TRUCK_VIN_ADDR;
      size = TRUCK_VIN_SIZE;
      break;
    case TRUCK_BUILD_DATE_CASE :                
      address = TRUCK_BUILD_DATE_ADDR;
      size = TRUCK_BUILD_DATE_SIZE;
      break;
    case TRUCK_WEIGHT_UNITS_CASE :                 
      address = TRUCK_WEIGHT_UNITS_ADDR;
      size = TRUCK_WEIGHT_UNITS_SIZE;
      break;
    case TRUCK_GVW_CASE :                        
      address = TRUCK_GVW_ADDR;
      size = TRUCK_GVW_SIZE;
      break;
    case INTELLICHECK_TYPE_CASE :           
      address = INTELLICHECK_TYPE_ADDR;
      size = INTELLICHECK_TYPE_SIZE;
      break;
    case OVERFILL_SENSOR_TYPE_CASE :              
      address = OVERFILL_SENSOR_TYPE_ADDR;
      size = OVERFILL_SENSOR_TYPE_SIZE;
      break;
    case RETAINED_SENSOR_TYPE_CASE :              
      address = RETAINED_SENSOR_TYPE_ADDR;
      size = RETAINED_SENSOR_TYPE_SIZE;
      break;
    case CPT1_BUILD_VOLUME_CASE :                 
      address = CPT1_BUILD_VOLUME_ADDR;
      size = CPT1_BUILD_VOLUME_SIZE;
      break;
    case CPT2_BUILD_VOLUME_CASE :              
      address = CPT2_BUILD_VOLUME_ADDR;
      size = CPT2_BUILD_VOLUME_SIZE;
      break;
    case CPT3_BUILD_VOLUME_CASE :                 
      address = CPT3_BUILD_VOLUME_ADDR;
      size = CPT3_BUILD_VOLUME_SIZE;
      break;
    case CPT4_BUILD_VOLUME_CASE :                
      address = CPT4_BUILD_VOLUME_ADDR;
      size = CPT4_BUILD_VOLUME_SIZE;
      break;
    case CPT5_BUILD_VOLUME_CASE :              
      address = CPT5_BUILD_VOLUME_ADDR;
      size = CPT5_BUILD_VOLUME_SIZE;
      break;
    case CPT6_BUILD_VOLUME_CASE :                
      address = CPT6_BUILD_VOLUME_ADDR;
      size = CPT6_BUILD_VOLUME_SIZE;
      break;
    case CPT7_BUILD_VOLUME_CASE :            
      address = CPT7_BUILD_VOLUME_ADDR;
      size = CPT7_BUILD_VOLUME_SIZE;
      break;
    case CPT8_BUILD_VOLUME_CASE :           
      address = CPT8_BUILD_VOLUME_ADDR;
      size = CPT8_BUILD_VOLUME_SIZE;
      break;
    case CPT9_BUILD_VOLUME_CASE :            
      address = CPT9_BUILD_VOLUME_ADDR;
      size = CPT9_BUILD_VOLUME_SIZE;
      break;
    case CPT10_BUILD_VOLUME_CASE :               
      address = CPT10_BUILD_VOLUME_ADDR;
      size = CPT10_BUILD_VOLUME_SIZE;
      break;
    case CPT11_BUILD_VOLUME_CASE :             
      address = CPT11_BUILD_VOLUME_ADDR;
      size = CPT11_BUILD_VOLUME_SIZE;
      break;
    case CPT12_BUILD_VOLUME_CASE :         
      address = CPT12_BUILD_VOLUME_ADDR;
      size = CPT12_BUILD_VOLUME_SIZE;
      break;
    case CPT13_BUILD_VOLUME_CASE :               
      address = CPT13_BUILD_VOLUME_ADDR;
      size = CPT13_BUILD_VOLUME_SIZE;
      break;
    case CPT14_BUILD_VOLUME_CASE :                
      address = CPT14_BUILD_VOLUME_ADDR;
      size = CPT14_BUILD_VOLUME_SIZE;
      break;
    case CPT15_BUILD_VOLUME_CASE :             
      address = CPT15_BUILD_VOLUME_ADDR;
      size = CPT15_BUILD_VOLUME_SIZE;
      break;
    case CPT16_BUILD_VOLUME_CASE :                
      address = CPT16_BUILD_VOLUME_ADDR;
      size = CPT16_BUILD_VOLUME_SIZE;
      break;
//    case NO_RELAY_CASE :                   
//      address = 0x499;
//      size = 1;
//      break;
    case SCULLY_SENSORS_CASE :        
      address = SCULLY_SENSORS_ADDR;
      size = SCULLY_SENSORS_SIZE;
      break;
    case TANK_MODEL_NUMBER_CASE :            
      address = TANK_MODEL_NUMBER_ADDR;
      size = TANK_MODEL_NUMBER_SIZE;
      break;
    case MAX_WORKING_PRESSURE_CASE :             
      address = MAX_WORKING_PRESSURE_ADDR;
      size = MAX_WORKING_PRESSURE_SIZE;
      break;
    case ALLOWABLE_WORKING_PRESSURE_CASE :            
      address = ALLOWABLE_WORKING_PRESSURE_ADDR;
      size = ALLOWABLE_WORKING_PRESSURE_SIZE;
      break;
    case PRESSURE_UNITS_CASE :            
      address = PRESSURE_UNITS_ADDR;
      size = PRESSURE_UNITS_SIZE;
      break;
    case BULKHEADS_CASE :          
      address = BULKHEADS_ADDR;
      size = BULKHEADS_SIZE;
      break;
    case TANK_PROFILE_CASE :          
      address = TANK_PROFILE_ADDR;
      size = TANK_PROFILE_SIZE;
      break;
    case OVERFILL_SENSOR1_LENGTH_CASE :        
      address = OVERFILL_SENSOR1_LENGTH_ADDR;
      size = OVERFILL_SENSOR1_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR2_LENGTH_CASE :           
      address = OVERFILL_SENSOR2_LENGTH_ADDR;
      size = OVERFILL_SENSOR2_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR3_LENGTH_CASE :          
      address = OVERFILL_SENSOR3_LENGTH_ADDR;
      size = OVERFILL_SENSOR3_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR4_LENGTH_CASE :  
      address = OVERFILL_SENSOR4_LENGTH_ADDR;
      size = OVERFILL_SENSOR4_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR5_LENGTH_CASE :            
      address = OVERFILL_SENSOR5_LENGTH_ADDR;
      size = OVERFILL_SENSOR5_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR6_LENGTH_CASE :         
      address = OVERFILL_SENSOR6_LENGTH_ADDR;
      size = OVERFILL_SENSOR6_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR7_LENGTH_CASE :         
      address = OVERFILL_SENSOR7_LENGTH_ADDR;
      size = OVERFILL_SENSOR7_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR8_LENGTH_CASE :           
      address = OVERFILL_SENSOR8_LENGTH_ADDR;
      size = OVERFILL_SENSOR8_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR9_LENGTH_CASE :           
      address = OVERFILL_SENSOR9_LENGTH_ADDR;
      size = OVERFILL_SENSOR9_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR10_LENGTH_CASE :           
      address = OVERFILL_SENSOR10_LENGTH_ADDR;
      size = OVERFILL_SENSOR10_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR11_LENGTH_CASE :        
      address = OVERFILL_SENSOR11_LENGTH_ADDR;
      size = OVERFILL_SENSOR11_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR12_LENGTH_CASE :         
      address = OVERFILL_SENSOR12_LENGTH_ADDR;
      size = OVERFILL_SENSOR12_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR13_LENGTH_CASE :            
      address = OVERFILL_SENSOR13_LENGTH_ADDR;
      size = OVERFILL_SENSOR13_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR14_LENGTH_CASE :             
      address = OVERFILL_SENSOR14_LENGTH_ADDR;
      size = OVERFILL_SENSOR14_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR15_LENGTH_CASE :       
      address = OVERFILL_SENSOR15_LENGTH_ADDR;
      size = OVERFILL_SENSOR15_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR16_LENGTH_CASE :             
      address = OVERFILL_SENSOR16_LENGTH_ADDR;
      size = OVERFILL_SENSOR16_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR17_LENGTH_CASE :        
      address = OVERFILL_SENSOR17_LENGTH_ADDR;
      size = OVERFILL_SENSOR17_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR18_LENGTH_CASE :          
      address = OVERFILL_SENSOR18_LENGTH_ADDR;
      size = OVERFILL_SENSOR18_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR19_LENGTH_CASE :        
      address = OVERFILL_SENSOR19_LENGTH_ADDR;
      size = OVERFILL_SENSOR19_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR20_LENGTH_CASE :         
      address = OVERFILL_SENSOR20_LENGTH_ADDR;
      size = OVERFILL_SENSOR20_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR21_LENGTH_CASE :           
      address = OVERFILL_SENSOR21_LENGTH_ADDR;
      size = OVERFILL_SENSOR21_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR22_LENGTH_CASE :    
      address = OVERFILL_SENSOR22_LENGTH_ADDR;
      size = OVERFILL_SENSOR22_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR23_LENGTH_CASE :          
      address = OVERFILL_SENSOR23_LENGTH_ADDR;
      size = OVERFILL_SENSOR23_LENGTH_SIZE;
      break;
    case OVERFILL_SENSOR24_LENGTH_CASE :        
      address = OVERFILL_SENSOR24_LENGTH_ADDR;
      size = OVERFILL_SENSOR24_LENGTH_SIZE;
      break;
      default:
      return MB_EXC_MEM_PAR_ERR;
  }

  /****************************** 7/28/2009 6:49AM ***************************
   * cnt is an option
   ***************************************************************************/
  sts = mbcGetByte (&cnt); /* Extract the section of memory the user wants */
  if (sts)
    cnt = size;                     /* if no count use default */

//  if ( cnt > size)
//  {
//    return MB_EXC_TIM_MEM_AREA_ERR;
//  }

  if (ascii_flag == TRUE)
  {
     for ( i=0; i<cnt; i++)
     {
        if (mem_ptr[i] == 0)
          break;
     }
     size = (unsigned char)i;
   }

  if ((sts = (MODBSTS)tim_block_read(mem_ptr, address, cnt)) != MB_OK)
  {
    return (sts);
  }

  sts = mbcPutByte (cnt);           /* Report string length */

  for ( i=0; i<cnt; i++)
  {
    sts = mbcPutByte (mem_ptr[i]);           /* Send back the indication for invalid entry */
    if (sts)
    {
      return (sts);
    }
  }

  dummy_func((unsigned char *)&size);

  return (sts) ;                     /* Return successfully */
}

/******************************* 6/15/2009 2:21PM ****************************
 * Write a section of the Super TIM memory
 *****************************************************************************/
static MODBSTS mbcWrBuilderInfo(void)
{
unsigned char builder_section, cnt;
MODBSTS sts;                          /* Status holding */
unsigned int i, size, address;
unsigned char *mem_ptr;
unsigned char temp_area[80];

  mem_ptr = temp_area;

  sts = mbcGetByte (&builder_section); /* Extract the section of memory the user wants */
  if (sts)
    return (sts);                     /* Propagate error */

  sts = mbcPutByte (builder_section);           /* "Echo" back section */
  if (sts)
    return (sts);

  switch ( builder_section)
  {
    case CPT1_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT1_TYPE_LOADED_ADDR;
      size = CPT1_TYPE_LOADED_SIZE;
      break;
    case CPT1_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT1_BATCH_ID_LOADED_ADDR;
      size = CPT1_BATCH_ID_LOADED_SIZE;
      break;
    case CPT1_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT1_VOLUME_LOADED_ADDR;
      size = CPT1_VOLUME_LOADED_SIZE;
      break;
    case CPT2_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT2_TYPE_LOADED_ADDR;
      size = CPT2_TYPE_LOADED_SIZE;
      break;
    case CPT2_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT2_BATCH_ID_LOADED_ADDR;
      size = CPT2_BATCH_ID_LOADED_SIZE;
      break;
    case CPT2_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT2_VOLUME_LOADED_ADDR;
      size = CPT2_VOLUME_LOADED_SIZE;
      break;
    case CPT3_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT3_TYPE_LOADED_ADDR;
      size = CPT3_TYPE_LOADED_SIZE;
      break;
    case CPT3_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT3_BATCH_ID_LOADED_ADDR;
      size = CPT3_BATCH_ID_LOADED_SIZE;
      break;
    case CPT3_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT3_VOLUME_LOADED_ADDR;
      size = CPT3_VOLUME_LOADED_SIZE;
      break;
    case CPT4_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT4_TYPE_LOADED_ADDR;
      size = CPT4_TYPE_LOADED_SIZE;
      break;
    case CPT4_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT4_BATCH_ID_LOADED_ADDR;
      size = CPT4_BATCH_ID_LOADED_SIZE;
      break;
    case CPT4_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT4_VOLUME_LOADED_ADDR;
      size = CPT4_VOLUME_LOADED_SIZE;
      break;
    case CPT5_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT5_TYPE_LOADED_ADDR;
      size = CPT5_TYPE_LOADED_SIZE;
      break;
    case CPT5_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT5_BATCH_ID_LOADED_ADDR;
      size = CPT5_BATCH_ID_LOADED_SIZE;
      break;
    case CPT5_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT5_VOLUME_LOADED_ADDR;
      size = CPT5_VOLUME_LOADED_SIZE;
      break;
    case CPT6_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT6_TYPE_LOADED_ADDR;
      size = CPT6_TYPE_LOADED_SIZE;
      break;
    case CPT6_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT6_BATCH_ID_LOADED_ADDR;
      size = CPT6_BATCH_ID_LOADED_SIZE;
      break;
    case CPT6_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT6_VOLUME_LOADED_ADDR;
      size = CPT6_VOLUME_LOADED_SIZE;
      break;
    case CPT7_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT7_TYPE_LOADED_ADDR;
      size = CPT7_TYPE_LOADED_SIZE;
      break;
    case CPT7_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT7_BATCH_ID_LOADED_ADDR;
      size = CPT7_BATCH_ID_LOADED_SIZE;
      break;
    case CPT7_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT7_VOLUME_LOADED_ADDR;
      size = CPT7_VOLUME_LOADED_SIZE;
      break;
    case CPT8_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT8_TYPE_LOADED_ADDR;
      size = CPT8_TYPE_LOADED_SIZE;
      break;
    case CPT8_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT8_BATCH_ID_LOADED_ADDR;
      size = CPT8_BATCH_ID_LOADED_SIZE;
      break;
    case CPT8_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT8_VOLUME_LOADED_ADDR;
      size = CPT8_VOLUME_LOADED_SIZE;
      break;
    case CPT9_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT9_TYPE_LOADED_ADDR;
      size = CPT9_TYPE_LOADED_SIZE;
      break;
    case CPT9_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT9_BATCH_ID_LOADED_ADDR;
      size = CPT9_BATCH_ID_LOADED_SIZE;
      break;
    case CPT9_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT9_VOLUME_LOADED_ADDR;
      size = CPT9_VOLUME_LOADED_SIZE;
      break;
    case CPT10_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT10_TYPE_LOADED_ADDR;
      size = CPT10_TYPE_LOADED_SIZE;
      break;
    case CPT10_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT10_BATCH_ID_LOADED_ADDR;
      size = CPT10_BATCH_ID_LOADED_SIZE;
      break;
    case CPT10_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT10_VOLUME_LOADED_ADDR;
      size = CPT10_VOLUME_LOADED_SIZE;
      break;
    case CPT11_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT11_TYPE_LOADED_ADDR;
      size = CPT11_TYPE_LOADED_SIZE;
      break;
    case CPT11_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT11_BATCH_ID_LOADED_ADDR;
      size = CPT11_BATCH_ID_LOADED_SIZE;
      break;
    case CPT11_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT11_VOLUME_LOADED_ADDR;
      size = CPT11_VOLUME_LOADED_SIZE;
      break;
    case CPT12_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT12_TYPE_LOADED_ADDR;
      size = CPT12_TYPE_LOADED_SIZE;
      break;
    case CPT12_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT12_BATCH_ID_LOADED_ADDR;
      size = CPT12_BATCH_ID_LOADED_SIZE;
      break;
    case CPT12_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT12_VOLUME_LOADED_ADDR;
      size = CPT12_VOLUME_LOADED_SIZE;
      break;
    case CPT13_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT13_TYPE_LOADED_ADDR;
      size = CPT13_TYPE_LOADED_SIZE;
      break;
    case CPT13_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT13_BATCH_ID_LOADED_ADDR;
      size = CPT13_BATCH_ID_LOADED_SIZE;
      break;
    case CPT13_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT13_VOLUME_LOADED_ADDR;
      size = CPT13_VOLUME_LOADED_SIZE;
      break;
    case CPT14_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT14_TYPE_LOADED_ADDR;
      size = CPT14_TYPE_LOADED_SIZE;
      break;
    case CPT14_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT14_BATCH_ID_LOADED_ADDR;
      size = CPT14_BATCH_ID_LOADED_SIZE;
      break;
    case CPT14_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT14_VOLUME_LOADED_ADDR;
      size = CPT14_VOLUME_LOADED_SIZE;
      break;
    case CPT15_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT15_TYPE_LOADED_ADDR;
      size = CPT15_TYPE_LOADED_SIZE;
      break;
    case CPT15_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT15_BATCH_ID_LOADED_ADDR;
      size = CPT15_BATCH_ID_LOADED_SIZE;
      break;
    case CPT15_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT15_VOLUME_LOADED_ADDR;
      size = CPT15_VOLUME_LOADED_SIZE;
      break;
    case CPT16_TYPE_LOADED_CASE :                  /* 51 */
      address = CPT16_TYPE_LOADED_ADDR;
      size = CPT16_TYPE_LOADED_SIZE;
      break;
    case CPT16_BATCH_ID_LOADED_CASE :                  /* 51 */
      address = CPT16_BATCH_ID_LOADED_ADDR;
      size = CPT16_BATCH_ID_LOADED_SIZE;
      break;
    case CPT16_VOLUME_LOADED_CASE :                  /* 51 */
      address = CPT16_VOLUME_LOADED_ADDR;
      size = CPT16_VOLUME_LOADED_SIZE;
      break;
     case TERMINAL_NAME_CASE :                  /* 51 */
      address = TERMINAL_NAME_ADDR;
      size = TERMINAL_NAME_SIZE;
      break;
     case TERMINAL_ADDRESS_CASE :                  /* 51 */
      address = TERMINAL_ADDRESS_ADDR;
      size = TERMINAL_ADDRESS_SIZE;
      break;
     case TERMINAL_GANTRY_NUMBER_CASE :                  /* 51 */
      address = TERMINAL_GANTRY_NUMBER_ADDR;
      size = TERMINAL_GANTRY_NUMBER_SIZE;
      break;
    default:
        if( builder_section <= 0xB3 )
        {
            return MB_READ_ONLY_VALUE;
        }
        else
        {
            return MB_EXC_TIM_MEM_AREA_ERR;            
        }
      break;
  }

  sts = mbcGetByte (&cnt); /* Fetch the size of buffer to be loaded */
  if (sts)
    return (sts);                     /* Propagate error */

  if ( cnt > size)      /* Make sure string is not bigger than the allocated space */
  {
    return MB_EXC_TIM_MEM_AREA_ERR;
  }

  sts = mbcPutByte (cnt); /* Return the size */
  if (sts)
  {
    return (MB_EXC_MEM_PAR_ERR)  /* Send back the indication for error */;
  }

  for ( i=0; i<size; i++)
  {
    mem_ptr[i] = 0;
  }
  for ( i=0; i<cnt; i++)
  {
    sts = mbcGetByte (&mem_ptr[i]); /* Fetch the data to be loaded */
    if (sts)
    {
      return (MB_EXC_MEM_PAR_ERR);
    }

    if ((builder_section == NUMBER_OF_COMPARTMENTS_CASE) && (mem_ptr[i] > MAX_COMPARTMENTS))
    {
      return MB_EXC_NUMBER_COMPARTMENTS_ERR;
    }

  }

  sts = (MODBSTS)tim_block_write(mem_ptr, address, size);

  service_charge();             /* Keep Service LED off */
  return (sts);                     /* Return successfully */
}

/*************************************************************************
* modbus_decode  --  Top-level ModBus Command dispatcher
*
* Call is:
*
*       modbus_decode (ilen, icmd, olen, orsp)
*
* Where:
*
*       <ilen> and <icmd> are the length of and pointer to the input ModBus
*       command/request message;
*
*       <olen> and <orsp> are the length of and the pointer to the output
*       ModBus response message buffer.
*
* modbus_decode() is called by the low-level ModBus processing code when a
* complete ModBus message has been received, CRC-checked, and is addressed
* to us (either directly or via "Broadcast" to all units).
*
* Return is the ModBus "Exception" value. "OK" means that the message was
* processed and that a "Response" message has been built in the orsp buffer, 
* with olen containing the byte count. (If olen is zero, then no response 
* is required -- e.g., actual addressee was Backup processor, etc.)
* If a non-zero response code is returned, then olen/orsp are meaningless
* and the low level code should build a ModBus Exception message and send
* that instead to the bus master.
*************************************************************************/

MODBSTS modbus_decode
    (
    unsigned char  ilen,                 /* Input ModBus message length (no CRC) */
    unsigned char *icmd,                 /* Input ModBus message pointer */
    unsigned char *olen,                 /* Output ModBus message length (no CRC) */
    unsigned char *orsp                  /* Output (response) ModBus message pointer */
    )
{
static char ctlbyte = 0;    /* Message-sequence processing state byte */
static char stsbyte = 0;    /* Accumulated error/status state byte */
unsigned save_iec0;

MODBSTS sts;                /* Status/result hold */
char sts1;                  /* Local status flag */
unsigned char data;                  /* Local data byte */


    /* Setup input message/buffer pointers/etc. */

    service_charge();             /* Keep Service LED off */
    getcnt = (char)(ilen - MODBUS_SKIP_CRC); /* CRC - already verified */
    getptr = icmd;                      /* Point to start of useful stuff */

    /* Setup output message/buffer pointers/etc. */

    putcnt = 0;                         /* Init count to count useful data */
    putptr = orsp;                      /* Point to start of useful stuff */

    /* First byte: Address field always */

    sts = mbcGetByte (&data);           /* Read in ModBus Address byte */
    if (sts)
        return (sts);

    sts = mbcPutByte (data);            /* Stash Address in potential response */
    if (sts)
        return (sts);

    /* Next byte is either the message type (ModBus Function Code), or if
       the sign bit is set, my newfangled "Message Control" field. Note that
       a slave response is *ALWAYS* old format, since a response message with
       byte 1 bit 7 set is an "error/exception" response! */

    sts = mbcGetByte (&data);           /* Read Control/Function byte */
    if (sts)
        return (sts);

    if (data & MBC_XCONTROL)            /* ModBus Control/Extensions? */
        {                               /* Yes */
        /* Extended ModBus Control field */

        if (data & MBC_XRESPMSG)        /* Is this a "Command" or "Response"? */
            {                           /* Response */
            return (MB_NO_RESPONSE);    /* Duh? Just eat it... */
            }

        /* Verify that this just-received message is the "next expected
           message". Each sequence of messages (first message with XMORE)
           starts with "0", so stand-alone (un-segmented; XMORE clear)
           messages are "always" expected if previous message was not a
           segmented messaged (i.e., previous message did not have XMORE). */

        sts = (char)(ctlbyte & MBC_XSEQUENCE); /* Extract expected seq number */
        if ((sts ^ data) & MBC_XSEQUENCE) /* Expected "next" message? */
            {                           /* No! */
            if (data & MBC_XMORE)       /* Is this a "last" (or only) segment? */
                {                       /* No (more segments coming) */
                if (stsbyte == 0)       /* If this is first error... */
                    stsbyte = MB_EXC_FAULT;/* First error - "queue" status */
                return (MB_NO_RESPONSE);/* And just eat this message */
                }
            else if (data & MBC_XNORESPONSE) /* Last segment -- forced quiet? */
                {                       /* Yes */
                ctlbyte = 0;            /* Reset seq num/et al */
                stsbyte = 0;            /* Reset accumulated error/status */
                return (MB_NO_RESPONSE);/* Eat this message, no action */
                }
            else                        /* Last segment, error response */
                {
                sts = (MODBSTS)stsbyte;          /* Accumulated error/status */
                ctlbyte = 0;            /* Reset seq num/at al. */
                stsbyte = 0;            /* Reset accumulated error/status */
                if (sts)                /* Previous error? */
                    return (sts);       /* Yes, it wins, return it as response */
                else                    /* No, this is our error */
                    return (MB_EXC_FAULT);  /* Bad message */
                }
            } /* End not expected next message number */

        /* Proper message in sequence (may be first/only message in seq) */

        if (data & MBC_XMORE)           /* More segments coming? */
            {                           /* Yes */
            sts++;                      /* Pre-set next expected number */
            sts &= MBC_XSEQUENCE;       /* Keep it mod-8 */
            sts |= MBC_XMORE;           /* (and control non-zero as well!) */
            }
        else                            /* This is the last segment */
            sts = 0;                    /* Clear sequence field */

        if (data & MBC_XNORESPONSE)     /* Master suppressing slave response? */
            sts |= MBC_XNORESPONSE;     /* Yes, remember that */

        ctlbyte = (char)sts;                  /* Save new control byte */

        /* Third byte: count of bytes (following) in message, excluding CRC */

        sts = mbcGetByte (&data);       /* Read in message byte count */
        if (sts)
            return (sts);

        if (data)                       /* Count non-zero? */
            {                           /* Yes (ignore zero count field) */
            if (data < getcnt)          /* Believe count field over total size */
                getcnt = data;          /* Truncate message to indicated count */
            }

        /* Fourth byte: ModBus Function Code byte */

        sts = mbcGetByte (&data);       /* Read in message type */
        if (sts)
            return (sts);

        /* Now fall into message dispatcher */

        } /* End New-Fangled Extended ModBus Control Extensions */
    else                                /* Old style ModBus message */
    {                                 /* Byte two is ModBus Function Code */
      if (ctlbyte)                    /* Any extensions processing dangling? */
      {                               /* Yes */
        ctlbyte = 0;                  /* Reset message processing state */
        sts = (MODBSTS)stsbyte;       /* Retrieve queued error/status */
        stsbyte = 0;                  /* Reset queued error/status */
        if (sts)                      /* If any previous error pending, */
          return (sts);               /* Return it now */
        return (MB_EXC_FAULT);        /* Return protocol error */
      }
    }

    /* All slave responses include the original ModBus Function Code field.
       This is ALWAYS the SECOND response buffer byte, since the slave can-
       not use bit 7 (MBC_XCONTROL) to talk to the master -- this is the
       error/exception response bit.*/

    sts = mbcPutByte (data);            /* Echo back ModBus Function Code */
    if (sts)
        return (sts);

    /* If a previously-processed message in this sequence of messages has
       failed for any reason, then suppress processing of all following
       messages until we can return the error/status code to the master */

    if (stsbyte == 0)                   /* Failed "sequence" ? */
        {                               /* No, process this message normally */
        modNVflag = 0;                  /* Init nvSysParmUpdate() needed flag */

        /* Dispatch on message type */

        switch (data)
            {
          case READ_OUTPUT_STATUS:      /* 0x01 -- Read Output Status Bits */
            sts = mbcRdOStatus ();
            break;

          case READ_INPUT_STATUS:       /* 0x02 -- Read Input Status Bits */
            sts = mbcRdIStatus ();
            break;

          case READ_MULTIPLE_REGS:      /* 0x03 -- Read Input Status Bits */
            sts = mbcRdRegs ();
            break;

          case FORCE_SINGLE_BIT:        /* 0x05 -- Force "Bit" */
            sts = mbcForceBit();
            break;

          case WRITE_SINGLE_REG:        /* 0x06 -- Write Single "Register" */
            sts = mbcWrOneReg ();
            break;

          case WRITE_MULTIPLE_REGS:     /* 0x10 -- Write Multiple "Registers" */
            sts = mbcWrRegs ();
            break;

          case WRITE_SINGLE_VEHICLE:    /* 0x41 -- Write Single Vehicle ID */
            sts = mbcWrOneTruckID ();
            break;

          case READ_SINGLE_VEHICLE:     /* 0x42 -- Read Single Vehicle ID */
            sts = mbcRdOneTruckID ();
            break;

          case READ_VIP_LOG_ELEMENT:    /* 0x43 -- Read VIP Log */
            sts = mbcRdVIPLog ();
            break;

          case WRITE_PASSWORD:          /* 0x44 -- Write "Password" */
            sts = mbcWrPassword ();
            break;

          case WRITE_COMPANY_ID:        /* 0x45 -- Write "Company ID" */
            sts = mbcWrCompanyID ();
            break;

          case WRITE_MULTIPLE_VEHICLES: /* 0x46 -- Write Multiple Vehicle IDs */
            sts = mbcWrTruckIDs ();
            break;

          case READ_MULTIPLE_VEHICLES:  /* 0x47 -- Read Multiple Vehicle IDs */
            sts = mbcRdTruckIDs ();
            break;

          case BACKUP_FUNCTIONS:        /* 0x48 -- Read Backup Processor */
            sts = MB_NO_RESPONSE;
            break;

          case READ_TRL_LOG_ELEMENT:    /* 0x49 -- Read Intellitrol Log */
            sts = mbcRdTrlLog ();
            break;

          case CRC_MULTIPLE_VEHICLES:   /* 0x4A -- CRC multiple vehicles */
            sts = mbcVrTruckIDs ();
            break;

          case WRITE_BYPASS_KEYS:       /* 0x4B -- Write Bypass Keys */
            sts = mbcWrKeys ();
            break;

          case READ_BYPASS_KEYS:        /* 0x4C -- Read Bypass Keys */
            sts = mbcRdKeys ();
            break;

          case WRITE_FEATURES_PASSWORD: /* 0x4D -- Write Features-enable */
            sts = mbcWrEnaFeatures ();
            break;

          case READ_EE_BLOCK:           /* 0x4E -- Read special EEPROM block */
            sts = mbcRdEEBlock ();
            break;

          case WRITE_EE_BLOCK:          /* 0x4F -- Write special EEPROM block */
            sts = mbcWrEEBlock ();
            break;

          /************************** 12/30/2008 9:33AM **********************
           * This command will send back the volume of the compartment the
           * originator requested.
           *******************************************************************/
          case REPORT_COMPARTMENT_VOLUME: /* 0x50 --  */
            sts = mbcRdTrCompt ();
            break;

          /************************** 6/22/2009 8:16AM ***********************
           * Fetch info from the Scully reserve area
           *******************************************************************/
          case READ_TIM_SCULLY_AREA:         /* 0x51 --  */
          {
           /****************************** 9/11/2008 10:33AM **************************
            * Disable interrupts
            ***************************************************************************/
            save_iec0 = IEC0;
            IEC0 = 0;                   /* Disable heart beat and DMA interrupt */
            if (!Read_Dallas_SN(COMM_ID))         /* Fetch the serial number */
            {
               sts = MB_READ_SERIAL_ERROR;
            }
            if ( sts == 0)
            {
              sts = readTIMarea(0x00, TIM_size);
               /****************************** 9/11/2008 10:35AM **************************
               * restore interrupts
               ***************************************************************************/
            }
            IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */
          }
            break;

          /************************** 6/22/2009 8:17AM ***********************
           * Write into the Scully reserve area
           *******************************************************************/
          case WRITE_TIM_SCULLY_AREA:         /* 0x52 --  */
          {
            /****************************** 9/11/2008 10:33AM **************************
             * Disable interrupts
             ***************************************************************************/
            save_iec0 = IEC0;
            IEC0 = 0;                   /* Disable heart beat and DMA interrupt */
            if (!Read_Dallas_SN(COMM_ID))         /* Fetch the serial number */
            {
               sts = MB_READ_SERIAL_ERROR;
            } else
            {
              sts = writeTIMarea(0x00, TIM_size);
            }
           /****************************** 9/11/2008 10:35AM **************************
            * restore interrupts
            ***************************************************************************/
           IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */
          }
           break;

          /************************** 6/15/2009 2:17PM ***********************
           * Fetch Trucker Builder Information from the Super TIM
           *******************************************************************/
          case READ_BUILDER_INFO:         /* 0x53 --  */
            begin_time = mstimer;
            /****************************** 9/11/2008 10:33AM **************************
             * Disable interrupts
             ***************************************************************************/
            save_iec0 = IEC0;
            IEC0 = 0;                   /* Disable heart beat and DMA interrupt */
            sts = mbcRdTrBuilderInfo();
            /****************************** 9/11/2008 10:35AM **************************
             * restore interrupts
             ***************************************************************************/
            IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */
            end_time = mstimer;
            break;

          /************************** 6/15/2009 2:19PM ***********************
           * Write Truck Builder Information into the Super TIM
           *******************************************************************/
          case WRITE_BUILDER_INFO:       /* 0x54 --  */
           /****************************** 9/11/2008 10:33AM **************************
            * Disable interrupts
            ***************************************************************************/
            save_iec0 = IEC0;
            IEC0 = 0;                   /* Disable heart beat and DMA interrupt */
            sts = mbcWrBuilderInfo();
            /****************************** 9/11/2008 10:35AM **************************
             * restore interrupts
             ***************************************************************************/
            IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */
            break;

          /************************** 6/22/2009 8:16AM ***********************
           * Fetch info from the Scully reserve area
           *******************************************************************/
          case READ_THIRD_PARTY:         /* 0x55 --  */
          /****************************** 9/11/2008 10:33AM **************************
           * Disable interrupts
           ***************************************************************************/
            save_iec0 = IEC0;
            IEC0 = 0;                   /* Disable heart beat and DMA interrupt */
            sts = readTIMarea(0x080, 0x0FF);
            /****************************** 9/11/2008 10:35AM **************************
             * restore interrupts
             ***************************************************************************/
            IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */
            break;

          /************************** 6/22/2009 8:17AM ***********************
           * Write into the Scully reserve area
           *******************************************************************/
          case WRITE_THIRD_PARTY:         /* 0x56 --  */
          /****************************** 9/11/2008 10:33AM **************************
           * Disable interrupts
           ***************************************************************************/
            save_iec0 = IEC0;
            IEC0 = 0;                   /* Disable heart beat and DMA interrupt */
            sts = writeTIMarea(0x080, 0x0FF);
            /****************************** 9/11/2008 10:35AM **************************
             * restore interrupts
             ***************************************************************************/
            IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */
            break;

          /************************** 6/22/2009 8:16AM ***********************
           * Fetch info from the Scully reserve area
           *******************************************************************/
          case READ_BUILDER_AREA:         /* 0x57 --  */
            /****************************** 9/11/2008 10:33AM **************************
             * Disable interrupts
             ***************************************************************************/
            save_iec0 = IEC0;
            IEC0 = 0;                   /* Disable heart beat and DMA interrupt */
           sts = readTIMarea(0x400, 0xBFF);
          /****************************** 9/11/2008 10:35AM **************************
           * restore interrupts
           ***************************************************************************/
            IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */
            break;

          /************************** 6/22/2009 8:17AM ***********************
           * Write into the Scully reserve area
           *******************************************************************/
          case WRITE_BUILDER_AREA:         /* 0x58 --  */
          /****************************** 9/11/2008 10:33AM **************************
            * Disable interrupts
            ***************************************************************************/
            save_iec0 = IEC0;
            IEC0 = 0;                   /* Disable heart beat and DMA interrupt */
            sts = writeTIMarea(0x400, 0xBFF);
            /****************************** 9/11/2008 10:35AM **************************
             * restore interrupts
             ***************************************************************************/
            IEC0 = save_iec0;           /* Re-enable Heart Beat and DMA interrupts */
            break;

          case INSERT_VEHICLE:     /* 0x59 -- Insert Single Vehicle ID */ 
            sts = mbcInsertTruckID ();
            break;

          case REMOVE_VEHICLE:     /* 0x5A -- Remove Vehicle ID */ 
            sts = mbcRemoveTruckID ();
            break;

          case READ_NUM_PROBES:     /* 0x5B -- Read Number of Probes */
            number_of_Probes = (unsigned int)(calc_tank() - 1);
            sts = mbcPutInt (number_of_Probes);         /* Write "Register" value */
            break;
            
          case USE_UPDATED_ADC_TABLE:       /* 0x5C -- Use updated ADC table for probe counting, 1 = new table, 0 = old table */
              if(pSysDia5->updatedADCTable == 0) {
                  nvSysDia5Update(1);       // Update eeprom block
                  sts = mbcPutByte (0x01);  // Return 01
              }
              
              else {
                  nvSysDia5Update(0);       // Update eeprom block
                  sts = mbcPutByte (0x00);  // Return 00
              }
            break;
            
          case GET_CURRENT_ADC_TABLE:       /* 0x5D -- Return value of selected ADC table for probe counting, 1 = new table, 0 = old table */
              if(pSysDia5->updatedADCTable == 1) {
                  sts = mbcPutByte (0x01);  // Return 01
              }
              
              else {
                  sts = mbcPutByte (0x00);  // Return 00
              }
            break;
            
          default:                      /* Unknown, Illegal, etc. */
            sts = MB_EXC_ILL_FUNC;
            break;
            }
        } /* End not-previously-failed sequence */

    if (modNVflag)                      /* Any Non-Volatile data changes? */
        {                               /* Yes */
        sts1 = nvSysParmUpdate ();      /* Update EEPROM copy */
        if (sts1)
            sts = MB_EXC_MEM_PAR_ERR;
        }

    /* If this is not the last (or only) message in a sequence, then just
       accumulate error/status until the last message in the sequence, then
       deal with the error/status. Meanwhile, just return indicating no
       action required, ready for immediate new message processing (do not
       turn RS-485 line around, etc.). */

    if (ctlbyte & MBC_XMORE)            /* More messages in seq coming? */
    {                               /* Yes */
      *olen = 0;                      /* No generated message data */
      if (stsbyte == MB_OK)           /* Already have outstanding error? */
        stsbyte = (char)sts;        /* No -- queue error for later return */
      return (MB_NO_RESPONSE);        /* No slave response/action just yet */
    }

    /* Just processed last (or only) message in sequence. Time to handle any
       accumulated error/status info (always return first error to occur,
       suppressing all following messages in the sequence until such time as
       we can report the exception status. */

    if ((stsbyte == MB_EXC_ACK)         /* If just an "ACK" queued up, */
        && (sts != MB_OK))              /*  and now have "real" error... */
        stsbyte = (char)sts;            /* Real error wins */

    if (stsbyte != MB_OK)               /* Queued error/status to report? */
        sts = (MODBSTS)stsbyte;         /* Yes, report it over "current" */
    stsbyte = 0;                        /* No queued error/status anymore */

    if (ctlbyte & MBC_XNORESPONSE)      /* Master suppressing slave response? */
        {                               /* Yes */
        ctlbyte &= ~MBC_XNORESPONSE;    /* Clear out sticky flag */
        return (MB_NO_RESPONSE);        /* Toss any error/status */
        }

    if (sts != MB_OK)
        {                               /* Error/Failure, no "data" */
        return (sts);                   /* Propagate error/exception */
        }
    else
        {
        *olen = putcnt;                 /* Set actual data count */
        return (MB_OK);                 /* Return happily */
        }

} /* End of modbus_decode() */

/******************************* 12/30/2008 10:04AM **************************
 * mbcRdTrCompt  --  Function 0x50: Report the Truck Compartment volume
 *
 * Call is:
 *
 *      mbcRdTrCompt ()
 *
 *  mbcRdTrCompt - This routine will fetch which Truck compartment to fetch
 * Verify the number is a valid compartment by verifying it is between 0 and the
 * value stored in the TIM location 0.
 * The volume is a 32 bit word and wimm be sent back. If it is zero then the
 * Compartment wase not configured or no truck is present.
 *****************************************************************************/
static MODBSTS mbcRdTrCompt (void)
{
unsigned char compartment_number;
MODBSTS sts;                          /* Status holding */
int i, index;
union
{
  unsigned char data[4];
  unsigned long lword;
} ul;

  sts = mbcGetByte (&compartment_number); /* Extract the compartment number */
  if (sts)
    return (sts);                     /* Propagate error */

  sts = mbcPutByte (compartment_number);           /* "Echo" back compartment number */
  if (sts)
    return (sts);


  if ((compartment_number < 1) || (compartment_number > Truck_TIM_Configuration[0]))
  {
    for ( i=0; i<COMPARTMENT_SIZE; i++)
    {
      sts = mbcPutByte (0);           /* Send back the indication for invalid entry */
      if (sts)
        return (sts);
    }
  }

  index = ((compartment_number -1) * COMPARTMENT_SIZE) +2;
  ul.data[3] = Truck_TIM_Configuration[index++];
  ul.data[2] = Truck_TIM_Configuration[index++];
  ul.data[1] = Truck_TIM_Configuration[index++];
  ul.data[0] = Truck_TIM_Configuration[index++];

  sts = mbcPutNString (COMPARTMENT_SIZE, (unsigned char *)&ul.lword); /* Return Truck ID */
  if (sts)
    return (sts);
  return (MB_OK);                     /* Return successfully */
}
/************************* End of MODCMD.C **********************************/
