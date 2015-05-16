#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "circularPrintF.h"
#include "main.h"
#include "FIFO.h"

static const char digits[201] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

static const char hex_digits[16] = "0123456789ABCDEF";

FIFO_BUFFER *target;

#define OutputChar(val) {FIFO_Write(target,val);}
#define OutputArray(data,len) {FIFO_WriteArray(target,len,data);}

int circularPrintf_uint32_to_hex(uint32_t value) {
    FIFO_WriteData(target, 10, '0', 'x',
            hex_digits[(value >> 28) & 0xF],
            hex_digits[(value >> 24) & 0xF],
            hex_digits[(value >> 20) & 0xF],
            hex_digits[(value >> 16) & 0xF],
            hex_digits[(value >> 12) & 0xF],
            hex_digits[(value >> 8) & 0xF],
            hex_digits[(value >> 4) & 0xF],
            hex_digits[(value) & 0xF]);
    return 10;
}

int circularPrintf_uint16_to_hex(uint16_t value) {
    FIFO_WriteData(target, 6, '0', 'x',
            hex_digits[(value >> 12) & 0xF],
            hex_digits[(value >> 8) & 0xF],
            hex_digits[(value >> 4) & 0xF],
            hex_digits[(value) & 0xF]);

    return 6;
}

int circularPrintf_uint8_to_hex(uint8_t value) {
    FIFO_WriteData(target, 4, '0', 'x',
            hex_digits[(value >> 04) & 0xF],
            hex_digits[(value) & 0xF]);


    return 4;
}

int circularPrintf_uint32_to_str(uint32_t value) {
    char dst[10];
    int length;
    if (value > 999999999) {
        length = 10;
    } else if (value > 99999999) {
        length = 9;
    } else if (value > 9999999) {
        length = 8;
    } else if (value > 999999) {
        length = 7;
    } else if (value > 99999) {
        length = 6;
    } else if (value > 9999) {
        length = 5;
    } else if (value > 999) {
        length = 4;
    } else if (value > 99) {
        length = 3;
    } else if (value > 9) {
        length = 2;
    } else {
        length = 1;
    }

    int next = length - 1;
    int i;
    while (value >= 100) {
        i = (value % 100) << 1;
        value /= 100;
        dst[next] = digits[i + 1];
        next--;
        dst[next] = digits[i];
        next--;
    }
    // Handle last 1-2 digits
    if (value < 10) {
        dst[next] = '0' + value;
        next--;
    } else {
        i = value << 1;
        dst[next] = digits[i + 1];
        next--;
        dst[next] = digits[i];
        next--;
    }
    OutputArray((BYTE *) dst, length);
    return length;
}

int circularPrintf_uint32_to_str_fixedLength(uint32_t value, int fixedLen) {
    char dst[10];
    int length;
    if (value > 999999999) {
        length = 10;
    } else if (value > 99999999) {
        length = 9;
    } else if (value > 9999999) {
        length = 8;
    } else if (value > 999999) {
        length = 7;
    } else if (value > 99999) {
        length = 6;
    } else if (value > 9999) {
        length = 5;
    } else if (value > 999) {
        length = 4;
    } else if (value > 99) {
        length = 3;
    } else if (value > 9) {
        length = 2;
    } else {
        length = 1;
    }

    int next = length - 1;
    int i;
    while (value >= 100) {
        i = (value % 100) << 1;
        value /= 100;
        dst[next] = digits[i + 1];
        next--;
        dst[next] = digits[i];
        next--;
    }
    // Handle last 1-2 digits
    if (value < 10) {
        dst[next] = '0' + value;
        next--;
    } else {
        i = value << 1;
        dst[next] = digits[i + 1];
        next--;
        dst[next] = digits[i];
        next--;
    }

    int needZeros = (fixedLen - length);
    while (needZeros--) {
        OutputChar('0');
    }

    OutputArray((BYTE *) dst, length);

    return fixedLen;
}

int circularPrintf_uint16_to_str(uint16_t value) {
    char dst[5];
    int length;
    if (value > 9999) {
        length = 5;
    } else if (value > 999) {
        length = 4;
    } else if (value > 99) {
        length = 3;
    } else if (value > 9) {
        length = 2;
    } else {
        length = 1;
    }
    int next = length - 1;
    int i;
    while (value >= 100) {
        i = (value % 100) << 1;
        value /= 100;
        dst[next] = digits[i + 1];
        next--;
        dst[next] = digits[i];
        next--;
    }
    // Handle last 1-2 digits
    if (value < 10) {
        dst[next] = '0' + value;
        next--;
    } else {
        i = value << 1;
        dst[next] = digits[i + 1];
        next--;
        dst[next] = digits[i];
        next--;
    }
    OutputArray((BYTE*) dst, length);
    return length;

}

int circularPrintf_float_to_str(float value, char digits) {
    int len = 0;
    uint32_t precision = 1;
    int x;
    for (x = 0; x < digits; x++) {
        precision *= 10;
    }
    if (value < 0) {
        value = 0 - value;
        OutputChar('-');
        len++;
    }
    uint32_t val = (uint32_t) value;
    len += circularPrintf_uint32_to_str(val);

    if (digits > 0) {

        value -= val;
        value *= precision;

        val = (uint32_t) value;
        OutputChar('.');
        len++;
        len += circularPrintf_uint32_to_str_fixedLength(val, digits);
    }
    return len;
}

int circularPrintf(FIFO_BUFFER *fifo, const char *format, ...) {
    va_list args;
    va_start(args, format);

    return circular_vPrintf(fifo, format, args);
}

int circular_vPrintf(FIFO_BUFFER *fifo, const char *format, va_list arguments) {
    target = fifo;
    char precision;
    char token;
    const char *str;
    int len = 0;
    int stringCharCount;

    OMNI numValue;

    while (*format) {
        token = *(format++);
        if (token == '%') {
            token = *(format++);
            switch (token) {
                case 'S':
                    stringCharCount = va_arg(arguments, int);
                    str = va_arg(arguments, const char *);
                    while (stringCharCount--) {
                        if (*str < 32 || *str > 126) {
                        } else {
                            OutputChar(*str);
                            len++;
                        }
                        str++;
                    }
                    break;
                case 's':
                    str = va_arg(arguments, const char *);
                    while (*str) {
                        OutputChar(*(str++));
                        len++;
                    }
                    break;
                case 'u':
                    token = *(format++);
                    switch (token) {
                        case 'l':
                            numValue.ul = va_arg(arguments, uint32_t);
                            len += circularPrintf_uint32_to_str(numValue.ul);
                            break;
                        case 'i':
                        case 'd':
                            numValue.ui[0] = va_arg(arguments, uint16_t);
                            len += circularPrintf_uint16_to_str(numValue.ui[0]);
                            break;
                        case 'b':
                            numValue.ub[0] = va_arg(arguments, uint8_t);
                            len += circularPrintf_uint16_to_str(numValue.ub[0]);
                            break;
                    }
                    break;
                case 'l':
                    numValue.l = va_arg(arguments, int32_t);
                    if (numValue.l < 0) {
                        numValue.l = 0 - numValue.l;
                        OutputChar('-');
                        len++;
                    }
                    len += circularPrintf_uint32_to_str(numValue.ul);
                    break;
                case 'd':
                case 'i':
                    numValue.i[0] = va_arg(arguments, int16_t);
                    if (numValue.i[0] < 0) {
                        numValue.i[0] = 0 - numValue.i[0];
                        OutputChar('-');
                        len++;
                    }
                    len += circularPrintf_uint16_to_str(numValue.ui[0]);
                    break;
                case 'b':
                    numValue.b[0] = va_arg(arguments, int8_t);
                    if (numValue.b[0] < 0) {
                        numValue.b[0] = 0 - numValue.b[0];
                        OutputChar('-');
                    }
                    len += circularPrintf_uint16_to_str(numValue.ub[0]);
                    break;
                case 'f':
                    token = *(format++);
                    precision = token - '0';
                    if (precision >= 0 && precision <= 9) {
                        numValue.f = va_arg(arguments, float);
                        len += circularPrintf_float_to_str(numValue.f, precision);
                    }
                    break;
                case 'X':
                case 'x':
                    token = *(format++);
                    if (token == 'l') {
                        numValue.ul = va_arg(arguments, uint32_t);
                        len += circularPrintf_uint32_to_hex(numValue.ul);
                    } else if (token == 'i') {
                        numValue.ui[0] = va_arg(arguments, uint16_t);
                        len += circularPrintf_uint16_to_hex(numValue.ui[0]);
                    } else if (token == 'b') {
                        numValue.ub[0] = va_arg(arguments, uint8_t);
                        len += circularPrintf_uint8_to_hex(numValue.ub[0]);
                    }
                    break;
            }
        } else {
            OutputChar(token);
            len++;
        }
    }
    return len;
}
