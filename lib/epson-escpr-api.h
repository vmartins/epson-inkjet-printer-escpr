/*_________________________________  epson-escpr-api.h   _______________________________*/

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
/*                    Epson ESC/PR Application Programming Interfaces                   */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef __EPSON_ESCPR_API_H__
#define __EPSON_ESCPR_API_H__
#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------  Compile options   --------------------------------*/
/*******************************************|********************************************/
/* GCOMSW_PRT_USE_USB 	Use USB printer.                                                */
/* GCOMSW_PRT_USE_LPR 	Use LPR printer.                                                */
/* GCOMSW_PRT_USE_RAW 	Use Raw printer.                                                */

/* GCOMSW_EL_CDLABEL	Extend layout. Auto clipping at CD/DVD label print. (RGB only)  */

/* GCOMSW_DEBUG 		Enable Debug monde.                                             */

#define GCOMSW_UPDATE_PAGE_REMAINDER 1

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"
#include "epson-escpr-def.h"
#include "epson-escpr-err.h"
#include "epson-escpr-media.h"

#include "epson-escpr-pvt.h"


/*----------------------------------  Generic Macros   ---------------------------------*/
/*******************************************|********************************************/

/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/

/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/

/*--------------------------------  Global Declarations   ------------------------------*/
/*******************************************|********************************************/

/*----------------------------  API Function Declarations   ----------------------------*/
/*******************************************|********************************************/
    /*** Printing API                                                                   */
    /*** -------------------------------------------------------------------------------*/
extern EPS_ERR_CODE epsInitDriver        (EPS_INT32, const EPS_USB_FUNC*, 
										  const EPS_NET_FUNC*, const EPS_CMN_FUNC*      );
extern EPS_ERR_CODE epsReleaseDriver     (void                                          );
extern EPS_ERR_CODE epsSetPrinter        (const EPS_PRINTER*                            );
extern EPS_ERR_CODE epsStartJob          (const EPS_JOB_ATTRIB*                         );
extern EPS_ERR_CODE epsStartPage         (const EPS_PAGE_ATTRIB*, EPS_UINT32            );
extern EPS_ERR_CODE epsSendData          (EPS_PRN_DATA*									);
extern EPS_ERR_CODE epsPrintBand         (const EPS_UINT8*, EPS_UINT32, EPS_UINT32*     );
#if GCOMSW_UPDATE_PAGE_REMAINDER
extern EPS_ERR_CODE epsEndPage           (EPS_INT32                                     );
#else
extern EPS_ERR_CODE epsEndPage           (EPS_BOOL                                      );
#endif
extern EPS_ERR_CODE epsEndJob            (void                                          );
extern EPS_ERR_CODE epsCancelJob         (EPS_INT32                                     );
extern EPS_ERR_CODE epsContinueJob       (void                                          );

extern EPS_ERR_CODE epsSetAdditionalData (EPS_UINT32, const void*                       );
extern EPS_ERR_CODE epsRemAdditionalData (EPS_UINT32                                    );

    /*** Find Printer Function                                                          */
    /*** -------------------------------------------------------------------------------*/
extern EPS_ERR_CODE epsFindPrinter       (EPS_INT32, EPS_UINT32                         );
extern EPS_ERR_CODE epsProbePrinter      (const EPS_PROBE*                              );
extern EPS_ERR_CODE epsCancelFindPrinter (void                                          );

    /*** Get Status Function                                                            */
    /*** -------------------------------------------------------------------------------*/
extern EPS_ERR_CODE epsGetStatus         (EPS_STATUS*                                   );
extern EPS_ERR_CODE epsGetInkInfo        (EPS_INK_INFO*                                 );
extern EPS_ERR_CODE epsGetSupplyInfo     (EPS_SUPPLY_INFO*                              );

    /*** Get Supported Media Function                                                   */
    /*** -------------------------------------------------------------------------------*/
extern EPS_ERR_CODE epsGetSupportedMedia (EPS_SUPPORTED_MEDIA*                          );

    /*** Get Printable Area                                                             */
    /*** -------------------------------------------------------------------------------*/
extern EPS_ERR_CODE epsGetPrintableArea  (EPS_JOB_ATTRIB*, EPS_UINT32*, EPS_UINT32*     );
extern EPS_ERR_CODE epsGetPrintAreaInfo  (const EPS_JOB_ATTRIB*, EPS_UINT32*, EPS_UINT32*,
		                                  EPS_LAYOUT_INFO*                            );
extern EPS_ERR_CODE epsGetPrintAreaInfoAll(EPS_UINT32, EPS_PRINT_AREA_INFO*             );
extern EPS_ERR_CODE epsGetUsersizeRange  (EPS_UINT8, EPS_UINT32*, EPS_UINT32*, 
										  EPS_UINT32*, EPS_UINT32*                      );

    /*** Maintenance Command function                                                   */
    /*** -------------------------------------------------------------------------------*/
extern EPS_ERR_CODE epsMakeMainteCmd     (EPS_INT32, EPS_UINT8*, EPS_UINT32*            );

#ifdef __cplusplus
}
#endif

#endif  /* def __EPSON_ESCPR_API_H__ */

/*_________________________________  epson-escpr-api.h   _______________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
