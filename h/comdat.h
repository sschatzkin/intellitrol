/****************************************************************************
 *
 *  Project:    Rack Controller
 *
 *  Module:    COMDAT.H
 *
 *  Revision:   REV 1.5
 *
 *  Author:    Ken Langlais
 *       @Copyright 2008, 2014  Scully Signal Company
 *
 *  Description:  All global (common) data definitions
 *
 * Revision History:
 *   Rev      Date       Who  Description of Change Made
 * --------    --------      ---     --------------------------------------------
 * 1.5.00  04/22/08  KLL   Original version
 * 1.5.27  03/25/14  DHP  Added TIM related variables
 *            04/11/14  DHP  Added Ground_Reference 
 * 1.5.30  08/10/14  DHP  Added ENA_INTELLITROL2 for Software features
 *                                     Added GND_FAIL_R, GND_FAIL_D for badgndflag
 *                                     Added unit_type, GND_FAIL_R, GND_FAIL_D
 *                                     Removed VAPOR_BPASS, VAPOR_PROBE_ERROR
 *                                     Removed unused "touch chip" variables
 * 1.6.34  07/08/16  DHP    Added probe_type[]
 ****************************************************************************/
#ifndef COMDAT_H
#define COMDAT_H

#include "esquared.h"

extern unsigned long    overfill_count;   /* Keep track of who many tiles the fill error occur */
extern unsigned int     optic5_table[16];

/***************************** Variables ************************************/
extern unsigned char    dry_once;           /* FogBugz 108 */
extern unsigned long    cycle_timeout;      /* FogBugz 108 */
extern unsigned int     high_3[];           /* QCCC 53 */
extern unsigned int     high_8[];           /* QCCC 53 */
extern unsigned int     unit_type;
extern unsigned int     Ground_Reference;
extern unsigned int     gnd_retry;
extern int              start_idle;
extern int              new_front_panel;
extern int              disable_domeout_logging;

extern unsigned long    tank_time;          /* force a 1 second trial time */

/******************************* 11/12/2009 1:56PM ***************************
 * variables used in communication with the backup processor
 *****************************************************************************/
extern unsigned char    communicate_RX_pkt[COMM_PKT_SIZE];
extern unsigned char    *communicate_RX_ptr;
extern int              receive_bk_status, rx_size;
extern unsigned int     receive_data;

extern unsigned int     debug_tbl[5];       /* Save read ADC mux results */

extern unsigned int     error_flag;
extern unsigned int     updater_rev;
extern unsigned int     error_good_data;
extern unsigned int     error_bad_data;
extern unsigned int     error_address;

extern const LED_NAME   LedAddr[8];         /* if we run short of RAMspace */

typedef struct
{
  unsigned char lenght;
  unsigned char cmd;
  unsigned char Bypass_Hot_Wired;
  unsigned char Relay_Error;
  unsigned char ROM_Error;
  unsigned char Jumper_Error;
  unsigned char Boot_Memory_Error;
  unsigned char CPU_Error;
  unsigned char PC_Error;
  unsigned char Memory_Error;
} BK_ERROR;

/* state machine levels & structures - see enum.h */
extern MAIN_STATE main_state;               /* state action tables */
extern unsigned int SpecialOps_state;       /* state action for SpecialOps */
extern unsigned int sub_state;              /* available to each main "state" */
extern ACQUIRE_STATE acquire_state;
extern unsigned int val_state;              /* truck_validate() state */
extern TANK_STATE tank_state;
extern TRUCK_STATE truck_state;
extern unsigned int TIM_state;
extern TWO_WIRE_STATE two_wire_state;
extern OPTIC5_STATE optic5_state;
extern PROBE_TRY_STATE probe_try_state;
extern unsigned int groundiodestate;        /* was ground diode in while idle? */
extern unsigned int Led_map_low_word_0;
extern unsigned int Led_map_low_word_1;
extern unsigned int Led_map_low_word_2;
extern unsigned int global_count;
extern unsigned int start_bit;
extern unsigned int interrupt_count;

extern unsigned int print_once_msg;         /* Status to print a message once during an active truck */
#define UN_AUTH                   0x0001    /* Print UN autherize message once flag */
#define BK_ERR_MSG                0x0002    /* Print Backup Processor not responding message once flag */
#define BK_NO_COMM_MSG            0x0004    /* Print not going to Backup Processor not responding message once flag */
#define BK_BYPASS_HOT_WIRED_MSG   0x0008    /* Print Backup Processor reporting Bypass Hot Wired message once flag */
#define BK_RELAY_ERROR_MSG        0x0010    /* Print Backup Processor reporting Relay_Error  message once flag */
#define BK_ROM_ERROR_MSG          0x0020    /* Print Backup Processor reporting ROM_Error message once flag */
#define BK_JUMPER_ERROR_MSG       0x0040    /* Print Backup Processor reporting Jumper_Error message once flag */
#define QUEDED_RESET_MSG          0x0080    /* Error detected that will cause a reset */
#define BK_BOOT_MEMORY_ERROR_MSG  0x0100    /* Print Backup Processor reporting Memory Error message once flag */
#define BK_PC_ERROR_MSG           0x0200    /* Print Backup Processor reporting Program Counter Error message once flag */
#define BK_CPU_ERROR_MSG          0x0400    /* Print Backup Processor reporting CPU Register Error message once flag */
#define BK_MEMORY_ERROR_MSG       0x0800    /* Print Backup Processor reporting Memory Error message once flag */
extern unsigned int begin_time, end_time;
extern unsigned int send_char;

extern unsigned char family_code;           /* Store away the current truck TIM family code */

#define  PRESENT_IDLE   1
#define  PRESENT_ACTIVE 2
extern PROBES_STATE     probes_state[16];
extern signed char      ledstate[NLEDBIT];          /* one byte per led    */
extern unsigned int     number_of_Probes;           /* number of 5 wire probes found on the current truck */
extern unsigned int     number_of_Compartments;     /* number of compartments stored in the TIM */
extern unsigned int     truck_first_time;           /* Indicate first time through the fire wire loop */
extern unsigned int     compartment_time;
extern   OPT_PULSE      opt_return;                 /* optic return pulse structure */

extern  unsigned int    act_therm_mask;             /* Active thermistor mask */

extern unsigned  char   start_point;                /* Start Channel 0 or 2 */


extern unsigned char    ConfigA;                    /* System configuration
                                                        (+enable_jumpers) */
extern unsigned char    ConfigC;                    /* System configuration */
extern unsigned char    enable_jumpers;             /* Hardware jumpers/enable byte;
                                                        1=TRUE (hardware invert) */

/******************************* 8/26/2008 8:54AM ****************************
 * io expander definitions
 *****************************************************************************/
#define ENA_TRUCK_HERE_NEW  0x40
#define ENA_DEADMAN_NEW     0x20
#define ENA_VAPOR_FLOW_NEW  0x10
#define ENA_VIP_NEW         0x08
#define ENA_GROUND_NEW      0x04
#define ENA_ADD_1_KEY_NEW   0x02
#define ENA_ERASE_KEYS_NEW  0x01

/******************************* 9/9/2008 2:58PM *****************************
 * Original jumper bit position
 *****************************************************************************/
#define     ENA_TRUCK_HERE  0x02
#define     ENA_VIP         0x04
#define     ENA_GROUND      0x08
#define     ENA_ADD_1_KEY   0x10
#define     ENA_ERASE_KEYS  0x20
#define     ENA_DEADMAN     0x40
#define     ENA_VAPOR_FLOW  0x80
#define     ENA_BYDEFAULT (ENA_TRUCK_HERE | ENA_ADD_1_KEY | ENA_ERASE_KEYS | ENA_DEADMAN)

extern   char     enable_soft;          /* Software features/enable byte;
                                             1=TRUE (hardware invert) */
#define     ENA_SHORT_CHK       0x02
#define     ENA_5_WIRE          0x04
#define     ENA_2_WIRE          0x08
#define     ENA_GRND_DELAY      0x10    /* Added June 18, 1998 M.R. */
#define     ENA_INTELLITROL2    0x20
#define     ENA_UNLOAD_TERM     0x40
#define     ENA_CPT_COUNT       0x80

#define     ENA_AUTO_FUEL_TYPE_WRITE    0x01

extern char     Ena_INTL_ShortNV;       /* Temp. Enable Shorts Test Flag READ ONLY */
extern char     Ena_Debug_Func_1;       /* Temp. Enable General Debug Pulse READ ONLY */
extern char     Ena_Debug_Func_2;       /* Temp. Enable General Debug Pulse READ ONLY */
extern char     Ena_Debug_Func_3;       /* Temp. Enable General Debug Pulse READ ONLY */
extern char     Ena_Debug_Func_4;       /* Temp. Enable General Debug Pulse READ ONLY */
extern char     Ena_Debug_Fail_1;       /* Temp. Enable Debug Pulse on Failure READ ONLY */
extern char     Ena_Debug_Fail_2;       /* Temp. Enable Debug Pulse on Failure READ ONLY */
extern char     Ena_Debug_Fail_3;       /* Temp. Enable Debug Pulse on Failure READ ONLY */
extern char     Ena_Debug_Fail_4;       /* Temp. Enable Debug Pulse on Failure READ ONLY */

extern char     Ena_fault_delay;        /* Set per ENA_GRND_DELAY */

extern   char     active_comm;          /* Active communication 1=TRUE */
#define     MODBUS         0x01
#define     TIM            0x02
#define     INTELLI        0x04
#define     GROUNDIODE     0x08
#define     CLOCK          0x10
#define     IRBYPASS       0x20
#define     ENA_INTL_SHORT 0x01         /* Located in EEPROM, Enables Shorts Tests */
#define     Ena_Debug_Func 0x00         /* Disable General Debug Pulse */
#define     Ena_Debug_Fail 0x00         /* Disable Debug Pulse on Failure */


extern   char     bystatus;         /* Bypass status/flags (ModBus) */
#define     BYS_HOTWIRED   0x01     /* Bypass key "hot-wired" */
#define     BYS_WAIT_OVFB  0x02     /* Must-wait before overfill bypass */
#define     BYS_NONBYPASS  0x04     /* Bypass used up / prohibited */
#define     BYS_DRY_NOOVFB 0x08     /* Tanks dry, no overfill bypass */
#define     BYS_KEYPRESENT 0x10     /* Bypass key is present */

extern unsigned char bylevel;       /* hierarchial bypassing levels */
#define     OVER_BYPASS    0x01     /* overflow bypass */
#define     GROUND_BYPASS  0x02     /* missing ground bolt bypass */
#define     VIP_BYPASS     0x08     /* VIP bypass */
/* Not really bypassable, but part of the same mask-set; returned as a
   "non-permit" reason in ModBus Register 68, which "parallels" bylevel... */
#define     DEADMAN_bypass 0x80     /* Deadman switch */

extern   char     baddeadman;       /* DEADMAN switch open or inactive */
extern   char     Permit_OK;        /* Tell backup to resume permit */
extern unsigned int deadman_voltage;    /* Save the deadman reading */
#define     CLOSE          0x00
#define     NO_DEAD        0x00
#define     OPEN           0x01

extern unsigned char badgndflag;
#define     GND_OK          0x00
#define     GND_FAIL        0x01
#define     GND_FAIL_R      0x02       /* Failure to detect resistive ground */
#define     GND_FAIL_D      0x04       /* Failure to detect diode ground */
#define     GND_INIT_TRIAL  0x08
#define     GND_NO_TEST     0x10
#define     GND_SHORTED     0x20
#define     GND_HARD_FAULT  0x40
#define     GND_NO_TRIAL    0x80
#define     GND_PROBLEMS    (GND_FAIL | GND_SHORTED | GND_HARD_FAULT)
extern unsigned short    BadGndCntr;
extern unsigned short    GoodGndCntr;

extern      char     badoverfillflag;   /* store the previous bad overfill state */

extern   unsigned int     badvipflag;
#define     BVF_UNAUTH      0x01        /* TIM serial number unauthorized */
#define     BVF_TIMCRC      0x02        /* TIM serial number CRC failure */
#define     BVF_TIMFAMILY   0x04        /* TIM not Dallas DS1993 family */
#define     BVF_TIMABSENT   0x08        /* TIM (Dallas chip) no response */
#define     BVF_TASDENY     0x10        /* TIM Unauthorized by TAS control */
#define     BVF_TASDELAY    0x20        /* Waiting for TAS to un/auth */
#define     BVF_DSNOAUTH    0x40        /* DateStamp did not authorize */
#define     BVF_DSERROR     0x80        /* DateStamp errors can't read */
#define     BVF_DONE        0x100       /* Already scanned the EEPROM */
#define     BVF_UNLOAD_EXP  0x200
#define     BVF_CPT_COUNT   0x400
#define     BVF_FUEL_TYPE   0x800
#define     BVF_INIT (BVF_TIMABSENT | BVF_UNAUTH)    /* 0x09 */
extern   short    BadVipCntr;
extern   char     badvipdscode;         /* Associated DateStamp code (stdsym.h) */

/*  code space variables */
extern unsigned int   KernelCRCval;         /* Actual Kernel CRC-16 calculated value */
extern unsigned int   Good_KernelCRCval;    /* Good Kernel CRC-16 calculated value */
extern unsigned int   ShellCRCval;          /* Actual Shell CRC-16 calculated value */
extern unsigned int   Good_Shell_CRC_val;   /* GOOD Shell CRC-16 calculated value */

extern   char           loopEighths;        /* .125-Second counter (delta time only) */
extern   char           secReset;           /* RESET counter (on trans to zero) */
extern   unsigned       hitCount;           /* Impact Sensor hits counted by SIM.C */
extern   unixtime       present_time;       /* present time */

/* Dallas type code buffers */
extern   unsigned char  touchbuf[33];           /* touch chip read/rite buffer   */
extern   unsigned char  truck_SN[BYTESERIAL];   /* Serial Number from truck  */
extern   unsigned char  bypass_SN[BYTESERIAL];  /* Last received bypass SN */
extern   unsigned char  month;                  /* Dallas converted UNIX time */
extern   unsigned char  day;
extern   unsigned int   year;
extern   unsigned char  hour;
extern   unsigned char  minute;
extern   unsigned char  second;
extern   unsigned char  clock_SN[BYTESERIAL];       /* Last received clock SN */
extern   char           clock_status;               /* 0 if OK */
extern   unsigned char  clock_SN[BYTESERIAL];       /* Last received clock SN */
extern   unsigned char  Intellitrol_SN[BYTESERIAL]; /* Last received SN */

#define  CLOCK_OK       0x00
#define  CLOCK_ABSENT   0x01
#define  CLOCK_CRC      0x02
#define  SERIAL_FAMILY  0x03
#define  CLOCK_RANGE    0x04
#define  CLOCK_STOPPED  0x05
#define  CLOCK_NOISY    0x06
#define  CLOCK_DEFAULT  0x07
#define  CLOCK_INVALID  0x08

#define  DALLAS_OK      0x00
#define  DALLAS_RESET   0x01
#define  DALLAS_ERR     0x02
#define  DALLAS_READ    0x03
#define  DALLAS_CRC     0x04
#define  DALLAS_FAMILY  0x05


extern   unsigned char  no_relay_flag;      /* Special Test Mode flag Use to make everything think we are permiting but do not activate the relay */
extern   unsigned char  scully_flag;        /* The truck has all Scully equipment */
extern   int probe_result_flag;             /* ADC interrupt flag */
extern   int dma_result_flag;               /* ADC interrupt flag */
extern   unsigned short mstimer;            /* Like TCNT, but milliseconds */
extern   unsigned long  freetimer;          /* timer 1 ms count */
extern   unsigned short service_time;
extern   unsigned long  dry_timer;          /* timer of dry probe attachment */
//UK >>>
extern   unsigned long  active_timer;       /* timer of dry probe attachment */
//UK <<<
extern   unsigned long  drive_time;         /* a force timer to prevent runaway */
extern   unsigned long  jump_time;          /* a timer to control JUMP_START */
extern   unsigned long  probe_time;         /* may NOT repeat for 30ms */
extern   unsigned long  bypass_time;        /* may only bypass for 1 hour */
extern   unsigned long  ground_time;        /* Time of next ground check */
extern   unsigned long  optic5_timeout;     /* Idle - try the optic5 every 2 seconds */
extern   unsigned long  relay_turn_on;      /* a force timer to prevent chatter */
extern   unsigned long  truck_timeout;      /* Used to determined if a truck left */

/******************************* 2/25/2009 1:20PM ****************************
 * Used to determind if the truck has stopped pulsing
 *****************************************************************************/
extern   unsigned int   probe_pulse_old;                /* Keeps track  */
extern   unsigned int   probe_pulse;                    /* Keeps track  */
extern   unsigned long  pulse_timeout;                  /* keep track of how long since last probe pulse */
extern   unsigned char  truck_pulsing;                  /* flag to indicate 2-wire pulses being seen */


extern   unsigned int   ReferenceVolt;                  /* Reference/diagnostic (1.000V) */
extern   unsigned int   Raw13Volt;                      /* Raw input (nominal 13V) voltage */
extern   unsigned int   BiasVolt;                       /* Probe bias (3.5/3.8) voltage */
extern   unsigned int   Optic5Volt;                     /* 5-Wire Optic Pulse voltage */
extern   signed int     noise_volt[COMPART_MAX];        /* Noise voltage margin */
extern   unsigned int   open_c_volt[2][COMPART_MAX];    /* first value 10vt / second 20vt */
extern   unsigned int   probe_volt[COMPART_MAX];        /* latest read of ADC's in mv */
extern   PROBE_TYPE     probe_type[2*COMPART_MAX];      /* sensor type */
extern   unsigned int   result_ptr[COMPART_MAX];        /* latest read of ADC's */
extern   unsigned int   old_probe_volt[COMPART_MAX];    /* previous read of ADC's in mv */
extern   int            adc_convert_flag;               /* Indicate that the analog conversion */
                                                        /*  has finished */
extern   char           berr;                           /* buss error occurred */
extern   char           dry_pass_count;                 /* scan passes for probe time */
extern   char           wet_pass_count;                 /* scan passes for probe time */
extern   char           gon_pass_count;                 /* scan passes for probe time */
extern   char           delay_time;                     /* settable delay relay on time */
extern   unsigned int   probe_index;                    /* index into the probe binary array */
extern   unsigned int   probe_array[MAX_ARRAY];         /* ADC bit conversion array */
extern   unsigned int   probe_signal[MAX_ARRAY];        /* ADC bit conversion array */
extern   char           main_array[MAX_RELAY];          /* main relay oscillation array */
extern   char           bak_array[MAX_RELAY];           /* backup relay oscillation array */
extern   char           bak_charge[MAX_RELAY];          /* backup charge driver array */

/******************************* 12/30/2008 7:12AM ***************************
 * Area to store the contents of the TIM truck configuration.
 * Byte 0 is the number of truck compartments
 * Byte 1 the volumn of the compartments are in gallons or liters. 1 - Gallons  2 - Liters
 * Byte 02 - 05 <= Volume of compartment 1
 * Byte 06 - 09 <= Volume of compartment 2
 * Byte 10 - 13 <= Volume of compartment 3
 * .
 * .
 * .
 * Byte 62 - 65 <= Volume of compartment 16
 *****************************************************************************/
extern unsigned char  Truck_TIM_Configuration[68];

/* MODBUS buffers & flags */

extern   unsigned char  modbus_addr;      /* ModBus address; 0 => ASCII/Debug */
extern   unsigned char  modbus_baud;      /* ModBus/Comm baud rate */
extern   unsigned char  modbus_parity;    /* ModBus/Comm parity */
extern   unsigned char  modbus_csize;     /* ModBus/Comm character size */
extern   unsigned char  modbus_err;
extern   unsigned char  modbus_state;
extern   unsigned char  modbusVIPmode;
extern   unsigned short modbus_eom_time;
extern   unsigned short modbus_swx_time;  /* ModBus "turnaround" time */
extern   unsigned short modbus_Recv_err;
extern   unsigned short modbus_PrepMsg_err;
extern   unsigned short modbus_DecodeMsg_err;

extern   unsigned char  modbus_rx_len;
extern   unsigned char  *modbus_rx_ptr;
extern   unsigned short modbus_rx_time;

extern   unsigned char  modbus_tx_len;
extern   unsigned char  *modbus_tx_ptr;
extern   unsigned short modbus_tx_time;

extern   unsigned char  modbus_rx_buff[MODBUS_MAX_LEN+1];
extern   unsigned char  modbus_tx_buff[MODBUS_MAX_LEN+1];
extern   unsigned char  save_last_recv[MODBUS_MAX_LEN+1];
extern   unsigned char  save_last_recv_len;
extern   unsigned char  save_last_xmit[MODBUS_MAX_LEN+1];
extern   unsigned char  save_last_xmit_len;

/* Main error logger */
extern   unsigned int   iambroke;       /* bit error logger */
#define     RAW_FAULT           0x0001  /* raw 13 voltage tolerance exceeded */
#define     REF_VOLT_FAULT      0x0002  /* Reference Volt error */
#define     PROBE_BIAS_ERROR    0x0004  /* the probe bias voltage is in error */
#define     NOISE_FAULT         0x0008  /* noise tolerance exceeded */
#define     TOL10V_FAULT        0x0010  /* 10 volt tolerance exceeded */
#define     TOL20V_FAULT        0x0020  /* 20 volt tolerance exceeded */
#define     TOL5WO_FAULT        0x0040  /* 5Wire Optic pulse voltage error */
#define     MEMORY_FAULT        0x0080  /* Internal memory fault was detected */

#define     VOLTS_FAULT (RAW_FAULT | PROBE_BIAS_ERROR | NOISE_FAULT \
                         | TOL10V_FAULT | TOL20V_FAULT | TOL5WO_FAULT )

#define     BK_CRC_ERR          0x0100  /* message CRC errors */
#define     BK_HW_ERR           0x0200  /* backup reported a hardware error */
#define     BK_RELAY_ERR        0x0400  /* backup reported relay error */
#define     BK_JUMPER_CHANGE    0x0800  /* backup saw jumper change */
#define     JUMPER_CHANGE       0x1000  /* jumper reading changed on fly */
#define     CABLE_SHORT         0x2000  /* line to line short in our cable */
#define     ADC_FAULT           0x4000  /* can't initialize the ADC's */
#define     BAD_TPUvsADC        0x8000  /* TPU sees oscillation & ADC doesn't */

extern   unsigned int   iamsuffering;   /* bit error logger */
#define     HARD_WIRE_FAULT  0x1000     /* both relays hard wired closed */


/* Main Intellitrol "Status" registers (e.g., as returned by ModBus)
   See STSBITS.H for definitions */

extern unsigned int    StatusA;         /* Status Reg "A" aka ISB 00 - 15 */
extern unsigned int    StatusB;         /* Status Reg "B" aka ISB 16 - 31 */
extern unsigned int    StatusO;         /* Status Reg "O" aka OSB 00 - 15 */
extern unsigned int    StatusP;         /* Status Reg "P" aka OSB 16 - 31 */

/* State bytes for backup and main processor relays */

extern char            BackRelaySt;     /* Backup Relay State */
extern char            MainRelaySt;     /* Main Relay State */
#define     RELAY_CLOSED    0x01        /* Relay is closed */
#define     RELAY_WANTED    0x02        /* Relay should be closed */
#define     RELAY_NOTSURE   0x10        /* Not sure/relay in transition */
#define     RELAY_BROKEN    0x20        /* WANTED and not CLOSED ("broken") */
#define     RELAY_SHORTED   0x40        /* CLOSED and not WANTED ("shorted") */


/*
   EEPROM (System/Non-Volatile) Variables
*/

extern unsigned char EE_status;         /* EEPROM system status -- 0 => OK */
#define     EE_DATAERROR    0x01        /* EEPROM Data (read or write) error */
//FogBugz 131 #define     EE_WRITETIMEOUT 0x02        /* EEPROM timeout writing data */
#define     EE_NEW          0x02        /* EEPROM unformatted - FogBugz 131  */
#define     EE_FORMAT       0x04        /* EEPROM needs formatting */
#define     EE_BADHOME      0x08        /* EEPROM DEAD no Home block access */
#define     EE_UPDATE       0x10        /* EEPROM old rev. - update nvsysblock etc.. */
#define     EE_CRC          0x20        /* EEPROM has bad CRC */
#define     EE_ACTIVE       0x40        /* EEPROM actively writing */
#define     EE_BUSY         0x80        /* EEPROM busy (bulk erase, etc.) */

extern unsigned char EE_valid;          /* EEPROM validity flags */
/* See esquared.h for definitions */

extern   E2SYSBLK       SysNonV;        /* copy of EEPROM's System NonVolatile area */
extern   SysParmNV      SysParm;        /* Active copy of Non-Volatile Sys Parms */
/* See esquared.h for definitions */

extern   DateStampNV    *pDateStamp;     /* Pointer to active DateStamp stuff */
extern   SysDia5NV      *pSysDia5;       /* Pointer to active SysDia5 stuff */
extern   SysVoltNV      *pSysVolt;       /* Pointer to active SysVolt stuff */
extern   SysSet1NV      *pSysSet1;       /* Pointer to active SysSet1 stuff */

extern   unsigned int   evMax;          /* Highest allowable entry */
extern   unsigned int   evIndex;        /* Index to next free entry */
extern   unsigned int   evLastRead;     /* Index last read via ModBus */

/*******************************5/20/2008 7:08AM******************************
 * eeprom stored blocks
 *****************************************************************************/
extern E2HOMEBLK        home_block;         /* Home block stored here */

extern unsigned long    probe_retry_count;  /* Used to determined if we are in a endless */
                                            /* loop trying to figure out what type of probe is connected */

/*****************************************************************************
 * TIM related data
 *****************************************************************************/
extern unsigned int     TIM_size;             /* Dallas chip NVRAM / EEPROM size */
extern unsigned int     TIM_scratchpad_size;  /* Dallas chip scratchpad size */
extern unsigned int     S_TIM_code;           /* Super TIM supported TRUE/FALSE */
extern unsigned int     TIM_fault_logged;
extern unsigned char    cert_ds_fails;
extern unsigned char    fuel_type_fails;
extern unsigned char    TIM_info_logged;
extern unsigned int     log_data_state;
extern unsigned int     log_time_address;
extern unsigned int     compare_volts;

/************************** end of comdat.h *********************************/
#endif      /* end of COMDAT_H */
