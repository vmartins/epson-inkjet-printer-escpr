/*_______________________________   epson-escpr-dbg.h   ________________________________*/

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
/*                            Epson debuger Module Definitions                          */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef _EPSON_ESPCR_DBG_H_
#define _EPSON_ESPCR_DBG_H_
#ifdef    __cplusplus
extern "C" {
#endif    /* def __cplusplus    */

#ifdef GCOMSW_DEBUG

/*------------------------------- Global Compiler Switch -------------------------------*/
/*******************************************|********************************************/

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"
#include "epson-escpr-pvt.h"


/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/
 
/*--------------------------  Public Function Declarations   ---------------------------*/
/*******************************************|********************************************/

/*** memory                                                                             */
/*** -----------------------------------------------------------------------------------*/
extern void dbgMemInit();
extern void dbgMemResetPeak();
extern void dbgMemReportPeak();
extern EPS_UINT32 dbgMemGetPeak();
extern void dbgMemReportLeak();
extern void *dbgAlloc(EPS_INT8 *pszFile, EPS_INT32 nLine, EPS_UINT32 s);
extern void dbgFree(void *p);

#define EPS_MEM_INIT        dbgMemInit();
#define EPS_MEM_RESET_PEAK	dbgMemResetPeak();
#define EPS_MEM_REPORT_PEAK	dbgMemReportPeak();
#define EPS_MEM_GET_PEAK	dbgMemGetPeak()
#define EPS_MEM_REPORT_LEAK dbgMemReportLeak();
#define EPS_ALLOC( s )      dbgAlloc( __FILE__, __LINE__, (EPS_UINT32)(s) )
#define EPS_FREE( p )	    dbgFree( (void*)(p) )


/*** PRINT                                                                              */
/*** -----------------------------------------------------------------------------------*/
extern char *dbgPrintHlp(const char* frm, ...);
extern void dbgPrint(EPS_INT8 *pszSrcFile, EPS_INT32 nSrcLine, EPS_INT8 *pMsg);
	
	#define EPS_DBGPRINT( f )	{					\
		char *pLine = dbgPrintHlp f;				\
		dbgPrint(__FILE__, __LINE__, pLine);	\
	}

	/* Dump */
extern void dbgDump(EPS_INT8 *pszFile, EPS_INT32 nLine, EPS_INT8* p, EPS_UINT32 s);
	#define EPS_DUMP( p, s )	dbgDump(__FILE__, __LINE__, (EPS_INT8*)p, s);

extern void dbgDumpStr(EPS_INT8 *pszSrcFile, EPS_INT32 nSrcLine, EPS_INT8 *p);
	#define EPS_DUMP_STR( str )	dbgDumpStr(__FILE__, __LINE__, str);


/*** Runtime LOG                                                                        */
/*** -----------------------------------------------------------------------------------*/
extern void dbgLogInit(const EPS_INT8 *pszFile, EPS_UINT32 nModule);
extern void dbgLogTrem();
extern void dbgLogFunc(EPS_UINT32 nModule, EPS_INT8 *pszFunc, EPS_BOOL blnIn, 
					   EPS_INT32 nLine, EPS_ERR_CODE e, EPS_UINT32 tm);

	#define EPS_LOG_INITIALIZE(f, m)	dbgLogInit(f, m);
	#define EPS_LOG_TREMINATE			dbgLogTrem();
	#define EPS_LOG_MODULE_NONE		(0x0000)
	#define EPS_LOG_MODULE_API		(0x0001)
	#define EPS_LOG_MODULE_PRTCOL	(0x0002)
	#define EPS_LOG_MODULE_PAGE  	(0x0004)
	#define EPS_LOG_MODULE_USB  	(0x0010)
	#define EPS_LOG_MODULE_LPR  	(0x0020)
	/*#define EPS_LOG_MODULE_UPNP  	(0x0040)*/
	#define EPS_LOG_MODULE_RAW  	(0x0080)
	#define EPS_LOG_MODULE_CBT  	(0x0100)
	#define EPS_LOG_MODULE_SER  	(0x0200)

	
#if defined(__func__)			/* C99 */
#define EPS_FUNCNAME	__func__
#elif defined(__FUNCTION__)		/* Visual Studio */
#define EPS_FUNCNAME	__FUNCTION__
#else
#define EPS_FUNCNAME	__FILE__
#endif

#if defined(EPS_FUNCNAME)
#define EPS_LOG_FUNCIN_L(l)																	\
			EPS_UINT32	prtm = 0;														\
			if(epsCmnFnc.getTime){														\
				prtm = epsCmnFnc.getTime();												\
				dbgLogFunc(EPS_LOG_MODULE, EPS_FUNCNAME, TRUE, l, 0, epsCmnFnc.getTime()); \
			} else{																		\
				dbgLogFunc(EPS_LOG_MODULE, EPS_FUNCNAME, TRUE, l, 0, 0);				\
			}

	#define EPS_LOG_FUNCOUT(l, e)														\
			if(epsCmnFnc.getTime){														\
				dbgLogFunc(EPS_LOG_MODULE, EPS_FUNCNAME, FALSE, l, e, epsCmnFnc.getTime() - prtm);	\
			} else{																		\
				dbgLogFunc(EPS_LOG_MODULE, EPS_FUNCNAME, FALSE, l, e, 0);				\
			}
	#define EPS_LOG_FUNCIN	EPS_LOG_FUNCIN_L(__LINE__)
	#define EPS_RETURN( e )	{						\
			EPS_INT32 res = e;						\
			EPS_LOG_FUNCOUT(__LINE__, res);			\
			return res;								\
	}

	#define EPS_RETURN_VOID	{						\
			EPS_LOG_FUNCOUT(__LINE__, 0);			\
			return;									\
	}

#else
	#define EPS_LOG_FUNCIN		
	#define EPS_LOG_FUNCOUT(e)	
	#define EPS_RETURN(e)		return e;
	#define EPS_RETURN_VOID		return;
#endif

/*** Dump to file                                                                       */
/*** -----------------------------------------------------------------------------------*/
extern FILE *dbgDumpFileOpen(EPS_INT8 *pszFile);
extern void dbgDumpFileClose(FILE *hFile);
extern void dbgDumpFileWrite(FILE *hFile, const void *pData, size_t nSize);

	#define DECRALE_DMP_FILE	extern FILE *g_pDmpFile;
	#define EPS_DF_OPEN( fn )						\
		{											\
			g_pDmpFile = dbgDumpFileOpen( fn );		\
		}
	#define EPS_DF_CLOSE							\
		{											\
			dbgDumpFileClose( g_pDmpFile );			\
			g_pDmpFile = NULL;						\
		}
	#define EPS_DF_WRITE( p, s )					\
		if( g_pDmpFile )							\
		{											\
			dbgDumpFileWrite(g_pDmpFile, p, s );	\
		}

#endif    /* GCOMSW_DEBUG */


#ifdef    __cplusplus
}
#endif    /* def __cplusplus    */

#endif    /* def _EPSON_ESPCR_DBG_H_ */

/*_______________________________   epson-escpr-dbg.h   ________________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
