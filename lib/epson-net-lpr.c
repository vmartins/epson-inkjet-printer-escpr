/*________________________________   epson-net-lpr.c   _________________________________*/

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
/*                                 LPR protocol Module                                  */
/*                                                                                      */
/*                                Public Function Calls                                 */
/*                              --------------------------                              */
/*      EPS_ERR_CODE lprFindStart       (sock, address, flg                     );      */
/*		EPS_ERR_CODE lprFind            (sock, printer                          );      */
/*      EPS_ERR_CODE lprFindEnd	        (sock                                   );      */
/*      EPS_ERR_CODE lprProbePrinterByID(printerUUID, timeout, printer          );      */
/*		EPS_ERR_CODE lprStartJob	    (printr                                 );      */
/*      EPS_ERR_CODE lprEndPage	        (                                       );      */
/*      EPS_ERR_CODE lprResetPrinter    (                                       );      */
/*      EPS_ERR_CODE lprWritePrintData  (buffer, bufferlen, sendlen             );      */
/*      EPS_ERR_CODE lprGetStatus       (printer, status, ioStatus              );      */
/*      EPS_ERR_CODE lprGetJobStatus    (pstInfo			                    );      */
/*      EPS_ERR_CODE lprGetInfo         (printer, pString, bufSize              );      */
/*      EPS_ERR_CODE lprMechCommand     (printer, Command                       );      */
/*                                                                                      */
/*******************************************|********************************************/


/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-def.h"
#include "epson-escpr-err.h"
#include "epson-escpr-mem.h"
#include "epson-escpr-services.h"
#include "epson-protocol.h"
#include "epson-net-snmp.h"
#include "epson-net-lpr.h"

/*-----------------------------  Local Macro Definitions -------------------------------*/
/*******************************************|********************************************/
#ifdef EPS_LOG_MODULE_LPR
#define EPS_LOG_MODULE	EPS_LOG_MODULE_LPR
#else
#define EPS_LOG_MODULE	0
#endif

#define LPR_PORT_NUM			(515)				/* Protocol port number				*/
#define LPR_MAX_BUF				(512)				/* Communication buffer size		*/
#define LPR_DUMMY_DATA_SIZE		("1073741824000")	/* Enhanced LPR buffer size			*/
#define LPR_HOST_NAME           "escpr-lib"         /* LPR print host name              */

#define IS_JOB_ACTIVE																		\
		( (NULL != printJob.printer) && (NULL != printJob.hProtInfo)						\
			&& (EPS_PROTOCOL_LPR == EPS_PRT_PROTOCOL(printJob.printer->protocol)) )

#define IS_VALID_DATA_SESSION																\
		( (NULL != printJob.printer) && (NULL != printJob.hProtInfo)						\
			&& (EPS_PROTOCOL_LPR == EPS_PRT_PROTOCOL(printJob.printer->protocol))			\
			&& (EPS_INVALID_SOCKET != ((EPS_PRINT_JOB_LPR*)printJob.hProtInfo)->socData) )

/*----------------------------  ESC/P-R Lib Global Variables  --------------------------*/
/*******************************************|********************************************/

	/*** Extern Function                                                                */
extern EPS_NET_FUNC    epsNetFnc;
extern EPS_CMN_FUNC    epsCmnFnc;

    /*** Print Job Structure                                                            */
    /*** -------------------------------------------------------------------------------*/
extern EPS_PRINT_JOB   printJob;

static EPS_SNMP_FUNCS	snmp;

/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/
typedef struct _tagEPS_PRINT_JOB_LPR_ {
	EPS_SOCKET	socData;					/* LPR socket for send data                 */
	EPS_SOCKET	socStat;					/* SNMP socket for check status             */
	EPS_BOOL	reseted;					/* reseted                                  */
	EPS_BOOL	pageend;					/* page end - start                         */
} EPS_PRINT_JOB_LPR;

/*--------------------------------  LPR Local Variables  -------------------------------*/
/*******************************************|********************************************/

/*--------------------------  Local Functions Declaration   ----------------------------*/
/*******************************************|********************************************/
static EPS_ERR_CODE StartDataSession	(EPS_PRINT_JOB_LPR*, const EPS_PRINTER_INN*     );
static EPS_ERR_CODE EndDataSession      (EPS_PRINT_JOB_LPR*                             );    

static EPS_INT16	GetJobId			(void                                           );


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------              Public Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void		lprSetupSTFunctions	(
								 
		 const EPS_PRINTER_INN*	printer
		 
){
	EPS_LOG_FUNCIN

	snmpSetupSTFunctions(&snmp, printer);

	EPS_RETURN_VOID
}


EPS_UINT16	lprGetDefautiPort    (
								  
		void
		
){
	return LPR_PORT_NUM;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprFindStart()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* sock		    EPS_SOCKET*         O: send discovery message socket                    */
/* address		EPS_INT8*           I: Destination address                              */
/* multi		EPS_BOOL            I: If TRUE, send multicast                          */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE					- Success                                       */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Sends EPSNET_NUM_DISCOVERIES number of upnp discovery message to find			*/
/*		devices.  Discovery message is sent more than once because udp is unreliable.   */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprFindStart(

		EPS_SOCKET*		sock,
		const EPS_INT8*	address,
		EPS_BOOL        multi,
		const EPS_UINT8*  mac
		
){
	EPS_LOG_FUNCIN
	EPS_RETURN( snmpFindStart(sock, address, multi, mac) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprFind()	                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* sock		    EPS_SOCKET          I: Discover Socket			                        */
/* printer		EPS_PRINTER_INN**   O: pointer for found printer structure              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_COMM_ERROR   	        - socket error		                            */
/*      EPS_ERR_MEMORY_ALLOCATION							                            */
/*      EPS_ERR_PRINTER_NOT_FOUND       - printer not found                             */
/*      EPS_ERR_PRINTER_NOT_USEFUL      - received but not usefl                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Receive discover messasge. Get printer name.                                    */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprFind     (
						  
		EPS_SOCKET			sock,
		EPS_PRINTER_INN**   printer
		
){
	EPS_LOG_FUNCIN
	EPS_RETURN( snmpFind(sock, LPR_PORT_NUM, EPS_PROTOCOL_LPR, printer) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprFindEnd() 				                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* sock		    EPS_SOCKET          I: Discover Socket			                        */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                - Success                                           */
/*      EPS_ERR_COMM_ERROR   	    - Close socket failed  		                        */
/*                                                                                      */
/* Description:                                                                         */
/*      close discover process.						                                    */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprFindEnd  (
						  
		EPS_SOCKET sock
		
){
	EPS_LOG_FUNCIN
	EPS_RETURN( snmpFindEnd(sock) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   lprProbePrinterByID()                                               */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* defstr  	    EPS_INT8*    		I: ID String of probe target                        */
/* timeout      EPS_UINT32          I: find timeout                                     */
/* printer	    EPS_PRINTER*		O: Pointer for Alloc Printer infomation structure   */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                - Success                                           */
/*      EPS_ERR_COMM_ERROR	        - socket failed      		                        */
/*      EPS_ERR_MEMORY_ALLOCATION   - Failed to allocate memory                         */
/*      EPS_ERR_PRINTER_NOT_FOUND   - printer not found                                 */
/*                                                                                      */
/* Description:                                                                         */
/*     Probe printer by ID String.				                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE   lprProbePrinterByID (

		EPS_INT8*			printerUUID,
		EPS_UINT32			timeout, 
		EPS_PRINTER_INN**	printer

){
	EPS_LOG_FUNCIN
	EPS_RETURN( snmpProbeByID(printerUUID, LPR_PORT_NUM, EPS_PROTOCOL_LPR, timeout, printer) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprStartJob() 													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printer		EPS_PRINTER_INN*	I: pointer to printer Structure						*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/* Description:                                                                         */
/*      Open socekt & Send start job messasge.                                          */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprStartJob(

		void

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_PRINT_JOB_LPR *lprPrintJob = NULL;

	EPS_LOG_FUNCIN

	/* Alloc LPR Job Data */
	lprPrintJob = (EPS_PRINT_JOB_LPR*)EPS_ALLOC( sizeof(EPS_PRINT_JOB_LPR) );
	if( NULL == lprPrintJob ){
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
	}
	memset(lprPrintJob, 0, sizeof(EPS_PRINT_JOB_LPR));
	lprPrintJob->socStat = lprPrintJob->socData = EPS_INVALID_SOCKET;
	lprPrintJob->reseted = FALSE;
	lprPrintJob->pageend = FALSE;

	/* Establish Connection & send start job message */
	ret = StartDataSession( lprPrintJob, printJob.printer );

	if( EPS_ERR_NONE == ret
		&& EPS_IS_BI_PROTOCOL(printJob.printer->protocol) ){
		/* create check status */
		ret = snmpOpenSocket( &(lprPrintJob->socStat) );
		if( EPS_ERR_NONE != ret ){
			goto lprStartJob_END;
		}
	}

	printJob.hProtInfo = (EPS_HANDLE)lprPrintJob;

lprStartJob_END:

	if( EPS_ERR_NONE != ret ){
		if( EPS_INVALID_SOCKET != lprPrintJob->socStat ){
			snmpCloseSocket( &lprPrintJob->socStat );
		}

		if( EPS_INVALID_SOCKET != lprPrintJob->socData ){
			epsNetFnc.close( lprPrintJob->socData );
			lprPrintJob->socData = EPS_INVALID_SOCKET;
		}
		EPS_SAFE_RELEASE( lprPrintJob );

		printJob.hProtInfo = NULL;
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprRestartJob() 													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* N/A                                                                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Open socekt & Send start job messasge.                                          */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprRestartJob(

		void

){
	EPS_PRINT_JOB_LPR *lprPrintJob = (EPS_PRINT_JOB_LPR*)printJob.hProtInfo;
	
	EPS_LOG_FUNCIN

	if( NULL == lprPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}

	/* Establish Connection & send start job message */
	EPS_RETURN( StartDataSession( lprPrintJob, printJob.printer ) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprWritePrintData()												*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* buffer	    EPS_UINT8*          I: Print data					                    */
/* bufferlen    EPS_INT32	        I: Print data length						        */
/* sendlen	    EPS_INT32*	        O: Sended length							        */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized						*/
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Called from SendCommand, Send print data.			                            */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprWritePrintData(

		const EPS_UINT8* buffer, 
		EPS_UINT32 bufferlen, 
		EPS_UINT32* sendlen
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_PRINT_JOB_LPR* lprPrintJob = (EPS_PRINT_JOB_LPR*)printJob.hProtInfo;
	EPS_UINT32 timeout;
	EPS_INT32  sentSize = 0;

	EPS_LOG_FUNCIN

	if( !lprPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}

	/* Send ESC/P-R data																*/
	timeout = (EPS_UINT32)((EPS_IS_BI_PROTOCOL(printJob.printer->protocol))?EPSNET_BAND_SEND_TIMEOUT:EPSNET_BAND_SEND_TIMEOUTL);
		
	sentSize = epsNetFnc.send(lprPrintJob->socData, (char*)buffer, (EPS_INT32)bufferlen, timeout);

	if( EPS_SOCKET_ERROR == sentSize ){
		*sendlen = 0;
		ret = EPS_ERR_COMM_ERROR;
	} else if( EPS_SOCKET_TIMEOUT == sentSize ){
		*sendlen = 0;
		ret = EPS_COM_TINEOUT;
	} else{
		*sendlen = (EPS_UINT32)sentSize;
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprEndJob()														*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* (none)                                                                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized						*/
/*                                                                                      */
/* Description:                                                                         */
/*      Send endjob messasge & Close connection.                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprEndJob()
{
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_PRINT_JOB_LPR* lprPrintJob = (EPS_PRINT_JOB_LPR*)printJob.hProtInfo;

	EPS_LOG_FUNCIN

	if( !lprPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}

	/* Close SendData Session */
	EndDataSession(lprPrintJob);

	if( EPS_IS_BI_PROTOCOL(printJob.printer->protocol) ){
		snmpCloseSocket( &lprPrintJob->socStat );
	}

	EPS_SAFE_RELEASE( printJob.hProtInfo );

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprResetPrinter()													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* (none)                                                                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized						*/
/*                                                                                      */
/* Description:                                                                         */
/*      Send endjob messasge & Close connection.                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprResetPrinter(

		void

){
	EPS_PRINT_JOB_LPR* lprPrintJob = (EPS_PRINT_JOB_LPR*)printJob.hProtInfo;

	EPS_LOG_FUNCIN

	if( NULL == printJob.hProtInfo || NULL == printJob.printer){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}

/*** If we already successfully called this function once for a given print job,        */
    if(printJob.resetSent != EPS_RESET_SENT 
		&& FALSE != printJob.transmittable
		&& TRUE == printJob.sendJS ){
        lprMechCommand(printJob.printer, EPS_CBTCOM_RJ);
    }	

	lprPrintJob->reseted = TRUE;

	/* Close SendData Session */
	EPS_RETURN( EndDataSession(lprPrintJob) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprStartPage() 													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* N/A                                                                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Open socekt & Send start job messasge.                                          */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprStartPage(

		void

){
	EPS_PRINT_JOB_LPR *lprPrintJob = (EPS_PRINT_JOB_LPR*)printJob.hProtInfo;
	
	EPS_LOG_FUNCIN

	if( NULL == lprPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}

	((EPS_PRINT_JOB_LPR*)printJob.hProtInfo)->pageend = FALSE;

	/* Establish Connection & send start job message */
	EPS_RETURN( StartDataSession( lprPrintJob, printJob.printer ) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprEndPage()   													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* (none)                                                                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized						*/
/*                                                                                      */
/* Description:                                                                         */
/*      Send endjob messasge & Close connection.                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprEndPage(

		void

){
	EPS_PRINT_JOB_LPR *lprPrintJob = (EPS_PRINT_JOB_LPR*)printJob.hProtInfo;

	EPS_LOG_FUNCIN

	if( NULL == printJob.hProtInfo ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}

	lprPrintJob->pageend = TRUE;

	/* Close SendData Session */
	EPS_RETURN( EndDataSession(lprPrintJob) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprGetStatus()													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printer	    EPS_PRINTER_INN*    I: Printer data	structure		                    */
/* status		EPS_STATUS_INFO*	O: retrieve printer satus						    */
/* ioStatus		EPS_INT32*      	O: It is possible to communicate				    */
/* canceling	EPS_BOOL*      		O: Cancel processing            				    */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */ 
/* Description:                                                                         */
/*      Get Printer status by SNMP.				                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprGetStatus(

		EPS_STATUS_INFO*        pstInfo,
		EPS_BOOL*				pIoStatus,
		EPS_BOOL*				pCancelling
		
){
	EPS_ERR_CODE		ret = EPS_ERR_NONE;
	EPS_PRINTER_INN*	printer = printJob.printer;
	EPS_SOCKET			sock = EPS_INVALID_SOCKET;

	EPS_LOG_FUNCIN

	if( !EPS_IS_BI_PROTOCOL(printer->protocol) ){
        EPS_RETURN( EPS_ERR_NEED_BIDIRECT );
	}

    memset(pstInfo, -1, sizeof(EPS_STATUS_INFO));

	/*** Get Printer status by SNMP                                                     */
	ret = snmpOpenSocket( &sock );
	if( EPS_ERR_NONE != ret ){
		EPS_RETURN( ret )
	}

	ret = snmp.GetStatus(sock, printer->location, snmp.egID, pstInfo );

	if( !IS_VALID_DATA_SESSION ){
		/* Another proceessing, or printing after endjob */
		if(    EPS_ST_WAITING == pstInfo->nState 
			&& EPS_PRNERR_NOERROR == pstInfo->nError){
			pstInfo->nState = EPS_ST_ERROR;
			pstInfo->nError = EPS_PRNERR_BUSY;
		}
	}

	if(NULL != pIoStatus && NULL != pCancelling){
		if(EPS_ERR_NONE == ret){
			if( IS_VALID_DATA_SESSION || pstInfo->nState == EPS_ST_IDLE ||
				(IS_JOB_ACTIVE && ((EPS_PRINT_JOB_LPR*)printJob.hProtInfo)->pageend) ){
				*pIoStatus = TRUE;
			} else{
				*pIoStatus = FALSE;
			}

			if( IS_JOB_ACTIVE ){
				if( TRUE == ((EPS_PRINT_JOB_LPR*)printJob.hProtInfo)->reseted ){
					if(EPS_ST_IDLE == pstInfo->nState){
						/* cancel finished */
						*pCancelling = FALSE;
						((EPS_PRINT_JOB_LPR*)printJob.hProtInfo)->reseted = FALSE;
					} else{
						/* cancelling */
						*pCancelling = TRUE;
					}
				} else	if( pstInfo->nCancel == EPS_CAREQ_CANCEL ){
					/* cancel request from printer */
					*pCancelling = TRUE;
				} else{
					*pCancelling = FALSE;
				}
			} else{
				*pCancelling = FALSE;
			}
		} else{
			*pIoStatus      = FALSE;
			*pCancelling	= FALSE;
		}
	}

	snmpCloseSocket( &sock );
	
	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprGetJobStatus()	    											*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pstInfo	    EPS_STATUS_INFO*    O: retrieve printer satus		                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized						*/
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Get Printer status by SNMP. It is possible to use in Job.                       */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprGetJobStatus(

		EPS_STATUS_INFO* pstInfo
		
){
	EPS_PRINT_JOB_LPR*   lprPrintJob = (EPS_PRINT_JOB_LPR*)printJob.hProtInfo;
	EPS_PRINTER_INN*     printer = (EPS_PRINTER_INN*)printJob.printer;

	EPS_LOG_FUNCIN

	/* Initialize variables */
	memset(pstInfo, 0, sizeof(EPS_STATUS_INFO));

	if( !lprPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}
	
	/*** Get Printer status by SNMP                                                     */
	EPS_RETURN( snmp.GetStatus( lprPrintJob->socStat, printer->location, snmp.egID, pstInfo ) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     lprGetInfo()	                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printer      EPS_PRINTER_INN*    I: Pointer to a PrinterInfo                         */
/* pString		EPS_UINT8*			O: Pointer to PM String                             */
/* bufSize		EPS_INT32			I: pString buffer size                              */
/*                                                                                      */
/* Return value:																		*/
/*      EPS_ERR_NONE					- Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PROTOCOL_NOT_SUPPORTED  - Unsupported function Error                    */
/*                                                                                      */
/* Description:                                                                         */
/*      Sends request to printer for supported media.  Parses response and stores		*/
/*		PM String : pString                                                             */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE lprGetInfo(
								  
		const EPS_PRINTER_INN*	printer, 
 		EPS_INT32               type,
        EPS_UINT8**             pString,
		EPS_INT32*              bufSize

){
	EPS_LOG_FUNCIN
	EPS_RETURN( snmp.InfoCommand(printer, type, pString, bufSize) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   lprMechCommand()                                                    */
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
EPS_ERR_CODE    lprMechCommand (

        const EPS_PRINTER_INN*	printer, 
        EPS_INT32   Command
        
){
	EPS_LOG_FUNCIN
	EPS_RETURN( snmp.MechCommand(printer, Command) )
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
/* Function name:     StartDataSession()												*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* lprPrintJob	EPS_PRINT_JOB_LPR*  O : LPR Job information        						*/
/* printer   	EPS_PRINTER_INN*    I : target printer information  					*/
/*                                                                                      */
/* Return value:																		*/
/*		EPS_ERR_NONE					- Success							            */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Starts a print job.  Establish Connection, Send StartJob message.       		*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE StartDataSession(

		EPS_PRINT_JOB_LPR* lprPrintJob,
		const EPS_PRINTER_INN* printer

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_INT16 nJobID = 0;
	EPS_INT8 dname[EPS_NAME_BUFFSIZE+1];
	EPS_INT8 cname[EPS_NAME_BUFFSIZE+1];
	EPS_INT8 controlfile[256];

    EPS_INT8* buffer = NULL;
	const char pqueue[16] = "P1";

	EPS_LOG_FUNCIN

	if(EPS_INVALID_SOCKET != lprPrintJob->socData){
		/* Already established */
		EPS_RETURN( EPS_ERR_NONE )
	}

	/* Initialize variables */
	buffer = (EPS_INT8*)EPS_ALLOC( LPR_MAX_BUF );
	if( NULL == buffer ){
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
	}

	/* create socket */
	lprPrintJob->socData = epsNetFnc.socket(EPS_PF_INET, EPS_SOCK_STREAM, EPS_PROTOCOL_TCP);  
	if( EPS_INVALID_SOCKET == lprPrintJob->socData ){
		EPS_DBGPRINT(("StartDataSession ERR\n"));
		ret = EPS_ERR_COMM_ERROR;
		goto StartDataSession_END;
	}
	                       
	/* connect to printer */
	if(EPS_SOCKET_SUCCESS != epsNetFnc.connect(lprPrintJob->socData, printer->location, printer->printPort)){
		EPS_DBGPRINT(("StartDataSession ERR\n"));
		ret = EPS_ERR_COMM_ERROR;
		goto StartDataSession_END;
	}

    /* -------------------------------------------------------------------------------- */
    /* COMMAND: PRINTJOB																*/
    /*      +----+-------+----+															*/
    /*      | 02 | Queue | LF |															*/
    /*      +----+-------+----+															*/
	sprintf(buffer, "\x02%s\n", pqueue);

	/* send command*/
	if( epsNetFnc.send(lprPrintJob->socData, buffer, (EPS_INT32)strlen(buffer), EPSNET_SEND_TIMEOUT)
		!= (EPS_INT32)strlen(buffer)){
		EPS_DBGPRINT(("StartDataSession ERR\n"));
		ret = EPS_ERR_COMM_ERROR;
		goto StartDataSession_END;
	}

	/* receive response */
	if( 0 >= epsNetFnc.receive(lprPrintJob->socData, buffer, LPR_MAX_BUF, EPSNET_RECV_TIMEOUT) 
		|| buffer[0] != 0){
		EPS_DBGPRINT(("StartDataSession ERR\n"));
		ret = EPS_ERR_COMM_ERROR;
		goto StartDataSession_END;
	}

    /* -------------------------------------------------------------------------------- */
    /* SUBCMD: CONTROL FILE																*/
    /*      +----+-------+----+------+----+												*/
    /*      | 02 | Count | SP | Name | LF |												*/
    /*      +----+-------+----+------+----+												*/
    /*      Command code - 2															*/
    /*      Operand 1 - Number of bytes in control file									*/
    /*      Operand 2 - Name of control file											*/
	nJobID = GetJobId();
	sprintf(dname, "dfA%d%s", nJobID, LPR_HOST_NAME);
    sprintf(cname, "cfA%d%s", nJobID, LPR_HOST_NAME);
	sprintf(controlfile, "H%s\nP%s\nv%s\nU%s\nN%s\n",
            LPR_HOST_NAME, "epson", dname, dname, "epson.prn");
	sprintf(buffer, "\x02%d %s\n", (EPS_UINT32)strlen(controlfile), cname);

	/* send command*/
	if( epsNetFnc.send(lprPrintJob->socData, buffer, (EPS_INT32)strlen(buffer), EPSNET_SEND_TIMEOUT)
		!= (EPS_INT32)strlen(buffer)){
		EPS_DBGPRINT(("StartDataSession ERR\n"));
		ret = EPS_ERR_COMM_ERROR;
		goto StartDataSession_END;
	}

	/* receive response */
	if( 0 >= epsNetFnc.receive(lprPrintJob->socData, buffer, LPR_MAX_BUF, EPSNET_RECV_TIMEOUT) 
		|| buffer[0] != 0){
		EPS_DBGPRINT(("StartDataSession ERR\n"));
		ret = EPS_ERR_COMM_ERROR;
		goto StartDataSession_END;
	}

    /* -------------------------------------------------------------------------------- */
    /* ADD CONTENT OF CONTROLFILE														*/
	/* send command*/
	if( epsNetFnc.send(lprPrintJob->socData, controlfile, (EPS_INT32)strlen(controlfile) + 1, EPSNET_SEND_TIMEOUT) 
		!= (EPS_INT32)strlen(controlfile)+1){	/* Send '\0'										*/
		EPS_DBGPRINT(("StartDataSession ERR\n"));
		ret = EPS_ERR_COMM_ERROR;
		goto StartDataSession_END;
	}

	/* receive response */
	if( 0 >= epsNetFnc.receive(lprPrintJob->socData, buffer, LPR_MAX_BUF, EPSNET_RECV_TIMEOUT) 
		|| buffer[0] != 0){
		EPS_DBGPRINT(("StartDataSession ERR\n"));
		ret = EPS_ERR_COMM_ERROR;
		goto StartDataSession_END;
	}

    /* -------------------------------------------------------------------------------- */
    /* SUBCMD: DATA FILE																*/
    /*      +----+-------+----+------+----+												*/
    /*      | 03 | Count | SP | Name | LF |												*/
    /*      +----+-------+----+------+----+												*/
    /*      Command code - 3															*/
    /*      Operand 1 - Number of bytes in data file									*/
    /*      Operand 2 - Name of data file												*/
	sprintf(buffer, "\x03%s %s\n", LPR_DUMMY_DATA_SIZE, dname);

	/* send command*/
	if( epsNetFnc.send(lprPrintJob->socData, buffer, (EPS_INT32)strlen(buffer), EPSNET_SEND_TIMEOUT)
		!= (EPS_INT32)strlen(buffer)){
		EPS_DBGPRINT(("StartDataSession ERR\n"));
		ret = EPS_ERR_COMM_ERROR;
		goto StartDataSession_END;
	}

	/* receive response */
	if( 0 >= epsNetFnc.receive(lprPrintJob->socData, buffer, LPR_MAX_BUF, EPSNET_RECV_TIMEOUT) 
		|| buffer[0] != 0){
		EPS_DBGPRINT(("StartDataSession ERR\n"));
		ret = EPS_ERR_COMM_ERROR;
		goto StartDataSession_END;
	}

	lprPrintJob->reseted = FALSE;
	lprPrintJob->pageend = FALSE;
	/*printJob.hProtInfo = (EPS_HANDLE)lprPrintJob;*/

StartDataSession_END:
	if( EPS_ERR_NONE != ret ){
		if( EPS_INVALID_SOCKET != lprPrintJob->socData ){
			epsNetFnc.close( lprPrintJob->socData );
			lprPrintJob->socData = EPS_INVALID_SOCKET;
		}
	}
	EPS_SAFE_RELEASE( buffer );

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     EndDataSession()													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* lprPrintJob	EPS_PRINT_JOB_LPR*  IO : LPR Job information       						*/
/*                                                                                      */
/* Return value:																		*/
/*      EPS_ERR_NONE				    - Success                                       */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Send EndJob line to printer, shutdown/close socket.                     		*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE EndDataSession(
						 
		EPS_PRINT_JOB_LPR* lprPrintJob
		
){	
	EPS_ERR_CODE ret = EPS_ERR_NONE;						/* Used for error handling */	

	EPS_LOG_FUNCIN

	if( EPS_INVALID_SOCKET != lprPrintJob->socData){
#if 0 /*	This command is ineffectual. */
    /* -------------------------------------------------------------------------------- */
    /* COMMAND: End Job																	*/
    /*      +----+																		*/
    /*      | 00 |																		*/
    /*      +----+																		*/
		EPS_INT8 buffer[LPR_MAX_BUF];
		buffer[0] = 0x00;
		if( epsNetFnc.send(lprPrintJob->socData, buffer, 1, EPSNET_SEND_TIMEOUT) >= 1){
			epsNetFnc.receive(lprPrintJob->socData, buffer, LPR_MAX_BUF, EPSNET_RECV_TIMEOUT);
		} else{
			ret = EPS_ERR_COMM_ERROR;
		}
#endif

	/*** If we already successfully called this function once for a given print job,        */
		epsNetFnc.shutdown(lprPrintJob->socData, EPS_SHUTDOWN_SEND);
		epsNetFnc.shutdown(lprPrintJob->socData, EPS_SHUTDOWN_RECV);
		epsNetFnc.shutdown(lprPrintJob->socData, EPS_SHUTDOWN_BOTH);

		ret = epsNetFnc.close(lprPrintJob->socData);
		lprPrintJob->socData = EPS_INVALID_SOCKET;
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     GetJobId()														*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* (none)										                                        */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_INT16	: JobID										                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Generate LPR job ID (value is between from 1 to 999).	                        */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16 GetJobId(

		void

){
	static EPS_INT16 nJobId = 0;

	nJobId++;			/* next number but            */
	if(nJobId > 999){	/* keep cnt between 0 and 999 */
		nJobId = 0;
	}

	return nJobId;
}

/*_______________________________    epson-net-lpr.c    ________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

