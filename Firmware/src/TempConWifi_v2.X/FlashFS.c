#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <p33Exxxx.h>
#include "integer.h"
#include "FlashFS.h"
#include "ESP8266.h"
#include "SystemConfiguration.h"

#define RANDOM_TIMER TMR4
#define FLASH_ASSERT_CS {SET_FLASH_CS(0);Nop();Nop();Nop();Nop();Nop();Nop();}
#define FLASH_RELEASE_CS {SET_FLASH_CS(1);Nop();Nop();Nop();Nop();Nop();Nop();}
#define FLASH_SPI_PORT  SPI1BUF
#define FLASH_SPI_RXFULL SPI1STATbits.SPIRBF
#define FLASH_SPI_TX(x) {FLASH_SPI_PORT=(x);while(!FLASH_SPI_RXFULL);}
#define FLASH_SPI_RX(x) {FLASH_SPI_PORT=0xFF;while(!FLASH_SPI_RXFULL);(x)=FLASH_SPI_PORT;}
#define ESCAPE_CHAR 0xAA


BYTE buff[256];
uint32_t CAT_Address = 0;
ff_File FET;

int LoggingOn = 0;

static BYTE xchg_spi(BYTE dat) {
    FLASH_SPI_PORT = dat;
    while (!FLASH_SPI_RXFULL);
    return (BYTE) FLASH_SPI_PORT;
}

BYTE diskReadStatus1() {
    BYTE ret;
    FLASH_ASSERT_CS;
    xchg_spi(0x05); //Secure Register Read
    ret = xchg_spi(0xFF);
    FLASH_RELEASE_CS;
    return ret;
}

BYTE diskReadStatus2() {
    BYTE ret;
    FLASH_ASSERT_CS;
    xchg_spi(0x35); //Secure Register Read
    ret = xchg_spi(0xFF);
    FLASH_RELEASE_CS;
    return ret;
}

void diskReadMFID(unsigned char *mfId, unsigned char *devId) {
    FLASH_ASSERT_CS;
    xchg_spi(0x90);
    xchg_spi(0x0);
    xchg_spi(0x0);
    xchg_spi(0x0);
    *mfId = xchg_spi(0xFF);
    *devId = xchg_spi(0xFF);
    FLASH_RELEASE_CS;
}

int diskReadUniqueID(unsigned char *out) {
    FLASH_ASSERT_CS;
    xchg_spi(0x4B);
    xchg_spi(0x0);
    xchg_spi(0x0);
    xchg_spi(0x0);
    xchg_spi(0x0);
    out[0] = xchg_spi(0xFF);
    out[1] = xchg_spi(0xFF);
    out[2] = xchg_spi(0xFF);
    out[3] = xchg_spi(0xFF);
    out[4] = xchg_spi(0xFF);
    out[5] = xchg_spi(0xFF);
    out[6] = xchg_spi(0xFF);
    out[7] = xchg_spi(0xFF);
    FLASH_RELEASE_CS;
    return FR_OK;
}

BYTE diskReadStatus3() {
    BYTE ret;
    FLASH_ASSERT_CS;
    xchg_spi(0x15); //Secure Register Read
    ret = xchg_spi(0xFF);
    FLASH_RELEASE_CS;
    return ret;
}

int diskReadSecure(char sector, unsigned char *out) {
    //if (LoggingOn) Log("       diskReadSecure: Address=%b bCount=%i\r\n", sector);
    FLASH_ASSERT_CS;
    xchg_spi(0x48); //Secure Register Read
    xchg_spi(0x00); //Address
    xchg_spi(sector << 4); //Address
    xchg_spi(0x00); //Address
    xchg_spi(0xFF); //Dummy Byte

    int x;
    for (x = 0; x < 256; x++) {
        *(out++) = xchg_spi(0xFF);
    }
    FLASH_RELEASE_CS;
    return 1;
}

int diskWriteSecure(char sector, unsigned char *data) {
    //if (LoggingOn) Log("         diskWriteSecure: sector=%b\r\n", sector);

    BYTE status;

    FLASH_ASSERT_CS;
    xchg_spi(0x06); //Enable Write
    FLASH_RELEASE_CS;

    FLASH_ASSERT_CS;
    xchg_spi(0x42); //Secure Register WRite Command
    xchg_spi(0);
    xchg_spi(sector << 4);
    xchg_spi(0);

    int bCount = 256;
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
    Log("Done\r\n");
    return 1;
}

int diskRead(uint32_t address, int bCount, BYTE * out) {
    //Log("       diskRead: Address=%xl bCount=%i\r\n", address, bCount);
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

int diskRead_StreamToWifi(uint32_t address, int bCount) {
    //Log("       diskRead_StreamToWifi: Address=%xl bCount=%i\r\n", address, bCount);
    BYTE out;
    FLASH_ASSERT_CS;
    xchg_spi(0x03);
    xchg_spi((address & 0xFF0000) >> 16);
    xchg_spi((address & 0xFF00) >> 8);
    xchg_spi((address & 0xFF));

    int bytesCanSend = 0;

    while (bCount) {
        DISABLE_INTERRUPTS;
        bytesCanSend = FIFO_FreeSpace(txFIFO);
        ENABLE_INTERRUPTS;
        bytesCanSend--;
        bytesCanSend--;
        //Log("bytesCanSend=%i\r\n", bytesCanSend);

        if (bytesCanSend > bCount) bytesCanSend = bCount;

        while (bytesCanSend) {
            //Read SPI Byte and pack into *out
            out = xchg_spi(0xFF);
            bCount--;
            if (out == ESCAPE_CHAR) {
                FIFO_Write(txFIFO, ESCAPE_CHAR);
                bytesCanSend--;
                FIFO_Write(txFIFO, ESCAPE_CHAR);
                bytesCanSend--;
            } else {
                FIFO_Write(txFIFO, out);
                bytesCanSend--;
            }
            if (U1STAbits.TRMT) _U1TXIF = 1;
        }
        DELAY_105uS;
    }
    FLASH_RELEASE_CS;
    return 1;
}

int diskWritePage(uint32_t address, int bCount, BYTE *data) {
    //Log("         diskWritePage: Address=%xl Count=%i...\r\n", address, bCount);
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
        status &= 0x01;
        if (status == 0) break;
    }
    FLASH_RELEASE_CS;
    return 1;
}

int diskWrite(uint32_t address, int bCount, BYTE * in) {
    //Log("      diskWrite: Address=%xl bCount=%i\r\n", address, bCount);
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
    //Log("diskEraseSector: Address=%xl\r\n", address);
    //Log("Done\r\n");
    return 1;
}

int diskEraseChip() {
    BYTE status;

    FLASH_ASSERT_CS;
    xchg_spi(0x06);
    FLASH_RELEASE_CS;

    FLASH_ASSERT_CS;
    xchg_spi(0x60);
    FLASH_RELEASE_CS;

    Log("      Chip Erase in progres...");
    unsigned int x;
    FLASH_ASSERT_CS;
    xchg_spi(0x05); //Read Status Register 1
    while (1) {
        status = xchg_spi(0xFF);
        //Log("EraseSector: Status=%b\r\n", status);
        status &= 0x01;
        if (status == 0) break;
        if (x++ > 50000) {
            Log(".");
            x = 0;
        }
    }
    FLASH_RELEASE_CS;
    Log("OK\r\n");
    return 1;
}

int diskEraseUserArea() {
    unsigned long curSector;
    for (curSector = 0; curSector < SECTORCOUNT; curSector++) {
        if ((curSector & 0x3F) == 0) Log("\r\n%xl ", curSector);
        Log(".");
        diskEraseSector(curSector << 12);
    }
    Log("EraseUserAreaFinished...\r\n");
    return 1;
}

int AllocateSector(uint32_t parentSector, uint32_t *sectorAddress) {
    uint16_t link;
    //Search for a Free Sector Entry
    uint16_t randTemp;
    BYTE bTemp;
    FIFO_Read(TRNG_fifo, bTemp);
    randTemp = bTemp;
    randTemp <<= 8;
    FIFO_Read(TRNG_fifo, bTemp);
    randTemp += bTemp;

    randTemp ^= TMR4;

    *sectorAddress = randTemp % SECTORCOUNT; //Randomize the start search sector to implement wear leveling...

    if (LoggingOn) Log("   AllocateSector: Starting Point=%xl\r\n", *sectorAddress);
    int idx;
    for (idx = 0; idx < SECTORCOUNT; idx++) {
        if ((*sectorAddress) > SECTORCOUNT) *sectorAddress = 5;
        if ((*sectorAddress) < 5) *sectorAddress = 5;
        diskRead(CAT_Address + (*sectorAddress << 1), 2, (BYTE *) & link);
        if (link == 0xFFFF) { //We found a free entry!!!
            //Claim the sector and write it as the EOF for the chain
            uint16_t val = 0x7FFF;
            uint32_t lTemp = CAT_Address + ((*sectorAddress)*2);
            if (LoggingOn) Log("   Free Sector Found: %xl EntryAddress=%xl\r\n", *sectorAddress, lTemp);
            diskWrite(lTemp, 2, (BYTE*) & val);
            uint16_t check = 0;
            diskRead(lTemp, 2, (BYTE*) & check);
            if (LoggingOn) Log("*****Check Read value=%xi\r\n", check);

            if (parentSector != 0) {
                //Go to the parent Sector's entry and change it from
                //0x7FFF (EOF) to instead point at this new entry
                //which is now the EOF record...
                if (LoggingOn) Log("   Update Parent Sector @ %xl to Complete Linking\r\n", parentSector);
                uint16_t itemp = *sectorAddress;
                diskWrite(CAT_Address + (parentSector << 1), 2, (BYTE*) & itemp);
                uint16_t chk2;
                diskRead(CAT_Address + (parentSector << 1), 2, (BYTE*) & chk2);
                if (LoggingOn) Log("        Check Parent Sector=%xi Expected=%xi\r\n", chk2, itemp);
            }
            if (LoggingOn) Log("   OK\r\n");
            return FR_OK;
        } else {
            (*sectorAddress) += 1;
        }
    }
    Log("   FF_ERROR! Unable to Find Free Sector...\r\n");
    LoggingOn = 0;
    return FR_DISK_FULL;
}

int GetNextSector(uint32_t sector, uint32_t *nextSector) {

    if (sector == 0x7FFF) {
        Log("   GetNextSector: FAIL Origin Sector=0x7FFF This is the last sector in the chain...\r\n");
        return FR_EOF;
    }
    if (sector > SECTORCOUNT) {
        Log("   GetNextSector: FAIL Origin Sector Out of Bounds = %xl\r\n", sector);
        return FR_CORRUPTED;
    }

    if (sector < 4) {
        Log("   GetNextSector: FAIL Origin Sector Out of Bounds = %xl\r\n", sector);
        return FR_CORRUPTED;
    }

    unsigned int nxtSect;
    uint32_t lTemp = CAT_Address + (sector * 2);
    diskRead(lTemp, 2, (BYTE*) & nxtSect);
    (*nextSector) = (uint32_t) nxtSect;

    if (nxtSect == 0x7FFF) return FR_EOF;
    if (nxtSect > SECTORCOUNT || nxtSect < 4) {
        Log("   GetNextSector: FAIL Corrupted...Origin=%xl, Next=%xi\r\n", sector, nxtSect);
        return FR_CORRUPTED;
    }
    //Log("   GetNextSector: OK Next=%xl\r\n", (*nextSector));
    return FR_OK;
}

int FindFileEntryAddress(const char *filename, uint32_t *FileTablePosition) {
    if (LoggingOn) Log("   *********\r\nFindFileEntryAddress filename='%s'\r\n", filename);

    BYTE fnameLength = strlen(filename);
    int bRead;

    ff_Seek(&FET, 0, ff_SeekMode_Absolute);

    while (1) {
        //Read the first byte to see if this is a valid entry
        *FileTablePosition = FET.Position;
        ff_Read(&FET, buff, 1, &bRead);
        if (buff[0] == 0) {
            if (LoggingOn) Log("   %xl: 0x00 = Erased Entry\r\n", *FileTablePosition);
            ff_Seek(&FET, 255, ff_SeekMode_Relative);
            //Erased entry... do nothing with it
        } else if (buff[0] == 0xFF) {
            if (LoggingOn) Log("   %xl: 0xFF=Found End Of Table Entry\r\n", *FileTablePosition);
            //This marks the end of the FileTable...
            return FR_NOT_FOUND;
        } else if (buff[0] > 123) {
            if (LoggingOn) Log("   CORRUPTED! Entry = Filename is too long...\r\n");
            return FR_CORRUPTED;
        } else if (buff[0] == fnameLength) { //Seems Valid so spend the time to read the entry
            if (LoggingOn) Log("   %xl:Filename Length Matches = %ub\r\n", *FileTablePosition, buff[0]);

            //See if we have a filename match...
            ff_Read(&FET, buff, fnameLength, &bRead);
            if (memcmp(buff, filename, fnameLength) == 0) {
                if (LoggingOn) Log("        Exact Entry Matches. \r\n");
                return FR_OK;
            } else {
                if (LoggingOn) Log("        Exact Entry Does NOT Match.\r\n");
                ff_Seek(&FET, 255 - fnameLength, ff_SeekMode_Relative);
            }
        } else {

            if (LoggingOn) Log("   %xl: Filename Length Does Not Match\r\n", *FileTablePosition);
            ff_Seek(&FET, 255, ff_SeekMode_Relative);
        }
    }
}

int SeekToEndOfFET(ff_File *fet) {
    int itemp;
    ff_Seek(fet, 0, ff_SeekMode_Absolute);
    while (1) {
        ff_Read(fet, buff, 1, &itemp);
        if (buff[0] == 0xFF) {
            fet->Position -= 1;
            fet->SectorOffset -= 1;
            return FR_OK;
        }
        ff_Seek(fet, 255, ff_SeekMode_Relative);
    }
}

int CreateFile(const char *filename, ff_File *file) {
    Log("Creating File: '%s'\r\n", filename);

    union {
        unsigned long ul;
        unsigned int ui[2];
        int i[2];
        unsigned char ub[4];
    } temp;

    //Clear the File Record
    memset(buff, 0, 128);
    memset(buff + 128, 0xFF, 128);
    BYTE *cursor = buff;

    //Set the Length of the filename
    *(cursor++) = strlen(filename);

    //Write the filename
    if (buff[0] > 123) {
        Log("   -Filename Too Long = %ub\r\n", buff[0]);
        return FR_FILENAME_TOO_LONG;
    }

    sprintf((char *) cursor, "%s", filename);
    sprintf(file->FileName, "%s", filename);
    cursor += 123;

    //Write the UID
    temp.ui[0] = TMR4;
    *(cursor++) = temp.ub[0];
    *(cursor++) = temp.ub[1];
    file->UID = temp.ui[0];

    int res;
    res = AllocateSector(0, &temp.ul);
    //Log("   -AllocateSector res=%i, ZeroParent, OriginSector=%ui\r\n", res, temp.ui);
    if (res != FR_OK) return res;
    *(cursor++) = temp.ub[0];
    *(cursor++) = temp.ub[1];
    file->OriginSector = temp.ul;

    SeekToEndOfFET(&FET);
    file->FileEntryAddress = FET.Position;
    buff[0] = strlen(filename);
    ff_Append(&FET, buff, 255, &temp.i[0]);
    //    Log("FET Entry Data: ");
    //    int p;
    //    for (p = 0; p < 256; p++) {
    //        Log("%xb ", buff[p]);
    //    }
    //    Log("\r\n");

    file->SectorOffset = 0;
    file->Position = 0;
    file->CurrentSector = file->OriginSector;
    file->Length = 0;

    res = ff_Seek(&FET, file->FileEntryAddress, ff_SeekMode_Absolute);
    if (res != FR_OK) return res;

    res = ff_Read(&FET, buff, 255, &temp.i[0]);
    if (res != FR_OK) return res;

    //    Log("***VERIFY RECORD BYTES: ");
    //    for (p = 0; p < 255; p++) Log("%xb ", buff[p]);
    //    Log("\r\n\r\n");


    return FR_OK;
}

int EraseSectorChain(uint32_t OriginSector) {
    int ret;
    uint16_t val = 0;
    Log("   EraseSectorChain: ");
    uint32_t nextSector, currentSector;
    currentSector = OriginSector;
    while (1) {
        ret = GetNextSector(currentSector, &nextSector);
        if (ret == FR_EOF) return FR_OK;
        if (ret != FR_OK) return ret;
        ret = diskEraseSector(currentSector << 12);
        Log("%xi->", (unsigned int) currentSector);
        if (ret != FR_OK) return ret;
        diskWrite(CAT_Address + (currentSector * 2), 2, (BYTE*) & val);
        if (nextSector == 0x7FFF) return FR_OK;
        currentSector = nextSector;
    }
    Log(" Done\r\n");
}

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
                Log("SPI Speed Set X=%i Y=%i...", x, y);
                return;
            }
            y++;
        }
        x *= 4;
        y = 1;
    }
    Log("SPI Speed Set - Unable to find speed setting...");
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
    SPI1CON1bits.CKE = 1;
    SPI1CON1bits.CKP = 0;
    SPI1CON1bits.SSEN = 0; //SS is GPIO

    FLASH_CS_TRIS = 0;
    SET_FLASH_CS(1);
    FLASH_CS_LAT = 1;
    FLASH_CS_PORT = 1;

    SetSPIBaudRate(30000000); //30Mhz Clock Speed
    Delay(0.25);
}

int ff_Initialize() {
    CAT_Address = 0;
    diskRead(CAT_Address, 1, buff);
    if (buff[0] != 0xAA) { //0xF0 = Active Table 0x00 = Tombstoned, 0xFF = EMPTY
        //Use alternate FAT Table
        CAT_Address = 0x2000;
        diskRead(CAT_Address, 1, buff);
        if (buff[0] != 0xAA) {
            Log("Unable to find Signature '0xAA'\r\n");
            return FR_NOT_FORMATTED;
        }
    }

    uint16_t itemp;
    diskRead(CAT_Address + 2, 2, (BYTE *) & itemp);
    FET.OriginSector = itemp;
    FET.CurrentSector = FET.OriginSector;
    FET.FileEntryAddress = 0;
    FET.UID = 0;
    FET.Position = 0;
    FET.SectorOffset = 0;

    Log("FlashFS_Initialize: CAT_Address=%xl\r\n\r\n", CAT_Address);

    return FR_OK;
}

int ff_Format() {
    Log("Format is starting erase user area...\r\n");
    diskEraseUserArea();
    Log("Mark Cluster Table 0 as Active\r\n");
    unsigned int temp = 0xAAAA;
    CAT_Address = 0x0000;
    diskWrite(CAT_Address, 2, (BYTE *) & temp);

    Log("Set FileTableCluster to 0x0004\r\n\r\n");
    temp = 0x0004;
    diskWrite(CAT_Address + 2, 2, (BYTE *) & temp);

    temp = 0x7FFF;
    diskWrite(CAT_Address + (4 * 2), 2, (BYTE *) & temp);

    temp = 0x0004;

    uint16_t itemp;
    diskRead(CAT_Address + 2, 2, (BYTE *) & itemp);
    FET.OriginSector = itemp;
    FET.CurrentSector = FET.OriginSector;
    FET.FileEntryAddress = 0;
    FET.UID = 0;
    FET.Position = 0;
    FET.SectorOffset = 0;

    return 1;
}

int ff_OpenByEntryAddress(ff_File *file, unsigned long FileTablePosition) {

    if (LoggingOn) Log("ff_OpenByEntryAddress: FileEntryAddress=%xl\r\n", FileTablePosition);

    union {
        unsigned long ul;
        unsigned int ui[2];
        int i[2];
        unsigned char ub[4];
    } tempLength;

    unsigned char *cursor = buff;
    file->FileEntryAddress = FileTablePosition;
    ff_Seek(&FET, FileTablePosition, ff_SeekMode_Absolute);
    ff_Read(&FET, buff, 256, &tempLength.i[0]);

    if (*cursor > 123 || *cursor == 0x00) {
        Log("ff_OpenByEntryAddress: Record is Either Deleted or Empty! Token=%xb\r\n", *cursor);
        return FR_CORRUPTED;
    }
    cursor += 1;

    memcpy(&file->FileName, cursor, 123); //Copy the Filename string;
    cursor += 123;
    if (LoggingOn) Log("---Filename='%s'\r\n", file->FileName);

    memcpy(&file->UID, cursor, 2); //Copy the UID
    cursor += 2;
    if (LoggingOn) Log("---UID=%ui\r\n", file->UID);

    unsigned int iTemp;
    memcpy(&iTemp, cursor, 2); //Copy the Origin Sector Address;
    cursor += 2;
    file->OriginSector = iTemp;
    if (file->OriginSector < 5 || file->OriginSector > 4095) {
        if (LoggingOn) Log("---CORRUPTED!!! OriginSector=%xl\r\n", file->OriginSector);
        return FR_CORRUPTED;
    }

    if (LoggingOn) Log("---OriginSector=%xl\r\n", file->OriginSector);

    //Zero out the cursor positions in the file handle
    file->Position = 0;
    file->CurrentSector = file->OriginSector;
    file->Length = 0;
    file->SectorOffset = 0;

    int idx;
    for (idx = 0; idx < 42; idx++) {
        tempLength.ub[0] = *(cursor++);
        tempLength.ub[1] = *(cursor++);
        tempLength.ub[2] = *(cursor++);
        tempLength.ub[3] = 0;
        if (tempLength.ul == 0xFFFFFFul) {
            break;
        } else {
            file->Length = tempLength.ul;
        }
    }
    if (LoggingOn) Log("---Length=%ul\r\n\r\n", file->Length);

    return FR_OK;
}

int ff_OpenByFileName(ff_File *file, const char *filename, char CreateIfNotFound) {
    uint32_t FileTablePosition;
    int res;
    if (LoggingOn) Log("ff_OpenByFileName: filename='%s'\r\n", filename);
    res = FindFileEntryAddress(filename, &FileTablePosition);
    if (LoggingOn) Log("ff_OpenByFileName: res=%i, FileEntryAddress=%xl\r\n", res, FileTablePosition);
    switch (res) {
        case FR_OK:
            if (LoggingOn) Log("File Found - Opening...\r\n");
            return ff_OpenByEntryAddress(file, FileTablePosition);
            break;
        case FR_NOT_FOUND:
            if (CreateIfNotFound) {
                if (LoggingOn) Log("File NOT Found - Creating...\r\n");
                return CreateFile(filename, file);
            } else {
                if (LoggingOn) Log("File NOT Found - ERROR...\r\n");
                return FR_NOT_FOUND;
            }
            break;
        default:
            return res;
    }
}

int ff_DeleteByHandle(ff_File *file) {
    uint16_t val = 0;
    int itemp;
    int res;
    if (LoggingOn) Log("ff_DeleteByHandle: Filename='%s'\r\n", file->FileName);

    //First Clobber the File Entry
    res = ff_Seek(&FET, file->FileEntryAddress, ff_SeekMode_Absolute);
    if (res != FR_OK) return res;

    res = ff_Append(&FET, (BYTE *) & val, 2, &itemp);
    if (res != FR_OK) return res;
    //Now mark the sector chain for deletion
    return res = EraseSectorChain(file->OriginSector);
}

int ff_Delete(const char *filename) {
    ff_File temp;
    Log("ff_Delete: Filename='%s'\r\n", filename);
    int res = ff_OpenByFileName(&temp, filename, 0);
    if (res != FR_OK) return res;

    return ff_DeleteByHandle(&temp);
}

int ff_Overwrite(ff_File *file, const char *filename) {
    Log("ff_Overwrite: Filename='%s'", filename);
    int res = ff_Delete(filename);
    if (res != FR_OK) return res;

    return ff_OpenByFileName(file, filename, 1);
}

int ff_Read_StreamToWifi(ff_File *file, int bCount) {
    int bytesToRead;
    uint32_t nextSector;
    int res;
    while (bCount) {
        bytesToRead = 0x1000 - file->SectorOffset;
        if (bytesToRead > bCount) bytesToRead = bCount;

        diskRead_StreamToWifi((file->CurrentSector << 12) + file->SectorOffset, bytesToRead);
        bCount -= bytesToRead;
        file->Position += bytesToRead;
        if (file->Position > file->Length) file->Length = file->Position;
        file->SectorOffset += bytesToRead;

        //Did we cross over the sector boundary?
        if (file->SectorOffset >= 0x1000) {
            //Log("Sector Change: SectorOffset=%xl\r\n", file->SectorOffset);
            //Okay get the next sector we should be writing to...
            res = GetNextSector(file->CurrentSector, &nextSector);
            if (res != FR_OK && res != FR_EOF) return res;
            //Is this the last sector that's been allocated?
            if (nextSector == 0x7FFF) {
                //Yep, so let's allocated one more...
                res = AllocateSector(file->CurrentSector, &nextSector);
                if (res != FR_OK) return res;
            }
            file->CurrentSector = nextSector;
            file->SectorOffset = 0;
        }
    }
    return FR_OK;
}

int ff_Read(ff_File *file, BYTE *out, int bCount, int *bytesRead) {
    //Log("\r\nff_Read:");
    int bytesToRead;
    uint32_t nextSector;
    int res;
    *bytesRead = 0;
    while (bCount) {
        bytesToRead = 0x1000 - file->SectorOffset;
        //        Log("bCount=%i, FilePosition=%xl, Sector=%xl, SectorOffset=%xl, bytesToRead=%i\r\n",
        //                bCount, file->Position, file->CurrentSector, file->SectorOffset, bytesToRead);
        if (bytesToRead > bCount) bytesToRead = bCount;

        diskRead((file->CurrentSector << 12) + file->SectorOffset, bytesToRead, out);
        out += bytesToRead;
        *bytesRead += bytesToRead;
        bCount -= bytesToRead;
        file->Position += bytesToRead;
        if (file->Position > file->Length) file->Length = file->Position;
        file->SectorOffset += bytesToRead;

        //Did we cross over the sector boundary?
        if (file->SectorOffset >= 0x1000) {
            //Log("Sector Change: SectorOffset=%xl\r\n", file->SectorOffset);
            //Okay get the next sector we should be writing to...
            res = GetNextSector(file->CurrentSector, &nextSector);
            if (res != FR_OK && res != FR_EOF) return res;
            //Is this the last sector that's been allocated?
            if (nextSector == 0x7FFF) {
                //Yep, so let's allocated one more...
                res = AllocateSector(file->CurrentSector, &nextSector);
                if (res != FR_OK) return res;
            }
            file->CurrentSector = nextSector;
            file->SectorOffset = 0;
        }
    }
    return FR_OK;
}

int ff_Append(ff_File *file, BYTE *dataToWrite, int bCount, int *bytesWritten) {
    if (LoggingOn) Log("ff_Append:\r\n");
    int bytesToWrite;
    uint32_t nextSector;
    int res;
    *bytesWritten = 0;
    while (bCount) {
        bytesToWrite = 0x1000 - file->SectorOffset;
        if (LoggingOn) Log("bCount=%i, FilePosition=%xl, Sector=%xl, SectorOffset=%xl, bytesToWrite=%i\r\n",
                bCount, file->Position, file->CurrentSector, file->SectorOffset, bytesToWrite);
        if (bytesToWrite > bCount) bytesToWrite = bCount;
        diskWrite((file->CurrentSector << 12) + file->SectorOffset, bytesToWrite, dataToWrite);
        dataToWrite += bytesToWrite;
        *bytesWritten += bytesToWrite;
        bCount -= bytesToWrite;
        file->Position += bytesToWrite;
        if (file->Position > file->Length) file->Length = file->Position;
        file->SectorOffset += bytesToWrite;
        //Did we cross over the sector boundary?
        if (file->SectorOffset >= 0x1000) {
            if (LoggingOn) Log("Sector Change: SectorOffset=%xl\r\n", file->SectorOffset);
            //Okay get the next sector we should be writing to...
            res = GetNextSector(file->CurrentSector, &nextSector);
            if (res != FR_OK && res != FR_EOF) return res;
            //Is this the last sector that's been allocated?
            if (nextSector == 0x7FFF) {
                //Yep, so let's allocated one more...
                res = AllocateSector(file->CurrentSector, &nextSector);

                if (res != FR_OK) return res;
            }
            file->CurrentSector = nextSector;
            file->SectorOffset = 0;
        }
    }
    return FR_OK;
}

int ff_UpdateLength(ff_File *file) {
    if (LoggingOn) Log("\r\nUpdate Length: '%s'\r\n", file->FileName);
    int res, idx, bRead;
    unsigned int zero = 0;

    union {
        unsigned long ul;
        unsigned char ub[4];
    } temp;
    temp.ul = 0;

    res = ff_Seek(&FET, file->FileEntryAddress, ff_SeekMode_Absolute);
    if (res != FR_OK) return res;

    res = ff_Read(&FET, buff, 256, &bRead);
    if (res != FR_OK) return res;

    BYTE *cursor = buff + 128;
    for (idx = 0; idx < 42; idx++) {
        if ((*(cursor) == 0xFF) && (*(cursor + 1) == 0xFF) && (*(cursor + 2) == 0xFF)) {
            if (temp.ul != file->Length) {
                temp.ul = file->Length;
                *(cursor++) = temp.ub[0];
                *(cursor++) = temp.ub[1];
                *(cursor++) = temp.ub[2];
                if (LoggingOn) Log("Free Slot Found At Index=%i\r\n", idx);

                res = ff_Seek(&FET, file->FileEntryAddress, ff_SeekMode_Absolute);
                if (res != FR_OK) return res;

                res = ff_Append(&FET, buff, 256, &bRead);
                if (res != FR_OK) return res;
            }
            return FR_OK;
        } else {
            temp.ub[0] = *(cursor++);
            temp.ub[1] = *(cursor++);
            temp.ub[2] = *(cursor++);
            temp.ub[3] = 0;
        }
    }

    if (LoggingOn) Log("No Free Slots in File Record... Creating new...\r\n");

    //We didn't find a free length entry... so let's create a new entry record...
    if (LoggingOn) Log("Zero Existing Entry\r\n");

    //Clobber the existing file entry...
    res = ff_Seek(&FET, file->FileEntryAddress, ff_SeekMode_Absolute);
    if (res != FR_OK) return res;
    res = ff_Append(&FET, (BYTE *) & zero, 2, &bRead);
    if (res != FR_OK) return res;


    //Clear the file lengths from the old entry
    cursor = &buff[128];
    memset(cursor, 0xFF, 128);

    //Write the length as the first entry
    temp.ul = file->Length;
    *(cursor++) = temp.ub[0];
    *(cursor++) = temp.ub[1];
    *(cursor++) = temp.ub[2];

    //Now write the clean entry to disk
    //Ask the system to get the entry...since we clobbered it we'll get a not found response
    //But the address returned will be located at the tail of the file entry table...
    if (LoggingOn) Log("Find the end of the File Entry Table...\r\n");
    SeekToEndOfFET(&FET);
    file->FileEntryAddress = FET.Position;
    buff[0] = strlen(file->FileName);
    res = ff_Append(&FET, buff, 255, &bRead);
    if (res != FR_OK) return res;

    return FR_OK;
}

int ff_Seek(ff_File *file, int32_t offset, ff_SeekMode mode) {
    if (LoggingOn) Log("ff_Seek_fw: offset=%l\r\n", offset);

    if (mode == ff_SeekMode_Absolute) {
        file->Position = 0;
        file->CurrentSector = file->OriginSector;
        file->SectorOffset = 0;
    } else {
        offset = file->Position + offset;
        file->Position = 0;
        file->CurrentSector = file->OriginSector;
        file->SectorOffset = 0;
    }

    int bytesToRead;
    uint32_t nextSector;
    int res;
    while (offset) {
        bytesToRead = 0x1000 - file->SectorOffset;
        if (LoggingOn) Log("bytesToRead=%i, Sector=%ui, SectorOffset=%ui\r\n", bytesToRead, file->CurrentSector, file->SectorOffset);
        if (bytesToRead > offset) bytesToRead = offset;
        offset -= bytesToRead;
        file->Position += bytesToRead;
        if (file->Position > file->Length) file->Length = file->Position;
        file->SectorOffset += bytesToRead;

        //Did we cross over the sector boundary?
        if (file->SectorOffset >= 0x1000) {
            if (LoggingOn) Log("Sector Change: SectorOffset=%i\r\n", file->SectorOffset);
            //Okay get the next sector we should be writing to...
            res = GetNextSector(file->CurrentSector, &nextSector);
            //Is this the last sector that's been allocated?
            if (nextSector == 0x7FFF) {
                //Yep, so let's allocated one more...
                res = AllocateSector(file->CurrentSector, &nextSector);
                if (res != FR_OK) return res;
            }
            file->CurrentSector = nextSector;
            file->SectorOffset = 0;
        }
    }
    if (LoggingOn) Log("OK\r\n");

    return FR_OK;
}

int ff_OpenDirectoryListing(ff_File * DirectoryIfo) {
    DirectoryIfo->CurrentSector = FET.OriginSector;
    DirectoryIfo->OriginSector = FET.OriginSector;
    DirectoryIfo->Position = 0;
    DirectoryIfo->SectorOffset = 0;
    return FR_OK;
}

int ff_GetNextEntryFromDirectory(ff_File *DirectoryIfo, char *fname) {
    int bytesRead, res;
    while (1) {
        //Read the first byte to see if this is a valid entry
        res = ff_Read(DirectoryIfo, buff, 256, &bytesRead);
        if (res != FR_OK) {
            return res;
        } else if (buff[0] == 0) {
            //Erased entry... do nothing with it
        } else if (buff[0] == 0xFF) {
            //This is the EOF record
            return FR_EOF;
        } else if (buff[0] < 125) {
            memcpy(fname, &buff[1], buff[0]);
            fname[buff[0]] = 0;
            return FR_OK;
        } else {
            return FR_CORRUPTED;
        }
    }
}

int ff_Trim() {
    uint16_t Entry, iTemp;
    int res;

    //Get the New SAT Table Setup
    uint32_t oldSAT, newSAT;
    oldSAT = CAT_Address;
    if (oldSAT == 0) {
        newSAT = 0x2000;
    } else {
        newSAT = 0x0;
    }
    Log("TRIM START\r\n");
    Log("   newSAT Cluster=%xi\r\n", (unsigned int) newSAT);
    diskEraseSector(newSAT);
    diskEraseSector(newSAT + 0x1000);

    //Create a new File Entry Table
    ff_File NewFileEntryTable;
    res = AllocateSector(0, &NewFileEntryTable.OriginSector);
    Log("  Set New FileTableCluster=%xi\r\n", (uint16_t) NewFileEntryTable.OriginSector);
    NewFileEntryTable.CurrentSector = NewFileEntryTable.OriginSector;
    NewFileEntryTable.Position = 0;
    NewFileEntryTable.SectorOffset = 0;

    //Point the New FAT Table to the New File Entry Table
    iTemp = NewFileEntryTable.OriginSector;
    diskWrite(newSAT + 2, 2, (BYTE *) & iTemp);

    //Condense the File Entry Old Table into the New Table...
    Log("Starting Compaction of File Entry Table\r\n");
    int bytesRead, bytesWritten;

    ff_Seek(&FET, 0, ff_SeekMode_Absolute);

    while (1) {
        res = ff_Read(&FET, buff, 256, &bytesRead);
        if (res == FR_EOF) {
            Log("DONE - End Of Table Reached\r\n");
            break;
        } else if (res != FR_OK) {
            Log("@%xl - ERROR = \"%s\"\r\n", FET.Position - 256, Translate_DRESULT(res));
            return res;
        }

        if (buff[0] == 0xFF) {
            Log("DONE - End Of Table Reached\r\n");
            break;
        } else if (buff[0] == 0) {
            Log("Deleted Entry:  Filename='%S...'\r\n", 60, &buff[1]);
        } else if (buff[0] < 121) {
            Log("Valid Entry: Filename='%S'\r\n", buff[0], &buff[1]);
            ff_Append(&NewFileEntryTable, buff, 256, &bytesWritten);
        } else {
            Log("@%xl - Invalid Entry = %xb\r\n", FET.Position - 256, buff[0]);
        }
    }

    //Copy all the Valid Non-Tombstoned SAT entries over.
    uint16_t idx;
    Log("0x0000: XXXXX");
    for (idx = 5; idx < SECTORCOUNT; idx++) {
        if ((idx & 0x3F) == 0) {
            Log("\r\n%xi: ", idx);
        }
        diskRead(oldSAT + (idx * 2), 2, (BYTE *) & Entry);
        if (Entry == 0) {
            Log("0");
        } else if (Entry == 0xFFFF) {
            Log("_");
        } else if (Entry == 0x7FFF) {
            Log("X");
            diskWrite(newSAT + (idx * 2), 2, (BYTE *) & Entry);
        } else if (Entry < SECTORCOUNT) {
            Log("x");
            diskWrite(newSAT + (idx * 2), 2, (BYTE *) & Entry);
        } else {
            Log("!");
        }
    }
    Log("\r\n");

    //Mark the new CAT as Active
    Log("Mark Sector Table %xl as Active\r\n", newSAT);
    unsigned int temp = 0xAAAA;
    diskWrite(newSAT, 2, (BYTE *) & temp);

    //And Make it Effective
    CAT_Address = newSAT;
    FET.OriginSector = NewFileEntryTable.OriginSector;
    FET.CurrentSector = FET.OriginSector;
    FET.Position = 0;
    FET.SectorOffset = 0;

    Log("Erase OLD CAT\r\n");
    diskEraseSector(oldSAT);
    diskEraseSector(oldSAT + 0x1000);

    Log("TRIM Complete\r\n");
    return FR_OK;
}

int ff_RepairFS() {
    BYTE LinkMap[512];
    int res;
    ff_File dir;
    uint32_t curSector, nxtSector;
    uint16_t val = 0;
    int itemp;
    dir.OriginSector = FET.OriginSector;
    dir.CurrentSector = FET.OriginSector;
    dir.SectorOffset = 0;
    dir.Position = 0;

    char isGood = 0;
    unsigned int mapIdx = 0;
    unsigned char mapBitIdx, mapMask;
    uint32_t fileEntryAddress = 0;
    memset(LinkMap, 0, 512);
    Log("SCANNING FILE TABLE\r\n");
    while (1) {
        fileEntryAddress = dir.Position;
        res = ff_Read(&dir, buff, 256, &itemp);
        if (buff[0] == 0) {
            Log("Deleted Entry: Filename='%S'\r\n", 120, &buff[1]);
            continue;
        } else if (buff[0] == 0xFF) {
            Log("\r\n***End Of Table Reached***\r\n");
            //This is the end of the file table...
            break;
        } else if (buff[0] > 125) {
            //Corrupted Entry...
            Log("Entry @%xl is corrupted (invalid first byte=%xb) Filename='%S'\r\n", dir.Position - 256, buff[0], 120, &buff[1]);
            isGood = 0;
        } else {
            Log("\r\nCluster Chain For Filename='%S'\r\n   ", buff[0], &buff[1]);
            memcpy((BYTE *) & itemp, &buff[126], 2);
            curSector = itemp;
            while (1) {
                itemp = curSector;
                Log("%xi->", itemp);
                res = GetNextSector(curSector, &nxtSector);
                if (res == FR_EOF) {
                    Log("OK END\r\n");
                    isGood = 1;
                    break;
                } else if (res != FR_OK) {
                    Log("CORRUPTED (Res=%s)\r\n", Translate_DRESULT(res));
                    isGood = 0;
                    break;
                } else if (nxtSector < 5 || nxtSector > SECTORCOUNT) {
                    itemp = nxtSector;
                    Log("CORRUPTED (Bad NxtSector=%xi)\r\n", itemp);
                    isGood = 0;
                    break;
                } else {
                    curSector = nxtSector;
                }
            }
        }

        if (isGood == 1) {
            memcpy((BYTE *) & itemp, &buff[126], 2);
            curSector = itemp;
            while (1) {
                mapIdx = (curSector & 0b1111111111111000) >> 3;
                mapBitIdx = (curSector & 0b111);
                mapMask = 1 << mapBitIdx;
                LinkMap[mapIdx] = LinkMap[mapIdx] | mapMask; //Set the bit in the field to indicate it is in use.
                res = GetNextSector(curSector, &nxtSector);
                if (res != FR_OK) break;
                if (nxtSector < 5 || nxtSector > SECTORCOUNT) break;
                curSector = nxtSector;
            }
        } else {
            ff_Seek(&FET, fileEntryAddress, ff_SeekMode_Absolute);
            ff_Append(&FET, (BYTE *) & val, 2, &itemp);
        }
    }

    //Add the sectors for the File Entry Table Itself!
    Log("Add FileEntryTable Sector Map\r\n   ");
    curSector = FET.OriginSector;
    while (1) {
        itemp = curSector;
        Log("%xi->", itemp);
        mapIdx = (curSector & 0b1111111111111000) >> 3;
        mapBitIdx = (curSector & 0b111);
        mapMask = 1 << mapBitIdx;
        LinkMap[mapIdx] = LinkMap[mapIdx] | mapMask; //Set the bit in the field to indicate it is in use.
        res = GetNextSector(curSector, &nxtSector);
        if (res != FR_OK) {
            Log("OK END\r\n");
            break;
        }
        curSector = nxtSector;
    }

    //The LinkMap should now be a complete mapping of all "allocated" sectors
    //Any orphans in the table will not  have their bit set.
    uint16_t SATEntry;
    Log("\r\n0x0000: 11111");
    for (curSector = 5; curSector < SECTORCOUNT; curSector++) {
        itemp = curSector;
        if ((itemp & 0x3F) == 0) {
            Log("\r\n%xi: ", itemp);
        }
        mapIdx = (curSector & 0b1111111111111000) >> 3;
        mapBitIdx = (curSector & 0b111);
        mapMask = 1 << mapBitIdx;
        if ((LinkMap[mapIdx] & mapMask) != 0) {
            Log("1");
        } else {
            //The entry is not part of any file chain
            diskRead(CAT_Address + (curSector * 2), 2, (BYTE *) & SATEntry);
            if (SATEntry == 0xFFFF || SATEntry == 0x0000) {
                Log("_");
            } else {
                Log("X");
                //The entry is incorrectly marked as allocated! So Erase it and mark
                //the cluster as TombStoned
                diskEraseSector(curSector << 12);
                SATEntry = 0x0000;
                diskWrite(CAT_Address + (curSector * 2), 2, (BYTE *) & SATEntry);
            }
        }
    }
    //At this point all files are non-corrupted AND the Sector Allocation Table is "clean"
    //We should TRIM!
    Log("\r\nStarting Trim\r\n");
    return ff_Trim();
}

int ff_exists(const char *fname) {
    ff_File temp;
    if (ff_OpenByFileName(&temp, fname, 0) != FR_OK) return 0;
    return 1;
}

int ff_GetUtilization(unsigned long *freeSpace, unsigned long *TrimSpace, unsigned long *UsedSpace) {
    uint16_t temp;
    uint16_t x;
    uint32_t fs = 0;
    uint32_t ts = 0;
    uint32_t us = 0;
    for (x = 0; x < SECTORCOUNT; x++) {
        diskRead(CAT_Address + (x * 2), 2, (BYTE *) & temp);
        if (temp == 0xFFFF) {
            fs += 4096;
        } else if (temp == 0) {
            ts += 4096;
        } else {
            us += 4096;
        }
    }
    *freeSpace = fs;
    *TrimSpace = ts;
    *UsedSpace = us;
    return FR_OK;
}

int ff_CheckFS() {
    ff_File dir;
    ff_File *DirectoryIfo = &dir;
    uint32_t originSector, nextSector, curSector;
    uint16_t itemp;
    int bytesRead, res;
    int fCount = 0;
    char *filename = (char *) &buff[1];

    res = ff_OpenDirectoryListing(&dir);
    if (res != FR_OK) return res;
    while (1) {
        //Read the first byte to see if this is a valid entry
        res = ff_Read(DirectoryIfo, buff, 256, &bytesRead);
        if (res != FR_OK) {
            Log("Unable to Read Entry res=\"%s\"", Translate_DRESULT(res));
            return res;
        } else if (buff[0] == 0) {
            //Log("Skipping Deleted Entry\r\n");
            //Erased entry... do nothing with it
        } else if (buff[0] == 0xFF) {
            //This is the EOF record
            Log("File Check Complete! FileCount=%i\r\n", fCount);
            return FR_OK;
        } else if (buff[0] < 125) {
            fCount++;
            if (fCount > 32766) {
                Log("Max File Count Exceeded... Something is wrong...\r\n");
                return FR_CORRUPTED;
            }

            if (strlen(filename) == 0) {
                Log("Error: @%xl Filename is zero length\r\n", DirectoryIfo->Position - 256);
                return FR_CORRUPTED;
            }

            memcpy((unsigned char *) &itemp, &buff[126], 2);
            originSector = itemp;
            if (originSector < 4 || originSector > SECTORCOUNT) {
                Log("Error: @%xl Filename=\"%s\" Origin Sector is out of bounds = %xl\r\n", DirectoryIfo->Position - 256, &buff[1], originSector);
                return FR_CORRUPTED;
            }

            //Log("Chain Check: \"%s\"\r\n", &buff[1]);
            curSector = originSector;
            while (1) {
                res = GetNextSector(curSector, &nextSector);
                if (res == FR_EOF) {
                    break;
                } else if (res != FR_OK) {
                    Log("Error Following Cluster Chain...res=\"%s\"", Translate_DRESULT(res));
                    return res;
                } else {
                    if (nextSector < 5 || nextSector > SECTORCOUNT) {
                        Log("nextSector is out of bounds! = %xl\r\n", nextSector);
                        return FR_CORRUPTED;
                    } else {
                        curSector = nextSector;
                    }
                }
            }
        } else {
            Log("Error: File @ %xl Has an illegal first byte=%xb\r\n", DirectoryIfo->Position - 256, buff[0]);
            return FR_CORRUPTED;
        }
    }
}



#ifdef FF_TEST
#define VERIFY_NON_ZERO(z,a) {\
    for(x=0; x < a; x++){ fftest_buffer[x]=x % 255;}\
    diskWrite((z), a, fftest_buffer);\
    diskRead((z), a, fftest_buffer);\
    for (x = 0; x < a; x++) {\
        if (fftest_buffer[x] != x % 255) {\
            Log("Failure to Verify CORRECT at offset:%i Actual=%xb, Expected=%xb\r\n", x,fftest_buffer[x],x % 255);\
            while (1);\
        }\
    }\
}

#define VERIFY_ZERO(z,a) {\
    memset(fftest_buffer, 0, a);\
    diskWrite((z), a, fftest_buffer);\
    diskRead((z), a, fftest_buffer);\
    for (x = 0; x < a; x++) {\
        if (fftest_buffer[x] != 0) {\
            Log("Failure to Verify ZERO at address:%xl Size:%i\r\n", (z) + x,a);\
            while (1);\
        }\
    }\
}

#define VERIFY_EMPTY(z,a) {\
    diskRead((z), a, fftest_buffer);\
    for (x = 0; x < a; x++) {\
        if (fftest_buffer[x] != 0xFF) {\
            Log("Failure to Verify EMPTY at address:%xl Size:%i\r\n", (z) + x,a);\
            while (1);\
        }\
    }\
}

BYTE fftest_buffer[4097];

void fftest_LowLevel() {
    int x;

    Log("Initialize: Erasing Sectors under test..\r\n");
    diskEraseSector(0); //First
    diskEraseSector(0x1000); //Second
    diskEraseSector(0xFFE000); //Second to Last
    diskEraseSector(0xFFF000); //Last

    Log("Writing Non-Zero Data..\r\n");
    Log("-Beginning - 4 Pages\r\n");
    VERIFY_NON_ZERO(0x0, 0x400); //Beginning - 4 Pages
    Log("-Non-Page ALigned - 4 Pages\r\n");
    VERIFY_NON_ZERO(0x401, 0x400); //Non-Page ALigned - 4 Pages
    Log("-/Second page - Random Page Size\r\n");
    VERIFY_NON_ZERO(0x1000, 0x185); //Second page - Random Page Size
    Log("-Single Byte\r\n");
    VERIFY_NON_ZERO(0x1401, 0x1); //Single Byte
    Log("-Full Sector\r\n");
    VERIFY_NON_ZERO(0xFFE000, 0x1000); //Full Sector
    Log("-END - 4 Pages\r\n");
    VERIFY_NON_ZERO(0xFFFBFF, 0x400); //END - 4 Pages

    Log("Writing Zero Data..\r\n");
    Log("-Beginning - 4 Pages\r\n");
    VERIFY_ZERO(0x0, 0x400); //Beginning - 4 Pages
    Log("-Non-Page ALigned - 4 Pages\r\n");
    VERIFY_ZERO(0x401, 0x400); //Non-Page ALigned - 4 Pages
    Log("-/Second page - Random Page Size\r\n");
    VERIFY_ZERO(0x1000, 0x185); //Second page - Random Page Size
    Log("-Single Byte\r\n");
    VERIFY_ZERO(0x1401, 0x1); //Single Byte
    Log("-Full Sector\r\n");
    VERIFY_ZERO(0xFFE000, 0x1000); //Full Sector
    Log("-END - 4 Pages\r\n");
    VERIFY_ZERO(0xFFFBFF, 0x400); //END - 4 Pages


    Log("Test: Erasing Sectors under test..\r\n");
    diskEraseSector(0); //First
    diskEraseSector(0x1000); //Second
    diskEraseSector(0xFFE000); //Second to Last
    diskEraseSector(0xFFF000); //Last

    Log("Verify Erasure...\r\n");
    Log("-Beginning - 4 Pages\r\n");
    VERIFY_EMPTY(0x0, 0x400); //Beginning - 4 Pages
    Log("-Non-Page ALigned - 4 Pages\r\n");
    VERIFY_EMPTY(0x401, 0x400); //Non-Page ALigned - 4 Pages
    Log("-Second page - Random Page Size\r\n");
    VERIFY_EMPTY(0x1000, 0x185); //Second page - Random Page Size
    Log("-Single Byte\r\n");
    VERIFY_EMPTY(0x1401, 0x1); //Single Byte
    Log("-Full Sector\r\n");
    VERIFY_EMPTY(0xFFE000, 0x1000); //Full Sector
    Log("-END - 4 Pages\r\n");
    VERIFY_EMPTY(0xFFFBFF, 0x400); //END - 4 Pages

    Log("Low Level Test Complete!\r\n");

}

void fftest_Allocation() {
    int ret;
    Log("Allocate Sector...");
    uint32_t thisSector;
    ret = AllocateSector(0, &thisSector);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("Check Allocate Sector...");
    uint16_t checkSector;
    diskRead(CAT_Address + (thisSector * 2), 2, (BYTE*) & checkSector);
    if (checkSector == 0x7FFF) {
        Log("OK\r\n");
    } else {
        Log("Error: Sector Entry=%xi Expected=0x7FFF", checkSector);
        while (1);
    }

    Log("GetNextSector...");
    uint32_t nextSector;
    ret = GetNextSector(thisSector, &nextSector);
    if (ret == FR_EOF) {
        Log("OK - NextSector=%xl\r\n", nextSector);
    } else {
        Log("Error Res=%i nextSector=%xl", ret, nextSector);
        while (1);
    }

}

void fftest_HighLevel() {
    int ret, bWritten, x;
    ff_File test;
    uint32_t expectedLength;
    uint32_t fs, us, ts;

    LoggingOn = 0;

    Log("Format Disk...");
    ff_Format();
    Log("OK\r\n");

    Log("Mount File System...");
    ret = ff_Initialize();
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    fftest_Allocation();

    Log("!!!!!!!!Create File!!!!!!!!...");
    ret = CreateFile("Test.dat", &test);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(CREATE) Append 4K of Data...");
    for (x = 0; x < 0x1000; x++) fftest_buffer[x] = x % 255;
    ret = ff_Append(&test, fftest_buffer, 0x1000, &bWritten);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(CREATE) Update Length...");
    ret = ff_UpdateLength(&test);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(CREATE) Re-Open...");
    ret = ff_OpenByFileName(&test, "Test.dat", 0);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(CREATE) Check length...");
    if (test.Length == 0x1000) {
        Log("OK\r\n");
    } else {
        Log("Error: Length=%xl Expected=0x1000", test.Length);
        while (1);
    }

    Log("(CREATE) Read 4K of Data...");
    ret = ff_Read(&test, fftest_buffer, 0x1000, &bWritten);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(CREATE) Verify Data...");
    for (x = 0; x < 0x1000; x++) {
        if (fftest_buffer[x] != x % 255) {
            Log("Error @ %i: Expected=%xb Actual=%xb", x, x % 255, fftest_buffer[x]);
            while (1);
        }
    }
    Log("OK\r\n");


    Log("!!!!!!!!Append 40 bytes and Update Length x 100 times!!!!!!!!...");
    for (x = 0; x < 0x1000; x++) fftest_buffer[x] = x % 64;
    expectedLength = 0x1000;

    for (x = 0; x < 100; x++) {
        LoggingOn = 0;
        ret = ff_Append(&test, fftest_buffer + (x * 40), 40, &bWritten);
        if (ret != FR_OK) {
            Log("Append Error On Cycle %i: Res=%i", x, ret);
            while (1);
        }
        ret = ff_UpdateLength(&test);
        if (ret != FR_OK) {
            Log("UpdateLength Error On Cycle %i: Res=%i", x, ret);
            while (1);
        }
        expectedLength += 40;
    }
    Log("OK\r\n");

    Log("(APPEND) Re-Open after updates...");
    ret = ff_OpenByFileName(&test, "Test.dat", 0);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(APPEND) Check length after updates...");
    if (test.Length == expectedLength) {
        Log("OK\r\n");
    } else {
        Log("Error: Length=%xl Expected=%xl", test.Length, expectedLength);
        while (1);
    }

    Log("(APPEND) Test Seek to offset=0x1000...");
    ret = ff_Seek(&test, 0x1000, ff_SeekMode_Absolute);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(APPEND) Read 4000 Bytes of Data...");
    ret = ff_Read(&test, fftest_buffer, 4000, &bWritten);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(APPEND) Verify Data...");
    for (x = 0; x < 4000; x++) {
        if (fftest_buffer[x] != x % 64) {
            Log("Error @ %i: Expected=%xb Actual=%xb", x, x % 255, fftest_buffer[x]);
            while (1);
        }
    }
    Log("OK\r\n");


    Log("!!!!!!!!Test Deletion of Only File!!!!!!!!...");
    ret = ff_Delete("Test.dat");
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(DELETE) Attempt to Open the Deleted File...");
    ret = ff_OpenByFileName(&test, "Test.dat", 0);
    if (ret == FR_NOT_FOUND) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(DELETE) Re-Create The Deleted File...");
    ret = ff_OpenByFileName(&test, "Test.dat", 1);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(DELETE) Write 4K of Data to File...");
    for (x = 0; x < 0x1000; x++) fftest_buffer[x] = x % 47;
    ret = ff_Append(&test, fftest_buffer, 0x1000, &bWritten);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(DELETE) Update Length...");
    ret = ff_UpdateLength(&test);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(DELETE) Re-Open...");
    ret = ff_OpenByFileName(&test, "Test.dat", 0);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(DELETE) Check length...");
    if (test.Length == 0x1000) {
        Log("OK\r\n");
    } else {
        Log("Error: Length=%xl Expected=0x1000", test.Length);
        while (1);
    }

    Log("(DELETE) Read 4K of Data...");
    ret = ff_Read(&test, fftest_buffer, 0x1000, &bWritten);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(DELETE) Verify Data...");
    for (x = 0; x < 0x1000; x++) {
        if (fftest_buffer[x] != x % 47) {
            Log("Error @ %i: Expected=%xb Actual=%xb", x, x % 255, fftest_buffer[x]);
            while (1);
        }
    }
    Log("OK\r\n");

    ff_GetUtilization(&fs, &ts, &us);
    Log("Freespace=%xl TrimSpace=%xl UsedSpace=%xl\r\n", fs, ts, us);

    Log("Checking File System:...");
    ret = ff_CheckFS();
    if (ret != FR_OK) {
        Log("Error Res=%i", ret);
        while (1);
    }

    LoggingOn = 0;
    Log("!!!!!!!!Trim the Drive!!!!!!!!...");
    ret = ff_Trim();
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("Checking File System:...");
    ret = ff_CheckFS();
    if (ret != FR_OK) {
        Log("Error Res=%i", ret);
        while (1);
    }

    ff_GetUtilization(&fs, &ts, &us);
    Log("Freespace=%xl TrimSpace=%xl UsedSpace=%xl\r\n", fs, ts, us);


    Log("(TRIM) Re-Open...");
    ret = ff_OpenByFileName(&test, "Test.dat", 0);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(TRIM) Check length...");
    if (test.Length == 0x1000) {
        Log("OK\r\n");
    } else {
        Log("Error: Length=%xl Expected=0x1000", test.Length);
        while (1);
    }

    Log("(TRIM) Read 4K of Data...");
    ret = ff_Read(&test, fftest_buffer, 0x1000, &bWritten);
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("(TRIM) Verify Data...");
    for (x = 0; x < 0x1000; x++) {
        if (fftest_buffer[x] != x % 47) {
            Log("Error @ %i: Expected=%xb Actual=%xb", x, x % 255, fftest_buffer[x]);
            while (1);
        }
    }
    Log("OK\r\n");

    ff_GetUtilization(&fs, &ts, &us);
    Log("Freespace=%xl TrimSpace=%xl UsedSpace=%xl\r\n", fs, ts, us);


    Log("Create 255 Files each random length (999-7992 bytes)...");
    char fname[128];
    BYTE flen;
    int q;
    for (x = 0; x < 255; x++) {
        Log("   File #%i ", x);
        memset(fftest_buffer, (BYTE) x, 999);
        sprintf(fname, "%d.test", x);
        ret = ff_OpenByFileName(&test, fname, 1);
        if (ret != FR_OK) {
            Log("ff_OpenByFileName Error Res=%i", ret);
            while (1);
        }
        FIFO_Read(TRNG_fifo, flen);
        flen = 1 + (flen % 8);
        Log("Length=%ubkb\r\n", flen);
        buff[0] = flen;
        ret = ff_Append(&test, buff, 1, &bWritten);
        if (ret != FR_OK) {
            Log("Write Length ff_Append Error Res=%i", ret);
            while (1);
        }
        for (q = 0; q < flen; q++) {
            ret = ff_Append(&test, fftest_buffer, 999, &bWritten);
            if (ret != FR_OK) {
                Log("ff_Append Error Res=%i", ret);
                while (1);
            }
            ret = ff_UpdateLength(&test);
            if (ret != FR_OK) {
                Log("ff_UpdateLength Error Res=%i", ret);
                while (1);
            }
        }
    }
    Log("OK\r\n");


    ff_GetUtilization(&fs, &ts, &us);
    Log("Freespace=%xl TrimSpace=%xl UsedSpace=%xl\r\n", fs, ts, us);

    Log("Verify 255 Files...");
    unsigned long lenCheck;
    int z;
    for (x = 0; x < 255; x++) {
        Log("*****FILE#:%i ", x);
        memset(fftest_buffer, (BYTE) x, 999);
        sprintf(fname, "%d.test", x);
        ret = ff_OpenByFileName(&test, fname, 1);
        if (ret != FR_OK) {
            Log("ff_OpenByFileName Error Res=%i", ret);
            while (1);
        }
        ret = ff_Read(&test, &flen, 1, &bWritten);
        if (ret != FR_OK) {
            Log("Read Length ff_Read Error Res=%i", ret);
            while (1);
        }
        Log("Length=%ubkb...", flen);
        lenCheck = flen;
        lenCheck *= 999;
        lenCheck += 1;

        if (test.Length != lenCheck) {
            Log("File Length does not match expected: Length=%xl Expected=%xl", test.Length, lenCheck);
            while (1);
        }

        for (q = 0; q < flen; q++) {
            ret = ff_Read(&test, fftest_buffer, 999, &bWritten);
            if (ret != FR_OK) {
                Log("ff_Read Error Res=%i", ret);
                while (1);
            }

            for (z = 0; z < 999; z++) {
                if (fftest_buffer[z] != x) {
                    Log("File Content Corrupted @ %xi|%xi Read=%xb Expected=%xb", q, z, fftest_buffer[z], (BYTE) x);
                    while (1);
                }
            }
        }
        Log("OK\r\n");
    }
    Log("OK\r\n");

    Log("Checking File System:...");
    ret = ff_CheckFS();
    if (ret != FR_OK) {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("Repair File System:...");
    ret = ff_RepairFS();
    if (ret != FR_OK) {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("Re-Checking File System After Repair:...");
    ret = ff_CheckFS();
    if (ret != FR_OK) {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("Delete every 2nd file...");
    for (x = 0; x < 255; x += 2) {
        sprintf(fname, "%d.test", x);
        ret = ff_Delete(fname);
        if (ret != FR_OK) {
            Log("ff_Delete Error: '%s' Res=%i '%s'", fname, ret, Translate_DRESULT(ret));
            while (1);
        }
    }
    Log("OK\r\n");

    Log("Verify File Deletion...");
    for (x = 0; x < 255; x += 2) {
        sprintf(fname, "%d.test", x);
        ret = ff_OpenByFileName(&test, fname, 0);
        if (ret != FR_NOT_FOUND) {
            Log("Error: File Exists! '%s' Res=%i", fname, ret);
            while (1);
        }
    }
    Log("OK\r\n");

    ff_GetUtilization(&fs, &ts, &us);
    Log("Freespace=%xl TrimSpace=%xl UsedSpace=%xl\r\n", fs, ts, us);

    Log("Verify Expected Files are still there...");
    for (x = 1; x < 255; x += 2) {
        Log("*****FILE#:%i ", x);
        memset(fftest_buffer, (BYTE) x, 999);
        sprintf(fname, "%d.test", x);
        ret = ff_OpenByFileName(&test, fname, 1);
        if (ret != FR_OK) {
            Log("ff_OpenByFileName Error Res=%i", ret);
            while (1);
        }
        ret = ff_Read(&test, &flen, 1, &bWritten);
        if (ret != FR_OK) {
            Log("Read Length ff_Read Error Res=%i", ret);
            while (1);
        }
        Log("Length=%ubkb...", flen);
        lenCheck = flen;
        lenCheck *= 999;
        lenCheck += 1;

        if (test.Length != lenCheck) {
            Log("File Length does not match expected: Length=%xl Expected=%xl", test.Length, lenCheck);
            while (1);
        }

        for (q = 0; q < flen; q++) {
            ret = ff_Read(&test, fftest_buffer, 999, &bWritten);
            if (ret != FR_OK) {
                Log("ff_Read Error Res=%i", ret);
                while (1);
            }

            for (z = 0; z < 999; z++) {
                if (fftest_buffer[z] != x) {
                    Log("File Content Corrupted @ %xi|%xi Read=%xb Expected=%xb", q, z, fftest_buffer[z], (BYTE) x);
                    while (1);
                }
            }
        }
        Log("OK\r\n");
    }
    Log("OK\r\n");

    ff_GetUtilization(&fs, &ts, &us);
    Log("PRE TRIM Freespace=%xl TrimSpace=%xl UsedSpace=%xl\r\n", fs, ts, us);

    Log("TRIM Round 2...");
    ret = ff_Trim();
    if (ret == FR_OK) {
        Log("OK\r\n");
    } else {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("Checking File System:...");
    ret = ff_CheckFS();
    if (ret != FR_OK) {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("Re-Verify Files after trim...");
    for (x = 1; x < 255; x += 2) {
        Log("*****FILE#:%i ", x);
        memset(fftest_buffer, (BYTE) x, 999);
        sprintf(fname, "%d.test", x);
        ret = ff_OpenByFileName(&test, fname, 1);
        if (ret != FR_OK) {
            Log("ff_OpenByFileName Error Res=%i", ret);
            while (1);
        }
        ret = ff_Read(&test, &flen, 1, &bWritten);
        if (ret != FR_OK) {
            Log("Read Length ff_Read Error Res=%i", ret);
            while (1);
        }
        Log("Length=%ubkb...", flen);
        lenCheck = flen;
        lenCheck *= 999;
        lenCheck += 1;

        if (test.Length != lenCheck) {
            Log("File Length does not match expected: Length=%xl Expected=%xl", test.Length, lenCheck);
            while (1);
        }

        for (q = 0; q < flen; q++) {
            ret = ff_Read(&test, fftest_buffer, 999, &bWritten);
            if (ret != FR_OK) {
                Log("ff_Read Error Res=%i", ret);
                while (1);
            }

            for (z = 0; z < 999; z++) {
                if (fftest_buffer[z] != x) {
                    Log("File Content Corrupted @ %xi|%xi Read=%xb Expected=%xb", q, z, fftest_buffer[z], (BYTE) x);
                    while (1);
                }
            }
        }
        Log("OK\r\n");
    }
    Log("OK\r\n");


    ff_GetUtilization(&fs, &ts, &us);
    Log("Freespace=%xl TrimSpace=%xl UsedSpace=%xl\r\n", fs, ts, us);

    uint16_t badData = 0xFEFE;
    ff_File pig;
    Log("Corrupting Origin Sector for \"103.test\"\r\n");
    ret = ff_OpenByFileName(&pig, "103.test", 0);
    if (ret != FR_OK) {
        Log("ff_OpenByFileName Error Res=%i", ret);
        while (1);
    }

    badData = 0;
    diskWrite(CAT_Address + (2 * pig.OriginSector), 2, (BYTE *) & badData);

    Log("Checking File System:...");
    ret = ff_CheckFS();
    if (ret != FR_CORRUPTED) {
        Log("Error FS is NOT Corrupted!!! Res=%i", ret);
        while (1);
    }
    Log("OK (IT is corrupted!)\r\n");

    Log("Repair File System...");
    ret = ff_RepairFS();
    if (ret != FR_OK) {
        Log("Error Res=%i", ret);
        while (1);
    }
    Log("OK\r\n");


    Log("Re-Check File System:...");
    ret = ff_CheckFS();
    if (ret != FR_OK) {
        Log("Error Res=%i", ret);
        while (1);
    }
    Log("OK (IT is NOT corrupted!)\r\n");

    Log("Are the Files Gone?...");
    ret = ff_OpenByFileName(&pig, "103.test", 0);
    if (ret != FR_NOT_FOUND) {
        Log("Error Res=%i", ret);
        while (1);
    }

    Log("OK The files are gone\r\n");


    Log("Re-Verify Files after REPAIR...");
    for (x = 1; x < 255; x += 2) {
        if (x == 101 || x == 103) continue;

        Log("*****FILE#:%i ", x);
        memset(fftest_buffer, (BYTE) x, 999);
        sprintf(fname, "%d.test", x);
        ret = ff_OpenByFileName(&test, fname, 1);
        if (ret != FR_OK) {
            Log("ff_OpenByFileName Error Res=%i", ret);
            while (1);
        }
        ret = ff_Read(&test, &flen, 1, &bWritten);
        if (ret != FR_OK) {
            Log("Read Length ff_Read Error Res=%i", ret);
            while (1);
        }
        Log("Length=%ubkb...", flen);
        lenCheck = flen;
        lenCheck *= 999;
        lenCheck += 1;

        if (test.Length != lenCheck) {
            Log("File Length does not match expected: Length=%xl Expected=%xl", test.Length, lenCheck);
            while (1);
        }

        for (q = 0; q < flen; q++) {
            ret = ff_Read(&test, fftest_buffer, 999, &bWritten);
            if (ret != FR_OK) {
                Log("ff_Read Error Res=%i", ret);
                while (1);
            }

            for (z = 0; z < 999; z++) {
                if (fftest_buffer[z] != x) {
                    Log("File Content Corrupted @ %xi|%xi Read=%xb Expected=%xb", q, z, fftest_buffer[z], (BYTE) x);
                    while (1);
                }
            }
        }
        Log("OK\r\n");
    }

    Log("Starting Seek Testing to 1Mb\r\n");

    uint32_t seekPos = 0;
    ff_File seekHandle;
    ff_OpenByFileName(&seekHandle, "seekTest.dat", 1);

    Log("8 Byte Writing Seek File:\r\n");
    for (seekPos = 0; seekPos < 0x40000; seekPos += 8) {
        if ((seekPos & 0x7FF) == 0) Log("\r\n%xl: ", seekPos);
        ff_Seek(&seekHandle, seekPos, ff_SeekMode_Absolute);
        ff_Append(&seekHandle, (BYTE *) & seekPos, 4, &bWritten);
        ff_Append(&seekHandle, (BYTE *) & seekPos, 4, &bWritten);
        if ((seekPos & 0b1111) == 0) Log(".");
    }

    uint32_t seekCheck, seekCheck2;
    Log("Done\r\n8 Byte Checking Seek File:\r\n");
    for (seekPos = 0; seekPos < 0x40000; seekPos += 8) {
        if ((seekPos & 0x7FF) == 0) Log("\r\n%xl: ", seekPos);
        ff_Seek(&seekHandle, seekPos, ff_SeekMode_Absolute);
        ff_Read(&seekHandle, (BYTE *) & seekCheck, 4, &bWritten);
        ff_Read(&seekHandle, (BYTE *) & seekCheck2, 4, &bWritten);
        if (seekCheck == seekPos && seekCheck == seekCheck2) {
            if ((seekPos & 0b1111) == 0) Log(".");
        } else {
            Log("Error: Expected=%xl Actual1=%xl Actual2=%xl", seekPos, seekCheck, seekCheck2);
            while (1);
        }
    }
    Log("Done\r\n");

    Log("Repair & Trim File SYstem\r\n");
    ff_RepairFS();

    Log("Re-Open File\r\n");
    ret = ff_OpenByFileName(&seekHandle, "seekTest.dat", 0);
    if (ret != FR_OK) {
        Log("ff_OpenByFileName Error Res=%i", Translate_DRESULT(ret));
        while (1);
    }

    Log("8 Byte Checking Seek File:\r\n");
    for (seekPos = 0; seekPos < 0x40000; seekPos += 8) {
        if ((seekPos & 0x7FF) == 0) Log("\r\n%xl: ", seekPos);
        ff_Seek(&seekHandle, seekPos, ff_SeekMode_Absolute);
        ff_Read(&seekHandle, (BYTE *) & seekCheck, 4, &bWritten);
        if (seekCheck == seekPos) {
            if ((seekPos & 0b1111) == 0) Log(".");
        } else {
            Log("Error: Expected=%xl Actual=%xl", seekPos, seekCheck);
            while (1);
        }
    }
    Log("Done\r\n");

    Log("High Level FS Test Complete\r\n");
}

#endif
