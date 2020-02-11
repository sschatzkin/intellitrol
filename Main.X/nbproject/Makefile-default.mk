#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/Main.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/Main.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS
SUB_IMAGE_ADDRESS_COMMAND=--image-address $(SUB_IMAGE_ADDRESS)
else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../source/write.c ../source/adc.c ../source/com_two.c ../source/comdat.c ../source/dallas.c ../source/delay.c ../source/dfile.c ../source/dumfile.c ../source/eeprom.c ../source/esquared.c ../source/grndchck.c ../source/CPURegisterTest.s ../source/PcTest.c ../source/i2c_1.c ../source/i2c_2.c ../source/init_ADC.c ../source/init_DMA.c ../source/init_ports.c ../source/init_timer.c ../source/isr_ADC.c ../source/isr_DMA.c ../source/isr_timer.c ../source/jumpers.c ../source/modbus.c ../source/modcmd.c ../source/modfrc.c ../source/modreg.c ../source/nvsystem.c ../source/nvtruck.c ../source/optic2.c ../source/optic5.c ../source/permit.c ../source/pod.c ../source/printout.c ../source/shorts.c ../source/sim.c ../source/specops.c ../source/thermist.c ../source/traps.c ../source/trukstat.c ../Tools/build_date.c ../source/tim_utl.c ../source/spi_mpol.c ../source/spi_eeprom.c ../source/memory.s ../source/uart.c ../source/isr_uart.c ../source/march_tst.c ../source/null_signature.s ../source/crt0_standard.s ../source/memory_test.s ../source/test_function.c ../source/main.c ../source/deadman.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/812168374/write.o ${OBJECTDIR}/_ext/812168374/adc.o ${OBJECTDIR}/_ext/812168374/com_two.o ${OBJECTDIR}/_ext/812168374/comdat.o ${OBJECTDIR}/_ext/812168374/dallas.o ${OBJECTDIR}/_ext/812168374/delay.o ${OBJECTDIR}/_ext/812168374/dfile.o ${OBJECTDIR}/_ext/812168374/dumfile.o ${OBJECTDIR}/_ext/812168374/eeprom.o ${OBJECTDIR}/_ext/812168374/esquared.o ${OBJECTDIR}/_ext/812168374/grndchck.o ${OBJECTDIR}/_ext/812168374/CPURegisterTest.o ${OBJECTDIR}/_ext/812168374/PcTest.o ${OBJECTDIR}/_ext/812168374/i2c_1.o ${OBJECTDIR}/_ext/812168374/i2c_2.o ${OBJECTDIR}/_ext/812168374/init_ADC.o ${OBJECTDIR}/_ext/812168374/init_DMA.o ${OBJECTDIR}/_ext/812168374/init_ports.o ${OBJECTDIR}/_ext/812168374/init_timer.o ${OBJECTDIR}/_ext/812168374/isr_ADC.o ${OBJECTDIR}/_ext/812168374/isr_DMA.o ${OBJECTDIR}/_ext/812168374/isr_timer.o ${OBJECTDIR}/_ext/812168374/jumpers.o ${OBJECTDIR}/_ext/812168374/modbus.o ${OBJECTDIR}/_ext/812168374/modcmd.o ${OBJECTDIR}/_ext/812168374/modfrc.o ${OBJECTDIR}/_ext/812168374/modreg.o ${OBJECTDIR}/_ext/812168374/nvsystem.o ${OBJECTDIR}/_ext/812168374/nvtruck.o ${OBJECTDIR}/_ext/812168374/optic2.o ${OBJECTDIR}/_ext/812168374/optic5.o ${OBJECTDIR}/_ext/812168374/permit.o ${OBJECTDIR}/_ext/812168374/pod.o ${OBJECTDIR}/_ext/812168374/printout.o ${OBJECTDIR}/_ext/812168374/shorts.o ${OBJECTDIR}/_ext/812168374/sim.o ${OBJECTDIR}/_ext/812168374/specops.o ${OBJECTDIR}/_ext/812168374/thermist.o ${OBJECTDIR}/_ext/812168374/traps.o ${OBJECTDIR}/_ext/812168374/trukstat.o ${OBJECTDIR}/_ext/2133044052/build_date.o ${OBJECTDIR}/_ext/812168374/tim_utl.o ${OBJECTDIR}/_ext/812168374/spi_mpol.o ${OBJECTDIR}/_ext/812168374/spi_eeprom.o ${OBJECTDIR}/_ext/812168374/memory.o ${OBJECTDIR}/_ext/812168374/uart.o ${OBJECTDIR}/_ext/812168374/isr_uart.o ${OBJECTDIR}/_ext/812168374/march_tst.o ${OBJECTDIR}/_ext/812168374/null_signature.o ${OBJECTDIR}/_ext/812168374/crt0_standard.o ${OBJECTDIR}/_ext/812168374/memory_test.o ${OBJECTDIR}/_ext/812168374/test_function.o ${OBJECTDIR}/_ext/812168374/main.o ${OBJECTDIR}/_ext/812168374/deadman.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/812168374/write.o.d ${OBJECTDIR}/_ext/812168374/adc.o.d ${OBJECTDIR}/_ext/812168374/com_two.o.d ${OBJECTDIR}/_ext/812168374/comdat.o.d ${OBJECTDIR}/_ext/812168374/dallas.o.d ${OBJECTDIR}/_ext/812168374/delay.o.d ${OBJECTDIR}/_ext/812168374/dfile.o.d ${OBJECTDIR}/_ext/812168374/dumfile.o.d ${OBJECTDIR}/_ext/812168374/eeprom.o.d ${OBJECTDIR}/_ext/812168374/esquared.o.d ${OBJECTDIR}/_ext/812168374/grndchck.o.d ${OBJECTDIR}/_ext/812168374/CPURegisterTest.o.d ${OBJECTDIR}/_ext/812168374/PcTest.o.d ${OBJECTDIR}/_ext/812168374/i2c_1.o.d ${OBJECTDIR}/_ext/812168374/i2c_2.o.d ${OBJECTDIR}/_ext/812168374/init_ADC.o.d ${OBJECTDIR}/_ext/812168374/init_DMA.o.d ${OBJECTDIR}/_ext/812168374/init_ports.o.d ${OBJECTDIR}/_ext/812168374/init_timer.o.d ${OBJECTDIR}/_ext/812168374/isr_ADC.o.d ${OBJECTDIR}/_ext/812168374/isr_DMA.o.d ${OBJECTDIR}/_ext/812168374/isr_timer.o.d ${OBJECTDIR}/_ext/812168374/jumpers.o.d ${OBJECTDIR}/_ext/812168374/modbus.o.d ${OBJECTDIR}/_ext/812168374/modcmd.o.d ${OBJECTDIR}/_ext/812168374/modfrc.o.d ${OBJECTDIR}/_ext/812168374/modreg.o.d ${OBJECTDIR}/_ext/812168374/nvsystem.o.d ${OBJECTDIR}/_ext/812168374/nvtruck.o.d ${OBJECTDIR}/_ext/812168374/optic2.o.d ${OBJECTDIR}/_ext/812168374/optic5.o.d ${OBJECTDIR}/_ext/812168374/permit.o.d ${OBJECTDIR}/_ext/812168374/pod.o.d ${OBJECTDIR}/_ext/812168374/printout.o.d ${OBJECTDIR}/_ext/812168374/shorts.o.d ${OBJECTDIR}/_ext/812168374/sim.o.d ${OBJECTDIR}/_ext/812168374/specops.o.d ${OBJECTDIR}/_ext/812168374/thermist.o.d ${OBJECTDIR}/_ext/812168374/traps.o.d ${OBJECTDIR}/_ext/812168374/trukstat.o.d ${OBJECTDIR}/_ext/2133044052/build_date.o.d ${OBJECTDIR}/_ext/812168374/tim_utl.o.d ${OBJECTDIR}/_ext/812168374/spi_mpol.o.d ${OBJECTDIR}/_ext/812168374/spi_eeprom.o.d ${OBJECTDIR}/_ext/812168374/memory.o.d ${OBJECTDIR}/_ext/812168374/uart.o.d ${OBJECTDIR}/_ext/812168374/isr_uart.o.d ${OBJECTDIR}/_ext/812168374/march_tst.o.d ${OBJECTDIR}/_ext/812168374/null_signature.o.d ${OBJECTDIR}/_ext/812168374/crt0_standard.o.d ${OBJECTDIR}/_ext/812168374/memory_test.o.d ${OBJECTDIR}/_ext/812168374/test_function.o.d ${OBJECTDIR}/_ext/812168374/main.o.d ${OBJECTDIR}/_ext/812168374/deadman.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/812168374/write.o ${OBJECTDIR}/_ext/812168374/adc.o ${OBJECTDIR}/_ext/812168374/com_two.o ${OBJECTDIR}/_ext/812168374/comdat.o ${OBJECTDIR}/_ext/812168374/dallas.o ${OBJECTDIR}/_ext/812168374/delay.o ${OBJECTDIR}/_ext/812168374/dfile.o ${OBJECTDIR}/_ext/812168374/dumfile.o ${OBJECTDIR}/_ext/812168374/eeprom.o ${OBJECTDIR}/_ext/812168374/esquared.o ${OBJECTDIR}/_ext/812168374/grndchck.o ${OBJECTDIR}/_ext/812168374/CPURegisterTest.o ${OBJECTDIR}/_ext/812168374/PcTest.o ${OBJECTDIR}/_ext/812168374/i2c_1.o ${OBJECTDIR}/_ext/812168374/i2c_2.o ${OBJECTDIR}/_ext/812168374/init_ADC.o ${OBJECTDIR}/_ext/812168374/init_DMA.o ${OBJECTDIR}/_ext/812168374/init_ports.o ${OBJECTDIR}/_ext/812168374/init_timer.o ${OBJECTDIR}/_ext/812168374/isr_ADC.o ${OBJECTDIR}/_ext/812168374/isr_DMA.o ${OBJECTDIR}/_ext/812168374/isr_timer.o ${OBJECTDIR}/_ext/812168374/jumpers.o ${OBJECTDIR}/_ext/812168374/modbus.o ${OBJECTDIR}/_ext/812168374/modcmd.o ${OBJECTDIR}/_ext/812168374/modfrc.o ${OBJECTDIR}/_ext/812168374/modreg.o ${OBJECTDIR}/_ext/812168374/nvsystem.o ${OBJECTDIR}/_ext/812168374/nvtruck.o ${OBJECTDIR}/_ext/812168374/optic2.o ${OBJECTDIR}/_ext/812168374/optic5.o ${OBJECTDIR}/_ext/812168374/permit.o ${OBJECTDIR}/_ext/812168374/pod.o ${OBJECTDIR}/_ext/812168374/printout.o ${OBJECTDIR}/_ext/812168374/shorts.o ${OBJECTDIR}/_ext/812168374/sim.o ${OBJECTDIR}/_ext/812168374/specops.o ${OBJECTDIR}/_ext/812168374/thermist.o ${OBJECTDIR}/_ext/812168374/traps.o ${OBJECTDIR}/_ext/812168374/trukstat.o ${OBJECTDIR}/_ext/2133044052/build_date.o ${OBJECTDIR}/_ext/812168374/tim_utl.o ${OBJECTDIR}/_ext/812168374/spi_mpol.o ${OBJECTDIR}/_ext/812168374/spi_eeprom.o ${OBJECTDIR}/_ext/812168374/memory.o ${OBJECTDIR}/_ext/812168374/uart.o ${OBJECTDIR}/_ext/812168374/isr_uart.o ${OBJECTDIR}/_ext/812168374/march_tst.o ${OBJECTDIR}/_ext/812168374/null_signature.o ${OBJECTDIR}/_ext/812168374/crt0_standard.o ${OBJECTDIR}/_ext/812168374/memory_test.o ${OBJECTDIR}/_ext/812168374/test_function.o ${OBJECTDIR}/_ext/812168374/main.o ${OBJECTDIR}/_ext/812168374/deadman.o

# Source Files
SOURCEFILES=../source/write.c ../source/adc.c ../source/com_two.c ../source/comdat.c ../source/dallas.c ../source/delay.c ../source/dfile.c ../source/dumfile.c ../source/eeprom.c ../source/esquared.c ../source/grndchck.c ../source/CPURegisterTest.s ../source/PcTest.c ../source/i2c_1.c ../source/i2c_2.c ../source/init_ADC.c ../source/init_DMA.c ../source/init_ports.c ../source/init_timer.c ../source/isr_ADC.c ../source/isr_DMA.c ../source/isr_timer.c ../source/jumpers.c ../source/modbus.c ../source/modcmd.c ../source/modfrc.c ../source/modreg.c ../source/nvsystem.c ../source/nvtruck.c ../source/optic2.c ../source/optic5.c ../source/permit.c ../source/pod.c ../source/printout.c ../source/shorts.c ../source/sim.c ../source/specops.c ../source/thermist.c ../source/traps.c ../source/trukstat.c ../Tools/build_date.c ../source/tim_utl.c ../source/spi_mpol.c ../source/spi_eeprom.c ../source/memory.s ../source/uart.c ../source/isr_uart.c ../source/march_tst.c ../source/null_signature.s ../source/crt0_standard.s ../source/memory_test.s ../source/test_function.c ../source/main.c ../source/deadman.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

# The following macros may be used in the pre and post step lines
Device=PIC24HJ256GP210
ProjectDir="C:\work\software\intellatrolGITHUB\intellitrol\Main.X"
ProjectName=Itrol_Main
ConfName=default
ImagePath="dist\default\${IMAGE_TYPE}\Main.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}"
ImageDir="dist\default\${IMAGE_TYPE}"
ImageName="Main.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}"
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IsDebug="true"
else
IsDebug="false"
endif

.build-conf:  .pre ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/Main.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
	@echo "--------------------------------------"
	@echo "User defined post-build step: []"
	@
	@echo "--------------------------------------"

MP_PROCESSOR_OPTION=24HJ256GP210
MP_LINKER_FILE_OPTION=,--script="..\gld\intellitrol.gld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/812168374/write.o: ../source/write.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/write.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/write.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/write.c  -o ${OBJECTDIR}/_ext/812168374/write.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/write.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/write.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/adc.o: ../source/adc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/adc.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/adc.c  -o ${OBJECTDIR}/_ext/812168374/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/adc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/adc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/com_two.o: ../source/com_two.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/com_two.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/com_two.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/com_two.c  -o ${OBJECTDIR}/_ext/812168374/com_two.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/com_two.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/com_two.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/comdat.o: ../source/comdat.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/comdat.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/comdat.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/comdat.c  -o ${OBJECTDIR}/_ext/812168374/comdat.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/comdat.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/comdat.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/dallas.o: ../source/dallas.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/dallas.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/dallas.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/dallas.c  -o ${OBJECTDIR}/_ext/812168374/dallas.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/dallas.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/dallas.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/delay.o: ../source/delay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/delay.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/delay.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/delay.c  -o ${OBJECTDIR}/_ext/812168374/delay.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/delay.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/delay.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/dfile.o: ../source/dfile.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/dfile.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/dfile.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/dfile.c  -o ${OBJECTDIR}/_ext/812168374/dfile.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/dfile.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/dfile.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/dumfile.o: ../source/dumfile.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/dumfile.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/dumfile.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/dumfile.c  -o ${OBJECTDIR}/_ext/812168374/dumfile.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/dumfile.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/dumfile.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/eeprom.o: ../source/eeprom.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/eeprom.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/eeprom.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/eeprom.c  -o ${OBJECTDIR}/_ext/812168374/eeprom.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/eeprom.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/eeprom.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/esquared.o: ../source/esquared.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/esquared.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/esquared.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/esquared.c  -o ${OBJECTDIR}/_ext/812168374/esquared.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/esquared.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/esquared.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/grndchck.o: ../source/grndchck.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/grndchck.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/grndchck.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/grndchck.c  -o ${OBJECTDIR}/_ext/812168374/grndchck.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/grndchck.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/grndchck.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/PcTest.o: ../source/PcTest.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/PcTest.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/PcTest.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/PcTest.c  -o ${OBJECTDIR}/_ext/812168374/PcTest.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/PcTest.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/PcTest.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/i2c_1.o: ../source/i2c_1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/i2c_1.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/i2c_1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/i2c_1.c  -o ${OBJECTDIR}/_ext/812168374/i2c_1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/i2c_1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/i2c_1.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/i2c_2.o: ../source/i2c_2.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/i2c_2.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/i2c_2.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/i2c_2.c  -o ${OBJECTDIR}/_ext/812168374/i2c_2.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/i2c_2.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/i2c_2.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/init_ADC.o: ../source/init_ADC.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_ADC.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_ADC.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/init_ADC.c  -o ${OBJECTDIR}/_ext/812168374/init_ADC.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/init_ADC.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/init_ADC.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/init_DMA.o: ../source/init_DMA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_DMA.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_DMA.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/init_DMA.c  -o ${OBJECTDIR}/_ext/812168374/init_DMA.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/init_DMA.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/init_DMA.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/init_ports.o: ../source/init_ports.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_ports.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_ports.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/init_ports.c  -o ${OBJECTDIR}/_ext/812168374/init_ports.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/init_ports.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/init_ports.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/init_timer.o: ../source/init_timer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_timer.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_timer.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/init_timer.c  -o ${OBJECTDIR}/_ext/812168374/init_timer.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/init_timer.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/init_timer.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/isr_ADC.o: ../source/isr_ADC.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_ADC.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_ADC.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/isr_ADC.c  -o ${OBJECTDIR}/_ext/812168374/isr_ADC.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/isr_ADC.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/isr_ADC.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/isr_DMA.o: ../source/isr_DMA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_DMA.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_DMA.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/isr_DMA.c  -o ${OBJECTDIR}/_ext/812168374/isr_DMA.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/isr_DMA.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/isr_DMA.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/isr_timer.o: ../source/isr_timer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_timer.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_timer.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/isr_timer.c  -o ${OBJECTDIR}/_ext/812168374/isr_timer.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/isr_timer.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/isr_timer.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/jumpers.o: ../source/jumpers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/jumpers.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/jumpers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/jumpers.c  -o ${OBJECTDIR}/_ext/812168374/jumpers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/jumpers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/jumpers.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/modbus.o: ../source/modbus.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/modbus.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/modbus.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/modbus.c  -o ${OBJECTDIR}/_ext/812168374/modbus.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/modbus.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/modbus.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/modcmd.o: ../source/modcmd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/modcmd.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/modcmd.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/modcmd.c  -o ${OBJECTDIR}/_ext/812168374/modcmd.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/modcmd.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/modcmd.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/modfrc.o: ../source/modfrc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/modfrc.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/modfrc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/modfrc.c  -o ${OBJECTDIR}/_ext/812168374/modfrc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/modfrc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/modfrc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/modreg.o: ../source/modreg.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/modreg.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/modreg.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/modreg.c  -o ${OBJECTDIR}/_ext/812168374/modreg.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/modreg.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/modreg.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/nvsystem.o: ../source/nvsystem.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/nvsystem.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/nvsystem.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/nvsystem.c  -o ${OBJECTDIR}/_ext/812168374/nvsystem.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/nvsystem.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/nvsystem.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/nvtruck.o: ../source/nvtruck.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/nvtruck.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/nvtruck.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/nvtruck.c  -o ${OBJECTDIR}/_ext/812168374/nvtruck.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/nvtruck.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/nvtruck.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/optic2.o: ../source/optic2.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/optic2.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/optic2.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/optic2.c  -o ${OBJECTDIR}/_ext/812168374/optic2.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/optic2.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/optic2.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/optic5.o: ../source/optic5.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/optic5.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/optic5.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/optic5.c  -o ${OBJECTDIR}/_ext/812168374/optic5.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/optic5.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/optic5.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/permit.o: ../source/permit.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/permit.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/permit.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/permit.c  -o ${OBJECTDIR}/_ext/812168374/permit.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/permit.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/permit.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/pod.o: ../source/pod.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/pod.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/pod.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/pod.c  -o ${OBJECTDIR}/_ext/812168374/pod.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/pod.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/pod.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/printout.o: ../source/printout.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/printout.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/printout.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/printout.c  -o ${OBJECTDIR}/_ext/812168374/printout.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/printout.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/printout.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/shorts.o: ../source/shorts.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/shorts.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/shorts.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/shorts.c  -o ${OBJECTDIR}/_ext/812168374/shorts.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/shorts.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/shorts.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/sim.o: ../source/sim.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/sim.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/sim.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/sim.c  -o ${OBJECTDIR}/_ext/812168374/sim.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/sim.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/sim.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/specops.o: ../source/specops.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/specops.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/specops.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/specops.c  -o ${OBJECTDIR}/_ext/812168374/specops.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/specops.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/specops.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/thermist.o: ../source/thermist.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/thermist.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/thermist.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/thermist.c  -o ${OBJECTDIR}/_ext/812168374/thermist.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/thermist.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/thermist.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/traps.o: ../source/traps.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/traps.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/traps.c  -o ${OBJECTDIR}/_ext/812168374/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/traps.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/trukstat.o: ../source/trukstat.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/trukstat.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/trukstat.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/trukstat.c  -o ${OBJECTDIR}/_ext/812168374/trukstat.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/trukstat.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/trukstat.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/2133044052/build_date.o: ../Tools/build_date.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2133044052" 
	@${RM} ${OBJECTDIR}/_ext/2133044052/build_date.o.d 
	@${RM} ${OBJECTDIR}/_ext/2133044052/build_date.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Tools/build_date.c  -o ${OBJECTDIR}/_ext/2133044052/build_date.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/2133044052/build_date.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/2133044052/build_date.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/tim_utl.o: ../source/tim_utl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/tim_utl.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/tim_utl.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/tim_utl.c  -o ${OBJECTDIR}/_ext/812168374/tim_utl.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/tim_utl.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/tim_utl.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/spi_mpol.o: ../source/spi_mpol.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/spi_mpol.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/spi_mpol.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/spi_mpol.c  -o ${OBJECTDIR}/_ext/812168374/spi_mpol.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/spi_mpol.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/spi_mpol.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/spi_eeprom.o: ../source/spi_eeprom.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/spi_eeprom.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/spi_eeprom.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/spi_eeprom.c  -o ${OBJECTDIR}/_ext/812168374/spi_eeprom.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/spi_eeprom.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/spi_eeprom.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/uart.o: ../source/uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/uart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/uart.c  -o ${OBJECTDIR}/_ext/812168374/uart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/uart.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/uart.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/isr_uart.o: ../source/isr_uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_uart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/isr_uart.c  -o ${OBJECTDIR}/_ext/812168374/isr_uart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/isr_uart.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/isr_uart.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/march_tst.o: ../source/march_tst.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/march_tst.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/march_tst.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/march_tst.c  -o ${OBJECTDIR}/_ext/812168374/march_tst.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/march_tst.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/march_tst.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/test_function.o: ../source/test_function.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/test_function.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/test_function.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/test_function.c  -o ${OBJECTDIR}/_ext/812168374/test_function.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/test_function.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/test_function.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/main.o: ../source/main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/main.c  -o ${OBJECTDIR}/_ext/812168374/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/main.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/deadman.o: ../source/deadman.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/deadman.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/deadman.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/deadman.c  -o ${OBJECTDIR}/_ext/812168374/deadman.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/deadman.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/deadman.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
else
${OBJECTDIR}/_ext/812168374/write.o: ../source/write.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/write.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/write.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/write.c  -o ${OBJECTDIR}/_ext/812168374/write.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/write.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/write.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/adc.o: ../source/adc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/adc.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/adc.c  -o ${OBJECTDIR}/_ext/812168374/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/adc.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/adc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/com_two.o: ../source/com_two.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/com_two.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/com_two.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/com_two.c  -o ${OBJECTDIR}/_ext/812168374/com_two.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/com_two.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/com_two.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/comdat.o: ../source/comdat.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/comdat.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/comdat.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/comdat.c  -o ${OBJECTDIR}/_ext/812168374/comdat.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/comdat.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/comdat.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/dallas.o: ../source/dallas.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/dallas.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/dallas.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/dallas.c  -o ${OBJECTDIR}/_ext/812168374/dallas.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/dallas.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/dallas.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/delay.o: ../source/delay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/delay.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/delay.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/delay.c  -o ${OBJECTDIR}/_ext/812168374/delay.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/delay.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/delay.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/dfile.o: ../source/dfile.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/dfile.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/dfile.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/dfile.c  -o ${OBJECTDIR}/_ext/812168374/dfile.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/dfile.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/dfile.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/dumfile.o: ../source/dumfile.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/dumfile.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/dumfile.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/dumfile.c  -o ${OBJECTDIR}/_ext/812168374/dumfile.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/dumfile.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/dumfile.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/eeprom.o: ../source/eeprom.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/eeprom.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/eeprom.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/eeprom.c  -o ${OBJECTDIR}/_ext/812168374/eeprom.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/eeprom.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/eeprom.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/esquared.o: ../source/esquared.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/esquared.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/esquared.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/esquared.c  -o ${OBJECTDIR}/_ext/812168374/esquared.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/esquared.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/esquared.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/grndchck.o: ../source/grndchck.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/grndchck.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/grndchck.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/grndchck.c  -o ${OBJECTDIR}/_ext/812168374/grndchck.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/grndchck.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/grndchck.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/PcTest.o: ../source/PcTest.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/PcTest.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/PcTest.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/PcTest.c  -o ${OBJECTDIR}/_ext/812168374/PcTest.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/PcTest.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/PcTest.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/i2c_1.o: ../source/i2c_1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/i2c_1.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/i2c_1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/i2c_1.c  -o ${OBJECTDIR}/_ext/812168374/i2c_1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/i2c_1.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/i2c_1.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/i2c_2.o: ../source/i2c_2.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/i2c_2.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/i2c_2.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/i2c_2.c  -o ${OBJECTDIR}/_ext/812168374/i2c_2.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/i2c_2.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/i2c_2.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/init_ADC.o: ../source/init_ADC.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_ADC.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_ADC.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/init_ADC.c  -o ${OBJECTDIR}/_ext/812168374/init_ADC.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/init_ADC.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/init_ADC.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/init_DMA.o: ../source/init_DMA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_DMA.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_DMA.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/init_DMA.c  -o ${OBJECTDIR}/_ext/812168374/init_DMA.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/init_DMA.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/init_DMA.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/init_ports.o: ../source/init_ports.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_ports.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_ports.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/init_ports.c  -o ${OBJECTDIR}/_ext/812168374/init_ports.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/init_ports.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/init_ports.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/init_timer.o: ../source/init_timer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_timer.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/init_timer.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/init_timer.c  -o ${OBJECTDIR}/_ext/812168374/init_timer.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/init_timer.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/init_timer.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/isr_ADC.o: ../source/isr_ADC.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_ADC.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_ADC.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/isr_ADC.c  -o ${OBJECTDIR}/_ext/812168374/isr_ADC.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/isr_ADC.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/isr_ADC.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/isr_DMA.o: ../source/isr_DMA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_DMA.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_DMA.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/isr_DMA.c  -o ${OBJECTDIR}/_ext/812168374/isr_DMA.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/isr_DMA.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/isr_DMA.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/isr_timer.o: ../source/isr_timer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_timer.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_timer.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/isr_timer.c  -o ${OBJECTDIR}/_ext/812168374/isr_timer.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/isr_timer.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/isr_timer.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/jumpers.o: ../source/jumpers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/jumpers.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/jumpers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/jumpers.c  -o ${OBJECTDIR}/_ext/812168374/jumpers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/jumpers.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/jumpers.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/modbus.o: ../source/modbus.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/modbus.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/modbus.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/modbus.c  -o ${OBJECTDIR}/_ext/812168374/modbus.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/modbus.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/modbus.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/modcmd.o: ../source/modcmd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/modcmd.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/modcmd.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/modcmd.c  -o ${OBJECTDIR}/_ext/812168374/modcmd.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/modcmd.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/modcmd.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/modfrc.o: ../source/modfrc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/modfrc.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/modfrc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/modfrc.c  -o ${OBJECTDIR}/_ext/812168374/modfrc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/modfrc.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/modfrc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/modreg.o: ../source/modreg.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/modreg.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/modreg.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/modreg.c  -o ${OBJECTDIR}/_ext/812168374/modreg.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/modreg.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/modreg.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/nvsystem.o: ../source/nvsystem.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/nvsystem.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/nvsystem.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/nvsystem.c  -o ${OBJECTDIR}/_ext/812168374/nvsystem.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/nvsystem.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/nvsystem.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/nvtruck.o: ../source/nvtruck.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/nvtruck.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/nvtruck.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/nvtruck.c  -o ${OBJECTDIR}/_ext/812168374/nvtruck.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/nvtruck.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/nvtruck.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/optic2.o: ../source/optic2.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/optic2.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/optic2.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/optic2.c  -o ${OBJECTDIR}/_ext/812168374/optic2.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/optic2.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/optic2.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/optic5.o: ../source/optic5.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/optic5.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/optic5.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/optic5.c  -o ${OBJECTDIR}/_ext/812168374/optic5.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/optic5.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/optic5.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/permit.o: ../source/permit.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/permit.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/permit.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/permit.c  -o ${OBJECTDIR}/_ext/812168374/permit.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/permit.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/permit.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/pod.o: ../source/pod.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/pod.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/pod.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/pod.c  -o ${OBJECTDIR}/_ext/812168374/pod.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/pod.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/pod.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/printout.o: ../source/printout.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/printout.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/printout.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/printout.c  -o ${OBJECTDIR}/_ext/812168374/printout.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/printout.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/printout.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/shorts.o: ../source/shorts.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/shorts.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/shorts.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/shorts.c  -o ${OBJECTDIR}/_ext/812168374/shorts.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/shorts.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/shorts.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/sim.o: ../source/sim.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/sim.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/sim.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/sim.c  -o ${OBJECTDIR}/_ext/812168374/sim.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/sim.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/sim.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/specops.o: ../source/specops.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/specops.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/specops.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/specops.c  -o ${OBJECTDIR}/_ext/812168374/specops.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/specops.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/specops.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/thermist.o: ../source/thermist.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/thermist.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/thermist.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/thermist.c  -o ${OBJECTDIR}/_ext/812168374/thermist.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/thermist.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/thermist.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/traps.o: ../source/traps.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/traps.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/traps.c  -o ${OBJECTDIR}/_ext/812168374/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/traps.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/traps.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/trukstat.o: ../source/trukstat.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/trukstat.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/trukstat.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/trukstat.c  -o ${OBJECTDIR}/_ext/812168374/trukstat.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/trukstat.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/trukstat.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/2133044052/build_date.o: ../Tools/build_date.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2133044052" 
	@${RM} ${OBJECTDIR}/_ext/2133044052/build_date.o.d 
	@${RM} ${OBJECTDIR}/_ext/2133044052/build_date.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Tools/build_date.c  -o ${OBJECTDIR}/_ext/2133044052/build_date.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/2133044052/build_date.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/2133044052/build_date.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/tim_utl.o: ../source/tim_utl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/tim_utl.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/tim_utl.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/tim_utl.c  -o ${OBJECTDIR}/_ext/812168374/tim_utl.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/tim_utl.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/tim_utl.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/spi_mpol.o: ../source/spi_mpol.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/spi_mpol.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/spi_mpol.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/spi_mpol.c  -o ${OBJECTDIR}/_ext/812168374/spi_mpol.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/spi_mpol.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/spi_mpol.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/spi_eeprom.o: ../source/spi_eeprom.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/spi_eeprom.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/spi_eeprom.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/spi_eeprom.c  -o ${OBJECTDIR}/_ext/812168374/spi_eeprom.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/spi_eeprom.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/spi_eeprom.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/uart.o: ../source/uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/uart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/uart.c  -o ${OBJECTDIR}/_ext/812168374/uart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/uart.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/uart.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/isr_uart.o: ../source/isr_uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/isr_uart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/isr_uart.c  -o ${OBJECTDIR}/_ext/812168374/isr_uart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/isr_uart.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/isr_uart.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/march_tst.o: ../source/march_tst.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/march_tst.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/march_tst.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/march_tst.c  -o ${OBJECTDIR}/_ext/812168374/march_tst.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/march_tst.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/march_tst.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/test_function.o: ../source/test_function.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/test_function.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/test_function.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/test_function.c  -o ${OBJECTDIR}/_ext/812168374/test_function.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/test_function.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/test_function.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/main.o: ../source/main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/main.c  -o ${OBJECTDIR}/_ext/812168374/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/main.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/main.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/812168374/deadman.o: ../source/deadman.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/deadman.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/deadman.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../source/deadman.c  -o ${OBJECTDIR}/_ext/812168374/deadman.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/812168374/deadman.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -mlarge-code -mlarge-data -mlarge-scalar -mconst-in-code -O0 -I"../inc" -I"../h" -I"." -msmart-io=1 -Werror -Wall -msfr-warn=off   -fno-schedule-insns -fno-schedule-insns2  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/deadman.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/812168374/CPURegisterTest.o: ../source/CPURegisterTest.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/CPURegisterTest.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/CPURegisterTest.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  ../source/CPURegisterTest.s  -o ${OBJECTDIR}/_ext/812168374/CPURegisterTest.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -I"../inc" -I"." -Wa,-MD,"${OBJECTDIR}/_ext/812168374/CPURegisterTest.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD4=1,-g,--no-relax,-g$(MP_EXTRA_AS_POST)  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/CPURegisterTest.o.d"  $(SILENT)  -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/812168374/memory.o: ../source/memory.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/memory.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  ../source/memory.s  -o ${OBJECTDIR}/_ext/812168374/memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -I"../inc" -I"." -Wa,-MD,"${OBJECTDIR}/_ext/812168374/memory.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD4=1,-g,--no-relax,-g$(MP_EXTRA_AS_POST)  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/memory.o.d"  $(SILENT)  -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/812168374/null_signature.o: ../source/null_signature.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/null_signature.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/null_signature.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  ../source/null_signature.s  -o ${OBJECTDIR}/_ext/812168374/null_signature.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -I"../inc" -I"." -Wa,-MD,"${OBJECTDIR}/_ext/812168374/null_signature.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD4=1,-g,--no-relax,-g$(MP_EXTRA_AS_POST)  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/null_signature.o.d"  $(SILENT)  -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/812168374/crt0_standard.o: ../source/crt0_standard.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/crt0_standard.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/crt0_standard.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  ../source/crt0_standard.s  -o ${OBJECTDIR}/_ext/812168374/crt0_standard.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -I"../inc" -I"." -Wa,-MD,"${OBJECTDIR}/_ext/812168374/crt0_standard.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD4=1,-g,--no-relax,-g$(MP_EXTRA_AS_POST)  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/crt0_standard.o.d"  $(SILENT)  -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/812168374/memory_test.o: ../source/memory_test.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/memory_test.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/memory_test.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  ../source/memory_test.s  -o ${OBJECTDIR}/_ext/812168374/memory_test.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -I"../inc" -I"." -Wa,-MD,"${OBJECTDIR}/_ext/812168374/memory_test.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD4=1,-g,--no-relax,-g$(MP_EXTRA_AS_POST)  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/memory_test.o.d"  $(SILENT)  -rsi ${MP_CC_DIR}../  
	
else
${OBJECTDIR}/_ext/812168374/CPURegisterTest.o: ../source/CPURegisterTest.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/CPURegisterTest.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/CPURegisterTest.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  ../source/CPURegisterTest.s  -o ${OBJECTDIR}/_ext/812168374/CPURegisterTest.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -I"../inc" -I"." -Wa,-MD,"${OBJECTDIR}/_ext/812168374/CPURegisterTest.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax,-g$(MP_EXTRA_AS_POST)  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/CPURegisterTest.o.d"  $(SILENT)  -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/812168374/memory.o: ../source/memory.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/memory.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  ../source/memory.s  -o ${OBJECTDIR}/_ext/812168374/memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -I"../inc" -I"." -Wa,-MD,"${OBJECTDIR}/_ext/812168374/memory.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax,-g$(MP_EXTRA_AS_POST)  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/memory.o.d"  $(SILENT)  -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/812168374/null_signature.o: ../source/null_signature.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/null_signature.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/null_signature.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  ../source/null_signature.s  -o ${OBJECTDIR}/_ext/812168374/null_signature.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -I"../inc" -I"." -Wa,-MD,"${OBJECTDIR}/_ext/812168374/null_signature.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax,-g$(MP_EXTRA_AS_POST)  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/null_signature.o.d"  $(SILENT)  -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/812168374/crt0_standard.o: ../source/crt0_standard.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/crt0_standard.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/crt0_standard.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  ../source/crt0_standard.s  -o ${OBJECTDIR}/_ext/812168374/crt0_standard.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -I"../inc" -I"." -Wa,-MD,"${OBJECTDIR}/_ext/812168374/crt0_standard.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax,-g$(MP_EXTRA_AS_POST)  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/crt0_standard.o.d"  $(SILENT)  -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/812168374/memory_test.o: ../source/memory_test.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/812168374" 
	@${RM} ${OBJECTDIR}/_ext/812168374/memory_test.o.d 
	@${RM} ${OBJECTDIR}/_ext/812168374/memory_test.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  ../source/memory_test.s  -o ${OBJECTDIR}/_ext/812168374/memory_test.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -I"../inc" -I"." -Wa,-MD,"${OBJECTDIR}/_ext/812168374/memory_test.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax,-g$(MP_EXTRA_AS_POST)  -mdfp=${DFP_DIR}/xc16
	@${FIXDEPS} "${OBJECTDIR}/_ext/812168374/memory_test.o.d"  $(SILENT)  -rsi ${MP_CC_DIR}../  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/Main.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    ../gld/intellitrol.gld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/Main.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include"  -mreserve=data@0x800:0x81F -mreserve=data@0x820:0x821 -mreserve=data@0x822:0x823 -mreserve=data@0x824:0x825 -mreserve=data@0x826:0x84F   -Wl,,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_ICD4=1,$(MP_LINKER_FILE_OPTION),--heap=1024,--stack=1024,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--library-path="C:/Program Files/Microchip/MPLAB C30/lib",--library-path="C:/Program Files/Microchip/MPLAB C30/lib/PIC24H",--library-path="C:/Program Files/Microchip/mplabc30/v3.31/lib",--library-path="C:/Program Files/Microchip/mplabc30/v3.31/lib/PIC24H",--library-path="../gld",--library-path=".",--no-force-link,--smart-io,-Map="${DISTDIR}/Main.X.${IMAGE_TYPE}.map",--report-mem,--warn-section-align,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp=${DFP_DIR}/xc16 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/Main.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   ../gld/intellitrol.gld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/Main.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"C:/Program Files (x86)/Microchip/xc16/v1.26/support/generic/h" -I"C:/Program Files (x86)/Microchip/xc16/v1.26/include" -Wl,,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--heap=1024,--stack=1024,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--library-path="C:/Program Files/Microchip/MPLAB C30/lib",--library-path="C:/Program Files/Microchip/MPLAB C30/lib/PIC24H",--library-path="C:/Program Files/Microchip/mplabc30/v3.31/lib",--library-path="C:/Program Files/Microchip/mplabc30/v3.31/lib/PIC24H",--library-path="../gld",--library-path=".",--no-force-link,--smart-io,-Map="${DISTDIR}/Main.X.${IMAGE_TYPE}.map",--report-mem,--warn-section-align,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp=${DFP_DIR}/xc16 
	${MP_CC_DIR}\\xc16-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/Main.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp=${DFP_DIR}/xc16 
	
endif

.pre:
	@echo "--------------------------------------"
	@echo "User defined pre-build step: [..\Tools\build_date.exe]"
	@..\Tools\build_date.exe
	@echo "--------------------------------------"

# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
