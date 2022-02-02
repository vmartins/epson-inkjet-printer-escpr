/*__________________________________   linux-cmn.h   ___________________________________*/

/*       1         2         3         4         5         6         7         8        */
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*******************************************|********************************************/
/*
 *   Copyright (c) 2007  Seiko Epson Corporation                 All rights reserved.
 *
 *   Copyright protection claimed includes all forms and matters of
 *   copyrightable material and information now allowed by statutory or judicial
 *   law or hereinafter granted, including without limitation, material generated
 *   from the software programs which are displayed on the screen such as icons,
 *   screen display looks, etc.
 *
 */
/*******************************************|********************************************/
/*                                                                                      */
/*                      Linux platform depend External Definitions                      */
/*                                                                                      */
/*                                    Function Calls                                    */
/*                              --------------------------                              */
/*                  void*		epsmpMemAlloc	(size								);  */
/*                  void		epsmpMemFree	(memblock							);  */
/*                  EPS_INT32   epsmpSleep		(time								);  */
/*                  EPS_UINT32  epsmpGetTime	(void								);  */
/*                  EPS_INT32   epsmpLockSync	(void								);  */
/*                  EPS_INT32   epsmpUnlockSync (void								);  */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef __LINUX_CMN__
#define __LINUX_CMN__

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"
#include "epson-escpr-def.h"

/*---------------------------  Public Function Declarations  ---------------------------*/
/*******************************************|********************************************/

void*		epsmpMemAlloc	(size_t														);
void		epsmpMemFree	(void*														);
EPS_INT32	epsmpSleep		(EPS_UINT32													);
EPS_UINT32	epsmpGetTime	(void														);
EPS_UINT32  epsmpGetLocalTime(EPS_LOCAL_TIME *                                          );
EPS_INT32	epsmpLockSync	(void														);
EPS_INT32	epsmpUnlockSync (void														);


EPS_INT32	_kbhit			(void														);
EPS_INT32	_getch			(void														);
void		_endthread		(void                             							);
void		InitMutex		(void														);
void		TermMutex		(void														);

#ifdef __cplusplus
}
#endif

#endif	/* def __LINUX_CMN__ */

/*__________________________________   linux-cmn.h   ___________________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
