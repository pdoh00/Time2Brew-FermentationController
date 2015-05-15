/* 
 * File:   FlashFS.h
 * Author: aaron
 *
 * Created on April 2, 2015, 6:16 PM
 */

#ifndef FLASHFS_H
#define	FLASHFS_H

#ifdef	__cplusplus
extern "C" {
#endif

    //#define FF_TEST

#define FR_OK    1
#define FR_NOT_FORMATTED    -1
#define FR_NOT_FOUND    -2
#define FR_DISK_FULL    -3
#define FR_CORRUPTED    -4
#define FR_EOF  -5
#define FR_FILENAME_TOO_LONG  -6
#define FR_ERR_OTHER -7

    typedef struct {
        char FileName[123];
        unsigned int UID;
        unsigned long Position, Length, FileEntryAddress;
        unsigned long CurrentSector, SectorOffset;
        unsigned long OriginSector;
    } ff_File;

    typedef enum {
        ff_SeekMode_Absolute,
        ff_SeekMode_Relative
    } ff_SeekMode;

    int ff_Overwrite(ff_File *file, const char *filename);
    int ff_OpenByFileName(ff_File *file, const char *filename, char CreateIfNotFound);
    int ff_OpenByEntryAddress(ff_File *file, unsigned long FileTablePosition);
    int ff_Delete(const char *filename);
    int ff_DeleteByHandle(ff_File *file);
    int ff_Read(ff_File *file, unsigned char *out, int bCount, int *bytesRead);
    int ff_Append(ff_File *file, unsigned char *in, int bCount, int *bytesWritten);
    int ff_Seek(ff_File *file, signed long offset, ff_SeekMode mode);
    int ff_Trim();
    int ff_OpenDirectoryListing(ff_File *DirectoryIfo);
    int ff_GetNextEntryFromDirectory(ff_File *DirectoryIfo, char *fname);
    int ff_Read_StreamToWifi(ff_File *file, int bCount);
    int ff_UpdateLength(ff_File *file);
    int ff_Initialize();
    int ff_Format();
    int ff_GetUtilization(unsigned long *freeSpace, unsigned long *TrimSpace, unsigned long *UsedSpace);
    int ff_RepairFS();
    int ff_CheckFS();
    int ff_exists(const char *fname);

    int diskRead(unsigned long address, int bCount, unsigned char * out);
    int diskRawWrite(unsigned long address, int bCount, unsigned char *data);
    int diskWrite(unsigned long address, int bCount, unsigned char * in);
    int diskEraseSector(unsigned long address);
    int diskReadSecure(char sector, unsigned char * out);
    int diskWriteSecure(char sector, unsigned char * in);
    int diskEraseSecure(char sector);

    void ff_SPI_initialize();

    void fftest_LowLevel();
    void fftest_HighLevel();

#ifdef	__cplusplus
}
#endif

#endif	/* FLASHFS_H */

