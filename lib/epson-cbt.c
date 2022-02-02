/*___________________________________  epson-cbt.c   ___________________________________*/

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
/*                                   Epson CBT Module                                   */
/*                                                                                      */
/*                                Public Function Calls                                 */
/*                              --------------------------                              */
/*              EPS_ERR_CODE cbtCommOpen         (void                          );      */
/*              EPS_INT32    cbtCommClose        (void                          );      */
/*              EPS_ERR_CODE cbtCommChannelOpen  (Channel, bRetry               );      */
/*              EPS_INT32    cbtCommChannelClose (Channel                       );      */
/*              EPS_INT32    cbtCommReadData     (Channel, Buffer, BuffLen, Size);      */
/*              EPS_INT32    cbtCommWriteData    (Channel, Buffer, BuffLen, Size);      */
/*                                                                                      */
/*******************************************|********************************************/

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-pvt.h"
#include "epson-escpr-err.h"
#include "epson-escpr-mem.h"
#include "epson-escpr-services.h"
#include "epson-cbt.h"

/*------------------------------------  Definition   -----------------------------------*/
/*******************************************|********************************************/
#define CBT_OPENCH_WAIT				(50)
#define CBT_OPENCH_RETRY            (50)

/*------------------------------- Local Compiler Switch --------------------------------*/
/*******************************************|********************************************/
#define LCOMSW_CBT_SOCKETFUNC_ON       1    /* the socket ID mode (0:OFF  1:ON(Default))*/

#define LCOMSW_CBT_ALLOC_MEM           1    /* Use static memory                        */
                                            /*  0 -> Use static memory                  */
                                            /*  1 -> Use epsFnc.memAlloc() (Default)    */

#ifdef EPS_LOG_MODULE_CBT
#define EPS_LOG_MODULE	EPS_LOG_MODULE_CBT
#else
#define EPS_LOG_MODULE	0
#endif

/*----------------------------  ESC/P-R Lib Global Variables  --------------------------*/
/*******************************************|********************************************/

    /*** Print Job/Page Struct                                                          */
    /*** -------------------------------------------------------------------------------*/
extern EPS_PRINT_JOB    printJob;

    /*** System Dependent Function (USB Communication)                                  */
    /*** -------------------------------------------------------------------------------*/
extern EPS_USB_FUNC  epsUsbFnc;
extern EPS_CMN_FUNC  epsCmnFnc;

    /*** I/O Channel                                                                    */
    /*** -------------------------------------------------------------------------------*/
EPS_BOOL    ioOpenState     = EPS_IO_NOT_OPEN; /* Open state of I/O port (Bi-Directional)  */
EPS_BOOL    ioDataChState	= EPS_IO_NOT_OPEN; /* Open state of Data Channel               */
EPS_BOOL    ioControlChState= EPS_IO_NOT_OPEN; /* Open state of Control Channel            */
EPS_BOOL    ioOpenUniDirect = EPS_IO_NOT_OPEN; /* Open state of I/O port (Uni-Directional) */

/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/

/*------------------------  Epson CBT Module Global Variables  -------------------------*/
/*******************************************|********************************************/
static CBTS_PRNINFO *gpCbtPrnInfo = NULL;       /* Printer Information                  */

/*--------------------------------  Local Definition   ---------------------------------*/
/*******************************************|********************************************/
#define CBT_DUMYREAD_MAX	(128)				/* dummy read try max                   */

/*---------------------  Memory allocation for Epson CBT Module ------------------------*/
/*******************************************|********************************************/
#if    LCOMSW_CBT_ALLOC_MEM == 0
static CBTS_PRNINFO sgcbtprninfo;
static EPS_UINT8    sgreadbuff[CBT_MAX_CH][CBT_MAX_RTMP];
static EPS_UINT8    sgrtmpbuff[CBT_MAX_CH][CBT_MAX_RTMP];
static EPS_UINT8    sgwritebuff[CBT_TXPSIZE];
static EPS_UINT8    sgcbtreadrtnbuff[CBT_MAX_RTMP];
static EPS_UINT8    sgcbtdatareadbuff[CBT_MAX_RTMP];
#endif    /* LCOMSW_CBT_ALLOC_MEM */

/*------------------------------------  Debug Dump   -----------------------------------*/
/*******************************************|********************************************/
    /*** ALL Debug Dump Switch for <epson-cbt.>                                         */
    /*** -------------------------------------------------------------------------------*/
#define _COMM_DEBUG         0           /* 0: OFF    1: ON                              */
#define _CBTDEBUG_          0           /* 0: OFF    1: ON                              */
#define CBT_DBG_ERROR       0           /* 0: OFF    1: ON                              */

    /*** _COMM_DEBUG --- Communication Module Debug                                     */
    /***--------------------------------------------------------------------------------*/
#if _COMM_DEBUG
#define COMM_DBG_ERROR
#define CBTDebugPrintf(a)  printf a
#else   /* def _COMM_DEBUG */
#define CBTDebugPrintf(a)
#endif  /* def _COMM_DEBUG */

    /*** _CBTDEBUG_ --- CBT Debug                                                       */
    /***--------------------------------------------------------------------------------*/
#if _CBTDEBUG_
#define CBT_DBG_IO

static EPS_INT8 debMsgWork[512];
static void     DebMsgOut(EPS_INT8 *buf)
{
#ifdef _WINCE32_TEST_
    HANDLE fh_Alloc;
    DWORD  wb;

    fh_Alloc = CreateFile(L"\\CBT_LOG.TXT",GENERIC_WRITE,FILE_SHARE_WRITE,NULL,
                          OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    SetFilePointer(fh_Alloc,0,NULL,FILE_END);
    WriteFile(fh_Alloc, buf, strlen(buf), &wb, NULL);
    CloseHandle(fh_Alloc);
#else  /* def _WINCE32_TEST_ */
    fprintf(stderr,buf);
#endif /* def _WINCE32_TEST_ */
}
#endif  /* def _CBTDEBUG_ */

/*--------------------------------  Local Functions   ----------------------------------*/
/*******************************************|********************************************/
    /*** Communication Module                                                           */
    /*** -------------------------------------------------------------------------------*/
static EPS_INT32    commOpen                (const EPS_USB_DEVICE*, EPS_FILEDSC*        );
static EPS_INT32    commClose               (EPS_FILEDSC                                );
static EPS_INT32    commOpenChannel         (EPS_FILEDSC, EPS_INT32, EPS_BOOL           );
static EPS_INT32    commCloseChannel        (EPS_FILEDSC, EPS_INT32                     );
static EPS_INT32    commChChange            (EPS_INT32, EPS_UINT8*                      );

    /*** Mini CBT Engine                                                                */
    /*** -------------------------------------------------------------------------------*/
static EPS_INT16    EPCBT_Open              (EPS_FILEDSC, void*, EPS_WritePortal, EPS_ReadPortal);
static EPS_INT16    EPCBT_Close             (EPS_FILEDSC                                );
static EPS_INT16    EPCBT_OpenChannel       (EPS_FILEDSC, EPS_UINT8, EPS_BOOL           );
static EPS_INT16    EPCBT_CloseChannel      (EPS_FILEDSC, EPS_UINT8                     );
static EPS_INT16    EPCBT_Write             (EPS_FILEDSC, EPS_UINT8, const EPS_UINT8*, EPS_INT32*);
static EPS_INT16    EPCBT_Read              (EPS_FILEDSC, EPS_UINT8, EPS_UINT8*, EPS_INT32*          );
#if LCOMSW_CBT_SOCKETFUNC_ON
static EPS_INT16    EPCBT_GetSocketID       (EPS_FILEDSC, EPS_UINT8*, EPS_UINT8*, EPS_INT32          );
#endif
static EPS_INT16    CbtWriteRtn             (EPS_FILEDSC, CBTS_PRNINFO*, const EPS_UINT8*, EPS_INT32, EPS_INT32*);
static EPS_INT16    CbtReadRtn              (EPS_FILEDSC, CBTS_PRNINFO*, EPS_UINT8, EPS_UINT8*, EPS_INT32, EPS_INT32*, EPS_UINT8);
static void         CbtDummyRead            (EPS_FILEDSC, CBTS_PRNINFO*                 );
static void         CbtChDummyRead          (EPS_FILEDSC, CBTS_PRNINFO*, CBTS_CHINFO*   );
static EPS_INT16    CbtReplyCheck           (EPS_UINT8*, EPS_INT32, EPS_UINT8           );
static EPS_INT16    CbtEpsonPacking         (EPS_FILEDSC, CBTS_PRNINFO*                 );
static EPS_INT16    CbtInitCommand          (EPS_FILEDSC, CBTS_PRNINFO*                              );
static EPS_INT32    CbtOpenChannelCommand   (EPS_FILEDSC, CBTS_PRNINFO*, EPS_UINT8, EPS_UINT32*, EPS_UINT32*, EPS_UINT32, EPS_UINT32);
static EPS_INT16    CbtCloseChannelCommand  (EPS_FILEDSC, CBTS_PRNINFO*, EPS_UINT8                   );
static EPS_INT32    CbtCreditReqCommand     (EPS_FILEDSC, CBTS_PRNINFO*, EPS_UINT8, EPS_UINT32, EPS_UINT32);
static EPS_INT16    CbtCreditCommand        (EPS_FILEDSC, CBTS_PRNINFO*, EPS_UINT8, EPS_UINT32       );
#if LCOMSW_CBT_SOCKETFUNC_ON
static EPS_INT16    CbtGetSocketIDCommand   (EPS_FILEDSC, CBTS_PRNINFO*, EPS_UINT8*, EPS_INT32, EPS_UINT8*);
#endif
static EPS_INT32    CbtDataWrite            (EPS_FILEDSC, CBTS_PRNINFO*, EPS_UINT8, const EPS_UINT8*, EPS_INT32);
static EPS_INT16    CbtDataRead             (EPS_FILEDSC, CBTS_PRNINFO*, EPS_UINT8, EPS_UINT8*, EPS_INT32*);
static EPS_INT16    CbtMemAlloc             (void                                       );
static void         CbtMemFree              (void                                       );
static void         CbtPutBigEndianByte2    (EPS_UINT32, EPS_UINT8*                     );


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------              Public Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   cbtCommOpen()                                                       */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* devinfo      EPS_USB_DEVICE      I: Pointer to a usb device infomation               */
/* fd           EPS_FILEDSC*        O: file discripter                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success (Opened Communication)                */
/*      EPSCBT_ERR_2NDOPEN              - CBT channels are already opened               */
/*      EPS_ERR_NOT_OPEN_IO             - Cannot Open I/O Portal                        */
/*      EPCBT_ERR_CBT2NDOPEN            - CBT channels are already opened               */
/*      EPCBT_ERR_MEMORY                - Fail to memory allocation                     */
/*      EPCBT_ERR_CBTDISABLE            - Fail to CBT communication                     */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_INITDENIED            - Printer cannot start CBT mode                 */
/*      EPCBT_ERR_VERSION               - Incompatible version                          */
/*      EPCBT_ERR_INITFAILED            - Not used by EPSON                             */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_CBTNOTOPEN            - Port is not open                              */
/*      EPCBT_ERR_PARAM                 - Parameter error                               */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_NOSERVICE             - The channel does not support the requested    */
/*                                        service                                       */
/*                                                                                      */
/* Description:                                                                         */
/*     Opens the communication on the specified.                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32   cbtCommOpen (

        const EPS_USB_DEVICE*	devinfo,
		EPS_FILEDSC*			fd
){
    
    EPS_INT32 Ret;                              /* Return status of internal calls      */

	EPS_LOG_FUNCIN;

/*** Open Communication                                                                 */
    if ((Ret = commOpen(devinfo, fd)) != EPCBT_ERR_NONE){
        CBTDebugPrintf(("EPS CBT: Open Failed [%d]\r\n",Ret));
		EPS_RETURN( Ret );
    }

    EPS_RETURN( EPS_ERR_NONE );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   cbtCommClose()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Closed Communication                          */
/*      EPSCBT_ERR_NOTOPEN              - I/O port is not opened                        */
/*      EPCBT_ERR_CBTNOTOPEN            - Port is not open                              */
/*                                                                                      */
/* Description:                                                                         */
/*      Closes the communication on the active printer.                                 */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32   cbtCommClose (

        EPS_FILEDSC		fd

){
    EPS_INT32 Ret;

	EPS_LOG_FUNCIN;

	CBTDebugPrintf(("EPS CBT:CLOSE --> Closing Communication on the device\r\n"));

    Ret = commClose(fd);

    if (Ret != EPCBT_ERR_NONE){
        CBTDebugPrintf(("EPS CBT: Close failed\r\n"));
        EPS_RETURN( Ret );
    }
    EPS_RETURN( EPS_ERR_NONE );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   cbtCommChannelOpen()                                                */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* Channel      EPS_INT32           I: Communication Channel                            */
/* bRetry       EPS_BOOL            I: If TRUE, retry open                              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success (Opened Communication)                */
/*      EPSCBT_ERR_NOTOPEN              - CBT channels are not opened                   */
/*      EPSCBT_ERR_PARAM                - Parameter error                               */
/*      EPCBT_ERR_CBTNOTOPEN            - Port is not open                              */
/*      EPCBT_ERR_PARAM                 - Parameter error                               */
/*      EPCBT_ERR_CH2NDOPEN             - Channel is already open                       */
/*      EPCBT_ERR_RPLYPSIZE             - Size of packet from OpenChannel is invalid    */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*      EPCBT_ERR_RESOURCE              - Insufficient printer resources for            */
/*                                        OpenChannel                                   */
/*      EPCBT_ERR_OPENCHANNEL           - Not used by EPSON                             */
/*      EPCBT_ERR_CHNOTSUPPORT          - The channel is not supported                  */
/*      EPCBT_ERR_PACKETSIZE            - Invalid packet size.                          */
/*                                        (0x0001 ? 0x0005 bytes)                       */
/*      EPCBT_ERR_NULLPACKETSZ          - Packet size is 0x0000 in both directions.     */
/*                                        No data can be transferred. Channel was       */
/*                                        not opened                                    */
/*                                                                                      */
/* Description:                                                                         */
/*      Open the communication channel.                                                 */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32   cbtCommChannelOpen (

		EPS_FILEDSC	fd,
        EPS_INT32   Channel,
		EPS_BOOL	bRetry

){
    EPS_INT32 Ret;

	EPS_LOG_FUNCIN;

    if ((Ret = commOpenChannel(fd, Channel, bRetry)) != EPCBT_ERR_NONE){

        if (Channel == EPS_CBTCHANNEL_CTRL) {
            CBTDebugPrintf(("EPS CBT: Control Channel Open Failed\r\n"));
            EPS_RETURN( Ret );
        } else if (Channel == EPS_CBTCHANNEL_DATA) {
            CBTDebugPrintf(("EPS CBT: Data Channel Open Failed\r\n"));
            EPS_RETURN( Ret );
        }

    }

    EPS_RETURN( EPS_ERR_NONE );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   cbtCommChannelClose()                                               */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* Channel      EPS_INT32           I: Communication Channel                            */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success (Closed Communication)                */
/*      EPSCBT_ERR_NOTOPEN              - CBT channels are not opened                   */
/*      EPSCBT_ERR_PARAM                - Parameter error                               */
/*      EPCBT_ERR_CBTNOTOPEN            - Port is not open                              */
/*      EPCBT_ERR_CHNOTOPEN             - Channel is not opened                         */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_CLOSEDENIED           - Close Channel is denied                       */
/*      EPCBT_ERR_CMDDENIED             - Command is denied (Channel is not opened)     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*                                                                                      */
/* Description:                                                                         */
/*      Close the communication channel.                                                */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32   cbtCommChannelClose (

		EPS_FILEDSC	fd,
        EPS_INT32   Channel

){
    EPS_INT32 Ret;

	EPS_LOG_FUNCIN;

	if ((Ret = commCloseChannel(fd, Channel)) != EPCBT_ERR_NONE){
        CBTDebugPrintf(("EPS CBT: Channel Close Failed\r\n"));
        EPS_RETURN( Ret );
    }

    EPS_RETURN( EPS_ERR_NONE );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   cbtCommReadData()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* Channel      EPS_INT32           I: Channel-Select                                   */
/*                                      00: Data Channel                                */
/*                                      01: Control Channel                             */
/* Buffer       EPS_UINT8*          I: Buffer Pointer for Read Data                     */
/* BuffLen      EPS_INT32           I: Read Data Buffer Length (bytes)                  */
/* Size         EPS_INT32*          O: Actuall Read Size                                */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPSCBT_ERR_NOTOPEN              - I/O port is not opened                        */
/*      EPSCBT_ERR_PARAM                - Parameter error                               */
/*      EPCBT_ERR_CBTNOTOPEN            - CBT is not opened                             */
/*      EPCBT_ERR_PARAM                 - Parameter error                               */
/*      EPCBT_ERR_CHNOTOPEN             - Channel is not opened                         */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_CREDITOVF             - Credit over flow                              */
/*      EPCBT_ERR_CMDDENIED             - Command is denied (Channel is not opened)     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*      EPCBT_ERR_READERROR             - Data read error                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Read the data from printer.                                                     */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32   cbtCommReadData (

		EPS_FILEDSC	fd,
        EPS_INT32   Channel,
        EPS_UINT8*  Buffer,
        EPS_INT32   BuffLen,
        EPS_INT32*  Size

){
    EPS_INT32   Ret   = EPCBT_ERR_NONE;
    EPS_INT32   InLen = 0;
    EPS_UINT8   CBTCh = 0;

	EPS_LOG_FUNCIN;

	if ( ioOpenState == EPS_IO_NOT_OPEN )          /* Open check */
        Ret = EPSCBT_ERR_NOTOPEN;
    else
    {
        Ret = commChChange( Channel, &CBTCh );              /* channel change */
        if( Ret == EPCBT_ERR_NONE )
        {
            *Size = 0;                                      /* size clear */
            Ret = EPCBT_Read( fd, CBTCh, NULL, &InLen );        /* Size Get */
            if ( Ret == EPCBT_ERR_NONE )
            {
                if ( InLen > BuffLen )                      /* small buffer size ? */
                    InLen = BuffLen;
                Ret = EPCBT_Read( fd, CBTCh, Buffer, &InLen );   /* Data Get */
                *Size = InLen;
            }
        }
    }

#ifdef COMM_DBG_ERROR
    if ( Ret != EPCBT_ERR_NONE )
        CBTDebugPrintf(("<<  ReadData Ret[%d] Size[%d]\r\n", Ret, *Size));
#endif

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   cbtCommWriteData()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* Channel      EPS_INT32           I: Channel-Select                                   */
/*                                      00: Data Channel                                */
/*                                      01: Control Channel                             */
/* Buffer       const EPS_UINT8*    I: Buffer Pointer for Write Data                    */
/* BuffLen      EPS_INT32           I: Write Data Buffer Length (bytes)                 */
/* Size         EPS_INT32*          O: Actuall Write Size                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPSCBT_ERR_NOTOPEN              - I/O port is not opened                        */
/*      EPCBT_ERR_CBTNOTOPEN            - CBT is not opened                             */
/*      EPSCBT_ERR_PARAM                - Parameter error                               */
/*      EPCBT_ERR_CHNOTOPEN             - Channel is not opened                         */
/*      EPCBT_ERR_FNCDISABLE            - Function is disable to be completely finished */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_CMDDENIED             - Command is denied (Channel is not opened)     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_WRITEERROR            - Failed to write                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Write the data to printer.                                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32   cbtCommWriteData (

		EPS_FILEDSC			fd,
        EPS_INT32           Channel,
        const EPS_UINT8*    Buffer,
        EPS_INT32           BuffLen,
        EPS_INT32*          Size

){
    EPS_INT32   Ret = EPCBT_ERR_NONE;
    EPS_INT32   Written;
    EPS_INT32   RestSize;
    EPS_UINT8   CBTCh = 0;

	EPS_LOG_FUNCIN;

	CBTDebugPrintf(("EPS CBT : cbtCommWriteData -> ioOpenState == %s\r\n", ioOpenState == EPS_IO_OPEN ? "ON" : "OFF"));

    if ( ioOpenState == EPS_IO_NOT_OPEN ) {                    /* Open check */
        Ret = EPSCBT_ERR_NOTOPEN;
    }
    else
    {
        /* Send Data */
        Ret = commChChange( Channel, &CBTCh );                          /* channel change */
        if( Ret == EPCBT_ERR_NONE )
        {
            *Size    = 0;                                               /* size clear */
            RestSize = BuffLen;

            while ( RestSize > 0 )
            {
                Written = RestSize;
                Ret = EPCBT_Write(fd, CBTCh, &Buffer[*Size], &Written );    /* Write */
                *Size    += Written;                                    /* Set Write Length */
                RestSize -= Written;
                
                if( Ret != EPCBT_ERR_NONE )
                    break;
            }
        }
    }

#ifdef COMM_DBG_ERROR
    if ( Ret != EPCBT_ERR_NONE )
        CBTDebugPrintf(("<<  WriteData Ret[%d] Size[%d]\r\n", Ret, *Size));
#endif

    EPS_RETURN( Ret );
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
/* Function name:   commOpen()                                                          */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* devinfo      EPS_USB_DEVICE      I: Pointer to a usb device infomation               */
/* fd           EPS_FILEDSC*        O: file discripter                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPSCBT_ERR_2NDOPEN              - CBT channels are already opened               */
/*      EPS_ERR_NOT_OPEN_IO             - Cannot Open I/O Portal                        */
/*      EPCBT_ERR_CBT2NDOPEN            - CBT channels are already opened               */
/*      EPCBT_ERR_MEMORY                - Fail to memory allocation                     */
/*      EPCBT_ERR_CBTDISABLE            - Fail to CBT communication                     */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_INITDENIED            - Printer cannot start CBT mode                 */
/*      EPCBT_ERR_VERSION               - Incompatible version                          */
/*      EPCBT_ERR_INITFAILED            - Not used by EPSON                             */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_CBTNOTOPEN            - Port is not open                              */
/*      EPCBT_ERR_PARAM                 - Parameter error                               */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_NOSERVICE             - The channel does not support the requested    */
/*                                        service                                       */
/*                                                                                      */
/* Description:                                                                         */
/*      Opens the communication.                                                        */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT32    commOpen (

        const EPS_USB_DEVICE*	devinfo,
		EPS_FILEDSC*			fd

){
    EPS_INT32   Ret = EPCBT_ERR_NONE;
    EPS_UINT8   Sid = 0;

	EPS_LOG_FUNCIN;

    if ( ioOpenState == EPS_IO_OPEN ) {        /* Open check */
        Ret = EPSCBT_ERR_2NDOPEN;
    } else {
        if ( (*fd = epsUsbFnc.openPortal(devinfo)) == EPS_INVALID_FILEDSC)
            EPS_RETURN( EPS_ERR_NOT_OPEN_IO );

        /* Open process */
        Ret = EPCBT_Open(*fd, NULL, epsUsbFnc.writePortal, epsUsbFnc.readPortal);        /* Open */

        if ( Ret == EPCBT_ERR_NONE ) {
            /* channel confirm */
            Ret = EPCBT_GetSocketID( *fd, &Sid, (EPS_UINT8*)"EPSON-DATA", 10 );
            if ( Ret == EPCBT_ERR_NONE )
                Ret = EPCBT_GetSocketID( *fd, &Sid, (EPS_UINT8*)"EPSON-CTRL", 10 );

			if(EPCBT_ERR_NONE != Ret ){
				EPCBT_Close(*fd);
			}
		} else if(EPS_INVALID_FILEDSC != *fd){
			epsUsbFnc.closePortal(*fd);
			*fd = EPS_INVALID_FILEDSC;
		}
    }

    if ( Ret == EPCBT_ERR_NONE )
        ioOpenState = EPS_IO_OPEN;        /* Open flag On */

    CBTDebugPrintf(("EPS CBT : commOpen -> ioOpenState == %s\r\n", ioOpenState == EPS_IO_OPEN ? "ON" : "OFF"));

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   commClose()                                                         */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPSCBT_ERR_NOTOPEN              - I/O port is not opened                        */
/*      EPCBT_ERR_CBTNOTOPEN            - Port is not open                              */
/*                                                                                      */
/* Description:                                                                         */
/*      Close the communication.                                                        */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT32    commClose (

        EPS_FILEDSC		fd

){
    EPS_INT32 Ret           = EPCBT_ERR_NONE;
    EPS_INT32 closePtlState = 0;

	EPS_LOG_FUNCIN;

	if ( ioOpenState == EPS_IO_NOT_OPEN )        /* Open check */
        Ret = EPSCBT_ERR_NOTOPEN;
    else
    {
        Ret = EPCBT_Close(fd);    /* Channel close & cbt exit */

        closePtlState = epsUsbFnc.closePortal(fd);

        ioOpenState = EPS_IO_NOT_OPEN;        /* Open flag Off */
    }

#ifdef COMM_DBG_ERROR
    if ( Ret != EPCBT_ERR_NONE )
        CBTDebugPrintf(("<<  Close Ret[%d]  Close Portal[%d]\r\n", Ret, closePtlState));
#endif

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   commOpenChannel()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* Channel      EPS_INT32           I: channel number                                   */
/* bRetry       EPS_BOOL            I: If TRUE, retry open                              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPSCBT_ERR_NOTOPEN              - CBT channels are not opened                   */
/*      EPSCBT_ERR_PARAM                - Parameter error                               */
/*      EPCBT_ERR_CBTNOTOPEN            - Port is not open                              */
/*      EPCBT_ERR_PARAM                 - Parameter error                               */
/*      EPCBT_ERR_CH2NDOPEN             - Channel is already open                       */
/*      EPCBT_ERR_RPLYPSIZE             - Size of packet from OpenChannel is invalid    */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*      EPCBT_ERR_RESOURCE              - Insufficient printer resources for            */
/*                                        OpenChannel                                   */
/*      EPCBT_ERR_OPENCHANNEL           - Not used by EPSON                             */
/*      EPCBT_ERR_CHNOTSUPPORT          - The channel is not supported                  */
/*      EPCBT_ERR_PACKETSIZE            - Invalid packet size.                          */
/*                                        (0x0001 ? 0x0005 bytes)                       */
/*      EPCBT_ERR_NULLPACKETSZ          - Packet size is 0x0000 in both directions.     */
/*                                        No data can be transferred. Channel was       */
/*                                        not opened                                    */
/*                                                                                      */
/* Description:                                                                         */
/*      Opens the communication channel.                                                */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT32    commOpenChannel (

			EPS_FILEDSC	fd,
			EPS_INT32   Channel,
			EPS_BOOL	bRetry

){
    EPS_INT32   Ret   = EPCBT_ERR_NONE;
    EPS_UINT8   CBTCh = 0;

	EPS_LOG_FUNCIN;

    if ( ioOpenState == EPS_IO_NOT_OPEN ) {        /* Open check */
        Ret = EPSCBT_ERR_NOTOPEN;
    } else{
        Ret = commChChange( Channel, &CBTCh );        /* Channel Change */

        if ( Ret == EPCBT_ERR_NONE ) {
            if ((CBTCh == CBT_CBTCH_DATA) && (ioDataChState == EPS_IO_NOT_OPEN)) {
                Ret = (EPS_INT16)EPCBT_OpenChannel(fd, CBTCh, bRetry);    /* Open Data Channel */
                if (Ret == EPCBT_ERR_NONE)
                    ioDataChState = EPS_IO_OPEN;
            }
            
            if ((CBTCh == CBT_CBTCH_CTRL) && (ioControlChState == EPS_IO_NOT_OPEN)) {
                Ret = (EPS_INT16)EPCBT_OpenChannel(fd, CBTCh, bRetry);    /* Open Control Channel */
                if (Ret == EPCBT_ERR_NONE)
                    ioControlChState = EPS_IO_OPEN;
            }
        }
    }

#ifdef COMM_DBG_ERROR
    CBTDebugPrintf(("EPS CBT : commOpenChannel(%x)->%d\r\n", CBTCh, Ret));
    CBTDebugPrintf(("EPS CBT : commOpenChannel-> ioOpenState == %s\r\n", ioOpenState == EPS_IO_OPEN ? "ON" : "OFF"));
    if ( Ret != EPCBT_ERR_NONE )
        CBTDebugPrintf(("<<  Close Channel Ret[%d]\r\n", Ret));
#endif

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   commCloseChannel()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* Channel      EPS_INT32           I: channel number                                   */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPSCBT_ERR_NOTOPEN              - CBT channels are not opened                   */
/*      EPSCBT_ERR_PARAM                - Parameter error                               */
/*      EPCBT_ERR_CBTNOTOPEN            - Port is not open                              */
/*      EPCBT_ERR_CHNOTOPEN             - Channel is not opened                         */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_CLOSEDENIED           - Close Channel is denied                       */
/*      EPCBT_ERR_CMDDENIED             - Command is denied (Channel is not opened)     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*                                                                                      */
/* Description:                                                                         */
/*      Close the communication channel.                                                */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT32    commCloseChannel (

		EPS_FILEDSC	fd,
        EPS_INT32   Channel

){
    EPS_INT32   Ret   = EPCBT_ERR_NONE;
    EPS_UINT8   CBTCh = 0;
    
	EPS_LOG_FUNCIN;

	if ( ioOpenState == EPS_IO_NOT_OPEN )        /* Open check */
        Ret = EPSCBT_ERR_NOTOPEN;
    else
    {
        /* Channel close */
        Ret = commChChange( Channel, &CBTCh );        /* Channel Change */

        if ( Ret == EPCBT_ERR_NONE ) {
            if ((CBTCh == CBT_CBTCH_DATA) && (ioDataChState == EPS_IO_OPEN)) {
                Ret = (EPS_INT16)EPCBT_CloseChannel(fd, CBTCh);    /* Close Data Channel */
                /*if (Ret == EPCBT_ERR_NONE)		Force reset */
                    ioDataChState = EPS_IO_NOT_OPEN;
            }
            
            if ((CBTCh == CBT_CBTCH_CTRL) && (ioControlChState == EPS_IO_OPEN)) {
                Ret = (EPS_INT16)EPCBT_CloseChannel(fd, CBTCh);    /* Close Control Channel */
                /*if (Ret == EPCBT_ERR_NONE)		Force reset */
                    ioControlChState = EPS_IO_NOT_OPEN;
            }
        }
	}

#ifdef COMM_DBG_ERROR
    if ( Ret != EPCBT_ERR_NONE )
        CBTDebugPrintf(("<<  Close Channel Ret[%d]\r\n", Ret));
#endif

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   commChChange()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* Channel      EPS_INT32           I: Channel number definition                        */
/*                                      00: Data Channel                                */
/*                                      01: Control Channel                             */
/* CBTCh        EPS_UINT8*          O: Pointer to the channel number                    */
/*                                      40: Data Channel                                */
/*                                      02: Control Channel                             */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPSCBT_ERR_PARAM                - Parameter error                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Set the channel number.                                                         */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT32    commChChange (

        EPS_INT32   Channel,
        EPS_UINT8   *CBTCh 

){
    EPS_INT32   Ret = EPCBT_ERR_NONE;

	EPS_LOG_FUNCIN;

	switch (Channel)
    {
        case EPS_CBTCHANNEL_DATA:    /* Data Channel */
            *CBTCh = CBT_CBTCH_DATA;
            break;
        case EPS_CBTCHANNEL_CTRL:    /* Control Channel */
            *CBTCh = CBT_CBTCH_CTRL;
            break;
        default:                    /* Error */
            Ret = EPSCBT_ERR_PARAM;
            break;
    }

    EPS_RETURN( Ret );
}


/*******************************************|********************************************/
/*vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv                       vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv    Mini CBT Engine    vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv                       vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*******************************************|********************************************/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   EPCBT_Open                                                          */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pvPortInfo   void *              I: Port information                                 */
/* lpWriteFunc  EPS_WritePortal     I: Pointer to write function                        */
/* lpReadFunc   EPS_ReadPortal      I: Pointer to read functiont information            */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_CBT2NDOPEN            - CBT channels are already opened               */
/*      EPCBT_ERR_MEMORY                - Fail to memory allocation                     */
/*      EPCBT_ERR_CBTDISABLE            - Fail to CBT communication                     */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_INITDENIED            - Printer cannot start CBT mode                 */
/*      EPCBT_ERR_VERSION               - Incompatible version                          */
/*      EPCBT_ERR_INITFAILED            - Not used by EPSON                             */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Opens the CBT communication                                                     */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    EPCBT_Open (

		EPS_FILEDSC			fd, 
        void*               pvPortInfo,
        EPS_WritePortal     lpWriteFunc,
        EPS_ReadPortal      lpReadFunc

){
    EPS_INT16 Ret = EPCBT_ERR_NONE;

	EPS_LOG_FUNCIN;

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_Open (S)\r\n");
    DebMsgOut(debMsgWork);
#endif

    if( gpCbtPrnInfo != NULL) {
        Ret = EPCBT_ERR_CBT2NDOPEN;
    } else {
        Ret = CbtMemAlloc( );
    }
    
    if ( Ret == EPCBT_ERR_NONE ) {
        gpCbtPrnInfo->pPortInfo   = pvPortInfo;
        gpCbtPrnInfo->pCbtWRtnFnc = lpWriteFunc;
        gpCbtPrnInfo->pCbtRRtnFnc = lpReadFunc;
        gpCbtPrnInfo->Mode        = CBT_MODE_CBTOFF;

        CbtDummyRead( fd, gpCbtPrnInfo );               /* Dummy read                       */
        Ret = CbtEpsonPacking( fd, gpCbtPrnInfo );      /* send EpsonPackingCommand         */
        if ( Ret == EPCBT_ERR_NONE )
            Ret = CbtInitCommand( fd, gpCbtPrnInfo );   /* send InitCommand                 */
    }

    if ( Ret == EPCBT_ERR_NONE ) {                  /* OK ?                             */
        gpCbtPrnInfo->Mode = CBT_MODE_CBTON;
    } else {
        if(Ret != EPCBT_ERR_CBT2NDOPEN)
            CbtMemFree( );
    }

#if    CBT_DBG_ERROR
    if( Ret != EPCBT_ERR_NONE ) {
        sprintf(debMsgWork," <- CBT_Open Ret[%d]\r\n",Ret);
        DebMsgOut(debMsgWork);
    }
#endif

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_Open (E)\r\n");
    DebMsgOut(debMsgWork);
#endif

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   EPCBT_Close()                                                       */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_CBTNOTOPEN            - Port is not open                              */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_CLOSEDENIED           - Close Channel is denied                       */
/*      EPCBT_ERR_CMDDENIED             - Command is denied (Channel is not opened)     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*                                                                                      */
/* Description:                                                                         */
/*      End CBT mode.                                                                   */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    EPCBT_Close (

        EPS_FILEDSC	fd

){
    EPS_INT16       Ret = EPCBT_ERR_NONE;
    EPS_INT32       lp1;
    CBTS_CHINFO*    ChPtr;

	EPS_LOG_FUNCIN;

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_Close (S)\r\n");
    DebMsgOut(debMsgWork);
#endif

    if( gpCbtPrnInfo == NULL) {
        Ret = EPCBT_ERR_CBTNOTOPEN;
    } else {
        gpCbtPrnInfo->Mode = CBT_MODE_REQOFF;

        for ( lp1 = 1; lp1 < CBT_MAX_CH ; lp1++ ) {
            ChPtr = &gpCbtPrnInfo->Chinfo[lp1];

            if ( ChPtr->No != 0 ) {
                Ret = CbtCloseChannelCommand( fd, gpCbtPrnInfo, ChPtr->No );    /* Channel Close */
            }
        }

        for ( lp1 = 0; lp1 < CBT_MAX_CH ; lp1++ ) {
            ChPtr           = &gpCbtPrnInfo->Chinfo[lp1];
            ChPtr->No       = 0;
            ChPtr->PtSsize  = 0;
            ChPtr->StPsize  = 0;
            ChPtr->CreditP  = 0;
            ChPtr->CreditH  = 0;
            ChPtr->ReadSize = 0;
            ChPtr->RTmpTop  = 0;
            ChPtr->RTmpEnd  = 0;
            ChPtr->RTmpCnt  = 0;
        }

        gpCbtPrnInfo->Mode = CBT_MODE_CBTOFF;
    }
    CbtMemFree( );

#if    CBT_DBG_ERROR
    if( Ret != EPCBT_ERR_NONE ) {
        sprintf(debMsgWork, " <- CBT_Close Error! Ret[%d]\r\n",Ret);
        DebMsgOut(debMsgWork);
    }
#endif

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_Close (E)\r\n");
    DebMsgOut(debMsgWork);
#endif

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   EPCBT_OpenChannel                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* SID          EPS_UINT8           I: socket id                                        */
/* bRetry       EPS_BOOL            I: If TRUE, retry open                              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Port is not open                              */
/*      EPCBT_ERR_CBTNOTOPEN            - Port is not open                              */
/*      EPCBT_ERR_PARAM                 - Parameter error                               */
/*      EPCBT_ERR_CH2NDOPEN             - Channel is already open                       */
/*      EPCBT_ERR_RPLYPSIZE             - Size of packet from OpenChannel is invalid    */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*      EPCBT_ERR_RESOURCE              - Insufficient printer resources for            */
/*                                        OpenChannel                                   */
/*      EPCBT_ERR_OPENCHANNEL           - Not used by EPSON                             */
/*      EPCBT_ERR_CHNOTSUPPORT          - The channel is not supported                  */
/*      EPCBT_ERR_PACKETSIZE            - Invalid packet size.                          */
/*                                        (0x0001 ? 0x0005 bytes)                       */
/*      EPCBT_ERR_NULLPACKETSZ          - Packet size is 0x0000 in both directions.     */
/*                                        No data can be transferred. Channel was       */
/*                                        not opened                                    */
/*                                                                                      */
/* Description:                                                                         */
/*      Opens the CBT communication.                                                    */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    EPCBT_OpenChannel (

			EPS_FILEDSC		fd, 
			EPS_UINT8		SID,
			EPS_BOOL		bRetry

){
    EPS_INT16   Ret     = EPCBT_ERR_NONE;
    EPS_UINT32  PtSsize = 0;
    EPS_UINT32  StPsize = 0;
    EPS_INT32   iCredit = 0;
    EPS_INT32   nOpenRetry = 0;
    EPS_INT32   lp1;
    CBTS_CHINFO *ChPtr;

	EPS_LOG_FUNCIN;

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_OpenChannel (S)    0x%02x\r\n", SID);
    DebMsgOut(debMsgWork);
#endif

    if( gpCbtPrnInfo == NULL) {
        Ret = EPCBT_ERR_CBTNOTOPEN;
    } else {
        if ( gpCbtPrnInfo->Mode == CBT_MODE_CBTON ) {
            if ( (SID == 0x00) || (SID == 0xff) )
                Ret = EPCBT_ERR_PARAM;
            else {
                for ( lp1 = 1; lp1 < CBT_MAX_CH ; lp1++ ) {
                    ChPtr = &gpCbtPrnInfo->Chinfo[lp1];
                    if ( ChPtr->No == SID ) {                /* channel open ? */
                        Ret = EPCBT_ERR_CH2NDOPEN;
                        break;
                    }
                }
                if ( Ret == EPCBT_ERR_NONE ) {
                    if ( gpCbtPrnInfo->Chinfo[lp1-1].No != 0 )     /* Over flow */
                        Ret = EPCBT_ERR_PARAM; 
                }
            }
        } else {
            Ret = EPCBT_ERR_CBTNOTOPEN;
        }
    }
    if ( Ret == EPCBT_ERR_NONE ) {
        if ( SID == CBT_SID_DATA ) {
            PtSsize = CBT_TXPSIZE;
            StPsize = CBT_RXPSIZE;
        } else {
            PtSsize = CBT_RXPSIZE;
            StPsize = CBT_RXPSIZE;
        }

		if(bRetry){
			for( ; EPCBT_ERR_RESOURCE == (iCredit = CbtOpenChannelCommand(fd, gpCbtPrnInfo, SID, &PtSsize, &StPsize, 0, 0) )
				&& nOpenRetry < CBT_OPENCH_RETRY; nOpenRetry++){

				serDelayThread(CBT_OPENCH_WAIT, &epsCmnFnc);
			}
		} else{
			iCredit = CbtOpenChannelCommand(fd, gpCbtPrnInfo, SID, &PtSsize, &StPsize, 0, 0);
		}
		
		if ( iCredit < 0 )
            Ret = (EPS_INT16)iCredit;
    }
    if ( Ret == EPCBT_ERR_NONE ) {

        if ( ( PtSsize == 0 ) && ( StPsize == 0 ) )
            Ret = EPCBT_ERR_RPLYPSIZE;
        else {
            if ( (0 < PtSsize) && (PtSsize < 6) )
                Ret = EPCBT_ERR_RPLYPSIZE;
            else {
                if ( (0 < StPsize) && (StPsize < 6) )
                    Ret = EPCBT_ERR_RPLYPSIZE;
            }
        }
    }

    if ( Ret == EPCBT_ERR_NONE ) {
        for ( lp1 = 1; ; lp1++ ) {
            if ( lp1 >= CBT_MAX_CH ) {
                Ret = EPCBT_ERR_PARAM;
                break;
            }

            ChPtr = &gpCbtPrnInfo->Chinfo[lp1];

            if ( ChPtr->No == 0 ) {
                ChPtr->No       = SID;
                ChPtr->PtSsize  = (EPS_INT32)PtSsize;
                ChPtr->StPsize  = (EPS_INT32)StPsize;
                ChPtr->CreditP  = iCredit;
                ChPtr->CreditH  = 0;
                ChPtr->ReadSize = 0;
                ChPtr->RTmpTop  = 0;
                ChPtr->RTmpEnd  = 0;
                ChPtr->RTmpCnt  = 0;
                if ( SID == CBT_SID_CTRL ) {
                    CbtChDummyRead( fd, gpCbtPrnInfo, ChPtr );    /* Dummy Read */
                }
                break;
            }
        }
    }

#if    CBT_DBG_ERROR
    if( Ret != EPCBT_ERR_NONE ) {
        sprintf(debMsgWork, " <- EPCBT_OpenChannel Ret[%d] CH[%x]\r\n", Ret,SID);
        DebMsgOut(debMsgWork);
    }
#endif

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_OpenChannel (E)    0x%02x\r\n", SID);
    DebMsgOut(debMsgWork);
#endif

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   EPCBT_CloseChannel()                                                */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* SID          EPS_UINT8           I: Socket ID                                        */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Port is not open                              */
/*      EPCBT_ERR_CBTNOTOPEN            - Port is not open                              */
/*      EPCBT_ERR_PARAM                 - Parameter error                               */
/*      EPCBT_ERR_CHNOTOPEN             - Channel is not opened                         */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_CLOSEDENIED           - Close Channel is denied                       */
/*      EPCBT_ERR_CMDDENIED             - Command is denied (Channel is not opened)     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*                                                                                      */
/* Description:                                                                         */
/*      Close the communication channel.                                                */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    EPCBT_CloseChannel (

        EPS_FILEDSC	fd,
		EPS_UINT8	SID

){
    EPS_INT16       Ret   = EPCBT_ERR_NONE;
    EPS_INT32       lp1;
    CBTS_CHINFO*    ChPtr = NULL;

	EPS_LOG_FUNCIN;

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_CloseChannel (S)    0x%02x\r\n", SID);
    DebMsgOut(debMsgWork);
#endif

    if( gpCbtPrnInfo == NULL) {
        Ret = EPCBT_ERR_CBTNOTOPEN;
    } else {
        if ( gpCbtPrnInfo->Mode == CBT_MODE_CBTON ) {
            if ( (SID == 0x00) || (SID == 0xff) )
                Ret = EPCBT_ERR_PARAM;
            else {
                for ( lp1 = 1; ; lp1++ ) {
                    if ( lp1 >= CBT_MAX_CH ) {
                        Ret = EPCBT_ERR_CHNOTOPEN;
                        break;
                    }
                    ChPtr = &gpCbtPrnInfo->Chinfo[lp1];
                    if ( ChPtr->No == SID )
                        break;
                }
            }
        } else {
            EPS_RETURN( Ret );                    /* return normal end */
        }
    }
    if ( Ret == EPCBT_ERR_NONE ) {
        Ret = CbtCloseChannelCommand( fd, gpCbtPrnInfo, SID );
        /*if ( Ret == EPCBT_ERR_NONE ) {	Force reset */
            ChPtr->No       = 0;
            ChPtr->PtSsize  = 0;
            ChPtr->StPsize  = 0;
            ChPtr->CreditP  = 0;
            ChPtr->CreditH  = 0;
            ChPtr->ReadSize = 0;
            ChPtr->RTmpTop  = 0;
            ChPtr->RTmpEnd  = 0;
            ChPtr->RTmpCnt  = 0;
        /*}									Force reset */
    }

#if    CBT_DBG_ERROR
    if( Ret != EPCBT_ERR_NONE ) {
        sprintf(debMsgWork, " <- CBT_CloseChannel Ret[%d] CH[%x]\r\n",Ret,SID);
        DebMsgOut(debMsgWork);
    }
#endif

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_CloseChannel (E)    0x%02x\r\n", SID);
    DebMsgOut(debMsgWork);
#endif

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   EPCBT_Write()                                                       */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* SID          EPS_UINT8           I: Channel-Select                                   */
/* lpBuffer     const EPS_UINT8*    I: Buffer Pointer for Write Data                    */
/* lpSize       EPS_INT32*          O: Write Data Buffer Length (bytes)                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_CBTNOTOPEN            - CBT is not opened                             */
/*      EPSCBT_ERR_PARAM                - Parameter error                               */
/*      EPCBT_ERR_CHNOTOPEN             - Channel is not opened                         */
/*      EPCBT_ERR_FNCDISABLE            - Function is disable to be completely finished */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_CMDDENIED             - Command is denied (Channel is not opened)     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_WRITEERROR            - Failed to write                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Write the data to printer.                                                      */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    EPCBT_Write (

        EPS_FILEDSC			fd,
		EPS_UINT8           SID,
        const EPS_UINT8*    lpBuffer,
        EPS_INT32*          lpSize

){
    EPS_INT16       Ret = EPCBT_ERR_NONE;
    EPS_INT32       Credit;
    EPS_INT32       Size;
    EPS_INT32       Count;
    EPS_INT32       lp1;
    EPS_INT32       loopSize;
    EPS_INT32       SendMaxSize;
    CBTS_CHINFO*    ChPtr = NULL;

	EPS_LOG_FUNCIN;

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_Write (S)    0x%02x(%04d)\r\n", SID, *lpSize);
    DebMsgOut(debMsgWork);
#endif

    if( gpCbtPrnInfo == NULL) {
        Ret = EPCBT_ERR_CBTNOTOPEN;
    } else {
        if ( gpCbtPrnInfo->Mode == CBT_MODE_CBTON ) {
            if ( (SID == 0x00) || (SID == 0xff) ) {
                Ret = EPCBT_ERR_PARAM;
            } else {
                for ( lp1 = 1; ; lp1++ ) {
                    if ( lp1 >= CBT_MAX_CH ) {
                        Ret = EPCBT_ERR_CHNOTOPEN;
                        break;
                    }
                    ChPtr = &gpCbtPrnInfo->Chinfo[lp1];
                    if ( ChPtr->No == SID )
                        break;
                }
            }
        } else {
            Ret = EPCBT_ERR_CBTNOTOPEN;
        }
    }
    if ( Ret == EPCBT_ERR_NONE ) {
        if( (ChPtr->PtSsize - 6) < *lpSize)
            SendMaxSize = ChPtr->PtSsize - 6;
        else
            SendMaxSize = *lpSize;
        loopSize = *lpSize;
        *lpSize  = 0;
        while(loopSize) {

            if ( ChPtr->CreditP == 0 ) {
                Credit = CbtCreditReqCommand(fd, gpCbtPrnInfo, SID, CBT_CREDIT_LPT, 0xffff);
                if ( Credit < 0 )
                    Ret = (EPS_INT16)Credit;
                else if ( Credit == 0 )
                    Ret = EPCBT_ERR_FNCDISABLE;
                else
                    ChPtr->CreditP = Credit;
            }

            if ( Ret != EPCBT_ERR_NONE )
                break;

            if(SendMaxSize < loopSize)
                Size = SendMaxSize;
            else
                Size = loopSize;
            Count = CbtDataWrite( fd, gpCbtPrnInfo, SID, &lpBuffer[*lpSize], Size );
            ChPtr->CreditP -= 1;
            if ( Count >= 0 ) {
                *lpSize  += Count;
                loopSize -= Count;
            } else {
                Ret = (EPS_INT16)Count;
                break;
            }
        }
    }

#if    CBT_DBG_ERROR
    if( Ret != EPCBT_ERR_NONE ) {
        sprintf(debMsgWork, " <- CBT_Write Ret[%d] CH[%x] Size[%d]\r\n",Ret,SID,*lpSize);
        DebMsgOut(debMsgWork);
    }
#endif

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_Write (E)    0x%02x(%04d)\r\n", SID, *lpSize);
    DebMsgOut(debMsgWork);
#endif

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   EPCBT_Read                                                          */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* SID          EPS_UINT8           I: Socket ID                                        */
/* lpBuffer     EPS_UINT8*          O: Pointer to buffer for read data                  */
/* lpSize       EPS_INT32*          O: Pointer to variable for read data size           */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_CBTNOTOPEN            - CBT is not opened                             */
/*      EPCBT_ERR_PARAM                 - Parameter error                               */
/*      EPCBT_ERR_CHNOTOPEN             - Channel is not opened                         */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_CREDITOVF             - Credit over flow                              */
/*      EPCBT_ERR_CMDDENIED             - Command is denied (Channel is not opened)     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*      EPCBT_ERR_READERROR             - Data read error                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Receive the data from printer.                                                  */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    EPCBT_Read (

		EPS_FILEDSC	fd,
        EPS_UINT8   SID,
        EPS_UINT8*  lpBuffer,
        EPS_INT32*  lpSize
        
){
    EPS_INT16       Ret   = EPCBT_ERR_NONE;
    EPS_INT32       Size  = 0;
    EPS_INT32       lp1;
    CBTS_CHINFO*    ChPtr = NULL;

	EPS_LOG_FUNCIN;

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_Read (S)    0x%02x(%04d)\r\n", SID, *lpSize);
    DebMsgOut(debMsgWork);
#endif

    if( gpCbtPrnInfo == NULL) {
        Ret = EPCBT_ERR_CBTNOTOPEN;
    } else {
        if ( gpCbtPrnInfo->Mode == CBT_MODE_CBTON ) {
            if ( (SID == 0x00) || (SID == 0xff) ) {
                Ret = EPCBT_ERR_PARAM;
            } else {
                for ( lp1 = 1; ; lp1++ ) {
                    if ( lp1 >= CBT_MAX_CH ) {
                        Ret = EPCBT_ERR_CHNOTOPEN;
                        break;
                    }
                    ChPtr = &gpCbtPrnInfo->Chinfo[lp1];
                    if ( ChPtr->No == SID )
                        break;
                }
            }
        } else {
            Ret = EPCBT_ERR_CBTNOTOPEN;
        }
    }
    if ( Ret == EPCBT_ERR_NONE ) {
        if ( lpBuffer == NULL ) {
            if ( ChPtr->CreditH == 0 ) {
                Ret = CbtCreditCommand( fd, gpCbtPrnInfo, SID, 1 );
                if ( Ret == EPCBT_ERR_NONE )
                    ChPtr->CreditH += 1;
            }
            if ( Ret == EPCBT_ERR_NONE ) {
                Ret     = CbtDataRead( fd, gpCbtPrnInfo, SID, NULL, &Size );
                *lpSize = Size;
            }
        } else {
            Size    = *lpSize;
            Ret     = CbtDataRead( fd, gpCbtPrnInfo, SID, lpBuffer, &Size );
            *lpSize = Size;
        }
    }

#if    CBT_DBG_ERROR
    if( Ret != EPCBT_ERR_NONE ) {
        sprintf(debMsgWork, " <- CBT_Read Ret[%d] CH[%x] Size[%d]\r\n",Ret,SID,*lpSize);
        DebMsgOut(debMsgWork);
    }
#endif

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_Read (E)    0x%02x(%04d)\r\n", SID, *lpSize);
    DebMsgOut(debMsgWork);
#endif

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   EPCBT_GetSocketID                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pSID         EPS_UINT8 *         I: Socket ID                                        */
/* pName        EPS_UINT8 *         I: Parameter Name                                   */
/* Size         EPS_INT32           I: Size of socket ID                                */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_CBTNOTOPEN            - Port is not open                              */
/*      EPCBT_ERR_PARAM                 - Parameter error                               */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_NOSERVICE             - The channel does not support the requested    */
/*                                        service                                       */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Gets the socket ID.                                                             */
/*                                                                                      */
/*******************************************|********************************************/
#if LCOMSW_CBT_SOCKETFUNC_ON
static EPS_INT16    EPCBT_GetSocketID (

		EPS_FILEDSC	fd,
        EPS_UINT8*  pSID,
        EPS_UINT8*  pName,
        EPS_INT32   Size

){
    EPS_INT16   Ret      = EPCBT_ERR_NONE;
    EPS_UINT8   SocketID = 0;

	EPS_LOG_FUNCIN;

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_GetSocketID (S)\r\n");
    DebMsgOut(debMsgWork);
#endif

    if( gpCbtPrnInfo == NULL) {
        Ret = EPCBT_ERR_CBTNOTOPEN;
    } else {
        if ( gpCbtPrnInfo->Mode == CBT_MODE_CBTON ) {
            if ( (pSID == NULL) || (pName == NULL) ) {
                Ret = EPCBT_ERR_PARAM;
            }
        } else {
            Ret = EPCBT_ERR_CBTNOTOPEN;
        }
    }
    if ( Ret == EPCBT_ERR_NONE ) {
        Ret = CbtGetSocketIDCommand( fd, gpCbtPrnInfo, pName, Size, &SocketID );
        if ( Ret == EPCBT_ERR_NONE )
            *pSID = SocketID;
    }

#if    CBT_DBG_ERROR
    if( Ret != EPCBT_ERR_NONE ) {
        sprintf(debMsgWork, " <- CBT_GetSocketID Ret[%d] SID[%02x]\r\n", Ret,*pSID);
        DebMsgOut(debMsgWork);
    }
#endif

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"EPCBT_GetSocketID (E)\r\n");
    DebMsgOut(debMsgWork);
#endif

    EPS_RETURN( Ret );
}
#endif  /* def LCOMSW_CBT_SOCKETFUNC_ON */

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtWriteRtn                                                         */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Printer information                              */
/* Buff         const EPS_UINT8*    I: Pointer to the write data buffer                 */
/* BuffLen      EPS_INT32           I: Size of the write data buffer                    */
/* Cnt          EPS_INT32*          O: Write data size                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_WRITEERROR            - Write error                                   */
/*                                                                                      */
/* Description:                                                                         */
/*      Sends the data to the printer.                                                  */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    CbtWriteRtn (

		EPS_FILEDSC			fd,
        CBTS_PRNINFO*       pCbtPrnInfo,
        const EPS_UINT8*    Buff,
        EPS_INT32           BuffLen,
        EPS_INT32*          Cnt

){
    EPS_INT32   Ans;
#ifdef    CBT_DBG_IO
    EPS_INT32   lp1;
#endif

	EPS_LOG_FUNCIN;

    Ans = ( pCbtPrnInfo->pCbtWRtnFnc )( fd, (EPS_UINT8 *)Buff, BuffLen, Cnt );

#ifdef    CBT_DBG_IO
    sprintf(debMsgWork,"  * Write * Size=%4d(%4d) Data= ",BuffLen, *Cnt);
    DebMsgOut(debMsgWork); 
    for (lp1 = 0; (lp1 < *Cnt); lp1++) {
        sprintf(debMsgWork,"%02X ",Buff[lp1]);
        DebMsgOut(debMsgWork);
    }
    DebMsgOut("\r\n");
#endif

	if ( Ans == 0 ){
        EPS_RETURN( EPCBT_ERR_NONE );
	}else{
        EPS_RETURN( EPCBT_ERR_WRITEERROR );
	}
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtReadRtn                                                          */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Printer information                              */
/* SID          EPS_UINT8           I: Socket ID                                        */
/* lpBuffer     EPS_UINT8*          I: Pointer to the receive data buffer               */
/* BuffSize     EPS_INT32           I: Receive data buffer size                         */
/* Cnt          EPS_INT32 *         O: Receive data size                                */
/* DummyRead    EPS_UINT8           I: Flag for the dummy read                          */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_PARAM                 - Parameter error                               */
/*      EPCBT_ERR_READERROR             - Read data error                               */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Recives the data from printer.                                                  */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    CbtReadRtn (

		EPS_FILEDSC		fd,
        CBTS_PRNINFO*   pCbtPrnInfo,
        EPS_UINT8       SID,
        EPS_UINT8*      lpBuffer,
        EPS_INT32       BuffSize,
        EPS_INT32*      Cnt,
        EPS_UINT8       DummyRead

){
    EPS_INT16   Ret    = EPCBT_ERR_NONE;
    EPS_INT32   Ans;
    EPS_INT32   Count  = 0;
    EPS_INT32   lp1;
    EPS_INT32   PSize  = 0;
    EPS_INT32   Top;
    EPS_UINT8   RxSID;
    CBTS_CHINFO *ChSer = NULL;
    CBTS_CHINFO *ChPtr = NULL;
    EPS_UINT8   *wRBuff;
    EPS_INT32   nDlySpan = 10;		/* first 10 ms */
    EPS_INT32   nDlyTotal = 2000;	/* total 2000 ms */

	EPS_LOG_FUNCIN;

    wRBuff = pCbtPrnInfo->pCbtReadRtnBuff;
    if ( DummyRead == CBT_READ_DMY ) {
        Ans = ( pCbtPrnInfo->pCbtRRtnFnc )( fd, wRBuff, CBT_MAX_RTMP, &Count );
        
        *Cnt = Count;
        if ( Ans != 0 ) {
            Ret = EPCBT_ERR_READERROR;
        }
#ifdef    CBT_DBG_IO
        sprintf(debMsgWork,"  * Read  * Size=%4d(%4d) Data= ",CBT_MAX_RTMP, Count);
        DebMsgOut(debMsgWork);
        for (lp1 = 0; (lp1 < Count); lp1++) {
            sprintf(debMsgWork,"%02X ",wRBuff[lp1]);
            DebMsgOut(debMsgWork);
        }
        DebMsgOut("\r\n");
#endif

#ifdef    CBT_DBG_READ
    sprintf(debMsgWork,"<CBT Dummy CbtReadRtn> Req-SID[%02x] Size[%d]\r\n",SID, *Cnt);
    DebMsgOut(debMsgWork);
    for (lp1 = 0; lp1 < *Cnt; lp1++) {
        sprintf(debMsgWork,"%02X ",wRBuff[lp1]);
        DebMsgOut(debMsgWork);
    }
    DebMsgOut("\r\n");
#endif
        EPS_RETURN( Ret );
    }


    for ( lp1 = 0; ; lp1++ ) {
		if ( lp1 >= CBT_MAX_CH ){
            EPS_RETURN( EPCBT_ERR_PARAM );
		}

        ChSer = &pCbtPrnInfo->Chinfo[lp1];
        if ( ChSer->No == SID )
            break;
    }

	while(nDlyTotal > 0){
        if ( ChSer->RTmpCnt > 0 ) {
            PSize = ChSer->RTmpBuff[(ChSer->RTmpTop+3) % CBT_MAX_RTMP]
                        + (ChSer->RTmpBuff[(ChSer->RTmpTop+2) % CBT_MAX_RTMP] << 8);

            if ( BuffSize < PSize ) {
                Ret = EPCBT_ERR_PARAM;
                break;
            }
            for ( lp1 = 0; lp1 < PSize; lp1++ ) {
                lpBuffer[lp1]  = ChSer->RTmpBuff[ChSer->RTmpTop];
                ChSer->RTmpTop = (ChSer->RTmpTop+1) % CBT_MAX_RTMP;
                ChSer->RTmpCnt = (ChSer->RTmpCnt-1);
            }
            *Cnt = PSize;
            break;
        }


		Ans = ( pCbtPrnInfo->pCbtRRtnFnc )( fd, wRBuff, CBT_MAX_RTMP, &Count );

#ifdef    CBT_DBG_IO
        sprintf(debMsgWork,"  * Read  * Size=%4d(%4d) Data= ",CBT_MAX_RTMP, Count);
        DebMsgOut(debMsgWork);
        for (lp1 = 0; (lp1 < Count); lp1++) {
            sprintf(debMsgWork,"%02X ",wRBuff[lp1]);
            DebMsgOut(debMsgWork);
        }
        DebMsgOut("\r\n");
#endif

        if ( Count != 0 ) {
            Top = 0;
            for ( ;; ) {
                PSize = wRBuff[Top+3] + (wRBuff[Top+2] << 8);
                if ( ( Count - Top < 4 ) || ( PSize == 0 ) || ( PSize > Count - Top ) ) {
                    Ret = EPCBT_ERR_MULFORMEDPACKET;
                    break;
                }
                RxSID = wRBuff[Top];

                for ( lp1 = 0; ; lp1++ ) {
                    if ( lp1 >= CBT_MAX_CH ) {
                        Ret = EPCBT_ERR_UEREPLY;
                        break;
                    }
                    ChPtr = &pCbtPrnInfo->Chinfo[lp1];

                    /* Match channel ? */
                    if ( ChPtr->No == RxSID )
                        break;
                }

                if ( Ret == EPCBT_ERR_NONE ) {

                    ChPtr->CreditH -= 1;

                    for ( lp1 = 0; lp1 < PSize; lp1++ ) {
                        ChPtr->RTmpBuff[ChPtr->RTmpEnd] = wRBuff[Top+lp1];
                        ChPtr->RTmpEnd = (ChPtr->RTmpEnd+1) % CBT_MAX_RTMP;
                        ChPtr->RTmpCnt = (ChPtr->RTmpCnt+1);
                    }

                    Top += PSize;
                    if ( Top >= Count )
                        break;
                } else {
                    break;
                }
            }
            continue;
        } else {
            if ( Ans != 0 ) {
                Ret = EPCBT_ERR_READERROR;
                break;
            }
        }

        /* Wait */
        serDelayThread(nDlySpan, &epsCmnFnc);

		nDlyTotal -= nDlySpan;
		if(nDlySpan < 200){
			nDlySpan += nDlySpan/2;
			if(nDlySpan > 200){
				nDlySpan = 200;		/* max 200ms */
			}
		}
	}

    if ( nDlyTotal < 0 )
        Ret = EPCBT_ERR_READERROR;

#ifdef    CBT_DBG_READ
    sprintf(debMsgWork,"<CBT CbtReadRtn> Req-SID[%02x] Size[%d]\r\n",SID, *Cnt);
    DebMsgOut(debMsgWork);
    for (lp1 = 0; lp1 < *Cnt; lp1++) {
        sprintf(debMsgWork,"%02X ",lpBuffer[lp1]);
        DebMsgOut(debMsgWork);
    }
    DebMsgOut("\r\n");
#endif

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtDummyRead                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Printer information                              */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Try to read data from printer.                                                  */
/*                                                                                      */
/*******************************************|********************************************/
static void     CbtDummyRead (

		EPS_FILEDSC		fd,
        CBTS_PRNINFO*	pCbtPrnInfo

){
    EPS_INT32 i = 0;
    EPS_INT32 Cnt = 0;

	EPS_LOG_FUNCIN;

    for ( i = 0; i < CBT_DUMYREAD_MAX ;i++ ) {
        if ( CbtReadRtn( fd, pCbtPrnInfo, CBT_SID_MAIN, NULL, 0, &Cnt, CBT_READ_DMY ) == EPCBT_ERR_NONE ) {
            if ( Cnt == 0 )
                break;
        } else {
            break;
        }
    }

    EPS_RETURN_VOID;
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtChDummyRead                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Printer information                              */
/* ChPtr        CBTS_CHINFO*        I: Channel                                          */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Try to read by using <ChPtr>.                                                   */
/*                                                                                      */
/*******************************************|********************************************/
static void     CbtChDummyRead (

		EPS_FILEDSC		fd,
        CBTS_PRNINFO*   pCbtPrnInfo,
        CBTS_CHINFO*    ChPtr

){
    EPS_INT16   Ret;
    EPS_INT32   Size;

	EPS_LOG_FUNCIN;

    do {
        Size = 0;
        if ( ChPtr->CreditH == 0 ) {
            Ret = CbtCreditCommand( fd, pCbtPrnInfo, ChPtr->No, 1 );
            if ( Ret == EPCBT_ERR_NONE )
                ChPtr->CreditH += 1;
            else
                break;
        }
        Ret = CbtReadRtn( fd, pCbtPrnInfo, ChPtr->No, NULL, 0, &Size, CBT_READ_DMY );
        if(Size)
            ChPtr->CreditH -= 1;
        Size -= 6;
    } while(Size > 0);

    EPS_RETURN_VOID;
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtReplyCheck                                                       */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* Reply        EPS_UINT8*          I: Reply data                                       */
/* ReplySz      EPS_INT32           I: Reply data size                                  */
/* RpyType      EPS_UINT8           I: Reply data type                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*                                                                                      */
/* Description:                                                                         */
/*     Checks the CBT command replys.                                                   */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    CbtReplyCheck (

        EPS_UINT8*  Reply,
        EPS_INT32   ReplySz,
        EPS_UINT8   RpyType

){
    EPS_INT16    Ret = EPCBT_ERR_NONE;

	EPS_LOG_FUNCIN;

	if ( ReplySz == 0 ){
        EPS_RETURN( EPCBT_ERR_NOREPLY );
	}

	if( ( Reply[0] != 0x00 ) || ( Reply[1] != 0x00 ) || ( Reply[2] != 0x00 ) ){
        EPS_RETURN( EPCBT_ERR_MULFORMEDPACKET );
	}

    switch ( Reply[6] ){
        case    CBT_RPY_INIT:
            if (Reply[3] != CBT_RPY_SIZE_INIT)
                Ret = EPCBT_ERR_MULFORMEDPACKET;
            else if ( RpyType != CBT_CMD_INIT )
                Ret = EPCBT_ERR_UEREPLY;
            break;

        case    CBT_RPY_OPENCHANNEL:
            if (Reply[3] != CBT_RPY_SIZE_OPENCHANNEL)
                Ret = EPCBT_ERR_MULFORMEDPACKET;
            else if ( RpyType != CBT_CMD_OPENCHANNEL )
                Ret = EPCBT_ERR_UEREPLY;
            break;

        case    CBT_RPY_CLOSECHANNEL:
            if (Reply[3] != CBT_RPY_SIZE_CLOSECHANNEL)
                Ret = EPCBT_ERR_MULFORMEDPACKET;
            else if ( RpyType != CBT_CMD_CLOSECHANNEL )
                Ret = EPCBT_ERR_UEREPLY;
            break;

        case    CBT_RPY_CREDIT:
            if (Reply[3] != CBT_RPY_SIZE_CREDIT)
                Ret = EPCBT_ERR_MULFORMEDPACKET;
            else if ( RpyType != CBT_CMD_CREDIT )
                Ret = EPCBT_ERR_UEREPLY;
            break;

        case    CBT_RPY_CREDITREQUEST:
            if (Reply[3] != CBT_RPY_SIZE_CREDITREQUEST)
                Ret = EPCBT_ERR_MULFORMEDPACKET;
            else if ( RpyType != CBT_CMD_CREDITREQUEST )
                Ret = EPCBT_ERR_UEREPLY;
            break;

        case    CBT_RPY_DEBIT:
        case    CBT_RPY_DEBITREQUEST:
        case    CBT_RPY_CONFIGSOCKET:
            break;

        case    CBT_RPY_EXIT:
            if (Reply[3] != CBT_RPY_SIZE_EXIT)
                Ret = EPCBT_ERR_MULFORMEDPACKET;
            else if ( RpyType != CBT_CMD_EXIT )
                Ret = EPCBT_ERR_UEREPLY;
            break;

        case    CBT_RPY_GETSOCKETID:
            if ( RpyType != CBT_CMD_GETSOCKETID )
                Ret = EPCBT_ERR_UEREPLY;
            break;

        case    CBT_RPY_GETSERVICENAME:
            break;

        case    CBT_RPY_EPSONPACKING:
            if (Reply[3] != CBT_RPY_SIZE_EPSONPACKING)
                Ret = EPCBT_ERR_MULFORMEDPACKET;
            else if ( RpyType != CBT_CMD_EPSONPACKING )
                Ret = EPCBT_ERR_UEREPLY;
            break;

        default:
            Ret = EPCBT_ERR_MULFORMEDPACKET;
    }

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtEpsonPacking                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Printer information                              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_CBTDISABLE            - Fail to CBT communication                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Sends EpsonPackingCommand and receives reply.                                   */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    CbtEpsonPacking (

		EPS_FILEDSC		fd,
        CBTS_PRNINFO*	pCbtPrnInfo

){
    EPS_INT16   Ret     = EPCBT_ERR_NONE;
    EPS_INT16   Ans;
    EPS_INT32   Cnt     = 0;
    EPS_INT32   BuffLen = CBT_CMD_SIZE_EPSONPACKING;
    EPS_INT32   RSize   = 0;
    static const EPS_UINT8 Buff[] = {
                                     0x00,0x00,0x00,0x1b,0x01,0x40,0x45,0x4a,0x4c,0x20,
                                     0x31,0x32,0x38,0x34,0x2e,0x34,0x0a,0x40,0x45,0x4a,
                                     0x4c,0x0a,0x40,0x45,0x4a,0x4c,0x0a
                                    };
    EPS_UINT8   RBuff[CBT_CMDREPLY_BUFFSZ];
    
	EPS_LOG_FUNCIN;

    memset(RBuff, 0, CBT_CMDREPLY_BUFFSZ);

    Ans = CbtWriteRtn( fd, pCbtPrnInfo, Buff, BuffLen, &Cnt );
    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_CBTDISABLE;
    else {
        if ( Cnt != BuffLen )
            Ret = EPCBT_ERR_CBTDISABLE;
    }

	if ( Ret != EPCBT_ERR_NONE ){
        EPS_RETURN( Ret );    /* return error end */
	}

    Ans = CbtReadRtn( fd, pCbtPrnInfo, CBT_SID_MAIN, RBuff, CBT_CMDREPLY_BUFFSZ, &RSize, CBT_READ_NORMAL );
    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_CBTDISABLE;    /* no receive */
    else {
        Ret = CbtReplyCheck( RBuff, RSize, CBT_CMD_EPSONPACKING );
        if ( Ret == EPCBT_ERR_NONE ) {
            if( ( RBuff[7] != 0x00 ) && ( RBuff[7] != 0x02 ) )
                Ret = EPCBT_ERR_CBTDISABLE;
        }
    }

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtInitCommand                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Printer information                              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_INITDENIED            - Printer cannot start CBT mode                 */
/*      EPCBT_ERR_VERSION               - Incompatible version                          */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Sends init command and receives reply.                                          */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    CbtInitCommand (

		EPS_FILEDSC		fd,
        CBTS_PRNINFO*   pCbtPrnInfo

){
    EPS_INT16   Ret     = EPCBT_ERR_NONE;
    EPS_INT16   Ans;
    EPS_INT32   Cnt     = 0;
    EPS_INT32   BuffLen = CBT_CMD_SIZE_INIT;
    EPS_INT32   RSize   = 0;
    static const EPS_UINT8 Buff[] = {0x00,0x00,0x00,CBT_CMD_SIZE_INIT,0x01,0x00,0x00,0x10};
    EPS_UINT8   RBuff[CBT_CMDREPLY_BUFFSZ];
    
	EPS_LOG_FUNCIN;

    memset(RBuff, 0, CBT_CMDREPLY_BUFFSZ);

    Ans = CbtWriteRtn( fd, pCbtPrnInfo, Buff, BuffLen, &Cnt );
    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_FATAL;
    else {
        if ( Cnt != BuffLen )
            Ret = EPCBT_ERR_FATAL;
    }

    if ( Ret != EPCBT_ERR_NONE )
        EPS_RETURN( Ret );    /* return error end */

    Ans = CbtReadRtn( fd, pCbtPrnInfo, CBT_SID_MAIN, RBuff, CBT_CMDREPLY_BUFFSZ, &RSize, CBT_READ_NORMAL );
    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_FATAL;        /* No receive */
    else {
        Ret = CbtReplyCheck( RBuff, RSize, CBT_CMD_INIT );
        if ( Ret == EPCBT_ERR_NONE ) {
            if ( RBuff[7] != 0x00 ) {
                if ( RBuff[7] == 0x01 )
                    Ret = EPCBT_ERR_INITDENIED;
                else if ( RBuff[7] == 0x02 )
                    Ret = EPCBT_ERR_VERSION;
                else if ( RBuff[7] == 0x0b )
                    Ret = EPCBT_ERR_INITFAILED;
                else
                    Ret = EPCBT_ERR_UERESULT;
            }
        }
    }

    EPS_RETURN( Ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtOpenChannelCommand()                                             */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Port information                                 */
/* SID          EPS_UINT8           I: Socket ID                                        */
/* pPtSsz       EPS_INT32*          O: Primary to secondary packet size                 */
/* pStPsz       EPS_INT32*          O: Secondary to primary packet size                 */
/* CreditReq    EPS_INT32           I: Credit Requested                                 */
/* CreditMax    EPS_INT32           I: Maximum Outstanding Credit                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      Credit number                   - Data packet number that is able to recieve    */
/*                                        by throughing <SID>.                          */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*      EPCBT_ERR_RESOURCE              - Insufficient printer resources for OpenChannel*/
/*      EPCBT_ERR_OPENCHANNEL           - Not used by EPSON                             */
/*      EPCBT_ERR_CHNOTSUPPORT          - The channel is not supported                  */
/*      EPCBT_ERR_PACKETSIZE            - Invalid packet size.                          */
/*                                        (0x0001 ? 0x0005 bytes)                       */
/*      EPCBT_ERR_NULLPACKETSZ          - Packet size is 0x0000 in both directions.     */
/*                                        No data can be transferred. Channel was       */
/*                                        not opened                                    */
/*                                                                                      */
/* Description:                                                                         */
/*      Sends the open channel command and receives the reply.                          */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT32    CbtOpenChannelCommand (

		EPS_FILEDSC		fd,
        CBTS_PRNINFO*   pCbtPrnInfo,
        EPS_UINT8       SID,
        EPS_UINT32*     pPtSsz,
        EPS_UINT32*     pStPsz,
        EPS_UINT32      CreditReq,
        EPS_UINT32      CreditMax

){
    EPS_INT32   Ret     = EPCBT_ERR_NONE;
    EPS_INT16   Ans;
    EPS_INT32   Cnt     = 0;
    EPS_INT32   BuffLen = CBT_CMD_SIZE_OPENCHANNEL;
    EPS_INT32   RSize   = 0;
    static EPS_UINT8    Buff[] = {0x00,0x00,0x00,CBT_CMD_SIZE_OPENCHANNEL,0x01,0x00,0x01,0x00,0x00,0,0,0,0,0,0,0,0};
    EPS_UINT8   RBuff[CBT_CMDREPLY_BUFFSZ];
    
	EPS_LOG_FUNCIN;

    memset(RBuff, 0, CBT_CMDREPLY_BUFFSZ);

    Buff[7] = SID;
    Buff[8] = SID;

    CbtPutBigEndianByte2(*pPtSsz,&Buff[9]);
    CbtPutBigEndianByte2(*pStPsz,&Buff[11]);
    CbtPutBigEndianByte2(CreditReq,&Buff[13]);
    CbtPutBigEndianByte2(CreditMax,&Buff[15]);

    Ans = CbtWriteRtn( fd, pCbtPrnInfo, Buff, BuffLen, &Cnt );
    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_FATAL;
    else {
        if ( Cnt != BuffLen )
            Ret = EPCBT_ERR_FATAL;
    }

	if ( Ret != EPCBT_ERR_NONE ){
        EPS_RETURN( Ret );    /* return error end */
	}

    Ans = CbtReadRtn( fd, pCbtPrnInfo, CBT_SID_MAIN, RBuff, CBT_CMDREPLY_BUFFSZ, &RSize, CBT_READ_NORMAL );
    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_FATAL;        /* No receive */
    else {
        Ret = CbtReplyCheck( RBuff, RSize, CBT_CMD_OPENCHANNEL );
        if ( Ret == EPCBT_ERR_NONE ) {
            if ( (RBuff[7] != 0x00) && (RBuff[7] != 0x06) ) {
                if ( RBuff[7] == 0x04 )
                    Ret = EPCBT_ERR_RESOURCE;
                else if ( RBuff[7] == 0x05 )
                    Ret = EPCBT_ERR_OPENCHANNEL;
                else if ( RBuff[7] == 0x09 )
                    Ret = EPCBT_ERR_CHNOTSUPPORT;
                else if ( RBuff[7] == 0x0C )
                    Ret = EPCBT_ERR_PACKETSIZE;
                else if ( RBuff[7] == 0x0D )
                    Ret = EPCBT_ERR_NULLPACKETSZ;
                else
                    Ret = EPCBT_ERR_UERESULT;
            }
        }
    }

	if ( Ret != EPCBT_ERR_NONE ){
        EPS_RETURN( Ret );    /* error end */
	}

    *pPtSsz = (EPS_UINT32)((RBuff[10] << 8) + RBuff[11]);
    *pStPsz = (EPS_UINT32)((RBuff[12] << 8) + RBuff[13]);
    Ret     = (RBuff[14] << 8) + RBuff[15];

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtCloseChannelCommand()                                            */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Port information                                 */
/* SID          EPS_UINT8           I: Socket ID                                        */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_CLOSEDENIED           - Close Channel is denied                       */
/*      EPCBT_ERR_CMDDENIED             - Command is denied (Channel is not opened)     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*                                                                                      */
/* Description:                                                                         */
/*      Send CloseChannel and receive reply.                                            */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    CbtCloseChannelCommand (

 		EPS_FILEDSC		fd,
		CBTS_PRNINFO*   pCbtPrnInfo,
        EPS_UINT8       SID

){
    EPS_INT16   Ret     = EPCBT_ERR_NONE;
    EPS_INT16   Ans;
    EPS_INT32   Cnt     = 0;
    EPS_INT32   BuffLen = CBT_CMD_SIZE_CLOSECHANNEL;
    EPS_INT32   RSize   = 0;
    static EPS_UINT8    Buff[] = {0x00,0x00,0x00,CBT_CMD_SIZE_CLOSECHANNEL,0x01,0x00,0x02,0x00,0x00,0x00};
    EPS_UINT8   RBuff[CBT_CMDREPLY_BUFFSZ];
    
	EPS_LOG_FUNCIN;

    memset(RBuff, 0, CBT_CMDREPLY_BUFFSZ);

    Buff[7] = SID;
    Buff[8] = SID;
    Ans = CbtWriteRtn( fd, pCbtPrnInfo, Buff, BuffLen, &Cnt );
    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_FATAL;
    else {
        if ( Cnt != BuffLen )
            Ret = EPCBT_ERR_FATAL;
    }

	if ( Ret != EPCBT_ERR_NONE ){
        EPS_RETURN( Ret );    /* return error end */
	}

    Ans = CbtReadRtn( fd, pCbtPrnInfo, CBT_SID_MAIN, RBuff, CBT_CMDREPLY_BUFFSZ, &RSize, CBT_READ_NORMAL );
    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_FATAL;        /* No receive */
    else{
        Ret = CbtReplyCheck( RBuff, RSize, CBT_CMD_CLOSECHANNEL );
        if ( Ret == EPCBT_ERR_NONE ) {
            if ( RBuff[7] != 0x00 ) {
                if ( RBuff[7] == 0x03 )
                    Ret = EPCBT_ERR_CLOSEDENIED;
                else if ( RBuff[7] == 0x08 )
                    Ret = EPCBT_ERR_CMDDENIED;
                else
                    Ret = EPCBT_ERR_UERESULT;
            }
        }
    }

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtCreditReqCommand()                                               */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Printer information                              */
/* SID          EPS_UINT8           I: SocketID                                         */
/* CreditReq    EPS_INT32           I: Credit Requested                                 */
/* MaxCredit    EPS_INT32           I: Maximum OutStanding Credit                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      Credit                          - Success                                       */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_CMDDENIED             - Command is denied (Channel is not opened)     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Request credits and return credit number.                                       */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT32    CbtCreditReqCommand (

        EPS_FILEDSC		fd,
		CBTS_PRNINFO*   pCbtPrnInfo,
        EPS_UINT8       SID,
        EPS_UINT32      CreditReq,
        EPS_UINT32      MaxCredit

){
    EPS_INT32       Ret     = EPCBT_ERR_NONE;
    EPS_INT16       Ans;
    EPS_INT32       Cnt     = 0;
    EPS_INT32       BuffLen = CBT_CMD_SIZE_CREDITREQUEST;
    EPS_INT32       RSize   = 0;
    static EPS_UINT8 Buff[] = {0x00,0x00,0x00,CBT_CMD_SIZE_CREDITREQUEST,0x01,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00};
    EPS_UINT8       RBuff[CBT_CMDREPLY_BUFFSZ];
    
	EPS_LOG_FUNCIN;

    memset(RBuff, 0, CBT_CMDREPLY_BUFFSZ);

    Buff[7] = SID;
    Buff[8] = SID;

    CbtPutBigEndianByte2(CreditReq,&Buff[9]);
    CbtPutBigEndianByte2(MaxCredit,&Buff[11]);

    Ans = CbtWriteRtn( fd, pCbtPrnInfo, Buff, BuffLen, &Cnt );
    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_FATAL;
    else {
        if ( Cnt != BuffLen )
            Ret = EPCBT_ERR_FATAL;
    }

	if ( Ret != EPCBT_ERR_NONE ){
        EPS_RETURN( Ret );    /* return error end */
	}

    Ans = CbtReadRtn( fd, pCbtPrnInfo, CBT_SID_MAIN, RBuff, CBT_CMDREPLY_BUFFSZ, &RSize, CBT_READ_NORMAL );
    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_FATAL;        /* No receive */
    else {
        Ret = CbtReplyCheck( RBuff, RSize, CBT_CMD_CREDITREQUEST );
        if ( Ret == EPCBT_ERR_NONE ) {
            if ( RBuff[7] != 0x00 ) {
                if ( RBuff[7] == 0x08 )
                    Ret = EPCBT_ERR_CMDDENIED;
                else
                    Ret = EPCBT_ERR_UERESULT;
            }
        }
    }

	if ( Ret != EPCBT_ERR_NONE ){
        EPS_RETURN( Ret );    /* return error end */
	}

    Ret = ( RBuff[11] + (RBuff[10] << 8) );

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtCreditCommand()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Printer information                              */
/* SID          EPS_UINT8           I: SocketID                                         */
/* Credit       EPS_INT32           I: Credit                                           */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_CREDITOVF             - Credit over flow                              */
/*      EPCBT_ERR_CMDDENIED             - Command is denied (Channel is not opened)     */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*      EPCBT_ERR_NOREPLY               - No reply from printer                         */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_UEREPLY               - Received reply for a command not issued       */
/*                                                                                      */
/* Description:                                                                         */
/*      Open the communication channel.                                                 */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    CbtCreditCommand (

		EPS_FILEDSC		fd,
        CBTS_PRNINFO*   pCbtPrnInfo,
        EPS_UINT8       SID,
        EPS_UINT32      Credit

){
    EPS_INT16       Ret     = EPCBT_ERR_NONE;
    EPS_INT16       Ans;
    EPS_INT32       Cnt     = 0;
    EPS_INT32       BuffLen = CBT_CMD_SIZE_CREDIT;
    EPS_INT32       RSize   = 0;
    static EPS_UINT8 Buff[] = {0x00,0x00,0x00,CBT_CMD_SIZE_CREDIT,0x01,0x00,0x03,0x00,0x00,0x00,0x00};
    EPS_UINT8       RBuff[CBT_CMDREPLY_BUFFSZ];
    
	EPS_LOG_FUNCIN;

    memset(RBuff, 0, CBT_CMDREPLY_BUFFSZ);

    Buff[7] = SID;
    Buff[8] = SID;

    CbtPutBigEndianByte2(Credit,&Buff[9]);

    Ans = CbtWriteRtn( fd, pCbtPrnInfo, Buff, BuffLen, &Cnt );
    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_FATAL;
    else {
        if ( Cnt != BuffLen )
            Ret = EPCBT_ERR_FATAL;
    }

	if ( Ret != EPCBT_ERR_NONE ){
        EPS_RETURN( Ret );    /* return error end */
	}

    Ans = CbtReadRtn( fd, pCbtPrnInfo, CBT_SID_MAIN, RBuff, CBT_CMDREPLY_BUFFSZ, &RSize, CBT_READ_NORMAL );

    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_FATAL;        /* No receive */
    else {
        Ret = CbtReplyCheck( RBuff, RSize, CBT_CMD_CREDIT );
        if ( Ret == EPCBT_ERR_NONE ) {
            if ( RBuff[7] != 0x00 ) {
                if ( RBuff[7] == 0x07 )
                    Ret = EPCBT_ERR_CREDITOVF;
                else if ( RBuff[7] == 0x08 )
                    Ret = EPCBT_ERR_CMDDENIED;
                else
                    Ret = EPCBT_ERR_UERESULT;
            }
        }
    }

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtGetSocketIDCommand                                               */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Printer information                              */
/* lpNamepName  EPS_UINT8 *         I: Parameter Name                                   */
/* Size         EPS_INT32           I: Size of socket ID                                */
/* SocketID     EPS_UINT8 *         O: SocketID                                         */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_FATAL                 - Fatal communication error                     */
/*      EPCBT_ERR_MULFORMEDPACKET       - Received invalid packet                       */
/*      EPCBT_ERR_NOSERVICE             - The channel does not support the requested    */
/*                                        service                                       */
/*      EPCBT_ERR_UERESULT              - Undefined result value                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Sends GetSocketID and receives reply.                                           */
/*                                                                                      */
/*******************************************|********************************************/
#if LCOMSW_CBT_SOCKETFUNC_ON
static EPS_INT16    CbtGetSocketIDCommand (

		EPS_FILEDSC		fd,
        CBTS_PRNINFO*   pCbtPrnInfo,
        EPS_UINT8*      lpName,
        EPS_INT32       Size,
        EPS_UINT8*      SocketID

){
/*** Declare Variable Local to Routine                                                  */
    EPS_INT16   Ret   = EPCBT_ERR_NONE;
    EPS_INT16   Ans;
    EPS_INT32   Cnt   = 0;
    EPS_INT32   BuffLen;
    EPS_INT32   lp1;
    EPS_INT32   RSize = 0;
    EPS_UINT8   Buff[CBT_CMD_SIZE_GETSOCKETID + 40];
    EPS_UINT8   RBuff[CBT_CMDREPLY_BUFFSZ];

	EPS_LOG_FUNCIN;

/*** Initialize Local Variables                                                         */
    memset(Buff,  0, CBT_CMD_SIZE_GETSOCKETID + 40);
    memset(RBuff, 0, CBT_CMDREPLY_BUFFSZ);

    BuffLen = CBT_CMD_SIZE_GETSOCKETID + Size;

    Buff[0] = 0x00;
    Buff[1] = 0x00;
    Buff[2] = 0x00;
    Buff[3] = (EPS_UINT8)BuffLen;
    Buff[4] = 0x01;
    Buff[5] = 0x00;
    Buff[6] = 0x09;

/*** Sends GetSocketID and receives reply                                               */
    for ( lp1 = 0; lp1 < Size; lp1++ )
        Buff[7+lp1] = lpName[lp1];

    Ans = CbtWriteRtn( fd, pCbtPrnInfo, Buff, BuffLen, &Cnt );

    if ( Ans != EPCBT_ERR_NONE )
        Ret = EPCBT_ERR_FATAL;
    else {
        if (Cnt != BuffLen)
            Ret = EPCBT_ERR_FATAL;
    }

	if ( Ret != EPCBT_ERR_NONE ){
        EPS_RETURN( Ret );    /* return error end */
	}

    Ans = CbtReadRtn( fd, pCbtPrnInfo, CBT_SID_MAIN, RBuff, CBT_CMDREPLY_BUFFSZ, &RSize, CBT_READ_NORMAL );
    if ( Ans != EPCBT_ERR_NONE ) 
        Ret = EPCBT_ERR_FATAL;        /* No receive */
    else {
        Ret = CbtReplyCheck( RBuff, RSize, CBT_CMD_GETSOCKETID );
        if ( Ret == EPCBT_ERR_NONE ) {
            if ( RBuff[3] != (Size + CBT_RPY_SIZE_GETSOCKETID) )
                Ret = EPCBT_ERR_MULFORMEDPACKET;
            else if ( RBuff[7] != 0x00 ) {
                if ( RBuff[7] == 0x0A )
                    Ret = EPCBT_ERR_NOSERVICE;
                else
                    Ret = EPCBT_ERR_UERESULT;
            } else {
                *SocketID = RBuff[8];
            }
        }
    }

    EPS_RETURN( Ret );
}
#endif    /* def LCOMSW_CBT_SOCKETFUNC_ON */

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtDataWrite()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Printer information                              */
/* SID          EPS_UINT8           I: SocketID                                         */
/* lpBuffer     const EPS_UINT8*    I: Pointer to the write data buffer                 */
/* Size         EPS_INT32           I: Size of the write data buffer                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      Actually Write Size             - Success                                       */
/*      EPCBT_ERR_WRITEERROR            - Failed to write                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Send data to printer.                                                           */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT32    CbtDataWrite (

        EPS_FILEDSC			fd,
		CBTS_PRNINFO*       pCbtPrnInfo,
        EPS_UINT8           SID,
        const EPS_UINT8*    lpBuffer,
        EPS_INT32           Size

){
    EPS_INT32   Ret = EPCBT_ERR_NONE;
    EPS_INT16   Ans;
    EPS_INT32   Cnt = 0;
    EPS_UINT32  BuffLen;
    EPS_UINT32  lp1;
    EPS_UINT8*  Buff;

	EPS_LOG_FUNCIN;

    Buff = pCbtPrnInfo->pWriteBuff;
    *(Buff + 0) = SID;
    *(Buff + 1) = SID;

    /* check packet size and Control */
    if( (Size+6) > CBT_TXPSIZE)
        BuffLen = CBT_TXPSIZE;
    else
        BuffLen = (EPS_UINT32)(Size + 6);

    CbtPutBigEndianByte2(BuffLen, (Buff+2));
    *(Buff + 4) = 0;
    *(Buff + 5) = 0x01;            /* end data */

    Buff += 6;
    for ( lp1=6; lp1<BuffLen; lp1++ )
        *Buff++ = *lpBuffer++;

    Ans = CbtWriteRtn( fd, pCbtPrnInfo, pCbtPrnInfo->pWriteBuff, (EPS_INT32)BuffLen, &Cnt );

    Ret = Cnt - 6;
    if (Ret < 0)
        Ret = EPCBT_ERR_WRITEERROR;
    else
        if ( Ans != EPCBT_ERR_NONE )
            Ret = EPCBT_ERR_WRITEERROR;

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtDataRead                                                         */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* fd           EPS_FILEDSC         I: file discripter                                  */
/* pCbtPrnInfo  CBTS_PRNINFO*       I: Printer information                              */
/* SID          EPS_UINT8           I: Socket ID                                        */
/* lpBuffer     EPS_UINT8*          O: Pointer to buffer for read data                  */
/* Size         EPS_INT32*          O: Pointer to variable for read data size           */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_CHNOTOPEN             - Channel is not opened                         */
/*      EPCBT_ERR_READERROR             - Data read error                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Read data from printer.                                                         */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16 CbtDataRead (

		EPS_FILEDSC		fd,
        CBTS_PRNINFO*   pCbtPrnInfo,
        EPS_UINT8       SID,
        EPS_UINT8*      lpBuffer,
        EPS_INT32*      Size

){
    EPS_INT16       Ret   = EPCBT_ERR_NONE;
    EPS_INT16       Ans;
    EPS_INT32       Cnt   = 0;
    EPS_INT32       lp1;
    CBTS_CHINFO*    ChPtr = NULL;
    EPS_UINT8*      RBuff;
    EPS_UINT8*      SBuff;

 	EPS_LOG_FUNCIN;

   for ( lp1 = 0; ; lp1++ ) {
	   if ( lp1 >= CBT_MAX_CH ){
            EPS_RETURN( EPCBT_ERR_CHNOTOPEN );
	   }

        ChPtr = &pCbtPrnInfo->Chinfo[lp1];
        if ( ChPtr->No == SID )            /* channel open ? */
            break;
    }

    SBuff = ChPtr->ReadBuff;
    if ( lpBuffer == NULL ) {
        RBuff = pCbtPrnInfo->pCbtDataReadBuff;
        Ans = CbtReadRtn( fd, pCbtPrnInfo, SID, RBuff, CBT_MAX_RTMP, &Cnt, CBT_READ_NORMAL );
        if ( Cnt > 6 ) {
            Cnt -= 6;
            RBuff += 6;
            SBuff += ChPtr->ReadSize;
            for ( lp1 = 0; lp1 < Cnt; lp1++ )
                *SBuff++ = *RBuff++;

            ChPtr->ReadSize += Cnt;
        } else {
            if ( Ans != EPCBT_ERR_NONE )
                Ret = EPCBT_ERR_READERROR;
        }
        *Size = ChPtr->ReadSize;
    } else {
        RBuff = lpBuffer;
        /* Buffer Size Check */
        if ( *Size > ChPtr->ReadSize )
            *Size = ChPtr->ReadSize;
        for ( lp1 = 0; lp1 < *Size; lp1++ )
            *RBuff++ = *SBuff++;
        ChPtr->ReadSize = 0;
    }

    EPS_RETURN( Ret );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtMemAlloc                                                         */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* -            void                -                                                   */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPCBT_ERR_NONE                  - Success                                       */
/*      EPCBT_ERR_MEMORY                - Fail to memory allocation                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Allocates the memory for CBT modules.                                           */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT16    CbtMemAlloc (

        void

){
    EPS_INT32       lp1;
    CBTS_CHINFO     *ChPtr;

#if    LCOMSW_CBT_ALLOC_MEM
    gpCbtPrnInfo = (CBTS_PRNINFO*)EPS_ALLOC(sizeof(CBTS_PRNINFO));
    if(!gpCbtPrnInfo)
        return    EPCBT_ERR_MEMORY;
    memset(gpCbtPrnInfo, 0, sizeof(CBTS_PRNINFO));
#else    /* LCOMSW_CBT_ALLOC_MEM */
    gpCbtPrnInfo = &sgcbtprninfo;
#endif    /* LCOMSW_CBT_ALLOC_MEM */

    /* clear */
    gpCbtPrnInfo->pPortInfo        = NULL;
    gpCbtPrnInfo->pWriteBuff       = NULL;
    gpCbtPrnInfo->pCbtReadRtnBuff  = NULL;
    gpCbtPrnInfo->pCbtDataReadBuff = NULL;
    for ( lp1 = 0; lp1 < CBT_MAX_CH; lp1++ ) {
        ChPtr           = &gpCbtPrnInfo->Chinfo[lp1];
        ChPtr->No       = (EPS_UINT8)(lp1 == 0 ? CBT_SID_MAIN : 0);
        ChPtr->PtSsize  = 0;
        ChPtr->StPsize  = 0;
        ChPtr->CreditP  = 0;
        ChPtr->CreditH  = 0;
        ChPtr->ReadSize = 0;
        ChPtr->ReadBuff = NULL;
        ChPtr->RTmpBuff = NULL;
        ChPtr->RTmpTop  = 0;
        ChPtr->RTmpEnd  = 0;
        ChPtr->RTmpCnt  = 0;
    }

#if    LCOMSW_CBT_ALLOC_MEM
    gpCbtPrnInfo->pWriteBuff       = (EPS_UINT8*)EPS_ALLOC(CBT_TXPSIZE);
    if(!gpCbtPrnInfo->pWriteBuff)
        return    EPCBT_ERR_MEMORY;
    gpCbtPrnInfo->pCbtReadRtnBuff  = (EPS_UINT8*)EPS_ALLOC(CBT_MAX_RTMP);
    if(!gpCbtPrnInfo->pCbtReadRtnBuff)
        return    EPCBT_ERR_MEMORY;
    gpCbtPrnInfo->pCbtDataReadBuff = (EPS_UINT8*)EPS_ALLOC(CBT_MAX_RTMP);
    if(!gpCbtPrnInfo->pCbtDataReadBuff)
        return    EPCBT_ERR_MEMORY;
#else    /* LCOMSW_CBT_ALLOC_MEM */
    gpCbtPrnInfo->pWriteBuff       = (EPS_UINT8*)&sgwritebuff[0];
    gpCbtPrnInfo->pCbtReadRtnBuff  = (EPS_UINT8*)&sgcbtreadrtnbuff[0];
    gpCbtPrnInfo->pCbtDataReadBuff = (EPS_UINT8*)&sgcbtdatareadbuff[0];
#endif    /* LCOMSW_CBT_ALLOC_MEM */

    for ( lp1 = 0; lp1 < CBT_MAX_CH; lp1++ ) {
        ChPtr = &gpCbtPrnInfo->Chinfo[lp1];

#if    LCOMSW_CBT_ALLOC_MEM
        ChPtr->ReadBuff = (EPS_UINT8*)EPS_ALLOC(CBT_MAX_RTMP);
        if(!ChPtr->ReadBuff)
            return    EPCBT_ERR_MEMORY;
        ChPtr->RTmpBuff = (EPS_UINT8*)EPS_ALLOC(CBT_MAX_RTMP);
        if(!ChPtr->RTmpBuff)
            return    EPCBT_ERR_MEMORY;
#else    /* LCOMSW_CBT_ALLOC_MEM */
        ChPtr->ReadBuff = (EPS_UINT8*)&sgreadbuff[lp1][0];
        ChPtr->RTmpBuff = (EPS_UINT8*)&sgrtmpbuff[lp1][0];
#endif    /* LCOMSW_CBT_ALLOC_MEM */

    }

    return    EPCBT_ERR_NONE;
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtMemFree                                                          */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* -            void                -                                                   */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Frees the memory for CBT modules.                                               */
/*                                                                                      */
/*******************************************|********************************************/
static void     CbtMemFree (

        void

){
    EPS_INT32       lp1;
    CBTS_CHINFO     *ChPtr;

    if(!gpCbtPrnInfo) {
        return;
    }

    for ( lp1 = 0; lp1 < CBT_MAX_CH; lp1++ ) {
        ChPtr = &gpCbtPrnInfo->Chinfo[lp1];
#if    LCOMSW_CBT_ALLOC_MEM
        EPS_SAFE_RELEASE(ChPtr->ReadBuff);
        EPS_SAFE_RELEASE(ChPtr->RTmpBuff);
#else    /* LCOMSW_CBT_ALLOC_MEM */
        ChPtr->ReadBuff = NULL;
        ChPtr->RTmpBuff = NULL;
#endif    /* LCOMSW_CBT_ALLOC_MEM */

    }

#if    LCOMSW_CBT_ALLOC_MEM
    EPS_SAFE_RELEASE(gpCbtPrnInfo->pWriteBuff);
    EPS_SAFE_RELEASE(gpCbtPrnInfo->pCbtReadRtnBuff);
    EPS_SAFE_RELEASE(gpCbtPrnInfo->pCbtDataReadBuff);
    EPS_SAFE_RELEASE(gpCbtPrnInfo);
#else    /* LCOMSW_CBT_ALLOC_MEM */
    gpCbtPrnInfo->pWriteBuff       = NULL;
    gpCbtPrnInfo->pCbtReadRtnBuff  = NULL;
    gpCbtPrnInfo->pCbtDataReadBuff = NULL;
    gpCbtPrnInfo = NULL;
#endif    /* LCOMSW_CBT_ALLOC_MEM */

    return;
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CbtPutBigEndianByte2()                                              */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* value        EPS_INT32           I: same value                                       */
/* buf[2]       EPS_UINT8           I/O: Big endian value                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Change input value to the big endian value.                                     */
/*                                                                                      */
/*******************************************|********************************************/
static void     CbtPutBigEndianByte2 (

        EPS_UINT32  value,
        EPS_UINT8   *buf

){

/*** Change 2 bytes value to the big endianness                                 */
    EPS_UINT16 value2byte = (EPS_UINT16)value;

    buf[0] = (EPS_UINT8)((value2byte >> 8) & 0x00ff);
    buf[1] = (EPS_UINT8)((value2byte     ) & 0x00ff);

}

/*___________________________________  epson-cbt.c  ____________________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

