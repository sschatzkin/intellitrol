/*********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         dfile.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009,2014  Scully Signal Company
 *
 *   Description:    Dallas "Touch Data" File-structure operations
 *
 *   Revision History:
 *  Revision   Date   Who   Description of Change Made
 *  --------   -----  ---   --------------------------------------------
 *  1.5.23  04/19/12  KLL  Moved is_truck_pulsing() routine from truckstat.c
 *                           because this is where this routine resides in 
 *                           Intellitrol firmware 1.0.23.
 *                         In the is_truck_pulsing() routine, changed the
 *                           ground check from idle ground check to truck is here
 *                           ground check. Remove some commented code. Changed
 *                           ground check to check for ground even when ground
 *                           checking is disabled.
 *                         In is_truck_pulsing() if a truck has left, instead of
 *                           setting flags which only worked half the time, set
 *                           main state to GONE.
 *  1.5.23  05/02/12  KLL  In is_truck_pulsing() in the timeout section
 *                           removed all the timeout updates and added
 *                           at the end to save code.
 *                          Removed the GND_NO_TEST from the ground check
 *                           this was causing the test to think the truck
 *                           is still here when it has left.
 *  1.5.26  10/09/12  KLL  Changed print statement from %d to %u for lint issue
 *  1.6.34  11/08/16  DHP  Added global truck_pulsing to is_truck_pulsing()
 *                           and timer to set false if no update within 50ms 
 *****************************************************************************/
#include "common.h"
#include "volts.h"   /* A/D voltage definitions */
/****************************************************************************
*****************************************************************************
*
* Data Definitions/declarations
*
*****************************************************************************
****************************************************************************/

#define O_RDONLY	0x00	/* open for reading only */
//#define O_WRONLY 0x01  /* open for writing only */
//#define O_RDWR     0x02  /* open for reading and writing */
//#define O_APPEND 0x08  /* writes done at eof */

//#define O_CREAT    0x10  /* create and open file */
//#define O_TRUNC    0x20  /* open and truncate */
//#define O_EXCL     0x40  /* open only if file doesn't already exist */

/* Structure of a directory name entry */

typedef struct
    {
    char name[4];               /* File name, left justified, blank filled */
    char extension;             /* File "extension" */
    char start;                 /* Start page number of file data */
    char size;                  /* Size (in pages) of file */
    } DF_DIR_ENT;

/* Directory "Control" field (name[0] == 0xAA; first entry of root directory
   block must be a control field). */

typedef struct
    {
    unsigned char rootmark;              /* Control: directory mark (0xAA) */
    unsigned char attributes;            /* Control: attributes field */
    unsigned char flags;                 /* Control: device flags (0x80) */
    unsigned char bitmap[2];             /* Control: page allocation bitmap */
    unsigned char zeroes[2];             /* Control: MBZ */
    } DF_DIR_CTL;

typedef struct
    {
    unsigned char len;
    unsigned char data[28];
    unsigned char next;
    unsigned CRC;
    } DALLASPAGE;

/* Local page buffer used for most "file" operations. This implies that
   callers cannot "mix" file operations -- only one Dallas "file" may be
   open/read(/written someday maybe?) at any one time. */

static DALLASPAGE dfBuf;

/* Local I/O state -- pointer to next data byte to retrieve */

static const char *dfIOPtr;

/* Local I/O state -- count of bytes left in current page (dfBuf.data) */

static int dfIOCount;

/* Local I/O state -- dfIOPage1 is remembered pointer to first page,
   dfIOPagen pointer to next page to read, or 0 if no more pages */

static char dfIOPagen;

/* Local I/O State -- Dallas I/O Port for I/O. If non-zero, then a "file"
   is currently "open" on that Dallas device. */

static unsigned char dfIOPort;

/****************************************************************************
*****************************************************************************
*
* Low-level "Block"-structured Dallas operations
*
*****************************************************************************
****************************************************************************/

/****************************************************************************
* dfGetPage -- Get (read) a "page" from the Dallas chip
*
* Call is:
*
*   dfGetPage (chan, pnum, buf)
*
* Where:
*
*   "chan" is the Dallas I/O channel (PORTMC bit) used to communicate with
*   the Dallas chip;
*
*   "pnum" is the physical page or block number to retrieve;
*
*   "buf" is a pointer to a Dallas-page-sized buffer to receive the page.
*
* The data page is CRC-verified (as per "suggested" by the Dallas Book of
* DS19xx Touch Memory Standards).
*
* On success, 0 is returned and the specified buffer contains the requested
* page of Dallas data. On error, a non-zero error status is returned.
***************************************************************************/

static char dfGetPage
    (
    unsigned char port,                  /* PORTMC data "port" for Dallas comm */
    char pnum,                  /* Dallas "page" number to read */
    DALLASPAGE *buf             /* Buffer to receive data page */
    )
{
    unsigned char *ptr;                  /* Scratch byte pointer */
    unsigned crc;                   /* Computed CRC-16 */
    unsigned char addrMSB, addrLSB;      /* High/Low byte address of page */
    unsigned char sts;                   /* Local status */
    unsigned int i;                   /* Local index */
    unsigned char low_byte;
    unsigned char high_byte;

    /* Always reset the Dallas chip into a known state ("ROM" or "Network"
       layer access) */

    sts = Dallas_Reset (port);          /* Nail the Dallas chip */
    if (!sts)
        return ((char)DRNOPRESENCE);          /* No Presence Pulse */

    /* Skip the ROM stuff (already read if interested), advance to the
       so-called "Transport" layer to access the non-ROM segments */

    sts = Dallas_Byte (0xCC, port);     /* Send SKIP_ROM command */
    if (sts != 0xCC)
        return ((char)DWFAIL);                /* Write command failed */

    /* Now ready to read the Dallas non-volatile SRAM storage */

    addrLSB = (unsigned char)(pnum * (int)sizeof(DALLASPAGE));
    addrMSB = (unsigned char)((unsigned char)pnum >> 3);

    sts = Dallas_Byte (0xF0, port);     /* Send READ_MEMORY command */
    if (sts != 0xF0)
        return ((char)DWFAIL);                /* Write command failed */

    sts = Dallas_Byte (addrLSB, port);  /* Send LSB of address */
    if (sts != addrLSB)
        return ((char)DWFAIL);                /* Write command failed */

    sts = Dallas_Byte (addrMSB, port);  /* Send MSB of address */
    if (sts != addrMSB)
        return ((char)DWFAIL);                /* Write command failed */

    /* Read in one page's worth (32 bytes) of data, as if the Dallas chip
       were a block-structured device... */

    ptr = (unsigned char *)buf;

    for (i = 0; i < sizeof(DALLASPAGE); i++)
    {
      ptr[i] = Dallas_Byte (0xFF, port); /* Extract next Dallas byte */
    }

    /* Now verify the validity of the Dallas page's worth of data */

    i = buf->len;                       /* Page data length */
    if (i >= (sizeof(buf->data) -1))                   /* 28 Bytes Data, "Next" pointer */
        return ((char)TCHIPLENBYT);           /* Bad format (junk length byte) */

    crc = modbus_CRC ((unsigned char *)buf,      /* Dallas page data, en toto */
                      i + 1,            /* Include len byte */
                      (unsigned short)(unsigned char)pnum);            /* Seed with page number */

    crc = ~crc;  /* This fixes a sign problem */
    low_byte = (unsigned char)crc;
    high_byte = (crc >> 8);
    if ((low_byte != (unsigned char)buf->data[i]) /* Verify byte-swapped and */
        || (high_byte != (unsigned char)buf->data[i+1])) /*  complemented CRC-16 */
        return ((char)DRAMCRC);               /* CRC-16 Error in "Dallas RAM Data" */

    return (0);                         /* Dallas Page successfully read */

} /* End of dfGetPage() */

/****************************************************************************
*****************************************************************************
*
* High-level File operations
*
*****************************************************************************
****************************************************************************/

/****************************************************************************
* dfOpenFile -- locate and open for reading a Dallas "file"
*
* Call is:
*
*   sts = dfOpenFile (port, name, oflag)
*
* Where:
*
*   "port" is the PORTMC bit/channel identifier (mask, as in COMM_ID)
*   specifying where the Dallas chip resides for underlying bit I/O
*   operations.
*
*   "name" is the 5-character name string (which kinda sorta maps out as a
*   4-character name and a 1-character "extension" a la "EXON.D" -- the
*   "." separator is implicit...)
*
*   "oflag" specifies the file operations (read, write) to be performed. At
*   present, only O_RDONLY is supported...
*
* For simplicity' sake, the name must exactly (case, blanks, nulls, etc.)
* match the 5-character Dallas Directory Entry field.
*
* On error, a non-zero error status is returned, otherwise a zero "success"
* status is returned and the named "file" is ready for I/O.
*
* Since this SFPOS (use your imagination) compiler won't let me use a half-
* way decent set of structs to overlay the data, just do it inline by hand.
* Deep Heavy Sigh.
****************************************************************************/

char dfOpenFile
    (
    unsigned char port,               /* PORTMC port for Dallas operations */
    const char *name,                 /* File name */
    char oflag                  /* File operations */
    )
{
    DF_DIR_ENT *pDirent;        /* File entry in directory */
    DF_DIR_CTL *pDirctl;        /* "Control" field in directory */

    char *ptr;                  /* Random scratch pointer */

    unsigned char len;                   /* Length of data */
    char filepage;              /* File page, if any */
    char sts;                   /* Local status */
    int loop = 1;

    /* Make sure our caller is as simple-minded as we are... */

    if (oflag != O_RDONLY)              /* Only "Read" supported */
        return ((char)TCHIPWRITELOCK);        /* Ergo report write protected */

    /* Initialize pointers ("Close" any previous "Open" file) */

    dfCloseFile();

    /* First thing to do is read the "root" directory block */

    sts = dfGetPage (port, 0, &dfBuf);
    if (sts)
        return (sts);

    len = dfBuf.len;                    /* Extract "data" length */
    ptr = (char *)dfBuf.data;           /* Extract data from page */

    if ((len < (sizeof(DF_DIR_ENT)))    /* Big enuf to be usefull? */
        || (len > 29))                  /* But not too big... */
        return ((char)TCHIPLENBYT);           /* Bad root block size */

    /* First "entry" is always control field */
    dummy_func((unsigned char *)&dfBuf.next);
    dummy_func((unsigned char *)&dfBuf.CRC);

    pDirctl = (DF_DIR_CTL *)ptr;        /* Control field entry */

    if (pDirctl->rootmark != 0xAA)      /* Look like root block mark? */
        return ((char)TCHIPFORMAT);           /* Improper format */

    if (pDirctl->flags != 0x80)         /* Still look like root block? */
        return ((char)TCHIPFORMAT);           /* Improper flags */

    if ((pDirctl->zeroes[0] != 0x00)    /* Are we really */
        || (pDirctl->zeroes[1] != 0x00)) /*   really sure? */
        return ((char)TCHIPFORMAT);           /* MBZ bytes not Z'ed */

    /* Root directory block looks OK, walk the directory list looking for
       the named file entry */
    dummy_func((unsigned char *)&pDirctl->attributes);
    dummy_func((unsigned char *)pDirctl->bitmap);

    filepage = 0;                       /* No file found yet */

    while (loop == 1)
    {
      ptr += sizeof(DF_DIR_ENT);      /* Advance "entry" pointer */
      len -= sizeof(DF_DIR_ENT);      /* Count down bytes remaining */
      if (len & 0x80)                 /* "Overflow"? */
          return ((char)TCHIPLENBYT);       /* Junk length */

      if (len == 1)                   /* Used up all entries on this page? */
          {                           /* Yes */
          /* This directory page does not have the requested file, try
             the next page in the directory.

             This relies on the "Cont-Pointer" field always immediately
             following the last Control/File-entry field (i.e., being the
             "len'th" data byte in the page). SFPOS compiler . . . */

          if (*ptr == 0)              /* Is there a next page? */
              break;                  /* No, file not found (filepage = 0) */

          sts = dfGetPage (port, *ptr, &dfBuf);
          if (sts)
              return (sts);

          len = dfBuf.len;            /* New page's data length */
          ptr = (char *)dfBuf.data;   /* New page's data */
          }

      pDirent = (DF_DIR_ENT *)ptr;    /* Next file entry in directory */

      if (pDirent->name[0] == name[0])
      {
        if ( pDirent->name[1] == name[1])
        {
          if ( pDirent->name[2] == name[2])
          {
            if ( pDirent->name[3] == name[3])
            {
              if ( pDirent->extension == name[4])
              {
                filepage = pDirent->start;  /* Start page of file */
                break;
              }
            }
          }
        }
      }
      dummy_func((unsigned char *)&pDirent->size);  /* Must keep lint happy */

    } /* End while loop searching all directory pages */

    if (filepage == 0)                  /* Find the file? */
        return ((char)FILENOTFND);            /* No - file not found */

    /* Prime the I/O engine... */

    dfIOPort = port;                    /* Dallas I/O Port to read */
    dfIOPagen = filepage;               /* Next I/O data page */
    dfIOCount = 0;                      /* Current count exhausted */
    return (0);                         /* Success, ready to read file */

} /* end dfOpenFile() */

/****************************************************************************
* dfGetByte -- get next Dallas file data byte
*
* Call is:
*
*   sts = dfGetByte (chptr)
*
* Where:
*
*   "chptr" is the pointer to receive the retrieved data byte, if any.
*
* dfGetByte() can be called after a successful call to dfOpenFile() has
* located a file to be read from the Dallas "file" structure, and the
* local I/O control has been initialized. In particular, dfGetByte() only
* reads from the "current" file (as specified via dfOpenFile()) -- you
* cannot have multiple Dallas files open concurrently...
*
* On error, a non-zero error status is returned, otherwise a zero "success"
* status is returned and the next data byte is stored as specified by the
* chptr pointer.
*
* At present, no distinction is made between "EOF", read/CRC errors, and
* the like -- it works or it doesn't.
****************************************************************************/

char dfGetByte
    (
    char *chptr                 /* Pointer to store retrieved data byte */
    )
{
    int len;                   /* Length byte */
    char sts;                   /* Local status */

    if (dfIOPort == 0)                  /* Channel for "Open" file? */
        return ((char)FILENOTOPEN);           /* No -- caller confused */

    while (dfIOCount == 0)              /* Any bytes left on current page? */
        {                               /* No */
        if (dfIOPagen == 0)             /* Any pages left in current file? */
            {                           /* No */
            break;                      /* Handle/return end of file */
            }

        /* Step to next data page in Dallas file */

        sts = dfGetPage (dfIOPort, dfIOPagen, &dfBuf);
        if (sts)                        /* If error, no need to clean up, */
            return sts;                 /*  as this error state is "latched" */

        len = dfBuf.len;                /* Count of "data" bytes + cont ptr */
        if (len >= (char)sizeof(dfBuf.data))                   /* Viable length? */
            return ((char)TCHIPLENBYT);       /* Junk length */
        if (len == 0)                   /* Totally empty? (legal?) */
            {                           /* Yes -- allowed or not... */
            dfIOPagen = 0;              /* Therefore, no next data page */
            break;                      /* Return end of file */
            }

        /* Looks OK, setup for extracting file data from this page */

        len--;                          /* Discount "Cont-Pointer" data byte */
        dfIOPagen = (char)dfBuf.data[len];    /* Pre-extract next page number */
        dfIOCount = len;                /* The rest is "user data" */
        dfIOPtr = (char *)&dfBuf.data[0];           /* Starting with first "data" byte */

      } /* End while dfIOCount */

    /* Retrieve and return next data byte in Dallas file */

    if (dfIOCount)                      /* Any bytes left on current page? */
        {                               /* Yes (easy case) */
        *chptr = *dfIOPtr++;            /* Stash retrieved data byte */
        dfIOCount--;                    /* One less byte left */
        return (0);                     /* Success */
        }
    else                                /* Any bytes left on current page? */
        {                               /* No -- End of File case */
        *chptr = 0;                     /* Return <NUL> data byte */
        return ((char)TCHIPEOF);              /* End of File */
        }

} /* End of dfGetByte() */

/****************************************************************************
* dfCloseFile -- terminate access to open Dallas file
*
* Call is:
*
*   sts = dfCloseFile (port)
*
* Where:
*
*   "port" is the PORTMC bit/channel identifier (mask, as in COMM_ID)
*   specifying where the Dallas chip resides for underlying bit I/O
*   operations.
*
* dfCloseFile() "cleans up" after a dfOpenFile() call. Basically, it just
* blasts all the pointers to the Dallas device file structure...making
* sure dfGetByte() (etc.) calls will fail. It's mostly here on G.P.s...
*
* On error, a non-zero error status is returned, otherwise a zero "success"
* status is returned.
*
* At present, no errors are possible... (write not implemented, so no data
* to "flush out", etc., so port is ignored.)
****************************************************************************/

void dfCloseFile (void)
{
    dfIOCount = 0;                      /* No more data in page buffer */
    dfIOPagen = 0;                      /* No more "next" page */
    dfIOPort = 0;                       /* No more Dallas device for I/O */
} /* End of dfCloseFile() */

/****************************************************************************
*****************************************************************************
*
* High-level "DateStamp" operations
*
* In all cases, caller is responsible for interlocking wrt active_comm (e.g.,
* GroundBolt testing, etc.)
*
*****************************************************************************
****************************************************************************/

static const unsigned char chxtab[16] =
{   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46 };

/* Current "state" for decrypting DateStamp file bytes */

static char dsXsn, dsXpsw;
static const unsigned char *dsXpswp;

char dsOpenFile
    (
    unsigned char port,                  /* PORTMC port for Dallas operations */
    const char *name,           /* File name */
    const unsigned char *psw,                  /* File password */
    char oflag                  /* File operations */
    )
{
    char sts;                   /* Local status */

    /* Initialize decryption state */

    dsXsn = 0;                  /* Index zero for serial number */
    dsXpsw = 0;                 /* Index zero for password */

    sts = dfOpenFile(port, name, oflag); /* Open the file */
    if (sts)
        return (sts);

    dsXpswp = psw;              /* Saved password base pointer */

    return (0);

} /* End of dsOpenFile() */

/*****************************************************************/
/*****************************************************************/

void dsCloseFile ()
{
    dfCloseFile ();
} /* End dsCloseFile() */

/*****************************************************************/
/*****************************************************************/

char dsGetByte
    (
    char *chptr                 /* Pointer to return DateStamp byte */
    )
{
    char ch;
    unsigned chx;               /* Local copy of char */
    char i;
    char sts;                   /* Local status */

    sts = dfGetByte (&ch);              /* Read next DateStamp file byte */
    if (sts)                            /* Errors? */
        return (sts);               /* Propagate error status */

    /* Now decrypt the data byte in ch */

    i = (char)(dsXsn++ & 0x03);         /* Advance serial number state */

    if (i & 0x02)                       /* Even or odd pair ? */
        chx = truck_SN[5];           /* Odd ==> SN nibbles 10 & 11 */
    else
        chx = truck_SN[4];           /* Even => SN nibbles 08 & 09 */

    if (i & 0x01)                       /* Even or odd nibble? */
        chx &= 0x0F;                    /* Odd ==> SN nibbles 09 & 11 */
    else
        chx >>= 4;                      /* Even => SN nibbles 08 & 10 */

    chx = chxtab[(int)chx];                  /* Map into XOR pattern 1 */

    i = ++dsXpsw;                       /* Advance password state */
    if (i > dsXpswp[0])                 /* Time to wrap? */
        {                               /* Yes */
        i = 1;                          /* Wrap back */
        dsXpsw = i;                     /* Set state too */
        }

    chx ^= dsXpswp[(int)i];

    ch -= (char)(chx & 0x3F);           /* Decrypt char in ch */
    if ((ch < '0') || (ch > '9'))       /* Valid numberic character? */
        return ((char)DSBADCHAR);             /* INVALID DateStamp data character */

    *chptr = ch;                        /* Return decrypted DateStamp char */

    return (0);

} /* End of dsGetBtye() */

/*****************************************************************/
/*****************************************************************/

char dsGetDecn
    (
    char size,                  /* How big of a decimal int to extract */
    unsigned *number                /* Where to stash the extracted integer */
    )
{
    unsigned num;                   /* Local for faster access */
    char ch;                    /* Local character holder */
    char i;                     /* Loop counter */
    char sts;                   /* Local status */

    num = 0;

    /* Extract the next "n" decimal digits (characters) from the Dallas file
       and build a decimal-integer out of them */

    for (i = size; i > 0; i--)
        {
        sts = dsGetByte (&ch);          /* Next DateStamp file character */
        if (sts)
            return (sts);

        if ((ch < '0') || (ch > '9'))
            return ((char)DSBADCHAR);         /* INVALID decimal character */

        num = (num * 10) + (unsigned int)((unsigned char)ch - '0');
        }

    *number = num;

    return (0);                         /* Successful return with number */

} /* End dsGetDecn() */

/*****************************************************************/
/*****************************************************************/

char dsTruckValidate
    (
    unsigned char port,                  /* PORTMC data port (bit) */
    const char *name,                 /* 5-character Dallas file name */
    const unsigned char *psw                   /* n-character password */
    )
{
    unixtime dsexp;
    unsigned dsmon, dsday, dsyear, dshour, dsmin;
    char sts;

    sts = dsOpenFile (port, name, psw, O_RDONLY);
    if (sts)
        return (sts);

    UNIX_to_Greg ();                    /* Update year/month/day/hour/minute */

    sts = dsGetDecn (2, &dsmon);        /* Extract month from Date Stamp */
    if (sts)                            /* dsGetDecn (2, &dsmon) returns 0 on succesful read */
        {
        dsCloseFile ();
        return (sts);
        }

    sts = dsGetDecn (2, &dsday);        /* Extract day-of-month */
    if (sts)
        {
        dsCloseFile ();
        return (sts);
        }

    sts = dsGetDecn (4, &dsyear);       /* Extract full year */
    if (sts)
        {
        dsCloseFile ();
        return (sts);
        }

    sts = dsGetDecn (2, &dshour);       /* Extract hour-of-day */
    if (sts)
        {
        dsCloseFile ();
        return (sts);
        }

    sts = dsGetDecn (2, &dsmin);        /* Extract minute-within-hour */
    if (sts)
        {
        dsCloseFile ();
        return (sts);
        }

    dsexp = Greg_to_UNIX (dsyear, dsmon, dsday, dshour, dsmin);
    if ((dsexp == 0)                    /* Should always convert... */
        || (dsexp < present_time))               /* DateStamp expired yet? */
        {                               /* Yes */
        dsCloseFile ();
        return ((char)DSEXPIRED);             /*  DateStamp has expired! */
        }

    printf("\n\r\n\rTruck expiration date: %02u/%02u/%02u @ %02u%02u\n\r\n\r",dsmon,dsday,dsyear,dshour,dsmin);
    sts = dsGetDecn (3, &dsmon);        /* Extract first authorized terminal */
    if (sts)                            /* Must be at least one terminal id */
        {                               /* If not, bad format file */
        dsCloseFile ();
        return (sts);
        }

    if (SysParm.Terminal != 999)        /* Special "wildcard" accept anything? */
        {                               /* No, need match from file */
        while (sts == 0)                /* Loop through all terminal ids */
            {                           /* in the Dallas DateStamp file */
            if ((dsmon == SysParm.Terminal) /* Valid for this terminal? */
                || (dsmon == 999))      /* Or is valid for all terminals? */
                break;                  /* Truck is authorized */
            sts = dsGetDecn (3, &dsmon); /* Extract next terminal ID, 0 on succesful read */
            }
        }

    dsCloseFile ();                     /* Done with Dallas DateStamp file */

    if (sts == (char)TCHIPEOF)                /* Not found in terminal list? */
        sts = (char)INVALID;                  /* Yes, not authorized here */

    return (sts);                       /* Return authorization */

    } /* End of dsTruckValidate() */

/******************************* 2/25/2009 3:18PM ****************************
 *
 *  FUNCTION:       is_truck_pulsing()
 *
 *  PARAMETERS:     None
 *
 *  DESCRIPTION:
 *    1. If truck_state == OPTIC_FIVE then return, shouldn't have been called
 *    2. If any of the probes are pulsing update truck_pulsing, probe_pulse_old and timers, then return
 *    3. If  > 50ms (max pulse period) since pulsing last seen then set truck_pulsing = FALSE
 *    4. If 5 seconds since pulsing last seen then read the probe voltages for no truck levels - if so then
 *        set the acquire_state as gone and probe_try_state as NO_TYPE
 *  Note: probe_pulse is updated in convert_to_binary() every 1ms
 *  RETURNS:        None
 *****************************************************************************/
void is_truck_pulsing()
{
int truck_status = TRUE;
unsigned int index;
unsigned int jump_start_save;
static unsigned long  time_2_check = 0;

  /****************************** 7/27/2011 6:42AM ***************************
   * This is for two wire problem. Just leave if 5 wire
   ***************************************************************************/
  if (truck_state == OPTIC_FIVE)
  {
    return;               /* Assume a truck is connected */
  }
  if ( probe_pulse_old != probe_pulse)
  {
    truck_pulsing = TRUE;
    probe_pulse_old = probe_pulse;
    pulse_timeout = (read_time() + 5000L);
    time_2_check = (read_time()+50L);
  }
  else if (time_2_check < read_time() )
  {
    truck_pulsing = FALSE;
    /************************** 2/25/2009 1:35PM ***********************
     * If no pulses within 5 seconds then either all probes are WET or
     * Truck has left
     *******************************************************************/
    if ( pulse_timeout < read_time())
    {
      {
        jump_start_save = JUMP_START;       /* JUMP_START currently enabled ? */
        JUMP_START = SET;                   /* Turn on the 20v */
        PULSE5VOLT = CLR;                   /* Chan 4 normal (10V) drive */
        LATE |= 0xFF;                       /* assure power on all channels */
        // last_routine = 0x8E;

        set_mux( M_PROBES );          /* assure mux is selecting probes */
        DelayUS(1000);                       /* allow JUMP_START's +20 to ramp up */
        if (read_ADC() == FAILED)   /* see what the probes are doing now */
        {
          return;
        }
        if (ConfigA & CFGA_8COMPARTMENT)
        {
          index = 0;
        }
        else
        {
          index = 2;
        }
        service_charge();             /* Keep Service LED off */
        for ( ; index<8; index++)
        {
          if (ConfigA & CFGA_EURO8VOLT)           /* "European 10 Volt" jumper?? */
          {                                       /* Yes, limited voltages... */
            if ((probe_volt[index] > ADC1V)       /* If current channel NOT grounded */
              && ((probe_volt[index] + ADC1V)     /* but it does show */
              < open_c_volt[1][index]))          /* a 250mV voltage "drop" */
            {
              truck_status = FALSE;               /* Assume a truck is connected */
              break;
            }
          } else
          {
            if ((probe_volt[index] > ADC1V)       /* If current channel NOT grounded */
              && ((probe_volt[index] + ADC4V)     /* but it does show */
              < open_c_volt[1][index]))           /* a 4 volt voltage "drop" */
            {
              truck_status = FALSE;               /* Assume a truck is connected */
              break;
            }
          }
        }
        JUMP_START = jump_start_save;                /* Restore the JUMP_START state */
      }

      if (short_6to4_5to3() )   /*  3,5,8 & 4,6,7 */
      {
        truck_status = FALSE;               /* Assume a truck is connected */
      }

      badgndflag = CheckGroundPresent(FALSE);    /* Test if we can see ground */

      if (badgndflag == GND_OK)                  /* If "Good" (00) ground, */
      {
        truck_status = FALSE;               /* Assume a truck is connected */
      }

      if (Read_Truck_Presence() == 1)            /* And look for an active TIM */
                                                 /* the Reset/Presence pulse ONLY */
      {
        truck_status = FALSE;               /* Assume a truck is connected */
      }

      if ( truck_status == TRUE)
      {
          xprintf( 47, DUMMY );          /* Force the next message to print */
          set_main_state (GONE);              /* Set up to enter idle state */
      }
      pulse_timeout = (read_time() + 500L);
    }
  }
}

/*********************** end of DFILE.C **********************************/
