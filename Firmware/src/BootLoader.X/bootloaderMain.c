/* 
 * File:   bootloaderMain.c
 * Author: THORAXIUM
 *
 * Created on February 11, 2015, 2:55 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "FlashFS.h"
#include "fletcherChecksum_1.h"
#include "FIFO.h"
#include "SystemConfiguration.h"
#include "circularPrintF.h"
#include "ESP_Flash.h"
#include <p33Exxxx.h>

/** CONFIGURATION Bits **********************************************/
_FICD(ICS_PGD3 & JTAGEN_OFF); //ICD takes place on PGD3 and PGC3 pins
_FPOR(ALTI2C1_OFF & ALTI2C2_OFF & WDTWIN_WIN25); //Do not use Alternate Pin Mapping for I2C
_FWDT(WDTPOST_PS256 & WDTPRE_PR128 & PLLKEN_ON & WINDIS_OFF & FWDTEN_OFF); //Turn off WDT in hardware but Timeout=1.024seconds with no Window
_FOSC(FCKSM_CSECMD & OSCIOFNC_ON & POSCMD_NONE & IOL1WAY_OFF); // Enable Clock Switching and Configure Primary Oscillator in XT mode
_FOSCSEL(FNOSC_FRC & IESO_OFF); // Select Internal FRC at POR and do not lock the PWM registers
_FGS(GWRP_OFF & GCP_OFF); //Turn off Code Protect

#define ECHO_RESPONSE 0xAB

#define ACK                 0xF1
#define NACK                0xF2
#define ACK_WAIT            0xF3
#define RQ_CONFIRM          0xF4

#define CMD_ECHO                0xA0
#define CMD_ERASE_BACKUP        0x02
#define CMD_ERASE_PRIMARY       0x03
#define CMD_CONFIRM             0x04
#define CMD_UPLOAD_BACKUP       0x05
#define CMD_UPLOAD_PRIMARY      0x06
#define CMD_FLASH_BACKUP        0x07
#define CMD_FLASH_PRIMARY       0x08
#define CMD_WIPE                0x0B
#define CMD_ESP_DIRECT          0x0C
#define CMD_TX_BLOCK            0x0D

extern void DoubleWordWrite(unsigned int PageAddress, unsigned int OffsetAddress, unsigned int *data);
extern void ErasePage(unsigned int PageAddress, unsigned int OffsetAddress);

int fileFlash(char flashBackup);

BYTE __attribute__((aligned)) logFIFOData[256];
FIFO_BUFFER logFIFO_real = {logFIFOData, logFIFOData, logFIFOData, logFIFOData + 256};
FIFO_BUFFER *logFIFO = &logFIFO_real;

union {
    unsigned long ul;
    unsigned int ui[2];
    unsigned char ub[4];
} currentAddress;

union {
    unsigned char ub[4];
} wordBuffer[2];

void EraseProgram() {
    //Erase all pages that will be programmed
    Log("---EraseProgram...");
    currentAddress.ul = 0;
    unsigned long firmwareAddressLimit = 0x27800;
    while (currentAddress.ul < firmwareAddressLimit) {
        ErasePage(currentAddress.ui[1], currentAddress.ui[0]);
        currentAddress.ul += 0x800;
    }
    Log("Done\r\n");
}

BYTE buffer[262]; //3 bytes per instruction and 64 instructions per buffer

#define RX_BYTE(x) {while (!U2STAbits.URXDA){if (U2STAbits.OERR) U2STAbits.OERR = 0;}(x)=U2RXREG;}
#define TX_BYTE(x) {while(!U2STAbits.TRMT);U2TXREG=x;}

void FlushRX() {
    BYTE dummy;
    while (U2STAbits.URXDA) dummy = U2RXREG;
}

void ReadBytes(BYTE *dst, int bCount) {
    while (bCount--) RX_BYTE(*(dst++));
}

void Wipe(uint32_t base) {
    unsigned long offset = 0;
    unsigned long limit = FIRMWARE_RESERVED_SIZE;
    for (offset = 0; offset < limit; offset += 4096) {
        diskEraseSector(base + offset);
    }
}

void SerialUploadBlock(uint32_t base) {
    uint16_t chksum, rxChecksum;
    uint32_t offset;

    ReadBytes(buffer, 262);
    chksum = fletcher16(buffer, 260);
    rxChecksum = buffer[261];
    rxChecksum <<= 8;
    rxChecksum += buffer[260];
    if (rxChecksum != chksum) {
        TX_BYTE(NACK);
        return;
    }

    offset = buffer[3];
    offset <<= 8;
    offset += buffer[2];
    offset <<= 8;
    offset += buffer[1];
    offset <<= 8;
    offset += buffer[0];

    offset += base;
    diskWrite(offset, 256, &buffer[4]);
    TX_BYTE(ACK);
}

void SerialTxBlock(uint32_t base) {
    unsigned long bCount = FIRMWARE_RESERVED_SIZE;
    unsigned long address = base;
    int x;
    while (bCount) {
        diskRead(address, 256, buffer);
        address += 256;
        bCount -= 256;
        for (x = 0; x < 256; x++) {
            TX_BYTE(buffer[x]);
        }
    }
}

int serialInterface() {
    BYTE command;

    while (1) {
        FlushRX();
        RX_BYTE(command);
        switch (command) {
            case CMD_ECHO:
                TX_BYTE(ACK);
                break;
            case CMD_ERASE_BACKUP:
                TX_BYTE(RQ_CONFIRM);
                RX_BYTE(command);
                if (command == CMD_CONFIRM) {
                    TX_BYTE(ACK_WAIT);
                    Wipe(FIRMWARE_BACKUP_ADDRESS);
                    TX_BYTE(ACK);
                } else {
                    TX_BYTE(NACK);
                }
                break;
            case CMD_ERASE_PRIMARY:
                TX_BYTE(RQ_CONFIRM);
                RX_BYTE(command);
                if (command == CMD_CONFIRM) {
                    TX_BYTE(ACK_WAIT);
                    Wipe(FIRMWARE_PRIMARY_ADDRESS);
                    TX_BYTE(ACK);
                } else {
                    TX_BYTE(NACK);
                }
                break;
            case CMD_UPLOAD_BACKUP:
                TX_BYTE(ACK);
                SerialUploadBlock(FIRMWARE_BACKUP_ADDRESS);
                break;
            case CMD_UPLOAD_PRIMARY:
                TX_BYTE(ACK);
                SerialUploadBlock(FIRMWARE_PRIMARY_ADDRESS);
                break;
            case CMD_FLASH_BACKUP:
                TX_BYTE(RQ_CONFIRM);
                RX_BYTE(command);
                if (command == CMD_CONFIRM) {
                    TX_BYTE(ACK);
                    fileFlash(1);
                } else {
                    TX_BYTE(NACK);
                }
                break;
            case CMD_FLASH_PRIMARY:
                TX_BYTE(RQ_CONFIRM);
                RX_BYTE(command);
                if (command == CMD_CONFIRM) {
                    TX_BYTE(ACK);
                    fileFlash(0);
                } else {
                    TX_BYTE(NACK);
                }
                break;
            case CMD_WIPE:
                TX_BYTE(RQ_CONFIRM);
                RX_BYTE(command);
                if (command == CMD_CONFIRM) {
                    TX_BYTE(ACK_WAIT);
                    diskEraseSecure(1);
                    diskEraseSecure(2);
                    diskEraseSecure(3);
                    diskEraseChip();
                    TX_BYTE(ACK);
                } else {
                    TX_BYTE(NACK);
                }
                break;
            case CMD_ESP_DIRECT:
                SET_WIFI_POWER(0);
                SET_WIFI_PROG(1);
                Delay(0.1);
                SET_WIFI_POWER(1);
                Delay(1);
                TX_BYTE(ACK);
                while (1) {
                    if (U1STAbits.OERR) U1STAbits.OERR = 0;
                    if (U2STAbits.OERR) U2STAbits.OERR = 0;
                    if (U1STAbits.URXDA) U2TXREG = U1RXREG;
                    if (U2STAbits.URXDA) U1TXREG = U2RXREG;
                }
                break;
            case CMD_TX_BLOCK:
                SerialTxBlock(FIRMWARE_BACKUP_ADDRESS);
                break;
            default:
                TX_BYTE(NACK);
                break;
        }
    }
}

int fileFlash(char flashBackup) {
    Log("Flash Starting: flashBackup=%b\r\n", flashBackup);
    BYTE *cursor;
    int x, ret;

    unsigned long fileAddress, dspicFirmwareByteCount;


    if (flashBackup) {
        fileAddress = FIRMWARE_BACKUP_ADDRESS;
        diskRead(fileAddress, 1, buffer);
        fileAddress++;
        if (buffer[0] != 0xAA) {
            Log("Backup Firmware: Signature Not Found! %xb", buffer[0]);
            Delay(1.0);
            asm("reset");
        }

    } else {
        fileAddress = FIRMWARE_PRIMARY_ADDRESS;
        diskRead(fileAddress, 1, buffer);
        fileAddress++;
        if (buffer[0] != 0xAA) {
            Log("Primary Firmware: Signature Not Found! %xb", buffer[0]);
            asm("reset");
        }
    }

    Log("-fileAddress=%xl", fileAddress);

    EraseProgram();
    currentAddress.ul = 0;

    diskRead(fileAddress, 4, (BYTE *) & dspicFirmwareByteCount);
    fileAddress += 4;

    unsigned long FirmwareAddressLimit = 2 * (dspicFirmwareByteCount / 3);
    while (currentAddress.ul < FirmwareAddressLimit) {
        diskRead(fileAddress, 192, buffer);
        fileAddress += 192;
        cursor = buffer;
        Log("%xl", currentAddress.ul);
        for (x = 0; x < 64; x += 2) { //Program two instructions at a time...
            Log(".");
            wordBuffer[0].ub[0] = *(cursor++);
            wordBuffer[0].ub[1] = *(cursor++);
            wordBuffer[0].ub[2] = *(cursor++);
            wordBuffer[0].ub[3] = 0;
            wordBuffer[1].ub[0] = *(cursor++);
            wordBuffer[1].ub[1] = *(cursor++);
            wordBuffer[1].ub[2] = *(cursor++);
            wordBuffer[1].ub[3] = 0;

            if (currentAddress.ul == 0) {
                wordBuffer[0].ub[0] = 0x00;
                wordBuffer[0].ub[1] = 0x78;
                wordBuffer[0].ub[2] = 0x04;
                wordBuffer[0].ub[3] = 0;
                wordBuffer[1].ub[0] = 0x02;
                wordBuffer[1].ub[1] = 0x00;
                wordBuffer[1].ub[2] = 0x00;
                wordBuffer[1].ub[3] = 0;
            }
            DoubleWordWrite(currentAddress.ui[1], currentAddress.ui[0], (unsigned int *) wordBuffer);
            currentAddress.ul += 4; //Advanced by two instructions (each instruction advances the address counter by 2)
        }
        Log("OK\r\n");
    }
    Log("Firmware Write Complete\r\n");

    Log("Connecting to ESP...");
    ret = espflash_connect();
    if (ret < 0) {
        Log("Failed\r\n");
        Delay(1);
        asm("reset");
    }
    Log("OK\r\n");

    unsigned long espOffset, espLength;

    while (1) {
        diskRead(fileAddress, 4, (BYTE *) & espOffset);
        fileAddress += 4;

        if (espOffset > 0x80000ul) break;

        diskRead(fileAddress, 4, (BYTE *) & espLength);
        fileAddress += 4;
        Log("Flashing ESP @ Offset = %xl Length=%xl\r\n", espOffset, espLength);
        for (x = 0; x < 3; x++) {
            ret = espflash_FlashFile(fileAddress, espLength, espOffset);
            if (ret < 0) {
                Log("RETRY: Flashing ESP @ Offset = %xl Length=%xl\r\n", espOffset, espLength);
            } else {
                break;
            }
        }
        fileAddress += espLength;
    }

    Log("Leave Flash\r\n");
    for (x = 0; x < 3; x++) {
        ret = espflash_FlashFinish(0);
        if (ret < 0) {
            Log("RETRY: Leave Flash\r\n");
        } else {
            break;
        }
    }


    asm("reset");
    return 0;
}

int main(int argc, char** argv) {
    SetupClock();
    SetupPortPins();
    Setup_UART();
    ff_SPI_initialize();

    int x;
    int sCount = 0;
    for (x = 0; x < 10; x++) {
        if (U2STAbits.OERR) U2STAbits.OERR = 0;
        while (U2STAbits.URXDA) {
            U2STAbits.OERR = 0;
            if (U2RXREG == CMD_ECHO) {
                sCount++;
            } else {
                sCount = 0;
            }
        }
        DELAY_40uS;
    }

    if (sCount > 1) serialInterface();

    Log("Serial Connection Not Found...\r\n");
    //Is someone holding the CFG button?
    long configCount = 0;
    while (!CFG_MODE_PORT) {
        DELAY_105uS;
        configCount++;
    }
    Log("ConfigCount=%ul\r\n", configCount);
    if (configCount > 95238) {//Hold more than 9 seconds to factory default the firmware
        fileFlash(1);
    } else if (configCount > 19000) { //Hold 2-9 seconds to flash updated firmware
        fileFlash(0);
    }

    asm("goto 0x200");
    return 0;
}


