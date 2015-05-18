#include <string.h>
#include <stdio.h>
#include "SystemConfiguration.h"
#include "circularPrintF.h"

#define TX_BYTE(x) {while(!U2STAbits.TRMT);U2TXREG=x;}

void Log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char tx;
    circular_vPrintf(logFIFO, format, args);
    while (FIFO_HasChars(logFIFO)) {
        FIFO_Read(logFIFO, tx);
        TX_BYTE(tx);
    }
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

