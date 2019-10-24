/********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         i2c.c
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
 *  --------  --------   ----    --------------------------------------------
 *   1.5.28  05/13/14  DHP  Added a STOP to MasterGetsI2C2() and MasterWriteI2C2()
 *                                        on an ACK/NACK timeout.
 *                                       Changed OpenI2C2() call to use variables, not absolute values.
 *                                       Added assertion of PEN (stop) to operations after timeout.
 *                                       Shortened wait time for ACK.
 *   1.5.31  01/14/15  DHP  Changed baud rate from 1 MHz to 400 Mhz
 *                                       Corrected 1st while loop in MasterWriteI2C1()
 *                                         to actually wait (change "<" to ">")
 ********************************************************************************************/
#include "common.h"
#include "i2c.h"

/************************************************************************
*    Function Name:  DataRdyI2C2
*    Description:    This routine provides the status whether the receive
*                    buffer is full by returning the RBF bit.
*    Parameters:     void
*    Return Value:   RBF bit status
*************************************************************************/

char DataRdyI2C2(void)
{
     return I2C2STATbits.RBF;
}

/************************************************************************
*    Function Name:  MastergetsI2C
*    Description:    This routine reads predetermined data string length
*                    from the I2C bus.
*    Parameters:     UINT16    : length
*                    UINT8 * : rdptr
*    Return Value:   UINT16
*************************************************************************/

int MastergetsI2C2(UINT16 length, UINT8 *rdptr,
                            UINT16 I2C2_data_wait)
{
UINT16 timeout;
unsigned int wait = 0;

    while(length)                    /* Receive length bytes */
    {
        I2C2CONbits.RCEN = 1;
        while(DataRdyI2C2() == 0)
        {
          if(wait < I2C2_data_wait)
              wait++ ;
          else
          {
            /********** DEBUG STATES *****************/
            printf("\n\rReceive Data time out: 0x%04X\n\r",I2C2STAT);
            /********** DEBUG STATES *****************/
            return -1;              /* Time out,
                                      return number of byte/word to be read */
          }
        }
        wait = 0;
        *rdptr = (unsigned char)I2C2RCV;            /* save byte received */
        rdptr++;
        length--;
        if(length == 0)              /* If last char, generate NACK sequence */
        {
            I2C2CONbits.ACKDT = 1;
            I2C2CONbits.ACKEN = 1;
        }
        else                         /* For other chars,generate ACK sequence */
        {
            I2C2CONbits.ACKDT = 0;
            I2C2CONbits.ACKEN = 1;
        }
        timeout = 10000;
        while(I2C2CONbits.ACKEN && (--timeout !=0))
        {
          DelayUS(1);
        }
        if ( timeout == 0)
        {
          /********** DEBUG STATES *****************/
          printf("\n\r1: MastergetsI2C2: ACK/NACK time out: I2C2CON: 0x%04X\n\r",I2C2CON);
          /********** DEBUG STATES *****************/
          I2C2CONbits.PEN = 1;    /* End current operation */
          return -1;
        }
    }
    /* return status that number of bytes specified by length was received */
    return 0;
}

/************************************************************************
*    Function Name:  MasterWriteI2C2
*    Description:    This routine is used to write a byte to the I2C bus.
*                    The input parameter data_out is written to the
*                    I2C2TRN register. If IWCOL bit is set,write collision
*                    has occured and -1 is returned, else 0 is returned.
*    Parameters:     UINT8 : data_out
*    Return Value:   UINT16
*************************************************************************/

char MasterWriteI2C2(UINT8 data_out)
{
UINT32 timeout;

  DelayUS(1);
  timeout = (read_time() + MSec5);    /* Wait for up to 5 Milliseconds */
  do
  {
    I2C2STATbits.IWCOL = 0;
  } while ((timeout > read_time()) && (I2C2STATbits.IWCOL));

  if(I2C2STATbits.IWCOL)        /* If collision occurs, return -1 */
  {
      I2C2STATbits.IWCOL = 0;
      /********** DEBUG STATES *****************/
      printf("\n\rIWCOL write collision: 0x%04X\n\r",I2C2STAT);
      /********** DEBUG STATES *****************/
      return -1;
  }
  if(I2C2STATbits.BCL)        /* If Master Bus collision occurs,return -1 */
  {
      /********** DEBUG STATES *****************/
      printf("\n\rBCL bus collision: 0x%04X\n\r",I2C2STAT);
      /********** DEBUG STATES *****************/
      return -1;
  }
  timeout = 1000;
  /* wait on Transmit Buffer Full */
  while(I2C2STATbits.TBF && (--timeout !=0))
  {
    DelayUS(1);
  }
  if ( timeout == 0)
  {
    /********** DEBUG STATES *****************/
    printf("\n\rTransmit Buffer Full time out: 0x%04X\n\r",I2C2STAT);
    /********** DEBUG STATES *****************/
    return -1;
  }

  I2C2TRN = data_out;

  timeout = read_time() + MSec5;    /* Wait for up to 5 Milliseconds */
  while ((timeout < read_time()) && (I2C2STATbits.IWCOL))
  {
    I2C2STATbits.IWCOL = 0;
    DelayUS(1);
    I2C2TRN = data_out;
  }

  if(I2C2STATbits.IWCOL)        /* If write collision occurs,return -1 */
  {
      I2C2STATbits.IWCOL = 0;
      /********** DEBUG STATES *****************/
      printf("\n\rIWCOL write collision: 0x%04X\n\r",I2C2STAT);
      /********** DEBUG STATES *****************/
      return -1;
  }

  if(I2C2STATbits.BCL)        /* If write collision occurs,return -1 */
  {
      /********** DEBUG STATES *****************/
      printf("\n\rBCL write collision: 0x%04X",I2C2STAT);
      /********** DEBUG STATES *****************/
      return -1;
  }

  timeout = 1000;
  while(I2C2STATbits.TBF && (--timeout !=0))
  {
    DelayUS(1);
  }
  if ( timeout == 0)
  {
    /********** DEBUG STATES *****************/
    printf("\n\rTransmit Buffer Full time out: 0x%04X\n\r",I2C2STAT);
    /********** DEBUG STATES *****************/
    return -1;
  }

  timeout = 1000;
  while(I2C2STATbits.TRSTAT && (--timeout !=0))
  {
    DelayUS(1);
  }
  if ( timeout == 0)
  {
    /********** DEBUG STATES *****************/
    printf("\n\rStop condition time out: 0x%04X\n\r",I2C2STAT);
    /********** DEBUG STATES *****************/
    return -1;
  }

  // wait for ack
  timeout = 1000;
  while(I2C2STATbits.ACKSTAT && (--timeout !=0))
  {
    DelayUS(1);
  }
  if ( timeout == 0)
  {
    /********** DEBUG STATES *****************/
    printf("\n\r2: MasterWriteI2C2: ACK/NACK time out: I2C2STAT: 0x%04X\n\r",I2C2STAT);
    /********** DEBUG STATES *****************/
    I2C2CONbits.PEN = 1;    /* End current operation */
    return -1;
  }
  return 0;
}

/******************************************************************************
*    Function Name:  OpenI2C2
*    Description:    This function configures the I2C module after disabling.
*                    enable bit,
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

void OpenI2C2(UINT16 config1, UINT16 config2)
{
  I2C2CON = config1;
  I2C2BRG = config2;
}

/*********************************************************************
*    Function Name:  RestartI2C2
*    Description:    This routine generates Restart condition
*                    during master mode.
*    Parameters:     void
*    Return Value:   void
*********************************************************************/

void RestartI2C2(void)
{
    I2C2CONbits.RSEN = 1; /* initiate restart on SDA and SCL pins */
}

/*********************************************************************
*    Function Name:  StartI2C2
*    Description:    This routine generates Start condition
*                    during master mode.
*    Parameters:     void
*    Return Value:   void
*********************************************************************/

void StartI2C2(void)
{
  I2C2CONbits.SEN = 1;  /* initiate Start on SDA and SCL pins */
}

/*********************************************************************
*    Function Name:  StopI2C
*    Description:    This routine generates Stop condition
*                    during master mode.
*    Parameters:     void
*    Return Value:   void
*********************************************************************/

int StopI2C2(void)
{
UINT16 timeout;

  I2C2CONbits.PEN = 1;  /* initiate Stop on SDA and SCL pins */
//  DelayUS(5);
  timeout = 1000;
  while(I2C2CONbits.PEN && (--timeout !=0))
  {
    DelayUS(1);
  }
  if ( timeout == 0)
  {
    /********** DEBUG STATES *****************/
    printf("\n\rStop condition time out: 0x%04X\n\r",I2C2CON);
    /********** DEBUG STATES *****************/
    return -1;
  }
  return 0;
}

int I2C2_reset(void)
{
int i;
int status;
unsigned int save_trisa;

  /******************************5/29/2008 11:53AM****************************
   * Turnoff the I2C bus
   ***************************************************************************/
  OpenI2C2(0x0000,0);     /* Disable I2C and clear Baud Rate */
  save_trisa = TRISA;     /* Save original setting */
  TRISA &= ~0x4;          /* Set I2C clock pin to output (PIN2 LOW) */
  TRISA &= 0x8;           /* and I2C data pin to input (PIN3 HIGH) */

  PORTAbits.RA2=0;            /*  */
  for ( i=0; i<9; i++)    /* output 9 clock pulses */
  {
    DelayUS(10);
    PORTAbits.RA2=1;          /*  */
    DelayUS(10);
    PORTAbits.RA2=0;          /*  */
  }
  DelayUS(10);
  if(PORTAbits.RA3)
  {
    status = PASSED;
  }
  else
  {
    status = FAILED;
  }
  TRISA = save_trisa;     /* Restore port A input/output settings */
  OpenI2C2(I2C2_CON,(unsigned int)I2C2_BRG);     /* Enable I2C2 and set Baud Rate */
  return status;
}

/*******************************6/11/2008 6:22AM******************************
 * Function Name:  I2C2_write
 * Description:    This routine writes a block of bytes to a I2C device
 *                 in master mode on I2C bus 2.
 * Parameters:     device_addr, reg_addr, pointer to the block of data,
 *                 length of block
 * Return Value:   error
 *****************************************************************************/
UINT16 I2C2_write(UINT8 device_addr, UINT8 reg_addr, const UINT8 *data_ptr,
UINT8 length)
{
UINT8 i;

  // start I2C sequence
  StartI2C2();
  // write 8 bit address
  if (MasterWriteI2C2(device_addr)) return 0x11;
  // write 8 bit register address
  if (MasterWriteI2C2(reg_addr)) return 0x12;
  // write length bytes of data
  for ( i=0; i<length; i++)
  {
    if (MasterWriteI2C2(*data_ptr++))
    {
      return 0x14;
    }
  }
  if (StopI2C2()) return 0x15;
  return 0;
}

/*******************************5/19/2008 8:09AM******************************
 * Function Name:  I2C2_read
 * Description:    This routine reads a block of bytes from a I2C device
 *                 in master mode on I2C bus 1.
 * Parameters:     device_addr, reg_addr, pointer to the block of data,
 *                 length of block
 *****************************************************************************/
UINT16 I2C2_read(UINT8 device_addr,  UINT8 reg_addr, UINT8 *data_ptr,
    UINT8 length)
{

  // start I2C sequence
  StartI2C2();

  if (MasterWriteI2C2(device_addr)) return 0x16;

  // write 8 bit address
  if (MasterWriteI2C2(reg_addr)) return 0x17;

  // start I2C sequence
  RestartI2C2();

  /******************************5/23/2008 2:38PM*****************************
   * write Device address - Start read cycle
   ***************************************************************************/
  if (MasterWriteI2C2(device_addr|1) == 0)
  {
    // wait for data to end transmission
    if (MastergetsI2C2(length, data_ptr, 1000)) return 9;
  } else
  {
    return 10;
  }

  if (StopI2C2()) return 12;
  return 0;
}


