/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         eeprom.c
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009  Scully Signal Company
 *
 *   Description:    This file contains all the eeprom procedures for Rack
 *                   controller main microprocessor PIC24HJ256GP210
 *
 *   Revision History:
 *   Rev      Date      Who   Description of Change Made
 *  --------  --------- ---  --------------------------------------------
 *  1.6.34    10/14/16  DHP  Added eeCRC() and call to it
 *
 *
 *****************************************************************************/
#include "common.h"

/*******************************5/19/2008 8:09AM******************************
 * Function Name:  EEPROM_read
 * Description:    This routine reads a block of bytes from the 24FC1025 I2C
 *                 device in master mode.
 * Parameters:     device_addr, reg_addr, pointer to the block of data,
 *                 length of block
 * Return Value:   00 ==  read OK else error code from call
 *                        06 == MasterWriteI2C2
 *                        07 == MasterWriteI2C upper address
 *                        08 == MasterWriteI2C lower address
 *                        09 == MastergetsI2C2
 *                        10 == MasterWriteI2C2
 *                        12 == StopI2C2
 * 
 *****************************************************************************/
unsigned int EEPROM_read(UINT8 device_addr, UINT32 reg_addr, UINT8 *data_ptr,
                   UINT8 length)
{
UINT16 status = 0;

  // last_routine = 0x19;
  if ( reg_addr > 0xFFFF)
  {
    device_addr |= 0x08;                /* Select upper bank of eeprom */
    reg_addr &= 0xFFFF;                 /* Remove upper address bit */
  }
  // start I2C sequence
  StartI2C2();
  if (MasterWriteI2C2(device_addr))
  {
    status = 6;
  }
  // last_routine = 0x19;
  else
  {
      // set upper address byte of 16bit address
    if (MasterWriteI2C2((UINT8)(reg_addr >> 8) & 0xFF)) status = 7;
  // last_routine = 0x19;

    if ( status == 0)
    {
        // set lower address byte of 16bit address
      if (MasterWriteI2C2((UINT8)(reg_addr & 0xFF))) status = 8;
  // last_routine = 0x19;

        // start I2C sequence
      RestartI2C2();
  // last_routine = 0x19;

      if ( status == 0)
      {
          /******************************5/23/2008 2:38PM*****************************
           * write Device address - Start read cycle
           ***************************************************************************/
          if (MasterWriteI2C2((UINT8)(device_addr | 1)) == 0)
          {
            // wait for data to end transmission
            if (MastergetsI2C2(length, data_ptr, 1000))
            {
              status = 9;
            }
          } else
          {
            status = 10;
          }
      }
    }
  }

  // last_routine = 0x19;
  if (StopI2C2()) status = 12;
  if ( status)
  {
    StatusB |= STSB_ERR_EEPROM;
  }

  return (status);
}

/*******************************5/19/2008 8:09AM******************************
 * Function Name:  EEPROM_write
 * Description:    This routine write a block of bytes to the 24FC1025 I2C
 *                 device in master mode.
 * Parameters:     device_addr, reg_addr, pointer to the block of data,
 *                 length of block
 * Return Value:   error
 *****************************************************************************/

unsigned int EEPROM_write(unsigned char device_addr, unsigned long reg_addr,
 const unsigned char *data_ptr, unsigned char length)
{
UINT16 status = 0;
UINT8 i;

  if ( reg_addr > 0xFFFF)
  {
    device_addr |= 0x08;                /* Select upper bank of eeprom */
    reg_addr &= 0xFFFF;                 /* Remove upper address bit */
  }

  // start I2C sequence
  // last_routine = 0x1A;
  StartI2C2();

  if (MasterWriteI2C2(device_addr)) status = 1;
  // last_routine = 0x1A;

  // set upper address byte of 16bit address
  if ( status == 0)
  {
    if (MasterWriteI2C2((UINT8)(reg_addr >> 8) & 0xFF)) status = 2;
  // last_routine = 0x1A;

    if ( status == 0)
    {
        // set lower address byte of 16bit address
      if (MasterWriteI2C2((UINT8)(reg_addr & 0xFF))) status = 3;
  // last_routine = 0x1A;

      if ( status == 0)
      {
        for ( i=0; i<length; i++)
        {
          if (MasterWriteI2C2(*data_ptr++))
          {
            status = 4;
            break;
          }
  // last_routine = 0x1A;
        }
      }
    }
  }

  if (StopI2C2()) status = 5;

  DelayMS(5);
  if ( status)
  {
    StatusB |= STSB_ERR_EEPROM;
  }
  return (status);
}

/****************************************************************************
* eeBlockWrite -- perform a block write.
*
* Call is:
*
*   eeBlockWrite (loc, *datum, count)
*
* Where:
*
*   "loc" is the EEPROM relative offset;
*
*   "*datum" is a pointer to the desired data;
*
*   "byte_count" is the count of bytes to be written ("filled") into EEPROM.
*
* eeBlockFill() fills a EEPROM block of <byte_count> bytes starting at EEPROM
* offset <loc> with the byte values located at <*datum>. In essence, it
* is a "memset()" to EEPROM. eeBlockFill() verifies the data written.
*
* Note: eeBlockFill() is a synchronous or blocking function -- it does not
*       return until the entire block has been successfully filled (or an
*       error has occurred). On average, this will take ((count/64) * 5)ms
*       to complete, twice that worst case.
*
* Successful return is a zero value; on error, eeBlockWrite() returns an
* EE_status value indicating the problem. On error return, eeBlockFill()
* has attempted to fill the entire block irrespective of any errors (i.e.,
* an error does not prevent eeBlockWrite from attempting to fill the rest
* of the EEPROM block).
****************************************************************************/
/*lint -e416 Likely creation of out-of-bounds pointer */
/*lint -e662 Possible creation of out-of-bounds pointer
 *  The eeBlockWrite and eeBlockRead functions use a char pointer and EEPROM
 *  offset, neither of which tells lint about the boundaries of the transfer.
 *  This can possibly be fixed we the use of a union of the structure used and
 *  a char array the size of the structure.  This would involve changing a lot
 *  of code - not worth the risk!
 */
char eeBlockWrite(unsigned long loc, const unsigned char *datum, unsigned int byte_count)
{
unsigned int transfer_byte_count;
char status;
unsigned long address;
unsigned char first_page;

  address = loc;

  /****************************** 11/10/2008 11:40AM *************************
   * Can only do a write up to a 128 byte page boundary
   ***************************************************************************/
  // last_routine = 0x1B;

  if ((first_page = (address % 0x80)) != 0)
  {
    transfer_byte_count = 0x80 - first_page;
    if (transfer_byte_count > byte_count)
    {
      transfer_byte_count = byte_count;
    }
  }
  else
  {
    transfer_byte_count = byte_count;
    if (transfer_byte_count>0x80)
    {
      transfer_byte_count = 0x80;
    }
  }
  byte_count -= transfer_byte_count;

  if ((status = (char)EEPROM_write(MC24FC1025_DEVICE, address,
                   datum, (unsigned char)transfer_byte_count)) != 0)
  {
    return status;
  }

  datum += transfer_byte_count;
  address += transfer_byte_count;

  while (byte_count)
  {
    if ( byte_count > 0x80)
    {
      transfer_byte_count = 0x80;
      byte_count -= 0x80;
    }
    else
    {
      transfer_byte_count = byte_count;
      byte_count = 0;
    }
    if ((status = (char)EEPROM_write(MC24FC1025_DEVICE, address,
                   datum, (unsigned char)transfer_byte_count)) != 0)
    {
      return status;
    }
  // last_routine = 0x1B;
    service_charge();             /* Keep Service LED off */
    datum += transfer_byte_count;
    address += transfer_byte_count;
  }

  return status;

} /* End eeBlockWrite() */
/*lint +e416 Likely creation of out-of-bounds pointer */
/****************************************************************************
* eeBlockRead -- perform a block read.
*
* Call is:
*
*   eeBlockWrite (offset, datum, count)
*
* Where:
*
*   "offset" is the EEPROM relative offset;
*
*   "datum" address to a buffer;
*
*   "count" is the count of bytes to be written ("filled") into EEPROM.
*
* eeBlockRead() fills a EEPROM block of <count> bytes starting at EEPROM
* offset <offset> with the byte value specified by <datum>. In essence, it
* is a "memcpy()" to EEPROM. .
*
* Note: eeBlockRead() is a synchronous or blocking function -- it does not
*       return until the entire block has been successfully read (or an
*       error has occured). On average, this will take ((count/64) * 5)ms
*       to complete, twice that worst case.
*
* Successful return is a zero value; on error, eeBlockRead() returns an
* EE_status value indicating the problem. On error return, eeBlockRead()
* has attempted to fill the entire block irrespective of any errors (i.e.,
* an error does not prevent eeBlockRead from attempting to fill the rest
* of the EEPROM block).
****************************************************************************/

char eeBlockRead(unsigned long loc, unsigned char *datum, unsigned byte_count)
{
unsigned int transfer_count;
char status;
unsigned long address;
unsigned char first_page;

  address = loc;

  /****************************** 11/10/2008 11:40AM *************************
   * Can only do a write up to a 128 byte page boundary
   ***************************************************************************/
  // last_routine = 0x1C;

  if ((first_page = (address % 0x80)) != 0)
  {
    transfer_count = 0x80 - first_page;
    if (transfer_count > byte_count)
    {
      transfer_count = byte_count;
    }
  }
  else
  {
    transfer_count = byte_count;
    if (transfer_count>0x80)
    {
      transfer_count = 0x80;
    }
  }
  byte_count -= transfer_count;

  if ((status = (char)EEPROM_read(MC24FC1025_DEVICE, address,
                   datum, (unsigned char)transfer_count)) != 0)
  {
    return status;
  }
  // last_routine = 0x1C;
  address += transfer_count;
  datum += transfer_count;
  while (byte_count)
  {
    if ( byte_count > 0x80)
    {
      transfer_count = 0x80;
      byte_count -= 0x80;
    }
    else
    {
      transfer_count = byte_count;
      byte_count = 0;
    }
    if ((status = (char)EEPROM_read(MC24FC1025_DEVICE, address,
                   datum, (unsigned char)transfer_count)) != 0)
    {
      return status;
    }
  // last_routine = 0x1C;
    address += transfer_count;
    datum += transfer_count;
    service_charge();             /* Keep Service LED off */
  }

  return status;

} /* End eeBlockRead() */
/*lint +e662 Possible creation of out-of-bounds pointer */

/*******************************5/19/2008 8:09AM******************************
 * Function Name:  EEPROM_fill
 * Description:    This routine fills a a block of eeprom with the passed byte
 *                 to the 24FC1025 I2C device in master mode.
 * Parameters:     device_addr, reg_addr, pointer to the block of data,
 *                 length of block
 * Return Value:   error
 *****************************************************************************/

UINT16 EEPROM_fill(UINT8 device_addr, UINT32 reg_addr,
 UINT8 fill_data, UINT8 length)
{
UINT16 status = 0;
UINT8 i;

  // last_routine = 0x1D;
  if ( reg_addr > 0xFFFF)
  {
    device_addr |= 0x08;                /* Select upper bank of eeprom */
    reg_addr &= 0xFFFF;                 /* Remove upper address bit */
  }

  // start I2C sequence
  StartI2C2();

  if (MasterWriteI2C2(device_addr)) status = 1;
  // last_routine = 0x1D;

    // set upper address byte of 16bit address
  if (MasterWriteI2C2((UINT8)(reg_addr >> 8) & 0xFF)) status = 2;
  // last_routine = 0x1D;

  if ( status == 0)
  {
        // set lower address byte of 16bit address
    if (MasterWriteI2C2((UINT8)(reg_addr & 0xFF))) status = 3;
  // last_routine = 0x1D;

    if ( status == 0)
    {
      for ( i=0; i<length; i++)
      {
        if (MasterWriteI2C2(fill_data))
        {
          status = 4;
          break;
        }
  // last_routine = 0x1D;
      }
    }
  }

  if (StopI2C2()) status = 5;
  DelayMS(5);
  if ( status)
  {
    StatusB |= STSB_ERR_EEPROM;
  }
  return (status);
}


/****************************************************************************
* eeBlockFill -- Fill block of EEPROM with specified value
*
* Call is:
*
*   eeBlockWrite (offset, datum, count)
*
* Where:
*
*   "offset" is the EEPROM relative offset;
*
*   "datum" is the desired byte value;
*
*   "count" is the count of bytes to be written ("filled") into EEPROM.
*
* eeBlockFill() fills a EEPROM block of <count> bytes starting at EEPROM
* offset <offset> with the byte value specified by <datum>. In essence, it
* is a "memset()" to EEPROM. eeBlockFill() verifies the data written.
*
* Note: eeBlockFill() is a synchronous or blocking function -- it does not
*       return until the entire block has been successfully filled (or an
*       error has occured). On average, this will take ((count/64) * 5)ms
*       to complete, twice that worst case.
*
* Successful return is a zero value; on error, eeBlockWrite() returns an
* EE_status value indicating the problem. On error return, eeBlockFill()
* has attempted to fill the entire block irrespective of any errors (i.e.,
* an error does not prevent eeBlockWrite from attempting to fill the rest
* of the EEPROM block).
****************************************************************************/

char eeBlockFill(unsigned long loc, unsigned char datum, unsigned int byte_count)
{
unsigned int transfer_count;
char status;
unsigned long address;
unsigned char first_page;

  address = loc;
  // last_routine = 0x1E;

  /****************************** 11/10/2008 11:40AM *************************
   * Can only do a write up to a 128 byte page boundry
   ***************************************************************************/

  if ((first_page = (address % 0x80)) != 0)
  {
    transfer_count = 0x80 - first_page;
    if (transfer_count > byte_count)
    {
      transfer_count = byte_count;
    }
  }
  else
  {
    transfer_count = byte_count;
    if (transfer_count>0x80)
    {
      transfer_count = 0x80;
    }
  }
  byte_count -= transfer_count;

  if ((status = (char)EEPROM_fill(MC24FC1025_DEVICE, address,
                   datum, (unsigned char)transfer_count)) != 0)
  {
    return status;
  }
  // last_routine = 0x1E;
  address += transfer_count;

  while (byte_count)
  {
    if ( byte_count > 0x80)
    {
      transfer_count = 0x80;
      byte_count -= 0x80;
    }
    else
    {
      transfer_count = byte_count;
      byte_count = 0;
    }
    if ((status = (char)EEPROM_fill(MC24FC1025_DEVICE, address,
                   datum, (unsigned char)transfer_count)) != 0)
    {
      return status;
    }
  // last_routine = 0x1E;
    address += transfer_count;
    service_charge();             /* Keep Service LED off */
  }

 return status;

} /* End eeBlockFill() */

char eeCRC(void)
{
  char sts;
  unsigned long eeprom_addr;
  unsigned int crc;                    /* Temporary storage for Checksum */
  unsigned int base;                   /* Base offset of System-NV partition */
  unsigned int size;                   /* Size (bytes) of System-NV partition */
  E2SYSBLK   *sysptr;                  /* EEPROM's System NonVolatile area */
  E2HOMEBLK  *Hptr;
  SysParmNV  *Pptr;
  DateStampNV *Dptr;
  SysDia5NV  *D5ptr;
  SysVoltNV  *Vptr;
  SysSet1NV  *S1ptr;

  eeprom_addr = HOME_BASE;
  Hptr = &home_block;
  if (eeReadBlock((unsigned int)eeprom_addr, (unsigned char *)Hptr, sizeof(E2HOMEBLK)) != 0)
  {
    EE_status |= EE_DATAERROR;
    StatusB |= STSB_ERR_EEPROM;            /* Note errors reading EEPROM */
  }    
  crc = modbus_CRC ((unsigned char *)&home_block,
                    sizeof(E2HOMEBLK) - 2,
                    INIT_CRC_SEED);
  if (crc != home_block.CRC)               /* Home block data valid? */
    {
      EE_status |= EE_CRC;
      StatusB |= STSB_ERR_EEPROM;
    }
  sts = eeMapPartition (EEP_SYSNV, &size, &base);
  if (sts)
  {
      EE_status |= EE_DATAERROR;
      StatusB |= STSB_ERR_EEPROM;          /* Note errors with EEPROM */
  } else
  {
    if ( eeReadBlock(base, (unsigned char *)&SysNonV, sizeof(E2SYSBLK)))
    {
      EE_status |= EE_FORMAT;
      StatusB |= STSB_ERR_EEPROM;
    }
  }
  sysptr = (E2SYSBLK *)&SysNonV;
  Pptr = &sysptr->ParmBlock;               /* Point to EEPROM SysParm block */
  crc = modbus_CRC ((unsigned char *)Pptr, /* Check the block CRC */
                       sizeof(SysParmNV) - 2,
                       INIT_CRC_SEED);
  if (crc != Pptr->CRC)                    /* EE's SysParm block look good? */
    {                                      /* No */
      EE_status |= EE_CRC;
      StatusB |= STSB_ERR_EEPROM;
    }

  Dptr = &sysptr->DateStampBlock;          /* Get EE's DateStamp block */
  crc = modbus_CRC ((unsigned char *)Dptr,
                      sizeof(DateStampNV) - 2,
                      INIT_CRC_SEED);
  if (crc != Dptr->CRC)                    /* EE's DateStamp block look good? */
  {                                        /* No */
#if 0 // DateStamp is only CRC'd when used, should be part of overall check
    EE_status |= EE_CRC; 
    StatusB |= STSB_ERR_EEPROM;
#else
    Nop();
#endif
  }
  D5ptr = &sysptr->Dia5Block;              /* Get EE's DIA5 block */
  crc = modbus_CRC ((unsigned char *)D5ptr,
                    sizeof(SysDia5NV) - 2,
                    INIT_CRC_SEED);
  if ( crc != D5ptr->CRC)                  /* EE's DIA5 block look good? */
  {                                        /* No */
    EE_status |= EE_CRC;
    StatusB |= STSB_ERR_EEPROM;
  }
  Vptr = &sysptr->VoltBlock;               /* Get EE's Volt block */
  crc = modbus_CRC ((unsigned char *)Vptr,
                      sizeof(SysVoltNV) - 2,
                      INIT_CRC_SEED);
  if ( crc != Vptr->CRC)                    /* EE's Volt block look good? */
  {                                         /* No */
    EE_status |= EE_CRC;
    StatusB |= STSB_ERR_EEPROM;
  }
  S1ptr = (SysSet1NV *)&sysptr->Set1Block;  /* Get EE's Set1 block */
  crc = modbus_CRC ((unsigned char *)S1ptr,
                        sizeof(SysSet1NV) - 2,
                        INIT_CRC_SEED);
  if (crc != S1ptr->CRC)                    /* EE's SysSet1 block look good? */
  {                                         /* No */
#if 0 // Set1Block is not used so not CRC'd, but should be as part of overall check
    EE_status |= EE_CRC; 
    StatusB |= STSB_ERR_EEPROM;
#else
    Nop();
#endif
  }
  if(EE_status || sts)
  {
    sts = TRUE;
  }else
  {
    sts = FALSE;
        }
  return sts;                               /* return is ignored but required */
}
/*********************** end of eeprom.C ************************************/
