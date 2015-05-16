/* 
 * File:   Base64.h
 * Author: THORAXIUM
 *
 * Created on January 4, 2015, 9:27 PM
 */
#include "FIFO.h"

int sprintf_Base64(char *output, unsigned char *sourcedata, int bCount);
int Base64_Length(int bCount);
int decode_Base64(const char *input, int inputLen, unsigned char *output);
void circular_ToBase64(FIFO_BUFFER *outFIFO, unsigned char *srcData, int srcCount);
