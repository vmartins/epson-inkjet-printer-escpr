/*________________________________  epson-escpr-mem.c   ________________________________*/

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
/*                                   memory Module                                      */
/*                                                                                      */
/*                                Public Function Calls                                 */
/*                              --------------------------                              */
/*      void*       memRealloc          (buffer, oldSizem, newSize              );      */
/*		EPS_INT8*   memStrStr           (strSrc, strFind, gotoEnd               );      */
/*      EPS_INT8*	memStrStrWithLen	(strSrc, nSrcLen, strFind               );      */
/*      void        memSetEndian        (Endianess, byteSize, endTag, value, array);    */
/*      void        memInspectEndian    (                                       );      */
/*      EPS_INT16   memHtoN16           (nSrc                                   );      */
/*      EPS_INT32   memHtoN32           (nSrc                                   );      */
/*      EPS_INT32	memGetBitCount		(bitfield								);		*/
/*                                                                                      */
/*******************************************|********************************************/

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-def.h"
#include "epson-escpr-err.h"
#include "epson-escpr-mem.h"

/*----------------------------  ESC/P-R Lib Global Variables  --------------------------*/
/*******************************************|********************************************/

	/*** Extern Function                                                                */
	/*** -------------------------------------------------------------------------------*/
extern EPS_CMN_FUNC    epsCmnFnc;

EPS_INT16   cpuEndian;                      /* Endian-ness                              */


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

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     memRealloc()   				                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* buffer		void*				I/O: pointer of buffer								*/
/* oldSize		EPS_UINT32			I:   original buffer size							*/
/* newSize		EPS_UINT32			I:   new buffer size								*/
/*                                                                                      */
/* Return value:                                                                        */
/*				EPS_INT8*			pointer to finded string                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Realocate buffer.																*/
/*                                                                                      */
/*******************************************|********************************************/
void* memRealloc (
				 
		void*       buffer, 
		EPS_UINT32  oldSize, 
		EPS_UINT32  newSize
		
){
	/* Create a temporary pointer to a new buffer of the desired size */
	void* newBuffer = EPS_ALLOC(newSize);
	if( NULL == newBuffer ){
		EPS_SAFE_RELEASE( buffer );
		return NULL;
	}

	memset(newBuffer, 0, newSize);

	/* Copy the data from the old buffer to the new one */
	if(oldSize < newSize)
	{
		memcpy(newBuffer, buffer, oldSize);
	}
	else
	{
		memcpy(newBuffer, buffer, newSize);
	}
	/* Free the original buffer */
	EPS_SAFE_RELEASE( buffer );

	/* Return a pointer to the new block of memory */
	return newBuffer;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     memStrStr()   				                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* strSrc		EPS_INT8*			I: Source string									*/
/* strFind		EPS_INT8*			I: Find string. Size MUST be less than 64 byte.		*/
/* gotoEnd		EPS_BOOL			I: If TRUE, the return value indicates              */
/*                                     the end of found string.                         */
/*                                                                                      */
/* Return value:                                                                        */
/*				EPS_INT8*			pointer to finded string                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Finds the first occurrence of a substring within a string.						*/
/*		The comparison is NOT case sensitive.											*/
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT8* memStrStr (

		EPS_INT8*       strSrc, 
		const EPS_INT8* strFind, 
		EPS_BOOL        gotoEnd
		
){
	EPS_INT8	strShadow[64];
	EPS_INT32	nSrcPos = 0;
	EPS_INT32	nFindPos = 0;
	EPS_INT32	nFindedTop = -1;
	EPS_UINT32	nFindStrLen = 0;
	
	if(NULL == strSrc || 64 <= strlen(strFind)){
		return NULL;
	}

	memset(strShadow, 0, sizeof(strShadow));

	/* create shadow string */
	do{
		if( 0x41 <= strFind[nFindPos] && strFind[nFindPos] <= 0x5A){
			strShadow[nFindPos] = (EPS_INT8)(strFind[nFindPos] + 0x20);
		} else if( 0x61 <= strFind[nFindPos] && strFind[nFindPos] <= 0x7A){
			strShadow[nFindPos] = (EPS_INT8)(strFind[nFindPos] - 0x20);
		} else{
			strShadow[nFindPos] = strFind[nFindPos];
		}
	}while('\0' != strFind[nFindPos++]);

	/* search by strFind & strShadow */
	nFindPos = 0;
	while( '\0' != strSrc[nSrcPos] && '\0' != strFind[nFindPos]){
		if( strSrc[nSrcPos] == strFind[nFindPos]
			|| strSrc[nSrcPos] == strShadow[nFindPos] ){
			if(-1 == nFindedTop){
				nFindedTop = nSrcPos;
			}

			nSrcPos++;
			nFindPos++;

		} else{
			if(-1 == nFindedTop){
				nSrcPos++;
				nFindPos = 0;
			} else{
				nSrcPos = nFindedTop + 1;
			}
			nFindedTop = -1;
		}
	}

	nFindStrLen = (EPS_UINT32)strlen(strFind);
	if(0 <= nFindedTop && (EPS_INT32)strlen(strFind) == nFindPos){
		if(!gotoEnd){
			return &strSrc[nFindedTop];
		} else{
			return &strSrc[nFindedTop] + nFindStrLen;
		}
	} else{
		return NULL;
	}
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     memStrStrWithLen()			                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* strSrc		EPS_INT8*			I: Source string									*/
/* nSrcLen		EPS_UINT32			I: Source string length								*/
/* strFind		EPS_INT8*			I: Find string. Size MUST be less than 64 byte.		*/
/*                                                                                      */
/* Return value:                                                                        */
/*				EPS_INT8*			pointer to finded string                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Finds the first occurrence of a substring within a string.						*/
/*		The comparison is NOT case sensitive.											*/
/*		Limit nSrcLen characters.														*/
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT8* memStrStrWithLen (
							
		EPS_INT8*       strSrc, 
		EPS_UINT32      nSrcLen, 
		const EPS_INT8* strFind
		
){
	EPS_INT8	strShadow[64];
	EPS_INT32	nSrcPos = 0;
	EPS_INT32	nFindPos = 0;
	EPS_INT32	nFindedTop = -1;

	if(NULL == strSrc || 64 <= strlen(strFind)){
		return NULL;
	}

	memset(strShadow, 0, sizeof(strShadow));

	/* create shadow string */
	do{
		if( 0x41 <= strFind[nFindPos] && strFind[nFindPos] <= 0x5A){
			strShadow[nFindPos] = (EPS_INT8)(strFind[nFindPos] + 0x20);
		} else if( 0x61 <= strFind[nFindPos] && strFind[nFindPos] <= 0x7A){
			strShadow[nFindPos] = (EPS_INT8)(strFind[nFindPos] - 0x20);
		} else{
			strShadow[nFindPos] = strFind[nFindPos];
		}
	}while('\0' != strFind[nFindPos++]);

	/* search by strFind & strShadow */
	nFindPos = 0;
	while( (EPS_INT32)nSrcLen > nSrcPos && '\0' != strFind[nFindPos]){
		if( strSrc[nSrcPos] == strFind[nFindPos]
			|| strSrc[nSrcPos] == strShadow[nFindPos] ){
			if(-1 == nFindedTop){
				nFindedTop = nSrcPos;
			}

			nSrcPos++;
			nFindPos++;
		} else{
			if(-1 == nFindedTop){
				nSrcPos++;
			} else{
				nSrcPos = nFindedTop + 1;
			}
			nFindPos = 0;
			nFindedTop = -1;
		}
	}

	if(0 <= nFindedTop && (EPS_INT32)strlen(strFind) == nFindPos){
		return &strSrc[nFindedTop];
	} else{
		return NULL;
	}
}


/*******************************************|********************************************/
/* Function name:   memSetEndian()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* Endianess    EPS_ENDIAN          I: Desired Endianess (Big/Little)                   */
/* byteSize     EPS_BYTE_SIZE       I: 2-byte or 4 bytes to convert                     */
/* value        EPS_UINT32          I: 4 Bytes to be swapped if necesaray               */
/* array        EPS_UINT8*          O: Correct endian-ness bytes                        */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Swap data depending on endian-ness.                                             */
/*                                                                                      */
/*******************************************|********************************************/
void     memSetEndian (

        EPS_ENDIAN      Endianess,          /* Desired Endianess (Big/Little)           */
        EPS_BYTE_SIZE   byteSize,           /* 2-byte or 4 bytes to convert             */
        EPS_UINT32      value,              /* 4 Bytes to be swapped if necesaray       */
        EPS_UINT8*      array               /* Correct endian-ness bytes                */

){

/*** Declare Variable Local to Routine                                                  */
    EPS_UINT16  value2byte;
    EPS_UINT32  value4byte;

/*** Initialize Local Variables                                                         */

/*** Based on desired Eniandess - Perform test and swap, if necessary                   */
    switch (byteSize + Endianess) {
        /*** Change 2 bytes value to the little endianness                              */
        case (EPS_2_BYTES + EPS_ENDIAN_LITTLE):
#if 0  /* Not Used */
            value2byte = (EPS_UINT16)value;
            array[0]   = (EPS_UINT8)((value2byte     ) & 0x00ff);
            array[1]   = (EPS_UINT8)((value2byte >> 8) & 0x00ff);
#endif
            break;
        /*** Change 2 bytes value to the big endianness                                 */
        case (EPS_2_BYTES + EPS_ENDIAN_BIG):
            value2byte = (EPS_UINT16)value;
            array[0]   = (EPS_UINT8)((value2byte >> 8) & 0x00ff);
            array[1]   = (EPS_UINT8)((value2byte     ) & 0x00ff);
            break;
        /*** Change 4 bytes value to the little endianness                              */
        case (EPS_4_BYTES + EPS_ENDIAN_LITTLE):
            value4byte = (EPS_UINT32)value;
            array[0]   = (EPS_UINT8)((value4byte      ) & 0x000000ff);
            array[1]   = (EPS_UINT8)((value4byte >>  8) & 0x000000ff);
            array[2]   = (EPS_UINT8)((value4byte >> 16) & 0x000000ff);
            array[3]   = (EPS_UINT8)((value4byte >> 24) & 0x000000ff);
            break;
        /*** Change 4 bytes value to the big endianness                                 */
        case (EPS_4_BYTES + EPS_ENDIAN_BIG):
            value4byte = (EPS_UINT32)value;
            array[0]   = (EPS_UINT8)((value4byte >> 24) & 0x000000ff);
            array[1]   = (EPS_UINT8)((value4byte >> 16) & 0x000000ff);
            array[2]   = (EPS_UINT8)((value4byte >>  8) & 0x000000ff);
            array[3]   = (EPS_UINT8)((value4byte      ) & 0x000000ff);
            break;
        default:
            break;
    }
}


/*******************************************|********************************************/
/* Function name:   memInspectEndian()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* (None)                                                                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Determine "Endian-ness" for the current cpu.                                    */
/*                                                                                      */
/*******************************************|********************************************/
void   memInspectEndian (

        void

){
	union {
		EPS_INT8    array[2];                   /* Endian-ness test array               */
		EPS_INT16   chars;                      /* Endian-ness test string              */
	} EndianTest;

    EndianTest.array[0] = 'a';    
    EndianTest.array[1] = 'b';
	if (EndianTest.chars == 0x6162) {
		cpuEndian = EPS_ENDIAN_BIG;
	} else{
        cpuEndian = EPS_ENDIAN_LITTLE;
	}
}


EPS_INT32 memGetBitCount(

		EPS_INT32 bitfield

){
	EPS_INT32	i = 0;
	EPS_INT32	cnt = 0;

	for(i = 0; i < sizeof(bitfield)*8; i++){
		if( (bitfield >> i) & 1 ){
			cnt++;
		}
	}

	return cnt;
}


EPS_UINT8 memSearchWhiteColorVal(
		
		EPS_UINT8	colorPlane,
		EPS_UINT8*	paletteData,
		EPS_UINT16	paletteSize
		
){
	EPS_UINT8	whiteColorValue = 255;	/* Set default index for white to negative one  */
	EPS_UINT16   jdx;					/* General indes/loop variable              */
	EPS_UINT8*  bufPtr = NULL;			/* Temporary buffer pointer                 */

	if( colorPlane == EPS_CP_256COLOR ){ 
        bufPtr = paletteData;
        for ( jdx =  0; jdx < paletteSize/3; jdx++ ) {
            if (*bufPtr == 255 && *(bufPtr+1) == 255 && *(bufPtr+2) == 255) {
                whiteColorValue = (EPS_UINT8)jdx;
                break;
            }
            bufPtr += 3;
        }
	} else /* if(EPS_CP_FULLCOLOR) */{
		whiteColorValue = 0xFF;
	}

	return whiteColorValue;
}

/*________________________________  epson-escpr-mem.c  _________________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
