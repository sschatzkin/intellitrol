/****************************************************************************
 *
 *       File:       checksum.h
 *
 *       Author:     Ken Langlais
 *
 *       @Copyright 2012  Scully Signal Company
 *
 *       Description:
 *           CONSTANTS, CODES AND ADDRESSES FOR ALL HARDWARE REGISTERS
 *
 *       Revision History:
 *  Revision   Date        Who   Description of Change Made
 *  --------   --------    ---   --------------------------------------------
 *  1.5.26     10/02/12    KLL   Added the ability to put the checksum into memory
 *
****************************************************************************/
#include "common.h"
/******************************* 10/2/2012 9:47AM ****************************
 * Place this in the loader section
 *****************************************************************************/
void __attribute__((__section__(".LoaderSection"))) C_calculate_crc(void);


/******************************* 10/2/2012 7:52AM ****************************
 * C_calculated_crc This routine will check to see if the Intellitrol CRC is
 * in memory. If not it will place it into memory. This routine must reside
 * in the boot area because it will modify program area with general security enable
 *****************************************************************************/

void C_calculate_crc()
{
unsigned long     end_address;
unsigned short    icrc_val;
unsigned long     ptr;
//char              status;
uReg32 Temp, SourceAddr, read_data;

  end_address = (unsigned long)__builtin_tbladdress(&_PROGRAM_END);
  icrc_val = INIT_CRC_SEED;

  for (ptr = SHELL_START;
       ptr < (end_address - FLASH_CRC_CHUNK);
       ptr += FLASH_CRC_CHUNK)
  {
    icrc_val = program_memory_CRC (ptr, FLASH_CRC_CHUNK, icrc_val);
    {                           /* Yes */
      MSTAT_LED ^= 1;                  /* Toggel status led */
      ClrWdt();
    }
  }

  /* Finish the last "few" bytes, kinda sorta modulo FLASH_CRC_CHUNK */

  icrc_val = program_memory_CRC (ptr,
                    (unsigned short)((end_address - ptr) + 1),
                    icrc_val);

  MSTAT_LED ^= 1;                  /* Toggel status led */
  ClrWdt();
  (void)_memcpy_p2d24((char *)&read_data.Val32, CHECKSUM_LOW_ADDR, 3);
  read_data.Val32 = read_data.Val32 & 0x00FFFFFF;

  if ((read_data.Val32 & 0x00FFFFFF) == 0x00FFFFFF)
  {
    printf("\n\r    *** Updating CRC(0x%04X) into Program Memory ***\n\r", icrc_val);
    // UART2PutChar('$');
    asm volatile("nop");    /* place for the breakpoint to stop */
    asm volatile("nop");    /* place for the breakpoint to stop */
    asm volatile("nop");    /* place for the breakpoint to stop */
    SourceAddr.Val32 = LAST_BLOCK_ADDR;
    // UART2PutChar('1');
    Erase(SourceAddr.Word.HW, SourceAddr.Word.LW, PM_ROW_ERASE);
    // UART2PutChar('2');
    SourceAddr.Val32 = CHECKSUM_LOW_ADDR;
    Temp.Word.LW = (icrc_val & 0xFFFF);
    Temp.Word.HW = 0;
    MSTAT_LED ^= 1;                  /* Toggel status led */
    ClrWdt();
    // UART2PutChar('3');
    L_WriteLatch(SourceAddr.Word.HW, SourceAddr.Word.LW,Temp.Word.HW,Temp.Word.LW);
    MSTAT_LED ^= 1;                  /* Toggel status led */
    ClrWdt();
    // UART2PutChar('4');
    L_WriteMem(PM_ROW_WRITE);
    // UART2PutChar('5');
    MSTAT_LED ^= 1;                  /* Toggel status led */
    ClrWdt();
//    (void)_memcpy_p2d24((char *)&read_data.Val32, CHECKSUM_LOW_ADDR, 3);
  }
}
