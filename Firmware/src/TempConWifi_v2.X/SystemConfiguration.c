#include <string.h>
#include <stdio.h>

#include "SystemConfiguration.h"
#include "Main.h"
#include "circularPrintF.h"

//Buffers for Ping-Pong ADC DMA
unsigned int ADC_BufferA[32] __attribute__((aligned(64)));
unsigned int ADC_BufferB[32] __attribute__((aligned(64)));
unsigned long V;

void Log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    circular_vPrintf(logFIFO, format, args);
    _U2TXIF = 1;
    while (FIFO_HasChars(logFIFO));
}

void SetupClock() {
    // Configure PLL prescaler, PLL postscaler, PLL divisor
    PLLFBD = 226; // M=65
    CLKDIVbits.PLLPOST = 0;
    CLKDIVbits.PLLPRE = 5;

    // Initiate Clock Switch to FRC oscillator with PLL (NOSC=0b001)
    __builtin_write_OSCCONH(0x01);
    __builtin_write_OSCCONL(OSCCON | 0x01);
    // Wait for Clock switch to occur
    while (OSCCONbits.COSC != 0b001);
    // Wait for PLL to lock
    while (OSCCONbits.LOCK != 1);

}

void Delay(float time_seconds) {
    unsigned long tmr;
    unsigned long endtime;
    GetTime(tmr);
    endtime = tmr + (SYSTEM_TIMER_FREQ * (time_seconds));
    while (tmr < endtime) {
        DELAY_10uS;
        GetTime(tmr);
    }
}

void SetupPortPins(void) {

    FLASH_MOSI_TRIS = 0;
    FLASH_CLOCK_TRIS = 0;
    FLASH_MISO_TRIS = 1;
    FLASH_CS_LAT = 0;
    FLASH_CS_TRIS = 0;
    SET_FLASH_CS(0);

    DEBUG_UART_TX_TRIS = 0;
    DEBUG_UART_RX_TRIS = 1;

    //WIFI
    WIFI_MCU_RX_TRIS = 1;
    WIFI_MCU_TX_TRIS = 0;
    SET_WIFI_POWER(0);
    SET_WIFI_PROG(0);
    WIFI_POWER_TRIS = 0;
    WIFI_PROG_TRIS = 0;

    HEAT_TRIS = 0;
    COOL_TRIS = 0;
    HEAT_LAT = 0;
    COOL_LAT = 0;
    SET_HEAT(0);
    SET_COOL(0);

    PROBE0_LAT = 0;
    PROBE0_PORT = 0;
    PROBE0_RELEASE;

    PROBE1_LAT = 0;
    PROBE1_PORT = 0;
    PROBE1_RELEASE;

    RTC_SCL_RELEASE;
    RTC_SDA_RELEASE;

    ANSELA = 0;
    ANSELB = 0;
    ANSELAbits.ANSA0 = 1; //Voltage Input OR NOISE generator...

    //Start Remap IO pins
    __builtin_write_OSCCONL(OSCCON & ~(1 << 6)); //UN-LOCK the Remapping Function

    //WIFI UART
    _U1RXR = WIFI_MCU_RX_RP;
    WIFI_MCU_TX_RP = 0b000001;

    //DEBUG UART
    _U2RXR = DEBUG_UART_RX_RP;
    DEBUG_UART_TX_RP = 0b000011;

    __builtin_write_OSCCONL(OSCCON | (1 << 6)); //LOCK the Remapping Function
}

void Setup_UART() {

    U1MODEbits.STSEL = 0; // 1-Stop bit
    U1MODEbits.PDSEL = 0; // No Parity, 8-Data bits
    U1MODEbits.ABAUD = 0; // Auto-Baud disabled
    U1MODEbits.BRGH = 1; // Standard-Speed mode 16x Clock Mode
    U1BRG = WIFI_BRGVAL; //
    U1STAbits.UTXISEL1 = 1;
    U1STAbits.UTXISEL0 = 0; // Interrupt after the FIFO buffer is empty
    U1MODEbits.UARTEN = 1; // Enable UART
    U1STAbits.UTXEN = 1; // Enable UART TX

    U2MODEbits.STSEL = 0; // 1-Stop bit
    U2MODEbits.PDSEL = 0; // No Parity, 8-Data bits
    U2MODEbits.ABAUD = 0; // Auto-Baud disabled
    U2MODEbits.BRGH = 1; // Standard-Speed mode 16x Clock Mode
    U2BRG = DEBUG_BRGVAL; //
    U2STAbits.UTXISEL1 = 1;
    U2STAbits.UTXISEL0 = 0; // Interrupt after the FIFO buffer is empty
    U2MODEbits.UARTEN = 1; // Enable UART
    U2STAbits.UTXEN = 1; // Enable UART TX

    DELAY_105uS
}

void Setup_PID_Timer() {
    //setup for 500Hz PID Timer;

    T1CONbits.TCKPS = 0b10; //64:1 prescaler
    TMR1 = 0;
    PR1 = 1875; //500Hz @ 60MIPS/64
    T1CONbits.TON = 1;
}

void Setup_OneWire_Timer() {
    //setup for 60Mhz Operation
    T2CONbits.TON = 0;
    T2CONbits.TCKPS = 0b00; //1:1 prescaler
    T2CONbits.TCS = 0;
}

void Setup_RandomizerTimer() {
    //setup for 60Mhz Operation
    T4CONbits.TON = 0;
    T4CONbits.TCKPS = 0b00; //1:1 prescaler
    T4CONbits.TCS = 0;
    T4CONbits.TON = 1;
}

void Setup_Interrupts() {
    /* Interrupt nesting enabled here */
    INTCON1bits.NSTDIS = 0;

    //One-Wire Timer
    _T2IP = 7;
    _T2IF = 0;
    _T2IE = 1;

    //ESP Serial RX Filler
    _U1RXIP = 6;
    _U1RXIF = 0;
    _U1RXIE = 1;

    //ESP Serial TX
    _U1TXIP = 5;
    _U1TXIF = 0;
    _U1TXIE = 1;

    //LOG Serial TX
    _U2TXIP = 5;
    _U2TXIF = 0;
    _U2TXIE = 1;

    //PID Loop Next
    _T1IP = 4;
    _T1IF = 0;
    _T1IE = 1;

    //ESP RX FIFO Processor
    _T3IP = 2;
    _T3IF = 0;
    _T3IE = 1;

    //ADC DMA Interrupts
    _DMA2IP = 6;
    _DMA2IF = 0;
    _DMA2IE = 1;

    _T4IE=0;

    INTCON2bits.GIE = 1;
}

void Setup_ADC() {
    AD1CON1bits.ADDMABM = 0; // DMA buffers are written in Scatter/Gather mode.
    AD1CON1bits.AD12B = 1; // 12-bit ADC operation
    AD1CON1bits.FORM = 0b00; // Data Output Format: UnSigned integer
    AD1CON1bits.SSRC = 0b111; // Sample Clock Source:  Internal counter ends sampling and starts conversion (auto-convert)
    AD1CON1bits.SSRCG = 0; //Sample Clock Source Group 0
    AD1CON1bits.SIMSAM = 0; // N/A in 12-bit mode
    AD1CON1bits.ASAM = 1; // Sampling begins immediately after conversion

    AD1CON2bits.VCFG = 0b000; //VrefH = AVDD , VrefL=AVSS
    AD1CON2bits.CSCNA = 1; // Scan CH0+ Input Selections during Sample A bit
    AD1CON2bits.CHPS = 0; // Channel Select bits: N/A in 12-bit mode
    AD1CON2bits.SMPI = 0; // Interrupt every sample - Bufffer Count = 1
    AD1CON2bits.BUFM = 0; // Buffer Fill Mode Select bit: Always starts filling the buffer from the start address.
    AD1CON2bits.ALTS = 0; //Always uses channel input selects for Sample MUXA

    //Setup for ~8kHz Sample Rate over one channels @ 16x average = ~500hz output rate
    //ADCS = 162 @ 60MIPS = TAD = 2700ns 
    //Conv Time = 31TAD Sample + 14 TAD Conversion = 46 TAD * 2700ns = 62.1us * 2
    //1 Channels Scanned = 124us
    //16X DMA transfer = 124us * 16 = 1.9872ms = 1/1.9872ms = ~503Hz /2 =
    AD1CON3bits.ADRC = 0; // ADC clock is derived from systems clock
    AD1CON3bits.SAMC = 31;
    AD1CON3bits.ADCS = 161;

    AD1CON4bits.ADDMAEN = 1; // Conversion results stored in ADC1BUF0 register, for transfer to RAM using DMA
    AD1CON4bits.DMABL = 4; // Each DMA buffer contains 2^4 = 16 words

    AD1CHS0bits.CH0NA = 0; // MUXA -ve input selection (VREF-) for CH0
    AD1CHS0bits.CH0SA = 0; // MUXA +ve input selection (AIN0) for CH0

    //AD1CHS123: Analog-to-Digital Input Select Register
    AD1CHS123bits.CH123SA = 0; // MUXA +ve input selection (AIN0) for CH1
    AD1CHS123bits.CH123NA = 0; // MUXA -ve input selection (VREF-) for CH1

    //AD1CSSH/AD1CSSL: Analog-to-Digital Input Scan Selection Register
    AD1CSSH = 0x0000;
    AD1CSSL = 0; // Scan AIN0, AIN1
    AD1CSSLbits.CSS0 = 1;
    //AD1CSSLbits.CSS1 = 1;

    DMA2CONbits.AMODE = 2; // Configure DMA for Peripheral Indirect mode
    DMA2CONbits.MODE = 2; // Configure DMA for Continuous Ping-Pong mode
    DMA2PAD = (volatile unsigned int) &ADC1BUF0; // Point DMA to ADC1BUF0
    DMA2CNT = 31; // 32 DMA request (2 buffers, each with 16 words)
    DMA2REQ = 13; // Select ADC1 as DMA request source
    DMA2STAL = (unsigned int) &ADC_BufferA;
    DMA2STAH = 0x0000;
    DMA2STBL = (unsigned int) &ADC_BufferB;
    DMA2STBH = 0x0000;
    DMA2CONbits.CHEN = 1;

    IFS0bits.AD1IF = 0; // Clear Analog-to-Digital Interrupt Flag bit
    IEC0bits.AD1IE = 0; // Do Not Enable Analog-to-Digital interrupt
    AD1CON1bits.ADON = 1; // Turn on the ADC
}
