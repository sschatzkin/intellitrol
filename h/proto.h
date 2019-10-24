/*********************************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         PROTO.H
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2008, 2014  Scully Signal Company
 *
 *   Description:    Main program functional prototype list for Rack controller
 *                   main microprocessor PIC24HJ256GP210
 *
  * Revision History:
 *   Rev      Date           Who   Description of Change Made
 *  --------  --------------  ---    --------------------------------------------
 *  1.5.30  08/10/14  DHP   Renamed GroundDiodePresent to CheckGroundPresent
 *                                       Removed truck_here 
 *                                       Removed vapor_test, read_ref_probe and read_flow_probe
 *                                       Added set_ground_reference
 *
 *********************************************************************************************/
#ifndef PROTO_H
#define PROTO_H

/************************** Microchip Builtin Prototypes *********************/
//unsigned int __builtin_dmaoffset(const void *p);
//unsigned long __builtin_tblpage(const void *p);
//void __builtin_write_OSCCONH(unsigned char value);
//void __builtin_write_OSCCONL(unsigned char value);

/************************** ADC Prototypes ***********************************/
char read_ADC(void);
void setup_probes(void);
int wait_for_probes(void);
char read_muxADC(unsigned int mux, SET_MUX muxchan, unsigned int *retval);
void convert_to_binary(void);
void read_probes(void);
void ops_ADC(char int_on);

/**************************** com_two Prototypes *****************************/
void HighI_Off(unsigned int probe);
void HighI_On(unsigned int probe);
void two_wire_start(void);
void active_two_wire(PROBE_TRY_STATE test_state);
char check_all_pulses(PROBE_TRY_STATE truck_probes);
unsigned int check_all_oscillating(void);
void check_shorts_opens(void);
int check_active_shorts(unsigned int channel);
unsigned int check_truck_gone(void);
char short_6to4_5to3(void);
unsigned char check_2wire(void);

/**************************** dallas Prototypes *****************************/
int reset_iButton(unsigned char port);
char Read_Intellitrol_SN (void);
char read_bypass (char action);
char Read_Dallas_SN (unsigned char port);
void Report_SN (const unsigned char *serial_number);
void report_bypass (unsigned int bypass_level);
unsigned char Dallas_Reset (unsigned char port);
char Dallas_Bit_Read (unsigned char port);
char Dallas_Bit_Write (char bit, unsigned char port);
unsigned char Dallas_Byte (unsigned char byte_sent, unsigned char port);
void report_clock ();
UINT8 Dallas_CRC8(UINT8 *buf, UINT8 len);
void Reset_Bypass_SN(unsigned char val);
void Reset_Truck_SN(unsigned char val);
char Read_Bypass_SN (void);
unsigned char Read_Truck_Presence (void);
char Read_Truck_SN (void);
char Check_Truck_SN(char slowflag);
char Read_Clock (void);
char Touch_Copied (char port);
char Write_Clock (void);
void UNIX_to_Greg (void);
unixtime Greg_to_UNIX(unsigned year, unsigned month, unsigned day, unsigned hour,
    unsigned minute);
void Print_Crnt_Time (void);
int read_TIM_compartment_info(void);
void test_for_new_front_panel(void);
void read_TIM_Go_NoGo_info(void);

/**************************** Delay Prototypes *****************************/
void DelayUS (unsigned int usperiod);
void DelayMS (unsigned int msperiod);

/***************************** dfile Prototypes ******************************/
char dfOpenFile(unsigned char port, const char *name, char oflag);
char dfGetByte(char *chptr);
void dfCloseFile(void);
char dsOpenFile(unsigned char port, const char *name, const unsigned char *psw, char oflag);
void dsCloseFile (void);
char dsGetByte(char *chptr);
char dsGetDecn(char size, unsigned *number);
char dsTruckValidate(unsigned char port, const char *name, const unsigned char *psw);
void is_truck_pulsing(void);

/**************************** dumfile Prototypes *****************************/
void  init_variables(void);
void  report_tank_state(void);
void  dry_probes(void);
void  dry_5W_probes(void);
void  unknown_probes(void);
void  probe_leds_off(void);

/****************************** eeprom Prototypes *****************************/
unsigned int EEPROM_write(unsigned char device_addr, unsigned long reg_addr,
                           const unsigned char *data_ptr, unsigned char length);
unsigned int EEPROM_read(unsigned char device_addr,
 unsigned long reg_addr, unsigned char *data_ptr, unsigned char length);
char eeBlockWrite(unsigned long loc, const unsigned char *datum, unsigned int count);
char eeBlockFill(unsigned long loc, unsigned char datum, unsigned int count);
UINT16 EEPROM_fill(UINT8 device_addr, UINT32 reg_addr,
                                                 UINT8 fill_data, UINT8 length);
char eeBlockRead(unsigned long loc, unsigned char *datum, unsigned count);
char eeCRC(void);

/**************************** esquared Prototypes *****************************/
char eeFormat (void);
char eeMapPartition(unsigned int partid, unsigned int *size, unsigned int *base);
char eeWriteBlock(unsigned int loc, const unsigned char *dptr, unsigned int count);
char eeWriteByte(unsigned int loc, unsigned char datum);
char eeReadBlock(unsigned int loc, unsigned char *dptr, unsigned int count);
void eeInit(void);
char eeUpdateHome(void);
void eeReport(void);
char eeUpdateSys(void);

/*************************** grndchck Prototypes ***************************/
unsigned char CheckGroundPresent(char truckhere);
void set_gcheck(void);
void clear_gcheck(void);
unsigned char test_gnd_idle(void);
void set_ground_reference(void);
/******************************* 7/25/2008 4:23PM ****************************
 * i2c1.c functional prototypes
 *****************************************************************************/
char DataRdyI2C1(void);
int MastergetsI2C1(UINT16 length, UINT8 *rdptr, UINT16 i2c1_data_wait);
char MasterWriteI2C1(UINT8 data_out);
void OpenI2C1(UINT16 config1, UINT16 config2);
void RestartI2C1(void);
void StartI2C1(void);
int StopI2C1(void);
int I2C1_reset(void);
UINT16 I2C1_write(UINT8 device_addr, UINT8 reg_addr, const UINT8 *data_ptr, UINT8 length);
UINT16 I2C1_read(UINT8 device_addr,  UINT8 reg_addr, UINT8 *data_ptr, UINT8 length);

/******************************* 7/25/2008 4:23PM ****************************
 * i2c2.c functional prototypes
 *****************************************************************************/
char DataRdyI2C2(void);
int MastergetsI2C2(UINT16 length, UINT8 *rdptr, UINT16 I2C2_data_wait);
char MasterWriteI2C2(UINT8 data_out);
void OpenI2C2(UINT16 config1, UINT16 config2);
void RestartI2C2(void);
void StartI2C2(void);
int StopI2C2(void);
int I2C2_reset(void);
UINT16 I2C2_write(UINT8 device_addr, UINT8 reg_addr, const UINT8 *data_ptr, UINT8 length);
UINT16 I2C2_read(UINT8 device_addr,  UINT8 reg_addr, UINT8 *data_ptr, UINT8 length);

/**************************** init_ADC Prototypes *****************************/
void Init_ADC(void);

/**************************** init_DMA Prototypes *****************************/
void Init_DMA0(void);

/************************** init_ports Prototypes **************************/
void init_ports(void);

/**************************** init_timer Prototypes *****************************/
void Init_Timer2(void);
void Init_Timer1(void);
void Init_Timer3(void);
void Init_Timer4(void);
void Init_32bit_Timer(void);
unsigned long read_32bit_realtime(void);
unsigned short DeltaMsTimer(unsigned short oldtime);      /* Old ("previous") value of mstimer */
unsigned long read_time(void);
UINT16 DeltaTCNT(unsigned short oldtime);  /* Old ("previous") value of timer count */
void debug_pulse(unsigned int x);
void DelayMS (unsigned int msperiod);
unsigned int read_realtime(void);

/*************************** jumpers Prototypes ****************************/
unsigned char fetch_jumpers(void);
int init_I_O_Expander(void);
char test_jumpers(void);
void read_jumpers(char flag);
unsigned char volts_jumper(unsigned volts);
char  deadman_ops(char past_deadman);

/*************************** memory_tests.s Prototypes ****************************/
void  memory_test(void);

/**************************** modbus Prototypes *****************************/
unsigned char  get_modbus_addr(void);
unsigned char  get_modbus_baud(void);
unsigned char  get_modbus_parity(void);
unsigned char  get_modbus_csize(void);
void  modbus_init(void);
void  modbus_handler(void);
void  modbus_execloop_process(void);
unsigned short  modbus_CRC(const unsigned char *, unsigned short, unsigned short);
unsigned short program_memory_CRC( unsigned long bufptr,  /* Starting address */
                           unsigned short buflen,  /* CRC Segment length */
                           unsigned short seed);   /* Initial CRC seed Value */

/**************************** modcmd Prototypes *****************************/
MODBSTS EnaFeatures(unsigned char *psw);

/**************************** modfrc Prototypes *****************************/
void mbfClrEnaFeatures(unsigned char bits);      /* Enable-flags to clear */
MODBSTS mbfSetEnaFeatures(unsigned char bits);      /* Enable-flags to clear */


/**************************** nvtruck Prototypes *****************************/
char nvKeyEmpty(word *index);
char nvKeyFind(unsigned char *key, word *index);
char nvKeyGet(unsigned char *key, word index);
char nvKeyGetMany(unsigned char *key, word index, unsigned char count);
char nvKeyPut(const unsigned char *key, word index);
char nvKeyPutMany(unsigned char *key, word index, unsigned char count);
char nvKeyDelete(word index);
char nvKeyErase (void);
char nvTrkEmpty(word *index);
char nvTrkFind(const unsigned char *trk_org, word *index);
char nvTrkGet(unsigned char *trk, word index);
char nvTrkGetMany(unsigned char *trk, word index, unsigned char count);
char nvTrkVrMany(word *trk, word index, word count);
char nvTrkPut(unsigned char *trk, word index);
char nvTrkPutMany(unsigned char *trk, word index, unsigned char count);
char nvTrkDelete(word index);
char nvTrkErase (void);

/**************************** nvsystem Prototypes *****************************/
char nvSysParmUpdate(void);
void nvSysParmDefaults (void);
char nvSysFormat (void);
char nvSysDia5Update (unsigned int updatedADCTable);
char nvSysVoltUpdate (void);
char nvSysDSUpdate(DateStampNV *dsptr);
char nvSysSet1Update (void);
char nvLogGet(unsigned char *eptr, unsigned int index);
char nvSysWrBlock(char etype, unsigned char bcnt, char *buf);
char nvLogInit (void);
char nvLogErase(void);
void nvSysInit (void);
void nvLogPut(char etyp, char esub, const char *eptr);
void nvLogMerge(char etyp, char esub, const char *eptr, unsigned int deltat);
void nvLogRepeat(char etyp, char esub, const char *eptr, unsigned long deltat);

/**************************** optic2 Prototypes *****************************/
void optic_2_setup(void);
char two_wire_optic(void);
char optic_present(void);

/**************************** optic5 Prototypes *****************************/
char check_time(char edge);
char try_five_wire(void);
char five_wire_optic(void);
void active_5wire(void);
void optic_5_setup(void);
void optic_5_pulse(void);
char check_echo(void);
char check_diag(void);
char check_5wire_fault(char do_two_wire);
unsigned int calc_tank(void);
char scully_probe(void);

/**************************** permit Prototypes *****************************/
void main_charge(void);
void init_permit_relay(void);
void permit_relay (void);
char permit_bypass(void);
char main_relay_state(char operation);
char bak_relay_state(char operation);
void clr_bypass(unsigned char bits);
void set_bypass(unsigned char bits);
void reset_bypass(void);

/**************************** POD Prototypes ********************************/
char diag_clock (void);
char flash_panel (void);
void show_revision(void);
char check_ref_volt (void);
char check_open_c_volt (void);
char check_tank_type(void);
char check_bias_noise (void);
char check_bias_volt (void);
char check_probe_noise(void);
char check_probe_volt (void);
char check_raw_13 (void);
char check_5wire_5volt(void);
char check_100OhmGnd (void);
char check_shell_crc (void);
void service_wait(char wait_time);
void service_charge(void);
char check_kernel_crc (void);
void diagnostics(unsigned ctlmsk);
char march_test(unsigned int *start_ptr, unsigned int *end_ptr);
char mem_test(void);

/**************************** printout Prototypes *****************************/
void ee83sts(unsigned int eests);
void xprintf(unsigned int message_number, unsigned int parameter1);

/************************** rino main Prototypes *****************************/
void loginit(char partid);                 /* EEV_* partition mask */
void logrelayerr(void);
void logvolterr(void);
void logmaintenanceerr(void);
void logcrcerr(void);
void logEEPROM(void);
void log_dome_out(void);
//UK >>>
void log_overfill(void);
//UK <<<                

void  toggle_led(void);

/************************** shorts Prototypes *********************************/
unsigned int StaticShortTest(bool probe_test);
unsigned char AllShortTest(void);

/************************** sim Prototypes *********************************/
void set_porte(UINT16 port_select);
void timer_heartbeat(void);
void set_mux(SET_MUX mux_enum);
SET_MUX fetch_mux(void);
void read_relays(void);
void set_permit(char data);
void set_nonpermit(unsigned char data);
unsigned long long_swap(unsigned long original);
unsigned int int_swap(unsigned int original);
void display_probe(void);
void dummy_func(unsigned char *dummy);
void set_new_led(LED_NAME led, char state);
void set_compartment_led(int led, char state);

/**************************** specops Prototypes ******************************/
void SpcLEDBlink(int compartment_flag);
void SpcLEDOff(void);
void SpecialOps(void);

/**************************** spi_eeprom Prototypes ******************************/
void SPI_EEPROMInit(void);
int SPI_EEPROMWriteByte(unsigned long Address, unsigned Data);
int SPI_EEPROMReadByte(unsigned long Address, unsigned char *data, unsigned int count);
int SPI_EEPROMPageWriteByte(unsigned long Address, const unsigned char *Data, unsigned int count);
char SPIPageWrite(unsigned long loc, const unsigned char *datum, unsigned count);
int SPI_EEPROMWriteEnable();
int SPI_EEPROMReadStatus(unsigned char cmd, _SPI_EEPROMStatus_ *status);
int spi_erase(void);
unsigned long SPI_EEPROMReadDeviceID(void);
int spi_erase_sector(unsigned long Address);

/**************************** spi_mpol Prototypes ******************************/
void SPIMPolInit(void);
unsigned SPIMPolPut(unsigned Data);
unsigned SPIMPolIsTransmitOver(void);

/************************** themister Prototypes ******************************/
void thermal_setup(void);
char two_wire_thermal(void);
char thermistor_present(void);

/************************** tim_utl Prototypes ******************************/
int write_to_scratchpad(unsigned int scratchpad_size, unsigned int count, unsigned int address, const unsigned char *buffer);
int verify_scratchpad(unsigned int scratchpad_size, unsigned int count, unsigned int address, const unsigned char *buffer, unsigned char *E_S);
int copy_scratchpad(unsigned int address, unsigned char E_S);
int tim_block_write(unsigned char *memory_ptr, unsigned int address, unsigned int count);
int dallas_fill(unsigned int scratchpad_size, unsigned int count, unsigned int address, const unsigned char *buffer);
int tim_block_read(unsigned char *memory_ptr, unsigned int address, unsigned int count);
unsigned char fetch_serial_number(unsigned char tim_type, unsigned char *tim_number);
void TIM_log_fault(unsigned int fault_val);
void log_date_and_time(unsigned int log_address);
unsigned char TIM_log_info(void);
char superTIM_ds_validate(void);
char check_unload_time(void);
char check_fuel_type(void);
char check_compartment_count(void);

/************************** truckstat Prototypes ******************************/
void set_main_state(MAIN_STATE newstate);
char bypass_operation(void);
void truck_gone(void);
void truck_fini(void);
void main_activity(void);
void which_probe_type(void);
void truck_inactive(void);

/******************** CPURegisterTest Prototypes *************************/
int CPU_RegisterTest(void);

/****************************** uart Prototypes ******************************/
void UART2Init(void);
void clear_tx2en(void);
void set_tx2_en(void);
void UART2PutChar(char Ch);
void flush_uart2(void);
void UART1Init(void);
void clear_tx1_en(void);
void flush_uart1(void);
void UART1PutChar(const unsigned char Ch);
void set_tx1en(void);
void send_backup_pkt(const unsigned char *pkt_ptr);

/***************************** SPI loader Prototype *****************************/
/******************************* 4/7/2009 10:05AM ****************************
 * The SPI Loader and all of its routines are located in Program memory
 * located between 0x400 and 0xC00
 *****************************************************************************/
int SPI_EEPROMWriteEnable(void);
void SPI_EEPROMInit(void);
int SPI_EEPROMWriteByte(unsigned long Address, unsigned Data);
int SPI_EEPROMReadByte(unsigned long Address, unsigned char *data, unsigned int count);
int SPI_EEPROMPageWriteByte(unsigned long Address, const unsigned char *Data, unsigned int count);
char SPIPageWrite(unsigned long loc, const unsigned char *datum, unsigned count);
int SPI_EEPROMReadStatus(unsigned char cmd, _SPI_EEPROMStatus_ *status);
int spi_erase(void);
unsigned long SPI_EEPROMReadDeviceID(void);
int spi_erase_sector(unsigned long Address);
int CPU_PCtest(void);
unsigned long TestFunction1(void);
unsigned long TestFunction2(void);

/******************************** From memory *********************************************/
void Erase(unsigned int, unsigned int, unsigned int);
void WriteLatch(unsigned int, unsigned int, unsigned int, unsigned int);
void WriteMem(unsigned int);

#define PC_TEST_PASS 1
#define PC_TEST_FAIL 0

#endif       /* end of PROTO_H */
