/*_______________________________   epson-net-lpr.h   ________________________________*/

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
/*                            Epson LPR Protocol Definitions                            */
/*                                                                                      */
/*******************************************|********************************************/

#ifndef __EPSON_NET_LPR_H__
#define __EPSON_NET_LPR_H__
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"

/*--------------------------  Public Function Declarations   ---------------------------*/
/*******************************************|********************************************/
/* Initialize                                                                           */
extern void			lprSetupSTFunctions	(const EPS_PRINTER_INN*							);
extern EPS_UINT16	lprGetDefautiPort	(void                                           );

/* Discovery message related Functions */
extern EPS_ERR_CODE lprFindStart        (EPS_SOCKET*, const EPS_INT8*, EPS_BOOL, const EPS_UINT8*);
extern EPS_ERR_CODE lprFind             (EPS_SOCKET, EPS_PRINTER_INN**                  );
extern EPS_ERR_CODE lprFindEnd          (EPS_SOCKET                                     );
extern EPS_ERR_CODE lprProbePrinterByID (EPS_INT8*, EPS_UINT32, EPS_PRINTER_INN**       );

/* Job Functions */
extern EPS_ERR_CODE lprStartJob         (void                                           );
extern EPS_ERR_CODE lprEndJob           (void                                           );
extern EPS_ERR_CODE lprRestartJob       (void                                           );
extern EPS_ERR_CODE lprWritePrintData   (const EPS_UINT8*, EPS_UINT32, EPS_UINT32*      );
extern EPS_ERR_CODE lprResetPrinter     (void                                           );
extern EPS_ERR_CODE lprStartPage        (void                                           );
extern EPS_ERR_CODE lprEndPage          (void                                           );
extern EPS_ERR_CODE lprMechCommand      (const EPS_PRINTER_INN*, EPS_INT32              );

/* Printer status Functions */
extern EPS_ERR_CODE lprGetStatus        (EPS_STATUS_INFO*, EPS_BOOL*, EPS_BOOL*         );
extern EPS_ERR_CODE lprGetJobStatus     (EPS_STATUS_INFO*                               );
extern EPS_ERR_CODE lprGetInfo          (const EPS_PRINTER_INN*, EPS_INT32, 
										 EPS_UINT8**, EPS_INT32*                        );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* def __EPSON_NET_LPR_H__ */

/*_______________________________   epson-net-lpr.h   ________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
