/*_______________________________   epson-escpr-pm.h   _________________________________*/

/*       1         2         3         4         5         6         7         8        */
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*******************************************|********************************************/
/*
 *   Copyright (c) 2014  Seiko Epson Corporation                 All rights reserved.
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
/*                         Epson ESC/PR print media Functions                           */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef __EPSON_ESCPR_PM_H__
#define __EPSON_ESCPR_PM_H__
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"
#include "epson-escpr-pvt.h"

/*------------------------------------- Data Types -------------------------------------*/
/*******************************************|********************************************/

/*--------------------------------------  Macros   -------------------------------------*/
/*******************************************|********************************************/

/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/

/*---------------------------  Public Function Declarations  ---------------------------*/
/*******************************************|********************************************/

    /*** pm reply                                                                       */
    /*** -------------------------------------------------------------------------------*/
extern EPS_ERR_CODE epspmCreateMediaInfo        (EPS_PRINTER_INN*, EPS_UINT8*, EPS_INT32, EPS_BOOL*);
extern EPS_ERR_CODE epspmCreateMediaInfo3       (EPS_PRINTER_INN*, EPS_UINT8*, EPS_INT32);
extern void         epspmClearMediaInfo         (EPS_SUPPORTED_MEDIA*                   );
extern EPS_ERR_CODE epspmMargePaperSource       (EPS_PRINTER_INN*, EPS_UINT8*, EPS_INT32);
extern EPS_ERR_CODE epspmChangeSpec_DraftOnly   (EPS_PRINTER_INN*, EPS_JOB_ATTRIB*      );

extern EPS_ERR_CODE epspmGetPrintAreaInfo       (EPS_BOOL, const EPS_JOB_ATTRIB*, EPS_PM_DATA*,
                                                 EPS_UINT32*, EPS_UINT32*, EPS_LAYOUT_INFO*,
												 EPS_RECT*                              );
extern EPS_ERR_CODE epspmCreatePrintAreaInfoFromTable (const EPS_SUPPORTED_MEDIA*, EPS_UINT32,
		                                         EPS_PRINT_AREA_INFO*                   );
extern EPS_ERR_CODE epspmCreatePrintAreaInfoFromPM(EPS_UINT8*, EPS_INT32, EPS_UINT32,
                                                 EPS_PRINT_AREA_INFO*                   );
extern void         epspmClearPrintAreaInfo     (EPS_PRINT_AREA_INFO*                   );
#ifdef __cplusplus
}
#endif

#endif  /* def __EPSON_ESCPR_PM_H__ */


/*_______________________________   epson-escpr-pm.h   _________________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
