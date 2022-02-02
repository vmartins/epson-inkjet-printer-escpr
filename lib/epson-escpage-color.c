/*______________________________  epson-escpage-color.c   ______________________________*/

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
#include "epson-escpr-err.h"
#include "epson-escpr-media.h"
#include "epson-escpr-mem.h"
#include "epson-escpage.h"
#include "epson-escpage-comp.h"
#include "epson-escpage-color.h"

/*-----------------------------  Local Macro Definitions -------------------------------*/
/*******************************************|********************************************/
#ifdef EPS_LOG_MODULE_PAGE
#define EPS_LOG_MODULE	EPS_LOG_MODULE_PAGE
#else
#define EPS_LOG_MODULE	0
#endif


/*-----------------------------  ESC/PAGE-Color commands -------------------------------*/
/*******************************************|********************************************/
const static EPS_UINT8 sbCR[]       = {0x0D};											/* Carriage return "CR" */
const static EPS_UINT8 sbFF[]		= {0x0C};											/* Form Feed "FF" */
const static EPS_UINT8 sbIP[]		= {0x1D, 'r', 'h', 'E'};							/* Initialize printer */
const static EPS_UINT8 sbSU150[]    = {0x1D, '0', ';', '.', '4', '8', 'm', 'u', 'E'};	/* Set unit to 150 */
const static EPS_UINT8 sbSU300[]    = {0x1D, '0', ';', '.', '2', '4', 'm', 'u', 'E'};	/* Set unit to 300 */
const static EPS_UINT8 sbSU600[]    = {0x1D, '0', ';', '.', '1', '2', 'm', 'u', 'E'};	/* Set unit to 600 */

const static EPS_UINT8 sbPSSL[]     = {0x1D, '3', '0', 'p', 's', 'E'};					/* Set Letter paper */
const static EPS_UINT8 sbPSSA4[]	= {0x1D, '1', '4', 'p', 's', 'E'};					/* Set A4 paper */
const static EPS_UINT8 sbPSSB5[]    = {0x1D, '2', '5', 'p', 's', 'E'};					/* Set B5 paper */
const static EPS_UINT8 sbPSSLE[]	= {0x1D, '3', '2', 'p', 's', 'E'};					/* Set Legal paper */
const static EPS_UINT8 sbPSSA3[]	= {0x1D, '1', '3', 'p', 's', 'E'};					/* Set A3 paper */
const static EPS_UINT8 sbPSSB4[]    = {0x1D, '2', '4', 'p', 's', 'E'};					/* Set B4 paper */
const static EPS_UINT8 sbPSSPS[]    = {0x1D, '3', '8', 'p', 's', 'E'};					/* Set B4 paper */

const static EPS_UINT8 sbPTE0[]		= {0x1D, '0', 'p', 't', 'E'};						/* select paper type 0:other */
const static EPS_UINT8 sbPTE1[]		= {0x1D, '1', 'p', 't', 'E'};						/* select paper type 1:plane */

const static EPS_UINT8 sbCMS[]      = {0x1D, '1', 'c', 'm', 'E'};						/* Clip Mode Set */
const static EPS_UINT8 sbSDS600_0[] = {0x1D, '0', ';', '6', '0', '0', ';', '6', '0', '0', 'd', 'r', 'E'};	/* Select Dot Size(600) 0:image */
const static EPS_UINT8 sbSDS600_1[] = {0x1D, '1', ';', '6', '0', '0', ';', '6', '0', '0', 'd', 'r', 'E'};	/* Select Dot Size(600) 1:font */
const static EPS_UINT8 sbSDS600_2[] = {0x1D, '2', ';', '6', '0', '0', ';', '6', '0', '0', 'd', 'r', 'E'};	/* Select Dot Size(600) 2:binary format pattern */
const static EPS_UINT8 sbSDS600_3[] = {0x1D, '3', ';', '6', '0', '0', ';', '6', '0', '0', 'd', 'r', 'E'};	/* Select Dot Size(600) 3:color pattern */
const static EPS_UINT8 sbSDS300_0[] = {0x1D, '0', ';', '3', '0', '0', ';', '3', '0', '0', 'd', 'r', 'E'};	/* Select Dot Size(300) 0:image */
const static EPS_UINT8 sbSDS300_1[] = {0x1D, '1', ';', '3', '0', '0', ';', '3', '0', '0', 'd', 'r', 'E'};	/* Select Dot Size(300) 1:font */
const static EPS_UINT8 sbSDS300_2[] = {0x1D, '2', ';', '3', '0', '0', ';', '3', '0', '0', 'd', 'r', 'E'};	/* Select Dot Size(300) 2:binary format pattern */
const static EPS_UINT8 sbSDS300_3[] = {0x1D, '3', ';', '3', '0', '0', ';', '3', '0', '0', 'd', 'r', 'E'};	/* Select Dot Size(300) 3:color pattern */
const static EPS_UINT8 sbSDS150_0[] = {0x1D, '0', ';', '1', '5', '0', ';', '1', '5', '0', 'd', 'r', 'E'};	/* Select Dot Size(150) 0:image */
const static EPS_UINT8 sbSDS150_1[] = {0x1D, '1', ';', '1', '5', '0', ';', '1', '5', '0', 'd', 'r', 'E'};	/* Select Dot Size(150) 1:font */
const static EPS_UINT8 sbSDS150_2[] = {0x1D, '2', ';', '1', '5', '0', ';', '1', '5', '0', 'd', 'r', 'E'};	/* Select Dot Size(150) 2:binary format pattern */
const static EPS_UINT8 sbSDS150_3[] = {0x1D, '3', ';', '1', '5', '0', ';', '1', '5', '0', 'd', 'r', 'E'};	/* Select Dot Size(150) 3:color pattern */

const static EPS_UINT8 sbMMS[]		= {0x1D, '0', 'm', 'm', 'E'};						/* Select Page memory mode */
const static EPS_UINT8 sbPDS[]      = {0x1D, '0', 'p', 'o', 'E'};						/* Set paper direction to normal */
const static EPS_UINT8 sbACRLFS[]   = {0x1D, '0', 'a', 'l', 'f', 'P'};					/* Does not perform auto carriage return */
const static EPS_UINT8 sbAFFS[]     = {0x1D, '0', 'a', 'f', 'f', 'P'};					/* Does not perform auto Form Feed */
const static EPS_UINT8 sbABPS[]     = {0x1D, '1', 'a', 'b', 'P'};						/* Set current position to 0 after bit image draw */
const static EPS_UINT8 sbSARGAS[]   = {0x1D, '2', 's', 'a', 'r', 'G'};					/* Set Absolute Graph Coordinate mode */
const static EPS_UINT8 sbPMPPS[]    = {0x1D, '0', 'p', 'm', 'P'};						/* Set current move mode to print pitch mode */
const static EPS_UINT8 sbCSE[]      = {0x1D, '2', 'c', 's', 'E'};						/* Select color space */
const static EPS_UINT8 sbPDDO[]     = {0x1D, '0', 'p', 'd', 'd', 'O'};					/* Select Imaging device 0:CPGI 1:PGI */
const static EPS_UINT8 sbCCME[]		= {0x1D, '0', ';', '2', ';', '4', 'c', 'c', 'm', 'E'};					/* Select color collect */
const static EPS_UINT8 sbCAME[]     = {0x1D, '7', ';', '0', ';', '2', ';', '3', 'c', 'a', 'm', '{', 'E'};
const static EPS_UINT8 sbRAE_1[]    = {0x1D, '1', ';', '1', 'r', 'a', 'E'};
const static EPS_UINT8 sbRAE_2[]    = {0x1D, '2', ';', '1', 'r', 'a', 'E'};
const static EPS_UINT8 sbRAE_3[]    = {0x1D, '3', ';', '2', 'r', 'a', 'E'};
const static EPS_UINT8 sbCMME_0[]	= {0x1D, '0', 'c', 'm', 'm', 'E'};
const static EPS_UINT8 sbCMME_1[]	= {0x1D, '1', 'c', 'm', 'm', 'E'};
const static EPS_UINT8 sbCLFP[]     = {0x1D, '0', ';', '0', ';', '0', 'c', 'l', 'f', 'P'};
const static EPS_UINT8 sbWFE[]      = {0x1D, '5', ';', '2', '0', '4', 'w', 'f', 'E'};

const static EPS_UINT8 sbBCTI20[]	= {0x1D, '2', ';', '2', '0', 'b', 'c', 't', '{', 'I'};
const static EPS_UINT8 sbC20[]      = {0x1, 0x2};

const static EPS_INT8 sbSCRI[]		= "scrI";
const static EPS_INT8 sbCRI[]		= "\x1D%d;%dcr{I";
const static EPS_INT8 sbCUI[]		= "cu{I";
const static EPS_UINT8 sbECRI[]		= {0x1D, 'e', 'c', 'r', 'I'};

const static EPS_INT8 sbplrBE[]     = "\x1D%d;0;2;256;256plr{E";

/*-----------------------------  Local Macro Definitions -------------------------------*/
/*******************************************|********************************************/
#define EPS_PAGE_RASTER				(64)		/* number of scanning rasters */
#define EPS_PAGE_CMDBUFF_SIZE		EPS_EJL_LINEMAX


/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/
typedef struct _tagEPS_PAGE_BAND_ 
{
	EPS_INT32	compressType;		/* compless mode */
	EPS_INT32	widthInPixels;
	EPS_INT32	currentWidthInPixels;
	EPS_INT32	rasterRow;
	EPS_INT32   encDataSize;		/* Compless data size */
	EPS_UINT8	*pEncData;			/* Compless data */
	EPS_INT32	scanLineSize;		/* Source data line size */
	EPS_INT32	scanBuffSize;		/* Source data buffer size */
	EPS_INT32   scanDataSize;		/* Source data size */
	EPS_UINT8	*pScanData;			/* Source data */
	EPS_BOOL	emited;
	EPS_BOOL	dataSent;
}EPS_PAGE_BAND;

/*----------------------------  ESC/P-R Lib Global Variables  --------------------------*/
/*******************************************|********************************************/
extern EPS_CMN_FUNC		epsCmnFnc;
extern EPS_PRINT_JOB	printJob;

/*----------------------------------   Local Variables  --------------------------------*/
/*******************************************|********************************************/
static EPS_PAGE_BAND		band;
static EPS_COMMAND_BUFFER	cmdBuf;
static EPS_INT32			pageCount = 0;
static EPS_INT32			emitRowCount = 0;


/*--------------------------  Local Functions Declaration   ----------------------------*/
/*******************************************|********************************************/
static EPS_ERR_CODE CmdBuffInit		(EPS_COMMAND_BUFFER *pCmdBuff, EPS_PAGE_BAND *pBand);
static EPS_ERR_CODE CmdBuffGrow		(EPS_COMMAND_BUFFER *pCmdBuff, EPS_INT32 addSize);
static void			CmdBuffTerm		(EPS_COMMAND_BUFFER *pCmdBuff);

static EPS_ERR_CODE RegisterPalette	(EPS_UINT16, EPS_UINT8*, EPS_COMMAND_BUFFER	*pCmdBuff);

static EPS_BOOL		BandInit		(EPS_PAGE_BAND* pBand);
static void			BandTerm		(EPS_PAGE_BAND* pBand);
static EPS_BOOL		BandEmit		(EPS_PAGE_BAND* pBand);	
static EPS_ERR_CODE BandSetScanData	(EPS_PAGE_BAND* pBand, const EPS_UINT8 *Row, EPS_UINT32 DeltaByte);
static EPS_ERR_CODE BandCompress	(EPS_PAGE_BAND*	pBand, EPS_INT32);
static void			BandClear		(EPS_PAGE_BAND* pBand);

static EPS_INT32 GetEncodeMode(EPS_INT32 bpp);

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------              Public Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageAllocBuffer_C()			                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* N/A																					*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Allocate buffer for ESC/Page Job.												*/
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE pageAllocBuffer_C(void)
{
	EPS_ERR_CODE	ret = EPS_ERR_NONE;

	EPS_LOG_FUNCIN

	ret = BandInit(&band);

	if ( EPS_ERR_NONE == ret ) {
		ret = CmdBuffInit(&cmdBuf, &band);
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageRelaseBuffer_C()		                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* N/A																					*/
/*                                                                                      */
/* Return value:                                                                        */
/*      N/A                                                                             */
/*                                                                                      */
/* Description:                                                                         */
/*      Free buffer for ESC/Page Job.			                                        */
/*                                                                                      */
/*******************************************|********************************************/
void pageRelaseBuffer_C(void)
{
	/*** Clear Memory */
	CmdBuffTerm(&cmdBuf);
	BandTerm(&band);
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageStartJob_C()													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* N/A																					*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Send ESC/Page start job commands.				                                */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE pageStartJob_C(void)
{
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
	EPS_UINT32      retBufSize = 0;

	EPS_LOG_FUNCIN

#define pgStartJob_ADDCMD(CMD) {							\
		ret = CmdBuffGrow(&cmdBuf, sizeof(CMD) );			\
		if( EPS_ERR_NONE != ret){EPS_RETURN( ret )}			\
		memcpy(cmdBuf.p + cmdBuf.len, CMD, sizeof(CMD));	\
		cmdBuf.len += sizeof(CMD);							\
	}

	cmdBuf.len = 0;	/* reset */

	/*** Make EJL Command ***/
	ret = ejlStart(&cmdBuf, CmdBuffGrow);
	if( EPS_ERR_NONE != ret){
		EPS_RETURN( ret )
	}

    /* Initialize the printer. */
	pgStartJob_ADDCMD(sbIP)

    /* Set the minimum Unit Setting (600 or 300 resolution) */
	/* Dot Pattern Resolution Selection						 */
	if(       EPS_IR_600X600 == printJob.attr.inputResolution) {
		pgStartJob_ADDCMD(sbSU600);
		pgStartJob_ADDCMD(sbSDS600_3);
		pgStartJob_ADDCMD(sbSDS600_2);
		pgStartJob_ADDCMD(sbSDS600_1);
		pgStartJob_ADDCMD(sbSDS600_0);
	} else if(EPS_IR_300X300 == printJob.attr.inputResolution) {	
		pgStartJob_ADDCMD(sbSU300);
		pgStartJob_ADDCMD(sbSDS300_3);
		pgStartJob_ADDCMD(sbSDS300_2);
		pgStartJob_ADDCMD(sbSDS300_1);
		pgStartJob_ADDCMD(sbSDS300_0);
	}
	else{
		pgStartJob_ADDCMD(sbSU150);
		pgStartJob_ADDCMD(sbSDS150_3);
		pgStartJob_ADDCMD(sbSDS150_2);
		pgStartJob_ADDCMD(sbSDS150_1);
		pgStartJob_ADDCMD(sbSDS150_0);
	}

	/* Set Memory Mode Selection to Page memory mode */
	pgStartJob_ADDCMD(sbMMS);
	
	/* Set Paper Size */
	switch ( printJob.attr.mediaSizeIdx ) {
	case EPS_MSID_A4:		pgStartJob_ADDCMD(sbPSSA4);	break;
	case EPS_MSID_B5:		pgStartJob_ADDCMD(sbPSSB5);	break;
	case EPS_MSID_B4:		pgStartJob_ADDCMD(sbPSSB4);	break;	
	case EPS_MSID_A3:		pgStartJob_ADDCMD(sbPSSA3);	break;
	case EPS_MSID_LETTER:	pgStartJob_ADDCMD(sbPSSL);	break;
	case EPS_MSID_LEGAL:	pgStartJob_ADDCMD(sbPSSLE);	break;
	case EPS_MSID_POSTCARD:	pgStartJob_ADDCMD(sbPSSPS);	break;
	default:				pgStartJob_ADDCMD(sbPSSA4);	break;
	}

	ret = ejlPageEsc(&cmdBuf, CmdBuffGrow);
	if( EPS_ERR_NONE != ret){
		EPS_RETURN( ret )
	}

	/* Set Clip Mode */
	pgStartJob_ADDCMD(sbCMS);
	
	/* Does not perform automatic carriage return line feed */
	pgStartJob_ADDCMD(sbACRLFS);

	/* Does not perform automatic Form Feed */
	pgStartJob_ADDCMD(sbAFFS);

	/* Set current position to 0 after Bit image draw */
	pgStartJob_ADDCMD(sbABPS);

    /* Step 4 - Adjust Vertical Print Position (if necessary) */
    /* CR */
    pgStartJob_ADDCMD(sbCR);
    
	/* Set Absolute graph coordinate mode */
	pgStartJob_ADDCMD(sbSARGAS);

	/* Set current position move mode to print pitch */
	pgStartJob_ADDCMD(sbPMPPS);

	/* select paper type */
	if(printJob.attr.mediaTypeIdx == EPS_MTID_PLAIN){
		pgStartJob_ADDCMD(sbPTE1);
	} else{
		pgStartJob_ADDCMD(sbPTE0);
	}

	/* Set paper direction */
	pgStartJob_ADDCMD(sbPDS);

	/* Select Imaging device */
	pgStartJob_ADDCMD(sbPDDO);

	/* Select color space */
	pgStartJob_ADDCMD(sbCSE);

	/* cancel color controle */
	pgStartJob_ADDCMD(sbCMME_0);

	/* Select color collect */
	pgStartJob_ADDCMD(sbCCME);

	/* Select color condition */
	pgStartJob_ADDCMD(sbCAME);

	/* Select screening */
	pgStartJob_ADDCMD(sbRAE_1);
	pgStartJob_ADDCMD(sbRAE_2);
	pgStartJob_ADDCMD(sbRAE_3);

	/* set color controle */
	pgStartJob_ADDCMD(sbCMME_1);

	pgStartJob_ADDCMD(sbCLFP);

	/* set logical operation */
	pgStartJob_ADDCMD(sbWFE);
	
	switch( band.compressType )
	{
	case EP_COMPRESS20:
		pgStartJob_ADDCMD(sbBCTI20);	/* set compless table */
		pgStartJob_ADDCMD(sbC20);		/* set data format */
		break;
	default:/* for 8bpp */
		break;
	}
	ret = SendCommand((EPS_UINT8*)cmdBuf.p, cmdBuf.len, &retBufSize, TRUE);

    pageCount = 0;

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageEndJob_C()														*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* N/A																					*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Send ESC/Page end job commands.					                                */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE pageEndJob_C()
{
	EPS_ERR_CODE	ret = EPS_ERR_NONE;           /* Return status of internal calls  */
	EPS_UINT32      retBufSize = 0;

	EPS_LOG_FUNCIN

	cmdBuf.len = 0;	/* reset */

	/* end color image drawing */
	memcpy((EPS_INT8*)cmdBuf.p, sbECRI, sizeof(sbECRI));
	cmdBuf.len += sizeof(sbECRI);

	/*** Make EJL Command ***/
	ret = ejlEnd(&cmdBuf, CmdBuffGrow, pageCount);
	if( EPS_ERR_NONE != ret){
		EPS_RETURN( ret )
	}

	ret = SendCommand((EPS_UINT8*)cmdBuf.p, cmdBuf.len, &retBufSize, TRUE);

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageStartPage_C()													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* N/A																					*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Send ESC/Page start page commands.					                            */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE pageStartPage_C()
{
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_UINT32  retBufSize = 0;

	EPS_LOG_FUNCIN

	cmdBuf.len = 0;	/* reset */
	if( 0 == pageCount ){
		if (printJob.attr.colorPlane == EPS_CP_256COLOR){
			ret = RegisterPalette(printJob.attr.paletteSize, printJob.attr.paletteData,
							&cmdBuf);
		}
	} else{
		memcpy(cmdBuf.p, sbFF, sizeof(sbFF));
		cmdBuf.len += sizeof(sbFF);

		ret = ejlPageEsc(&cmdBuf, CmdBuffGrow);
	}
	if( EPS_ERR_NONE != ret){
		EPS_RETURN( ret )
	}

	if(cmdBuf.len > 0){
		ret = SendCommand((EPS_UINT8*)cmdBuf.p, cmdBuf.len, &retBufSize, TRUE);
	}

	emitRowCount = 0;
	pageCount++;

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageEndPage_C()														*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* N/A																					*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Send ESC/Page end page commands.					                            */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE pageEndPage_C()
{
	EPS_ERR_CODE ret = EPS_ERR_NONE;

	EPS_LOG_FUNCIN

	if( EP_COMPRESS0 != band.compressType ){
		BandCompress(&band, printJob.bpp*8);
		if( band.encDataSize > 0 ) {
			ret = BandEmit(&band);
		}
	} else{
		if( band.scanDataSize > 0 ){
			ret = BandEmit(&band);
		}
	}

	if( EPS_ERR_NONE == ret){
		BandClear(&band);
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageColorRow_C()													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* N/A																					*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Send ESC/Page raster commands.						                            */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE pageColorRow_C(        
						  
		const EPS_BANDBMP*  pInBmp,
        EPS_RECT*           pBandRec

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
    EPS_UINT16      linePixelSize;
    EPS_UINT16      widthBytes;

	EPS_LOG_FUNCIN

    if( (EPS_UINT32)(pBandRec->right - pBandRec->left) <= printJob.printableAreaWidth){
        linePixelSize = (EPS_UINT16)(pBandRec->right - pBandRec->left);
    } else{
        linePixelSize = (EPS_UINT16) printJob.printableAreaWidth;
    }
	widthBytes = linePixelSize * printJob.bpp;
	band.currentWidthInPixels = linePixelSize;
	/* reset scan size from current line size */
	band.scanLineSize = widthBytes;
	if( band.scanLineSize & 3 ){
		/* 4bite align */
		band.scanLineSize = (band.scanLineSize & ~3) + 4;
	}

	BandSetScanData(&band, pInBmp->bits, widthBytes);

	if(EP_COMPRESS0 != band.compressType){
		if( (band.rasterRow >= EPS_PAGE_RASTER )) {
			BandCompress(&band, printJob.bpp*8);
			if( band.encDataSize > 0 ) {
				ret = BandEmit(&band);
			}

			emitRowCount += band.rasterRow;
			if( EPS_ERR_NONE == ret){
				BandClear(&band);		/* reset */
			}
		}
	} else{
		ret = BandEmit(&band);
		emitRowCount += band.rasterRow;
		if( EPS_ERR_NONE == ret){
			BandClear(&band);		/* reset */
		}
	}

 	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageSendLeftovers_C()												*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* N/A																					*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_INVALID_CALL            - This call was unnecessary                     */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      send leftovers data.                                                            */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE pageSendLeftovers_C(
							   
		   void
							   
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
	EPS_UINT32      retBufSize = 0;

	EPS_LOG_FUNCIN

	if( NULL != printJob.contData.sendData && 0 < printJob.contData.sendDataSize){
		/* send command */
		ret = SendCommand(printJob.contData.sendData, 
								printJob.contData.sendDataSize, &retBufSize, TRUE);

		if( EPS_ERR_NONE == ret ){
			if( band.dataSent ){
				/* send encode data */
				if( EP_COMPRESS0 != band.compressType ){
					ret = SendCommand(band.pEncData, band.encDataSize, &retBufSize, TRUE);
				} else{
					ret = SendCommand(band.pScanData, band.scanDataSize, &retBufSize, TRUE);
				}

				band.dataSent = FALSE;
			}
			if( EPS_ERR_NONE == ret){
				BandClear(&band);
			}
		}
	} else{
		ret = EPS_ERR_INVALID_CALL;
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


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CmdBuffInit()														*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pCmdBuff     EPS_COMMAND_BUFFER*     I: command buffer strcture                      */
/* pBand		EPS_PAGE_BAND*			I: band data strcture		                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Allocate command buffer.			                                            */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE CmdBuffInit(
								
		EPS_COMMAND_BUFFER*	pCmdBuff, 
		EPS_PAGE_BAND*		pBand
		
){
	pCmdBuff->pExtent	= pBand;

	if(EP_COMPRESS0 == pBand->compressType){
		pCmdBuff->size = EPS_EJL_LINEMAX;
		pCmdBuff->p = EPS_ALLOC(pCmdBuff->size);
		if( NULL == pCmdBuff->p){
			return EPS_ERR_MEMORY_ALLOCATION;
		}

	} else{
		/* use scan-buffer as command-buffer */
		pCmdBuff->size	= pBand->scanBuffSize;
		pCmdBuff->p		= (EPS_INT8*)pBand->pScanData;
	}

	pCmdBuff->len = 0;

	return EPS_ERR_NONE;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CmdBuffGrow()														*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pCmdBuff     EPS_COMMAND_BUFFER*     I: command buffer strcture                      */
/* addSize      EPS_INT32			    I: growing size                                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*                                                                                      */
/* Description:                                                                         */
/*      ReAllocate command buffer.			                                            */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE CmdBuffGrow(
								
		EPS_COMMAND_BUFFER*	pCmdBuff, 
		EPS_INT32			addSize
		
){
	EPS_INT32	block, sizeNew;
	EPS_PAGE_BAND	*pBand = (EPS_PAGE_BAND*)pCmdBuff->pExtent;

	if(pCmdBuff->size < pCmdBuff->len + addSize){
		block = (((pCmdBuff->len+addSize) / EPS_PAGE_CMDBUFF_SIZE) + 1);
		sizeNew = block * EPS_PAGE_CMDBUFF_SIZE;
		pCmdBuff->p =  memRealloc(pCmdBuff->p, pCmdBuff->size, sizeNew);
		if( NULL == pCmdBuff->p){
			return EPS_ERR_MEMORY_ALLOCATION;
		}
		pCmdBuff->size = sizeNew;

		if(EP_COMPRESS0 != pBand->compressType){
			pBand->scanBuffSize	= cmdBuf.size;
			pBand->pScanData	= (EPS_UINT8*)cmdBuf.p;
		}
	}

	return EPS_ERR_NONE;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CmdBuffTerm()														*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pCmdBuff     EPS_COMMAND_BUFFER*     I: command buffer strcture                      */
/*                                                                                      */
/* Return value:                                                                        */
/* N/A																					*/
/*                                                                                      */
/* Description:                                                                         */
/*      Free command buffer.				                                            */
/*                                                                                      */
/*******************************************|********************************************/
static void CmdBuffTerm		(
									 
		EPS_COMMAND_BUFFER*	pCmdBuff
									 
){
	EPS_PAGE_BAND* pBand = (EPS_PAGE_BAND*)pCmdBuff->pExtent;

	if( NULL != pBand ){
		if(EP_COMPRESS0 == pBand->compressType){
			EPS_SAFE_RELEASE(pCmdBuff->p);
		} else{
			pCmdBuff->p = NULL;
		}
	}

	pCmdBuff->len = 0;
	pCmdBuff->size = 0;
	pCmdBuff->pExtent = NULL;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   RegisterPalette()													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* paletteSize  EPS_UINT16			    I: Palette Size									*/
/* paletteData  EPS_UINT8*			    I: Palette Size									*/
/* pCmdBuff     EPS_COMMAND_BUFFER*     I: command buffer strcture                      */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Register the palette for Printer.                                               */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE RegisterPalette(  

		EPS_UINT16			paletteSize,
		EPS_UINT8*			paletteData,
		EPS_COMMAND_BUFFER*	pCmdBuff

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_INT8	*p = NULL;
	EPS_UINT16	entryNum = paletteSize / 3;	/* 3 = RGB */
	EPS_INT32	i = 0;

	EPS_LOG_FUNCIN

#ifdef GCOMSW_DEBUG
	if(pCmdBuff->len+EPS_EJL_LINEMAX > pCmdBuff->size){EPS_DBGPRINT(("Command buffer not enough!\r\n"))}
#endif
	p = pCmdBuff->p + pCmdBuff->len;

	sprintf(p, sbplrBE, entryNum*4);		/* 4 = RGBA */
	pCmdBuff->len += (EPS_UINT32)strlen(p);

	/* expand for palette data */
	ret = CmdBuffGrow(pCmdBuff, entryNum*4);
	if( EPS_ERR_NONE != ret){
		EPS_RETURN( ret )
	}
	p = pCmdBuff->p + pCmdBuff->len;

	/* set palette */
	for(i = 0; i < paletteSize; i+=3){
		*p++ = paletteData[i+2];	/* B */
		*p++ = paletteData[i+1];	/* G */
		*p++ = paletteData[i];		/* R */
		*p++ = 0;					/* dummy */
	}
	pCmdBuff->len = (EPS_INT32)(p - pCmdBuff->p);

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   BandInit()															*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pBand		EPS_PAGE_BAND*			I: band data strcture		                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Allocate band data buffers.														*/
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE BandInit(
					  
		EPS_PAGE_BAND*	pBand
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;

	EPS_LOG_FUNCIN

	memset(pBand, 0, sizeof(EPS_PAGE_BAND));
	pBand->compressType		= GetEncodeMode( printJob.attr.colorPlane );	/* Set compless mode */
	pBand->widthInPixels	= printJob.printableAreaWidth;
	pBand->scanDataSize		= 0;
	pBand->encDataSize		= 0;
	pBand->emited			= FALSE;
	pBand->dataSent			= FALSE;

	if(EP_COMPRESS0 == pBand->compressType){
		pBand->scanLineSize = pBand->widthInPixels * printJob.bpp;
		pBand->scanBuffSize = pBand->scanLineSize;	/* one line buffering */
	} else{
		pBand->scanLineSize = (pBand->widthInPixels * printJob.bpp);
		if( pBand->scanLineSize & 3 ){
			/* 4bite align */
			pBand->scanLineSize = (pBand->scanLineSize & ~3) + 4;
		}
		pBand->scanBuffSize = pBand->scanLineSize * EPS_PAGE_RASTER;
	}
	pBand->pScanData = (EPS_UINT8*)EPS_ALLOC( pBand->scanBuffSize );
	if( NULL == pBand->pScanData ){
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   BandTerm()															*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pBand		EPS_PAGE_BAND*			I: band data strcture		                    */
/*                                                                                      */
/* Return value:                                                                        */
/* N/A																					*/
/*                                                                                      */
/* Description:                                                                         */
/*      free band data buffers.															*/
/*                                                                                      */
/*******************************************|********************************************/
static void BandTerm(
					 
		EPS_PAGE_BAND* pBand
		
){
	EPS_LOG_FUNCIN

	EPS_SAFE_RELEASE(pBand->pScanData);
	BandClear(pBand);

	EPS_RETURN_VOID
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   BandEmit()															*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pBand		EPS_PAGE_BAND*			I: band data strcture		                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Emit data to printer.															*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE BandEmit(
							 
		EPS_PAGE_BAND*	pBand
		
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
	EPS_INT8		*p = NULL;
	EPS_UINT32      retBufSize = 0;

	EPS_LOG_FUNCIN

	cmdBuf.len = 0;	/* reset */
	p = (EPS_INT8*)cmdBuf.p;

	if( pBand->emited ){
		/* end color image drawing for previous data */
		memcpy(p, sbECRI, sizeof(sbECRI));
		p += sizeof(sbECRI);
	} else{
		pBand->emited = TRUE;
	}

	/* Reset the current absolute position */
	strcpy(p, "\x1D""0X");
	p += strlen(p);

	/*EPS_DBGPRINT(("Emit Y pos = %d \r\n", emitRowCount));*/
	sprintf(p, "\x1D%dY", emitRowCount);
	p += strlen(p);

	/* ----- stretch color image -----  */
	if(printJob.attr.colorPlane == EPS_CP_256COLOR){
		strcpy(p, "\x1D""2;204;0;");		/* Indirectly RGB8, palett=0 */
	} else{
		strcpy(p, "\x1D""2;102;0;");		/* RGB888 */
	}
	p += strlen(p);

	switch(pBand->compressType){
	case EP_COMPRESS20:
		strcpy(p, "20;");
		break;
	case EP_COMPRESS0:
	default:
		strcpy(p, "0;");
		break;
	}
	p += strlen(p);

	sprintf(p, "%d;%d;%d;%d;0%s", pBand->currentWidthInPixels, pBand->rasterRow, 
					pBand->currentWidthInPixels, pBand->rasterRow, sbSCRI);
	p += strlen(p);

	/*** Draw color image ***/ 
	if(EP_COMPRESS20 == pBand->compressType){
		/* Number of data bytes */
		/* Bit image width */
		sprintf(p, sbCRI, pBand->encDataSize, pBand->rasterRow);
		p += strlen(p);
		ret = SendCommand((EPS_UINT8*)cmdBuf.p, (EPS_INT32)(p - (EPS_INT8*)cmdBuf.p), &retBufSize, TRUE);

		pBand->dataSent = TRUE;			/* encode date until send  */
		if( EPS_ERR_NONE == ret){
			ret = SendCommand(pBand->pEncData, pBand->encDataSize, &retBufSize, TRUE);
			pBand->dataSent = FALSE;	/* encode date has sent */
		}

	} else{	/* non compless */
		/* Number of data bytes */
		/* Bit image width */
		sprintf(p, "\x1D""%d;%d%s", pBand->scanDataSize, pBand->rasterRow, sbCUI);
		p += strlen(p);

		ret = SendCommand((EPS_UINT8*)cmdBuf.p, (EPS_INT32)(p - (EPS_INT8*)cmdBuf.p), &retBufSize, TRUE);

		pBand->dataSent = TRUE;			/* encode date until send  */
		if( EPS_ERR_NONE == ret){
			ret = SendCommand(pBand->pScanData, pBand->scanDataSize, &retBufSize, TRUE);
			pBand->dataSent = FALSE;	/* encode date has sent */
		}
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   BandSetScanData()													*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pBand		EPS_PAGE_BAND*			I: band data strcture		                    */
/* Row			const EPS_UINT8*        I: band data                                    */
/* DeltaByte	EPS_UINT32				I: number of data bytes							*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*                                                                                      */
/* Description:                                                                         */
/*      save band data.																	*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE BandSetScanData(
									
		EPS_PAGE_BAND*		pBand, 
		const EPS_UINT8*	Row, 
		EPS_UINT32			DeltaByte
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;

	EPS_LOG_FUNCIN

	pBand->rasterRow++;

	if(EP_COMPRESS0 != pBand->compressType){
		memcpy(pBand->pScanData + (pBand->scanLineSize * (EPS_PAGE_RASTER - pBand->rasterRow)), Row, DeltaByte);
	} else{
		memcpy(pBand->pScanData, Row, DeltaByte);
	}
	pBand->scanDataSize += DeltaByte;

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   BandCompress()														*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pBand		EPS_PAGE_BAND*			I: band data strcture		                    */
/* WidthBytes	EPS_INT32		        I: data length                                  */
/* bitCount		EPS_INT32				I: image bit count                              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*                                                                                      */
/* Description:                                                                         */
/*      Data is compressed.																*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE BandCompress(
								 
		EPS_PAGE_BAND*	pBand, 
		EPS_INT32		bitCount
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_BITMAPINFO		BitMapInfo;
	EPS_UINT32			lCompressType;
	EPS_UINT32			lImageSize = 0;
	EPS_RECT rect;
	EPS_UINT8*	pRealBits;

	EPS_LOG_FUNCIN
	
	memset(&BitMapInfo, 0, sizeof(EPS_BITMAPINFO));

	BitMapInfo.biWidth = pBand->currentWidthInPixels;
    BitMapInfo.biHeight = pBand->rasterRow;
 	BitMapInfo.biBitCount = bitCount;

	rect.left = rect.top = 0;
	rect.bottom = pBand->rasterRow;
	rect.right = pBand->currentWidthInPixels;

	pRealBits = pBand->pScanData + (pBand->scanLineSize * (EPS_PAGE_RASTER - pBand->rasterRow));

	pBand->pEncData =	CompressBitImage(
								&rect,
								&BitMapInfo,
								pRealBits,
								pBand->compressType,
								&lCompressType,
								&lImageSize
							);
	pBand->encDataSize = lImageSize;

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   BandClear()															*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pBand		EPS_PAGE_BAND*			I: band data strcture		                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*                                                                                      */
/* Description:                                                                         */
/*      Reset band data.																*/
/*                                                                                      */
/*******************************************|********************************************/
static void BandClear(
					  
		EPS_PAGE_BAND *pBand
		
){
	EPS_LOG_FUNCIN
	EPS_SAFE_RELEASE(pBand->pEncData);
	pBand->scanDataSize = 0;
	pBand->encDataSize = 0;
	pBand->rasterRow = 0;

	EPS_RETURN_VOID
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   GetEncodeMode()														*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pBand		EPS_PAGE_BAND*			I: band data strcture		                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    compless mode									*/
/*                                                                                      */
/* Description:                                                                         */
/*      choose compless mode.															*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT32 GetEncodeMode(

		EPS_INT32 bpp

){
	if(bpp == EPS_CP_256COLOR){
		return EP_COMPRESS0;
	} else{ /* if EPS_CP_FULLCOLOR */
		return EP_COMPRESS20;
	}
}

/*______________________________  epson-escpage-color.c   ______________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
