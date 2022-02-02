/*________________________________  epson-escpage-cmp.h   ______________________________*/

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
/*                            Epson ESC/Page Compless Functions                         */
/*                                                                                      */
/*******************************************|********************************************/

#ifndef	_EPSON_ESCPAGE_COMP_H_
#define _EPSON_ESCPAGE_COMP_H_
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-pvt.h"

/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/
/* Define Compress Mode Constant */
#define	EP_COMPRESS0		0		/* not commpress */
#define EP_COMPRESS20		20

/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/
typedef struct tagEPS_BITMAPINFO{
        EPS_INT32       biWidth;
        EPS_INT32       biHeight;
        EPS_UINT16       biBitCount;
} EPS_BITMAPINFO;


/*---------------------------  Public Function Declarations  ---------------------------*/
/*******************************************|********************************************/
extern EPS_UINT8*	CompressBitImage(
	EPS_RECT			*pRec,
	EPS_BITMAPINFO		*pBitMapInfo,
	EPS_UINT8			*pRealBits,
	EPS_UINT32			ulDataFormat,
	EPS_UINT32			*pulCompressType,
	EPS_UINT32			*pulImageSize
	);


#ifdef __cplusplus
}
#endif

#endif	/* _EPSON_ESCPAGE_COMP_H_ */

/*________________________________  epson-escpage-cmp.h   ______________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
