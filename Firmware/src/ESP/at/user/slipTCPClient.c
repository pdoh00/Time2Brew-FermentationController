#include "../../sdk/include/ets_sys.h"
#include "driver/uart.h"
#include "../../sdk/include/osapi.h"
#include "../../sdk/include/mem.h"
#include "../../sdk/include/user_interface.h"
#include "../../sdk/include/os_type.h"
#include "../../sdk/include/espconn.h"
#include "ProtocolDefs.h"
#include "../../sdk/include/c_types.h"
#include "user_main.h"

void ICACHE_FLASH_ATTR slipTCP_Server_OnIncommingConnection(void *arg) {
    if (arg == NULL) return;

    struct espconn *con = (struct espconn *) arg;

    uint8_t x;
    type_IP_Channel *chan;
    for (x = 0; x < ChannelCount; x++) {
        chan = &TCP_Channels[x];
        if (chan->pCon == NULL && chan->linkEn == false) {
            con->reverse = chan;
            chan->linkId = x;
            chan->pCon = con;
            chan->linkEn = true;
            espconn_regist_recvcb(con, slipTCPClient_OnRecieve);
            espconn_regist_reconcb(con, slipTCPClient_OnReconnect);
            espconn_regist_disconcb(con, slipTCPClient_OnDisconnect);
            espconn_regist_sentcb(con, slipTCPClient_OnSendComplete);
            return;
        }
    }

}

void ICACHE_FLASH_ATTR slipTCPClient_OnDisconnect(void *arg) {
    struct espconn *con = (struct espconn *) arg;
    if (con == NULL) return;
    type_IP_Channel *channel = (type_IP_Channel *) con->reverse;
    if (channel == NULL) return;

    if (channel->isSending == true) {
        channel->isSending = false;
        SendMessage(ESP_TCP_SEND_RESULT, channel->linkId, SendFail_Closed, 0, 0, 0, 0, NULL, 0);
    }

    //SendMessage(ESP_TCP_CLOSED, channel->linkId, 0, 0, 0, 0, 0, NULL, 0);

    channel->linkEn = false;
    channel->pCon = NULL;
    channel->isSending = false;
    return;
}

void ICACHE_FLASH_ATTR slipTCPClient_OnReconnect(void *arg, sint8 err) {
    struct espconn *con = (struct espconn *) arg;
    if (con == NULL) return;
    type_IP_Channel *channel = (type_IP_Channel *) con->reverse;
    if (channel == NULL) return;

    if (channel->isSending == true) {
        channel->isSending = false;
        SendMessage(ESP_TCP_SEND_RESULT, channel->linkId, SendFail_Reconnect, 0, 0, 0, 0, NULL, 0);
    }

    //SendMessage(ESP_TCP_CLOSED, channel->linkId, 0, 0, 0, 0, 0, NULL, 0);

    channel->linkEn = false;
    channel->pCon = NULL;
    channel->isSending = false;
    return;
}

void ICACHE_FLASH_ATTR slipTCPClient_OnRecieve(void *arg, char *pdata, unsigned short len) {
    if (arg == NULL) return;
    struct espconn *con = (struct espconn *) arg;
    if (con->reverse == NULL) return;
    type_IP_Channel *channel = (type_IP_Channel *) con->reverse;
    if (channel->pCon == NULL) return;
    if (channel->linkEn == false) return;
    SendMessage(ESP_TCP_RECIEVE, channel->linkId, 0, 0, 0, 0, 0, (uint8_t *) pdata, len);
}

void ICACHE_FLASH_ATTR slipTCPClient_OnSendComplete(void *arg) {
    if (arg == NULL) return;
    struct espconn *con = (struct espconn *) arg;
    if (con->reverse == NULL) return;
    type_IP_Channel *channel = (type_IP_Channel *) con->reverse;
    if (channel->pCon == NULL) return;
    if (channel->linkEn == false) return;
    if (channel->isSending == false) return;
    channel->isSending = false;
    SendMessage(ESP_TCP_SEND_RESULT, channel->linkId, SendOK, 0, 0, 0, 0, NULL, 0);
}

void ICACHE_FLASH_ATTR slip_mDNS_OnRecieve(void *arg, char *pdata, unsigned short len) {
    if (arg == NULL) return;
    struct espconn *con = (struct espconn *) arg;

    con->proto.udp->local_port = 5353;
    con->proto.udp->remote_port = 5353;
    con->proto.udp->remote_ip[0] = 224;
    con->proto.udp->remote_ip[1] = 0;
    con->proto.udp->remote_ip[2] = 0;
    con->proto.udp->remote_ip[3] = 251;

    SendMessage(ESP_MDNS_RECIEVE, 0, 0, 0, 0, 0, 0, (uint8_t *) pdata, len);
}

void ICACHE_FLASH_ATTR slip_mDNS_OnSendComplete(void *arg) {
    if (mDNS_Con.isSending == false) return;
    mDNS_Con.isSending = false;
    SendMessage(ESP_MDNS_SEND_RESULT, 0, SendOK, 0, 0, 0, 0, NULL, 0);
}

void ICACHE_FLASH_ATTR slip_uPnP_OnRecieve(void *arg, char *pdata, unsigned short len) {
    if (arg == NULL) return;
    struct espconn *con = (struct espconn *) arg;

    con->proto.udp->local_port = 1900;
    con->proto.udp->remote_port = 1900;
    con->proto.udp->remote_ip[0] = 239;
    con->proto.udp->remote_ip[1] = 255;
    con->proto.udp->remote_ip[2] = 255;
    con->proto.udp->remote_ip[3] = 250;

    SendMessage(ESP_UPNP_RECIEVE, 0, 0, 0, 0, 0, 0, (uint8_t *) pdata, len);
}

void ICACHE_FLASH_ATTR slip_uPnP_OnSendComplete(void *arg) {
    if (uPnP_Con.isSending == false) return;
    uPnP_Con.isSending = false;
    SendMessage(ESP_UPNP_SEND_RESULT, 0, SendOK, 0, 0, 0, 0, NULL, 0);
}