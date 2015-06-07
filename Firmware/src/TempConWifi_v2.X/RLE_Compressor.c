#include <stdio.h>

#include "main.h"
#include <string.h>
#include "FlashFS.h"
#include "RLE_Compressor.h"
#include "RTC.h"
#include "TemperatureController.h"
#include "SystemConfiguration.h"

int RLE_CreateNew(RLE_State *state, const char *filename, ff_File *fileHandle, void *SampleBuffer, size_t sizeofSamples, int nvram_address) {
    int res;
    BYTE RunLength = 0;
    state->file = fileHandle;
    state->lastSample = SampleBuffer;
    state->packetCount = 0;
    state->sizeofSample = sizeofSamples;
    state->SampleCount = 0;
    state->nvSRAM_AddressOffset = nvram_address;

    nvsRAM_Write(&RunLength, state->nvSRAM_AddressOffset, 1);

    ff_Delete(filename);
    res = ff_OpenByFileName(state->file, filename, 1);
    if (res != FR_OK) return res;

    return FR_OK;
}

int RLE_Open(RLE_State *state, const char *filename, ff_File *fileHandle, void *SampleBuffer, size_t sizeofSamples, unsigned long NextSampleIdx, int nvram_address) {
    int res, bw;

    BYTE zeroBuffer[128];

    state->lastSample = SampleBuffer;
    state->packetCount = 0;
    state->sizeofSample = sizeofSamples;
    state->SampleCount = 0;
    state->file = fileHandle;
    state->nvSRAM_AddressOffset = nvram_address;

    BYTE RunLength;

    res = ff_OpenByFileName(state->file, filename, 0);
    if (res != FR_OK) return RLE_CreateNew(state, filename, fileHandle, SampleBuffer, sizeofSamples, nvram_address);

    //Determine how many samples have been saved...
    while (1) {
        ff_Read(state->file, state->lastSample, state->sizeofSample, &bw); //Read the Sample Data in...
        ff_Read(state->file, &RunLength, 1, &bw); //Read the run length for this sample.

        if (RunLength == 0xFF) { //An invalid run length means this packet is the last one...
            ff_Seek(state->file, state->file->Position - 1); //Back up so the file is at the right place to write the sample count...
            //Restore the nvRAM RunLength
            nvsRAM_Read(&RunLength, state->nvSRAM_AddressOffset, 1);
            state->SampleCount += RunLength;
            Log("  Restored Runlength=%b SampleCount=%l\r\n", RunLength, state->SampleCount);

            //And complete the record...by writing a valid RunLength
            //ff_Append(state->file, &RunLength, 1, &bw);

            //Clear the LastSampleData to zero indicating this is a GAP FILLER
            memset(zeroBuffer, 0, state->sizeofSample);

            //Now we can add zero samples until the file is back to the sample count required (to fill the gap)
            while (state->SampleCount < NextSampleIdx) {
                res = RLE_addSample(state, zeroBuffer);
                if (res != FR_OK) return res;
            }
            return FR_OK;

            //And write the packet sample start to disk...
            //ff_Append(state->file, state->lastSample, state->sizeofSample, &bw);
            //RunLength = 1; //Reset the Run Length back to 1
            //nvsRAM_Write(&RunLength, state->nvSRAM_AddressOffset, 1);

        } else {
            state->SampleCount += RunLength;
            state->packetCount++;
        }
    }
}

int RLE_close(RLE_State *state) {
    BYTE RunLength;
    nvsRAM_Read(&RunLength, state->nvSRAM_AddressOffset, 1);
    int bw;
    //Write the Run Length to complete the previously started packet.
    unsigned long filePosition = (state->packetCount * (state->sizeofSample + 1)) + state->sizeofSample;
    if (state->file->Position != filePosition) {
        ff_Seek(state->file, filePosition);
    }
    ff_Append(state->file, &RunLength, 1, &bw);
    state->file->Length = state->file->Position;
    ff_UpdateLength(state->file);

    return FR_OK;
}

int RLE_Flush(RLE_State *state) {
    BYTE RunLength;
    nvsRAM_Read(&RunLength, state->nvSRAM_AddressOffset, 1);
    int bw;
    //Write the Run Length to complete the previously started packet.
    unsigned long filePosition = (state->packetCount * (state->sizeofSample + 1)) + state->sizeofSample;
    if (state->file->Position != filePosition) {
        ff_Seek(state->file, filePosition);
    }
    ff_Append(state->file, &RunLength, 1, &bw);
    state->packetCount++; //Increment the Packet

    //Reset the run-length
    RunLength = 0;
    nvsRAM_Write(&RunLength, state->nvSRAM_AddressOffset, 1);

    //Now write the start of the next packet...
    ff_Append(state->file, state->lastSample, state->sizeofSample, &bw);

    return FR_OK;

}

int RLE_addSample(RLE_State *state, void *Sample) {
    int bw;
    BYTE RunLength;
    nvsRAM_Read(&RunLength, state->nvSRAM_AddressOffset, 1);

    if (state->SampleCount == 0) {
        state->packetCount = 0;
        ff_Seek(state->file, 0);
        ff_Append(state->file, Sample, state->sizeofSample, &bw);
        memcpy(state->lastSample, Sample, state->sizeofSample);
        RunLength = 1;
        nvsRAM_Write(&RunLength, state->nvSRAM_AddressOffset, 1);
        state->SampleCount++;
        return FR_OK;
    }

    state->SampleCount++;
    //Is this sample the same as the last sample?
    if (state->lastSample == Sample || memcmp(state->lastSample, Sample, state->sizeofSample) == 0) {
        //This is the same so increase the RunLength
        RunLength++;
        nvsRAM_Write(&RunLength, state->nvSRAM_AddressOffset, 1);

        if (RunLength != 0xFF) return FR_OK;

        //Run length is too long so...flush it to disk and then add the sample.
        RunLength--;
        unsigned long filePosition = (state->packetCount * (state->sizeofSample + 1)) + state->sizeofSample;
        if (state->file->Position != filePosition) {
            ff_Seek(state->file, filePosition);
        }
        ff_Append(state->file, &RunLength, 1, &bw);
        state->packetCount++; //Increment the Packet

        //Add the start of the next packet...
        ff_Append(state->file, Sample, state->sizeofSample, &bw);
        RunLength = 1;
        nvsRAM_Write(&RunLength, state->nvSRAM_AddressOffset, 1);
    } else {
        //The sample has changed so flush this record and start the next...

        //Write the Run Length to complete the previously started packet.
        unsigned long filePosition = (state->packetCount * (state->sizeofSample + 1)) + state->sizeofSample;
        if (state->file->Position != filePosition) {
            ff_Seek(state->file, filePosition);
        }
        ff_Append(state->file, &RunLength, 1, &bw);
        state->packetCount++; //Increment the Packet

        //Write the Actual Sample data for the current packet...
        ff_Append(state->file, Sample, state->sizeofSample, &bw);
        memcpy(state->lastSample, Sample, state->sizeofSample);
        RunLength = 1; //Reset the Run Length back to 1
        nvsRAM_Write(&RunLength, state->nvSRAM_AddressOffset, 1);
    }
    return FR_OK;
}

//void RLE_Test() {
//    int res, x, bw;
//    ff_File backingFile;
//    BYTE lastSampleBuffer[6], testbuffer[6];
//    RLE_State dut;
//    BYTE fReadBuffer[1024];
//
//    res = RLE_CreateNew(&dut, "testRLE.dat", &backingFile, lastSampleBuffer, 6, sizeof (RECOVERY_RECORD) + 4);
//    if (res != FR_OK) {
//        Log("Error Creating DUT res='%s'", Translate_DRESULT(res));
//        while (1);
//    }
//
//    for (x = 0; x < 6; x++) testbuffer[x] = 0xAA;
//    for (x = 0; x < 10; x++) {
//        RLE_addSample(&dut, testbuffer);
//    }
//
//    for (x = 0; x < 6; x++) testbuffer[x] = 0xBB;
//    for (x = 0; x < 10; x++) {
//        RLE_addSample(&dut, testbuffer);
//    }
//
//    for (x = 0; x < 6; x++) testbuffer[x] = 0xCC;
//    for (x = 0; x < 10; x++) {
//        RLE_addSample(&dut, testbuffer);
//    }
//
//    for (x = 0; x < 6; x++) testbuffer[x] = 0xDD;
//    for (x = 0; x < 0xFE; x++) {
//        RLE_addSample(&dut, testbuffer);
//    }
//
//    for (x = 0; x < 6; x++) testbuffer[x] = 0xEE;
//    for (x = 0; x < 0xFF; x++) {
//        RLE_addSample(&dut, testbuffer);
//    }
//
//    for (x = 0; x < 6; x++) testbuffer[x] = 0x11;
//    for (x = 0; x < 0xFFF; x++) {
//        RLE_addSample(&dut, testbuffer);
//    }
//
//    for (x = 0; x < 6; x++) testbuffer[x] = 0x22;
//    RLE_addSample(&dut, testbuffer);
//    for (x = 0; x < 6; x++) testbuffer[x] = 0x23;
//    RLE_addSample(&dut, testbuffer);
//    for (x = 0; x < 6; x++) testbuffer[x] = 0x24;
//    RLE_addSample(&dut, testbuffer);
//    for (x = 0; x < 6; x++) testbuffer[x] = 0x25;
//    RLE_addSample(&dut, testbuffer);
//    RLE_addSample(&dut, testbuffer);
//    RLE_addSample(&dut, testbuffer);
//
//    Log("File Length=%l\r\n", backingFile.Length);
//    Log("Sample Count=%l\r\n", dut.SampleCount);
//    Log("Packet Count=%l\r\n", dut.packetCount);
//    ff_Seek(&backingFile, 0);
//    ff_Read(&backingFile, fReadBuffer, backingFile.Length, &bw);
//    for (x = 0; x < backingFile.Length; x++) {
//        if (x % 7 == 0) Log("\r\n");
//        Log("%xb ", fReadBuffer[x]);
//    }
//    Log("\r\nDone\r\n");
//
//    Log("Flushing...");
//    RLE_Flush(&dut);
//    Log("OK\r\n\r\n");
//
//    Log("File Length=%l\r\n", backingFile.Length);
//    Log("File Length=%l\r\n", backingFile.Length);
//    Log("Sample Count=%l\r\n", dut.SampleCount);
//    Log("Packet Count=%l\r\n", dut.packetCount);
//    ff_Seek(&backingFile, 0);
//    ff_Read(&backingFile, fReadBuffer, backingFile.Length, &bw);
//    for (x = 0; x < backingFile.Length; x++) {
//        if (x % 7 == 0) Log("\r\n");
//        Log("%xb ", fReadBuffer[x]);
//    }
//    Log("\r\nDone\r\n");
//
//    Log("Adding Samples then Re-Opening DUT\r\n");
//    for (x = 0; x < 6; x++) testbuffer[x] = 0x26;
//    RLE_addSample(&dut, testbuffer);
//    RLE_addSample(&dut, testbuffer);
//    RLE_addSample(&dut, testbuffer);
//
//    memset(&dut, 0, sizeof (dut));
//
//    res = RLE_Open(&dut, "testRLE.dat", &backingFile, lastSampleBuffer, 6, 5000, sizeof (RECOVERY_RECORD) + 4);
//
//    for (x = 0; x < 6; x++) testbuffer[x] = 0x40;
//    RLE_addSample(&dut, testbuffer);
//    RLE_addSample(&dut, testbuffer);
//    RLE_addSample(&dut, testbuffer);
//    RLE_addSample(&dut, testbuffer);
//    RLE_Flush(&dut);
//
//    Log("File Length=%l\r\n", backingFile.Length);
//    Log("File Length=%l\r\n", backingFile.Length);
//    Log("Sample Count=%l\r\n", dut.SampleCount);
//    Log("Packet Count=%l\r\n", dut.packetCount);
//    ff_Seek(&backingFile, 0);
//    ff_Read(&backingFile, fReadBuffer, backingFile.Length, &bw);
//    for (x = 0; x < backingFile.Length; x++) {
//        if (x % 7 == 0) Log("\r\n");
//        Log("%xb ", fReadBuffer[x]);
//    }
//    Log("\r\nDone\r\n");
//
//    while (1);
//
//}
