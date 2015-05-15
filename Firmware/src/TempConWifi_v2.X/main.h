/* 
 * File:   main.h
 * Author: THORAXIUM
 *
 * Created on March 3, 2015, 8:04 PM
 */

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif


#include <p33Exxxx.h>
#include "ESP8266.h"
#include "FIFO.h"


#define ENABLE_INTERRUPTS {INTCON2bits.GIE=1;}
#define DISABLE_INTERRUPTS {INTCON2bits.GIE=0;__builtin_nop();__builtin_nop();}


#define ESP_TX_read(x) {\
    x=*(txFIFO->Read++);\
    if(txFIFO->Read==txFIFO->End) txFIFO->Read=txFIFO->Start;\
}
#define ESP_TX_IsEmpty (txFIFO->Read==txFIFO->Write)
#define ESP_TX_HasData (txFIFO->Read!=txFIFO->Write)


#define ESP_RX_write(x) {\
    *(rxFIFO->Write++)=x;\
    if(rxFIFO->Write==rxFIFO->End) rxFIFO->Write=rxFIFO->Start;\
}
#define ESP_RX_read(x) {\
    x=*(rxFIFO->Read++);\
    if(rxFIFO->Read==rxFIFO->End) rxFIFO->Read=rxFIFO->Start;\
}
#define ESP_RX_IsEmpty (rxFIFO->Read==rxFIFO->Write)
#define ESP_RX_HasData (rxFIFO->Read!=rxFIFO->Write)


#define LOG_write(x) {\
    while (logFIFO->NextWrite == logFIFO->Read){\
        logFIFO->NextWrite=logFIFO->Write+1;\
        if(logFIFO->NextWrite==logFIFO->End) logFIFO->NextWrite=logFIFO->Start;\
        _U2TXIF = 1;\
    }\
    *(logFIFO->Write) = x;\
    logFIFO->Write=logFIFO->NextWrite;\
    logFIFO->NextWrite++;\
    if(logFIFO->NextWrite==logFIFO->End) logFIFO->NextWrite=logFIFO->Start;\
    if (U2STAbits.TRMT) _U2TXIF = 1;\
}

#define LOG_read(x) {\
    x=*(logFIFO->Read);\
    *(logFIFO->Read++)=0;\
    if(logFIFO->Read==logFIFO->End) logFIFO->Read=logFIFO->Start;\
}
#define LOG_IsEmpty (logFIFO->Read==logFIFO->Write)
#define LOG_HasData (logFIFO->Read!=logFIFO->Write)

    const char *Translate_DRESULT(int res);

    void LOG_WriteBytes(int bCount, ...);
    void LOG_WriteArray(BYTE *dat, int bCount);

    extern FIFO_BUFFER *rxFIFO, *txFIFO, *logFIFO, *TRNG_fifo;
    extern const char *WiFiConfigFilename;
    extern const char *Version;

#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */

