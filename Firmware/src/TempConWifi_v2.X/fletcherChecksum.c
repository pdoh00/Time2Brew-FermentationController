#include <stdint.h>
#include "fletcherChecksum.h"

unsigned int fletcher16(unsigned char const *data, unsigned int bytes) {
    unsigned int sum1 = 0xff, sum2 = 0xff;

    while (bytes) {
        unsigned int tlen = bytes > 20 ? 20 : bytes;
        bytes -= tlen;
        do {
            sum2 += sum1 += *data++;
        } while (--tlen);
        sum1 = (sum1 & 0xff) + (sum1 >> 8);
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
    }
    /* Second reduction step to reduce sums to 8 bits */
    sum1 = (sum1 & 0xff) + (sum1 >> 8);
    sum2 = (sum2 & 0xff) + (sum2 >> 8);
    return sum2 << 8 | sum1;
}
