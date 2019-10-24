/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         tim_utl.h
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2012  Scully Signal Company
 *
 *   Description:    Super TIM definitions
 *
 * Revision History:
 *   Rev      Date   Who  Description of Change Made
 * -------- -------- ---  --------------------------------------------
 * 1.5.20   04/04/12 KLL  Changed Super TIM fuel type from byte to 16 bit word.
 *          03/25/14 DHP  Added DS2433 as known 1-wire chip.
 *                        Added defaults for TIM memory sizes
 *
 *****************************************************************************/
#ifndef TIM_UTL_H
#define TIM_UTL_H

#define DS1992        0x08
#define DS1993        0x06
#define DS1996        0x0C
#define DS2401        0x01
#define DS2431        0x2D
#define DS2433        0x23
#define DS28EC20      0x43

#define DS1992_SIZE   128
#define DS1993_SIZE   128
#define DS1996_SIZE   8192
#define DS2401_SIZE   0
#define DS2431_SIZE   128
#define DS2433_SIZE   512
#define DS28EC20_SIZE 2560

#define DS1992_SCRATCHPAD_SIZE   32
#define DS1993_SCRATCHPAD_SIZE   32
#define DS1996_SCRATCHPAD_SIZE   32
#define DS2401_SCRATCHPAD_SIZE   0
#define DS2431_SCRATCHPAD_SIZE   8
#define DS2433_SCRATCHPAD_SIZE   32
#define DS28EC20_SCRATCHPAD_SIZE 32
/*****************************************************************************
 * TIM memory size defaults
 *****************************************************************************/
#define DEFAULT_SCRATCHPAD_SIZE  0       /* Don't allow read/write to unknown*/
#define DEFAULT_SIZE             0       /* Don't allow read/write to unknown*/



#define ALTERNATE_TIM           0x024  /* Store a user defined TIM number allocate 6 bytes but the most significant byte must be 0 */

/******************************* 6/15/2009 8:20AM ****************************
 * Addresses of Builder Trailer Information stroed in the Super Tim
 *****************************************************************************/
#define VALID_ENTRIES_ADR           0x400  /* Table is valid in this two byte location contain 0xAA55 */

/******************************* 6/15/2009 8:24AM ****************************
 * The rev can be used to determine is new entries to the table are setup in
 * this Super Tim
 *****************************************************************************/
#ifdef NO_DEF
#define REV_OF_TABLE_ADR            0x402  /* 2 byte location containing version of this table */

#define VALID_TRUCKTEST_TIM_ADR     0x404  /* 1 byte - The stored Truck Tester TIM is valid (0x33) */
#define TRUCK_TEST_TIM_NUMBER_ADR   0x405  /* 6 bytes - Where the Truck Tester TIM is stored */
#define VALID_ALT_TIM_ADR           0x40B  /* 1 byte - The stored Alternate TIM is valid (0xCC)*/
#define ALT_TIM_NUMBER_ADR          0x40C  /* 6 bytes - Where the Alternate TIM is stored */
#define BUILDER_NAME_ADR            0x412  /* 40 byte ascii field */
#define BUILDER_ADDRESS_ADR         0x43A  /* The Truck Builder address can not exceed 70 bytes */
#define SERIAL_NUMBER_ADR           0x480  /* 10 bytes allocated for a serial number */
#define VIN_ADR                     0x48A  /* Store the Truck VIN number 20 bytes */
#define BUILD_DATE_ADR              0x49E  /* Build date YYYY/MM/DD a total of 4 bytes */
#define GVW_UNIT_ADR                0x4A9  /* Gross Vehicle Weight rating units 1 - pounds, 2 - Kilograms */
#define GVW_ADR                     0x4AA  /* Gross Vehicle Weight rating 4 bytes allocated */
#define INTELLICHECK_P_ADR          0x4AE  /* Intellicheck present 1 Byte: 0 - none, 1 - Version 1, 2 - Version 2, 3 - Version 3 */
#define SENSOR_TYPE_ADR             0x4AF  /* Sensor Type 1 byte: 1 - Thermistor, 2 - 2 wire optical, 3 - 5 wire */
#define NUMBER_COMPARTMENTS_ADR     0x4B0  /* Allocate 1 byte */
#define COMPARTMENT_UNIT_ADR        0x4B1  /* Allocate 1 byte: Compartment volume: 1 - Gallon, 2 - Imperial Gallon, 3 - Liters */
#define COMPARTMENT_1_ADR           0x4B2  /* The volume of compartment 1: 4 bytes allocated */
#define COMPARTMENT_2_ADR           0x4B6  /* The volume of compartment 2: 4 bytes allocated */
#define COMPARTMENT_3_ADR           0x4BA  /* The volume of compartment 3: 4 bytes allocated */
#define COMPARTMENT_4_ADR           0x4BE  /* The volume of compartment 4: 4 bytes allocated */
#define COMPARTMENT_5_ADR           0x4C2  /* The volume of compartment 5: 4 bytes allocated */
#define COMPARTMENT_6_ADR           0x4C6  /* The volume of compartment 6: 4 bytes allocated */
#define COMPARTMENT_7_ADR           0x4CA  /* The volume of compartment 7: 4 bytes allocated */
#define COMPARTMENT_8_ADR           0x4CE  /* The volume of compartment 8: 4 bytes allocated */
#define COMPARTMENT_9_ADR           0x4D2  /* The volume of compartment 9: 4 bytes allocated */
#define COMPARTMENT_10_ADR          0x4D6  /* The volume of compartment 10: 4 bytes allocated */
#define COMPARTMENT_11_ADR          0x4DA  /* The volume of compartment 11: 4 bytes allocated */
#define COMPARTMENT_12_ADR          0x4DE  /* The volume of compartment 12: 4 bytes allocated */
#define COMPARTMENT_13_ADR          0x4E2  /* The volume of compartment 13: 4 bytes allocated */
#define COMPARTMENT_14_ADR          0x4E6  /* The volume of compartment 14: 4 bytes allocated */
#define COMPARTMENT_15_ADR          0x4EA  /* The volume of compartment 15: 4 bytes allocated */
#define COMPARTMENT_16_ADR          0x4EE  /* The volume of compartment 16: 4 bytes allocated */
#define NO_RELAY_ADR                0x4F2  /* NO_RELAY: 1 byte allocated */
#define SCULLY_EQUIPMENT_ADR        0x4F3  /* Truck is outfitted with Scully Equipment 1 byte allocated */
#define FUEL_TYPE_1_ADR             0x4F4  /* The fuel type compartment 1: 1 bytes allocated */
#define FUEL_TYPE_2_ADR             0x4F6  /* The fuel type compartment 2: 1 bytes allocated */
#define FUEL_TYPE_3_ADR             0x4F8  /* The fuel type compartment 3: 1 bytes allocated */
#define FUEL_TYPE_4_ADR             0x4FA  /* The fuel type compartment 4: 1 bytes allocated */
#define FUEL_TYPE_5_ADR             0x4FC  /* The fuel type compartment 5: 1 bytes allocated */
#define FUEL_TYPE_6_ADR             0x4FE  /* The fuel type compartment 6: 1 bytes allocated */
#define FUEL_TYPE_7_ADR             0x500  /* The fuel type compartment 7: 1 bytes allocated */
#define FUEL_TYPE_8_ADR             0x502  /* The fuel type compartment 8: 1 bytes allocated */
#define FUEL_TYPE_9_ADR             0x504  /* The fuel type compartment 9: 1 bytes allocated */
#define FUEL_TYPE_10_ADR            0x506  /* The fuel type compartment 10: 1 bytes allocated */
#define FUEL_TYPE_11_ADR            0x508  /* The fuel type compartment 11: 1 bytes allocated */
#define FUEL_TYPE_12_ADR            0x50A  /* The fuel type compartment 12: 1 bytes allocated */
#define FUEL_TYPE_13_ADR            0x50C  /* The fuel type compartment 13: 1 bytes allocated */
#define FUEL_TYPE_14_ADR            0x50E  /* The fuel type compartment 14: 1 bytes allocated */
#define FUEL_TYPE_15_ADR            0x510  /* The fuel type compartment 15: 1 bytes allocated */
#define FUEL_TYPE_16_ADR            0x512  /* The fuel type compartment 16: 1 bytes allocated */
#define BUILDER_END_ADR             0x514  /* End of valid Builder info area  */
#endif

#define LOAD_ITROL_SN_ADDR          0x100
#define ITROL_FW_VERSION_ADDR       0x10A
#define FAULT_LOG_PRT_ADDR          0x134
#define UNLOAD_ITROL_SN_ADDR        0x135

#define CARRIER_NAME_ADDR           0x200
#define CARRIER_ADDRESS_ADDR        0x214
#define CONTRACT_NUMBER_ADDR        0x23C
#define OPPERATING_SERVICE_ADDR     0x246
#define DRIVER_ID_ADDR              0x25A
#define ALLOWABLE_VOL_CPT1_ADDR     0x26E
#define ALLOWABLE_VOL_CPT2_ADDR     0x270
#define ALLOWABLE_VOL_CPT3_ADDR     0x272
#define ALLOWABLE_VOL_CPT4_ADDR     0x274
#define ALLOWABLE_VOL_CPT5_ADDR     0x276
#define ALLOWABLE_VOL_CPT6_ADDR     0x278
#define ALLOWABLE_VOL_CPT7_ADDR     0x27A
#define ALLOWABLE_VOL_CPT8_ADDR     0x27C
#define ALLOWABLE_VOL_CPT9_ADDR     0x27E
#define ALLOWABLE_VOL_CPT10_ADDR    0x280
#define ALLOWABLE_VOL_CPT11_ADDR    0x282
#define ALLOWABLE_VOL_CPT12_ADDR    0x284
#define ALLOWABLE_VOL_CPT13_ADDR    0x286
#define ALLOWABLE_VOL_CPT14_ADDR    0x288
#define ALLOWABLE_VOL_CPT15_ADDR    0x28A
#define ALLOWABLE_VOL_CPT16_ADDR    0x28C

#define VAP_TIGHT_CERT_TYPE_ADDR        0x300
#define VAP_TIGHT_CERT_DATE_ADDR        0x301
#define VAP_TIGHT_CERT_NUMBER_ADDR      0x304
#define SAFE_PASS_CERT_TYPE_ADDR        0x322
#define SAFE_PASS_CERT_DATE_ADDR        0x323
#define SAFE_PASS_CERT_NUMBER_ADDR      0x326
#define CERT_3_TYPE_ADDR                0x344
#define CERT_3_DATE_ADDR                0x345
#define CERT_3_NUMBER_ADDR              0x348
#define CERT_4_TYPE_ADDR                0x366
#define CERT_4_DATE_ADDR                0x367
#define CERT_4_NUMBER_ADDR              0x36A
#define CERT_5_TYPE_ADDR                0x388
#define CERT_5_DATE_ADDR                0x389
#define CERT_5_NUMBER_ADDR              0x38C

#define TABLE_VALID_ADDR                0x400
#define TABLE_REVISION_ADDR             0x402
#define ALT_TIM_ID_VALID_ADDR           0x40B
#define ALT_TIM_ID_ADDR                 0x40C
#define NUMBER_OF_COMPARTMENTS_ADDR     0x477
#define COMPARTMENT_VOLUME_UNITS_ADDR   0x478
#define TRAILER_ID_NUMBER_ADDR          0x4A2
#define COMPARTMENT_CONFIG_ADDR         0x4CA
#define VAPOR_INTERLOCK_TYPE_ADDR       0x4D0
#define CPT1_TYPES_ALLOWED_ADDR         0x4D1
#define CPT2_TYPES_ALLOWED_ADDR         0x4D4
#define CPT3_TYPES_ALLOWED_ADDR         0x4D7
#define CPT4_TYPES_ALLOWED_ADDR         0x4DA
#define CPT5_TYPES_ALLOWED_ADDR         0x4DD
#define CPT6_TYPES_ALLOWED_ADDR         0x4E0
#define CPT7_TYPES_ALLOWED_ADDR         0x4E3
#define CPT8_TYPES_ALLOWED_ADDR         0x4E6
#define CPT9_TYPES_ALLOWED_ADDR         0x4E9
#define CPT10_TYPES_ALLOWED_ADDR        0x4EC
#define CPT11_TYPES_ALLOWED_ADDR        0x4EF
#define CPT12_TYPES_ALLOWED_ADDR        0x4F2
#define CPT13_TYPES_ALLOWED_ADDR        0x4F5
#define CPT14_TYPES_ALLOWED_ADDR        0x4F8
#define CPT15_TYPES_ALLOWED_ADDR        0x4FB
#define CPT16_TYPES_ALLOWED_ADDR        0x4FE
#define MAX_LOADING_TEMP_ADDR           0x501
#define TEMPERATURE_UNITS_ADDR          0x502

#define CPT1_TYPE_LOADED_ADDR           0x600
#define CPT1_BATCH_ID_LOADED_ADDR       0x603
#define CPT1_VOLUME_LOADED_ADDR         0x606
#define CPT2_TYPE_LOADED_ADDR           0x608
#define CPT2_BATCH_ID_LOADED_ADDR       0x60B
#define CPT2_VOLUME_LOADED_ADDR         0x60E
#define CPT3_TYPE_LOADED_ADDR           0x610
#define CPT3_BATCH_ID_LOADED_ADDR       0x613
#define CPT3_VOLUME_LOADED_ADDR         0x616
#define CPT4_TYPE_LOADED_ADDR           0x618
#define CPT4_BATCH_ID_LOADED_ADDR       0x61B
#define CPT4_VOLUME_LOADED_ADDR         0x61E
#define CPT5_TYPE_LOADED_ADDR           0x620
#define CPT5_BATCH_ID_LOADED_ADDR       0x623
#define CPT5_VOLUME_LOADED_ADDR         0x626
#define CPT6_TYPE_LOADED_ADDR           0x628
#define CPT6_BATCH_ID_LOADED_ADDR       0x62B
#define CPT6_VOLUME_LOADED_ADDR         0x62E
#define CPT7_TYPE_LOADED_ADDR           0x630
#define CPT7_BATCH_ID_LOADED_ADDR       0x633
#define CPT7_VOLUME_LOADED_ADDR         0x636
#define CPT8_TYPE_LOADED_ADDR           0x638
#define CPT8_BATCH_ID_LOADED_ADDR       0x63B
#define CPT8_VOLUME_LOADED_ADDR         0x63E
#define CPT9_TYPE_LOADED_ADDR           0x640
#define CPT9_BATCH_ID_LOADED_ADDR       0x643
#define CPT9_VOLUME_LOADED_ADDR         0x646
#define CPT10_TYPE_LOADED_ADDR          0x648
#define CPT10_BATCH_ID_LOADED_ADDR      0x64B
#define CPT10_VOLUME_LOADED_ADDR        0x64E
#define CPT11_TYPE_LOADED_ADDR          0x650
#define CPT11_BATCH_ID_LOADED_ADDR      0x653
#define CPT11_VOLUME_LOADED_ADDR        0x656
#define CPT12_TYPE_LOADED_ADDR          0x658
#define CPT12_BATCH_ID_LOADED_ADDR      0x65B
#define CPT12_VOLUME_LOADED_ADDR        0x65E
#define CPT13_TYPE_LOADED_ADDR          0x660
#define CPT13_BATCH_ID_LOADED_ADDR      0x663
#define CPT13_VOLUME_LOADED_ADDR        0x666
#define CPT14_TYPE_LOADED_ADDR          0x668
#define CPT14_BATCH_ID_LOADED_ADDR      0x66B
#define CPT14_VOLUME_LOADED_ADDR        0x66E
#define CPT15_TYPE_LOADED_ADDR          0x670
#define CPT15_BATCH_ID_LOADED_ADDR      0x673
#define CPT15_VOLUME_LOADED_ADDR        0x676
#define CPT16_TYPE_LOADED_ADDR          0x678
#define CPT16_BATCH_ID_LOADED_ADDR      0x67B
#define CPT16_VOLUME_LOADED_ADDR        0x67E
#define TERMINAL_NAME_ADDR              0x687
#define TERMINAL_ADDRESS_ADDR           0x69B
#define TERMINAL_GANTRY_NUMBER_ADDR     0x6C3

#define FAULT_LOG1_ADDR                 0x10C
#define FAULT_LOG2_ADDR                 0x112
#define FAULT_LOG3_ADDR                 0x118
#define FAULT_LOG4_ADDR                 0x11E
#define FAULT_LOG5_ADDR                 0x124
#define SERVICE_CENTER_NAME_ADDR        0x3AD
#define SERVICE_CENTER_ADDRESS_ADDR     0x3C1

#define BUILDER_NAME_ADDR               0x412
#define BUILDER_ADDRESS_ADDR            0x426
#define TRUCK_SERIAL_NUMBER_ADDR        0x44E
#define TRUCK_VIN_ADDR                  0x458
#define TRUCK_BUILD_DATE_ADDR           0x46C
#define TRUCK_WEIGHT_UNITS_ADDR         0x46F
#define TRUCK_GVW_ADDR                  0x470
#define INTELLICHECK_TYPE_ADDR          0x474
#define OVERFILL_SENSOR_TYPE_ADDR       0x475
#define RETAINED_SENSOR_TYPE_ADDR       0x476
#define CPT1_BUILD_VOLUME_ADDR          0x479
#define CPT2_BUILD_VOLUME_ADDR          0x47B
#define CPT3_BUILD_VOLUME_ADDR          0x47D
#define CPT4_BUILD_VOLUME_ADDR          0x47F
#define CPT5_BUILD_VOLUME_ADDR          0x481
#define CPT6_BUILD_VOLUME_ADDR          0x483
#define CPT7_BUILD_VOLUME_ADDR          0x485
#define CPT8_BUILD_VOLUME_ADDR          0x487
#define CPT9_BUILD_VOLUME_ADDR          0x489
#define CPT10_BUILD_VOLUME_ADDR         0x48B
#define CPT11_BUILD_VOLUME_ADDR         0x48D
#define CPT12_BUILD_VOLUME_ADDR         0x48F
#define CPT13_BUILD_VOLUME_ADDR         0x491
#define CPT14_BUILD_VOLUME_ADDR         0x493
#define CPT15_BUILD_VOLUME_ADDR         0x495
#define CPT16_BUILD_VOLUME_ADDR         0x497
#define SCULLY_SENSORS_ADDR             0x49A
#define TANK_MODEL_NUMBER_ADDR          0x4B6
#define MAX_WORKING_PRESSURE_ADDR       0x4CB
#define ALLOWABLE_WORKING_PRESSURE_ADDR 0x4CD
#define PRESSURE_UNITS_ADDR             0x4CF
#define BULKHEADS_ADDR                  0x503
#define TANK_PROFILE_ADDR               0x504
#define OVERFILL_SENSOR1_LENGTH_ADDR    0x518
#define OVERFILL_SENSOR2_LENGTH_ADDR    0x519
#define OVERFILL_SENSOR3_LENGTH_ADDR    0x51A
#define OVERFILL_SENSOR4_LENGTH_ADDR    0x51B
#define OVERFILL_SENSOR5_LENGTH_ADDR    0x51C
#define OVERFILL_SENSOR6_LENGTH_ADDR    0x51D
#define OVERFILL_SENSOR7_LENGTH_ADDR    0x51E
#define OVERFILL_SENSOR8_LENGTH_ADDR    0x51F
#define OVERFILL_SENSOR9_LENGTH_ADDR    0x520
#define OVERFILL_SENSOR10_LENGTH_ADDR   0x521
#define OVERFILL_SENSOR11_LENGTH_ADDR   0x522
#define OVERFILL_SENSOR12_LENGTH_ADDR   0x523
#define OVERFILL_SENSOR13_LENGTH_ADDR   0x524
#define OVERFILL_SENSOR14_LENGTH_ADDR   0x525
#define OVERFILL_SENSOR15_LENGTH_ADDR   0x526
#define OVERFILL_SENSOR16_LENGTH_ADDR   0x527
#define OVERFILL_SENSOR17_LENGTH_ADDR   0x528
#define OVERFILL_SENSOR18_LENGTH_ADDR   0x529
#define OVERFILL_SENSOR19_LENGTH_ADDR   0x52A
#define OVERFILL_SENSOR20_LENGTH_ADDR   0x52B
#define OVERFILL_SENSOR21_LENGTH_ADDR   0x52C
#define OVERFILL_SENSOR22_LENGTH_ADDR   0x52D
#define OVERFILL_SENSOR23_LENGTH_ADDR   0x52E
#define OVERFILL_SENSOR24_LENGTH_ADDR   0x52F
#define LENGTH_UNITS_ADDR               0x530

#define LAST_LOAD_DATE_TIME_ADDR        0x681
#define LOAD_HISTORY_PRT_ADDR           0x944
#define LAST_UNLOAD_DATE_TIME_ADDR      0x945


/******************************* 6/15/2009 11:55AM ***************************
 * Modbus case entries to read/modify the above fields
 *****************************************************************************/
#ifdef NO_DEF
#define VALID_ENTRIES_CASE          1
#define REV_OF_TABLE_CASE           2
#define VALID_TRUCKTEST_TIM_CASE    3
#define TRUCK_TEST_TIM_NUMBER_CASE  4
#define VALID_ALT_TIM_CASE          5
#define ALT_TIM_NUMBER_CASE         6
#define BUILDER_NAME_CASE           7
#define BUILDER_ADDRESS_CASE        8
#define SERIAL_NUMBER_CASE          9
#define VIN_CASE                    10
#define BUILD_DATE_CASE             11
#define GVW_UNIT_CASE               12
#define GVW_CASE                    13
#define INTELLICHECK_P_CASE         14
#define SENSOR_TYPE_CASE            15
#define NUMBER_COMPARTMENTS_CASE    16
#define COMPARTMENT_UNIT_CASE       17
#define COMPARTMENT_1_CASE          18
#define COMPARTMENT_2_CASE          19
#define COMPARTMENT_3_CASE          20
#define COMPARTMENT_4_CASE          21
#define COMPARTMENT_5_CASE          22
#define COMPARTMENT_6_CASE          23
#define COMPARTMENT_7_CASE          24
#define COMPARTMENT_8_CASE          25
#define COMPARTMENT_9_CASE          26
#define COMPARTMENT_10_CASE         27
#define COMPARTMENT_11_CASE         28
#define COMPARTMENT_12_CASE         29
#define COMPARTMENT_13_CASE         30
#define COMPARTMENT_14_CASE         31
#define COMPARTMENT_15_CASE         32
#define COMPARTMENT_16_CASE         33
#define NO_RELAY_CASE               34
#define SCULLY_EQUIPMENT_CASE       35
#define FUEL_TYPE_1_CASE            36
#define FUEL_TYPE_2_CASE            37
#define FUEL_TYPE_3_CASE            38
#define FUEL_TYPE_4_CASE            39
#define FUEL_TYPE_5_CASE            40
#define FUEL_TYPE_6_CASE            41
#define FUEL_TYPE_7_CASE            42
#define FUEL_TYPE_8_CASE            43
#define FUEL_TYPE_9_CASE            44
#define FUEL_TYPE_10_CASE           45
#define FUEL_TYPE_11_CASE           46
#define FUEL_TYPE_12_CASE           47
#define FUEL_TYPE_13_CASE           48
#define FUEL_TYPE_14_CASE           49
#define FUEL_TYPE_15_CASE           50
#define FUEL_TYPE_16_CASE           51
#endif
#define CARRIER_NAME_CASE           0x01
#define CARRIER_ADDRESS_CASE        0x02
#define CONTRACT_NUMBER_CASE        0x03
#define OPPERATING_SERVICE_CASE     0x04
#define DRIVER_ID_CASE              0x05
#define ALLOWABLE_VOL_CPT1_CASE     0x06
#define ALLOWABLE_VOL_CPT2_CASE     0x07
#define ALLOWABLE_VOL_CPT3_CASE     0x08
#define ALLOWABLE_VOL_CPT4_CASE     0x09
#define ALLOWABLE_VOL_CPT5_CASE     0x0A
#define ALLOWABLE_VOL_CPT6_CASE     0x0B
#define ALLOWABLE_VOL_CPT7_CASE     0x0C
#define ALLOWABLE_VOL_CPT8_CASE     0x0D
#define ALLOWABLE_VOL_CPT9_CASE     0x0E
#define ALLOWABLE_VOL_CPT10_CASE     0x0F
#define ALLOWABLE_VOL_CPT11_CASE     0x10
#define ALLOWABLE_VOL_CPT12_CASE     0x11
#define ALLOWABLE_VOL_CPT13_CASE     0x12
#define ALLOWABLE_VOL_CPT14_CASE     0x13
#define ALLOWABLE_VOL_CPT15_CASE     0x14
#define ALLOWABLE_VOL_CPT16_CASE     0x15

#define VAP_TIGHT_CERT_TYPE_CASE        0x16
#define VAP_TIGHT_CERT_DATE_CASE        0x17
#define VAP_TIGHT_CERT_NUMBER_CASE      0x18
#define SAFE_PASS_CERT_TYPE_CASE        0x19
#define SAFE_PASS_CERT_DATE_CASE        0x1A
#define SAFE_PASS_CERT_NUMBER_CASE      0x1B
#define CERT_3_TYPE_CASE                0x1C
#define CERT_3_DATE_CASE                0x1D
#define CERT_3_NUMBER_CASE              0x1E
#define CERT_4_TYPE_CASE                0x1F
#define CERT_4_DATE_CASE                0x20
#define CERT_4_NUMBER_CASE              0x21
#define CERT_5_TYPE_CASE                0x22
#define CERT_5_DATE_CASE                0x23
#define CERT_5_NUMBER_CASE              0x24

#define TABLE_VALID_CASE                0x25
#define TABLE_REVISION_CASE             0x26
#define ALT_TIM_ID_VALID_CASE           0x27
#define ALT_TIM_ID_CASE                 0x28
#define NUMBER_OF_COMPARTMENTS_CASE     0x29
#define COMPARTMENT_VOLUME_UNITS_CASE   0x2A
#define TRAILER_ID_NUMBER_CASE          0x2B
#define COMPARTMENT_CONFIG_CASE         0x2C
#define VAPOR_INTERLOCK_TYPE_CASE       0x2D
#define CPT1_TYPES_ALLOWED_CASE         0x2E
#define CPT2_TYPES_ALLOWED_CASE         0x2F
#define CPT3_TYPES_ALLOWED_CASE         0x30
#define CPT4_TYPES_ALLOWED_CASE         0x31
#define CPT5_TYPES_ALLOWED_CASE         0x32
#define CPT6_TYPES_ALLOWED_CASE         0x33
#define CPT7_TYPES_ALLOWED_CASE         0x34
#define CPT8_TYPES_ALLOWED_CASE         0x35
#define CPT9_TYPES_ALLOWED_CASE         0x36
#define CPT10_TYPES_ALLOWED_CASE        0x37
#define CPT11_TYPES_ALLOWED_CASE        0x38
#define CPT12_TYPES_ALLOWED_CASE        0x39
#define CPT13_TYPES_ALLOWED_CASE        0x3A
#define CPT14_TYPES_ALLOWED_CASE        0x3B
#define CPT15_TYPES_ALLOWED_CASE        0x3C
#define CPT16_TYPES_ALLOWED_CASE        0x3D
#define MAX_LOADING_TEMP_CASE           0x3E
#define TEMPERATURE_UNITS_CASE          0x3F

#define CPT1_TYPE_LOADED_CASE           0x40
#define CPT1_BATCH_ID_LOADED_CASE       0x41
#define CPT1_VOLUME_LOADED_CASE         0x42
#define CPT2_TYPE_LOADED_CASE           0x43
#define CPT2_BATCH_ID_LOADED_CASE       0x44
#define CPT2_VOLUME_LOADED_CASE         0x45
#define CPT3_TYPE_LOADED_CASE           0x46
#define CPT3_BATCH_ID_LOADED_CASE       0x47
#define CPT3_VOLUME_LOADED_CASE         0x48
#define CPT4_TYPE_LOADED_CASE           0x49
#define CPT4_BATCH_ID_LOADED_CASE       0x4A
#define CPT4_VOLUME_LOADED_CASE         0x4B
#define CPT5_TYPE_LOADED_CASE           0x4C
#define CPT5_BATCH_ID_LOADED_CASE       0x4D
#define CPT5_VOLUME_LOADED_CASE         0x4E
#define CPT6_TYPE_LOADED_CASE           0x4F
#define CPT6_BATCH_ID_LOADED_CASE       0x50
#define CPT6_VOLUME_LOADED_CASE         0x51
#define CPT7_TYPE_LOADED_CASE           0x52
#define CPT7_BATCH_ID_LOADED_CASE       0x53
#define CPT7_VOLUME_LOADED_CASE         0x54
#define CPT8_TYPE_LOADED_CASE           0x55
#define CPT8_BATCH_ID_LOADED_CASE       0x56
#define CPT8_VOLUME_LOADED_CASE         0x57
#define CPT9_TYPE_LOADED_CASE           0x58
#define CPT9_BATCH_ID_LOADED_CASE       0x59
#define CPT9_VOLUME_LOADED_CASE         0x5A
#define CPT10_TYPE_LOADED_CASE          0x5B
#define CPT10_BATCH_ID_LOADED_CASE      0x5C
#define CPT10_VOLUME_LOADED_CASE        0x5D
#define CPT11_TYPE_LOADED_CASE          0x5E
#define CPT11_BATCH_ID_LOADED_CASE      0x5F
#define CPT11_VOLUME_LOADED_CASE        0x60
#define CPT12_TYPE_LOADED_CASE          0x61
#define CPT12_BATCH_ID_LOADED_CASE      0x62
#define CPT12_VOLUME_LOADED_CASE        0x63
#define CPT13_TYPE_LOADED_CASE          0x64
#define CPT13_BATCH_ID_LOADED_CASE      0x65
#define CPT13_VOLUME_LOADED_CASE        0x66
#define CPT14_TYPE_LOADED_CASE          0x67
#define CPT14_BATCH_ID_LOADED_CASE      0x68
#define CPT14_VOLUME_LOADED_CASE        0x69
#define CPT15_TYPE_LOADED_CASE          0x6A
#define CPT15_BATCH_ID_LOADED_CASE      0x6B
#define CPT15_VOLUME_LOADED_CASE        0x6C
#define CPT16_TYPE_LOADED_CASE          0x6D
#define CPT16_BATCH_ID_LOADED_CASE      0x6E
#define CPT16_VOLUME_LOADED_CASE        0x6F
#define TERMINAL_NAME_CASE              0x70
#define TERMINAL_ADDRESS_CASE           0x71
#define TERMINAL_GANTRY_NUMBER_CASE     0x72

#define FAULT_LOG1_CASE                 0x73
#define FAULT_LOG2_CASE                 0x74
#define FAULT_LOG3_CASE                 0x75
#define FAULT_LOG4_CASE                 0x76
#define FAULT_LOG5_CASE                 0x77
#define SERVICE_CENTER_NAME_CASE        0x78
#define SERVICE_CENTER_ADDRESS_CASE     0x79

#define BUILDER_NAME_CASE               0x7A
#define BUILDER_ADDRESS_CASE            0x7B
#define TRUCK_SERIAL_NUMBER_CASE        0x7C
#define TRUCK_VIN_CASE                  0x7D
#define TRUCK_BUILD_DATE_CASE           0x7E
#define TRUCK_WEIGHT_UNITS_CASE         0x7F
#define TRUCK_GVW_CASE                  0x80
#define INTELLICHECK_TYPE_CASE          0x81
#define OVERFILL_SENSOR_TYPE_CASE       0x82
#define RETAINED_SENSOR_TYPE_CASE       0x83
#define CPT1_BUILD_VOLUME_CASE          0x84
#define CPT2_BUILD_VOLUME_CASE          0x85
#define CPT3_BUILD_VOLUME_CASE          0x86
#define CPT4_BUILD_VOLUME_CASE          0x87
#define CPT5_BUILD_VOLUME_CASE          0x88
#define CPT6_BUILD_VOLUME_CASE          0x89
#define CPT7_BUILD_VOLUME_CASE          0x8A
#define CPT8_BUILD_VOLUME_CASE          0x8B
#define CPT9_BUILD_VOLUME_CASE          0x8C
#define CPT10_BUILD_VOLUME_CASE         0x8D
#define CPT11_BUILD_VOLUME_CASE         0x8E
#define CPT12_BUILD_VOLUME_CASE         0x8F
#define CPT13_BUILD_VOLUME_CASE         0x90
#define CPT14_BUILD_VOLUME_CASE         0x91
#define CPT15_BUILD_VOLUME_CASE         0x92
#define CPT16_BUILD_VOLUME_CASE         0x93
#define SCULLY_SENSORS_CASE             0x94
#define TANK_MODEL_NUMBER_CASE          0x95
#define MAX_WORKING_PRESSURE_CASE       0x96
#define ALLOWABLE_WORKING_PRESSURE_CASE 0x97
#define PRESSURE_UNITS_CASE             0x98
#define BULKHEADS_CASE                  0x99
#define TANK_PROFILE_CASE               0x9A
#define OVERFILL_SENSOR1_LENGTH_CASE    0x9B
#define OVERFILL_SENSOR2_LENGTH_CASE    0x9C
#define OVERFILL_SENSOR3_LENGTH_CASE    0x9D
#define OVERFILL_SENSOR4_LENGTH_CASE    0x9E
#define OVERFILL_SENSOR5_LENGTH_CASE    0x9F
#define OVERFILL_SENSOR6_LENGTH_CASE    0xA0
#define OVERFILL_SENSOR7_LENGTH_CASE    0xA1
#define OVERFILL_SENSOR8_LENGTH_CASE    0xA2
#define OVERFILL_SENSOR9_LENGTH_CASE    0xA3
#define OVERFILL_SENSOR10_LENGTH_CASE   0xA4
#define OVERFILL_SENSOR11_LENGTH_CASE   0xA5
#define OVERFILL_SENSOR12_LENGTH_CASE   0xA6
#define OVERFILL_SENSOR13_LENGTH_CASE   0xA7
#define OVERFILL_SENSOR14_LENGTH_CASE   0xA8
#define OVERFILL_SENSOR15_LENGTH_CASE   0xA9
#define OVERFILL_SENSOR16_LENGTH_CASE   0xAA
#define OVERFILL_SENSOR17_LENGTH_CASE   0xAB
#define OVERFILL_SENSOR18_LENGTH_CASE   0xAC
#define OVERFILL_SENSOR19_LENGTH_CASE   0xAD
#define OVERFILL_SENSOR20_LENGTH_CASE   0xAE
#define OVERFILL_SENSOR21_LENGTH_CASE   0xAF
#define OVERFILL_SENSOR22_LENGTH_CASE   0xB0
#define OVERFILL_SENSOR23_LENGTH_CASE   0xB1
#define OVERFILL_SENSOR24_LENGTH_CASE   0xB2
#define LENGTH_UNITS_CASE               0xB3

/******************************* 6/15/2009 2:51PM ****************************
 * Super TIM memory entries sizes
 *****************************************************************************/
#ifdef NO_DEF
#define VALID_ENTRIES_SIZE          2
#define REV_OF_TABLE_SIZE           2
#define VALID_TRUCKTEST_TIM_SIZE    1
#define TRUCK_TEST_TIM_NUMBER_SIZE  6
#define VALID_ALT_TIM_SIZE          1
#define TRUCK_ALT_TIM_NUMBER_SIZE   6
#define BUILDER_NAME_SIZE           40
#define BUILDER_ADDRESS_SIZE        70
#define SERIAL_NUMBER_SIZE          10
#define VIN_SIZE                    20
#define BUILD_DATE_SIZE             11
#define GVW_UNIT_SIZE               1
#define GVW_SIZE                    4
#define INTELLICHECK_P_SIZE         1
#define SENSOR_TYPE_SIZE            1
#define NUMBER_COMPARTMENTS_SIZE    1
#define COMPARTMENT_UNIT_SIZE       1
#define COMPARTMENT_1_SIZE          4
#define COMPARTMENT_2_SIZE          4
#define COMPARTMENT_3_SIZE          4
#define COMPARTMENT_4_SIZE          4
#define COMPARTMENT_5_SIZE          4
#define COMPARTMENT_6_SIZE          4
#define COMPARTMENT_7_SIZE          4
#define COMPARTMENT_8_SIZE          4
#define COMPARTMENT_9_SIZE          4
#define COMPARTMENT_10_SIZE         4
#define COMPARTMENT_11_SIZE         4
#define COMPARTMENT_12_SIZE         4
#define COMPARTMENT_13_SIZE         4
#define COMPARTMENT_14_SIZE         4
#define COMPARTMENT_15_SIZE         4
#define COMPARTMENT_16_SIZE         4
#define NO_RELAY_SIZE               1
#define SCULLY_EQUIPMENT_SIZE       1
#define FUEL_TYPE_1_SIZE            2
#define FUEL_TYPE_2_SIZE            2
#define FUEL_TYPE_3_SIZE            2
#define FUEL_TYPE_4_SIZE            2
#define FUEL_TYPE_5_SIZE            2
#define FUEL_TYPE_6_SIZE            2
#define FUEL_TYPE_7_SIZE            2
#define FUEL_TYPE_8_SIZE            2
#define FUEL_TYPE_9_SIZE            2
#define FUEL_TYPE_10_SIZE           2
#define FUEL_TYPE_11_SIZE           2
#define FUEL_TYPE_12_SIZE           2
#define FUEL_TYPE_13_SIZE           2
#define FUEL_TYPE_14_SIZE           2
#define FUEL_TYPE_15_SIZE           2
#define FUEL_TYPE_16_SIZE           2
#endif

#define CARRIER_NAME_SIZE           20
#define CARRIER_ADDRESS_SIZE        40
#define CONTRACT_NUMBER_SIZE        10
#define OPPERATING_SERVICE_SIZE     20
#define DRIVER_ID_SIZE              10
#define ALLOWABLE_VOL_CPT1_SIZE     2
#define ALLOWABLE_VOL_CPT2_SIZE     2
#define ALLOWABLE_VOL_CPT3_SIZE     2
#define ALLOWABLE_VOL_CPT4_SIZE     2
#define ALLOWABLE_VOL_CPT5_SIZE     2
#define ALLOWABLE_VOL_CPT6_SIZE     2
#define ALLOWABLE_VOL_CPT7_SIZE     2
#define ALLOWABLE_VOL_CPT8_SIZE     2
#define ALLOWABLE_VOL_CPT9_SIZE     2
#define ALLOWABLE_VOL_CPT10_SIZE    2
#define ALLOWABLE_VOL_CPT11_SIZE    2
#define ALLOWABLE_VOL_CPT12_SIZE    2
#define ALLOWABLE_VOL_CPT13_SIZE    2
#define ALLOWABLE_VOL_CPT14_SIZE    2
#define ALLOWABLE_VOL_CPT15_SIZE    2
#define ALLOWABLE_VOL_CPT16_SIZE    2

#define VAP_TIGHT_CERT_TYPE_SIZE        1
#define VAP_TIGHT_CERT_DATE_SIZE        3
#define VAP_TIGHT_CERT_NUMBER_SIZE      20
#define SAFE_PASS_CERT_TYPE_SIZE        1
#define SAFE_PASS_CERT_DATE_SIZE        3
#define SAFE_PASS_CERT_NUMBER_SIZE      20
#define CERT_3_TYPE_SIZE                1
#define CERT_3_DATE_SIZE                3
#define CERT_3_NUMBER_SIZE              20
#define CERT_4_TYPE_SIZE                1
#define CERT_4_DATE_SIZE                3
#define CERT_4_NUMBER_SIZE              20
#define CERT_5_TYPE_SIZE                1
#define CERT_5_DATE_SIZE                3
#define CERT_5_NUMBER_SIZE              20

#define TABLE_VALID_SIZE                2
#define TABLE_REVISION_SIZE             2
#define ALT_TIM_ID_VALID_SIZE           1
#define ALT_TIM_ID_SIZE                 6
#define NUMBER_OF_COMPARTMENTS_SIZE     1
#define COMPARTMENT_VOLUME_UNITS_SIZE   1
#define TRAILER_ID_NUMBER_SIZE          20
#define COMPARTMENT_CONFIG_SIZE         1
#define VAPOR_INTERLOCK_TYPE_SIZE       1
#define CPT1_TYPES_ALLOWED_SIZE         3
#define CPT2_TYPES_ALLOWED_SIZE         3
#define CPT3_TYPES_ALLOWED_SIZE         3
#define CPT4_TYPES_ALLOWED_SIZE         3
#define CPT5_TYPES_ALLOWED_SIZE         3
#define CPT6_TYPES_ALLOWED_SIZE         3
#define CPT7_TYPES_ALLOWED_SIZE         3
#define CPT8_TYPES_ALLOWED_SIZE         3
#define CPT9_TYPES_ALLOWED_SIZE         3
#define CPT10_TYPES_ALLOWED_SIZE        3
#define CPT11_TYPES_ALLOWED_SIZE        3
#define CPT12_TYPES_ALLOWED_SIZE        3
#define CPT13_TYPES_ALLOWED_SIZE        3
#define CPT14_TYPES_ALLOWED_SIZE        3
#define CPT15_TYPES_ALLOWED_SIZE        3
#define CPT16_TYPES_ALLOWED_SIZE        3
#define MAX_LOADING_TEMP_SIZE           1
#define TEMPERATURE_UNITS_SIZE          1

#define CPT1_TYPE_LOADED_SIZE           3
#define CPT1_BATCH_ID_LOADED_SIZE       3
#define CPT1_VOLUME_LOADED_SIZE         2
#define CPT2_TYPE_LOADED_SIZE           3
#define CPT2_BATCH_ID_LOADED_SIZE       3
#define CPT2_VOLUME_LOADED_SIZE         2
#define CPT3_TYPE_LOADED_SIZE           3
#define CPT3_BATCH_ID_LOADED_SIZE       3
#define CPT3_VOLUME_LOADED_SIZE         2
#define CPT4_TYPE_LOADED_SIZE           3
#define CPT4_BATCH_ID_LOADED_SIZE       3
#define CPT4_VOLUME_LOADED_SIZE         2
#define CPT5_TYPE_LOADED_SIZE           3
#define CPT5_BATCH_ID_LOADED_SIZE       3
#define CPT5_VOLUME_LOADED_SIZE         2
#define CPT6_TYPE_LOADED_SIZE           3
#define CPT6_BATCH_ID_LOADED_SIZE       3
#define CPT6_VOLUME_LOADED_SIZE         2
#define CPT7_TYPE_LOADED_SIZE           3
#define CPT7_BATCH_ID_LOADED_SIZE       3
#define CPT7_VOLUME_LOADED_SIZE         2
#define CPT8_TYPE_LOADED_SIZE           3
#define CPT8_BATCH_ID_LOADED_SIZE       3
#define CPT8_VOLUME_LOADED_SIZE         2
#define CPT9_TYPE_LOADED_SIZE           3
#define CPT9_BATCH_ID_LOADED_SIZE       3
#define CPT9_VOLUME_LOADED_SIZE         2
#define CPT10_TYPE_LOADED_SIZE          3
#define CPT10_BATCH_ID_LOADED_SIZE      3
#define CPT10_VOLUME_LOADED_SIZE        2
#define CPT11_TYPE_LOADED_SIZE          3
#define CPT11_BATCH_ID_LOADED_SIZE      3
#define CPT11_VOLUME_LOADED_SIZE        2
#define CPT12_TYPE_LOADED_SIZE          3
#define CPT12_BATCH_ID_LOADED_SIZE      3
#define CPT12_VOLUME_LOADED_SIZE        2
#define CPT13_TYPE_LOADED_SIZE          3
#define CPT13_BATCH_ID_LOADED_SIZE      3
#define CPT13_VOLUME_LOADED_SIZE        2
#define CPT14_TYPE_LOADED_SIZE          3
#define CPT14_BATCH_ID_LOADED_SIZE      3
#define CPT14_VOLUME_LOADED_SIZE        2
#define CPT15_TYPE_LOADED_SIZE          3
#define CPT15_BATCH_ID_LOADED_SIZE      3
#define CPT15_VOLUME_LOADED_SIZE        2
#define CPT16_TYPE_LOADED_SIZE          3
#define CPT16_BATCH_ID_LOADED_SIZE      3
#define CPT16_VOLUME_LOADED_SIZE        2
#define TERMINAL_NAME_SIZE              20
#define TERMINAL_ADDRESS_SIZE           40
#define TERMINAL_GANTRY_NUMBER_SIZE     1

#define FAULT_LOG1_SIZE                 6
#define FAULT_LOG2_SIZE                 6
#define FAULT_LOG3_SIZE                 6
#define FAULT_LOG4_SIZE                 6
#define FAULT_LOG5_SIZE                 6
#define SERVICE_CENTER_NAME_SIZE        20
#define SERVICE_CENTER_ADDRESS_SIZE     40

#define BUILDER_NAME_SIZE               20
#define BUILDER_ADDRESS_SIZE            40
#define TRUCK_SERIAL_NUMBER_SIZE        10
#define TRUCK_VIN_SIZE                  20
#define TRUCK_BUILD_DATE_SIZE           3
#define TRUCK_WEIGHT_UNITS_SIZE         1
#define TRUCK_GVW_SIZE                  4
#define INTELLICHECK_TYPE_SIZE          1
#define OVERFILL_SENSOR_TYPE_SIZE       1
#define RETAINED_SENSOR_TYPE_SIZE       1
#define CPT1_BUILD_VOLUME_SIZE          2
#define CPT2_BUILD_VOLUME_SIZE          2
#define CPT3_BUILD_VOLUME_SIZE          2
#define CPT4_BUILD_VOLUME_SIZE          2
#define CPT5_BUILD_VOLUME_SIZE          2
#define CPT6_BUILD_VOLUME_SIZE          2
#define CPT7_BUILD_VOLUME_SIZE          2
#define CPT8_BUILD_VOLUME_SIZE          2
#define CPT9_BUILD_VOLUME_SIZE          2
#define CPT10_BUILD_VOLUME_SIZE         2
#define CPT11_BUILD_VOLUME_SIZE         2
#define CPT12_BUILD_VOLUME_SIZE         2
#define CPT13_BUILD_VOLUME_SIZE         2
#define CPT14_BUILD_VOLUME_SIZE         2
#define CPT15_BUILD_VOLUME_SIZE         2
#define CPT16_BUILD_VOLUME_SIZE         2
#define SCULLY_SENSORS_SIZE             1
#define TANK_MODEL_NUMBER_SIZE          20
#define MAX_WORKING_PRESSURE_SIZE       2
#define ALLOWABLE_WORKING_PRESSURE_SIZE 2
#define PRESSURE_UNITS_SIZE             1
#define BULKHEADS_SIZE                  1
#define TANK_PROFILE_SIZE               20
#define OVERFILL_SENSOR1_LENGTH_SIZE    1
#define OVERFILL_SENSOR2_LENGTH_SIZE    1
#define OVERFILL_SENSOR3_LENGTH_SIZE    1
#define OVERFILL_SENSOR4_LENGTH_SIZE    1
#define OVERFILL_SENSOR5_LENGTH_SIZE    1
#define OVERFILL_SENSOR6_LENGTH_SIZE    1
#define OVERFILL_SENSOR7_LENGTH_SIZE    1
#define OVERFILL_SENSOR8_LENGTH_SIZE    1
#define OVERFILL_SENSOR9_LENGTH_SIZE    1
#define OVERFILL_SENSOR10_LENGTH_SIZE   1
#define OVERFILL_SENSOR11_LENGTH_SIZE   1
#define OVERFILL_SENSOR12_LENGTH_SIZE   1
#define OVERFILL_SENSOR13_LENGTH_SIZE   1
#define OVERFILL_SENSOR14_LENGTH_SIZE   1
#define OVERFILL_SENSOR15_LENGTH_SIZE   1
#define OVERFILL_SENSOR16_LENGTH_SIZE   1
#define OVERFILL_SENSOR17_LENGTH_SIZE   1
#define OVERFILL_SENSOR18_LENGTH_SIZE   1
#define OVERFILL_SENSOR19_LENGTH_SIZE   1
#define OVERFILL_SENSOR20_LENGTH_SIZE   1
#define OVERFILL_SENSOR21_LENGTH_SIZE   1
#define OVERFILL_SENSOR22_LENGTH_SIZE   1
#define OVERFILL_SENSOR23_LENGTH_SIZE   1
#define OVERFILL_SENSOR24_LENGTH_SIZE   1
#define LENGTH_UNITS_SIZE               1



#define MAX_COMPARTMENTS            16  /*  */
/******************************* 6/22/2009 1:24PM ****************************
 * TIM type
 *****************************************************************************/
#define TEST_TIM  2
#define ALT_TIM   3

#define TIM_VALID 0x33
#endif                    /* TIM_UTL_H */


