/*_____________________________  epson-escpr-services.h   ______________________________*/

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
/*                            Epson ESC/PR Sevice Functions                             */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef __EPSON_ESCPR_SERVICES_H__
#define __EPSON_ESCPR_SERVICES_H__
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"
#include "epson-escpr-pvt.h"

/*------------------------------------- Data Types -------------------------------------*/
/*******************************************|********************************************/

/*--------------------------------------  Macros   -------------------------------------*/
/*******************************************|********************************************/

/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/
    /*** Ink Cartridge Type                                                             */
    /*** -------------------------------------------------------------------------------*/
#define MI_CARTRIDGE_INDEP          0x00        /* Independent Cartridge                */
#define MI_CARTRIDGE_ONE            0x01        /* One Cartridge                        */

    /*** Device States                                                                  */
    /*** -------------------------------------------------------------------------------*/
#define EPS_ST_ERROR                        0x01
#define EPS_ST_SELF_PRINTING                0x02
#define EPS_ST_BUSY                         0x04
#define EPS_ST_WAITING                      0x08
#define EPS_ST_IDLE                         0x10
/*#define EPS_ST_PAUSE                      0x20*/   /* Not supported by 2006 Model  */
/*#define EPS_ST_INKDRYING                  0x40*/   /* Not supported by 2006 Model  */
#define EPS_ST_CLEANING                     0x80
#define EPS_ST_FACTORY_SHIPMENT            0x100
/*#define EPS_ST_MOTOR_DRIVE_OFF           0x200*/   /* Not supported by 2006 Model  */
#define EPS_ST_SHUTDOWN                    0x400
/*#define EPS_ST_WAITPAPERINIT             0x800*/   /* Not supported by 2006 Model  */
/*#define EPS_ST_INIT_PAPER               0x1000*/   /* Not supported by 2006 Model  */

    /*** Device Warnings                                                                */
    /*** -------------------------------------------------------------------------------*/
#define EPS_PRNWARN_NONE                       0x00     /* No Warnings                  */
#define EPS_PRNWARN_INKLOW                 (1 << 0)     /* Ink Low                      */
#define EPS_PRNWARN_DISABLE_CLEAN          (1 << 1)     /* disable cleaning             */
#define EPS_PRNWARN_COLOR_INKOUT           (1 << 2)     /* disable color print          */
	

    /*** Ink Error                                                                      */
    /*** -------------------------------------------------------------------------------*/
#define EPS_INKERR_NONE                0

#define EPS_INKERR_INKENDB            -1
#define EPS_INKERR_INKENDC            -2
#define EPS_INKERR_INKENDCY           -3
#define EPS_INKERR_INKENDMA           -4
#define EPS_INKERR_INKENDYL           -5
#define EPS_INKERR_INKENDLC           -6
#define EPS_INKERR_INKENDLM           -7
#define EPS_INKERR_INKENDLY           -8
#define EPS_INKERR_INKENDDY           -9
#define EPS_INKERR_INKENDB2          -10
#define EPS_INKERR_INKENDRD          -11
#define EPS_INKERR_INKENDBL          -12
#define EPS_INKERR_INKENDMB          -13
#define EPS_INKERR_INKENDPB          -14
#define EPS_INKERR_INKENDGO          -15
#define EPS_INKERR_INKENDALL         -16
#define EPS_INKERR_INKENDBC          -17

#define EPS_INKERR_CEMPTYB          -101
#define EPS_INKERR_CEMPTYC          -102
#define EPS_INKERR_CEMPTYCY         -103
#define EPS_INKERR_CEMPTYMA         -104
#define EPS_INKERR_CEMPTYYL         -105
#define EPS_INKERR_CEMPTYLC         -106
#define EPS_INKERR_CEMPTYLM         -107
#define EPS_INKERR_CEMPTYLY         -108
#define EPS_INKERR_CEMPTYDY         -109
#define EPS_INKERR_CEMPTYB2         -110
#define EPS_INKERR_CEMPTYRD         -111
#define EPS_INKERR_CEMPTYBL         -112
#define EPS_INKERR_CEMPTYMB         -113
#define EPS_INKERR_CEMPTYPB         -114
#define EPS_INKERR_CEMPTYGO         -115
#define EPS_INKERR_CEMPTYBC         -116
#define EPS_INKERR_CEMPTYALL        -117

#define EPS_INKERR_CFAILB           -201
#define EPS_INKERR_CFAILC           -202
#define EPS_INKERR_CFAILCY          -203
#define EPS_INKERR_CFAILMA          -204
#define EPS_INKERR_CFAILYL          -205
#define EPS_INKERR_CFAILLC          -206
#define EPS_INKERR_CFAILLM          -207
#define EPS_INKERR_CFAILLY          -208
#define EPS_INKERR_CFAILDY          -209
#define EPS_INKERR_CFAILB2          -210
#define EPS_INKERR_CFAILRD          -211
#define EPS_INKERR_CFAILBL          -212
#define EPS_INKERR_CFAILMB          -213
#define EPS_INKERR_CFAILPB          -214
#define EPS_INKERR_CFAILGO          -215
#define EPS_INKERR_CFAILBC          -216
#define EPS_INKERR_CFAILALL         -217

    /*** Cancel request from printer                                                    */
    /*** -------------------------------------------------------------------------------*/
#define EPS_CAREQ_CANCEL            -300
#define EPS_CAREQ_NOCANCEL          -301

    /*** Other Printer Information                                                      */
    /*** -------------------------------------------------------------------------------*/
#define EPS_PREPARE_OVERHEAT        -400
#define EPS_PREPARE_NORMALHEAT      -401
#define EPS_PREPARE_TRAYOPENED      -402
#define EPS_PREPARE_TRAYCLOSED      -403

    /*** reset mode                                                                      */
    /*** -------------------------------------------------------------------------------*/
#define EPS_PM_PAGE                   1
#define EPS_PM_JOB                    2

    /*** Misc                                                                           */
    /*** -------------------------------------------------------------------------------*/
#define _SECOND_                    1000    /* Unit for changing milli second to second */
#define _STATUS_REPLY_BUF            512    /* Printer status buffer size               */


    /*** Model depend                                                                   */
    /*** -------------------------------------------------------------------------------*/
#define EPS_MDC_STATUS				(1)
#define EPS_MDC_NOZZLE				(2)

/*---------------------------  Public Function Declarations  ---------------------------*/
/*******************************************|********************************************/
extern EPS_INT32    serParseDeviceID            (EPS_INT8*, EPS_INT32, EPS_INT8*, EPS_INT8*, 
                                                 EPS_INT32*, EPS_UINT32*, EPS_UINT32*   );
extern EPS_BOOL		serCheckLang				(EPS_INT8*, EPS_UINT32*					);
extern EPS_ERR_CODE serAnalyzeStatus            (EPS_INT8*, EPS_UINT32, EPS_STATUS_INFO*);
extern void         serDelayThread              (EPS_UINT32, EPS_CMN_FUNC*              );
extern EPS_INT32    serInkLevelNromalize        (EPS_INT32                              );
extern EPS_ERR_CODE serGetSerialNo              (EPS_INT8*, EPS_INT32, EPS_INT8*        );
extern EPS_ERR_CODE serGetSerialNoFormST        (EPS_INT8*, EPS_INT8*, EPS_INT32        );
extern EPS_ERR_CODE serGetSupplyInfo            (EPS_UINT8*, EPS_SUPPLY_INFO*           );

#ifdef GCOMSW_EPSON_SLEEP
extern EPS_ERR_CODE serSleep                    (EPS_UINT32                             );
#endif


extern void         obsSetPrinter               (const EPS_PRINTER_INN*                 );
extern void         obsSetColorPlane            (EPS_UINT8                              );
extern void         obsClear                    (void                                   );
extern EPS_INT32    obsGetPageMode              (void                                   );
extern EPS_BOOL		obsIsA3Model				(EPS_INT32                              );
extern EPS_BOOL		obsEnableDuplex				(EPS_INT32                              );
extern EPS_BOOL		obsEnableAutoFeed			(void                                   );

#ifdef __cplusplus
}
#endif

#endif  /* def __EPSON_ESCPR_SERVICES_H__ */


/*_____________________________  epson-escpr-services.h  _______________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
