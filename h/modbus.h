/*
 *
 *   Project:        Rack Controller
 *
 *   Module:         MODBUS.H
 *
 *   Revision:       REV 1.0
 *
 *   Author:        Ken Langlais (KLL); edits by Dave Paquette (DHP)
 *
 *                   @Copyright 2008  Scully Signal Company
 *
 *   Description:    Main program Modbus routines for Rack
 *                   controller main microprocessor PIC24HJ256GP210
 *
 *   Revision History:
 *
 *****************************************************************************/
#ifndef MODBUS_H
#define MODBUS_H


/*   MODBUS MESSAGE COMMAND DEFINITIONS
 *
 *****************************************************************************/

#define READ_OUTPUT_STATUS          0x01
#define READ_INPUT_STATUS           0x02
#define READ_MULTIPLE_REGS          0x03
#define FORCE_SINGLE_BIT            0x05
#define WRITE_SINGLE_REG            0x06
#define WRITE_MULTIPLE_REGS         0x10
#define WRITE_SINGLE_VEHICLE        0x41
#define READ_SINGLE_VEHICLE         0x42
#define READ_VIP_LOG_ELEMENT        0x43
#define WRITE_PASSWORD              0x44
#define WRITE_COMPANY_ID            0x45
#define WRITE_MULTIPLE_VEHICLES     0x46
#define READ_MULTIPLE_VEHICLES      0x47
#define BACKUP_FUNCTIONS            0x48
#define READ_TRL_LOG_ELEMENT        0x49
#define CRC_MULTIPLE_VEHICLES       0x4A
#define WRITE_BYPASS_KEYS           0x4B
#define READ_BYPASS_KEYS            0x4C
#define WRITE_FEATURES_PASSWORD     0x4D
#define READ_EE_BLOCK               0x4E
#define WRITE_EE_BLOCK              0x4F
#define REPORT_COMPARTMENT_VOLUME   0x50
#define READ_TIM_SCULLY_AREA        0x51
#define WRITE_TIM_SCULLY_AREA       0x52
#define READ_BUILDER_INFO           0x53
#define WRITE_BUILDER_INFO          0x54
#define READ_THIRD_PARTY            0x55
#define WRITE_THIRD_PARTY           0x56
#define READ_BUILDER_AREA           0x57
#define WRITE_BUILDER_AREA          0x58
#define INSERT_VEHICLE              0x59
#define REMOVE_VEHICLE              0x5A
#define READ_NUM_PROBES             0x5B
#define USE_UPDATED_ADC_TABLE       0x5C


/*
 *  Standard ModBus Error/Exception/Completion codes
 *
 *****************************************************************************/

#define MB_OK                             0x00
#define MB_EXC_ILL_FUNC                   0x01
#define MB_EXC_ILL_ADDR                   0x02
#define MB_EXC_ILL_DATA                   0x03
#define MB_EXC_FAULT                      0x04
#define MB_EXC_ACK                        0x05
#define MB_EXC_BUSY                       0x06
#define MB_EXC_NAK                        0x07
#define MB_EXC_MEM_PAR_ERR                0x08
#define MB_EXC_TIM_CMD_ERR                0x09
#define MB_EXC_TIM_FAMILY_ERR             0x0A
#define MB_EXC_TIM_NOT_VALID_ERR          0x0B
#define MB_EXC_NUMBER_COMPARTMENTS_ERR    0x0C
#define MB_SPI_FAMILY_ERR                 0x0D       
#define MB_SPI_WRITE_ERR                  0x0E
#define MB_SPI_READ_ERR                   0x0F
#define MB_EXC_TIM_MEM_AREA_ERR           0x10
#define MB_WRITE_TO_SCRATCHPAD_ERR        0x11
#define MB_VERIFY_SCRATCHPAD_ERR          0x12
#define MB_COPY_SCRATCHPAD_ERR            0x13
#define MB_EXC_TIM_ENTRY_NOT_VALID_ERR    0x14
#define MB_READ_SERIAL_ERROR              0x15
#define MB_MALLOC_ERROR                   0x16
#define MB_I2C_ERROR                      0x17
#define MB_READ_CLOCK_ERROR               0x18
#define MB_READ_ONLY_VALUE                0x19

#define MB_NO_RESPONSE      0x80


/*
 *
 * "Header" (RTU address and function code) size, in bytes
 *
 *****************************************************************************/

#define MODBUS_SKIP_HEADER    2
#define MODBUS_SKIP_CRC       2
//#define MODBUS_MAX_DATA (MODBUS_MAX_LEN - MODBUS_SKIP_HEADER) - MODBUS_SKIP_CRC)
#define MODBUS_MAX_DATA 76


/*
 *
 * ModBus Control/Extensions flags/fields (second message byte sign bit set)
 *
 *****************************************************************************/

/* Control/Extensions flag bit (if set, second byte is extended control) */

#define MBC_XCONTROL            0x80

/* Command/Response flag bit: 0 ==> master command; 1 ==> slave response */

#define MBC_XRESPMSG            0x40

/* Suppress normal ModBus slave response */

#define MBC_XNORESPONSE         0x20

/* Segmentation -- more data (packets/messages) coming */

#define MBC_XMORE               0x10

/* This packet/message segment/sequence number (each sequence of segments
   [MBC_XMORE set] always starts with 0 and counts up modulo 8)  */

#define MBC_XSEQUENCE           0x07



#define EOM_192   911
#define EOM_096  1823
#define EOM_048  3646
#define EOM_024  7292
#define EOM_012 14584
#define EOM_006 29168
#define EOM_003 58336


//#define NO_MESSAGE    0
//#define RESET        -1
//#define PROCESS_MSG   1


/* Function return "code" (Exception codes as above) */

typedef unsigned char MODBSTS;



extern char modNVflag;          /* Non-Volatile data changes flag */


/*
      D A T A   S T R U C T U R E S
*/


void modbus_msg_process() ;
void modbus_msg_response();
MODBSTS modbus_decode (unsigned char  recv_len,
                       unsigned char *recv_cmd,
                       unsigned char *xmit_len,
                       unsigned char *xmit_rsp);
MODBSTS mbrRdReg (unsigned int, unsigned int *);
MODBSTS mbrWrReg (unsigned int, unsigned int *);
MODBSTS mbxForce (unsigned int, unsigned int);

#endif    /* end of MODBUS_H */
