#include <stdbool.h>
#include "Base64.h"
#include "integer.h"

const char* encode64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const unsigned char decode64[] = {
    66, 66, 66, 66, 66, 66, 66, 66, 66, 64, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 62, 66, 66, 66, 63, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 66, 66, 66, 65, 66, 66, 66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 66, 66, 66, 66, 66, 66, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66
};

int Base64_Length(int bCount) {
    int QuadCount = (bCount / 3);
    if ((bCount % 3) != 0) QuadCount++;
    QuadCount *= 4;
    return (QuadCount);
}

int sprintf_Base64(char *output, unsigned char *sourcedata, int bCount) {
    int byteNo; // I need this after the loop
    int modulusLen = bCount % 3;

    int QuadCount = (bCount / 3);
    if (modulusLen != 0) QuadCount++;
    QuadCount *= 4;

    for (byteNo = 0; byteNo <= bCount - 3; byteNo += 3) {
        unsigned char BYTE0 = sourcedata[byteNo];
        unsigned char BYTE1 = sourcedata[byteNo + 1];
        unsigned char BYTE2 = sourcedata[byteNo + 2];
        *(output++) = encode64[ BYTE0 >> 2 ];
        *(output++) = encode64[ ((0x3 & BYTE0) << 4) + (BYTE1 >> 4) ];
        *(output++) = encode64[ ((0x0f & BYTE1) << 2) + (BYTE2 >> 6) ];
        *(output++) = encode64[ 0x3f & BYTE2 ];
    }

    if (modulusLen == 1) { //One byte is in the last nibble so encode two fillers
        *(output++) = encode64[ sourcedata[byteNo] >> 2 ];
        *(output++) = encode64[ (0x3 & sourcedata[byteNo]) << 4 ];
        *(output++) = '=';
        *(output++) = '=';

    } else if (modulusLen == 2) { //Two bytes are in the last nibble so encode one filler
        *(output++) = encode64[ sourcedata[byteNo] >> 2 ];
        *(output++) = encode64[ ((0x3 & sourcedata[byteNo]) << 4) + (sourcedata[byteNo + 1] >> 4) ];
        *(output++) = encode64[ (0x0f & sourcedata[byteNo + 1]) << 2 ];
        *(output++) = '=';

    }
    return (QuadCount);
}

void circular_ToBase64(FIFO_BUFFER *outFIFO, unsigned char *srcData, int srcCount) {
    int byteNo; // I need this after the loop
    int modulusLen = srcCount % 3;

    int QuadCount = (srcCount / 3);
    if (modulusLen != 0) QuadCount++;
    QuadCount *= 4;

    for (byteNo = 0; byteNo <= srcCount - 3; byteNo += 3) {
        unsigned char BYTE0 = srcData[byteNo];
        unsigned char BYTE1 = srcData[byteNo + 1];
        unsigned char BYTE2 = srcData[byteNo + 2];
        FIFO_Write(outFIFO, encode64[ BYTE0 >> 2 ]);
        FIFO_Write(outFIFO, encode64[ ((0x3 & BYTE0) << 4) + (BYTE1 >> 4) ]);
        FIFO_Write(outFIFO, encode64[ ((0x0f & BYTE1) << 2) + (BYTE2 >> 6) ]);
        FIFO_Write(outFIFO, encode64[ 0x3f & BYTE2 ]);
    }

    if (modulusLen == 1) { //One byte is in the last nibble so encode two fillers
        FIFO_Write(outFIFO, encode64[ srcData[byteNo] >> 2 ]);
        FIFO_Write(outFIFO, encode64[ (0x3 & srcData[byteNo]) << 4 ]);
        FIFO_Write(outFIFO, '=');
        FIFO_Write(outFIFO, '=');

    } else if (modulusLen == 2) { //Two bytes are in the last nibble so encode one filler
        FIFO_Write(outFIFO, encode64[ srcData[byteNo] >> 2 ]);
        FIFO_Write(outFIFO, encode64[ ((0x3 & srcData[byteNo]) << 4) + (srcData[byteNo + 1] >> 4) ]);
        FIFO_Write(outFIFO, encode64[ (0x0f & srcData[byteNo + 1]) << 2 ]);
        FIFO_Write(outFIFO, '=');

    }
}

int decode_Base64(const char *input, int inputLen, unsigned char *output) {
    int len = 0;

    union {
        unsigned long ul;
        unsigned char ub[4];
    } buffer;

    unsigned char c;
    buffer.ul = 1;

    while (inputLen--) {
        c = decode64[(BYTE)*(input++)];
        switch (c) {
            case 64:
                continue;
                break;
            case 65:
                inputLen = 0;
                break;
            case 66:
                return (-1);
                break;
            default:
                buffer.ul = (buffer.ul << 6);
                buffer.ub[0] = buffer.ub[0] | c;
                if (buffer.ub[3]) {
                    *(output++) = buffer.ub[2];
                    *(output++) = buffer.ub[1];
                    *(output++) = buffer.ub[0];
                    buffer.ul = 1;
                    len += 3;
                }
                break;
        }
    }

    if (buffer.ul & 0x40000) {
        *output++ = buffer.ul >> 10;
        *output++ = buffer.ul >> 2;
        len += 2;
    } else if (buffer.ul & 0x1000) {
        *output++ = buffer.ul >> 4;
        len++;
    }
    return (len);
}


