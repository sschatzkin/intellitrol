/*********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         init_ports.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *  Description:  This file contain the functions that initialize the I/O ports
 *                for the main microprocessor PIC24HJ256GP210
 *                All unused pins are set as inputs
 *
 * Revision History:
 *   Rev      Date        Who   Description of Change Made
 *  --------  ------------   ----    --------------------------------------------
 *  1.5.28  05/15/14  DHP  Added port G ODC for IO Expander p14 and
 *                                       asserted signal to remove reset.
 *  1.5.30  08/10/14  DHP  Replaced TRUCK_HERE with TB3P5P6
 *  1.6.34  07/08/16  DHP  QCCC 53: Changed TRISA/B/D and F for HC_OFF
 *
 *********************************************************************************************/
#include "common.h"

void init_ports()
{
  /******************************6/2/2008 6:53AM******************************
   * initialize PORT A
   * MMAIN_CONTACT and MBACK_CONTACT as inputs
   >>> QCCC 53
   * set HC_OFF4/7/8/3/6/1(4, 5, 6, 7, 12, 15), MAIN_CHARGE(13) as output
   <<< QCCC 53
   ***************************************************************************/
  MAIN_CHARGE = CLR;      /* Make sure we don't permit */
//    TRISA = 0xCFFF;                    /* QCCC 53 change from 0xCFFF */
  TRISA = 0x4F0F;                   /* QCCC 53 change from 0xCFFF */

  /******************************6/2/2008 7:23AM******************************
   * initialize PORT B
   * set PULSE5VOLT as output
   >>> QCCC 53
   * set HC_OFF5(10), SERIALBIT(15), PULSE5VOLT(14) as output
   ***************************************************************************/
  //CSBOOT = SET;             //QCCC 53 change - not used
// <<< QCCC 53
  PULSE5VOLT = CLR;       /* Default Chan 4 normal (10V) drive */
  SERIAL_BIT = 1; // Drives DQ high
//    TRISB = 0x2BFF;
  TRISB = 0x2BFF;              /* QCCC 53 DHP_PULSE set as output */

  /******************************6/2/2008 7:25AM******************************
   * initialize PORT C
   * set MTXEN as output
   ***************************************************************************/
  MBLINK1 = 0;
  MBLINK2 = 0;
  MBLINK3 = 0;
  TRISC = 0x7801;

  /******************************6/2/2008 7:40AM******************************
   * initialize PORT D
   * set MBACK_CHARGE as input
   * READ_BYPASS and COMM_ID are bidirectional default as input
   *
   * >>> QCCC 53
   * set READ_COMM_ID_BIT, READ_BYPASS_BIT, DEBUG_IN,  as input
   * GCHECK is bidirectional; default as input
   * COMM_ID_BIT(0), BYPASS_BIT(2), HC_OFF2(5), MSTAT_LED(6), MUX2(7), TXEN(9),
   * TB3P5P6(12), MSERV(13) and SCALE_AN0/1 (14, 15) as outputs
   <<< QCCC 53
   ***************************************************************************/
  TXEN  = 0;               /* Turn off the transmiter to the MODBUS */

  TB3P5P6 = 1;          /* Turn off active low output */
  MSERV_TRG_NEG = 0;
  MSTAT_LED = 0;
  COMM_ID_BIT = 1;        /* Drive pin high */
  BYPASS_BIT = 1;         /* Drive pin high */
  ODCDbits.ODCD0 = 1;     /* enable open drain for bi-directional */
  ODCDbits.ODCD2 = 1;     /* enable open drain for bi-directio */
//  TRISD = 0x0D3A;         /* QCCC 53 change from 0x0D3A */
  TRISD = 0x0D1A;         /* QCCC 53 change from 0x0D3A */

  /******************************6/2/2008 7:51AM******************************
   * initialize PORT E
   * Turn on all test channel probes pins 0 - 7 for 8 probes
   ***************************************************************************/
  TRISE = 0xFF00;

  /******************************6/2/2008 8:03AM******************************
   * initialize PORT F
   * set JUMP_START, DIAGNOSTIC_EN, LED_DISP_CLK, MAIN_ENABLE, and
   * LED_DISP_DAT as outputs
   * set METER_ON_NEG as input
   ***************************************************************************/
  LATF = 0x207A;          /* Turn off JumpStart by changing from a 2073 to a 2072 on 01-29-08
                           * to fix backup processot from thinking 8 wired truck */
  MAIN_ENABLE = CLR;      /* Disable permit relay */
  LED_DISP_CLK = CLR;

//  TRISF = 0xEE34;    // >>> QCCC 53
  TRISF = 0xFE34;

  /******************************6/2/2008 8:37AM******************************
   * initialize PORT G
   ***************************************************************************/
  ODCG  = 0x4000;
  COMM_RESET = SET;
  TRISG = 0x000C;
  DIS_MARX = 0;      /* Enable the modbus communication bus */
  LATGbits.LATG14 = 1;     /* Remove reset from IO Expander */
  LATGbits.LATG15 = 0;     /* Clear m_OB_CS */
  SCALE_AN1 = 0;
  SCALE_AN0 = 0;

  /******************************6/3/2008 9:34AM******************************
   * Enable AN0 to AN7 as analog inputs
   ***************************************************************************/
  AD1PCFGL = 0xFF00;
  AD1PCFGH = 0xFFFF;
}
