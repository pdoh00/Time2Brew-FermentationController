#include <p33Exxxx.h>
#include "RTC.h"
#include "SystemConfiguration.h"
#include "TemperatureController.h"

unsigned long SecondsFromEpoch(int y, int m, int d, int hour, int minute, int second) {
    if (y < 1970) return 0;
    if (m == 0 || d == 0) return 0;
    m = (m + 9) % 12;
    y = y - m / 10;
    long ret = y;
    long yr = y;
    long mnth = m;
    long hr = hour;
    long min = minute;
    long sec = second;
    ret = (365 * yr);
    ret += (yr / 4);
    ret -= (yr / 100);
    ret += (yr / 400);
    ret += ((mnth * 306) + 5) / 10;
    ret += (d - 1);
    ret -= 719468; //Reference EPOCH = 1970-01-01 00:00:00s
    ret *= 86400; //Convert from Days to Seconds
    ret += (hr * 3600);
    ret += (min * 60);
    ret += sec;
    return ret;
}

void i2c_start_cond(void) {
    //Is RTC_SDA_PORT Low?
    if (!RTC_SDA_PORT) {
        //Is RTC_SCL_PORT high?
        if (RTC_SCL_PORT) {
            RTC_SCL_ASSERT;
            RTC_I2C_TICK;
        }
        //Release RTC_SDA_PORT to High
        RTC_SDA_RELEASE;
        RTC_I2C_TICK;
    }

    //Is the Clock Low?
    if (!RTC_SCL_PORT) {
        RTC_SCL_RELEASE;
        RTC_I2C_TICK;
    }

    //With RTC_SCL_PORT High we Bring RTC_SDA_PORT Low
    RTC_SDA_ASSERT;
    RTC_I2C_TICK;
}

void i2c_stop_cond(void) {
    if (RTC_SDA_PORT) { //Is RTC_SDA_PORT high already?
        if (RTC_SCL_PORT) { //Bring RTC_SCL_PORT low if needed
            RTC_SCL_ASSERT;
            RTC_I2C_TICK;
        }
        //Now with RTC_SCL_PORT Low bring RTC_SDA_PORT Low as well
        RTC_SDA_ASSERT;
        RTC_I2C_TICK;
    }

    if (!RTC_SCL_PORT) {
        RTC_SCL_RELEASE;
        RTC_I2C_TICK;
    }

    //RTC_SCL_PORT is High and RTC_SDA_PORT is Low
    RTC_SDA_RELEASE;
    RTC_I2C_TICK;
    //Now both RTC_SDA_PORT and RTC_SCL_PORT are high and the bus is Idle
}

void i2c_write_bit(int dat) {
    RTC_SCL_ASSERT;
    RTC_I2C_TICK;
    if (dat) {
        RTC_SDA_RELEASE;
    } else {
        RTC_SDA_ASSERT;
    }
    RTC_I2C_TICK;
    RTC_SCL_RELEASE;
    RTC_I2C_TOCK;
}

int i2c_read_bit(void) {
    RTC_SCL_ASSERT;
    RTC_I2C_TICK;
    RTC_SDA_RELEASE;
    RTC_I2C_TICK;

    RTC_SCL_RELEASE;
    RTC_I2C_TICK;
    int dat = RTC_SDA_PORT;
    RTC_I2C_TICK;
    return dat;
}

int i2c_write_byte(int send_start, int send_stop, unsigned char byte) {
    int nack;
    if (send_start) {
        i2c_start_cond();
    }
    int x;
    for (x = 0; x < 8; x++) {
        i2c_write_bit((byte & 0x80) != 0);
        byte <<= 1;
    }
    nack = i2c_read_bit();
    if (send_stop) {
        i2c_stop_cond();
    }
    return nack;
}

unsigned char i2c_read_byte(int nack, int send_stop) {
    unsigned char dat = 0;
    int x;
    for (x = 0; x < 8; x++) {
        dat = (dat << 1) | i2c_read_bit();
    }
    i2c_write_bit(nack);
    if (send_stop) {
        i2c_stop_cond();
    }
    return dat;
}

int RTC_Initialize() {
    Delay(0.2);

    i2c_start_cond();
    i2c_write_byte(0, 0, 0b11011110);
    i2c_write_byte(0, 0, 0x00);

    i2c_start_cond(); //Restart condition
    i2c_write_byte(0, 0, 0b11011111);
    unsigned char RTCSEC = i2c_read_byte(0, 0);
    Log("RTCSEC=%xb\r\n", RTCSEC);
    i2c_stop_cond();

    //Set the VBAT enable bit, Weekday =1, and PWRFAIL is reset...
    i2c_start_cond();
    i2c_write_byte(0, 0, 0b11011110);
    i2c_write_byte(0, 0, 0x03);
    i2c_write_byte(0, 0, 0b00001001);
    i2c_start_cond();


    if (!(RTCSEC & 0b10000000)) {
        Log("ST Not Started...Setting the Status Bit and writing...\r\n");
        RTCSEC |= 0b10000000;
        i2c_start_cond();
        if (i2c_write_byte(0, 0, 0b11011110)) { //Address the RTC for write so that we can set the address
            Log("NACK Addressing the RTC for Write\r\n");
            return 0;
        }
        if (i2c_write_byte(0, 0, 0x00)) { //RTCSEC register 0x00
            Log("NACK Addressing Register 0x00\r\n");
            return 0;
        }

        if (i2c_write_byte(0, 0, RTCSEC)) { //RTCSEC register 0x00
            Log("NACK On Write of Register 0x00\r\n");
            return 0;
        }

        i2c_stop_cond();
        Log("ST Should be Runnin\r\n");
    }

    globalstate.SystemTime = RTC_GetTime();

    Log("RTC Initialize Finished\r\n");
    return 1;
}

unsigned long RTC_GetTime() {
    //Set the Read Address to 0x00
    i2c_start_cond();
    i2c_write_byte(0, 0, 0b11011110);
    i2c_write_byte(0, 0, 0x0);

    //Read the Data
    i2c_start_cond();
    i2c_write_byte(0, 0, 0b11011111);
    unsigned char RTCSEC = i2c_read_byte(0, 0);
    unsigned char RTCMIN = i2c_read_byte(0, 0);
    unsigned char RTCHOUR = i2c_read_byte(0, 0);
    i2c_read_byte(0, 0);
    unsigned char RTCDATE = i2c_read_byte(0, 0);
    unsigned char RTCMTH = i2c_read_byte(0, 0);
    unsigned char RTCYEAR = i2c_read_byte(1, 1);
    i2c_stop_cond();

    //Log("GET S=%xb M=%xb H=%xb D=%xb M=%xb Y=%xb\r\n", RTCSEC, RTCMIN, RTCHOUR, RTCDATE, RTCMTH, RTCYEAR);

    int second = ((RTCSEC & 0b01110000) >> 4)*10;
    second += (RTCSEC & 0b00001111);

    int minute = ((RTCMIN & 0b01110000) >> 4)*10;
    minute += (RTCMIN & 0b00001111);

    int hour = ((RTCHOUR & 0b00110000) >> 4)*10;
    hour += (RTCHOUR & 0b00001111);

    int d = ((RTCDATE & 0b00110000) >> 4)*10;
    d += (RTCDATE & 0b00001111);

    int m = ((RTCMTH & 0b00010000) >> 4)*10;
    m += (RTCMTH & 0b00001111);

    int y = ((RTCYEAR & 0b11110000) >> 4)*10;
    y += (RTCYEAR & 0b00001111);
    y += 2000;

    //Log("Time=%i/%i/%i - %i:%i:%i\r\n", m, d, y, hour, minute, second);

    return SecondsFromEpoch(y, m, d, hour, minute, second);
}

int RTC_SetTime(unsigned long time) {
    unsigned long g = (time / 86400);
    Log("g=%ul\r\n", g);
    unsigned long second = time - (g * 86400);
    Log("second=%ul\r\n", second);

    g += 719468; //Derefference from Epoch 1970-01-01
    Log("g=%ul\r\n", g);

    float yTemp = g;
    yTemp += 1.478;
    yTemp /= 365.2425;
    long year = (long) (yTemp);
    Log("yTemp=%f3 year=%ul", yTemp, year);
    long ddd = g - ((365 * year) + (year / 4) - (year / 100) + (year / 400));
    Log("ddd=%ul\r\n", ddd);
    if (ddd < 0) {
        year--;
        Log("year=%ul\r\n", year);
        ddd = g - ((365 * year) + (year / 4) - (year / 100) + (year / 400));
        Log("ddd=%ul\r\n", ddd);
    }

    long mi = ((100 * ddd) + 52) / 3060;
    Log("mi=%ul\r\n", mi);
    long month = (mi + 2) % 12 + 1;
    Log("month=%ul\r\n", month);
    year = year + (mi + 2) / 12;
    Log("year=%ul\r\n", year);
    long day = ddd - (mi * 306 + 5) / 10 + 1;
    Log("day=%ul\r\n", day);
    long hour = second / 3600;
    Log("hour=%ul\r\n", hour);
    second -= (hour * 3600);
    Log("second=%ul\r\n", second);
    long minute = second / 60;
    Log("minute=%ul\r\n", minute);
    second -= (minute * 60);
    Log("second=%ul\r\n", second);
    year = year % 100;
    Log("year=%ul\r\n", year);


    Log("SET %l/%l/%l - %l:%l:%l\r\n", month, day, year, hour, minute, second);

    unsigned char RTCSEC = ((second / 10) << 4);
    RTCSEC |= (second % 10);
    unsigned char RTCMIN = ((minute / 10) << 4);
    RTCMIN |= (minute % 10);
    unsigned char RTCHOUR = ((hour / 10) << 4);
    RTCHOUR |= (hour % 10);
    unsigned char RTCWKDAY = 0b00001001; //vbat enabled - Power fail cleared
    unsigned char RTCDATE = ((day / 10) << 4);
    RTCDATE |= (day % 10);
    unsigned char RTCMTH = ((month / 10) << 4);
    RTCMTH |= (month % 10);
    unsigned char RTCYEAR = ((year / 10) << 4);
    RTCYEAR |= (year % 10);

    Log("SET S=%xb M=%xb H=%xb D=%xb M=%xb Y=%xb\r\n", RTCSEC, RTCMIN, RTCHOUR, RTCDATE, RTCMTH, RTCYEAR);

    i2c_start_cond();
    i2c_write_byte(0, 0, 0b11011110);
    i2c_write_byte(0, 0, 0x00);
    i2c_write_byte(0, 0, RTCSEC); //Stop the Timer and set the seconds
    i2c_write_byte(0, 0, RTCMIN);
    i2c_write_byte(0, 0, RTCHOUR);
    i2c_write_byte(0, 0, RTCWKDAY);
    i2c_write_byte(0, 0, RTCDATE);
    i2c_write_byte(0, 0, RTCMTH);
    i2c_write_byte(0, 0, RTCYEAR);
    i2c_stop_cond();

    i2c_start_cond();
    i2c_write_byte(0, 0, 0b11011110);
    i2c_write_byte(0, 0, 0x00);
    RTCSEC |= 0b10000000;
    i2c_write_byte(0, 0, RTCSEC);
    i2c_stop_cond();

    return 1;
}

int nvsRAM_Read(unsigned char *dst, unsigned char Address, unsigned char bCount) {
    int nack;
    unsigned char dat;
    Address += 0x20;

    //Set the Read Address to 0x00
    i2c_start_cond();
    nack = i2c_write_byte(0, 0, 0b11011110);
    if (nack) {
        Log("nvsRAM_Read NACK Addressing for Write\r\n");
        return 0;
    }

    nack = i2c_write_byte(0, 0, Address);
    if (nack) {
        Log("nvsRAM_Read NACK Setting Address\r\n");
        return 0;
    }

    i2c_start_cond();
    nack = i2c_write_byte(0, 0, 0b11011111);
    if (nack) {
        Log("nvsRAM_Read NACK Addressing for Read\r\n");
        return 0;
    }

    while (bCount > 1) {
        dat = i2c_read_byte(0, 0);
        *(dst++) = dat;
        bCount--;
    }
    dat = i2c_read_byte(1, 1);
    *(dst++) = dat;
    i2c_stop_cond();
    return 1;
}

int nvsRAM_Write(unsigned char *buffer, unsigned char Address, unsigned char bCount) {
    int nack;
    Address += 0x20;

    //Set the Read Address to 0x00
    i2c_start_cond();
    i2c_write_byte(0, 0, 0b11011110);
    i2c_write_byte(0, 0, Address);

    while (bCount--) {
        nack = i2c_write_byte(0, 0, *buffer);
        if (nack) {
            Log("nvsRAM_Write: NACK\r\n");
            return 0;
        }
        buffer++;
    }
    i2c_stop_cond();
    return 1;
}

