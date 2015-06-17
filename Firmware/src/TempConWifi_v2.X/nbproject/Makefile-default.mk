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
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/TempConWifi_v2.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/TempConWifi_v2.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=OneWire.c ESP8266.c rtc.c uPnP.c mDNS.c FIFO.c circularPrintF.c Base64.c MD5.c pack.c fletcherChecksum.c GOTHtraps.c geterrorloc.s FlashFS.c PID.c IIR.c RLE_Compressor.c BlobFS.c Http_API.c Settings.c SystemConfiguration.c http_server.c main.c TemperatureControler.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/OneWire.o ${OBJECTDIR}/ESP8266.o ${OBJECTDIR}/rtc.o ${OBJECTDIR}/uPnP.o ${OBJECTDIR}/mDNS.o ${OBJECTDIR}/FIFO.o ${OBJECTDIR}/circularPrintF.o ${OBJECTDIR}/Base64.o ${OBJECTDIR}/MD5.o ${OBJECTDIR}/pack.o ${OBJECTDIR}/fletcherChecksum.o ${OBJECTDIR}/GOTHtraps.o ${OBJECTDIR}/geterrorloc.o ${OBJECTDIR}/FlashFS.o ${OBJECTDIR}/PID.o ${OBJECTDIR}/IIR.o ${OBJECTDIR}/RLE_Compressor.o ${OBJECTDIR}/BlobFS.o ${OBJECTDIR}/Http_API.o ${OBJECTDIR}/Settings.o ${OBJECTDIR}/SystemConfiguration.o ${OBJECTDIR}/http_server.o ${OBJECTDIR}/main.o ${OBJECTDIR}/TemperatureControler.o
POSSIBLE_DEPFILES=${OBJECTDIR}/OneWire.o.d ${OBJECTDIR}/ESP8266.o.d ${OBJECTDIR}/rtc.o.d ${OBJECTDIR}/uPnP.o.d ${OBJECTDIR}/mDNS.o.d ${OBJECTDIR}/FIFO.o.d ${OBJECTDIR}/circularPrintF.o.d ${OBJECTDIR}/Base64.o.d ${OBJECTDIR}/MD5.o.d ${OBJECTDIR}/pack.o.d ${OBJECTDIR}/fletcherChecksum.o.d ${OBJECTDIR}/GOTHtraps.o.d ${OBJECTDIR}/geterrorloc.o.d ${OBJECTDIR}/FlashFS.o.d ${OBJECTDIR}/PID.o.d ${OBJECTDIR}/IIR.o.d ${OBJECTDIR}/RLE_Compressor.o.d ${OBJECTDIR}/BlobFS.o.d ${OBJECTDIR}/Http_API.o.d ${OBJECTDIR}/Settings.o.d ${OBJECTDIR}/SystemConfiguration.o.d ${OBJECTDIR}/http_server.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/TemperatureControler.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/OneWire.o ${OBJECTDIR}/ESP8266.o ${OBJECTDIR}/rtc.o ${OBJECTDIR}/uPnP.o ${OBJECTDIR}/mDNS.o ${OBJECTDIR}/FIFO.o ${OBJECTDIR}/circularPrintF.o ${OBJECTDIR}/Base64.o ${OBJECTDIR}/MD5.o ${OBJECTDIR}/pack.o ${OBJECTDIR}/fletcherChecksum.o ${OBJECTDIR}/GOTHtraps.o ${OBJECTDIR}/geterrorloc.o ${OBJECTDIR}/FlashFS.o ${OBJECTDIR}/PID.o ${OBJECTDIR}/IIR.o ${OBJECTDIR}/RLE_Compressor.o ${OBJECTDIR}/BlobFS.o ${OBJECTDIR}/Http_API.o ${OBJECTDIR}/Settings.o ${OBJECTDIR}/SystemConfiguration.o ${OBJECTDIR}/http_server.o ${OBJECTDIR}/main.o ${OBJECTDIR}/TemperatureControler.o

# Source Files
SOURCEFILES=OneWire.c ESP8266.c rtc.c uPnP.c mDNS.c FIFO.c circularPrintF.c Base64.c MD5.c pack.c fletcherChecksum.c GOTHtraps.c geterrorloc.s FlashFS.c PID.c IIR.c RLE_Compressor.c BlobFS.c Http_API.c Settings.c SystemConfiguration.c http_server.c main.c TemperatureControler.c


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
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/TempConWifi_v2.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP256MC204
MP_LINKER_FILE_OPTION=,--script="TempCon_p33EP256MC204.gld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/geterrorloc.o: geterrorloc.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/geterrorloc.o.d 
	@${RM} ${OBJECTDIR}/geterrorloc.o.ok ${OBJECTDIR}/geterrorloc.o.err 
	@${RM} ${OBJECTDIR}/geterrorloc.o 
	@${FIXDEPS} "${OBJECTDIR}/geterrorloc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_AS} $(MP_EXTRA_AS_PRE)  geterrorloc.s -o ${OBJECTDIR}/geterrorloc.o -omf=elf -p=$(MP_PROCESSOR_OPTION) --defsym=__MPLAB_BUILD=1 --defsym=__MPLAB_DEBUG=1 --defsym=__ICD2RAM=1 --defsym=__DEBUG=1 --defsym=__MPLAB_DEBUGGER_PK3=1 -g  -MD "${OBJECTDIR}/geterrorloc.o.d"$(MP_EXTRA_AS_POST)
	
else
${OBJECTDIR}/geterrorloc.o: geterrorloc.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/geterrorloc.o.d 
	@${RM} ${OBJECTDIR}/geterrorloc.o.ok ${OBJECTDIR}/geterrorloc.o.err 
	@${RM} ${OBJECTDIR}/geterrorloc.o 
	@${FIXDEPS} "${OBJECTDIR}/geterrorloc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_AS} $(MP_EXTRA_AS_PRE)  geterrorloc.s -o ${OBJECTDIR}/geterrorloc.o -omf=elf -p=$(MP_PROCESSOR_OPTION) --defsym=__MPLAB_BUILD=1 -g  -MD "${OBJECTDIR}/geterrorloc.o.d"$(MP_EXTRA_AS_POST)
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/OneWire.o: OneWire.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/OneWire.o.d 
	@${RM} ${OBJECTDIR}/OneWire.o.ok ${OBJECTDIR}/OneWire.o.err 
	@${RM} ${OBJECTDIR}/OneWire.o 
	@${FIXDEPS} "${OBJECTDIR}/OneWire.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/OneWire.o.d" -o ${OBJECTDIR}/OneWire.o OneWire.c    
	
${OBJECTDIR}/ESP8266.o: ESP8266.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/ESP8266.o.d 
	@${RM} ${OBJECTDIR}/ESP8266.o.ok ${OBJECTDIR}/ESP8266.o.err 
	@${RM} ${OBJECTDIR}/ESP8266.o 
	@${FIXDEPS} "${OBJECTDIR}/ESP8266.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/ESP8266.o.d" -o ${OBJECTDIR}/ESP8266.o ESP8266.c    
	
${OBJECTDIR}/rtc.o: rtc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/rtc.o.d 
	@${RM} ${OBJECTDIR}/rtc.o.ok ${OBJECTDIR}/rtc.o.err 
	@${RM} ${OBJECTDIR}/rtc.o 
	@${FIXDEPS} "${OBJECTDIR}/rtc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/rtc.o.d" -o ${OBJECTDIR}/rtc.o rtc.c    
	
${OBJECTDIR}/uPnP.o: uPnP.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uPnP.o.d 
	@${RM} ${OBJECTDIR}/uPnP.o.ok ${OBJECTDIR}/uPnP.o.err 
	@${RM} ${OBJECTDIR}/uPnP.o 
	@${FIXDEPS} "${OBJECTDIR}/uPnP.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/uPnP.o.d" -o ${OBJECTDIR}/uPnP.o uPnP.c    
	
${OBJECTDIR}/mDNS.o: mDNS.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/mDNS.o.d 
	@${RM} ${OBJECTDIR}/mDNS.o.ok ${OBJECTDIR}/mDNS.o.err 
	@${RM} ${OBJECTDIR}/mDNS.o 
	@${FIXDEPS} "${OBJECTDIR}/mDNS.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/mDNS.o.d" -o ${OBJECTDIR}/mDNS.o mDNS.c    
	
${OBJECTDIR}/FIFO.o: FIFO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/FIFO.o.d 
	@${RM} ${OBJECTDIR}/FIFO.o.ok ${OBJECTDIR}/FIFO.o.err 
	@${RM} ${OBJECTDIR}/FIFO.o 
	@${FIXDEPS} "${OBJECTDIR}/FIFO.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/FIFO.o.d" -o ${OBJECTDIR}/FIFO.o FIFO.c    
	
${OBJECTDIR}/circularPrintF.o: circularPrintF.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/circularPrintF.o.d 
	@${RM} ${OBJECTDIR}/circularPrintF.o.ok ${OBJECTDIR}/circularPrintF.o.err 
	@${RM} ${OBJECTDIR}/circularPrintF.o 
	@${FIXDEPS} "${OBJECTDIR}/circularPrintF.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/circularPrintF.o.d" -o ${OBJECTDIR}/circularPrintF.o circularPrintF.c    
	
${OBJECTDIR}/Base64.o: Base64.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Base64.o.d 
	@${RM} ${OBJECTDIR}/Base64.o.ok ${OBJECTDIR}/Base64.o.err 
	@${RM} ${OBJECTDIR}/Base64.o 
	@${FIXDEPS} "${OBJECTDIR}/Base64.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/Base64.o.d" -o ${OBJECTDIR}/Base64.o Base64.c    
	
${OBJECTDIR}/MD5.o: MD5.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/MD5.o.d 
	@${RM} ${OBJECTDIR}/MD5.o.ok ${OBJECTDIR}/MD5.o.err 
	@${RM} ${OBJECTDIR}/MD5.o 
	@${FIXDEPS} "${OBJECTDIR}/MD5.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/MD5.o.d" -o ${OBJECTDIR}/MD5.o MD5.c    
	
${OBJECTDIR}/pack.o: pack.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pack.o.d 
	@${RM} ${OBJECTDIR}/pack.o.ok ${OBJECTDIR}/pack.o.err 
	@${RM} ${OBJECTDIR}/pack.o 
	@${FIXDEPS} "${OBJECTDIR}/pack.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/pack.o.d" -o ${OBJECTDIR}/pack.o pack.c    
	
${OBJECTDIR}/fletcherChecksum.o: fletcherChecksum.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/fletcherChecksum.o.d 
	@${RM} ${OBJECTDIR}/fletcherChecksum.o.ok ${OBJECTDIR}/fletcherChecksum.o.err 
	@${RM} ${OBJECTDIR}/fletcherChecksum.o 
	@${FIXDEPS} "${OBJECTDIR}/fletcherChecksum.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/fletcherChecksum.o.d" -o ${OBJECTDIR}/fletcherChecksum.o fletcherChecksum.c    
	
${OBJECTDIR}/GOTHtraps.o: GOTHtraps.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/GOTHtraps.o.d 
	@${RM} ${OBJECTDIR}/GOTHtraps.o.ok ${OBJECTDIR}/GOTHtraps.o.err 
	@${RM} ${OBJECTDIR}/GOTHtraps.o 
	@${FIXDEPS} "${OBJECTDIR}/GOTHtraps.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/GOTHtraps.o.d" -o ${OBJECTDIR}/GOTHtraps.o GOTHtraps.c    
	
${OBJECTDIR}/FlashFS.o: FlashFS.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/FlashFS.o.d 
	@${RM} ${OBJECTDIR}/FlashFS.o.ok ${OBJECTDIR}/FlashFS.o.err 
	@${RM} ${OBJECTDIR}/FlashFS.o 
	@${FIXDEPS} "${OBJECTDIR}/FlashFS.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/FlashFS.o.d" -o ${OBJECTDIR}/FlashFS.o FlashFS.c    
	
${OBJECTDIR}/PID.o: PID.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/PID.o.d 
	@${RM} ${OBJECTDIR}/PID.o.ok ${OBJECTDIR}/PID.o.err 
	@${RM} ${OBJECTDIR}/PID.o 
	@${FIXDEPS} "${OBJECTDIR}/PID.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/PID.o.d" -o ${OBJECTDIR}/PID.o PID.c    
	
${OBJECTDIR}/IIR.o: IIR.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/IIR.o.d 
	@${RM} ${OBJECTDIR}/IIR.o.ok ${OBJECTDIR}/IIR.o.err 
	@${RM} ${OBJECTDIR}/IIR.o 
	@${FIXDEPS} "${OBJECTDIR}/IIR.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/IIR.o.d" -o ${OBJECTDIR}/IIR.o IIR.c    
	
${OBJECTDIR}/RLE_Compressor.o: RLE_Compressor.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/RLE_Compressor.o.d 
	@${RM} ${OBJECTDIR}/RLE_Compressor.o.ok ${OBJECTDIR}/RLE_Compressor.o.err 
	@${RM} ${OBJECTDIR}/RLE_Compressor.o 
	@${FIXDEPS} "${OBJECTDIR}/RLE_Compressor.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/RLE_Compressor.o.d" -o ${OBJECTDIR}/RLE_Compressor.o RLE_Compressor.c    
	
${OBJECTDIR}/BlobFS.o: BlobFS.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/BlobFS.o.d 
	@${RM} ${OBJECTDIR}/BlobFS.o.ok ${OBJECTDIR}/BlobFS.o.err 
	@${RM} ${OBJECTDIR}/BlobFS.o 
	@${FIXDEPS} "${OBJECTDIR}/BlobFS.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/BlobFS.o.d" -o ${OBJECTDIR}/BlobFS.o BlobFS.c    
	
${OBJECTDIR}/Http_API.o: Http_API.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Http_API.o.d 
	@${RM} ${OBJECTDIR}/Http_API.o.ok ${OBJECTDIR}/Http_API.o.err 
	@${RM} ${OBJECTDIR}/Http_API.o 
	@${FIXDEPS} "${OBJECTDIR}/Http_API.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/Http_API.o.d" -o ${OBJECTDIR}/Http_API.o Http_API.c    
	
${OBJECTDIR}/Settings.o: Settings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Settings.o.d 
	@${RM} ${OBJECTDIR}/Settings.o.ok ${OBJECTDIR}/Settings.o.err 
	@${RM} ${OBJECTDIR}/Settings.o 
	@${FIXDEPS} "${OBJECTDIR}/Settings.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/Settings.o.d" -o ${OBJECTDIR}/Settings.o Settings.c    
	
${OBJECTDIR}/SystemConfiguration.o: SystemConfiguration.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/SystemConfiguration.o.d 
	@${RM} ${OBJECTDIR}/SystemConfiguration.o.ok ${OBJECTDIR}/SystemConfiguration.o.err 
	@${RM} ${OBJECTDIR}/SystemConfiguration.o 
	@${FIXDEPS} "${OBJECTDIR}/SystemConfiguration.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/SystemConfiguration.o.d" -o ${OBJECTDIR}/SystemConfiguration.o SystemConfiguration.c    
	
${OBJECTDIR}/http_server.o: http_server.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/http_server.o.d 
	@${RM} ${OBJECTDIR}/http_server.o.ok ${OBJECTDIR}/http_server.o.err 
	@${RM} ${OBJECTDIR}/http_server.o 
	@${FIXDEPS} "${OBJECTDIR}/http_server.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/http_server.o.d" -o ${OBJECTDIR}/http_server.o http_server.c    
	
${OBJECTDIR}/main.o: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o.ok ${OBJECTDIR}/main.o.err 
	@${RM} ${OBJECTDIR}/main.o 
	@${FIXDEPS} "${OBJECTDIR}/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/main.o.d" -o ${OBJECTDIR}/main.o main.c    
	
${OBJECTDIR}/TemperatureControler.o: TemperatureControler.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/TemperatureControler.o.d 
	@${RM} ${OBJECTDIR}/TemperatureControler.o.ok ${OBJECTDIR}/TemperatureControler.o.err 
	@${RM} ${OBJECTDIR}/TemperatureControler.o 
	@${FIXDEPS} "${OBJECTDIR}/TemperatureControler.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/TemperatureControler.o.d" -o ${OBJECTDIR}/TemperatureControler.o TemperatureControler.c    
	
else
${OBJECTDIR}/OneWire.o: OneWire.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/OneWire.o.d 
	@${RM} ${OBJECTDIR}/OneWire.o.ok ${OBJECTDIR}/OneWire.o.err 
	@${RM} ${OBJECTDIR}/OneWire.o 
	@${FIXDEPS} "${OBJECTDIR}/OneWire.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/OneWire.o.d" -o ${OBJECTDIR}/OneWire.o OneWire.c    
	
${OBJECTDIR}/ESP8266.o: ESP8266.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/ESP8266.o.d 
	@${RM} ${OBJECTDIR}/ESP8266.o.ok ${OBJECTDIR}/ESP8266.o.err 
	@${RM} ${OBJECTDIR}/ESP8266.o 
	@${FIXDEPS} "${OBJECTDIR}/ESP8266.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/ESP8266.o.d" -o ${OBJECTDIR}/ESP8266.o ESP8266.c    
	
${OBJECTDIR}/rtc.o: rtc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/rtc.o.d 
	@${RM} ${OBJECTDIR}/rtc.o.ok ${OBJECTDIR}/rtc.o.err 
	@${RM} ${OBJECTDIR}/rtc.o 
	@${FIXDEPS} "${OBJECTDIR}/rtc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/rtc.o.d" -o ${OBJECTDIR}/rtc.o rtc.c    
	
${OBJECTDIR}/uPnP.o: uPnP.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uPnP.o.d 
	@${RM} ${OBJECTDIR}/uPnP.o.ok ${OBJECTDIR}/uPnP.o.err 
	@${RM} ${OBJECTDIR}/uPnP.o 
	@${FIXDEPS} "${OBJECTDIR}/uPnP.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/uPnP.o.d" -o ${OBJECTDIR}/uPnP.o uPnP.c    
	
${OBJECTDIR}/mDNS.o: mDNS.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/mDNS.o.d 
	@${RM} ${OBJECTDIR}/mDNS.o.ok ${OBJECTDIR}/mDNS.o.err 
	@${RM} ${OBJECTDIR}/mDNS.o 
	@${FIXDEPS} "${OBJECTDIR}/mDNS.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/mDNS.o.d" -o ${OBJECTDIR}/mDNS.o mDNS.c    
	
${OBJECTDIR}/FIFO.o: FIFO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/FIFO.o.d 
	@${RM} ${OBJECTDIR}/FIFO.o.ok ${OBJECTDIR}/FIFO.o.err 
	@${RM} ${OBJECTDIR}/FIFO.o 
	@${FIXDEPS} "${OBJECTDIR}/FIFO.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/FIFO.o.d" -o ${OBJECTDIR}/FIFO.o FIFO.c    
	
${OBJECTDIR}/circularPrintF.o: circularPrintF.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/circularPrintF.o.d 
	@${RM} ${OBJECTDIR}/circularPrintF.o.ok ${OBJECTDIR}/circularPrintF.o.err 
	@${RM} ${OBJECTDIR}/circularPrintF.o 
	@${FIXDEPS} "${OBJECTDIR}/circularPrintF.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/circularPrintF.o.d" -o ${OBJECTDIR}/circularPrintF.o circularPrintF.c    
	
${OBJECTDIR}/Base64.o: Base64.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Base64.o.d 
	@${RM} ${OBJECTDIR}/Base64.o.ok ${OBJECTDIR}/Base64.o.err 
	@${RM} ${OBJECTDIR}/Base64.o 
	@${FIXDEPS} "${OBJECTDIR}/Base64.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/Base64.o.d" -o ${OBJECTDIR}/Base64.o Base64.c    
	
${OBJECTDIR}/MD5.o: MD5.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/MD5.o.d 
	@${RM} ${OBJECTDIR}/MD5.o.ok ${OBJECTDIR}/MD5.o.err 
	@${RM} ${OBJECTDIR}/MD5.o 
	@${FIXDEPS} "${OBJECTDIR}/MD5.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/MD5.o.d" -o ${OBJECTDIR}/MD5.o MD5.c    
	
${OBJECTDIR}/pack.o: pack.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pack.o.d 
	@${RM} ${OBJECTDIR}/pack.o.ok ${OBJECTDIR}/pack.o.err 
	@${RM} ${OBJECTDIR}/pack.o 
	@${FIXDEPS} "${OBJECTDIR}/pack.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/pack.o.d" -o ${OBJECTDIR}/pack.o pack.c    
	
${OBJECTDIR}/fletcherChecksum.o: fletcherChecksum.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/fletcherChecksum.o.d 
	@${RM} ${OBJECTDIR}/fletcherChecksum.o.ok ${OBJECTDIR}/fletcherChecksum.o.err 
	@${RM} ${OBJECTDIR}/fletcherChecksum.o 
	@${FIXDEPS} "${OBJECTDIR}/fletcherChecksum.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/fletcherChecksum.o.d" -o ${OBJECTDIR}/fletcherChecksum.o fletcherChecksum.c    
	
${OBJECTDIR}/GOTHtraps.o: GOTHtraps.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/GOTHtraps.o.d 
	@${RM} ${OBJECTDIR}/GOTHtraps.o.ok ${OBJECTDIR}/GOTHtraps.o.err 
	@${RM} ${OBJECTDIR}/GOTHtraps.o 
	@${FIXDEPS} "${OBJECTDIR}/GOTHtraps.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/GOTHtraps.o.d" -o ${OBJECTDIR}/GOTHtraps.o GOTHtraps.c    
	
${OBJECTDIR}/FlashFS.o: FlashFS.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/FlashFS.o.d 
	@${RM} ${OBJECTDIR}/FlashFS.o.ok ${OBJECTDIR}/FlashFS.o.err 
	@${RM} ${OBJECTDIR}/FlashFS.o 
	@${FIXDEPS} "${OBJECTDIR}/FlashFS.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/FlashFS.o.d" -o ${OBJECTDIR}/FlashFS.o FlashFS.c    
	
${OBJECTDIR}/PID.o: PID.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/PID.o.d 
	@${RM} ${OBJECTDIR}/PID.o.ok ${OBJECTDIR}/PID.o.err 
	@${RM} ${OBJECTDIR}/PID.o 
	@${FIXDEPS} "${OBJECTDIR}/PID.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/PID.o.d" -o ${OBJECTDIR}/PID.o PID.c    
	
${OBJECTDIR}/IIR.o: IIR.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/IIR.o.d 
	@${RM} ${OBJECTDIR}/IIR.o.ok ${OBJECTDIR}/IIR.o.err 
	@${RM} ${OBJECTDIR}/IIR.o 
	@${FIXDEPS} "${OBJECTDIR}/IIR.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/IIR.o.d" -o ${OBJECTDIR}/IIR.o IIR.c    
	
${OBJECTDIR}/RLE_Compressor.o: RLE_Compressor.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/RLE_Compressor.o.d 
	@${RM} ${OBJECTDIR}/RLE_Compressor.o.ok ${OBJECTDIR}/RLE_Compressor.o.err 
	@${RM} ${OBJECTDIR}/RLE_Compressor.o 
	@${FIXDEPS} "${OBJECTDIR}/RLE_Compressor.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/RLE_Compressor.o.d" -o ${OBJECTDIR}/RLE_Compressor.o RLE_Compressor.c    
	
${OBJECTDIR}/BlobFS.o: BlobFS.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/BlobFS.o.d 
	@${RM} ${OBJECTDIR}/BlobFS.o.ok ${OBJECTDIR}/BlobFS.o.err 
	@${RM} ${OBJECTDIR}/BlobFS.o 
	@${FIXDEPS} "${OBJECTDIR}/BlobFS.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/BlobFS.o.d" -o ${OBJECTDIR}/BlobFS.o BlobFS.c    
	
${OBJECTDIR}/Http_API.o: Http_API.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Http_API.o.d 
	@${RM} ${OBJECTDIR}/Http_API.o.ok ${OBJECTDIR}/Http_API.o.err 
	@${RM} ${OBJECTDIR}/Http_API.o 
	@${FIXDEPS} "${OBJECTDIR}/Http_API.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/Http_API.o.d" -o ${OBJECTDIR}/Http_API.o Http_API.c    
	
${OBJECTDIR}/Settings.o: Settings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Settings.o.d 
	@${RM} ${OBJECTDIR}/Settings.o.ok ${OBJECTDIR}/Settings.o.err 
	@${RM} ${OBJECTDIR}/Settings.o 
	@${FIXDEPS} "${OBJECTDIR}/Settings.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/Settings.o.d" -o ${OBJECTDIR}/Settings.o Settings.c    
	
${OBJECTDIR}/SystemConfiguration.o: SystemConfiguration.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/SystemConfiguration.o.d 
	@${RM} ${OBJECTDIR}/SystemConfiguration.o.ok ${OBJECTDIR}/SystemConfiguration.o.err 
	@${RM} ${OBJECTDIR}/SystemConfiguration.o 
	@${FIXDEPS} "${OBJECTDIR}/SystemConfiguration.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/SystemConfiguration.o.d" -o ${OBJECTDIR}/SystemConfiguration.o SystemConfiguration.c    
	
${OBJECTDIR}/http_server.o: http_server.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/http_server.o.d 
	@${RM} ${OBJECTDIR}/http_server.o.ok ${OBJECTDIR}/http_server.o.err 
	@${RM} ${OBJECTDIR}/http_server.o 
	@${FIXDEPS} "${OBJECTDIR}/http_server.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/http_server.o.d" -o ${OBJECTDIR}/http_server.o http_server.c    
	
${OBJECTDIR}/main.o: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o.ok ${OBJECTDIR}/main.o.err 
	@${RM} ${OBJECTDIR}/main.o 
	@${FIXDEPS} "${OBJECTDIR}/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/main.o.d" -o ${OBJECTDIR}/main.o main.c    
	
${OBJECTDIR}/TemperatureControler.o: TemperatureControler.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/TemperatureControler.o.d 
	@${RM} ${OBJECTDIR}/TemperatureControler.o.ok ${OBJECTDIR}/TemperatureControler.o.err 
	@${RM} ${OBJECTDIR}/TemperatureControler.o 
	@${FIXDEPS} "${OBJECTDIR}/TemperatureControler.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Werror -Wall -mlarge-code -mlarge-data -MMD -MF "${OBJECTDIR}/TemperatureControler.o.d" -o ${OBJECTDIR}/TemperatureControler.o TemperatureControler.c    
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/TempConWifi_v2.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    TempCon_p33EP256MC204.gld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -o dist/${CND_CONF}/${IMAGE_TYPE}/TempConWifi_v2.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}         -Wl,--defsym=__MPLAB_BUILD=1,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map"$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1
else
dist/${CND_CONF}/${IMAGE_TYPE}/TempConWifi_v2.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   TempCon_p33EP256MC204.gld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf -mcpu=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/TempConWifi_v2.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}         -Wl,--defsym=__MPLAB_BUILD=1,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map"$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION)
	${MP_CC_DIR}\\pic30-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/TempConWifi_v2.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -omf=elf
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
