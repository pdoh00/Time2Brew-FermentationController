/* 
 * File:   mDNS.h
 * Author: aaron
 *
 * Created on February 23, 2015, 3:01 PM
 */



#ifndef MDNS_H
#define	MDNS_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "integer.h"
#include "ESP8266.h"

    void mDNS_RecieveMsg(ESP8266_SLIP_MESSAGE *msg);
    void mDNS_ProcessLoop();
    void mDNS_Init(const char *Name, unsigned long IPAddress);

#ifdef	__cplusplus
}
#endif

#endif	/* MDNS_H */

