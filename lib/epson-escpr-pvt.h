/*_____________________________   epson-escpr-pvt.h   __________________________________*/

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
/*                        Epson ESC/PR Private Data Definitions                         */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef __EPSON_ESCPR_PVT_H__
#define __EPSON_ESCPR_PVT_H__
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------  Local Compiler Switch  -------------------------------*/
/*******************************************|********************************************/

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"
#include "epson-escpr-def.h"

#ifdef GCOMSW_EXTENTION
#include "epson-escpr-extention.h"
#endif
#ifdef GCOMSW_PRIVATE
#include "epson-escpr-closed.h"
#endif

/*------------------------------------- Data Types -------------------------------------*/
/*******************************************|********************************************/
typedef void*          EPS_HANDLE;		/* handle                                       */

/*--------------------------------------  Macros   -------------------------------------*/
/*******************************************|********************************************/
#ifndef EPS_PRV_COMMANDS
#define EPS_PRV_COMMANDS(p1, p2, p3)
#endif

/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/
 
    /*** API Initialization States                                                      */
    /*** -------------------------------------------------------------------------------*/
#define EPS_STATUS_NOT_INITIALIZED      0
#define EPS_STATUS_INITIALIZED          1
#define EPS_STATUS_ESTABLISHED          2
#define EPS_STATUS_PROCCESSING          3

	/*** Definitions for pm string data                                                 */
    /*** -------------------------------------------------------------------------------*/
#define EPS_PM_MAXSIZE					1024
#define EPS_PM_BASESIZE					512
#define EPS_PM_HEADER_LEN               9
#define EPS_PM2_HEADER_LEN              10
#define EPS_PM_TERMINATOR_LEN           2
#define EPS_PM_DATA_LEN               (EPS_PM_MAXSIZE - EPS_PM_HEADER_LEN)

    /*** Definitions for device id string data                                          */
    /*** -------------------------------------------------------------------------------*/
#define EPS_DI_MAXSIZE                512

	/*** reset command states                                                           */
    /*** -------------------------------------------------------------------------------*/
#define EPS_RESET_NOT_SENT              0
#define EPS_RESET_SENT                  1

    /*** I/O port states                                                                */
    /*** -------------------------------------------------------------------------------*/
#define EPS_IO_NOT_OPEN                 0
#define EPS_IO_OPEN                     1

    /*** pm Date States                                                                 */
    /*** -------------------------------------------------------------------------------*/
#define EPS_PM_STATE_NOT_FILTERED       0
#define EPS_PM_STATE_FILTERED           1


    /*** CBT Channel                                                                    */
    /*** -------------------------------------------------------------------------------*/
#define EPS_CBTCHANNEL_CTRL           0x01           /* Control Channel                  */
#define EPS_CBTCHANNEL_DATA           0x00           /* Data Channel                     */

    /*** CBT Mechanical Commands                                                        */
    /*** -------------------------------------------------------------------------------*/
#define EPS_CBTCOM_CH                    0
#define EPS_CBTCOM_NC                    1
#define EPS_CBTCOM_EI                    2
#define EPS_CBTCOM_CSA                   3
#define EPS_CBTCOM_CSB                   4
#define EPS_CBTCOM_CX                    5
#define EPS_CBTCOM_XIA                   6
#define EPS_CBTCOM_XIB                   7
#define EPS_CBTCOM_PE                    8
#define EPS_CBTCOM_PJ                    9
#define EPS_CBTCOM_ST                   10
#define EPS_CBTCOM_XIA2                 12
#define EPS_CBTCOM_XIA3                 13
#define EPS_CBTCOM_PM                   14
#define EPS_CBTCOM_DI                   15
#define EPS_CBTCOM_PM2                  16
#define EPS_CBTCOM_RJ                   17
#define EPS_CBTCOM_CD                   18
#define EPS_CBTCOM_PM3                  19
#define EPS_CBTCOM_VI5                  20

    /*** CBT Error Code                                                                 */
    /*** -------------------------------------------------------------------------------*/
#define EPCBT_ERR_NONE                  0       /* Normal End                           */

#define EPSCBT_ERR_INITDENIED           -1       /* Printer Result=0x01                 */
#define EPSCBT_ERR_VERSION              -2       /* Printer Result=0x02                 */
#define EPSCBT_ERR_CLOSEDENIED          -3       /* Printer Result=0x03                 */
#define EPSCBT_ERR_RESOURCE             -4       /* Printer Result=0x04                 */
#define EPSCBT_ERR_OPENCHANNEL          -5       /* Printer Result=0x05                 */
#define EPSCBT_ERR_CHOPENDED            -6       /* Printer Result=0x06                 */
#define EPSCBT_ERR_CREDITOVF            -7       /* Printer Result=0x07                 */
#define EPSCBT_ERR_CMDDENIED            -8       /* Printer Result=0x08                 */
#define EPSCBT_ERR_CHNOTSUPPORT         -9       /* Printer Result=0x09                 */
#define EPSCBT_ERR_NOSERVICE           -10       /* Printer Result=0x0A                 */
#define EPSCBT_ERR_INITFAILED          -11       /* Printer Result=0x0B                 */
#define EPSCBT_ERR_PACKETSIZE          -12       /* Printer Result=0x0C                 */
#define EPSCBT_ERR_NULLPACKETSZ        -13       /* Printer Result=0x0D                 */

#define EPSCBT_ERR_PARAM               -20       /* parameter error                     */
#define EPSCBT_ERR_NOTOPEN             -22       /* Port Not open                       */
#define EPSCBT_ERR_2NDOPEN             -23       /* Port open second                    */
#define EPSCBT_ERR_CHNOTOPEN           -24       /* Channel not open or close           */
#define EPSCBT_ERR_CH2NDOPEN           -25       /* Channel open second                 */
#define EPSCBT_ERR_RPLYPSIZE           -28       /* OpenChannel Paket Size Fail         */
#define EPSCBT_ERR_WRITEERROR          -31       /* Write Error                         */
#define EPSCBT_ERR_READERROR           -32       /* Read Error                          */
#define EPSCBT_ERR_FNCDISABLE          -33       /* Not send or receive                 */

#define EPSCBT_ERR_FATAL               -50       /* FATAL Error                         */
#define EPSCBT_ERR_CBTDISABLE          -52       /* fail EpsonPackingCommand            */
#define EPSCBT_ERR_NOREPLY             -55       /* no Reply                            */

#define EPSCBT_ERR_MULFORMEDPACKET     -80       /* packet data fail                    */
#define EPSCBT_ERR_UEREPLY             -82       /* miss match reply data               */
#define EPSCBT_ERR_UERESULT            -85       /* Not Result value                    */

    /*** Service Pack Error                                                             */
    /*** -------------------------------------------------------------------------------*/
#define EPS_ERR_SP_INVALID_POINTER      -200
#define EPS_ERR_SP_INVALID_HEADER       -201
#define EPS_ERR_SP_INVALID_TERMINATOR   -202
#define EPS_ERR_SP_NO_VALID_FIELD       -203

    /*** Misc                                                                           */
    /*** -------------------------------------------------------------------------------*/
#define MERGED_FIELD                 0xFF

    /*** Save process for continue                                                      */
    /*** -------------------------------------------------------------------------------*/
#define EPS_SAVEP_NONE		            0
#define EPS_SAVEP_START_PAGE            1
#define EPS_SAVEP_END_PAGE				2
#define EPS_SAVEP_LEFTOVERS				3


/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/
    /*** Printer Status                                                                 */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_STATUS_INFO_ {
    EPS_INT32   nState;
    EPS_INT32   nError;
    EPS_INT32   nWarn;
    EPS_INT32   nCancel;
    EPS_INT32   nPrepare;
    EPS_INT32   nInkNo;
    EPS_INT32   nInkError;
    EPS_INT32   nColor[EPS_INK_NUM];
    EPS_INT32   nColorType[EPS_INK_NUM];
    EPS_INT32   nColorStatus[EPS_INK_NUM];
} EPS_STATUS_INFO;

    /*** Basic Rect Object                                                              */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_RECT_ {
    EPS_INT32   top;
    EPS_INT32   left;
    EPS_INT32   bottom;
    EPS_INT32   right;
} EPS_RECT;

typedef struct _tagEPS_LINE_SEGMENT_ {
    EPS_INT32   start;
    EPS_INT32   length;
} EPS_LINE_SEGMENT;

    /*** BMP Band Data                                                                  */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_BANDBMP_ {
    EPS_UINT32   widthBytes;             /* number of bytes in each scan line            */
    const EPS_UINT8*  bits;              /* pointer to the the bit values for the bitmap */
} EPS_BANDBMP;

    /*** Base Point                                                                     */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_BASEPOINT_ {
    EPS_RECT    border;
    EPS_RECT    borderless;
    EPS_RECT    pad;
} EPS_BASEPT;

    /*** "pm" Command Reply Data                                                        */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_PM_DATA_ {
    EPS_INT32   version;
    EPS_INT32   length;
    EPS_UINT8*  pmString;
} EPS_PM_DATA;


    /*** Printer Infomation                                                             */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_PRINTER_INN_ {
	EPS_INT32       protocol;				    /* Communication protocol               */
	EPS_INT32	    supportFunc;
	EPS_INT8        manufacturerName[EPS_NAME_BUFFSIZE];
	EPS_INT8        modelName[EPS_NAME_BUFFSIZE];         
	EPS_INT8		friendlyName[EPS_NAME_BUFFSIZE];
	EPS_INT8        location[EPS_ADDR_BUFFSIZE];
	EPS_INT8		serialNo[EPS_ADDR_BUFFSIZE];
	EPS_INT8		printerID[EPS_PRNID_BUFFSIZE];
	/* unused EPS_BOOL		needUpgrade; */
	EPS_HANDLE		protocolInfo;
	EPS_UINT16	    printPort;

	EPS_UINT32		language;				/* print language */
	EPS_UINT32		egID;					/* error group id */
	EPS_INT8        macAddress[EPS_ADDR_BUFFSIZE];

	/*** Supported Media                                                                */
    /*** -------------------------------------------------------------------------------*/
    EPS_SUPPORTED_MEDIA supportedMedia;
    EPS_INT32       JpgMax;
	EPS_PRINT_AREA_INFO	printAreaInfo;

    /*** Original PM reply                                                              */
    /*** -------------------------------------------------------------------------------*/
    EPS_PM_DATA pmData;

	EPS_UINT8        feedDir;
} EPS_PRINTER_INN;


    /*** Printer List                                                                   */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_PL_NODE_ {
	EPS_PRINTER_INN*			printer;
	/* unused EPS_BOOL					notified; */
	struct _tagEPS_PL_NODE_* 	next;
}EPS_PL_NODE;

typedef struct _tagEPS_PRINTER_LIST_ {
	EPS_INT32               num;            /* printer count of espFindPrinter result   */
	EPS_PL_NODE*            root;	        /* printer list of espFindPrinter result    */
}EPS_PRINTER_LIST;

/*---------------------------  Internal Print Job Structure   --------------------------*/
/*******************************************|********************************************/
typedef struct _tagEPS_PRINT_JOB_ {

    /*** Job / Page Status State                                                        */
    /*** -------------------------------------------------------------------------------*/
    EPS_INT32   jobStatus;                  /*  Job status                              */
    EPS_INT32   pageStatus;                 /*  Page status                             */
    EPS_INT32   findStatus;                 /*  Find status                             */

    /*** Communication Mode                                                             */
    /*** -------------------------------------------------------------------------------*/
    EPS_INT32   commMode;                   /* Computer byte storage order              */

    /*** Platform Specifics                                                             */
    /*** -------------------------------------------------------------------------------*/
#ifdef GCOMSW_EPSON_SLEEP
    EPS_INT32   sleepSteps;                 /* Sleep loop value                         */
#endif

	/*** Page Attributes                                                                */
    /*** -------------------------------------------------------------------------------*/
    EPS_JOB_ATTRIB attr;

	/*** Run-Time Job Printing Parameters                                               */
    /*** -------------------------------------------------------------------------------*/
    EPS_INT32   verticalOffset;
    EPS_UINT32  jpegSize;
    
    /*** Borderless Printing Parameters                                                 */
    /*** -------------------------------------------------------------------------------*/
    EPS_INT32   borderlessMode;             /* Borderless mode selected                 */
    /* unused EPS_INT32   topCnt;              Counter - many top    rasters saved      */
    /* unused EPS_INT32   botCnt;              Counter - many bottom rasters saved      */
    /* unused EPS_BOOL    topFlushed;          Flag to determine if top to be flushed   */
    /* unused EPS_BASEPT  basePt;                                                       */

    EPS_RECT    border;                     /* Border     Base Point                    */
    EPS_RECT    borderless;                 /* Borderless Base Point                    */
    EPS_RECT    pad;                        /* Pad        Base Point                    */

	/*** Media                                                                          */
    /*** -------------------------------------------------------------------------------*/
    EPS_INT32   paperWidth;                 /* Width of Media                           */
    EPS_INT32   paperHeight;                /* Height of Media                          */

    /*** Margins - based on Media and border Mode                                       */
    /*** -------------------------------------------------------------------------------*/
    EPS_INT32   topMargin;                  /* Media/Border-Mode Top    Margin          */
    EPS_INT32   leftMargin;                 /* Media/Border-Mode Left   Margin          */
    /*EPS_INT32   bottomMargin;                Media/Border-Mode Bottom Margin          */
    /*EPS_INT32   rightMargin;                 Media/Border-Mode Right  Margin          */

    /*** Printable                                                                      */
    /*** -------------------------------------------------------------------------------*/
    EPS_UINT32  printableAreaWidth;         /* users setting                            */
    EPS_UINT32  printableAreaHeight;        /* users setting                            */
    EPS_UINT8   bpp;                        /* 3 for RGB, 1 for 256                     */
    EPS_INT16   offset_x;                   /* offset to be used. calculated when       */
    EPS_INT16   offset_y;                   /* changing basepoint                       */

    /*** Print Band Information                                                         */
    /*** -------------------------------------------------------------------------------*/
    /* unused const EPS_UINT8*  printBandData;        Print Band Data                   */
    /* unused EPS_UINT32  printBandWidth;             Print Band Width in Pixels        */
    EPS_UINT32  printBandHeight;            /* Print Band Height in image lines         */
    /* unused EPS_UINT32  printBandBPL;               Print Band Bytes Per Line         */
    /* unused EPS_RECT    printBandRect;              Rect for print band               */

    /*** Job Trarget Printer                                                            */
    /*** -------------------------------------------------------------------------------*/
	EPS_PRINTER_INN* printer;

    /*** Protocol depend info                                                           */
    /*** -------------------------------------------------------------------------------*/
	EPS_HANDLE      hProtInfo;

    /*** Reset State Flag                                                               */
    /*** -------------------------------------------------------------------------------*/
	EPS_INT32       resetSent;	/* sent */
	EPS_BOOL		resetReq;	/* reqest from printer */
	EPS_BOOL		sendJS;

	/*** Communication                                                                  */
    /*** -------------------------------------------------------------------------------*/
	EPS_BOOL		bComm;
	EPS_BOOL        transmittable;			/* possible to transmit */
	EPS_BOOL		bJpgLimit;

	/*** Misc                                                                           */
    /*** -------------------------------------------------------------------------------*/
    EPS_UINT8		whiteColorValue;        /* Define white value or index              */
	EPS_BOOL		needBand;
	EPS_UINT8		platform;
	EPS_INT32		pageCount;

	struct {								/* Leftovers */
		EPS_INT32   lastError;
		EPS_INT32   savePoint;
		EPS_INT32   saveStep;
		const EPS_UINT8*	sendData;		/* buffer */
		EPS_UINT32	sendDataSize;			/* Data size */
		EPS_UINT32	jpgSize;				/* Jpeg Leftovers size */
		EPS_BOOL    nextPage;
		EPS_INT32   epRetry;
		EPS_BOOL	skipLine;
	} contData;

}EPS_PRINT_JOB;

/*--------------------------  Public Function Declarations   ---------------------------*/
/*******************************************|********************************************/
EPS_ERR_CODE SendCommand	(const EPS_UINT8 *Buffer, EPS_UINT32, EPS_UINT32*, EPS_BOOL );


#ifdef __cplusplus
}
#endif

#endif    /* def __EPSON_ESCPR_PVT_H__ */

/*________________________________  epson-escpr-pvt.h  _________________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

