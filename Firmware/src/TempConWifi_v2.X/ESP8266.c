#include <stddef.h>
#include <p33Exxxx.h>
#include "main.h"
#include "ESP8266.h"
#include "circularPrintF.h"
#include "SystemConfiguration.h"
#include "mDNS.h"
#include "uPnP.h"
#include "http_server.h"

#define TCP_CHANNEL_COUNT 6
#define WaitForMsgStart 1
#define RecievingMsgHeader 2
#define RecievingMsgBody 3

typedef enum {
    TCP_SEND_STATE_Idle,
    TCP_SEND_STATE_Submitted,
    TCP_SEND_STATE_Sending,
    TCP_SEND_STATE_Success,
    TCP_SEND_STATE_Fail
} TCP_SEND_STATE;

typedef struct {
    int TCP_ChannelID;
    int IsOpen;
    int SendStateDetail;
    unsigned long timeout;
    TCP_SEND_STATE SendState;
} TCP_CHANNEL;

typedef enum {
    RESET_IDLE,
    RESET_IN_PROGRESS,
    RESET_FAIL,
    RESET_SUCESS
} RESET_STATE;

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


ESP8266_SLIP_MESSAGE Msg;
NOTIFY Notify;
TCP_CHANNEL TCP_Channels[TCP_CHANNEL_COUNT], uPnP_Channel, mDNS_Channel;

IP_Address_type IP_Address;

RESET_STATE ResetState = RESET_IDLE;
int FailReason;

int InvalidSlipMsgReported = 0;

int StartMDNS_State = 0;
int StartUPNP_State = 0;

static void ESP_ProcessMessage(ESP8266_SLIP_MESSAGE *msg);
static void DisposeMessage(ESP8266_SLIP_MESSAGE *dat);
static int ESP_Reset(float PowerOffTime);
static const char *translateESP_RESP_CODE(int code);

union {
    int i;
    unsigned char ub[2];
    char b[2];
} ESP_RX_FIFO_Processor_token;

void ESP_RX_FIFO_Processor() {
    static char state = WaitForMsgStart;
    static char isEscaping = 0;
    static char HeaderIdx = 0;
    static unsigned char *MsgDataCursor;
    static unsigned int rxByteCount = 0;

    BYTE rxByte;
    while (1) {
        _U2TXIF = 1;
        DISABLE_INTERRUPTS;
        if (FIFO_IsEmpty(rxFIFO)) {
            _T3IF = 0;
            ENABLE_INTERRUPTS;
            return;
        }
        FIFO_Read(rxFIFO, rxByte);
        ENABLE_INTERRUPTS;

        if (isEscaping == 1) {
            isEscaping = 0;
            switch (rxByte) {
                case ESCAPE_CHAR:
                    //The data stream contains the escape character...Let it go...
                    break;
                case START_OF_MESSAGE:
                    DisposeMessage(&Msg);
                    HeaderIdx = 0;
                    state = RecievingMsgHeader;
                    continue;
                    break;
                case END_OF_MESSAGE:
                    if (state == RecievingMsgBody) {
                        if (rxByteCount == Msg.DataLength) {
                            ESP_ProcessMessage(&Msg);
                            continue;
                        }
                    }
                    //Invalid Message...Start Over
                    DisposeMessage(&Msg);
                    state = WaitForMsgStart;
                    continue;
                    break;
                default:
                    DisposeMessage(&Msg);
                    state = WaitForMsgStart;
                    continue;
                    break;
            }
        } else if (rxByte == ESCAPE_CHAR) {
            isEscaping = 1;
            continue;
        }

        switch (state) {
            case WaitForMsgStart:
                break;
            case RecievingMsgHeader:
                switch (HeaderIdx) {
                    case 0:
                        Msg.MessageType = rxByte;
                        break;
                    case 1:
                        Msg.TCP_ChannelID = rxByte;
                        break;
                    case 2:
                        ESP_RX_FIFO_Processor_token.ub[0] = rxByte;
                        break;
                    case 3:
                        ESP_RX_FIFO_Processor_token.ub[1] = rxByte;
                        Msg.ResponseCode = ESP_RX_FIFO_Processor_token.i;
                        break;
                    case 4:
                        ESP_RX_FIFO_Processor_token.ub[1] = rxByte;
                        break;
                    case 5:
                        ESP_RX_FIFO_Processor_token.ub[0] = rxByte;
                        Msg.DataLength = ESP_RX_FIFO_Processor_token.i;
                        break;
                    case 6:
                        Msg.Detail.ub[0] = rxByte;
                        break;
                    case 7:
                        Msg.Detail.ub[1] = rxByte;
                        break;
                    case 8:
                        Msg.Detail.ub[2] = rxByte;
                        break;
                    case 9:
                        Msg.Detail.ub[3] = rxByte;
                        if (Msg.DataLength > ESP8266_SLIP_MESSAGE_MAX_LEN) {
                            DisposeMessage(&Msg);
                            state = WaitForMsgStart;
                        } else {
                            state = RecievingMsgBody;
                            MsgDataCursor = Msg.Data;
                            rxByteCount = 0;
                        }
                        break;
                }
                HeaderIdx++;
                break;
            case RecievingMsgBody:
                if (rxByteCount < Msg.DataLength) {
                    *(MsgDataCursor++) = rxByte;
                    rxByteCount++;
                } else {
                    DisposeMessage(&Msg);
                    state = WaitForMsgStart;
                }
                break;
            default:
                DisposeMessage(&Msg);
                state = WaitForMsgStart;

                break;
        }
    }
}

void LogEspCon(unsigned char *data, int len) {

    union {
        signed long l;
        unsigned char ub[4];
    } temp;

    temp.ub[0] = data[0];
    temp.ub[1] = data[1];
    temp.ub[2] = data[2];
    temp.ub[3] = data[3];
    Log("Remote: %ub.%ub.%ub.%ub:%l ", data[12], data[13], data[14], data[15], temp.l);

    temp.ub[0] = data[4];
    temp.ub[1] = data[5];
    temp.ub[2] = data[6];
    temp.ub[3] = data[7];
    Log("Local:  %ub.%ub.%ub.%ub:%i\r\n", data[8], data[9], data[10], data[11], temp.l);

    //    Log("-raw:");
    //    int x;
    //    for (x = 0; x < len; x++) {
    //        Log("%ub ", data[x]);
    //    }
    //    Log("\r\n");
}

static void ESP_ProcessMessage(ESP8266_SLIP_MESSAGE *msg) {
    BYTE msgID = msg->MessageType;
    BYTE ChannelID = msg->TCP_ChannelID;
    int ResponseCode = msg->ResponseCode;
    TCP_CHANNEL *TCP = NULL;
    if (ChannelID < TCP_CHANNEL_COUNT) {
        TCP = &TCP_Channels[ChannelID];
    }

    switch (msgID) {
        case ESP_START_MDNS_RESP:
            if (ResponseCode == Init_OK) {
                StartMDNS_State = 1;
            } else {
                StartMDNS_State = -1;
                //Log("mDNS Start Failed: %s\r\n", translateESP_RESP_CODE(ResponseCode));
                FailReason = ResponseCode;
            }
            break;
        case ESP_START_UPNP_RESP:
            if (ResponseCode == Init_OK) {
                StartUPNP_State = 1;
            } else {
                //Log("uPnP Start Failed: %s\r\n", translateESP_RESP_CODE(ResponseCode));
                StartUPNP_State = -1;
                FailReason = ResponseCode;
            }
            break;
        case ESP_IP_INFO:
            //Log("ESP IP Address Changed: %d.%d.%d.%d\r\n", msg->Detail.ub[3], msg->Detail.ub[2], msg->Detail.ub[1], msg->Detail.ub[0]);
            IP_Address.b[3] = msg->Detail.ub[3];
            IP_Address.b[2] = msg->Detail.ub[2];
            IP_Address.b[1] = msg->Detail.ub[1];
            IP_Address.b[0] = msg->Detail.ub[0];
            break;
        case ESP_TCP_SEND_RESULT:
            if (TCP != NULL) {
                TCP->SendStateDetail = ResponseCode;
                if (ResponseCode == SendOK) {
                    TCP->SendState = TCP_SEND_STATE_Success;
                } else if (ResponseCode == InProgress_Sending) {
                    TCP->SendState = TCP_SEND_STATE_Sending;
                } else {
                    Log("%b: TCP Send Failed: %s\r\n", ChannelID, translateESP_RESP_CODE(ResponseCode));
                    TCP->SendState = TCP_SEND_STATE_Fail;
                }
            }
            break;
        case ESP_TCP_RECIEVE:
            if (TCP != NULL) {
                WifiCommunicationsAreAlive = 1;
                ParseMessage_HTTP(msg);
            }
            break;
        case ESP_UPNP_SEND_RESULT:
            uPnP_Channel.SendStateDetail = ResponseCode;
            if (ResponseCode == SendOK) {
                uPnP_Channel.SendState = TCP_SEND_STATE_Success;
            } else {
                Log("uPnP Send Failed: %s\r\n", translateESP_RESP_CODE(ResponseCode));
                uPnP_Channel.SendState = TCP_SEND_STATE_Fail;
            }
            break;
        case ESP_UPNP_RECIEVE:
            uPnP_RecieveMsg(msg);
            WifiCommunicationsAreAlive = 1;
            break;
        case ESP_MDNS_SEND_RESULT:
            mDNS_Channel.SendStateDetail = ResponseCode;
            if (ResponseCode == SendOK) {
                mDNS_Channel.SendState = TCP_SEND_STATE_Success;
            } else {
                Log("mDNS Send Failed: %s\r\n", translateESP_RESP_CODE(ResponseCode));
                mDNS_Channel.SendState = TCP_SEND_STATE_Fail;
            }
            break;
        case ESP_MDNS_RECIEVE:
            mDNS_RecieveMsg(msg);
            WifiCommunicationsAreAlive = 1;
            break;
        case ESP_INIT_RESP:
            if (ResponseCode == Init_OK) {
                ResetState = RESET_SUCESS;
            } else {
                //Log("Reset Failed: %s\r\n", translateESP_RESP_CODE(ResponseCode));
                ResetState = RESET_FAIL;
                FailReason = ResponseCode;
            }
            break;
        case ESP_TCP_RECONNECT:
            if (ResponseCode == 0) {
                Log("?: RECONNECT: Arg is NULL: Err=%xb\r\n", msg->Detail.ub[0]);
            } else if (ResponseCode == 1) {
                Log("?: RECONNECT: Channel is NULL: Err=%xb ", msg->Detail.ub[0]);
                LogEspCon(msg->Data, msg->DataLength);
            } else if (ResponseCode == 2) {
                Log("%b: RECONNECT: Err=%b State=%b ", ChannelID, msg->Detail.ub[0], msg->Detail.ub[1]);
                LogEspCon(msg->Data, msg->DataLength);
            } else {
                Log("?: RECONNECT: Unknown ResponseCode=%i", ResponseCode);
            }
            break;
        case ESP_TCP_CONNECT:
            Log("%b: ESP_TCP_CONNECT: State=%xi ", ChannelID, ResponseCode);
            LogEspCon(msg->Data, msg->DataLength);
            WifiCommunicationsAreAlive = 1;
            break;
        case ESP_TCP_CLOSED:
            Log("%b: ESP_TCP_CLOSED: State=%xi ", ChannelID, ResponseCode);
            LogEspCon(msg->Data, msg->DataLength);
            break;
        case ESP_TCP_CONNECTFAIL:
            Log("?: ESP_TCP_CONNECTFAIL: ");
            LogEspCon(msg->Data, msg->DataLength);
            break;
        case ESP_TCP_RECV_ERROR:
            if (ResponseCode == 0) {
                Log("?: ESP_TCP_RECV_ERROR: Arg is NULL\r\n");
                msg->Data[msg->DataLength] = 0;
                Log("%s\r\n--END--\r\n", msg->Data);
                Log("\r\n");
            } else if (ResponseCode == 1) {
                Log("?: ESP_TCP_RECV_ERROR: Channel is NULL\r\n");
                msg->Data[msg->DataLength] = 0;
                Log("%s\r\n--END--\r\n", msg->Data);
                Log("\r\n");
            } else if (ResponseCode == 2) {
                Log("%b: ESP_TCP_RECV_ERROR: LinkEn=FASLE\r\n", ChannelID);
                msg->Data[msg->DataLength] = 0;
                Log("%s\r\n--END--\r\n", msg->Data);
                Log("\r\n");
            }
            break;
        case ESP_TCP_CLOSE_ERROR:
            if (ResponseCode == 0) {
                Log("?: TCP_CLOSE_ERROR: Arg is NULL\r\n\r\n");
            } else if (ResponseCode == 1) {
                Log("?: TCP_CLOSE_ERROR: Channel is NULL | ");
                LogEspCon(msg->Data, msg->DataLength);
            } else {
                Log("?: TCP_CLOSE_ERROR: Unknown ResponseCode=%i\r\n", ResponseCode);
            }
            break;
        case ESP_TCP_SENDCOMPLETE_ERROR:
            if (ResponseCode == 0) {
                Log("?: ESP_TCP_SENDCOMPLETE_ERROR: Arg is NULL\r\n");
                Log("\r\n");
            } else if (ResponseCode == 1) {
                Log("?: ESP_TCP_SENDCOMPLETE_ERROR: Channel is NULL ");
                LogEspCon(msg->Data, msg->DataLength);
            } else if (ResponseCode == 2) {
                Log("%b: ESP_TCP_SENDCOMPLETE_ERROR: LinkEn=FALSE ", ChannelID);
                LogEspCon(msg->Data, msg->DataLength);
            } else if (ResponseCode == 3) {
                Log("%b: ESP_TCP_SENDCOMPLETE_ERROR: isSending=FALSE ", ChannelID);
                LogEspCon(msg->Data, msg->DataLength);
            } else {
                Log("?: ESP_TCP_SENDCOMPLETE_ERROR: Unknown ResponseCode=%i\r\n", ResponseCode);
            }
            break;

        case ESP_SEND_GEN_MESSAGE:
            if (ResponseCode == Invalid_SLIP_Packet) {
                Log("?: ESP! invalid SLIP Subtype=%xb\r\n", msg->Detail.ub[0]);
                InvalidSlipMsgReported = 1;
            } else if (ResponseCode == InProgress_Sending) {
                if (TCP != NULL) {
                    TCP->SendState = TCP_SEND_STATE_Sending;
                }
            }
            break;
        case 0xEE://boot time progress
            break;
        default:
            Log("?: UNKNOWN MSG: %xb\r\n", msgID);
            Log("\r\n");
            break;
    }
    DisposeMessage(msg);
}

static void DisposeMessage(ESP8266_SLIP_MESSAGE *dat) {
    if (dat != NULL) {

        dat->TCP_ChannelID = 0;
        dat->DataLength = 0;
        dat->MessageType = 0;
        dat->ResponseCode = 0;
    }
}

int ESP_Init() {
    Log("   ESP init Started...\r\n");
    return ESP_Reset(1);
}

static int ESP_Reset(float PowerOffTime) {
    _U1RXIE = 0;
    unsigned char abaudData[2];
    int temp;
    int tock;

    if (GET_WIFI_POWER) {
        Log("   Power is already ON... so turn it off and wait...\r\n");
        SET_WIFI_POWER(0);
        Delay(PowerOffTime);
    }

    Log("   Turning Power ON...");
    SET_WIFI_POWER(1);

    Log("OK\r\n   AutoBaud In Progress...");

    int retry = 4; //Each try is 250ms * 4 = 1 second total
    while (retry--) {
        U1MODEbits.ABAUD = 1;
        while (U1MODEbits.ABAUD);
        while (U1STAbits.URXDA) abaudData[0] = U1RXREG;
        abaudData[0] = 0;
        for (temp = 0; temp < 2500; temp++) { //Total Time is 250ms
            if (U1STAbits.OERR == 1) U1STAbits.OERR = 0; //Clear any overflow condition
            while (U1STAbits.URXDA) { //While there are characters in the rxFIFO
                abaudData[1] = abaudData[0];
                abaudData[0] = U1RXREG;
                if (abaudData[0] == 'U' && abaudData[1] == 'U') goto autoBaudOK;
            }
            DELAY_105uS;
        }
    }
    Log("FAILED\r\n");
    return 0;

autoBaudOK:
    _U1RXIE = 1;
    Log("OK: BRG=%ui\r\n     Connecting. SSID=\"%s\".", U1BRG, ESP_Config.SSID);
    ResetState = RESET_IN_PROGRESS;
    //FIFO_WriteData(txFIFO, 24, 0xAA, 0x66, 0x10, 0x02, 0x54, 0x45, 0x53, 0x54, 0x53, 0x53, 0x49, 0x44, 0x00, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64, 0x00, 0xAA, 0x55);
    if (ESP_Config.Mode == SOFTAP_MODE) {
        FIFO_WriteData(txFIFO, 4, ESCAPE_CHAR, START_OF_MESSAGE, MCU_INIT, SOFTAP_MODE);
        circularPrintf(txFIFO, "%s", ESP_Config.SSID);
        FIFO_Write(txFIFO, 0);
        circularPrintf(txFIFO, "%s", ESP_Config.Password);
        FIFO_Write(txFIFO, 0);
        FIFO_WriteData(txFIFO, 4, ESP_Config.Channel, ESP_Config.EncryptionMode, ESCAPE_CHAR, END_OF_MESSAGE);
    } else {
        FIFO_WriteData(txFIFO, 4, ESCAPE_CHAR, START_OF_MESSAGE, MCU_INIT, STATION_MODE);
        circularPrintf(txFIFO, "%s", ESP_Config.SSID);
        FIFO_Write(txFIFO, 0);
        circularPrintf(txFIFO, "%s", ESP_Config.Password);
        FIFO_Write(txFIFO, 0);
        circularPrintf(txFIFO, "%s", ESP_Config.Name);
        FIFO_Write(txFIFO, 0);
        FIFO_WriteData(txFIFO, 2, ESCAPE_CHAR, END_OF_MESSAGE);
    }
    _U1TXIF = 1;

    //Prepare to wait 30 seconds...
    unsigned long tmr, timeout;
    GetTime(tmr);
    timeout = tmr + (SYSTEM_TIMER_FREQ * 60);
    while (1) {
        tock++;
        if (tock == 4700) {
            FIFO_Write(logFIFO, '.');
            _U2TXIF = 1;
            tock = 0;
        }

        DISABLE_INTERRUPTS;
        temp = ResetState;
        tmr = Timer500Hz;
        ENABLE_INTERRUPTS;
        if (temp == RESET_SUCESS) {
            goto ResetGood;
        } else if (temp != RESET_IN_PROGRESS) {
            Log("Reset Failed: %s\r\n", translateESP_RESP_CODE(FailReason));
            return 0;
        } else if (tmr > timeout) {
            Log("Reset Timeout!\r\n");
            return 0;
        }

        DELAY_105uS;
    }

ResetGood:

    IP_Address.l = 0;
    FIFO_WriteData(txFIFO, 5, ESCAPE_CHAR, START_OF_MESSAGE, MCU_GET_IP, ESCAPE_CHAR, END_OF_MESSAGE);
    _U1TXIF = 1;
    GetTime(tmr);
    timeout = tmr + (SYSTEM_TIMER_FREQ * 30);
    tock = 0;
    Log("Connected\r\n   Get IP Address:.");
    int badCount = 0;
    IP_Address_type tempIP;
    while (1) {
        tock++;
        if (tock == 4700) {
            FIFO_Write(logFIFO, '.');
            _U2TXIF = 1;
            tock = 0;
        }

        DISABLE_INTERRUPTS;
        tmr = Timer500Hz;
        tempIP.l = IP_Address.l;
        ENABLE_INTERRUPTS;
        if (tmr > timeout) {
            Log("Failed: Timeout\r\n");
            return 0;
        }
        if (tempIP.l != 0) {
            Log("%d.%d.%d.%d\r\n", IP_Address.b[3], IP_Address.b[2], IP_Address.b[1], IP_Address.b[0]);
            if (ESP_Config.Mode == STATION_MODE) {
                if (tempIP.b[3] == 192 && tempIP.b[2] == 168 && tempIP.b[1] == 4 && tempIP.b[0] == 1) {
                    badCount++;
                    if (badCount > 10) {
                        Log("Failed: Unable to Find DHCP Address\r\n");
                        return 0;
                    }
                    Delay(0.500);
                    IP_Address.l = 0;
                    FIFO_WriteData(txFIFO, 5, ESCAPE_CHAR, START_OF_MESSAGE, MCU_GET_IP, ESCAPE_CHAR, END_OF_MESSAGE);
                    _U1TXIF = 1;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        DELAY_105uS;
    }

    Log(" OK = %d.%d.%d.%d\r\n    Starting mDNS Server.", IP_Address.b[3], IP_Address.b[2], IP_Address.b[1], IP_Address.b[0]);
    StartMDNS_State = 0;
    //FIFO_WriteData(txFIFO, 5, ESCAPE_CHAR, START_OF_MESSAGE, MCU_START_MDNS, ESCAPE_CHAR, END_OF_MESSAGE);
    FIFO_WriteData(txFIFO, 3, ESCAPE_CHAR, START_OF_MESSAGE, MCU_START_MDNS);
    circularPrintf(txFIFO, "%s", ESP_Config.Name);
    FIFO_Write(txFIFO, 0);
    FIFO_WriteData(txFIFO, 2, ESCAPE_CHAR, END_OF_MESSAGE);
    _U1TXIF = 1;
    
    GetTime(tmr);
    timeout = tmr + (SYSTEM_TIMER_FREQ);
    tock = 0;
    while (1) {
        tock++;

        if (tock == 470) {
            FIFO_Write(logFIFO, '.');
            _U2TXIF = 1;
            tock = 0;
        }

        DISABLE_INTERRUPTS;
        temp = StartMDNS_State;
        tmr = Timer500Hz;
        ENABLE_INTERRUPTS;

        if (tmr > timeout) {
            Log("Failed: Timeout\r\n");
            return 0;
        } else if (temp == 0) {
            continue;
        } else if (temp == 1) {
            break;
        } else if (temp == -1) {
            Log("mDNS Failed: %s\r\n", translateESP_RESP_CODE(FailReason));
            return 0;
        }
    }
    Log(" OK\r\n    Starting uPnP Server.");


    StartUPNP_State = 0;
    FIFO_WriteData(txFIFO, 5, ESCAPE_CHAR, START_OF_MESSAGE, MCU_START_UPNP, ESCAPE_CHAR, END_OF_MESSAGE);
    _U1TXIF = 1;
    GetTime(tmr);
    timeout = tmr + (SYSTEM_TIMER_FREQ);
    tock = 0;
    while (1) {
        tock++;

        if (tock == 470) {
            FIFO_Write(logFIFO, '.');
            _U2TXIF = 1;
            tock = 0;
        }

        DISABLE_INTERRUPTS;
        temp = StartUPNP_State;
        tmr = Timer500Hz;
        ENABLE_INTERRUPTS;

        if (tmr > timeout) {
            Log("Failed: Timeout\r\n");
            return 0;
        } else if (temp == 0) {
            continue;
        } else if (temp == 1) {
            break;
        } else if (temp == -1) {
            Log("uPnP Failed: %s\r\n", translateESP_RESP_CODE(FailReason));
            return 0;
        }
    }

    Log("OK\r\nReset Success!\r\n");
    return 1;

}

int ESP_TCP_StartStream(BYTE ChannelID) {
    if (ChannelID > 9) return -1;
    FIFO_WriteData(txFIFO, 4, ESCAPE_CHAR, START_OF_MESSAGE, MCU_TCP_ASYNCSEND, ChannelID);

    return (1);
}

int ESP_TCP_TriggerWiFi_Send(BYTE ChannelID) {
    TCP_SEND_STATE state;
    unsigned long tmr, timeout;
    GetTime(tmr);
    timeout = tmr + SYSTEM_TIMER_FREQ * 0.1;

    if (ChannelID > 9) {
        Log("Bad ChannelID=%xb\r\n", ChannelID);
        return -1;
    }
    TCP_CHANNEL * con = &TCP_Channels[ChannelID];

    DISABLE_INTERRUPTS;
    if (con->SendState != TCP_SEND_STATE_Idle) {
        Log("SendState!=Idle\r\n");
        ENABLE_INTERRUPTS;
        return -1;
    }
    con->SendState = TCP_SEND_STATE_Submitted;
    InvalidSlipMsgReported = 0;
    ENABLE_INTERRUPTS;
    FIFO_WriteData(txFIFO, 2, ESCAPE_CHAR, END_OF_MESSAGE);
    _U1TXIF = 1;

    while (1) {
        DISABLE_INTERRUPTS;
        tmr = Timer500Hz;
        state = con->SendState;
        ENABLE_INTERRUPTS;

        if (InvalidSlipMsgReported) {
            Log("Invalid SLIP Message Reported!\r\n");
            con->SendState = TCP_SEND_STATE_Idle;
            return -1;
        }

        if (tmr > timeout) {
            Log("Wait for acceptance Timeout\r\n");
            con->SendState = TCP_SEND_STATE_Idle;
            return -1;
        }

        switch (state) {
            case TCP_SEND_STATE_Success:
            case TCP_SEND_STATE_Submitted:
                DELAY_105uS;
                continue;
                break;
            case TCP_SEND_STATE_Sending:
                return 1;
                break;
            default:
                Log("Error! Unknown State: %xi\r\n", state);
                DISABLE_INTERRUPTS;
                con->SendState = TCP_SEND_STATE_Idle;
                ENABLE_INTERRUPTS;
                return -1;
                break;
        }
    }
}

int ESP_TCP_CancelSend(BYTE ChannelID) {
    FIFO_WriteData(txFIFO, 2, ESCAPE_CHAR, START_OF_MESSAGE, ESCAPE_CHAR, END_OF_MESSAGE);
    return 1;
}

int ESP_TCP_Wait_WiFi_SendCompleted(BYTE ChannelID) {
    unsigned long tmr, timeout;
    GetTime(tmr);
    timeout = tmr + SYSTEM_TIMER_FREQ * 5;
    if (ChannelID > 9) return -1;
    TCP_SEND_STATE state;
    TCP_CHANNEL * con = &TCP_Channels[ChannelID];

    while (1) {
        DISABLE_INTERRUPTS;
        tmr = Timer500Hz;
        state = con->SendState;
        ENABLE_INTERRUPTS;

        if (tmr > timeout) {
            Log("Wait Timeout\r\n");
            con->SendState = TCP_SEND_STATE_Idle;
            return 0;
        }

        switch (state) {
            case TCP_SEND_STATE_Sending:
                DELAY_105uS;
                continue;
                break;
            case TCP_SEND_STATE_Success:
                DISABLE_INTERRUPTS;
                con->SendState = TCP_SEND_STATE_Idle;
                ENABLE_INTERRUPTS;
                return 1;
                break;
            default:
                Log("ERROR: Unexpected State=%b\r\n", state);
                DISABLE_INTERRUPTS;
                con->SendState = TCP_SEND_STATE_Idle;
                ENABLE_INTERRUPTS;
                return 0;
                break;
        }
    }
}

int ESP_TCP_CloseConnection(BYTE ChannelID) {
    if (ChannelID > 9) return -1;
    TCP_CHANNEL * con = &TCP_Channels[ChannelID];
    con->SendState = TCP_SEND_STATE_Idle;
    FIFO_WriteData(txFIFO, 6, ESCAPE_CHAR, START_OF_MESSAGE, MCU_TCP_CLOSE_CONNECTION, ChannelID, ESCAPE_CHAR, END_OF_MESSAGE);

    return 1;
}

int ESP_mDNS_BeginSend() {
    TCP_SEND_STATE state;

    DISABLE_INTERRUPTS;
    state = mDNS_Channel.SendState;
    ENABLE_INTERRUPTS;

    if (state != TCP_SEND_STATE_Idle) return 0;

    mDNS_Channel.SendState = TCP_SEND_STATE_Sending;
    FIFO_WriteData(txFIFO, 3, ESCAPE_CHAR, START_OF_MESSAGE, MCU_MDNS_ASYNCSEND);



    return (1);
}

int ESP_mDNS_CompleteSend() {
    unsigned long tmr, timeout;
    GetTime(tmr);
    timeout = tmr + (SYSTEM_TIMER_FREQ * 20);

    FIFO_WriteData(txFIFO, 2, ESCAPE_CHAR, END_OF_MESSAGE);
    _U1TXIF = 1;

    TCP_SEND_STATE state;
    while (1) {
        DISABLE_INTERRUPTS;
        tmr = Timer500Hz;
        state = mDNS_Channel.SendState;
        ENABLE_INTERRUPTS;

        if (tmr > timeout) {
            mDNS_Channel.SendState = TCP_SEND_STATE_Idle;
            return 0;
        }

        switch (state) {
            case TCP_SEND_STATE_Sending:
                DELAY_105uS;
                continue;
                break;
            case TCP_SEND_STATE_Success:
                mDNS_Channel.SendState = TCP_SEND_STATE_Idle;
                return 1;
                break;
            default:
                mDNS_Channel.SendState = TCP_SEND_STATE_Idle;

                return 0;
                break;
        }
    }
}

int ESP_uPnP_BeginSend() {
    TCP_SEND_STATE state;

    DISABLE_INTERRUPTS;
    state = uPnP_Channel.SendState;
    ENABLE_INTERRUPTS;

    if (state != TCP_SEND_STATE_Idle) return 0;

    uPnP_Channel.SendState = TCP_SEND_STATE_Sending;
    FIFO_WriteData(txFIFO, 3, ESCAPE_CHAR, START_OF_MESSAGE, MCU_UPNP_ASYNCSEND);

    return (1);
}

int ESP_uPnP_CompleteSend() {
    unsigned long tmr, timeout;
    GetTime(tmr);
    timeout = tmr + (SYSTEM_TIMER_FREQ * 20);

    FIFO_WriteData(txFIFO, 2, ESCAPE_CHAR, END_OF_MESSAGE);
    _U1TXIF = 1;

    TCP_SEND_STATE state;
    while (1) {
        DISABLE_INTERRUPTS;
        tmr = Timer500Hz;
        state = uPnP_Channel.SendState;
        ENABLE_INTERRUPTS;

        if (tmr > timeout) {
            uPnP_Channel.SendState = TCP_SEND_STATE_Idle;
            return 0;
        }

        switch (state) {
            case TCP_SEND_STATE_Sending:
                DELAY_105uS;
                continue;
                break;
            case TCP_SEND_STATE_Success:
                uPnP_Channel.SendState = TCP_SEND_STATE_Idle;
                return 1;
                break;
            default:
                uPnP_Channel.SendState = TCP_SEND_STATE_Idle;

                return 0;
                break;
        }
    }
}

void ESP_StreamArray(BYTE *data, int len) {

    while (1) {
        DISABLE_INTERRUPTS;
        if (FIFO_FreeSpace(txFIFO) > 512) {
            ENABLE_INTERRUPTS;
            break;
        }
        ENABLE_INTERRUPTS;
        DELAY_5uS;
    }

    while (len--) {
        if (*data == ESCAPE_CHAR) {
            FIFO_Write(txFIFO, ESCAPE_CHAR);
            FIFO_Write(txFIFO, ESCAPE_CHAR);
        } else {
            FIFO_Write(txFIFO, *data);
        }
        if (U1STAbits.TRMT) _U1TXIF = 1;
        data++;
    }
}

static const char *translateESP_RESP_CODE(int code) {
    switch (code) {
        case Fail_UnableToGetSoftAP_Config:
            return "Fail_UnableToGetSoftAP_Config";
            break;
        case Fail_UnableToSetSoftAP_Config:
            return "Fail_UnableToSetSoftAP_Config";
            break;
        case Fail_UnableToGetValidIP:
            return "Fail_UnableToGetValidIP";
            break;
        case FailOnStationConnect:
            return "FailOnStationConnect";
            break;
        case Fail_UnableToSetWifiConfig:
            return "Fail_UnableToSetWifiConfig";
            break;
        case Fail_UnableToSetWifiOpMode:
            return "Fail_UnableToSetWifiOpMode";
            break;
        case Fail_UnableToConnect:
            return "Fail_UnableToConnect";
            break;
        case Fail_UnableToAllocateTCPServer:
            return "Fail_UnableToAllocateTCPServer";
            break;
        case Fail_UnableToAllocatemDNSServer:
            return "Fail_UnableToAllocatemDNSServer";
            break;
        case Fail_UnableToAllocateuPnPServer:
            return "Fail_UnableToAllocateuPnPServer";
            break;
        case Fail_UnableToRegisterTCP_ConnectCB:
            return "Fail_UnableToRegisterTCP_ConnectCB";
            break;
        case Fail_UnableToSetTCPTimeout:
            return "Fail_UnableToSetTCPTimeout";
            break;
        case Fail_UnableToRegister_mDNS_RecieveCB:
            return "Fail_UnableToRegister_mDNS_RecieveCB";
            break;
        case Fail_UnableToStartTCPListener:
            return "Fail_UnableToStartTCPListener";
            break;
        case Fail_UnableToRegister_mDNS_SentCB:
            return "Fail_UnableToRegister_mDNS_SentCB";
            break;
        case Fail_UnableCreate_mDNS_Connection:
            return "Fail_UnableCreate_mDNS_Connection";
            break;
        case Fail_UnableJoin_mDNS_IGMP:
            return "Fail_UnableJoin_mDNS_IGMP";
            break;
        case Fail_UnableToRegister_uPnP_SentCB:
            return "Fail_UnableToRegister_uPnP_SentCB";
            break;
        case Fail_UnableCreate_uPnP_Connection:
            return "Fail_UnableCreate_uPnP_Connection";
            break;
        case Fail_UnableJoin_uPnP_IGMP:
            return "Fail_UnableJoin_uPnP_IGMP";
            break;
        case Fail_UnableToRegister_uPnP_RecieveCB:
            return "Fail_UnableToRegister_uPnP_RecieveCB";
            break;
        case Init_OK:
            return "Init_OK";
            break;
        case Fail_AlreadyInitialized:
            return "Fail_AlreadyInitialized";
            break;
        case Fail_SendInProgress:
            return "Fail_SendInProgress";
            break;
        case Fail_LinkNotEnabled:
            return "Fail_LinkNotEnabled";
            break;
        case Fail_EspError:
            return "Fail_EspError";
            break;
        case Accepted:
            return "Accepted";
            break;
        case Fail_Channel_pCon_IsNull:
            return "Fail_Channel_pCon_IsNull";
            break;
        case SendFail_Timeout:
            return "SendFail_Timeout";
            break;
        case SendFail_Closed:
            return "SendFail_Closed";
            break;
        case SendFail_Reconnect:
            return "SendFail_Reconnect";
            break;
        case SendOK:
            return "SendOK";
            break;
    }
    return "UNKNOWN";
}
