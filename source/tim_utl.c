/**********************************************************************************************
 *
 *  Project:      Rack Controller
 *
 *  Module:       tim_utl.c
 *
 *  Revision:     REV 1.6
 *
 *  Author:       Ken Langlais (KLL) @Copyright 2009, 2016  Scully Signal Company
 *  Edits:         Dave Paquette (DHP))
 *
 *  Description:  Routines for interfacing to the Dallas chip on tnhe Super TIM
 *
 * Revision History:
 *   Rev      Date   Who  Description of Change Made
 * -------- -------- ---  --------------------------------------------
 * 1.0      06/12/09 KLL  Original Version
 * 1.5.27   03/25/14 DHP  Replaced multiple checks and local variables for
 *                          TIM memory and scratchpad sizes with global variables
 *                          set in Read_Dallas_SN()
 *                        Removed multiple resets and read SN in tim_block_read()
 *                        Added delay in copy_scratchpad() before chip reset
 *                        Removed lines of commented out code
 * 1.5.31  01/14/15  DHP  Commented debug print lines in tim_block_write()
 * 1.6.34  11/08/16  DHP  FogBugz 143: Added check for communication line
 *                          being available in tim_block_write().
 * 1.6.37   2/14/19  SME  Added functions:
 *                          TIM_log_fault
 *                          TIM_log_info
 *                          superTIM_ds_validate
 *                          check_unload_time
 *                          check_compartment_count
 *                        for support of super TIM
 *********************************************************************************************/
#include <ctype.h>
#include "common.h"
#include "tim_utl.h"
#include "version.h"

static unsigned char last_load_date_time[6];
static unsigned char load_history_ptr;
static unsigned int base_addr;
static unsigned int type_base_addr;
static unsigned int vol_base_addr;
static unsigned int write_counter;
static unsigned int iteration_counter;

//static unsigned char compartment_count_flagged;

/******************************* 5/20/2009 7:59AM ****************************
* write_to_scratchpad()
 * Write data into the scratchpad memory. The number of bytes must not cause an
 * overflow the scratchpad buffer.
 *****************************************************************************/
int write_to_scratchpad(unsigned int scratchpad_size, unsigned int count, unsigned int address, const unsigned char *buffer)
{
unsigned int i;
union
{
  unsigned char data[2];
  unsigned int word;
} uw;
unsigned char temp_byte;

  if ( count > scratchpad_size)  /* Can't be greater than the scratch pad size */
  {
    printf("\n\rCount greater than scratchpad_size");
    return FAILED;
  }

  if (((address & (scratchpad_size-1))  + count) > scratchpad_size)
  {
    printf("\n\rCount will cause the address to cross over the scratch pad boundary");
    return FAILED;
  }

  if (reset_iButton(COMM_ID) != 0)
  {
    printf(" - Reset");
    return FAILED;                        /* Command not sent properly */
  }
  if (Dallas_Byte (0xCC, COMM_ID) != 0xCC)  /* Issue Skip ROM command */
  {
    printf(" - Skip ROM command");
    return FAILED;                        /* Command not sent properly */
  }
  if (Dallas_Byte (0x0F, COMM_ID) != 0x0F)  /* Issue Write Scratchpad command */
  {
    printf(" - Write Scratchpad command");
    return FAILED;                        /* Command not sent properly */
  }
  uw.word = address;
  if (Dallas_Byte (uw.data[0], COMM_ID) != uw.data[0])  /* issue TA1 lower byte of the address */
  {
    printf(" - TA1");
    return FAILED;                        /* Command not sent properly */
  }
  if (Dallas_Byte (uw.data[1], COMM_ID) != uw.data[1])  /* Issue TA2 upper byte of the address */
  {
    printf(" - TA2");
    return FAILED;                        /* Command not sent properly */
  }
  /****************************** 12/23/2008 11:59AM *************************
   * The scratchpad is 'count' bytes so we must write 'count' bytes at a wack
   ***************************************************************************/
  for ( i=0; i<count; i++)
  {
    if ((temp_byte = Dallas_Byte (buffer[i], COMM_ID)) != buffer[i])  /* BYTE 0 - 1 compartment */
    {
      printf(" - BYTE %u", i);
      printf(" Wrote: 0x%02X Received: 0x%02X\n\r", buffer[i], temp_byte);
      return FAILED;                        /* Command not sent properly */
    }
  }

  if (reset_iButton(COMM_ID) != 0)
  {
    printf(" - Reset");
    return FAILED;                        /* Command not sent properly */
  }

  return GOOD;
}

/*******************************************************************************
 * verify_scratchpad()
 *******************************************************************************/
int verify_scratchpad(unsigned int scratchpad_size, unsigned int count, unsigned int address, const unsigned char *buffer, unsigned char *E_S)
{
unsigned int i;
union
{
  unsigned char data[2];
  unsigned int word;
} uw;
unsigned char fetch_buffer[32];
unsigned char temp_byte;

  uw.word = address;
  for ( i=0; i<sizeof(fetch_buffer); i++)
  {
    fetch_buffer[i] = 0;
  }

  if ( count > scratchpad_size)  /* Can't be greater than the scratch pad size */
  {
    printf("\n\rCount greater than scratchpad_size");
    return FAILED;
  }

  if (((address & (scratchpad_size-1))  + count) > scratchpad_size)
  {
    printf("\n\rCount will cause the address to cross over the scratch pad boundary");
    return FAILED;
  }

  if (reset_iButton(COMM_ID) != 0)
  {
    printf(" - Reset");
    return FAILED;                        /* Command not sent properly */
  }
  if (Dallas_Byte (0xCC, COMM_ID) != 0xCC)  /* Issue Skip ROM command */
  {
    printf(" - Skip ROM command");
    return FAILED;                        /* Command not sent properly */
  }
  if (Dallas_Byte (0xAA, COMM_ID) != 0xAA)  /* Issue Write Scratchpad command */
  {
    printf(" - Write Scratchpad command");
    return FAILED;                        /* Command not sent properly */
  }
  temp_byte = Dallas_Byte (0xFF, COMM_ID);  /* Read TA1 lower byte of the address */
  if (temp_byte != uw.data[0])  /* Read TA1 lower byte of the address */
  {
    printf(" - TA1\n\r   Good: 0x%02X    Bad: 0x%02X", uw.data[0], temp_byte);
    return FAILED;                        /* Command not sent properly */
  }
  temp_byte = Dallas_Byte (0xFF, COMM_ID);  /* Read TA2 upper byte of the address */
  if (temp_byte != uw.data[1])  /* Read TA1 lower byte of the address */
  {
    printf(" - TA2\n\r   Good: 0x%02X    Bad: 0x%02X", uw.data[1], temp_byte);
    return FAILED;                        /* Command not sent properly */
  }
  if (address != uw.word)
  {
    printf("\n\rFetch wrong address:\n\rRead: 0x%X Expected: 0x%X", uw.word, address);
  }

  *E_S = Dallas_Byte (0xFF, COMM_ID);  /* Read E/S, ending offset, flags=0 */

  for ( i=0; i<count; i++)
  {
    fetch_buffer[i] = Dallas_Byte (0xFF, COMM_ID);  /* Fetch contents of the scratchpad buffer */
  }

  if (reset_iButton(COMM_ID) != 0)
  {
    printf(" - Reset");
    return FAILED;                        /* Command not sent properly */
  }

  for ( i=0; i<count; i++)
  {
    if (fetch_buffer[i] != buffer[i])
    {
      printf("\n\rVerification error:\n\r");
      printf("Start Address: 0x%04X   Offset: 0x%X\n\r", address, i);
      printf("Good data: 0x%02X\n\r", buffer[i]);
      printf("Bad data:  0x%02X\n\r", fetch_buffer[i]);
      return FAILED;
    }
  }
  return GOOD;
}

/*******************************************************************************
 * copy_scratchpad()
 *******************************************************************************/
int copy_scratchpad(unsigned int address, unsigned char E_S)
{
union
{
  unsigned char data[2];
  unsigned int word;
} uw;

  if (reset_iButton(COMM_ID) != 0)
  {
    printf(" - Reset");
    return FAILED;                        /* Command not sent properly */
  }
  if (Dallas_Byte (0xCC, COMM_ID) != 0xCC)  /* Issue Skip ROM command */
  {
    printf(" - Skip ROM command");
    return FAILED;                        /* Command not sent properly */
  }
  if (Dallas_Byte (0x55, COMM_ID) != 0x55)  /* Issue Write Scratchpad command */
  {
    printf(" - Write Scratchpad command");
    return FAILED;                        /* Command not sent properly */
  }
  uw.word = address;
  if (Dallas_Byte (uw.data[0], COMM_ID) != uw.data[0])  /* issue TA1 AUTHORIZATION CODE */
  {
    printf(" - TA1");
    return FAILED;                        /* Command not sent properly */
  }
  if (Dallas_Byte (uw.data[1], COMM_ID) != uw.data[1])  /* Issue TA2 AUTHORIZATION CODE */
  {
    printf(" - TA2");
    return FAILED;                        /* Command not sent properly */
  }
  if (Dallas_Byte (E_S, COMM_ID) != E_S)  /* Issue E/S AUTHORIZATION CODE */
  {
    printf(" - E/S");
    return FAILED;                        /* Command not sent properly */
  }
  DelayMS(10);
  if (reset_iButton(COMM_ID) != 0)
  {
    printf(" - Reset");
    return FAILED;                        /* Command not sent properly */
  }
  return GOOD;
}

/*******************************************************************************
 * dallas_fill()
 *******************************************************************************/
int dallas_fill(unsigned int scratchpad_size, unsigned int count, unsigned int address, const unsigned char *buffer)
{
unsigned char E_S;

  if (write_to_scratchpad(scratchpad_size, count, address, buffer))
  {
    return MB_WRITE_TO_SCRATCHPAD_ERR;
  }

  if (verify_scratchpad(scratchpad_size, count, address, buffer, (unsigned char *)&E_S))
  {
    return MB_VERIFY_SCRATCHPAD_ERR;
  }

  if (copy_scratchpad(address, E_S))
  {
    return MB_COPY_SCRATCHPAD_ERR;
  }

  return MB_OK;
}

/*******************************************************************************
* tim_block_write()
* This will write a block of data into the TIM
********************************************************************************/
int tim_block_write(unsigned char *memory_ptr, unsigned int address, unsigned int count)
{
unsigned int transfer_count;
unsigned char first_page;
unsigned char *datum;
int sts;

// >>> FogBugz 143
  if (active_comm & (INTELLI | GROUNDIODE)) /* COMM_ID aka TXA/RXA in use? */
  {                                           /* Might be by Ground Test */
    return (MB_EXC_BUSY);                   /* Yes, try again later !!!! */
  }
  active_comm |= TIM;                  /* Mark COMM_ID in use by us! (2) */
// >>> FogBugz 143
  clear_gcheck();                   /* Drive GCHECK low */
  ODCD = 0x5;
  TRISD &= ~COMM_ID;      /* Set to output */
  COMM_ID_BIT = 1;
  if (reset_iButton(COMM_ID) != 0)
  {
    printf(" - Reset");
    active_comm &= ~TIM;                 /* FogBugz 143 COMM_ID line free now */
    return MB_EXC_TIM_CMD_ERR;                        /* Command not sent properly */
  }
  if (!Read_Dallas_SN(COMM_ID))         /* Fetch the serial number */
  {
    active_comm &= ~TIM;                 /* FogBugz 143 COMM_ID line free now */
    return MB_READ_SERIAL_ERROR;
  }

  printf("\n\r");
  if (TIM_size < (address + count))  /* Would we exceed the memory? */
  {
    printf("address + count (0x%04X) exceeds memory size(0x%04X)\n\r", address + count, TIM_size);
    active_comm &= ~TIM;                 /* FogBugz 143 COMM_ID line free now */
    return MB_EXC_TIM_CMD_ERR;
  }

  /*
   * Can only do block writes upto the size of the scratch pad memory
   */
  datum = memory_ptr;
  if ((first_page = (unsigned char)(address % TIM_scratchpad_size)) != 0)
  {
    transfer_count = TIM_scratchpad_size - first_page;
    if (transfer_count > count)
    {
      transfer_count = count;
    }
  }
  else
  {
    transfer_count = count;
    if (transfer_count>TIM_scratchpad_size)
    {
      transfer_count = TIM_scratchpad_size;
    }
  }
  count -= transfer_count;

  if ((sts = dallas_fill(TIM_scratchpad_size, transfer_count, address, datum)) != MB_OK)
  {
    active_comm &= ~TIM;                 /* FogBugz 143 COMM_ID line free now */
    return sts;
  }

  if ( count == 0)
  {
    active_comm &= ~TIM;                 /* FogBugz 143 COMM_ID line free now */
    return MB_OK;
  }
  /*lint -e{662} inhibit Possible creation of out-of-bounds pointer by operator message */
  datum += transfer_count;
  address += transfer_count;

  while (count)
  {
    if ( count > TIM_scratchpad_size)
    {
      transfer_count = TIM_scratchpad_size;
      count -= TIM_scratchpad_size;
    }
    else
    {
      transfer_count = count;
      count = 0;
    }
    if ((sts = dallas_fill(TIM_scratchpad_size, transfer_count, address, datum)) != MB_OK)
    {
      active_comm &= ~TIM;                 /* FogBugz 143 COMM_ID line free now */
      return sts;
    }
    if ( count != 0)
    {
      address += transfer_count;
      /*lint -e{662} inhibit Possible creation of out-of-bounds pointer by operator message */
      datum += transfer_count;
    }
  }
  active_comm &= ~TIM;                 /* FogBugz 143 COMM_ID line free now */
  return MB_OK;
}

/*******************************************************************************
 * tim_block_read()
 *******************************************************************************/
int tim_block_read(unsigned char *memory_ptr, unsigned int address, unsigned int count)
{
unsigned int i;
union
{
  unsigned char data[2];
  unsigned int word;
} uw;

  if (active_comm & (INTELLI | GROUNDIODE)) /* COMM_ID aka TXA/RXA in use? (4|8) */
     return MB_OK;                            /* Yes, try again later */
  clear_gcheck();                   /* Drive GCHECK low */
  ODCD = 0x5;
  TRISD &= ~COMM_ID;      /* Set to output */
  COMM_ID_BIT = 1;

  if ( TIM_size < (address+count))
  {
    printf("\n\rTIM address(0x%X) and count(0x%X) exceeds TIM memory size(0x%X)\n\r", address, count, TIM_size);
    return MB_EXC_MEM_PAR_ERR;
  }

    /****************************** 12/24/2008 7:07AM **************************
    * Setup to read the EEPROM memory
    ***************************************************************************/
  if (reset_iButton(COMM_ID) != 0)
  {
//    printf(" - Reset");
    return MB_EXC_TIM_CMD_ERR;                        /* Command not sent properly */
  }
  if (Dallas_Byte (0xCC, COMM_ID) != 0xCC)  /* Issue Skip ROM command */
  {
//    printf(" - Skip ROM command");
    return MB_EXC_TIM_CMD_ERR;                        /* Command not sent properly */
  }
  if (Dallas_Byte (0xF0, COMM_ID) != 0xF0)  /* Issue Read Memory command */
  {
//    printf(" - Write Scratchpad command");
    return MB_EXC_TIM_CMD_ERR;                        /* Command not sent properly */
  }
  uw.word = address;
  if (Dallas_Byte (uw.data[0], COMM_ID) != uw.data[0])  /* Issue TA1 = 0 for first page location 0 */
  {
//    printf(" - TA1");
    return MB_EXC_TIM_CMD_ERR;                        /* Command not sent properly */
  }
  if (Dallas_Byte (uw.data[1], COMM_ID) != uw.data[1])  /* Issue TA2 = 0 for first page location 0 */
  {
//    printf(" - TA2");
    return MB_EXC_TIM_CMD_ERR;                        /* Command not sent properly */
  }

  for ( i=0; i<count; i++)
  {
    *memory_ptr++ = Dallas_Byte (0xFF, COMM_ID);  /* Fetch  */
  }
  if (reset_iButton(COMM_ID) != 0)
  {
//    printf(" - Reset Failed");
    return MB_EXC_TIM_CMD_ERR;                        /* Command not sent properly */
  }

  return MB_OK;
}

/*******************************************************************************
 * fetch_serial_number()
 * This routine will fetch either the TIM serial number or alternative serial number
 * from the memory inside the SuperTIM.
 * Input: type - Target type is TIM or Alternative TIM
 *        TIM_number - buffer to store the read serial number
 ******************************************************************************/
unsigned char fetch_serial_number(unsigned char tim_type, unsigned char *tim_number)
{
MODBSTS sts;                          /* Status holding */
unsigned int valid_address, tim_address, i;

  switch ( tim_type)
  {
    case TEST_TIM :
      valid_address = 0x404;
      tim_address = 0x405;
      break;
    case ALT_TIM :
      valid_address = ALT_TIM_ID_VALID_ADDR;
      tim_address = ALT_TIM_ID_ADDR;
      break;
    default:
      return MB_EXC_TIM_CMD_ERR;
  }

  if ( tim_number == NULL)
    return MB_EXC_ILL_ADDR;

  for ( i=0; i<6; i++)
  {
    tim_number[i] = 0;
  }

  
  /*
   * Test if TIM serial number is valid
   */
  if ((sts = (unsigned char)tim_block_read(tim_number, valid_address, 1)) != MB_OK)
  {
    return (sts);
  }

  if (tim_number[0] != TIM_VALID)
  {
    return MB_EXC_TIM_ENTRY_NOT_VALID_ERR;
  }

  sts = (unsigned char)tim_block_read(tim_number, tim_address, 6);
  return (sts);                     /* Return successfully */
}

/*************************************************************************
 *  subroutine: TIM_log_fault
 *
 *  function:   Log a fault to the TIM memory fault log.
 *              1 - Read the index to the next entry.
 *              2 - Get the date and time
 *              3 - Write the entry
 *              4 - Increment and save the index
 *         
 *
 *  input:  fault_val - The value for the fault
 *  output: None
 *
 *************************************************************************/
void TIM_log_fault(unsigned int fault_val)
{
unsigned char log_ptr,log_year;
unsigned char log_buf[8];
unsigned int addr;
int sts;                /* Status holding */

    UNIX_to_Greg();
    if ((sts = tim_block_read(&log_ptr, FAULT_LOG_PRT_ADDR, 1)) != MB_OK)
    {
        log_ptr = 1;
    }
    if( (log_ptr < 1) || (log_ptr > 5) )
    {
        log_ptr = 1;
    }
    
    addr = ((log_ptr - 1) * 6) + 0x10C;
    
    log_year = (unsigned char)(year - 2000);
    log_buf[0] = (unsigned char)month;
    log_buf[1] = (unsigned char)day;
    log_buf[2] = (unsigned char)log_year;
    log_buf[3] = (unsigned char)hour;
    log_buf[4] = (unsigned char)minute;
    log_buf[5] = (unsigned char)fault_val;
    tim_block_write(log_buf,addr,6);
    log_ptr++;
    if( log_ptr > 5 )
    {
        log_ptr = 1;
    }
    tim_block_write(&log_ptr,FAULT_LOG_PRT_ADDR,1);
    
    
}

void log_date_and_time(unsigned int log_address)
{
unsigned char write_buf[12],log_year;

    UNIX_to_Greg();
    log_year = (unsigned char)(year - 2000);
    write_buf[0] = (unsigned char)month;
    write_buf[1] = (unsigned char)day;
    write_buf[2] = (unsigned char)log_year;
    write_buf[3] = (unsigned char)hour;
    write_buf[4] = (unsigned char)minute;
    tim_block_write(write_buf,log_address,5);

}

/*************************************************************************
 *  subroutine: TIM_log_info
 *
 *  function:   Log info to the TIM on connection.
 *              For an unload terminal log Intellitrol SN
 *              and date and time
 *              For a load terminal log the Intellitrol SN,
 *              firmware version and load date and time. Then update
 *              load history log
 *         
 *
 *  input:  None
 *  output: None
 *
 *************************************************************************/
unsigned char TIM_log_info(void)
{
unsigned int ver,i;
unsigned int type_read_addr,vol_read_addr;
unsigned char write_buf[12],ret_stat;


//    compartment_count_flagged = 0;
    
    if(SysParm.EnaSftFeatures & ENA_UNLOAD_TERM)    // Unload terminal mode
    {
        ret_stat = 1;
        switch( log_data_state )
        {
            case 0:
                tim_block_read(Truck_TIM_Configuration, NUMBER_OF_COMPARTMENTS_ADDR, 1);
                number_of_Compartments = (unsigned int)Truck_TIM_Configuration[0];  /* number of compartments stored in the TIM */
                service_charge();             /* Keep Service LED off */
                write_buf[0] = clock_SN[5];
                write_buf[1] = clock_SN[4];
                write_buf[2] = clock_SN[3];
                write_buf[3] = clock_SN[2];
                write_buf[4] = clock_SN[1];
                write_buf[5] = clock_SN[0];
                // log the Intellitrol SN
                tim_block_write(write_buf,UNLOAD_ITROL_SN_ADDR,6);
                service_charge();             /* Keep Service LED off */
        
                log_time_address = LAST_UNLOAD_DATE_TIME_ADDR;
                // Log unload date and time
                log_date_and_time(log_time_address);
                service_charge();             /* Keep Service LED off */
                write_counter = 0;
                log_data_state = 1;
                break;
                
            default:
                type_read_addr = (write_counter * 8) + 0x600;
                write_buf[0] = write_buf[1] = write_buf[2] = 0;                 
                tim_block_write(write_buf,type_read_addr,3);
                write_counter++;
                if( write_counter >= number_of_Compartments )
                {
                    ret_stat = 0;
                }
                break;
        }
    
    }
    else
    {

        ret_stat = 1;
        switch( log_data_state )
        {
            case 0:
                // read the number of compartments while were here
                tim_block_read(Truck_TIM_Configuration, NUMBER_OF_COMPARTMENTS_ADDR, 1);
                number_of_Compartments = (unsigned int)Truck_TIM_Configuration[0];  /* number of compartments stored in the TIM */
        
                // Log the firmware version
                ver = (unsigned int)SHELLVER;
                write_buf[1] = (unsigned char)ver;
                write_buf[0] = (unsigned char)(ver >> 8);    
                tim_block_write(write_buf,ITROL_FW_VERSION_ADDR,2);
                service_charge();             /* Keep Service LED off */
   
                // Log the Intellitrol SN
                UNIX_to_Greg();
                write_buf[0] = clock_SN[5];
                write_buf[1] = clock_SN[4];
                write_buf[2] = clock_SN[3];
                write_buf[3] = clock_SN[2];
                write_buf[4] = clock_SN[1];
                write_buf[5] = clock_SN[0];    
                tim_block_write(write_buf,LOAD_ITROL_SN_ADDR,6);
                service_charge();             /* Keep Service LED off */
                log_data_state = 1;
                break;
 
            case 1:
                // Update the load history by moving the last load info into the next log slot
                tim_block_read(last_load_date_time,LAST_LOAD_DATE_TIME_ADDR,5);
                tim_block_read(&load_history_ptr,LOAD_HISTORY_PRT_ADDR,1);
                load_history_ptr++;
                if( load_history_ptr > 3 )
                {
                    load_history_ptr = 0;
                }
                tim_block_write(&load_history_ptr,LOAD_HISTORY_PRT_ADDR,1);
        
                log_time_address = LAST_LOAD_DATE_TIME_ADDR;
                // log the last load date and time
                log_date_and_time(log_time_address);
                service_charge();             /* Keep Service LED off */
                
                
                base_addr = (load_history_ptr * 10) + 0x6C4;
                type_base_addr = (load_history_ptr * 10) + 0x6C9;
                vol_base_addr = (load_history_ptr * 10) + 0x6CC;
                write_counter = 0;
                iteration_counter = 0;
                log_data_state = 2;
                break;
            
            default:
                if( (iteration_counter % 2) == 0 )
                {
                    service_charge();             
                    tim_block_write(last_load_date_time,base_addr + (write_counter * 40),5);
        
                    type_read_addr = (write_counter * 8) + 0x600;
                    tim_block_read(write_buf,type_read_addr,3);
                    tim_block_write(write_buf,type_base_addr + (write_counter * 40),3);
                    service_charge(); 
                    if( SysParm.EnaSftFeatures2 & ENA_AUTO_FUEL_TYPE_WRITE )
                    {
                        for( i = 0; i < 3; i++ )
                        {
                            write_buf[i] = SysParm.default_fuel_type[i];
                        }
                    }
                    else
                    {
                        write_buf[0] = write_buf[1] = write_buf[2] = 0;
                    }
                    tim_block_write(write_buf,type_read_addr,3);
                    iteration_counter++;
                }
                else
                {
                    service_charge();             
                    vol_read_addr = (write_counter * 8) + 0x606;
                    tim_block_read(write_buf,vol_read_addr,2);
                    tim_block_write(write_buf,vol_base_addr + (write_counter * 40),2);
                    service_charge();             
                    write_buf[0] = write_buf[1] = 0;
                    tim_block_write(write_buf,vol_read_addr,2);       
                    iteration_counter++;
                    write_counter++;
                    if( write_counter >= number_of_Compartments )
                    {
                        ret_stat = 0;
                    }
                }
            break;
        }
    }

return(ret_stat);
}

/*************************************************************************
 *  subroutine: superTIM_ds_validate
 *
 *  function:   Go through the cert date mask and for each indicated cert,
 *              read the expiration date and compare to the current date. 
 *              If expired set the appropriate bit in the cert_ds_fails flag
 *              and return DSEXPIRED status;
 *              
 *         
 *
 *  input:  None
 *  output: 0 - OK, non zero - Date expired
 *
 *************************************************************************/
char superTIM_ds_validate(void)
{
char ret_val,mask,fault_mask;
unsigned int cert_year,cert_month,cert_day,cert_hour,cert_minute,read_addr;
unsigned char cert_buf[7];
unixtime cert_time;
int i;

    ret_val = 0;
    mask = 1;
    fault_mask = 0;
    cert_ds_fails = 0;
    for( i = 0; i < 5; i++)
    {
        if( SysParm.Cert_Expiration_Mask & mask )
        {
            read_addr = (i * 34) + 0x301;
            tim_block_read(cert_buf,read_addr,3);
            cert_month = (unsigned int)cert_buf[0];
            cert_day = (unsigned int)cert_buf[1];
            cert_year = (unsigned int)cert_buf[2];
            cert_year += 2000;
            cert_hour = 23;
            cert_minute = 59;
            cert_time = Greg_to_UNIX (cert_year, cert_month, cert_day, cert_hour, cert_minute);
            if( cert_time < present_time )
            {
                ret_val = DSEXPIRED;
                cert_ds_fails = cert_ds_fails | mask;
            }
        }
        mask = mask << 1;
    }

    return (ret_val);
    
    
}

/*************************************************************************
 *  subroutine: check_unload_time
 *
 *  function:   Check the load date and time stamp with the 
 *              current date and time. If more the the unload max time
 *              return expired status, if no return ok
 *              
 *         
 *
 *  input:  None
 *  output: 0 - OK, non zero - Date expired
 *
 *************************************************************************/
char check_unload_time(void)
{
char ret_val;
unixtime load_time,max_time;
unsigned int load_year,load_month,load_day,load_hour,load_minute;
unsigned char load_buf[7];


    ret_val = 0;
    
    tim_block_read(load_buf,LAST_LOAD_DATE_TIME_ADDR,5);
    load_month = (unsigned int)load_buf[0];
    load_day = (unsigned int)load_buf[1];
    load_year = (unsigned int)load_buf[2];
    load_hour = (unsigned int)load_buf[3];
    load_minute = (unsigned int)load_buf[4];
    load_year += 2000;
    
    load_time = Greg_to_UNIX (load_year, load_month, load_day, load_hour, load_minute);
    max_time  = (unixtime)(SysParm.Unload_Max_Time_min * 60);
    if((present_time - load_time) > max_time)
    {
        ret_val = 1;
    }
    
    return(ret_val);
}

char check_fuel_type(void)
{
char ret_val,mask;
unsigned char allowable_buf[4];
unsigned char loaded_buf[4];
unsigned int allowable_addr,load_addr;
unsigned char num_compartments;
int i,j;


    ret_val = 0;
    
    tim_block_read(&num_compartments, NUMBER_OF_COMPARTMENTS_ADDR, 1);
    mask = 1;
    fuel_type_fails = 0;
    for( i = 0; i < num_compartments; i++)
    {
        if( SysParm.fuel_type_check_mask & mask )
        {
            allowable_addr = (i * 3) + CPT1_TYPES_ALLOWED_ADDR;
            load_addr = (i * 8) + CPT1_TYPE_LOADED_ADDR;
            tim_block_read(allowable_buf,allowable_addr,3);
            tim_block_read(loaded_buf,load_addr,3);
            for( j = 0; j < 3; j++ )
            {
                if( toupper(allowable_buf[j]) != toupper(loaded_buf[j]) )
                {
                    ret_val = 1;
                    fuel_type_fails = fuel_type_fails | mask;
                }
            }
        }
        mask = mask << 1;
    }

    return(ret_val);
}

/*************************************************************************
 *  subroutine: check_compartment_count
 *
 *  function:   Compares the number of compartments detected with the
 *              number of compartments programmed into the TIM.
 *              If they mismatch set the fault flags
 *              
 *         
 *
 *  input:  None
 *  output: None
 *
 *************************************************************************/
char check_compartment_count(void)
{
char ret_val;
unsigned char ichek_present,cpt_count;

    ret_val = 0;
    ichek_present = 0;
    tim_block_read(&ichek_present,INTELLICHECK_TYPE_ADDR,1);
    if( ichek_present == 0 )
    {
        tim_block_read(&cpt_count, NUMBER_OF_COMPARTMENTS_ADDR, 1);
        number_of_Compartments = (unsigned int)cpt_count;  /* number of compartments stored in the TIM */
        if( (number_of_Compartments != number_of_Probes) /*&& (compartment_count_flagged == 0)*/ )
        {
            ret_val = 1;
        }
    }
    return(ret_val);
}