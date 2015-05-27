/* 
 * File:   OneWireTemperature.h
 * Author: aaron
 *
 * Created on March 23, 2015, 5:03 PM
 */

#ifndef ONEWIRETEMPERATURE_H
#define	ONEWIRETEMPERATURE_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        ONEWIRE_ISR_STATE_IDLE,
        ONEWIRE_ISR_STATE_WRITE_1_START,
        ONEWIRE_ISR_STATE_WRITE_1_A,
        ONEWIRE_ISR_STATE_WRITE_1_B,
        ONEWIRE_ISR_STATE_WRITE_0_START,
        ONEWIRE_ISR_STATE_WRITE_0_C,
        ONEWIRE_ISR_STATE_WRITE_0_D,
        ONEWIRE_ISR_STATE_READ_START,
        ONEWIRE_ISR_STATE_READ_A,
        ONEWIRE_ISR_STATE_READ_E,
        ONEWIRE_ISR_STATE_READ_F,
        ONEWIRE_ISR_STATE_RESET_START,
        ONEWIRE_ISR_STATE_RESET_H,
        ONEWIRE_ISR_STATE_RESET_I,
        ONEWIRE_ISR_STATE_RESET_J
    } ONEWIRE_ISR_STATES;

    extern volatile ONEWIRE_ISR_STATES ONEWIRE_ISR_STATE;

    extern volatile char OneWireISR_Probe;
    extern volatile char OneWireISR_ReadResult, OneWireISR_Prescence;

    int OneWireReset(int ProbeId);
    void OneWireWriteByte(int ProbeId, unsigned char dat);
    int OneWireReadByte(int ProbeId, unsigned char *retV);

#ifdef	__cplusplus
}
#endif

#endif	/* ONEWIRETEMPERATURE_H */

