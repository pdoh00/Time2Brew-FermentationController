#include "SystemConfiguration.h"
#include "FlashFS.h"
#include "ESP_Flash.h"
#include "Pack.h"
#include <string.h>

#define ESP_CHECKSUM_MAGIC  0xEF

#define ESP_FLASH_BEGIN  0x02
#define ESP_FLASH_DATA   0x03
#define ESP_FLASH_END    0x04
#define ESP_MEM_BEGIN    0x05
#define ESP_MEM_END      0x06
#define ESP_MEM_DATA     0x07
#define ESP_SYNC         0x08
#define ESP_WRITE_REG    0x09
#define ESP_READ_REG     0x0A

#define ESP_FLASH_BLOCK  0x100

float espflash_portTimeout = 0.5;

void Delay(float time) {
    time /= 0.000105;
    unsigned long x = (unsigned long) time;
    while (x--) DELAY_105uS;
}

static int espflash_portRead() {
    unsigned long timeout = espflash_portTimeout * 2396658UL;
    if (U1STAbits.OERR) U1STAbits.OERR = 0;
    if (U1STAbits.FERR) U1STAbits.FERR = 0;
    if (U1STAbits.PERR) U1STAbits.PERR = 0;
    unsigned char token;
    while (!U1STAbits.URXDA) {
        timeout--;
        if (timeout == 0) {
            return -1;
        }
    }
    if (U1STAbits.FERR) {
        return -2;
    }
    if (U1STAbits.PERR) {
        return -3;
    }
    token = U1RXREG;
    return (int) token;
}

static int espflash_portReadSlip() {
    int token = espflash_portRead();

    if (token < 0) {
        return token;
    } else if (token == 0xdb) {
        token = espflash_portRead();
        if (token == 0xdc) {
            return 0xc0;
        } else if (token == 0xdd) {
            return 0xdb;
        } else {
            return -1;
        }
    } else {
        return token;
    }
}

static void espflash_portWrite(unsigned char dat) {
    while (!U1STAbits.TRMT);
    U1TXREG = dat;
}

static void espflash_portWriteSlip(unsigned char token) {
    if (token == 0xC0) {
        espflash_portWrite(0xDB);
        espflash_portWrite(0xDC);
    } else if (token == 0xDB) {
        espflash_portWrite(0xDB);
        espflash_portWrite(0xDD);
    } else {
        espflash_portWrite(token);
    }
}

static void espflash_portFlush() {
    if (U1STAbits.OERR) U1STAbits.OERR = 0;
    unsigned char token;
    while (U1STAbits.URXDA) token = U1RXREG;
}

static unsigned char espflash_checksum(int bCount, unsigned char *in) {
    unsigned char chk = ESP_CHECKSUM_MAGIC;
    while (bCount--) {
        chk ^= *(in++);
    }
    return chk;
}

static int espflash_command(unsigned char command, unsigned int lenPayload, unsigned char *Payload, unsigned char chk, unsigned char *Body, unsigned int *lenBody, unsigned char *resp_val) {
    int token, resp_dir, resp_command;
    unsigned int resp_len;

    if (command) {
        espflash_portWrite(0xC0);
        espflash_portWriteSlip(0x00);
        espflash_portWriteSlip(command);
        espflash_portWriteSlip((lenPayload) & 0xFF);
        espflash_portWriteSlip((lenPayload >> 8) & 0xFF);
        espflash_portWriteSlip(chk);
        espflash_portWriteSlip(0x00);
        espflash_portWriteSlip(0x00);
        espflash_portWriteSlip(0x00);
        while (lenPayload--) {
            espflash_portWriteSlip(*Payload);
            Payload++;
        }
        espflash_portWrite(0xC0);
    }

    token = espflash_portRead();
    if (token != 0xC0) {
        Log("Error: Invalid Start of Packet Token. Expected=0xC0 Actual=%xi\r\n", token);
        return -1;
    }

    resp_dir = espflash_portReadSlip();
    if (resp_dir != 1) {
        Log("Error: Invalid Response. Expected=0x01 Actual=%xi\r\n", resp_dir);
        return -1;
    }

    resp_command = espflash_portReadSlip();
    if (command != 0 && (resp_command != command)) {
        Log("Error: Invalid Response Command. Expected=%xb Actual=%xi\r\n", command, resp_command);
        return -1;
    }


    int x;
    for (x = 0; x < 2; x++) {
        token = espflash_portReadSlip();
        if (token < 0) {
            Log("Error: Timeout Reading resp_val[%i]\r\n", x);
            return token;
        }
        resp_val[x] = token;
    }
    resp_len = resp_val[1];
    resp_len <<= 8;
    resp_len += resp_val[0];
    *lenBody = resp_len;

    for (x = 0; x < 4; x++) {
        token = espflash_portReadSlip();
        if (token < 0) {
            Log("Error: Timeout Reading resp_val[%i]\r\n", x);
            return token;
        }
        resp_val[x] = token;
    }

    while (resp_len--) {
        token = espflash_portReadSlip();
        if (token < 0) {
            Log("Error: Timeout Reading resp_body\r\n");
            return token;
        }
        *(Body++) = (unsigned char) token;
    }

    token = espflash_portRead();
    if (token != 0xC0) {
        Log("Error: Invalid End of Packet Token. Expected=0xC0 Actual=%xb\r\n", token);
        return token;
    }

    return 0;
}

static int espflash_Sync() {
    int ret;
    unsigned char syncPacket[] = {0x07, 0x07, 0x12, 0x20,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
    unsigned char body[4], resp_val[4];
    unsigned int bodyLen;

    espflash_portFlush();
    ret = espflash_command(ESP_SYNC, sizeof (syncPacket), syncPacket, 0, body, &bodyLen, resp_val);
    if (ret != 0) return ret;

    int x;
    for (x = 0; x < 7; x++) {
        ret = espflash_command(0, 0, 0, 0, body, &bodyLen, resp_val);
        if (ret != 0) {
            Log(" FailedOn:%i ", x);
            return ret;
        }
    }
    return 0;
}

int espflash_connect() {
    SET_WIFI_POWER(0);
    SET_WIFI_PROG(1);
    Delay(0.5);
    SET_WIFI_POWER(1);
    Delay(1);

    _U1TXIE = 0;
    _U1RXIE = 0;

    espflash_portFlush();
    espflash_portTimeout = 0.5;


    int x, ret;
    for (x = 0; x < 10; x++) {
        Log("Connecting...");
        ret = espflash_Sync();
        if (ret == 0) {
            Log("OK!\r\n");
            return 0;
        } else {
            Log("Fail ret=%i\r\n", ret);
            Delay(0.5);
        }
    }

    SET_WIFI_POWER(0);
    SET_WIFI_PROG(0);
    Delay(0.5);
    espflash_portFlush();
    _U1TXIE = 1;
    _U1RXIE = 1;
    return ret;
}

int espflash_disconnect() {
    SET_WIFI_POWER(0);
    SET_WIFI_PROG(0);
    Delay(0.5);
    espflash_portFlush();
    _U1TXIE = 1;
    _U1RXIE = 1;
    return 0;
}

static int espflash_FlashBegin(unsigned long size, unsigned long offset) {
    unsigned char packet[16];
    unsigned char resp_body[4];
    unsigned int resp_lenBody;
    unsigned char resp_val[4];
    float old_tmo = espflash_portTimeout;
    unsigned long num_blocks = (size + ESP_FLASH_BLOCK - 1) / ESP_FLASH_BLOCK;
    unsigned long flashBlockSize = ESP_FLASH_BLOCK;
    int ret;
    espflash_portTimeout = 10;
    Pack(packet, "llll", size, num_blocks, flashBlockSize, offset);
    ret = espflash_command(ESP_FLASH_BEGIN, 16, packet, 0, resp_body, &resp_lenBody, resp_val);
    if (ret < 0) return ret;

    if (resp_body[0] != 0x00 || resp_body[1] != 0x00) {
        Log("Invalid resp_body. Expected=0x00 0x00  Actual=%xb %xb", resp_body[0], resp_body[1]);
        return -1;
    }
    espflash_portTimeout = old_tmo;
    return 0;
}

static int espflash_FlashBlock(unsigned char *data, unsigned long lenData, unsigned long seq) {
    unsigned char packet[272];
    unsigned char resp_body[4];
    unsigned int resp_lenBody;
    unsigned char resp_val[4];
    unsigned char chksum = espflash_checksum(lenData, data);
    int ret;
    Pack(packet, "llll", lenData, seq, 0, 0);
    memcpy(&packet[16], data, lenData);

    unsigned long packetLength = 16 + lenData;
    ret = espflash_command(ESP_FLASH_DATA, packetLength, packet, chksum, resp_body, &resp_lenBody, resp_val);
    if (ret < 0) return ret;

    if (resp_body[0] != 0x00 || resp_body[1] != 0x00) {
        Log("Invalid resp_body. Expected=0x00 0x00  Actual=%xb %xb", resp_body[0], resp_body[1]);
        return -1;
    }

    return 0;
}

int espflash_FlashFinish(char reboot) {
    unsigned char packet[4];
    unsigned char resp_body[4];
    unsigned int resp_lenBody;
    unsigned char resp_val[4];
    int ret;
    packet[0] = (!reboot);
    packet[1] = 0;
    packet[2] = 0;
    packet[3] = 0;
    ret = espflash_command(ESP_FLASH_END, 4, packet, 0, resp_body, &resp_lenBody, resp_val);
    if (ret < 0) return ret;

    if (resp_body[0] != 0x00 || resp_body[1] != 0x00) {
        Log("Invalid resp_body. Expected=0x00 0x00  Actual=%xb %xb", resp_body[0], resp_body[1]);
        return -1;
    }
    SET_WIFI_POWER(0);
    SET_WIFI_PROG(0);
    return 0;
}

int espflash_FlashFile(unsigned long imageAddress, unsigned long imageLength, unsigned long esp_flashOffset) {
    unsigned char buff[ESP_FLASH_BLOCK];
    int ret;
    unsigned long seq = 0;

    Log("Erasing Flash...");
    ret = espflash_FlashBegin(imageLength, esp_flashOffset);
    if (ret < 0) return ret;
    Log("OK\r\n");

    while (imageLength) {
        Log("Writing at %xl...", esp_flashOffset + (seq * ESP_FLASH_BLOCK));
        diskRead(imageAddress, ESP_FLASH_BLOCK, buff);
        ret = espflash_FlashBlock(buff, ESP_FLASH_BLOCK, seq);
        if (ret < 0) return ret;
        Log("OK\r\n");
        seq++;
        imageLength -= ESP_FLASH_BLOCK;
        imageAddress += ESP_FLASH_BLOCK;
    }
    return 0;
}

