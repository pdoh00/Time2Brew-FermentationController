#include <stdio.h>
#include <stdarg.h>
#include "integer.h"
#include "FIFO.h"

void FIFO_WriteData(FIFO_BUFFER *buff, int bCount, ...) {
    va_list args;
    va_start(args, bCount);
    BYTE token;
    while (bCount--) {
        token = va_arg(args, BYTE);
        FIFO_Write(buff, token);
    }
}

void FIFO_WriteArray(FIFO_BUFFER *buff, int bCount, BYTE *dat) {
    while (bCount--) {
        FIFO_Write(buff, *(dat++));
    }
}
