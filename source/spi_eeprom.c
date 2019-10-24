/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         spi_eeprom.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009  Scully Signal Company
 *
 *   Description:    This file contains all the spi eeprom procedures used to access the
 *                   eeprom on the option board for Rack controller main microprocessor
 *                   PIC24HJ256GP610
 *
 *   Revision History:
 * -------- ---------  ---      --------------------------------------------
 * 1.5.31  01/14/15  DHP  Replaced lint -e(838) fix with (void) return calls
 *                                     Removed dummy_func() 
 *
 *****************************************************************************/
#include "common.h"
#include "spi_mpol.h"
#include "spi_eeprom.h"

/******************************* 3/17/2009 6:27AM ****************************
 * Function: SPI_EEPROMInit
 *
 * Preconditions: SPI module must be configured to operate with
 *                 parameters: Master, MODE16 = 0, CKP = 1, SMP = 1.
 *
 * Overview: This function setup SPI IOs connected to EEPROM.
 *
 * Input: None.
 *
 * Output: None.
 *****************************************************************************/
void SPI_EEPROMInit(void)
{
    // Set IOs directions for EEPROM SPI
    SPI_EEPROM_CE_PORT = 1;
    SPI_EEPROM_CE_TRIS = 0;
    SPI_EEPROM_SCK_TRIS = 0;
    SPI_EEPROM_SDO_TRIS = 0;
    SPI_EEPROM_SDI_TRIS = 1;
}

/******************************* 3/17/2009 6:28AM ****************************
 * Function: SPI_EEPROMWriteByte()
 *
 * Preconditions: SPI module must be configured to operate with  EEPROM.
 *
 * Overview: This function writes a new value to address specified.
 *
 * Input: Data to be written and address.
 *
 * Output: None.
 *
 *****************************************************************************/
int SPI_EEPROMWriteByte(unsigned long Address, unsigned Data)
{
_SPI_EEPROMStatus_ status;

  if (SPI_EEPROMWriteEnable() == BAD) return BAD;
  mSPI_EEPROMCELow();  /* dummy read */

  if (SPIMPolPut(SPI_EEPROM_CMD_WRITE)) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  if (SPIMPolPut(Hi(Address))) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  if (SPIMPolPut(Md(Address))) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  if (SPIMPolPut(Lo(Address))) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  if (SPIMPolPut(Data)) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  mSPI_EEPROMCEHigh();

  // wait for completion of previous write operation
  status.Bits.BSY = 1;
  while(status.Bits.BSY)
  {
    if (SPI_EEPROMReadStatus(SPI_EEPROM_CMD_RDSR, &status) == BAD) return BAD;
  }
  asm volatile("nop");    /* These NOPs are needed for the  */
  asm volatile("nop");    /* write cycle to complete */
  asm volatile("nop");    /*  */
  return GOOD;
}


/******************************* 3/17/2009 6:28AM ****************************
 * Function: SPI_EEPROMReadByte()
 *
 * Preconditions: SPI module must be configured to operate with  EEPROM.
 *
 * Overview: This function reads a value from address specified.
 *
 * Input: Address.
 *
 * Output: Data read.
 *****************************************************************************/
int SPI_EEPROMReadByte(unsigned long Address, unsigned char *data, unsigned int cnt)
{
unsigned int i;

  mSPI_EEPROMCELow();

  if (SPIMPolPut(SPI_EEPROM_CMD_READ)) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  if (SPIMPolPut(Hi(Address))) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  if (SPIMPolPut(Md(Address))) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  if (SPIMPolPut(Lo(Address))) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  for ( i=0; i<cnt; i++)
  {
    if (SPIMPolPut(0)) return BAD;
    *data++ = (unsigned char)mSPIMPolGet();
  }
  mSPI_EEPROMCEHigh();
  return GOOD;
}

/******************************* 3/20/2009 11:56AM ***************************
 * Function: SPI_EEPROMPageWriteByte()
 *
 * Preconditions: SPI module must be configured to operate with EEPROM.
 *
 * Overview: This function writes a new value to address specified.
 *
 * Input: Data to be written and address.
 *
 * Output: None.
 *
 *****************************************************************************/
int SPI_EEPROMPageWriteByte(unsigned long Address, const unsigned char *Data, unsigned int cnt)
{
_SPI_EEPROMStatus_ status;
unsigned int i;

  if (SPI_EEPROMWriteEnable()) return BAD;
  mSPI_EEPROMCELow();

  if (SPIMPolPut(SPI_EEPROM_CMD_WRITE)) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  if (SPIMPolPut(Hi(Address))) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  if (SPIMPolPut(Md(Address))) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  if (SPIMPolPut(Lo(Address))) return BAD;
  (void) mSPIMPolGet();  /* dummy read */

  for ( i=0; i<cnt; i++)
  {
    if (SPIMPolPut(*Data++)) return BAD;
    (void) mSPIMPolGet();  /* dummy read */
  }

  mSPI_EEPROMCEHigh();

  // wait for completion of previous write operation
  status.Bits.BSY = 1;
  while(status.Bits.BSY)
  {
    if (SPI_EEPROMReadStatus(SPI_EEPROM_CMD_RDSR, &status) == BAD) return BAD;
  }
  asm volatile("nop");    /* These NOPs are needed for the  */
  asm volatile("nop");    /* write cycle to complete */
  asm volatile("nop");    /*  */
  return GOOD;
} /* End SPI_EEPROMPageWriteByte() */

/******************************* 3/20/2009 11:49AM ***************************
 * This routine will perform a page write. A page size is 256 bytes.
 * The page must be erased before the write can be sucessful
 *****************************************************************************/
char SPIPageWrite(unsigned long loc, const unsigned char *datum, unsigned cnt)
{
unsigned int transfer_count;
char status;
unsigned long address;
unsigned int first_page;

  address = loc;

  /****************************** 11/10/2008 11:40AM *************************
   * Can only do a write up to a 128 byte page boundry
   ***************************************************************************/

  if ((first_page = (address % 0x100)) != 0)
  {
    transfer_count = 0x100 - first_page;
    if (transfer_count > cnt)
    {
      transfer_count = cnt;
    }
  }
  else
  {
    transfer_count = cnt;
    if (transfer_count>0x100)
    {
      transfer_count = 0x100;
    }
  }
  cnt -= transfer_count;

  if ((status = (char)SPI_EEPROMPageWriteByte(address,
                   datum, transfer_count)) != 0)
  {
    return status;
  }
  datum += transfer_count;
  address += transfer_count;

  while (cnt)
  {
    if ( cnt > 0x100)
    {
      transfer_count = 0x100;
      cnt -= 0x100;
    }
    else
    {
      transfer_count = cnt;
      cnt = 0;
    }
    if ((status = (char)SPI_EEPROMPageWriteByte(address,
                   datum, transfer_count)) != 0)
    {
      return status;
    }
    datum += transfer_count;
    address += transfer_count;
  }

  return status;

} /* End SPIPageWrite() */

/******************************* 3/17/2009 6:29AM ****************************
 * Function: SPI_EEPROMWriteEnable()
 *
 * Preconditions: SPI module must be configured to operate with EEPROM.
 *
 * Overview: This function allows a writing into EEPROM. Must be called
 * before every writing command.
 *
 * Input: None.
 *
 * Output: None.
 *****************************************************************************/
int SPI_EEPROMWriteEnable()
{
    mSPI_EEPROMCELow();
    if (SPIMPolPut(SPI_EEPROM_CMD_WREN)) return BAD;
    mSPIMPolGet();
    mSPI_EEPROMCEHigh();
    return GOOD;
}

/******************************* 3/17/2009 6:30AM ****************************
 * Function: SPI_EEPROMReadStatus()
 *
 * Preconditions: SPI module must be configured to operate with  EEPROM.
 *
 * Overview: This function reads status register from EEPROM.
 *
 * Input: None.
 *
 * Output: Status register value.
 *****************************************************************************/
int SPI_EEPROMReadStatus(unsigned char cmd, _SPI_EEPROMStatus_ *status)
{
unsigned int Temp;

    mSPI_EEPROMCELow();
    if (SPIMPolPut(cmd) == BAD) return BAD;
    (void) mSPIMPolGet();  /* Dummy Read */
    if (SPIMPolPut(0x00) == BAD) return BAD;
    Temp = mSPIMPolGet();  /* Read status */
    mSPI_EEPROMCEHigh();
    status->Char = (char)Temp;
    return GOOD;
}


/******************************* 3/17/2009 7:38AM ****************************
 * spi erase
 *****************************************************************************/
int spi_erase(void)
{
_SPI_EEPROMStatus_ status;
unsigned int  temp_data;

  if (SPI_EEPROMWriteEnable()) return BAD;
  mSPI_EEPROMCELow();
  if (SPIMPolPut(SPI_EEPROM_CMD_CHIP_ERASE)) return BAD;
  temp_data = mSPIMPolGet();  /* Dummy Read */
  mSPI_EEPROMCEHigh();
  status.Bits.BSY = 1;
  while(status.Bits.BSY)
  {
    if (SPI_EEPROMReadStatus(SPI_EEPROM_CMD_RDSR, &status)) return BAD;
  }

  dummy_func((unsigned char *)&temp_data);
  return GOOD;
}

/******************************* 3/17/2009 11:36AM ***************************
 * SPI_EEPROMReadDeviceID: Read the device ID
 *****************************************************************************/
unsigned long SPI_EEPROMReadDeviceID(void)
{
unsigned long temp_word;
unsigned int temp_data;

//    temp_word = 0;
    mSPI_EEPROMCELow();
    if (SPIMPolPut(SPI_EEPROM_CMD_DID) == BAD) return BAD;
    temp_data = mSPIMPolGet();  /* Dummy Read */
    if (SPIMPolPut(0x00)) return BAD;
    (void) mSPIMPolGet();
    temp_word = temp_data;
    if (SPIMPolPut(0x00)) return BAD;
    temp_data = mSPIMPolGet();
    temp_word |= ((unsigned long)temp_data << 8);
    if (SPIMPolPut(0x00)) return BAD;
    temp_data = mSPIMPolGet();
    temp_word |= (unsigned long)temp_data << 16;
    mSPI_EEPROMCEHigh();

    return temp_word;
}

int spi_erase_sector(unsigned long Address)
{
_SPI_EEPROMStatus_ status;

  if (SPI_EEPROMWriteEnable()) return BAD;
  mSPI_EEPROMCELow();
  if (SPIMPolPut(SPI_EEPROM_CMD_SECTOR_ERASE)) return BAD;
  (void) mSPIMPolGet();

  if (SPIMPolPut(Hi(Address))) return BAD;
  (void) mSPIMPolGet();

  if (SPIMPolPut(Md(Address))) return BAD;
  (void) mSPIMPolGet();

  if (SPIMPolPut(Lo(Address))) return BAD;
  (void) mSPIMPolGet();

  mSPI_EEPROMCEHigh();

//  status.Bits.BSY = 1;
  do
  {
    if (SPI_EEPROMReadStatus(SPI_EEPROM_CMD_RDSR, &status) == BAD) return BAD;
  } while(status.Bits.BSY);
  return GOOD;
}
/************************************************************************
 * EOF
 ************************************************************************/
