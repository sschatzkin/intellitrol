/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         test_function.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2010  Scully Signal Company
 *
 *  Description:  Ported from Microchip Class B sample code
 *
 *   Revision History:
 *   Rev      Date       Who   Description of Change Made
 * -------- ---------  ---      --------------------------------------------
 * 1.6.34  08/05/16  DHP   FogBugz 128: To allow consistency with XC16, changed 
 *                          variable type in CPU_PCtest() and switched to 
 *                          built_in to get function address.
 **********************************************************************/
#include "common.h"

  /********************************************************************
  * Description:
  * This function is placed in the section called "SslTestSection1".The section
  * is located in a defined address where the PC needs to jump using the custom linker script,
  * This function returns the address of the TestFunction1.
  * In this example the function TestFunction1 is placed in a section SslTestSection1 at the
  * address 0x900 by modifying the linker script as follows.
  *
  *  SslTestSection1 0x900 :
  *  {
  *      *(.SslTestSection1);
  *  } program
  *
  * Return Values:
  *      returnAddress: returns the address of TestFunction1
  *
  ********************************************************************/
unsigned long __attribute__((__section__(".SslTestSection1"))) TestFunction1() //FogBugz 128
 //FogBugz 128 unsigned long TestFunction1()
{
 //FogBugz 128   return(__builtin_tbladdress(&TestFunction1));
  return(__builtin_tbladdress(TestFunction1)); //FogBugz 128
}

/********************************************************************
  * Description:
  *  This function is placed in the section called "SslTestSection2".The section
  * is located in a defined address where the PC needs to jump using the custom linker script,
  * This function returns the address of the TestFunction2.
  * Return Values:
  *      returnAddress: returns the address of TestFunction2
  *
  ********************************************************************/

unsigned long __attribute__((__section__(".SslTestSection2"))) TestFunction2() //FogBugz 128
 //FogBugz 128 xxx unsigned long TestFunction2()
{
 //FogBugz 128   return(__builtin_tbladdress(&TestFunction2));
  return(__builtin_tbladdress(TestFunction2)); //FogBugz 128
}

