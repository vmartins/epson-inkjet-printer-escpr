/*______________________________  epson-escpage-color.h   ______________________________*/

/*       1         2         3         4         5         6         7         8        */
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*******************************************|********************************************/
/*
 *   Copyright (c) 2010  Seiko Epson Corporation                 All rights reserved.
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
/*                         Epson ESC/Page-Color command Functions                       */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef __EPSON_ESCPAGE_COLOR_H__
#define __EPSON_ESCPAGE_COLOR_H__
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-pvt.h"

/*--------------------------  Public Function Declarations   ---------------------------*/
/*******************************************|********************************************/
extern EPS_ERR_CODE pageAllocBuffer_C	(void);
extern void			pageRelaseBuffer_C	(void);
extern EPS_ERR_CODE pageStartJob_C		(void);
extern EPS_ERR_CODE pageEndJob_C		(void);
extern EPS_ERR_CODE pageStartPage_C		(void);
extern EPS_ERR_CODE pageEndPage_C		(void);
extern EPS_ERR_CODE pageColorRow_C		(const EPS_BANDBMP*, EPS_RECT*);
extern EPS_ERR_CODE pageSendLeftovers_C	(void);

#ifdef __cplusplus
}
#endif

#endif  /* def __EPSON_ESCPAGE_COLOR_H__ */

/*______________________________  epson-escpage-color.h   ______________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
