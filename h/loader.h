/****************************************************************************
 *
 *       File:       loader.h
 *
 *       REV 1    1.5
 *
 *       Author:     Ken Langlais; edits by Dave Paquette
 *
 *       @Copyright 2009, 2014  Scully Signal Company
 *
 *       Description:
 *           These are the definitions of the memory locations of the loader
 *           memory module
 *
  * Revision History:
 *   Rev      Date        Who   Description of Change Made
 *  -------  ---------  ----    --------------------------------------------
 *  1.5.28  06/11/14  DHP  Added L_LOADED_COUNT_ADDR  
 *  1.5.31  01/14/15  DHP  Added LOAD_ERROR_ defines
****************************************************************************/

#define L_START_OF_INTERRUPT_VECTOR_ADDR  0x00000
#define L_START_OF_CODE_ADDR                             0x00C00
#define L_BYTE_COUNT_ADDR                                    0x40000
#define L_CHECKSUM_ADDR                                        0x40004
#define L_FEATURES_ADDR                                           0x40008
#define L_SERIAL_NUMBER_ADDR                               0x40020
#define L_SAVED_LOADER_TIME_ADDR                     0x41000
#define L_LOADED_COUNT_ADDR                               0x41004

#define L_PROGRAM_MEMORY_BLOCK_SIZE       0x400
#define L_SPI_ROM_BLOCK_SIZE              0x600

#define LOAD_ERROR_NONE  0x96
#define LOAD_ERROR_NOLOAD  0x33
#define LOAD_ERROR_1          0x01    // Error on read of SPI count
#define LOAD_ERROR_2          0x02    // Error on read of options
#define LOAD_ERROR_3          0x03    // option board is blank
#define LOAD_ERROR_4          0x04    // Error on checksum verification
#define LOAD_ERROR_5          0x05    // Error on read of SPI data
#define LOAD_ERROR_6          0x06    // Error on program memory erase verification
#define LOAD_ERROR_7          0x07    // SPI memoy ID incorrect (not 13 or 14)
#define LOAD_ERROR_8          0x08    // unable to read back what we just wrote(S/N)
#define LOAD_ERROR_9          0x09    // unable to read back what we just wrote(Time)
#define LOAD_ERROR_10        0x010  // S/N neither -1 nor match for this unit
#define LOAD_SUCCESS          0x069  // Successful load
