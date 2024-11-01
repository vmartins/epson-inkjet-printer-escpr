#ifndef __EPCG_CUSTOM_PAGESIZE_H__
#define __EPCG_CUSTOM_PAGESIZE_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../lib/epson-typedefs.h"


#define	EPS_ERR_NONE   0
#define EPS_ERR_INV_ARGUMENT -1
	

EPS_ERR_CODE
convert_custom_pagesize_to_dot(const EPS_INT8 * pageSizeID, EPS_FLOAT * width, EPS_FLOAT * height, EPS_INT32 base_dpi);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EPCG_CUSTOM_PAGESIZE_H__ */

