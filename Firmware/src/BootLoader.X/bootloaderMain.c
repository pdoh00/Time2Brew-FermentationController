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
#include <p33Exxxx.h>

/** CONFIGURATION Bits **********************************************/
_FICD(ICS_PGD3 & JTAGEN_OFF); //ICD takes place on PGD3 and PGC3 pins
_FPOR(ALTI2C1_OFF & ALTI2C2_OFF & WDTWIN_WIN75); //Do not use Alternate Pin Mapping for I2C
_FWDT(WDTPOST_PS32768 & WDTPRE_PR128 & PLLKEN_ON & WINDIS_OFF & FWDTEN_OFF); //Turn off WDT
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
#define CMD_SEND_BACKUP         0x09
#define CMD_SEND_PRIMARY        0x0A
#define CMD_WIPE                0x0B

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
    unsigned long firmwareAddressLimit = FIRMWARE_SIZE;
    firmwareAddressLimit *= 2;
    while (currentAddress.ul < firmwareAddressLimit) {
        ErasePage(currentAddress.ui[1], currentAddress.ui[0]);
        currentAddress.ul += 0x800;
    }
    Log("Done\r\n");
}

BYTE buffer[256]; //3 bytes per instruction and 64 instructions per buffer

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
    unsigned long limit = FIRMWARE_SIZE;
    limit *= 3;
    for (offset = 0; offset < limit; offset += 4096) {
        diskEraseSector(base + offset);
    }
}

void SerialUploadBlock(uint32_t base) {
    uint16_t chksum, rxChecksum;
    uint32_t offset;

    ReadBytes(buffer, 198);
    chksum = fletcher16(buffer, 196);
    rxChecksum = buffer[197];
    rxChecksum <<= 8;
    rxChecksum += buffer[196];
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
    diskWrite(offset, 192, &buffer[4]);
    TX_BYTE(ACK);

}

void SerialSendData(uint32_t base) {
    unsigned long offset = 0;

    union {
        unsigned long ul;
        unsigned char ub[4];
    } length;
    int x;
    length.ul = FIRMWARE_SIZE;
    length.ul *= 3;

    TX_BYTE(length.ub[0]);
    TX_BYTE(length.ub[1]);
    TX_BYTE(length.ub[2]);
    TX_BYTE(length.ub[3]);

    for (offset = 0; offset < length.ul; offset += 192) {
        diskRead(base + offset, 192, buffer);
        for (x = 0; x < 192; x++) {
            TX_BYTE(buffer[x]);
        }
    }
}

int serialFlash() {
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
                    Wipe(FIRMWARE_BACKUP_BLOCK_ADDRESS);
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
                    Wipe(FIRMWARE_BLOCK_ADDRESS);
                    TX_BYTE(ACK);
                } else {
                    TX_BYTE(NACK);
                }
                break;
            case CMD_UPLOAD_BACKUP:
                TX_BYTE(ACK);
                SerialUploadBlock(FIRMWARE_BACKUP_BLOCK_ADDRESS);
                break;
            case CMD_UPLOAD_PRIMARY:
                TX_BYTE(ACK);
                SerialUploadBlock(FIRMWARE_BLOCK_ADDRESS);
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
            case CMD_SEND_BACKUP:
                SerialSendData(FIRMWARE_BACKUP_BLOCK_ADDRESS);
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
            default:
                TX_BYTE(NACK);

                break;
        }
    }
}

int fileFlash(char flashBackup) {
    Log("Flash Starting: flashBackup=%b\r\n", flashBackup);
    BYTE *cursor;
    int x;
    unsigned long fileAddress;

    if (flashBackup) {
        fileAddress = FIRMWARE_BACKUP_BLOCK_ADDRESS;
        diskRead(fileAddress, 1, buffer);
        fileAddress++;
        if (buffer[0] != 0xAA) {
            Log("Backup Firmware: Signature Not Found! %xb", buffer[0]);
            asm("reset");
        }
    } else {
        fileAddress = FIRMWARE_BLOCK_ADDRESS;
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
    unsigned long FirmwareAddressLimit = FIRMWARE_SIZE;
    FirmwareAddressLimit *= 2;
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

    if (sCount > 1) serialFlash();

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


