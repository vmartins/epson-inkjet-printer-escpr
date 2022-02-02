/*___________________________________  epson-usb.c   ___________________________________*/

/*       1         2         3         4         5         6         7         8        */
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*******************************************|********************************************/
/*
 *   Copyright (c) 2009  Seiko Epson Corporation   All rights reserved.
 *
 *   Copyright protection claimed includes all forms and matters of copyrightable
 *   material and information now allowed by statutory or judicial law or hereinafter
 *   granted, including without limitation, material generated from the software
 *   programs which are displayed on the screen such as icons, screen display looks,
 *   etc.
 */
/*******************************************|********************************************/
/*                                                                                      */
/*                                   Epson USB Module                                   */
/*                                                                                      */
/*                                Public Function Calls                                 */
/*                              --------------------------                              */
/*		EPS_ERR_CODE usbFind            (protocol                                    );	*/
/*	    EPS_ERR_CODE usbProbePrinterByID(modelNameTgt, protocol, printer             );	*/
/*	    EPS_ERR_CODE usbStartJob		(printer                                     );	*/
/*	    EPS_ERR_CODE usbStartPage		(                                            );	*/
/*	    EPS_ERR_CODE usbEndJob          (printer                                     );	*/
/*	    EPS_ERR_CODE usbGetInfo         (printer, pString, bufSize	                 );	*/
/*      EPS_ERR_CODE usbGetStatus       (printer, status, ioStatus                   );	*/
/*      EPS_ERR_CODE usbGetJobStatus    (pstInfo							         );	*/
/*	    EPS_ERR_CODE usbWritePrintData  (Buffer, BuffLen, sentSize                   );	*/
/*	    EPS_ERR_CODE usbResetPrinter    (void                                        );	*/
/*	    EPS_ERR_CODE usbMechCommand     (Command                                     );	*/
/*                                                                                      */
/*******************************************|********************************************/

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-pvt.h"
#include "epson-escpr-err.h"
#include "epson-escpr-services.h"
#include "epson-escpr-mem.h"
#include "epson-protocol.h"
#include "epson-cbt.h"
#include "epson-usb.h"

/*--------------------------------  Local Definition   ---------------------------------*/
/*******************************************|********************************************/
#ifdef EPS_LOG_MODULE_USB
#define EPS_LOG_MODULE	EPS_LOG_MODULE_USB
#else
#define EPS_LOG_MODULE	0
#endif

/* USB protocol */
#define EPS_USBP_UNK		(0)		/* unknown */
#define EPS_USBP_CBT		(1)
#define EPS_USBP_ND4		(2)
#define EPS_USBP_UNI		(3)

#define EPS_IS_CBT			(EPS_USBP_CBT == ((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->usbProtocol)

/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/
typedef struct _tagEPS_USB_PRINTER_INFO_ {
	EPS_USB_DEVICE	dev;
	EPS_BOOL		bCheckDataChannel;
	EPS_UINT32		usbProtocol;
}EPS_USB_PRINTER_INFO;

typedef struct _tagEPS_PRINT_JOB_USB_ {
	EPS_FILEDSC	fd;							/* usb file descriptor                      */
	EPS_BOOL    resetRequest;				/* recv reset request & yet not reset       */
} EPS_PRINT_JOB_USB;


/*--------------------------  ESC/P-R USB Lib Global Variables  ------------------------*/
/*******************************************|********************************************/

	/*** Extern Function                                                                */
	/*** -------------------------------------------------------------------------------*/
extern EPS_USB_FUNC    epsUsbFnc;
extern EPS_CMN_FUNC    epsCmnFnc;

    /*** Print Job Structure                                                            */
    /*** -------------------------------------------------------------------------------*/
extern EPS_PRINT_JOB   printJob;

	/*** Find                                                                           */
	/*** -------------------------------------------------------------------------------*/
extern EPS_BOOL     g_FindBreak;							/* Find printer end flag    */

    /*** I/O Channel                                                                    */
    /*** -------------------------------------------------------------------------------*/
extern EPS_BOOL    ioOpenState;             /* Open state of I/O port (Bi-Directional)  */
extern EPS_BOOL    ioDataChState;           /* Open state of Data Channel               */
extern EPS_BOOL    ioControlChState;        /* Open state of Control Channel            */
extern EPS_BOOL    ioOpenUniDirect;         /* Open state of I/O port (Uni-Directional) */

/*--------------------------------  Local Functions   ----------------------------------*/
/*******************************************|********************************************/
static EPS_ERR_CODE ProbePrinterByName	(const EPS_INT8*, EPS_BOOL, EPS_UINT32,
										 EPS_USB_PRINTER_INFO*, EPS_INT8*, EPS_INT8*,
										 EPS_INT32*, EPS_UINT32*                        );
static EPS_ERR_CODE ProbeESCPR			(EPS_USB_PRINTER_INFO*, EPS_INT8*, EPS_INT8*, 
										 EPS_INT32*, EPS_UINT32*                        );
static EPS_ERR_CODE CreatePrinterInfo	(EPS_USB_PRINTER_INFO*, const EPS_INT8*, 
										 const EPS_INT8*, const EPS_INT8*, EPS_INT32, 
										 EPS_UINT32, EPS_PRINTER_INN**                  );
static EPS_ERR_CODE PortResolution		(const EPS_PRINTER_INN*, EPS_FILEDSC*           );
static EPS_ERR_CODE GetBinaryStatus		(EPS_FILEDSC, EPS_UINT32, EPS_UINT32, EPS_STATUS_INFO*);
static EPS_ERR_CODE InfoCommand			(EPS_FILEDSC, EPS_UINT32, EPS_INT32, EPS_UINT8**, EPS_INT32*);
static EPS_ERR_CODE CbtInfoCommand		(EPS_FILEDSC, EPS_INT32, EPS_UINT8**, EPS_INT32*);
static EPS_ERR_CODE RmtInfoCommand		(EPS_FILEDSC, EPS_INT32, EPS_UINT8**, EPS_INT32*);

static EPS_ERR_CODE nd4WritePortal      (EPS_FILEDSC fd, const EPS_UINT8* buff, EPS_INT32* buffSize);
static EPS_ERR_CODE nd4ReadPortal       (EPS_FILEDSC fd, EPS_UINT8* buff, EPS_INT32* buffSize);
static EPS_ERR_CODE OpenPortal          (EPS_USB_PRINTER_INFO *usbInfo, EPS_FILEDSC	*fd );
static EPS_ERR_CODE	ClosePortal         (EPS_UINT32	usbProtocol, EPS_FILEDSC fd         );
static void         ParseUsbProtocol    (EPS_INT8* deviceIDString, EPS_INT32 strlength, 
	                                     EPS_UINT32* usbProtocol                        );
static EPS_ERR_CODE GetSerialNumber     (EPS_USB_PRINTER_INFO*, EPS_INT8* serialNo      );


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------              Public Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     usbFind()	        	                                            */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* timeout      EPS_UINT32          I: find timeout                                     */
/* protocol	    EPS_INT32           I: communication mode                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                - Success Printer found                             */
/*      EPS_ERR_PRINTER_NOT_FOUND	- Printer not found (or error occur)		        */
/*      EPS_ERR_MEMORY_ALLOCATION   - memory allocation failed                          */
/*      EPS_ERR_NOT_OPEN_IO         - Cannot Open I/O Portal                            */
/*                                                                                      */
/* Description:                                                                         */
/*      find all USB printer.                                                           */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE usbFind (

        EPS_UINT32*         timeout,
		EPS_INT32           protocol
		
){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE		ret = EPS_ERR_NONE;			 /* Return status of internal calls */
	EPS_FILEDSC			fd = EPS_INVALID_FILEDSC;
	EPS_PRINTER_INN*	printer = NULL;
	EPS_USB_PRINTER_INFO usbInfo;
	EPS_INT8			manufacturer[EPS_NAME_BUFFSIZE];    /* Manufacturer name        */
	EPS_INT8			modelName[EPS_NAME_BUFFSIZE];       /* Printer model name       */
	EPS_INT8			serialNo[EPS_ADDR_BUFFSIZE];        /* Printer serial number    */
	EPS_UINT32			tmStart, tmNow, tmSpan;
    EPS_INT32           cmdLevel = 0;
	EPS_UINT32			egID = 0;

	EPS_LOG_FUNCIN;

	memset(&usbInfo, 0, sizeof(usbInfo));
	memset(manufacturer, 0, sizeof(manufacturer));
	memset(modelName, 0, sizeof(modelName));
	memset(serialNo, 0, sizeof(serialNo));

/***------------------------------------------------------------------------------------*/
/*** Bi-Directional Communication Mode                                                  */
/***------------------------------------------------------------------------------------*/
    if ( EPS_IS_BI_PROTOCOL(protocol) ) {
		fd = epsUsbFnc.findFirst( &usbInfo.dev );
		if(EPS_INVALID_FILEDSC != fd){
			if(epsCmnFnc.getTime){
				tmStart = epsCmnFnc.getTime();
			} else{
				*timeout = tmStart = tmNow = tmSpan = 0;
			}

			do{
			/*** Validate this is an Epson ESC/PR Printer                                   */
				ret = ProbeESCPR(&usbInfo, manufacturer, modelName, &cmdLevel, &egID);

				if(EPS_ERR_NONE == ret){
					/*retStatus = */GetSerialNumber(&usbInfo, serialNo);
					ret = CreatePrinterInfo(&usbInfo, manufacturer, modelName, serialNo, 
											cmdLevel, egID, &printer);
					if( EPS_ERR_NONE == ret ){
						ret = prtRegPrinter( printer, TRUE );
					} 

					if( EPS_ERR_NONE != ret ){
						break;
					}

					if(*timeout > 0){
						tmNow = epsCmnFnc.getTime();
						tmSpan = (EPS_UINT32)(tmNow - tmStart);
						/*EPS_DBGPRINT( ("TM %u - %u <> %u\n", tmNow, tmStart, tmSpan) )*/
						if( tmSpan >= *timeout ){
							break;
						}
					}
					if( epsCmnFnc.lockSync && epsCmnFnc.unlockSync ){
						if( 0 == epsCmnFnc.lockSync() ){
							if( g_FindBreak ){
								epsCmnFnc.unlockSync();
								break;
							}
							epsCmnFnc.unlockSync();
						}
					}
				}
			}while( epsUsbFnc.findNext( fd, &usbInfo.dev ) );

			epsUsbFnc.findClose(fd);

			if(*timeout > 0){
				/* calculate elapsed time */
				tmNow = epsCmnFnc.getTime();
				tmSpan = (EPS_UINT32)(tmNow - tmStart);
				if(tmSpan < *timeout){
					*timeout -= tmSpan;
				} else{
					*timeout = 1;
				}
			}

		} else{
			ret = EPS_ERR_PRINTER_NOT_FOUND;
		}

/***------------------------------------------------------------------------------------*/
/*** Uni-Directional Communication Mode                                                 */
/***------------------------------------------------------------------------------------*/
	} else{
    /*** Open Portal check                                                              */
        fd = epsUsbFnc.openPortal(NULL);
		if(EPS_INVALID_FILEDSC != fd){
			epsUsbFnc.closePortal(fd);

			/*** Create printer info structure                                          */
			usbInfo.usbProtocol = EPS_USBP_UNI;
			ret = CreatePrinterInfo(&usbInfo, "", "", "", 0, 0, &printer);
			if( EPS_ERR_NONE == ret ){
				ret = prtRegPrinter( printer, TRUE );
			} 
		} else{
            ret = EPS_ERR_PRINTER_NOT_FOUND;
        }
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   usbProbePrinterByID()                                               */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printerUUID EPS_INT8*            I: ID String of probe target                        */
/* timeout      EPS_UINT32          I: find timeout                                     */
/* printer		EPS_PRINTER_INN**   O: pointer for found printer structure              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success (printer found)                       */
/*      EPS_ERR_PRINTER_NOT_FOUND       - printer not found                             */
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_NOT_OPEN_IO             - Cannot Open I/O Portal                        */
/*                                                                                      */
/* Description:                                                                         */
/*     looks for specified printer.                                                     */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE   usbProbePrinterByID (

		EPS_INT8*			printerUUID,
        EPS_UINT32			timeout,
		EPS_PRINTER_INN**   printer

){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE	ret = EPS_ERR_NONE;				 /* Return status of internal calls */
	EPS_FILEDSC		fd = EPS_INVALID_FILEDSC;
	EPS_USB_PRINTER_INFO usbInfo;
	EPS_INT8		manufacturer[EPS_NAME_BUFFSIZE];    /* Manufacturer name            */
	EPS_INT8		modelName[EPS_NAME_BUFFSIZE];       /* Printer model name           */
	EPS_INT8		serialNo[EPS_ADDR_BUFFSIZE];        /* Printer serial number        */
    EPS_INT8*       pPos = NULL;
    EPS_INT32       nSegCnt = 0;
    EPS_UINT32      nTmp = 0;
	EPS_INT8		modelNameTgt[EPS_NAME_BUFFSIZE];    /* target printer model name    */
	EPS_BOOL		enableBreak = FALSE;
	EPS_INT32       cmdLevel = 0;
	EPS_UINT32      egID = 0;

	EPS_LOG_FUNCIN;

/*** Parse definition String                                                        */
	pPos = strtok(printerUUID, EPS_USBID_SEP);
    for(nSegCnt = 0; pPos != NULL && nSegCnt < EPS_USBID_SEGNUM; nSegCnt++){
		switch(nSegCnt){
		case 0:			/* Get VID */
		case 1:			/* Get PID */
			sscanf(pPos, "%x", &nTmp);
			if(nTmp == 0){
				EPS_RETURN( EPS_ERR_INV_ARG_PRINTER_ID );
			}
			/* lib do not use VID, PID */
			break;

		case 2:			/* Get model name */
			strcpy(modelNameTgt, pPos);
			break;
		}

		pPos = strtok(NULL, EPS_USBID_SEP);
    }
	if(nSegCnt < EPS_USBID_SEGNUM){
		EPS_RETURN( EPS_ERR_INV_ARG_PRINTER_ID );
	}

	memset(&usbInfo, 0, sizeof(usbInfo));

/***------------------------------------------------------------------------------------*/
/*** Bi-Directional Communication Mode                                                  */
/***------------------------------------------------------------------------------------*/
    if ( EPS_IS_BI_PROTOCOL(printJob.commMode) ) {
		enableBreak = (epsCmnFnc.lockSync && epsCmnFnc.unlockSync );
		memset(manufacturer, 0, sizeof(manufacturer));
		memset(modelName, 0, sizeof(modelName));
		memset(serialNo, 0, sizeof(serialNo));
		ret = ProbePrinterByName(modelNameTgt, enableBreak, timeout, 
			                     &usbInfo, manufacturer, modelName, &cmdLevel, &egID);
		if(EPS_ERR_NONE == ret){
			/*retStatus = */GetSerialNumber(&usbInfo, serialNo);

			/*** Create printer info structure                                          */
			ret = CreatePrinterInfo(&usbInfo, manufacturer, modelName, serialNo, 
									cmdLevel, egID, printer);
		}
/***------------------------------------------------------------------------------------*/
/*** Uni-Directional Communication Mode                                                 */
/***------------------------------------------------------------------------------------*/
    } else{
    /*** Open Portal check                                                              */
        fd = epsUsbFnc.openPortal(NULL);
		if(EPS_INVALID_FILEDSC != fd){
			epsUsbFnc.closePortal(fd);

			/*** Create printer info structure                                          */
			usbInfo.usbProtocol = EPS_USBP_UNI;
			ret = CreatePrinterInfo(&usbInfo, "", "", "", 0, 0, printer);

		} else{
            ret = EPS_ERR_PRINTER_NOT_FOUND;
        }
    }

/*** Return to Caller                                                                   */
    EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     usbStartJob()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* printer          EPS_PRINTER_INN*        I: Pointer to a PrinterInfo                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      << Error >>                                                                     */
/*      (Uni/Bi-Directional)                                                            */
/*      EPS_ERR_LIB_NOT_INITIALIZED     - ESC/P-R Lib is NOT initialized                */
/*      EPS_ERR_NOT_OPEN_IO             - Failed to open I/O                            */
/*      (Bi-Directional Only)                                                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Creates a new print job.                                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    usbStartJob (

        void

){
/*** Declare Variable Local to Routine                                                  */
	EPS_ERR_CODE		retStatus = EPS_ERR_NONE;   /* Return status of internal calls  */
	const EPS_PRINTER_INN* printer = printJob.printer;
	EPS_PRINT_JOB_USB	*usbPrintJob = NULL;

	EPS_LOG_FUNCIN;

/*** Create USB Job data                                                                */
	usbPrintJob = (EPS_PRINT_JOB_USB*)EPS_ALLOC( sizeof(EPS_PRINT_JOB_USB) );
	if( NULL == usbPrintJob ){
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
	}
	memset(usbPrintJob, 0, sizeof(EPS_PRINT_JOB_USB));
	usbPrintJob->fd = EPS_INVALID_FILEDSC;
	usbPrintJob->resetRequest = FALSE;

/***------------------------------------------------------------------------------------*/
/*** Bi-Directional Communication Mode                                                  */
/***------------------------------------------------------------------------------------*/
	if( EPS_IS_BI_PROTOCOL(printer->protocol) ){
	/*** Resolve present port number. And Open the I/O Port for communication           */
		retStatus = PortResolution(printer, &usbPrintJob->fd);
        if (retStatus != EPS_ERR_NONE) {
 			cbtCommClose(usbPrintJob->fd);
			retStatus = (EPS_ERR_CODE)EPS_ERR_NOT_OPEN_IO;
            goto epsStartJob_END;
        }

		if( EPS_IS_CBT ){
		/*** Open the command channel                                                   */
			retStatus = cbtCommChannelOpen(usbPrintJob->fd, EPS_CBTCHANNEL_CTRL, TRUE);
			if (retStatus != EPS_ERR_NONE) {
				retStatus = cbtCommClose(usbPrintJob->fd);
				retStatus = (EPS_ERR_CODE)EPS_ERR_NOT_OPEN_IO;
				goto epsStartJob_END;
			}

		/*** Open Data Channel                                                          */
			retStatus = cbtCommChannelOpen(usbPrintJob->fd, EPS_CBTCHANNEL_DATA, TRUE);
			if (retStatus != EPS_ERR_NONE) {
				retStatus = cbtCommChannelClose(usbPrintJob->fd, EPS_CBTCHANNEL_CTRL);
				retStatus = cbtCommClose(usbPrintJob->fd);
				retStatus = (EPS_ERR_CODE)EPS_ERR_PRINTER_ERR_OCCUR/*EPS_ERR_CANNOT_PRINT*/;
				goto epsStartJob_END;
			}
		}
		((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->bCheckDataChannel = FALSE;

/***------------------------------------------------------------------------------------*/
/*** Uni-Directional Communication Mode                                                 */
/***------------------------------------------------------------------------------------*/
    } else{
    /*** Open Portal                                                                    */
        if (ioOpenUniDirect == EPS_IO_OPEN) {
            retStatus = EPS_ERR_2ND_OPEN_IO;
			goto epsStartJob_END;
        } else {
            usbPrintJob->fd = epsUsbFnc.openPortal( NULL );
            if (EPS_INVALID_FILEDSC == usbPrintJob->fd) {
                retStatus = EPS_ERR_NOT_OPEN_IO;
				goto epsStartJob_END;
            }
            ioOpenUniDirect = EPS_IO_OPEN;
        }
    }

	printJob.hProtInfo = (EPS_HANDLE)usbPrintJob;

epsStartJob_END:
	if(EPS_ERR_NONE != retStatus){
		EPS_SAFE_RELEASE(usbPrintJob);
	}

/*** Return to Caller                                                                   */
    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     usbRestartJob()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* N/A                                                                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      << Error >>                                                                     */
/*      (Uni/Bi-Directional)                                                            */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized						*/
/*      EPS_ERR_NOT_OPEN_IO             - Failed to open I/O                            */
/*      (Bi-Directional Only)                                                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      The port shut by usbResetPrinter() is opened again.                             */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    usbRestartJob (

        void

){

/*** Declare Variable Local to Routine                                                  */
	EPS_ERR_CODE		retStatus = EPS_ERR_NONE;   /* Return status of internal calls  */
	const EPS_PRINTER_INN* printer = printJob.printer;
	EPS_PRINT_JOB_USB* usbPrintJob = (EPS_PRINT_JOB_USB*)printJob.hProtInfo;

	EPS_LOG_FUNCIN;

 	if( !usbPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED );
	}

/***------------------------------------------------------------------------------------*/
/*** Bi-Directional(CBT) Communication Mode                                             */
/***------------------------------------------------------------------------------------*/
	if( EPS_IS_BI_PROTOCOL(printer->protocol) && EPS_IS_CBT){
		if(EPS_IO_NOT_OPEN == ioDataChState){
		/*if( EPS_RESET_SENT == printJob.resetSent ){*/
		/* If pre page canceled, Re open data channel */
			retStatus = cbtCommChannelOpen(usbPrintJob->fd, EPS_CBTCHANNEL_DATA, TRUE);
			if (retStatus == EPS_ERR_NONE) {
				((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->bCheckDataChannel = FALSE;
			} else{
				retStatus = (EPS_ERR_CODE)EPS_ERR_PRINTER_ERR_OCCUR/*EPS_ERR_CANNOT_PRINT*/;
			}
		}
/***------------------------------------------------------------------------------------*/
/*** END4 or Uni-Directional Communication Mode                                         */
/***------------------------------------------------------------------------------------*/
/*   } else{ 
		no operation*/
    }

/*** Return to Caller                                                                   */
    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   usbEndJob()                                                         */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* N/A                                                                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      << Error >>                                                                     */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized                        */
/*      EPS_ERR_NOT_CLOSE_IO            - Cannot close I/O portal                       */
/*                                                                                      */
/* Description:                                                                         */
/*      Ends the current print job.                                                     */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    usbEndJob (

        void

){
    EPS_ERR_CODE    retStatus = EPS_ERR_NONE;       /* Return status of internal calls  */
	const EPS_PRINTER_INN* printer = printJob.printer;
	EPS_PRINT_JOB_USB* usbPrintJob = (EPS_PRINT_JOB_USB*)printJob.hProtInfo;

EPS_LOG_FUNCIN;

	if( !usbPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED );
	}

/***------------------------------------------------------------------------------------*/
/*** Bi-Directional(CBT) Communication Mode                                             */
/***------------------------------------------------------------------------------------*/
	if( EPS_IS_BI_PROTOCOL(printer->protocol) ){
		if( EPS_IS_CBT ){
		/*** Close communication                                                            */
			retStatus = cbtCommChannelClose(usbPrintJob->fd, EPS_CBTCHANNEL_DATA);
			retStatus = cbtCommChannelClose(usbPrintJob->fd, EPS_CBTCHANNEL_CTRL);
			retStatus = cbtCommClose(usbPrintJob->fd);
		} else{
			retStatus = ClosePortal( ((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->usbProtocol, usbPrintJob->fd );
		}
		if (retStatus != EPS_ERR_NONE) {
			retStatus = EPS_ERR_NOT_CLOSE_IO;
		}
/***------------------------------------------------------------------------------------*/
/*** END4 or Uni-Directional Communication Mode                                         */
/***------------------------------------------------------------------------------------*/
	} else{
		/*** Close Portal                                                               */
		if (ioOpenUniDirect == EPS_IO_OPEN) {
			retStatus = epsUsbFnc.closePortal(usbPrintJob->fd);
			if (retStatus != 0) {
				EPS_RETURN( EPS_ERR_NOT_CLOSE_IO );
			}
			ioOpenUniDirect = EPS_IO_NOT_OPEN;
		}
    }

	EPS_SAFE_RELEASE( printJob.hProtInfo );

	EPS_RETURN( retStatus );
}



/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   usbWritePrintData()                                                 */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* Buffer       const EPS_UINT8*    I: Buffer Pointer for Write Data                    */
/* BuffLen      EPS_UINT32          I: Write Data Buffer Length (bytes)                 */
/* sentSize     EPS_UINT32*         O: Actuall Write Size                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized						*/
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Write the data to printer Data Channel.                                         */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE   usbWritePrintData (

		const EPS_UINT8*    Buffer,
        EPS_UINT32          BuffLen,
        EPS_UINT32*         sentSize

){
    EPS_INT32		   retCom;
	const EPS_PRINTER_INN* printer = printJob.printer;
	EPS_PRINT_JOB_USB* usbPrintJob = (EPS_PRINT_JOB_USB*)printJob.hProtInfo;

EPS_LOG_FUNCIN;

	if( !usbPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED );
	}

/***------------------------------------------------------------------------------------*/
/*** Bi-Directional(CBT) Communication Mode                                             */
/***------------------------------------------------------------------------------------*/
	if( EPS_IS_BI_PROTOCOL(printJob.printer->protocol) && EPS_IS_CBT ){
		retCom = cbtCommWriteData(usbPrintJob->fd, EPS_CBTCHANNEL_DATA, Buffer, (EPS_INT32)BuffLen, (EPS_INT32*)sentSize);
		if(EPCBT_ERR_NONE == retCom){
			EPS_RETURN( EPS_ERR_NONE );
		} else if(EPSCBT_ERR_FNCDISABLE == retCom){
			EPS_RETURN( EPS_COM_TINEOUT );
		}else{
			EPS_RETURN( EPS_ERR_COMM_ERROR );
		}

/***------------------------------------------------------------------------------------*/
/*** END4 or Uni-Directional Communication Mode                                         */
/***------------------------------------------------------------------------------------*/
	} else{
		retCom = epsUsbFnc.writePortal(usbPrintJob->fd, Buffer, (EPS_INT32)BuffLen, (EPS_INT32*)sentSize);
		if(0 == retCom){
			EPS_RETURN( EPS_ERR_NONE );
		} else{
			EPS_RETURN( EPS_ERR_COMM_ERROR );
		}
	}
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   usbResetPrinter()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* void         -                   -                                                   */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized						*/
/*      EPS_ERR_COMM_ERROR              - Failed to send command                        */
/*      EPS_ERR_CAN_NOT_RESET           - Failed to reset printer                       */
/*      EPS_ERR_NOT_CLOSE_IO            - Cannot Close I/O Portal                       */
/*                                                                                      */
/* Description:                                                                         */
/*      Send "rj" command and reset printer.                                            */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE   usbResetPrinter (

        void

){
/*** Declare Variable Local to Routine                                                  */
#define EPS_RSREPLY_SIZE	(32)
    EPS_ERR_CODE    Ret;
    EPS_INT32       ComSize = 0;
    EPS_INT32       retBufSize;                         /* Size of buffer written       */
    EPS_INT32       retryComm;
    EPS_INT32       retryReset;
    EPS_INT32       Size = EPS_RSREPLY_SIZE;
    EPS_INT32       lSize;
    EPS_UINT8       pResult[EPS_RSREPLY_SIZE];
	EPS_PRINT_JOB_USB* usbPrintJob = (EPS_PRINT_JOB_USB*)printJob.hProtInfo;
	const EPS_PRINTER_INN* printer = printJob.printer;
    
    EPS_UINT8 CBTcmd_rj[] = { 'r',   'j', 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         /* E   , S   , C   , P   , R   , L   , i   , b    */
                            0x45, 0x53, 0x43, 0x50, 0x52, 0x4c, 0x69, 0x62 };
#if 0  /* Not use "rs" command */
    EPS_UINT8 CBTcmd_rs[] = {'r', 's', 0x01, 0x00, 0x01};
#endif


EPS_LOG_FUNCIN;

	if( !EPS_IS_CBT ){
		Ret = epsUsbFnc.softReset(usbPrintJob->fd);
		if(0 == Ret){
			EPS_RETURN( EPS_ERR_NONE );
		} else{
			EPS_RETURN( EPS_ERR_COMM_ERROR );
		}
	}

/*** Initialize Local Variables                                                         */
    retBufSize = 0;
    memset(pResult, 0, 32);

	if( !usbPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED );
	}

/*** If we already successfully called this function once for a given print job,        */
    if( printJob.resetSent == EPS_RESET_SENT 
		|| FALSE == printJob.transmittable
		|| FALSE == printJob.sendJS ){
        EPS_RETURN( EPS_ERR_NONE );/*  no need to send an "rj" command again. Therefore,   */
                                   /*  return EPS_ERR_NONE and exit gracefully.            */
    }

	EPS_DBGPRINT((">>>> Send Printer Reset Command.\r\n"));
	serDelayThread(500, &epsCmnFnc);				/* wait printer 500ms */  

    retryReset = 5;  /* 5 times time out */
    do{
/*** Send "rj" command                                                                  */
        retryComm = 5;
        do{
            Ret = cbtCommWriteData(usbPrintJob->fd, EPS_CBTCHANNEL_CTRL, (const EPS_UINT8*)CBTcmd_rj, sizeof(CBTcmd_rj), &ComSize);
            
            if ( Ret != EPCBT_ERR_NONE){
                if (Ret != EPSCBT_ERR_FNCDISABLE){
                    EPS_DBGPRINT(("CBT Command Write Failed [%d]\r\n",Ret));
                    EPS_RETURN( EPS_ERR_COMM_ERROR );
                }
                retryComm--;
                if (!retryComm) {
                    EPS_DBGPRINT(("CBT Command Write Retry Failed \r\n"));
                    EPS_RETURN( EPS_ERR_COMM_ERROR );
                }        
                serDelayThread(1*_SECOND_, &epsCmnFnc);        /* retry command after 1 seconds*/
            }
        }while (Ret == EPSCBT_ERR_FNCDISABLE);
        
/*** Read "rj" Reply                                                                    */
        retryComm = 5;
        lSize = Size-1;
        do{
            if ((Ret = cbtCommReadData(usbPrintJob->fd, EPS_CBTCHANNEL_CTRL, pResult, lSize, &Size)) != EPCBT_ERR_NONE){
                if (Ret != EPSCBT_ERR_FNCDISABLE){
                    EPS_DBGPRINT(("CBT Command Write Reply Failed [%d]\r\n",Ret));
                    EPS_RETURN( EPS_ERR_COMM_ERROR );
                }    
            }else if (Size != 0){
                break;
            }

            retryComm--;
            if (!retryComm){
                EPS_DBGPRINT(("CBT Command Write Reply Retry Failed \r\n"));
                EPS_RETURN( EPS_ERR_COMM_ERROR );
            }
            serDelayThread(1*_SECOND_, &epsCmnFnc);        /* retry command after 1 seconds*/
        }while(retryComm > 0);
        
/*** Make sure we received a correct reply in response to the reset command we sent.    */
		pResult[EPS_RSREPLY_SIZE-1] = '\0';
		EPS_DUMP(pResult, Size);
		if (strstr((EPS_INT8*)pResult, "rj:OK;") == NULL) {
            retryReset--;
            if (!retryReset) {
                EPS_DBGPRINT(("Failed to <rj> reset \r\n"));
                EPS_RETURN( EPS_ERR_CAN_NOT_RESET );
            }
            serDelayThread(500, &epsCmnFnc);        /* retry command after 500m seconds*/
        } else{
            break;
        }

    }while(retryReset > 0);

	/*** Close Data Channel after sent "rj" command                                     */
	Ret = cbtCommChannelClose(usbPrintJob->fd, EPS_CBTCHANNEL_DATA);
    if (Ret != EPS_ERR_NONE) {
        EPS_RETURN( EPS_ERR_NOT_CLOSE_IO );
    }
	if(printJob.printer && printJob.printer->protocolInfo){
		((EPS_USB_PRINTER_INFO*)printJob.printer->protocolInfo)->bCheckDataChannel = TRUE;
	}

	usbPrintJob->resetRequest = FALSE;

/*** Return to Caller                                                                   */
    EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   usbGetInfo()                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* printer          EPS_PRINTER_INN*        I: Pointer to a PrinterInfo                 */
/* type 			EPS_INT32				I: PM kind 1 or 2                           */
/* pString			EPS_UINT8*				O: Pointer to PM String                     */
/* bufSize			EPS_INT32				I: pString buffer size                      */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized						*/
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_NOT_OPEN_IO             - Cannot Open I/O Portal                        */
/*      EPS_ERR_COMM_ERROR              - Info command execution error                  */
/*                                                                                      */
/* Description:                                                                         */
/*      Get PM string from usb printer.                                                 */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE     usbGetInfo (

		const EPS_PRINTER_INN*	printer,
		EPS_INT32               type,
		EPS_UINT8**             pString,
		EPS_INT32*              bufSize

){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE		ret = EPS_ERR_NONE;     /* Return status of internal calls      */
	EPS_FILEDSC			fd = EPS_INVALID_FILEDSC;
	EPS_PRINT_JOB_USB*	usbPrintJob = (EPS_PRINT_JOB_USB*)printJob.hProtInfo;
	EPS_BOOL            selfOpened = FALSE;

EPS_LOG_FUNCIN;

	if ( EPS_IO_NOT_OPEN == ioOpenState) {
	/*** Resolve present port number. And Open the I/O Port for communication           */
		ret = PortResolution(printer, &fd);
        if (ret != EPS_ERR_NONE) {
			ClosePortal(((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->usbProtocol, fd);
			EPS_RETURN( EPS_ERR_NOT_OPEN_IO );
		}

		if( EPS_IS_CBT ){
			/*** Open the control channel                                               */
			ret = cbtCommChannelOpen(fd, EPS_CBTCHANNEL_CTRL, TRUE);
			if (ret != EPS_ERR_NONE) {
				cbtCommChannelClose(fd, EPS_CBTCHANNEL_CTRL);
				ClosePortal(((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->usbProtocol, fd);
				EPS_RETURN( EPS_ERR_NOT_OPEN_IO );
			}
		}
		selfOpened = TRUE;

    } else{ /* Job running now */
 		if( !usbPrintJob ){
			EPS_RETURN( EPS_ERR_OPR_FAIL );
		}
		fd = usbPrintJob->fd;
	}

/*** Get PM from Printer                                                                */
	ret = InfoCommand(fd, ((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->usbProtocol,
						type, pString, bufSize);
	if( TRUE == selfOpened ){
		/* Close Port */
		if( EPS_IS_CBT ){
			cbtCommChannelClose(fd, EPS_CBTCHANNEL_CTRL);
		}
		ClosePortal(((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->usbProtocol, fd);
	}

/*** Return to Caller                                                                   */
    EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   usbGetStatus()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printer		EPS_PRINTER_INN*    I: Pointer to a PrinterInfo                         */
/* status       EPS_STATUS_INFO*    O: Pointer to the printer status.                   */
/* ioStatus		EPS_INT32*      	O: It is possible to communicate				    */
/* canceling	EPS_BOOL*      		O: Cancel processing            				    */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      << Error >>                                                                     */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Info command execution error                  */
/*      EPS_ERR_NOT_OPEN_IO             - Cannot Open I/O Portal                        */
/*      EPS_ERR_NOT_CLOSE_IO            - Cannot Close I/O Portal                       */
/*                                                                                      */
/* Description:                                                                         */
/*      Gets the printer status.                                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    usbGetStatus (

		EPS_STATUS_INFO*		pstInfo,
		EPS_BOOL*				pIoStatus,
		EPS_BOOL*				pCancelling

){

/*** Declare Variable Local to Routine                                                  */
	EPS_ERR_CODE		ret;                    /* Return status of internal calls      */
	EPS_FILEDSC			fd = EPS_INVALID_FILEDSC;
	EPS_PRINTER_INN*	printer = printJob.printer;
	EPS_PRINT_JOB_USB*	usbPrintJob = (EPS_PRINT_JOB_USB*)printJob.hProtInfo;
	EPS_BOOL			selfOpend = FALSE;

EPS_LOG_FUNCIN;

/*** Initialize Local Variables                                                         */
    ret           = EPS_ERR_NONE;
    memset(pstInfo, -1, sizeof(EPS_STATUS_INFO));

    if ( EPS_IO_NOT_OPEN == ioOpenState) {
		selfOpend = TRUE;

		/*** Resolve present port number. And Open the I/O Port for communication       */
		ret = PortResolution(printer, &fd);
        if (ret != EPS_ERR_NONE) {
            ret = EPS_ERR_NOT_OPEN_IO;
			goto usbGetStatus_END;
        }

		if( EPS_IS_CBT ){
            /*** Open the control channel                                               */
			ret = cbtCommChannelOpen(fd, EPS_CBTCHANNEL_CTRL, TRUE);
			if (ret != EPS_ERR_NONE) {
				ret = EPS_ERR_NOT_OPEN_IO;
				goto usbGetStatus_END;
			}
		}

    } else{ /* Job running now */
 		if( !usbPrintJob ){
			EPS_RETURN( EPS_ERR_OPR_FAIL );
		}
		fd = usbPrintJob->fd;
	}


    /*** Get Printer Status                                                         */
	ret = GetBinaryStatus(fd, ((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->usbProtocol,
							printer->egID, pstInfo);
    if (ret != EPS_ERR_NONE) {
		goto usbGetStatus_END;
    }

	if(NULL != pIoStatus && NULL != pCancelling){
		/*** Check I/O status                                                       */
		if( EPS_IS_CBT ){
			*pCancelling = FALSE;
			if(EPS_CAREQ_CANCEL == pstInfo->nCancel
				|| (usbPrintJob && TRUE == usbPrintJob->resetRequest)){
				*pIoStatus = FALSE;
				*pCancelling = TRUE;
				if(usbPrintJob){
					/* Because EPS_CAREQ_CANCEL can be acquired only once, it memorizes. */
					usbPrintJob->resetRequest = TRUE;
				}
			} else if(EPS_IO_OPEN == ioDataChState && FALSE == selfOpend){
				*pIoStatus = TRUE;	/* Evidence */
			} else if(EPS_ST_IDLE == pstInfo->nState){
				if( ((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->bCheckDataChannel ){
					if( EPS_IS_CBT ){
						/*** Open Data Channel                                              */
						if (cbtCommChannelOpen(fd, EPS_CBTCHANNEL_DATA, FALSE) == EPS_ERR_NONE) {
							((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->bCheckDataChannel = FALSE;
							*pIoStatus = TRUE;
						} else{
							*pIoStatus = FALSE;
							*pCancelling = TRUE;
						}
						cbtCommChannelClose(fd, EPS_CBTCHANNEL_DATA);
					}
				} else{
					*pIoStatus = TRUE;
				}
			} else{
				*pIoStatus = FALSE;
				if( ((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->bCheckDataChannel ){
					*pCancelling = TRUE;
				}
			}
		
		} else{
			*pIoStatus = TRUE;      /* Evidence */
			*pCancelling = FALSE;   /* cannot distinguish */
		}
	}

usbGetStatus_END:
	if(selfOpend){
		/* Close Port */
		if( EPS_IS_CBT ){
			ret = cbtCommChannelClose(fd, EPS_CBTCHANNEL_CTRL);
		}

		/*ret = */ClosePortal(((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->usbProtocol, fd);
	}

/*** Return to Caller                                                                   */
    EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   usbGetJobStatus()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pstInfo      EPS_STATUS_INFO*    O: Printer Status information                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized						*/
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Get the printer status and analyze the status string.                           */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    usbGetJobStatus (

        EPS_STATUS_INFO*    pstInfo

){
	EPS_ERR_CODE		ret = EPS_ERR_NONE;
 	EPS_PRINT_JOB_USB*	usbPrintJob = (EPS_PRINT_JOB_USB*)printJob.hProtInfo;

EPS_LOG_FUNCIN;

	if( !usbPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED );
	}
	
	ret = GetBinaryStatus(usbPrintJob->fd, 
			((EPS_USB_PRINTER_INFO*)printJob.printer->protocolInfo)->usbProtocol,
			printJob.printer->egID, pstInfo);
	if(EPS_ERR_NONE == ret
		&& EPS_CAREQ_CANCEL == pstInfo->nCancel){
		usbPrintJob->resetRequest = TRUE;
	}

	EPS_RETURN( ret ); 
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   usbMechCommand()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* Command      EPS_INT32           I: Command Code                                     */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Mech command executed successfully            */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized						*/
/*      EPS_ERR_COMM_ERROR              - Mech command execution error                  */
/*                                                                                      */
/* Description:                                                                         */
/*      Sends mechanincal commands to the printer.                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    usbMechCommand (

        EPS_INT32   Command
        
){
/*** Declare Variable Local to Routine                                                  */
    static EPS_UINT8 stCom[] = {'x', 'x', 0x01, 0x00, 0x01};
    EPS_INT8        Reply[32];
    EPS_INT32       Size = 32;
    EPS_INT32       Retry;
    EPS_INT32       ComSize = 0;
    EPS_INT32       Ret;
    EPS_UINT8*      Com;
    EPS_INT32       cSize;
	EPS_PRINT_JOB_USB* usbPrintJob = (EPS_PRINT_JOB_USB*)printJob.hProtInfo;

EPS_LOG_FUNCIN;

	if( !usbPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED );
	}

/*** Initialize Local Variables                                                         */
    memset(Reply, 0, 32);

/*** Select Command                                                                     */
    switch(Command){
    case EPS_CBTCOM_XIA:
        stCom[0] = 'x'; stCom[1] = 'i';
        break;
    case EPS_CBTCOM_XIA2:
        stCom[0] = 'x'; stCom[1] = 'i'; stCom[4] = 0x03;
        break;
    case EPS_CBTCOM_XIA3:
        stCom[0] = 'x'; stCom[1] = 'i'; stCom[4] = 0x04;
        break;
    case EPS_CBTCOM_XIB:
        stCom[0] = 'x'; stCom[1] = 'i';stCom[4] = 0x80;
        break;
    case EPS_CBTCOM_CH:
        stCom[0] = 'c'; stCom[1] = 'h';
        break;
    case EPS_CBTCOM_NC:
        stCom[0] = 'n'; stCom[1] = 'c'; stCom[4] = 0x00;
        break;
    case EPS_CBTCOM_EI:
        stCom[0] = 'e'; stCom[1] = 'i'; stCom[4] = 0x00;
        break;
    case EPS_CBTCOM_PE:
        stCom[0] = 'p'; stCom[1] = 'e';
        break;
    case EPS_CBTCOM_PJ:
        stCom[0] = 'p'; stCom[1] = 'j';
        break;
    }
    
/*** Send Command                                                                       */
    Retry = 5;
    do{
        Com   = stCom;
        cSize = 5;
        
        EPS_DBGPRINT(("EPS SER : Sending CHANNEL_COMMAND %c%c\r\n", Com[0], Com[1]));
		if ((Ret = cbtCommWriteData(usbPrintJob->fd, EPS_CBTCHANNEL_CTRL, Com, cSize, &ComSize)) != EPCBT_ERR_NONE){
            if (Ret != EPSCBT_ERR_FNCDISABLE){
                EPS_RETURN( EPS_ERR_COMM_ERROR );
            }
            Retry--;
            if (!Retry) {
                EPS_RETURN( EPS_ERR_COMM_ERROR );
            }        
            serDelayThread(2*_SECOND_, &epsCmnFnc);        /* retry command after 2 seconds */
        }
    }while (Ret == EPSCBT_ERR_FNCDISABLE);

/*** Read Reply                                                                         */
    Retry = 5;
    do{
        if ((Ret = cbtCommReadData(usbPrintJob->fd, EPS_CBTCHANNEL_CTRL, (EPS_UINT8 *)Reply, 32, &Size)) != EPCBT_ERR_NONE){
            if (Ret != EPSCBT_ERR_FNCDISABLE){
                EPS_DBGPRINT(("EPS SER : CBT Command Write Reply Failed [%d]\r\n",Ret));
                EPS_RETURN( EPS_ERR_COMM_ERROR );
            }    
        }else if (Size != 0){
            break;
        }

        Retry--;
        if (!Retry){
            EPS_DBGPRINT(("EPS SER : CBT Command Write Reply Retry Failed \r\n"));
            EPS_RETURN( EPS_ERR_COMM_ERROR );
        }
        serDelayThread(2*_SECOND_, &epsCmnFnc);        /* retry command after 2 seconds*/
    }while(Retry > 0);

/*** Check Reply                                                                        */
    EPS_DBGPRINT(("EPS SER : Mech Command reply -> %s\r\n", Reply));
    if (strstr(Reply,"OK") == NULL){
        EPS_RETURN( EPS_ERR_COMM_ERROR );
    }
    
/*** Return to Caller                                                                   */
    EPS_RETURN( EPS_ERR_NONE );
}


EPS_ERR_CODE   usbInfoCommand (

		const EPS_PRINTER_INN*	printer,
		EPS_INT32   command,
        EPS_UINT8** buffer,
        EPS_INT32*  bufSize
        
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
	EPS_FILEDSC		fd = EPS_INVALID_FILEDSC;

	ret = OpenPortal((EPS_USB_PRINTER_INFO*)printer->protocolInfo, &fd );
	if( EPS_ERR_NONE == ret ){
		if( EPS_IS_CBT ){
			ret = cbtCommChannelOpen(fd, EPS_CBTCHANNEL_CTRL, TRUE);
		}

		if( EPS_ERR_NONE == ret ){
			ret = InfoCommand(fd, ((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->usbProtocol,
							command, buffer, bufSize);
		
			if( EPS_IS_CBT ){
				cbtCommChannelClose(fd, EPS_CBTCHANNEL_CTRL);
			}
		}

		ClosePortal(((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->usbProtocol, fd);
	}

	return ret;
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------               Local Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   GetBinaryStatus()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd			EPS_FILEDSC		    I: file discripter                                  */
/* usbProtocol  EPS_UINT32          I: usb protocol kind                                */
/* egID         EPS_UINT32          I: printer error group id                           */
/* pstInfo      EPS_STATUS_INFO*    O: Printer Status information                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Get the printer status and analyze the status string.                           */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE    GetBinaryStatus (

		EPS_FILEDSC         fd,
        EPS_UINT32          usbProtocol,
		EPS_UINT32          egID, 
        EPS_STATUS_INFO*    pstInfo

){
    EPS_UINT8   Status[_STATUS_REPLY_BUF];
	EPS_INT32   Size = _STATUS_REPLY_BUF;
	EPS_INT32   ret = 0;
	EPS_UINT8*	pStatus = &Status[0];

EPS_LOG_FUNCIN;

	memset(Status, 0x00, _STATUS_REPLY_BUF);

	ret = InfoCommand(fd, usbProtocol, EPS_CBTCOM_ST, &pStatus, &Size);
	if( EPS_ERR_NONE != ret ){
		EPS_DBGPRINT(("EPS SER: Get Stat -> CBT Com Failed\r\n"));
		EPS_RETURN( EPS_ERR_COMM_ERROR );
	}

    EPS_RETURN( serAnalyzeStatus((EPS_INT8*)Status, egID, pstInfo) );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   ProbeESCPR()                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* dev			EPS_USB_DEVICE*     I: Pointer to a usb device infomatio                */
/* manufacturer EPS_INT8*           O: Pointer to a 64-byte buffer for the manufacturer */
/*                                     name.                                            */
/* modelName    EPS_INT8*           O: Pointer to a 64-byte buffer for the printer      */
/*                                     model name.                                      */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_NOT_OPEN_IO             - Cannot Open I/O Portal                        */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_NOT_FOUND       - Connected printer is not supported            */
/*                                                                                      */
/* Description:                                                                         */
/*      Opens the portal, gets the printer's DeviceString, parses it.                   */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE     ProbeESCPR (

		EPS_USB_PRINTER_INFO*	usbInfo,
        EPS_INT8*				manufacturer,
        EPS_INT8*				modelName,
		EPS_INT32*              cmdLevel,
		EPS_UINT32*				egID

){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE    retStatus = EPS_ERR_NONE;
	EPS_INT32		ret = 0;
	EPS_FILEDSC		fd = EPS_INVALID_FILEDSC;
    EPS_INT8*       tmpDeviceIdBuf = NULL;
	EPS_INT32		bufSizeOrg = 0;
	EPS_INT32		bufSize = EPS_DI_MAXSIZE-1;
    EPS_INT32       found_dev;

EPS_LOG_FUNCIN;

/*** Initialize Local Variables                                                         */
    retStatus = EPS_ERR_NONE;
    found_dev = FALSE;

	EPS_DBGPRINT(("PROT=%d / PID=%04X / VID=%04X\n", usbInfo->dev.port, usbInfo->dev.pid, usbInfo->dev.vid))

    if ( EPS_IO_OPEN != ioOpenState  ) {
        if ( (fd = epsUsbFnc.openPortal(&usbInfo->dev)) == EPS_INVALID_FILEDSC){
            EPS_RETURN( EPS_ERR_NOT_OPEN_IO );
		}
	} else{
		EPS_RETURN( EPS_ERR_2ND_OPEN_IO );
	}

	do{
		bufSizeOrg = bufSize + 1; /* +1=NULL terminate */

		/* Allocate temp buffer */
		EPS_SAFE_RELEASE(tmpDeviceIdBuf);
		tmpDeviceIdBuf = (EPS_INT8*)EPS_ALLOC(bufSizeOrg);
		if (tmpDeviceIdBuf == NULL) {
			EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
		}
		memset(tmpDeviceIdBuf, 0, bufSizeOrg);

		/* Get deviceID string */
		ret = epsUsbFnc.getDeviceID(fd, tmpDeviceIdBuf, &bufSize);
	}while(bufSize > bufSizeOrg && 0 != ret);

	if( 0 == ret ){
		found_dev = (serParseDeviceID(tmpDeviceIdBuf, bufSize, manufacturer, modelName, cmdLevel, NULL, egID) > 0);
		if(found_dev <= 0) {
			retStatus = EPS_ERR_PRINTER_NOT_FOUND;
		}

	} else{
		retStatus = EPS_ERR_COMM_ERROR;
	}

	if( EPS_ERR_NONE == retStatus ){
		ParseUsbProtocol(tmpDeviceIdBuf, bufSize, &usbInfo->usbProtocol);
	}

	EPS_SAFE_RELEASE(tmpDeviceIdBuf);
	
    if ( EPS_IO_OPEN != ioOpenState  ) {
		epsUsbFnc.closePortal(fd);
    }

    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CreatePrinterInfo()                                                 */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* dev			EPS_USB_DEVICE*     I: Pointer to a usb device infomatio                */
/* manufacturer EPS_INT8*           I: Pointer to a 64-byte buffer for the manufacturer */
/*                                     name.                                            */
/* modelName    EPS_INT8*           I: Pointer to a 64-byte buffer for the printer      */
/*                                     model name.                                      */
/* printer		EPS_PRINTER_INN**   O: pointer for found printer structure              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Create Printer info structure and setup data.                                   */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE     CreatePrinterInfo (

		EPS_USB_PRINTER_INFO*	usbInfo,
        const EPS_INT8*			manufacturer,
        const EPS_INT8*			modelName,
        const EPS_INT8*			serialNo,
		EPS_INT32               cmdLevel,
		EPS_UINT32              egID,
		EPS_PRINTER_INN**		printer

){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE    retStatus;
    EPS_INT8		usbIDString[EPS_NAME_BUFFSIZE];
	EPS_USB_PRINTER_INFO*	pUsbPrinter = NULL;

EPS_LOG_FUNCIN;
   
/*** Initialize Local Variables                                                         */
    retStatus = EPS_ERR_NONE;
    
/*** Create printer info structure                                                      */
	*printer = (EPS_PRINTER_INN*)EPS_ALLOC( sizeof(EPS_PRINTER_INN) );
	if(NULL == *printer){
		retStatus = EPS_ERR_MEMORY_ALLOCATION;
		goto CreatePrinterInfo_ERROR;
	}
	memset( *printer, 0, sizeof(EPS_PRINTER_INN) );

	pUsbPrinter = (EPS_USB_PRINTER_INFO*)EPS_ALLOC( sizeof(EPS_USB_PRINTER_INFO) );
	if(NULL == pUsbPrinter){
		retStatus = EPS_ERR_MEMORY_ALLOCATION;
		goto CreatePrinterInfo_ERROR;
	}
	memcpy(pUsbPrinter, usbInfo, sizeof(EPS_USB_PRINTER_INFO));
	(*printer)->protocolInfo = pUsbPrinter;

    strcpy( (*printer)->location, EPS_USB_NAME );
	strcpy( (*printer)->manufacturerName, manufacturer );
	strcpy( (*printer)->modelName, modelName );
	strcpy( (*printer)->serialNo, serialNo );
	(*printer)->protocol = EPS_PROTOCOL_USB;
	(*printer)->protocol |= EPS_PRT_DIRECTION(printJob.commMode);
	(*printer)->language = EPS_LANG_ESCPR; /* ESC/P-R only via USB */
	(*printer)->egID = egID;

	if(0 == cmdLevel){
		cmdLevel = 0xFFFFFFFF;	/* Support all. For Uin communication  */
	}
	(*printer)->supportFunc |= EPS_SPF_RGBPRINT; /* RGB print */
	if(cmdLevel & 0x0002){
		(*printer)->supportFunc |= EPS_SPF_JPGPRINT; /* Jpeg print */
	}

	sprintf(usbIDString, EPS_USB_IDPRM_STR, usbInfo->dev.vid, usbInfo->dev.pid, modelName);
	prtSetIdStr(*printer, usbIDString);

CreatePrinterInfo_ERROR:
	if(EPS_ERR_NONE != retStatus){
		EPS_SAFE_RELEASE( pUsbPrinter );
		EPS_SAFE_RELEASE( *printer );
	}

    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   PortResolution()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printer		EPS_PRINTER_INN*    I: pointer for found printer structure              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_NOT_OPEN_IO             - Cannot Open I/O Portal                        */
/*      EPS_ERR_PRINTER_NOT_FOUND	    - Printer not found (or error occur)	        */
/*                                                                                      */
/* Description:                                                                         */
/*      Resolve present port number.                                                    */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE   PortResolution(
	
		const EPS_PRINTER_INN*		printer,
		EPS_FILEDSC*				pFd

){

/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE	ret = EPS_ERR_NONE;			 /* Return status of internal calls */
    EPS_BOOL		bNeedResolve = TRUE;
	EPS_FILEDSC		fdInt = EPS_INVALID_FILEDSC;
	EPS_INT8		modelName[EPS_NAME_BUFFSIZE];       /* Printer model name       */

EPS_LOG_FUNCIN;

	*pFd = EPS_INVALID_FILEDSC;

    if( !EPS_IS_BI_PROTOCOL(printer->protocol) ){
		EPS_RETURN( EPS_ERR_NEED_BIDIRECT );
	}

	/*** Validate this is an Epson ESC/PR Printer                                       */
	ret = ProbeESCPR((EPS_USB_PRINTER_INFO*)printer->protocolInfo, 
						NULL, modelName, NULL, NULL);
	if(EPS_ERR_NONE == ret){
		/*** Printer names compare.                                                     */
		if( 0 == strcmp(modelName, printer->modelName) ){
			bNeedResolve = FALSE;
		}
	}

	if(bNeedResolve){
		if(EPS_INVALID_FILEDSC != fdInt){
			ClosePortal( ((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->usbProtocol, fdInt);
			fdInt = EPS_INVALID_FILEDSC;
		}

		/* Probe other port */
		ret = ProbePrinterByName(printer->modelName, FALSE, 0, (EPS_USB_PRINTER_INFO*)printer->protocolInfo,
								NULL, modelName, NULL, NULL);
	}

	/* open portal */
	if( EPS_ERR_NONE == ret ){
		ret = OpenPortal((EPS_USB_PRINTER_INFO*)printer->protocolInfo, &fdInt);
	}

	if( EPS_ERR_NONE == ret ){
		*pFd = fdInt;
	} else{
		if(EPS_INVALID_FILEDSC != fdInt){
			ClosePortal( ((EPS_USB_PRINTER_INFO*)printer->protocolInfo)->usbProtocol, fdInt);
		}
	} 

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   ProbePrinterByName()                                                */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* modelNameTgt EPS_INT8*           I: target printer modelname                         */
/* enableBreak	EPS_BOOL            I: enable break                                     */
/* timeout      EPS_UINT32          I: find timeout                                     */
/* dev			EPS_USB_DEVICE*     O: Pointer to a usb device infomatio                */
/* manufacturer EPS_INT8*           O: Pointer to a 64-byte buffer for the manufacturer */
/*                                     name.                                            */
/* modelName    EPS_INT8*           O: Pointer to a 64-byte buffer for the printer      */
/*                                     model name.                                      */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success (printer found)                       */
/*      EPS_ERR_PRINTER_NOT_FOUND       - printer not found                             */
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_NOT_OPEN_IO             - Cannot Open I/O Portal                        */
/*                                                                                      */
/* Description:                                                                         */
/*     looks for specified printer.                                                     */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE   ProbePrinterByName (

		const EPS_INT8*	modelNameTgt,
		EPS_BOOL		enableBreak,
        EPS_UINT32      timeout,
		EPS_USB_PRINTER_INFO*	usbInfo,
		EPS_INT8*		manufacturer,
		EPS_INT8*		modelName,
		EPS_INT32*      cmdLevel,
		EPS_UINT32*		egID

){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE	ret = EPS_ERR_NONE;			 /* Return status of internal calls     */
    EPS_BOOL		found = FALSE;
	EPS_FILEDSC		fd = EPS_INVALID_FILEDSC;
	EPS_UINT32		tmStart, tmNow, tmSpan;
    
EPS_LOG_FUNCIN;

	if(NULL != modelName)*modelName = '\0';
	if(NULL != manufacturer)*manufacturer = '\0';

	fd = epsUsbFnc.findFirst( &usbInfo->dev );
	if(EPS_INVALID_FILEDSC != fd){
		if(epsCmnFnc.getTime){
			tmStart = epsCmnFnc.getTime();
		} else{
			timeout = tmStart = tmNow = tmSpan = 0;
		}

		do{
			/*** Validate this is an Epson ESC/PR Printer                               */
			ret = ProbeESCPR(usbInfo, manufacturer, modelName, cmdLevel, egID);

			if(EPS_ERR_NONE == ret){
				if( 0 == strcmp(modelNameTgt, modelName) ){
					/* Found target printer */
					found = TRUE;
					break;
				}
			} else if(EPS_ERR_PRINTER_NOT_FOUND != ret){
				break;
			}

			if(timeout > 0){
				tmNow = epsCmnFnc.getTime();
				tmSpan = (EPS_UINT32)(tmNow - tmStart);
				EPS_DBGPRINT( ("TM %u - %u <> %u\n", tmNow, tmStart, tmSpan) )
				if( tmSpan >= timeout ){
					break;
				}
			}
			if( enableBreak ){
				if( 0 == epsCmnFnc.lockSync() ){
					if( g_FindBreak ){
						epsCmnFnc.unlockSync();
						break;
					}
					epsCmnFnc.unlockSync();
				}
			}
		}while( epsUsbFnc.findNext( fd, &usbInfo->dev ) );

		epsUsbFnc.findClose(fd);
	}

	if(EPS_ERR_NONE == ret){
		ret = (found?EPS_ERR_NONE:EPS_ERR_PRINTER_NOT_FOUND);
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   InfoCommand()                                                       */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* Command      EPS_INT32           I: Command Code                                     */
/* pResult      EPS_UINT8*          O: Result of Info command                           */
/* Size         EPS_INT32*          I/O: size of buffer                                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Info command executed successfully            */
/*      EPS_ERR_COMM_ERROR              - Info command execution error                  */
/*                                                                                      */
/* Description:                                                                         */
/*      Sends Information commands to the printer.                                      */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE   InfoCommand (

		EPS_FILEDSC	fd,
        EPS_UINT32  usbProtocol,
        EPS_INT32   Command,
        EPS_UINT8** pResult,
        EPS_INT32*  Size
        
){
    EPS_ERR_CODE    ret = EPS_ERR_NONE;

	EPS_LOG_FUNCIN;

	if(EPS_USBP_CBT == usbProtocol){
		ret = CbtInfoCommand(fd, Command, pResult, Size);
	} else /* END4 */ {
		ret = RmtInfoCommand(fd, Command, pResult, Size);
	}

    EPS_RETURN( ret );
}


static EPS_ERR_CODE   CbtInfoCommand (

		EPS_FILEDSC	fd,
        EPS_INT32   Command,
        EPS_UINT8** pResult,
        EPS_INT32*  Size
        
){
    static EPS_UINT8 stCom[5] = {'x', 'x', 0x01,0x00,0x01};
    EPS_INT32        ComSize  = 0;
    EPS_INT32        Retry;
    EPS_INT32        Ret;
    EPS_INT32        lSize;

	EPS_LOG_FUNCIN;

	if(NULL == *pResult && 0 == *Size){
		*pResult = (EPS_UINT8*)EPS_ALLOC(EPS_PM_BASESIZE);
		if(NULL == *pResult){
			EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
		}
		*Size = EPS_PM_BASESIZE;
	}

	switch(Command){
    case EPS_CBTCOM_DI:
        stCom[0] = 'd'; stCom[1] = 'i'; stCom[4] = 0x01;
        break;
    case EPS_CBTCOM_ST:
        stCom[0] = 's'; stCom[1] = 't'; stCom[4] = 0x01;
        break;
    case EPS_CBTCOM_PM:
        stCom[0] = 'p'; stCom[1] = 'm'; stCom[4] = 0x01;
        break;
    case EPS_CBTCOM_PM2:
        stCom[0] = 'p'; stCom[1] = 'm'; stCom[4] = 0x02;
        break;
    case EPS_CBTCOM_PM3:
        stCom[0] = 'p'; stCom[1] = 'm'; stCom[4] = 0x03;
        break;
    case EPS_CBTCOM_CD:
        stCom[0] = 'c'; stCom[1] = 'd'; stCom[4] = 0x00;
        break;
	case EPS_CBTCOM_VI5:
        stCom[0] = 'v'; stCom[1] = 'i'; stCom[4] = 0x05;
        break;
	default:
		EPS_RETURN( EPS_ERR_OPR_FAIL );
    }

    Retry = 5;
    do{
        if ((Ret = cbtCommWriteData(fd, EPS_CBTCHANNEL_CTRL, stCom, 5, &ComSize)) != EPCBT_ERR_NONE){
            EPS_DBGPRINT(("EPS SER : CBT Command Write Failed [%d]\r\n",Ret));
            if (Ret != EPSCBT_ERR_FNCDISABLE && Ret != EPSCBT_ERR_WRITEERROR){
                EPS_RETURN( EPS_ERR_COMM_ERROR );
            }

            Retry--;
            if (!Retry) {
                EPS_DBGPRINT(("EPS SER : CBT Command Write Retry Failed \r\n"));
                EPS_RETURN( EPS_ERR_COMM_ERROR );
            }        
            serDelayThread(2*_SECOND_, &epsCmnFnc);       /* retry command after 2 seconds*/
        }
    }while (Ret == EPSCBT_ERR_FNCDISABLE);

    Retry = 5;
    lSize = *Size;
    do{
        if ((Ret = cbtCommReadData(fd, EPS_CBTCHANNEL_CTRL, *pResult, lSize, Size)) != EPCBT_ERR_NONE){
            EPS_DBGPRINT(("EPS SER : CBT Command Read Reply Failed [%d]\r\n",Ret));
            if (Ret != EPSCBT_ERR_FNCDISABLE && Ret != EPSCBT_ERR_READERROR){
				EPS_SAFE_RELEASE(*pResult);
				*Size = 0;
                EPS_RETURN( EPS_ERR_COMM_ERROR );
            }    
        }else if (*Size != 0){
            break;
        }

        Retry--;
        if (!Retry){
            EPS_DBGPRINT(("EPS SER : CBT Command Read Reply Retry Failed \r\n"));
			EPS_SAFE_RELEASE(*pResult);
			*Size = 0;
            EPS_RETURN( EPS_ERR_COMM_ERROR );
        }
        serDelayThread(2*_SECOND_, &epsCmnFnc);       /* retry command after 2 seconds*/
    }while(Retry > 0);
    
	if( *Size < 5 || 0 == memcmp(&(*pResult)[3], "NA", 2) ){
		EPS_SAFE_RELEASE(*pResult);
		*Size = 0;
		EPS_RETURN( EPS_ERR_PRINTER_NOT_SUPPORTED );
	}

    (*pResult)[*Size] = '\0';

    EPS_RETURN( EPS_ERR_NONE );
}


static EPS_ERR_CODE   RmtInfoCommand (

		EPS_FILEDSC	fd,
        EPS_INT32   Command,
        EPS_UINT8** pResult,
        EPS_INT32*  Size
        
){
    EPS_UINT8 stCom[27] = {0x1B, 0x28, 0x52, 0x08, 0x00, 0x00, 'R', 'E', 'M', 'O', 'T', 'E', '1', 
							'P', 'I', 0x06,0x00,0x01, 0x00, 0x00,0x00, 0x00,0x00,
							0x1B, 0x00, 0x00, 0x00};
    const EPS_INT8 *rsp = "@BDC PI\x0D\x0A";
    EPS_UINT8 rspBuff[EPS_PM_BASESIZE];
    EPS_INT32 rspSize = sizeof(rspBuff);
    EPS_INT32 ComSize = sizeof(stCom);
    EPS_INT32 ret = 0;
 	EPS_INT32 retry = 0;
    EPS_INT32 packetSize = 0;
    EPS_INT32 packetNum = 0;
    EPS_INT32 totalRespSize = 0;
    EPS_INT32 n = 0;
    EPS_BOOL  rcvResponce = FALSE;
	EPS_STATUS_INFO stInfo;
 
	EPS_LOG_FUNCIN;

	if(NULL == *pResult && 0 == *Size){
		*pResult = (EPS_UINT8*)EPS_ALLOC(EPS_PM_BASESIZE);
		if(NULL == *pResult){
			EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
		}
		*Size = EPS_PM_BASESIZE;
	}

	/* step 0: dummy(ST) read */
	for(retry = 0; retry < EPS_STAT_RETRY; retry++){
		rspSize = *Size;
		ret = nd4ReadPortal(fd, *pResult, &rspSize);
		/*EPS_DUMP(*pResult, rspSize)*/
		if( EPS_ERR_NONE != ret ){
			EPS_RETURN( EPS_ERR_COMM_ERROR );
		}
		if( 0 == memcmp(*pResult, "@BDC ST2\x0D\x0A", 10) ){
			break;
		}
	}
	if(EPS_CBTCOM_ST == Command){
	    EPS_RETURN( EPS_ERR_NONE );
	}

	/* check IDLE */
	ret = serAnalyzeStatus((EPS_INT8*)*pResult, 0, &stInfo);
	if( EPS_ERR_NONE != ret ){
		EPS_RETURN( EPS_ERR_COMM_ERROR );
	}
	if( EPS_ST_IDLE != stInfo.nState ){
		EPS_RETURN( EPS_ERR_COMM_ERROR );
	}

	/* set command parameter */
	rspSize = sizeof(rspBuff);
	stCom[21] = rspSize & 0x000000FF;
	stCom[22] = (rspSize & 0x0000FF00) >> 8;
	switch(Command){
    case EPS_CBTCOM_PM:
		stCom[18] = 0x80;
		strcpy((EPS_INT8*)*pResult, "@BDC PM\x0D\x0A");
		totalRespSize = 9;
        break;
    case EPS_CBTCOM_PM2:
		stCom[18] = 0x81;
		strcpy((EPS_INT8*)*pResult, "@BDC PM\x0D\x0A");
		totalRespSize = 9;
        break;
    case EPS_CBTCOM_PM3:
		stCom[18] = 0x82;
		totalRespSize = 0;
        break;
    case EPS_CBTCOM_CD:
		stCom[18] = 0x00;
        break;
	default:
		EPS_RETURN( EPS_ERR_OPR_FAIL );
    }

	/* step 1: get reply size */
	/*EPS_DUMP(stCom, ComSize)*/
	ret = nd4WritePortal(fd, stCom, &ComSize);
	if( EPS_ERR_NONE != ret ){
		EPS_RETURN( EPS_ERR_COMM_ERROR );
	}

	for(retry = 0; retry < EPS_STAT_RETRY; retry++){
		rspSize = sizeof(rspBuff);
		ret = nd4ReadPortal(fd, rspBuff, &rspSize);
		/*EPS_DUMP(rspBuff, rspSize)*/

		if( EPS_ERR_NONE != ret ){
			EPS_RETURN( EPS_ERR_COMM_ERROR );
		}
		if( 0 == memcmp(rsp, rspBuff, strlen(rsp)) ){
			rcvResponce = TRUE;
			break;
		} 
	}
	if( FALSE == rcvResponce){
		EPS_RETURN( EPS_ERR_COMM_ERROR );
	}
	if( 0 == memcmp(&rspBuff[21], "NA", 2) ){
		EPS_RETURN( EPS_ERR_PRINTER_NOT_SUPPORTED );
	} else if( rspSize < 24 || 0 == memcmp(&rspBuff[21], "BUSY", 4)){
		EPS_RETURN( EPS_ERR_COMM_ERROR );
	}

	packetNum = rspBuff[21] + (rspBuff[22]<<8);
	packetSize= rspBuff[23] + (rspBuff[24]<<8);

	EPS_MEM_GROW(EPS_UINT8*, *pResult, Size, packetNum*packetSize )
	if(NULL == *pResult){
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
	}

	/* step 2: send command */
	for(n = 1; n <= packetNum; n++ ){
		ComSize = sizeof(stCom);
		stCom[19] = n & 0x000000FF;
		stCom[20] = n >> 8;

		/*EPS_DUMP(stCom, ComSize)*/
		ret = nd4WritePortal(fd, stCom, &ComSize);
		if( EPS_ERR_NONE != ret ){
			EPS_RETURN( EPS_ERR_COMM_ERROR );
		}

		for(retry = 0; retry < EPS_STAT_RETRY; retry++){
			rspSize = sizeof(rspBuff);
			ret = nd4ReadPortal(fd, rspBuff, &rspSize);
			/*EPS_DUMP(rspBuff, rspSize)*/

			if( EPS_ERR_NONE != ret ){
				EPS_RETURN( EPS_ERR_COMM_ERROR );
			}
			if( 0 == memcmp(rsp, rspBuff, strlen(rsp)) ){
				rcvResponce = TRUE;
				break;
			} 
		}
		if( 0 == memcmp(&rspBuff[21], "NA", 2) ){
			EPS_RETURN( EPS_ERR_PRINTER_NOT_SUPPORTED );
		} else if( 0 == memcmp(&rspBuff[21], "BUSY", 4)){
			EPS_RETURN( EPS_ERR_COMM_ERROR );
		}

		rspSize = rspBuff[29] + (rspBuff[30]<<8);
		if(*Size < rspSize + totalRespSize){
			EPS_RETURN( EPS_ERR_OPR_FAIL ); /* buffer not enough */
		}

		memcpy(*pResult+totalRespSize, rspBuff+35, rspSize);
		totalRespSize += rspSize;
	}
	*Size = totalRespSize;

    EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   nd4WritePortal()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC*        I: usb file descriptor                              */
/* buff         EPS_UINT8*          I: write data                                       */
/* buffSize     EPS_INT32*          I/O: write data size                                */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                - Info command executed successfully                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Write portal via CBT or END4.                                                   */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE  nd4WritePortal (

		EPS_FILEDSC		fd,
		const EPS_UINT8	*buff,
		EPS_INT32		*buffSize

){
	EPS_INT32 ret = 0;
	EPS_INT32 retry = 0;
	EPS_INT32 writeSize = 0;
	EPS_INT32 writeTotal = 0;

	EPS_LOG_FUNCIN;

	for(retry = 0; retry < EPS_STAT_RETRY; retry++){
		ret = epsUsbFnc.writePortal(fd, buff+writeTotal, *buffSize, &writeSize);
		if( EPS_ERR_NONE != ret ){
			EPS_RETURN( EPS_ERR_COMM_ERROR );
		}
		writeTotal += writeSize;
		*buffSize -=  writeSize;
		
		if(0 >= *buffSize){
			break;
		}
		serDelayThread(2*_SECOND_, &epsCmnFnc);       /* retry command after 2 seconds*/
	}
	*buffSize = writeTotal;

	if(retry >= EPS_STAT_RETRY){
		EPS_RETURN( EPS_ERR_COMM_ERROR );
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   nd4ReadPortal()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC*        I: usb file descriptor                              */
/* buff         EPS_UINT8*          I: read buffer                                      */
/* buffSize     EPS_INT32*          I/O: buffer size                                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                - Info command executed successfully                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Read portal via CBT or END4.                                                    */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE  nd4ReadPortal (

		EPS_FILEDSC		fd,
		EPS_UINT8		*buff,
		EPS_INT32		*buffSize

){
	EPS_INT32 ret = 0;
	EPS_INT32 retry = 0;
	EPS_INT32 readSize = 0;
	EPS_INT32 readTotal = 0;
	EPS_INT32 buffRemain = *buffSize;
	EPS_UINT8 *dataSizePos = 0;
	EPS_INT32 dataSize = 0;

	EPS_LOG_FUNCIN;

	for(retry = 0; retry < EPS_STAT_RETRY; ){
		ret = epsUsbFnc.readPortal(fd, buff+readTotal, buffRemain, &readSize);
		if( EPS_ERR_NONE != ret ){
			EPS_DBGPRINT(("read error %d", ret));
			EPS_RETURN( EPS_ERR_COMM_ERROR );
		}
		if(readSize <= 0){
			retry++;
			serDelayThread(500, &epsCmnFnc);       /* retry command after 500 milli seconds*/
			continue;
		}
		readTotal += readSize;
		buffRemain -=  readSize;

		if( 0 >= dataSize){
			/* get data size */
			if( (dataSizePos = (EPS_UINT8*)strstr((EPS_INT8*)buff, "\x0D\x0A")) != NULL ){
				if( (dataSizePos-buff) + 4 < readTotal){
					dataSize  = *(dataSizePos+2);
					dataSize += *(dataSizePos+3) << 8;
					dataSize += dataSizePos - buff + 4;
					if(*buffSize < dataSize){
						EPS_RETURN( EPS_ERR_OPR_FAIL ); /* buffer not enough */
					}
				}
			}
		}

		if( 0 < dataSize && dataSize <= readTotal){
			break;
		}
	}
	*buffSize = readTotal;

	if(retry >= EPS_STAT_RETRY){
		EPS_RETURN( EPS_ERR_COMM_ERROR );
	}

	EPS_RETURN( ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   OpenPortal()                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* usbInfo      EPS_USB_PRINTER_INFO* I: usb protocol info                              */
/* fd           EPS_FILEDSC*        O: usb file descriptor                              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                - Info command executed successfully                */
/*      EPS_ERR_NOT_OPEN_IO         - Cannot Open I/O Portal                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Open portal via CBT or END4.                                                    */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE  OpenPortal (

	EPS_USB_PRINTER_INFO    *usbInfo,
	EPS_FILEDSC             *fd

){
	EPS_INT32 ret = 0;

	EPS_LOG_FUNCIN;

	if(EPS_USBP_CBT == usbInfo->usbProtocol){
		ret = cbtCommOpen( &usbInfo->dev, fd );
	} else{
		*fd = epsUsbFnc.openPortal(&usbInfo->dev);
		if( EPS_INVALID_FILEDSC == *fd ){
			ret = EPS_ERR_NOT_OPEN_IO;
		} else{
			ioOpenState = EPS_IO_OPEN;
		}
	}

	if(0 == ret){
		EPS_RETURN( EPS_ERR_NONE );
	} else{
		EPS_RETURN( EPS_ERR_NOT_OPEN_IO );
	}
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   ClosePortal()                                                       */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* usbProtocol  EPS_UINT32          I: usb protocol kind                                */
/* fd           EPS_FILEDSC*        I: usb file descriptor                              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                - Info command executed successfully                */
/*      EPS_ERR_NOT_CLOSE_IO        - Cannot close I/O portal                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Open portal via CBT or END4.                                                    */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE  ClosePortal (

	EPS_UINT32	usbProtocol,
	EPS_FILEDSC	fd

){
	EPS_INT32 ret = 0;

	EPS_LOG_FUNCIN;

	if(EPS_USBP_CBT == usbProtocol){
		ret = cbtCommClose(fd);
	} else{
		ret = epsUsbFnc.closePortal(fd);
		ioOpenState = EPS_IO_NOT_OPEN;
	}

	if(0 == ret){
		EPS_RETURN( EPS_ERR_NONE );
	} else{
		EPS_RETURN( EPS_ERR_NOT_CLOSE_IO );
	}
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   ParseUsbProtocol()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* deviceIDString EPS_INT8*         I: deviceID string                                  */
/* strlength    EPS_INT32           I: string length                                    */
/* usbProtocol  EPS_UINT32*         O: usb protocol kind                                */
/*                                                                                      */
/* Return value:                                                                        */
/*      N/A                                                                             */
/*                                                                                      */
/* Description:                                                                         */
/*      Get the epson usb protocol from DeviceID string.                                */
/*                                                                                      */
/*******************************************|********************************************/
static void   ParseUsbProtocol (

        EPS_INT8*   deviceIDString,
		EPS_INT32   strlength, 
        EPS_UINT32* usbProtocol
        
){
    EPS_INT8*       i = NULL;
    EPS_INT8*       j = NULL;
    EPS_INT8*       k = NULL;

	EPS_LOG_FUNCIN;

	*usbProtocol = EPS_USBP_CBT; /* default */
	
	if(strlength < 2){
		EPS_RETURN_VOID;
	}
    if(deviceIDString[0] == 0x00 || deviceIDString[1] == 0x00){
        deviceIDString += 2;
	}

    if((i = strstr(deviceIDString, "CMD:")) == NULL) {
		EPS_RETURN_VOID;
	}
	i += 4;

    if((j = strstr(i, ";")) == NULL) {
		EPS_RETURN_VOID;
	}
	*j = '\0';

	while( NULL != i ){
		if((k = strstr(i, ",")) != NULL) {
			*k = '\0';
		}

/*		if( strncmp(i, "D4", 4) == 0 ){
			*usbProtocol = EPS_USBP_CBT;
			break;
		}
*/
		if( strncmp(i, "END4", 4) == 0 ){
			*usbProtocol = EPS_USBP_ND4;
			break;
		}

		if(k){
			*k = ',';
			i = k+1;
			if(i >= j){
				break;
			}
		} else{
			break;
		}
	}
	if(k)*k = ',';

	*j = ';';

    EPS_RETURN_VOID;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   GetSerialNumber()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* usbInfo      EPS_USB_PRINTER_INFO* I: usb protocol info                              */
/* deviceIDString EPS_INT8*         I: deviceID string                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                - Info command executed successfully                */
/*                                                                                      */
/* Description:                                                                         */
/*      Get the epson usb protocol from DeviceID string.                                */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE   GetSerialNumber (

        EPS_USB_PRINTER_INFO    *usbInfo,
		EPS_INT8*               serialNo
        
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_FILEDSC fd;
	EPS_UINT8*  tmpBuff = NULL;
	EPS_INT32   tmpBuffSize = EPS_DI_MAXSIZE;

	EPS_LOG_FUNCIN;

	ret = OpenPortal( usbInfo, &fd );
	if(EPS_ERR_NONE != ret){
		EPS_RETURN( ret );
	}

	tmpBuff = (EPS_INT8*)EPS_ALLOC(tmpBuffSize);
	if (tmpBuff == NULL) {
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
	}
	memset(tmpBuff, 0, tmpBuffSize);

	if( EPS_USBP_CBT == usbInfo->usbProtocol ){
		ret = cbtCommChannelOpen(fd, EPS_CBTCHANNEL_CTRL, TRUE);
		if (EPS_ERR_NONE == ret ) {
			ret = InfoCommand(fd, usbInfo->usbProtocol, EPS_CBTCOM_CD, &tmpBuff, &tmpBuffSize);
			if (EPS_ERR_NONE == ret ) {
				/*retStatus = */serGetSerialNo((EPS_INT8*)tmpBuff, tmpBuffSize, serialNo);
			} else{
				ret = EPS_ERR_NONE; /* ignoure */
			}
		} else{
			ret = (EPS_ERR_CODE)EPS_ERR_NOT_OPEN_IO;
		}

		cbtCommChannelClose(fd, EPS_CBTCHANNEL_CTRL);
	} else{
		ret = RmtInfoCommand(fd, EPS_CBTCOM_ST, &tmpBuff, &tmpBuffSize);
		if (EPS_ERR_NONE == ret ) {
			/*retStatus = */serGetSerialNoFormST((EPS_INT8*)tmpBuff, serialNo, EPS_ADDR_BUFFSIZE);
		} else{
			ret = EPS_ERR_NONE; /* ignoure */
		}
	}

	EPS_SAFE_RELEASE(tmpBuff);

	ClosePortal(usbInfo->usbProtocol, fd);

	EPS_RETURN( ret );
}

/*___________________________________  epson-usb.c  ____________________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
