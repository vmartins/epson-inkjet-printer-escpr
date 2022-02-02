/*____________________________   epson-escpr-services.c   ______________________________*/

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
/*                      Epson ESC/P-R Lib Internal Service Routines                     */
/*                                                                                      */
/*                                Public Function Calls                                 */
/*                              --------------------------                              */
/*              EPS_ERR_CODE serAnalyzeStatus       (Status, StatusInfo         );      */
/*              void         serDelayThread         (Milliseconds               );      */
/*              EPS_ERR_CODE serSleep               (Milliseconds               );      */
/*                                                                                      */
/*******************************************|********************************************/

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-err.h"
#include "epson-escpr-mem.h"
#include "epson-escpr-media.h"
#include "epson-escpr-services.h"
#ifdef GCOMSW_CMD_ESCPAGE_S
#include "epson-escpage-s.h"
#endif
#ifdef GCOMSW_CMD_PCL
#include "epson-pcl.h"
#endif

#ifndef EPS_PARSE_CARTRIDGE
#define EPS_PARSE_CARTRIDGE(p1, p2, p3, p4)	parseCartridge(p1, p2, p3, p4);
#endif


/*------------------------------------  Definition   -----------------------------------*/
/*******************************************|********************************************/

#ifdef EPS_LOG_MODULE_SER
#define EPS_LOG_MODULE	EPS_LOG_MODULE_SER
extern EPS_CMN_FUNC    epsCmnFnc;
#else
#define EPS_LOG_MODULE	0
#endif


/*----------------------------  ESC/P-R Lib Global Variables  --------------------------*/
/*******************************************|********************************************/

/*------------------------------  Local Global Variables  ------------------------------*/
/*******************************************|********************************************/
typedef	struct _tagSP_OBSERVER
{
	const EPS_PRINTER_INN	 *printer;	         /* current printer                     */
    EPS_UINT8                colorPlane;         /* Image color plane                   */
}SP_OBSERVER;

static SP_OBSERVER	g_observer = {0};

/*--------------------------------  Local Functions   ----------------------------------*/
/*******************************************|********************************************/
#ifndef GCOMSW_EXTENTION
static EPS_UINT8* parseCartridge	(EPS_UINT8 ParameterByte, EPS_UINT8* param, 
									 EPS_INK_INFO* ink, EPS_INT32 cartridges[]          );
#endif
static EPS_UINT32 pptops            (EPS_UINT8* pp                                      );


/*-----------------------------------  Debug Dump  -------------------------------------*/
/*******************************************|********************************************/
#define _DEBUG_BIN_STATUS_          0       /* 0: OFF    1: ON                          */

#if _DEBUG_BIN_STATUS_ || _VALIDATE_SUPPORTED_MEDIA_DATA_
#define SerDebugPrintf(a)  EPS_DBGPRINT( a )
#else
#define SerDebugPrintf(a)
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
/* Function name:   serParseDeviceID()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:           Description:                                        */
/* deviceIDString	EPS_INT8*       I: DeviceID string                                  */
/* manufacturer		EPS_INT8*       I: Manufacturer Name                                */
/* modelName		EPS_INT8*       I: Model Name                                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      1                       - Success                                               */
/*      -1                      - Bad format or Not Supported device                    */
/*      0                       - unknown                                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Get the manufacturer name and modelName from DeviceID string.                   */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32     serParseDeviceID (

		EPS_INT8*			deviceIDString,
		EPS_INT32           strlength,
        EPS_INT8*			manufacturer,
        EPS_INT8*			modelName,
        EPS_INT32*			cmdLevel,
		EPS_UINT32*			lang,
        EPS_UINT32*			errorID

){
    EPS_INT8*       i = NULL;
    EPS_INT8*       j = NULL;
    EPS_INT8*       k = NULL;
    EPS_INT8*       p = NULL;
	EPS_UINT32		tmpLang = EPS_LANG_UNKNOWN;

	EPS_LOG_FUNCIN;

/*** Delete the data of the device ID length from string                                */
	if(strlength < 2){
		EPS_RETURN( -1 );
	}
    *(deviceIDString+strlength) = '\0';
    if(deviceIDString[0] == 0x00 || deviceIDString[1] == 0x00){
        deviceIDString += 2;
	}
	EPS_DBGPRINT(("DI : %s\n", deviceIDString))

/*** ================================================================================== */
/*** Step 1: Get Manufacturer Name                                                      */
/*** ================================================================================== */
    if((i = strstr(deviceIDString, "MFG:")) != NULL) {
	    i += 4;
	} else if((i = strstr(deviceIDString, "MANUFACTURER:")) != NULL) {
	    i += 13;
	} else{
        /* Invalid device ID. */
		EPS_RETURN( -1 );
	}
    j  = i;
    
    while(*j != ';')
    {
        j++;
        
        /* Emergency exit condition to prevent an infinite loop scenario; if we hit a   */
        /* carriage return, we've run too far                                           */
        if(*j == 0x0D || *j == 0x00) {
			/* EPS_RETURN( -1 ); When there is not ';', assume the whole a value */
			break;
       }
    }
    /* Null-terminate the MFG substring. */
    *j = 0;
    
	if((strncmp(i, "EPSON", 5) != 0) && (strncmp(i, "Epson", 5) != 0) ){
	    *j = ';';
		EPS_RETURN( -1 );
	}

	if(NULL != manufacturer){
		/* Make sure the manufacturer name is not longer than 64 bytes. */
		if(strlen(i) < 64) {
			strcpy(manufacturer, i);    /* If the name is OK, copy the whole string as-is */
		}
		else {
			memcpy(manufacturer, i, 63);    /* If the name is longer than 64 bytes, only copy */
		}                                   /* 63 bytes and leave the 64th as null terminator */
	}

    /* Return the string to its original format. */
    *j = ';';
    

/*** ================================================================================== */
/*** Step 2: Get Model Name                                                             */
/*** ================================================================================== */
	if(NULL != modelName){
		if((i = strstr(deviceIDString, "MDL:")) != NULL) {
			i += 4;
		} else if((i = strstr(deviceIDString, "MODEL:")) != NULL) {
			i += 6;
		} else{
			/* Invalid device ID. */
			EPS_RETURN( -1 );
		}
		j  = i;
    
		while (*j != ';')
		{
			j++;
        
			/* Emergency exit condition to prevent an infinite loop scenario; if we hit a   */
			/* carriage return, we've run too far                                           */
			if(*j == 0x0D || *j == 0x00){
				/* Invalid device ID. */
				/* EPS_RETURN( -1 ); */
				break;
			}
		}
    
		/* Null-terminate the MDL substring. */
		*j = 0;
    
		/* Make sure the model name is not longer than 64 bytes. */
		if(strlen(i) < 64) {
			strcpy(modelName, i);   /* If the name is OK, copy the whole string as-is */
		}
		else {
			memcpy(modelName, i, 63);   /* If the name is longer than 64 bytes, only copy */
										/* 63 bytes, leaving the 64th as null terminator  */
		}
	
		*j = ';';
	}


/*** ================================================================================== */
/*** Step 3: Check for ESC/PR Support                                                   */
/*** ================================================================================== */
    if((i = strstr(deviceIDString, "CMD:")) == NULL) {
		EPS_RETURN( 0 );
	}
	i += 4;
    j  = i;

	while (*j != ';')
	{
		j++;
        
		/* Emergency exit condition to prevent an infinite loop scenario; if we hit a   */
		/* carriage return, we've run too far                                           */
		if(*j == 0x0D || *j == 0x00){
			/* Invalid device ID. */
			/* EPS_RETURN( -1 ); */
			break;
		}
	}
	*j = '\0';

	while( NULL != i ){
		if((k = strstr(i, ",")) != NULL) {
			*k = '\0';
		}

		if((p = strstr(i, "ESCPR")) != NULL) {
			tmpLang = EPS_LANG_ESCPR;
			if(NULL != cmdLevel){
				sscanf(p+5, "%d", cmdLevel);
			}
			break;

#ifdef GCOMSW_CMD_ESCPAGE
		} else if(strstr(i, "ESCPAGECOLOR") != NULL) {
			tmpLang = EPS_LANG_ESCPAGE_COLOR;
			if(NULL != cmdLevel){
				*cmdLevel = 1;
			}
			break;

		} else if(strstr(i, "ESCPAGE") != NULL) {
			if(strstr(i, "ESCPAGES") == NULL) {
				tmpLang = EPS_LANG_ESCPAGE;
				if(NULL != cmdLevel){
					*cmdLevel = 1;
				}
				/* break; ESC/Page Color takes priority */
#ifdef GCOMSW_CMD_ESCPAGE_S
			} else{
				if(TRUE == pageS_ParseLang(i, cmdLevel, &tmpLang)){
					/* break; ESC/Page takes priority */
				}
#endif
			}

#ifdef GCOMSW_CMD_PCL
		} else if( (strstr(i, "PCLXL") != NULL) && (strlen(i) == 5) ){
			if(EPS_LANG_UNKNOWN == tmpLang){
				tmpLang = EPS_LANG_PCL;
			}
			if(NULL != cmdLevel){
				*cmdLevel = 1;
			}
			/*break; ESC/P-R, ESC/Page takes priority */
#endif

#endif /* GCOMSW_CMD_ESCPAGE */
		}

		if(k){
			*k = ',';
			i = k+1;
			if(i >= j){
				break;
			}
		} else{
			break;
		}
	}
	if(k)*k = ',';

	*j = ';';

	if(EPS_LANG_UNKNOWN == tmpLang){
		EPS_RETURN( -1 );
	}
	if(NULL != lang){
		*lang = tmpLang;
	}

/*** ================================================================================== */
/*** Step 4: Error Group ID                                                             */
/*** ================================================================================== */
	if(NULL != errorID){
		if((i = strstr(deviceIDString, "ELG:")) != NULL) {
			i += 4;
		} else{
			EPS_RETURN( 1 );
		}
		j  = i;
    
		while (*j != ';')
		{
			j++;
			if(*j == 0x0D || *j == 0x00){
				EPS_RETURN( 1 );
			}
		}
    
		/* Null-terminate the MDL substring. */
		*j = 0;
    
		if(strlen(i) <= 4) {
			sscanf(i, "%x", errorID);
		}
		*j = ';';
	}

    EPS_RETURN( 1 );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   serCheckLang()		                                                */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* langString   EPS_INT8*           I: Print lang string                                */
/* lang         EPS_UINT32*         O: lang id                                          */
/*                                                                                      */
/* Return value:                                                                        */
/*      TRUE                        - Success                                           */
/*      FALSE                       - Not Supported device                              */
/*                                                                                      */
/* Description:                                                                         */
/*      Get the printer status and analyze the status string.                           */
/*                                                                                      */
/*******************************************|********************************************/
EPS_BOOL    serCheckLang (

        EPS_INT8*		langString,
        EPS_UINT32*		lang

){
	if( (strncmp(langString, "ESC-P/R", 7) == 0) ){
		*lang = EPS_LANG_ESCPR;
		return TRUE;
#ifdef GCOMSW_CMD_PCL
	} else if( (strcmp(langString, "PCL 6") == 0) ){
		*lang = EPS_LANG_PCL;
		return TRUE;
#endif
	}

	return FALSE;
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   serAnalyzeStatus()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* Status       EPS_INT8*           I: Printer Status string                            */
/* egID         EPS_UINT32          I: printer error group id                           */
/* pstInfo      EPS_STATUS_INFO*    O: Printer Status information                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Get the printer status and analyze the status string.                           */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    serAnalyzeStatus (

        EPS_INT8*           Status,
		EPS_UINT32          egID,
        EPS_STATUS_INFO*    pstInfo

){
    EPS_UINT8*  Field;
    EPS_UINT8*  EndField;
    EPS_INT32   i;
    
    EPS_UINT8   Header;
    EPS_UINT8   ParameterByte;
    EPS_INT8*   Ink;
    EPS_INT8    Parameter[128];
	EPS_BOOL	mediaErr = FALSE;
    EPS_UINT8   mediaErrNum, jamErrNum;
	EPS_INT32	inkNum = 0;
	EPS_INT32	nInkError = EPS_INKERR_NONE;

	EPS_LOG_FUNCIN;

	if((Field = (EPS_UINT8*)strstr(Status,"ST2")) == NULL ){
        SerDebugPrintf(("EPS SER: Get Stat -> ST not found [%s]\r\n",Status));
        EPS_RETURN( EPS_ERR_COMM_ERROR );
    }
    
	memset(pstInfo, 0, sizeof(EPS_STATUS_INFO));
    pstInfo->nState = EPS_ST_IDLE;
    pstInfo->nError = EPS_PRNERR_NOERROR;
    pstInfo->nWarn  = EPS_PRNWARN_NONE;
	mediaErrNum = jamErrNum = 0;

#if _DEBUG_BIN_STATUS_
	EPS_DUMP(Status, 256);    
#endif
    Field    = Field + 5;
    EndField = Field + (2+(*Field)+ ((*(Field+1))*256) );
    Field    = Field + 2;
    
    while ( Field < EndField ) {
    
        Header        = (EPS_UINT8) Field[0];
        ParameterByte = (EPS_UINT8) Field[1];
        memcpy(Parameter, Field+2, (EPS_INT8)ParameterByte );
        
        Field = Field + 2 + ParameterByte;
        
        switch( Header ) {
            case 0x01: /* ST */
                switch( Parameter[0] ) {
                    case 0x00: pstInfo->nState = EPS_ST_ERROR;               break;
                    case 0x01: pstInfo->nState = EPS_ST_SELF_PRINTING;       break;
                    case 0x02: pstInfo->nState = EPS_ST_BUSY;                break;
                    case 0x03: pstInfo->nState = EPS_ST_WAITING;             break;
                    case 0x04: pstInfo->nState = EPS_ST_IDLE;                break;
                /*  case 0x05: pstInfo->nState = EPS_ST_PAUSE;               break;*/    /* Not supported by 2006 Model */
                /*  case 0x06: pstInfo->nState = EPS_ST_INKDRYING            break;*/    /* Not supported by 2006 Model */
                    case 0x07: pstInfo->nState = EPS_ST_CLEANING;            break;
                    case 0x08: pstInfo->nState = EPS_ST_FACTORY_SHIPMENT;    break;
                /*  case 0x09: pstInfo->nState = EPS_ST_MOTOR_DRIVE_OFF;     break;*/    /* Not supported by 2006 Model */
                    case 0x0A: pstInfo->nState = EPS_ST_SHUTDOWN;            break;
                /*  case 0x0B: pstInfo->nState = EPS_ST_WAITPAPERINIT;       break;*/    /* Not supported by 2006 Model */
                /*  case 0x0C: pstInfo->nState = EPS_ST_INIT_PAPER;          break;*/    /* Not supported by 2006 Model */
                    case 0x0D: pstInfo->nState = EPS_ST_BUSY;                break;
                    default:   
						pstInfo->nState = EPS_ST_ERROR;               
						if(pstInfo->nError == EPS_PRNERR_NOERROR){
							pstInfo->nError = EPS_PRNERR_GENERAL;
						}
						break;
                }
                break;

            case 0x02: /* ER */
				EPS_DBGPRINT(("* ERR 0x%02X *\n", Parameter[0]))
                switch(Parameter[0]) {
                    case 0x00: pstInfo->nError = EPS_PRNERR_FATAL;                  break;
                    case 0x01: pstInfo->nError = EPS_PRNERR_INTERFACE;              break;
                    case 0x02: 
						if ( obsIsA3Model(EPS_MDC_STATUS) == TRUE ) {
							pstInfo->nError = EPS_PRNERR_CDRGUIDEOPEN;
						} else{
							pstInfo->nError = EPS_PRNERR_COVEROPEN;
						}
						break;
                /*  case 0x03: pstInfo->nError = EPS_PRNERR_LEVERPOSITION;          break;*/    /* Not supported by 2006 Model */
                    case 0x04: pstInfo->nError = EPS_PRNERR_PAPERJAM;               break;
                    case 0x05: pstInfo->nError = EPS_PRNERR_INKOUT;					break;
                    case 0x06: pstInfo->nError = EPS_PRNERR_PAPEROUT;               break;
                /*  case 0x07: pstInfo->nError = EPS_PRNERR_INITIALIZESETTING;      break;*/    /* Not supported by 2006 Model */
                /*  case 0x08: pstInfo->nError = EPS_PRNERR_UNKNOWN;                break;*/    /* Not supported by 2006 Model */
                    case 0x09: pstInfo->nError = EPS_PRNERR_CDRGUIDEOPEN/*EPS_PRNERR_PAPERCHANGE_UNCOMP*/;     break; /* supported by 2012 Model */
                    case 0x0A: pstInfo->nError = EPS_PRNERR_SIZE_TYPE_PATH/*EPS_PRNERR_PAPERSIZE*/;break;/* supported by 2008 Model */
                /*  case 0x0B: pstInfo->nError = EPS_PRNERR_RIBBONJAM;              break;*/    /* Not supported by 2006 Model */
                    case 0x0C: pstInfo->nError = EPS_PRNERR_SIZE_TYPE_PATH;         break;
                /*  case 0x0D: pstInfo->nError = EPS_PRNERR_PAPERTHICKLEVER;        break;*/    /* Not supported by 2006 Model */
                /*  case 0x0E: pstInfo->nError = EPS_PRNERR_PAPERFEED;              break;*/    /* Not supported by 2006 Model */
                /*  case 0x0F: pstInfo->nError = EPS_PRNERR_SIMMCOPY;               break;*/    /* Not supported by 2006 Model */
                    case 0x10: pstInfo->nError = EPS_PRNERR_SERVICEREQ;             break;
                    case 0x11: pstInfo->nError = EPS_PRNERR_ANY/*EPS_PRNERR_WAITTEAROFFRETURN*/; break;
                    case 0x12: pstInfo->nError = EPS_PRNERR_DOUBLEFEED;             break;
                /*  case 0x13: pstInfo->nError = EPS_PRNERR_HEADHOT;                break;*/    /* Not supported by 2006 Model */
                /*  case 0x14: pstInfo->nError = EPS_PRNERR_PAPERCUTMIS;            break;*/    /* Not supported by 2006 Model */
                /*  case 0x15: pstInfo->nError = EPS_PRNERR_HOLDLEVERRELEASE;       break;*/    /* Not supported by 2006 Model */
                /*  case 0x16: pstInfo->nError = EPS_PRNERR_NOT_CLEANING;           break;*/    /* Not supported by 2006 Model */
                    case 0x17: pstInfo->nError = EPS_PRNERR_SIZE_TYPE_PATH/*EPS_PRNERR_PAPERCONFIG*/; break; /* supported by 2012 Model */
                /*  case 0x18: pstInfo->nError = EPS_PRNERR_PAPERSLANT;             break;*/    /* Not supported by 2006 Model */
                /*  case 0x19: pstInfo->nError = EPS_PRNERR_CLEANINGNUMOVER;        break;*/    /* Not supported by 2006 Model */
                    case 0x1A: pstInfo->nError = EPS_PRNERR_INKCOVEROPEN;           break;
                /*  case 0x1B: pstInfo->nError = EPS_PRNERR_LFP_INKCARTRIDGE;       break;*/    /* Not supported by 2006 Model */
                /*  case 0x1C: pstInfo->nError = EPS_PRNERR_CUTTER;                 break;*/    /* Not supported by 2006 Model */
                /*  case 0x1D: pstInfo->nError = EPS_PRNERR_CUTTERJAM;              break;*/    /* Not supported by 2006 Model */
                /*  case 0x1E: pstInfo->nError = EPS_PRNERR_INKCOLOR;               break;*/    /* Not supported by 2006 Model */
                    case 0x1E: pstInfo->nError = EPS_PRNERR_ANY/*EPS_PRNERR_INKCOLOR*/; break;      /* Not supported by 2006 Model */
                /*  case 0x1F: pstInfo->nError = EPS_PRNERR_CUTTERCOVEROPEN;        break;*/    /* Not supported by 2006 Model */
                /*  case 0x20: pstInfo->nError = EPS_PRNERR_LFP_INKLEVERRELEASE;    break;*/    /* Not supported by 2006 Model */
                /*  case 0x22: pstInfo->nError = EPS_PRNERR_LFP_NOMAINTENANCETANK1; break;*/    /* Not supported by 2006 Model */
                /*  case 0x23: pstInfo->nError = EPS_PRNERR_CARTRIDGECOMBINATION;   break;*/    /* Not supported by 2006 Model */
                /*  case 0x24: pstInfo->nError = EPS_PRNERR_LFP_COMMAND;            break;*/    /* Not supported by 2006 Model */
                /*  case 0x25: pstInfo->nError = EPS_PRNERR_LEARCOVEROPEN;          break;*/    /* Not supported by 2006 Model */
                    case 0x25: pstInfo->nError = EPS_PRNERR_COVEROPEN;              break;
                /*  case 0x26: pstInfo->nError = EPS_PRNERR_MULTICENSORGAIN;        break;*/    /* Not supported by 2006 Model */
                /*  case 0x27: pstInfo->nError = EPS_PRNERR_NOT_AUTOADJUST;         break;*/    /* Not supported by 2006 Model */
                /*  case 0x28: pstInfo->nError = EPS_PRNERR_FAILCLEANING;           break;*/    /* Not supported by 2006 Model */
                    case 0x29: pstInfo->nError = EPS_PRNERR_NOTRAY;                 break;
                    case 0x2A: pstInfo->nError = EPS_PRNERR_CARDLOADING;            break;
                    case 0x2B: 
						if ( obsIsA3Model(EPS_MDC_STATUS) == TRUE ) {
							pstInfo->nError = EPS_PRNERR_CDRGUIDEOPEN;
						} else {
							pstInfo->nError = EPS_PRNERR_CDDVDCONFIG;							/* supported by 2008 Model */
						}
						break;
                    case 0x2C: pstInfo->nError = EPS_PRNERR_CARTRIDGEOVERFLOW;      break;
                /*  case 0x2D: pstInfo->nError = EPS_PRNERR_LFP_NOMAINTENANCETANK2; break;*/    /* Not supported by 2006 Model */
                /*  case 0x2E: pstInfo->nError = EPS_PRNERR_INKOVERFLOW2;           break;*/    /* Not supported by 2006 Model */
                    case 0x2F: pstInfo->nError = EPS_PRNERR_BATTERYVOLTAGE;         break;
                    case 0x30: pstInfo->nError = EPS_PRNERR_BATTERYTEMPERATURE;     break;
                    case 0x31: pstInfo->nError = EPS_PRNERR_BATTERYEMPTY;           break;
                    case 0x32: pstInfo->nError = EPS_PRNERR_SHUTOFF;                break;      /* Not supported by 2006 Model */
                    case 0x33: pstInfo->nError = EPS_PRNERR_NOT_INITIALFILL;        break;      /* Not supported by 2006 Model */
                    case 0x34: pstInfo->nError = EPS_PRNERR_PRINTPACKEND;           break;      /* Not supported by 2006 Model */
                /*  case 0x35: pstInfo->nError = EPS_PRNERR_ABNORMALHEAT;           break;*/    /* Not supported by 2006 Model */
                    case 0x37:
						if ( obsIsA3Model(EPS_MDC_STATUS) == TRUE ) {
							pstInfo->nError = EPS_PRNERR_COVEROPEN;
						} else if(0x0D04 == egID || 0x0D84 == egID){
							pstInfo->nError = EPS_PRNERR_COVEROPEN;
						} else{
							pstInfo->nError = EPS_PRNERR_SCANNEROPEN;
						}
						break;
                    case 0x38:
						if ( obsIsA3Model(EPS_MDC_STATUS) == TRUE ) {
							pstInfo->nError = EPS_PRNERR_CDDVDCONFIG;
						} else if((0x0D00 <= egID && 0x0D05 >= egID) || 0x0E21 == egID){
							pstInfo->nError = EPS_PRNERR_TRAYCLOSE;
						} else{
							pstInfo->nError = EPS_PRNERR_CDRGUIDEOPEN;
						}
						break;
                    case 0x44: pstInfo->nError = EPS_PRNERR_CDRGUIDEOPEN;			break;
					case 0x45: pstInfo->nError = EPS_PRNERR_CDREXIST_MAINTE;		break;
					case 0x46: pstInfo->nError = EPS_PRNERR_TRAYCLOSE;				break;
					case 0x47: pstInfo->nError = EPS_PRNERR_INKOUT;					break;		/* BlackPrint Error */

					case 0x4C: pstInfo->nError = EPS_PRNERR_PAPEROUT;				break;
					case 0x4D: pstInfo->nError = EPS_PRNERR_CDDVDCONFIG;			break;
					case 0x4E: pstInfo->nError = EPS_PRNERR_FEEDERCLOSE;			break;
					case 0x55: mediaErr = TRUE;  									break;						
					case 0x56: pstInfo->nError = EPS_PRNERR_INTERRUPT_BY_INKEND;	break;
					case 0x57: pstInfo->nError = EPS_PRNERR_PAPEROUT;				break;
					case 0x5D: pstInfo->nError = EPS_PRNERR_NO_BATTERY;				break;
					case 0x5E: pstInfo->nError = EPS_PRNERR_LOW_BATTERY_FNC;		break;
					case 0x5F: pstInfo->nError = EPS_PRNERR_BATTERY_CHARGING;		break;
					case 0x60: pstInfo->nError = EPS_PRNERR_BATTERY_TEMPERATURE_HIGH;break;
					case 0x61: pstInfo->nError = EPS_PRNERR_BATTERY_TEMPERATURE_LOW;break;
					case 0x62: pstInfo->nError = EPS_PRNERR_ANY;					break;
						
					default:   
						pstInfo->nError = EPS_PRNERR_GENERAL;                
						break;
                }
                break;

            case 0x04: /* WR */
                /* ESC/P-R Lib does not notified the warning to application, */
                /* so warning analysis dose not need to be done completely.  */
				pstInfo->nWarn = EPS_PRNWARN_NONE;
				for(i = 0; i < ParameterByte; i++){
					if( Parameter[i] >= 0x10 && Parameter[i] <= 0x1A ){
						/* Ink Low Warning */
						pstInfo->nWarn |= EPS_PRNWARN_INKLOW;
					} else if( Parameter[i] >= 0x51 && Parameter[i] <= 0x5A ){
						pstInfo->nWarn |= EPS_PRNWARN_DISABLE_CLEAN;
					} else if( Parameter[i] >= 0x71 && Parameter[i] <= 0x7A ){
					/*} else if( Parameter[i] == 0x44 ){	not use
						pstInfo->nWarn |= EPS_PRNWARN_COLOR_INKOUT;*/
					}
				}
                break;

            case 0x0F: /* INK */
				inkNum = Min((ParameterByte-1) / Parameter[0], EPS_INK_NUM);
                Ink = Parameter+1;

				for( i=0; i < inkNum; i++ ) {
					switch( Ink[2] ) {
					case 'w' :
					case 'r' :
						nInkError = EPS_PRNERR_CFAIL;
						break;
					case 'n' :
						nInkError = EPS_PRNERR_CEMPTY;
						break;
					case 0:
						nInkError = EPS_PRNERR_INKOUT;
					default:
						break;
					}
                    Ink = Ink + Parameter[0];
				}
	            break;

			case 0x13: /* CANCEL REQUEST by Printer cancel botton */
                switch((EPS_UINT8)Parameter[0]) {
                    case 0x81:
                        pstInfo->nCancel = EPS_CAREQ_CANCEL;
                        break;
                    default:
                        pstInfo->nCancel = EPS_CAREQ_NOCANCEL;
                        break;
                }
                break;
            case 0x18: /* Stacker(tray) open status */
                switch((EPS_UINT8)Parameter[0]) {
                    case 0x02:  /* Closed*/
                        pstInfo->nPrepare = EPS_PREPARE_TRAYCLOSED;
                        break;
                    case 0x03: /* Open*/
                        pstInfo->nPrepare = EPS_PREPARE_TRAYOPENED;
                        break;
                }
                break;
            case 0x1C: /* Temperature information */
                switch((EPS_UINT8)Parameter[0]) {
                    case 0x01: /* The printer temperature is higher than 40C*/
                        pstInfo->nPrepare = EPS_PREPARE_OVERHEAT;
                        break;
                    case 0x00: /* The printer temperature is lower than 40C*/
                        pstInfo->nPrepare = EPS_PREPARE_NORMALHEAT;
                        break;
                }
                break;

            case 0x35: /* Paper Jam error information */
				jamErrNum = (EPS_UINT8)Parameter[0];
                break;

			case 0x3B: /* Media */
				mediaErrNum = (EPS_UINT8)Parameter[0];
                break;

            default:
                break;
        }
    }

	if( TRUE == mediaErr && EPS_PRNERR_NOERROR == pstInfo->nError){
		switch( mediaErrNum ){
		case 0x01:	pstInfo->nError = EPS_PRNERR_3DMEDIA_FACE;		break;
		case 0x02:	pstInfo->nError	= EPS_PRNERR_3DMEDIA_DIRECTION;	break;
		default:	pstInfo->nError	= EPS_PRNERR_PAPEROUT;			break;
		}
	} else if ( EPS_PRNERR_PAPERJAM == pstInfo->nError ){
		if( 0x10 == jamErrNum && (0x0D00 <= egID && 0x0D05 >= egID)){
			pstInfo->nError = EPS_PRNERR_CDDVDCONFIG;
		}
	} else if ( EPS_PRNERR_PAPEROUT == pstInfo->nError ){
		switch( jamErrNum ){
		case 0x90:	
			if(0x0D04 == egID){
				pstInfo->nError = EPS_PRNERR_MANUALFEED_SET_PAPER_NOLCD;
			} else if(0x0D00 <= egID && 0x0D05 >= egID){
				pstInfo->nError = EPS_PRNERR_MANUALFEED_SET_PAPER;
			}
			break;
		case 0x91:
			if(0x0D04 == egID){
				pstInfo->nError	= EPS_PRNERR_MANUALFEED_FAILED_NOLCD;
			} else if(0x0D00 <= egID && 0x0D05 >= egID){
				pstInfo->nError	= EPS_PRNERR_MANUALFEED_FAILED;
			}
			break;
		case 0x92:
			if(0x0D04 == egID){
				pstInfo->nError	= EPS_PRNERR_MANUALFEED_EXCESSIVE_NOLCD;
			} else if(0x0D00 <= egID && 0x0D05 >= egID){
				pstInfo->nError	= EPS_PRNERR_MANUALFEED_EXCESSIVE;
			}
			break;
		case 0xA3:	
			pstInfo->nError = EPS_PRNERR_ROLLPAPER_TOOSHORT;
			break;
		default: break;
		}
	} else if(EPS_PRNERR_INKOUT == pstInfo->nError && EPS_INKERR_NONE != nInkError ){
		pstInfo->nError = nInkError;
	}

	if ( EPS_PRNERR_CDDVDCONFIG == pstInfo->nError ){
		if(0x0D04 == egID){
			pstInfo->nError	= EPS_PRNERR_CDDVDCONFIG_FEEDBUTTON;
		} else if (0x0D00 <= egID && 0x0D05 >= egID){
			pstInfo->nError	= EPS_PRNERR_CDDVDCONFIG_STARTBUTTON;
		}
	}

	if( EPS_CAREQ_CANCEL == pstInfo->nCancel ){
		EPS_DBGPRINT(("*** Cancel Request (ignore error) ***\n"))
		pstInfo->nState = EPS_ST_WAITING;
		pstInfo->nError = EPS_PRNERR_NOERROR;
	}


#if _DEBUG_BIN_STATUS_
    SerDebugPrintf(("***** ST = %d\r\n", pstInfo->nState));
    SerDebugPrintf(("***** ER = %d\r\n", pstInfo->nError));
    SerDebugPrintf(("***** WR = %d\r\n", pstInfo->nWarn));
    SerDebugPrintf(("***** CA = %d\r\n", pstInfo->nCancel));
    SerDebugPrintf(("***** INK NUM = %d\r\n", pstInfo->nInkNo));
    for(i = 0; i < pstInfo->nInkNo; i++){
    SerDebugPrintf(("***** INK = %d\t%d\r\n", pstInfo->nColorType[i], pstInfo->nColor[i]));
    }
#endif

    EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   serGetSerialNo()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* cdBuff       EPS_INT8*           I: 'cd' command reply                               */
/* buflen       EPS_INT32           I: 'cd' data length                                 */
/* serialNo     EPS_INT8*           O: serial number                                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*                                                                                      */
/* Description:                                                                         */
/*      parse 'cd' command reply to get serial number string.                           */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    serGetSerialNo (

        EPS_INT8*       cdBuff,
		EPS_INT32       buflen,
        EPS_INT8*       serialNo

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_INT8*	p = NULL;
	EPS_INT32   datalen = 0;
	EPS_UINT8   formVer = 0;

#define EPS_GET_SERIAL(POS, LEN)				\
		if(datalen >= POS+LEN){					\
			strncpy(serialNo, p+POS, LEN);		\
		} else{									\
			ret = EPS_ERR_COMM_ERROR;			\
		}

	EPS_LOG_FUNCIN

	/* verify */
	if( buflen < 14 ){
		EPS_RETURN( EPS_ERR_COMM_ERROR)
	}
	if( 0 != memcmp(cdBuff+buflen-2, ";\x0C", 2) ){
		EPS_RETURN( EPS_ERR_COMM_ERROR)
	}

	p = strstr(cdBuff, "cd:");
	if( NULL == p || buflen-(p-cdBuff) < 6 ){
		EPS_RETURN( EPS_ERR_COMM_ERROR)
	}
	p += 5;	/* skip head */

	if( 0 == strncmp(p, "NA", 2) || 0 == strncmp(p, "BUSY", 4)){
		EPS_RETURN( EPS_ERR_COMM_ERROR)
	}

	datalen = (EPS_INT32)*p + ((EPS_INT32)*(p+1) << 8);
	p += 2;	/* skip lenght field */

	/* get version */
	formVer = 0x07;
	if( buflen-(EPS_INT32)(p-cdBuff) > datalen+2 &&		/* 2 = ";\x0C" */
		*(cdBuff+buflen-4) == '/'){
		formVer = *(cdBuff+buflen-3);
	}
	
	/* get serial number */
	/* serialNo buffer size is EPS_ADDR_BUFFSIZE */
	switch(formVer){
	case 0x07:
		/* EPS_GET_SERIAL(31, 18) not product serial */
		break;
	case 0x08:
	case 0x10:
		EPS_GET_SERIAL(31, 10)
		break;

	case 0x11:
	default:
		EPS_GET_SERIAL(46, 10)
		break;
	}

	EPS_DBGPRINT(("Ser : %s\n", serialNo));

	EPS_RETURN( ret )
}


EPS_ERR_CODE    serGetSerialNoFormST (

        EPS_INT8*       Status,
        EPS_INT8*       serialNo,
		EPS_INT32       fieldSize

){
    EPS_UINT8*  Field;
    EPS_UINT8*  EndField;
    
    EPS_UINT8   Header;
    EPS_UINT8   ParameterByte;

	EPS_LOG_FUNCIN;

	memset(serialNo, 0, fieldSize);

	if((Field = (EPS_UINT8*)strstr(Status,"ST2")) == NULL ){
        SerDebugPrintf(("EPS SER: Get Stat -> ST not found [%s]\r\n",Status));
        EPS_RETURN( EPS_ERR_COMM_ERROR );
    }

	Field    = Field + 5;
    EndField = Field + (2+(*Field)+ ((*(Field+1))*256) );
    Field    = Field + 2;
    
    while ( Field < EndField ) {
        Header        = (EPS_UINT8) Field[0];
        ParameterByte = (EPS_UINT8) Field[1];

		if( 0x40 == Header ) {
			if( fieldSize > ParameterByte){
				memcpy(serialNo, Field+2, Min(ParameterByte, fieldSize-1));
			    EPS_RETURN( EPS_ERR_NONE );
			}
			break;
		}
        
        Field = Field + 2 + ParameterByte;
    }

    EPS_RETURN( EPS_ERR_OPR_FAIL );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   serGetSupplyInfo()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* Status       EPS_INT8*           I: 'st' command reply                               */
/* info         EPS_SETTING_INFO*   O: serial number                                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*                                                                                      */
/* Description:                                                                         */
/*      parse 'st' command reply to get paper setting status.                           */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    serGetSupplyInfo (

        EPS_UINT8*			Status,
        EPS_SUPPLY_INFO*	info

){
    EPS_UINT8*  Field;
    EPS_UINT8*  EndField;
    
    EPS_UINT8   Header;
    EPS_UINT8   ParameterByte;
    EPS_UINT8   formatVer;
    EPS_UINT8*  param;
    EPS_INT32   unifiedCartridge[EPS_INK_NUM];
    EPS_INT32   inkWarn[EPS_INK_NUM];
    EPS_INT32   i;

#define	EPS_PISS_SIZE	(9)

	EPS_LOG_FUNCIN;

	memset(info, 0, sizeof(EPS_SUPPLY_INFO));
	for(i = 0; i < EPS_INK_NUM; i++){
		unifiedCartridge[i] = -1;
		inkWarn[i] = EPS_INK_ST_NORMAL;
	}

	if((Field = (EPS_UINT8*)strstr((EPS_INT8*)Status, "ST2")) == NULL ){
        SerDebugPrintf(("EPS SER: Get Stat -> ST not found [%s]\r\n",Status));
        EPS_RETURN( EPS_ERR_COMM_ERROR );
    }

	Field    = Field + 5;
    EndField = Field + (2+(*Field)+ ((*(Field+1))*256) );
    Field    = Field + 2;
    
	info->ink.number = -1;
	info->paperSource.number = -1;
	info->powerSource.type = EPS_POWER_SOUECE_NOT_SUPPORTED;

    while ( Field < EndField ) {
        Header        = (EPS_UINT8) Field[0];
		ParameterByte = (EPS_UINT8) Field[1];
        param = Field + 2;
        Field = Field + 2 + ParameterByte;

		switch( Header ) {
        case 0x04: /* WR */
			for(i = 0; i < ParameterByte; i++){
				if( param[i] >= 0x10 && param[i] <= 0x1A ){
					/* Ink Low Warning */
					inkWarn[ param[i]-0x10 ] = EPS_INK_ST_LOW;
				} else if( param[i] >= 0x51 && param[i] <= 0x5A ){
					inkWarn[ param[i]-0x51 ] = EPS_INK_ST_LOW;
				} else if( param[i] >= 0x71 && param[i] <= 0x7A ){
					inkWarn[ param[i]-0x71 ] = EPS_INK_ST_LOW;
				}
			}
            break;

		case 0x0F: /* INK */
			param = EPS_PARSE_CARTRIDGE(ParameterByte, param, &info->ink, unifiedCartridge);
            break;

		case 0x3F:	/* Paper Setting */
			formatVer = *param;
			if( 1 == formatVer ) {
				info->paperSource.number = Min((ParameterByte-1)/EPS_PISS_SIZE, EPS_PAPERSOURCE_NUM);
				param++;
				for(i = 0; i < info->paperSource.number; i++){
					info->paperSource.id[i] = pptops(param);
					info->paperSource.mediaTypeID[i] = *(param + 2);
					info->paperSource.mediaSizeID[i] = *(param + 3);
					param += EPS_PISS_SIZE;
				}
			}
			break;

		case 0x22:	/* Power Supply */
			if( (5 == ParameterByte) && (0x00 == *(param+4)) ){ /* Genuine */
				switch(*param){
				case 0x00: info->powerSource.type = EPS_POWER_SOUECE_AC;		break;
				case 0x01: info->powerSource.type = EPS_POWER_SOUECE_BATTERY;	break;
				default:   info->powerSource.type = EPS_POWER_SOUECE_UNKNOWN;	break;
				}
				switch(*(param+2)){
				case 0x01: info->powerSource.chargeState = EPS_CHARGE_STATE_CHARGING;	break;
				default:
				case 0x00: info->powerSource.chargeState = EPS_CHARGE_STATE_NONE;		break;
				}
				if((*(param+3) >= 0) && (*(param+3) <= 100)) {
					info->powerSource.remaining = *(param+3);
				} else{
					info->powerSource.remaining = 0;
				}
			} else{
				info->powerSource.type = EPS_POWER_SOUECE_UNKNOWN;
				info->powerSource.chargeState = EPS_CHARGE_STATE_NONE;
				info->powerSource.remaining = 0;
			}
			break;
		}
    }

	{
		EPS_INT32 warnIdx = 0;
		for(i = 0; i < info->ink.number; i++){
			if(EPS_INK_ST_NORMAL == info->ink.status[i]){
				if( EPS_INK_ST_NORMAL != inkWarn[warnIdx] ){
					info->ink.status[i] = inkWarn[warnIdx];
				}
			}
			if( i < info->ink.number-1 &&
				unifiedCartridge[i] != unifiedCartridge[i+1]){ /* Cartridge transition */
				warnIdx++;
			}
		}
	}

    EPS_RETURN( EPS_ERR_NONE );
}

#ifndef GCOMSW_EXTENTION
EPS_UINT8* parseCartridge(
	
	EPS_UINT8		ParameterByte, 
	EPS_UINT8*		param, 
	EPS_INK_INFO*	ink, 
	EPS_INT32		cartridges[]
	
){
    EPS_UINT8   blockSize;
    EPS_INT32   i;

	blockSize = *param;
	ink->number = Min((ParameterByte-1) / blockSize, EPS_INK_NUM);

	param++;

	for( i=0; i < ink->number; i++ ) {
		cartridges[i] = *param - 0x40; /* Positive number is unified cartridge index */
		if( cartridges[i] >= EPS_INK_NUM ){
			cartridges[i] = -1;
		}
				
        switch( *(param+1) ) {
            case 0x00:    ink->colors[i] = EPS_COLOR_BLACK;        break;
            case 0x01:    ink->colors[i] = EPS_COLOR_CYAN;         break;
            case 0x02:    ink->colors[i] = EPS_COLOR_MAGENTA;      break;
            case 0x03:    ink->colors[i] = EPS_COLOR_YELLOW;       break;
            case 0x04:    ink->colors[i] = EPS_COLOR_LIGHTCYAN;    break;
            case 0x05:    ink->colors[i] = EPS_COLOR_LIGHTMAGENTA; break;
            case 0x06:    ink->colors[i] = EPS_COLOR_LIGHTYELLOW;  break;
            case 0x07:    ink->colors[i] = EPS_COLOR_DARKYELLOW;   break;
            case 0x08:    ink->colors[i] = EPS_COLOR_LIGHTBLACK;   break;
            case 0x09:    ink->colors[i] = EPS_COLOR_RED;          break;
            case 0x0A:    ink->colors[i] = EPS_COLOR_VIOLET;       break;
            case 0x0B:    ink->colors[i] = EPS_COLOR_CLEAR;        break;
            case 0x0C:    ink->colors[i] = EPS_COLOR_LIGHTLIGHTBLACK; break;
            case 0x0D:    ink->colors[i] = EPS_COLOR_ORANGE;       break;
            case 0x0E:    ink->colors[i] = EPS_COLOR_GREEN;        break;
            case 0x0F:    ink->colors[i] = EPS_COLOR_WHITE;        break;
            case 0x40:    ink->colors[i] = EPS_COLOR_COMPOSITE;    break;
            default:     
				ink->colors[i] = EPS_COLOR_UNKNOWN;      
				break;
        }
        switch( *param ) {
            case 0x0B:    ink->colors[i] = EPS_COLOR_MATTEBLACK;   break;
            default:      break;
		}

        switch( *(param+2) ) {
        case 'w' :
        case 'r' :
			ink->status[i] = EPS_INK_ST_FAIL;
            break;
        case 'n' :
			ink->status[i] = EPS_INK_ST_NOTPRESENT;
            break;
        case 'i' :
			ink->status[i] = EPS_INK_ST_NOREAD;
            break;
        case 'g' :
			ink->status[i] = EPS_INK_ST_NOTAVAIL;
            break;
		case 0:
			ink->status[i] = EPS_INK_ST_END;
        default:
            break;
        }
		ink->remaining[i] = EPS_INK_OBSOLETE;

		param += blockSize;
    }

	return param;
}
#endif

EPS_UINT32 pptops(EPS_UINT8* pp)
{
	switch(*pp){
	case 0x01:
		switch(*(pp+1)){
		case 0x00:	return EPS_MPID_REAR;	break;
		case 0x01:	return EPS_MPID_FRONT1;	break;
		case 0x02:	return EPS_MPID_FRONT2;	break;
		case 0x03:	return EPS_MPID_FRONT3;	break;
		case 0x04:	return EPS_MPID_FRONT4;	break;
		case 0xFF:	return EPS_MPID_AUTO;	break;
		}
		break;
	case 0x02:
		switch(*(pp+1)){
		case 0x00:	return EPS_MPID_MANUAL;	break;
		case 0x01:	return EPS_MPID_CDTRAY;	break;
		case 0x02:	return EPS_MPID_MANUAL2;break;
		}
		break;
	case 0x03:
		switch(*(pp+1)){
		case 0x00:	return EPS_MPID_ROLL;	break;
		}
		break;
	}
	return EPS_MPID_NOT_SPEC;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   serDelayThread()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* milliseconds EPS_UINT32          I: Sleep Period in microseconds                     */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Wait <milliseconds>.                                                            */
/*      If OS sleep function is used, change the unit of sleep time from milliseconds   */
/*      to microseconds.                                                                */
/*                                                                                      */
/*******************************************|********************************************/
void    serDelayThread (

        EPS_UINT32		milliseconds,
		EPS_CMN_FUNC*	epsCmnFnc

){

#ifdef GCOMSW_EPSON_SLEEP
    if (epsCmnFnc->sleep == serSleep) {
        epsCmnFnc->sleep((EPS_UINT32)milliseconds); /* Ignore return value of sleep func */
    } else{
        epsCmnFnc->sleep(milliseconds * 1000);      /* Ignore return value of sleep func */
    }
#else
    epsCmnFnc->sleep(milliseconds * 1000);          /* Ignore return value of sleep func */
#endif /* GCOMSW_EPSON_SLEEP */

}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   serSleep()                                                          */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* sleepPeriod  EPS_UINT32          I: Sleep Period in milliseconds                     */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*                                                                                      */
/* Description:                                                                         */
/*      ESC/P-R Lib original sleep function.                                            */
/*      This function is used when "epsCmnFnc.sleep = NULL".                            */
/*                                                                                      */
/*******************************************|********************************************/
#ifdef GCOMSW_EPSON_SLEEP
EPS_ERR_CODE    serSleep (

        EPS_UINT32  sleepPeriod             /* Sleep Period in milliseconds             */

){

/*** Declare Variable Local to Routine                                                  */
    EPS_INT32    idx;                       /* General loop/index varaible              */
    EPS_INT32    endx;
    struct timeb sleepM;

/*** Initialize Local Variables                                                         */
    endx = sleepPeriod*printJob.sleepSteps;
    if (printJob.sleepSteps <= 0) endx = sleepPeriod/(-printJob.sleepSteps);

/*** Sleep for about the requested sleepPeriod                                          */
    for (idx = 1; idx < endx; idx++) {ftime(&sleepM); } 

/*** Return to Caller                                                                   */
    return((EPS_ERR_CODE)EPS_ERR_NONE);

}
#endif /* GCOMSW_EPSON_SLEEP */


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   obsSetPrinter()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printer      EPS_PRINTER_INN8*   I: Pointer to printer info                          */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      set the observation target printer.                                             */
/*                                                                                      */
/*******************************************|********************************************/
void     obsSetPrinter (

        const EPS_PRINTER_INN* printer

){
	g_observer.printer = printer;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   obsSetColorPlane()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* colorPlane   EPS_UINT8           I: color plane                                      */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      set color plane of current job.                                                 */
/*                                                                                      */
/*******************************************|********************************************/
void     obsSetColorPlane (

        EPS_UINT8 colorPlane

){
	g_observer.colorPlane = colorPlane;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   obsClear()                                                          */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* N/A                                                                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      None                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      clear all property.                                                             */
/*                                                                                      */
/*******************************************|********************************************/
void     obsClear (

        void

){
	g_observer.printer = NULL;
	g_observer.colorPlane = EPS_CP_FULLCOLOR;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   obsGetPageMode()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* N/A                                                                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      Means to page process.                                                          */
/*                                                                                      */
/* Description:                                                                         */
/*      The means to page process is decided.                                           */
/*                                                                                      */
/*******************************************|********************************************/
EPS_INT32     obsGetPageMode (

        void

){
	const EPS_INT8 *modelName = NULL;

	if( NULL == g_observer.printer ){
		return EPS_PM_PAGE;
	}

	if(EPS_CP_JPEG == g_observer.colorPlane )
	{
		modelName = g_observer.printer->modelName;
		if( (strcmp(modelName, "EP-801A"             ) == 0) ||
		    (strcmp(modelName, "Artisan 700"         ) == 0) ||
		    (strcmp(modelName, "Stylus Photo TX700W" ) == 0) ||
		    (strcmp(modelName, "Stylus Photo PX700W" ) == 0) ||
		    (strcmp(modelName, "EP-901F"             ) == 0) ||
		    (strcmp(modelName, "EP-901A"             ) == 0) ||
		    (strcmp(modelName, "Artisan 800"         ) == 0) ||
		    (strcmp(modelName, "Stylus Photo PX800FW") == 0) ||
		    (strcmp(modelName, "Stylus Photo TX800FW") == 0) )
		{
			return EPS_PM_JOB;
		}
	}

	return EPS_PM_PAGE;
}


EPS_BOOL    obsIsA3Model (
							  
		EPS_INT32 ch
							  
){
	const EPS_INT8 *modelName = NULL;
	
	modelName = g_observer.printer->modelName;
	if(EPS_MDC_STATUS == ch){			/* status code */
		if( (strcmp(modelName, "PX-5V"						) == 0) ||
			   (strcmp(modelName, "Epson Stylus Photo R3000") == 0) ||
			   (strcmp(modelName, "PX-7V"					) == 0) ||
			   (strcmp(modelName, "Epson Stylus Photo R2000") == 0) ||
			   (strcmp(modelName, "EP-4004"					) == 0) ||
			   (strcmp(modelName, "Artisan 1430"            ) == 0) ||
			   (strcmp(modelName, "Epson Stylus Photo 1430") == 0)  ||
			   (strcmp(modelName, "Epson Stylus Photo 1500") == 0) )
		{
			return TRUE;
		}
	} else if(EPS_MDC_NOZZLE == ch){	/* nozzle patern */
		if( (strcmp(modelName, "PX-5V"						) == 0) ||
			   (strcmp(modelName, "Epson Stylus Photo R3000") == 0) ||
			   (strcmp(modelName, "PX-7V"					) == 0) ||
			   (strcmp(modelName, "Epson Stylus Photo R2000") == 0) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

static EPS_INT8 modelFY11Bussiness[][16] = {
					"PX-1600F", "WF-7510 Series", "WF-7511 Series", "WF-7515 Series",
					"PX-1700F", "WF-7520 Series", "WF-7521 Series", "WF-7525 Series",
					"PX-1200", "WF-7010 Series", "WF-7011 Series", "WF-7012 Series", "WF-7015 Series",
					"PX-B750F", "WP-4511 Series", "WP-4515 Series", "WP-4521 Series", "WP-4525 Series", 
					"WP-4530 Series", "WP-4531 Series", "WP-4535 Series", 
					"WP-4540 Series", "WP-4545 Series", 
					"PX-B700", "WP-4015 Series", "WP-4011 Series", "WP-4020 Series", "WP-4025 Series"
};

EPS_BOOL    obsEnableDuplex (
							  
		EPS_INT32	sizeID
							  
){
	const EPS_INT8 *modelName = NULL;
	EPS_INT32 i = 0;
	
	if( !(sizeID == EPS_MSID_A4		|| 
		sizeID == EPS_MSID_LETTER	|| 
		sizeID == EPS_MSID_B5		))
	{
		modelName = g_observer.printer->modelName;
		for(i = 0; i < 28; i++){
			if( strcmp(modelName, modelFY11Bussiness[i]) == 0){
				return FALSE;
			}
		}
	}

	return TRUE;
}

EPS_BOOL    obsEnableAutoFeed (
							  
		void
							  
){
	const EPS_INT8 *modelName = NULL;
	EPS_INT32 i = 0;
	
	modelName = g_observer.printer->modelName;
	for(i = 0; i < 28; i++){
		if( strcmp(modelName, modelFY11Bussiness[i]) == 0){
			return TRUE;
		}
	}

	return FALSE;
}
/*____________________________   epson-escpr-services.c   ______________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
