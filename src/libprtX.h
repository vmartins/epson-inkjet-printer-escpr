/*
 * Epson Inkjet Printer Driver (ESC/P-R) for Linux
 * Copyright (C) Seiko Epson Corporation 2002-2015.
 *
 *  This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA.
 */

#ifndef LIB_PRT_X_H
#define LIB_PRT_X_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define NAME_MAX 41

#include "pipsDef.h"
#include "../lib/epson-escpr-pvt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*** Index of printing quality                                                      */
/*** -------------------------------------------------------------------------------*/
typedef EPS_INT32 (* EPS_FPSPOOLFUNC)(void* hParam, 
        const EPS_UINT8* pBuf, EPS_UINT32 cbBuf);

typedef struct tagEPS_OPT{
    EPS_FPSPOOLFUNC    fpspoolfunc;    /* pointer to spool function */
}EPS_OPT;

typedef struct rtp_filter_option {
	char model[NAME_MAX + 1];
	char model_low[NAME_MAX + 1];
	char ink[NAME_MAX + 1];
	char media[NAME_MAX + 1];
	char quality[NAME_MAX + 1];
	char duplex[NAME_MAX + 1];
	char inputslot[NAME_MAX + 1];
	char brightness[NAME_MAX + 1];
	char contrast[NAME_MAX + 1];
	char saturation[NAME_MAX + 1];
} filter_option_t;


typedef void *handle_t;
typedef int (*INIT_FUNC) (const EPS_OPT *, const EPS_PRINT_JOB*);
typedef int (*PINIT_FUNC) (void);
typedef int (*OUT_FUNC) (const EPS_BANDBMP *, EPS_RECT *);
typedef int (*PEND_FUNC) (const EPS_UINT8);
typedef int (*END_FUNC) (void);  
typedef int (*FILTER_PMPEPLY_FUNC) (EPS_UINT8 *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIB_PRT_X_H */
