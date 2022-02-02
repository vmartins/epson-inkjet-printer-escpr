/*_______________________________   epson-escpr-pm.c   _________________________________*/

/*       1         2         3         4         5         6         7         8        */
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*******************************************|********************************************/
/*
 *   Copyright (c) 2014  Seiko Epson Corporation                 All rights reserved.
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
/*                         Epson ESC/PR print media Functions                           */
/*                                                                                      */
/*******************************************|********************************************/

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-err.h"
#include "epson-escpr-mem.h"
#include "epson-escpr-media.h"
#include "epson-escpr-services.h"
#include "epson-layout.h"


/*------------------------------------  Definition   -----------------------------------*/
/*******************************************|********************************************/
#ifdef EPS_LOG_MODULE_SER
#define EPS_LOG_MODULE	EPS_LOG_MODULE_SER
extern EPS_CMN_FUNC    epsCmnFnc;
#else
#define EPS_LOG_MODULE	0
#endif

    /*** Index of printing quality                                                      */
    /*** -------------------------------------------------------------------------------*/
#define Q_DRAFT                         0
#define Q_NORMAL                        1
#define Q_HIGH                          2

typedef struct _tagEPS_PRINTAREA_SOURCE_ {
    const EPS_MEDIA_INFO* pMI;
	EPS_UINT32  resolution;
	EPS_INT32	factor;
	/* for user defined size */
    EPS_INT32   defaultBorder;
	EPS_INT32   borderlessBottom;
} EPS_PRINTAREA_SOURCE;

/*----------------------------  ESC/P-R Lib Global Variables  --------------------------*/
/*******************************************|********************************************/
extern EPS_CMN_FUNC		epsCmnFnc;

/*------------------------------  Local Global Variables  ------------------------------*/
/*******************************************|********************************************/
static const EPS_UINT8 PM_REPLY_HEADER[EPS_PM_HEADER_LEN] = {
      /*  @     B     D     C   <SP>    P     M   <CR>  <LF> */
        0x40, 0x42, 0x44, 0x43, 0x20, 0x50, 0x4D, 0x0D, 0x0A
    };

/*---------------------------  Special PM String table   -------------------------------*/
/*******************************************|********************************************/
    /*** PM string resource ID                                                          */
    /*** -------------------------------------------------------------------------------*/
#define EPS_PMS_E300	            (1)
#define EPS_PMS_E500	            (2)
#define EPS_PMS_E700	            EPS_PMS_E500
#define EPS_PMS_PM200	            (3)
#define EPS_PMS_PM240	            (4)

typedef	struct _tagSP_PM_STRINGS
{
	EPS_UINT32		 id;	/* ID */
	const EPS_UINT8* res;	/* PM String */
	EPS_UINT32		 len;	/* length */
}SP_PM_STRINGS;

const EPS_UINT8 spm_E300[] = {0x40, 0x42, 0x44, 0x43, 0x20, 0x50, 0x4D, 0x0D, 0x0A, 0x53, 0x0F, 0x54, 0x26, 0x82, 0x2F, 0x54,
							0x0B, 0x82, 0x2F, 0x54, 0x2B, 0x82, 0x2F, 0x2F, 0x53, 0x0A, 0x54, 0x26, 0x82, 0x2F, 0x54, 0x0B,
							0x82, 0x2F, 0x54, 0x2B, 0x82, 0x2F, 0x2F, 0x53, 0x10, 0x54, 0x0B, 0x82, 0x2F, 0x54, 0x08, 0x02, 
							0x2F, 0x2F, 0x53, 0x23, 0x54, 0x0B, 0x82, 0x2F, 0x2F, 0x0D, 0x0A};
const EPS_UINT8 spm_E500[] = {0x40, 0x42, 0x44, 0x43, 0x20, 0x50, 0x4D, 0x0D, 0x0A, 0x53, 0x0F, 0x54, 0x26, 0x82, 0x2F, 0x54,
							0x0B, 0x82, 0x2F, 0x54, 0x2B, 0x82, 0x2F, 0x2F, 0x53, 0x0A, 0x54, 0x26, 0x82, 0x2F, 0x54, 0x0B,
							0x82, 0x2F, 0x54, 0x2B, 0x82, 0x2F, 0x2F, 0x53, 0x2B, 0x54, 0x0B, 0x82, 0x2F, 0x2F, 0x53, 0x10,
							0x54, 0x0B, 0x82, 0x2F, 0x54, 0x08, 0x02, 0x2F, 0x2F, 0x53, 0x23, 0x54, 0x0B, 0x82, 0x2F, 0x2F,
							0x0D, 0x0A};
const EPS_UINT8 spm_PM200[] = {0x40, 0x42, 0x44, 0x43, 0x20, 0x50, 0x4D, 0x0D, 0x0A, 0x53, 0x0A, 0x54, 0x26, 0x82, 0x2F, 0x54,
                            0x0B, 0x82, 0x2F, 0x54, 0x2B, 0x82, 0x2F, 0x2F, 0x53, 0x10, 0x54, 0x08, 0x02, 0x2F, 0x2F, 0x0D, 
							0x0A};
const EPS_UINT8 spm_PM240[] = {0x40, 0x42, 0x44, 0x43, 0x20, 0x50, 0x4D, 0x0D, 0x0A, 0x53, 0x0A, 0x54, 0x26, 0x82, 0x2F, 0x54, 
                            0x0B, 0x82, 0x2F, 0x54, 0x2B, 0x82, 0x2F, 0x2F, 0x53, 0x2B, 0x54, 0x0B, 0x82, 0x2F, 0x2F, 0x53, 
							0x10, 0x54, 0x08, 0x42, 0x2F, 0x2F, 0x0D, 0x0A};

const SP_PM_STRINGS	spPMStrTbl[] = 
{
	{ EPS_PMS_E300,	 spm_E300,  sizeof(spm_E300)	},
	{ EPS_PMS_E500,  spm_E500,  sizeof(spm_E500)	},
	{ EPS_PMS_PM200, spm_PM200, sizeof(spm_PM200)	},
	{ EPS_PMS_PM240, spm_PM240, sizeof(spm_PM240)	}
};

const EPS_UINT32 EPS_SPM_STRINGS = sizeof(spPMStrTbl) / sizeof(SP_PM_STRINGS);

/*--------------------------------  Local Functions   ----------------------------------*/
/*******************************************|********************************************/
static EPS_INT32    _SP_LoadPMString                (EPS_UINT32, EPS_UINT8*, EPS_UINT32 );
static EPS_ERR_CODE _SP_ChangeSpec_UpdatePMReply(EPS_PRINTER_INN*, EPS_UINT8*, EPS_INT32);
static EPS_INT32    _pmFindSfield                   (EPS_UINT8, EPS_UINT8*, EPS_UINT8**, EPS_UINT8**);
static EPS_UINT8*   _pmScanTfield                   (EPS_UINT8, EPS_UINT8*              );
static EPS_INT16    _pmAppendTfield                 (EPS_UINT8*, EPS_UINT8*             );
static void         _pmValidateRemoveDelimiter      (EPS_UINT8*, EPS_UINT8*, EPS_INT32  );
static EPS_INT16    _pmValidateRemoveUnknownSfield  (EPS_UINT8*, EPS_UINT8*             );
static void         _pmCorrectUnknownTfield         (EPS_UINT8*, EPS_UINT8*             );
static void         _pmCorrectDupulicatedFields     (EPS_UINT8*, EPS_UINT8*, EPS_INT32* );
static void         _pmAdjustQuality                (EPS_UINT8*                         );
static void         AppendMedia                     (EPS_SUPPORTED_MEDIA*               );
static EPS_UINT32	ComplementPaperSource	        (EPS_INT32                          );

static EPS_ERR_CODE GetPrintAreaSource              (EPS_UINT32, EPS_PRINTAREA_SOURCE*	);
static EPS_ERR_CODE CalcPrintMargin                 (EPS_BOOL, EPS_PRINTAREA_SOURCE*, 
	                                                 const EPS_JOB_ATTRIB*,
	                                                 EPS_LAYOUT_INFO*, EPS_RECT*        );
static EPS_INT32    LeftBorderPixel                 (EPS_PRINTAREA_SOURCE*              );
static EPS_INT32    RightBorderPixel                (EPS_PRINTAREA_SOURCE*              );
static EPS_INT32    TopBorderPixel                  (EPS_PRINTAREA_SOURCE*              );
static EPS_INT32    BottomBorderPixel               (EPS_PRINTAREA_SOURCE*              );

static EPS_ERR_CODE pm3GetPrintMargin               (EPS_PM_DATA*, const EPS_JOB_ATTRIB*, 
                                                     EPS_LAYOUT_INFO*, EPS_RECT*        );
static EPS_ERR_CODE pm3ScanSfield                   (EPS_UINT8*, EPS_INT32, EPS_INT32*,
                                                     EPS_PRINTAREA_SOURCE*, EPS_INT32, 
                                                     EPS_INT32, EPS_PRINT_AREA_INFO*    );
static EPS_ERR_CODE pm3ScanTfield                   (EPS_UINT8*, EPS_INT32, EPS_INT32*, 
                                                     EPS_INT32*, EPS_UINT8**, EPS_UINT32*);
static EPS_ERR_CODE pm3ScanLfield                   (EPS_UINT8*, EPS_INT32, EPS_INT32*,
                                                     EPS_UINT8*, EPS_INT32, EPS_SUPPORTED_MEDIA*);
static EPS_ERR_CODE pm3ScanPfield                   (EPS_UINT8*, EPS_INT32, EPS_INT32*,
                                                     EPS_INT32*                         );
static EPS_ERR_CODE pm3GetTfieldValue               (EPS_UINT8*, EPS_INT32, EPS_INT32*,
                                                     EPS_INT32*, EPS_UINT8**, EPS_UINT32*);
static EPS_UINT8*   pm3FindTfield                   (EPS_UINT8*, EPS_INT32, EPS_UINT8   );
static EPS_ERR_CODE pm3ChangeSpec_UpdatePMReply     (EPS_PRINTER_INN*, EPS_UINT8*, EPS_INT32*);

/*------------------------------------  Debug Dump   -----------------------------------*/
/*******************************************|********************************************/
    /*** ALL Debug Dump Switch for <epson-escpr-pm.c>                              */
    /*** -------------------------------------------------------------------------------*/
#define _ESCPR_DEBUG_SP             0       /* 0: OFF    1: ON                          */
#define _ESCPR_DEBUG_SP_VERBOSE     0       /* 0: OFF    1: ON                          */

    /*** _ESCPR_DEBUG_SP --- Definition of << DUMP_PMREPLY() >>                         */
    /*** -------------------------------------------------------------------------------*/
#if _ESCPR_DEBUG_SP | _VALIDATE_SUPPORTED_MEDIA_DATA_

typedef enum _DUMP_TYPE {
    DUMP_HEX = 0,
    DUMP_ASCII,
    DUMP_S_TAG_ONLY,
} DUMP_TYPE;

char* str[] ={
                 "DUMP_HEX",
                 "DUMP_ASCII",
                 "DUMP_S_TAG_ONLY",
             };

static void print_PMREPLY(EPS_UINT8* pm, DUMP_TYPE type, EPS_INT8* msg)
{
    EPS_UINT8* p = pm;
    EPS_INT16 col = 0;

    if(*p != 'S') {
        if(type != DUMP_HEX) {
            return; /* do not anything */
        }

        /* Anyway if type is DUMP_HEX then dump it */
    }

    printf("%s\r\n", msg);
    printf("PM REPLY DUMP [TYPE:%s]\r\n", str[type]);

    if(type == DUMP_HEX) {
        while(!((*p == 0x0D) && (*(p+1) == 0x0A))) {
            printf("0x%02X ",   *p++);

            if((++col % 10) == 0) {
                printf("\r\n");
            }
        }

    } else {
        while(*p == 'S') {
            printf("%c ",   *p++);
            printf("%02d\r\n", *p++);
            while(*p == 'T') {
                printf("  %c",     *p++);
                printf("  %02d",   *p++);
                printf("  [0x%02X]", *p++);
                printf("  %c\r\n",     *p++);
            }
            printf("%c\r\n",     *p++);

            if(type == DUMP_S_TAG_ONLY) {
                break;
            }

            if ((*p == 0x0D) && (*(p+1) == 0x0A)) {
                break;
            }
        }

    }

    if(type != DUMP_S_TAG_ONLY) {
        printf("0x%02X ",   *p++);
        printf("0x%02X ",   *p);
    }

    printf("\r\nEND\r\n");

}
#endif

#if _ESCPR_DEBUG_SP
#define DUMP_PMREPLY(a) print_PMREPLY a
#else
#define DUMP_PMREPLY(a)
#endif

    /*** _ESCPR_DEBUG_SP_VERBOSE                                                        */
    /***    --- Definition of << VERBOSE_DUMP_PMREPLY() >>  and << verbose_dbprint() >> */
    /*** -------------------------------------------------------------------------------*/
#if _ESCPR_DEBUG_SP_VERBOSE
#define VERBOSE_DUMP_PMREPLY(a) print_PMREPLY a
#define verbose_dbprint(a)      EPS_DBGPRINT(a)
#else
#define VERBOSE_DUMP_PMREPLY(a)
#define verbose_dbprint(a)  
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
/* Function name:   epspmCreateMediaInfo()                                              */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:               Description:									*/
/* printer          EPS_PRINTER_INN*    I: printer that it has original structure       */
/* pmString			EPS_INT8*			I: PM reply string                              */
/* pmSize			EPS_INT32			I: size of PM reply string                      */
/* needPaperSource	EPS_BOOK*			o: extend paper source is exist                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Alloc memory failed                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Marge paper source to EPS_SUPPORTED_MEDIA.                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epspmCreateMediaInfo(
	
		EPS_PRINTER_INN*	printer,
		EPS_UINT8*			pmString,
		EPS_INT32			pmSize,
		EPS_BOOL*           extPaper
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
    EPS_INT32       pmIdx;                      /* pm string index                      */
    EPS_INT32       idx;                        /* Index                                */
    EPS_INT32       sIdx = 0;                   /* Media size index                     */
    EPS_INT32       tIdx = 0;                   /* Media type index                     */
    EPS_INT32       num_mType = 0;              /* Media type number                    */
    EPS_INT32       num_mSize = 0;              /* Media size number                    */
    EPS_BOOL        breakParse = FALSE;

	EPS_LOG_FUNCIN;

#if _VALIDATE_SUPPORTED_MEDIA_DATA_
	if(printer->supportedMedia.numSizes != -1){
		/*** "Filter" Raw "PM" data (Remake the correct pm stirng)                      */
		ret = _SP_ChangeSpec_UpdatePMReply(printer, pmString, pmSize);
		if (ret != EPS_ERR_NONE) {
			EPS_RETURN( ret );
		}
	}
#else
/*** "Filter" Raw "PM" data (Remake the correct pm stirng)                              */
    ret = _SP_ChangeSpec_UpdatePMReply(printer, pmString, pmSize);
    if (ret != EPS_ERR_NONE) {
        EPS_RETURN( EPS_ERR_OPR_FAIL );  /* Invalid format */
    }
#endif

/*** Create the structure of the support media                                          */
	printer->supportedMedia.resolution = EPS_IR_360X360;	/* default support */

    /*** Count "Paper Size" field  & check format */
	pmIdx = EPS_PM_HEADER_LEN;				/* skip the command header of pm string     */
    while( pmIdx < pmSize && FALSE == breakParse) {
        switch(pmString[pmIdx]) {
		case 'R':
			if( 720 == ((pmString[pmIdx+1] << 8) + pmString[pmIdx+2]) ){
				printer->supportedMedia.resolution |= EPS_IR_720X720;
			} else if( 600 == ((pmString[pmIdx+1] << 8) + pmString[pmIdx+2]) ){
				printer->supportedMedia.resolution |= EPS_IR_300X300 | EPS_IR_600X600;
			} else if( 300 == ((pmString[pmIdx+1] << 8) + pmString[pmIdx+2]) ){
				printer->supportedMedia.resolution |= EPS_IR_300X300;
			}
			pmIdx += 6;
			break;

		case 'M':
			printer->supportedMedia.JpegSizeLimit = 
				(pmString[pmIdx+1] << 24) + (pmString[pmIdx+2] << 16) + (pmString[pmIdx+3] << 8) + pmString[pmIdx+4]; 
			printer->JpgMax = printer->supportedMedia.JpegSizeLimit;
			pmIdx += 6;
			break;

		case 'S':
			/* move T field */
			if(pmIdx < pmSize-2){
				pmIdx += 2;
			} else{
				EPS_RETURN( EPS_ERR_OPR_FAIL );
			}

			num_mSize++;

			for(; pmIdx < pmSize-4; pmIdx += 4) {	/* 4 = T x x / */
				if(pmString[pmIdx] == '/'){
					pmIdx += 1;
					break;
				} else if(pmString[pmIdx] != 'T') {
					EPS_RETURN( EPS_ERR_OPR_FAIL );
				}
			}
			break;

		default:
			breakParse = TRUE; /* unknown field */
			break;
		}

		if(FALSE == breakParse){
			/* If we run into an occurrence of carriage return followed by line feed,
			 * we have found the terminating characters of the string. */
			if(pmString[pmIdx] == 0x0D && pmString[pmIdx+1] == 0x0A) {
				break;
			}
		}
	}                      
    
    /* Allocate memory for the media size list. */
    printer->supportedMedia.sizeList =
            (EPS_MEDIA_SIZE*)EPS_ALLOC( sizeof(EPS_MEDIA_SIZE) * num_mSize );   
    if(NULL == printer->supportedMedia.sizeList){
        EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
    }
	memset(printer->supportedMedia.sizeList, 0, sizeof(EPS_MEDIA_SIZE) * num_mSize);
    printer->supportedMedia.numSizes = num_mSize;
    
	pmIdx = EPS_PM_HEADER_LEN;				/* skip the command header of pm string     */
    for(sIdx = 0; sIdx < num_mSize; sIdx++) {
		if(pmString[pmIdx] == 'M' || pmString[pmIdx] == 'R') {
			pmIdx += 6;
			sIdx--;
			continue;
		}

        printer->supportedMedia.sizeList[sIdx].mediaSizeID = pmString[pmIdx+1];
/*		EPS_DBGPRINT(("Size=%d\r\n", pmString[pmIdx+1]));*/
		pmIdx += 2;

        /* For the given paper type, iterate through the paper type to get the number
         * of media types contained in it */
        num_mType = 0;
		for(idx = pmIdx; idx < pmSize-4; idx += 4) {
			if(pmString[idx] == '/'){
				idx += 1;
				break;
			}
			num_mType++;

		}

        /* Allocate memory for the media type array. */
        printer->supportedMedia.sizeList[sIdx].typeList = 
            (EPS_MEDIA_TYPE*)EPS_ALLOC( sizeof(EPS_MEDIA_TYPE) * num_mType );
        
        if (printer->supportedMedia.sizeList[sIdx].typeList == NULL) {
            EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
        }
        
		memset(printer->supportedMedia.sizeList[sIdx].typeList, 0, sizeof(EPS_MEDIA_TYPE) * num_mType);
        printer->supportedMedia.sizeList[sIdx].numTypes = num_mType;

		for(tIdx = 0; tIdx < num_mType; tIdx++) {
#ifndef GCOMSW_USE_3DMEDIA
			if( EPS_MTID_3D == pmString[pmIdx+1] ){
				printer->supportedMedia.sizeList[sIdx].numTypes--;
				num_mType--; tIdx--;
				pmIdx += 4;	/* move next field */
				continue;
			}
#endif
#ifndef GCOMSW_USE_ROLLE_PAPER
			if( EPS_MTID_PLAIN_ROLL_STICKER  == pmString[pmIdx+1] ||
				EPS_MTID_GROSSY_ROLL_STICKER == pmString[pmIdx+1] ){
				printer->supportedMedia.sizeList[sIdx].numTypes--;
				pmIdx += 4;	/* move next field */
				continue;
			}
#endif
            printer->supportedMedia.sizeList[sIdx].typeList[tIdx].mediaTypeID       = pmString[pmIdx+1];
/*            EPS_DBGPRINT(("\tType=%d (%02X)\r\n", pmString[pmIdx+1], pmString[pmIdx+2]));*/
                               
            /* Bitwise OR with 10000000 - Check for borderless */
            if( pmString[pmIdx+2] & 0x80 ){
                printer->supportedMedia.sizeList[sIdx].typeList[tIdx].layout |= EPS_MLID_BORDERLESS;
            }
            /* Bitwise OR with 01000000 - Check for border "disable" mode */
            if( !(pmString[pmIdx+2] & 0x40) ){
				printer->supportedMedia.sizeList[sIdx].typeList[tIdx].layout |= EPS_MLID_BORDERS;
            }

            /* set quality */
            printer->supportedMedia.sizeList[sIdx].typeList[tIdx].quality |= (pmString[pmIdx+2] & EPS_MQID_ALL);

			/* set duplex */
            if( pmString[pmIdx+2] & 0x10 &&
				obsEnableDuplex(printer->supportedMedia.sizeList[sIdx].mediaSizeID) ){
				printer->supportedMedia.sizeList[sIdx].typeList[tIdx].duplex = EPS_DUPLEX_ENABLE;/*EPS_DUPLEX_SHORT*/;
			} else{
				printer->supportedMedia.sizeList[sIdx].typeList[tIdx].duplex = EPS_DUPLEX_DISABLE;
			}

			/* Bitwise OR with 00001000 - Check for extend paper source */
#if _VALIDATE_SUPPORTED_MEDIA_DATA_
            if( pmString[pmIdx+2] & 0x08 ){
                *extPaper = TRUE;
			} else {
				/* DEFAULT. All printer support rear paper source */
				printer->supportedMedia.sizeList[sIdx].typeList[tIdx].paperSource = EPS_MPID_REAR;
			}

			/* param2 value check */
			if( !(pmString[pmIdx+2] & (0x01 | 0x02 | 0x04)) ){
				printf("\n\n!!!!!!!!!  Quality is not described. !!!!!!!!!\n"
						"SizeID=0x%02X / TypeID=0x%02X / param2=0x%02X\n", 
						printer->supportedMedia.sizeList[sIdx].mediaSizeID,
						pmString[pmIdx+1], pmString[pmIdx+2]);
			} 
			if( !(pmString[pmIdx+2] & 0x80) && (pmString[pmIdx+2] & 0x40) ){
				printf("\n\n!!!!!!!!!  Layout is not described. !!!!!!!!!\n"
						"SizeID=0x%02X / TypeID=0x%02X / param2=0x%02X\n", 
						printer->supportedMedia.sizeList[sIdx].mediaSizeID,
						pmString[pmIdx+1], pmString[pmIdx+2]);
			}

#else
            if( pmString[pmIdx+2] & 0x08 ){
                *extPaper = TRUE;
			}
			/* DEFAULT. All printer support rear paper source */
			printer->supportedMedia.sizeList[sIdx].typeList[tIdx].paperSource = EPS_MPID_REAR;
#endif
			pmIdx += 4;	/* move next field */
		}
		pmIdx += 1;	/* skip terminater */
	}

/*** Add extend infomation                                                              */
	AppendMedia(&printer->supportedMedia);

	EPS_RETURN( ret );
}


void    epspmClearMediaInfo(
	
		EPS_SUPPORTED_MEDIA*		supportedMedia

){
    EPS_INT32 idx;
	if( NULL != supportedMedia->sizeList ){
		for(idx = 0; idx < supportedMedia->numSizes; idx++) {
			EPS_SAFE_RELEASE(supportedMedia->sizeList[idx].typeList);
		}
		EPS_SAFE_RELEASE(supportedMedia->sizeList);
		supportedMedia->numSizes = 0;
	}
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epspmMargePaperSource()                                             */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                  Description:                                 */
/* printer          EPS_PRINTER_INN*       I: printer that it has original structure    */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*                                                                                      */
/* Description:                                                                         */
/*      Marge paper source to EPS_SUPPORTED_MEDIA.                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epspmMargePaperSource(
	
		EPS_PRINTER_INN*	printer,
		EPS_UINT8*			pmString,
		EPS_INT32			pmSize
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
    EPS_INT32       pmIdx;                      /* pm string index                      */
    EPS_INT32       sIdx = 0;                   /* Media size index                     */
    EPS_INT32       tIdx = 0;                   /* Media type index                     */
	EPS_MEDIA_SIZE  *pMSize = NULL;

	EPS_LOG_FUNCIN;

    if(memcmp(pmString, PM_REPLY_HEADER, EPS_PM_HEADER_LEN) != 0) {
        EPS_DBGPRINT(("_SP_ChangeSpec_UpdatePMReply > EPS_ERR_SP_INVALID_HEADER\r\n"));
        EPS_RETURN( EPS_ERR_SP_INVALID_HEADER );
    }

	/* Delete the command header of pm string                                           */
	pmIdx = EPS_PM_HEADER_LEN;				/* skip the command header of pm string     */

/*** Check to make sure the PM reply has a valid beginning                              */
    if(pmString[pmIdx] != 'S' && pmString[pmIdx+2] != 'T') {
        EPS_RETURN( EPS_ERR_OPR_FAIL );
    }
    
/*** Create the structure of the support media                                          */
    /*** Count "Paper Size" field  & check format */
    for(; pmIdx < pmSize-7; ) {		/* 7 = S x T x x // */
        if(pmString[pmIdx] != 'S') {
			EPS_RETURN( EPS_ERR_OPR_FAIL ); /* bad format */
		}
		
		/* search size ID */
		pmIdx++;
		pMSize = NULL;
	    for(sIdx = 0; sIdx < printer->supportedMedia.numSizes; sIdx++){
			if(pmString[pmIdx] == printer->supportedMedia.sizeList[sIdx].mediaSizeID){
				pMSize = &printer->supportedMedia.sizeList[sIdx];
/*				EPS_DBGPRINT(("Size = %d\n", printer->supportedMedia.sizeList[sIdx].mediaSizeID))*/
				break;
			}
		}

		pmIdx++;	/* move next field */

		while( pmIdx < pmSize ){
			if(pmString[pmIdx] == 'T'){
				if(NULL != pMSize){
					/* search type ID */
					pmIdx++;
					for(tIdx = 0; tIdx < pMSize->numTypes && pmIdx < pmSize-4; tIdx++){	/* 4 = T x x / */
						if(pmString[pmIdx] == pMSize->typeList[tIdx].mediaTypeID){
							pMSize->typeList[tIdx].paperSource = pmString[pmIdx+1];
				            if( 0 == pMSize->typeList[tIdx].paperSource ){
								pMSize->typeList[tIdx].paperSource = ComplementPaperSource(pMSize->mediaSizeID);
							}
				            if( obsEnableAutoFeed() ){
								pMSize->typeList[tIdx].paperSource |= EPS_MPID_AUTO;
							}
#if !_VALIDATE_SUPPORTED_MEDIA_DATA_
							pMSize->typeList[tIdx].paperSource &= EPS_MPID_ALL_ESCPR;
#endif
							pmIdx += 3;
							break;
						}
					}
					if(tIdx >= pMSize->numTypes){
						/* Skip unknown T */
#if _VALIDATE_SUPPORTED_MEDIA_DATA_
						printf("\n\n!!!!!!!!! pm2 contains TypeID(0x%02X) that doesn't exist in pm1.  !!!!!!!!!\n", pmString[pmIdx]);
#endif
						pmIdx += 3;
					}
				} else{
					/* Skip unknown S */
#if _VALIDATE_SUPPORTED_MEDIA_DATA_
						printf("\n\n!!!!!!!!! pm2 contains SizeID(0x%02X) that doesn't exist in pm1.  !!!!!!!!!\n", pmString[pmIdx]);
#endif
					pmIdx += 4;
				}
			} else if(pmString[pmIdx] == '/') {
				pmIdx++;
				break;
			} else{
				EPS_RETURN( EPS_ERR_OPR_FAIL ); /* bad format */
			}
		}
		
		/* If we run into an occurrence of carriage return followed by line feed,
		 * we have found the terminating characters of the string. */
		if(pmString[pmIdx] == 0x0D && pmString[pmIdx+1] == 0x0A) {
			break;
		}
	}                      
	if(pmIdx >= pmSize){
		EPS_RETURN( EPS_ERR_OPR_FAIL ); /* bad format */
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epspmGetPrintAreaInfo()                                             */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                  Description:                                 */
/* supportedMedia   EPS_SUPPORTED_MEDIA*   I : supported media structure                */
/* resolution       EPS_UINT32             I : input resolution                         */
/* layoutInfo       EPS_LAYOUT_INFO        O : print area information structure         */
/* baseBorder       EPS_RECT*              O : custom border base margin for print      */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_INV_INPUT_RESOLUTION    - Invalid Input Resolution                      */
/*      EPS_ERR_INV_MEDIA_SIZE          - Invalid Media Size                            */
/*      EPS_ERR_INV_BORDER_MODE         - Invalid Border Mode                           */
/*      EPS_ERR_INV_TOP_MARGIN          - Invalid Top Magirn                            */
/*      EPS_ERR_INV_LEFT_MARGIN         - Invalid Left Margin                           */
/*      EPS_ERR_INV_BOTTOM_MARGIN       - Invalid Bottom Margin                         */
/*      EPS_ERR_INV_RIGHT_MARGIN        - Invalid Right Margin                          */
/*      EPS_ERR_INV_CD_OUTDIM           - Invalid CD Outside Diameter                   */
/*      EPS_ERR_MEMORY_ALLOCATION       - Alloc memory failed                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Create print size and margin information.                                       */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epspmGetPrintAreaInfo(
	
		EPS_BOOL			preview,
		const EPS_JOB_ATTRIB* pAttr,
		EPS_PM_DATA*		pmData,
        EPS_UINT32*         paperWidth,
        EPS_UINT32*         paperHeight,
		EPS_LAYOUT_INFO*	layoutInfo,
		EPS_RECT*			baseBorder

){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
	EPS_PRINTAREA_SOURCE	src;
	EPS_INT32		idx;
	EPS_MEDIA_INFO	tmpMI;

	EPS_LOG_FUNCIN;

	ret = GetPrintAreaSource(pAttr->inputResolution, &src);
    if (EPS_ERR_NONE != ret) {
        EPS_RETURN( ret );
    }

	for (idx = 0; src.pMI[idx].id != -1; idx++) {
		if (src.pMI[idx].id == pAttr->mediaSizeIdx)
			break;
	}
	if (src.pMI[idx].id == -1) {
        EPS_RETURN( EPS_ERR_INV_MEDIA_SIZE );
	}

	if(EPS_MSID_USER != src.pMI[idx].id){
		src.pMI = &src.pMI[idx]; /* point target */
		*paperWidth  = src.pMI->paper_x * src.factor;
		*paperHeight = src.pMI->paper_y * src.factor;

	} else{
		/* temparaly media info for user-defined size */
		tmpMI.id = EPS_MSID_USER;
		tmpMI.paper_x = *paperWidth  = pAttr->userDefWidth;
		tmpMI.paper_y = *paperHeight = pAttr->userDefHeight;
		tmpMI.print_area_x_border = *paperWidth - src.defaultBorder * src.factor * 2;
		tmpMI.print_area_y_border = *paperHeight - src.defaultBorder * src.factor * 2;
		tmpMI.left_margin_borderless = src.pMI[idx].left_margin_borderless;
		tmpMI.top_margin_borderless  = src.pMI[idx].top_margin_borderless;
		tmpMI.print_area_x_borderless = tmpMI.paper_x - tmpMI.left_margin_borderless * 2;
		tmpMI.print_area_y_borderless = tmpMI.paper_y 
												- tmpMI.top_margin_borderless - src.borderlessBottom;
		src.pMI = &tmpMI; /* point target */

		*paperWidth  = pAttr->userDefWidth;
		*paperHeight = pAttr->userDefHeight;
	}

	if(EPS_MLID_CDLABEL == pAttr->printLayout){
		if(preview){
			*paperWidth  = 
			*paperHeight = elGetDots(pAttr->inputResolution, EPS_CDDIM_OUT_MAX*10);
		}
	}

	/*** Margin                                                                     */
	layoutInfo->layout = pAttr->printLayout;
	if( ( NULL != pmData && 3 <= pmData->version ) &&
		( EPS_MLID_BORDERLESS != pAttr->printLayout && 
		  EPS_MLID_CDLABEL    != pAttr->printLayout) )
	{		/*** from pm3 */
		ret = pm3GetPrintMargin(pmData, pAttr, layoutInfo, baseBorder);
	} else{ /*** from table */
		ret = CalcPrintMargin(preview, &src, pAttr, layoutInfo, baseBorder);
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epspmCreatePrintAreaInfoFromTable()                                 */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                  Description:                                 */
/* supportedMedia   EPS_SUPPORTED_MEDIA*   I : supported media structure                */
/* resolution       EPS_UINT32             I : input resolution                         */
/* layoutInfo       EPS_LAYOUT_INFO        O : print area information structure         */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_INV_INPUT_RESOLUTION    - Invalid Input Resolution                      */
/*      EPS_ERR_INV_MEDIA_SIZE          - Invalid Media Size                            */
/*      EPS_ERR_INV_BORDER_MODE         - Invalid Border Mode                           */
/*      EPS_ERR_INV_TOP_MARGIN          - Invalid Top Magirn                            */
/*      EPS_ERR_INV_LEFT_MARGIN         - Invalid Left Margin                           */
/*      EPS_ERR_INV_BOTTOM_MARGIN       - Invalid Bottom Margin                         */
/*      EPS_ERR_INV_RIGHT_MARGIN        - Invalid Right Margin                          */
/*      EPS_ERR_MEMORY_ALLOCATION       - Alloc memory failed                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Create print size and margin information fom internal table.                    */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epspmCreatePrintAreaInfoFromTable(
	
		const EPS_SUPPORTED_MEDIA*  supportedMedia,
		EPS_UINT32					resolution,
		EPS_PRINT_AREA_INFO*		printAreaInfo
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
	EPS_INT32		sizeNum, i, idx;
	EPS_LAYOUTSIZE_INFO *pSize;
	EPS_PRINTAREA_SOURCE	src;
	const EPS_MEDIA_INFO*	tmpMI;

	EPS_LOG_FUNCIN;

	ret = GetPrintAreaSource(resolution, &src);
    if (EPS_ERR_NONE != ret) {
        EPS_RETURN( ret );
    }

	sizeNum = (NULL != supportedMedia)?supportedMedia->numSizes:EPS_NUM_MEDIA_SIZES;
	pSize = (EPS_LAYOUTSIZE_INFO*)EPS_ALLOC(sizeof(EPS_LAYOUTSIZE_INFO) * sizeNum);
	if( pSize == NULL ){
        EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
    }
	memset(pSize, 0, sizeof(EPS_LAYOUTSIZE_INFO) * sizeNum);
	printAreaInfo->sizeList = pSize;
	printAreaInfo->numSizes = sizeNum;

	if(NULL == supportedMedia){
		idx = -1;	/* use static table */
	} else{
		idx = 0;
	}
	tmpMI = src.pMI;
	for(i = 0; i < sizeNum; i++){
		if(NULL != supportedMedia){
			for (idx = 0; tmpMI[idx].id != -1; idx++) {
				if (tmpMI[idx].id == supportedMedia->sizeList[i].mediaSizeID)
					break;
			}
		} else{
			idx++;
		}
		if (tmpMI[idx].id == -1 || tmpMI[idx].id == EPS_MSID_USER) {
			printAreaInfo->numSizes--;
			continue;
		}
		src.pMI = &tmpMI[idx]; /* point target */

		pSize->mediaSizeID = src.pMI->id;
		pSize->numLayouts = 2;
		pSize->layoutList = (EPS_LAYOUT_INFO*)EPS_ALLOC(sizeof(EPS_LAYOUT_INFO) * 2);

		pSize->paperWidth  = src.pMI->paper_x * src.factor;
		pSize->paperHeight = src.pMI->paper_y * src.factor;

		pSize->layoutList[0].layout = EPS_MLID_BORDERLESS;
		ret = CalcPrintMargin(TRUE, &src, NULL, &pSize->layoutList[0], NULL);
	    if (EPS_ERR_NONE != ret) {
	        break;
		}

		pSize->layoutList[1].layout = EPS_MLID_BORDERS;
		ret = CalcPrintMargin(TRUE, &src, NULL, &pSize->layoutList[1], NULL);
	    if (EPS_ERR_NONE != ret) {
	        break;
		}

		pSize++;
	}

	EPS_RETURN( ret );
}


void    epspmClearPrintAreaInfo(
	
		EPS_PRINT_AREA_INFO*		printAreaInfo

){
    EPS_INT32 idx;
	if( NULL != printAreaInfo->sizeList ){
		for(idx = 0; idx < printAreaInfo->numSizes; idx++) {
			EPS_SAFE_RELEASE(printAreaInfo->sizeList[idx].layoutList);
		}
		EPS_SAFE_RELEASE(printAreaInfo->sizeList);
		printAreaInfo->numSizes = 0;
	}
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
/* Function name:   _SP_ChangeSpec_UpdatePMReply()                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printer      EPS_PRINTER_INN*    I/O: Pointer to a printer infomation                */
/* orgPmString  EPS_UINT8*          I: String of the pm command reply                   */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_SP_INVALID_POINTER      - Input pointer error                           */
/*      EPS_ERR_SP_INVALID_HEADER       - pm string header error                        */
/*      EPS_ERR_SP_INVALID_TERMINATOR   - pm string terminator error                    */
/*      EPS_ERR_SP_NO_VALID_FIELD       - pm string field error                         */
/*                                                                                      */
/* Description:                                                                         */
/*      - Invalid formats       : Delete                                                */
/*      - Unknown 'S' field     : Delete                                                */
/*      - Unknown 'T' field     : Replace to PGPP-Premium Glossy Photo Paper(id:0x0b)   */
/*                                field If PGPP aleady exist its 'S' field then just    */
/*                                combine the mode property                             */
/*      - Duplicated 'S' fields : Merge together                                        */
/*      - Duplicated 'T' fields : Merge together and combine each mode properties       */
/*      - Only DRAFT mode exist : Add NORMAL mode to its print quality property         */
/*                                                                                      */
/* NOTE:                                                                                */
/*      Be sure that the pData is a pointers that a starting address of 512 bytes       */
/*      buffer should be assigned or memory acces violation should be occured.          */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE     _SP_ChangeSpec_UpdatePMReply (

		EPS_PRINTER_INN*    printer,
        EPS_UINT8*		    orgPmString,
        EPS_INT32		    bufSize

){
/*** Declare Variable Local to Routine                                                  */
    EPS_UINT8* pBefore = NULL;
    EPS_UINT8* pAfter  = NULL;
    EPS_UINT8* pSrc    = NULL;
    EPS_UINT8* pDes    = NULL;
    EPS_UINT8  tempPmString[EPS_PM_MAXSIZE];    /* Retrieved PM data from printer       */
    EPS_UINT8  paperSize;
    EPS_UINT8  paperType;
	EPS_INT32  desSize = EPS_PM_MAXSIZE - EPS_PM_HEADER_LEN;
#if _VALIDATE_SUPPORTED_MEDIA_DATA_
	EPS_UINT8  PmStringForCompare[EPS_PM_MAXSIZE];    /* Retrieved PM data from printer       */
#endif    
    EPS_INT16 idx;

	EPS_LOG_FUNCIN;

/*** Validate input parameters                                                          */
    if(orgPmString == NULL) {
        EPS_DBGPRINT(("_SP_ChangeSpec_UpdatePMReply > EPS_ERR_SP_INVALID_POINTER\r\n"));
        EPS_RETURN( EPS_ERR_SP_INVALID_POINTER );
    }

    if(memcmp(orgPmString, PM_REPLY_HEADER, EPS_PM_HEADER_LEN) != 0) {
        EPS_DBGPRINT(("_SP_ChangeSpec_UpdatePMReply > EPS_ERR_SP_INVALID_HEADER\r\n"));
        EPS_RETURN( EPS_ERR_SP_INVALID_HEADER );
    }

	for(idx = EPS_PM_HEADER_LEN; idx <= (EPS_PM_MAXSIZE-EPS_PM_TERMINATOR_LEN); idx++) {
        if(orgPmString[idx]== 0x0D && orgPmString[idx+1] == 0x0A) {
            break;
        }
    }
    if(idx > (EPS_PM_MAXSIZE-EPS_PM_TERMINATOR_LEN)) {
        EPS_DBGPRINT(("_SP_ChangeSpec_UpdatePMReply > EPS_ERR_SP_INVALID_TERMINATOR\r\n"));
        EPS_RETURN( EPS_ERR_SP_INVALID_TERMINATOR );
    }

/*** Initialize Local Variables                                                         */
    memset(tempPmString, 0x00, EPS_PM_MAXSIZE);
#if _VALIDATE_SUPPORTED_MEDIA_DATA_
    memset(PmStringForCompare, 0x00, EPS_PM_MAXSIZE);
#endif

    /* Initialize pm data state */
    printer->pmData.version = 0;
	printer->pmData.length = 0;
	EPS_SAFE_RELEASE( printer->pmData.pmString )

/*** Correct PM REPLY following 7 steps                                                 */
/*** ---------------------------------------------------------------------------------- */
/*** STEP 1 : Replace Built-in resource. Because PM REPLY of the following printers     */
/***          is insufficient,                                                          */
/*** ---------------------------------------------------------------------------------- */
	if(        (strcmp(printer->modelName, "E-300" ) == 0) ){
		bufSize = _SP_LoadPMString(EPS_PMS_E300, orgPmString, bufSize);

	} else if( (strcmp(printer->modelName, "E-500" ) == 0) ||
		       (strcmp(printer->modelName, "E-700" ) == 0) ) {
        bufSize = _SP_LoadPMString(EPS_PMS_E500, orgPmString, bufSize);

	} else if( (strcmp(printer->modelName, "PictureMate PM 200" ) == 0) ||
		       (strcmp(printer->modelName, "PictureMate PM 210" ) == 0) ){
        bufSize = _SP_LoadPMString(EPS_PMS_PM200, orgPmString, bufSize);

	} else if( (strcmp(printer->modelName, "PictureMate PM 240" ) == 0) ||
		       (strcmp(printer->modelName, "PictureMate PM 250" ) == 0) ||
			   (strcmp(printer->modelName, "PictureMate PM 280" ) == 0) ){
        bufSize = _SP_LoadPMString(EPS_PMS_PM240, orgPmString, bufSize);
	}
	if(bufSize <= 0){
		return bufSize;
	}

	/* Use work pointers to call each filter functions */
    pBefore = orgPmString + EPS_PM_HEADER_LEN;             /* position of first tab 'S' */
    pAfter  = tempPmString;
	bufSize -= EPS_PM_HEADER_LEN;

/*** ---------------------------------------------------------------------------------- */
/*** STEP 2 : Remove <CR><LF> on the way                                                */
/*** ---------------------------------------------------------------------------------- */
    pSrc = pBefore;
    pDes = pAfter;

    DUMP_PMREPLY((pSrc, DUMP_HEX, "< ORIGINAL >"));

    _pmValidateRemoveDelimiter(pDes, pSrc, bufSize);

#if _VALIDATE_SUPPORTED_MEDIA_DATA_
	if(memcmp(pBefore, pAfter, bufSize - EPS_PM_HEADER_LEN) != 0){
		printf("!!!!!!!!! PM reply data modified on STEP 2. !!!!!!!!!\nRemove <CR><LF> on the way\n\n");
	}
#endif
    /* Update orgPmString                                                               */
	memcpy(pBefore, pAfter, bufSize);

    VERBOSE_DUMP_PMREPLY((pDes, DUMP_ASCII, "< STEP 1 PASSED >"));
	
/*** ---------------------------------------------------------------------------------- */
/*** STEP 3 : Copy only valid fields to reply buffer and remove unknown 'S' from reply  */
/*** ---------------------------------------------------------------------------------- */
    pSrc = pBefore;
    pDes = pAfter;

    DUMP_PMREPLY((pSrc, DUMP_HEX, "< ORIGINAL >"));

	if(_pmValidateRemoveUnknownSfield(pDes, pSrc) == 0) {
        EPS_DBGPRINT(("_SP_ChangeSpec_UpdatePMReply > EPS_ERR_SP_NO_VALID_FIELD\r\n"));
        EPS_RETURN( EPS_ERR_SP_NO_VALID_FIELD );
    }

#if _VALIDATE_SUPPORTED_MEDIA_DATA_
	if(memcmp(pBefore, pAfter, bufSize) != 0){
		printf("!!!!!!!!! PM reply data modified on STEP 3. !!!!!!!!!\n\n\n");
	    print_PMREPLY(pAfter, DUMP_HEX, "< Filterd >");
	}
#endif
    /* Update orgPmString                                                               */
	memcpy(pBefore, pAfter, bufSize);

    VERBOSE_DUMP_PMREPLY((pDes, DUMP_ASCII, "< STEP 1 PASSED >"));

/*** ---------------------------------------------------------------------------------- */
/*** STEP 4 : Correct unknown 'T' fields                                                */
/*** ---------------------------------------------------------------------------------- */
    pSrc = pBefore;
    pDes = pAfter;

    _pmCorrectUnknownTfield(pDes, pSrc);
#if _VALIDATE_SUPPORTED_MEDIA_DATA_
	if(memcmp(pBefore, pAfter, bufSize) != 0){
		printf("!!!!!!!!! PM reply data modified on STEP 4. !!!!!!!!!\n\n\n");
	    print_PMREPLY(pAfter, DUMP_HEX, "< Filterd >");
	}
#endif

    /* Update orgPmString                                                               */
	memcpy(pBefore, pAfter, bufSize);

	VERBOSE_DUMP_PMREPLY((pDes, DUMP_ASCII, "< STEP 2 PASSED >"));

/*** ---------------------------------------------------------------------------------- */
/*** STEP 5 : Merge duplicated fields                                                   */
/*** ---------------------------------------------------------------------------------- */
    pSrc = pBefore;
    pDes = pAfter;

    _pmCorrectDupulicatedFields(pDes, pSrc, &desSize);
#if _VALIDATE_SUPPORTED_MEDIA_DATA_
	if(memcmp(pBefore, pAfter, bufSize) != 0){
		printf("!!!!!!!!! PM reply data modified on STEP 5. !!!!!!!!!\n\n\n");
	    print_PMREPLY(pAfter, DUMP_HEX, "< Filterd >");
	}
#endif

    /* Update orgPmString                                                               */
	memcpy(pBefore, pAfter, bufSize);

	VERBOSE_DUMP_PMREPLY((pDes, DUMP_ASCII, "< STEP 3 PASSED >"));

    /* Now, Service Pack retains filtered data its original quality properties */
    /* within the inner buffer g_PMinfo.data */
    /* This data would be referenced whenever it is required to compare its originality */
    DUMP_PMREPLY((orgPmString, DUMP_ASCII, \
                  "< FILTERED (Retained within SP-same printer's caps) >"));


/*** ---------------------------------------------------------------------------------- */
/*** STEP 6 : Delete the paper type "CD/DVD label" from the pm string when "Stylus      */
/***          Photo R380" or "Stylus Photo RX580" is used.                              */
/*** ---------------------------------------------------------------------------------- */
	if ((strcmp(printer->modelName, "Stylus Photo R380" ) == 0) ||
        (strcmp(printer->modelName, "Stylus Photo RX580") == 0)    ) {

	    pSrc = pBefore;
        pDes = pAfter;
        paperSize = 0xFF;
        paperType = 0xFF;

        while (*pSrc == 'S') {
            paperSize = *(pSrc + 1);        /* Save the media size                      */

            *pDes++ = *pSrc++;            /* set 'S'                                  */
            *pDes++ = *pSrc++;            /* set the meida size                       */

            while (*pSrc == 'T') {
                paperType = *(pSrc + 1);    /* Save the media type                      */
                
                if ((paperSize == EPS_MSID_LETTER) && (paperType == EPS_MTID_CDDVD)) {
                    pSrc += 4;              /* Move to next 'T'                         */
                } else{
                    *pDes++ = *pSrc++;    /* set 'T'                                  */
                    *pDes++ = *pSrc++;    /* set the media type                       */
                    *pDes++ = *pSrc++;    /* set the printing mode info               */
                    *pDes++ = *pSrc++;    /* set '/'                                  */
                }
            }
            if (*pSrc == '/') {
                *pDes++ = *pSrc++;        /* set '/'                                  */
            }

            /* check for string termination                                                 */
            if ((*pSrc == 0xD) && (*(pSrc+1) == 0xA)) {
                *pDes++ = *pSrc++;
                *pDes++ = *pSrc++;
                break;
            }
        }

        /* Update orgPmString                                                               */
        memcpy(pBefore, pAfter, bufSize);
    }

/*** ---------------------------------------------------------------------------------- */
/*** STEP 7 : Adjust quality properties to the formal in order to return to the driver. */
/***          it dose not change the filtered data through previous steps retained      */
/***          within Service Pack. but just change the buffer asigned as parameter.     */
/***          (in this case orgPmString)                                                */
/***          after duplicating the filtered data to it.                                */
/*** ---------------------------------------------------------------------------------- */
    /* set filterd value "printer->pmData.pmString" */
	desSize += EPS_PM_HEADER_LEN;
	printer->pmData.pmString = (EPS_UINT8*)EPS_ALLOC(desSize);
    if( NULL == printer->pmData.pmString){
        EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
    }
    memcpy(printer->pmData.pmString, orgPmString, desSize);
	printer->pmData.length = desSize;
	printer->pmData.version = 1;

#if _VALIDATE_SUPPORTED_MEDIA_DATA_
	memcpy(PmStringForCompare, orgPmString, bufSize);
#endif

    /* If Draft Quality is only supported, add Normal Quality */
    _pmAdjustQuality(orgPmString);
#if _VALIDATE_SUPPORTED_MEDIA_DATA_
	if(memcmp(PmStringForCompare, orgPmString, bufSize - EPS_PM_HEADER_LEN) != 0){
		printf("!!!!!!!!! PM reply data modified on STEP 7. !!!!!!!!!\n\n\n");
	    print_PMREPLY(PmStringForCompare + EPS_PM_HEADER_LEN, DUMP_HEX, "< Origin >");
	    print_PMREPLY(orgPmString + EPS_PM_HEADER_LEN, DUMP_HEX, "< Filterd >");
	}
#endif
	
	DUMP_PMREPLY((orgPmString+EPS_PM_HEADER_LEN, DUMP_ASCII, \
                  "< FILTERED (Returned data to the driver-adjusted quality properties) >"));

/*** Return to caller                                                                   */
    EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epspmChangeSpec_DraftOnly()                                         */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printer      EPS_PRINTER_INN*    I/O: Pointer to a printer infomation                */
/* jobAtter     EPS_JOB_ATTRIB*     I: Data structure containing page attribut settings */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*                                                                                      */
/* Description:                                                                         */
/*      If the quality mode which is not supported by printer is assigned, replace it   */
/*      to printer's support mode.                                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE     epspmChangeSpec_DraftOnly (

		EPS_PRINTER_INN*    printer,
        EPS_JOB_ATTRIB*     jobAtter          /* Print Attributes for this Job         */

){
/*** Declare Variable Local to Routine                                                  */
    EPS_UINT8  mediaSizeID;
    EPS_UINT8  mediaTypeID;
    EPS_UINT8* pPMinfo;
    EPS_UINT8* pS_begin;
    EPS_UINT8* pS_end;
    EPS_UINT8* pTfield;
    EPS_UINT8  quality[3];      /* Q_DRAFT / Q_NORMAL / Q_HIGH                          */

	EPS_LOG_FUNCIN;

/*** Validate input parameters                                                          */
    if(printer->pmData.version <= 0) {
        EPS_DBGPRINT(("ChangeSpec_DraftOnly : PM info not initialized\r\n"));
        /* it is not able to hadle this situation so do nothing */
        EPS_RETURN( EPS_ERR_NONE ); 
    }

/*** Initialize Global/Local Variables                                                  */
    mediaSizeID = (EPS_UINT8)jobAtter->mediaSizeIdx;
    mediaTypeID = (EPS_UINT8)jobAtter->mediaTypeIdx;
    pPMinfo     = NULL;
    pS_begin    = NULL;
    pS_end      = NULL;
    pTfield     = NULL;
	memset(quality, 0, sizeof(quality));

/*** Change quality                                                                     */
    /* Refer the data retained within Service Pack. */
    /* "printer->pmData.pmString" has the header.   */
    pPMinfo = (EPS_UINT8*)(printer->pmData.pmString + EPS_PM_HEADER_LEN);

    if(1 == printer->pmData.version) {
		/* S field start postion */
		if(_pmFindSfield(mediaSizeID, pPMinfo, &pS_begin, &pS_end) < 0) {
			EPS_DBGPRINT(("ChangeSpec_DraftOnly : cannot find mediaSizeID(%d)\r\n", mediaSizeID));
			/* it is not able to hadle this situation so do nothing */
			EPS_RETURN( EPS_ERR_NONE ); 
		};

		VERBOSE_DUMP_PMREPLY((pS_begin, DUMP_S_TAG_ONLY, 
				"< ChangeSpec_DraftOnly : retained S field info >"));

		/* Fetch the T field */
		if((pTfield = _pmScanTfield(mediaTypeID, pS_begin)) == NULL) {
			EPS_DBGPRINT(("ChangeSpec_DraftOnly : cannot find mediaTypeID(%d)\r\n", mediaTypeID));
			/* it is not able to hadle this situation so do nothing */
			EPS_RETURN( EPS_ERR_NONE ); 
		}

		pTfield += 2;
	} else{
		/* Fetch the T field */
		if((pTfield = pm3FindTfield(printer->pmData.pmString, printer->pmData.length, mediaTypeID)) == NULL) {
			EPS_DBGPRINT(("ChangeSpec_DraftOnly : cannot find mediaTypeID(%d)\r\n", mediaTypeID));
			/* it is not able to hadle this situation so do nothing */
			EPS_RETURN( EPS_ERR_NONE ); 
		}
		pTfield += 2;
	}

    /* Quality should be assigned to the only supported mode */
    verbose_dbprint((" >> adjusted PrintQuality : %d -> ", jobAtter->printQuality));

	if(!((*pTfield & 0x07) &   /* Printer's support mode actually */
        (jobAtter->printQuality))) { /* Upper layer(driver) assigned mode */

        /* The quality mode which is not supported by printer is assigned */
        /* Replace it to printer's support mode */ 
        switch(*pTfield & 0x07) {
            case 0x01: /* 0 0 1 : Draft  only       */
                quality[Q_DRAFT]  = EPS_MQID_DRAFT;
                quality[Q_NORMAL] = EPS_MQID_DRAFT;
                quality[Q_HIGH]   = EPS_MQID_DRAFT;
                break;
            case 0x02: /* 0 1 0 : Normal only       */
                quality[Q_DRAFT]  = EPS_MQID_NORMAL;
                quality[Q_NORMAL] = EPS_MQID_NORMAL;
                quality[Q_HIGH]   = EPS_MQID_NORMAL;
                break;
            case 0x04: /* 1 0 0 : High   only       */
                quality[Q_DRAFT]  = EPS_MQID_HIGH;
                quality[Q_NORMAL] = EPS_MQID_HIGH;
                quality[Q_HIGH]   = EPS_MQID_HIGH;
                break;
            case 0x03: /* 0 1 1 : Normal and Draft  */
                quality[Q_DRAFT]  = EPS_MQID_DRAFT;
                quality[Q_NORMAL] = EPS_MQID_NORMAL;
                quality[Q_HIGH]   = EPS_MQID_NORMAL;
                break;
            case 0x05: /* 1 0 1 : High   and Draft  */
                quality[Q_DRAFT]  = EPS_MQID_DRAFT;
                quality[Q_NORMAL] = EPS_MQID_HIGH;
                quality[Q_HIGH]   = EPS_MQID_HIGH;
                break;
            case 0x06: /* 1 1 0 : High   and Normal */
                quality[Q_DRAFT]  = EPS_MQID_NORMAL;
                quality[Q_NORMAL] = EPS_MQID_NORMAL;
                quality[Q_HIGH]   = EPS_MQID_HIGH;
                break;
            case 0x07: /* 1 1 1 : Anything possible */
                break;
            default: 
                break;
        }

        /* Now, the value of quality array of index which is same as PrintQuality is valid */
		switch(jobAtter->printQuality) {
			case EPS_MQID_DRAFT:
				jobAtter->printQuality= quality[Q_DRAFT];
				break;
			case EPS_MQID_NORMAL:
				jobAtter->printQuality= quality[Q_NORMAL];
				break;
			case EPS_MQID_HIGH:
				jobAtter->printQuality= quality[Q_HIGH];
				break;
		}
    }

    verbose_dbprint(("%d\r\n", jobAtter->printQuality));

    EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   _SP_LoadPMString()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* resouceID    EPS_UINT32          I: PM resouce ID                                    */
/* pString      EPS_INT8*           O: PM string                                        */
/* bufSize      EPS_INT32           I: pString size                                     */
/*                                                                                      */
/* Return value:                                                                        */
/* EPS_INT32    buffer size or error                                                    */
/*                                                                                      */
/* Description:                                                                         */
/*      Load special PM string. Because a part of model is insufficient PM reply info.  */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32  _SP_LoadPMString (

        EPS_UINT32  resouceID,
        EPS_UINT8*  pString,
		EPS_UINT32  bufSize

){
    EPS_UINT32 i = 0;

	for (i = 0; i < EPS_SPM_STRINGS; i++){
		if (spPMStrTbl[i].id == resouceID){
			if(bufSize < spPMStrTbl[i].len){
				return EPS_ERR_OPR_FAIL;
			}
			memcpy(pString, spPMStrTbl[i].res, spPMStrTbl[i].len);
			return spPMStrTbl[i].len;
		}
	}

	return EPS_ERR_OPR_FAIL;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   _pmFindSfield()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* id           EPS_UINT8           I: Media Size ID                                    */
/* pSrc         EPS_UINT8*          I: pm String                                        */
/* pStart       EPS_UINT8**         O: Start Pointer of 'S' fields                      */
/* pEnd         EPS_UINT8**         O: End Pointer of 'S' fields                        */
/*                                                                                      */
/* Return value:                                                                        */
/*      Length of founded 'S' fields    - Success                                       */
/*      -1                              - There is NOT the Media Size ID in pm string   */
/*                                                                                      */
/* Description:                                                                         */
/*      Find a 'S' field that includes the <id> in <pSrc> and save its starting('S')    */
/*      and ending pointer('/') to <pStart> and <pEnd>.                                 */
/*      <pSrc> should be a complete PM REPLY format that start with 'S' and terminate   */
/*      at "0x0D 0x0A".                                                                 */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT32    _pmFindSfield (

        EPS_UINT8   id,
        EPS_UINT8*  pSrc,
        EPS_UINT8** pStart,
        EPS_UINT8** pEnd

){
	EPS_LOG_FUNCIN;

    while (*pSrc != 0xD || *(pSrc+1) != 0xA) {

        *pStart = NULL;
        *pEnd   = NULL;

        /* find 'S' */
        while(*pSrc == 'S') {
            if(id == *(pSrc+1)) {
                *pStart = pSrc;
            }

            pSrc += 2;

            while(*pSrc == 'T') {
                pSrc += 4;
            }

            /* Found id */
            if(*pStart != NULL) {
                *pEnd = pSrc;
                EPS_RETURN( (EPS_INT32)(*pEnd - *pStart)+1 );
            }

            /* next 'S' */
            pSrc++;
        }

		if(*pSrc == 'M' || *pSrc == 'R'){
			pSrc += 6;
		}
    }

    EPS_RETURN( (-1) );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   _pmScanTfield()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* id           EPS_UINT8           Media Type ID                                       */
/* pSfield      EPS_UINT8*          Pointer to 'S' field                                */
/*                                                                                      */
/* Return value:                                                                        */
/*      Pointer to 'T' on the pSfield   - Success                                       */
/*      NULL                            - There is NOT 'T' in the pSfield               */
/*                                                                                      */
/* Description:                                                                         */
/*      Find 'T' field that includs the <id>.                                           */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_UINT8*   _pmScanTfield (

        EPS_UINT8   id,
        EPS_UINT8*  pSfield

){
    EPS_UINT8* pScan = pSfield;
    EPS_UINT8* pT    = NULL;

	if(*pScan == 'S') {
        pScan += 2;

        while(*pScan == 'T') {
            if(id == *(pScan+1)) {
                pT = pScan;
                break;
            }

            pScan += 4;
        }
    }

    return pT;
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   _pmAppendTfield()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pT           EPS_UINT8*          I: Pointer to 'T' field                             */
/* pDes         EPS_UINT8*          O: Pointer to 'S' field                             */
/*                                                                                      */
/* Return value:                                                                        */
/*      4                               - Success (Return increased bytes)              */
/*      -1                              - Failure                                       */
/*                                                                                      */
/* Description:                                                                         */
/*      Append 'T' field to <pDes> if same field dose not exsit, but same one aleady    */
/*      exsits just combine mode properdy.                                              */
/*      <pDes> should have a complete 'S' field consist of 'S' and '/' and pT should    */
/*      have a 'T' field of 4 bytes starts with 'T'.                                    */
/*      This function returns the increased bytes so that caller change the last        */
/*      position or (-1) to indicate nothing changed.                                   */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    _pmAppendTfield (

        EPS_UINT8*  pT,
        EPS_UINT8*  pDes

){
    EPS_UINT8 t_id    = *(pT+1);
    EPS_INT16 t_exist = 0;

	if(*pDes == 'S') {

        pDes += 2; /* move to first 'T' */

        while(*pDes == 'T') {

            /* same id exist */
            if(t_id == *(pDes+1)) {
                /* Just combine mode property */
                *(pDes+2) |= *(pT+2);

                t_exist = 1;
                break;
            }

            /* next 'T' */
            pDes += 4;
        }

        /* samd id field dose not exist */
        /* Append new 'T' fields */
        if(t_exist == 0) {
            memcpy(pDes, pT, 4);
            pDes += 4;
            *pDes = '/';

            return 4; /* size of 'T' field */
        }

        /* type id aleady exist then do not anything */
    }

    return (-1);
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   _pmValidateRemoveDelimiter()                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pDes         EPS_UINT8*          O: Pointer to validated pm string                   */
/* pSrc         EPS_UINT8*          I: Pointer to original pm string                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      The number of valid fields                                                      */
/*                                                                                      */
/* Description:                                                                         */
/*      Copy valid fields to reply buffer only.                                         */
/*      Remove <CR><LF> on the way.                                                     */
/*                                                                                      */
/*******************************************|********************************************/
static void    _pmValidateRemoveDelimiter (

        EPS_UINT8*  pDes,
        EPS_UINT8*  pSrc, 
		EPS_INT32   bufSize

){
    EPS_UINT8* pEOF  = pSrc + bufSize;

	EPS_LOG_FUNCIN;

    while (pSrc < pEOF) {

        if(*pSrc == 'S') {
			memcpy(pDes, pSrc, 2);
            pSrc += 2;
			pDes += 2;

            while(*pSrc == 'T') {
				memcpy(pDes, pSrc, 3);
                pSrc += 3;
				pDes += 3;

                if(*pSrc == '/') {
                    *pDes++ = *pSrc++;
                }
            }

            if(*pSrc == '/') {
                *pDes++ = *pSrc++;
            }

		} else if(*pSrc == 'M' || *pSrc == 'R'){
			/* Jpeg size limit */ 
			if( *(pSrc + 5) == '/' ){
				memcpy(pDes, pSrc, 6 );
				pDes += 6;
			}
			pSrc += (pEOF-pSrc > 6)?6:(pEOF-pSrc);

		} else if(*pSrc == 0xD || *(pSrc+1) == 0xA){
			/* Terminater skip */
			pSrc += 2;
		} else{
			/* unknown field */
	        pSrc++;
		}
    }

	/* set truth terminater */
    *pDes++ = 0x0d;   /* 0xD */
    *pDes   = 0x0a;   /* 0xA */

    EPS_RETURN_VOID;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   _pmValidateRemoveUnknownSfield()                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pDes         EPS_UINT8*          O: Pointer to validated pm string                   */
/* pSrc         EPS_UINT8*          I: Pointer to original pm string                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      The number of valid fields                                                      */
/*                                                                                      */
/* Description:                                                                         */
/*      Copy valid fields to reply buffer only.                                         */
/*      Remove unknown 'S' field.                                                       */
/*      Minimum conditons for valid PM REPLY are                                        */
/*       - it must have a complete 'S' field more than one ( 'S' ~ '/').                */
/*       - it must end with 0xD and 0xA.                                                */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    _pmValidateRemoveUnknownSfield (

        EPS_UINT8*  pDes,
        EPS_UINT8*  pSrc

){
    EPS_UINT8* pPrev = NULL;    /* save previous pointer                                */
    EPS_UINT8* pS    = NULL;    /* valid field's starting position                      */
    EPS_UINT8* pE    = NULL;    /* valid field's ending postion                         */

    EPS_INT16  valid = 0;       /* flag for indicating 'S' field's validation           */
    EPS_INT16  t_cnt = 0;       /* count valid 'T' fields                               */
    EPS_UINT16 s_idx = 0;       /* index of epsMediaSize                                */

    EPS_INT16  num_valid_fields = 0;    /* value for returning                          */

	EPS_LOG_FUNCIN;

#ifdef _TEST_PM_STEP_1 /* Change first 'S' field's id to unknown id such as 0xFF */
    *(pSrc+1) = 0xFF;
#endif

    while (*pSrc != 0xD || *(pSrc+1) != 0xA) {
        pPrev = pSrc;

        pS    = NULL;
        pE    = NULL;
        valid = 0;
        t_cnt = 0;
        s_idx = 0;

        if(*pSrc == 'S') {
            pS    = pSrc;
            pSrc += 2;

            while(*pSrc == 'T') {
                pSrc += 3;

                if(*pSrc == '/') {
                    pSrc++;
                    t_cnt++;
                }
            }

            if(t_cnt && *pSrc == '/') {
                pE = pSrc;
            }

		} else if(*pSrc == 'M' || *pSrc == 'R'){
			/* Jpeg size limit */ 
			if( *(pSrc + 5) == '/' ){
				memcpy(pDes, pSrc, 6 );
				pDes += 6;
				pSrc += 6;
				continue;
			}
		}

        /* Copy valid and support 'S' fields only */
        /* Valid means size id should be find in its table */
        /* and 'T' field exist at least more than one */
        /* Unknown 'S' field should be removed */
        if(pS && pE) {
            for(s_idx = 0; s_idx < EPS_NUM_MEDIA_SIZES; s_idx++) {
                if(epsMediaSize[s_idx].id == *(pS+1)) {
                    memcpy(pDes, pS, (EPS_UINT32)((pE-pS)+1) );
                    pDes += (pE-pS)+1;
                    valid = 1;

                    /* now increase num of valid fields */
                    num_valid_fields++;

                    break;
                }
            }
        }

        /* Restore work buffer pos to the previous */
        /* cause fail to get a valid fields */
        if(valid == 0) {
            pSrc = pPrev;
        }

        pSrc++;
    }

    *pDes++ = *pSrc++;   /* 0xD */
    *pDes++ = *pSrc;     /* 0xA */

    EPS_RETURN( num_valid_fields );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   _pmCorrectUnknownTfield()                                           */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pDes         EPS_UINT8*          O: Pointer to validated pm string                   */
/* pSrc         EPS_UINT8*          I: Pointer to original pm string                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Change an unknown 'T' field to PGPP's in case that PGPP dose not exist in       */
/*      'S' field. If aleady PGPP exist delete it.                                      */
/*                                                                                      */
/*******************************************|********************************************/
static void     _pmCorrectUnknownTfield (

        EPS_UINT8*  pDes,
        EPS_UINT8*  pSrc

){
    static const EPS_UINT8 PGPP_FIELD [ ] = { 0x54, 0x0B, 0x87, 0x2F };

    EPS_INT16 PGPP    = 0;      /* Premium Glossy Photo Paper (type id : 0x0b)          */
    EPS_UINT16 t_idx  = 0;      /* Index of table defined Support 'T' id table          */
    EPS_UINT8 * pScan = NULL;   /* word pointer for scanning id                         */

	EPS_LOG_FUNCIN;

#ifdef _TEST_PM_STEP_2 /* Change 'T' field's id to unknown id such as 0xFF */
    *(pSrc+3) = 0xFF;
#endif

    while (*pSrc != 0xD || *(pSrc+1) != 0xA) {
        /* reset PGPP flag each new 'S' field */
        PGPP = 0;

        if(*pSrc == 'S') {
            /* Scan PGPP in current 'S' field */
            pScan = pSrc;

            if(_pmScanTfield(EPS_MTID_PGPHOTO, pScan) != NULL) {
                PGPP = 1;
            }

            *pDes++ = *pSrc++;
            *pDes++ = *pSrc++;

            while(*pSrc == 'T') {
                /* Copy support 'T' field */
                for(t_idx = 0; t_idx < EPS_NUM_MEDIA_TYPES; t_idx++) {
                    if(epsMediaTypeIndex[t_idx] == *(pSrc+1)) {
                        memcpy(pDes, pSrc, 4);
                        pDes += 4;
                        break;
                    }
                }

                /* Unknown type id encountered */
                /* if PGPP did not exist in 'S' field */
                /* then append PGPP fields to pDes */
                if(t_idx == EPS_NUM_MEDIA_TYPES && PGPP == 0) {
                    memcpy(pDes, PGPP_FIELD, 4);
                    pDes += 4;
                    PGPP  = 1;
                }

                /* move to next 'T' */
                pSrc += 4;
            }

            /* copy '/' and move next 'S' */
            *pDes++ = *pSrc++;

		}else if(*pSrc == 'M' || *pSrc == 'R') {
			memcpy(pDes, pSrc, 6);
			pDes += 6;
			pSrc += 6;
		}
    }

    *pDes++ = *pSrc++;   /* 0xD */
    *pDes   = *pSrc;     /* 0xA */

    EPS_RETURN_VOID;
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   _pmCorrectDupulicatedFields()                                       */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pDes         EPS_UINT8*          O: Pointer to validated pm string                   */
/* pSrc         EPS_UINT8*          I: Pointer to original pm string                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Merge duplicated fields.                                                        */
/*                                                                                      */
/*******************************************|********************************************/
static void     _pmCorrectDupulicatedFields (

        EPS_UINT8*  pDes,
        EPS_UINT8*  pSrc,
		EPS_INT32*  desSize

){
    EPS_UINT8 merged_buf[EPS_PM_MAXSIZE];

    EPS_UINT8* pDesTop = pDes;
    EPS_UINT8* pFieldS = NULL;      /* current 'S' in merged buffer                     */
    EPS_UINT8* pFieldT = NULL;      /* work pontter to merge a 'T'                      */
    EPS_UINT8* pS      = NULL;      /* duplicated field's starting position             */
    EPS_UINT8* pE      = NULL;      /* duplicated field's ending postion                */
    EPS_UINT8* pM      = NULL;      /* pos of merged buffer                             */
    EPS_UINT8* pScan   = NULL;      /* work pointer to find a field                     */
    EPS_UINT8  s_id    = 0xFF;      /* current 'S' id                                   */
    EPS_INT16  bytes;

	EPS_LOG_FUNCIN;

#ifdef _TEST_PM_STEP_3
    *(pSrc+8) = 0x0F; /* make duplicate 'S' */
#endif
	memset(merged_buf, 0, EPS_PM_MAXSIZE);
    pM = &merged_buf[0];

    /* Aleady merged fields no need to copy again */
    while (*pSrc != 0xD || *(pSrc+1) != 0xA) {
        pFieldS = NULL;

        if(*pSrc == 'S') {
            VERBOSE_DUMP_PMREPLY((pSrc, DUMP_S_TAG_ONLY, "< STEP 3 : SOURCE 'S' ... >"));

            /* save current 'S' id */
            s_id = *(pSrc+1);

            if(s_id != MERGED_FIELD) {
                /* Current 'S' field's starting pos */
                /* it is used to merge fields later */
                pFieldS = pM;

                COPY_BYTES(pM, pSrc, 2);
            }

            pSrc += 2; /* move to first 'T' */

            /* Merge 'T' fields */
            while(*pSrc == 'T') {

                if(pFieldS && s_id != MERGED_FIELD) {
                    /* if 'T' aleady exist just combine its property by BIT OR operation */
                    if((pFieldT = _pmScanTfield(*(pSrc+1), pFieldS)) != NULL) {
                        *(pFieldT+2) |= *(pSrc+2);
                    }

                    /* Copy only new 'T' field */
                    if(pFieldT == NULL) {
                        COPY_BYTES(pM, pSrc, 4);
                    }
                }

                pSrc += 4; /* next 'T' */
            }
 		}else if(*pSrc == 'M' || *pSrc == 'R') {
			memcpy(pM, pSrc, 6);
			pM += 6;
			pSrc += 6;
			continue;
		}

        if(s_id != MERGED_FIELD) {
            COPY_BYTES(pM, pSrc, 1);
        }
        pSrc++;

        /* aleady merged field just go on next */
        if(s_id == MERGED_FIELD)  {
            continue;
        }

        /*----------------------------------------------------*/
        /* Find dupulicated 'S' being followed and merge them */

        pScan = pSrc; /* do not change pSrc in following loop */

        while(_pmFindSfield(s_id, pScan, &pS, &pE) > 0) {

            /* Change source's 'S' id to MERGED_FIELD */
            *(pS+1) = MERGED_FIELD;
            pS     += 2;

            /* merge dupulicated 'T' */
            while(*pS == 'T') {

                /* Append NEW 'T' field to the current 'S' field */
                /* aleady same 'T' exist only its mode property will be combined */
                /* after called function */
                if(pFieldS) {
                    if((bytes = _pmAppendTfield(pS, pFieldS)) > 0) {

                    /* update merged_buf's the last pos that pM point it */
                    pM += bytes; /* MUST 4 BYTES(size of 'T' field) ! */
                    }
                }

                pS += 4; /* next 'T' */
            }

            /* find next 'S' */
            pScan = (pE+1);

            VERBOSE_DUMP_PMREPLY((pFieldS, DUMP_S_TAG_ONLY, "< STEP 3 : MERGE PROCESSING ... >"));
        }
    }

    /* 0x0D & 0x0A */
    COPY_BYTES(pM, pSrc, 2);

    /*----------------------------------*/
    /* Copy the merged PM REPLY to pDes */

    pM = &merged_buf[0];

    while (*pM != 0xD || *(pM+1) != 0xA) {
        *pDes++ = *pM++;
    }

    *pDes++ = *pM++; /* 0xD */
    *pDes   = *pM;   /* 0xA */

	*desSize = (EPS_INT32)(pDes - pDesTop) + 1;

    EPS_RETURN_VOID;
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   _pmAdjustQuality()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pData        EPS_UINT8*          I/O: Pointer to pm string                           */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Adjust quality properties to the formal.                                        */
/*      example : quality has only draft mode -> turn on normal mode.                   */
/*                                                                                      */
/*******************************************|********************************************/
static void     _pmAdjustQuality (

        EPS_UINT8*  pData

){
    EPS_UINT8* p = pData;

	EPS_LOG_FUNCIN;

	/* skip pm heder */
    p += EPS_PM_HEADER_LEN;

    verbose_dbprint(("< STEP 4 :  Adjust quality >\r\n"));

    /* adjuct each quality properties */
    while(!(*p == 0x0D && *(p+1) == 0x0A)) {

        while(*p == 'S') {

            verbose_dbprint(("%c %02d\r\n", *p, *(p+1)));

            p += 2; /* move to the first T field */

            while(*p == 'T') {

                verbose_dbprint(("\t%c %02d 0x%02x %c -> ", *p, *(p+1), *(p+2), *(p+3)));

                p += 2; /* move to quality pos */

                /* Quality property */
                switch(*p & 0x07) {
                    /* Should be handled following case 1 bit of Draft turned on only */
                    case 0x01: /* 0 0 1 -> 0 1 1 */
                        *p |= (1<<1); /* turn normal on */
                        break;
                    default:
                        break;
                }

                verbose_dbprint(("%c %02d 0x%02x %c\r\n", *(p-2), *(p-1), *(p), *(p+1)));

                p += 2; /* move to the next T field */
            }

            p += 1; /* move to the next S field */
        }

		if(*p == 'M' || *p == 'R') {
			p += 6;
		}
    }

    EPS_RETURN_VOID;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   AppendMedia()                                                       */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:                Description:                                       */
/* pMedia       EPS_SUPPORTED_MEDIA* I/O: supported media structuer                     */
/*                                                                                      */
/* Return value:                                                                        */
/*      none                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Append Media attribute from PM reply info.                                      */
/*                                                                                      */
/*******************************************|********************************************/
void     AppendMedia (

        EPS_SUPPORTED_MEDIA*  pMedia

){
    EPS_INT32       idxSize, idxType;
    EPS_BOOL        foundCD = FALSE;

	EPS_LOG_FUNCIN;

	for(idxSize = 0; idxSize < pMedia->numSizes; idxSize++){
		/*** Append "CD/DVD Label" rayout to plain, CDDVD ***/
		for(idxType = 0; idxType < pMedia->sizeList[idxSize].numTypes; idxType++){

			if( EPS_IS_CDDVD( pMedia->sizeList[idxSize].typeList[idxType].mediaTypeID ) ){
				/* Set "CD/DVD Label" rayout to CDDVD */
				pMedia->sizeList[idxSize].typeList[idxType].layout = EPS_MLID_CDLABEL;

				/* paperSource is only CD tray */
				pMedia->sizeList[idxSize].typeList[idxType].paperSource = EPS_MPID_CDTRAY;
				
				foundCD = TRUE;
			} else if(EPS_MTID_MINIPHOTO == pMedia->sizeList[idxSize].typeList[idxType].mediaTypeID){
				/* Append "16 Division" rayout to Photo Stickers */
				pMedia->sizeList[idxSize].typeList[idxType].layout |= EPS_MLID_DIVIDE16;
			}
		}

		if(foundCD){
			for(idxType = 0; idxType < pMedia->sizeList[idxSize].numTypes; idxType++){
				if(EPS_MTID_PLAIN == pMedia->sizeList[idxSize].typeList[idxType].mediaTypeID ){
					/* Append "CD/DVD Label" rayout to plain */
					pMedia->sizeList[idxSize].typeList[idxType].layout |= EPS_MLID_CDLABEL;
					break;
				}
			}
			foundCD = FALSE;
		}
	}

	EPS_RETURN_VOID
}


EPS_UINT32  ComplementPaperSource (

        EPS_INT32  mediaSizeID

){
	switch(mediaSizeID){
	case EPS_MSID_A4:
	case EPS_MSID_LETTER:
	case EPS_MSID_LEGAL:
		return EPS_MPID_FRONT1;
		break;
	default:
		if(EPS_MSID_KAKU_2 <= mediaSizeID){
			return EPS_MPID_FRONT1;
		}
	}

	return EPS_MPID_FRONT2;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   GetPrintAreaSource()                                                */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* resolution   EPS_UINT32          I : input resolusion                                */
/* src          EPS_PRINTAREA_SOURCE* O: Former information  @                         */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                            - Success                               */
/*      EPS_ERR_INV_INPUT_RESOLUTION            - Invalid Input Resolution              */
/*                                                                                      */
/* Description:                                                                         */
/*      setup former information for margin calculation.                                */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    GetPrintAreaSource(
	
		EPS_UINT32				resolution,
		EPS_PRINTAREA_SOURCE*	source

){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;

	EPS_LOG_FUNCIN;

	source->resolution = resolution;
	switch(resolution){
	case EPS_IR_360X360:
		source->pMI = epsMediaSize;
		source->factor = 1;
		source->defaultBorder = EPS_BORDERS_MARGIN_360;
		source->borderlessBottom = EPS_BOTTOM_MARGIN_BORDERLESS_360;
		break;
	case EPS_IR_720X720:
		source->pMI = epsMediaSize;
		source->factor = 2;
		source->defaultBorder = EPS_BORDERS_MARGIN_360;
		source->borderlessBottom = EPS_BOTTOM_MARGIN_BORDERLESS_360;
		break;
	case EPS_IR_300X300:
		source->pMI = epsMediaSize300;
		source->factor = 1;
		source->defaultBorder = EPS_BORDERS_MARGIN_300;
		source->borderlessBottom = EPS_BOTTOM_MARGIN_BORDERLESS_300;
		break;
	case EPS_IR_600X600:
		source->pMI = epsMediaSize300;
	    source->factor = 2;
		source->defaultBorder = EPS_BORDERS_MARGIN_300;
		source->borderlessBottom = EPS_BOTTOM_MARGIN_BORDERLESS_300;
		break;
	default:
        ret = EPS_ERR_INV_INPUT_RESOLUTION;
	}
    EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CalcPrintMargin()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* preview      EPS_BOOL            I : values for preview or print command             */
/* src          EPS_PRINTAREA_SOURCE* I: Former information  @                         */
/* cdDimOut     EPS_BOOL            I : CD Outer diameter   @                          */
/* lm           EPS_LAYOUT_INFO*    O : margin information                              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_INV_BORDER_MODE         - Invalid Border Mode                           */
/*      EPS_ERR_INV_CD_OUTDIM           - Invalid CD Outside Diameter                   */
/*      EPS_ERR_INV_TOP_MARGIN          - Invalid Top Magirn                            */
/*      EPS_ERR_INV_LEFT_MARGIN         - Invalid Left Margin                           */
/*      EPS_ERR_INV_BOTTOM_MARGIN       - Invalid Bottom Margin                         */
/*      EPS_ERR_INV_RIGHT_MARGIN        - Invalid Right Margin                          */
/*                                                                                      */
/* Description:                                                                         */
/*      create margin informations.                                                     */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    CalcPrintMargin(
	
		EPS_BOOL                preview, 
		EPS_PRINTAREA_SOURCE*   src,
		const EPS_JOB_ATTRIB*   pAttr,
		EPS_LAYOUT_INFO*        lm,
		EPS_RECT*               baseBorder

){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;

	EPS_LOG_FUNCIN;

	switch(lm->layout){
	case EPS_MLID_BORDERLESS:
		lm->margin.top    = src->pMI->top_margin_borderless * src->factor;
		lm->margin.bottom = -(src->pMI->print_area_y_borderless - src->pMI->paper_y + src->pMI->top_margin_borderless) * src->factor;
		lm->margin.left   = src->pMI->left_margin_borderless * src->factor;
		lm->margin.right  = -(src->pMI->print_area_x_borderless - src->pMI->paper_x + src->pMI->left_margin_borderless) * src->factor;
		break;

	case EPS_MLID_BORDERS:
	case EPS_MLID_DIVIDE16:
		lm->margin.top    = TopBorderPixel(src);
		lm->margin.bottom = BottomBorderPixel(src);
		lm->margin.left   = LeftBorderPixel(src);
		lm->margin.right  = RightBorderPixel(src);
		break;

	case EPS_MLID_CDLABEL:
		if(NULL != pAttr){
			if( (pAttr->cdDimOut >= EPS_CDDIM_OUT_MIN) && (pAttr->cdDimOut <= EPS_CDDIM_OUT_MAX) ){
				if(preview){
					lm->margin.top    =
					lm->margin.bottom =
					lm->margin.left   =
					lm->margin.right  = ( elGetDots(src->resolution, EPS_CDDIM_OUT_MAX*10)
										- elGetDots(src->resolution, pAttr->cdDimOut*10) ) / 2;
				} else{
					lm->margin.top    = CDDVD_OFFSET_Y(src->resolution, pAttr->cdDimOut);
					lm->margin.bottom = src->pMI->paper_y * src->factor - lm->margin.top 
										- elGetDots(src->resolution, pAttr->cdDimOut*10);
					lm->margin.left   = CDDVD_OFFSET_X(src->resolution, pAttr->cdDimOut);
					lm->margin.right  = src->pMI->paper_x * src->factor - lm->margin.left
										- elGetDots(src->resolution, pAttr->cdDimOut*10);
				}
			} else{
				ret = EPS_ERR_INV_CD_OUTDIM;
			}
		}
		break;

	case EPS_MLID_CUSTOM:
		if(NULL != pAttr){
			if (pAttr->topMargin    < TopBorderPixel(src)   ) EPS_RETURN( EPS_ERR_INV_TOP_MARGIN );
			if (pAttr->leftMargin   < LeftBorderPixel(src)  ) EPS_RETURN( EPS_ERR_INV_LEFT_MARGIN );
			if (pAttr->bottomMargin < BottomBorderPixel(src)) EPS_RETURN( EPS_ERR_INV_BOTTOM_MARGIN );
			if (pAttr->rightMargin  < RightBorderPixel(src) ) EPS_RETURN( EPS_ERR_INV_RIGHT_MARGIN );
		
			lm->margin.top    = pAttr->topMargin;
			lm->margin.left   = pAttr->leftMargin;
			lm->margin.bottom = pAttr->bottomMargin;
			lm->margin.right  = pAttr->rightMargin;
		}
		if (NULL != baseBorder) {
			baseBorder->top        = TopBorderPixel(src);
			baseBorder->bottom     = BottomBorderPixel(src);
			baseBorder->left       = LeftBorderPixel(src);
			baseBorder->right      = RightBorderPixel(src);
		}
		break;

	default:
	    ret = EPS_ERR_INV_BORDER_MODE;
	}

	EPS_RETURN( ret );
}

EPS_INT32 LeftBorderPixel(EPS_PRINTAREA_SOURCE* src)
{
	EPS_INT32 sizeID = src->pMI->id;
	if( EPS_MSID_ENV_C4_P == sizeID || EPS_MSID_KAKU_2 == sizeID || EPS_MSID_KAKU_20 == sizeID || 
		EPS_MSID_A5_24HOLE == sizeID ||
		(EPS_MSID_ENV_10_L <= sizeID && EPS_MSID_YOKEI_4 >= sizeID) ||
	    (EPS_MSID_ENV_10_P <= sizeID && EPS_MSID_NEWENV_P >= sizeID) )
	{
		return (src->pMI->paper_x - src->pMI->print_area_x_border) * src->factor / 2;
	} else{
		return src->defaultBorder * src->factor;
	}
}
EPS_INT32 RightBorderPixel(EPS_PRINTAREA_SOURCE* src)
{
	return (src->pMI->paper_x - src->pMI->print_area_x_border - LeftBorderPixel(src)) * src->factor;
}
EPS_INT32 TopBorderPixel(EPS_PRINTAREA_SOURCE* src)
{
	EPS_INT32 sizeID = src->pMI->id;
	if(EPS_MSID_A5_24HOLE == sizeID )
	{
		return (src->pMI->paper_y - src->pMI->print_area_y_border) * src->factor / 2;
	} else{
		return src->defaultBorder * src->factor;
	}
}
EPS_INT32 BottomBorderPixel(EPS_PRINTAREA_SOURCE* src)
{
	return (src->pMI->paper_y - src->pMI->print_area_y_border - TopBorderPixel(src)) * src->factor;
}



/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------                PM3 Functions                ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epspmCreateMediaInfo3()                                             */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printer      EPS_PRINTER_INN*    I:Pointer to printer data                           */
/* pmString     EPS_PM_DATA*        I:Pointer to pm data                                */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_INV_MEDIA_SIZE          - Invalid Media Size                            */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*                                                                                      */
/* Description:                                                                         */
/*      create support media information from pm3.                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epspmCreateMediaInfo3(
	
		EPS_PRINTER_INN*	printer,
		EPS_UINT8*			pmString,
		EPS_INT32			pmSize
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
    EPS_INT32       pmIdx;                      /* pm string index                      */
    EPS_INT32       totalTypeNum = 0;           /* Total Media type number              */
	EPS_UINT8*		pTVals = NULL;              /* Type&Quality pair of T field         */
	EPS_BOOL		complete = FALSE;

	EPS_SUPPORTED_MEDIA *pMedia = &printer->supportedMedia;

	EPS_LOG_FUNCIN;

	/*EPS_DUMP(pmString, pmSize);*/

/*** "Filter" Raw "PM" data (Remake the correct pm stirng)                              */
	ret = pm3ChangeSpec_UpdatePMReply(printer, pmString, &pmSize);
	if(EPS_ERR_NONE != ret){
		EPS_RETURN( ret );
	}
	
	pmIdx = 0;				/* skip the command header of pm string     */

/*** Create the structure of the support media                                          */
	pMedia->resolution = EPS_IR_360X360;	/* default support */

    while( EPS_ERR_NONE == ret && pmIdx < pmSize-3 && FALSE == complete) {
        switch(pmString[pmIdx]) {
		case 'S':
			ret = pm3ScanSfield(pmString, pmSize, &pmIdx, NULL, 0, 0, NULL);	/* skip S field */
			break;

		case 'T':
			/* make type & quality pair */
			ret = pm3GetTfieldValue(pmString, pmSize, &pmIdx, &totalTypeNum, &pTVals, &pMedia->resolution);
			break;

		case 'L':
			if(NULL == pTVals){
				ret = pm3GetTfieldValue(pmString, pmSize, &pmIdx, &totalTypeNum, &pTVals, &pMedia->resolution);
			}
			ret = pm3ScanLfield(pmString, pmSize, &pmIdx, pTVals, totalTypeNum, pMedia);
			break;

		case 'P':
			ret = pm3ScanPfield(pmString, pmSize, &pmIdx, &pMedia->JpegSizeLimit);
			if(EPS_ERR_NONE == ret){
				printer->JpgMax = pMedia->JpegSizeLimit;
			}
			break;

		case ';':
		default:	/* unknown category */
			complete = TRUE;
			break;
		}

		/* If we run into an occurrence of carriage return followed by line feed,
		 * we have found the terminating characters of the string. */
		if(pmString[pmIdx] == ';' && pmString[pmIdx+1] == 0xFF) {
			break;
		}
	}
	if( NULL != pTVals){
		EPS_FREE(pTVals);
	}

	if(EPS_ERR_NONE == ret){
/*** Add extend infomation                                                              */
		AppendMedia(&printer->supportedMedia);
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pm3GetPrintMargin()                                                 */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pmString     EPS_PM_DATA*        I:Pointer to pm data                                */
/* pAttr        EPS_JOB_ATTRIB*     I:Pointer to print settings                         */
/* lm           EPS_LAYOUT_INFO*    I/O:Pointer to margin information                   */
/* baseBorder   EPS_RECT*           I:basic border size                                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_INV_MEDIA_SIZE          - Invalid Media Size                            */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*                                                                                      */
/* Description:                                                                         */
/*      create print margin information from pm3.                                       */
/*                                                                                      */
/*******************************************|********************************************/
#define EPS_PM3_RESCOMV(v) (EPS_INT32)((EPS_INT32)(v) * factor * resVal / baseResVal)

EPS_ERR_CODE    pm3GetPrintMargin(
	
		EPS_PM_DATA*		pmData,
		const EPS_JOB_ATTRIB* pAttr,
		EPS_LAYOUT_INFO*	lm,
		EPS_RECT*			baseBorder

){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
	EPS_UINT8*		pmString = pmData->pmString;
	EPS_INT32		pmSize = pmData->length;
    EPS_INT32       pmIdx = 0;                  /* pm string index                      */
    EPS_INT32       num_mSize = 0;              /* Media size number                    */
    EPS_INT32       idx;                        /* Index                                */
    EPS_INT32       blkSize = 0;                /* data block size                      */
	EPS_UINT32		baseResolution = 0;
	EPS_INT32		resVal, baseResVal, factor;
	EPS_MARGIN		fixedMargin;
	EPS_BOOL		complete = FALSE;

	EPS_LOG_FUNCIN;

	pmIdx = 0;				/* skip the command header of pm string     */
	while( EPS_ERR_NONE == ret && pmIdx < pmSize-3 && FALSE == complete) {
        switch(pmString[pmIdx]) {
		case 'S':
			if(0 == baseResolution){
				ret = pm3GetTfieldValue(pmString, pmSize, &pmIdx, NULL, NULL, &baseResolution);
				if( EPS_ERR_NONE != ret ){
					break;
				}
			}
			resVal = ((pAttr->inputResolution == EPS_IR_360X360) || (pAttr->inputResolution == EPS_IR_720X720))?
					360:300;
			baseResVal = ((baseResolution == EPS_IR_360X360) || (baseResolution == EPS_IR_720X720))?
					360:300;
			factor = ((pAttr->inputResolution == EPS_IR_360X360) || (pAttr->inputResolution == EPS_IR_300X300))?1:2;
			
			pmIdx++;
			num_mSize = pmString[pmIdx];
			pmIdx++;
			for(idx = 0; idx < num_mSize && FALSE == complete; idx++){
				blkSize = pmString[pmIdx] + 1;
				if(blkSize < 14){
					ret = EPS_ERR_OPR_FAIL;  /* Invalid format */	
					break;
				}

				if(pmString[pmIdx+1] == pAttr->mediaSizeIdx){
					switch(lm->layout){
					case EPS_MLID_BORDERLESS: 
						/* not reached
						lm->margin.top    = -EPS_PM3_RESCOMV( pmString[pmIdx+2] << 8 | pmString[pmIdx+3] );
						lm->margin.left   = 
						lm->margin.right  = -EPS_PM3_RESCOMV( pmString[pmIdx+4] << 8 | pmString[pmIdx+5] );
						lm->margin.bottom = -EPS_PM3_RESCOMV( pmString[pmIdx+6] << 8 | pmString[pmIdx+7] );
						*/
						ret = EPS_ERR_OPR_FAIL;
						break;
					case EPS_MLID_BORDERS:
					case EPS_MLID_DIVIDE16:
						lm->layout = EPS_MLID_BORDERS;
						lm->margin.top    = EPS_PM3_RESCOMV( pmString[pmIdx+8] << 8 | pmString[pmIdx+9] );
						lm->margin.left   = 
						lm->margin.right  = EPS_PM3_RESCOMV( pmString[pmIdx+10] << 8 | pmString[pmIdx+11] );
						lm->margin.bottom = EPS_PM3_RESCOMV( pmString[pmIdx+12] << 8 | pmString[pmIdx+13] );
						break;
					case EPS_MLID_CUSTOM:
						if(NULL != pAttr){
							fixedMargin.top   = EPS_PM3_RESCOMV( pmString[pmIdx+8] << 8 | pmString[pmIdx+9] );
							fixedMargin.left  =
							fixedMargin.right = EPS_PM3_RESCOMV( pmString[pmIdx+10] << 8 | pmString[pmIdx+11] );
							fixedMargin.bottom= EPS_PM3_RESCOMV( pmString[pmIdx+12] << 8 | pmString[pmIdx+13] );

							if (pAttr->topMargin    < fixedMargin.top)	 EPS_RETURN( EPS_ERR_INV_TOP_MARGIN );
							if (pAttr->leftMargin   < fixedMargin.left)	 EPS_RETURN( EPS_ERR_INV_LEFT_MARGIN );
							if (pAttr->bottomMargin < fixedMargin.bottom)EPS_RETURN( EPS_ERR_INV_BOTTOM_MARGIN );
							if (pAttr->rightMargin  < fixedMargin.right) EPS_RETURN( EPS_ERR_INV_RIGHT_MARGIN );
		
							lm->margin.top    = pAttr->topMargin;
							lm->margin.left   = pAttr->leftMargin;
							lm->margin.bottom = pAttr->bottomMargin;
							lm->margin.right  = pAttr->rightMargin;
							if(NULL != baseBorder){
								baseBorder->top   = fixedMargin.top;
								baseBorder->left  = fixedMargin.left;
								baseBorder->right = fixedMargin.right;
								baseBorder->bottom= fixedMargin.bottom;
							}
						}
						break;

					case EPS_MLID_CDLABEL:
					default:
						ret = EPS_ERR_INV_BORDER_MODE;
					}
					complete = TRUE;

				}
				pmIdx += blkSize;
			}
			if(FALSE == complete){
				ret = EPS_ERR_INV_MEDIA_SIZE;
			}
			break;

		case 'T':
			/* get base resolution */
			ret = pm3GetTfieldValue(pmString, pmSize, &pmIdx, NULL, NULL, &baseResolution);
			break;
		case 'L':
			ret = pm3ScanLfield(pmString, pmSize, &pmIdx, NULL, 0, NULL);		/* skip L field */
			break;
		case 'P':
			ret = pm3ScanPfield(pmString, pmSize, &pmIdx, NULL);
			break;
		case ';':
		default:	/* unknown category */
			ret = EPS_ERR_OPR_FAIL;
			break;
		}

		/* If we run into an occurrence of carriage return followed by line feed,
		 * we have found the terminating characters of the string. */
		if(pmString[pmIdx] == ';' && pmString[pmIdx+1] == 0xFF) {
			break;
		}
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epspmCreatePrintAreaInfoFromPM()                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pmString     EPS_UINT8*          I:Pointer to pm string                              */
/* pmSize       EPS_INT32           I:Length of pm string                               */
/* resolution   EPS_INT32           I:caluclate resotution                              */
/* printAreaInfo EPS_PRINT_AREA_INFO* O:pointer to size information                     */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Alloc memory failed                           */
/*                                                                                      */
/* Description:                                                                         */
/*      create printable area information from pm3.                                     */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epspmCreatePrintAreaInfoFromPM(
	
		EPS_UINT8*				pmString,
		EPS_INT32				pmSize,
		EPS_UINT32				resolution,
		EPS_PRINT_AREA_INFO*	printAreaInfo
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
	EPS_PRINTAREA_SOURCE	src;
    EPS_INT32       pmIdx = 0;                  /* pm string index                      */
	EPS_UINT32		baseResolution = 0;
	EPS_INT32		resVal, baseResVal;
	EPS_BOOL		complete = FALSE;

	EPS_LOG_FUNCIN;

	ret = GetPrintAreaSource(resolution, &src);
    if (EPS_ERR_NONE != ret) {
        EPS_RETURN( ret );
    }

	pmIdx = 0;				/* skip the command header of pm string     */
	while( EPS_ERR_NONE == ret && pmIdx < pmSize-3 && FALSE == complete) {
        switch(pmString[pmIdx]) {
		case 'S':
			if(0 == baseResolution){
				ret = pm3GetTfieldValue(pmString, pmSize, &pmIdx, NULL, NULL, &baseResolution);
			}
			resVal = ((resolution & EPS_IR_360X360) || (resolution & EPS_IR_720X720))?
					360:300;
			baseResVal = ((baseResolution & EPS_IR_360X360) || (baseResolution & EPS_IR_720X720))?
					360:300;
			
			ret = pm3ScanSfield(pmString, pmSize, &pmIdx, &src, resVal, baseResVal, printAreaInfo);
			break;

		case 'T':
			/* get base resolution */
			ret = pm3GetTfieldValue(pmString, pmSize, &pmIdx, NULL, NULL, &baseResolution);
			break;
		case 'L':
			ret = pm3ScanLfield(pmString, pmSize, &pmIdx, NULL, 0, NULL);		/* skip L field */
			break;
		case 'P':
			ret = pm3ScanPfield(pmString, pmSize, &pmIdx, NULL);
			break;
		case ';':
		default:	/* unknown category */
			complete = TRUE;
			break;
		}

		/* If we run into an occurrence of carriage return followed by line feed,
		 * we have found the terminating characters of the string. */
		if(pmString[pmIdx] == ';' && pmString[pmIdx+1] == 0xFF) {
			break;
		}
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pm3ScanSfield()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pmString     EPS_UINT8*          I:Pointer to pm string                              */
/* pmSize       EPS_INT32           I:Length of pm string                               */
/* pPmIdx       EPS_INT32*          I/O:pointer to index of pm buffer                   */
/* src          EPS_PRINTAREA_SOURCE* I:pointer to basic size values                    */
/* resVal       EPS_INT32           I:caluclate resotution                              */
/* baseResVal   EPS_INT32           I:pm data resotution                                */
/* printAreaInfo EPS_PRINT_AREA_INFO* O:pointer to size information                     */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Alloc memory failed                           */
/*                                                                                      */
/* Description:                                                                         */
/*      parse S field and make size information. move next field.                       */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE   pm3ScanSfield(
	
		EPS_UINT8*				pmString,
		EPS_INT32				pmSize,
        EPS_INT32*				pPmIdx,
		EPS_PRINTAREA_SOURCE*	src,
		EPS_INT32				resVal, 
		EPS_INT32				baseResVal,
		EPS_PRINT_AREA_INFO*	printAreaInfo
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
	EPS_LAYOUTSIZE_INFO *pSize = NULL;
    EPS_INT32       pmIdx = 0;                  /* pm string index                      */
    EPS_INT32       idx, sIdx;                  /* Index                                */
    EPS_INT32       num_mSize = 0;              /* Media size number                    */
    EPS_INT32       blkSize = 0;                /* data block size                      */
	EPS_INT32       factor = 0;

	EPS_LOG_FUNCIN;

	pmIdx = *pPmIdx;

	pmIdx++;
	num_mSize = pmString[pmIdx];
	pmIdx++;

	if(NULL != printAreaInfo){
		pSize = (EPS_LAYOUTSIZE_INFO*)EPS_ALLOC(sizeof(EPS_LAYOUTSIZE_INFO) * num_mSize);
		if( NULL == pSize ){
			EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
		}
		memset(pSize, 0, sizeof(EPS_LAYOUTSIZE_INFO) * num_mSize);
		printAreaInfo->sizeList = pSize;
		printAreaInfo->numSizes = num_mSize;
	}

	for(idx = 0; idx < num_mSize; idx++){
		if(pmIdx >= pmSize){
			ret = EPS_ERR_OPR_FAIL;  /* Invalid format */	
			break;
		}

		blkSize = pmString[pmIdx] + 1;
		if(blkSize < 14){
			ret = EPS_ERR_OPR_FAIL;  /* Invalid format */	
			break;
		}

		if(NULL != printAreaInfo){
			pSize->mediaSizeID = pmString[pmIdx+1];
			for (sIdx = 0; src->pMI[sIdx].id != -1; sIdx++) {
				if (src->pMI[sIdx].id == pSize->mediaSizeID){
					break;
				}
			}

			pSize->numLayouts = 2; 
			pSize->layoutList = (EPS_LAYOUT_INFO*)EPS_ALLOC(sizeof(EPS_LAYOUT_INFO) * 2);
			if( NULL == pSize->layoutList ){
				EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
			}

			pSize->paperWidth  = src->pMI[sIdx].paper_x * src->factor;
			pSize->paperHeight = src->pMI[sIdx].paper_y * src->factor;

			factor = src->factor;
			pSize->layoutList[0].layout = EPS_MLID_BORDERLESS;
			ret = CalcPrintMargin(TRUE, src, NULL, &pSize->layoutList[0], NULL);
			if (EPS_ERR_NONE != ret) {
				break;
			}
			/* not use printer data
			pSize->layoutList[0].margin.top    = -EPS_PM3_RESCOMV( pmString[pmIdx+2] << 8 | pmString[pmIdx+3] );
			pSize->layoutList[0].margin.left   = 
			pSize->layoutList[0].margin.right  = -EPS_PM3_RESCOMV( pmString[pmIdx+4] << 8 | pmString[pmIdx+5] );
			pSize->layoutList[0].margin.bottom = -EPS_PM3_RESCOMV( pmString[pmIdx+6] << 8 | pmString[pmIdx+7] );
			*/

			pSize->layoutList[1].layout = EPS_MLID_BORDERS;
			pSize->layoutList[1].margin.top    = EPS_PM3_RESCOMV( pmString[pmIdx+8] << 8 | pmString[pmIdx+9] );
			pSize->layoutList[1].margin.left   = 
			pSize->layoutList[1].margin.right  = EPS_PM3_RESCOMV( pmString[pmIdx+10] << 8 | pmString[pmIdx+11] );
			pSize->layoutList[1].margin.bottom = EPS_PM3_RESCOMV( pmString[pmIdx+12] << 8 | pmString[pmIdx+13] );

			pSize++;
		}
		pmIdx += blkSize;
	}

	*pPmIdx = pmIdx;

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pm3ScanTfield()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pmString     EPS_UINT8*          I:Pointer to pm string                              */
/* pmSize       EPS_INT32           I:Length of pm string                               */
/* pPmIdx       EPS_INT32*          I/O:pointer to index of pm buffer                   */
/* totalTypeNum EPS_INT32*          O:pointer to index of pm buffer                     */
/* pTVals       EPS_UINT8**         O:pointer to type & quality pair                    */
/* resolution   EPS_UINT32*         O:pointer to basic resolution                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Alloc memory failed                           */
/*                                                                                      */
/* Description:                                                                         */
/*      parse T field and make type & quality pair. move next field.                    */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE   pm3ScanTfield(
	
		EPS_UINT8*			pmString,
		EPS_INT32			pmSize,
        EPS_INT32*          pPmIdx,
		EPS_INT32*          totalTypeNum,
		EPS_UINT8**		    pTVals,
		EPS_UINT32*         resolution
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
    EPS_INT32       pmIdx;                      /* pm string index                      */
    EPS_INT32       idx;                        /* Index                                */
    EPS_INT32       num_mType = 0;              /* Media type number                    */
    EPS_INT32       blkSize = 0;                /* data block size                      */

	EPS_LOG_FUNCIN;

	pmIdx = *pPmIdx;

	/* make type & quality pair */
	pmIdx++;
	num_mType = pmString[pmIdx];
	if(NULL != totalTypeNum){
		*totalTypeNum = num_mType;
	}
	if(NULL != pTVals){
		*pTVals = (EPS_UINT8*)EPS_ALLOC( *totalTypeNum * 2 );
		if( NULL == pTVals){
			EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
		}
	}

	pmIdx++;
	for(idx = 0; idx < num_mType; idx++){
		if(pmIdx >= pmSize){
			ret = EPS_ERR_OPR_FAIL;  /* Invalid format */	
			break;
		}

		blkSize = pmString[pmIdx] + 1;
		if(blkSize < 7){
			ret = EPS_ERR_OPR_FAIL;  /* Invalid format */	
			break;
		}

		if(NULL != pTVals){
			(*pTVals)[2*idx]   = pmString[pmIdx+1];
			(*pTVals)[2*idx+1] = (pmString[pmIdx+2] & EPS_MQID_ALL);
		    /* If Draft Quality is only supported, add Normal Quality */
			if(EPS_MQID_DRAFT == (*pTVals)[2*idx+1]){
				(*pTVals)[2*idx+1] |= EPS_MQID_NORMAL;
			}
		}

		if(NULL != resolution){
			if( 720 == ((pmString[pmIdx+3] << 8) + pmString[pmIdx+4]) ){
				*resolution |= EPS_IR_720X720;
			} else if( 600 == ((pmString[pmIdx+3] << 8) + pmString[pmIdx+4]) ){
				*resolution |= EPS_IR_300X300 | EPS_IR_600X600;
			} else if( 300 == ((pmString[pmIdx+3] << 8) + pmString[pmIdx+4]) ){
				*resolution |= EPS_IR_300X300;
			}
		}

		pmIdx += blkSize;
	}

	*pPmIdx = pmIdx;

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pm3ScanLfield()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pmString     EPS_UINT8*          I:Pointer to pm string                              */
/* pmSize       EPS_INT32           I:Length of pm string                               */
/* pPmIdx       EPS_INT32*          I/O:pointer to index of pm buffer                   */
/* pTVals       EPS_UINT8*          I:pointer to type & quality pair                    */
/* totalTypeNum EPS_INT32           I:pointer to index of pm buffer                     */
/* pMedia       EPS_SUPPORTED_MEDIA* O:pointer to support media structure               */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Alloc memory failed                           */
/*                                                                                      */
/* Description:                                                                         */
/*      parse P field and create support media information. move next field.            */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE   pm3ScanLfield(
	
		EPS_UINT8*			pmString,
		EPS_INT32			pmSize,
        EPS_INT32*          pPmIdx,
		EPS_UINT8*			pTVals,
		EPS_INT32           totalTypeNum,
		EPS_SUPPORTED_MEDIA* pMedia
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
    EPS_INT32       pmIdx;                      /* pm string index                      */
    EPS_INT32       idx;                        /* Index                                */
    EPS_INT32       sIdx = 0;                   /* Media size index                     */
    EPS_INT32       tIdx = 0;                   /* Media type index                     */
    EPS_INT32       num_mType = 0;              /* Media type number                    */
    EPS_INT32       num_mSize = 0;              /* Media size number                    */
    EPS_INT32       blkSize = 0;                /* data block size                      */

	EPS_LOG_FUNCIN;

	pmIdx = *pPmIdx;

	pmIdx++;
	num_mSize = pmString[pmIdx];
	pmIdx++;

	if(NULL != pMedia){
		pMedia->numSizes = num_mSize;
		pMedia->sizeList = (EPS_MEDIA_SIZE*)EPS_ALLOC( sizeof(EPS_MEDIA_SIZE) * pMedia->numSizes );
		if( NULL == pMedia->sizeList){
			EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
		}
		memset(pMedia->sizeList, 0, sizeof(EPS_MEDIA_SIZE) * pMedia->numSizes);
	}

	for(sIdx = 0; sIdx < num_mSize; sIdx++){
		if(pmIdx >= pmSize){
			ret = EPS_ERR_OPR_FAIL;  /* Invalid format */	
			break;
		}

		num_mType = pmString[pmIdx+1];

		if(NULL != pMedia){
			pMedia->sizeList[sIdx].mediaSizeID = pmString[pmIdx];
			pMedia->sizeList[sIdx].numTypes = num_mType;
	
			pMedia->sizeList[sIdx].typeList = 
				(EPS_MEDIA_TYPE*)EPS_ALLOC( sizeof(EPS_MEDIA_TYPE) * num_mType );
			if (NULL == pMedia->sizeList[sIdx].typeList) {
				EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
			}
			memset(pMedia->sizeList[sIdx].typeList, 0, sizeof(EPS_MEDIA_TYPE) * num_mType);
		}
		pmIdx += 2;

		for(tIdx = 0; tIdx < num_mType; tIdx++){
			if(pmIdx >= pmSize){
				ret = EPS_ERR_OPR_FAIL;  /* Invalid format */	
				break;
			}

			blkSize = pmString[pmIdx] + 1;
			if(blkSize < 5){
				ret = EPS_ERR_OPR_FAIL;  /* Invalid format */	
				break;
			}

			if(NULL != pMedia){
#ifndef GCOMSW_USE_3DMEDIA
				if( EPS_MTID_3D == pmString[pmIdx+1] ){
					pMedia->sizeList[sIdx].numTypes--;
					num_mType--; tIdx--;
					pmIdx += blkSize;	/* move next field */
					continue;
				}
#endif
#ifndef GCOMSW_USE_ROLLE_PAPER
				if( EPS_MTID_PLAIN_ROLL_STICKER  == pmString[pmIdx+1] ||
					EPS_MTID_GROSSY_ROLL_STICKER == pmString[pmIdx+1] ){
					pMedia->sizeList[sIdx].numTypes--;
					num_mType--; tIdx--;
					pmIdx += blkSize;	/* move next field */
					continue;
				}
#endif
				/* type id */
				pMedia->sizeList[sIdx].typeList[tIdx].mediaTypeID = pmString[pmIdx+1];

				/* layout */
				if( pmString[pmIdx+2] & 0x01 ){	/* bit0 */
					pMedia->sizeList[sIdx].typeList[tIdx].layout |= EPS_MLID_BORDERS;
				}
				if( pmString[pmIdx+2] & 0x02 ){	/* bit1 */
					pMedia->sizeList[sIdx].typeList[tIdx].layout |= EPS_MLID_BORDERLESS;
				}
		
				/* duplex */
				if( pmString[pmIdx+2] & 0x04 ){	/* bit2 */
					pMedia->sizeList[sIdx].typeList[tIdx].duplex = EPS_DUPLEX_ENABLE;
				} else{
					pMedia->sizeList[sIdx].typeList[tIdx].duplex = EPS_DUPLEX_DISABLE;
				}

				/* paper source */
				pMedia->sizeList[sIdx].typeList[tIdx].paperSource = 
					(pmString[pmIdx+3] << 8 | pmString[pmIdx+4]) & EPS_MPID_ALL_ESCPR;

				/* quality */
				for(idx = 0; idx < totalTypeNum; idx++){
					if(pMedia->sizeList[sIdx].typeList[tIdx].mediaTypeID == pTVals[2*idx]){
						pMedia->sizeList[sIdx].typeList[tIdx].quality = pTVals[2*idx+1];
						break;
					}
				}
				if(0 == pMedia->sizeList[sIdx].typeList[tIdx].quality){
					ret = EPS_ERR_OPR_FAIL;  /* Invalid format */	
					break;
				}
			}

			if(EPS_ERR_NONE != ret){
				break;
			}
			pmIdx += blkSize;
		}
		if(EPS_ERR_NONE != ret){
			break;
		}
	}

	*pPmIdx = pmIdx;

	EPS_RETURN( ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pm3ScanPfield()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pmString     EPS_UINT8*          I:Pointer to pm string                              */
/* pmSize       EPS_INT32           I:Length of pm string                               */
/* pPmIdx       EPS_INT32*          I/O:pointer to index of pm buffer                   */
/* jpegMax      EPS_INT32*          O:pointer to jpeg max size                          */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*                                                                                      */
/* Description:                                                                         */
/*      parse P field and get jpeg max size. move next field.                           */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE   pm3ScanPfield(
	
		EPS_UINT8*			pmString,
		EPS_INT32			pmSize,
        EPS_INT32*          pPmIdx,
		EPS_INT32*          jpegMax
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
    EPS_INT32       pmIdx;                      /* pm string index                      */
    EPS_INT32       blkSize = 0;                /* data block size                      */

	EPS_LOG_FUNCIN;

	pmIdx = *pPmIdx;

	pmIdx++;
	if(pmIdx+2 >= pmSize){
		EPS_RETURN( EPS_ERR_OPR_FAIL );  /* Invalid format */	
	}

	blkSize = ((pmString[pmIdx] << 8) & 0xFF) + (pmString[pmIdx+1] & 0xFF) + 2;
	if(blkSize < 7){
		EPS_RETURN( EPS_ERR_OPR_FAIL );  /* Invalid format */	
	}
	pmIdx += 2;

	if(NULL != jpegMax){
		*jpegMax = (pmString[pmIdx+2] << 24) + (pmString[pmIdx+3] << 16) + (pmString[pmIdx+4] << 8) + pmString[pmIdx+5]; 
	}

	pmIdx += blkSize;
	*pPmIdx = pmIdx;

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pm3GetTfieldValue()                                                 */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pmString     EPS_UINT8*          I:Pointer to pm string                              */
/* pmSize       EPS_INT32           I:Length of pm string                               */
/* pPmIdx       EPS_INT32*          I/O:pointer to index of pm buffer                   */
/* totalTypeNum EPS_INT32*          O:pointer to index of pm buffer                     */
/* pTVals       EPS_UINT8**         O:pointer to type & quality pair                    */
/* resolution   EPS_UINT32*         O:pointer to basic resolution                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Alloc memory failed                           */
/*                                                                                      */
/* Description:                                                                         */
/*      move to T field and make type & quality pair.                                   */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE   pm3GetTfieldValue(
	
		EPS_UINT8*			pmString,
		EPS_INT32			pmSize,
        EPS_INT32*          pPmIdx,
		EPS_INT32*          totalTypeNum,
		EPS_UINT8**		    pTVals,
		EPS_UINT32*         resolution
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
    EPS_INT32       pmIdx;                      /* pm string index                      */
	EPS_BOOL        complete = FALSE;
	EPS_BOOL        updateMainCounter = FALSE;

	EPS_LOG_FUNCIN;
	pmIdx = *pPmIdx;

	while( EPS_ERR_NONE == ret && FALSE == complete && pmIdx < pmSize-3 ) {
        switch(pmString[pmIdx]) {
		case 'S':
			ret = pm3ScanSfield(pmString, pmSize, &pmIdx, NULL, 0, 0, NULL);	/* skip S field */
			break;

		case 'T':
			updateMainCounter = (pmIdx == *pPmIdx);
			ret = pm3ScanTfield(pmString, pmSize, &pmIdx, totalTypeNum, pTVals, resolution);
			if(updateMainCounter){
				*pPmIdx = pmIdx;
			}
			complete = TRUE;
			break;

		case 'L':
			ret = pm3ScanLfield(pmString, pmSize, &pmIdx, NULL, 0, NULL);		/* skip L field */
			break;

		case 'P':
			ret = pm3ScanPfield(pmString, pmSize, &pmIdx, NULL);
			break;

		case ';':
		default:	/* unknown category */
			complete = TRUE;
			break;
		}

		/* If we run into an occurrence of carriage return followed by line feed,
		 * we have found the terminating characters of the string. */
		if(pmString[pmIdx] == ';' && pmString[pmIdx+1] == 0xFF) {
			break;
		}
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pm3FindTfield()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pmString     EPS_UINT8*          Pointer to pm string                                */
/* pmSize       EPS_INT32           Length of pm string                                 */
/* mediaTypeID  EPS_UINT8           Media Type ID                                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      Pointer to 'T' on the pSfield   - Success                                       */
/*      NULL                            - There is NOT 'T' in the pSfield               */
/*                                                                                      */
/* Description:                                                                         */
/*      Find 'T' field that includs the <id>.                                           */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_UINT8* pm3FindTfield(
	
		EPS_UINT8*			pmString,
		EPS_INT32			pmSize,
        EPS_UINT8           mediaTypeID

){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
    EPS_INT32       pmIdx;                      /* pm string index                      */
    EPS_INT32       idx;                        /* Index                                */
    EPS_INT32       num_mType = 0;              /* Media type number                    */
    EPS_INT32       blkSize = 0;                /* data block size                      */
	EPS_BOOL		complete = FALSE;
    EPS_UINT8*      pT = NULL;

	pmIdx = 0;				/* skip the command header of pm string     */
	while( EPS_ERR_NONE == ret && pmIdx < pmSize-3 && FALSE == complete) {
        switch(pmString[pmIdx]) {
		case 'S':
			ret = pm3ScanSfield(pmString, pmSize, &pmIdx, NULL, 0, 0, NULL);	/* skip S field */
			break;

		case 'T':
			pmIdx++;
			num_mType = pmString[pmIdx];
			pmIdx++;
			for(idx = 0; idx < num_mType; idx++){
				blkSize = pmString[pmIdx] + 1;
				if(blkSize < 7){
					ret = EPS_ERR_OPR_FAIL;  /* Invalid format */	
					break;
				}

				if(mediaTypeID == pmString[pmIdx+1]){
					pT = &pmString[pmIdx];
					complete = TRUE;
					break;
				}

				pmIdx += blkSize;
			}
			break;
		case 'L':
			ret = pm3ScanLfield(pmString, pmSize, &pmIdx, NULL, 0, NULL);		/* skip L field */
			break;
		case 'P':
			ret = pm3ScanPfield(pmString, pmSize, &pmIdx, NULL);		        /* skip P field */
			break;
		case ';':
		default:	/* unknown category */
			complete = TRUE;
			break;
		}

		/* If we run into an occurrence of carriage return followed by line feed,
		 * we have found the terminating characters of the string. */
		if(pmString[pmIdx] == ';' && pmString[pmIdx+1] == 0xFF) {
			break;
		}
	}

	return pT;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pm3ChangeSpec_UpdatePMReply()                                       */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printer      EPS_PRINTER_INN*    I/O: Pointer to a printer infomation                */
/* orgPmString  EPS_UINT8*          I: String of the pm command reply                   */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_SP_INVALID_POINTER      - Input pointer error                           */
/*      EPS_ERR_SP_INVALID_HEADER       - pm string header error                        */
/*      EPS_ERR_SP_INVALID_TERMINATOR   - pm string terminator error                    */
/*      EPS_ERR_SP_NO_VALID_FIELD       - pm string field error                         */
/*                                                                                      */
/* Description:                                                                         */
/*      - Invalid formats       : Delete                                                */
/*      - Unknown 'S' field     : Delete                                                */
/*      - Unknown 'T' field     : Delete                                                */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE     pm3ChangeSpec_UpdatePMReply (

		EPS_PRINTER_INN*    printer,
        EPS_UINT8*		    orgPmString,
        EPS_INT32*		    bufSize

){
/*** Declare Variable Local to Routine                                                  */
	EPS_ERR_CODE ret = EPS_ERR_NONE;
    EPS_UINT8   *tempPmString = NULL;           /* Retrieved PM data from printer       */
    EPS_UINT8   *pSrc, *pDes, *pEOF;
	EPS_UINT8	*pSNum, *pTNum;
    EPS_INT32   idx, s_idx, t_idx;              /* Index                                */
    EPS_INT32   sblkNum, tblkNum; 
    EPS_INT32   blkSize = 0;                    /* data block size                      */
    EPS_UINT8	validSNum, validTNum;
	EPS_BOOL	validSfield;
	EPS_BOOL	complete = FALSE;

	EPS_LOG_FUNCIN;

/*** Validate input parameters                                                          */
    if(orgPmString == NULL) {
        EPS_DBGPRINT(("_SP_ChangeSpec_UpdatePMReply > EPS_ERR_SP_INVALID_POINTER\r\n"));
        EPS_RETURN( EPS_ERR_SP_INVALID_POINTER );
    }

/*** Initialize Local Variables                                                         */
	tempPmString = (EPS_UINT8*)EPS_ALLOC(*bufSize);
	if(NULL == tempPmString){
        EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
	}
    memset(tempPmString, 0x00, *bufSize);

    /* Initialize pm data state */
    printer->pmData.version = 0;
	printer->pmData.length = 0;
	EPS_SAFE_RELEASE( printer->pmData.pmString )

	pSrc = orgPmString;
	pDes = tempPmString;
	pEOF = pSrc + *bufSize;

	while( EPS_ERR_NONE == ret && pSrc < pEOF && FALSE == complete) {
        switch(*pSrc) {
		case 'S':	/*** Correct unknown 'S' fields */
			*pDes++ = *pSrc++;
			sblkNum = *pSrc++;
			pSNum = pDes;
			pDes++;

			validSNum = 0;
			for(s_idx = 0; s_idx < sblkNum; s_idx++){
				blkSize = *pSrc + 1;
				for(idx = 0; idx < EPS_NUM_MEDIA_SIZES; idx++) {
					if(epsMediaSize[idx].id == *(pSrc+1)) {
						memcpy(pDes, pSrc, blkSize);
						pDes += blkSize;
						validSNum++;
	                    break;
					}
                }
				pSrc += blkSize;
			}
			*pSNum = validSNum;
			break;

		case 'T':	/*** Correct unknown 'T' fields */
			*pDes++ = *pSrc++;
			tblkNum = *pSrc++;
			pTNum = pDes;
			pDes++;

			validTNum = 0;
			for(t_idx = 0; t_idx < tblkNum; t_idx++){
				blkSize = *pSrc + 1;
                 for(idx = 0; idx < EPS_NUM_MEDIA_TYPES; idx++) {
                    if(epsMediaTypeIndex[idx] == *(pSrc+1)) {
						memcpy(pDes, pSrc, blkSize);
						pDes += blkSize;
						validTNum++;
                        break;
                    }
                }
				pSrc += blkSize;
            }
			*pTNum = validTNum;
			break;

		case 'L':	/*** Correct unknown Size, Type */
			*pDes++ = *pSrc++;
			sblkNum = *pSrc++;
			pSNum = pDes;
			pDes++;

			validSNum = 0;
			for(s_idx = 0; s_idx < sblkNum; s_idx++){
				validSfield = FALSE;
				for(idx = 0; idx < EPS_NUM_MEDIA_SIZES; idx++) {
					if(epsMediaSize[idx].id == *(pSrc)) {
						validSfield = TRUE;
						validSNum++;
	                    break;
					}
                }

				if(validSfield){
					*pDes++ = *pSrc++;
					tblkNum = *pSrc++;
					pTNum = pDes;
					pDes++;

					validTNum = 0;
					for(t_idx = 0; t_idx < tblkNum; t_idx++){
						blkSize = *pSrc + 1;
						 for(idx = 0; idx < EPS_NUM_MEDIA_TYPES; idx++) {
							if(epsMediaTypeIndex[idx] == *(pSrc+1)) {
								memcpy(pDes, pSrc, blkSize);
								pDes += blkSize;
								validTNum++;
								break;
							}
						}
						pSrc += blkSize;
					}
					*pTNum = validTNum;
				} else{
					pSrc++;
					tblkNum = *pSrc++;
					for(t_idx = 0; t_idx < tblkNum; t_idx++){
						blkSize = *pSrc + 1;
						pSrc += blkSize;
					}
				}
			}
			*pSNum = validSNum;
			break;

		case 'P':
			*pDes++ = *pSrc++;
			blkSize = ((*pSrc << 8) & 0xFF) + (*(pSrc+1) & 0xFF) + 2;
			memcpy(pDes, pSrc, blkSize);
			pSrc += blkSize;
			pDes += blkSize;
			break;

		case ';':
			*pDes++ = *pSrc++;
			*pDes++ = 0xFF;
			complete = TRUE;
			break;

		default:	/* unknown category */
			complete = TRUE;
			break;
		}
	}
	*bufSize = (EPS_INT32)(pDes - tempPmString);

    /* Update orgPmString                                                               */
	memcpy(orgPmString, tempPmString, *bufSize);

	/* set filterd value "printer->pmData.pmString" */
	printer->pmData.pmString = (EPS_UINT8*)EPS_ALLOC(*bufSize);
    if( NULL != printer->pmData.pmString){
		memcpy(printer->pmData.pmString, tempPmString, *bufSize);
		printer->pmData.length = *bufSize;
		printer->pmData.version = 3;
	} else{
		ret = EPS_ERR_MEMORY_ALLOCATION;
    }

	EPS_SAFE_RELEASE( tempPmString )

/*** Return to caller                                                                   */
    EPS_RETURN( EPS_ERR_NONE );
}

/*_______________________________   epson-escpr-pm.c   _________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
