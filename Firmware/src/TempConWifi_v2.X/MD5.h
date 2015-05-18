/* 
 * File:   MD5.h
 * Author: aaron
 *
 * Created on March 9, 2015, 11:33 AM
 */
 
#ifndef MD5_H
#define	MD5_H

#ifdef	__cplusplus
extern "C" {
#endif


    typedef unsigned long MD5_u32plus;

    typedef struct {
        MD5_u32plus lo, hi;
        MD5_u32plus a, b, c, d;
        unsigned char buffer[64];
        MD5_u32plus block[16];
    } MD5_CTX;

    void MD5_Init(MD5_CTX *ctx);
    void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
    void MD5_Final(char *result, MD5_CTX *ctx);


#ifdef	__cplusplus
}
#endif

#endif	/* MD5_H */

