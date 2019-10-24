/*********************************************************************************************
 *
 *  Project:   Rack Controller
 *
 *  Module:    comdat.c
 *
 *  Revision:  REV 1.6
 *
 *   Author:         Ken Langlais, edits by Dave Paquette
 *
 *                   @Copyright 2009, 2015  Scully Signal Company
 *
 *  Description:  All global (common) data definitions (reserve memory space)
 *
 * Revision History:
 *   Rev      Date         Who  Description of Change Made
 *  --------   ------------  ----   --------------------------------------------
 *  1.5.00  04/22/08  KLL  Original Version
 *  1.5.23  04/19/12  KLL  Added gnd_retry variable.
 *                                     Removed probe_hi that was used in optic/thermistor test
 *                                     Remove thermistor_retry. It was part of the thermistor
 *                                      test that switch between two wire optic and thermistor
 *                                      and is no longer needed.
 *                                     Removed probe_signal array that was used to debugging
 *  1.5.27  03/25/14 DHP  Added TIM related variables
 *             04/11/14 DHP  Added Ground_Reference
 * 1.5.30  08/10/14  DHP  Added unit_type
 *                                     Removed badvaporflag, vapor_time, ref_voltage, flow_voltage,
 *                                       and vapor_timeout
 * 1.6.31  01/05/15  DHP  Removed unused "touch chip" variables
 * 1.6.34  07/08/16  DHP  QCCC 53: Added probe_type[]
 *********************************************************************************************/

#include "common.h"

/*
    See defines in comdat.h
*/

const LED_NAME LedAddr[8] = {
  COMPARTMENT_1,
  COMPARTMENT_2,
  COMPARTMENT_3,
  COMPARTMENT_4,
  COMPARTMENT_5,
  COMPARTMENT_6,
  COMPARTMENT_7,
  COMPARTMENT_8};

unsigned int optic5_table[16];

/***************************** Variables ************************************/
unsigned char dry_once;               /* FogBugz 108 */
unsigned long cycle_timeout;         /* FogBugz 108 */
unsigned int high_3[8];                  /* QCCC 53 */
unsigned int high_8[8];                  /* QCCC 53 */
unsigned int unit_type;
unsigned int Ground_Reference;
unsigned int gnd_retry;
int start_idle;
int new_front_panel;
int disable_domeout_logging;

/******************************* 11/12/2009 1:56PM ***************************
 * variables used in communication with the backup processor
 *****************************************************************************/
unsigned char communicate_RX_pkt[COMM_PKT_SIZE];
unsigned char *communicate_RX_ptr;
int receive_bk_status, rx_size;
unsigned int receive_data;

unsigned int debug_tbl[5];  /* Save read ADC mux results */

unsigned int error_flag            __attribute__ ((address(0x3FF0))) __attribute__ ((noload));
unsigned int error_address     __attribute__ ((address(0x3FF8))) __attribute__ ((noload));
unsigned int error_good_data __attribute__ ((address(0x3FFA))) __attribute__ ((noload));
unsigned int error_bad_data   __attribute__ ((address(0x3FFC))) __attribute__ ((noload));
unsigned int updater_rev         __attribute__ ((address(0x3FFE))) __attribute__ ((noload));

unsigned long     tank_time;                  /* force a 1 second trial time */

/* state machine levels & structures - see enum.h */
unsigned long     overfill_count;             /* Keep track of who many tiles the fill error occur */
MAIN_STATE        main_state;                 /* state action tables */
unsigned int      SpecialOps_state;           /* state action for SpecialOps */
unsigned int      sub_state;                  /* available to each main "state" */
ACQUIRE_STATE     acquire_state;
unsigned int      val_state;                  /* truck_validate() state */
TANK_STATE        tank_state;
TRUCK_STATE       truck_state;
unsigned int      TIM_state;
TWO_WIRE_STATE    two_wire_state;
OPTIC5_STATE      optic5_state;
PROBE_TRY_STATE   probe_try_state;
unsigned int      groundiodestate;
PROBES_STATE      probes_state[16];
signed char       ledstate[(int)NLEDBIT];     /* one byte per led - serial */
unsigned int      number_of_Probes;           /* number of 5 wire probes found on the current truck */
unsigned int      number_of_Compartments;     /* number of compartments stored in the TIM */
unsigned int      truck_first_time;           /* Indicate first time through the fire wire loop */
unsigned int      compartment_time;
unsigned int      start_bit;

unsigned int      print_once_msg;             /* Status to print a message once during an active truck */

OPT_PULSE         opt_return;                 /* 5-wire optic return pulse structure */

unsigned int      act_therm_mask;             /* Mask of active thermistor probes */

unsigned char     start_point;                /* Start Channel 0 or 2 */

unsigned int      send_char;                  /* For debug */

unsigned char     family_code;                /* Store away the current truck TIM family code */

unsigned int      Led_map_low_word_0;
unsigned int      Led_map_low_word_1;
unsigned int      Led_map_low_word_2;
unsigned int      global_count;
unsigned int      interrupt_count;

unsigned int      begin_time, end_time;
/* Config "A" status word (ModBus Register) comprised of ConfigA [high] byte
   plus enable_jumpers [low] byte; Config "B" status word (ModBus Register)
   parallels "A", with SysParm.ConfigB & SysParm.EnaFeatures [low] byte. */

unsigned char     ConfigA;                   /* System hardware configuration
                                        (+enable_jumpers) */
unsigned char     ConfigC;                   /* System Software configuration */

unsigned char     enable_jumpers;            /* Hardware jumpers/enable byte, 1=TRUE */
                                    /* see comdat.h for defines */

char     enable_soft;               /* Software feature/enable byte, 1=TRUE */
                                    /* see comdat.h for defines */
char     active_comm;               /* Active communication 1=TRUE */
char     bystatus;                  /* Bypass status/flags (ModBus) */
unsigned char bylevel;              /* hierarchical bypassing levels */
char     baddeadman;                /* DEADMAN switch open or inactive */
char     Permit_OK;                 /* Tell backup to resume permit */
unsigned char   badgndflag;         /* bad ground diode */
unsigned int    deadman_voltage;    /* Save the deadman reading */
unsigned short  BadGndCntr;
unsigned short  GoodGndCntr;
char     badoverfillflag;           /* upper nibble = D has gone dry earlier */
unsigned int badvipflag;                /* See comdat.h */
short    BadVipCntr;
char     badvipdscode;              /* Associated DateStamp code (stdsym.h) */

/*  code space variables */
unsigned int   KernelCRCval;        /* Actual Kernel CRC-16 calculated value */
unsigned int   Good_KernelCRCval;   /* Good Kernel CRC-16 calculated value */
unsigned int   ShellCRCval;         /* Actual Shell CRC-16 calculated value */
unsigned int   Good_Shell_CRC_val;  /* Good Shell CRC-16 calculated value */

char           loopEighths;         /* .125-Second counter (delta time only) */
char           secReset;            /* RESET counter (reset on zero) */
unsigned       hitCount;            /* Impact Sensor hits counted by SIM.C */
unixtime       present_time;        /* present time */

/* Dallas type code buffers */
unsigned char  touchbuf[33];        /* touch chip read/write buffer */
unsigned char  truck_SN[BYTESERIAL];/* Serial Number(s) from truck */
unsigned char  bypass_SN[BYTESERIAL]; /* Last received bypass SN */
unsigned char  month;               /* Dallas converted unix time */
unsigned char  day;
unsigned int   year;
unsigned char  hour;
unsigned char  minute;
unsigned char  second;
unsigned char  clock_SN[BYTESERIAL];   /* last received clock SN */
unsigned char  Intellitrol_SN[BYTESERIAL];   /* last received SN */
char           clock_status;           /* 0 if OK */
unsigned char  no_relay_flag;          /* Special Test Mode flag Use to make everything think we are permiting but do not activate the relay */
unsigned char  scully_flag;            /* The truck has all Scully equipment */

int probe_result_flag;                 /* ADC interrupt flag */
int dma_result_flag;                   /* ADC interrupt flag */
unsigned short mstimer;                /* Like TCNT, but milliseconds */
unsigned long  freetimer;              /* timer 1 ms count */
unsigned short service_time;         /* service charge counter */
unsigned long  dry_timer;              /* timer of dry probe attachment */
unsigned long  drive_time;             /* a force timer to prevent charge runaway */
unsigned long  jump_time;              /* a timer to control JUMP_START */
unsigned long  probe_time;             /* may NOT repeat for 25-30ms */
unsigned long  bypass_time;            /* may only bypass for 1 hour */
unsigned long  ground_time;            /* Time of next ground check */
unsigned long  optic5_timeout;         /* Idle - try the optic5 every 2 seconds */
unsigned long  relay_turn_on;          /* a force timer to prevent chatter */
unsigned long  truck_timeout;          /* Used to determined if a truck left */
/******************************* 2/25/2009 1:20PM ****************************
 * Used to determind if the truck has stopped pulsing
 *****************************************************************************/
unsigned int   probe_pulse_old;        /* Keeps track  */
unsigned int   probe_pulse;            /* Keeps track  */
unsigned long  pulse_timeout;          /* keep track of how long since last probe pulse */
unsigned char truck_pulsing;           /* flag to indicate 2-wire pulses being seen */

unsigned int   ReferenceVolt;          /* Reference/diagnostic volt (1.000V) */
unsigned int   Raw13Volt;              /* Raw input (nominal 13V) voltage */
unsigned int   BiasVolt;               /* Probe bias (3.5/3.8) voltage */
unsigned int   Optic5Volt;             /* 5-Wire Optic Pulse voltage */
signed int     noise_volt[COMPART_MAX];  /* Noise voltage margin */
unsigned int   open_c_volt[2][COMPART_MAX]; /* first value 10vt / second 20vt */
unsigned int   probe_volt[COMPART_MAX];     /* latest read of ADC's in mv */
// >>> QCCC 53
PROBE_TYPE     probe_type[2*COMPART_MAX];    /* Sensor type */
// <<< QCCC 53
unsigned int   result_ptr[COMPART_MAX];     /* latest read of ADC's in mv */
unsigned int   old_probe_volt[COMPART_MAX]; /* last read of ADC's in mv */
int            adc_convert_flag;       /* Indicate that the analog conversion */
                                       /*  has finished */
char           berr;                   /* buss error occurred */
char           dry_pass_count;         /* scan passes for probe time */
char           wet_pass_count;         /* scan passes for probe time */
char           gon_pass_count;         /* scan passes for probe time */
char           delay_time;             /* settable delay relay anti-chatter on time */
unsigned int   probe_index;            /* index into the probe binary array */
unsigned int   probe_array[MAX_ARRAY]; /* ADC bit conversion array */
char           main_array[MAX_RELAY];  /* main relay oscillation array */
char           bak_array[MAX_RELAY];   /* backup relay oscillation array */
char           bak_charge[MAX_RELAY];  /* backup charge driver array */

/******************************* 12/30/2008 7:12AM ***************************
 * Area to store the contents of the TIM truck configuration.
 * Byte 0 is the number of truck compartments
 * Byte 1 is the compartment volume unit; 1 - Gallons  2 - Liters
 * Byte 02 - 05 <= Volume of compartment 1
 * Byte 06 - 09 <= Volume of compartment 2
 * Byte 10 - 13 <= Volume of compartment 3
 * .
 * .
 * .
 * Byte 62 - 65 <= Volume of compartment 16
 *****************************************************************************/
unsigned char  Truck_TIM_Configuration[68];

/* MODBUS buffers & flags */

unsigned char  modbus_addr;         /* ModBus address; 0 => ASCII/Debug */
unsigned char  modbus_baud;         /* ModBus/Comm baud rate */
unsigned char  modbus_parity;       /* ModBus/Comm parity */
unsigned char  modbus_csize;        /* ModBus/Comm character size */
unsigned char  modbus_err;
unsigned char  modbus_state;
unsigned char  modbusVIPmode;
unsigned short modbus_eom_time;
unsigned short modbus_Recv_err;
unsigned short modbus_PrepMsg_err;
unsigned short modbus_DecodeMsg_err;
unsigned char  modbus_rx_len;
unsigned char  *modbus_rx_ptr;
unsigned short modbus_rx_time;

unsigned char  modbus_tx_len;
unsigned char  *modbus_tx_ptr;
unsigned short modbus_tx_time;

unsigned char  modbus_rx_buff[MODBUS_MAX_LEN+1];
unsigned char  modbus_tx_buff[MODBUS_MAX_LEN+1];
unsigned char  save_last_recv[MODBUS_MAX_LEN+1];
unsigned char  save_last_recv_len;
unsigned char  save_last_xmit[MODBUS_MAX_LEN+1];
unsigned char  save_last_xmit_len;

/* Main Cybertrol error logger */
unsigned int   iambroke;               /* bit error logger */
unsigned int   iamsuffering;           /* bit error logger */

/* Main Intellitrol "Status" registers (e.g., as returned by ModBus)
   See STSBITS.H for definitions */

unsigned int    StatusA;                /* Status Reg "A" aka ISB 00 - 15 */
unsigned int    StatusB;                /* Status Reg "B" aka ISB 16 - 31 */
unsigned int    StatusO;                /* Status Reg "O" aka OSB 00 - 15 */
unsigned int    StatusP;                /* Status Reg "P" aka OSB 16 - 31 */

/* State bytes for backup and main processor relays */

char            BackRelaySt;            /* Backup Relay State */
char            MainRelaySt;            /* Main Relay State */

/*
   EEPROM (System/Non-Volatile) Variables
*/

unsigned char  EE_status;           /* EEPROM system status */
unsigned char  EE_valid;            /* EEPROM [in]valid blocks */

E2SYSBLK       SysNonV;             /* EEPROM's System NonVolatile area */
SysParmNV      SysParm;             /* Active copy of Non-Volatile Sys Parms */

DateStampNV    *pDateStamp;          /* Pointer to active DateStamp, if any */
SysDia5NV      *pSysDia5;            /* Pointer to active SysDia5 stuff */
SysVoltNV      *pSysVolt;            /* Pointer to active SysVolt stuff */
SysSet1NV      *pSysSet1;            /* Pointer to active SysSet1 stuff */

unsigned int   evMax;               /* Highest allowable entry */
unsigned int   evIndex;             /* Index to next free entry */
unsigned int   evLastRead;          /* Index last read via ModBus */


/*******************************5/20/2008 7:08AM******************************
 * eeprom stored blocks
 *****************************************************************************/
E2HOMEBLK   home_block;                 /* Home block stored here */
//E2BOOTBLK   boot_block;               /* boot block stored here */
//E2CRASHBLK  crash_block;              /* crash block stored here */
//E2LOGREC    log_entry;                /* current log entry stored here */
//E2KEYREC    bypass_entry;             /* current bypass entry stored here */
//E2TIMREC    tim_entry;                /* current entry entry stored here */

unsigned long probe_retry_count;        /* Used to determined if we are in a endless */
                                        /* loop trying to figure out what type of probe is connected */

/*****************************************************************************
 * TIM related data
 *****************************************************************************/
unsigned int    TIM_size;                 /* Dallas chip NVRAM / EEPROM size */
unsigned int    TIM_scratchpad_size;      /* Dallas chip scratchpad size */
unsigned int    S_TIM_code;               /* Super TIM supported TRUE/FALSE */
unsigned int    TIM_fault_logged = 0;
unsigned char   cert_ds_fails=0;
unsigned char   fuel_type_fails=0;
unsigned char   TIM_info_logged = 0;
unsigned int    log_data_state = 0;
unsigned int    log_time_address;
unsigned int    compare_volts;

/******************************* end of comdat.c ****************************/
