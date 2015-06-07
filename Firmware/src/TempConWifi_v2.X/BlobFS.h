/* 
 * File:   BlobFS.h
 * Author: THORAXIUM
 *
 * Created on June 6, 2015, 11:18 AM
 */

#ifndef BLOBFS_H
#define	BLOBFS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <string.h>
#include "integer.h"
#include "FlashFS.h"

    typedef struct {
        char filename[256];
        unsigned long length;
        unsigned int checksum;
        unsigned long blobOffset;
    } BLOB_FILE;

    int BLOB_openFile(BLOB_FILE *file, const char *filename);
    int BLOB_read(BLOB_FILE *file, unsigned long fileOffset, unsigned long readLength, BYTE *out);
    int BLOB_readStreamToWifi(BLOB_FILE *file, unsigned long fileOffset, unsigned long readLength);

#ifdef	__cplusplus
}
#endif

#endif	/* BLOBFS_H */

