#include <stdarg.h>
#include <stdio.h>

unsigned char *Packv(unsigned char *buffer, const char *format, va_list arguments) {

    union {
        float f;
        unsigned long ul;
        signed long l;
        unsigned int ui[2];
        signed int i[2];
        unsigned char ub[4];
        signed char b[4];
    } q;

    char *str;
    int arrayLength;
    unsigned char *ptrArray;

    while (1) {
        switch (*(format++)) {
            case 0:
                return buffer;
                break;
            case 'b':
                *(buffer++) = va_arg(arguments, unsigned char);
                break;
            case 'i':
                q.ui[0] = va_arg(arguments, unsigned int);
                *(buffer++) = q.ub[0];
                *(buffer++) = q.ub[1];
                break;
            case 'l':
                q.ul = va_arg(arguments, unsigned long);
                *(buffer++) = q.ub[0];
                *(buffer++) = q.ub[1];
                *(buffer++) = q.ub[2];
                *(buffer++) = q.ub[3];
                break;
            case 'B':
                q.b[0]=va_arg(arguments, char);
                *(buffer++)=q.ub[0];
                break;
            case 'I':
                q.i[0] = va_arg(arguments, int);
                *(buffer++) = q.ub[0];
                *(buffer++) = q.ub[1];
                break;
            case 'L':
                q.l = va_arg(arguments, long);
                *(buffer++) = q.ub[0];
                *(buffer++) = q.ub[1];
                *(buffer++) = q.ub[2];
                *(buffer++) = q.ub[3];
                break;
            case 'f':
                q.f = va_arg(arguments, float);
                *(buffer++) = q.ub[0];
                *(buffer++) = q.ub[1];
                *(buffer++) = q.ub[2];
                *(buffer++) = q.ub[3];
                break;
            case 's':
                str = va_arg(arguments, char*);
                while (*str) *(buffer++) = *(str++);
                break;
            case 'S':
                str = va_arg(arguments, char*);
                while (*str) *(buffer++) = *(str++);
                *(buffer++) = 0;
                break;
            case 'a':
                ptrArray = va_arg(arguments, unsigned char *);
                arrayLength = va_arg(arguments, unsigned int);
                while (arrayLength--) *(buffer++) = *(ptrArray++);
                break;
        }
    }
}

unsigned char *Pack(unsigned char *buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);

    return Packv(buffer, format, args);
}

unsigned char *Unpackv(unsigned char *buffer, const char *format, va_list arguments) {

    union {
        float f;
        unsigned long ul;
        signed long l;
        unsigned int ui[2];
        signed int i[2];
        unsigned char ub[4];
        signed char b[4];
    } q;

    char *ptrChar;
    unsigned char *ptrByte;
    unsigned int *ptrUINT16;
    unsigned long *ptrUINT32;
    signed int *ptrINT16;
    signed long *ptrINT32;
    float *ptrFloat;


    char *str;
    int arrayLength;
    unsigned char *ptrArray;

    while (1) {
        switch (*(format++)) {
            case 0:
                return buffer;
                break;
            case 'b':
                ptrByte = va_arg(arguments, unsigned char*);
                q.ub[0] = *(buffer++);
                *ptrByte = q.ub[0];
                break;
            case 'i':
                ptrUINT16 = va_arg(arguments, unsigned int*);
                q.ub[0] = *(buffer++);
                q.ub[1] = *(buffer++);
                *ptrUINT16 = q.ui[0];
                break;
            case 'l':
                ptrUINT32 = va_arg(arguments, unsigned long*);
                q.ub[0] = *(buffer++);
                q.ub[1] = *(buffer++);
                q.ub[2] = *(buffer++);
                q.ub[3] = *(buffer++);
                *ptrUINT32 = q.ul;
                break;
            case 'I':
                ptrINT16 = va_arg(arguments, signed int*);
                q.ub[0] = *(buffer++);
                q.ub[1] = *(buffer++);
                *ptrINT16 = q.i[0];
                break;
            case 'L':
                ptrINT32 = va_arg(arguments, signed long*);
                q.ub[0] = *(buffer++);
                q.ub[1] = *(buffer++);
                q.ub[2] = *(buffer++);
                q.ub[3] = *(buffer++);
                *ptrINT32 = q.l;
                break;
            case 'f':
                ptrFloat = va_arg(arguments, float*);
                q.ub[0] = *(buffer++);
                q.ub[1] = *(buffer++);
                q.ub[2] = *(buffer++);
                q.ub[3] = *(buffer++);
                *ptrFloat = q.f;
                break;
            case 'c':
                ptrChar = va_arg(arguments, char*);
                q.ub[0] = *(buffer++);
                *ptrChar = q.b[0];
                break;
            case 's':
                str = va_arg(arguments, char*);
                while (*buffer) {
                    *(str++) = *(buffer++);
                }
                break;
            case 'a':
                ptrArray = va_arg(arguments, unsigned char *);
                arrayLength = va_arg(arguments, unsigned int);
                while (arrayLength--) *(ptrArray++) = *(buffer++);
                break;
        }
    }
}

unsigned char *Unpack(unsigned char *buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);

    return Unpackv(buffer, format, args);
}
