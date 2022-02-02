/*________________________________  epson-protocol.c   _________________________________*/

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
/*                              Protocol Depend functions                               */
/*                                                                                      */
/*                                Public Function Calls                                 */
/*                              --------------------------                              */
/*		EPS_ERR_CODE prtFunctionCheck       (commMode, usbFuncPtrs, netFuncPtrs,        */
/*                                           cmnFuncPtrs                        );      */
/*      EPS_ERR_CODE prtFindPrinter         (timeout							);      */
/*      void         prtCancelFindPrinter	(                                   );      */
/*      EPS_ERR_CODE prtProbePrinterByID	(probeParam                         );      */
/*      EPS_ERR_CODE prtProbePrinterByAddr	(probeParam                         );      */
/*      EPS_ERR_CODE prtRecoverPE           (                                   );      */
/*      EPS_ERR_CODE prtGetInfo             (printer, buffer, bufSize           );      */
/*      EPS_ERR_CODE prtRegPrinter          (printer, bNotify                   );      */
/*      EPS_ERR_CODE prtIsRegistered        (Address, Protocol                  );      */
/*      EPS_ERR_CODE prtClearPrinterList    (printer                            );      */
/*      EPS_ERR_CODE prtClearPrinterAttribute(printer                           );      */
/*      EPS_ERR_CODE prtSetIdStr            (printer, idString                  );      */
/*                                                                                      */
/*******************************************|********************************************/

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-pvt.h"
#include "epson-escpr-err.h"
#include "epson-escpr-services.h"
#include "epson-escpr-pm.h"
#include "epson-escpr-mem.h"
#ifdef GCOMSW_CMD_ESCPAGE
#include "epson-escpage.h"
#endif
#ifdef GCOMSW_CMD_ESCPAGE_S
#include "epson-escpage-s.h"
#endif
#ifdef GCOMSW_CMD_PCL
#include "epson-pcl.h"
#endif

#include "epson-protocol.h"

#ifdef GCOMSW_PRT_USE_USB
#include "epson-usb.h"
#endif

#ifdef GCOMSW_PRT_USE_LPR
#include "epson-net-lpr.h"
#endif

#ifdef GCOMSW_PRT_USE_RAW
#include "epson-net-raw.h"
#endif

/*-----------------------------  Local Macro Definitions -------------------------------*/
/*******************************************|********************************************/
#ifdef EPS_LOG_MODULE_PRTCOL
#define EPS_LOG_MODULE	EPS_LOG_MODULE_PRTCOL
#else
#define EPS_LOG_MODULE	0
#endif

/*------------------------------------  Definition   -----------------------------------*/
/*******************************************|********************************************/
/* Find Printer functions                                                               */
typedef EPS_ERR_CODE	(*NET_FindStart	)(EPS_SOCKET*, const EPS_INT8*, EPS_BOOL, const EPS_UINT8*);
typedef EPS_ERR_CODE	(*NET_FindCheck	)(EPS_SOCKET, EPS_PRINTER_INN**                 );
typedef EPS_ERR_CODE	(*NET_FindEnd	)(EPS_SOCKET                                    );

typedef struct tagEPS_FIND_FUNCS {
	NET_FindStart	fncStart;			/* Start function                               */
	NET_FindCheck	fncCheck;			/* Check function                               */
	NET_FindEnd		fncEnd;				/* End function                                 */
	EPS_SOCKET		sock;				/* socekt                                       */
	EPS_INT8		address[EPS_ADDR_BUFFSIZE];
	EPS_UINT8*		ifName;
}EPS_FIND_FUNCS;

#define	EPS_FINDNOTIFY_DELAY_COUNT		(10)

/*--------------------------  ESC/P-R Net Lib Global Variables  ------------------------*/
/*******************************************|********************************************/

	/*** Extern Function                                                                */
	/*** -------------------------------------------------------------------------------*/
extern EPS_NET_FUNC    epsNetFnc;
extern EPS_CMN_FUNC    epsCmnFnc;

    /*** Print Job Structure                                                            */
    /*** -------------------------------------------------------------------------------*/
extern EPS_PRINT_JOB   printJob;

	/*** Find                                                                           */
	/*** -------------------------------------------------------------------------------*/
EPS_BOOL     g_FindBreak;									/* Find printer end flag    */
EPS_INT32    g_FindProtocol;									

/*-------------------------  Module "Local Global" Variables  --------------------------*/
/*******************************************|********************************************/

    /*** internal stock                                                                 */
    /*** -------------------------------------------------------------------------------*/
static EPS_PRINTER_LIST	epsPrinterList;				/* Printer List						*/

/*--------------------------------  Local Functions   ----------------------------------*/
/*******************************************|********************************************/
#ifdef GCOMSW_PRT_USE_NETWORK
static EPS_ERR_CODE FindNetPrinter	(EPS_INT32, EPS_UINT32, EPS_BOOL, const EPS_INT8*	);
#endif
static void MakePrinterStructure	(EPS_PRINTER* dst, EPS_PRINTER_INN* src				);
static EPS_BOOL     IsValidAddress  (const EPS_INT8*                                    );


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------              Public Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     prtFunctionCheck()                                                */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* commMode     EPS_INT8            I: Communication Mode                               */
/*                                     (Bi-Directional or Uni-Directional)              */
/* usbFuncPtrs  EPS_USB_FUNC*       I: Data structure containing function pointers to   */
/*                                     USB I/O functions.                               */
/* netFuncPtrs  EPS_NET_FUNC*       I: Data structure containing function pointers to   */
/*                                     Socket I/O functions.                            */
/* cmnFuncPtrs  EPS_CMN_FUNC*       I: Data structure containing function pointers to   */
/*                                     external memory management functions.            */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                     - Success                                      */
/*      << Error >>                                                                     */
/*      EPS_ERR_INV_ARG_COMMMODE         - Invalid argument "commMode"                  */
/*      EPS_ERR_INV_FNCP_NULL            - Invalid function pointer structure           */
/*      EPS_ERR_INV_FNCP_FINDCALLBACK    - Invalid function pointer "findCallback"      */
/*      EPS_ERR_INV_FNCP_MEMALLOC        - Invalid function pointer "memAlloc"          */
/*      EPS_ERR_INV_FNCP_MEMFREE         - Invalid function pointer "memFree"           */
/*      EPS_ERR_INV_FNCP_SLEEP           - Invalid function pointer "sleep"             */
/*      EPS_ERR_INV_FNCP_GETTIME         - Invalid function pointer "GetTime"           */
/*      EPS_ERR_INV_FNCP_OPENPORTAL      - Invalid function pointer "openPortal"        */
/*      EPS_ERR_INV_FNCP_CLOSEPORTAL     - Invalid function pointer "closePortal"       */
/*      EPS_ERR_INV_FNCP_READPORTAL      - Invalid function pointer "readPortal"        */
/*      EPS_ERR_INV_FNCP_WRITEPORTAL     - Invalid function pointer "writePortal"       */
/*      EPS_ERR_INV_FNCP_FINDFIRST       - Invalid function pointer "findFirst"         */
/*      EPS_ERR_INV_FNCP_FINDNEXT        - Invalid function pointer "findNext"          */
/*      EPS_ERR_INV_FNCP_FINDCLOSE       - Invalid function pointer "findClose"         */
/*      EPS_ERR_INV_FNCP_NETSOCKET       - Invalid function pointer "socket"            */
/*      EPS_ERR_INV_FNCP_NETCLOSE        - Invalid function pointer "close"             */
/*      EPS_ERR_INV_FNCP_NETCONNECT      - Invalid function pointer "connect"           */
/*      EPS_ERR_INV_FNCP_NETSHUTDOWN     - Invalid function pointer "shutdown"          */
/*      EPS_ERR_INV_FNCP_NETBIND         - Invalid function pointer "bind"              */
/*      EPS_ERR_INV_FNCP_NETLISTEN       - Invalid function pointer "listen"            */
/*      EPS_ERR_INV_FNCP_NETACCEPT       - Invalid function pointer "accept"            */
/*      EPS_ERR_INV_FNCP_NETSEND         - Invalid function pointer "send"              */
/*      EPS_ERR_INV_FNCP_NETSENDTO       - Invalid function pointer "sendTo"            */
/*      EPS_ERR_INV_FNCP_NETRECEIVE      - Invalid function pointer "receive"           */
/*      EPS_ERR_INV_FNCP_NETRECEIVEFROM  - Invalid function pointer "receiveFrom"       */
/*      EPS_ERR_INV_FNCP_NETGETSOCKNAME  - Invalid function pointer "getsockname"       */
/*      EPS_ERR_INV_FNCP_NETSETBROADCAST - Invalid function pointer "setBroadcast"      */
/*      EPS_ERR_MEMORY_ALLOCATION        - Failed to allocate memory                    */
/*                                                                                      */
/* Description:                                                                         */
/*      Validate external functions.                                                    */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    prtFunctionCheck (

        EPS_INT32               commMode, 
		const EPS_USB_FUNC*	    usbFuncPtrs,
        const EPS_NET_FUNC*     netFuncPtrs,
        const EPS_CMN_FUNC*	    cmnFuncPtrs

){
	EPS_UINT8*      testptr;                        /* Test pointer to do memory test   */
    EPS_UINT32      validProtocol; 

	EPS_LOG_FUNCIN

/*** Validate communication mode                                                        */
	if( (commMode & EPS_COMM_UNID) && (commMode & EPS_COMM_BID) ){
		EPS_RETURN( EPS_ERR_INV_ARG_COMMMODE )
	}
	if( !(commMode & EPS_PROTOCOL_ALL) ){
		EPS_RETURN( EPS_ERR_INV_ARG_COMMMODE )
	}

	validProtocol = EPS_PROTOCOL_INVALID;
#ifdef GCOMSW_PRT_USE_USB
	validProtocol |= EPS_PROTOCOL_USB;
#endif
#ifdef GCOMSW_PRT_USE_LPR
	validProtocol |= EPS_PROTOCOL_LPR;
#endif
#ifdef GCOMSW_PRT_USE_RAW
	validProtocol |= EPS_PROTOCOL_RAW;
#endif
	if( EPS_PRT_PROTOCOL(commMode) & ~validProtocol ){
		EPS_RETURN( EPS_ERR_INV_ARG_COMMMODE )
	}

/*** Validate External Functions                                                        */
	if(cmnFuncPtrs == NULL){
        EPS_RETURN( EPS_ERR_INV_FNCP_NULL )
	}

	if (cmnFuncPtrs->memAlloc       == NULL){
        EPS_RETURN( EPS_ERR_INV_FNCP_MEMALLOC )
	}

	if (cmnFuncPtrs->memFree        == NULL){
        EPS_RETURN( EPS_ERR_INV_FNCP_MEMFREE )
	}
#ifdef GCOMSW_EPSON_SLEEP
	if (cmnFuncPtrs->sleep          != NULL){
#else
	if (cmnFuncPtrs->sleep          == NULL){
#endif
        EPS_RETURN( EPS_ERR_INV_FNCP_SLEEP )
	}

	if (cmnFuncPtrs->findCallback   == NULL){
        EPS_RETURN( EPS_ERR_INV_FNCP_FINDCALLBACK )
	}

	if (cmnFuncPtrs->getTime == NULL
		&& ((cmnFuncPtrs->lockSync == NULL) || (cmnFuncPtrs->unlockSync == NULL)) ){
        EPS_RETURN( EPS_ERR_INV_FNCP_GETTIME )
	}

	/*** Check Memory Functions                                                         */
    testptr = NULL;
    testptr = (EPS_UINT8 *)cmnFuncPtrs->memAlloc(10);
    if (testptr == NULL) {
        EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION ) 
    }
    cmnFuncPtrs->memFree(testptr);


/*** Validate USB Functions                                                            */
#ifdef GCOMSW_PRT_USE_USB
	if (commMode & EPS_PROTOCOL_USB) {
		if (usbFuncPtrs == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_NULL )
		}

		if (usbFuncPtrs->openPortal     == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_OPENPORTAL )
		}
		if (usbFuncPtrs->closePortal    == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_CLOSEPORTAL )
		}
		if (usbFuncPtrs->writePortal    == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_WRITEPORTAL )
		}

        if ( commMode & EPS_COMM_BID ) {
			if (usbFuncPtrs->readPortal    == NULL){
				EPS_RETURN( EPS_ERR_INV_FNCP_READPORTAL )
			}
			if (usbFuncPtrs->findFirst     == NULL){
				EPS_RETURN( EPS_ERR_INV_FNCP_FINDFIRST )
			}
			if (usbFuncPtrs->findNext      == NULL){
				EPS_RETURN( EPS_ERR_INV_FNCP_FINDNEXT )
			}
			if (usbFuncPtrs->findClose     == NULL){
				EPS_RETURN( EPS_ERR_INV_FNCP_FINDCLOSE )
			}
			if (usbFuncPtrs->getDeviceID   == NULL){
				EPS_RETURN( EPS_ERR_INV_FNCP_GETDEVICEID )
			}
			if (usbFuncPtrs->softReset     == NULL){
				EPS_RETURN( EPS_ERR_INV_FNCP_SOFTRESET )
			}
#if 0 /* not necessary */
			if (cmnFuncPtrs->stateCallback == NULL)
				return (EPS_ERR_CODE)EPS_ERR_INV_FNCP_NOTIFYCALLBACK;
#endif
		}
    }
#else
	(void)usbFuncPtrs;
#endif


/*** Validate Net Functions                                                            */
#ifdef GCOMSW_PRT_USE_NETWORK
	if( (commMode & EPS_PROTOCOL_NET) ){
		if (netFuncPtrs == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_NULL )
		}

		if (netFuncPtrs->socket        == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_NETSOCKET )
		}
		if (netFuncPtrs->close         == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_NETCLOSE )
		}
		if (netFuncPtrs->connect       == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_NETCONNECT )
		}
		if (netFuncPtrs->shutdown      == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_NETSHUTDOWN )
		}
		if (netFuncPtrs->send          == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_NETSEND )
		}
		if (netFuncPtrs->sendTo        == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_NETSENDTO )
		}
		if (netFuncPtrs->receive       == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_NETRECEIVE )
		}
		if (netFuncPtrs->receiveFrom   == NULL){
            EPS_RETURN( EPS_ERR_INV_FNCP_NETRECEIVEFROM )
		}

#ifdef GCOMSW_PRT_USE_LPR
		if(commMode & EPS_PROTOCOL_LPR){
			if (netFuncPtrs->setBroadcast       == NULL){
				EPS_RETURN( EPS_ERR_INV_FNCP_NETSETBROADCAST )
			}
		}
#endif

	}
#else
	(void)netFuncPtrs;
#endif /* GCOMSW_PRT_USE_NETWORK */

#ifdef GCOMSW_EPSON_SLEEP
/*** Initial parameters for internal (simple) timing routine                            */
/*** Used when system sleep routines are not available                                  */
    if (epsCmnFnc.sleep == serSleep) {
        tdx = sleepTime = 0;
        ftime(&sleepS); 
        while (!sleepTime) {
            ftime(&sleepE);
            sleepTime  = 1000*(sleepE.time    - sleepS.time) +
                              (sleepE.millitm - sleepS.millitm);
            tdx++;
        }
        tdx *= 50;
        for (idx = 1; idx < tdx; idx++) {ftime(&sleepE); } 
        sleepTime  = 1000*(sleepE.time    - sleepS.time) +
                          (sleepE.millitm - sleepS.millitm);
        printJob.sleepSteps = tdx/sleepTime;                /* Num steps for one  msec  */
        if (!printJob.sleepSteps) 
            printJob.sleepSteps = -sleepTime/tdx;           /* Num  msec for each step  */
    }
#endif /* GCOMSW_EPSON_SLEEP */

	EPS_RETURN( EPS_ERR_NONE )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   prtFindPrinter()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* protocol     EPS_INT32           I: traget protocol                                  */
/* timeout      EPS_UINT32          I: find timeout                                     */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE					- Success (Opened Communication)                */
/*      EPS_ERR_MEMORY_ALLOCATION		- Fail to memory allocation                     */
/*      EPS_ERR_PRINTER_NOT_FOUND		- Printer not found (or error occur)		    */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/*      EPS_FIND_CANCELED           - user discontinued it                              */
/*                                                                                      */
/* Description:                                                                         */
/*     Find USB and network printers.                                                   */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE prtFindPrinter (

		EPS_INT32           protocol,
        EPS_UINT32          timeout

){
    EPS_ERR_CODE    ret = EPS_ERR_PRINTER_NOT_FOUND; /* Return status of internal calls */
    EPS_INT32       initProtocol = EPS_PRT_PROTOCOL(printJob.commMode);
    EPS_INT32       tgtProtocol = EPS_PRT_PROTOCOL(protocol);

	EPS_LOG_FUNCIN

	if( (tgtProtocol | initProtocol) != initProtocol){
		EPS_RETURN( EPS_ERR_INV_ARG_COMMMODE )
	}
	g_FindProtocol = tgtProtocol;

	/* Reset break flag */
	if( epsCmnFnc.lockSync && epsCmnFnc.unlockSync ){
		epsCmnFnc.lockSync();
		g_FindBreak = FALSE;
		epsCmnFnc.unlockSync();
	}

/***------------------------------------------------------------------------------------*/
/*** USB find                                                                           */
/***------------------------------------------------------------------------------------*/
#ifdef GCOMSW_PRT_USE_USB
	if( protocol & EPS_PROTOCOL_USB ){

		ret = usbFind( &timeout, printJob.commMode );

		if( !(EPS_ERR_NONE == ret || EPS_ERR_PRINTER_NOT_FOUND == ret
			|| EPS_ERR_NOT_OPEN_IO == ret) ){
			EPS_RETURN( ret )
		}
	}
#endif

/***------------------------------------------------------------------------------------*/
/*** Net find                                                                           */
/***------------------------------------------------------------------------------------*/
#ifdef GCOMSW_PRT_USE_NETWORK
	if( protocol & EPS_PROTOCOL_NET ){

		ret = FindNetPrinter( protocol, timeout, TRUE, NULL );

	}
#endif /* GCOMSW_PRT_USE_NETWORK */

	if( epsPrinterList.num <= 0 ){
		if(EPS_ERR_NONE == ret ||
		   EPS_ERR_PRINTER_NOT_USEFUL== ret ){
			ret = EPS_ERR_PRINTER_NOT_FOUND;
		}
	} else{
		if(EPS_ERR_PRINTER_NOT_FOUND == ret ||
		   EPS_ERR_PRINTER_NOT_USEFUL== ret ){
			ret = EPS_ERR_NONE;
		}
	}
		
	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   prtCancelFindPrinter()                                              */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* -			void                -                                                   */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE               - Success                                            */
/*      EPS_ERR_INVALID_CALL       - invalid called                                     */
/*                                                                                      */
/* Description:                                                                         */
/*     Cancel epsFindPrinter(), epsProbePrinter().                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE prtCancelFindPrinter(

        void					

){
	EPS_LOG_FUNCIN

	if( epsCmnFnc.lockSync && epsCmnFnc.unlockSync ){
		if( 0 == epsCmnFnc.lockSync() ){
			g_FindBreak = TRUE;
			epsCmnFnc.unlockSync();
		} 
		EPS_RETURN( EPS_ERR_NONE )
	} else{
		EPS_RETURN( EPS_ERR_INVALID_CALL )
	}
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   prtProbePrinterByID()                                               */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* probeParam   EPS_PROBE*          Pointer to a Prober parameter structure             */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success (Opened Communication)                */
/*      EPS_ERR_PRINTER_NOT_FOUND       - printer not found                             */
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_PROTOCOL_NOT_SUPPORTED  - Unsupported function Error                    */
/*                                                                                      */
/* Description:                                                                         */
/*     Probe printer by ID string.                                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE   prtProbePrinterByID (

		const EPS_PROBE*   probeParam

){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE		ret = EPS_ERR_NONE;          /* Return status of internal calls */
    EPS_PRINTER_INN*	innerPrinter = NULL;
	EPS_UINT16          protocol = EPS_PROTOCOL_INVALID;
    EPS_INT8            sParse[EPS_PRNID_BUFFSIZE];
    EPS_INT8*           pPos = NULL;
    EPS_INT32           nSegCnt = 0;
    EPS_UINT32          nTmp = 0;
    EPS_INT8            defStr[EPS_PRNID_BUFFSIZE];

	EPS_LOG_FUNCIN

/*** Initialize Local Variables                                                         */
    ret = EPS_ERR_NONE;

/*** Parse ID String                                                                    */
	strcpy(sParse, probeParam->dsc.identify);

	pPos = strtok(sParse, EPS_PID_SEP);
    for(nSegCnt = 0; pPos != NULL && nSegCnt < EPS_PID_SEGNUM; nSegCnt++){
		switch(nSegCnt){
		case 0:			/* P */
			if( 0 != strcmp(pPos, EPS_PID_PRT) ){
				EPS_RETURN( EPS_ERR_INV_ARG_PRINTER_ID )
			}
			break;

		case 1:			/* Get protocol number */
			sscanf(pPos, "%x", &nTmp);
			if(nTmp == 0){
				EPS_RETURN( EPS_ERR_INV_ARG_PRINTER_ID )
			}
			protocol = (EPS_UINT16)nTmp;
			break;

		case 2:			/* D */
			if( 0 != strcmp(pPos, EPS_PID_DEF) ){
				EPS_RETURN( EPS_ERR_INV_ARG_PRINTER_ID )
			}
			break;

		case 3:			/* Get description string */
			strcpy(defStr, pPos);
			break;
		}

		pPos = strtok(NULL, EPS_PID_SEP);
    }
	if(nSegCnt < EPS_PID_SEGNUM){
		EPS_RETURN( EPS_ERR_INV_ARG_PRINTER_ID )
	}

	/* Reset break flag */
	if( epsCmnFnc.lockSync && epsCmnFnc.unlockSync ){
		epsCmnFnc.lockSync();
		g_FindBreak = FALSE;
		epsCmnFnc.unlockSync();
	}

/*** protocol depend probe                                                              */
	switch( EPS_PRT_PROTOCOL( protocol ) ){
#ifdef GCOMSW_PRT_USE_USB
	case EPS_PROTOCOL_USB:
		if( printJob.commMode & EPS_PROTOCOL_USB ){
			ret = usbProbePrinterByID(defStr, probeParam->timeout, &innerPrinter);
		} else{
			ret = EPS_ERR_PROTOCOL_NOT_SUPPORTED;
		}
		break;
#endif

#ifdef GCOMSW_PRT_USE_LPR
	case EPS_PROTOCOL_LPR:
		if( printJob.commMode & EPS_PROTOCOL_LPR ){
			ret = lprProbePrinterByID(defStr, probeParam->timeout, &innerPrinter);
		} else{
			ret = EPS_ERR_PROTOCOL_NOT_SUPPORTED;
		}
		break;
#endif

#ifdef GCOMSW_PRT_USE_RAW
	case EPS_PROTOCOL_RAW:
		if( printJob.commMode & EPS_PROTOCOL_RAW ){
			ret = rawProbePrinterByID(defStr, probeParam->timeout, &innerPrinter);
		} else{
			ret = EPS_ERR_PROTOCOL_NOT_SUPPORTED;
		}
		break;
#endif

	default:
		ret = EPS_ERR_INV_ARG_PRINTER_ID;
	}

	if(EPS_ERR_NONE == ret){
	    innerPrinter->protocol = protocol | EPS_PRT_DIRECTION(printJob.commMode);
		ret = prtRegPrinter( innerPrinter, TRUE );
		if(EPS_FIND_CANCELED == ret){
			ret = EPS_ERR_NONE;
		}
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   prtProbePrinterByAddr()                                             */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* probeParam   EPS_PROBE*          Pointer to a Prober parameter structure             */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success (Opened Communication)                */
/*      EPS_ERR_PRINTER_NOT_FOUND       - printer not found                             */
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_PROTOCOL_NOT_SUPPORTED  - Unsupported function Error                    */
/*      EPS_ERR_PRINTER_NOT_USEFUL      - received but not usefl                        */
/*                                                                                      */
/* Description:                                                                         */
/*     Probe network printer by IP Address.                                             */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE   prtProbePrinterByAddr (

		const EPS_PROBE*    probeParam

){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE	ret = EPS_ERR_PRINTER_NOT_FOUND; /* Return status of internal calls */
    EPS_INT32       initProtocol = EPS_PRT_PROTOCOL(printJob.commMode);
    EPS_INT32       tgtProtocol = EPS_PROTOCOL_INVALID;

	EPS_LOG_FUNCIN

/*** Validate input parameters                                                          */
	if (NULL == probeParam) {
		EPS_RETURN( EPS_ERR_INV_ARG_PROBEINFO )
	}
	tgtProtocol = EPS_PRT_PROTOCOL(probeParam->dsc.addr.protocol);
	if( (tgtProtocol | initProtocol) != initProtocol){
		EPS_RETURN( EPS_ERR_INV_ARG_COMMMODE )
	}
	g_FindProtocol = tgtProtocol;

	/* Reset break flag */
	if( epsCmnFnc.lockSync && epsCmnFnc.unlockSync ){
		epsCmnFnc.lockSync();
		g_FindBreak = FALSE;
		epsCmnFnc.unlockSync();
	}

	if( IsValidAddress(probeParam->dsc.addr.address) ){
#if defined(GCOMSW_PRT_USE_NETWORK)
		/***--------------------------------------------------------------------------------*/
		/*** If IPAddress, Probe network printer                                            */
		/***--------------------------------------------------------------------------------*/
		if( tgtProtocol & EPS_PROTOCOL_NET ){
			ret = FindNetPrinter(tgtProtocol, probeParam->timeout, FALSE, probeParam->dsc.addr.address);
			if(EPS_FIND_CANCELED == ret){
				ret = EPS_ERR_NONE;
			}
		} else{
			ret =  EPS_ERR_PROTOCOL_NOT_SUPPORTED;
		}
#else
		ret =  EPS_ERR_PROTOCOL_NOT_SUPPORTED;
#endif /* GCOMSW_PRT_USE_NETWORK */
	} else{
		ret =  EPS_ERR_INV_ARG_PRINTER_ADDR;
	}

	if( epsPrinterList.num <= 0 ){
		if(EPS_ERR_NONE == ret){
			ret = EPS_ERR_PRINTER_NOT_FOUND;
		}
	} else{
		if(EPS_ERR_PRINTER_NOT_FOUND == ret ||
		   EPS_ERR_PRINTER_NOT_USEFUL== ret ){
			ret = EPS_ERR_NONE;
		}
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     prtSetupJobFunctions()                                            */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* printer          const EPS_PRINTER*      I: Pointer to a target printer info.        */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*                                                                                      */
/* Description:                                                                         */
/*      construct job function table.                                                   */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE prtSetupJobFunctions (
								   
		const EPS_PRINTER_INN*	printer, 
		EPS_JOB_FUNCS*		    pFuncs
		
){
/*** Declare Variable Local to Routine                                                  */
	EPS_ERR_CODE		ret = EPS_ERR_NONE;         /* Return status of internal calls  */

	EPS_LOG_FUNCIN

	switch( EPS_PRT_PROTOCOL( printer->protocol ) ){
#ifdef GCOMSW_PRT_USE_USB
	case EPS_PROTOCOL_USB:
		pFuncs->StartJob        = &usbStartJob;
		pFuncs->EndJob          = &usbEndJob;
		pFuncs->RestartJob		= &usbRestartJob;
		pFuncs->WriteData       = &usbWritePrintData;
		pFuncs->ResetPrinter    = &usbResetPrinter;
		pFuncs->MonitorStatus	= &usbGetJobStatus;
		pFuncs->GetStatus		= &usbGetStatus;
		break;
#endif

#ifdef GCOMSW_PRT_USE_LPR
	case EPS_PROTOCOL_LPR:
		pFuncs->StartJob        = &lprStartJob;
		pFuncs->EndJob          = &lprEndJob;
		pFuncs->RestartJob		= &lprRestartJob;
		pFuncs->WriteData       = &lprWritePrintData;
		pFuncs->ResetPrinter    = &lprResetPrinter;
		pFuncs->StartPage       = &lprStartPage;
		pFuncs->EndPage         = &lprEndPage;
		pFuncs->MonitorStatus	= &lprGetJobStatus;
		pFuncs->GetStatus		= &lprGetStatus;
		lprSetupSTFunctions(printer);
		break;
#endif

#ifdef GCOMSW_PRT_USE_RAW
	case EPS_PROTOCOL_RAW:
		pFuncs->StartJob        = &rawStartJob;
		pFuncs->EndJob          = &rawEndJob;
		pFuncs->RestartJob		= &rawRestartJob;
		pFuncs->WriteData       = &rawWritePrintData;
		pFuncs->ResetPrinter    = &rawResetPrinter;
		pFuncs->StartPage       = &rawStartPage;
		pFuncs->EndPage         = &rawEndPage;
		pFuncs->MonitorStatus	= &rawGetJobStatus;
		pFuncs->GetStatus		= &rawGetStatus;
		rawSetupSTFunctions(printer);
		break;
#endif

	default:
		ret = EPS_ERR_OPR_FAIL;
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   prtGetInfo()                                                        */
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
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PROTOCOL_NOT_SUPPORTED  - Unsupported function Error                    */
/*                                                                                      */
/* Description:                                                                         */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE     prtGetInfo (

        const EPS_PRINTER_INN*  printer,
		EPS_INT32               type,
        EPS_UINT8**             buffer,
		EPS_INT32*              bufSize

){
/*** Declare Variable Local to Routine                                                  */
	EPS_ERR_CODE     ret = EPS_ERR_NONE;           /* Return status of internal calls   */

	EPS_LOG_FUNCIN

/*** Validate communication mode                                                        */
	if ( !EPS_IS_BI_PROTOCOL(printer->protocol) ){
        EPS_RETURN( EPS_ERR_NEED_BIDIRECT )
	}

	switch( EPS_PRT_PROTOCOL( printer->protocol ) ){
#ifdef GCOMSW_PRT_USE_USB
	case EPS_PROTOCOL_USB:
		ret = usbGetInfo(printer, type, buffer, bufSize);
		break;
#endif

#ifdef GCOMSW_PRT_USE_LPR
	case EPS_PROTOCOL_LPR:
		ret = lprGetInfo(printer, type, buffer, bufSize);
		break;
#endif

#ifdef GCOMSW_PRT_USE_RAW
	case EPS_PROTOCOL_RAW:
		ret = rawGetInfo(printer, type, buffer, bufSize);
		break;
#endif

	default:
		ret = EPS_ERR_OPR_FAIL;
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   prtRegPrinter()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printer      EPS_PRINTER*        I:Data structure containing printer information.  	*/
/* bNotify      EPS_BOOL            I:This item need notify.                            */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE				- Printer settled into one of the requested state(s)*/
/*      EPS_ERR_OPR_FAIL			- Internal Error                                    */
/*      EPS_ERR_MEMORY_ALLOCATION   - Failed to allocate memory                         */
/*      EPS_FIND_CANCELED           - cancel find by user                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Registers a new printer with the global linked list managed by this module.		*/
/*		And Notify to user.                                                             */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE prtRegPrinter(
						   
		EPS_PRINTER_INN* printer,
		EPS_BOOL		 bNotify

){
/*** Declare Variables Local to Routine */
	EPS_PRINTER		usrPrinter;						/* for user notify                  */
	EPS_PL_NODE*	current = NULL;
	EPS_BOOL		registered = FALSE;

	EPS_LOG_FUNCIN

	EPS_DBGPRINT( ("%X / %s\n", printer->protocol, printer->location) )

	if (printer->protocol == EPS_PROTOCOL_INVALID){
		EPS_RETURN( EPS_ERR_OPR_FAIL )
	}

	if( NULL == epsPrinterList.root){
        /*** This is the first printer in the list.										*/
		epsPrinterList.root = (EPS_PL_NODE*)EPS_ALLOC(sizeof(EPS_PL_NODE));
		if(epsPrinterList.root == NULL) {
			EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
		}
		epsPrinterList.num = 1;

		current = epsPrinterList.root;
		current->printer = printer;
		current->next = NULL;

	} else {

		current = epsPrinterList.root;
		while( current ){
			/* If we've found a printer in the list that matches the module's printer,		*/
			if( 0 == strcmp(printer->location, current->printer->location)){
				if(0 == strcmp(printer->modelName, current->printer->modelName)){
					registered = TRUE;
				}
			}

			if (registered ){
				
				/* And the new protocol has priority over the old one...					*/
				if (EPS_PRT_PROTOCOL(current->printer->protocol) > EPS_PRT_PROTOCOL(printer->protocol) ) {
					/* Prioritize the protocols. The lower-numbered protocol always			*/
					/* has precendence over the higher-numbered one (1 outranks 3).			*/
					current->printer->protocol = printer->protocol;

					/* Replase protocol */
					EPS_DBGPRINT( ("Discover %d %s %s (Upgrade)\n", printer->protocol, printer->modelName, printer->printerID) )
					EPS_SAFE_RELEASE( current->printer->protocolInfo );
					EPS_SAFE_RELEASE( current->printer );
					current->printer = printer;
					break;
				} else{
 					/* In this case, the current protocol already has priority. Simply return.	*/
					EPS_SAFE_RELEASE( printer->protocolInfo );
					EPS_SAFE_RELEASE( printer );
					EPS_RETURN( EPS_ERR_NONE )
				}
			}

			if( NULL == current->next){
				break;			
			}
			current = current->next;
		}

		/* If we've reached this part of the code, it means we haven't found the printer.	*/
		/* Time to add a new node to the list.												*/
		/* The variable "current" is right now pointing to the last member of the list.		*/
		if( !registered ){
			/* Allocate a new printer on the end of the linked list.							*/
			current->next = (EPS_PL_NODE*)EPS_ALLOC(sizeof(EPS_PL_NODE));
			if(current->next == NULL) {
				EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
			}
			
			/* Fill in the list node.           												*/
			current->next->printer = printer;
			current->next->next = NULL;
			epsPrinterList.num++;

			current = current->next;
		}
	}

	EPS_DBGPRINT( ("Discover x%X (x%X) %s %s\n", printer->protocol, printer->supportFunc, printer->modelName, printer->printerID) )

	if( TRUE == bNotify ){
		MakePrinterStructure(&usrPrinter, printer);

		if( EPS_USER_CANCEL == epsCmnFnc.findCallback(usrPrinter) ){
			EPS_RETURN( EPS_FIND_CANCELED )
		}
	}

	EPS_RETURN( EPS_ERR_NONE )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   prtIsRegistered()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* Address      EPS_INT8*           IP Address.											*/
/* modelName    EPS_INT8*           Printer model name.									*/
/* Protocol     EPS_INT32           Protocol ID.										*/
/*                                                                                      */
/* Return value:                                                                        */
/* TRUE (registered), FALSE(not registered).											*/
/*                                                                                      */
/* Description:                                                                         */
/*      Whether the printer has registered is confirmed.								*/
/*                                                                                      */
/*******************************************|********************************************/
EPS_BOOL prtIsRegistered(
						 
		const EPS_INT8 *Address, 
		const EPS_INT8 *modelName,
		EPS_INT32 Protocol

){
	EPS_PL_NODE* current = epsPrinterList.root;
	EPS_BOOL	 bMatch = FALSE;

	EPS_LOG_FUNCIN

	while(NULL != current) {
		/* If we've found a printer in the list that matches the module's printer,		*/
		bMatch = FALSE;
		if( 0 == strcmp(current->printer->location, Address) ){
			if(NULL != modelName){
				if( 0 == strcmp(current->printer->modelName, modelName) ){
					bMatch = TRUE;
				}
			} else{
				bMatch = TRUE;
			}
		}

		if(TRUE == bMatch){
			/* And the new protocol has priority over the old one...					*/
			if (EPS_PRT_PROTOCOL(current->printer->protocol) <= EPS_PRT_PROTOCOL(Protocol)) {
				EPS_RETURN( TRUE )
			} else{
				/* already registered However it's low level protocol					*/
				EPS_RETURN( FALSE )
			}
		}

		current = current->next;
	}

	EPS_RETURN( FALSE )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   prtAddUsrPrinter()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* usrPrinter   EPS_PRINTER*        I: User specified printer.							*/
/* printer      EPS_PRINTER*        O: Internal printer information.					*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_INV_ARG_COMMMODE        - Invalid argument "usrPrinter.protocol"        */
/*      EPS_ERR_INV_ARG_PRINTER_ADDR    - Invalid format " printer.location"            */
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_INV_PRINT_LANGUAGE      - Invalid argument "usrPrinter.language"        */
/*                                                                                      */
/* Description:                                                                         */
/*      Register a user specified printer.				                                */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE prtAddUsrPrinter(
			
		const EPS_PRINTER*  usrPrinter,
		EPS_PRINTER_INN**	printer

){
    EPS_ERR_CODE		ret = EPS_ERR_NONE;
    EPS_INT32			initProtocol = EPS_PRT_PROTOCOL(printJob.commMode);
    EPS_INT32			tgtProtocol = EPS_PRT_PROTOCOL(usrPrinter->protocol);
	EPS_INT8			address[EPS_ADDR_BUFFSIZE];
	EPS_INT8*			p = NULL;
	EPS_INT32			nPort = 0;
	EPS_PRINTER_INN*	innerPrinter = NULL;

	EPS_LOG_FUNCIN

	*printer = NULL;

	/*** Validate input parameters */
	if( (tgtProtocol | initProtocol) != initProtocol){	/* Not Init protocol */
		EPS_RETURN( EPS_ERR_INV_ARG_COMMMODE )
	}
	if( tgtProtocol & EPS_PROTOCOL_USB ){				/* USB */
		EPS_RETURN( EPS_ERR_INV_ARG_COMMMODE )
	}
	if( memGetBitCount(tgtProtocol) > 1 ){				/* Multi protocol */
		EPS_RETURN( EPS_ERR_INV_ARG_COMMMODE )
	}
	if( '\0' == usrPrinter->location[0] ){
		EPS_RETURN( EPS_ERR_INV_ARG_PRINTER_ADDR )
	}
	if( EPS_ADDR_BUFFSIZE < strlen(usrPrinter->location) ){
		EPS_RETURN( EPS_ERR_INV_ARG_PRINTER_ADDR )
	}

	if( !(EPS_LANG_ESCPR			== usrPrinter->language ||
#ifdef GCOMSW_CMD_ESCPAGE_S
		  EPS_LANG_ESCPAGE_S		== usrPrinter->language ||
#endif
#ifdef GCOMSW_CMD_PCL
		  EPS_LANG_PCL				== usrPrinter->language ||
		  EPS_LANG_PCL_COLOR		== usrPrinter->language ||
#endif
		  EPS_LANG_ESCPAGE			== usrPrinter->language ||
		  EPS_LANG_ESCPAGE_COLOR	== usrPrinter->language  ) ){
		EPS_RETURN( EPS_ERR_INV_PRINT_LANGUAGE )
	}

	/*** pase location */
	strcpy(address, usrPrinter->location);
	p = strchr(address, ':');
	if( NULL != p ){
		*p = '\0';
		p++;					/* 1 = ':' */
		sscanf(p, "%d", &nPort);
	} else{
		/* Default Port */
		if( tgtProtocol & EPS_PROTOCOL_LPR ){
#ifdef GCOMSW_PRT_USE_LPR
			nPort = lprGetDefautiPort();
#else
			EPS_RETURN( EPS_ERR_INV_ARG_COMMMODE )
#endif
		} else if( tgtProtocol & EPS_PROTOCOL_RAW ){
#ifdef GCOMSW_PRT_USE_RAW
			nPort = rawGetDefautiPort();
#else
			EPS_RETURN( EPS_ERR_INV_ARG_COMMMODE )
#endif
		} else{
			EPS_RETURN( EPS_ERR_INV_ARG_COMMMODE )
		}
	}
	if( FALSE == IsValidAddress( address ) ){
		EPS_RETURN( EPS_ERR_INV_ARG_PRINTER_ADDR )
	}

	/*** Create new printer */
	innerPrinter = (EPS_PRINTER_INN*)EPS_ALLOC(sizeof(EPS_PRINTER_INN));
	if( NULL == innerPrinter ){
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
	}

	memset(innerPrinter, 0x00, sizeof(EPS_PRINTER_INN));

	strcpy(innerPrinter->location, address);
	innerPrinter->printPort		= (EPS_UINT16)nPort;
	innerPrinter->protocol		= usrPrinter->protocol;
	innerPrinter->supportFunc	= EPS_SPF_RGBPRINT | EPS_SPF_JPGPRINT;
    innerPrinter->JpgMax		= EPS_JPEG_SIZE_UNLIMIT;
	innerPrinter->language		= usrPrinter->language;
	strcpy(innerPrinter->modelName,	usrPrinter->modelName);
	/* innerPrinter->manufacturerName */
	/* innerPrinter->friendlyName */
	/* innerPrinter->printerID; */
	/* innerPrinter->protocolInfo; */

	/*** Append to printer lsit */
	ret = prtRegPrinter(innerPrinter, FALSE);
	if( EPS_ERR_NONE == ret ){
		*printer = innerPrinter;
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     prtGetInnerPrinter()                                              */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* printer          EPS_PRINTER             Pointer to public infomation structure      */
/*                                                                                      */
/* Return value:                                                                        */
/* EPS_PRINTER_INN                          Pointer to private infomation structure     */
/*                                                                                      */
/* Description:                                                                         */
/*      Get private infomation of printer.                                              */
/*                                                                                      */
/*******************************************|********************************************/
EPS_PRINTER_INN* prtGetInnerPrinter(
			
		const EPS_PRINTER*  printer

){
    EPS_PL_NODE*		pCur = epsPrinterList.root;
	EPS_PRINTER_INN*	innerPrinter = NULL;

    /* collates it with found printer */
	while(NULL != pCur){
		if(0 == strcmp(printer->modelName, pCur->printer->modelName)
			&&  0 == strcmp(printer->location, pCur->printer->location) ){
			innerPrinter = pCur->printer;
			break;
		}
		pCur = pCur->next;
	} 

	return innerPrinter;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     prtClearPrinterList()                                             */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* void                                                                                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      void                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Crean up inside list of printer structure .                                     */
/*                                                                                      */
/*******************************************|********************************************/
void prtClearPrinterList(
						 
		void
		
){
	EPS_PL_NODE* pCur = epsPrinterList.root;
	EPS_PL_NODE* pNext = NULL;

	EPS_LOG_FUNCIN

	/* Clear stock printer list */
	while(NULL != pCur){
		pNext = pCur->next;

		prtClearPrinterAttribute(pCur->printer);

		EPS_SAFE_RELEASE( pCur->printer->protocolInfo );
		EPS_SAFE_RELEASE( pCur->printer );
		EPS_SAFE_RELEASE( pCur );

		pCur = pNext;
	} 

	epsPrinterList.num = 0;
	epsPrinterList.root = NULL;

 	EPS_RETURN_VOID
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     prtClearPrinterAttribute()                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* printer          EPS_PRINTER_INN*		I: Pointer to a printer info structure      */
/*                                                                                      */
/* Return value:                                                                        */
/*      void                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Crean up inside list of supported media structure.                              */
/*                                                                                      */
/*******************************************|********************************************/
void prtClearPrinterAttribute(
							
	EPS_PRINTER_INN*  printer

){
	EPS_LOG_FUNCIN

    /* Clear "supportedMedia"                                              */
	if( NULL == printer ){
		return;
	}

	epspmClearPrintAreaInfo(&printer->printAreaInfo);

#ifdef GCOMSW_CMD_ESCPAGE
	if(EPS_LANG_ESCPR == printer->language ){
#endif
		/*** ESC/P-R ***/
		epspmClearMediaInfo(&printer->supportedMedia);
		EPS_SAFE_RELEASE(printer->pmData.pmString);
#ifdef GCOMSW_CMD_ESCPAGE
	} else{
		/*** ESC/Page ***/
		pageClearSupportedMedia(printer);
	}
#endif

	printer->supportedMedia.numSizes = 0;
	printer->supportedMedia.JpegSizeLimit = EPS_JPEG_SIZE_MAX;  /* lowest guarantee */
	printer->supportedMedia.resolution = EPS_IR_360X360;

 	EPS_RETURN_VOID
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     prtSetIdStr()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* printer          EPS_PRINTER_INN*		I/O: Pointer to a printer info structure    */
/* idString         EPS_INT8*				I: protocol depended unique string          */
/*                                                                                      */
/* Return value:                                                                        */
/*      void                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Set the Printer ID.                                                             */
/*                                                                                      */
/*******************************************|********************************************/
void prtSetIdStr(
				 
			EPS_PRINTER_INN* printer, 
			const EPS_INT8* idString
			
){
	sprintf(printer->printerID, EPS_PRINTER_ID_STR, 
        EPS_PRT_PROTOCOL_EX(printer->protocol), idString);

}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   prtRecoverPE()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* void         -                   -                                                   */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Recovered successfully                        */
/*      EPS_ERR_COMM_ERROR              - Recovery failed                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Tries to recover from the paper end error                                       */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    prtRecoverPE (

        void

){
    EPS_ERR_CODE ret;
    
   	EPS_LOG_FUNCIN

	switch( EPS_PRT_PROTOCOL( printJob.printer->protocol ) ){
#ifdef GCOMSW_PRT_USE_USB
	case EPS_PROTOCOL_USB:
		ret = usbMechCommand(EPS_CBTCOM_PE);
		break;
#endif

#ifdef GCOMSW_PRT_USE_LPR
	case EPS_PROTOCOL_LPR:
		ret = lprMechCommand(printJob.printer, EPS_CBTCOM_PE);
		break;
#endif

#ifdef GCOMSW_PRT_USE_RAW
	case EPS_PROTOCOL_RAW:
		ret = rawMechCommand(printJob.printer, EPS_CBTCOM_PE);
		break;
#endif
	default:
		EPS_RETURN( EPS_ERR_NONE )       /* Other protocol Not Support recover command */
	}

    EPS_RETURN( ret )
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------               Local Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
#ifdef GCOMSW_PRT_USE_NETWORK
/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   FindNetPrinter()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* protocol     EPS_INT32           I: traget protocol                                  */
/* timeout      EPS_UINT32          I: find timeout                                     */
/* plural       EPS_BOOL            I: TRUE= find plural printer / FALSE= one printer   */
/* address      EPS_INT8*           I: send discover message address.                   */
/*                                     (If plural=TRUE, this is NULL)                   */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                - Success (Opened Communication)                    */
/*      EPS_ERR_MEMORY_ALLOCATION   - Fail to memory allocation                         */
/*      EPS_ERR_PRINTER_NOT_FOUND	- Printer not found (or error occur)		        */
/*      EPS_ERR_COMM_ERROR          - socket error                                      */
/*                                                                                      */
/*      EPS_FIND_CANCELED           - user discontinued it                              */
/*                                                                                      */
/* Description:                                                                         */
/*     Find network printers.                                                           */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE FindNetPrinter (

		EPS_INT32           protocol, 
        EPS_UINT32          timeout,
        EPS_BOOL            plural,
		const EPS_INT8*		address

){
    EPS_ERR_CODE    ret = EPS_ERR_PRINTER_NOT_FOUND; /* Return status of internal calls */
	EPS_PRINTER_INN*	innerPrinter = NULL;

	EPS_FIND_FUNCS* pProtocolFncs = NULL;
	EPS_FIND_FUNCS* pFncs = NULL;
	EPS_UINT32		tmStart, tmNow, tmSpan, tmReq;
	EPS_INT32		nProtocolCnt = 0;
	EPS_INT32		nCnt = 0;
	EPS_BOOL		bBreak = FALSE;

	EPS_INT32		ifNum = 0;
	EPS_UINT8*		ifBuf = NULL;
#if LCOMSW_USE_MULTI_IF
	EPS_UINT32		ifBufSize = 0;
#endif
	EPS_INT32		ifCnt = 0;

	EPS_LOG_FUNCIN

	if(epsCmnFnc.getTime){
		tmStart = epsCmnFnc.getTime();
		tmNow = tmSpan = tmReq = 0;
	} else{
		timeout = tmStart = tmNow = tmSpan = tmReq = 0;
	}

#if LCOMSW_USE_MULTI_IF
	if(plural){
		ifNum = epsNetFnc.enumInterface(NULL, ifBufSize);
		if(ifNum > 0){
			ifBufSize = ifNum * EPS_IFNAME_LEN;
			ifBuf = (EPS_UINT8*)EPS_ALLOC(ifBufSize);
			if(NULL == ifBuf){
				EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
			}
			memset(ifBuf, 0, ifBufSize);
			ifNum = epsNetFnc.enumInterface(ifBuf, ifBufSize);
		}
		if(ifNum <= 0){
			EPS_SAFE_RELEASE( ifBuf );
			EPS_RETURN( EPS_ERR_COMM_ERROR )
		}
	} else{
		ifNum = 1;
	}
#else
	ifNum = 1;
#endif

#ifdef GCOMSW_PRT_USE_LPR
	if(protocol & EPS_PROTOCOL_LPR) nProtocolCnt += ifNum;
#endif
#ifdef GCOMSW_PRT_USE_RAW
	if( (protocol & EPS_PROTOCOL_RAW) && !(protocol & EPS_PROTOCOL_LPR) ) nProtocolCnt += ifNum;
#endif
	if(0 == nProtocolCnt){
		EPS_SAFE_RELEASE( ifBuf );
		EPS_RETURN( EPS_ERR_PRINTER_NOT_FOUND )
	}

	/* Setup net Find functions */
	pProtocolFncs = (EPS_FIND_FUNCS*)EPS_ALLOC(sizeof(EPS_FIND_FUNCS)*nProtocolCnt);
    if(NULL == pProtocolFncs){
		EPS_SAFE_RELEASE( ifBuf );
        EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
    }
	memset(pProtocolFncs, 0, sizeof(EPS_FIND_FUNCS)*nProtocolCnt);
	pFncs = pProtocolFncs;

#ifdef GCOMSW_PRT_USE_LPR
	if(protocol & EPS_PROTOCOL_LPR){
		for(ifCnt = 0; ifCnt < ifNum; ifCnt++){
			pFncs->fncStart = &lprFindStart;
			pFncs->fncCheck = &lprFind;
			pFncs->fncEnd = &lprFindEnd;
			pFncs->sock = EPS_INVALID_SOCKET;
			if(plural){
				pFncs->ifName = ifBuf+(ifCnt*EPS_IFNAME_LEN);
				strcpy(pFncs->address, EPSNET_UDP_BROADCAST_ADDR);
			} else{
				strcpy(pFncs->address, address);
			}
			pFncs++;
		}
	}
#endif
#ifdef GCOMSW_PRT_USE_RAW
	if( (protocol & EPS_PROTOCOL_RAW) && !(protocol & EPS_PROTOCOL_LPR) ){
		for(ifCnt = 0; ifCnt < ifNum; ifCnt++){
			pFncs->fncStart = &rawFindStart;
			pFncs->fncCheck = &rawFind;
			pFncs->fncEnd = &rawFindEnd;
			pFncs->sock = EPS_INVALID_SOCKET;
			if(plural){
				pFncs->ifName = ifBuf+(ifCnt*EPS_IFNAME_LEN);
				strcpy(pFncs->address, EPSNET_UDP_BROADCAST_ADDR);
			} else{
				strcpy(pFncs->address, address);
			}
			pFncs++;
		}
	}
#endif

	/* Start (send discover message) */
	pFncs = pProtocolFncs;
	ret = EPS_ERR_NONE;
	for(nCnt = 0; (nCnt < nProtocolCnt) && (ret == EPS_ERR_NONE); nCnt++, pFncs++){
		ret = pFncs->fncStart( &(pFncs->sock), pFncs->address, plural, pFncs->ifName );
	}

	/* Check response */
	if(	EPS_ERR_NONE == ret){
		ret = EPS_ERR_PRINTER_NOT_FOUND;
	}
	while( (EPS_ERR_PRINTER_NOT_FOUND == ret || EPS_ERR_PRINTER_NOT_USEFUL == ret) && !bBreak ){
		pFncs = pProtocolFncs;
		for(nCnt = 0; nCnt < nProtocolCnt; ){
			innerPrinter = NULL;
			ret = pFncs->fncCheck(pFncs->sock, &innerPrinter);
			
			if( EPS_COM_NOT_RECEIVE == ret ){
				ret = EPS_ERR_PRINTER_NOT_FOUND;
				/* next protocol */
				nCnt++;
				pFncs++;
			} else{
				tmReq = 0;
			}

			if( EPS_ERR_NONE == ret ){
				innerPrinter->protocol |= EPS_PRT_DIRECTION(printJob.commMode);
				
				ret = prtRegPrinter( innerPrinter, TRUE );

				if(EPS_FIND_CANCELED == ret){
					break;
				} else if(FALSE == plural){
					/* find one printer */
					bBreak = TRUE;
					break;
				}
			} else if(EPS_ERR_PRINTER_NOT_FOUND == ret ){
			} else if(EPS_ERR_PRINTER_NOT_USEFUL == ret ){
				if(FALSE == plural){
					/* find one printer */
					bBreak = TRUE;
					break;
				}
			} else{
				bBreak = TRUE;
				break;
			}

			/* epsCancelFindPriter() */
			if( epsCmnFnc.lockSync && epsCmnFnc.unlockSync ){
				if( 0 == epsCmnFnc.lockSync() ){
					if( g_FindBreak ){
						epsCmnFnc.unlockSync();
						bBreak = TRUE;
						break;
					}
					epsCmnFnc.unlockSync();
				}
			}

			/* Timeout */
			if(timeout > 0){
				tmNow = epsCmnFnc.getTime();
				tmSpan = (EPS_UINT32)(tmNow - tmStart);
				/*EPS_DBGPRINT( ("TM %u - %u <> %u\n", tmNow, tmStart, tmSpan) )*/
				if( tmSpan >= timeout ){
					bBreak = TRUE;
					break;
				}
			}
		}
		if(bBreak)break;

		/* epsCancelFindPriter() */
		if( epsCmnFnc.lockSync && epsCmnFnc.unlockSync ){
			if( 0 == epsCmnFnc.lockSync() ){
				if( g_FindBreak ){
					epsCmnFnc.unlockSync();
					bBreak = TRUE;
					break;
				}
				epsCmnFnc.unlockSync();
			}
		}

		/* re isuue request */
		/*EPS_DBGPRINT( ("TM %u - %u <> %u\n", tmNow, tmStart, tmSpan) )*/
		if( (EPS_ERR_PRINTER_NOT_FOUND == ret ||
			 EPS_ERR_PRINTER_NOT_USEFUL == ret) && epsCmnFnc.getTime )
		{
			tmNow = epsCmnFnc.getTime();
			if( 0 == tmReq ){
				tmReq = tmNow;
			} else{
				/* beef up */
				if( EPSNET_FIND_REREQUEST_TIME <= (EPS_UINT32)(tmNow - tmReq) ){
					/*EPS_DBGPRINT( ("beef up TM %u - %u <> %u\n", tmNow, tmReq, (EPS_UINT32)(tmNow - tmReq)) )*/
					pFncs = pProtocolFncs;
					ret = EPS_ERR_NONE;
					for(nCnt = 0; (nCnt < nProtocolCnt) && (ret == EPS_ERR_NONE); nCnt++, pFncs++){
						ret = pFncs->fncStart( &(pFncs->sock), pFncs->address, plural, pFncs->ifName );
					}
					tmReq = 0;
					if(	EPS_ERR_NONE == ret){
						ret = EPS_ERR_PRINTER_NOT_FOUND;
					}
				}
			}
		}
	}

	/* End */
	pFncs = pProtocolFncs;
	for(nCnt = 0; (nCnt < nProtocolCnt); nCnt++, pFncs++){
		pFncs->fncEnd( pFncs->sock );
	}

	EPS_SAFE_RELEASE(pProtocolFncs);
	EPS_SAFE_RELEASE( ifBuf );

	EPS_RETURN( ret )
}

#endif	/* GCOMSW_PRT_USE_NETWORK */


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     MakePrinterStructure()	                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* dst	        EPS_PRINTER*        O: Printer information                              */
/* src	        EPS_PRINTER_INN*    I: Printer information internal                     */
/*                                                                                      */
/* Return value:                                                                        */
/*      void                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Copy printer information.                                                       */
/*                                                                                      */
/*******************************************|********************************************/
static void MakePrinterStructure(
		
		EPS_PRINTER* dst,
		EPS_PRINTER_INN* src

){
	EPS_DBGPRINT(("%s : %d\n", src->modelName, src->protocol))
	dst->protocol					= EPS_PRT_PROTOCOL(src->protocol);
	dst->supportFunc				= src->supportFunc;
	dst->language					= src->language;
	strcpy(dst->location,			src->location);
	strcpy(dst->macAddress,			src->macAddress);
	strcpy(dst->manufacturerName,	src->manufacturerName);
	strcpy(dst->modelName,			src->modelName);
	strcpy(dst->friendlyName,		src->friendlyName);
	strcpy(dst->serialNo,			src->serialNo);
	strcpy(dst->printerID,			src->printerID);
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     IsValidAddress()		                                            */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* addr	        EPS_INT8*           I: IP Address                                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      TRUE                        - valid                                             */
/*      FALSE                       - invalid            		                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Validation of IP(v4) Address.                                                   */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_BOOL IsValidAddress(
					   
		const EPS_INT8* addr
		
){
#define EPS_IPSEGNUM   (4)

	EPS_UINT32 nSeg[EPS_IPSEGNUM];
	EPS_INT16  nCnt;

	memset(nSeg, 0xFF, sizeof(EPS_UINT32)*EPS_IPSEGNUM);

	sscanf(addr, "%d.%d.%d.%d", &nSeg[0], &nSeg[1], &nSeg[2], &nSeg[3]);

	for(nCnt = 0; nCnt < EPS_IPSEGNUM; nCnt++){
		if(nSeg[nCnt] > 255){
			return FALSE;
		}
	}

	return TRUE;
}

/*________________________________  epson-protocol.c   _________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

