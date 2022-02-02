/*_________________________________   epson-layout.c   _________________________________*/

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
/*                                Extent Layout Module                                  */
/*                                                                                      */
/*                                Public Function Calls                                 */
/*                              --------------------------                              */
/*		EPS_ERR_CODE elGetDots	        (resolution, millimeter                 );      */
/*      EPS_ERR_CODE elCDClipping	    (pSrc, pDst, bpp, pRec                  );      */
/*                                                                                      */
/*******************************************|********************************************/

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-def.h"
#include "epson-escpr-err.h"
#include "epson-escpr-media.h"
#include "epson-escpr-mem.h"
#include "epson-layout.h"

/*-----------------------------  Local Macro Definitions -------------------------------*/
/*******************************************|********************************************/
#ifdef EPS_LOG_MODULE_ELT
#define EPS_LOG_MODULE EPS_LOG_MODULE_ELT
#else
#define EPS_LOG_MODULE 0
#endif

/*----------------------------  ESC/P-R Lib Global Variables  --------------------------*/
/*******************************************|********************************************/

	/*** Extern Function                                                                */
extern EPS_CMN_FUNC    epsCmnFnc;

    /*** Print Job Structure                                                            */
    /*** -------------------------------------------------------------------------------*/
extern EPS_PRINT_JOB   printJob;


/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/


/*--------------------------  Local Functions Declaration   ----------------------------*/
/*******************************************|********************************************/
#ifdef GCOMSW_EL_CDLABEL
static EPS_UINT32 isqrt(EPS_UINT32 x);
#endif

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------              Public Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   elGetDots()		                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* resolution   EPS_UINT8			I: resolution	                                    */
/* millimeter   EPS_UINT32          I: length(millimeter x 10)                          */
/*                                                                                      */
/* Return value:                                                                        */
/*      Dot num                                                                         */
/*                                                                                      */
/* Description:                                                                         */
/*      calc the number of the dots in the resolution.                                  */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32     elGetDots (

        EPS_UINT8 resolution, 
		EPS_UINT32 millimeter

){
	EPS_INT32 dots = 0;

	if(resolution == EPS_IR_360X360){	/* EPS_IR_360X360 == 0 */
		dots = (EPS_INT32)(millimeter * 14173);	/* 0.03937 * 360 = 14.1732 -> x1000 */
	} else if( resolution & EPS_IR_720X720 ){
		dots = (EPS_INT32)(millimeter * 28346);	/* 0.03937 * 720 = 28.3464 -> x1000 */
	} else if( resolution & EPS_IR_300X300 ){
		dots = (EPS_INT32)(millimeter * 11811);	/* 0.03937 * 300 = 11.811  -> x1000 */
	} else if( resolution & EPS_IR_600X600 ){
		dots = (EPS_INT32)(millimeter * 23622);	/* 0.03937 * 600 = 23.622  -> x1000 */
	} else{
		/* default 360dpi */
		dots = (EPS_INT32)(millimeter * 14173);	/* 0.03937 * 360 = 14.1732 -> x1000 */
	}

	return dots/10000;	/* input:10 times, inche:1000 times  */
}


#ifdef GCOMSW_EL_CDLABEL
/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   elCDClipping()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pSrc         const EPS_UINT8*    I: source buffer                                    */
/* pDst         EPS_UINT8*          O: destination buffer                               */
/* bpp          EPS_UINT8           I: byte per pixel                                   */
/* pRec         EPS_RECT*           I/O: byte per pixel                                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      clip band line to cd label                                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE     elCDClipping (

        const EPS_UINT8* pSrc,
        EPS_UINT8*       pDst,
        EPS_UINT8        bpp,
        EPS_RECT*        pRec

){
#define GET_RADIUS(mm)		(EPS_INT32)(elGetDots(printJob.attr.inputResolution, mm*10)/2)
#define X_OF_SECANT(r, y)   ((EPS_INT32)isqrt((r-y) * (r+y)))

	EPS_LINE_SEGMENT segOut, segIn;
	EPS_INT32 rOut = GET_RADIUS(printJob.attr.cdDimOut);
	EPS_INT32 rIn  = GET_RADIUS(printJob.attr.cdDimIn);
	EPS_INT32 y    = 0;
    EPS_UINT32 wOut = 0;
    EPS_UINT32 wIn = 0;

	EPS_LOG_FUNCIN;

	/* calc clipping segment */
	y = rOut - pRec->top;
    if(y <= 0) {
        y -= -1;
    }

    wOut = X_OF_SECANT(rOut, y);
	segOut.start = rOut - wOut;
	segOut.length = wOut * 2;

    if(y <= rIn && y >= -rIn) {
        wIn = X_OF_SECANT(rIn, y);
		segIn.start = wOut - wIn;
		segIn.length = wIn * 2;
	} else{
		segIn.start = 
		segIn.length = 0;
	}

/*	EPS_DBGPRINT((" (%04d - %04d) (%04d - %04d)\n",
		segOut.start, segOut.length,
		segIn.start, segIn.length));
*/
	/* outside clip */
	memcpy(pDst, pSrc + segOut.start * bpp, segOut.length * bpp);

	/* inside mask  */
	if( 0 < segIn.start){
		memset(pDst + segIn.start * bpp, 0xFF, segIn.length * bpp);
	}

	pRec->left = segOut.start;
	pRec->right = segOut.start + segOut.length;

    EPS_RETURN( EPS_ERR_NONE );
}

static EPS_UINT32 isqrt(EPS_UINT32 x) 
{ 
    EPS_INT32	b = 15;		/* this is the next bit we try								*/
    EPS_UINT32	r = 0;		/* r will contain the result								*/
    EPS_UINT32	r2= 0;		/* here we maintain r squared								*/

    while(b>=0)  
    { 
        EPS_UINT32 sr2=r2; 
        EPS_UINT32 sr=r; 
                    /* compute (r+(1<<b))**2, we have r**2 already. */
        r2 += (EPS_UINT32)((r<<(1+b))+(1<<(b+b)));       
        r  += (EPS_UINT32)(1<<b); 
        if (r2>x)  
        { 
            r=sr; 
            r2=sr2; 
        } 
        b--; 
    } 
    return r; 
}
#endif /* GCOMSW_EL_CDLABEL */

/*_________________________________   epson-layout.c   _________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

