/* 
 * File:   uPnP.h
 * Author: aaron
 *
 * Created on March 4, 2015, 4:46 PM
 */

#ifndef UPNP_H
#define	UPNP_H

#ifdef	__cplusplus
extern "C" {
#endif

    

    void uPnP_RecieveMsg(ESP8266_SLIP_MESSAGE *msg);
    void uPnP_ProcessLoop();
    void uPnP_Init(const char *Name, const char *UUID, unsigned long IPAddress);

#ifdef	__cplusplus
}
#endif

#endif	/* UPNP_H */

