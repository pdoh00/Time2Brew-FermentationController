#include "OneWireTemperature.h"
#include "main.h"
#include "SystemConfiguration.h"

#define DELAY_A asm volatile ("REPEAT, #360"); Nop();
#define DELAY_B asm volatile ("REPEAT, #3841"); Nop();
#define DELAY_C asm volatile ("REPEAT, #3601"); Nop();
#define DELAY_D asm volatile ("REPEAT, #600"); Nop();
#define DELAY_E asm volatile ("REPEAT, #640"); Nop();
#define DELAY_F asm volatile ("REPEAT, #3301"); Nop();
#define DELAY_H asm volatile ("REPEAT, #28809"); Nop();
#define DELAY_I asm volatile ("REPEAT, #4201"); Nop();
#define DELAY_J asm volatile ("REPEAT, #24608"); Nop();

#define DELAY_CONVERSION 750000
#define COUNTS_PER_US   8

int OneWireIsBusShorted() {
    PROBE0_RELEASE;
    PROBE1_RELEASE;
    DELAY_105uS;
    int x;
    for (x = 0; x < 10; x++) {
        if (PROBE0_PORT != 1 || PROBE1_PORT != 1) return 1;
    }
    return 0;
}

int OneWireReset(int ProbeId) {
    int ret = 0;
    OneWireISR_Probe = ProbeId;
    ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_RESET_START;
    _T2IF = 1;
    while (ONEWIRE_ISR_STATE != ONEWIRE_ISR_STATE_IDLE);
    ret = OneWireISR_Prescence;
    return ret;
}

void OneWireWrite1(int ProbeId) {
    OneWireISR_Probe = ProbeId;
    ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_WRITE_1_START;
    _T2IF = 1;
    while (ONEWIRE_ISR_STATE != ONEWIRE_ISR_STATE_IDLE);
}

void OneWireWrite0(int ProbeId) {
    OneWireISR_Probe = ProbeId;
    ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_WRITE_0_START;
    _T2IF = 1;
    while (ONEWIRE_ISR_STATE != ONEWIRE_ISR_STATE_IDLE);
}

unsigned char OneWireReadBit(int ProbeId) {
    unsigned char ret = 0;

    OneWireISR_Probe = ProbeId;
    ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_READ_START;
    _T2IF = 1;
    while (ONEWIRE_ISR_STATE != ONEWIRE_ISR_STATE_IDLE);
    ret = OneWireISR_ReadResult;
    return ret;
}

void OneWireWriteByte(int ProbeId, unsigned char dat) {
    char idx = 0;
    for (idx = 0; idx < 8; idx++) {
        if (dat & 0x1) {
            OneWireWrite1(ProbeId);
        } else {
            OneWireWrite0(ProbeId);
        }
        dat >>= 1;
    }
}

int OneWireReadByte(int ProbeId, BYTE *retV) {
    BYTE ret = 0;
    char idx = 0;
    char Val;
    for (idx = 0; idx < 8; idx++) {
        ret >>= 1;
        Val = OneWireReadBit(ProbeId);
        if (Val == -1) {
            return -1;
        } else if (Val == 1) {
            ret |= 0x80;
        }
    }
    *retV = ret;
    return 1;
}
