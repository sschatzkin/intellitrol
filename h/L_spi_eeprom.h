/****************************************************************************
 *
 *  File:               L_spi_eeprom.h
 *
 *   Author:         Ken Langlais; edits by Dave Paquette
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *       Description:
 *           CONSTANTS, CODES AND ADDRESSES FOR ALL HARDWARE REGISTERS
 *           Prototype definitions for Loader specific functions.
 *
 * Revision History:
 *   Rev      Date        Who   Description of Change Made
 *  -------  --------  ----    --------------------------------------------
 *  1.5.28  06/13/14  DHP  Included file spimpol.h and deleted everything that was duplicated.
 *
****************************************************************************/
#include "spi_eeprom.h"

#ifndef L_SPI_EEPROM_H
#define L_SPI_EEPROM_H

/******************************* 3/16/2009 3:52PM ****************************
 * Function: SPI_EEPROMReadStatus()
 *
 * Preconditions: SPI module must be configured to operate with  SPI_EEPROM.
 *
 * Overview: This function reads status register from SPI_EEPROM.
 *
 * Input: None.
 *
 * Output: Status register value.
 *****************************************************************************/
int __attribute__((__section__(".LoaderSection"))) L_SPI_EEPROMReadStatus(char cmd, _SPI_EEPROMStatus_ *status);

/******************************* 3/16/2009 3:53PM ****************************
 * Function: SPI_EEPROMWriteByte()
 *
 * Preconditions: SPI module must be configured to operate with  SPI_EEPROM.
 *
 * Overview: This function writes a new value to address specified.
 *
 * Input: Data to be written and address.
 *
 * Output: None.
 *****************************************************************************/
int __attribute__((__section__(".LoaderSection"))) L_SPI_EEPROMWriteByte(unsigned long Address, unsigned Data);

/******************************* 3/16/2009 3:53PM ****************************
 * Function: SPI_EEPROMReadByte()
 *
 * Preconditions: SPI module must be configured to operate with  SPI_EEPROM.
 *
 * Overview: This function reads a value from address specified.
 *
 * Input: Address.
 *
 * Output: Data read.
 *****************************************************************************/
int __attribute__((__section__(".LoaderSection"))) L_SPI_EEPROMReadByte(unsigned long Address, unsigned char *data, unsigned int count);

/******************************* 3/16/2009 3:54PM ****************************
 * Function: SPI_EEPROMWriteEnable()
 *
 * Preconditions: SPI module must be configured to operate with SPI_EEPROM.
 *
 * Overview: This function allows a writing into SPI_EEPROM. Must be called
 * before every writing command.
 *
 * Input: None.
 *
 * Output: None.
 *****************************************************************************/
int __attribute__((__section__(".LoaderSection"))) L_SPI_EEPROMWriteEnable(void);

/******************************* 3/17/2009 7:38AM ****************************
 * This command will fetch and display the contents of the SPI status register
 * in the SPI EEPROM on the option board
 *****************************************************************************/

#endif    /* end of L_SPI_EEPROM_H */
