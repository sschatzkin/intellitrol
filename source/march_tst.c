/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         march_tst.c
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009  Scully Signal Company
 *
 *   Description:    The test memory using the march alogrithm using binary tree as data
 *
 *   Channels are numbered from 0 to 7 rather than 1 to 8 as they are
 *   referenced in the drawings.
 *
 *   Revision History:
 *  Revision   Date        Who   Description of Change Made
 *  --------   --------    ---   --------------------------------------------
 *  1.5.23     04/18/12    KLL   Changed the test memory area due to more memory is
 *                                 used by the C compiler.
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         march_tst.c
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2009  Scully Signal Company
 *
 *   Description:    This file contains a march memory test
 *   Verify a section of unused RAM inside the MicroChip
 *
 *   Channels are numbered from 0 to 7 rather than 1 to 8 as they are
 *   referenced in the drawings.
 *
 *   Revision History:
 *
 *****************************************************************************/
#include "common.h"
static const unsigned int test_table[] = {0xFF00, 0x0F0F, 0x3333, 0x5555, 0x0000};

char march_test(unsigned int *m_start_ptr, unsigned int *m_end_ptr)
{
register unsigned int *start_ptr = m_start_ptr;
register unsigned int *end_ptr = m_end_ptr;
register unsigned int *test_ptr;                             /* address of location under test */
register unsigned int i, test_data;
  LATD ^= 0x2000;           /* "Pump" Service LED (turn it off) */
  /****************************** 2/3/2010 10:00AM ***************************
   * Init error variables
   ***************************************************************************/
  error_good_data = 0;
  error_bad_data = 0;
  error_address = 0;

  /****************************** 2/3/2010 9:44AM ****************************
   * Write back ground pattern (sizeof(test_table)/sizeof(test_table[0])) = 5
   ***************************************************************************/
  for ( i= 0; i < 5; i++)
  {
    test_data = test_table[i];
    for ( test_ptr = start_ptr; test_ptr < end_ptr; test_ptr++ )
    {
      *test_ptr = test_data;
    }
    LATD ^= 0x2000;           /* "Pump" Service LED (turn it off) */
    for ( test_ptr = start_ptr; test_ptr < end_ptr; test_ptr++ )
    {
      if (*test_ptr != test_data)
      {
        error_good_data = test_data;
        error_bad_data = *test_ptr;
        error_address = (unsigned int)test_ptr;
        return FAILED;
      }
      *test_ptr = ~test_data;  /* Write complement data */
    }
    /***************************** 2/3/2010 10:09AM **************************
     * Now from the end of memory test for complement data
     *************************************************************************/
    LATD ^= 0x2000;           /* "Pump" Service LED (turn it off) */
    for ( test_ptr = (end_ptr-2); test_ptr >= start_ptr; test_ptr-- )
    {
      if (*test_ptr != ~test_data)
      {
        error_good_data = ~test_data;
        error_bad_data = *test_ptr;
        error_address = (unsigned int)test_ptr;
        return FAILED;
      }
      *test_ptr = test_data;  /* Put back original data */
    }
  }
  return PASSED;
}

#ifdef __DEBUG
char full_memory_tst(void)
#else
static char full_memory_tst(void)
#endif
{
char status;
unsigned char *source_ptr;
unsigned char *destination_ptr;
unsigned int i;

  /****************************** 2/4/2010 7:41AM ****************************
   * Test empty area
   ***************************************************************************/
  printf("    Memory Test: Bank 0 - ");
  if ((status = march_test((unsigned int *)0x2000, (unsigned int *)0x3F00)) != PASSED)
  {
    printf("failed: \n\r");
    printf("Address: 0x%X\n\r", error_address);
    printf("Good Data: 0x%04X\n\r", error_good_data);
    printf("Bad Data: 0x%04X\n\r", error_bad_data);
    return status;
  }


  /****************************** 2/4/2010 7:43AM ****************************
   * copy variable area to tested area
   ***************************************************************************/
  source_ptr = (unsigned char*)0x800;
  destination_ptr = (unsigned char*)0x2000;
  for ( i=0; i<(0x2000-0x800); i++)
  {
    *destination_ptr++ = *source_ptr++;
  }

  /****************************** 2/4/2010 7:41AM ****************************
   * Test variable area
   ***************************************************************************/
  printf("Bank 1 ");
  if ((status = march_test((unsigned int *)0x0800, (unsigned int *)0x1000)) != PASSED)
  {
    printf("failed: \n\r");
    printf("Address: 0x%X\n\r", error_address);
    printf("Good Data: 0x%04X\n\r", error_good_data);
    printf("Bad Data: 0x%04X\n\r", error_bad_data);
    return status;
  }


  /****************************** 2/4/2010 7:43AM ****************************
   * put variable area back
   ***************************************************************************/
  source_ptr = (unsigned char*)0x2000;
  destination_ptr = (unsigned char*)0x800;
  for ( i=0; i<(0x2000-0x800); i++)
  {
    *destination_ptr++ = *source_ptr++;
  }

  return status;
}
/**********************************************************************************************
 *  subroutine:  mem_test
 *  function: Invoke the below test routines and report results.
 *       CPU_RegisterTest()
 *       CPU_PCtest()
 *       full_memory_tst()
 *  input:  none
 *  output: PASSED / FAILED (reflects combined results of all tests) 
 *
*********************************************************************************************/
char mem_test(void)
{
#ifdef __DEBUG            /* These tests interfere with REAL ICE so don't run */
  return PASSED;          /* them when debugging */
#else

int save_ipl2_0;
char status, cpu_status, pc_status;

  /* Disable interrupts */
  save_ipl2_0 = SRbits.IPL;
  SRbits.IPL = 7;

  printf("\n\r    CPU Register Test ");
  if (CPU_RegisterTest())
  {
    cpu_status = PASSED;
    printf("is OK");
  } else
  {
    cpu_status = FAILED;
    printf("Failed");
  }

  service_charge();             /* Keep Service LED off */
  printf("\n\r    CPU Program Counter Test ");
  if (CPU_PCtest())
  {
    pc_status = PASSED;
    printf("is OK\n\r");
  } else
  {
    pc_status = FAILED;
    printf("Failed\n\r");
  }

  status = full_memory_tst();
  if (status != 0)
  {
    printf("- Failed");
  } else
  {
    printf("is OK");
  }
  /* restore interrupts */
  SRbits.IPL = (unsigned)save_ipl2_0;
  return status | cpu_status | pc_status;
#endif
}




