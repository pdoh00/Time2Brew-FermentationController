/* 
 * File:   circularPrintF.h
 * Author: aaron
 *
 * Created on March 3, 2015, 12:30 PM
 */

#ifndef CIRCULARPRINTF_H
#define	CIRCULARPRINTF_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "FIFO.h"
#include <stdarg.h>
    int circularPrintf(FIFO_BUFFER *fifo, const char *format, ...);
    int circular_vPrintf(FIFO_BUFFER *fifo, const char *format, va_list arguments);
#ifdef	__cplusplus
}
#endif

#endif	/* CIRCULARPRINTF_H */

