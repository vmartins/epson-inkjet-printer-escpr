/*________________________________  epson-protocol.h   _________________________________*/

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
/*                        Epson Protocol layer Module Definitions                       */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef _EPSON_PROROCOL_H_
#define _EPSON_PROROCOL_H_
#ifdef    __cplusplus
extern "C" {
#endif    /* def __cplusplus    */

/*------------------------------- Local Compiler Switch --------------------------------*/
/*******************************************|********************************************/
/* #define GCOMSW_PRT_USE_USB		1*/		/* Use USB Communication */
/* #define GCOMSW_PRT_USE_LPR		1*/		/* Use LPR Communication */
/* #define GCOMSW_PRT_USE_RAW		1*/		/* Use RawPort Communication */

#if defined(GCOMSW_PRT_USE_LPR) || defined(GCOMSW_PRT_USE_RAW)
#define GCOMSW_PRT_USE_NETWORK
#endif

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"
#include "epson-escpr-pvt.h"

/*------------------------------------- Data Types -------------------------------------*/
/*******************************************|********************************************/

/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/
#define EPS_USB_NAME                "USB"

#define EPSNET_UDP_BROADCAST_ADDR	"255.255.255.255"
#define EPSNET_UPNP_MULTICAST_ADDR	"239.255.255.250"
#define EPSNET_NUM_DISCOVERIES		(1)			/* times to send discovery msg	        */
#define EPSNET_RECV_TIMEOUT			(5000)		/* default receive tiomeout				*/
#define EPSNET_FIND_RECV_TIMEOUT	(100)		/* discover receive tiomeout			*/
#define EPSNET_STAT_RECV_TIMEOUT	(1000)		/* status receive tiomeout			    */
#define EPSNET_SEND_TIMEOUT			(500)		/* default send tiomeout                */
#define EPSNET_BAND_SEND_TIMEOUT	(100)		/* band data send tiomeout	            */
#define EPSNET_BAND_SEND_TIMEOUTL	(3*60*1000)	/* band data send tiomeout	            */
#define EPSNET_FIND_REREQUEST_TIME	(2000)		/* request re issue                     */
#define EPSNET_MAX_PATH				(255)		/* path buffer length           	    */
#define EPSNET_STAT_RETRY		    (5)			/* getstatus retry count			    */

#define EPS_STAT_RETRY			    (5)			/* getstatus retry count			    */


    /*** PrinterID string format                                                        */
    /*** -------------------------------------------------------------------------------*/
#define EPS_PID_PRT         "P"
#define EPS_PID_DEF         "D"
#define EPS_PID_SEP         "#"
#define EPS_PID_SEGNUM		(4)
#define EPS_PRINTER_ID_STR  EPS_PID_PRT EPS_PID_SEP "%04X" EPS_PID_SEP EPS_PID_DEF EPS_PID_SEP "%s"
	                        /* P#xx#D#sssssssss */

	/* USB ID String 'D' parameter      */
#define EPS_USBID_SEP       "."
#define EPS_USBID_SEGNUM	(3)
#define EPS_USB_IDPRM_STR	"%04X" EPS_USBID_SEP "%04X" EPS_USBID_SEP "%s"	
	                        /* [VID].[PID].[ModelName] */

	/* Net ID String 'D' parameter      */
#define EPS_NETID_SEP       "?"
#define EPS_NETID_SEGNUM	(2)
#define EPS_NET_IDPRM_STR	"%s" EPS_NETID_SEP "%s"	
	                        /* [ModelName]?[uuid] */

/*-----------------------------------  Error Code  -------------------------------------*/
/*******************************************|********************************************/
#define EPS_COM_TINEOUT             (-2)        /* recv time out                        */
#define EPS_COM_READ_MORE           (-3)		/* need more receive                    */
#define EPS_COM_ERR_REPLY           (-4)		/* error reply                          */
#define EPS_COM_NEXT_RECORD         (-5)		/* mib next record received             */
#define EPS_COM_NOT_RECEIVE         (-6)		/* not recieve                          */

/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/

/* Print Job functions                                                                  */
typedef EPS_ERR_CODE	(*PRT_StartJob	    )(void                                      );
typedef EPS_ERR_CODE	(*PRT_EndJob	    )(void                                      );
typedef EPS_ERR_CODE	(*PRT_RestartJob    )(void                                      );
typedef EPS_ERR_CODE	(*PRT_WritePrintData)(const EPS_UINT8*, EPS_UINT32, EPS_UINT32* );
typedef EPS_ERR_CODE	(*PRT_ResetPrinter  )(void                                      );
typedef EPS_ERR_CODE	(*PRT_StartPage     )(void                                      );
typedef EPS_ERR_CODE	(*PRT_EndPage       )(void                                      );
typedef EPS_ERR_CODE	(*PRT_MonitorStatus )(EPS_STATUS_INFO*                          );
typedef EPS_ERR_CODE	(*PRT_GetStatus		)(EPS_STATUS_INFO*, EPS_BOOL*, EPS_BOOL*    );

typedef struct tagEPS_JOB_FUNCS {
	PRT_StartJob        StartJob;
	PRT_EndJob          EndJob;
	PRT_RestartJob      RestartJob;
	PRT_WritePrintData  WriteData;
	PRT_ResetPrinter    ResetPrinter;
	PRT_StartPage       StartPage;
	PRT_EndPage         EndPage;
	PRT_MonitorStatus   MonitorStatus;
	PRT_GetStatus		GetStatus;
}EPS_JOB_FUNCS;

#define	PRT_INVOKE_JOBFNC( f, prm )	((NULL != f)?f prm:EPS_ERR_NONE)


/*--------------------------  Public Function Declarations   ---------------------------*/
/*******************************************|********************************************/
    /*** Initialize                                                                     */
    /*** -------------------------------------------------------------------------------*/
extern EPS_ERR_CODE prtFunctionCheck        (EPS_INT32, const EPS_USB_FUNC*, 
											 const EPS_NET_FUNC*, const EPS_CMN_FUNC*   );

    /*** Find                                                                           */
    /*** -------------------------------------------------------------------------------*/
extern EPS_ERR_CODE prtFindPrinter          (EPS_INT32, EPS_UINT32                      );
extern EPS_ERR_CODE prtCancelFindPrinter    (void                                       );
extern EPS_ERR_CODE prtProbePrinterByID     (const EPS_PROBE*                           );
extern EPS_ERR_CODE prtProbePrinterByAddr   (const EPS_PROBE*                           );

    /*** Job                                                                            */
    /*** -------------------------------------------------------------------------------*/
extern EPS_ERR_CODE prtSetupJobFunctions    (const EPS_PRINTER_INN*, EPS_JOB_FUNCS*     );
extern EPS_ERR_CODE prtRecoverPE			(void                                       );

    /*** Information                                                                    */
    /*** -------------------------------------------------------------------------------*/
extern EPS_ERR_CODE prtGetInfo              (const EPS_PRINTER_INN*, EPS_INT32, 
											 EPS_UINT8**, EPS_INT32*                    );

    /*** Printer list                                                                   */
    /*** -------------------------------------------------------------------------------*/
extern EPS_ERR_CODE prtRegPrinter	        (EPS_PRINTER_INN*, EPS_BOOL                 );
extern EPS_BOOL     prtIsRegistered         (const EPS_INT8*, const EPS_INT8*, EPS_INT32);
extern void         prtClearPrinterList     (void                                       );
extern EPS_ERR_CODE prtDuplPrinterList      (void                                       );
extern EPS_ERR_CODE prtAddUsrPrinter		(const EPS_PRINTER*, EPS_PRINTER_INN**		);
extern EPS_PRINTER_INN* prtGetInnerPrinter  (const EPS_PRINTER*                         );
extern void         prtClearPrinterAttribute(EPS_PRINTER_INN*                           );

extern void         prtSetIdStr             (EPS_PRINTER_INN*, const EPS_INT8*          );

#ifdef    __cplusplus
}
#endif    /* def __cplusplus */

#endif    /* def _EPSON_PROROCOL_H_ */

/*________________________________  epson-protocol.h   _________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
