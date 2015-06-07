#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "ESP8266.h"
#include "uPnP.h"
#include "circularPrintF.h"
#include "SystemConfiguration.h"
#include "FlashFS.h"
#include "http_server.h"

#define uPnP_Interval 150000ul; //Every 5 minutes
const char *XML_TEMPLATE = "<?xml version=\"1.0\"?>"
        "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
        "<specVersion>"
        "<major>1</major>"
        "<minor>0</minor>"
        "</specVersion>"
        "<URLBase>http://@I</URLBase>"
        "<device>"
        "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
        "<friendlyName>@N</friendlyName>"
        "<manufacturer>YouKnowWho</manufacturer>"
        "<manufacturerURL>http://www.YouKnowWho.com/</manufacturerURL>"
        "<modelDescription>Temperature Controller</modelDescription>"
        "<modelName>What's A Good Name</modelName>"
        "<modelNumber>1</modelNumber>"
        "<modelURL>http://www.YouKnowWho.com/</modelURL>"
        "<serialNumber>@U</serialNumber>"
        "<UDN>uuid:@UUID</UDN>"
        "<UPC>N/A</UPC>"
        "<iconList>"
        "<icon>"
        "<mimetype>image/x-icon</mimetype>"
        "<width>64</width>"
        "<height>64</height>"
        "<depth>32</depth>"
        "<url>/favicon.ico</url>"
        "</icon> "
        "</iconList>"
        "<presentationURL>/index.htm</presentationURL>"
        "</device></root>";

char uPnP_str_IPAddress[17];
char uPnP_str_UUID[40];
char uPnP_str_Name[64];

unsigned long tmrSendRoot, tmrSendUUID, tmrSendURN;

void SendRootNotify();
void SendUUIDNotify();
void SendURNNotify();
int uPnP_GenerateDeviceXML();

void uPnP_RecieveMsg(ESP8266_SLIP_MESSAGE *msg) {

    if (memcmp(msg->Data, "M-SEARCH * HTTP", 15) != 0) {
        //Log("   uPnP is NOT an M-SEARCH * HTTP Request\r\n");
        return;
    }

    unsigned long tmr;
    float random = (float) (TMR1 & 0xFF) / 765;
    if (random < 0.1) random += 0.1;
    unsigned long RandInc = (random * SYSTEM_TIMER_FREQ);
    GetTime(tmr);
    msg->Data[msg->DataLength - 1] = 0;
    if (strstr((char *) msg->Data, "ssdp:all") != NULL) {
        Log("uPnP Recived M-SEARCH ssdp:all\r\n");
        tmrSendRoot = tmr + RandInc;
        tmrSendUUID = tmrSendRoot + RandInc;
        tmrSendURN = tmrSendUUID + RandInc;
    } else if (strstr((char *) msg->Data, "upnp:rootdevice") != NULL) {
        tmrSendRoot = tmr + RandInc;
        Log("uPnP Recived M-SEARCH upnp:rootdevice\r\n");
    } else if (strstr((char *) msg->Data, uPnP_str_UUID) != NULL) {
        tmrSendUUID = tmrSendRoot + RandInc;
        Log("uPnP Recived M-SEARCH for ME!!!\r\n");
    } else if (strstr((char *) msg->Data, "device:Basic") != NULL) {
        Log("uPnP Recived M-SEARCH for device:Basic\r\n");
        tmrSendURN = tmrSendUUID + RandInc;
    }

}

void uPnP_ProcessLoop() {
    char sendRoot, sendUUID, sendURN;

    DISABLE_INTERRUPTS;
    if (Timer500Hz > tmrSendRoot) sendRoot = 1;
    else sendRoot = 0;
    if (Timer500Hz > tmrSendUUID) sendUUID = 1;
    else sendUUID = 0;
    if (Timer500Hz > tmrSendURN) sendURN = 1;
    else sendURN = 0;
    ENABLE_INTERRUPTS;

    if (sendRoot) {
        Log("uPnP: Sending Root Notify\r\n");
        ESP_uPnP_BeginSend();
        SendRootNotify();
        ESP_uPnP_CompleteSend();
        DISABLE_INTERRUPTS;
        tmrSendRoot = Timer500Hz + uPnP_Interval;
        ENABLE_INTERRUPTS;
        return;
    }

    if (sendUUID) {
        Log("uPnP: Sending Device UUID Notify\r\n");
        ESP_uPnP_BeginSend();
        SendUUIDNotify();
        ESP_uPnP_CompleteSend();
        DISABLE_INTERRUPTS;
        tmrSendUUID = Timer500Hz + uPnP_Interval;
        ENABLE_INTERRUPTS;
        return;
    }

    if (sendURN) {
        Log("uPnP: Sending URN Notify\r\n");
        ESP_uPnP_BeginSend();
        SendURNNotify();
        ESP_uPnP_CompleteSend();
        DISABLE_INTERRUPTS;
        tmrSendURN = Timer500Hz + uPnP_Interval;
        ENABLE_INTERRUPTS;
        return;
    }

}

void uPnP_Init(const char *Name, const char *UUID, unsigned long IPAddress, char GenerateNewXML) {
    unsigned long tmr;
    GetTime(tmr);

    union {
        unsigned long ul;
        BYTE b[4];
    } IP;

    IP.ul = IPAddress;

    sprintf(uPnP_str_IPAddress, "%d.%d.%d.%d", IP.b[3], IP.b[2], IP.b[1], IP.b[0]);
    sprintf(uPnP_str_UUID, "%s", UUID);
    sprintf(uPnP_str_Name, "%s", Name);

    //if (GenerateNewXML) uPnP_GenerateDeviceXML();

    tmrSendRoot = tmr + SYSTEM_TIMER_FREQ;
    tmrSendUUID = tmrSendRoot + (SYSTEM_TIMER_FREQ * 0.1);
    tmrSendURN = tmrSendUUID + (SYSTEM_TIMER_FREQ * 0.1);

}

void SendRootNotify() {
    circularPrintf(txFIFO,
            "NOTIFY * HTTP/1.1\r\n"
            "HOST: 239.255.255.250:1900\r\n"
            "NT: upnp:rootdevice\r\n"
            "NTS: ssdp:alive\r\n"
            "LOCATION: http://%s/device.xml\r\n"
            "USN: uuid:%s::upnp:rootdevice\r\n"
            "Cache-Control: max-age = 1800\r\n"
            "SERVER: CinefluxEmbedded/1.0 UPnP/1.0 UPnP-Device-Host/1.0\r\n"
            "\r\n", uPnP_str_IPAddress, uPnP_str_UUID);
}

void SendUUIDNotify() {
    circularPrintf(txFIFO,
            "NOTIFY * HTTP/1.1\r\n"
            "HOST: 239.255.255.250:1900\r\n"
            "CACHE-CONTROL: max-age = 1800\r\n"
            "LOCATION: HTTP://%s/device.xml\r\n"
            "NT: uuid:%s\r\n"
            "NTS: ssdp:alive\r\n"
            "SERVER: CinefluxEmbedded/1.0 UPnP/1.0 Radian/1.0\r\n"
            "USN: uuid:%s\r\n"
            "\r\n", uPnP_str_IPAddress, uPnP_str_UUID, uPnP_str_UUID);
}

void SendURNNotify() {
    circularPrintf(txFIFO,
            "NOTIFY * HTTP/1.1\r\n"
            "HOST: 239.255.255.250:1900\r\n"
            "CACHE-CONTROL: max-age = 1800\r\n"
            "LOCATION: HTTP://%s/device.xml\r\n"
            "NT: urn:schemas-upnp-org:device:Basic:1\r\n"
            "NTS: ssdp:alive\r\n"
            "SERVER: CinefluxEmbedded/1.0 UPnP/1.0 Radian/1.0\r\n"
            "USN: uuid:%s::urn:schemas-upnp-org:device:Basic:1\r\n"
            "\r\n", uPnP_str_IPAddress, uPnP_str_UUID);
}

void uPnP_GetDeviceXML(char *outCursor) {
    const char *inputCursor = XML_TEMPLATE;
    while (*inputCursor) {
        char token = *(inputCursor++);
        if (token == '@') {
            token = *(inputCursor++);
            switch (token) {
                case 'I':
                    outCursor += sprintf(outCursor, "%s", uPnP_str_IPAddress);
                    break;
                case 'N':
                    outCursor += sprintf(outCursor, "%s", uPnP_str_Name);
                    break;
                case 'U':
                    outCursor += sprintf(outCursor, "%s", uPnP_str_UUID);
                    break;
                default:
                    *(outCursor++) = '@';
                    *(outCursor++) = token;
                    break;
            }
        } else {
            *(outCursor++) = token;
        }
    }
    *(outCursor++) = 0; //Null Terminate
}

