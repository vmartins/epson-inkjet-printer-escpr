/*_________________________________   epson-layout.h   _________________________________*/

/*       1         2         3         4         5         6         7         8        */
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*******************************************|********************************************/
/*
*   Copyright (c) 2009  Seiko Epson Corporation                 All rights reserved.
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
/*                            Epson Extent Layout Definitions                           */
/*                                                                                      */
/*******************************************|********************************************/

#ifndef __EPSON_LAYOUT_H__
#define __EPSON_LAYOUT_H__
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"
#include "epson-escpr-pvt.h"


/*----------------------------------  Generic Macros   ---------------------------------*/
/*******************************************|********************************************/

   /*** CD/DVD Sizing Information                                                      */
    /*** -------------------------------------------------------------------------------*/
#define CDDVD_OFFSET_X(r, d)    (EPS_INT32)(elGetDots(r, (140 + (EPS_CDDIM_OUT_DEF*10 - d*10)/2)) + elGetDots(r, 30))
#define CDDVD_OFFSET_Y(r, d)    (EPS_INT32)(elGetDots(r, ( 65 + (EPS_CDDIM_OUT_DEF*10 - d*10)/2)) + elGetDots(r, 30))

/*----------------------------  API Function Declarations   ----------------------------*/
/*******************************************|********************************************/
extern EPS_INT32	elGetDots		(EPS_UINT8 inputResolution, EPS_UINT32 millimeter   );

#ifdef GCOMSW_EL_CDLABEL
extern EPS_ERR_CODE elCDClipping	(const EPS_UINT8*, EPS_UINT8*, EPS_UINT8, EPS_RECT* );
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* def __EPSON_LAYOUT_H__ */

/*_________________________________   epson-layout.h   _________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
