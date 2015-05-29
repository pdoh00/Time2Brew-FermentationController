/* 
 * File:   SystemConfiguration.h
 * Author: THORAXIUM
 *
 * Created on February 9, 2015, 10:08 AM
 */

#ifndef SYSTEMCONFIGURATION_H
#define	SYSTEMCONFIGURATION_H

#include <p33Exxxx.h>
#include "ESP8266.h"

/*Constants*/
#define FCY                         60000000
#define MCU_WIFI_BAUD               460800
#define DEBUG_BAUD                  460800
#define WIFI_BRGVAL                 ((FCY/MCU_WIFI_BAUD)/4)-1
#define DEBUG_BRGVAL                ((FCY/DEBUG_BAUD)/4)-1
#define MES_TICK_FREQUENCY          500
#define SYSTEM_TIMER_FREQ           500

void Log(const char *format, ...);
void SetupClock();
void SetupPortPins();
void Setup_UART();
void Setup_PID_Timer();
void Setup_OneWire_Timer();
void Setup_Interrupts();
void Setup_ADC();
void Setup_RandomizerTimer();
void Delay(float time_seconds);

extern unsigned int ADC_BufferA[32] __attribute__((aligned(64)));
extern unsigned int ADC_BufferB[32] __attribute__((aligned(64)));
extern unsigned long V;
extern int Global_Config_Mode;
extern char TemperatureControllerIsAlive, WifiCommunicationsAreAlive;

#define HW_REV_E

#define SECTORCOUNT   3830
#define FIRMWARE_PRIMARY_ADDRESS 15691776UL
#define FIRMWARE_BACKUP_ADDRESS 16232448UL
#define FIRMWARE_RESERVED_SIZE  540672UL

/*System Wide Variables*/
extern unsigned long Timer500Hz;
extern char __attribute__((aligned)) mDebugEchoSuspend;

/*System Wide Functions*/
extern void (*getErrLoc(void))(void); // Get Address Error Loc

/*Macro Functions*/
#define GetTime(x) ({INTCON2bits.GIE = 0;Nop();Nop();x = Timer500Hz;INTCON2bits.GIE = 1;})

#define DELAY_105uS asm volatile ("REPEAT, #6302"); Nop(); // 105uS delay
#define DELAY_10uS asm volatile ("REPEAT, #602"); Nop(); // 10uS delay
#define DELAY_5uS asm volatile ("REPEAT, #302"); Nop(); // 10uS delay
#define DELAY_100nS asm volatile ("REPEAT, #6"); Nop(); // 10uS delay

/*I/O Functions*/
#define FLASH_MISO_TRIS      TRISAbits.TRISA9
#define FLASH_MOSI_TRIS      TRISAbits.TRISA4
#define FLASH_CLOCK_TRIS      TRISCbits.TRISC3

#define FLASH_CS_TRIS      TRISBbits.TRISB4
#define FLASH_CS_LAT       LATBbits.LATB4
#define FLASH_CS_PORT      PORTBbits.RB4
#define SET_FLASH_CS(x)    {FLASH_CS_LAT=x;FLASH_CS_PORT=x;}


#define WIFI_MCU_RX_PORT    PORTBbits.RB6
#define WIFI_MCU_RX_LAT     LATBbits.LATB6
#define WIFI_MCU_RX_TRIS    TRISBbits.TRISB6
#define WIFI_MCU_RX_RP      38

#define WIFI_MCU_TX_PORT    PORTBbits.RB7
#define WIFI_MCU_TX_LAT     LATBbits.LATB7
#define WIFI_MCU_TX_TRIS    TRISBbits.TRISB7
#define WIFI_MCU_TX_RP      _RP39R

#define WIFI_POWER_LAT      LATBbits.LATB8
#define WIFI_POWER_TRIS     TRISBbits.TRISB8
#define WIFI_POWER_PORT     PORTBbits.RB8
#define SET_WIFI_POWER(x)   {WIFI_POWER_LAT=x;WIFI_POWER_PORT=x;}
#define GET_WIFI_POWER      WIFI_POWER_LAT

#ifdef HW_REV_D
#define DEBUG_UART_TX_PORT  PORTBbits.RB11
#define DEBUG_UART_TX_LAT   LATBbits.LATB11
#define DEBUG_UART_TX_TRIS  TRISBbits.TRISB11
#define DEBUG_UART_TX_RP    _RP43R

#define DEBUG_UART_RX_PORT  PORTBbits.RB10
#define DEBUG_UART_RX_LAT   LATBbits.LATB10
#define DEBUG_UART_RX_TRIS  TRISBbits.TRISB10
#define DEBUG_UART_RX_RP    42

#define WIFI_PROG_LAT      LATBbits.LATB9
#define WIFI_PROG_TRIS     TRISBbits.TRISB9
#define WIFI_PROG_PORT     PORTBbits.RB9
#define SET_WIFI_PROG(x)   (WIFI_PROG_LAT=!x)
#define GET_WIFI_PROG      !WIFI_PROG_LAT

#define PROBE0_LAT          LATCbits.LATC7
#define PROBE0_TRIS         TRISCbits.TRISC7
#define PROBE0_PORT         PORTCbits.RC7
#define PROBE0_ASSERT       {PROBE0_TRIS=0;PROBE0_PORT=0;PROBE0_LAT=0;}
#define PROBE0_RELEASE      {PROBE0_TRIS=1;}

#define PROBE1_LAT          LATCbits.LATC6
#define PROBE1_TRIS         TRISCbits.TRISC6
#define PROBE1_PORT         PORTCbits.RC6
#define PROBE1_ASSERT       {PROBE1_TRIS=0;PROBE1_PORT=0;PROBE1_LAT=0;}
#define PROBE1_RELEASE      {PROBE1_TRIS=1;}

#define RTC_SDA_LAT          LATCbits.LATC9
#define RTC_SDA_TRIS         TRISCbits.TRISC9
#define RTC_SDA_PORT         PORTCbits.RC9
#define RTC_SDA_ASSERT       {RTC_SDA_TRIS=0;RTC_SDA_PORT=0;RTC_SDA_LAT=0;}
#define RTC_SDA_RELEASE      {RTC_SDA_TRIS=1;}

#define RTC_SCL_LAT          LATCbits.LATC8
#define RTC_SCL_TRIS         TRISCbits.TRISC8
#define RTC_SCL_PORT         PORTCbits.RC8
#define RTC_SCL_ASSERT       {RTC_SCL_TRIS=0;RTC_SCL_PORT=0;RTC_SCL_LAT=0;}
#define RTC_SCL_RELEASE      {RTC_SCL_TRIS=1;}

#endif

#ifdef HW_REV_E

#define DEBUG_UART_TX_PORT  PORTCbits.RC6
#define DEBUG_UART_TX_LAT   LATCbits.LATC6
#define DEBUG_UART_TX_TRIS  TRISCbits.TRISC6
#define DEBUG_UART_TX_RP    _RP54R

#define DEBUG_UART_RX_PORT  PORTBbits.RB9
#define DEBUG_UART_RX_LAT   LATBbits.LATB9
#define DEBUG_UART_RX_TRIS  TRISBbits.TRISB9
#define DEBUG_UART_RX_RP    41

#define WIFI_PROG_LAT      LATCbits.LATC7
#define WIFI_PROG_TRIS     TRISCbits.TRISC7
#define WIFI_PROG_PORT     PORTCbits.RC7
#define SET_WIFI_PROG(x)   {WIFI_PROG_LAT=!x;WIFI_PROG_PORT=!x;}
#define GET_WIFI_PROG      !WIFI_PROG_LAT

#define PROBE0_LAT          LATCbits.LATC9
#define PROBE0_TRIS         TRISCbits.TRISC9
#define PROBE0_PORT         PORTCbits.RC9
#define PROBE0_ASSERT       {PROBE0_TRIS=0;PROBE0_PORT=0;PROBE0_LAT=0;}
#define PROBE0_RELEASE      {PROBE0_TRIS=1;}

#define PROBE1_LAT          LATCbits.LATC8
#define PROBE1_TRIS         TRISCbits.TRISC8
#define PROBE1_PORT         PORTCbits.RC8
#define PROBE1_ASSERT       {PROBE1_TRIS=0;PROBE1_PORT=0;PROBE1_LAT=0;}
#define PROBE1_RELEASE      {PROBE1_TRIS=1;}

#define RTC_SDA_LAT          LATBbits.LATB13
#define RTC_SDA_TRIS         TRISBbits.TRISB13
#define RTC_SDA_PORT         PORTBbits.RB13
#define RTC_SDA_ASSERT       {RTC_SDA_TRIS=0;RTC_SDA_PORT=0;RTC_SDA_LAT=0;}
#define RTC_SDA_RELEASE      {RTC_SDA_TRIS=1;}

#define RTC_SCL_LAT          LATBbits.LATB12
#define RTC_SCL_TRIS         TRISBbits.TRISB12
#define RTC_SCL_PORT         PORTBbits.RB12
#define RTC_SCL_ASSERT       {RTC_SCL_TRIS=0;RTC_SCL_PORT=0;RTC_SCL_LAT=0;}
#define RTC_SCL_RELEASE      {RTC_SCL_TRIS=1;}

#endif

#define HEAT_LAT      LATCbits.LATC5
#define HEAT_TRIS     TRISCbits.TRISC5
#define HEAT_PORT     PORTCbits.RC5
#define SET_HEAT(x)   (HEAT_LAT=x)
#define GET_HEAT      !HEAT_LAT

#define COOL_LAT      LATBbits.LATB2
#define COOL_TRIS     TRISBbits.TRISB2
#define COOL_PORT     PORTBbits.RB2
#define SET_COOL(x)   (COOL_LAT=x)
#define GET_COOL      !COOL_LAT


#define CFG_MODE_LAT      LATBbits.LATB5
#define CFG_MODE_TRIS     TRISBbits.TRISB5
#define CFG_MODE_PORT     PORTBbits.RB5
#define GET_CFG_MODE      !CFG_MODE_PORT


#define RTC_I2C_TOCK {asm volatile ("REPEAT, #75"); Nop();} // 1.25uS delay or 800kHz (Time Low & Time High)
#define RTC_I2C_TICK {asm volatile ("REPEAT, #38"); Nop();} // 1.25uS delay or 800kHz (Time Low & Time High)
//#define RTC_I2C_TOCK {asm volatile ("REPEAT, #300"); Nop();} // 5uS delay or 200kHz (Time Low & Time High)
//#define RTC_I2C_TICK {asm volatile ("REPEAT, #150"); Nop();} // 2.5uS delay or 400kHz (Time Low & Time High)

#ifdef	__cplusplus
}
#endif

#endif	/* SETTINGS_H */

