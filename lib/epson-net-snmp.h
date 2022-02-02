/*_______________________________   epson-net-snmp.h   _________________________________*/

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
/*                           Epson SNMP Protocol Definitions                            */
/*                                                                                      */
/*******************************************|********************************************/

#ifndef __EPSON_NET_SNMP_H__
#define __EPSON_NET_SNMP_H__
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"
#include "epson-escpr-pvt.h"


/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/
/* SNMP variables                                                                       */
#define ASN_VT_INTEGER			(0x02)				/* integer                          */
#define ASN_VT_OCTET_STRING		(0x04)				/* octet stream                     */
#define ASN_VT_NULL				(0x05)				/* null                             */
#define ASN_VT_OBJECT_ID		(0x06)				/* OID                              */
#define ASN_VT_SEQUENCE			(0x30)				/* sequence                         */

/* SNMP message                                                                         */
#define ASN_PDU_GET				(EPS_UINT8)(0xA0)   /* GetRequest                       */
#define ASN_PDU_GET_NEXT		(EPS_UINT8)(0xA1)   /* GetNextRequest                   */
#define ASN_PDU_RESP			(EPS_UINT8)(0xA2)   /* Response                         */
#define ASN_PDU_SET				(EPS_UINT8)(0xA3)   /* SetRequest                       */
#define ASN_PDU_TRAP			(EPS_UINT8)(0xA4)   /* Trap                             */
#define ASN_PDU_TRAP_RSP		(EPS_UINT8)(0xC2)   /* TrapResponse                     */


/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/
/* SNMP variant type */
typedef struct tag_ASN_VARIANT {       
	EPS_INT8	type;
	EPS_UINT32	length;
	union{
		EPS_INT32	v_long;            /* Integer                                       */
		EPS_INT8*	v_str;             /* OCTET_STRING, ASN_VT_OBJECT_ID                */
	} val;				
}ASN_VARIANT;

typedef struct tag_EPS_SNMP_VARBIND {
	const EPS_INT8*	identifire;
	ASN_VARIANT	value;
}EPS_SNMP_VARBIND;


typedef struct tag_EPS_SNMP_PDU {
	EPS_INT32			requestID;
	EPS_UINT32			errorStatus;
	EPS_UINT32			errorIndex;
	EPS_SNMP_VARBIND	*var;
	EPS_INT32			varNum;
}EPS_SNMP_PDU;


/* If type is OBJECT_ID, ParseField() alloc heap.   									*/
#define EPS_SNMP_REREASE_VARIANT( v )			\
	{											\
		if( ASN_VT_OBJECT_ID == (v).type ){		\
			EPS_SAFE_RELEASE( (v).val.v_str );	\
		}										\
		(v).type = ASN_VT_NULL;					\
	}
#define EPS_SNMP_REREASE_PDU( v )	     		\
	{											\
		EPS_INT32 vc;                           \
		for( vc = 0; vc < (v).varNum; vc++ ){	\
			EPS_INT8* p = (EPS_INT8*)((v).var[vc].identifire); \
			EPS_SAFE_RELEASE( p );	\
		}										\
		EPS_FREE( (v).var );                    \
	}

/****************************************************************************************/
/* public SNMP functions                                                                */
typedef EPS_ERR_CODE	(*SNMP_GetStatus    )(EPS_SOCKET, const EPS_INT8*, EPS_UINT32, 
	                                          EPS_STATUS_INFO*);
typedef EPS_ERR_CODE	(*SNMP_GetInkInfo   )(const EPS_INT8*, EPS_STATUS_INFO*         );
typedef EPS_ERR_CODE	(*SNMP_InfoCommand  )(const EPS_PRINTER_INN*, EPS_INT32, 
										      EPS_UINT8**, EPS_INT32*                   );
typedef EPS_ERR_CODE	(*SNMP_MechCommand  )(const EPS_PRINTER_INN*, EPS_INT32         );

typedef struct tagEPS_SNMP_FUNCS {
	SNMP_GetStatus		GetStatus;
	SNMP_GetInkInfo		GetInkInfo;
	SNMP_InfoCommand    InfoCommand;
	SNMP_MechCommand	MechCommand;
	EPS_UINT32			egID;
}EPS_SNMP_FUNCS;


/*--------------------------  Public Function Declarations   ---------------------------*/
/*******************************************|********************************************/
extern void			snmpSetupSTFunctions(EPS_SNMP_FUNCS*, const EPS_PRINTER_INN*        );

extern EPS_ERR_CODE snmpOpenSocket  (EPS_SOCKET*                                        );
extern void         snmpCloseSocket (EPS_SOCKET*                                        );
extern EPS_ERR_CODE snmpTransactS   (EPS_SOCKET, const EPS_INT8*, EPS_INT32, 
									 const EPS_INT8*, EPS_UINT8, const EPS_INT8*, 
                                     const ASN_VARIANT*, ASN_VARIANT*);

extern EPS_ERR_CODE snmpFindStart   (EPS_SOCKET*, const EPS_INT8*, EPS_BOOL, const EPS_UINT8*);
extern EPS_ERR_CODE snmpFind        (EPS_SOCKET, EPS_UINT16, EPS_INT32, EPS_PRINTER_INN**);
extern EPS_ERR_CODE snmpFindEnd	    (EPS_SOCKET                                         );
extern EPS_ERR_CODE snmpProbeByID	(EPS_INT8*, EPS_UINT16, EPS_INT32, EPS_UINT32, EPS_PRINTER_INN**);

extern EPS_ERR_CODE snmpCreatePDU   (EPS_INT8*, EPS_UINT8, EPS_INT32, 
	                                 const EPS_SNMP_VARBIND*, EPS_INT32, EPS_INT32*     );
extern EPS_ERR_CODE snmpParsePDU    (EPS_INT8*, EPS_INT32, EPS_SNMP_PDU*                );
extern EPS_INT8*    snmpMakeIntField(EPS_INT32, EPS_INT8*                               );
extern EPS_INT8*    snmpMakeStrField(const EPS_INT8*, EPS_UINT32, EPS_INT8*             );
extern EPS_ERR_CODE snmpParseField  (EPS_INT8**, EPS_INT32*, ASN_VARIANT*               );
extern EPS_INT32    snmpGetRequestId(void                                               );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* def __EPSON_NET_SNMP_H__ */

/*_______________________________   epson-net-snmp.h   _________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
