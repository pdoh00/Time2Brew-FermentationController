/* 
 * File:   RLE_Compressor.h
 * Author: THORAXIUM
 *
 * Created on June 5, 2015, 3:35 PM
 */

#ifndef RLE_COMPRESSOR_H
#define	RLE_COMPRESSOR_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "integer.h"
#include "FlashFS.h"
#include <stddef.h>

    typedef struct {
        ff_File *file;
        void *lastSample;
        unsigned long packetCount;
        unsigned long SampleCount;
        int nvSRAM_AddressOffset;
        size_t sizeofSample;
    } RLE_State;

    int RLE_CreateNew(RLE_State *state, const char *filename, ff_File *fileHandle, void *SampleBuffer, size_t sizeofSamples, int nvram_address);
    int RLE_Open(RLE_State *state, const char *filename, ff_File *fileHandle, void *SampleBuffer, size_t sizeofSamples, unsigned long NextSampleIdx, int nvram_address);
    int RLE_addSample(RLE_State *state, void *Sample);
    int RLE_Flush(RLE_State *state);
    int RLE_close(RLE_State *state);
    void RLE_Test();

#ifdef	__cplusplus
}
#endif

#endif	/* RLE_COMPRESSOR_H */

