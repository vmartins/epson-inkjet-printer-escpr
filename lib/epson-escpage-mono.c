/*_______________________________  epson-escpage-mono.c   ______________________________*/

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
#include "epson-escpage-mono.h"

/*-----------------------------  Local Macro Definitions -------------------------------*/
/*******************************************|********************************************/
#ifdef EPS_LOG_MODULE_PAGE
#define EPS_LOG_MODULE	EPS_LOG_MODULE_PAGE
#else
#define EPS_LOG_MODULE	0
#endif

/*--------------------------------  ESC/PAGE commands ----------------------------------*/
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
const static EPS_UINT8 sbSDS150[]   = {0x1D, '0', ';', '1', '5', '0', ';', '1', '5', '0', 'd', 'r', 'E'};	/* Select Dot Size(150) */
const static EPS_UINT8 sbSDS300[]   = {0x1D, '0', ';', '3', '0', '0', ';', '3', '0', '0', 'd', 'r', 'E'};	/* Select Dot Size(300) */
const static EPS_UINT8 sbSDS600[]   = {0x1D, '0', ';', '6', '0', '0', ';', '6', '0', '0', 'd', 'r', 'E'};	/* Select Dot Size(600) */

const static EPS_UINT8 sbMMS[]		= {0x1D, '0', 'm', 'm', 'E'};						/* Select Page memory mode */
const static EPS_UINT8 sbPDS[]      = {0x1D, '0', 'p', 'o', 'E'};						/* Set paper direction to normal */
const static EPS_UINT8 sbACRLFS[]	= {0x1D, '0', 'a', 'l', 'f', 'P'};					/* Does not perform auto carriage return */
const static EPS_UINT8 sbAFFS[]		= {0x1D, '0', 'a', 'f', 'f', 'P'};					/* Does not perform auto Form Feed */
const static EPS_UINT8 sbABPS[]		= {0x1D, '0', 'a', 'b', 'P'};						/* Set current position to 0 after bit image draw */
const static EPS_UINT8 sbSARGAS[]	= {0x1D, '0', 's', 'a', 'r', 'G'};					/* Set Absolute Graph Coordinate mode */
const static EPS_UINT8 sbPMPPS[]	= {0x1D, '1', 'p', 'm', 'P'};						/* Set current move mode to print pitch mode */

const static EPS_UINT8 sbSPRCS[]	= {0x1D, '1', '6', '0', 'i', 's', 'E'};				/* Screen Pattern Record Count set to 160 */
const static EPS_UINT8 sbFASCS[]	= {0x1D, '7', 'i', 'a', 'F'};						/* Font Attribute Store Count set to 7 */
const static EPS_UINT8 sbCPSCS[]	= {0x1D, '5', 'i', 'p', 'P'};						/* Current Position Store Count set to 5 */
const static EPS_UINT8 sbMRCS[]		= {0x1D, '1', '6', '0', 'i', 'm', 'M'};				/* Macro Record Store Count set to 160 */
const static EPS_UINT8 sbBIRCS[]	= {0x1D, '2', '3', '2', '8', 'i', 'b', 'I'};		/* Bit Image Count Store Count set to 2328 */
const static EPS_UINT8 sbBIOS[]		= {0x1D, '0', 'b', 'o', 'P'};						/* Set the bit image drawing offset to 0 */
const static EPS_UINT8 sbILGS[]		= {0x1D, '1', '6', '0', 'i', 'l', 'G'};				/* Set line type record count to 160 */
const static EPS_UINT8 sbTSESS[]	= {0x1D, '1', 't', 's', 'E'};						/* Set Screen Mode */
const static EPS_UINT8 sbSPES[]		= {0x1D, '1', ';', '0', ';', '1', '0', '0', 's', 'p', 'E'};	/* Set Screen Pattern */
const static EPS_UINT8 sbOWES[]		= {0x1D, '2', 'o', 'w', 'E'};						/* Set Superimpose Mode */
const static EPS_UINT8 sbBCI1S[]	= {0x1D, '1', 'b', 'c', 'I'};						/* Select Data compression type #1 */
const static EPS_INT8 sbBID[]      = "\x1D""%d;%d;1;0bi{I";								/* Output data */

/*-----------------------------  Local Macro Definitions -------------------------------*/
/*******************************************|********************************************/
#define EPS_EJL_LINEMAX				(256)
#define EPS_PAGE_CMDBUFF_SIZE		EPS_EJL_LINEMAX

#define EMIT_TIFF_REPEAT(n, x)          \
{                                       \
    nRowTIFF += 3;                      \
    if (RowTIFF)                        \
    {                                   \
		*RowTIFF++ = x;                 \
		*RowTIFF++ = x;                 \
		*RowTIFF++ = (EPS_UINT8)n;		\
    }                                   \
}

#define EMIT_TIFF_LITERAL(n, p)         \
{                                       \
    nRowTIFF += n;                      \
    if (RowTIFF)                        \
    {                                   \
        memcpy(RowTIFF, p, n);          \
        RowTIFF += n;                   \
    }                                   \
}

#define TIFF_MAXIMUM_LITERAL 128
#define TIFF_MAXIMUM_REPEAT  129

#define CALCULATE_INTENSITY(r,g,b)  (((((b)<<8) * 11)/100 + (((g)<<8) * 59)/100 + (((r)<<8) * 30)/100)/256)
#define MAX_8           ((1 << 8) - 1)
#define DOT_K           100
#define E_MAX           (1 << 13)
#define E_MID           (E_MAX >> 1)

/* The following table is used to convert a from an intensity we desire	*/
/* to an intensity to tell the device. It is calibrated for the			*/
/* error-diffusion code.												*/
const EPS_UINT8 Intensity2Intensity[256] = 
{
     0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  4,  5,  6,  7,  8,
     9,  9, 10, 10, 11, 12, 12, 13, 13, 14, 14, 14, 15, 15, 16, 16,
    16, 17, 17, 17, 18, 18, 19, 19, 19, 20, 20, 21, 21, 21, 22, 22,
    22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27,
    27, 28, 28, 28, 29, 30, 31, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 41, 42, 43, 44, 44, 45, 46, 47, 47, 48, 48, 49, 49, 49,
    50, 50, 50, 51, 51, 51, 52, 52, 52, 53, 53, 54, 54, 54, 55, 55,
    55, 56, 56, 56, 57, 57, 58, 59, 59, 60, 60, 61, 61, 62, 63, 64,
    64, 65, 65, 65, 66, 66, 67, 67, 68, 68, 69, 70, 70, 71, 72, 73,
    74, 75, 76, 76, 77, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87,
    88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,101,103,104,
   105,106,108,109,110,112,113,114,115,116,118,120,122,124,126,128,
   130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,159,
   160,162,164,165,166,168,170,172,173,174,176,178,180,182,184,186,
   188,190,192,194,196,198,200,202,204,206,208,210,212,214,216,218,
   220,222,224,226,228,230,232,234,236,238,240,242,244,248,252,255 
};

/* Used in color correction */
const static EPS_INT32 BitMask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };


/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/
/* Band data */
typedef struct _tagEPS_PAGE_BAND_M_ {
    EPS_INT32   WidthInBytes;
    EPS_INT32   WidthInPixels;
	EPS_INT32	currentWidthInPixels;
    EPS_UINT8	*pRasterData;
    EPS_UINT8	*pIntensity;
    EPS_INT32   encDataSize;
    EPS_UINT8	*pEncData;
	EPS_UINT8	*pZeroRow;
}EPS_PAGE_BAND_M;

/* Error Diffusion */
typedef struct tagHT_PARAM{
	EPS_INT32	iDK;
	EPS_INT16	*pEK;
	EPS_INT32	iOrder;
	EPS_INT32	iScanPels;
}   HT_PARAM;


/*----------------------------  ESC/P-R Lib Global Variables  --------------------------*/
/*******************************************|********************************************/
extern EPS_CMN_FUNC		epsCmnFnc;
extern EPS_PRINT_JOB	printJob;

/*----------------------------------   Local Variables  --------------------------------*/
/*******************************************|********************************************/
static EPS_PAGE_BAND_M		band;
static EPS_COMMAND_BUFFER	cmdBuf;
static EPS_UINT8*			Palette2DeviceIntensity = NULL;
static EPS_INT32			iRow = 0;
static EPS_INT32			iNumBytes = 0;
static EPS_INT32			iNumDots = 0;
static EPS_INT32			pageCount = 0;	/* Current Page number */

#if EPS_PAGE_HT_ERR_DIFFUSION
HT_PARAM	htParam;
#else
EPS_UINT8 * pP = 0;
EPS_INT32 iOrder;
EPS_INT32 iScanPels;
#endif

/*--------------------------  Local Functions Declaration   ----------------------------*/
/*******************************************|********************************************/
static EPS_ERR_CODE CmdBuffInit		(EPS_COMMAND_BUFFER *pCmdBuff);
static EPS_ERR_CODE CmdBuffGrow		(EPS_COMMAND_BUFFER *pCmdBuff, EPS_INT32 addSize);
static void			CmdBuffTerm		(EPS_COMMAND_BUFFER *pCmdBuff);

static void			ConvertBYTE2Intensity	(const EPS_BANDBMP* pInBmp, EPS_PAGE_BAND_M *pBand);
static EPS_ERR_CODE ConvertPaletteToIntensity(EPS_UINT16 paletteSize, EPS_UINT8 *paletteData);

static EPS_ERR_CODE BandInit		(EPS_PAGE_BAND_M *pBand, EPS_INT32 widthInPixels);
static void			BandTerm		(EPS_PAGE_BAND_M* pBand);
static void			BandEncode		(EPS_PAGE_BAND_M* pBand);
static EPS_ERR_CODE BandEmit		(EPS_PAGE_BAND_M *pBand, EPS_INT32 iNByte, EPS_INT32 iNDot);	

static EPS_ERR_CODE HT_Init			(EPS_INT32 WidthPixels);
static void			HT_End			(void);
static void			HT_StartPage	(void);
static void			HT_Scan			(EPS_UINT8 *Con, EPS_UINT8 *Bin, EPS_INT32 widthInPixels);

static EPS_INT32 DoDeltaRow			(EPS_UINT8 *Row, EPS_INT32 nRow, EPS_UINT8 *RowDeltaRow, EPS_UINT8 *Seed);


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------              Public Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageAllocBuffer_M()			                                        */
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
EPS_ERR_CODE pageAllocBuffer_M(void)
{
	EPS_ERR_CODE	ret = EPS_ERR_NONE;

	EPS_LOG_FUNCIN

	ret = BandInit(&band, printJob.printableAreaWidth);
	if ( EPS_ERR_NONE != ret ) {
		EPS_RETURN( ret )
	}
	
	if (printJob.attr.colorPlane == EPS_CP_256COLOR){
		ret = ConvertPaletteToIntensity(printJob.attr.paletteSize, printJob.attr.paletteData);
		if ( EPS_ERR_NONE != ret ) {
			EPS_RETURN( ret )
		}
	}

	/* Halftoning Initialization */
	ret = HT_Init( printJob.printableAreaWidth );
	if ( EPS_ERR_NONE != ret ) {
		EPS_RETURN( ret )
	}

	ret = CmdBuffInit(&cmdBuf);

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageRelaseBuffer_M()		                                        */
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
void pageRelaseBuffer_M(void)
{
 	EPS_LOG_FUNCIN

	BandTerm(&band);
	
	EPS_SAFE_RELEASE( Palette2DeviceIntensity );

	CmdBuffTerm(&cmdBuf);

	/* Halftoning Ending process */
	HT_End();

	EPS_RETURN_VOID
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageStartJob_M()													*/
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
EPS_ERR_CODE pageStartJob_M(void)
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

	ret = ejlStart(&cmdBuf, CmdBuffGrow);
	if( EPS_ERR_NONE != ret){
		EPS_RETURN( ret )
	}

    /* Step 1 - Start Job */
    /* Initialize the printer. */
	pgStartJob_ADDCMD(sbIP)

    /* Set the minimum Unit Setting (600 or 300 resolution)	 */
	/* Dot Pattern Resolution Selection						 */
	if(       EPS_IR_600X600 == printJob.attr.inputResolution) {
		pgStartJob_ADDCMD(sbSU600);
		pgStartJob_ADDCMD(sbSDS600);
	} else if(EPS_IR_300X300 == printJob.attr.inputResolution) {	
		pgStartJob_ADDCMD(sbSU300);
		pgStartJob_ADDCMD(sbSDS300);
	}else{
		pgStartJob_ADDCMD(sbSU150);
		pgStartJob_ADDCMD(sbSDS150);
	}

	/* Set Memory Mode Selection to Page memory mode */
	pgStartJob_ADDCMD(sbMMS);

	/* Set Screen Record Count */
	pgStartJob_ADDCMD(sbSPRCS);
	
	/* Set Font Attribute Store Count */
	pgStartJob_ADDCMD(sbFASCS);

	/* Set Current Position Store Count */
	pgStartJob_ADDCMD(sbCPSCS);

	/* Set Macro Record Store Count */
	pgStartJob_ADDCMD(sbMRCS);

	/* Set Bit Image Record Store Count */
	pgStartJob_ADDCMD(sbBIRCS);

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

	/* select paper type */
	if(printJob.attr.mediaTypeIdx == EPS_MTID_PLAIN){
		pgStartJob_ADDCMD(sbPTE1);
	}else{
		pgStartJob_ADDCMD(sbPTE0);
	}

	/* Set paper direction */
	pgStartJob_ADDCMD(sbPDS);

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

	/* Set the bit image drawing offset to 0 */
	pgStartJob_ADDCMD(sbBIOS);

	/* Set current position to 0 after Bit image draw */
	pgStartJob_ADDCMD(sbABPS);

	/* Set line type record count to 160 */
	pgStartJob_ADDCMD(sbILGS);

    /* Step 4 - Adjust Vertical Print Position (if necessary) */
    /* CR */
    pgStartJob_ADDCMD(sbCR);
    
    /* Select compression type */
	pgStartJob_ADDCMD(sbBCI1S);

	/* Set Absolute graph coordinate mode */
	pgStartJob_ADDCMD(sbSARGAS);

	/* Set current position move mode to print pitch */
	pgStartJob_ADDCMD(sbPMPPS);

	/* Set Screen Mode */
	pgStartJob_ADDCMD(sbTSESS);

	/* Set Screen Pattern */
	pgStartJob_ADDCMD(sbSPES);

	/* Set Screen Pattern */
	pgStartJob_ADDCMD(sbOWES);

	ret = SendCommand((EPS_UINT8*)cmdBuf.p, cmdBuf.len, &retBufSize, TRUE);

    pageCount = 0;

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageEndJob_M()														*/
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
EPS_ERR_CODE pageEndJob_M()
{
	EPS_ERR_CODE	ret = EPS_ERR_NONE;           /* Return status of internal calls  */
	EPS_UINT32      retBufSize = 0;

	EPS_LOG_FUNCIN

	cmdBuf.len = 0;	/* reset */

	/* Check empty document */
	if( iRow != 0 ) {
		/* Formfeed (FF) */
		memcpy((EPS_INT8*)cmdBuf.p, sbFF, sizeof(sbFF));
		cmdBuf.len += sizeof(sbFF);

		/* Send command to initialize the printer. */
		memcpy((EPS_INT8*)cmdBuf.p+cmdBuf.len, sbIP, sizeof(sbIP));
		cmdBuf.len += sizeof(sbIP);
	}

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
/* Function name:   pageStartPage_M()													*/
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
EPS_ERR_CODE pageStartPage_M()
{
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_UINT32  retBufSize = 0;

	EPS_LOG_FUNCIN

	HT_StartPage();
    iRow = 0;

	if (pageCount++) {

		cmdBuf.len = 0;	/* reset */

		memcpy(cmdBuf.p, sbFF, sizeof(sbFF));
		cmdBuf.len += sizeof(sbFF);

		ret = ejlPageEsc(&cmdBuf, CmdBuffGrow);
		if( EPS_ERR_NONE != ret){
			EPS_RETURN( ret )
		}

		ret = SendCommand((EPS_UINT8*)cmdBuf.p, cmdBuf.len, &retBufSize, TRUE);
    }

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageColorRow_M()													*/
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
EPS_ERR_CODE pageColorRow_M(        
						  
		const EPS_BANDBMP*  pInBmp,
        EPS_RECT*           pBandRec

){
 	EPS_ERR_CODE	ret = EPS_ERR_NONE;

	EPS_LOG_FUNCIN

    if( (EPS_UINT32)(pBandRec->right - pBandRec->left) <= printJob.printableAreaWidth){
        band.currentWidthInPixels = (EPS_UINT16)(pBandRec->right - pBandRec->left);
    } else{
        band.currentWidthInPixels = (EPS_UINT16) printJob.printableAreaWidth;
    }

	ConvertBYTE2Intensity( pInBmp, &band );

    memset( band.pRasterData, 0, band.WidthInBytes);
	HT_Scan( band.pIntensity, (EPS_UINT8*)band.pRasterData, band.currentWidthInPixels);

	iNumBytes = 0;
	iNumDots = 0;

	BandEncode(&band);

    if( band.encDataSize ) {
		ret = BandEmit(&band, iNumBytes, iNumDots);
    }

	iRow++;

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   pageSendLeftovers_M()												*/
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
EPS_ERR_CODE pageSendLeftovers_M(
							   
		   void
							   
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
	EPS_UINT32      retBufSize = 0;

	EPS_LOG_FUNCIN

	if( NULL != printJob.contData.sendData && 0 < printJob.contData.sendDataSize){
		/* send command */
		ret = SendCommand(printJob.contData.sendData, 
								printJob.contData.sendDataSize, &retBufSize, TRUE);
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
								
		EPS_COMMAND_BUFFER *pCmdBuff
		
){
	pCmdBuff->size = EPS_EJL_LINEMAX*2;
	pCmdBuff->p = (EPS_INT8*)EPS_ALLOC(pCmdBuff->size);
	if( NULL == pCmdBuff->p){
		return EPS_ERR_MEMORY_ALLOCATION;
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
								
		EPS_COMMAND_BUFFER *pCmdBuff, 
		EPS_INT32			addSize
		
){
	EPS_INT32	block, sizeNew;

	if(pCmdBuff->size < pCmdBuff->len + addSize){
		block = (((pCmdBuff->len+addSize) / EPS_PAGE_CMDBUFF_SIZE) + 1);
		sizeNew = block * EPS_PAGE_CMDBUFF_SIZE;
		pCmdBuff->p =  memRealloc(pCmdBuff->p, pCmdBuff->size, sizeNew);
		if( NULL == pCmdBuff->p){
			return EPS_ERR_MEMORY_ALLOCATION;
		}
		pCmdBuff->size = sizeNew;
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
									 
		EPS_COMMAND_BUFFER	*pCmdBuff
									 
){
	EPS_SAFE_RELEASE(pCmdBuff->p);
	pCmdBuff->len = 0;
	pCmdBuff->size = 0;
	pCmdBuff->pExtent = NULL;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   ConvertBYTE2Intensity()												*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* paletteSize  EPS_UINT16			    I: Palette Size									*/
/* paletteData  EPS_UINT8*			    I: Palette Size									*/
/*                                                                                      */
/* Return value:                                                                        */
/* N/A																					*/
/*                                                                                      */
/* Description:                                                                         */
/*      Convert palette to Intensity.													*/
/*                                                                                      */
/*******************************************|********************************************/
static void ConvertBYTE2Intensity(
								  
		const EPS_BANDBMP* pInBmp, 
		EPS_PAGE_BAND_M *pBand
		
){
	EPS_UINT32 idx;
	EPS_INT32 i, j, widthBytes;

	EPS_LOG_FUNCIN;

	widthBytes = pBand->currentWidthInPixels * printJob.bpp;
	j = 0;
	if(printJob.bpp == 3){
		for (i = 0; i < widthBytes; i += printJob.bpp) {
			/* idx = CALCULATE_INTENSITY(pInBmp->bits[i], pInBmp->bits[i+1], pInBmp->bits[i+2]);*/
			idx = pInBmp->bits[i];
			idx = 255 - idx;
			if (idx > 255) idx = 255;

			pBand->pIntensity[j++] = Intensity2Intensity[idx];

			if(j >= pBand->WidthInPixels){
				break;
			}
	   }
	} else{
		for (i = 0; i < widthBytes; i += printJob.bpp) {
			idx = pInBmp->bits[i];
			pBand->pIntensity[j++] = Palette2DeviceIntensity[idx];
			if(j >= pBand->WidthInPixels){
				break;
			}
		}
	}
 
   EPS_RETURN_VOID
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   ConvertPaletteToIntensity()											*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* paletteSize  EPS_UINT16			    I: Palette Size									*/
/* paletteData  EPS_UINT8*			    I: Palette Size									*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Convert palette to Intensity.													*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE ConvertPaletteToIntensity(

		EPS_UINT16  paletteSize,
		EPS_UINT8	*paletteData
					   
){
	EPS_UINT32 idx;
	EPS_UINT32 i, j;
	EPS_LOG_FUNCIN;

	Palette2DeviceIntensity = (EPS_UINT8*)EPS_ALLOC( (paletteSize/3)*sizeof(EPS_UINT8) );
	if (!Palette2DeviceIntensity) {
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
	}
	j = 0;
    for (i = 0; i < paletteSize; i += 3) {
		/* idx = CALCULATE_INTENSITY(paletteData[i], paletteData[i+1], paletteData[i+2]); */
		idx = paletteData[i];
		idx = 255 - idx;
        if (idx > 255) idx = 255;
		Palette2DeviceIntensity[j++] = Intensity2Intensity[idx];
	}

	EPS_RETURN( EPS_ERR_NONE )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   BandInit()															*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pBand		EPS_PAGE_BAND_M*		I: band data strcture		                    */
/* widthInPixels EPS_INT32			    I: width in Pixels								*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Allocate band data buffers.														*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE BandInit(
							 
		EPS_PAGE_BAND_M*	pBand, 
		EPS_INT32		widthInPixels
		
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
	EPS_INT32 maxEpsonEncodedData;

	EPS_LOG_FUNCIN;

	memset(pBand, 0, sizeof(EPS_PAGE_BAND_M));

    pBand->WidthInPixels = widthInPixels;
    pBand->WidthInBytes = (pBand->WidthInPixels+7)/8;

    /* We leave room for an entire TIFF literal -plus- room	*/
    /* for the longest possible XFER header (3 bytes).		*/
    maxEpsonEncodedData = 256 + 3 + pBand->WidthInBytes
        + (pBand->WidthInBytes + TIFF_MAXIMUM_LITERAL - 1)/TIFF_MAXIMUM_LITERAL;

    pBand->pRasterData       = (EPS_UINT8*)EPS_ALLOC(pBand->WidthInBytes);
    if (NULL == pBand->pRasterData) {
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
    }

	pBand->pEncData = (EPS_UINT8*)EPS_ALLOC(maxEpsonEncodedData);
    if (NULL == pBand->pEncData) {
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
    }

	pBand->pZeroRow			 = (EPS_UINT8*)EPS_ALLOC(pBand->WidthInBytes);
    if (NULL == pBand->pZeroRow) {
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
    }
    memset(pBand->pZeroRow, 0, pBand->WidthInBytes);

    pBand->pIntensity        = (EPS_UINT8*)EPS_ALLOC(pBand->WidthInPixels);
	if (!pBand->pIntensity){
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
	}

	EPS_RETURN(ret)
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   BandTerm()															*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pBand		EPS_PAGE_BAND_M*		I: band data strcture		                    */
/*                                                                                      */
/* Return value:                                                                        */
/* N/A																					*/
/*                                                                                      */
/* Description:                                                                         */
/*      free band data buffers.															*/
/*                                                                                      */
/*******************************************|********************************************/
static void BandTerm(
					 
		EPS_PAGE_BAND_M* pBand
		
){
	EPS_LOG_FUNCIN

	EPS_SAFE_RELEASE( pBand->pRasterData );
	EPS_SAFE_RELEASE( pBand->pEncData );
	EPS_SAFE_RELEASE( pBand->pZeroRow );
	EPS_SAFE_RELEASE( pBand->pIntensity );

	EPS_RETURN_VOID
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   BandEmit()															*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pBand		EPS_PAGE_BAND_M*		I: band data strcture		                    */
/* pCmdBuff     EPS_COMMAND_BUFFER*     I: command buffer strcture                      */
/* iNByte		EPS_INT32				I: number of data bytes							*/
/* iNDot		EPS_INT32				I: number of data dots							*/
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
							 
		EPS_PAGE_BAND_M*		pBand, 
		EPS_INT32			iNByte, 
		EPS_INT32			iNDot
							 
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
	EPS_INT8		*p;
	EPS_UINT32      retBufSize = 0;

	EPS_LOG_FUNCIN

	cmdBuf.len = 0;	/* reset */
	p = (EPS_INT8*)cmdBuf.p;

	/*EPS_DBGPRINT(("Y = %d \r\n", iRow));*/
	sprintf(p, "\x1D%dY", iRow);
	p += strlen(p);

	/* Number of data bytes	 */
	/* Bit image width		 */
	/* Bit image height		 */
	/* Angle of rotation	 */
	sprintf(p, sbBID, iNByte, iNDot);
	p += strlen(p);

	cmdBuf.len += (EPS_UINT32)(p - cmdBuf.p);	
	ret = CmdBuffGrow(&cmdBuf, pBand->encDataSize);
	if( EPS_ERR_NONE != ret){
		EPS_RETURN( ret )
	}
	p = cmdBuf.p + cmdBuf.len;

	memcpy(p, pBand->pEncData, pBand->encDataSize);
	cmdBuf.len += pBand->encDataSize;

	pBand->encDataSize = 0;

	ret = SendCommand((EPS_UINT8*)cmdBuf.p, cmdBuf.len, &retBufSize, TRUE);

	EPS_RETURN(ret)
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   BandEncode()														*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:		Type:					Description:                                    */
/* pBand		EPS_PAGE_BAND_M*		I: band data strcture		                    */
/*                                                                                      */
/* Return value:                                                                        */
/* N/A																					*/
/*                                                                                      */
/* Description:                                                                         */
/*      Do Compression.																	*/
/*                                                                                      */
/*******************************************|********************************************/
static void BandEncode(
					   
		EPS_PAGE_BAND_M* pBand
		
){
	EPS_BOOL isZero = FALSE;
    EPS_INT32 widthInBytes = (pBand->currentWidthInPixels+7)/8;

	EPS_LOG_FUNCIN

	/* Check blank line */
	isZero = (memcmp(pBand->pRasterData, pBand->pZeroRow, widthInBytes) == 0)?TRUE:FALSE;

	/* No compression for blank line */
	if( isZero ) {
		pBand->encDataSize = 0;
	} else{
       	pBand->encDataSize = DoDeltaRow( pBand->pRasterData,
                                    widthInBytes,
                                    pBand->pEncData,
                                    pBand->pZeroRow
                                  );
	}

	EPS_RETURN_VOID
}


/*=========================================================*/
/*                                                         */
/* Function:                                               */
/*                                                         */
/*      HT_Init											   */
/*                                                         */
/* Description:                                            */
/*                                                         */
/*      Halftoning initialization						   */
/*                                                         */
/*=========================================================*/
static EPS_ERR_CODE HT_Init(EPS_INT32 WidthPixels)
{
	EPS_LOG_FUNCIN
	 
	srand(119);

    htParam.pEK			= NULL;
    htParam.iScanPels	= WidthPixels;
    htParam.iOrder		= 5;
    htParam.iDK			= DOT_K * E_MAX / 100;

	htParam.pEK = (EPS_INT16*)EPS_ALLOC((htParam.iScanPels + 2)*sizeof(EPS_INT16));
	if( NULL == htParam.pEK){
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
	}

	EPS_RETURN( EPS_ERR_NONE )
}


/*=========================================================*/
/*                                                         */
/* Function:                                               */
/*                                                         */
/*      HT_End											   */
/*                                                         */
/* Description:                                            */
/*                                                         */
/*      Halftoning Ending process						   */
/*                                                         */
/*=========================================================*/
static void HT_End()
{
EPS_LOG_FUNCIN
	EPS_SAFE_RELEASE( htParam.pEK );
EPS_RETURN_VOID
}


/*=========================================================*/
/*                                                         */
/* Function:                                               */
/*                                                         */
/*      HT_StartPage									   */
/*                                                         */
/* Description:                                            */
/*                                                         */
/*      Halftoning start page process					   */
/*                                                         */
/*=========================================================*/
static void HT_StartPage()
{
	EPS_LOG_FUNCIN

	if (htParam.pEK){
        memset(htParam.pEK, 0, (htParam.iScanPels + 2) * sizeof(EPS_INT16));
	}

	EPS_RETURN_VOID
}


/*=========================================================*/
/*                                                         */
/* Function:                                               */
/*                                                         */
/*      HT_Scan											   */
/*                                                         */
/* Description:                                            */
/*                                                         */
/*      Check halftoning method							   */
/*                                                         */
/*=========================================================*/
static void HT_Scan(EPS_UINT8 *Con, EPS_UINT8 *Bin, EPS_INT32 widthInPixels)
{
/*      Error Diffusion									   */
    EPS_INT32 i, j, k;
    EPS_INT32 iE, iE1, iE3, iE5, iE7, iR;
    EPS_INT32 iCon;
    EPS_INT32 iDot;
	EPS_INT32 iF;
    EPS_INT16 *p;

	EPS_LOG_FUNCIN

	p    = htParam.pEK + 1;
	iDot = htParam.iDK;

    iE1 = 0;
    iE7 = 0;
    iR  = 0;

	i = 0;
	j = 1;
    
    for (k = 0; k < widthInPixels; k++) {
        if (Con[i] == 0) {
        } else if (Con[i] == MAX_8) {
            Bin[i >> 3] |= BitMask[i & 7];
        } else {
            iCon = Con[i] * (E_MAX / (MAX_8 + 1));

            iE = rand() * (iCon - E_MID) / RAND_MAX;
            if (((iR > 0) && (iE > 0)) || ((iR < 0) && (iE < 0))) {
                iE = -iE;
			}
            iR = iE;

            iCon += p[i] + iE7;
            
            if (iCon + iR > E_MID) {
                Bin[i >> 3] |= BitMask[i & 7];
                iE = iCon - iDot;
            } else {
                iE = iCon;
            }

			iF = iE / 16;
            iE3 = iF * 3;
            iE5 = iF * 5;

            p[i]      = iE5 + iE1;
            p[i - j] += iE3;

            iE1 = iF;
            iE7 = iE - iE1 - iE3 - iE5;
        }

        i += j;
    }

    p[i - j] += iE7 + iE1;

    p[0] += p[-1];
    p[htParam.iScanPels - 1] += p[htParam.iScanPels];

    p[-1] = 0;
    p[htParam.iScanPels] = 0;
	
	EPS_RETURN_VOID
}


/*=========================================================*/
/*                                                         */
/* Function:                                               */
/*                                                         */
/*      DoTIFF                                             */
/*                                                         */
/* Description:                                            */
/*                                                         */
/*      Do TIFF compression                                */
/*                                                         */
/*=========================================================*/
EPS_INT32 DoTIFF(EPS_UINT8 *Row, EPS_INT32 nRow, EPS_UINT8 *RowTIFF)
{
    EPS_INT32 nRowTIFF = 0;      /* Total size of TIFF encoded row. */
    EPS_INT32 nLiteral = 0;      /* Count of literal bytes. */
    EPS_INT32 nRepeat  = 0;      /* Count of repeated byte. */
	EPS_UINT8 ch; 
	EPS_INT32 n;

    /* Load first byte from row into parser. */
    nRepeat++;
	nLiteral++;
    ch = *Row++;

    for( ; nRow--; ch = *Row++ ) {
        if( nLiteral > TIFF_MAXIMUM_LITERAL ) {
            /* Literal part has grown too large, emit the largest literal pattern we can. */
            EMIT_TIFF_LITERAL( TIFF_MAXIMUM_LITERAL, Row-nLiteral );
            nLiteral -= TIFF_MAXIMUM_LITERAL;
        } else if( nRepeat > TIFF_MAXIMUM_REPEAT ) {
            /* Repeated part has grown too large, emit the literal (if present) and then  */
			/* emit the largest repeated pattern we can. */
			EMIT_TIFF_REPEAT( TIFF_MAXIMUM_REPEAT, ch );
            nRepeat -= TIFF_MAXIMUM_REPEAT;
        }
        if( ch == *Row ) {
			if( nLiteral ) {
				if( nLiteral >= 2 ) {
					EMIT_TIFF_LITERAL( nLiteral-1, Row-nLiteral );
				}
				nLiteral = 1;
			}
            nRepeat++;
        } else {
			if( nRepeat >= 2 ) {
				EMIT_TIFF_REPEAT( nRepeat, ch );
				nRepeat = 1;
				nLiteral = 0;
			}
			nLiteral++;
        }
    }
    if( nRepeat == 1 ) {
        nRepeat  = 0;
    }
	if( nLiteral == 1 && nRepeat >= 2 ) {
        nLiteral = 0;
    }

    while (nLiteral) {
        n = nLiteral;
        if (n > TIFF_MAXIMUM_LITERAL) {
            n = TIFF_MAXIMUM_LITERAL;
        }
		if( nRepeat ) {
			EMIT_TIFF_LITERAL(n-1, Row-nLiteral);
		} else {
			EMIT_TIFF_LITERAL(n-1, Row-nLiteral);
		}
        nLiteral -= n;
    }

    while (nRepeat) {
        n = nRepeat;
        if (n > TIFF_MAXIMUM_REPEAT)  {
            n = TIFF_MAXIMUM_REPEAT;
        }
        EMIT_TIFF_REPEAT(n, ch);
        nRepeat -= n;
    }

	return nRowTIFF;
}


/*=========================================================*/
/*                                                         */
/* Function:                                               */
/*                                                         */
/*      DeltaRowEmitXFER                                   */
/*                                                         */
/* Description:                                            */
/*                                                         */
/*      Construct DeltaRow control code and data           */
/*                                                         */
/*=========================================================*/
void DeltaRowEmitXFER(EPS_UINT8 *Row, EPS_INT32 nXFER, EPS_INT32 nXFERTIFF, EPS_INT32 *nRowDeltaRow, EPS_UINT8 **RowDeltaRow)
{
	EPS_LOG_FUNCIN;

    /* Get data length and send before all data of the current row */
    iNumBytes += nXFERTIFF;
	iNumDots += nXFER * 8;

    /* TIFF data for XFER */
    *nRowDeltaRow += nXFERTIFF;
    if (*RowDeltaRow)
    {
        DoTIFF(Row, nXFER, *RowDeltaRow);
        *RowDeltaRow += nXFERTIFF;
    }

	EPS_RETURN_VOID
}

/*=========================================================*/
/*                                                         */
/* Function:                                               */
/*                                                         */
/*      DeltaRowEmitMOVX                                   */
/*                                                         */
/* Description:                                            */
/*                                                         */
/*      Construct DeltaRow horizontal position movement    */
/*                                                         */
/*=========================================================*/
void DeltaRowEmitMOVX(EPS_INT32 nSkip, EPS_INT32 *nRowDeltaRow, EPS_UINT8 **RowDeltaRow)
{
    EPS_UINT8 N;
    EPS_INT32 num=0;
    EPS_INT32 num1=0;

	EPS_LOG_FUNCIN;

    /* Send repeat "00" instead. Send 255 "00" the most each time */
    if( nSkip == 0 ) {
		EPS_RETURN_VOID
    }
    
    if( nSkip > 255 ) {
        num1 = nSkip - 255;
        num = 255;
    } else {
        num = nSkip;
    }
    *nRowDeltaRow += 3;
    iNumBytes += 3;
	iNumDots += num * 8;

    if (*RowDeltaRow) {
        N = num;
        *((*RowDeltaRow)++) = 0x00;
		*((*RowDeltaRow)++) = 0x00;
		*((*RowDeltaRow)++) = N;
    }
    if( num1 > 255 ) {
        /* Still have more than 255 "00" */
        DeltaRowEmitMOVX(num1, nRowDeltaRow, RowDeltaRow);
    } else {
        if( num1 > 0 ) {
            *nRowDeltaRow += 3;
            iNumBytes += 3;
			iNumDots += num1 * 8;
            if (*RowDeltaRow)
            {
                N = num1;
                *((*RowDeltaRow)++) = 0x00;
				*((*RowDeltaRow)++) = 0x00;
				*((*RowDeltaRow)++) = N;
            }
        }
    }

	EPS_RETURN_VOID
}


/*=========================================================*/
/*                                                         */
/* Function:                                               */
/*                                                         */
/*      DeltaRowEmitReduce                                 */
/*                                                         */
/* Description:                                            */
/*                                                         */
/*                                                         */
/*                                                         */
/*=========================================================*/
void DeltaRowEmitReduce(EPS_UINT8 *Row, EPS_INT32 *nDelta0, EPS_INT32 *nSkip, EPS_INT32 *nDelta1, EPS_INT32 *nRowDeltaRow, EPS_UINT8 **RowDeltaRow)
{
    EPS_INT32 nDelta0TIFF, nDelta1TIFF, nDeltaTIFF, SizeofDSD, SizeofD;

	EPS_LOG_FUNCIN;
	
	if (*nDelta0) {
        /* (DSD) state. */
        nDelta0TIFF = DoTIFF(Row - *nDelta1 - *nSkip - *nDelta0, *nDelta0, 0);
        nDelta1TIFF = DoTIFF(Row - *nDelta1,                     *nDelta1, 0);
        nDeltaTIFF  = DoTIFF(Row - *nDelta1 - *nSkip - *nDelta0, *nDelta0 + *nSkip + *nDelta1, 0);

        SizeofDSD = nDelta0TIFF + nDelta1TIFF + (nDelta0TIFF>15?1:0)
            + (nDelta0TIFF>255?1:0) + (nDelta1TIFF>15?1:0)
            + (nDelta1TIFF>255?1:0) + (*nSkip>7?1:0) + (*nSkip>127?1:0) + 3;
        SizeofD = nDeltaTIFF + (nDeltaTIFF>15?1:0) + (nDeltaTIFF>255?1:0) + 1;

    } else {
        /* (-SD) state. */
        nDelta0TIFF = 0;
        nDelta1TIFF = DoTIFF(Row - *nDelta1, *nDelta1, 0);
        nDeltaTIFF  = DoTIFF(Row - *nDelta1 - *nSkip, *nSkip + *nDelta1, 0);

        SizeofDSD = nDelta1TIFF + (nDelta1TIFF>15?1:0)
            + (nDelta1TIFF>255?1:0) + (*nSkip>7?1:0) + (*nSkip>127?1:0) + 2;
        SizeofD = nDeltaTIFF + (nDeltaTIFF>15?1:0) + (nDeltaTIFF>255?1:0) + 1;
    }

    if (SizeofD <= SizeofDSD) {
        /* Turn the delta/skip/delta into a single delta. */
        *nDelta0 += *nSkip + *nDelta1;
        *nSkip = 0;
        *nDelta1 = 0;

    } else {
        /* Emit a delta, then a skip, and the shift the leftover delta. */
        if (*nDelta0) {
            DeltaRowEmitXFER(Row- *nDelta0 - *nSkip - *nDelta1, *nDelta0, nDelta0TIFF, nRowDeltaRow, RowDeltaRow);
        }
        DeltaRowEmitMOVX(*nSkip, nRowDeltaRow, RowDeltaRow);

        /* Shift remaining delta */
        *nDelta0 = *nDelta1;
        *nSkip = 0;
        *nDelta1 = 0;
    }

	EPS_RETURN_VOID
}


/*=========================================================*/
/*                                                         */
/* Function:                                               */
/*                                                         */
/*      DoDeltaRow                                         */
/*                                                         */
/* Description:                                            */
/*                                                         */
/*      Do DeltaRow compression                            */
/*                                                         */
/*=========================================================*/
static EPS_INT32 DoDeltaRow(EPS_UINT8 *Row, EPS_INT32 nRow, EPS_UINT8 *RowDeltaRow, EPS_UINT8 *Seed)
{
	EPS_INT32 nRowDeltaRow = 0;
    EPS_INT32 nSkip        = 0;
    EPS_INT32 nDelta1      = 0;
    EPS_INT32 nDelta0      = 0;
	EPS_INT32 nDelta0TIFF;

    for ( ; nRow--; Row++, Seed++) {
		if (*Row != *Seed) {
			nDelta1++;
		}
        else
        {
            if (nDelta1)
            {
                if (nDelta0 == 0 && nSkip ==0)
                {
                    nDelta0 = nDelta1;
                    nDelta1 = 0;
                }
                else
                {
                    if (nSkip <= 3)
                    {
                        nDelta0 += nSkip + nDelta1;
                        nSkip = 0;
                        nDelta1 = 0;
                    }
                    else
                    {
                       DeltaRowEmitReduce(Row, &nDelta0, &nSkip, &nDelta1,
                            &nRowDeltaRow, &RowDeltaRow);
                    }
                }
            }
            nSkip++;
        }
    }

    /* Have reached the end of the line. */
    if (nDelta0)
    {
        if (nDelta1)
        {
            DeltaRowEmitReduce(Row, &nDelta0, &nSkip, &nDelta1, &nRowDeltaRow, &RowDeltaRow);
            nDelta0TIFF = DoTIFF(Row-nDelta0, nDelta0, 0);
            DeltaRowEmitXFER(Row-nDelta0, nDelta0, nDelta0TIFF, &nRowDeltaRow, &RowDeltaRow);
        }
        else
        {
            nDelta0TIFF = DoTIFF(Row-nSkip-nDelta0, nDelta0, 0);
            DeltaRowEmitXFER(Row-nDelta0-nSkip, nDelta0, nDelta0TIFF, &nRowDeltaRow, &RowDeltaRow);
        }
    }
    else if (nDelta1)
    {
        if (nSkip)
        {
            DeltaRowEmitReduce(Row, &nDelta0, &nSkip, &nDelta1, &nRowDeltaRow, &RowDeltaRow);
            nDelta0TIFF = DoTIFF(Row-nDelta0, nDelta0, 0);
            DeltaRowEmitXFER(Row-nDelta0, nDelta0, nDelta0TIFF, &nRowDeltaRow, &RowDeltaRow);
        }
        else
        {
            nDelta0TIFF = DoTIFF(Row-nDelta1, nDelta1, 0);
            DeltaRowEmitXFER(Row-nDelta1, nDelta1, nDelta0TIFF, &nRowDeltaRow, &RowDeltaRow);
        }
    }

    return nRowDeltaRow;
}

/*_______________________________  epson-escpage-mono.c   ______________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
