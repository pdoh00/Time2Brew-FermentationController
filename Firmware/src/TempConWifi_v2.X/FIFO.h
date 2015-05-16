/* 
 * File:   FIFO.h
 * Author: THORAXIUM
 *
 * Created on March 3, 2015, 8:10 PM
 */

#ifndef FIFO_H
#define	FIFO_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "integer.h"

#define FIFO_FreeSpace(F) (((F)->Write < (F)->Read) ? ((F)->Read - (F)->Write) : (((F)->End - (F)->Write)+((F)->Read - (F)->Start)))
#define FIFO_Write(F,x) {*((F)->Write++)=x; if ((F)->Write==(F)->End) (F)->Write=(F)->Start;}
#define FIFO_Read(F,x) {x=*((F)->Read++); if ((F)->Read==(F)->End) (F)->Read=(F)->Start;}
#define FIFO_HasChars(F) (((F)->Write != (F)->Read) ? 1 : 0)
#define FIFO_IsEmpty(F) (((F)->Write == (F)->Read) ? 1 : 0)

    typedef struct {
        BYTE *Start, *Read, *Write, *End;
    } FIFO_BUFFER;

    void FIFO_WriteData(FIFO_BUFFER *buff, int bCount, ...);
    void FIFO_WriteArray(FIFO_BUFFER *buff, int bCount, BYTE *dat);


#ifdef	__cplusplus
}
#endif

#endif	/* FIFO_H */

