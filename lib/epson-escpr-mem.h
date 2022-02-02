/*_______________________________   epson-escpr-mem.h   ________________________________*/

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
/*                             Epson memory Module Definitions                          */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef _EPSON_ESPCR_MEM_H_
#define _EPSON_ESPCR_MEM_H_
#ifdef    __cplusplus
extern "C" {
#endif    /* def __cplusplus    */


/*------------------------------- Global Compiler Switch -------------------------------*/
/*******************************************|********************************************/

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"


/*------------------------------  Import Debug utility   -------------------------------*/
/*******************************************|********************************************/
#ifdef GCOMSW_DEBUG
#include "epson-escpr-dbg.h"
#else
/* Redefine Debug macros */
#define EPS_ALLOC( s )	    epsCmnFnc.memAlloc((EPS_UINT32)(s))
#define EPS_FREE( p )	    epsCmnFnc.memFree( (p) )
#define EPS_RETURN(e)		return e;
#define EPS_RETURN_VOID		return;
	
#define EPS_MEM_INIT	
#define EPS_MEM_RESET_PEAK
#define EPS_MEM_REPORT_PEAK
#define EPS_MEM_GET_PEAK	(0)
#define EPS_MEM_REPORT_LEAK 

#define EPS_DBGPRINT( f )
#define EPS_DUMP( p, s )
#define EPS_DUMP_STR( str )

#define EPS_LOG_INITIALIZE(f, m)
#define EPS_LOG_TREMINATE
#define EPS_LOG_FUNCIN		
#define EPS_LOG_FUNCOUT(e)	

#define DECRALE_DMP_FILE
#define EPS_DF_OPEN( fn )
#define EPS_DF_CLOSE
#define EPS_DF_WRITE( p, s )
#endif

/*----------------------------------  Generic Macros   ---------------------------------*/
/*******************************************|********************************************/

#define EPS_SAFE_RELEASE( p )	\
	if( NULL != (p) ){			\
		EPS_FREE( (p) );		\
		(p) = NULL;				\
	}

/*---------------------------------- CPU Endian-ness -----------------------------------*/
/*******************************************|********************************************/
typedef enum _EPS_ENDIAN {
    EPS_ENDIAN_NOT_TESTED = 1000,
    EPS_ENDIAN_BIG,
    EPS_ENDIAN_LITTLE
} EPS_ENDIAN;

typedef enum _EPS_BYTE_SIZE {
    EPS_2_BYTES   = 2000,
    EPS_4_BYTES   = 4000
} EPS_BYTE_SIZE;


#define EPS_SWP_ORDER16(data) (EPS_INT16)((((data) >>  8) & 0x00FF    )|(((data) <<  8) & 0xFF00    ) )
#define EPS_SWP_ORDER32(data) (EPS_INT32)((((data) >> 24) & 0x000000FF)|(((data) >>  8) & 0x0000FF00) | \
                          (((data) <<  8) & 0x00FF0000)|(((data) << 24) & 0xFF000000) )


/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/
    /*** Max, Min                                                                       */
    /*** -------------------------------------------------------------------------------*/
#define Max(a,b) ( ((a) > (b)) ? (a) : (b))
#define Min(a,b) ( ((a) < (b)) ? (a) : (b))

    /*** memcpy macro for readability                                                   */
    /*** -------------------------------------------------------------------------------*/
#define COPY_BYTES(des,src,size)    memcpy(des,src,size); des+=size;


/*--------------------------  Public Function Declarations   ---------------------------*/
/*******************************************|********************************************/
extern void*		memRealloc			(void*, EPS_UINT32, EPS_UINT32                  );

extern EPS_INT8*	memStrStr			(EPS_INT8*, const EPS_INT8*, EPS_BOOL			);
extern EPS_INT8*	memStrStrWithLen	(EPS_INT8*, EPS_UINT32, const EPS_INT8*	        );

extern void         memSetEndian        (EPS_ENDIAN, EPS_BYTE_SIZE, EPS_UINT32, EPS_UINT8*);
extern void         memInspectEndian    (void                                           );
extern EPS_INT32	memGetBitCount		(EPS_INT32                                      );
extern EPS_UINT8	memSearchWhiteColorVal(EPS_UINT8, EPS_UINT8*, EPS_UINT16			);

#define EPS_MEM_GROW(t, p, pCurSize, nNewSize)						\
		/*EPS_DBGPRINT(("GROW %d->%d\n", *pCurSize, (nNewSize)))*/	\
		if(*pCurSize < (nNewSize)){									\
			p = (t)memRealloc(p, *pCurSize, (nNewSize));			\
			*pCurSize = (nNewSize);									\
		}
		
#ifdef    __cplusplus
}
#endif    /* def __cplusplus    */

#endif    /* def _EPSON_ESPCR_MEM_H_ */

/*_______________________________   epson-escpr-mem.h   ________________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
