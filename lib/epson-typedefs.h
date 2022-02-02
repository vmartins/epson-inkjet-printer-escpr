/*________________________________  epson-typedefs.h   _________________________________*/

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
/*                           Epson Common Type Definitions                              */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef EPSON_TYPEDEFS_H_
#define EPSON_TYPEDEFS_H_
#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------  System Includes   ---------------------------------*/
/*******************************************|********************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef GCOMSW_EPSON_SLEEP
#include <sys/timeb.h>
#endif

/*------------------------------- Define Basic Data Types ------------------------------*/
/*******************************************|********************************************/
typedef unsigned char  EPS_UINT8;       /* unsigned  8-bit  Min: 0          Max: 255        */
typedef unsigned short EPS_UINT16;      /* unsigned 16-bit  Min: 0          Max: 65535      */
typedef unsigned int   EPS_UINT32;      /* unsigned 32-bit  Min: 0          Max: 4294967295 */
typedef signed char    EPS_INT8;        /*   signed  8-bit  Min: -128       Max: 127        */
typedef short          EPS_INT16;       /*   signed 16-bit  Min: -32768     Max: 32767      */
typedef int            EPS_INT32;       /*   signed 32-bit  Min:-2147483648 Max: 2147483647 */
typedef float          EPS_FLOAT;       /*    float 32-bit  Min:3.4E-38     Max: 3.4E+38    */
typedef EPS_INT32      EPS_BOOL;        /* Boolean type                                     */
typedef EPS_INT32      EPS_ERR_CODE;    /* Error code for API's and routines                */
typedef int            EPS_SOCKET;      /* socket discripter                                */

#if defined(unix)||defined(__unix)
typedef int            EPS_FILEDSC;     /* file discripter                                  */
#else
typedef void*          EPS_FILEDSC;     /* file discripter                                  */
#endif

/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/

/*----------------------------------- Data Structure -----------------------------------*/
/*******************************************|********************************************/

/*--------------------------------- Basic State Types ----------------------------------*/
/*******************************************|********************************************/
#ifndef NULL
#define NULL            0                   /* Value of null                            */
#endif

#ifndef TRUE
#define TRUE            1                   /* Value of "true"                          */
#endif

#ifndef FALSE
#define FALSE           0                   /* Value of "false"                         */
#endif

	/*** USB                                                                            */
    /*** -------------------------------------------------------------------------------*/
    /* sign of invalid file discripter                                                  */
#define EPS_INVALID_FILEDSC        (EPS_FILEDSC)(-1)	  


	/*** Socket                                                                         */
    /*** -------------------------------------------------------------------------------*/
    /* sign of invalid socket                                                           */
#define EPS_INVALID_SOCKET        (EPS_SOCKET)(-1)	  

	/*** Socket function retrieve value                                                 */
    /*** -------------------------------------------------------------------------------*/
#define EPS_SOCKET_SUCCESS          (0)
#define EPS_SOCKET_ERROR            (-1)	  
#define EPS_SOCKET_TIMEOUT          (-2)

    /* protocol family                                                                  */
#define EPS_PF_INET				1		/* Internetwork protocol family					*/
	/* semantics                                                                        */
#define	EPS_SOCK_STREAM			1		/* Stream socket								*/
#define	EPS_SOCK_DGRAM			2		/* Datagram socket								*/
	/* protocol                                                                         */
#define EPS_PROTOCOL_TCP		1		/* TCP protocol									*/
#define EPS_PROTOCOL_UDP		2		/* UDP protocol									*/
	/* shutdown method                                                                  */
#define EPS_SHUTDOWN_SEND		1		/* Shutdown dir: send							*/
#define EPS_SHUTDOWN_RECV		2		/* Shutdown dir: receive						*/
#define EPS_SHUTDOWN_BOTH		3		/* Shutdown dir: both							*/

#ifdef __cplusplus
}
#endif

#endif /* for EPSON_TYPEDEFS_H_                                                         */

/*________________________________  epson-typedefs.h   _________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
