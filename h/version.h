/*******************************************************************************
 *
 *   Project:        Rack Controller Intellitrol
 *
 *   Module:         version.h
 *
 *   Revision:       REV 1.6.35
 *
 *   Author:         Ken Langlais (KLL); edits by Dave Paquette (DHP)
 *
 *                   @Copyright 2009, 2016  Scully Signal Company
 *
 *   Description:    Version specification
 *
 *   Revision History:
 *
 *   Rev    Date    Who   Description of Change Made
 *  -----  -------  ---  --------------------------------------------
 * 1.5.00  04/22/08  KLL  Started porting old Intellitrol Modbus code from the
 *                         MC68HC16Y1 CPU on the original Intellitrol
 * 1.5.21  04/04/12  KLL  Changed Super TIM fuel type from byte to 16 bit word
 * 1.5.25  07/16/12  KLL  Fix shorts test with thermistors
 * 1.5.26  10/09/12  KLL  Added security.
 *                        Changed the CRC storage location to 0x2ABF0 which is a
 *                         start of the last page in program memory. With
 *                         security on, writing to the last program address
 *                         causes address traps because the Microchip builtins
 *                         that allow the writing into program memory actually
 *                         go beyond the address being updating.
 * 1.5.27  10/19/12  KLL  Changed open probe timeout from 19 seconds to 1 minute
 *                        For the short_6to4_5to3() in is_truck_pulsing() to work
 *                         properly probe_try_state must be set to NO_TYPE. So
 *                         is_truck_pulsing saves the probe_try_state before
 *                         running the test and restores it when completed.
 *                         If PULSE5VOLT is set the Probe 4 will force that
 *                         channel low. Make sure this signal is turned off when
 *                         testing for truck present.
 *                        With the Jump Start voltage limit jumpers in the open
 *                          probe voltage and the voltage reading from the
 *                          Intellicheck with the AUX open fakes the firmware to
 *                          believe the truck has left. But the Channel 5 voltage
 *                          reference test will fail. So when the limit jumpers
 *                          are in a Channel 5 voltage reference test is performed.
 *                          If it fails the firmware assumes the truck is present.
 *         03/24/14  DHP  Incorporate non-SIL Intellitrol v 1.0.27 / 28 changes
 *                         1)Reduce deadman switch response check from 1.1s to 0.2s
 *                         2)Corrected handling of 1st time shorts test failure
 *                           so permit is allowed if later tests are successful.
 *                         3)Stop shorts tests after 10 failures.
 *                         4)Remove cause of secondary, unwanted pulse in 
 *                           ground output.
 *                         5)Fixed reset problem (in EU) with resistive ground.
 *                         6)Increased Probe Short and Probe Power levels to
 *                           correctly identify connection of certain devices.
 *         03/25/14  DHP  Removed DS28CM00 references, this chip is not used.
 *                        Remove Dallas/Maxim 1-wire family code checking for
 *                         TIMs.
 *                        Added TIM related variables.
 * 1.5.28  05/13/14  DHP  New batch of boards have ACK/NACK errors on I2C2 IO
 *                          expander. Slowing BAUD rate helps.
 *                        The errors showed that even after a failure to
 *                         initialize the IO expander the jumper values being
 *                         read, which come through the expander, were used.
 *                         Changed this so that I2C2 failures on initialize are
 *                         fatal.  Also added attempt at error recovery via
 *                         retries and changed fetch_jumpers() to return value
 *                         indicating all jumpers out if error on reading.
 *                        Changed OpenI2C2() call to use variables rather than
 *                         absolute values.
 * 1.5.29  06/24/14  DHP  Program module download had several issues which were
 *                         caused by the module programming.  Many print 
 *                         statements and L_VerifyPMzero() were added to aid
 *                         debug. Restructured much of the updater code in main.
 *
 * 1.5.30  07/10/14  DHP  Auto detect ground as resistive or diode if "gnd bolt"
 *                         jumper is open.
 *                        Add code to support both Intellitrol2 and Intellitrol
 *                         PIC with the option being set via the Modbus features
 *                         enable command.
 *                        Reverse the default versus option sense of TB3 pins 5 
 *                         and 6. Intellitrol2 default is "good Ground" and
 *                         option is "Truck Here"and the reverse for Intellitrol.
 *                        Remove all code for vapor; Ground Bolt detected now
 *                         owns what was the vapor LED.
 *                        Remove checks for ground enable being an option - all
 *                         units are sold with ground proving. Ground can still
 *                         be disabled with jumper.
 * 1.5.31  01/12/15  DHP  New boards have ACK/NACK errors on I2C2 IO expander. 
 *                         Again! Changed baud from 1MHz (beyond capabilities of
 *                         PIC) to 400kHz. Expander supports only 400KHz and
 *                         1.7MHz and not the 1MHz apparently.
 *                        In 1st steps towards a working loader and Intellitrol2
 *                         support the following changes were made:
 *                         -- The loader script sets the loader section at
 *                           0xC00 - 0x2000 to allow for ICD and loader growth.
 *                         -- The loader files were all moved or copied to their
 *                            own folders and named L_*.*
 * 1.6.32  03/10/15  DHP  Removed loader code from this program and set addresses
 *                         for main(), error_flag, rcon, etc.
 *                        The following changes were made to correct the handling
 *                         of the MUX for AN0/1 analog signals; signals other 
 *                         than pvolt were being placed into probes_array and 
 *                         used as pvolt.
 *                         -- In wait_for_probes() changed while() wait to 
 *                            pre-decrement timeout to allow following
 *                            "if (timeout == 0)" to catch timeout.
 *                         -- In ops_ADC() added braces to include turning off DMA in else
 *                         -- In  read_ADC(), enabled DMA when wait_for_probes() fails
 *                         -- In two_wire_start() removed read_ADC()
 *                         -- In Init_DMA0() added set_mux(M_PROBES)
 *                        The following changes were made to satisfy beta
 *                         request for better ground fault annunciation and for
 *                         indication of 5-wire compartment count.
 *                         -- In report_tank_state() when no ground and Intellitrol2; 
 *                            turn on Ground error LED, turn off Ground LED.
 *                         -- In display_probe() added display of compartment count.
 *                         -- In optic_5_setup(() added check for consistent
 *                             and stable number_of_Probes count.
 * 1.6.33  10/07/15  DHP  Merged with loader; updated loader error codes  
 *                        Corrected nvLogInit() for size increase over Motorola
 *                        Changed check_diag() 
 *                        Corrected check_5wire_fault for IntelliChecks
 *                        Changed scully_probe() call to check_5wire() to not
 *                         check or 2-wire to prevent confusion on wet 5-wire.
 * 1.6.34  08/15/16  DHP  QCCC 53: Modified code to support new hardware
 *                         circuitry to turn off high current to sensors.  
 *                         High current is only enabled (in new code) for
 *                         thermistor sensors. hardass.h, comdat.h, comdat.c, 
 *                         adc.c, com_two.c, dumfile.c and init_ports.c modified.
 *                        FogBugz 108: Connects of non-permitting 5-wire
 *                         IntelliChecks and their changing states from wet to
 *                         dry was handled incorrectly.  Added dry_once variable
 *                         and if never dry than restart trying to determine 
 *                         sensor type.
 *                        FogBugz 111: The register 7B option did not turn on 
 *                         the VIP standby LED when no truck was present.
 *                         Corrected flash_panel() and report_tank_state().
 *                        FogBugz 128: Modified pctest.c and test_function.c 
 *                         routines to allow for XC16 complier which requires
 *                         use of built-in to obtain function address
 *                         rather than the & operator.
 *                        FogBugz 129: QCCC) 52 Investigation showed that when 
 *                         pin 9 of the vehicle connection (TB4) is shorted to
 *                         ground with no other connection the short was 
 *                         ignored. Modified grndchk.c check_ground_present().
 *                        FogBugz 131: A CRC error of EEPROM resulted in the 
 *                         default values for EEPROM being used. Added code to
 *                         handle EEPROM errors as fatal and log errors. Front
 *                         panel will show service LED and compartments 7 and 8
 *                         blinking.
 *                        FogBugz 134: For consistency with Motorola CPUs 
 *                         changed GND_REF_DEFAULT from 2k to near 2.3k ohms.
 *                        FogBugz 136: To aid in debug added messages in 
 *                         truck_idle() to indicate what flagged a connection.
 *                        FogBugz 137: In dumfile.c modified init of probes_state[], 
 *                         and modified dry_probes() to only change state on 
 *                         number_of_Probes not max.
 * E.6.34  11/22/16  DHP  Compiled with MPLAB X and XC16; no code changes
 * D.6.35  02/07/17  DHP  FogBugz 146: Implement an active deadman function
 *                         Changed setting of baddeadman to always be TRUE or 
 *                         FALSE rather than the misleading OPEN/CLOSE.
 *                         Changed baddeadman_close to Permit_OK for clarity.
 *                         Created file deadman.c and included a rewritten 
 *                         deadman_ops() to handle the active deadman function.
 *                         Created deadman_init() and in truck_idle() when truck
 *                         is detected replaced call to deadman_ops() with this.
 *                         Removed calls to deadman_ops() in truck_idle() when
 *                         no connection and in truck_gone().
 *                        FogBugz 147: Factory options erased with code reload. 
 *                         Removed offending code in eeUpdateSys().
 *                        FogBugz 148: Co-processor message lost on initial
 *                         connection to 5-wire vehicle.
 *                         Increased the wait time in check_bk().
 *                        Other:
 *                         Changed new EEPROM parameters in eeInit() to accept
 *                         a 0 in 1st nibble of device.
 *                        FogBugz 146 revisited:  Changed check_bk() to use a
 *                         timer rather than counts to determine wait time. This
 *                         so the permit OK messaging for bypass and deadman 
 *                         could be better handled.  The call for a new message
 *                         is now immediate rather than off a flag which was
 *                         only being looked at once per second.
 * 1.6.36  07/14/17  DHP  QCCC: Change RAW13VOLT_HI from 17000 to 20000 to 
 *                         ignore raw 13v failures caused by Scotland providing 
 *                         260v to the Intellitrol.  Raw 13v is a measurement of 
 *                         the line voltage and its measurement is deemed to be
 *                         meaningless by electrical engineering.
*******************************************************************************/
#ifndef VERSION_H
#define VERSION_H

/* Shell version identification */

#define MAJVER      01
#define MINVER      06
#define EDTVER      38
#define EDTVERHEX   0x38

#define SHELLVER (((unsigned int)MAJVER << 12) | ((unsigned int)MINVER << 8) | (unsigned int)EDTVERHEX)

#define HWD_MAJVER  0x1
#define HWD_MINVER  0x5
#define HWD_EDTVER  0x00

#define HWDVER (/*lint -e{835} */((unsigned int)HWD_MAJVER << 12) | ((unsigned int)HWD_MINVER << 8) | (unsigned int)HWD_EDTVER)

/*********************  End of VERSION.H  *********************************/
#endif      /* end of VERSION_H */
