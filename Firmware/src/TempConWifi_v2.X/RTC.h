/* 
 * File:   RTC.h
 * Author: THORAXIUM
 *
 * Created on March 22, 2015, 11:51 PM
 */

#ifndef RTC_H
#define	RTC_H

#ifdef	__cplusplus
extern "C" {
#endif

    unsigned long RTC_GetTime();
    int RTC_SetTime(unsigned long time);
    int nvsRAM_Read(unsigned char *dst, unsigned char Address, unsigned char bCount);
    int nvsRAM_Write(unsigned char *buffer, unsigned char Address, unsigned char bCount);
    unsigned long SecondsFromEpoch(int y, int m, int d, int hour, int minute, int second);
    int RTC_Initialize();
    void i2c_start_cond(void);
    void i2c_stop_cond(void);
    int i2c_write_byte(int send_start, int send_stop, unsigned char byte);
    unsigned char i2c_read_byte(int nack, int send_stop);
    
#ifdef	__cplusplus
}
#endif

#endif	/* RTC_H */

