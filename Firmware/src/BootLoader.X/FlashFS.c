#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <p33Exxxx.h>
#include "integer.h"
#include "FlashFS.h"
#include "FIFO.h"
#include "SystemConfiguration.h"

#define RANDOM_TIMER TMR4
#define FLASH_ASSERT_CS {SET_FLASH_CS(0);Nop();Nop();Nop();Nop();Nop();Nop();}
#define FLASH_RELEASE_CS {SET_FLASH_CS(1);Nop();Nop();Nop();Nop();Nop();Nop();}
#define FLASH_SPI_PORT  SPI1BUF
#define FLASH_SPI_RXFULL SPI1STATbits.SPIRBF

void SetSPIBaudRate(long targetBaud) {
    int x, y;
    long speed;
    x = 1;
    y = 1;
    SPI1STATbits.SPIEN = 0;
    while (x <= 64) {
        while (y <= 8) {
            speed = FCY / (x * y);
            if (speed <= targetBaud) {
                switch (x) {
                    case 64:
                        SPI1CON1bits.PPRE = 0b00;
                        break;
                    case 16:
                        SPI1CON1bits.PPRE = 0b01;
                        break;
                    case 4:
                        SPI1CON1bits.PPRE = 0b10;
                        break;
                    case 1:
                        if (y == 1) return;
                        SPI1CON1bits.PPRE = 0b11;
                        break;
                }
                SPI1CON1bits.SPRE = 8 - (BYTE) (y);
                SPI1STATbits.SPIEN = 1;

                return;
            }
            y++;
        }
        x *= 4;
        y = 1;
    }
}

static BYTE xchg_spi(BYTE dat) {
    FLASH_SPI_PORT = dat;
    while (!FLASH_SPI_RXFULL);
    return (BYTE) FLASH_SPI_PORT;
}

int diskRead(uint32_t address, int bCount, BYTE * out) {
    FLASH_ASSERT_CS;
    xchg_spi(0x03);
    xchg_spi((address & 0xFF0000) >> 16);
    xchg_spi((address & 0xFF00) >> 8);
    xchg_spi((address & 0xFF));
    while (bCount--) {
        //Read SPI Byte and pack into *out
        *(out++) = xchg_spi(0xFF);
    }
    FLASH_RELEASE_CS;
    return 1;
}

int diskWritePage(uint32_t address, int bCount, BYTE *data) {
    BYTE status;

    FLASH_ASSERT_CS;
    xchg_spi(0x06);
    FLASH_RELEASE_CS;

    FLASH_ASSERT_CS;
    xchg_spi(0x02);
    xchg_spi((address & 0xFF0000) >> 16);
    xchg_spi((address & 0xFF00) >> 8);
    xchg_spi((address & 0xFF));
    while (bCount--) {
        xchg_spi(*data);
        data++;
    }
    FLASH_RELEASE_CS;

    FLASH_ASSERT_CS;
    xchg_spi(0x05); //Read Status Register 1
    while (1) {
        status = xchg_spi(0xFF);
        while (FIFO_HasChars(logFIFO));
        status &= 0x01;
        if (status == 0) break;
    }
    FLASH_RELEASE_CS;
    //Log("OK\r\n");
    return 1;
}

int diskWrite(uint32_t address, int bCount, BYTE * in) {
    int pageOffset, bytesToWrite;
    while (bCount) {
        pageOffset = (address & 0xFF);
        bytesToWrite = (256 - pageOffset);
        if (bytesToWrite > bCount) bytesToWrite = bCount;
        diskWritePage(address, bytesToWrite, in);
        in += bytesToWrite;
        address += bytesToWrite;
        bCount -= bytesToWrite;
    }
    return 0;
}

int diskEraseSector(uint32_t address) {
    //Log("      diskEraseSector: Address=%xl.", address);
    BYTE buff[256];
    BYTE status;
    char isEmpty = 1;
    unsigned int x, y;
    unsigned long checkAddress = address;
    for (x = 0; x < 16; x++) {
        diskRead(checkAddress, 256, buff);
        checkAddress += 256;
        for (y = 0; y < 256; y++) {
            if (buff[y] != 0xFF) {
                isEmpty = 0;
                goto exitPoint;
            }
        }
    }

exitPoint:
    if (isEmpty) return 1;

    FLASH_ASSERT_CS;
    xchg_spi(0x06);
    FLASH_RELEASE_CS;

    FLASH_ASSERT_CS;
    xchg_spi(0x20);
    xchg_spi((address & 0xFF0000) >> 16);
    xchg_spi((address & 0xFF00) >> 8);
    xchg_spi((address & 0xFF));
    FLASH_RELEASE_CS;

    FLASH_ASSERT_CS;
    xchg_spi(0x05); //Read Status Register 1
    while (1) {
        status = xchg_spi(0xFF);
        status &= 0x01;
        if (status == 0) break;
    }
    FLASH_RELEASE_CS;
    //Log("Done\r\n");
    return 1;
}

int diskEraseSecure(char sector) {
    //Log("      diskEraseSecure: sector=%b...", sector);
    BYTE status;

    FLASH_ASSERT_CS;
    xchg_spi(0x06);
    FLASH_RELEASE_CS;

    FLASH_ASSERT_CS;
    xchg_spi(0x44); //Erase Secure Register Command
    xchg_spi(0);
    xchg_spi(sector << 4);
    xchg_spi(0);
    FLASH_RELEASE_CS;

    FLASH_ASSERT_CS;
    xchg_spi(0x05); //Read Status Register 1
    while (1) {
        status = xchg_spi(0xFF);
        status &= 0x01;
        if (status == 0) break;
    }
    FLASH_RELEASE_CS;
    //Log("Done\r\n");
    return 1;
}

void ff_SPI_initialize() {
    SPI1STATbits.SPIEN = 0;
    SPI1CON1bits.DISSCK = 0; //Internal Serial Clock Enabled
    SPI1CON1bits.DISSDO = 0; //SDO is controlled by the moudle
    SPI1CON1bits.MODE16 = 0; //8-bit Mode
    SPI1CON1bits.MSTEN = 1; //Master Mode
    SPI1STATbits.SPIROV = 0;
    SPI1CON1bits.SMP = 0; //Data is sampled in the middle of the clock
    //Setup for mode 11
    SPI1CON1bits.CKE = 0;
    SPI1CON1bits.CKP = 1;
    SPI1CON1bits.SSEN = 0; //SS is GPIO

    SPI1CON1bits.PPRE = 0b00;
    SPI1CON1bits.SPRE == 0b000;

    SPI1STATbits.SPIEN = 1; //Enable the module

    FLASH_CS_TRIS = 0;
    SET_FLASH_CS(1);
    FLASH_CS_LAT = 1;
    FLASH_CS_PORT = 1;

    SetSPIBaudRate(25000000);

    int x;
    for (x = 0; x < 2380; x++)DELAY_105uS;
}

int diskEraseChip() {
    BYTE status;

    FLASH_ASSERT_CS;
    xchg_spi(0x06);
    FLASH_RELEASE_CS;

    FLASH_ASSERT_CS;
    xchg_spi(0x60);
    FLASH_RELEASE_CS;

    //Log("      Chip Erase in progres...");
    unsigned int x;
    FLASH_ASSERT_CS;
    xchg_spi(0x05); //Read Status Register 1
    while (1) {
        status = xchg_spi(0xFF);
        //Log("EraseSector: Status=%b\r\n", status);
        status &= 0x01;
        if (status == 0) break;
        if (x++ > 50000) {
            //Log(".");
            x = 0;
        }
    }
    FLASH_RELEASE_CS;
    //Log("OK\r\n");
    return 1;
}
