/****************************************************************************
*
*   Project:        Rack Controller
*
*   Module:         nvtruck.c
*
*   Revision:       REV 1.5
*
*   Author:         Ken Langlais  @Copyright 2009  Scully Signal Company
*
*   Description:    Main program Non-Volatile storage (EEPROM) routines for
*                   Truck ID and Bypass Key data.
*
* $Log:   Q:\INTELTRL\68HC16\VCS\nvtruck.C_v  $
 *
 *    Rev 1.5   23 Sep 2008 07:58:32   KLANGLAIS
 * Rev. 1.5, porting old Intellitrol code to the PIC24HJ256GP210 cpu
 *
 *    Rev 1.4   29 Sep 1997 11:16:30   unknown
 * Rev. 1.05E, changed Ground Test.
 *
 *    Rev 1.3   14 Jul 1997 14:08:18   MRIVIN
 * Intellitrol Rev. 1.05 prerelease
 *
 *    Rev 1.2   28 Apr 1997 17:03:40   MRIVIN
 * Incorporated changes in ADC.C, some cleanup of commented out code used
 * for debugging.
 *
 *    Rev 1.0 Robert D. Houk
 * Original
 * 1.6.32  04/12/15  DHP  In nvTrkErase() changed return code on error to MB_EXC_FAULT;
 *                                           from what on invalid partition decoded as MB_EXC_ILL_FUNC 
****************************************************************************/

#include "common.h"
//#include "stsbits.h"            /* StatusA/B/O/P bits */
//#include "modbus.h"             /* ModBus-specific definitions */
//#include "esquared.h"           /* EEPROM/NonVolatile definitions */
//#include "volts.h"              /* SysVolts stuff */


/* A convenient "erased flash/EEPROM" character pattern */

static const char erased[] = { "\377\377\377\377\377\377\377\377" };

/****************************************************************************
*****************************************************************************
*
* Higher-level Partition services
*
*****************************************************************************
****************************************************************************/

/****************************************************************************
*
* Bypass Key Partition services
*
****************************************************************************/

/****************************************************************************
* nvKeyEmpty -- Find empty bypass key slot in NonVolatile store
*
* Call is:
*
*   nvKeyEmpty (index)
*
* Where:
*
*   "index" is pointer to unsigned integer to receive index of the first
*   free ("empty" == "erased" == 0xFFFFFFFFFFFF) if found.
*
* On successful location of an empty/available key slot, nvKeyEmpty()
* returns zero (with the matching key index stored as requested); On error,
* an EE_status error value is returned.
****************************************************************************/

char nvKeyEmpty
    (
    word *index                 /* Pointer to return matching index */
    )
{
//    E2KEYREC *keyarray;         /* Base pointer to NV Key store */
    E2KEYREC key_store;         /* Temp storage of current key eeprom entry */
    word key_address;
    word keybase;               /* Bypass key partition offset */
    word size;                  /* Size of NV Key store */

    unsigned char *op;                   /* Scratch pointer */
    unsigned int keymax, i;
    signed char k;
    char sts;

  // last_routine = 0x4D;
    sts = eeMapPartition (EEP_KEY, &size, &keybase);
    if (sts)
        return (sts);

    key_address = KEY_BASE;
//    keyarray = (E2KEYREC *)&key_store;  /* Point to key storage area */

    /* Search the Bypass Key array. It's always fairly small, so ultimate
       searching speed is not a big issue here. One might even argue that
       we should return the first free slot after the last used slot (i.e.,
       keep sliding up in memory) in order to even out the EEPROM usage.
       'Twould be an interesting one+, eh? */

    keymax = size/sizeof(E2KEYREC);
    for (i = 0; i < keymax; i++)
        {
        if ((sts = eeReadBlock(key_address, (unsigned char *)&key_store, sizeof(E2KEYREC))) != 0) return sts;
  // last_routine = 0x4D;
        key_address += sizeof(E2KEYREC);  /* bump address for next time through */
        op = key_store.Key + (BYTESERIAL-1); /* Latest stored key to compare */
        for (k = (signed char)(BYTESERIAL-1); k >= 0; k--) /* Match from LSB to MSB */
            {
            if (*op-- != (unsigned char)0xFF)          /* keyarray[i].Key[k] != "erased" */
                break;                  /* This stored key doesn't match */
            if (k == 0)                 /* If all 6 "digits" match */
                {                       /* This key is a match! */
                *index = i;             /* Return matching index */
                if (key_store.CRC == 0xFFFF) /* *ALL* bytes erased? */
                    return (0);         /* Return with successful match */
                /* Else just ignore "bad" entry */
                }
            } /* End compare one key */
        } /* End search all keys */

    return (-1);

} /* End nvKeyEmpty() */

/****************************************************************************
* nvKeyFind -- Find bypass key in NonVolatile store
*
* Call is:
*
*   nvKeyFind (key, index)
*
* Where:
*
*   "key" is the pointer to a 6-digit Dallas key "serial number" string;
*
*   "index" is pointer to unsigned integer to receive index if found.
*
* On successful match, nvKeyFind() returns zero (with the matching key index
* stored as requested); On error, an EE_status error value is returned.
****************************************************************************/

char nvKeyFind
    (
    unsigned char *key,                  /* Pointer to 6-digit Dallas ser no */
    word *index                 /* Pointer to return matching index */
    )
{
//E2KEYREC *keyarray;         /* Base pointer to NV Key store */
E2KEYREC key_store;         /* Temp storage of current key eeprom entry */
word key_address;
word keybase;               /* Bypass key partition offset */
word size;                  /* Size of NV Key store */

  unsigned char *ip, *op;              /* Scratch pointers */
  unsigned int keymax, i;
  int k;
  word crc;
  char sts;

  // last_routine = 0x4E;
  sts = eeMapPartition (EEP_KEY, &size, &keybase);
  if (sts)
      return (sts);

  key_address = KEY_BASE;
//  keyarray = (E2KEYREC *)&key_store;  /* Point to key storage area */

  /* Search the Bypass Key array. It's always fairly small, so ultimate
     searching speed is not a big issue here */

  keymax = size/sizeof(E2KEYREC);
  for (i = 0; i < keymax; i++)
  {
    if ((sts = eeReadBlock(key_address, (unsigned char *)&key_store, sizeof(E2KEYREC))) != 0) return sts;
    service_charge();             /* Keep Service LED off */

  // last_routine = 0x4E;
    key_address += sizeof(E2KEYREC);  /* bump address for next time through */
    ip = key;                       /* Key to find */
    op = key_store.Key;           /* Latest stored key to compare */

    for (k = (BYTESERIAL-1); k >= 0; k--)  /* Match from LSB to MSB */
    {
      if (ip[k] != op[k])         /* key[k] != keyarray[i].Key[k] */
        break;                  /* This stored key doesn't match */
    } /* End compare one key */

    if (k < 0)                      /* If all 6 "digits" matched */
    {                           /* This key is a match! */
      *index = i;                 /* Return matching index */
      crc = modbus_CRC (key, BYTESERIAL, INIT_CRC_SEED); /* Calculate CRC of key */
      if (crc == key_store.CRC)
        return (0);             /* Return with successful match */
        /* Else just ignore "bad" entry */
    }
  } /* End search all keys */

  printf("\n\r*** Bypass Key not found in EEPROM ***\n\r");
  return (-1);

} /* End nvKeyFind() */

/****************************************************************************
* nvKeyGet -- Retrieve ("get") bypass key from NonVolatile Key store
*
* Call is:
*
*   nvKeyGet (key, index)
*
* Where:
*
*   "key" is the pointer to a 6-digit Dallas key "serial number" string
*   to be filled in with the specified key index;
*
*   "index" is the NonVolatile Key store index to retrieve.
*
* On successful match, nvKeyGet() returns zero, having copied the  Non-
* Volatile Key entry stored at the specified index into the caller's key
* buffer (assumed to be large enough!). If the NonVolatile copy is "erased"
* (all 0xFF's), then the returned key will be all zeroes. On error, an
* EE_status error value is returned.
****************************************************************************/

char nvKeyGet
    (
    unsigned char *key,                  /* Pointer to 6-digit Dallas ser no */
    word index                  /* Pointer to index at which to insert */
    )
{
//    E2KEYREC *keycrc;           /* NonVolatile store key record */
    E2KEYREC key_store;         /* Temp storage of current key eeprom entry */
    word key_address;
    int csts;                   /* For memcmp() and the compiler */
    word base;                  /* Base offset of Key partition */
    word size;                  /* Size (bytes) of Key partition */
    word crc;                   /* Alleged CRC of copied key */

    char sts;

  // last_routine = 0x4F;
    sts = eeMapPartition (EEP_KEY, &size, &base);
    if (sts)
        return (sts);

    if ((index * sizeof(E2KEYREC)) > size) /* Legal index? */
        return (-1);                    /* No */

    /* Generate access pointer to nvKey key record */

//    keycrc = (E2KEYREC *)&key_store;
    key_address = KEY_BASE + (unsigned int)((unsigned int)index * (unsigned int)sizeof(E2KEYREC));
    if ((sts = eeReadBlock(key_address, (unsigned char *)&key_store, sizeof(E2KEYREC))) != 0) return sts;
  // last_routine = 0x4F;

    memcpy (key, (unsigned char *)&key_store, BYTESERIAL); /* Copy key to caller's buffer */

    /* If the key is "erased", return null key to user */
    csts = memcmp (key, erased, BYTESERIAL); /* Erased? */
    if ((csts == 0)                     /* Key bytes erased? */
        && (key_store.CRC == 0xFFFF))     /* *ALL* bytes erased? */
        {                               /* Yes */
        key[0] = 0x00;                  /* We all know that BYTESERIAL */
        key[1] = 0x00;                  /*   is 6, and this is much faster */
        key[2] = 0x00;                  /*   than a call to memset(), */
        key[3] = 0x00;                  /*   and takes up *HALF* as many HC16 */
        key[4] = 0x00;                  /*   instruction bytes as the call! */
        key[5] = 0x00;                  /*   ... */
        return (0);                     /* Return "empty" key */
        }

    /* Appears to be real key, verify key contents are valid */

    crc = modbus_CRC (key,
                      BYTESERIAL,
                      INIT_CRC_SEED);   /* Calculate corresponding CRC */
    if (crc != key_store.CRC)             /* CRC match? */
        {                               /* No */
        /* VIPER-II currently (released version) doesn't deal with our
           returning a CRC ("Memory Parity Error") very well. As an ex-
           pedient concession, just return "empty" slot as error. */

        key[0] = 0x00;                  /* We all know that BYTESERIAL */
        key[1] = 0x00;                  /*   is 6, and this is much faster */
        key[2] = 0x00;                  /*   than a call to memset(), */
        key[3] = 0x00;                  /*   and takes up *HALF* as many HC16 */
        key[4] = 0x00;                  /*   instruction bytes as the call! */
        key[5] = 0x00;                  /*   ... */
        return (0);                     /* Don't confuse VIPER */
        }

    return (0);                         /* Successful return */

} /* End nvKeyGet() */

/****************************************************************************
* nvKeyGetMany -- Retrieve multiple bypass keys from NonVolatile Key store
*
* Call is:
*
*   nvKeyGetMany (key, index, cnt)
*
* Where:
*
*   "key" is the pointer to a 6-digit Dallas key "serial number" string
*   to be filled in with the specified key index;
*
*   "index" is the NonVolatile Key store index to retrieve;
*
*   "cnt" is the cnt of consecutive keys to retrieve.
*
* On successful match, nvKeyGet() returns zero, having copied the Non-
* Volatile Key entries stored at the specified index into the caller's key
* buffer (assumed to be large enough!). If the NonVolatile copy is "erased"
* (all 0xFF's), then the returned key will be all zeroes. On error, an
* EE_status error value is returned.
****************************************************************************/

char nvKeyGetMany
    (
    unsigned char *key,                  /* Pointer to 6-digit Dallas ser no */
    word index,                 /* Pointer to index at which to insert */
    unsigned char cnt                  /* Count of Keys to return */
    )
{
//    E2KEYREC *keycrc;           /* NonVolatile store key record */
    E2KEYREC key_store;         /* Temp storage of current key eeprom entry */
    word key_address;
    unsigned char *ptr;                  /* Handy dandy copy pointer */
    int csts;                   /* For memcmp() and the compiler */
    word base;                  /* Base offset of Key partition */
    word size;                  /* Size (bytes) of Key partition */
    word crc;                   /* Alleged CRC of copied key */

    word i;
    char sts;

  // last_routine = 0x50;
    sts = eeMapPartition (EEP_KEY, &size, &base);
  // last_routine = 0x50;
    if (sts)
        return (sts);

    if (((index + cnt) * sizeof(E2KEYREC)) > size) /* Legal index? */
        return (-1);                    /* No */

    ptr = key;                  /* Scratch pointer to caller's buffer */
//    keycrc = (E2KEYREC *)&key_store;

    for (i = 0; i < cnt; i++)
        {
        /* Generate access pointer to nvKey key record */

        key_address = KEY_BASE + (word)((word)(index+i) * (word)sizeof(E2KEYREC));
        if ((sts = eeReadBlock(key_address, (unsigned char *)&key_store, sizeof(E2KEYREC))) != 0) return sts;

  // last_routine = 0x50;
        memcpy (ptr, (unsigned char *)&key_store, BYTESERIAL); /* Copy key to caller's buffer */

        /* If the key is "erased", return null key to user */

        csts = memcmp (ptr, erased, BYTESERIAL); /* Erased? */
        if ((csts == 0)                 /* Key bytes erased? */
            && (key_store.CRC == 0xFFFF)) /* *ALL* bytes erased? */
        {                           /* Yes */
              ptr[0] = 0x00;                  /* We all know that BYTESERIAL */
              ptr[1] = 0x00;                  /*   is 6, and this is much faster */
              ptr[2] = 0x00;                  /*   than a call to memset(), */
              ptr[3] = 0x00;                  /*   and takes up *HALF* as many HC16 */
              ptr[4] = 0x00;                  /*   instruction bytes as the call! */
              ptr[5] = 0x00;                  /*   ... */
        }
        else    /* Appears to be real key, verify key contents are valid */
        {
            crc = modbus_CRC (ptr,
                              BYTESERIAL,
                              INIT_CRC_SEED); /* Calculate corresponding CRC */
  // last_routine = 0x50;
            if (crc != key_store.CRC)         /* CRC match? */
            {                           /* No */
                /* VIPER-II currently (released version) doesn't deal with our
                   returning a CRC ("Memory Parity Error") very well. As an ex-
                   pedient concession, just return "empty" slot as error. */

              ptr[0] = 0x00;                  /* We all know that BYTESERIAL */
              ptr[1] = 0x00;                  /*   is 6, and this is much faster */
              ptr[2] = 0x00;                  /*   than a call to memset(), */
              ptr[3] = 0x00;                  /*   and takes up *HALF* as many HC16 */
              ptr[4] = 0x00;                  /*   instruction bytes as the call! */
              ptr[5] = 0x00;                  /*   ... */
            }
        }

        ptr += BYTESERIAL;              /* Next key in caller's buffer */

        } /* End loop copying keys */

    return (0);                         /* Successful return */

} /* End nvKeyGetMany() */

/****************************************************************************
* nvKeyPut -- Insert ("put" / overwrite) bypass key into NonVolatile Key store
*
* Call is:
*
*   nvKeyPut (key, index)
*
* Where:
*
*   "key" is the pointer to a 6-digit Dallas key "serial number" string;
*
*   "index" is the NonVolatile Key store index to receive the new key.
*
* On successful match, nvKeyPut() returns zero, having overwritten the
* NonVolatile Key stored at the specified index. On error, an EE_status
* error value is returned.
****************************************************************************/

char nvKeyPut
    (
    const unsigned char *key,                  /* Pointer to 6-digit Dallas ser no */
    word index                  /* Pointer to index at which to insert */
    )
{
    E2KEYREC keycrc;            /* Local Key record */
    word base;                  /* Base offset of Key partition */
    word size;                  /* Size (bytes) of Key partition */

    char sts;
  // last_routine = 0x51;

    sts = eeMapPartition (EEP_KEY, &size, &base);
    if (sts)
        return (sts);
  // last_routine = 0x51;

    if ((index * sizeof(E2KEYREC)) > size) /* Legal index? */
        return (-1);                    /* No */

    memcpy (keycrc.Key, key, BYTESERIAL); /* Copy user's key */
    keycrc.CRC = modbus_CRC (key,
                             BYTESERIAL,
                             INIT_CRC_SEED); /* Calculate corresponding CRC */

    sts = eeBlockWrite ((unsigned long)((unsigned long)base + ((unsigned long)index * (unsigned long)sizeof(E2KEYREC))),
                        (unsigned char *)&keycrc,
                        sizeof(E2KEYREC));
  // last_routine = 0x51;
    printf("\n\r*** Bypass Key ");
    Report_SN(key);
    printf(" added to EEPROM ***\n\r");

    return (sts);                       /* Propagate success/failure */

} /* End nvKeyPut() */

/****************************************************************************
* nvKeyPutMany -- Insert multiple bypass keys into NonVolatile Key store
*
* Call is:
*
*   nvKeyPutMany (key, index, cnt)
*
* Where:
*
*   "key" is the pointer to a 6-digit Dallas key "serial number" string;
*
*   "index" is the NonVolatile Key store index to receive the new key;
*
*   "cnt" is the cnt of keys to be written into NonVolatile store.
*
* On successful match, nvKeyPut() returns zero, having overwritten the
* NonVolatile Keys stored at the specified index. On error, an EE_status
* error value is returned.
****************************************************************************/

char nvKeyPutMany
    (
    unsigned char *key,                  /* Pointer to 6-digit Dallas ser no */
    word index,                 /* Pointer to index at which to insert */
    unsigned char cnt                  /* Count of keys to insert */
    )
{
    E2KEYREC keycrc[10];        /* Local Key record (BIG stack user) */
                                /* Big enuf for "largest" ModBus write req */
    unsigned char *ptr;                  /* Local scratch copy */
    word base;                  /* Base offset of Key partition */
    word size;                  /* Size (bytes) of Key partition */

    unsigned char i;
    char sts;

  // last_routine = 0x52;
    if (cnt > 10)                     /* Asking for "reasonable" size? */
        return (-1);                    /* Someone's outta their mind! */

    sts = eeMapPartition (EEP_KEY, &size, &base);
  // last_routine = 0x52;
    if (sts)
        return (sts);

    if (((index + cnt) * sizeof(E2KEYREC)) > size) /* Legal index? */
        return (-1);                    /* No */

    ptr = key;                          /* Scratch copy of buffer pointer */

    for ( i=0; i<10; i++)
    {
      memset(keycrc[i].Key, 0, BYTESERIAL);
    }
    for (i = 0; i < cnt; i++)         /* Loop copying keys */
        {                               /* Calculating CRC-16 as we go */
        memcpy (keycrc[i].Key, ptr, BYTESERIAL); /* Copy user's key */
        keycrc[i].CRC = modbus_CRC (ptr,
                                    BYTESERIAL,
                                    INIT_CRC_SEED); /* Calculate CRC-16 */
        }

    /* We've now got a temporary "image" of the EEPROM bytes, copy them to
       the EEPROM in one bulk write operation. */
    if ( cnt != 0)
    {
      sts = eeBlockWrite ((unsigned long)((unsigned long)base + ((unsigned long)index * (unsigned long)sizeof(E2KEYREC))),
                        (unsigned char *)&keycrc[0],
                        (unsigned)((unsigned int)cnt * sizeof(E2KEYREC)));
    }

//    sts = eeBlockWrite (base + (index * sizeof(E2KEYREC)),
//                        (unsigned char *)&keycrc[0],
//                        cnt * sizeof(E2KEYREC));
  // last_routine = 0x52;

    return (sts);                       /* Propagate success/failure */

} /* End nvKeyPutMany() */

/****************************************************************************
* nvKeyDelete -- Delete bypass key from NonVolatile Key store
*
* Call is:
*
*   nvKeyDelete (index)
*
* Where:
*
*   "index" is the NonVolatile Key store index to be deleted.
*
* On successful match, nvKeyDelete returns zero, having overwritten the
* NonVolatile Key stored at the specified index with an "erased" pattern
* (all 0xFF bytes). On error, an EE_status error value is returned.
****************************************************************************/

char nvKeyDelete
    (
    word index                  /* Index to key to be erased */
    )
{
    word base;                  /* Base offset of Key partition */
    word size;                  /* Size (bytes) of Key partition */

    char sts;

  // last_routine = 0x53;
    sts = eeMapPartition (EEP_KEY, &size, &base);
    if (sts)
        return (sts);

    if ((index * sizeof(E2KEYREC)) > size) /* Legal index? */
        return (-1);                    /* No */

    sts = eeBlockFill ((unsigned long)base + ((unsigned long)index * (unsigned long)sizeof(E2KEYREC)),
                        0xFF,
                        sizeof(E2KEYREC) );
  // last_routine = 0x53;

    return (sts);                       /* Propagate success/failure */

} /* End nvKeyDelete() */

/****************************************************************************
* nvKeyErase -- Erase NonVolatile Key store
*
* Call is:
*
*   nvKeyErase
*
* On success, nvKeyErase returns zero, having overwritten the  entire
* NonVolatile Key store with an "erased" pattern (all 0xFF bytes).
* On error, an EE_status error value is returned.
*
* Note: nvKeyErase() uses eeBlockFill() to "erase" the Bypass Key Non-
*       Volatile store. As such, it "blocks" until completion. Since the
*       Bypass Key store is "small" (32 keys), this should not present a
*       timing problem to the system...
****************************************************************************/

char nvKeyErase (void)
{
    word base;                  /* Base offset of Key partition */
    word size;                  /* Size (bytes) of Key partition */

    char sts;

  // last_routine = 0x54;
    sts = eeMapPartition (EEP_KEY, &size, &base);
    if (sts)
        return (sts);

    /* Block-fill Bypass Key NonVolatile store with 0xFF ("erase it"). */

    sts = eeBlockFill ((unsigned long)base, 0xff, size);   /* Erase the home block */

  // last_routine = 0x54;
    return (sts);                       /* Propagate success/failure */

} /* End nvKeyErase() */

/****************************************************************************
*
* Truck ID/Serial Number ("TIM") Partition services
*
****************************************************************************/


/****************************************************************************
* nvTrkEmpty -- Find empty TIM slot in NonVolatile store
*
* Call is:
*
*   nvTrkEmpty (index)
*
* Where:
*
*   "index" is pointer to unsigned integer to receive index of the first
*   free ("empty" == "erased" == 0xFFFFFFFFFFFF) Truck ID slot, if found.
*
* On successful location of an empty/available TIM slot, nvTrkEmpty()
* returns zero (with the matching TIM index stored as requested); On error,
* an EE_status error value is returned.
****************************************************************************/

char nvTrkEmpty
    (
    word *index                 /* Pointer to return matching index */
    )
{
    E2TIMREC *tidarray;         /* Base pointer to NV TIM store */
    E2TIMREC tim_store;         /* Temp storage of current TIM eeprom entry */
    word tim_address;
    word tidbase;               /* TIM partition offset */
    word size;                  /* Size of NV TIM store */
    int csts;                   /* For memcmp() and the compiler */
    unsigned char in_buf[BYTESERIAL];
    unsigned char index_found;
    unsigned int tidmax, i;
     char sts;

  // last_routine = 0x54;
    sts = eeMapPartition (EEP_TIM, &size, &tidbase);
  // last_routine = 0x54;
    if (sts)
        return (sts);

    tidarray = (E2TIMREC *)&tim_store;
    tim_address = TIM_BASE;

    /* Search the Truck ID array. This array is huge, so speed is of the
       essence. We "know" that the E2TIMREC is just a 6-byte array, and
       that the whole partition is just an array of these arrays... */

    tidmax = size/sizeof(E2TIMREC);
    index_found = 0;
    for (i = 0; i < tidmax; i++)
    {
      sts = eeBlockRead((unsigned long)tim_address, in_buf, BYTESERIAL);
      if( sts != 0 )
      {
          return sts;
      }
      csts = memcmp (in_buf, erased, BYTESERIAL); /* Erased? */
      if (csts == 0)                      /* TIM bytes erased? */
      {     
           *index = i;         /* Return matching index */
           index_found = 1;
      }                          /* Yes */

      if( index_found == 1 )
      {
          return(0);
      }
      tim_address += sizeof(E2TIMREC);
  // last_routine = 0x54;
    } /* End search all TIMs */

    return (-1);                        /* No empty TIM slot found */

} /* End nvTrkEmpty() */

/****************************************************************************
* nvTrkFind -- Find Truck ID/Serial Number in NonVolatile store
*
* Call is:
*
*   nvTrkFind (trk, index)
*
* Where:
*
*   "trk" is the pointer to a 6-digit Dallas TIM "serial number" string;
*
*   "index" is pointer to unsigned integer to receive index if found.
*
* On successful match, nvTrkFind() returns zero (with the matching TIM index
* stored as requested); On error, an EE_status error value is returned.
****************************************************************************/

char nvTrkFind
    (
    const unsigned char *trk_org,                  /* Pointer to 6-digit Dallas ser no */
    word *index                 /* Pointer to return matching index */
    )
{
  unsigned char tim_store[sizeof(E2TIMREC)];
  word tim_address;
  word tidbase;               /* Truck ID/TIM partition offset */
  word size;                  /* Size of NV TIM store */
  unsigned char temp_byte;
  unsigned int tidmax, i;              /* TIM-wide outer loop iteration vars */
  int k;                      /* Inner loop counter */
  unsigned char iplo;                  /* Pre-fetched LSB of trk */
  unsigned char crc;
  char sts;
  unsigned char trk[sizeof(E2TIMREC)];

  // last_routine = 0x55;
  if (badvipflag & BVF_DONE)
  {
    if ( badvipflag & BVF_UNAUTH)
    {
      return 1;         /* Not found */
    } else
    {
      return 0;         /* Alread found */
    }
  }
  badvipflag |= BVF_DONE;

  for ( i=0; i<sizeof(E2TIMREC); i++)  /* Make passed TIM number local */
  {
    trk[i] = trk_org[i];
  }

  sts = eeMapPartition (EEP_TIM, &size, &tidbase);
  // last_routine = 0x55;
  if (sts)
      return (sts);

  tim_address = TIM_BASE;

  /* Search the Truck ID array. This array is huge, so speed is of the
     essence. We "know" that the E2TIMREC is just a 6-byte array, and
     that the whole partition is just an array of these arrays... com-
     pare TIMs LSB-to-MSB(-1) since LSB should vary most, and many many
     MSB's will be zeros (and thus would match uselessly). */

  tidmax = size/sizeof(E2TIMREC);     /* Max entries to search */

  iplo = trk[BYTESERIAL-1];           /* Pre-extract desired TIM LSB */

  for (i = 0; i < tidmax; i++)        /* Loop over whole NV store */
  {
    for ( k=0; k < 6; k++)
    {
      tim_store[k]=0;
    }
    if ((sts = eeBlockRead((unsigned long)tim_address, (unsigned char *)&tim_store[0], sizeof(E2TIMREC))) != 0)
    {
      return sts;
    }
  // last_routine = 0x55;

    temp_byte = tim_store[BYTESERIAL-1];
    if (temp_byte != iplo)   /* Possible match? */
    {                               /* No (255 out of 256 cases...) */
      tim_address += BYTESERIAL;           /* Advance to next TIM candidate */
    }
    else                            /* Possible match, */
    {                               /*   must compare rest of TIM */
      for (k = ((BYTESERIAL-1) - 1); k > 0; k--)
      {                           /* Match from TIM[4] to TIM[1] */
        if (tim_store[k] != trk[k])     /* tidarray[i].Serial[k] != trk[k] */
          break;              /* This stored TIM doesn't match */
      }
      if (k == 0)                 /* If low *5* "digits" matched */
      {                           /* This TIM is a match */
        /* ***KROCK*** always assume MSB of TIM is a "0", and use
           that byte for the Dallas CRC-8 byte.   ***KROCK*** */
        *index = i;             /* Return matching index */
        crc = Dallas_CRC8 ((UINT8 *)&trk[1], (BYTESERIAL-1));
  // Last_routine = 0x55;
        if ((crc == tim_store[0])      /* Valid TIM entry? */
            && (trk[0] == 0))   /* ***and trk[0] assumption true*** */
        {
          return (0);         /* Yes, match found */
        }
      }
        /* This TIM entry doesn't match afterall, advance to next one */
      tim_address += BYTESERIAL; /* Pointer to current TIM */
    } /* End possible match one TIM */

  } /* End search all TIMs */
  badvipflag |= BVF_UNAUTH;
  return (-1);
} /* End nvTrkFind() */

/****************************************************************************
* nvTrkGet -- Retrieve ("get") Truck serial number from NonVolatile Key store
*
* Call is:
*
*   nvTrkGet (trk, index)
*
* Where:
*
*   "trk" is the pointer to a 6-digit Dallas key "serial number" string
*   to be filled in with the specified Truck ID/serial number.
*
*   "index" is the NonVolatile TIM store index to retrieve.
*
* On successful match, nvTrkGet() returns zero, having copied the  Non-
* Volatile TIM entry stored at the specified index into the caller's TIM
* buffer (assumed to be large enough!). If the NonVolatile copy is "erased"
* (all 0xFF's), then the returned TIM will be all zeroes. On error, an
* EE_status error value is returned.
****************************************************************************/

char nvTrkGet
    (
    unsigned char *trk,                  /* Pointer to 6-digit Dallas ser no */
    word index                  /* Pointer to index to retrieve */
    )
{
    E2TIMREC *tidcrc;           /* NonVolatile store TIM record */
    E2TIMREC tim_store;         /* Temp storage of current TIM eeprom entry */
    word tim_address;
    int csts;                   /* For memcmp() and the compiler */
    word base;                  /* Base offset of TIM partition */
    word size;                  /* Size (bytes) of TIM partition */
    unsigned char crc;          /* Alleged CRC of copied TIM */

    char sts;

  // last_routine = 0x56;
    sts = eeMapPartition (EEP_TIM, &size, &base);
  // last_routine = 0x56;
    if (sts)
        return (sts);

    tidcrc = (E2TIMREC *)&tim_store;
    tim_address = TIM_BASE + (word)(index * sizeof(E2TIMREC));

    if ((index * sizeof(E2TIMREC)) > size) /* Legal index? */
        return (-1);                    /* No */

    /* Generate access pointer to nvTrk TIM record */

    if ((sts = eeBlockRead((unsigned long)tim_address, (unsigned char *)&tim_store, sizeof(E2TIMREC))) != 0) return sts;

  // last_routine = 0x56;
    memcpy (trk, tidcrc->Serial, BYTESERIAL); /* Copy TIM to caller's buffer */

    /* If the TIM is "erased", return null TIM to user */

//RDH See eeWriteBlock()!
//RDH    if ((memcmp(trk, erased, BYTESERIAL) == 0) /* Erased? */
    csts = memcmp (trk, erased, BYTESERIAL); /* Erased? */
    if (csts == 0)                      /* TIM bytes erased? */
        {                               /* Yes */
//        memset (trk, 0x00, BYTESERIAL); /* "Empty" the caller's buffer */
        trk[0] = 0x00;       /* We all know that BYTESERIAL */
        trk[1] = 0x00;       /*   is 6, and this is much faster */
        trk[2] = 0x00;       /*   than a call to memset(), */
        trk[3] = 0x00;       /*   and takes up *HALF* as many HC16 */
        trk[4] = 0x00;       /*   instruction bytes as the call! */
        trk[5] = 0x00; /*   ... */
        return (0);                     /* Return "empty" TIM */
        }

    /* Appears to be real TIM, verify TIM contents are valid */

    crc = Dallas_CRC8 ((UINT8 *)&trk[1], (BYTESERIAL-1)); /* Calculate corresponding CRC */
  // last_routine = 0x56;
    if (crc != trk[0])                  /* CRC-8 match? */
        {                               /* No */
        /* VIPER-II currently (released version) doesn't deal with our
           returning a CRC ("Memory Parity Error") very well. As an ex-
           pedient concession, just return "empty" slot as error. */

        trk[0] = 0x00;       /* We all know that BYTESERIAL */
        trk[1] = 0x00;       /*   is 6, and this is much faster */
        trk[2] = 0x00;       /*   than a call to memset(), */
        trk[3] = 0x00;       /*   and takes up *HALF* as many HC16 */
        trk[4] = 0x00;       /*   instruction bytes as the call! */
        trk[5] = 0x00;  /*   ... */
        return (0);                     /* Don't confuse VIPER */
        }

    trk[0] = 0;                         /* TIM MSB is always zero... */

    return (0);                         /* Successful return */

} /* End nvTrkGet() */

/****************************************************************************
* nvTrkGetMany -- Retrieve many Truck IDs from NonVolatile Key store
*
* Call is:
*
*   nvTrkGetMany (trk, index, cnt)
*
* Where:
*
*   "trk" is the pointer to the "first of many" 6-digit Dallas key "serial
*   number" strings (in essence, an array of serial numbers) to be filled
*   in with the specified Truck ID/serial number.
*
*   "index" is the first NonVolatile TIM store index to retrieve;
*
*   "cnt" is the cnt of successive Truck IDs to extract and return.
*
* On successful match, nvTrkGet() returns zero, having copied the  Non-
* Volatile TIM entries stored at the specified index into the caller's TIM
* buffer (assumed to be large enough!). If the NonVolatile copy is "erased"
* (all 0xFF's), then the returned TIM will be all zeroes. On error, an
* EE_status error value is returned.
****************************************************************************/

char nvTrkGetMany
    (
    unsigned char *trk,                  /* Pointer to 6-digit Dallas ser no */
    word index,                 /* Pointer to first index to retrieve */
    unsigned char cnt                  /* Count of entries to retrieve */
    )
{
    unsigned char *ptr;                  /* NonVolatile store TIM record */
    word tim_address;
    int csts;                   /* For memcmp() and the compiler */
    word base;                  /* Base offset of TIM partition */
    word size;                  /* Size (bytes) of TIM partition */
    unsigned char crc;                   /* Alleged CRC of copied TIM */

    unsigned char i;
    char sts;

  // last_routine = 0x57;
    sts = eeMapPartition (EEP_TIM, &size, &base);
  // last_routine = 0x57;
    if (sts)
        return (sts);

    if (((index + cnt) * sizeof(E2TIMREC)) > size) /* Legal index? */
        return (-1);                    /* No */

    /* Generate access pointer to nvTrk TIM record (note that in actuality,
       we "know" the NV store is just an array of 6-byte serial numbers with
       the MSB being the CRC-8 of the low-order 5 bytes, and MSB really 0) */

    tim_address = TIM_BASE + (word)(index * sizeof(E2TIMREC));

    if ((sts = eeBlockRead((unsigned long)tim_address, (unsigned char *)trk, cnt * BYTESERIAL)) != 0) return sts;
  // last_routine = 0x57;

    /* If the TIM is "erased", return null TIM to user */

    ptr = trk;
    for (i = cnt; i > 0; i--)
    {
      csts = memcmp (ptr, erased, BYTESERIAL); /* Erased? */
      if (csts == 0)                  /* TIM bytes erased? */
      {                           /* Yes */
        ptr[0] = 0x00;   /* We all know that BYTESERIAL */
        ptr[1] = 0x00;   /*   is 6, and this is much faster */
        ptr[2] = 0x00;   /*   than a call to memset(), */
        ptr[3] = 0x00;   /*   and takes up *HALF* as many HC16 */
        ptr[4] = 0x00;   /*   instruction bytes as the call! */
        ptr[5] = 0x00;   /*   ... */
      }
      else
      {
          /* Appears to be real TIM, verify TIM contents are valid */

        crc = Dallas_CRC8 ((UINT8 *)&ptr[1], (BYTESERIAL-1)); /* Calculate CRC-8 */
  // last_routine = 0x57;
        if (crc != *ptr)            /* CRC-8 match? */
        {                       /* No */
            /* VIPER-II currently (released version) doesn't deal with our
               returning a CRC ("Memory Parity Error") very well. As an ex-
               pedient concession, just return "empty" slot as error. */

            ptr[0] = 0x00; /* We all know that BYTESERIAL */
            ptr[1] = 0x00; /*   is 6, and this is much faster */
            ptr[2] = 0x00; /*   than a call to memset(), */
            ptr[3] = 0x00; /*   and takes up *HALF* as many HC16 */
            ptr[4] = 0x00; /*   instruction bytes as the call! */
            ptr[5] = 0x00; /*   ... */
        }
        *ptr = 0;                   /* MSB always zero */
      }
      ptr += BYTESERIAL;            /* Next returned */
    }

    /* If we got here, all is well; return successfully to caller */

    return (0);                         /* Successful return */

} /* End nvTrkGetMany() */

/****************************************************************************
* nvTrkVrMany -- Verify "many" Truck IDs from NonVolatile Key store
*
* Call is:
*
*   nvTrkVrMany (crc, index, cnt)
*
* Where:
*
*   "crc" is the return pointer to the calculated "verification code" (aka
*   CRC-16)
*
*   "index" is the first NonVolatile TIM store index to retrieve;
*
*   "cnt" is the cnt of successive Truck IDs to extract and return.
*
* On successful return, the ModBus-style CRC-16 value of the contiguous
* array of 6-byte "Truck ID"s from NonVolatile storage is calculated and
* returned as the "verification" code for the specified "slice" of the
* Truck ID array.
*
* If the NonVolatile copy is "erased" (all 0xFF's), then the TIM will be
* CRC'ed as all zeroes.
*
* Note especially that the individual TIM entries are not themselves veri-
* fied for integrity (CRC'ed).
*
* On error, an EE_status error value is returned.
****************************************************************************/

char nvTrkVrMany
    (
    word *vfc,                  /* Pointer to returned CRC value */
    word index,                 /* Pointer to first index to verify */
    word cnt                  /* Count of entries to verify */
    )
{
    word tim_address;
    int csts;                 /* For memcmp() and the compiler */
    word base;                  /* Base offset of TIM partition */
    word size;                  /* Size (bytes) of TIM partition */
    word crc;                   /* Alleged CRC of copied TIM */
    word i;

    unsigned char tim[BYTESERIAL];       /* Local holding copy */
    char sts;

  // last_routine = 0x58;
    sts = eeMapPartition (EEP_TIM, &size, &base);
  // last_routine = 0x58;
    if (sts)
        return (sts);

    if (((index + cnt) * sizeof(E2TIMREC)) > size) /* Off end of array? */
        return (-1);                    /* Yes, error */

    crc = INIT_CRC_SEED;                /* Prime the pump, so to speak */

    /* Generate access pointer to nvTrk TIM record (note that in actuality,
       we "know" the NV store is just an array of 6-byte serial numbers with
       the MSB being the CRC-8 of the low-order 5 bytes, and MSB really 0) */

    tim_address = TIM_BASE + (word)(index * sizeof(E2TIMREC));

    /* Loop accumulating the CRC-16 value for all the TIMs as if they were
       in a single contiguous array...

       At 16.78MHz, this seems to take about 240us/TIM. As such, TAS/VIPER
       should limit themselves to 100 or so TIMs/call. */

    for (i = cnt; i > 0; i--)
    {
        if ((sts=eeBlockRead((unsigned long)tim_address, (unsigned char *)tim, BYTESERIAL)) != 0) return sts;
  // last_routine = 0x58;

        /* If the TIM is "erased", then CRC it as all zeroes */

        csts = memcmp (tim, erased, BYTESERIAL);
        if (csts == 0)                  /* TIM bytes erased? */
        {                           /* Yes */
          tim[0] = 0x00;   /* We all know that BYTESERIAL */
          tim[1] = 0x00;   /*   is 6, and this is much faster */
          tim[2] = 0x00;   /*   than a call to memset(), */
          tim[3] = 0x00;   /*   and takes up *HALF* as many HC16 */
          tim[4] = 0x00;   /*   instruction bytes as the call! */
          tim[5] = 0x00;   /*   ... */
        }

        /* Accumulate "verification code" (CRC-16) on TIM contents */

        tim[0] = 0x00;                  /* KROCK high byte zero KROCK */
        crc = modbus_CRC (tim, BYTESERIAL, crc); /* Cumulative CRC-16 */

  // last_routine = 0x58;
        tim_address += BYTESERIAL;              /* Step to next TIM slot */
    }

    /* If we got here, all is well; return successfully to caller */

    *vfc = crc;                         /* Return calculated CRC-16 */
    return (0);                         /* Successful return */

} /* End nvTrkVrMany() */

/****************************************************************************
* nvTrkPut -- Insert ("put" / overwrite) Truck ID into NonVolatile TIM store
*
* Call is:
*
*   nvTrkPut (trk, index)
*
* Where:
*
*   "trk" is the pointer to a 6-digit Dallas key "serial number" string;
*
*   "index" is the NonVolatile TIM store index to receive the new Truck ID.
*
* On successful match, nvTrkPut() returns zero, having overwritten the
* NonVolatile TIM stored at the specified index. On error, an EE_status
* error value is returned.
****************************************************************************/

char nvTrkPut
    (
    unsigned char *trk,                  /* Pointer to 6-digit Dallas ser no */
    word index                  /* Pointer to index at which to insert */
    )
{
//    word tim_address;
    E2TIMREC tidcrc;            /* Local TIM record */
    word base;                  /* Base offset of TIM partition */
    word size;                  /* Size (bytes) of TIM partition */

    char sts;

  // last_routine = 0x59;
    sts = eeMapPartition (EEP_TIM, &size, &base);
  // last_routine = 0x59;
    if (sts)
        return (sts);

    if ((index * sizeof(E2TIMREC)) > size) /* Legal index? */
        return (-1);                    /* No */

    /* This ***KROCK*** depends on TIM MSB being zero, so we can "hide" the
       CRC-8 as part of the 6 bytes stored */

    if (trk[0] != 0)
        return (EE_DATAERROR);          /* The *KROCK* is broken! */

//    tim_address = TIM_BASE + (word)(index * sizeof(E2TIMREC));

    memcpy (tidcrc.Serial, trk, BYTESERIAL); /* Copy user's Truck ID */

    tidcrc.Serial[0] = (char)Dallas_CRC8 ((UINT8 *)&trk[1],
                                   (BYTESERIAL-1)); /* Calculate CRC-8 */

  // last_routine = 0x59;
    sts = eeBlockWrite ((unsigned long)base + ((unsigned long)index * (unsigned long)sizeof(E2TIMREC)),
                        (unsigned char *)&tidcrc,
                        sizeof(E2TIMREC));
  // last_routine = 0x59;

    return (sts);                       /* Propagate success/failure */

    } /* End nvTrkPut() */

/****************************************************************************
* nvTrkPutMany -- Insert multiple Truck IDs into NonVolatile TIM store
*
* Call is:
*
*   nvTrkPutMany (trk, index, cnt)
*
* Where:
*
*   "trk" is the pointer to the "first of many" 6-digit Dallas key "serial
*   number" string (in essence, a pointer to an array of serial numbers);
*
*   "index" is the NonVolatile TIM store index to receive the new Truck ID;
*
*   "cnt" is the cnt of consecutive Truck IDs to be written.
*
* On successful match, nvTrkPutMany() returns zero, having overwritten the
* NonVolatile TIM stored at the specified indices. On error, an EE_status
* error value is returned.
*
* Note: nvTrkPutMany *overwrites* the caller's buffer! It must calculate
*       the CRC-8's "in place", and uses the caller's buffer for this
*       purpose. The only caller should be ModBus code, and it expects this
*       behavior.
****************************************************************************/

char nvTrkPutMany
    (
    unsigned char *trk,                  /* Pointer to 6-digit Dallas ser no */
    word index,                 /* Pointer to index at which to insert */
    unsigned char cnt                  /* Count of consecutive TIMs */
    )
{
    unsigned char *ptr;                  /* Scratch pointer */
    word base;                  /* Base offset of TIM partition */
    word size;                  /* Size (bytes) of TIM partition */

    unsigned char i;
    char sts;

  // last_routine = 0x5A;
    sts = eeMapPartition (EEP_TIM, &size, &base);
  // last_routine = 0x5A;
    if (sts)
        return (sts);

    if (((index + cnt) * sizeof(E2TIMREC)) > size) /* Legal index? */
        return (-1);                    /* No */

    /* This ***KROCK*** depends on TIM MSB being zero, so we can "hide" the
       CRC-8 as part of the 6 bytes stored */

    ptr = trk;                          /* Start of callers TIM array */
    for (i = cnt; i > 0; i--)           /* Loop over the TIM array */
    {                                   /*  krocking in the CRC-8 */
        if (*ptr != 0)                  /* Is the KROCK byte zero? */
            return (EE_DATAERROR);      /* NO! Cannot do this */
        *ptr = Dallas_CRC8 ((UINT8 *)ptr+1, BYTESERIAL-1); /* Fix up the CRC-8 "field" */
  // last_routine = 0x5A;
        ptr += BYTESERIAL;              /* Next caller serial number */
    }

    /* Now write the caller's modified buffer en bloc directly to EEPROM */

    sts = eeBlockWrite ((unsigned long)base + ((unsigned long)index * (unsigned long)sizeof(E2TIMREC)),
                        (unsigned char *)trk,
                        cnt * sizeof(E2TIMREC));
  // last_routine = 0x5A;

    /* If it ever becomes important, we can "restore" the caller's buffer
       by zeroing out the [0] bytes again... */

    return (sts);                       /* Propagate success/failure */

} /* End nvTrkPutMany() */

/****************************************************************************
* nvTrkDelete -- Delete Truck ID from NonVolatile TIM store
*
* Call is:
*
*   nvTrkDelete (index)
*
* Where:
*
*   "index" is the NonVolatile TIM index to be deleted.
*
* On successful match, nvTrkDelete returns zero, having overwritten the
* NonVolatile TIM stored at the specified index with an "erased" pattern
* (all 0xFF bytes). On error, an EE_status error value is returned.
****************************************************************************/

char nvTrkDelete
    (
    word index                  /* Index to TIM to be erased */
    )
{
    E2TIMREC tidcrc;            /* Local TIM record */
    word base;                  /* Base offset of TIM partition */
    word size;                  /* Size (bytes) of TIM partition */

    char sts;

  // last_routine = 0x5B;
    sts = eeMapPartition (EEP_TIM, &size, &base);
  // last_routine = 0x5B;
    if (sts)
        return (sts);

    if ((index * sizeof(E2TIMREC)) > size) /* Legal index? */
        return (-1);                    /* No */

    memset ((char *)&tidcrc, 0xFF, sizeof(tidcrc)); /* "Erased" TIM entry */

    sts = eeBlockWrite ((unsigned long)base + ((unsigned long)index * (unsigned long)sizeof(E2TIMREC)),
                        (unsigned char *)&tidcrc,
                        sizeof(E2TIMREC));
  // last_routine = 0x5B;

    return (sts);                       /* Propagate success/failure */

} /* End nvTrkDelete() */

/****************************************************************************
* nvTrkErase -- Erase NonVolatile TIM store
*
* Call is:
*
*   nvTrkErase
*
* On success, nvTrkErase returns zero, having overwritten the entire
* NonVolatile TIM store with an "erased" pattern (all 0xFF bytes).
* On error, an EE_status error value is returned.
*
* Note: nvTrkErase() uses eeBlockFill() to "erase" the Truck ID Non-
*       Volatile store. As such, it "blocks" until completion. Since the
*       TIM store is relatively huge (30KB), this takes a long time! This
*       presents a serious timing problem to the system...
****************************************************************************/

char nvTrkErase (void)
{
    word base;                  /* Base offset of TIM partition */
    word size;                  /* Size (bytes) of TIM partition */

    char sts;

  // last_routine = 0x5C;
    sts = eeMapPartition (EEP_TIM, &size, &base);
  // last_routine = 0x5C;
    if (sts)
        return (MB_EXC_FAULT);

    /* Block-fill Truck ID NonVolatile store with 0xFF ("erase it"). */

    sts = eeBlockFill ((unsigned long)base, 0xFF, size);

  // last_routine = 0x5C;
    return (sts);                       /* Propagate success/failure */

} /* End nvTrkErase() */

/*********************** end of nvtruck.c **********************************/
