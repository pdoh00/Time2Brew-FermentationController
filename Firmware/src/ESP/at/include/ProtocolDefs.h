/*
 * File:   ProtocolDefs.h
 * Author: aaron
 *
 * Created on February 26, 2015, 4:46 PM
 */

#ifndef PROTOCOLDEFS_H
#define	PROTOCOLDEFS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define ESCAPE_CHAR 0xAA
#define START_OF_MESSAGE    0x66
#define END_OF_MESSAGE  0x77

#define MCU_TCP_ASYNCSEND   0x01
#define ESP_TCP_ASYNCSEND_RESP  0x02
#define ESP_TCP_SEND_RESULT  0x03
#define ESP_TCP_RECIEVE   0x04

#define MCU_UPNP_ASYNCSEND  0x05
#define ESP_UPNP_ASYNCSEND_RESP  0x06
#define ESP_UPNP_SEND_RESULT  0x07
#define ESP_UPNP_RECIEVE   0x08

#define MCU_MDNS_ASYNCSEND  0x09
#define ESP_MDNS_ASYNCSEND_RESP  0x0A
#define ESP_MDNS_SEND_RESULT  0x0B
#define ESP_MDNS_RECIEVE   0x0C

#define MCU_TCP_CLOSE_CONNECTION  0x0D
#define ESP_TCP_CLOSED  0x0E
#define ESP_TCP_RECONNECT  0x1A
#define ESP_TCP_CONNECT  0x1B
#define ESP_TCP_CONNECTFAIL  0x1C
#define ESP_TCP_RECV_ERROR  0x1D
#define ESP_TCP_CLOSE_ERROR 0x1E
#define ESP_TCP_SENDCOMPLETE_ERROR 0x1F

#define MCU_INIT  0x10
#define ESP_INIT_RESP 0x11
#define ESP_READY   0x12
#define ESP_IP_INFO 0x13

#define MCU_GET_IP  0x14
#define MCU_START_MDNS 0x15
#define MCU_START_UPNP 0x16
#define ESP_START_MDNS_RESP 0x17
#define ESP_START_UPNP_RESP 0x18

#define ESP_SEND_GEN_MESSAGE    0x19

#define ESP_WIFI_EVENT  0x20
#define ESP_BEACON_SENT 0x21

    typedef enum {
        Fail_UnableToGetSoftAP_Config = 1,
        Fail_UnableToSetSoftAP_Config = 2,
        Fail_UnableToGetValidIP = 3,
        FailOnStationConnect = 4,
        Fail_UnableToSetWifiConfig = 5,
        Fail_UnableToSetWifiOpMode = 6,
        Fail_UnableToConnect = 7,
        Fail_UnableToAllocateTCPServer = 8,
        Fail_UnableToAllocatemDNSServer = 9,
        Fail_UnableToAllocateuPnPServer = 10,
        Fail_UnableToRegisterTCP_ConnectCB = 11,
        Fail_UnableToSetTCPTimeout = 12,
        Fail_UnableToRegister_mDNS_RecieveCB = 13,
        Fail_UnableToStartTCPListener = 14,
        Fail_UnableToRegister_mDNS_SentCB = 15,
        Fail_UnableCreate_mDNS_Connection = 16,
        Fail_UnableJoin_mDNS_IGMP = 17,
        Fail_UnableToRegister_uPnP_SentCB = 18,
        Fail_UnableCreate_uPnP_Connection = 19,
        Fail_UnableJoin_uPnP_IGMP = 20,
        Fail_UnableToRegister_uPnP_RecieveCB = 21,
        Init_OK = 22,
        Fail_AlreadyInitialized = 23,
        Fail_SendInProgress = 24,
        Fail_LinkNotEnabled = 25,
        Fail_EspError = 26,
        Accepted = 27,
        Fail_Channel_pCon_IsNull = 28,
        SendFail_Timeout = 29,
        SendFail_Closed = 30,
        SendFail_Reconnect = 31,
        SendOK = 32,
        Fail_Init_ModeIsInvalid = 33,
        InProgress_Sending = 34,
        Invalid_SLIP_Packet = 35
    } ESP_INIT_RESP_codes;




#ifdef	__cplusplus
}
#endif

#endif	/* PROTOCOLDEFS_H */

