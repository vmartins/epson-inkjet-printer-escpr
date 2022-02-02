/*________________________________   epson-net-raw.c   _________________________________*/

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
/*                               Raw Port protocol Module                               */
/*                                                                                      */
/*                                Public Function Calls                                 */
/*                              --------------------------                              */
/*      EPS_ERR_CODE rawFindStart       (sock, address, flg                     );      */
/*		EPS_ERR_CODE rawFind            (sock, printer                          );      */
/*      EPS_ERR_CODE rawFindEnd	        (sock                                   );      */
/*      EPS_ERR_CODE rawProbePrinterByID(printerUUID, timeout, printer          );      */
/*		EPS_ERR_CODE rawStartJob	    (printr                                 );      */
/*      EPS_ERR_CODE rawEndPage	        (                                       );      */
/*      EPS_ERR_CODE rawResetPrinter    (                                       );      */
/*      EPS_ERR_CODE rawWritePrintData  (buffer, bufferlen, sendlen             );      */
/*      EPS_ERR_CODE rawGetStatus       (printer, status, ioStatus              );      */
/*      EPS_ERR_CODE rawGetJobStatus    (pstInfo			                    );      */
/*      EPS_ERR_CODE rawGetInfo         (printer, pString, bufSize              );      */
/*      EPS_ERR_CODE rawMechCommand     (printer, Command                       );      */
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
#include "epson-net-raw.h"

/*-----------------------------  Local Macro Definitions -------------------------------*/
/*******************************************|********************************************/
#ifdef EPS_LOG_MODULE_RAW
#define EPS_LOG_MODULE	EPS_LOG_MODULE_RAW
#else
#define EPS_LOG_MODULE	0
#endif

#define RAW_PORT_NUM			(9100)				/* Protocol port number				*/

#define IS_JOB_ACTIVE																		\
		( (NULL != printJob.printer) && (NULL != printJob.hProtInfo)						\
			&& (EPS_PROTOCOL_RAW == EPS_PRT_PROTOCOL(printJob.printer->protocol)) )

#define IS_VALID_DATA_SESSION																\
		( (NULL != printJob.printer) && (NULL != printJob.hProtInfo)						\
			&& (EPS_PROTOCOL_RAW == EPS_PRT_PROTOCOL(printJob.printer->protocol))			\
			&& (EPS_INVALID_SOCKET != ((EPS_PRINT_JOB_RAW*)printJob.hProtInfo)->socData) )

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
typedef struct _tagEPS_PRINT_JOB_RAW_ {
	EPS_SOCKET	socData;					/* RAW socket for send data                 */
	EPS_SOCKET	socStat;					/* SNMP socket for check status             */
	EPS_BOOL	reseted;					/* reseted                                  */
	EPS_BOOL	pageend;					/* page end - start                         */
} EPS_PRINT_JOB_RAW;

/*--------------------------------  RAW Local Variables  -------------------------------*/
/*******************************************|********************************************/

/*--------------------------  Local Functions Declaration   ----------------------------*/
/*******************************************|********************************************/
static EPS_ERR_CODE StartDataSession	(EPS_PRINT_JOB_RAW*, const EPS_PRINTER_INN*     );
static EPS_ERR_CODE EndDataSession      (EPS_PRINT_JOB_RAW*                             );    

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------              Public Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void		rawSetupSTFunctions	(
								 
		 const EPS_PRINTER_INN*	printer
		 
){
	EPS_LOG_FUNCIN

	snmpSetupSTFunctions(&snmp, printer);

	EPS_RETURN_VOID
}


EPS_UINT16	rawGetDefautiPort    (
								  
		void
		
){
	return RAW_PORT_NUM;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     rawFindStart()                                                    */
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
EPS_ERR_CODE rawFindStart(

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
/* Function name:     rawFind()	                                                        */
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
EPS_ERR_CODE rawFind     (
						  
		EPS_SOCKET			sock,
		EPS_PRINTER_INN**   printer
		
){
	EPS_LOG_FUNCIN
	EPS_RETURN( snmpFind(sock, RAW_PORT_NUM, EPS_PROTOCOL_RAW, printer) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     rawFindEnd() 				                                        */
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
EPS_ERR_CODE rawFindEnd  (
						  
		EPS_SOCKET sock
		
){
	EPS_LOG_FUNCIN
	EPS_RETURN( snmpFindEnd(sock) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   rawProbePrinterByID()                                               */
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
EPS_ERR_CODE   rawProbePrinterByID (

		EPS_INT8*			printerUUID,
		EPS_UINT32			timeout, 
		EPS_PRINTER_INN**	printer

){
	EPS_LOG_FUNCIN
	EPS_RETURN( snmpProbeByID(printerUUID, RAW_PORT_NUM, EPS_PROTOCOL_RAW, timeout, printer) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     rawStartJob() 													*/
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
EPS_ERR_CODE rawStartJob(

		void

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_PRINT_JOB_RAW *rawPrintJob = NULL;

	EPS_LOG_FUNCIN

	/* Alloc RAW Job Data */
	rawPrintJob = (EPS_PRINT_JOB_RAW*)EPS_ALLOC( sizeof(EPS_PRINT_JOB_RAW) );
	if( NULL == rawPrintJob ){
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
	}
	memset(rawPrintJob, 0, sizeof(EPS_PRINT_JOB_RAW));
	rawPrintJob->socStat = rawPrintJob->socData = EPS_INVALID_SOCKET;
	rawPrintJob->reseted = FALSE;
	rawPrintJob->pageend = FALSE;

	/* Establish Connection & send start job message */
	ret = StartDataSession( rawPrintJob, printJob.printer );

	if( EPS_ERR_NONE == ret
		&& EPS_IS_BI_PROTOCOL(printJob.printer->protocol) ){
		/* create check status */
		ret = snmpOpenSocket( &(rawPrintJob->socStat) );
		if( EPS_ERR_NONE != ret ){
			goto rawStartJob_END;
		}
	}

	printJob.hProtInfo = (EPS_HANDLE)rawPrintJob;

rawStartJob_END:

	if( EPS_ERR_NONE != ret ){
		if( EPS_INVALID_SOCKET != rawPrintJob->socStat ){
			snmpCloseSocket( &rawPrintJob->socStat );
		}

		if( EPS_INVALID_SOCKET != rawPrintJob->socData ){
			epsNetFnc.close( rawPrintJob->socData );
			rawPrintJob->socData = EPS_INVALID_SOCKET;
		}
		EPS_SAFE_RELEASE( rawPrintJob );

		printJob.hProtInfo = NULL;
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     rawRestartJob() 													*/
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
EPS_ERR_CODE rawRestartJob(

		void

){
	EPS_PRINT_JOB_RAW *rawPrintJob = (EPS_PRINT_JOB_RAW*)printJob.hProtInfo;
	
	EPS_LOG_FUNCIN

	if( NULL == rawPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}

	/* Establish Connection & send start job message */
	EPS_RETURN( StartDataSession( rawPrintJob, printJob.printer ) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     rawWritePrintData()												*/
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
/*      Called from epsPrintBand, Send ESC/P-R data.		                            */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE rawWritePrintData(

		const EPS_UINT8* buffer, 
		EPS_UINT32 bufferlen, 
		EPS_UINT32* sendlen
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_PRINT_JOB_RAW* rawPrintJob = (EPS_PRINT_JOB_RAW*)printJob.hProtInfo;
	EPS_UINT32 timeout;
	EPS_INT32  sentSize = 0;

	EPS_LOG_FUNCIN

	if( !rawPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}

	/* Send ESC/P-R data																*/
	timeout = (EPS_UINT32)((EPS_IS_BI_PROTOCOL(printJob.printer->protocol))?EPSNET_BAND_SEND_TIMEOUT:EPSNET_BAND_SEND_TIMEOUTL);
		
	sentSize = epsNetFnc.send(rawPrintJob->socData, (char*)buffer, (EPS_INT32)bufferlen, timeout);

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
/* Function name:     rawEndJob()														*/
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
EPS_ERR_CODE rawEndJob()
{
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_PRINT_JOB_RAW* rawPrintJob = (EPS_PRINT_JOB_RAW*)printJob.hProtInfo;

	EPS_LOG_FUNCIN

	if( !rawPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}

	/* Close SendData Session */
	EndDataSession(rawPrintJob);

	if( EPS_IS_BI_PROTOCOL(printJob.printer->protocol) ){
		snmpCloseSocket( &rawPrintJob->socStat );
	}

	EPS_SAFE_RELEASE( printJob.hProtInfo );

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     rawResetPrinter()													*/
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
EPS_ERR_CODE rawResetPrinter(

		void

){
	EPS_PRINT_JOB_RAW* rawPrintJob = (EPS_PRINT_JOB_RAW*)printJob.hProtInfo;

	EPS_LOG_FUNCIN

	if( NULL == printJob.hProtInfo || NULL == printJob.printer){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}

/*** If we already successfully called this function once for a given print job,        */
    if(printJob.resetSent != EPS_RESET_SENT 
		&& FALSE != printJob.transmittable
		&& TRUE == printJob.sendJS ){
        rawMechCommand(printJob.printer, EPS_CBTCOM_RJ);
    }	

	rawPrintJob->reseted = TRUE;

	/* Close SendData Session */
	EPS_RETURN( EndDataSession(rawPrintJob) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     rawStartPage() 													*/
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
EPS_ERR_CODE rawStartPage(

		void

){
	EPS_PRINT_JOB_RAW *rawPrintJob = (EPS_PRINT_JOB_RAW*)printJob.hProtInfo;
	
	EPS_LOG_FUNCIN

	if( NULL == rawPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}

	((EPS_PRINT_JOB_RAW*)printJob.hProtInfo)->pageend = FALSE;

	/* Establish Connection & send start job message */
	EPS_RETURN( StartDataSession( rawPrintJob, printJob.printer ) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     rawEndPage()   													*/
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
EPS_ERR_CODE rawEndPage(

		void

){
	EPS_PRINT_JOB_RAW *rawPrintJob = (EPS_PRINT_JOB_RAW*)printJob.hProtInfo;

	EPS_LOG_FUNCIN

	if( NULL == printJob.hProtInfo ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}

	rawPrintJob->pageend = TRUE;

	/* Close SendData Session */
	EPS_RETURN( EndDataSession(rawPrintJob) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     rawGetStatus()													*/
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
EPS_ERR_CODE rawGetStatus(

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
				(IS_JOB_ACTIVE && ((EPS_PRINT_JOB_RAW*)printJob.hProtInfo)->pageend) ){
				*pIoStatus = TRUE;
			} else{
				*pIoStatus = FALSE;
			}

			if( IS_JOB_ACTIVE ){
				if( TRUE == ((EPS_PRINT_JOB_RAW*)printJob.hProtInfo)->reseted ){
					if(EPS_ST_IDLE == pstInfo->nState){
						/* cancel finished */
						*pCancelling = FALSE;
						((EPS_PRINT_JOB_RAW*)printJob.hProtInfo)->reseted = FALSE;
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
/* Function name:     rawGetJobStatus()	    											*/
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
EPS_ERR_CODE rawGetJobStatus(

		EPS_STATUS_INFO* pstInfo
		
){
	EPS_PRINT_JOB_RAW*   rawPrintJob = (EPS_PRINT_JOB_RAW*)printJob.hProtInfo;
	EPS_PRINTER_INN*     printer = (EPS_PRINTER_INN*)printJob.printer;

	EPS_LOG_FUNCIN

	/* Initialize variables */
	memset(pstInfo, 0, sizeof(EPS_STATUS_INFO));

	if( !rawPrintJob ){
		EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED )
	}
	
	/*** Get Printer status by SNMP                                                     */
	EPS_RETURN( snmp.GetStatus( rawPrintJob->socStat, printer->location, snmp.egID, pstInfo ) )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     rawGetInfo()	                                                    */
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
EPS_ERR_CODE rawGetInfo(
								  
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
/* Function name:   rawMechCommand()                                                    */
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
EPS_ERR_CODE    rawMechCommand (

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
/* rawPrintJob	EPS_PRINT_JOB_RAW*  O : RAW Job information        						*/
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

		EPS_PRINT_JOB_RAW* rawPrintJob,
		const EPS_PRINTER_INN* printer

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;

	EPS_LOG_FUNCIN

	if(EPS_INVALID_SOCKET != rawPrintJob->socData){
		/* Already established */
		EPS_RETURN( EPS_ERR_NONE )
	}

	/* Check status */
	if( EPS_IS_BI_PROTOCOL(printer->protocol) ){
		EPS_STATUS_INFO	stInfo;
		ret = rawGetStatus(&stInfo, NULL, NULL);
		if(EPS_ERR_NONE != ret){
			EPS_RETURN( ret )
		} else if(stInfo.nState != EPS_ST_IDLE){
			EPS_RETURN( EPS_ERR_PRINTER_ERR_OCCUR )
		}
	}

	/* create socket */
	rawPrintJob->socData = epsNetFnc.socket(EPS_PF_INET, EPS_SOCK_STREAM, EPS_PROTOCOL_TCP);  
	if( EPS_INVALID_SOCKET == rawPrintJob->socData ){
		EPS_DBGPRINT(("StartDataSession ERR\n"));
		ret = EPS_ERR_COMM_ERROR;
		goto StartDataSession_END;
	}
	                       
	/* connect to printer */
	if(EPS_SOCKET_SUCCESS != epsNetFnc.connect(rawPrintJob->socData, printer->location, printer->printPort)){
		EPS_DBGPRINT(("StartDataSession ERR\n"));
		ret = EPS_ERR_COMM_ERROR;
		goto StartDataSession_END;
	}

	rawPrintJob->reseted = FALSE;
	rawPrintJob->pageend = FALSE;
	/*printJob.hProtInfo = (EPS_HANDLE)rawPrintJob;*/

StartDataSession_END:
	if( EPS_ERR_NONE != ret ){
		if( EPS_INVALID_SOCKET != rawPrintJob->socData ){
			epsNetFnc.close( rawPrintJob->socData );
			rawPrintJob->socData = EPS_INVALID_SOCKET;
		}
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     EndDataSession()													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* rawPrintJob	EPS_PRINT_JOB_RAW*  IO : RAW Job information       						*/
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
						 
		EPS_PRINT_JOB_RAW* rawPrintJob
		
){	
	EPS_ERR_CODE ret = EPS_ERR_NONE;						/* Used for error handling */	

	EPS_LOG_FUNCIN

	if( EPS_INVALID_SOCKET != rawPrintJob->socData){
	/*** If we already successfully called this function once for a given print job,        */
		epsNetFnc.shutdown(rawPrintJob->socData, EPS_SHUTDOWN_SEND);
		epsNetFnc.shutdown(rawPrintJob->socData, EPS_SHUTDOWN_RECV);
		epsNetFnc.shutdown(rawPrintJob->socData, EPS_SHUTDOWN_BOTH);

		ret = epsNetFnc.close(rawPrintJob->socData);
		rawPrintJob->socData = EPS_INVALID_SOCKET;
	}

	EPS_RETURN( ret )
}
/*_______________________________    epson-net-raw.c    ________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

