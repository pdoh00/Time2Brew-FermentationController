/* GOTHtraps.h
 *
 *	Function prototypes for GOTHtraps.h
 */
#ifndef __GOTH_TRAPS_H__
#define __GOTH_TRAPS_H__

#include "p33Exxxx.h"
#include "stdio.h"
/* void (*getErrLoc(void))(void)
 * These functions are in the getErrLoc.s file
 */
void (*getErrLoc(void))(void); // Get Address Error Loc
void (*getUpLoc(void))(void); // Get Address Upper Loc

#endif //__GOTH_TRAPS_H__
