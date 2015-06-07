/* 
 * File:   main.c
 * Author: THORAXIUM
 *
 * Created on March 3, 2015, 8:04 PM
 */

#include "mDNS.h"
#include "TemperatureController.h"
#include "BlobFS.h"
#include <p33Exxxx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "integer.h"
#include "FIFO.h"
#include "main.h"
#include "ESP8266.h"
#include "RTC.h"
#include "SystemConfiguration.h"
#include "uPnP.h"
#include "Pack.h"
#include "mDNS.h"
#include "Settings.h"
#include "http_server.h"
#include "OneWireTemperature.h"
#include "FlashFS.h"
#include "RLE_Compressor.h"


/** CONFIGURATION Bits **********************************************/
_FICD(ICS_PGD3 & JTAGEN_OFF); //ICD takes place on PGD3 and PGC3 pins
_FPOR(ALTI2C1_OFF & ALTI2C2_OFF & WDTWIN_WIN25); //Do not use Alternate Pin Mapping for I2C
_FWDT(WDTPOST_PS256 & WDTPRE_PR128 & PLLKEN_ON & WINDIS_OFF & FWDTEN_OFF); //Turn off WDT in hardware but Timeout=1.024seconds with no Window
_FOSC(FCKSM_CSECMD & OSCIOFNC_ON & POSCMD_NONE & IOL1WAY_OFF); // Enable Clock Switching and Configure Primary Oscillator in XT mode
_FOSCSEL(FNOSC_FRC & IESO_OFF); // Select Internal FRC at POR and do not lock the PWM registers
_FGS(GWRP_OFF & GCP_OFF); //Turn off Code Protect

const char *WiFiConfigFilename = "secure.wificonfig";

const char *Version = "0.1.10";

unsigned long Timer500Hz;

char TemperatureControllerIsAlive, WifiCommunicationsAreAlive, MainLoopIsAlive, WDT_Enabled;

int TriggerConfigReset = 0;

int TriggerWifiReset = 0;

int Global_Config_Mode = 0;

ESP8266_CONFIG ESP_Config;

BYTE __attribute__((aligned)) rxFIFOData[2048];
FIFO_BUFFER rxFIFO_real = {rxFIFOData, rxFIFOData, rxFIFOData, rxFIFOData + 2048};
FIFO_BUFFER *rxFIFO = &rxFIFO_real;

BYTE __attribute__((aligned)) txFIFOData[2048];
FIFO_BUFFER txFIFO_real = {txFIFOData, txFIFOData, txFIFOData, txFIFOData + 2048};
FIFO_BUFFER *txFIFO = &txFIFO_real;

BYTE __attribute__((aligned)) logFIFOData[1024];
FIFO_BUFFER logFIFO_real = {logFIFOData, logFIFOData, logFIFOData, logFIFOData + 1024};
FIFO_BUFFER *logFIFO = &logFIFO_real;

BYTE __attribute__((aligned)) TRNG_Data[128];
FIFO_BUFFER TRNG_real = {TRNG_Data, TRNG_Data, TRNG_Data, TRNG_Data + 128};
FIFO_BUFFER *TRNG_fifo = &TRNG_real;

void __attribute__((__interrupt__, no_auto_psv)) _U1RXInterrupt(void) {
    BYTE token;
    _U1RXIF = 0;
    /* Must clear any overrun error to keep UART receiving */
    if (U1STAbits.OERR == 1) {
        U1STAbits.OERR = 0;
    }

    while (U1STAbits.URXDA) {
        token = U1RXREG;
        FIFO_Write(rxFIFO, token);
        _T3IF = 1;
    }
}

void __attribute__((__interrupt__, no_auto_psv)) _T3Interrupt(void) {
    ESP_RX_FIFO_Processor();
}

void __attribute__((__interrupt__, no_auto_psv)) _U1TXInterrupt(void) {
    BYTE token;
    _U1TXIE = 0;
    while (!U1STAbits.UTXBF) {
        if (FIFO_HasChars(txFIFO)) {
            FIFO_Read(txFIFO, token);
            U1TXREG = token;
            //U2TXREG = token;
        } else {
            break;
        }
    }
    _U1TXIF = 0;
    _U1TXIE = 1;
}

void __attribute__((__interrupt__, no_auto_psv)) _U2TXInterrupt(void) {
    char token;
    _U2TXIE = 0;
    while (!U2STAbits.UTXBF) {
        if (FIFO_HasChars(logFIFO)) {
            FIFO_Read(logFIFO, token);
            U2TXREG = token;
        } else {
            break;
        }
    }
    _U2TXIF = 0;
    _U2TXIE = 1;
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {
    static int HalfSecondTick = 0;
    static int CFG_MODE_COUNT = 0;

    _T1IF = 0;
    asm("CLRWDT");

    Timer500Hz++;
    HalfSecondTick++;
    if (HalfSecondTick == 250) {
        _CNIF = 1; //Tell the Temperature Controller to Run! but on a lower priority thread.
        HalfSecondTick = 1;
    }

    if (!CFG_MODE_PORT) {
        CFG_MODE_COUNT++;
        if (CFG_MODE_COUNT > 1000) {
            asm("RESET");
        }
    } else {
        if (CFG_MODE_COUNT > 10) {
            TriggerConfigReset = 1;
        }
        CFG_MODE_COUNT = 0;
    }


}

void __attribute__((__interrupt__, no_auto_psv)) _CNInterrupt(void) {
    static unsigned long WDT_TIME = 0;
    static unsigned long lastTempConRun = 0;
    static unsigned long lastWifiCommunication = 0;
    static unsigned long lastMainLoopRun = 0;

    _CNIF = 0;
    TemperatureController_Interrupt();

    if (WDT_Enabled) {
        WDT_TIME++;

        if (TemperatureControllerIsAlive) {
            lastTempConRun = WDT_TIME;
            TemperatureControllerIsAlive = 0;
        }

        if (WifiCommunicationsAreAlive) {
            lastWifiCommunication = WDT_TIME;
            WifiCommunicationsAreAlive = 0;
        }

        if (MainLoopIsAlive) {
            lastMainLoopRun = WDT_TIME;
            MainLoopIsAlive = 0;
        }

        if ((WDT_TIME - lastTempConRun) > 240) { //2 minute
            Log("WDT: Temperature Controller Is Not Alive!");
            Delay(0.1);
            asm("RESET");
        }

        if ((WDT_TIME - lastWifiCommunication) > 1200) { //10 minutes
            Log("WDT: WiFi Comms Are Not Alive!");
            TriggerWifiReset = 1;
            lastWifiCommunication = WDT_TIME;
        }

        if ((WDT_TIME - lastMainLoopRun) > 240) { //2 minutes
            Log("WDT: Main Loop Is Not Alive!");
            Delay(0.1);
            asm("RESET");
        }


    }


}

void __attribute__((__interrupt__, no_auto_psv)) _DMA2Interrupt(void) {

    static unsigned int ADC_PingPong = 0;
    int x;

    union {
        unsigned long l;
        unsigned char b[4];
    } randomLong;

    IFS1bits.DMA2IF = 0; // Clear the DMA2 Interrupt Flag
    // Switch between Primary and Secondary Ping-Pong buffers
    if (ADC_PingPong == 0) {
        for (x = 0; x < 16; x++) {
            randomLong.l <<= 1;
            if (ADC_BufferA[x] & 0x1) randomLong.l += 1;
        }
        if (FIFO_FreeSpace(TRNG_fifo) > 2) {
            FIFO_Write(TRNG_fifo, randomLong.b[0]);
            FIFO_Write(TRNG_fifo, randomLong.b[1]);
        }
    } else {
        for (x = 0; x < 16; x++) {
            randomLong.l <<= 1;
            if (ADC_BufferB[x] & 0x1) randomLong.l += 1;
        }
        if (FIFO_FreeSpace(TRNG_fifo) > 2) {

            FIFO_Write(TRNG_fifo, randomLong.b[0]);
            FIFO_Write(TRNG_fifo, randomLong.b[1]);
        }
    }
    ADC_PingPong = !ADC_PingPong;
}

volatile ONEWIRE_ISR_STATES ONEWIRE_ISR_STATE;
volatile char OneWireISR_Probe;
volatile char OneWireISR_ReadResult, OneWireISR_Prescence;

void __attribute__((__interrupt__, no_auto_psv)) _T2Interrupt(void) {
    _T2IF = 0;
    T2CONbits.TON = 0;
    TMR2 = 0;
    BYTE accum;
    switch (ONEWIRE_ISR_STATE) {
            //..........................................
            //WRITE 1
            //..........................................
        case ONEWIRE_ISR_STATE_WRITE_1_START:
            if (OneWireISR_Probe) {
                PROBE1_ASSERT;
            } else {
                PROBE0_ASSERT;
            }
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_WRITE_1_A;
            PR2 = 360;
            T2CONbits.TON = 1;
            break;
        case ONEWIRE_ISR_STATE_WRITE_1_A:
            if (OneWireISR_Probe) {
                PROBE1_RELEASE;
            } else {
                PROBE0_RELEASE;
            }
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_WRITE_1_B;
            PR2 = 3840;
            T2CONbits.TON = 1;
            break;
        case ONEWIRE_ISR_STATE_WRITE_1_B:
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_IDLE;
            break;


            //..........................................
            //WRITE 0
            //..........................................
        case ONEWIRE_ISR_STATE_WRITE_0_START:
            if (OneWireISR_Probe) {
                PROBE1_ASSERT;
            } else {
                PROBE0_ASSERT;
            }
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_WRITE_0_C;
            PR2 = 3600;
            T2CONbits.TON = 1;
            break;
        case ONEWIRE_ISR_STATE_WRITE_0_C:
            if (OneWireISR_Probe) {
                PROBE1_RELEASE;
            } else {
                PROBE0_RELEASE;
            }
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_WRITE_0_D;
            PR2 = 600;
            T2CONbits.TON = 1;
            break;
        case ONEWIRE_ISR_STATE_WRITE_0_D:
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_IDLE;
            break;

            //..........................................
            //READ bit
            //..........................................
        case ONEWIRE_ISR_STATE_READ_START:
            if (OneWireISR_Probe) {
                PROBE1_ASSERT;
            } else {
                PROBE0_ASSERT;
            }
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_READ_A;
            PR2 = 360;
            T2CONbits.TON = 1;
            break;
        case ONEWIRE_ISR_STATE_READ_A:
            if (OneWireISR_Probe) {
                PROBE1_RELEASE;
            } else {
                PROBE0_RELEASE;
            }
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_READ_E;
            PR2 = 540;
            T2CONbits.TON = 1;
            break;
        case ONEWIRE_ISR_STATE_READ_E:
            if (OneWireISR_Probe) {
                accum = 0;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (accum > 7) {
                    OneWireISR_ReadResult = 1;
                } else if (accum < 3) {
                    OneWireISR_ReadResult = 0;
                } else {
                    OneWireISR_ReadResult = -1;
                }
            } else {
                //Sample 10 times over ~2us
                accum = 0;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (accum > 7) {
                    OneWireISR_ReadResult = 1;
                } else if (accum < 3) {
                    OneWireISR_ReadResult = 0;
                } else {
                    OneWireISR_ReadResult = -1;
                }
            }
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_READ_F;
            PR2 = 3200;
            T2CONbits.TON = 1;
            break;
        case ONEWIRE_ISR_STATE_READ_F:
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_IDLE;
            break;

            //..........................................
            //Reset
            //..........................................
        case ONEWIRE_ISR_STATE_RESET_START:
            if (OneWireISR_Probe) {
                PROBE1_ASSERT;
            } else {
                PROBE0_ASSERT;
            }
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_RESET_H;
            PR2 = 28800;
            T2CONbits.TON = 1;
            break;
        case ONEWIRE_ISR_STATE_RESET_H:
            if (OneWireISR_Probe) {
                PROBE1_RELEASE;
            } else {
                PROBE0_RELEASE;
            }
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_RESET_I;
            PR2 = 4200;
            T2CONbits.TON = 1;
            break;
        case ONEWIRE_ISR_STATE_RESET_I:
            if (OneWireISR_Probe) {
                accum = 0;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (PROBE1_PORT) accum++;
                if (accum > 7) {
                    OneWireISR_Prescence = 1;
                } else if (accum < 3) {
                    OneWireISR_Prescence = 0;
                } else {
                    OneWireISR_Prescence = -1;
                }
            } else {
                accum = 0;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (PROBE0_PORT) accum++;
                if (accum > 7) {
                    OneWireISR_Prescence = 1;
                } else if (accum < 3) {
                    OneWireISR_Prescence = 0;
                } else {
                    OneWireISR_Prescence = -1;
                }
            }
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_RESET_J;
            PR2 = 24500;
            T2CONbits.TON = 1;
            break;
        case ONEWIRE_ISR_STATE_RESET_J:
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_IDLE;
            break;
        default:
            ONEWIRE_ISR_STATE = ONEWIRE_ISR_STATE_IDLE;

            break;
    }
}

void MakeSafe() {

    SET_HEAT(0);
    SET_COOL(0);
    SET_WIFI_POWER(0);
    SET_WIFI_PROG(0);
}

int main(int argc, char** argv) {
    WDT_Enabled = 0;
    INTCON2bits.GIE = 0;

    SetupClock();
    SetupPortPins();
    Setup_UART();
    Setup_PID_Timer();
    Setup_OneWire_Timer();
    Setup_RandomizerTimer();
    Setup_ADC();
    MakeSafe();
    Setup_Interrupts();

    Log("RTC Initializing...\r\n");
    RTC_Initialize();
    Delay(1);

    TriggerConfigReset = 0;

    if (GlobalStartup(0) != FR_OK) {
        Log("Failure To Configure...Starting in Config Mode...\r\n");
        while (GlobalStartup(1) != FR_OK) {
            Log("Failure To Configure...Retry...\r\n");
        };
    }

    Log("Testing Blob...\r\n");
    BLOB_FILE dut;
    BLOB_openFile(&dut, "nevergonnafindit");
    
    Log("Starting WDT:...");
    if (RCONbits.WDTO) {
        Log("Last Restart was due to WDT Timeout!...");
        RCONbits.WDTO = 0;
        TemperatureControllerIsAlive = 0;
        WifiCommunicationsAreAlive = 0;
        MainLoopIsAlive = 0;
    }
    asm("CLRWDT");
    RCONbits.SWDTEN = 1;
    WDT_Enabled = 1;
    Log("OK\r\n");



    Log("\r\n**System Online**\r\n");

    unsigned long now;
    while (1) {
        MainLoopIsAlive = 1;
        now = RTC_GetTime();
        DISABLE_INTERRUPTS;
        globalstate.SystemTime = now;
        ENABLE_INTERRUPTS;
        WriteRecoveryRecord(&globalstate);

        HTTP_ServerLoop();
        mDNS_ProcessLoop();
        uPnP_ProcessLoop();
        TrendBufferCommitt();
        if (TriggerConfigReset) {
            TriggerConfigReset = 0;
            GlobalStartup(1);
        }

        if (TriggerWifiReset) {
            TriggerWifiReset = 0;
            Log("\r\nInitializing WiFi Module\r\n");
            if (ESP_Init() != 1) {
                Log("Unable to initialize the ESP Module...");

                return -1;
            }
            Log("   WiFI Module Ready. IP Address=%d.%d.%d.%d\r\n", IP_Address.b[3], IP_Address.b[2], IP_Address.b[1], IP_Address.b[0]);
            Log("Initializing mDNS Service\r\n");
            mDNS_Init(ESP_Config.Name, IP_Address.l);
            Log("    mDNS Service Online\r\n");

            Log("\r\nInitializing uPnP Service\r\n");
            uPnP_Init(ESP_Config.Name, ESP_Config.UUID, IP_Address.l, 0);
            Log("   uPnP Service Online\r\n");
        }
    }
    return (EXIT_SUCCESS);
}

const char *Translate_DRESULT(int res) {
    switch (res) {
        case FR_OK:
            return "Succeeded";
            break;
        case FR_DISK_FULL:
            return "Disk Full";
            break;
        case FR_EOF:
            return "EOF";
            break;
        case FR_FILENAME_TOO_LONG:
            return "Filename too long";
            break;
        case FR_NOT_FORMATTED:
            return "Disk Not Formatted";
            break;
        case FR_NOT_FOUND:
            return "NOT FOUND";
            break;
        case FR_ERR_OTHER:
            return "ERR_OTHER";
            break;
        default:
            return "Unknown Error";
    }
}

