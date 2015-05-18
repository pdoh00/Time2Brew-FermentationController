/* 
 * File:   Pack.h
 * Author: THORAXIUM
 *
 * Created on March 14, 2015, 12:19 PM
 */

#ifndef PACK_H
#define	PACK_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdarg.h>

    unsigned char * Packv(unsigned char *buffer, const char *format, va_list arguments);
    unsigned char * Pack(unsigned char *buffer, const char *format, ...);
    unsigned char * Unpackv(unsigned char *buffer, const char *format, va_list arguments);
    unsigned char * Unpack(unsigned char *buffer, const char *format, ...);



#ifdef	__cplusplus
}
#endif

#endif	/* PACK_H */

