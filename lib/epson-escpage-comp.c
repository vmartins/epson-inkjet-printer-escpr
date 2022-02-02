/*_______________________________   epson-escpage-comp.c   _____________________________*/

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

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-mem.h"
#include "epson-escpage-comp.h"

/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/

#ifdef EPS_LOG_MODULE_PAGE
#define EPS_LOG_MODULE	EPS_LOG_MODULE_PAGE
#else
#define EPS_LOG_MODULE	0
#endif

/* Define Bitmap Format */
#define EP_BGR				0
#define EP_RGB				1

#define ConvertEndianWord(w)	((EPS_UINT16) ((((w) >> 8) & 0xFF) | (((w) << 8) & 0xFF00)))
#define ConvertEndianDWORD(d)	(((EPS_UINT32) (d) << 24) | (((EPS_UINT32) (d) << 8) & 0x00FF0000) | (((EPS_UINT32) (d) >> 8) & 0x0000FF00) | ((EPS_UINT32) (d) >> 24))
#define GetCompressMode(d)		((EPS_UINT16) ((d) >> 16))
#define GetCompressQuality(d)	((EPS_UINT16) ((d) & 0xFFFF))

#define BAND_PIXEL				4
#define MakeCompress20Mode(q)	(0x20 + (q))
#define MakeUncompress20Mode(q)	(0x00 + (q))

/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/
typedef struct tagCODINGDATA {
	EPS_UINT16		hufCode[256] ;
	EPS_UINT8		hufCodeLen[256] ;
	EPS_UINT8*		lpCurPtr ;
	EPS_UINT32		leftCount ;
	EPS_UINT32		code ;
	EPS_UINT16		codeBits ;
} CODINGDATA ;

typedef struct tagCOMPRESSBUFFER {
	EPS_UINT16		wMode ;				/* Compress Mode */
	EPS_UINT16		wQuality ;			/* Compress Quality */
	EPS_UINT8*		lpBuffer ;			/* Output Buffer */
	EPS_UINT32		sizeBuffer ;		/* Output Buffer Size */
	EPS_UINT32		dwWidth ;					
	EPS_UINT32		dwHeight ;					
	EPS_UINT32		format ;			/* Align Format */
	EPS_UINT16		biBitCount ;		/* Bitmap Bit Count */
	EPS_INT32		nextLine ;
	EPS_BOOL		bInvertX ;			/* Invert Image X */
	EPS_UINT8*		lpBitsOrg ;			/* Start Address */
	EPS_UINT32		ySrc ;
} COMPRESSBUFFER ;

typedef struct tagBLOCKHEADER20 {
	EPS_UINT32		size ;		/* 0 */
	EPS_UINT16		width ;		/* 4 */
	EPS_UINT16		bandPixel ;	/* 6 */
	EPS_UINT16		mode ;		/* 8 */
} BLOCKHEADER20 ;
#define SIZEOF_BLOCKHEADER20	(4+2+2+2)

typedef struct tagCOMPBUFFER20 {
	COMPRESSBUFFER	cbuf ;
	BLOCKHEADER20	bh ;
	CODINGDATA		cd ;
	EPS_UINT32		bandLimit ;
	EPS_UINT8		mode[4] ;
} COMPBUFFER20 ;

/*----------------------------  ESC/P-R Lib Global Variables  --------------------------*/
/*******************************************|********************************************/
extern EPS_CMN_FUNC		epsCmnFnc;

/*------------------------------  Local Global Variables  ------------------------------*/
/*******************************************|********************************************/
static const EPS_UINT16 HufmanDefTable2[] 
					= {0, 0, 1, 2, 2, 4, 4, 7, 9, 14, 17, 24, 172} ;
/*static const EPS_UINT16 HufmanDefTable3[][2] 
					= { 0x00, 2, 0x02, 2, 0x01, 2, 0x03, 4, 0x0b, 4, 
					    0x07, 5, 0x17, 5, 0x0f, 5, 0x1f, 5,
					} ;
*/

/*--------------------------  Private Functions Declaration   --------------------------*/
/*******************************************|********************************************/
static EPS_UINT32 EPGetCompressBufferSize(EPS_UINT32 dwMode) ;
static EPS_UINT32 EPCompressImage(
				EPS_UINT32 dwMode, 
				EPS_UINT32 xSrc, EPS_UINT32 ySrc, EPS_UINT32 dwWidth, EPS_UINT32 dwHeight, 
				EPS_BITMAPINFO *lpbmi, EPS_UINT32 biFormat, EPS_UINT8* lpBits,
				EPS_UINT8* lpBuf, EPS_UINT32 sizeBuf, 
				EPS_UINT8* lpWorkBuf ) ;
static EPS_UINT32 Compress20(COMPBUFFER20* lpC20Buf) ;
static EPS_BOOL MakeHufmanCodeTable(EPS_UINT16 wMode, EPS_UINT16 wQuality, CODINGDATA* lpcd) ;
static EPS_BOOL WriteBandData20(EPS_INT32 band, EPS_INT32 incptr, EPS_UINT8* lpSrc, COMPBUFFER20* lpC20Buf) ;
static EPS_UINT8 GetByteBLOCKHEADER20(BLOCKHEADER20 *p, EPS_INT32 n);
static void MakeHufmanTable1(const EPS_UINT16 cdn[], CODINGDATA* lpcd) ;
static void CodeHufmanData(const EPS_UINT8 d, CODINGDATA* const lpcd);
static void WriteDataToBuffer(CODINGDATA* const lpcd);


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------              Public Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

EPS_UINT8*	CompressBitImage(

	EPS_RECT			*pRec,
	EPS_BITMAPINFO		*pBitMapInfo,
	EPS_UINT8*			pRealBits,
	EPS_UINT32			ulDataFormat,
	EPS_UINT32			*pulCompressType,
	EPS_UINT32			*pulImageSize

){
	EPS_BITMAPINFO		bmi;
	EPS_UINT32			dwComMode = 0;
	EPS_UINT32			dwwkBufferSize;
	EPS_UINT8*			pwkBuffer = NULL;
	EPS_UINT32			dwBuffer;
	EPS_UINT8*			pTmp = NULL;
	EPS_UINT32			dwCompressedSize;

	EPS_INT32			ulSrcleft, ulSrcright, ulSrcbottom, ulSrctop;

	ulSrcleft = pRec->left; 
	ulSrcright= pRec->right;
	ulSrcbottom = pRec->bottom;
	ulSrctop= pRec->top;

	bmi = *pBitMapInfo;

	dwComMode = ulDataFormat;

	*pulCompressType = dwComMode;
	dwComMode = dwComMode << 16;

	dwwkBufferSize = EPGetCompressBufferSize( dwComMode);

	if(dwwkBufferSize == 0){
		*pulCompressType = 0;
		return NULL;
	}

	if(!(pwkBuffer = (EPS_UINT8*)EPS_ALLOC( dwwkBufferSize ))){
		*pulCompressType = 0;
		return NULL;
	}

	dwBuffer = (((ulSrcright - ulSrcleft ) * 24 + 31) / 32) * 4 * (ulSrcbottom - ulSrctop );
	if(!(pTmp = (EPS_UINT8*)EPS_ALLOC( dwBuffer ))){
		EPS_FREE(pwkBuffer );
		*pulCompressType = 0;
		return NULL;
	}

	dwCompressedSize = EPCompressImage( dwComMode,
											ulSrcleft,
											ulSrctop,
											ulSrcright - ulSrcleft ,
											ulSrcbottom - ulSrctop ,
											&bmi,
											EP_RGB,
											pRealBits,
											pTmp,
											dwBuffer,
											pwkBuffer );

	EPS_FREE(pwkBuffer);
	if(dwCompressedSize == 0){
		EPS_FREE(pTmp);
		*pulCompressType = 0;
		return NULL;
	}

	*pulImageSize = dwCompressedSize;

	return pTmp;
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------               Local Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

static EPS_UINT32  EPGetCompressBufferSize (EPS_UINT32 dwMode)
{
	EPS_UINT16 mode;
	
	EPS_LOG_FUNCIN

	mode = GetCompressMode(dwMode) ;

	switch (mode) {

	case EP_COMPRESS20 :
		EPS_RETURN( sizeof(COMPBUFFER20) )
	default:
		break;
	}

	EPS_RETURN( 0 )
}


static EPS_UINT32 EPCompressImage (
		EPS_UINT32 dwMode,
		EPS_UINT32 xSrc,
		EPS_UINT32 ySrc,
		EPS_UINT32 dwWidth,
		EPS_UINT32 dwHeight,
		EPS_BITMAPINFO *lpbmi,
		EPS_UINT32 biFormat,
		EPS_UINT8* lpBits,
		EPS_UINT8* lpBuf,
		EPS_UINT32 sizeBuf,
		EPS_UINT8* lpWorkBuf )
{
	COMPRESSBUFFER* lpCBuf ;
	EPS_UINT32 height ;

	EPS_LOG_FUNCIN

	if (dwWidth == 0 || dwHeight == 0) {
		EPS_RETURN( 0 )
	}
	if (lpWorkBuf == NULL) {
		EPS_RETURN( 0 )
	}

	lpCBuf = (COMPRESSBUFFER*) lpWorkBuf ;
		
	/* Check Width and calculate Scan Size */

	if (lpbmi->biWidth <= 0 || (EPS_UINT32) lpbmi->biWidth < xSrc + dwWidth) {
		EPS_RETURN( 0 )
	}

	lpCBuf->dwWidth = dwWidth ;
	lpCBuf->biBitCount = lpbmi->biBitCount ;
	lpCBuf->format = biFormat ;
	
	lpCBuf->nextLine = ((lpbmi->biWidth * lpCBuf->biBitCount + 7) / 8 + 3) & 0xFFFFFFFC ;
	lpCBuf->lpBitsOrg = lpBits + (xSrc*lpCBuf->biBitCount / 8) ;
	/*lpCBuf->bitOffset = (EPS_INT16) (xSrc*lpCBuf->biBitCount % 8) ;*/
	
	/* Check Height and calculate start addr */
	
	lpCBuf->dwHeight = dwHeight ;
	if (lpbmi->biHeight > 0) {
		/* bottom up bitmap */
		height = (EPS_UINT32) lpbmi->biHeight ;
		lpCBuf->lpBitsOrg += lpCBuf->nextLine * (ySrc + dwHeight - 1);
		lpCBuf->nextLine = -lpCBuf->nextLine ;
	}
	else {
		/* top down bitmap */
		height = (EPS_UINT32) -lpbmi->biHeight;
		lpCBuf->lpBitsOrg += lpCBuf->nextLine * ySrc;
	}
	if (height == 0 || height < ySrc + dwHeight) {
		EPS_RETURN( 0 )
	}
	
	/* Fill Other fields */
	lpCBuf->wMode = GetCompressMode(dwMode) ;
	lpCBuf->wQuality = GetCompressQuality(dwMode) ;
	lpCBuf->lpBuffer = lpBuf ;
	lpCBuf->sizeBuffer = sizeBuf ;
	lpCBuf->bInvertX = FALSE ;
	
	switch (lpCBuf->wMode) {
	case EP_COMPRESS20 :
		EPS_RETURN( Compress20((COMPBUFFER20*) lpCBuf) )
	default:
		break;
	}
	
	EPS_RETURN( 0 )
}

/*	24BPP Image Compress Mode 20 */
static EPS_UINT32 Compress20(COMPBUFFER20* lpC20Buf)
{
	EPS_UINT8* lpSrc ;
	EPS_INT32 nextBand ;
	EPS_UINT32 y, yblock, ymod ;
	EPS_INT32 incptr ;
	
	EPS_LOG_FUNCIN

	if (lpC20Buf->cbuf.biBitCount == 24) {
		incptr = 3 ;
	}
	else if (lpC20Buf->cbuf.biBitCount == 32) {
		incptr = 4 ;
	}
	else {
		EPS_RETURN( 0 )
	}
	
	if (lpC20Buf->cbuf.bInvertX) {
		/* invert image */
		lpSrc = lpC20Buf->cbuf.lpBitsOrg + (lpC20Buf->cbuf.dwWidth - 1) * incptr ;
		incptr = -incptr ;
	}
	else {
		lpSrc = lpC20Buf->cbuf.lpBitsOrg ;
	}
	
	if (lpC20Buf->cbuf.format == EP_RGB) {
		lpC20Buf->mode[0] = 2 ;
		lpC20Buf->mode[1] = 1 ;
		lpC20Buf->mode[2] = 0 ;
	}
	else {
		lpC20Buf->mode[0] = 0 ;
		lpC20Buf->mode[1] = 1 ;
		lpC20Buf->mode[2] = 2 ;
	}
	
	/* Make Hufman Code Table */
	MakeHufmanCodeTable(lpC20Buf->cbuf.wMode, lpC20Buf->cbuf.wQuality, &lpC20Buf->cd) ;
	
	/* Make Block Header */
	lpC20Buf->bh.width		= ConvertEndianWord(lpC20Buf->cbuf.dwWidth) ;
	lpC20Buf->bh.bandPixel	= ConvertEndianWord(BAND_PIXEL) ;
	
	/* Initialize */
	lpC20Buf->cd.lpCurPtr = lpC20Buf->cbuf.lpBuffer ;
	lpC20Buf->cd.leftCount = lpC20Buf->cbuf.sizeBuffer ;

	yblock	= lpC20Buf->cbuf.dwHeight / BAND_PIXEL ;
	ymod	= lpC20Buf->cbuf.dwHeight % BAND_PIXEL ;

	nextBand = lpC20Buf->cbuf.nextLine * BAND_PIXEL ;

	lpC20Buf->bandLimit = SIZEOF_BLOCKHEADER20 + lpC20Buf->cbuf.dwWidth * BAND_PIXEL ;

	for (y = 0 ; y < yblock ; y++) {
		if (! WriteBandData20(BAND_PIXEL, incptr, lpSrc, lpC20Buf)) {
			EPS_RETURN( 0 )
		}
		lpSrc += nextBand ;
	}
	if (ymod) {
		lpC20Buf->bh.bandPixel	= ConvertEndianWord(ymod) ;
		lpC20Buf->bandLimit = SIZEOF_BLOCKHEADER20 + lpC20Buf->cbuf.dwWidth * ymod ;
		
		if (! WriteBandData20((EPS_UINT16) ymod, incptr, lpSrc, lpC20Buf)) {
			EPS_RETURN( 0 )
		}
	}

	if (lpC20Buf->cd.leftCount == 0) {
			EPS_RETURN( 0 )
	}

	EPS_RETURN( lpC20Buf->cbuf.sizeBuffer - lpC20Buf->cd.leftCount )
}


static EPS_BOOL WriteBandData20(EPS_INT32 band, EPS_INT32 incptr, EPS_UINT8* lpSrc, COMPBUFFER20* lpC20Buf) 
{
	EPS_UINT32 sizeBlockData ;
	EPS_UINT8* lpBandStart ;
	EPS_UINT32 countBandStart ;
	EPS_UINT8 *lpBand, *lpPt, *lpPrePt;
	EPS_UINT8 left ;

	EPS_INT32 i, j ;
	EPS_UINT32 x ;

	EPS_LOG_FUNCIN

	for (i = 0 ; i <= 2 ; i++) {
		if (lpC20Buf->cd.leftCount <= SIZEOF_BLOCKHEADER20) {
			EPS_RETURN( FALSE )
		}

		lpBandStart = lpC20Buf->cd.lpCurPtr ;
		countBandStart = lpC20Buf->cd.leftCount ;

		lpC20Buf->cd.lpCurPtr += SIZEOF_BLOCKHEADER20;
		lpC20Buf->cd.leftCount -= SIZEOF_BLOCKHEADER20;
		lpC20Buf->cd.code = 0 ;
		lpC20Buf->cd.codeBits = 0 ;
		
		lpBand = lpSrc + i ;
		lpPt = lpBand ;
		left = 0 ;
		
		for (x = 0 ; x < lpC20Buf->cbuf.dwWidth ; x++) {
			CodeHufmanData((EPS_UINT8) (*lpPt-left), &lpC20Buf->cd) ;
			left = *lpPt ;
			lpPt += incptr ;
		}
		
		for (j = 1 ; j < band ; j++) {
			lpPrePt = lpBand ;
			lpBand += lpC20Buf->cbuf.nextLine ;
			lpPt = lpBand ;
			left = 0 ;
			
			for (x = 0 ; x < lpC20Buf->cbuf.dwWidth ; x++) {
				CodeHufmanData((EPS_UINT8) (*lpPt-((left+*lpPrePt)>>1)), &lpC20Buf->cd) ;
				left = *lpPt ;
				lpPt += incptr ;
				lpPrePt += incptr ;
			}
		}
		if (lpC20Buf->cd.codeBits) WriteDataToBuffer(&lpC20Buf->cd) ;
		
		if (countBandStart <= lpC20Buf->cd.leftCount + lpC20Buf->bandLimit) {
			/* Write Block Header */
			sizeBlockData = countBandStart - lpC20Buf->cd.leftCount ;
			lpC20Buf->bh.size = ConvertEndianDWORD(sizeBlockData) ;
			lpC20Buf->bh.mode = ConvertEndianWord(MakeCompress20Mode(lpC20Buf->mode[i])) ;

			for (x = 0 ; x < SIZEOF_BLOCKHEADER20 ; x++) {
				*lpBandStart++ = GetByteBLOCKHEADER20(&lpC20Buf->bh, x);
			}
		}
		else {
			/* Output Un-Compress Data */
			if (countBandStart < lpC20Buf->bandLimit) {
				EPS_RETURN( FALSE )
			}

			sizeBlockData = lpC20Buf->bandLimit ;
			lpC20Buf->bh.size = ConvertEndianDWORD(sizeBlockData) ;
			lpC20Buf->bh.mode = ConvertEndianWord(MakeUncompress20Mode(lpC20Buf->mode[i])) ;

			for (x = 0 ; x < SIZEOF_BLOCKHEADER20; x++) {
				*lpBandStart++ = GetByteBLOCKHEADER20(&lpC20Buf->bh, x);
			}

			lpBand = lpSrc + i ;
			for (j = 0 ; j < band ; j++) {
				lpPt = lpBand ;
				lpBand += lpC20Buf->cbuf.nextLine ;
				for (x = 0 ; x < lpC20Buf->cbuf.dwWidth ; x++) {
					*lpBandStart++ = *lpPt ;
					lpPt += incptr ;
				}
			}
			
			lpC20Buf->cd.lpCurPtr = lpBandStart ;
			lpC20Buf->cd.leftCount = countBandStart - lpC20Buf->bandLimit ;
		}
	}

	EPS_RETURN( TRUE )
}


static EPS_UINT8 GetByteBLOCKHEADER20(BLOCKHEADER20 *p, EPS_INT32 n)
{
	EPS_UINT8 b = 0;
	switch(n){
	case 0:		b = (EPS_UINT8)(p->size)       & 0x000000FF;	break;
	case 1:		b = (EPS_UINT8)(p->size >> 8)  & 0x000000FF;	break;
	case 2:		b = (EPS_UINT8)(p->size >> 16) & 0x000000FF;	break;
	case 3:		b = (EPS_UINT8)(p->size >> 24) & 0x000000FF;	break;

	case 4:		b = (EPS_UINT8)(p->width)      & 0x00FF;		break;
	case 5:		b = (EPS_UINT8)(p->width >> 8) & 0x00FF;		break;

	case 6:		b = (EPS_UINT8)(p->bandPixel)      & 0x00FF;	break;
	case 7:		b = (EPS_UINT8)(p->bandPixel >> 8) & 0x00FF;	break;

	case 8:		b = (EPS_UINT8)(p->mode)       & 0x00FF;		break;
	case 9:		b = (unsigned char)(p->mode >> 8)  & 0x00FF;	break;

	default:	b = 0;
	}
	return b;
}


static void WriteDataToBuffer(CODINGDATA* const lpcd) 
{
	if (lpcd->leftCount) {
		*lpcd->lpCurPtr++ = (EPS_UINT8) lpcd->code ;
		lpcd->leftCount-- ;
		lpcd->code >>= 8 ;
	}
	lpcd->codeBits -= 8 ;
}


static void CodeHufmanData(const EPS_UINT8 d, CODINGDATA* const lpcd) 
{
	lpcd->code |= ((EPS_UINT32) lpcd->hufCode[d]) << (lpcd->codeBits) ;
	lpcd->codeBits += lpcd->hufCodeLen[d] ;
	
	while (lpcd->codeBits >= 8) WriteDataToBuffer(lpcd) ;
}


static EPS_BOOL MakeHufmanCodeTable(EPS_UINT16 wMode, EPS_UINT16 wQuality, CODINGDATA* lpcd)
{
	switch (wMode) {
	case EP_COMPRESS20 :
		MakeHufmanTable1(HufmanDefTable2, lpcd) ;
		return TRUE ;
	}

	return FALSE ;
}


static void MakeHufmanTable1(const EPS_UINT16 cdn[], CODINGDATA* lpcd) 
{

	EPS_UINT16 huf, rhuf, temp, i ;
	EPS_INT32 len, num, j ;

	huf = 0 ;
	num = 0 ;
	
	for(len = 1 ; len <= 12; len++) {
		huf <<= 1;

		for(i = 0; i < cdn[len] ; i++) {
			rhuf = 0 ;
			temp = huf ;

			for (j = 0 ; j < len ; j++) {
				rhuf = (rhuf << 1) | (temp & 1) ;
				temp >>= 1 ;
			}
			
			if (num) {
				j = (num+1) >> 1 ;
				if ((num & 0x1) == 0) j = 256 - j ;
			}
			else {
				j = 0 ;
			}

			lpcd->hufCode[j] = rhuf ;
			lpcd->hufCodeLen[j] = (EPS_UINT8) len ;

			huf ++ ;
			num ++ ;
		}
	}
}


/*_______________________________   epson-escpage-comp.c   _____________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
