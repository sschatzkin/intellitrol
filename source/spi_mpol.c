/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         spimpol.c
 *
 *   Revision:       REV 1.0
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
 *
 *****************************************************************************/
#include "common.h"
#include "spi_mpol.h"
//#include "spi_eeprom.h"

/******************************* 3/17/2009 7:21AM ****************************
 * Function: SPIMPolInit
 *
 * Preconditions: TRIS bits of SCK and SDO should be made output.
 * TRIS bit of SDI should be made input. TRIS bit of Slave Chip Select
 * pin (if any used) should be made output. Overview This function is
 * used for initializing the SPI module. It initializes the module
 * according to Application Maestro options.
 *
 * Input: Application Maestro options
 *
 * Output: None
 *****************************************************************************/
void SPIMPolInit()
{
    SPISTAT = 0x40;
    SPICON = 0;
    SPICON = (SPIM_PPRE|SPIM_SPRE);
    SPICONbits.MSTEN = 1;
    SPICON2 = 0;
    SPICONbits.MODE16 = SPIM_MODE16;
    SPICONbits.CKE = SPIM_CKE;
    SPICONbits.CKP = SPIM_CKP;
    SPICONbits.SMP = SPIM_SMP;
    SPIINTENbits.SPIIE = 0;
    SPIINTFLGbits.SPIIF = 0;
    SPISTATbits.SPIEN = 1;
}

/******************************* 3/17/2009 7:22AM ****************************
 * Function SPIMPolPut
 *
 * Preconditions: 'SPIMPolInit' should have been called.
 * Overview: in non Blocking Option this function sends the byte
 * over SPI bus and checks for Write Collision; in Blocking Option
 * it waits for a free transmission buffer.
 *
 * Input: Data to be sent.
 *
 * Output: 'This function returns 0  on proper initialization of
 * transmission and SPIM_STS_WRITE_COLLISION on occurrence of
 * the Write Collision error.
 *****************************************************************************/
unsigned SPIMPolPut(unsigned Data)
{
    // Wait for a data byte reception
    while(SPISTATbits.SPITBF) {}
    SPIBUF = Data;
    if (SPIMPolIsTransmitOver()) return BAD;
  return GOOD;
}

/******************************* 3/17/2009 7:23AM ****************************
 * Function: SPIMPolIsTransmitOver
 *
 * Preconditions: SPIMPolPut should have been called.
 * Overview: in non Blocking Option this function checks whether
 * the transmission of the byte is completed; in Blocking Option
 * it waits till the transmission of the byte is completed.
 *
 * Input: None
 *
 * Output: in Blocking Option none and in non Blocking Option
 * it returns: 0 - if the transmission is over,
 * SPIM_STS_TRANSMIT_NOT_OVER - if the transmission is not yet over.
 *****************************************************************************/
unsigned SPIMPolIsTransmitOver()
{
int i;

  for ( i=0; i<10; i++)
  {
    if(SPISTATbits.SPIRBF) break;
    asm volatile("nop");    /* place for the breakpoint to stop */
    asm volatile("nop");    /* place for the breakpoint to stop */
    asm volatile("nop");    /* place for the breakpoint to stop */
  }
  if ( i>= 10)
  {
    return SPIM_STS_TRANSMIT_NOT_OVER;
  }
  return GOOD;
}


