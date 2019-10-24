/****************************************************************************
 *
 *       File:       spi_eeprom.h
 *
 *       Author:     Ken Langlais
 *
 *       @Copyright 2008  Scully Signal Company
 *
 *       Description:
 *           CONSTANTS, CODES AND ADDRESSES FOR ALL HARDWARE REGISTERS
 *
 *       Revision History:
 *
****************************************************************************/
#ifndef SPI_EEPROM_H
#define SPI_EEPROM_H

/******************************* 3/16/2009 3:47PM ****************************
 * EEPROM Commands
 *****************************************************************************/
#define SPI_EEPROM_PAGE_SIZE        (unsigned)256
#define SPI_EEPROM_PAGE_MASK        (unsigned)0x00FF
#define SPI_EEPROM_CMD_READ         (unsigned)0x03      /* Read Data */
#define SPI_EEPROM_CMD_WRITE        (unsigned)0x02      /* Page Program (Write bytes) */
#define SPI_EEPROM_CMD_WRDI         (unsigned)0x04      /* Write Disabled */
#define SPI_EEPROM_CMD_WREN         (unsigned)0x06      /*  Write Enable */
#define SPI_EEPROM_CMD_RDSR         (unsigned)0x05      /* Read Status Register */
#define SPI_EEPROM_CMD_WRSR         (unsigned)0x01      /* Write Status Register */
#define SPI_EEPROM_CMD_DID          (unsigned)0x9F      /* Manufacturing Device ID */
#define SPI_EEPROM_CMD_CHIP_ERASE   (unsigned)0xC7      /* Chip erase */
#define SPI_EEPROM_CMD_BLOCK_ERASE  (unsigned)0xD8      /* Block erase */
#define SPI_EEPROM_CMD_SECTOR_ERASE (unsigned)0x20      /* Sector erase */

/******************************* 3/16/2009 2:59PM ****************************
 * SPI 2 Interface to communicate with SPI SPI_EEPROM on Option Board
 *****************************************************************************/
#define SPI_EEPROM_CE_TRIS      TRISGbits.TRISG15
#define SPI_EEPROM_CE_PORT      PORTGbits.RG15  /* SPI chip select */
#define SPI_EEPROM_SCK_TRIS     TRISGbits.TRISG6
#define SPI_EEPROM_SDO_TRIS     TRISGbits.TRISG8
#define SPI_EEPROM_SDI_TRIS     TRISGbits.TRISG7

/******************************* 3/16/2009 3:49PM ****************************
 * Macro: Lo
 *
 * Preconditions: None
 *
 * Overview: This macro extracts a low byte from a 3 byte word.
 *
 * Input: None.
 *
 * Output: None.
 *****************************************************************************/
#define Lo(X)   (unsigned char)((unsigned long)X&0x00ff)

/******************************* 3/16/2009 3:49PM ****************************
 * Macro: Md
 *
 * Preconditions: None
 *
 * Overview: This macro extracts a middle byte from a 3 byte word.
 *
 * Input: None.
 *
 * Output: None.
 *****************************************************************************/
#define Md(X)   (unsigned char)(((unsigned long)X>>8)&0x00ff)

/******************************* 3/16/2009 3:49PM ****************************
 * Macro: Hi
 *
 * Preconditions: None
 *
 * Overview: This macro extracts a high byte from a 3 byte word.
 *
 * Input: None.
 *
 * Output: None.
 *****************************************************************************/
#define Hi(X)   (unsigned char)(((unsigned long)X>>16)&0x00ff)

/******************************* 3/16/2009 3:50PM ****************************
 * Macro: mSPI_EEPROMCELow
 *
 * Preconditions: CE IO must be configured as output.
 *
 * Overview: This macro pulls down CE line
 *           to start a new SPI_EEPROM operation.
 *
 * Input: None.
 *
 * Output: None.
 *****************************************************************************/
#define mSPI_EEPROMCELow()      SPI_EEPROM_CE_PORT=0

/******************************* 3/16/2009 3:51PM ****************************
 * Macro: mSPI_EEPROMCEHigh
 *
 * Preconditions: CE IO must be configured as output.
 *
 * Overview: This macro set CE line to high level
 *           to start a new SPI_EEPROM operation.
 *
 * Input: None.
 *
 * Output: None.
 *****************************************************************************/
#define mSPI_EEPROMCEHigh()     SPI_EEPROM_CE_PORT=1

#endif    /* end of SPI_EEPROM_H */
