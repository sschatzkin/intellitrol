/*********************************************************************************************
 *
 *       File:       hardass.h
 *
 *       Author:     Ken Langlais
 *
 *       @Copyright 2008, 2014  Scully Signal Company
 *
 *       Description:
 *           CONSTANTS, CODES AND ADDRESSES FOR ALL HARDWARE REGISTERS
 *
 *       Revision History:
 *
 * Revision History:
 *   Rev      Date   Who  Description of Change Made
 * -------- -------- ---  --------------------------------------------
 * 1.5.27   03/25/14 DHP  Removed DS28CM00 references, this chip is not used.
 * 1.5.30  08/10/14  DHP  Renamed TRUCK_HERE to TB3P5P6
 * 1.5.31  01/14/15  DHP  Changed MUX0, MUX1 from latch to port 
 * 1.6.34  07/08/16  DHP  Added (deleting old references) HC_OFFx references
 *********************************************************************************************/
#ifndef HARDASS_H
#define HARDASS_H

// #include "p24Hxxxx.h"

#define COMM_PKT_SIZE 20  /* Size of packets used to communicate between CPUs */

/******************************* 10/27/2009 6:18AM ***************************
 * Program Memory addresses where checksum is stored
 *****************************************************************************/
#define CHECKSUM_LOW_ADDR   0x2ABF0
#define LAST_BLOCK_ADDR     0x2A800  /* Last block of Program Memory */
#define FLASH_CRC_CHUNK     0x1000

/******************************* 10/27/2009 6:17AM ***************************
 * Program memory constants
 *****************************************************************************/
#define PM_ROW_SIZE         64 * 8
#define CM_ROW_SIZE         8
#define CONFIG_WORD_SIZE    1

#define PM_ROW_ERASE        0x4042
#define PM_ROW_WRITE        0x4001
#define CONFIG_WORD_WRITE   0X4000

/*******************************5/20/2008 6:17AM******************************
 * I2C device address
 *****************************************************************************/

#define MC24FC1025_DEVICE 0xA0  /* MicroChip 24FC1025 128K EEPROM On I2C 2 bus*/
#define DS1371_DEVICE     0xD0  /* Real Time Clock On I2C 1 bus */
#define MCP23017_DEVICE   0x4A  /* Jumper MUX On I2C 2 bus */
#define PFC8570_DEVICE    0xA2  /* On I2C 1 bus */

#define EEPROM_DEVICE     MC24FC1025_DEVICE
#define RTC_DEVICE        DS1371_DEVICE
#define JUMPER_MUX        MCP23017_DEVICE

#define DS2401_SERIAL_ID    0x01  /* DS2401 family code */

/*******************************6/13/2008 8:33AM******************************
 * Intellitrol PIC24HJ256GP210 port definitions
 *****************************************************************************/

#define  PD15     (1 << 15 )
#define  PD14     (1 << 14 )
#define  PD13     (1 << 13 )
#define  PD12     (1 << 12 )
#define  PD11     (1 << 11 )
#define  PD10     (1 << 10 )
#define  PD9      (1 << 9  )
#define  PD8      (1 << 8  )
#define  PD7      (1 << 7  )
#define  PD6      (1 << 6  )
#define  PD5      (1 << 5  )
#define  PD4      (1 << 4  )
#define  PD3      (1 << 3  )
#define  PD2      (1 << 2  )
#define  PD1      (1 << 1  )
#define  PD0      1
#define  PB15     ((unsigned int)1 << 15 )
#define  PB13     (1 << 13 )

#define CHTST_INx       PORTE
#define CHTST_IN1       0x01
#define CHTST_IN2       0x02
#define CHTST_IN3       0x04
#define CHTST_IN4       0x08
#define CHTST_IN5       0x10
#define CHTST_IN6       0x20
#define CHTST_IN7       0x40
#define CHTST_IN8       0x80

#define HC_OFF1         LATAbits.LATA15
#define HC_OFF2         LATDbits.LATD5
#define HC_OFF3         LATAbits.LATA7
#define HC_OFF4         LATAbits.LATA4
#define HC_OFF5         LATBbits.LATB10
#define HC_OFF6         LATAbits.LATA12
#define HC_OFF7         LATAbits.LATA5
#define HC_OFF8         LATAbits.LATA6

#define MMAIN_CONTACT   PORTAbits.RA0
#define MBACK_CONTACT   PORTAbits.RA1
//#define MA14            LATAbits.LATA6
//#define MA15            LATAbits.LATA7
#define MAIN_CHARGE     LATAbits.LATA13  /* MAIN_RELAY charge pump handle */
// #define PVOLT2BUF       LATBbits.LATB8
// #define PVOLT1BUF       LATBbits.LATB9
//#define MA16            LATBbits.LATB10
#define MD6             LATBbits.LATB11
#define PULSE5VOLT      LATBbits.LATB14  /* 5-Wire-Optic Ch4 5 volt enable */
#define MBLINK1         LATCbits.LATC2
#define MBLINK2         LATCbits.LATC3
#define MBLINK3         LATCbits.LATC4

#define COMM_ID             PD0
#define RX_COMM_ID          PD1
#define READ_BYPASS         PD2
#define RX_READ_BYPASS      PD3
#define INTELLITROL_SN_INPUT  PB15
#define INTELLITROL_SN      5

#define COMM_ID_BIT         LATDbits.LATD0
#define READ_COMM_ID_BIT    PORTDbits.RD1
#define BYPASS_BIT          LATDbits.LATD2
#define READ_BYPASS_BIT     PORTDbits.RD3
#define SERIAL_BIT          LATBbits.LATB15
#define DEBUG_IN            PORTDbits.RD4     /* DEBUG jumper installed */
#define TXEN                LATDbits.LATD9
#define TXEN_BIT            PD9
#define OPTBDPRES           PORTDbits.RD10

#define GCHECK          LATDbits.LATD11
#define GCHECK_BIT      PD11
#define TB3P5P6           LATDbits.LATD12  /* "Good Ground" / "TRUCK_HERE" output signal */
#define MSERV_TRG_NEG   LATDbits.LATD13  /* Main "Service Led" (off) pump handle */
#define SCALE_AN1       LATDbits.LATD14  /*  */
#define SCALE_AN0       LATDbits.LATD15  /*  */
#define CH_TEST1        LATEbits.LATE0   /* Channel/Probe # 1 10 volt output enable */
#define CH_TEST2        LATEbits.LATE1   /* Channel/Probe # 2 10 volt output enable */
#define CH_TEST3        LATEbits.LATE2   /* Channel/Probe # 3 10 volt output enable */
#define CH_TEST4        LATEbits.LATE3   /* Channel/Probe # 4 10 volt output enable */
#define CH_TEST5        LATEbits.LATE4   /* Channel/Probe # 5 10 volt output enable */
#define CH_TEST6        LATEbits.LATE5   /* Channel/Probe # 6 10 volt output enable */
#define CH_TEST7        LATEbits.LATE6   /* Channel/Probe # 7 10 volt output enable */
#define CH_TEST8        LATEbits.LATE7   /* Channel/Probe # 8 10 volt output enable */
#define PROBE_CH_TEST1  0x01             /* Channel/Probe # 1 10 volt output enable */
#define PROBE_CH_TEST2  0x02             /* Channel/Probe # 2 10 volt output enable */
#define PROBE_CH_TEST3  0x04             /* Channel/Probe # 3 10 volt output enable */
#define PROBE_CH_TEST4  0x08             /* Channel/Probe # 4 10 volt output enable */
#define PROBE_CH_TEST5  0x10             /* Channel/Probe # 5 10 volt output enable */
#define PROBE_CH_TEST6  0x20             /* Channel/Probe # 6 10 volt output enable */
#define PROBE_CH_TEST7  0x40             /* Channel/Probe # 7 10 volt output enable */
#define PROBE_CH_TEST8  0x80             /* Channel/Probe # 8 10 volt output enable */
#define HHO             0xFF             /*  probes 1,2,3,4,5,6,7,8 w/JS */
#define PULSE_TEST      0xFF             /*  probes 1,2,3,4,5,6,7,8 */
#define OPTIC_DRIVE    0xD7             /*  probes 1,2,3,  5,  7,8 */
#define OPTIC_PULSE    0xDF             /*  probes 1,2,3,4,5,  7,8 */
#define OPTIC_DIAG      0x10              /*  probe 5*/
#define OPTIC_RTN       0x20               /*  probe 6*/
#define PIN6                    0x20              /*  probe 6 */
#define PIN8                    0x80              /*  probe 8 */

#define JUMP_START      LATFbits.LATF0   /* Jump-Start enable */
#define DIAGNOSTIC_EN   LATFbits.LATF1   /* 5-Wire-Optic Ch5 6.75 volt diag enable */
#define LED_DISP_CLK    LATFbits.LATF6
#define MAIN_ENABLE     LATFbits.LATF7   /* MAIN_RELAY "enable" or turn-on signal */
#define LED_DISP_DAT    LATFbits.LATF8
#define READ_IMPACT_SENSOR   TRISFbits.TRISF8  /* Set this bit to be able to read the impact sensor bit*/
#define IMPACT_SENSOR   PORTFbits.RF8  /* Read for abuse by looking at the impact sensor bit*/
#define METER_ON_NEG    PORTFbits.RF13   /* External "Meter Input (110AC)" signal */
#define EMUX            PD0 | PD1 | PD9  /* A/D Ch 0 & 1 Mux (U19, U20) select */
#define MUX0            PORTGbits.RG0   /*  */
#define MUX1            PORTGbits.RG1   /*  */
#define MSTAT_LED       LATDbits.LATD6
#define MUX2            PORTDbits.RD7   /*  */
#define DIS_MARX        LATGbits.LATG12  /* One of the controls a or gate for the serial modbus communications */
#define COMM_RESET      LATGbits.LATG13  /* System/Board RESET (low to reset) */
#define PEXT_RST        LATGbits.LATG14  /* RESET LAT expander MCP23017 U31(low to reset) */
#define DHP_PULSE     LATGbits.LATG15  /* On schematic as m_OB_CS */
//#define DHP_PULSE          LATBbits.LATB12

#define THERMISTOR_SWITCH_COUNT 10  /* The Thermistor high voltage must remain between the Thermistor and Optic before calling it a Thermistor probe */

#endif    /* end of HARDASS_H */
