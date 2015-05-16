/* 
 * File:   usermain.h
 * Author: THORAXIUM
 *
 * Created on February 27, 2015, 12:44 AM
 */

#ifndef USERMAIN_H
#define	USERMAIN_H

#ifdef	__cplusplus
extern "C" {
#endif

#define ChannelCount        10
#define rxTaskPrio          1
#define rxTaskQueueLen      64

#define txTaskPrio          0
#define txTaskQueueLen      64

    typedef struct {
        BOOL linkEn;
        BOOL isSending;
        uint8_t linkId;
        struct espconn *pCon;
    } type_IP_Channel;

    typedef struct {
        uint8_t wifiMode;
        uint8_t wifiEncryptionMode;
        uint8_t wifiChannel;
        char SSID[32];
        char Password[32];
    } slipConfig_type;

    extern type_IP_Channel TCP_Channels[ChannelCount];
    extern type_IP_Channel mDNS_Con, uPnP_Con;

    void user_init(void);
    void ICACHE_FLASH_ATTR rxTask(ETSEvent *e);
    void ICACHE_FLASH_ATTR txTask(ETSEvent * e);
    void ICACHE_FLASH_ATTR ProcessMessage(uint8_t *buffer, uint16_t len);
    void ICACHE_FLASH_ATTR cmd_MCU_INIT(uint8_t *buffer, uint16_t len);
    void ICACHE_FLASH_ATTR WifiHasIPCheck_OnTimer(void *timer_arg);
    void ICACHE_FLASH_ATTR WifiConnectCompletedCheck_OnTimer(void *timer_arg);
    void ICACHE_FLASH_ATTR cmd_MCU_INIT_afterWifiHasIP();
    void ICACHE_FLASH_ATTR cmd_MCU_TCP_ASYNCSEND(uint8_t *buffer, uint16_t len);
    void ICACHE_FLASH_ATTR cmd_MCU_TCP_CLOSE_CONNECTION(uint8_t *buffer, uint16_t len);
    void ICACHE_FLASH_ATTR cmd_MCU_uPnP_ASYNCSEND(uint8_t *buffer, uint16_t len);
    void ICACHE_FLASH_ATTR cmd_MCU_mDNS_ASYNCSEND(uint8_t *buffer, uint16_t len);
    void ICACHE_FLASH_ATTR cmd_MCU_Start_mDNS();
    void ICACHE_FLASH_ATTR cmd_MCU_Start_uPnP();
    void ICACHE_FLASH_ATTR cmd_MCU_GetIP();
    void ICACHE_FLASH_ATTR SendMessage(uint8_t MessageID, uint8_t ChannelID, sint16_t ResponseCode,
            uint8_t dat0, uint8_t dat1, uint8_t dat2, uint8_t dat3,
            uint8_t *args, uint16_t arglen);

    void ICACHE_FLASH_ATTR slipTCP_Server_OnIncommingConnection(void *arg);
    void ICACHE_FLASH_ATTR slipTCPClient_OnDisconnect(void *arg);
    void ICACHE_FLASH_ATTR slipTCPClient_OnRecieve(void *arg, char *pdata, unsigned short len);
    void ICACHE_FLASH_ATTR slipTCPClient_OnSendComplete(void *arg);
    void ICACHE_FLASH_ATTR slip_mDNS_OnRecieve(void *arg, char *pdata, unsigned short len);
    void ICACHE_FLASH_ATTR slip_mDNS_OnSendComplete(void *arg);
    void ICACHE_FLASH_ATTR slip_uPnP_OnRecieve(void *arg, char *pdata, unsigned short len);
    void ICACHE_FLASH_ATTR slip_uPnP_OnSendComplete(void *arg);
    void ICACHE_FLASH_ATTR slipTCPClient_OnReconnect(void *arg, sint8 err);
    void ICACHE_FLASH_ATTR SyncSend_OnTimer(void *timer_arg);

#ifdef	__cplusplus
}
#endif

#endif	/* USERMAIN_H */

