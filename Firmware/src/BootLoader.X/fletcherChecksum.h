/* 
 * File:   fletcherChecksum.h
 * Author: aaron
 *
 * Created on May 8, 2015, 3:54 PM
 */

#ifndef FLETCHERCHECKSUM_H
#define	FLETCHERCHECKSUM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

unsigned int fletcher16(unsigned char const *data, unsigned int bytes);


#ifdef	__cplusplus
}
#endif

#endif	/* FLETCHERCHECKSUM_H */

