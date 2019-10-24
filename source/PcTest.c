/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         PcTest.c
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2010  Scully Signal Company
 *
 *  Description:  CPU register test ported from Microchip Class B sample
 *                code
 *
 *   Revision History:
 *   Rev    Date     Who   Description of Change Made
 * 1.6.34  08/05/16  DHP   FogBugz 128: To allow consistency with XC16, changed 
 *                          variable type in CPU_PCtest() and switched to 
 *                          built_in to get function address.
 **********************************************************************/
#include "common.h"

  /*******************************************************************
  * Description:
  * The Program Counter test is a functional test of the PC. The PC holds the address
  * of the next instruction tO be executed.
  * The tests performs the following major tasks:
  *		1. The Program Counter test invokes functions that are located in Flash memory at different addresses.
  *		2. These functions return a unique value( address of the respective functions).
  *   3. The returned value is verified using the "CPU_PCtesT" function.
  * Return Values:
  *     PC_TEST_FAIL :  return value = 0.
  *     PC_TEST_PASS :  return value = 1.
  *
  *******************************************************************/

int CPU_PCtest (void)
{
//FogBugz 128 long returnAddress, tempAddress;
unsigned long returnAddress, tempAddress; //FogBugz 128

    //store Address of TestFunction1
 //FogBugz 128  tempAddress = (long)TestFunction1;
  tempAddress = __builtin_tbladdress(TestFunction1); //FogBugz 128

    // Branch to TestFunction1
  returnAddress = TestFunction1();

    // Test if the address of "TestFunction1" returned is the same
  if( tempAddress != returnAddress)
  		 	return PC_TEST_FAIL;

   // store Address of TestFunction2
 //FogBugz 128   tempAddress = (long )TestFunction2;
  tempAddress = __builtin_tbladdress(TestFunction2); //FogBugz 128
   // Branch to TestFunction2
  returnAddress = TestFunction2();
   // Test if the address of "TestFunction2" returned is the same
		if( tempAddress != returnAddress)
  			 return PC_TEST_FAIL;
  return PC_TEST_PASS;
}


