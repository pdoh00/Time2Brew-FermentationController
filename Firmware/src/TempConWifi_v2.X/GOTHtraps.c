/** GOTHtraps.c
 *
 * This file contains the code that is executed when one of the
 * following software traps occurs:
 *	- Oscillator fail
 *	- Adress error
 *	- Stack error
 *	- Math error
 *
 * The primary handlers as well as the alternative handlers are
 * implemented.
 * When the system is not running in debug mode, the processor
 * is reset. This could problematic in case of an oscillator
 * failure. However, this has not been tested.
 *
 * The errorlocations should be printed as debug output (UART).
 * However, it is uncertain if they are correct.
 */

#include "GOTHtraps.h"
#include "SystemConfiguration.h"

void (*errLoc)(void); // Function Pointer
void (*upLoc)(void);

/* Primary Exception Vector handlers:
These routines are used if INTCON2bits.ALTIVT = 0.
All trap service routines in this file simply ensure that device
continuously executes code within the trap service routine. Users
may modify the basic framework provided here to suit to the needs
of their application.
 */
void __attribute__((interrupt, no_auto_psv)) _OscillatorFail(void) {
    errLoc = getErrLoc(); // get the error function
    upLoc = getUpLoc(); // get the upper func of error func

    INTCON1bits.OSCFAIL = 0; // clear the trap flag

    while(1);

    ///@todo add your error code here

    asm("reset"); // reset processor

    // caution!
    /* Whenever the RESET instruction is executed, the device
       asserts SYSRST. This Reset state does not re-initialize
       the clock. The clock source in effect prior to the RESET
       instruction remains.
     */
}

void __attribute__((interrupt, no_auto_psv)) _AddressError(void) {
    char buff[256];
    errLoc = getErrLoc(); // get the error function
    upLoc = getUpLoc(); // get the upper func of error func


    INTCON1bits.ADDRERR = 0; // clear the trap flag

    ///@todo add your error code here

    sprintf(buff, "Address Error: errLoc=%04x upLoc=%04x\r\nHALT", *(unsigned int *) errLoc, *(unsigned int *) upLoc);
    char *cursor = buff;
    while (*cursor) {
        while (U2STAbits.UTXBF);
        U2TXREG = *(cursor++);
    }
    while (1);

    asm("reset"); // reset processor
}

void __attribute__((interrupt, no_auto_psv)) _StackError(void) {
    char buff[256];
    errLoc = getErrLoc(); // get the error function
    upLoc = getUpLoc(); // get the upper func of error func

    INTCON1bits.STKERR = 0; //Clear the trap flag

    ///@todo add your error code here
    sprintf(buff, "Stack Error: errLoc=%04x upLoc=%04x\r\nHALT", *(unsigned int *) errLoc, *(unsigned int *) upLoc);
    char *cursor = buff;
    while (*cursor) {
        while (U2STAbits.UTXBF);
        U2TXREG = *(cursor++);
    }
    while (1);

    asm("reset"); // reset processor

}

void __attribute__((interrupt, no_auto_psv)) _MathError(void) {
    char buff[256];
    errLoc = getErrLoc(); // get the error function
    upLoc = getUpLoc(); // get the upper func of error func

    ///@todo add your error code here
    sprintf(buff, "Math Error: errLoc=%04x upLoc=%04x\r\nHALT", *(unsigned int *) errLoc, *(unsigned int *) upLoc);
    char *cursor = buff;
    while (*cursor) {
        while (U2STAbits.UTXBF);
        U2TXREG = *(cursor++);
    }
    while (1);

    INTCON1bits.MATHERR = 0; // clear the trap flag

    asm("reset"); // reset processor
}

