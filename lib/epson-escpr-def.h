/*________________________________  epson-escpr-def.h   ________________________________*/

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
/*                           Epson ESC/PR External Definitions                          */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef __EPSON_ESCPR_DEF_H__
#define __EPSON_ESCPR_DEF_H__
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"

/*------------------------------  Local Compiler Switch  -------------------------------*/
/*******************************************|********************************************/
/* Network Intarface bind option. following items are only either one in effect. */
#define LCOMSW_BINDIF_IF_NEED		0	/* 1: bind interface when send find packet error*/
#define LCOMSW_USE_MULTI_IF			0	/* 1: bind multi interface                      */

/*------------------------------------- Data Types -------------------------------------*/
/*******************************************|********************************************/

/*----------------------------------  Generic Macros   ---------------------------------*/
/*******************************************|********************************************/

/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/

#define EPS_IFNAME_LEN	(16)

    /*** Maximum Ink Cartridge Number                                                   */
    /*** -------------------------------------------------------------------------------*/
#define EPS_INK_NUM                 20

    /*** Maximum PaperSource Number                                                     */
    /*** -------------------------------------------------------------------------------*/
#define EPS_PAPERSOURCE_NUM         20

    /*** Buffer size for manufacter name and model name                                 */
    /*** -------------------------------------------------------------------------------*/
#define EPS_NAME_BUFFSIZE           64
#define EPS_PRNID_BUFFSIZE          128
#define EPS_ADDR_BUFFSIZE           32

    /*** Communication Mode                                                             */
    /*** -------------------------------------------------------------------------------*/
#define EPS_COMM_UNID			0x00000001  /* Uni-Directional Communication            */
#define EPS_COMM_BID			0x00000002  /* Bi-Directional Communication             */

	/*** print protocol                                                                 */
    /*** -------------------------------------------------------------------------------*/
#define EPS_PROTOCOL_INVALID	0x00000000	/* Invalid                                  */
#define EPS_PROTOCOL_USB		0x00000010  /* USB                                      */
/*#define EPS_PROTOCOL_UPNP		0x00000020     UPnP                                     */
#define EPS_PROTOCOL_LPR		0x00000040  /* LPR                                      */
#define EPS_PROTOCOL_RAW		0x00000080  /* RAW Port                                 */
#define EPS_PROTOCOL_NET        (EPS_PROTOCOL_LPR | EPS_PROTOCOL_RAW)	/* All Network  */
#define EPS_PROTOCOL_ALL        (EPS_PROTOCOL_USB | EPS_PROTOCOL_NET)	/* All          */

#define EPS_COMM_USB_UNID   (EPS_COMM_UNID | EPS_PROTOCOL_USB )  /* USB Uni-Directional Communication  */
#define EPS_COMM_USB_BID    (EPS_COMM_BID  | EPS_PROTOCOL_USB )  /* USB Bi-Directional Communication   */
#define EPS_COMM_LPR_UNID   (EPS_COMM_UNID | EPS_PROTOCOL_LPR )  /* LPR Uni-Directional Communication  */
#define EPS_COMM_LPR_BID    (EPS_COMM_BID  | EPS_PROTOCOL_LPR )  /* LPR Bi-Directional Communication   */
#define EPS_COMM_RAW_UNID   (EPS_COMM_UNID | EPS_PROTOCOL_RAW )  /* RAW Uni-Directional Communication  */
#define EPS_COMM_RAW_BID    (EPS_COMM_BID  | EPS_PROTOCOL_RAW )  /* RAW Bi-Directional Communication   */
#define EPS_COMM_NET_UNID   (EPS_COMM_UNID | EPS_PROTOCOL_NET )  /* Net Uni-Directional Communication  */
#define EPS_COMM_NET_BID    (EPS_COMM_BID  | EPS_PROTOCOL_NET )  /* Net Bi-Directional Communication   */
#define EPS_COMM_ALL_UNID   (EPS_COMM_USB_UNID | EPS_COMM_NET_UNID)  /* USB & Net Uni-Directional Communication */
#define EPS_COMM_ALL_BID    (EPS_COMM_USB_BID  | EPS_COMM_NET_BID)  /* USB & Net Bi-Directional Communication */

	/*** sub protocol                                                                   */
    /*** -------------------------------------------------------------------------------*/
#define EPS_SUBPROTOCOL_PCDEV	0x00001000	/* UPnP PC device                           */

	  /* mask                                                                           */
#define EPS_PRT_MASK_DIRECTION		0x0000000F
#define EPS_PRT_MASK_PROTOCOL		0x00000FF0
#define EPS_PRT_MASK_SUBPROTOCOL	0x0000F000

#define EPS_PRT_DIRECTION(p)		( (p) & EPS_PRT_MASK_DIRECTION )
#define EPS_PRT_PROTOCOL(p)			( (p) & EPS_PRT_MASK_PROTOCOL )
#define EPS_PRT_SUBPROTOCOL(p)		( (p) & EPS_PRT_MASK_SUBPROTOCOL )
#define EPS_PRT_PROTOCOL_EX(p)		( (p) & (EPS_PRT_MASK_PROTOCOL | EPS_PRT_MASK_SUBPROTOCOL) )
#define EPS_IS_BI_PROTOCOL(p)       ( ( (p) & EPS_COMM_BID) == EPS_COMM_BID )


	/*** print language                                                                  */
    /*** -------------------------------------------------------------------------------*/
#define EPS_LANG_UNKNOWN			(0)			
#define EPS_LANG_ESCPR				(1)			/* ESC/P-R */
#define EPS_LANG_ESCPAGE			(2)			/* ESC/Page(Mono) */
#define EPS_LANG_ESCPAGE_COLOR		(3)			/* ESC/Page-Color */

	/*** probe method                                                                   */
    /*** -------------------------------------------------------------------------------*/
#define EPS_PRB_BYID               1		/* probe by ID string                       */
#define EPS_PRB_BYADDR             2		/* probe by IP Address                      */


	/*** Supported functions                                                            */
    /*** -------------------------------------------------------------------------------*/
#define EPS_SPF_RGBPRINT        0x0001      /* bitmap print.                            */
#define EPS_SPF_JPGPRINT        0x0002      /* jpeg print.                              */
/*#define EPS_SPF_PAPERSOURCE     0x0004       Specify the Paper source. epsStatPage() */
               

    /*** Print Page Attribute Values                                                    */
    /*** -------------------------------------------------------------------------------*/
        /*** Media Size                                                                 */
        /*** ---------------------------------------------------------------------------*/
/*** See EPS_MSID_* in epson-escpr-media.h                                              */

        /*** Media Type                                                                 */
        /*** ---------------------------------------------------------------------------*/
/*** See EPS_MTID_* in epson-escpr-media.h                                              */

        /*** Border Mode                                                                */
        /*** ---------------------------------------------------------------------------*/
/*** See EPS_MTID_* in epson-escpr-media.h                                              */

        /*** Print Quality                                                              */
        /*** ---------------------------------------------------------------------------*/
/*** See EPS_MTID_* in epson-escpr-media.h                                              */

        /*** Color Mode                                                                 */
        /*** ---------------------------------------------------------------------------*/
#define EPS_CM_COLOR                0       /* Color                                    */
#define EPS_CM_MONOCHROME           1       /* Monochrome                               */
#define EPS_CM_SEPIA                2       /* Sepia                                    */


        /*** In (input image) Resolution                                                */
        /*** ---------------------------------------------------------------------------*/
#define EPS_IR_360X360              (1 << 0)	/* 360 x 360 dpi                        */
#define EPS_IR_720X720              (1 << 1) 	/* 720 x 720 dpi				        */
#define EPS_IR_150X150              (1 << 2)    /* 150 x 150 dpi                        */
#define EPS_IR_300X300              (1 << 3)    /* 300 x 300 dpi                        */
#define EPS_IR_600X600              (1 << 4)	/* 600 x 600 dpi                        */

        /*** Print Direction                                                            */
        /*** ---------------------------------------------------------------------------*/
#define EPS_PD_BIDIREC              0       /* Bi-direction                             */
#define EPS_PD_UNIDIREC             1       /* Uni-direction                            */

        /*** Color Plane                                                                */
        /*** ---------------------------------------------------------------------------*/
#define EPS_CP_FULLCOLOR            0       /* Full Color                               */
#define EPS_CP_256COLOR             1       /* 256 Color                                */
#define EPS_CP_JPEG                 2       /* Jpeg                                     */
#define EPS_CP_PRINTCMD             3       /* Print command                            */

        /*** Command data type                                                          */
        /*** ---------------------------------------------------------------------------*/
#define EPS_MNT_UNKNOWN              0       /* invalid type                            */
#define EPS_MNT_CUSTOM               1       /* Cusutom data                            */
#define EPS_MNT_CLEANING             2       /* Cleaning command data                   */
#define EPS_MNT_NOZZLE               3       /* NozzleCheck command data                */

		/*** Palette Size                                                               */
        /*** ---------------------------------------------------------------------------*/
/*** Specified in Developer code                                                        */

        /*** Palette Data                                                               */
        /*** ---------------------------------------------------------------------------*/
/*** Specified in Developer code                                                        */

        /*** Brightness (range: -50 <= brightness <= 50)                                */
        /*** ---------------------------------------------------------------------------*/
/*** Specified in Developer code                                                        */

        /*** Contrast   (range: -50 <= contrast   <= 50)                                */
        /*** ---------------------------------------------------------------------------*/
/*** Specified in Developer code                                                        */

        /*** Saturation (range: -50 <= saturation <= 50)                                */
        /*** ---------------------------------------------------------------------------*/
/*** Specified in Developer code                                                        */

        /*** Top Margin                                                                 */
        /*** ---------------------------------------------------------------------------*/
/*** Specified in Developer code                                                        */

        /*** Left Margin                                                                */
        /*** ---------------------------------------------------------------------------*/
/*** Specified in Developer code                                                        */

        /*** Bottom Margin                                                              */
        /*** ---------------------------------------------------------------------------*/
/*** Specified in Developer code                                                        */

        /*** Right Margin                                                               */
        /*** ---------------------------------------------------------------------------*/
/*** Specified in Developer code                                                        */

        /*** CD/DVD Inside Dimameter                                                    */
        /*** ---------------------------------------------------------------------------*/
#define EPS_CDDIM_IN_MIN			18
#define EPS_CDDIM_IN_MAX			46
#define EPS_CDDIM_IN_DEF			43

        /*** CD/DVD Outside Dimameter                                                   */
        /*** ---------------------------------------------------------------------------*/
#define EPS_CDDIM_OUT_MIN			114
#define EPS_CDDIM_OUT_MAX			120
#define EPS_CDDIM_OUT_DEF			116

		/*** APF Automatic Correction                                                   */
        /*** ---------------------------------------------------------------------------*/
#define EPS_APF_ACT_NOTHING         0       /* Nothing                                  */
#define EPS_APF_ACT_STANDARD        1       /* Standard (use Exif)                      */
#define EPS_APF_ACT_PIM             2       /* use PIM                                  */
#define EPS_APF_ACT_PORTRATE        3       /* Portrate                                 */
#define EPS_APF_ACT_VIEW	        4       /* View										*/
#define EPS_APF_ACT_NIGHTVIEW       5       /* Night View                               */

        /*** APF Redeye                                                                 */
        /*** ---------------------------------------------------------------------------*/
#define EPS_APF_RDE_NOTHING         0       /* Nothing                                  */
#define EPS_APF_RDE_CORRECT         1       /* MoreSoft                                 */

        /*** max Jpeg data size                                                         */
        /*** ---------------------------------------------------------------------------*/
#define EPS_JPEG_SIZE_MAX           (1024*1024*3)   /* 3MB                              */
#define EPS_JPEG_PIXEL_MAX          (9600)          
#define EPS_JPEG_CHUNK_SIZE_MAX		(1024*64-1)		/* 0xFFFF max short */
#define EPS_JPEG_SIZE_UNLIMIT       ~(1<<(sizeof(EPS_INT32)*8-1))   /* 2GB              */


    /*** Layout template value                                                          */
    /*** -------------------------------------------------------------------------------*/
#define EPS_POS_HLEFT			(-1)
#define EPS_POS_HRIGHT			(-2)
#define EPS_POS_HCENTER			(-3)
#define EPS_POS_VTOP			(-1)
#define EPS_POS_VBOTTOM			(-2)
#define EPS_POS_VMIDDLE			(-3)


	/*** duplex                                                                         */
    /*** -------------------------------------------------------------------------------*/
#define EPS_DUPLEX_NONE			(0)
#define EPS_DUPLEX_LONG			(1 << 0)
#define EPS_DUPLEX_SHORT		(1 << 1)


	/*** feed direction                                                                 */
    /*** -------------------------------------------------------------------------------*/
#define EPS_FEEDDIR_PORTRAIT	(0)
#define EPS_FEEDDIR_LANDSCAPE	(1)


    /*** Ink Color                                                                      */
    /*** -------------------------------------------------------------------------------*/
enum EPS_COLOR {
    EPS_COLOR_BLACK,
    EPS_COLOR_CYAN,
    EPS_COLOR_MAGENTA,
    EPS_COLOR_YELLOW,
    EPS_COLOR_LIGHTCYAN,
    EPS_COLOR_LIGHTMAGENTA,
    EPS_COLOR_LIGHTYELLOW,
    EPS_COLOR_DARKYELLOW,
    EPS_COLOR_LIGHTBLACK,
    EPS_COLOR_RED,
    EPS_COLOR_VIOLET,
    EPS_COLOR_MATTEBLACK,
	EPS_COLOR_LIGHTLIGHTBLACK,
    EPS_COLOR_PHOTOBLACK,
    EPS_COLOR_CLEAR,
    EPS_COLOR_GRAY,
    EPS_COLOR_UNKNOWN,

	/* add ver5.0*/
    EPS_COLOR_BLACK2,
	EPS_COLOR_ORANGE,
	EPS_COLOR_GREEN,
	EPS_COLOR_WHITE,
	EPS_COLOR_CLEAN,

	/* add ver5.4*/
	EPS_COLOR_COMPOSITE,
};

    /*** Power info                                                                     */
    /*** -------------------------------------------------------------------------------*/
enum EPS_POWER_SOUECE {
    EPS_POWER_SOUECE_NOT_SUPPORTED = -1,
    EPS_POWER_SOUECE_UNKNOWN       = 0,
    EPS_POWER_SOUECE_AC,
    EPS_POWER_SOUECE_BATTERY,
};

enum EPS_CHARGE_STATE {
    EPS_CHARGE_STATE_NONE,
    EPS_CHARGE_STATE_CHARGING,
};


/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/

    /*** Basic Rect Object                                                              */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_POS_ {
    EPS_INT32   x;
    EPS_INT32   y;
} EPS_POS;

    /*** Printer Infomation                                                             */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_PRINTER_ {
	EPS_UINT32			supportFunc;
	EPS_INT8            manufacturerName[EPS_NAME_BUFFSIZE];
	EPS_INT8            modelName[EPS_NAME_BUFFSIZE];         
	EPS_INT8			friendlyName[EPS_NAME_BUFFSIZE];
	EPS_INT8            location[EPS_ADDR_BUFFSIZE];
	EPS_INT8		    printerID[EPS_PRNID_BUFFSIZE];
	/* ver 4.2 */
	EPS_INT32			protocol;			    /* Communication protocol               */
	/* ver 5.0 */
	EPS_UINT32			language;				/* print language */
	EPS_INT8            macAddress[EPS_ADDR_BUFFSIZE];
	/* ver 5.2 */
	EPS_INT8            serialNo[EPS_ADDR_BUFFSIZE];
} EPS_PRINTER;

	
	/*** Job Attributes                                                                 */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_JOB_ATTRIB_ {
	EPS_UINT16			version;			/* structure version                        */

	/*** ------------------------------------------------------------------------------ */
	/*** Source Image Attribute                                                         */
    EPS_UINT8           colorPlane;         /* Image color plane                        */
    EPS_UINT16          paletteSize;        /* Palette Size (if present)                */
    EPS_UINT8*          paletteData;        /* Palette Data (if present)                */
    EPS_UINT8           inputResolution;    /* Input Image Resolution                   */

	/*** ------------------------------------------------------------------------------ */
	/*** Output Attribute                                                               */
    EPS_INT32           mediaSizeIdx;       /* Media Size Index (EPS_MSID_*)            */
                                            /* (see epson-escpr-media.h)                */
    EPS_INT32           mediaTypeIdx;       /* Media Type Index (EPS_MTID_*)            */
                                            /* (see epson-escpr-media.h)                */

	EPS_UINT32          printLayout;        /* Print Layout                             */
    EPS_UINT32          printQuality;       /* Print Quality                            */
    EPS_UINT32          paperSource;        /* Paper Source (Lv1 ext)                   */
    EPS_UINT8           printDirection;     /* Printing Direction                       */

    EPS_UINT8           colorMode;          /* Color Mode                               */
    EPS_INT8            brightness;         /* Print Brightness                         */
    EPS_INT8            contrast;           /* Print Contrast                           */
    EPS_INT8            saturation;         /* Print Saturation                         */
    EPS_UINT8           apfAutoCorrect;     /* APF Automatic  (Jpeg only)               */
    EPS_INT8            sharpness;          /* sharpness      (Jpeg only)               */
    EPS_UINT8           redeye;             /* redeye correct (Jpeg only)               */

	/*** Custom border ***/
    EPS_INT16           topMargin;          /* Desired Top    print margin              */
    EPS_INT16           leftMargin;         /* Desired Left   print margin              */
    EPS_INT16           bottomMargin;       /* Desired Bottom print margin              */
    EPS_INT16           rightMargin;        /* Desired Right  print margin              */

	/*** CD/DVD Label ***/
	EPS_UINT8           cdDimIn;            /* CD Diameter Inside                       */
    EPS_UINT8           cdDimOut;           /* CD Diameter Outside                      */

	/* -------------------------------------------------------------------------------- */
	/* Ver 2                                                                            */
	EPS_INT32           cmdType;            /* Print command type                       */

	/* -------------------------------------------------------------------------------- */
	/* Ver 3                                                                            */
	EPS_INT32			duplex;             /* Duplex print                             */
	EPS_INT32           copies;             /* copies count                             */
	EPS_INT32           feedDirection;      /* paper feed direction                     */

	/* -------------------------------------------------------------------------------- */
	/* Ver 4                                                                            */
	EPS_UINT32			userDefWidth;       /* user defined width                       */
	EPS_UINT32			userDefHeight;      /* user defined height                      */
	EPS_INT32			pageNum;            /* amount of pages                          */
} EPS_JOB_ATTRIB;

#define EPS_JOB_ATTRIB_VER_1		1
#define EPS_JOB_ATTRIB_VER_2		2
#define EPS_JOB_ATTRIB_VER_3		3
#define EPS_JOB_ATTRIB_VER_4		4
#define EPS_JOB_ATTRIB_VER_CUR	EPS_JOB_ATTRIB_VER_4


	/*** Page Attributes                                                                */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_PAGE_ATTRIB_ {
	EPS_UINT16				version;		/* structure version                        */
} EPS_PAGE_ATTRIB;
#define EPS_PAGE_ATTRIB_VERS_0	0
#define EPS_PAGE_ATTRIB_VER_CUR	EPS_PAGE_ATTRIB_VER_0


	/*** Print Data                                                                     */
    /*** -------------------------------------------------------------------------------*/
typedef union _tagEPS_PRN_DATA_ {
	EPS_UINT16				version;		/* structure version                        */

	struct {
		EPS_UINT16			version;
		const EPS_UINT8*	data;
		EPS_UINT32			heightPixels;
		EPS_UINT32			widthPixels;
	} band;

	struct {
		EPS_UINT16			version;
		const EPS_UINT8*	data;
		EPS_UINT32			dataSize;
	} chunk;
} EPS_PRN_DATA;
#define EPS_PRN_DATA_VER_1	1
#define EPS_PRN_DATA_VER_CUR	EPS_PRN_DATA_VER_1


    /*** Printer Status                                                                 */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_STATUS_ {
    EPS_INT32           printerStatus;
    EPS_INT32           errorCode;
	EPS_BOOL			jobContinue;
} EPS_STATUS;

    /*** Ink infomation                                                                 */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_INK_INFO_ {
    EPS_INT32           number;
    EPS_INT32           colors[EPS_INK_NUM];
    EPS_INT32           remaining[EPS_INK_NUM];
    EPS_INT32           status[EPS_INK_NUM];
} EPS_INK_INFO;

    /*** Paper setting infomation                                                       */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_PAPERSOURCE_INFO_ {
    EPS_INT32			number;
    EPS_UINT32          id[EPS_PAPERSOURCE_NUM];
    EPS_INT32           mediaSizeID[EPS_PAPERSOURCE_NUM];
    EPS_INT32           mediaTypeID[EPS_PAPERSOURCE_NUM];
} EPS_PAPERSOURCE_INFO;

    /*** Power source infomation                                                        */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_POWERSOURCE_INFO_ {
    EPS_UINT32			type;
    EPS_UINT32          chargeState;
    EPS_INT32           remaining;
} EPS_POWERSOURCE_INFO;

    /*** supplies infomation                                                            */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_SUPPLY_INFO_ {
	EPS_INK_INFO			ink;
	EPS_PAPERSOURCE_INFO	paperSource;
    EPS_POWERSOURCE_INFO	powerSource;
} EPS_SUPPLY_INFO;

	/*** Supported Media Information                                                    */
    /*** -------------------------------------------------------------------------------*/
        /* Media Type and Printing Modes                                                */
        /* (Container for media type information (nested inside EPS_MEDIA_SIZE) )       */
typedef struct _tagEPS_MEDIA_TYPE_V1_ {
    EPS_INT32           mediaTypeID;
    EPS_UINT32          layout;
    EPS_UINT32          quality;
    EPS_UINT32          paperSource;        /* Paper Source                             */
}EPS_MEDIA_TYPE_V1;

typedef struct _tagEPS_MEDIA_TYPE_V2_ {
    EPS_INT32           mediaTypeID;
    EPS_UINT32          layout;
    EPS_UINT32          quality;
    EPS_UINT32          paperSource;        /* Paper Source                             */
    EPS_UINT32          duplex;				/* duplex print support                     */
}EPS_MEDIA_TYPE_V2;
#define EPS_MEDIA_TYPE	EPS_MEDIA_TYPE_V2

        /* Media Size                                                                   */
        /* (Container for media size information (nested inside EPS_CAPABILITIES) )     */
typedef struct _tagEPS_MEDIA_SIZE_ {
    EPS_INT32           mediaSizeID;
    EPS_INT32           numTypes;
    EPS_MEDIA_TYPE*     typeList;
}EPS_MEDIA_SIZE;

        /* Supported Media                                                              */
typedef struct _tagEPS_SUPPORTED_MEDIA_ {
    EPS_INT32           JpegSizeLimit;
    EPS_INT32           numSizes;
    EPS_MEDIA_SIZE*     sizeList;
    EPS_UINT32          resolution;			/* supported input resolution */
}EPS_SUPPORTED_MEDIA;


	/*** Print Area Information                                                         */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_MARGIN_ {
    EPS_INT32   top;
    EPS_INT32   left;
    EPS_INT32   bottom;
    EPS_INT32   right;
} EPS_MARGIN;

typedef struct _tagEPS_LAYOUT_INFO_ {
    EPS_UINT32          layout;
    EPS_MARGIN          margin;
}EPS_LAYOUT_INFO;

typedef struct _tagEPS_LAYOUTSIZE_INFO_ {
    EPS_INT32           mediaSizeID;
    EPS_UINT32          paperWidth;
    EPS_UINT32          paperHeight;
    EPS_INT32           numLayouts;
    EPS_LAYOUT_INFO*    layoutList;
}EPS_LAYOUTSIZE_INFO;

typedef struct _tagEPS_PRINT_AREA_INFO_ {
    EPS_INT32				numSizes;
    EPS_LAYOUTSIZE_INFO*	sizeList;
}EPS_PRINT_AREA_INFO;


	/*** USB device Information                                                         */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_USB_DEVICE_ {
    EPS_UINT32          vid;		/* vender ID */
    EPS_UINT32          pid;		/* product ID */
    EPS_UINT32          port;		/* port number */
}EPS_USB_DEVICE;


    /*** Probe printer parameter                                                        */
    /*** -------------------------------------------------------------------------------*/
typedef struct tagEPS_PROBE_ {
    EPS_INT32   version;                    /* this version                             */
	EPS_INT32   method;
    EPS_UINT32  timeout;
	union{
		EPS_INT8	identify[EPS_PRNID_BUFFSIZE];
		struct {
			EPS_INT32   protocol;
			EPS_INT8	address[EPS_ADDR_BUFFSIZE];
		} addr;
	} dsc;
}EPS_PROBE;
#define EPS_PROBE_VER_1		1
#define EPS_PROBE_VER_CUR	EPS_PROBE_VER_1

    /* Local time                                                                       */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_LOCAL_TIME_ {
    EPS_UINT16          year;
    EPS_UINT8           mon;
    EPS_UINT8           day;
    EPS_UINT8           hour;
    EPS_UINT8           min;
    EPS_UINT8           sec;
}EPS_LOCAL_TIME;


/*--------------------------  External Function Definitions   --------------------------*/
/*******************************************|********************************************/

    /*** Common Function Definitions                                                    */
    /*** -------------------------------------------------------------------------------*/
typedef EPS_INT32   (*EPS_FindCallback  )(EPS_PRINTER                      );
typedef EPS_INT32   (*EPS_StateCallback )(EPS_INT32, EPS_INT32             );
typedef void*       (*EPS_MemAlloc      )(size_t                           );
typedef void        (*EPS_MemFree       )(void*                            );
    /* ESC/P-R Lib ignores sleep function return value, so any sleep function can be used. */
typedef EPS_INT32   (*EPS_Sleep         )(EPS_UINT32                       );
/*typedef void      (*EPS_Sleep         )(EPS_UINT32                       );*/
typedef EPS_UINT32	(*EPS_GetTime       )(void                             );
typedef EPS_UINT32	(*EPS_GetLocalTime  )(EPS_LOCAL_TIME*                  );
typedef EPS_INT32	(*EPS_LockSync      )(void                             );
typedef EPS_INT32	(*EPS_UnlockSync    )(void                             );


    /*** USB Function Definitions                                                       */
    /*** -------------------------------------------------------------------------------*/
typedef EPS_FILEDSC (*EPS_FindFirstPort )(EPS_USB_DEVICE*                               );
typedef EPS_BOOL    (*EPS_FindNextPort  )(EPS_FILEDSC, EPS_USB_DEVICE*                  );
typedef EPS_BOOL    (*EPS_FindClose     )(EPS_FILEDSC                                   );
typedef EPS_FILEDSC (*EPS_OpenPortal    )(const EPS_USB_DEVICE*                         );
typedef EPS_INT32   (*EPS_ClosePortal   )(EPS_FILEDSC                                   );
typedef EPS_INT32   (*EPS_ReadPortal    )(EPS_FILEDSC, EPS_UINT8*, EPS_INT32, EPS_INT32*);
typedef EPS_INT32   (*EPS_WritePortal   )(EPS_FILEDSC, const EPS_UINT8*, EPS_INT32, EPS_INT32*);
typedef EPS_INT32   (*EPS_GetDeviceID   )(EPS_FILEDSC, EPS_INT8*, EPS_INT32*            );
typedef EPS_INT32   (*EPS_SoftReset     )(EPS_FILEDSC                                   );


    /*** Sockets Function Definitions                                                   */
    /*** -------------------------------------------------------------------------------*/
typedef EPS_SOCKET  (*EPS_NetSocket            )(EPS_INT32, EPS_INT32, EPS_INT32        );
typedef EPS_INT32   (*EPS_NetClose             )(EPS_SOCKET                             );
typedef EPS_INT32	(*EPS_NetConnect	       )(EPS_SOCKET, const EPS_INT8*, EPS_UINT16);			
typedef EPS_INT32	(*EPS_NetShutdown          )(EPS_SOCKET, EPS_INT32                  );
typedef EPS_INT32	(*EPS_NetBind	           )(EPS_SOCKET, EPS_UINT16, EPS_UINT16*    );
typedef EPS_INT32	(*EPS_NetListen            )(EPS_SOCKET, EPS_INT32                  );
typedef EPS_INT32	(*EPS_NetAccept            )(EPS_SOCKET, EPS_UINT32, EPS_SOCKET*, 
                                                 EPS_INT8*, EPS_UINT16*                 );
typedef EPS_INT32	(*EPS_NetSend		       )(EPS_SOCKET, const void*, EPS_INT32, EPS_UINT32);
typedef EPS_INT32	(*EPS_NetSendTo	           )(EPS_SOCKET, const void*, EPS_INT32, 
                                                 const EPS_INT8*, EPS_UINT16, EPS_UINT32 );
typedef EPS_INT32	(*EPS_NetReceive	       )(EPS_SOCKET, void*, EPS_INT32, EPS_UINT32);
typedef EPS_INT32	(*EPS_NetReceiveFrom       )(EPS_SOCKET, void*, EPS_INT32,
						                         EPS_INT8*, EPS_UINT16*, EPS_UINT32     );
typedef EPS_INT32	(*EPS_NetGetSockName       )(EPS_SOCKET, EPS_INT8*, EPS_UINT16*     );
typedef EPS_INT32	(*EPS_NetSetMulticastTTL   )(EPS_SOCKET, EPS_INT32                  );
typedef EPS_INT32	(*EPS_NetSetBroadCast      )(EPS_SOCKET                             );
#if LCOMSW_USE_MULTI_IF
typedef EPS_INT32	(*EPS_NetEnumInterface     )(EPS_UINT8*, EPS_UINT32                 );
typedef EPS_INT32	(*EPS_NetBindInterface     )(EPS_SOCKET, const EPS_UINT8*           );
#elif LCOMSW_BINDIF_IF_NEED
typedef EPS_INT32	(*EPS_NetBindInterface     )(EPS_SOCKET                             );
#endif

    /*** External Function Definitions                                                  */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_CMN_FUNC_ {
    EPS_INT32			version;            /* this version                             */
    EPS_FindCallback    findCallback;       /* Find Notification Callback Function      */
    EPS_StateCallback   stateCallback;      /* unused */
    EPS_MemAlloc        memAlloc;           /* Allocate Memory Function                 */
    EPS_MemFree         memFree;            /* Free Memory Function                     */
    EPS_Sleep           sleep;              /* Sleep Function                           */
    EPS_GetTime         getTime;            /* GetTime Function                         */
	EPS_GetLocalTime    getLocalTime;       /* Get Local Time Function                  */
	EPS_LockSync		lockSync;	    	/* Thread lock Function                     */
	EPS_UnlockSync		unlockSync; 		/* Thread unlock Function                   */
} EPS_CMN_FUNC;

#define EPS_CMNFUNC_VER_1		1
#define EPS_CMNFUNC_VER_CUR		EPS_CMNFUNC_VER_1

    /*** USB External Function Definitions                                              */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_USB_FUNC_ {
    EPS_INT32			version;            /* this version                             */
    EPS_OpenPortal      openPortal;         /* Open I/O Function                        */
    EPS_ClosePortal     closePortal;        /* Close I/O Function                       */
    EPS_ReadPortal      readPortal;         /* Read Data Function                       */
    EPS_WritePortal     writePortal;        /* Write Data Function                      */
	EPS_FindFirstPort	findFirst;          /* Find usb device start Function           */
	EPS_FindNextPort	findNext;           /* Find next usb device Function            */
	EPS_FindClose		findClose;          /* Find process end Function                */
    EPS_GetDeviceID     getDeviceID;        /* Get Device ID Function                   */
	EPS_SoftReset       softReset;          /* USB soft reset Function                  */
} EPS_USB_FUNC;

#define EPS_USBFUNC_VER_1		1
#define EPS_USBFUNC_VER_CUR		EPS_USBFUNC_VER_1

    /*** NET External Function Definitions                                              */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_NET_FUNC_ {
    EPS_INT32				version;            /* this version                         */
	EPS_NetSocket           socket;             /* create socket Function               */
	EPS_NetClose            close;              /* close Function                       */
	EPS_NetConnect          connect;            /* connect Function                     */
	EPS_NetShutdown         shutdown;           /* shutdown Function                    */
	EPS_NetBind             bind;               /* bind Function                        */
	EPS_NetListen           listen;             /* listen Function                      */
	EPS_NetAccept           accept;             /* accept Function                      */
	EPS_NetSend             send;               /* send Function                        */
	EPS_NetSendTo           sendTo;             /* sendto Function                      */
	EPS_NetReceive          receive;            /* recv Function                        */
	EPS_NetReceiveFrom      receiveFrom;        /* revcfrom Function                    */
	EPS_NetGetSockName      getsockname;        /* getsockname Function                 */
	EPS_NetSetMulticastTTL  setMulticastTTL;    /* Set Multicast TTL Function           */
	EPS_NetSetBroadCast     setBroadcast;       /* Set enable broadcast Function        */
#if LCOMSW_USE_MULTI_IF
	EPS_NetEnumInterface    enumInterface;
	EPS_NetBindInterface    bindInterface;
#elif LCOMSW_BINDIF_IF_NEED 
	EPS_NetBindInterface    bindInterface;
#endif
} EPS_NET_FUNC;
#define EPS_NETFUNC_VER_1		1
#define EPS_NETFUNC_VER_CUR		EPS_USBFUNC_VER_1

#ifdef __cplusplus
}
#endif

#endif  /* def __EPSON_ESCPR_DEF_H__ */

/*________________________________  epson-escpr-def.h   ________________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
