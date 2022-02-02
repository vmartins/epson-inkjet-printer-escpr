/*___________________________________  epson-cbt.h   ___________________________________*/

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
/*                             Epson CBT Module Definitions                             */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef _EPSON_CBT_H_
#define _EPSON_CBT_H_
#ifdef    __cplusplus
extern "C" {
#endif    /* def __cplusplus    */

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"
#include "epson-escpr-def.h"

/*------------------------------------- Data Types -------------------------------------*/
/*******************************************|********************************************/

/*----------------------------------  Generic Macros   ---------------------------------*/
/*******************************************|********************************************/

/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/

    /*** Socket ID                                                                      */
    /*** -------------------------------------------------------------------------------*/
#define CBT_CBTCH_DATA               0x40    /* CBT Data Channel                         */
#define CBT_CBTCH_CTRL               0x02    /* CBT Control Channel                      */

    /*** CBT Module Global Variables                                                    */
    /*** -------------------------------------------------------------------------------*/
#define CBT_MAX_CH                     3    /* Channel Count                            */
#define CBT_MAX_RTMP                 512    /* receive data work buffer size            */

#define CBT_SID_MAIN                0x00    /* Main Channel                             */
#define CBT_SID_CTRL                0x02    /* Control Channel                          */
#define CBT_SID_DATA                0x40    /* data channel (40H)                       */

#define CBT_TXPSIZE                 4096    /* JOB data packet size (4090+6)            */
#define CBT_RXPSIZE                  512    /* ReverseData packet size                  */

#define CBT_CREDIT_LPT            0x0080    /* Printer credit                           */
#define CBT_CREDIT_DEF            0x0008    /* Host Credit                              */

#define CBT_CMDREPLY_BUFFSZ           64    /* Command Reply Data Buffer size           */

#define CBT_READ_DMY                   0    /* Read Option: Dummy Read                  */
#define CBT_READ_NORMAL                1    /* Read Option: Normal Read                 */

    /*** CBT Mode                                                                       */
    /*** -------------------------------------------------------------------------------*/
#define CBT_MODE_CBTOFF                0    /* CBT Mode Off                             */
#define CBT_MODE_CBTON                 1    /* CBT Mode On                              */
#define CBT_MODE_REQOFF                2    /* Request Off                              */
#define CBT_MODE_CBTERR                3    /* CBT Error                                */

    /*** Command Packet Length                                                          */
    /*** -------------------------------------------------------------------------------*/
#define CBT_CMD_SIZE_INIT           0x08    /* Init Command                             */
#define CBT_CMD_SIZE_OPENCHANNEL    0x11    /* OpenChannel Command                      */
#define CBT_CMD_SIZE_CLOSECHANNEL   0x0A    /* CloseChannel Command                     */
#define CBT_CMD_SIZE_CREDIT         0x0B    /* Credit Command                           */
#define CBT_CMD_SIZE_CREDITREQUEST  0x0D    /* CreditRequest Command                    */
#define CBT_CMD_SIZE_EXIT           0x07    /* Exit Command                             */
#define CBT_CMD_SIZE_GETSOCKETID    0x07    /* GetSoketID Command (7-47)                */
#define CBT_CMD_SIZE_GETSERVICENAME 0x08    /* GetServiceName Command                   */
#define CBT_CMD_SIZE_ERROR          0x0A    /* Error Command                            */
#define CBT_CMD_SIZE_EPSONPACKING   0x1B    /* EpsonPackingCommand Command              */

#define CBT_RPY_SIZE_INIT           0x09    /* InitReply                                */
#define CBT_RPY_SIZE_OPENCHANNEL    0x10    /* OpenChannelReply                         */
#define CBT_RPY_SIZE_CLOSECHANNEL   0x0A    /* CloseChannelReply                        */
#define CBT_RPY_SIZE_CREDIT         0x0A    /* CreditReply                              */
#define CBT_RPY_SIZE_CREDITREQUEST  0x0C    /* CreditRequestReply                       */
#define CBT_RPY_SIZE_EXIT           0x08    /* ExitReply                                */
#define CBT_RPY_SIZE_GETSOCKETID    0x09    /* GetSoketIDReply                          */
#define CBT_RPY_SIZE_GETSERVICENAME 0x09    /* GetServiceNameReply                      */
#define CBT_RPY_SIZE_EPSONPACKING   0x08    /* EpsonPackingCommandReply                 */

    /*** CBT Command                                                                    */
    /*** -------------------------------------------------------------------------------*/
#define CBT_CMD_INIT                0x00
#define CBT_CMD_OPENCHANNEL         0x01
#define CBT_CMD_CLOSECHANNEL        0x02
#define CBT_CMD_CREDIT              0x03
#define CBT_CMD_CREDITREQUEST       0x04
#define CBT_CMD_DEBIT               0x05
#define CBT_CMD_DEBITREQUEST        0x06
#define CBT_CMD_CONFIGSOCKET        0x07
#define CBT_CMD_EXIT                0x08
#define CBT_CMD_GETSOCKETID         0x09
#define CBT_CMD_GETSERVICENAME      0x0A
#define CBT_CMD_EPSONPACKING        0x45
#define CBT_CMD_CBT_ERROR           0x7F
#define CBT_CMD_CBT_NONE            0xFF

    /*** Reply Command                                                                  */
    /*** -------------------------------------------------------------------------------*/
#define CBT_RPY_INIT                0x80
#define CBT_RPY_OPENCHANNEL         0x81
#define CBT_RPY_CLOSECHANNEL        0x82
#define CBT_RPY_CREDIT              0x83
#define CBT_RPY_CREDITREQUEST       0x84
#define CBT_RPY_DEBIT               0x85
#define CBT_RPY_DEBITREQUEST        0x86
#define CBT_RPY_CONFIGSOCKET        0x87
#define CBT_RPY_EXIT                0x88
#define CBT_RPY_GETSOCKETID         0x89
#define CBT_RPY_GETSERVICENAME      0x8A
#define CBT_RPY_EPSONPACKING        0xC5

/*-----------------------------------  Error Code  -------------------------------------*/
/*******************************************|********************************************/
#define EPCBT_ERR_INITDENIED          -1    /* Printer Result=0x01                      */
#define EPCBT_ERR_VERSION             -2    /* Printer Result=0x02                      */
#define EPCBT_ERR_CLOSEDENIED         -3    /* Printer Result=0x03                      */
#define EPCBT_ERR_RESOURCE            -4    /* Printer Result=0x04                      */
#define EPCBT_ERR_OPENCHANNEL         -5    /* Printer Result=0x05                      */
#define EPCBT_ERR_CHOPENDED           -6    /* Printer Result=0x06                      */
#define EPCBT_ERR_CREDITOVF           -7    /* Printer Result=0x07                      */
#define EPCBT_ERR_CMDDENIED           -8    /* Printer Result=0x08                      */
#define EPCBT_ERR_CHNOTSUPPORT        -9    /* Printer Result=0x09                      */
#define EPCBT_ERR_NOSERVICE          -10    /* Printer Result=0x0A                      */
#define EPCBT_ERR_INITFAILED         -11    /* Printer Result=0x0B                      */
#define EPCBT_ERR_PACKETSIZE         -12    /* Printer Result=0x0C                      */
#define EPCBT_ERR_NULLPACKETSZ       -13    /* Printer Result=0x0D                      */

#define EPCBT_ERR_PARAM              -20    /* Parameter Error                          */
#define EPCBT_ERR_MEMORY             -21    /* Memory Error                             */
#define EPCBT_ERR_CBTNOTOPEN         -22    /* Port Not Open                            */
#define EPCBT_ERR_CBT2NDOPEN         -23    /* Port end Open error                      */
#define EPCBT_ERR_CHNOTOPEN          -24    /* Channel Not Open or Close                */
#define EPCBT_ERR_CH2NDOPEN          -25    /* Channel end Open                         */
#define EPCBT_ERR_RPLYPSIZE          -28    /* OpenChannel return Paket Size Fail       */
#define EPCBT_ERR_WRITEERROR         -31    /* Write Error                              */
#define EPCBT_ERR_READERROR          -32    /* Read Error                               */
#define EPCBT_ERR_FNCDISABLE         -33    /* Not send or receive                      */

#define EPCBT_ERR_FATAL              -50    /* FATAL Error                              */
#define EPCBT_ERR_CBTDISABLE         -52    /* EpsonPackingCommand Fail                 */
#define EPCBT_ERR_NOREPLY            -55    /* Reply Time out                           */

#define EPCBT_ERR_MULFORMEDPACKET    -80    /* receibe paket data Fail                  */
#define EPCBT_ERR_UEREPLY            -82    /* miss match Reply                         */
#define EPCBT_ERR_UERESULT           -85    /* no define Result value                   */

/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/

    /*** Channel Information                                                            */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagCBTS_CHINFO_ {
    EPS_UINT8       No;                 /* Channel No                                   */
    EPS_INT32       PtSsize;            /* Packet Size (PC -> Printer)                  */
    EPS_INT32       StPsize;            /* Packet Size (Printer -> PC)                  */
    EPS_INT32       CreditP;            /* receive credit from printer                  */
    EPS_INT32       CreditH;            /* send credit from host                        */
    EPS_INT32       ReadSize;           /* receive packet data size                     */
    EPS_UINT8*      ReadBuff;           /* receive packet data buffer                   */
    EPS_UINT8*      RTmpBuff;           /* receive data work buffer                     */
    EPS_INT32       RTmpTop;            /* receive data work buffer, start data point   */
    EPS_INT32       RTmpEnd;            /* receive data work buffer, end data point     */
    EPS_INT32       RTmpCnt;            /* receive data work buffer, data size          */
} CBTS_CHINFO;

    /*** Printer Status                                                                 */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagCBTS_PRNINFO_ {
    EPS_INT16       Mode;                   /* CBT Mode                                 */
    void*           pPortInfo;              /* Port Info                                */
    EPS_WritePortal pCbtWRtnFnc;            /* Write Func                               */
    EPS_ReadPortal  pCbtRRtnFnc;            /* Read Func                                */
    EPS_UINT8*      pWriteBuff;             /* Write Work                               */
    EPS_UINT8*      pCbtReadRtnBuff;        /* Read Work 1                              */
    EPS_UINT8*      pCbtDataReadBuff;       /* Read Work 2                              */
    CBTS_CHINFO     Chinfo[CBT_MAX_CH];     /* Channel information                      */
} CBTS_PRNINFO;

/*--------------------------  Public Function Declarations   ---------------------------*/
/*******************************************|********************************************/
extern EPS_INT32    cbtCommOpen         (const EPS_USB_DEVICE*, EPS_FILEDSC*            );
extern EPS_INT32    cbtCommClose        (EPS_FILEDSC                                    );
extern EPS_INT32    cbtCommChannelOpen  (EPS_FILEDSC, EPS_INT32, EPS_BOOL               );
extern EPS_INT32    cbtCommChannelClose (EPS_FILEDSC, EPS_INT32                         );
extern EPS_INT32    cbtCommReadData     (EPS_FILEDSC, EPS_INT32, EPS_UINT8*, EPS_INT32, EPS_INT32*);
extern EPS_INT32    cbtCommWriteData    (EPS_FILEDSC, EPS_INT32, const EPS_UINT8*, EPS_INT32, EPS_INT32* );


#ifdef    __cplusplus
}
#endif    /* def __cplusplus    */

#endif    /* def _EPSON_CBT_H_ */

/*___________________________________  epson-cbt.h  ____________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
