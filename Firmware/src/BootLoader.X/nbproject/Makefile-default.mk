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
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/BootLoader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/BootLoader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=LoadLatch.s bootloaderMain.c FlashFS.c SystemConfiguration.c circularPrintF.c FIFO.c fletcherChecksum_1.c ESP_Flash.c pack.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/LoadLatch.o ${OBJECTDIR}/bootloaderMain.o ${OBJECTDIR}/FlashFS.o ${OBJECTDIR}/SystemConfiguration.o ${OBJECTDIR}/circularPrintF.o ${OBJECTDIR}/FIFO.o ${OBJECTDIR}/fletcherChecksum_1.o ${OBJECTDIR}/ESP_Flash.o ${OBJECTDIR}/pack.o
POSSIBLE_DEPFILES=${OBJECTDIR}/LoadLatch.o.d ${OBJECTDIR}/bootloaderMain.o.d ${OBJECTDIR}/FlashFS.o.d ${OBJECTDIR}/SystemConfiguration.o.d ${OBJECTDIR}/circularPrintF.o.d ${OBJECTDIR}/FIFO.o.d ${OBJECTDIR}/fletcherChecksum_1.o.d ${OBJECTDIR}/ESP_Flash.o.d ${OBJECTDIR}/pack.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/LoadLatch.o ${OBJECTDIR}/bootloaderMain.o ${OBJECTDIR}/FlashFS.o ${OBJECTDIR}/SystemConfiguration.o ${OBJECTDIR}/circularPrintF.o ${OBJECTDIR}/FIFO.o ${OBJECTDIR}/fletcherChecksum_1.o ${OBJECTDIR}/ESP_Flash.o ${OBJECTDIR}/pack.o

# Source Files
SOURCEFILES=LoadLatch.s bootloaderMain.c FlashFS.c SystemConfiguration.c circularPrintF.c FIFO.c fletcherChecksum_1.c ESP_Flash.c pack.c


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

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/BootLoader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP256MC204
MP_LINKER_FILE_OPTION=,--script="BootLoader_p33EP256MC204.gld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/LoadLatch.o: LoadLatch.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/LoadLatch.o.d 
	@${RM} ${OBJECTDIR}/LoadLatch.o.ok ${OBJECTDIR}/LoadLatch.o.err 
	@${RM} ${OBJECTDIR}/LoadLatch.o 
	@${FIXDEPS} "${OBJECTDIR}/LoadLatch.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_AS} $(MP_EXTRA_AS_PRE)  LoadLatch.s -o ${OBJECTDIR}/LoadLatch.o -omf=elf -p=$(MP_PROCESSOR_OPTION) --defsym=__MPLAB_BUILD=1 --defsym=__MPLAB_DEBUG=1 --defsym=__ICD2RAM=1 --defsym=__DEBUG=1 --defsym=__MPLAB_DEBUGGER_PK3=1 -g  -MD "${OBJECTDIR}/LoadLatch.o.d"$(MP_EXTRA_AS_POST)
	
else
${OBJECTDIR}/LoadLatch.o: LoadLatch.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/LoadLatch.o.d 
	@${RM} ${OBJECTDIR}/LoadLatch.o.ok ${OBJECTDIR}/LoadLatch.o.err 
	@${RM} ${OBJECTDIR}/LoadLatch.o 
	@${FIXDEPS} "${OBJECTDIR}/LoadLatch.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_AS} $(MP_EXTRA_AS_PRE)  LoadLatch.s -o ${OBJECTDIR}/LoadLatch.o -omf=elf -p=$(MP_PROCESSOR_OPTION) --defsym=__MPLAB_BUILD=1 -g  -MD "${OBJECTDIR}/LoadLatch.o.d"$(MP_EXTRA_AS_POST)
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/bootloaderMain.o: bootloaderMain.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/bootloaderMain.o.d 
	@${RM} ${OBJECTDIR}/bootloaderMain.o.ok ${OBJECTDIR}/bootloaderMain.o.err 
	@${RM} ${OBJECTDIR}/bootloaderMain.o 
	@${FIXDEPS} "${OBJECTDIR}/bootloaderMain.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/bootloaderMain.o.d" -o ${OBJECTDIR}/bootloaderMain.o bootloaderMain.c    
	
${OBJECTDIR}/FlashFS.o: FlashFS.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/FlashFS.o.d 
	@${RM} ${OBJECTDIR}/FlashFS.o.ok ${OBJECTDIR}/FlashFS.o.err 
	@${RM} ${OBJECTDIR}/FlashFS.o 
	@${FIXDEPS} "${OBJECTDIR}/FlashFS.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/FlashFS.o.d" -o ${OBJECTDIR}/FlashFS.o FlashFS.c    
	
${OBJECTDIR}/SystemConfiguration.o: SystemConfiguration.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/SystemConfiguration.o.d 
	@${RM} ${OBJECTDIR}/SystemConfiguration.o.ok ${OBJECTDIR}/SystemConfiguration.o.err 
	@${RM} ${OBJECTDIR}/SystemConfiguration.o 
	@${FIXDEPS} "${OBJECTDIR}/SystemConfiguration.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/SystemConfiguration.o.d" -o ${OBJECTDIR}/SystemConfiguration.o SystemConfiguration.c    
	
${OBJECTDIR}/circularPrintF.o: circularPrintF.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/circularPrintF.o.d 
	@${RM} ${OBJECTDIR}/circularPrintF.o.ok ${OBJECTDIR}/circularPrintF.o.err 
	@${RM} ${OBJECTDIR}/circularPrintF.o 
	@${FIXDEPS} "${OBJECTDIR}/circularPrintF.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/circularPrintF.o.d" -o ${OBJECTDIR}/circularPrintF.o circularPrintF.c    
	
${OBJECTDIR}/FIFO.o: FIFO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/FIFO.o.d 
	@${RM} ${OBJECTDIR}/FIFO.o.ok ${OBJECTDIR}/FIFO.o.err 
	@${RM} ${OBJECTDIR}/FIFO.o 
	@${FIXDEPS} "${OBJECTDIR}/FIFO.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/FIFO.o.d" -o ${OBJECTDIR}/FIFO.o FIFO.c    
	
${OBJECTDIR}/fletcherChecksum_1.o: fletcherChecksum_1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/fletcherChecksum_1.o.d 
	@${RM} ${OBJECTDIR}/fletcherChecksum_1.o.ok ${OBJECTDIR}/fletcherChecksum_1.o.err 
	@${RM} ${OBJECTDIR}/fletcherChecksum_1.o 
	@${FIXDEPS} "${OBJECTDIR}/fletcherChecksum_1.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/fletcherChecksum_1.o.d" -o ${OBJECTDIR}/fletcherChecksum_1.o fletcherChecksum_1.c    
	
${OBJECTDIR}/ESP_Flash.o: ESP_Flash.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/ESP_Flash.o.d 
	@${RM} ${OBJECTDIR}/ESP_Flash.o.ok ${OBJECTDIR}/ESP_Flash.o.err 
	@${RM} ${OBJECTDIR}/ESP_Flash.o 
	@${FIXDEPS} "${OBJECTDIR}/ESP_Flash.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/ESP_Flash.o.d" -o ${OBJECTDIR}/ESP_Flash.o ESP_Flash.c    
	
${OBJECTDIR}/pack.o: pack.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pack.o.d 
	@${RM} ${OBJECTDIR}/pack.o.ok ${OBJECTDIR}/pack.o.err 
	@${RM} ${OBJECTDIR}/pack.o 
	@${FIXDEPS} "${OBJECTDIR}/pack.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/pack.o.d" -o ${OBJECTDIR}/pack.o pack.c    
	
else
${OBJECTDIR}/bootloaderMain.o: bootloaderMain.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/bootloaderMain.o.d 
	@${RM} ${OBJECTDIR}/bootloaderMain.o.ok ${OBJECTDIR}/bootloaderMain.o.err 
	@${RM} ${OBJECTDIR}/bootloaderMain.o 
	@${FIXDEPS} "${OBJECTDIR}/bootloaderMain.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/bootloaderMain.o.d" -o ${OBJECTDIR}/bootloaderMain.o bootloaderMain.c    
	
${OBJECTDIR}/FlashFS.o: FlashFS.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/FlashFS.o.d 
	@${RM} ${OBJECTDIR}/FlashFS.o.ok ${OBJECTDIR}/FlashFS.o.err 
	@${RM} ${OBJECTDIR}/FlashFS.o 
	@${FIXDEPS} "${OBJECTDIR}/FlashFS.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/FlashFS.o.d" -o ${OBJECTDIR}/FlashFS.o FlashFS.c    
	
${OBJECTDIR}/SystemConfiguration.o: SystemConfiguration.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/SystemConfiguration.o.d 
	@${RM} ${OBJECTDIR}/SystemConfiguration.o.ok ${OBJECTDIR}/SystemConfiguration.o.err 
	@${RM} ${OBJECTDIR}/SystemConfiguration.o 
	@${FIXDEPS} "${OBJECTDIR}/SystemConfiguration.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/SystemConfiguration.o.d" -o ${OBJECTDIR}/SystemConfiguration.o SystemConfiguration.c    
	
${OBJECTDIR}/circularPrintF.o: circularPrintF.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/circularPrintF.o.d 
	@${RM} ${OBJECTDIR}/circularPrintF.o.ok ${OBJECTDIR}/circularPrintF.o.err 
	@${RM} ${OBJECTDIR}/circularPrintF.o 
	@${FIXDEPS} "${OBJECTDIR}/circularPrintF.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/circularPrintF.o.d" -o ${OBJECTDIR}/circularPrintF.o circularPrintF.c    
	
${OBJECTDIR}/FIFO.o: FIFO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/FIFO.o.d 
	@${RM} ${OBJECTDIR}/FIFO.o.ok ${OBJECTDIR}/FIFO.o.err 
	@${RM} ${OBJECTDIR}/FIFO.o 
	@${FIXDEPS} "${OBJECTDIR}/FIFO.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/FIFO.o.d" -o ${OBJECTDIR}/FIFO.o FIFO.c    
	
${OBJECTDIR}/fletcherChecksum_1.o: fletcherChecksum_1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/fletcherChecksum_1.o.d 
	@${RM} ${OBJECTDIR}/fletcherChecksum_1.o.ok ${OBJECTDIR}/fletcherChecksum_1.o.err 
	@${RM} ${OBJECTDIR}/fletcherChecksum_1.o 
	@${FIXDEPS} "${OBJECTDIR}/fletcherChecksum_1.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/fletcherChecksum_1.o.d" -o ${OBJECTDIR}/fletcherChecksum_1.o fletcherChecksum_1.c    
	
${OBJECTDIR}/ESP_Flash.o: ESP_Flash.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/ESP_Flash.o.d 
	@${RM} ${OBJECTDIR}/ESP_Flash.o.ok ${OBJECTDIR}/ESP_Flash.o.err 
	@${RM} ${OBJECTDIR}/ESP_Flash.o 
	@${FIXDEPS} "${OBJECTDIR}/ESP_Flash.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/ESP_Flash.o.d" -o ${OBJECTDIR}/ESP_Flash.o ESP_Flash.c    
	
${OBJECTDIR}/pack.o: pack.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pack.o.d 
	@${RM} ${OBJECTDIR}/pack.o.ok ${OBJECTDIR}/pack.o.err 
	@${RM} ${OBJECTDIR}/pack.o 
	@${FIXDEPS} "${OBJECTDIR}/pack.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -MMD -MF "${OBJECTDIR}/pack.o.d" -o ${OBJECTDIR}/pack.o pack.c    
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/BootLoader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    BootLoader_p33EP256MC204.gld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -o dist/${CND_CONF}/${IMAGE_TYPE}/BootLoader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}         -Wl,--defsym=__MPLAB_BUILD=1,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map"$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1
else
dist/${CND_CONF}/${IMAGE_TYPE}/BootLoader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   BootLoader_p33EP256MC204.gld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf -mcpu=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/BootLoader.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}         -Wl,--defsym=__MPLAB_BUILD=1,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map"$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION)
	${MP_CC_DIR}\\pic30-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/BootLoader.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -omf=elf
endif


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
