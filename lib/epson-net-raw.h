/*_________________________________   epson-net-raw.h   ________________________________*/

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
/*                         Epson Raw Port Protocol Definitions                          */
/*                                                                                      */
/*******************************************|********************************************/

#ifndef __EPSON_NET_RAW_H__
#define __EPSON_NET_RAW_H__
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"

/*----------------------------  API Function Declarations   ----------------------------*/
/*******************************************|********************************************/
/* Initialize                                                                           */
extern void			rawSetupSTFunctions	(const EPS_PRINTER_INN*							);
extern EPS_UINT16	rawGetDefautiPort	(void                                           );

/* Discovery message related Functions */
extern EPS_ERR_CODE rawFindStart        (EPS_SOCKET*, const EPS_INT8*, EPS_BOOL, const EPS_UINT8*);
extern EPS_ERR_CODE rawFind             (EPS_SOCKET, EPS_PRINTER_INN**                  );
extern EPS_ERR_CODE rawFindEnd          (EPS_SOCKET                                     );
extern EPS_ERR_CODE rawProbePrinterByID (EPS_INT8*, EPS_UINT32, EPS_PRINTER_INN**       );

/* Job Functions */
extern EPS_ERR_CODE rawStartJob         (void                                           );
extern EPS_ERR_CODE rawEndJob           (void                                           );
extern EPS_ERR_CODE rawRestartJob       (void                                           );
extern EPS_ERR_CODE rawWritePrintData   (const EPS_UINT8*, EPS_UINT32, EPS_UINT32*      );
extern EPS_ERR_CODE rawResetPrinter     (void                                           );
extern EPS_ERR_CODE rawStartPage        (void                                           );
extern EPS_ERR_CODE rawEndPage          (void                                           );
extern EPS_ERR_CODE rawMechCommand      (const EPS_PRINTER_INN*, EPS_INT32              );

/* Printer status Functions */
extern EPS_ERR_CODE rawGetStatus        (EPS_STATUS_INFO*, EPS_BOOL*, EPS_BOOL*         );
extern EPS_ERR_CODE rawGetJobStatus     (EPS_STATUS_INFO*                               );
extern EPS_ERR_CODE rawGetInfo          (const EPS_PRINTER_INN*, EPS_INT32, 
										 EPS_UINT8**, EPS_INT32*                        );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* def __EPSON_NET_RAW_H__ */

/*_________________________________   epson-net-raw.h   ________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
