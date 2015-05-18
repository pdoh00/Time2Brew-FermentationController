#include <stdio.h>
#include <string.h>
#include "mDNS.h"
#include "integer.h"
#include "SystemConfiguration.h"
#include "ESP8266.h"

#define mDNS_Interval 150000ul   //Every 5 minutes...

typedef struct {
    unsigned int TransactionID;
    unsigned int IsResponse;
    unsigned int OPCode;
    unsigned int AA;
    unsigned int TC;
    unsigned int RD;
    unsigned int RA;
    unsigned int Z;
    unsigned int RCode;
    unsigned int QDCount;
    unsigned int ANCount;
    unsigned int NSCount;
    unsigned int ARCount;
} mDNS_Header_Type;

BYTE * mDNS_ParseName(BYTE *inBuffer, char *outBuffer, BYTE *mDNS_Buffer);
BYTE *mDNS_DeserializeHeader(mDNS_Header_Type *Header, BYTE *source);

void mDNS_WriteHeader(mDNS_Header_Type *Header);
void mDNS_Write_SRV_Record(char Cache);
void mDNS_Write_PTR_Record(char Cache);
void mDNS_Write_A_Record(char Cache);

BYTE *ToDNS_Resource(const char *source, BYTE *output);
BYTE *Encode_DNS_Segment(const char *segment, BYTE *output, BYTE *length);
BYTE *mDNS_Extract_DNS_Resource(BYTE *MessageStart, BYTE *source, char *dst);


#define WRITE_UINT16(x,y) ({*(y++)=(x>>8); *(y++)=(x & 0xFF);})
#define READ_UINT16(x,y) ({x=*(y++); x<<=8; x+=*(y++);})

BYTE mDNS_A_Name[72];
unsigned int mDNS_A_NameLength;
BYTE mDNS_PTR_Name[20];
unsigned int mDNS_PTR_NameLength;
BYTE mDNS_SRV_Name[82];
unsigned int mDNS_SRV_NameLength;
BYTE mDNS_IP[4];

unsigned long mDNS_Timeout;

char mDNS_flagSend = 0;

void mDNS_Init(const char *Name, unsigned long IPAddress) {
    char temp[80];
    Log("    mDNS Init: SRV Record=%s._http._tcp.local\r\n", Name);
    sprintf(temp, "%s._http._tcp.local", Name);
    mDNS_SRV_NameLength = ToDNS_Resource(temp, mDNS_SRV_Name) - mDNS_SRV_Name;

    Log("    mDNS Init: A Record=%s.local\r\n", Name);
    sprintf(temp, "%s.local", Name);
    mDNS_A_NameLength = ToDNS_Resource(temp, mDNS_A_Name) - mDNS_A_Name;

    Log("    mDNS Init: PTR Record=_http._tcp.local\r\n");
    sprintf(temp, "_http._tcp.local");
    mDNS_PTR_NameLength = ToDNS_Resource(temp, mDNS_PTR_Name) - mDNS_PTR_Name;

    mDNS_IP[3] = (IPAddress >> 24) & 0xFF;
    mDNS_IP[2] = (IPAddress >> 16) & 0xFF;
    mDNS_IP[1] = (IPAddress >> 8) & 0xFF;
    mDNS_IP[0] = (IPAddress) & 0xFF;
}

void mDNS_RecieveMsg(MESSAGE *msg) {
    mDNS_Header_Type header;
    BYTE *cursor = msg->Data;
    char QName[256];
    unsigned int QType, QClass;

    cursor = mDNS_DeserializeHeader(&header, cursor);

    if (header.IsResponse) {
        return;
    }

    if (header.QDCount == 0) {
        return;
    }

    while (header.QDCount--) {
        cursor = mDNS_Extract_DNS_Resource(msg->Data, cursor, QName);

        READ_UINT16(QType, cursor);
        READ_UINT16(QClass, cursor);

        if (strlen(QName) == 0) continue;

        if (memcmp(mDNS_PTR_Name, QName, mDNS_PTR_NameLength) == 0) {
            Log("mDNS Query: Match My PTR \r\n");
            mDNS_flagSend = 1;
            continue;
        }

        if (memcmp(mDNS_SRV_Name, QName, mDNS_SRV_NameLength) == 0) {
            Log("mDNS Query: Match My SRV \r\n");
            mDNS_flagSend = 1;
            continue;
        }

        if (memcmp(mDNS_A_Name, QName, mDNS_A_NameLength) == 0) {
            Log("mDNS Query: Match My A Record \r\n");
            mDNS_flagSend = 1;
            continue;
        }
    }
    //Log("   Complete\r\n\r\n");
}

BYTE * mDNS_DeserializeHeader(mDNS_Header_Type *Header, BYTE * source) {
    unsigned int temp;
    READ_UINT16(Header->TransactionID, source);

    READ_UINT16(temp, source);
    Header->RCode = temp & 0b1111;
    temp >>= 4;
    Header->Z = temp & 0b111;
    temp >>= 3;
    Header->RA = temp & 0b1;
    temp >>= 1;
    Header->RD = temp & 0b1;
    temp >>= 1;
    Header->TC = temp & 0b1;
    temp >>= 1;
    Header->AA = temp & 0b1;
    temp >>= 1;
    Header->OPCode = temp & 0b1111;
    temp >>= 4;
    Header->IsResponse = temp & 0b1;

    READ_UINT16(Header->QDCount, source);
    READ_UINT16(Header->ANCount, source);
    READ_UINT16(Header->NSCount, source);
    READ_UINT16(Header->ARCount, source);

    return source;
}

void mDNS_WriteHeader(mDNS_Header_Type *Header) {

    union {
        unsigned int i;
        BYTE b[2];
    } Flags;

    Flags.i = 0;
    Flags.i |= Header->IsResponse;
    Flags.i <<= 1;
    Flags.i |= Header->OPCode;
    Flags.i <<= 4;
    Flags.i |= Header->AA;
    Flags.i <<= 1;
    Flags.i |= Header->TC;
    Flags.i <<= 1;
    Flags.i |= Header->RD;
    Flags.i <<= 1;
    Flags.i |= Header->RA;
    Flags.i <<= 1;
    Flags.i |= Header->Z;
    Flags.i <<= 3;
    Flags.i |= Header->RCode;

    FIFO_WriteData(txFIFO, 12,
            0, 0,
            0x84, 00,
            (Header->QDCount >> 8) &0xFF, (Header->QDCount) & 0xFF,
            (Header->ANCount >> 8) &0xFF, (Header->ANCount) & 0xFF,
            (Header->NSCount >> 8) &0xFF, (Header->NSCount) & 0xFF,
            (Header->ARCount >> 8) &0xFF, (Header->ARCount) & 0xFF);
}

void mDNS_Write_SRV_Record(char Cache) {

    union {
        unsigned int i;
        BYTE b[2];
    } type, class, ttl, RDLength;


    FIFO_WriteArray(txFIFO, mDNS_SRV_NameLength, mDNS_SRV_Name);

    type.i = 0x21;
    class.i = 1;
    if (Cache) class.i += 0x8000;

    ttl.i = 600;
    RDLength.i = mDNS_A_NameLength + 6;

    FIFO_WriteData(txFIFO, 16,
            type.b[1], type.b[0], //Type
            class.b[1], class.b[0], //Class
            0, 0, ttl.b[1], ttl.b[0], //TTL
            RDLength.b[1], RDLength.b[0], //Content Length
            0x0, 0x0, //Priority
            0x0, 0x0, //Weight
            0x0, 80//Port
            );

    FIFO_WriteArray(txFIFO, mDNS_A_NameLength, mDNS_A_Name);
}

void mDNS_Write_PTR_Record(char Cache) {

    union {
        unsigned int i;
        BYTE b[2];
    } type, class, ttl, RDLength;

    FIFO_WriteArray(txFIFO, mDNS_PTR_NameLength, mDNS_PTR_Name);

    type.i = 0x0C;
    class.i = 1;
    if (Cache) class.i += 0x8000;
    ttl.i = 600;
    RDLength.i = mDNS_SRV_NameLength;

    FIFO_WriteData(txFIFO, 10,
            type.b[1], type.b[0],
            class.b[1], class.b[0],
            0, 0,
            ttl.b[1], ttl.b[0],
            RDLength.b[1], RDLength.b[0]
            );

    FIFO_WriteArray(txFIFO, mDNS_SRV_NameLength, mDNS_SRV_Name);
}

void mDNS_Write_A_Record(char Cache) {

    union {
        unsigned int i;
        BYTE b[2];
    } type, class, ttl, RDLength;

    FIFO_WriteArray(txFIFO, mDNS_A_NameLength, mDNS_A_Name);

    type.i = 0x01;
    class.i = 1;
    if (Cache) class.i += 0x8000;
    ttl.i = 600;
    RDLength.i = 4;

    FIFO_WriteData(txFIFO, 10,
            type.b[1], type.b[0],
            class.b[1], class.b[0],
            0, 0,
            ttl.b[1], ttl.b[0],
            RDLength.b[1], RDLength.b[0]
            );

    FIFO_WriteData(txFIFO, 4, mDNS_IP[3], mDNS_IP[2], mDNS_IP[1], mDNS_IP[0]);
}

void mDNS_ProcessLoop() {
    unsigned long tmr;
    GetTime(tmr);

    if (tmr > mDNS_Timeout) mDNS_flagSend = 1;
    if (!mDNS_flagSend) return;

    Log("mDNS: Sending Response/Advertisement\r\n");


    mDNS_Header_Type Header;
    memset(&Header, 0, sizeof (mDNS_Header_Type));
    Header.IsResponse = 1;
    Header.AA = 1;
    Header.ANCount = 3;

    ESP_mDNS_BeginSend();
    mDNS_WriteHeader(&Header);
    mDNS_Write_A_Record(1);
    mDNS_Write_PTR_Record(1);
    mDNS_Write_SRV_Record(1);
    ESP_mDNS_CompleteSend();
    DISABLE_INTERRUPTS;
    mDNS_flagSend = 0;
    ENABLE_INTERRUPTS;

    GetTime(tmr);
    mDNS_Timeout = tmr + mDNS_Interval;
    return;
}

BYTE * ToDNS_Resource(const char *source, BYTE * output) {
    BYTE segLen = 0;
    while (*source) {
        output = Encode_DNS_Segment(source, output, &segLen);
        source += segLen;
    }
    *(output++) = 0;
    return output;
}

BYTE * Encode_DNS_Segment(const char *segment, BYTE *output, BYTE * length) {
    const char *cursor = segment;
    BYTE len = 0;
    while (*cursor != '.' && *cursor != 0) {
        len++;
        cursor++;
    }

    if (*cursor == 0) {
        *length = len;
    } else {
        *length = len + 1;
    }

    *(output++) = len;
    while (len--) {
        *(output++) = *(segment++);
    }
    return output;
}

BYTE * mDNS_Extract_DNS_Resource(BYTE *MessageStart, BYTE *source, char *dst) {
    BYTE *retCursor = source;
    BYTE segLength = 0;
    char isCompressed = 0;

    union {
        unsigned int ui;
        unsigned char ub[2];
    } offset;

    while (1) {
        segLength = *(source++);
        if (segLength >= 192) {
            offset.ub[1] = segLength;
            offset.ub[0] = *(source++);
            retCursor = source;
            isCompressed = 1;
            offset.ui &= 0b0011111111111111;
            source = MessageStart + offset.ui;
            segLength = *(source++);
        }
        *(dst++) = segLength;
        if (segLength == 0) {
            if (isCompressed) {
                return retCursor;
            } else {
                return source;
            }
        }

        while (segLength--) {
            *(dst++) = *(source++);
        }
    }
}
