/* 
 * File:   ESP8266.h
 * Author: THORAXIUM
 *
 * Created on March 3, 2015, 8:05 PM
 */

#ifndef ESP8266_H
#define	ESP8266_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "main.h"
#include "integer.h"

    void ESP_RX_FIFO_Processor();
    int ESP_Init();
    int ESP_mDNS_BeginSend();
    int ESP_mDNS_CompleteSend();
    int ESP_uPnP_BeginSend();
    int ESP_uPnP_CompleteSend();
    int ESP_TCP_StartStream(BYTE ChannelID);
    int ESP_TCP_TriggerWiFi_Send(BYTE ChannelID);
    int ESP_TCP_Wait_WiFi_SendCompleted(BYTE ChannelID);
    int ESP_TCP_CloseConnection(BYTE ChannelID);
    int ESP_TCP_CancelSend(BYTE ChannelID);
    void ESP_StreamArray(BYTE *data, int len);

    typedef union {
        float f;
        unsigned long ul;
        long l;
        unsigned int ui[2];
        int i[2];
        unsigned char ub[4];
        char b[4];
    } OMNI;

    typedef struct {
        int MessageType;
        int TCP_ChannelID;
        int ResponseCode;
        OMNI Detail;
        int DataLength;
        BYTE __attribute__((aligned)) Data[1536];
    } MESSAGE;

    typedef struct {
        int MessageType;
        int TCP_ChannelID;
        int ResponseCode;
        OMNI Detail;
        int DataLength;
        BYTE __attribute__((aligned)) Data[1];
    } NOTIFY;

    typedef enum _auth_mode {
        AUTH_OPEN = 0,
        AUTH_WEP,
        AUTH_WPA_PSK,
        AUTH_WPA2_PSK,
        AUTH_WPA_WPA2_PSK,
        AUTH_MAX
    } AUTH_MODE;

    typedef enum _esp_mode {
        NULL_MODE = 0x00,
        STATION_MODE = 0x01,
        SOFTAP_MODE = 0x02,
        STATIONAP_MODE = 0x03
    } ESP_MODE;

    typedef struct {
        char Name[64];
        char UUID[40];
        ESP_MODE Mode;
        AUTH_MODE EncryptionMode;
        int Channel;
        char SSID[32];
        char Password[32];
        char HA1[34];        
    } ESP8266_CONFIG;

    extern ESP8266_CONFIG ESP_Config;

    typedef union {
        unsigned long l;
        unsigned char b[4];
    } IP_Address_type;

    extern IP_Address_type IP_Address;

#ifdef	__cplusplus
}
#endif

#endif	/* ESP8266_H */

