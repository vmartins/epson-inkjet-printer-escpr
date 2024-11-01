#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "custompage.h"
#include <math.h>
typedef EPS_INT32 (* UnitConversionFunction) (const EPS_INT8 *, EPS_FLOAT *, EPS_FLOAT *, EPS_INT32);
static const EPS_INT8 * WIDTH_x_HEIGHT = "%fx%f";

static EPS_INT32 /* return 0 if conversion succeeded or error */
unit_convert_inch_to_dot(const EPS_INT8 * pageSizeID, EPS_FLOAT * width, EPS_FLOAT * height, EPS_INT32 base_dpi)
{


	const EPS_INT8 * UNIT = "in";

	EPS_INT32 scanNum = 0;
	EPS_FLOAT w;
	EPS_FLOAT h;

	if(strstr(pageSizeID, UNIT)) {
		scanNum = sscanf(pageSizeID, WIDTH_x_HEIGHT, &w, &h);
		if(scanNum == 2) {
			*width = w * base_dpi;
			*height = h * base_dpi;
		}
	}

	return (scanNum == 2) ? 0 : 1;
}

static EPS_INT32 /* return 0 if conversion succeeded or error */
unit_convert_cm_to_dot(const EPS_INT8 * pageSizeID, EPS_FLOAT * width, EPS_FLOAT * height, EPS_INT32 base_dpi)
{
	const EPS_INT8 * UNIT = "cm";

	EPS_INT32 scanNum = 0;
	EPS_FLOAT w;
	EPS_FLOAT h;

	if(strstr(pageSizeID, UNIT)) {
		scanNum = sscanf(pageSizeID, WIDTH_x_HEIGHT, &w, &h);
		if(scanNum == 2) {
			*width = w / 25.4 * base_dpi;
			*height = h / 25.4 * base_dpi;
		}
	}

	return (scanNum == 2) ? 0 : 1;
}

static EPS_INT32 /* return 0 if conversion succeeded or error */
unit_convert_mm_to_dot(const EPS_INT8 * pageSizeID, EPS_FLOAT * width, EPS_FLOAT * height, EPS_INT32 base_dpi)
{
	const EPS_INT8 * UNIT = "mm";

	EPS_INT32 scanNum = 0;
	EPS_FLOAT w;
	EPS_FLOAT h;

	if(strstr(pageSizeID, UNIT)) {
		scanNum = sscanf(pageSizeID, WIDTH_x_HEIGHT, &w, &h);
		if(scanNum == 2) {
			*width = w / 25.4 * base_dpi;
			*height = h / 25.4 * base_dpi;
		}
	}

	return (scanNum == 2) ? 0 : 1;
}

static EPS_INT32 /* return 0 if conversion succeeded or error */
unit_convert_72dpi_to_dot(const EPS_INT8 * pageSizeID, EPS_FLOAT * width, EPS_FLOAT * height, EPS_INT32 base_dpi)
{
	EPS_INT32 scanNum = 0;
	EPS_FLOAT w;
	EPS_FLOAT h;

	scanNum = sscanf(pageSizeID, WIDTH_x_HEIGHT, &w, &h);
	if(scanNum == 2) {
		*width = w * base_dpi / 72.0;
		*height = h * base_dpi / 72.0;
	}

	return (scanNum == 2) ? 0 : 1;
}


EPS_ERR_CODE
convert_custom_pagesize_to_dot(const EPS_INT8 * pageSizeID, EPS_FLOAT * width, EPS_FLOAT * height, EPS_INT32 base_dpi)
{
	const EPS_INT8 * CUSTOM = "custom.";
	static char CustomPageSize [256];

	UnitConversionFunction conversionFunctions [] = {
		unit_convert_inch_to_dot,
		unit_convert_cm_to_dot,
		unit_convert_mm_to_dot,
		unit_convert_72dpi_to_dot,
		NULL
	};

	UnitConversionFunction * convert = &conversionFunctions [0];

	EPS_ERR_CODE error = EPS_ERR_INV_ARGUMENT;
	EPS_FLOAT widthDot;
	EPS_FLOAT heightDot;
	EPS_INT8 * p = NULL;
	EPS_INT32 i;
	EPS_INT32 len;

	if (strncasecmp(pageSizeID, CUSTOM, strlen(CUSTOM)) == 0) {
		memset(CustomPageSize, 0x00, sizeof(CustomPageSize));
		len = strlen(pageSizeID);
		p = &CustomPageSize [0];
		for(i = strlen(CUSTOM); i < len; i++) {
			*p = pageSizeID[i];
			if(*p >= 'A' && *p <= 'Z') {
				*p += ('a' - 'A');
			}
			p++;
		}

		while(*convert != NULL) {
			error = (*convert)(CustomPageSize, &widthDot, &heightDot, base_dpi);
			if (error == EPS_ERR_NONE) {
				break;
			}

			convert++;
		}
	}

	if (error == EPS_ERR_NONE) {
		*width = widthDot;
		*height = heightDot;

		// Truncate to the seconddecimal place
		int tmp_width = floor(*width * 100.0);
		*width = tmp_width / 100.0;

		int tmp_height = floor(*height * 100.0);
		*height = tmp_height / 100.0;

	} else {
		error = EPS_ERR_INV_ARGUMENT;
	}

	return error;
}
