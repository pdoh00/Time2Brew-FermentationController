#include "../../sdk/include/ets_sys.h"
#include "driver/uart.h"
#include "../../sdk/include/osapi.h"
#include "../../sdk/include/mem.h"
#include "../../sdk/include/user_interface.h"
#include "../../sdk/include/os_type.h"
#include "../../sdk/include/espconn.h"
#include "ProtocolDefs.h"
#include "user_main.h"

#define PushByte(x) ({*(txWriteCursor++) = x;if (txWriteCursor > txBufferEND) txWriteCursor = txBuffer;})
//#define PushByte(x) ({uart_tx_one_char(UART0, x);})
#define PushDataByte(x) ({if (x==ESCAPE_CHAR){PushByte(x);PushByte(x);}else{PushByte(x);}})

bool isInitialized = false;
bool isSyncFinished = false;
uint8_t current_wifiMode;
type_IP_Channel TCP_Channels[ChannelCount];
type_IP_Channel mDNS_Con, uPnP_Con, TCP_Server;

struct station_config stationConf;

struct softap_config apConfig;

os_timer_t tmr_WifiConnectCompletedCheck;
os_timer_t tmr_WifiHasIPCheck;
os_timer_t tmr_SyncSend;

os_event_t rxTaskQueue[rxTaskQueueLen];
os_event_t txTaskQueue[txTaskQueueLen];

struct ip_info LocalIP;

uint8_t rxBufferActive = 0;
uint8_t rxBufferA[11776];
uint8_t rxBufferB[11776];
uint8_t *rxBuffer = rxBufferA;
uint8_t *rxCursor = rxBufferA;
uint8_t *rxCursorEND = rxBufferA + 11776;

uint8_t txBuffer[4096];
uint8_t *txWriteCursor = txBuffer;
uint8_t *txReadCursor = txBuffer;
uint8_t *txBufferEND = txBuffer + 4096;

void user_init(void) {
    wifi_station_set_auto_connect(0);
    uint8_t x;
    uart_init(BIT_RATE_921600, BIT_RATE_921600);
    current_wifiMode = wifi_get_opmode();
    system_os_task(rxTask, rxTaskPrio, rxTaskQueue, rxTaskQueueLen);
    system_os_task(txTask, txTaskPrio, txTaskQueue, txTaskQueueLen);
    for (x = 0; x < ChannelCount; x++) {
        TCP_Channels[x].linkId = x;
        TCP_Channels[x].linkEn = false;
    }
    wifi_station_disconnect();

    os_timer_disarm(&tmr_SyncSend);
    os_timer_setfn(&tmr_SyncSend, (os_timer_func_t *) SyncSend_OnTimer, NULL);
    os_timer_arm(&tmr_SyncSend, 10, 0);
}

/*This task is called when the uart rx interrupt handler posts a message to the task.
 Before that it disables the interrupts so that the task isn't posted for every byte
 recieved.  We must ensure that we re-enable the interrupt afterwards to re-arm*/
void ICACHE_FLASH_ATTR rxTask(ETSEvent *e) {
    static uint8_t isEscaping = false;
    static uint8_t isStarted = false;
    static uint16_t len = 0;
    uint8_t rxByte;

    while (READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) {
        WRITE_PERI_REG(0X60000914, 0x73); //WTD
        rxByte = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;

        if (isEscaping == true) { //The previous rxByte was an ESCAPE character so this byte is an escaped command
            isEscaping = false;
            switch (rxByte) {
                case ESCAPE_CHAR: //The data stream contains the ESCAPE character... so instert it.
                    if (isStarted == true) {
                        if (rxCursor != rxCursorEND) {
                            *(rxCursor++) = ESCAPE_CHAR;
                            len++;
                        }
                    } else {
                        SendMessage(ESP_SEND_GEN_MESSAGE, 0, Invalid_SLIP_Packet, 1, 1, 1, 1, NULL, 0);
                        rxCursor = rxBuffer;
                    }
                    break;
                case END_OF_MESSAGE://End Packet Token
                    ProcessMessage(rxBuffer, len);
                    rxBufferActive++;
                    if (rxBufferActive == 2) rxBufferActive = 0;
                    switch (rxBufferActive) {
                        case 0:
                            rxBuffer = rxBufferA;
                            break;
                        case 1:
                            rxBuffer = rxBufferB;
                            break;
                    }
                    rxCursor = rxBuffer;
                    rxCursorEND = rxBuffer + 11776;
                    isStarted = false;
                    len = 0;
                    break;
                case START_OF_MESSAGE://Start Packet Token
                    isSyncFinished = true; //The sync must be done since we've recieved two known characters...
                    rxCursor = rxBuffer;
                    isStarted = true;
                    len = 0;
                    break;
                default: //Unknown command....ignore and start over!
                    //Maybe this should send back an error to the MCU?
                    SendMessage(ESP_SEND_GEN_MESSAGE, 0, Invalid_SLIP_Packet, 2, 2, 2, 2, NULL, 0);
                    rxCursor = rxBuffer;
                    isStarted = false;
                    break;
            }
        } else {
            //Okay so the previous byte was NOT an escaped command...

            if (rxByte == ESCAPE_CHAR) { //Is this byte the ESCAPE Charcter?
                isEscaping = true; //Yes - so flag it so that the next byte is properly interpreted as an escaped command
            } else {
                //Okay so this is a normal character
                if (isStarted == true) { //Has the message start command been recieved?
                    //Yes - the message has been started... is there room in the buffer?
                    if (rxCursor != rxCursorEND) {
                        *(rxCursor++) = rxByte; //There is-- so push it in.
                        len++;
                    } else {
                        SendMessage(ESP_SEND_GEN_MESSAGE, 0, Invalid_SLIP_Packet, 3, 3, 3, 3, NULL, 0);
                        rxCursor = rxBuffer;
                        isStarted = false;
                    }
                }
            }
        }
        //Repeat until the FIFO buffer is empty...
    }
    //Re-Enable the UART Interrupt since it was disabled in the interrupt handler and we want to re-arm it
    if (UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST)) {
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
    } else if (UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_TOUT_INT_ST)) {
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
    }
    ETS_UART_INTR_ENABLE();
}

/*This task is called by posting to the event queue.  Initially this doesn't get called.
 But, when anyone writes to the TX circular buffer they MUST post a message to get this
 function queued up.

 When this function runs it will put data into the TX FIFO until it's full or there isn't anymore data.
 If there is more data to send this function will post a message for itself...therefore becoming
 self-sustaining until all the data has been sent.*/
void ICACHE_FLASH_ATTR txTask(ETSEvent * e) {
    uint32 fifo_cnt;
    while (1) {
        WRITE_PERI_REG(0X60000914, 0x73); //WTD
        fifo_cnt = READ_PERI_REG(UART_STATUS(UART0)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);
        //Is the TX FIFO Full?
        if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) >= 126) {
            //Since the FIFO is full we're done.  But make sure we call ourselved again...
            system_os_post(txTaskPrio, 0, 0);
            return;
        }
        //Is there anything left to send?
        if (txReadCursor == txWriteCursor) {
            //The TX Buffer is empty so return...Do not ask the OS to come back to us
            //Instead whenever someone writes to the buffer they should post
            //the message asking the OS to run this function again...            
            return;
        }
        WRITE_PERI_REG(UART_FIFO(UART0), *(txReadCursor++));
        if (txReadCursor > txBufferEND) txReadCursor = txBuffer;
    }
}

/*After a pull message has been recieved (Escaped Start...Data...Escaped END) this function is called.
 It will dispatch the correct function based on the command ID*/
void ICACHE_FLASH_ATTR ProcessMessage(uint8_t *buffer, uint16_t len) {
    /*The first byte of the message contains the command...so look at it and go!
    We send buffer[1] and len-1 so that the function doesn't have to worry about
    all the preamble...*/
    switch (buffer[0]) {
        case MCU_TCP_ASYNCSEND:
            cmd_MCU_TCP_ASYNCSEND(&buffer[1], len - 1);
            break;
        case MCU_UPNP_ASYNCSEND:
            cmd_MCU_uPnP_ASYNCSEND(&buffer[1], len - 1);
            break;
        case MCU_MDNS_ASYNCSEND:
            cmd_MCU_mDNS_ASYNCSEND(&buffer[1], len - 1);
            break;
        case MCU_INIT:
            cmd_MCU_INIT(&buffer[1], len - 1);
            break;
        case MCU_TCP_CLOSE_CONNECTION:
            cmd_MCU_TCP_CLOSE_CONNECTION(&buffer[1], len - 1);
            break;
        case MCU_START_MDNS:
            cmd_MCU_Start_mDNS();
            break;
        case MCU_START_UPNP:
            cmd_MCU_Start_uPnP();
            break;
        case MCU_GET_IP:
            cmd_MCU_GetIP();
            break;
        default:
            break;
    }
}

void ICACHE_FLASH_ATTR cmd_MCU_INIT(uint8_t *buffer, uint16_t len) {
    uint8_t *cursor = buffer;
    char *dstCursor;
    bool bres;
    if (isInitialized == true) {
        SendMessage(ESP_INIT_RESP, 0, Fail_AlreadyInitialized, 0, 0, 0, 0, NULL, 0);
        return;
    }

    uint8 mode = *(cursor++);

    if (mode != wifi_get_opmode()) {
        ETS_UART_INTR_DISABLE();
        bres = wifi_set_opmode(mode);
        ETS_UART_INTR_ENABLE();
        if (bres == false) {
            SendMessage(ESP_INIT_RESP, 0, Fail_UnableToSetWifiOpMode, 0, 0, 0, 0, NULL, 0);
            return;
        }
    }


    if (mode == STATION_MODE) {
        current_wifiMode = STATION_MODE;

        os_bzero(&stationConf, sizeof (struct station_config));
        dstCursor = stationConf.ssid;
        while (*cursor) {
            *(dstCursor++) = *(cursor++);
        }
        *(dstCursor++) = *(cursor++);

        dstCursor = stationConf.password;
        while (*cursor) {
            *(dstCursor++) = *(cursor++);
        }
        *(dstCursor++) = *(cursor++);

        stationConf.bssid[0] = 0;
        stationConf.bssid_set = 0;

        wifi_station_disconnect();

        ETS_UART_INTR_DISABLE();
        bres = wifi_station_set_config(&stationConf);
        ETS_UART_INTR_ENABLE();
        if (bres == false) {
            SendMessage(ESP_INIT_RESP, 0, Fail_UnableToSetWifiConfig, 0, 0, 0, 0, NULL, 0);
            return;
        }

        bres = wifi_station_connect();
        if (bres == false) {
            SendMessage(ESP_INIT_RESP, 0, FailOnStationConnect, 0, 0, 0, 0, NULL, 0);
            return;
        }

        SendMessage(0xEE, 1, 1, 0, 0, 0, 0, NULL, 0);

        os_timer_disarm(&tmr_WifiConnectCompletedCheck);
        os_timer_setfn(&tmr_WifiConnectCompletedCheck, (os_timer_func_t *) WifiConnectCompletedCheck_OnTimer, NULL);
        WifiConnectCompletedCheck_OnTimer(NULL);
        return;

    } else if (mode == SOFTAP_MODE) {
        current_wifiMode = SOFTAP_MODE;

        os_bzero(&apConfig, sizeof (struct softap_config));
        bres = wifi_softap_get_config(&apConfig);
        if (bres == false) {
            SendMessage(ESP_INIT_RESP, 0, Fail_UnableToGetSoftAP_Config, 0, 0, 0, 0, NULL, 0);
            return;
        }

        dstCursor = apConfig.ssid;
        while (*cursor) {
            *(dstCursor++) = *(cursor++);
        }
        *(dstCursor++) = *(cursor++);


        dstCursor = apConfig.password;
        while (*cursor) {
            *(dstCursor++) = *(cursor++);
        }
        *(dstCursor++) = *(cursor++);

        apConfig.ssid_len = strlen(apConfig.ssid);
        apConfig.channel = *(cursor++);
        apConfig.authmode = *(cursor++);
        apConfig.ssid_hidden = 0;
        apConfig.max_connection = 4;
        apConfig.beacon_interval = 100;

        ETS_UART_INTR_DISABLE();
        bres = wifi_softap_set_config(&apConfig);
        ETS_UART_INTR_ENABLE();
        if (bres == false) {
            SendMessage(ESP_INIT_RESP, 0, Fail_UnableToSetSoftAP_Config, 0, 0, 0, 0, NULL, 0);
            return;
        }
        SendMessage(0xEE, 2, 2, 0, 0, 0, 0, NULL, 0);
        os_timer_disarm(&tmr_WifiHasIPCheck);
        os_timer_setfn(&tmr_WifiHasIPCheck, (os_timer_func_t *) WifiHasIPCheck_OnTimer, NULL);
        WifiHasIPCheck_OnTimer(NULL);
    } else {
        SendMessage(ESP_INIT_RESP, 0, Fail_Init_ModeIsInvalid, 0, 0, 0, 0, NULL, 0);
    }
}

void ICACHE_FLASH_ATTR WifiHasIPCheck_OnTimer(void *timer_arg) {
    static uint8_t retryCount = 0;
    os_timer_disarm(&tmr_WifiHasIPCheck);
    SendMessage(0xEE, 3, 3, retryCount, 0, 0, 0, NULL, 0);

    if (retryCount++ > 20) {
        retryCount = 0;
        SendMessage(ESP_INIT_RESP, 0, Fail_UnableToGetValidIP, 0, 0, 0, 0, NULL, 0);
        return;
    }

    LocalIP.ip.addr = 0;

    if (wifi_get_opmode() == SOFTAP_MODE) {
        wifi_get_ip_info(0x01, &LocalIP);
    } else {
        wifi_get_ip_info(0x00, &LocalIP);
    }

    if (LocalIP.ip.addr != 0 && LocalIP.ip.addr != 0xFFFFFFFF) {
        retryCount = 0;
        SendMessage(0xEE, 4, 4, retryCount, 0, 0, 0, NULL, 0);
        cmd_MCU_INIT_afterWifiHasIP();
        return;
    }

    os_timer_arm(&tmr_WifiHasIPCheck, 1000, 0);
}

void ICACHE_FLASH_ATTR WifiConnectCompletedCheck_OnTimer(void *timer_arg) {
    static uint8_t retryCount = 0;
    os_timer_disarm(&tmr_WifiConnectCompletedCheck);
    SendMessage(0xEE, 5, 5, retryCount, 0, 0, 0, NULL, 0);
    retryCount++;
    uint8_t status = wifi_station_get_connect_status();
    if (status == STATION_GOT_IP) {
        retryCount = 0;
        SendMessage(0xEE, 6, 6, retryCount, 0, 0, 0, NULL, 0);
        os_timer_disarm(&tmr_WifiHasIPCheck);
        os_timer_setfn(&tmr_WifiHasIPCheck, (os_timer_func_t *) WifiHasIPCheck_OnTimer, NULL);
        os_timer_arm(&tmr_WifiHasIPCheck, 100, 0);
        return;
    } else if (retryCount >= 30) {
        wifi_station_disconnect();
        retryCount = 0;
        SendMessage(ESP_INIT_RESP, 0, Fail_UnableToConnect, 0, 0, 0, 0, (uint8_t *) & status, 1);
        return;
    } else {
        os_timer_arm(&tmr_WifiConnectCompletedCheck, 1000, 0);
    }
}

void ICACHE_FLASH_ATTR cmd_MCU_INIT_afterWifiHasIP() {
    sint8 res;
    SendMessage(0xEE, 7, 7, 0, 0, 0, 0, NULL, 0);

    union {
        uint32 l;
        uint8 ub[4];
    } temp;
    temp.l = LocalIP.ip.addr;
    SendMessage(ESP_IP_INFO, 0, 0, temp.ub[3], temp.ub[2], temp.ub[1], temp.ub[0], NULL, 0);

    //Start TCP Listener
    TCP_Server.pCon = (struct espconn *) os_zalloc(sizeof (struct espconn));
    if (TCP_Server.pCon == NULL) {
        SendMessage(ESP_INIT_RESP, 0, Fail_UnableToAllocateTCPServer, 0, 0, 0, 0, NULL, 0);
        return;
    }
    TCP_Server.pCon->type = ESPCONN_TCP;
    TCP_Server.pCon->state = ESPCONN_NONE;
    TCP_Server.pCon->proto.tcp = (esp_tcp *) os_zalloc(sizeof (esp_tcp));
    TCP_Server.pCon->proto.tcp->local_port = 80;
    TCP_Server.pCon->reverse = NULL;
    TCP_Server.linkEn = true;
    res = espconn_regist_connectcb(TCP_Server.pCon, slipTCP_Server_OnIncommingConnection);
    if (res != ESPCONN_OK) {
        SendMessage(ESP_INIT_RESP, 0, Fail_UnableToRegisterTCP_ConnectCB, 0, 0, 0, 0, (uint8_t *) & res, 1);
        return;
    }
    res = espconn_accept(TCP_Server.pCon);
    if (res != ESPCONN_OK) {
        SendMessage(ESP_INIT_RESP, 0, Fail_UnableToStartTCPListener, 0, 0, 0, 0, (uint8_t *) & res, 1);
        return;
    }
    res = espconn_regist_time(TCP_Server.pCon, 5, 0);
    if (res != ESPCONN_OK) {
        SendMessage(ESP_INIT_RESP, 0, Fail_UnableToSetTCPTimeout, 0, 0, 0, 0, (uint8_t *) & res, 1);
        return;
    }

    isInitialized = true;
    SendMessage(ESP_INIT_RESP, 0, Init_OK, 0, 0, 0, 0, NULL, 0);
}

void ICACHE_FLASH_ATTR cmd_MCU_Start_mDNS() {
    sint8 res;
    struct ip_info mDNS_MulticastIP;

    /*-----------------------------------------
     * Start mDNS Connection
     ------------------------------------------*/
    mDNS_Con.pCon = (struct espconn *) os_zalloc(sizeof (struct espconn));
    if (mDNS_Con.pCon == NULL) {
        SendMessage(ESP_START_MDNS_RESP, 0, Fail_UnableToAllocatemDNSServer, 0, 0, 0, 0, NULL, 0);
        return;
    }

    mDNS_Con.pCon->type = ESPCONN_UDP;
    mDNS_Con.pCon->state = ESPCONN_NONE;
    mDNS_Con.pCon->proto.udp = (esp_udp *) os_zalloc(sizeof (esp_udp));
    mDNS_Con.pCon->proto.udp->local_port = 5353;
    mDNS_Con.pCon->proto.udp->remote_port = 5353;
    mDNS_Con.pCon->proto.udp->remote_ip[0] = 224;
    mDNS_Con.pCon->proto.udp->remote_ip[1] = 0;
    mDNS_Con.pCon->proto.udp->remote_ip[2] = 0;
    mDNS_Con.pCon->proto.udp->remote_ip[3] = 251;
    mDNS_Con.pCon->reverse = NULL;
    res = espconn_regist_recvcb(mDNS_Con.pCon, slip_mDNS_OnRecieve);
    if (res != ESPCONN_OK) {
        SendMessage(ESP_START_MDNS_RESP, 0, Fail_UnableToRegister_mDNS_RecieveCB, 0, 0, 0, 0, (uint8_t *) & res, 1);
        return;
    }
    res = espconn_regist_sentcb(mDNS_Con.pCon, slip_mDNS_OnSendComplete);
    if (res != ESPCONN_OK) {
        SendMessage(ESP_START_MDNS_RESP, 0, Fail_UnableToRegister_mDNS_SentCB, 0, 0, 0, 0, (uint8_t *) & res, 1);
        return;
    }
    res = espconn_create(mDNS_Con.pCon);
    if (res != ESPCONN_OK) {
        SendMessage(ESP_START_MDNS_RESP, 0, Fail_UnableCreate_mDNS_Connection, 0, 0, 0, 0, (uint8_t *) & res, 1);
        return;
    }
    IP4_ADDR(&mDNS_MulticastIP.ip, 224, 0, 0, 251);
    res = espconn_igmp_join(&LocalIP.ip, &mDNS_MulticastIP.ip);
    if (res != ESPCONN_OK) {
        SendMessage(ESP_START_MDNS_RESP, 0, Fail_UnableJoin_mDNS_IGMP, 0, 0, 0, 0, (uint8_t *) & res, 1);
        return;
    }
    SendMessage(ESP_START_MDNS_RESP, 0, Init_OK, 0, 0, 0, 0, NULL, 0);
}

void ICACHE_FLASH_ATTR cmd_MCU_Start_uPnP() {
    sint8 res;
    struct ip_info uPnP_MulticastIP;
    /*-----------------------------------------
     * Start uPnP Connection
       ------------------------------------------*/
    uPnP_Con.pCon = (struct espconn *) os_zalloc(sizeof (struct espconn));
    if (uPnP_Con.pCon == NULL) {
        SendMessage(ESP_START_UPNP_RESP, 0, Fail_UnableToAllocateuPnPServer, 0, 0, 0, 0, NULL, 0);
        return;
    }
    uPnP_Con.pCon->type = ESPCONN_UDP;
    uPnP_Con.pCon->state = ESPCONN_NONE;
    uPnP_Con.pCon->proto.udp = (esp_udp *) os_zalloc(sizeof (esp_udp));
    uPnP_Con.pCon->proto.udp->local_port = 1900;
    uPnP_Con.pCon->proto.udp->remote_port = 1900;
    uPnP_Con.pCon->proto.udp->remote_ip[0] = 239;
    uPnP_Con.pCon->proto.udp->remote_ip[1] = 255;
    uPnP_Con.pCon->proto.udp->remote_ip[2] = 255;
    uPnP_Con.pCon->proto.udp->remote_ip[3] = 250;

    uPnP_Con.pCon->reverse = NULL;
    res = espconn_regist_recvcb(uPnP_Con.pCon, slip_uPnP_OnRecieve);
    if (res != ESPCONN_OK) {
        SendMessage(ESP_START_UPNP_RESP, 0, Fail_UnableToRegister_uPnP_RecieveCB, 0, 0, 0, 0, (uint8_t *) & res, 1);
        return;
    }
    res = espconn_regist_sentcb(uPnP_Con.pCon, slip_uPnP_OnSendComplete);
    if (res != ESPCONN_OK) {
        SendMessage(ESP_START_UPNP_RESP, 0, Fail_UnableToRegister_uPnP_SentCB, 0, 0, 0, 0, (uint8_t *) & res, 1);
        return;
    }
    res = espconn_create(uPnP_Con.pCon);
    if (res != ESPCONN_OK) {
        SendMessage(ESP_START_UPNP_RESP, 0, Fail_UnableCreate_uPnP_Connection, 0, 0, 0, 0, (uint8_t *) & res, 1);
        return;
    }

    IP4_ADDR(&uPnP_MulticastIP.ip, 239, 255, 255, 250);
    res = espconn_igmp_join(&LocalIP.ip, &uPnP_MulticastIP.ip);
    if (res != ESPCONN_OK) {
        SendMessage(ESP_START_UPNP_RESP, 0, Fail_UnableJoin_uPnP_IGMP, 0, 0, 0, 0, (uint8_t *) & res, 1);
        return;
    }
    SendMessage(ESP_START_UPNP_RESP, 0, Init_OK, 0, 0, 0, 0, NULL, 0);
}

void ICACHE_FLASH_ATTR cmd_MCU_GetIP() {

    union {
        uint32 l;
        uint8 ub[4];
    } temp;

    LocalIP.ip.addr = 0;
    if (wifi_get_opmode() == SOFTAP_MODE) {
        wifi_get_ip_info(0x01, &LocalIP);
    } else {
        wifi_get_ip_info(0x00, &LocalIP);
    }

    temp.l = LocalIP.ip.addr;
    SendMessage(ESP_IP_INFO, 0, 0, temp.ub[3], temp.ub[2], temp.ub[1], temp.ub[0], NULL, 0);
}

void ICACHE_FLASH_ATTR cmd_MCU_TCP_ASYNCSEND(uint8_t *buffer, uint16_t len) {
    uint8_t id = buffer[0];
    uint8_t *txData = &buffer[1];
    uint16_t txDataLength = len - 1;

    type_IP_Channel *chan = &TCP_Channels[id];

    if (chan->pCon == NULL) {
        SendMessage(ESP_TCP_SEND_RESULT, id, Fail_Channel_pCon_IsNull, 0, 0, 0, 0, NULL, 0);
        return;
    }

    if (chan->isSending == true) {
        SendMessage(ESP_TCP_SEND_RESULT, id, Fail_SendInProgress, 0, 0, 0, 0, NULL, 0);
        return;
    }

    if (chan->linkEn == false) {
        SendMessage(ESP_TCP_SEND_RESULT, id, Fail_LinkNotEnabled, 0, 0, 0, 0, NULL, 0);
        return;
    }
    chan->isSending = true;
    uint8_t res = espconn_sent(chan->pCon, txData, txDataLength);
    if (res != ESPCONN_OK) {
        TCP_Channels[id].isSending = false;
        SendMessage(ESP_TCP_SEND_RESULT, id, Fail_EspError, res, (len >> 8), (len & 0xFF), res, NULL, 0);
        return;
    }

    SendMessage(ESP_TCP_SEND_RESULT, id, InProgress_Sending, res, (len >> 8), (len & 0xFF), res, NULL, 0);
}

void ICACHE_FLASH_ATTR cmd_MCU_TCP_CLOSE_CONNECTION(uint8_t *buffer, uint16_t len) {
    uint8_t id = buffer[0];
    type_IP_Channel *chan = &TCP_Channels[id];

    if (chan->pCon == NULL) return;
    espconn_disconnect(chan->pCon);
}

void ICACHE_FLASH_ATTR cmd_MCU_mDNS_ASYNCSEND(uint8 *buffer, uint16 len) {
    if (mDNS_Con.isSending == true) {
        SendMessage(ESP_MDNS_SEND_RESULT, 0, Fail_SendInProgress, 0, 0, 0, 0, NULL, 0);
        return;
    }
    mDNS_Con.isSending = true;

    sint8_t res = espconn_sent(mDNS_Con.pCon, buffer, len);
    if (res != ESPCONN_OK) {
        mDNS_Con.isSending = false;
        SendMessage(ESP_MDNS_SEND_RESULT, 0, Fail_EspError, res, (len >> 8), (len & 0xFF), res, NULL, 0);
        return;
    }
}

void ICACHE_FLASH_ATTR cmd_MCU_uPnP_ASYNCSEND(uint8_t *buffer, uint16_t len) {
    if (uPnP_Con.isSending == true) {
        SendMessage(ESP_UPNP_SEND_RESULT, 0, Fail_SendInProgress, 0, 0, 0, 0, NULL, 0);
        return;
    }
    uPnP_Con.isSending = true;
    sint8_t res = espconn_sent(uPnP_Con.pCon, buffer, len);
    if (res != ESPCONN_OK) {
        uPnP_Con.isSending = false;
        SendMessage(ESP_UPNP_SEND_RESULT, 0, Fail_EspError, res, (len >> 8), (len & 0xFF), res, NULL, 0);
        return;
    }
}

void ICACHE_FLASH_ATTR SendMessage(uint8_t MessageID, uint8_t ChannelID, sint16_t ResponseCode,
        uint8_t dat0, uint8_t dat1, uint8_t dat2, uint8_t dat3,
        uint8_t *args, uint16_t arglen) {
    uint8_t *respCode = (uint8_t *) & ResponseCode;
    PushByte(ESCAPE_CHAR);
    PushByte(START_OF_MESSAGE);
    PushDataByte(MessageID);
    PushDataByte(ChannelID);
    PushDataByte(*respCode);
    PushDataByte(*(respCode + 1));
    PushDataByte((arglen >> 8) & 0xFF);
    PushDataByte(arglen & 0xFF);
    PushDataByte(dat0);
    PushDataByte(dat1);
    PushDataByte(dat2);
    PushDataByte(dat3);
    while (arglen--) {
        PushDataByte(*args);
        args++;
    }
    PushByte(ESCAPE_CHAR);
    PushByte(END_OF_MESSAGE);

    ETSEvent nothing;
    nothing.par = 0;
    nothing.sig = 0;
    txTask(&nothing);
    //system_os_post(txTaskPrio, 0, 0);
}

void ICACHE_FLASH_ATTR SyncSend_OnTimer(void *timer_arg) {
    os_timer_disarm(&tmr_SyncSend);
    if (isSyncFinished == true) return;
    uart_tx_one_char(UART0, 'U');
    os_timer_arm(&tmr_SyncSend, 10, 0);
}