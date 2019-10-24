/****************************************************************************
 *
 *       File:       spimpol.h
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
#ifndef SPIMPOL_H
#define SPIMPOL_H

#define SPIM_BLOCKING_FUNCTION
#define SPIM_MODE16 0     /* Set to 8 bit mode */
#define SPIM_SMP 0        /* Input data sampled at middle of data output time */

/******************************* 3/17/2009 9:13AM ****************************
 * CKE: SPIx Clock Edge Select bit
 * 1 = Serial output data changes on transition from active clock state to Idle clock state (see SPIM_CKP)
 * 0 = Serial output data changes on transition from Idle clock state to active clock state (see SPIM_CKP)
 *****************************************************************************/
#define SPIM_CKE 0

/******************************* 3/17/2009 9:14AM ****************************
 * CKP: Clock Polarity Select bit
 * 1 = Idle state for clock is a high level; active state is a low level
 * 0 = Idle state for clock is a low level; active state is a high level
 *
 ****************************************************************************/
#define SPIM_CKP 0

 /****************************** 3/17/2009 10:21AM ***************************
 * Prescale Settings:
 * FSCK = FCY / (SPIM_PPRE * SPIM_SPRE)
 *  PPRE              SPRE
 *  11 - 1:1          111 - 1:1          011 - 5:1
 *  10 - 4:1          110 - 2:1          010 - 6:1
 *  01 - 16:1         101 - 3:1          001 - 7:1
 *  00 - 64:1         100 - 4:1          000 - 8:1
 *****************************************************************************/

/******************************* 3/17/2009 10:20AM ***************************
 * Set the sclk frequency to 10Mhz
 *****************************************************************************/
#define SPIM_PPRE (unsigned)0x3
#define SPIM_SPRE (unsigned)(0x6 << 2)


/******************************* 3/16/2009 3:24PM ****************************
 * This section defines names of control registers of SPI Module.
 *****************************************************************************/
#define SPIBUF  SPI2BUF
#define SPISTAT SPI2STAT
#define SPIBUFbits  SPI2BUFbits
#define SPISTATbits SPI2STATbits
#define SPIINTEN IEC2
#define SPIINTFLG IFS2
#define SPIINTENbits IEC2bits
#define SPIINTFLGbits IFS2bits
#define SPIIF SPI2IF
#define SPIIE SPI2IE
#define SPICON SPI2CON1
#define SPICONbits SPI2CON1bits
#define SPICON2 SPI2CON2
#define SPICON2bits SPI2CON2bits
#define GOOD 0
#define BAD 1

/******************************* 3/16/2009 3:25PM ****************************
 * Error and Status Flags
 * SPIM_STS_WRITE_COLLISION indicates that, Write collision has occurred
 * while trying to transmit the byte.
 *
 * SPIM_STS_TRANSMIT_NOT_OVER indicates that, the transmission is
 * not yet over. This is to be checked only when non Blocking
 * option is opted.
 *
 * SPIM_STS_DATA_NOT_READY indicates that reception SPI buffer is empty
 * and there's no data avalable yet.
 *****************************************************************************/
#define SPIM_STS_WRITE_COLLISION    1
#define SPIM_STS_TRANSMIT_NOT_OVER  2
#define SPIM_STS_DATA_NOT_READY     3

/******************************* 3/16/2009 3:26PM ****************************
 * Macro: mSPIMPolGet
 *
 * PreCondition: 'SPIMPolIsTransmitOver' should return a '0'.
 *
 * Overview: This macro reads a data received
 *
 * Input: None
 *
 *Output: Data received
 *****************************************************************************/
#define  mSPIMPolGet() SPIBUF

#endif                    /* SPIMPOL_H */
