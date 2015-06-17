#include <stddef.h>
#include <string.h>
#include "FlashFS.h"
#include "BlobFS.h"
#include "ESP8266.h"
#include "SystemConfiguration.h"

void ToLower(char *src) {
    char token;
    while (*src) {
        token = *src;
        if (token >= 'A' && token <= 'Z') {
            token -= 'A';
            token += 'a';
        }
        *(src++) = token;
    }
}

int BLOB_openFile(BLOB_FILE *file, const char *filename) {
    ToLower((char *) filename);
    //Log("BLOB: OpenFile '%s'\r\n", filename);
    BLOB_FILE entry;
    unsigned long offset = BLOB_START_ADDRESS;
    while (1) {
        //Log("  Offset=%l ", offset);
        diskRead(offset, sizeof (BLOB_FILE), (BYTE *) & entry);
        ToLower(entry.filename);
        if ((BYTE) entry.filename[0] == 0xFF) {
            //Log(" 0xFF found... End of Blob\r\n");
            return FR_NOT_FOUND;
        } else if (strcmp(entry.filename, filename) == 0) {
            memcpy(file, &entry, 256); //Copy the filename
            file->blobOffset = offset + (256 + 4 + 2);
            file->length = entry.length;
            file->checksum = entry.checksum;
            //Log(" Match - blobOffset=%l fileLength=%l\r\n", file->blobOffset, file->length);
            return FR_OK;
        } else {
            //Log(" No Match '%s' Offset=%l Length=%l Checksum=%xi\r\n", entry.filename, entry.blobOffset, entry.length, entry.checksum);
            //Move past this blob file entry
            offset += (256 + 4 + 2 + entry.length);
        }
    }
}

int BLOB_read(BLOB_FILE *file, unsigned long fileOffset, unsigned long readLength, BYTE *out) {
    if (readLength == 0) readLength = file->length - fileOffset;
    diskRead(file->blobOffset + fileOffset, readLength, out);
    return FR_OK;
}

int BLOB_readStreamToWifi(BLOB_FILE *file, unsigned long fileOffset, unsigned long readLength) {
    BYTE buffer[256];
    unsigned long bytesToRead;
    if (readLength == 0) readLength = file->length - fileOffset;

    //Log("   BLOB: streamtoWifi readLength=%l\r\n", readLength);

    while (readLength) {
        bytesToRead = readLength;
        if (bytesToRead > 256) bytesToRead = 256;
        //Log("      blobOffset=%l fileOffset=%l reading %l bytes from Blob into buffer\r\n", file->blobOffset, fileOffset, bytesToRead);
        diskRead(file->blobOffset + fileOffset, bytesToRead, buffer);
        fileOffset += bytesToRead;
        readLength -= bytesToRead;
        //Log("         Streaming to ESP\r\n");
        ESP_StreamArray(buffer, bytesToRead);
    }
    //Log("   BLOB: done\r\n");
    return FR_OK;
}
