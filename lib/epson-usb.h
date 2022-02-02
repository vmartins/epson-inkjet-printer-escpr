/*___________________________________  epson-usb.h   ___________________________________*/

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
/*                               Epson USB I/F Definitions                              */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef _EPSON_USB_H_
#define _EPSON_USB_H_
#ifdef    __cplusplus
extern "C" {
#endif    /* def __cplusplus    */

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"

/*------------------------------------- Data Types -------------------------------------*/
/*******************************************|********************************************/

/*----------------------------------  Generic Macros   ---------------------------------*/
/*******************************************|********************************************/

/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/

/*--------------------------  Public Function Declarations   ---------------------------*/
/*******************************************|********************************************/
extern EPS_ERR_CODE usbFind		        (EPS_UINT32*, EPS_INT32                         );
extern EPS_ERR_CODE usbProbePrinterByID (EPS_INT8*, EPS_UINT32, EPS_PRINTER_INN**       );

extern EPS_ERR_CODE usbStartJob		    (void                                           );
extern EPS_ERR_CODE usbRestartJob       (void                                           );
extern EPS_ERR_CODE usbEndJob           (void                                           );
extern EPS_ERR_CODE usbWritePrintData   (const EPS_UINT8*, EPS_UINT32, EPS_UINT32*      );
extern EPS_ERR_CODE usbResetPrinter     (void                                           );

extern EPS_ERR_CODE usbGetStatus        (EPS_STATUS_INFO*, EPS_BOOL*, EPS_BOOL*         );
extern EPS_ERR_CODE usbGetJobStatus     (EPS_STATUS_INFO*							    );
extern EPS_ERR_CODE usbGetInfo          (const EPS_PRINTER_INN*, EPS_INT32,
                                         EPS_UINT8**, EPS_INT32*                        );

extern EPS_ERR_CODE usbMechCommand      (EPS_INT32                                      );
extern EPS_ERR_CODE usbInfoCommand		(const EPS_PRINTER_INN*, EPS_INT32, EPS_UINT8**, EPS_INT32* );

#ifdef    __cplusplus
}
#endif    /* def __cplusplus    */

#endif    /* def _EPSON_USB_H_ */

/*___________________________________  epson-usb.h  ____________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
