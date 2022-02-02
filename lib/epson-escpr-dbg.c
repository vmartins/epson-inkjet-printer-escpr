/*________________________________  epson-escpr-dbg.c   ________________________________*/

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
/****************************************************************************************/
/* debug utility                                                                        */
/****************************************************************************************/

#ifdef GCOMSW_DEBUG
/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-def.h"
#include "epson-escpr-err.h"
#include "epson-escpr-dbg.h"

/*----------------------------  ESC/P-R Lib Global Variables  --------------------------*/
/*******************************************|********************************************/

	/*** Extern Function                                                                */
	/*** -------------------------------------------------------------------------------*/
extern EPS_CMN_FUNC    epsCmnFnc;


/*--------------------------------  Local Definition   ---------------------------------*/
/*******************************************|********************************************/


/*--------------------------------  Local Functions   ----------------------------------*/
/*******************************************|********************************************/



/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------              Public Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/


#ifdef WIN32
#include <crtdbg.h>
#endif

#ifdef WINCE
#include <windef.h>
#include <winnt.h>
#include <dbgapi.h>
#endif

static FILE *g_pLogFile = NULL;


/* ------------------------------------------------------------------------------------ */
/* Leak check																		    */
typedef struct _tagEPS_DBG_MEMINFO {
	void			*pAddress;
	EPS_UINT32		size;
	unsigned long	nLine;
	char			szFile[64];
}EPS_DBG_MEMINFO;

#define EPS_DBG_MEMINFO_NUM		(100)


EPS_DBG_MEMINFO	g_EpsDbgMemInfo[EPS_DBG_MEMINFO_NUM];
EPS_UINT32      g_nCurSize = 0;
EPS_UINT32      g_nPeakSize = 0;


void dbgMemInit()
{
	memset( g_EpsDbgMemInfo, 0, sizeof(g_EpsDbgMemInfo) );
	dbgMemResetPeak();
}

void dbgMemResetPeak()
{
	g_nPeakSize = g_nCurSize;
}

void dbgMemReportPeak()
{
#if defined(unix) || defined(__unix)
	fprintf(stderr, "*** peak memory size = %d byte ***\n", g_nPeakSize);
#elif defined(WIN32)
	_RPT1(_CRT_WARN, "*** peak memory size = %d byte ***\n", g_nPeakSize);
#elif defined(WINCE)
	NKDbgPrintfW(L"*** peak memory size = %d ***\n", g_nPeakSize);
#endif
}

EPS_UINT32 dbgMemGetPeak()
{
	return g_nPeakSize;
}

void dbgMemReportLeak()
{
	int i;
	for(i = 0; i < EPS_DBG_MEMINFO_NUM; i++){
		if( 0 != g_EpsDbgMemInfo[i].pAddress ){
#if defined(unix) || defined(__unix)
			fprintf(stderr, "!***** memory leak *****! %s (%ld) Size: %d / Addr: 0x%08X\n", 
				g_EpsDbgMemInfo[i].szFile, g_EpsDbgMemInfo[i].nLine, g_EpsDbgMemInfo[i].size, (EPS_UINT32)g_EpsDbgMemInfo[i].pAddress);
#elif defined(WIN32)
			_RPT4(_CRT_WARN, "!***** memory leak *****! %s (%d) Size: %d / Addr: 0x%08X\n", 
				g_EpsDbgMemInfo[i].szFile, g_EpsDbgMemInfo[i].nLine, g_EpsDbgMemInfo[i].size, g_EpsDbgMemInfo[i].pAddress);
#elif defined(WINCE)
			NKDbgPrintfW(L"!***** memory leak *****! %hs (%d) Addr: 0x%08X\n",
				g_EpsDbgMemInfo[i].szFile, g_EpsDbgMemInfo[i].nLine, g_EpsDbgMemInfo[i].pAddress);
#endif
		}
	}
}
 

void *dbgAlloc( EPS_INT8 *pszFile, EPS_INT32 nLine, EPS_UINT32 s )
{
	int i;
	char *pFile;
	void *p = epsCmnFnc.memAlloc(s);

	if( NULL != p ){
		for(i = 0; i < EPS_DBG_MEMINFO_NUM; i++){
			if( NULL == g_EpsDbgMemInfo[i].pAddress ){
				g_EpsDbgMemInfo[i].pAddress = p;
				g_EpsDbgMemInfo[i].size = s;
				g_EpsDbgMemInfo[i].nLine = nLine;
#if defined(unix) || defined(__unix)
				pFile = strrchr(pszFile, '/');
				strcpy(g_EpsDbgMemInfo[i].szFile, (pFile?pFile:pszFile));
#else				
				pFile = strrchr(pszFile, '\\');
				strcpy(g_EpsDbgMemInfo[i].szFile, ++pFile);
#endif				
				break;
			}
		}
 		if(i >= EPS_DBG_MEMINFO_NUM){
#if defined(unix) || defined(__unix)
			fprintf(stderr, "!!! WAN Debuginfo space not enough! \n");
#elif defined(WIN32)
			_RPT0(_CRT_WARN, "!!! WAN Debuginfo space not enough! \n");
#elif defined(WINCE)
			NKDbgPrintfW(L"!!! WAN Debuginfo space not enough! \n");
#endif
		}

		g_nCurSize += s;

		g_nPeakSize = ((g_nPeakSize > g_nCurSize)?g_nPeakSize:g_nCurSize);

#if 0 /*defined(WIN32)*/
		{
			char* pSrc = strrchr(pszFile, '\\');
			_RPT_BASE((_CRT_WARN, NULL, 0, NULL, 
				"ALC : %s(%d)\t+%5d -> %5d(%5d)\n", pSrc, nLine, s, g_nCurSize, g_nPeakSize));
		}
#endif

	}

	return p;
}


void dbgFree(void *p)
{
	int i;

	if(NULL == p){
		return;
	}

	for(i = 0; i < EPS_DBG_MEMINFO_NUM; i++){
		if( p == g_EpsDbgMemInfo[i].pAddress ){
			g_nCurSize -= g_EpsDbgMemInfo[i].size;
			g_EpsDbgMemInfo[i].pAddress = NULL;

#if 0/* defined(WIN32)*/
			{
				_RPT_BASE((_CRT_WARN, NULL, 0, NULL, 
					"FRR : %s(%d)\t-%5d -> %5d(%5d)\n", 
					g_EpsDbgMemInfo[i].szFile, g_EpsDbgMemInfo[i].nLine, g_EpsDbgMemInfo[i].size, g_nCurSize, g_nPeakSize));
			}
#endif

			break;
		}
	}
	if(i >= EPS_DBG_MEMINFO_NUM){
#if defined(unix) || defined(__unix)
		fprintf(stderr, "!!! WAN mem 0x%08X not found \n", (EPS_UINT32)p);
#elif defined(WIN32)
		_RPT1(_CRT_WARN, "!!! WAN mem 0x%08X not found \n", p);
#elif defined(WINCE)
		NKDbgPrintfW(L"!!! WAN mem 0x%08X not found \n", p);
#endif
	}

	epsCmnFnc.memFree( p );
}


/* ------------------------------------------------------------------------------------ */
/* Print                                                                                */
#define EPSN_MAX_MESSAGE (255)

static EPS_INT8 g_DebugMsg[EPSN_MAX_MESSAGE+1];

char *dbgPrintHlp(const char* frm, ...)
{
	int nLen = 0;
	va_list ptr;

	va_start(ptr, frm);
#if defined(unix) || defined(__unix)
	nLen = vsnprintf(g_DebugMsg, EPSN_MAX_MESSAGE, frm, ptr);
#elif defined(WIN32) || defined(WINCE)
	nLen = _vsnprintf(g_DebugMsg, EPSN_MAX_MESSAGE, frm, ptr);
#endif
	va_end(ptr);

	if(nLen > 0 && nLen < EPSN_MAX_MESSAGE){
		*(g_DebugMsg+nLen) = '\0';
	} else{
		*(g_DebugMsg+EPSN_MAX_MESSAGE) = '\0';
	}
	return g_DebugMsg;
}


void dbgPrint(EPS_INT8 *pszSrcFile, EPS_INT32 nSrcLine, EPS_INT8 *pMsg)
{
#if defined(unix) || defined(__unix)
	char* pFile = strrchr(pszSrcFile, '/');
	fprintf(stderr, "%s(%d): %s", (pFile?pFile:pszSrcFile), nSrcLine, pMsg);
#elif defined(WIN32)
	char* pFile = strrchr(pszSrcFile, '\\');
	if(g_pLogFile){
		fprintf(g_pLogFile, "%s(%d): %s", ++pFile, nSrcLine, pMsg);
	} else{
		_RPT3(_CRT_WARN, "%s(%d): %s", ++pFile, nSrcLine, pMsg);
	}
#elif defined(WINCE)
	char* pFile = strrchr(pszSrcFile, '\\');
	NKDbgPrintfW(L"%hs(%d): %hs", ++pFile, nSrcLine, pMsg);
#endif
}


void dbgDumpStr(EPS_INT8 *pszSrcFile, EPS_INT32 nSrcLine, EPS_INT8 *p)
{
	unsigned long n = 0;
	EPS_UINT32 pos = 0;
	char c;
	char *pFile;
	EPS_UINT32 nLen = (EPS_UINT32)strlen(p);

#if defined(unix) || defined(__unix)
	pFile = strrchr(pszSrcFile, '/');
	fprintf(stderr, "---  Dump String [%d] src: %s[%d] ---\n", nLen, (pFile?pFile:pszSrcFile), nSrcLine);
#elif defined(WIN32)
	pFile = strrchr(pszSrcFile, '\\');
	_RPT3(_CRT_WARN, "---  Dump String [%d] src: %s[%d] ---\r\n", nLen, ++pFile, nSrcLine);
#elif defined(WINCE)
	pFile = strrchr(pszSrcFile, '\\');
	NKDbgPrintfW(L"---  Dump  String [%d] ---\n", nLen);
#endif

	for(n = 0; n < nLen; n += 255){
		if(n+255 < nLen){
			pos = 255;
		} else{
			pos = nLen - n;
		}

		c = *(p + n + pos);
		*(p + n + pos) = '\0';
#if defined(unix) || defined(__unix)
		printf("%s\n", p+n);
#elif defined(WIN32)
		_RPT1(_CRT_WARN, "%s\n", p+n);
#elif defined(WINCE)
		NKDbgPrintfW(L"%hs\n", p+n);
#endif
		*(p + n + pos) = c;
	}
}


void dbgDump(EPS_INT8 *pszSrcFile, EPS_INT32 nSrcLine, EPS_INT8 *p, EPS_UINT32 s)
{
#define EPSNET_DUMP_LINEBYTE (76)
	int i;
	EPS_UINT32 nCnt = 0;
	char cBin[64];
	char cChar[32];
	char cLines[EPSNET_DUMP_LINEBYTE*4+1];
	int nLine = 0;
	char *pFile = NULL;

#if defined(unix) || defined(__unix)
	pFile = strrchr(pszSrcFile, '/');
	fprintf(stderr, "---  Dump  src: %s[%d] // data: 0x%08X [%dbyte]  ---\n", (pFile?pFile:pszSrcFile), nSrcLine, (EPS_INT32)p, s);
#elif defined(WIN32)
	pFile = strrchr(pszSrcFile, '\\');
	_RPT4(_CRT_WARN, "---  Dump  src: %s[%d] // data: 0x%08X [%dbyte]  ---\n", ++pFile, nSrcLine, p, s);
#elif defined(WINCE)
	pFile = strrchr(pszSrcFile, '\\');
	NKDbgPrintfW(L"---  Dump  src: %hs[%d] // data: 0x%08X [%dbyte]  ---\n", ++pFile, nSrcLine, p, s);
#endif

	while( nCnt < s ){
		for(i = 0; i < 16 && nCnt < s; i++, nCnt++){
			sprintf(&cBin[i*3], "%02X ", (unsigned char)*(p + nCnt));
			cChar[i] = ( 0x20 <= *(p + nCnt) && *(p + nCnt) <= 0x7e)?(*(p + nCnt)):'.';
		}
		cBin[i*3] = cChar[i] = '\0';

		sprintf(cLines + (EPSNET_DUMP_LINEBYTE * nLine), "%08X %-48s %s\r\n", nCnt - i, cBin, cChar);
		nLine++;

		if(nLine >= 4){
#if defined(unix) || defined(__unix)
			fprintf(stderr, "%s", cLines);	
#elif defined(WIN32)
			_RPT1(_CRT_WARN, "%s", cLines);	
#elif defined(WINCE)
			NKDbgPrintfW(L"%hs", cLines);	
#endif
			nLine = 0;
		}
	}

	if(nLine > 0 && nLine < 4){
#if defined(unix) || defined(__unix)
		fprintf(stderr, "%s", cLines);	
#elif defined(WIN32)
		_RPT1(_CRT_WARN, "%s", cLines);	
#elif defined(WINCE)
		NKDbgPrintfW(L"%hs", cLines);	
#endif
	}
}


/* ------------------------------------------------------------------------------------ */
/* Runtime LOG                                                                          */
static long g_logModule = EPS_LOG_MODULE_NONE;
static long g_logFuncLv = 0;
/*static FILE *g_pLogFile = NULL;*/

void dbgLogInit(const EPS_INT8 *pszFile, EPS_UINT32 nModule)
{
	if(pszFile){
		if(g_pLogFile){
			fclose(g_pLogFile);
		}

		g_pLogFile = fopen(pszFile, "w");
	}

	g_logModule = nModule;
}

void dbgLogTrem()
{
	if(g_pLogFile){
		fclose(g_pLogFile);
		g_pLogFile = NULL;
	}

	g_logModule = EPS_LOG_MODULE_NONE;
}

void dbgLogFunc(EPS_UINT32 nModule, EPS_INT8 *pszFunc, EPS_BOOL blnIn, EPS_INT32 nLine, EPS_ERR_CODE e, EPS_UINT32 tm)
{
	char szTag[256];
#if defined(unix) || defined(__unix)
	char 	*pFile;
	pFile = strrchr(pszFunc, '/');
	if(pFile)pszFunc = pFile;
#endif

#define EPS_DBGTAG_LEN	48
	
	if(g_logModule & nModule){
		memset(szTag, ' ', EPS_DBGTAG_LEN);
		if(blnIn){
			sprintf(szTag + g_logFuncLv*2, "+ %s", pszFunc);
			szTag[strlen(szTag)] = ' ';
			sprintf(szTag + EPS_DBGTAG_LEN,  "L:%d\tTM:%d", nLine, tm);
			g_logFuncLv++;
		} else{
			if(g_logFuncLv>0)g_logFuncLv--;
			sprintf(szTag + g_logFuncLv*2, "- %s(%d)", pszFunc, e);
			szTag[strlen(szTag)] = ' ';
			sprintf(szTag + EPS_DBGTAG_LEN, " L:%d\tTM:%d", nLine, tm);
		}

		if(g_pLogFile){
			fprintf(g_pLogFile, "%s\n", szTag);
		} else{
#if defined(unix) || defined(__unix)
			fprintf(stderr, "%s\r\n", szTag);
#elif defined(WIN32)
			_RPT1(_CRT_WARN, "%s\n", szTag);
#elif defined(WINCE)
			NKDbgPrintfW(L"%s\n", szTag);
#endif
		}
	}
}



/* ------------------------------------------------------------------------------------ */
/* Dump to file                                                                         */
FILE *g_pDmpFile = NULL;
FILE *dbgDumpFileOpen(EPS_INT8 *pszFile)
{
	FILE *hFile = fopen(pszFile, "wb");
	return hFile;
}


void dbgDumpFileClose(FILE *hFile)
{
	if(hFile)fclose(hFile);
}


void dbgDumpFileWrite(FILE *hFile, const void *pData, size_t nSize)
{
	if(hFile)fwrite(pData, nSize, 1, hFile);
}


#endif	/* GCOMSW_DEBUG */

/*________________________________  epson-escpr-dbg.c  _________________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
