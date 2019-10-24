/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         i2c_1.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *   Description:    This file contains all the I2C procedures for Rack controller main
 *                   microprocessor PIC24HJ256GP610
 *
 *   Revision History:
 *   Rev      Date        Who   Description of Change Made
 *  --------  -------------  -----    --------------------------------------------
 * 1.5.28  05/13/14  DHP  Added assertion of PEN (stop) to operations after timeout.
 * 1.5.31  01/14/15  DHP  Corrected 1st while loop in MasterWriteI2C1()
 *                                       to actually wait (change "<" to ">")
 *
 *****************************************************************************/
#include "common.h"
#include "i2c.h"

/************************************************************************
*    Function Name:  DataRdyI2C1
*    Description:    This routine provides the status whether the receive
*                    buffer is full by returning the RBF bit.
*    Parameters:     void
*    Return Value:   RBF bit status
*************************************************************************/

char DataRdyI2C1(void)
{
     return I2C1STATbits.RBF;
}

/************************************************************************
*    Function Name:  MastergetsI2C
*    Description:    This routine reads predetermined data string length
*                    from the I2C bus.
*    Parameters:     UINT16    : length
*                    UINT8 * : rdptr
*    Return Value:   UINT16
*************************************************************************/

int MastergetsI2C1(UINT16 length, UINT8 *rdptr,
                            UINT16 I2C1_data_wait)
{
UINT16 timeout;
unsigned int wait = 0;

    while(length)                    /* Receive length bytes */
    {
        I2C1CONbits.RCEN = 1;
        while(DataRdyI2C1() == 0)
        {
          if(wait < I2C1_data_wait)
              wait++ ;
          else
          {
            /********** DEBUG STATES *****************/
            printf("\n\rReceive Data time out: 0x%04X\n\r",I2C1STAT);
            /********** DEBUG STATES *****************/
            return -1;              /* Time out,
                                      return number of byte/word to be read */
          }
        }
        wait = 0;
        *rdptr = (unsigned char)I2C1RCV;            /* save byte received */
        rdptr++;
        length--;
        if(length == 0)              /* If last char, generate NACK sequence */
        {
            I2C1CONbits.ACKDT = 1;
            I2C1CONbits.ACKEN = 1;
        }
        else                         /* For other chars,generate ACK sequence */
        {
            I2C1CONbits.ACKDT = 0;
            I2C1CONbits.ACKEN = 1;
        }
        timeout = 10000;
        while(I2C1CONbits.ACKEN && (--timeout !=0))
        {
          DelayUS(1);
        }
        if ( timeout == 0)
        {
          /********** DEBUG STATES *****************/
          printf("\n\rMastergetsI2C1: ACK/NACK time out: I2C1CON: 0x%04X\n\r",I2C1CON);
          /********** DEBUG STATES *****************/
          return -1;
        }
    }
    /* return status that number of bytes specified by length was received */
    return 0;
}

/************************************************************************
*    Function Name:  MasterWriteI2C1
*    Description:    This routine is used to write a byte to the I2C bus.
*                    The input parameter data_out is written to the
*                    I2CTRN register. If IWCOL bit is set,write collision
*                    has occured and -1 is returned, else 0 is returned.
*    Parameters:     UINT8 : data_out
*    Return Value:   UINT16
*************************************************************************/
char MasterWriteI2C1(UINT8 data_out)
{
UINT32 timeout;

  DelayUS(1);
  timeout = read_time() + MSec5;    /* Wait for up to 5 Milliseconds */
  do
  {
    I2C1STATbits.IWCOL = 0;
  } while ((timeout > read_time()) && (I2C1STATbits.IWCOL));

  if(I2C1STATbits.IWCOL)        /* If write collision occurs,return -1 */
  {
      I2C1STATbits.IWCOL = 0;
      /********** DEBUG STATES *****************/
      printf("\n\r1: IWCOL write collision: 0x%04X\n\r",I2C1STAT);
      /********** DEBUG STATES *****************/
      return -1;
  }
  if(I2C1STATbits.BCL)        /* If write collision occurs,return -1 */
  {
      /********** DEBUG STATES *****************/
      printf("\n\rBCL write collision: 0x%04X\n\r",I2C1STAT);
      /********** DEBUG STATES *****************/
      return -1;
  }

  timeout = 1000;
  while(I2C1STATbits.TBF && (--timeout !=0))
  {
    DelayUS(1);
  }

  if ( timeout == 0)
  {
    /********** DEBUG STATES *****************/
    printf("\n\rTransmit Buffer Full time out: 0x%04X\n\r",I2C1STAT);
    /********** DEBUG STATES *****************/
    return -1;
  }

  I2C1TRN = data_out;

  timeout = read_time() + MSec5;    /* Wait for up to 5 Milliseconds */
  while ((timeout < read_time()) && (I2C1STATbits.IWCOL))
  {
    I2C1STATbits.IWCOL = 0;
    DelayUS(1);
    I2C1TRN = data_out;
  }

  if(I2C1STATbits.IWCOL)        /* If write collision occurs,return -1 */
  {
      I2C1STATbits.IWCOL = 0;
      /********** DEBUG STATES *****************/
      printf("\n\r2: IWCOL write collision: 0x%04X  data: 0x%02X\n\r", I2C1STAT, data_out);
      /********** DEBUG STATES *****************/
      return -1;
  }

  if(I2C1STATbits.BCL)        /* If write collision occurs,return -1 */
  {
      /********** DEBUG STATES *****************/
      printf("\n\rBCL write collision: 0x%04X\n\r",I2C1STAT);
      /********** DEBUG STATES *****************/
      return -1;
  }

  timeout = 1000;
  while(I2C1STATbits.TBF && (--timeout !=0))
  {
    DelayUS(1);
  }
  if ( timeout == 0)
  {
    /********** DEBUG STATES *****************/
    printf("\n\rTransmit Buffer Full time out: 0x%04X\n\r",I2C1STAT);
    /********** DEBUG STATES *****************/
    return -1;
  }

  timeout = 1000;
  while(I2C1STATbits.TRSTAT && (--timeout !=0))
  {
    DelayUS(1);
  }
  if ( timeout == 0)
  {
    /********** DEBUG STATES *****************/
    printf("\n\rStop condition time out: 0x%04X\n\r",I2C1STAT);
    /********** DEBUG STATES *****************/
    return -1;
  }

  // wait for ack
  timeout = 10000;
  while(I2C1STATbits.ACKSTAT && (--timeout !=0))
  {
    DelayUS(1);
  }
  if ( timeout == 0)
  {
    /********** DEBUG STATES *****************/
    printf("\n\rMasterWriteI2C1: ACK/NACK time out: I2C1STAT: 0x%04X\n\r",I2C1STAT);
    /********** DEBUG STATES *****************/
    return -1;
  }
  return 0;
}

/******************************************************************************
*    Function Name:  OpenI2C1
*    Description:    This function configures the I2C module for enable bit,
*                    disable slew rate, SM bus input levels, SCL release,
*                    Intelligent Peripheral Management Interface enable,
*                    sleep mode, general call enable,acknowledge data bit,
*                    acknowledge sequence enable, receive enable, stop
*                    condition enable, restart condition enable and start
*                    condition enable. The Baud rate value is also configured
*    Parameters:     UINT16 : config1
*                    UINT16 : config2
*    Return Value:   void
*******************************************************************************/

void OpenI2C1(UINT16 config1, UINT16 config2)
{
  I2C1BRG = config2;
  I2C1CON = config1;
}

/*********************************************************************
*    Function Name:  RestartI2C1
*    Description:    This routine generates Restart condition
*                    during master mode.
*    Parameters:     void
*    Return Value:   void
*********************************************************************/

void RestartI2C1(void)
{
    I2C1CONbits.RSEN = 1; /* initiate restart on SDA and SCL pins */
}

/*********************************************************************
*    Function Name:  StartI2C1
*    Description:    This routine generates Start condition
*                    during master mode.
*    Parameters:     void
*    Return Value:   void
*********************************************************************/

void StartI2C1(void)
{
  I2C1CONbits.SEN = 1;  /* initiate Start on SDA and SCL pins */
}

/*********************************************************************
*    Function Name:  StopI2C
*    Description:    This routine generates Stop condition
*                    during master mode.
*    Parameters:     void
*    Return Value:   void
*********************************************************************/

int StopI2C1(void)
{
UINT16 timeout;

  I2C1CONbits.PEN = 1;  /* initiate Stop on SDA and SCL pins */
//  DelayUS(5);
  timeout = 1000;
  while(I2C1CONbits.PEN && (--timeout !=0))
  {
    DelayUS(1);
  }

  if ( timeout == 0)
  {
    /********** DEBUG STATES *****************/
    printf("\n\rStop condition time out: 0x%04X\n\r",I2C1CON);
    /********** DEBUG STATES *****************/
    return -1;
  }
  return 0;
}

int I2C1_reset(void)
{
int i;
int status;
unsigned int save_trisg;

  /******************************5/29/2008 11:53AM****************************
   * Turnoff the I2C bus
   ***************************************************************************/
  OpenI2C1(0x0000,0);     /* Disable I2C and clear Baud Rate */
  save_trisg = TRISG;     /* Save original setting */
  TRISG &= ~0x4;          /* Set I2C clock pin to output (PIN2 LOW) */
  TRISG &= 0x8;           /* and I2C data pin to input (PIN3 HIGH) */

  PORTGbits.RG2=0;            /*  */
  for ( i=0; i<9; i++)    /* output 9 clock pulses */
  {
    DelayUS(10);
    PORTGbits.RG2=1;          /*  */
    DelayUS(10);
    PORTGbits.RG2=0;          /*  */
  }
  DelayUS(10);
  if(PORTGbits.RG3)
  {
    /********** DEBUG STATES *****************/
    status = PASSED;
    /********** DEBUG STATES *****************/
  }
  else
  {
    /********** DEBUG STATES *****************/
    status = FAILED;
    /********** DEBUG STATES *****************/
  }
  TRISG = save_trisg;     /* Restore port G input/output settings */
  OpenI2C1(0x8200,(unsigned int)I2C1_BRG);     /* Enable I2C and set Baud Rate */
  return status;
}

/*******************************6/11/2008 6:22AM******************************
 * Function Name:  I2C1_write
 * Description:    This routine write a block of bytes to a I2C device
 *                 in master mode on I2C bus 1.
 * Parameters:     device_addr, reg_addr, pointer to the block of data,
 *                 length of block
 * Return Value:   error
 *****************************************************************************/
UINT16 I2C1_write(UINT8 device_addr, UINT8 reg_addr, const UINT8 *data_ptr,
UINT8 length)
{
UINT8 i;

  // start I2C sequence
  StartI2C1();

  if (MasterWriteI2C1(device_addr)) return 0x11;

    // write 8 bit address
  if (MasterWriteI2C1(reg_addr)) return 0x12;

  for ( i=0; i<length; i++)
  {
    if (MasterWriteI2C1(*data_ptr++))
    {
      return 0x14;
    }
  }

  if (StopI2C1()) return 0x15;
  return 0;
}

/*******************************5/19/2008 8:09AM******************************
 * Function Name:  I2C1_read
 * Description:    This routine reads a block of bytes from a I2C device
 *                 in master mode on I2C bus 1.
 * Parameters:     device_addr, reg_addr, pointer to the block of data,
 *                 length of block
 *****************************************************************************/
UINT16 I2C1_read(UINT8 device_addr,  UINT8 reg_addr, UINT8 *data_ptr,
    UINT8 length)
{

  // start I2C sequence
  StartI2C1();

  if (MasterWriteI2C1(device_addr)) return 0x16;

  // write 8 bit address
  if (MasterWriteI2C1(reg_addr)) return 0x17;

  // start I2C sequence
  RestartI2C1();

  /******************************5/23/2008 2:38PM*****************************
   * write Device address - Start read cycle
   ***************************************************************************/
  if (MasterWriteI2C1(device_addr|1) == 0)
  {
    // wait for data to end transmission
    if (MastergetsI2C1(length, data_ptr, 1000)) return 9;
  } else
  {
    return 10;
  }

  if (StopI2C1()) return 12;
  return 0;
}


