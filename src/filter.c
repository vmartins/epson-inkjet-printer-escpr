/*
 * Epson Inkjet Printer Driver (ESC/P-R) for Linux
 * Copyright (C) Seiko Epson Corporation 2002-2016.
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#define HAVE_PPM (0)

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "epson-protocol.h"
#include "epson-escpr-api.h"
#include "epson-escpr-mem.h"

#include "err.h"
#include "mem.h"
#include "str.h"
#include "debug.h"
#include "libprtX.h"
#include "optBase.h"
#include "linux_cmn.h"
#include "custompage.h"

#define WIDTH_BYTES(bits) (((bits) + 31) / 32 * 4)

#define PIPSLITE_FILTER_VERSION "* epson-escpr is a part of " PACKAGE_STRING

#define PIPSLITE_FILTER_USAGE "Usage: $ epson-escpr model width_pixel height_pixel Ink PageSize MediaType Duplex InputSlot Brightness Contrast Saturation"

FILE* outfp = NULL;

/* static functions */
static int set_pips_parameter (filter_option_t *, EPS_OPT *);
static int getMediaTypeID(char *);
static EPS_INT32 print_spool_fnc(void* , const EPS_UINT8*, EPS_UINT32);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
const int band_line = 1;
extern EPS_INT32    libStatus;                  /*  Library (epsInitDriver) status      */
extern EPS_PRINT_JOB   printJob;
extern EPS_UINT32   sendDataBufSize;
extern EPS_UINT8*   sendDataBuf;    /* buffer of SendCommand(save) input                */
extern EPS_UINT32   tmpLineBufSize;
extern EPS_UINT8*   tmpLineBuf;

extern EPS_CMN_FUNC epsCmnFnc;
EPS_JOB_FUNCS		jobFnc;

extern EPS_INT32 tonerSave;
extern EPS_INT32 back_type;
extern EPS_INT32 lWidth;
extern EPS_INT32 lHeight;
extern EPS_INT32 areaWidth;
extern EPS_INT32 areaHeight;

#ifndef ESCPR_HEADER_LENGTH	
#define ESCPR_HEADER_LENGTH            10    /* ESC + CLASS + ParamLen + CmdName */	
#endif

#ifndef ESCPR_SEND_DATA_LENGTH
#define ESCPR_SEND_DATA_LENGTH          7
#endif

EPS_JOB_ATTRIB     jobAttr;
void* context;
///////////////////////////////////////////////////////////////////////////////////////////////////////////

EPS_ERR_CODE epsInitLib(){
	EPS_CMN_FUNC cmnFuncPtrs;
	memset(&cmnFuncPtrs, 0, sizeof(EPS_CMN_FUNC));
	
	cmnFuncPtrs.version = EPS_CMNFUNC_VER_CUR;
	cmnFuncPtrs.findCallback = NULL;
	cmnFuncPtrs.memAlloc = &epsmpMemAlloc;
	cmnFuncPtrs.memFree = &epsmpMemFree;
	cmnFuncPtrs.sleep = &epsmpSleep;
	cmnFuncPtrs.getTime = &epsmpGetTime;
	cmnFuncPtrs.getLocalTime = &epsmpGetLocalTime;
	cmnFuncPtrs.lockSync = &epsmpLockSync;
	cmnFuncPtrs.unlockSync = &epsmpUnlockSync;
	cmnFuncPtrs.stateCallback = NULL; /* current version unused */

	memcpy((void*)(&epsCmnFnc), (void*)&cmnFuncPtrs, sizeof(EPS_CMN_FUNC));
	
	return EPS_ERR_NONE;
}

EPS_ERR_CODE epsInitJob(){
	
	memset(&printJob, 0, sizeof(EPS_PRINT_JOB));
	printJob.printer = (EPS_PRINTER_INN*) malloc(sizeof(EPS_PRINTER_INN));
	memset(printJob.printer, 0, sizeof(EPS_PRINTER_INN));
	
	printJob.jobStatus  = EPS_STATUS_NOT_INITIALIZED;
	printJob.pageStatus = EPS_STATUS_NOT_INITIALIZED;
	printJob.findStatus = EPS_STATUS_NOT_INITIALIZED;
	printJob.bComm		= TRUE;	
	printJob.platform   = 0x04; /* '0x04 = linux' is default */ 

	libStatus   = EPS_STATUS_NOT_INITIALIZED;
	sendDataBufSize = 0;
	sendDataBuf = NULL;
	tmpLineBufSize = 0;
	tmpLineBuf  = NULL;

/*** Initialize continue buffer                                                         */
	printJob.contData.sendData = NULL;
	printJob.contData.sendDataSize = 0;

	obsClear();

/*** Set "Endian-ness" for the current cpu                                              */
	memInspectEndian();
	
/*** Change ESC/P-R Lib Status                                                          */
	libStatus = EPS_STATUS_INITIALIZED;
	EPS_PRINTER_INN*    printer = printJob.printer;
//	printer->pmData.state = EPS_PM_STATE_NOT_FILTERED;
	printer->language = EPS_LANG_ESCPR;
	return EPS_ERR_NONE;
}

EPS_ERR_CODE epsInitVariable(){
	sendDataBufSize = (EPS_INT32)(ESCPR_HEADER_LENGTH    +
									ESCPR_SEND_DATA_LENGTH +
									(printJob.printableAreaWidth * printJob.bpp));
	sendDataBuf = (EPS_UINT8*)EPS_ALLOC(sendDataBufSize);
	if(sendDataBuf == NULL){
		sendDataBufSize = 0;
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
	}
	
	memset(sendDataBuf, 0xFF, (EPS_UINT32)sendDataBufSize);
	/*** Allocate buffer for RLE complession                                        */
	tmpLineBufSize = (EPS_INT32)(printJob.printableAreaWidth * printJob.bpp) + 256; /* 256 is temp buffer */
	tmpLineBuf = (EPS_UINT8*)EPS_ALLOC(tmpLineBufSize);
	if(tmpLineBuf == NULL){
		tmpLineBufSize = 0;
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
	}
	memset(tmpLineBuf, 0xFF, (EPS_UINT32)tmpLineBufSize);
	return EPS_ERR_NONE;
}

void eps_toupper(char *str){
	int i = 0;
	int len;
	if(str == NULL || strlen(str) == 0){
		return;
	}
	len = strlen(str);
	for(i = 0; i < len; i++){
		str[i] = toupper(str[i]);
	}
	return;
}

static int  getMediaSizeID(char *media_name){
  int j;
	debug_msg("media name = %s\n", media_name);

  if (strncmp(media_name, "Custom.",7) == 0){
	return PM_MSID_USER;
  }

  for(j = 0; mediaSizeData[j].value != END_ARRAY; j++){
    if((strlen(mediaSizeData[j].rsc_name) == strlen(media_name)) && strncmp(mediaSizeData[j].rsc_name,media_name, strlen(mediaSizeData[j].rsc_name)) == 0){
    	debug_msg("mediaSizeData[%d].rsc_name: %s, x_name = %s\n", j, mediaSizeData[j].rsc_name, mediaSizeData[j].x_name);
      return mediaSizeData[j].value;
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////


/* Useage: epson-escpr model width_pixel height_pixel Ink PageSize MediaType */
int
main (int argc, char *argv[])
{
	filter_option_t fopt;

	long width_pixel, height_pixel;
	long HWResolution;
	long bytes_per_line;
	int byte_par_pixel;
	double x_mag, y_mag;
	double print_area_x, print_area_y;
	char *image_raw;
	unsigned char *band;

	long read_size = 0;
	long read_page_no = 0;
	int band_line;

	int err = 0;
	int i, j;

	/* 2004.04.15 added for 'cancel page' */
	int cancel = 0;
	
	/* 2005.11.28 added  */
	char *paper;
	char *point;

	/* library options */
	EPS_OPT printOpt;
	EPS_BANDBMP bandBmp;

	/* Fifo for Backend */
//	context = (void*)XFIFOOpen();

///////////////////////////////////////////////////////////////////////////////////////////////////////////
	memset (&jobAttr, 0, sizeof(jobAttr));
///////////////////////////////////////////////////////////////////////////////////////////////////////////

/* attach point */
#ifdef USE_DEBUGGER
	int flag = 1;
	while (flag) sleep (3);
#endif /* USE_DEBUGGER */

	DEBUG_START;
	err_init (argv[0]);

	if (argc != 14)
	{
		for ( i = 1; i < argc; i++ ) {
			if ( (0 == strncmp(argv[i], "-v", (strlen("-v")+1)) )
				|| (0 == strncmp(argv[i], "--version", (strlen("--version")+1)) )
				) {
				fprintf(stderr, "%s\n", PIPSLITE_FILTER_VERSION);
				return 0;
			} else if ( (0 == strncmp(argv[i], "-u", (strlen("-u")+1)) )
				|| (0 == strncmp(argv[i], "--usage", (strlen("--usage")+1)) )
				) {
				fprintf(stderr, "%s\n", PIPSLITE_FILTER_USAGE);
				return 0;
			} else if ( (0 == strncmp(argv[i], "-h", (strlen("-h")+1)) )
				|| (0 == strncmp(argv[i], "--help", (strlen("--help")+1)) )
				) {
				fprintf(stderr, "%s\n%s\n", PIPSLITE_FILTER_VERSION, PIPSLITE_FILTER_USAGE);
				return 0;
			}
		}
		fprintf (stderr, "%s\n", PIPSLITE_FILTER_USAGE);
		return 1;
	}

	/* set filter options */
	memset (&fopt, 0, sizeof (filter_option_t));
	memset (&printOpt, 0, sizeof (EPS_OPT));
	memset (&bandBmp, 0, sizeof(EPS_BANDBMP));

	strncpy (fopt.model, argv[1], NAME_MAX);
	for (i = 0; i < NAME_MAX - 1 && fopt.model[i] != '\0' ; i ++)
		fopt.model_low[i] = tolower (fopt.model[i]);
	fopt.model_low[i] = '\0';

	width_pixel = atol (argv[2]);
	height_pixel = atol (argv[3]);
	HWResolution = atol (argv[4]);

	strncpy (fopt.ink, argv[5], NAME_MAX);
	strncpy (fopt.media, argv[6], NAME_MAX);
	strncpy (fopt.quality, argv[7], NAME_MAX);
	strncpy (fopt.duplex, argv[8], NAME_MAX);
	strncpy (fopt.inputslot, argv[9], NAME_MAX);
	strncpy (fopt.brightness, argv[10], NAME_MAX);
	strncpy (fopt.contrast, argv[11], NAME_MAX);
	strncpy (fopt.saturation, argv[12], NAME_MAX);
	strncpy (fopt.quietmode, argv[13], NAME_MAX);
	
	debug_msg("all para\n");
	for(i = 0; i< argc; i++){
		debug_msg("argv[%d] = %s\n", i, argv[i]);
	}

	outfp = stdout;
	printOpt.fpspoolfunc = (EPS_FPSPOOLFUNC)print_spool_fnc;
	
	if (set_pips_parameter (&fopt, &printOpt))
		err_fatal ("Cannot get option of PIPS."); /* exit */
		
	point = fopt.media;
	if(point[0]=='T')
	{
		paper=str_clone(++point,strlen(fopt.media)-1);
		jobAttr.printLayout = EPS_MLID_BORDERLESS;
		debug_msg("borderless\n");
	}
	else
	{
		paper=str_clone(point,strlen(fopt.media));
		jobAttr.printLayout = EPS_MLID_BORDERS;
		debug_msg("border\n");
	}

	eps_toupper(fopt.media);
	jobAttr.mediaSizeIdx = getMediaSizeID(paper);

	int base_dpi = 360;
	switch(HWResolution){
	case 360:
		jobAttr.inputResolution = EPS_IR_360X360;
		base_dpi = 360;
		break;
	case 720:
		jobAttr.inputResolution = EPS_IR_720X720;
		base_dpi = 720;
		break;
	case 300:
		jobAttr.inputResolution = EPS_IR_300X300;
		base_dpi = 300;
		break;
	case 600:
		jobAttr.inputResolution = EPS_IR_600X600;
		base_dpi = 600;
		break;					
	}

	if (jobAttr.mediaSizeIdx == PM_MSID_USER){
		EPS_FLOAT Custom_Width = 0;
		EPS_FLOAT Custom_Height = 0;

		convert_custom_pagesize_to_dot(paper, &Custom_Width, &Custom_Height, base_dpi);

		jobAttr.userDefWidth = Custom_Width;
		jobAttr.userDefHeight = Custom_Height;
		jobAttr.mediaSizeIdx = EPS_MSID_USER;

	}
	
	band_line = 1;

	if (strcmp (fopt.ink, "COLOR") == 0)
		byte_par_pixel = 3;
	else
		byte_par_pixel = 1;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
	epsInitLib();
	epsInitJob();

	debug_msg("mediasizeIDx = %d\n", 	jobAttr.mediaSizeIdx);
	err = SetupJobAttrib(&jobAttr);
	if (err){
		debug_msg("Error occurred in \"SetupJobAttrib\": %d\n", err);
		err_fatal ("Error occurred in \"SetupJobAttrib\".");	/* exit */
	}

	epsInitVariable();
	
//	debug_msg("begin pageAllocBuffer()\n");
//	err = pageAllocBuffer();
//	if(err){
//		debug_msg ("Error occurred in \"pageAllocBuffer\".");	/* exit */
//		err_fatal ("Error occurred in \"pageAllocBuffer\".");	/* exit */
//	}else{
//		debug_msg ("pageAllocBuffer() success\n");
//	}

	EPS_PRINTER_INN curPrinter;
	memset(&curPrinter, 0, sizeof(curPrinter));
	curPrinter.language = EPS_LANG_ESCPR;
	curPrinter.protocol = EPS_PROTOCOL_LPR;

	printJob.printer = &curPrinter;

	prtSetupJobFunctions(printJob.printer, &jobFnc);

	debug_msg("call SendStartJob function\n");

	err = SendStartJob(FALSE);

	if (err){
		debug_msg("Error occurred in \"pageStartJob\": %d\n", err);
		err_fatal ("Error occurred in \"pageStartJob\".");	/* exit */
	}else{
		debug_msg("pageStartJob() success");
	}
	
	printJob.jobStatus = EPS_STATUS_ESTABLISHED;
	int printHeight = 0;
///////////////////////////////////////////////////////////////////////////////////////////////////////////	

	print_area_x = printJob.printableAreaWidth;
	print_area_y = printJob.printableAreaHeight;

	/* setup band struct */
	bandBmp.widthBytes = WIDTH_BYTES (print_area_x * 3 * 8 );
	band = (unsigned char *)mem_new (char, bandBmp.widthBytes * band_line);
	memset (band, 0xff, bandBmp.widthBytes * band_line);
	bandBmp.bits = band;

	/* debug */
	DEBUG_JOB_STRUCT (printJob);

	if (jobAttr.mediaSizeIdx == PM_MSID_USER){
	    print_area_x = width_pixel;
	    print_area_y = height_pixel;
	}

	x_mag = (double)print_area_x / width_pixel;
	y_mag = (double)print_area_y / height_pixel;
	band_line = 1;
 	bytes_per_line = width_pixel * byte_par_pixel;
	debug_msg("bytes_per_line = %d\n", bytes_per_line);
	image_raw = mem_new0 (char, bytes_per_line);

	int page_count = 0;

	char page_num;

	while ((read_page_no = read (STDIN_FILENO, &page_num, 1)) > 0)  // 最初に page番号を読み込み
	{
		long x_count, y_count;
		int band_line_count;
		y_count = 0;
		band_line_count = 0;

		if(page_num == 99){		
			err = epsStartPage(NULL, 2);
		}else{
			err = epsStartPage(NULL, page_num);
			page_num--;
		}		

		page_count++;
		if (err)
			err_fatal ("Error occurred in \"PINIT_FUNC\"."); /* exit */
		debug_msg("ppm image width = %d; height = %d\n", bytes_per_line/3, (int)print_area_y);
		printJob.verticalOffset = 0;
		debug_msg("start page %d ======================\n", page_count);
		if (jobAttr.duplex ==  EPS_DUPLEX_LONG && ((page_count % 2) == 0))
		{
			char* pagebuf = malloc((int)(print_area_y) * (bandBmp.widthBytes));
			char* startpage = pagebuf;
			long int posbuf = 0;

			for (i = 0; i < print_area_y; i ++)
			{
				char *line;
				line = (char *)(band + (bandBmp.widthBytes * band_line_count));
				while ((0 == y_count) || ((i > (y_mag * y_count) - 1) && (y_count < height_pixel))) {
					while ((0 == err) && (read_size < bytes_per_line)) {
						int rsize;
						rsize = read(STDIN_FILENO, image_raw + read_size, bytes_per_line - read_size);
						if (rsize <= 0) {
							if ((errno != EINTR) && (errno != EAGAIN) && (errno != EIO)) {
								err = -1;
								cancel = 1;
								goto quit;
							}
							usleep(50000);
						} else {
							read_size += rsize;
						}
					}
					if( read_size == bytes_per_line ){
						y_count++;
						read_size = 0;
					}
				}
				read_size = 0;
				/* convert input raster data to output byte data. */
				int copybyte = 1;
				if ( 1 == byte_par_pixel )
					copybyte = 3;
				if (x_mag == 1)
				{
					if ( 1 == byte_par_pixel )
					{
						int k;
						for ( j = 0; j < print_area_x; j++ )
						{
							for ( k = 0; k < copybyte ; k++ )
								line[j * 3 + k] = image_raw[j];
						}
					}
					else
						memcpy (line, image_raw, bytes_per_line);
				}
				else
				{
					x_count = 0;
					for (j = 0; j < print_area_x; j ++)
					{
						int k;
						while ( x_count == 0 || j > x_mag * x_count)
							x_count ++;
						for ( k = 0; k < copybyte ; k++ )
							memcpy (line + (j * 3 ) + k,
							image_raw + ((x_count - 1) * byte_par_pixel),
							byte_par_pixel);
					}
				}
				band_line_count ++;
				if (band_line_count >= band_line)
				{
					printHeight = 1;
					memcpy(pagebuf, bandBmp.bits, bandBmp.widthBytes);
					pagebuf+= bandBmp.widthBytes;
					posbuf+=bandBmp.widthBytes;

					band_line_count -= printHeight;
					bandBmp.bits += band_line_count;
					
				}
			}
			if (band_line_count > 0)
			{
				printHeight = 1;
				memcpy(pagebuf, bandBmp.bits, bandBmp.widthBytes);
				pagebuf+= bandBmp.widthBytes;
				posbuf+= bandBmp.widthBytes;
				band_line_count -= printHeight;
				bandBmp.bits += band_line_count;
			}
		
			int revert = 0;
			int pos = posbuf - bandBmp.widthBytes ;
			char *rever_buf = malloc(bandBmp.widthBytes + 1000);
			for (revert = print_area_y; revert > 0; revert--)
			{
				if (3 != byte_par_pixel)
				{
					int k = 0;
					for (k = bandBmp.widthBytes/3; k >= 0; k--)
					{
						memcpy(rever_buf + k*3, startpage + pos + (bandBmp.widthBytes) - k*3, 3);
					}
				}
				else
				{
					int k = 0;
					for (k = bandBmp.widthBytes/3; k >= 0; k--)
					{
						memcpy(rever_buf + k*3, startpage + pos + (bandBmp.widthBytes - 6) - k*3, 3);
					}
				}
				PrintBand (rever_buf, bandBmp.widthBytes, &printHeight);
				pos -= bandBmp.widthBytes;
			}

			debug_msg("free rever\n");
			if (rever_buf != NULL)
			{
				free(rever_buf);
				rever_buf = NULL;
			}
			debug_msg("free page\n");
			if (pagebuf != NULL)
			{
				pagebuf = startpage;
				free (pagebuf);
			}
			debug_msg("free page sucessfull\n");
			err = epsEndPage(FALSE);

			if(err)
				err_fatal ("Error occurred in \"PEND_FUNC\".");	/* exit */
		}
		else //if(duplex)
		{
			for (i = 0; i < print_area_y; i ++)
			{
				char *line;
	
				line = (char *)(band + (bandBmp.widthBytes * band_line_count));
				while ((0 == y_count) || ((i > (y_mag * y_count) - 1) && (y_count < height_pixel))) {
					while ((0 == err) && (read_size < bytes_per_line)) {
						int rsize;
						
						rsize = read(STDIN_FILENO, image_raw + read_size, bytes_per_line - read_size);
						/* 2009.03.17 epson-escpr-1.0.0 */
						/* if user cancels job from CUPS Web Admin, */
						/* epson-escpr-wrapper exit normally, */
						/* and read() return rsize = 0 */
						if (rsize <= 0) {
							if ((errno != EINTR) && (errno != EAGAIN) && (errno != EIO)) {
								err = -1;
								/* 2004.04.15		*/
								/* error then quit	*/ 
								/* don't care err = -1  */
								cancel = 1;
								goto quit;
							}
							usleep(50000);
						} else {
							read_size += rsize;
						}
					}
	
					/* 2004.04.01                                              */
					/* for "skip reading"                                      */
					/* modified y_count count up condition and clear read_size */
					if( read_size == bytes_per_line ){
						y_count++;
						/* need to clear read_size               */
						/* not clear, data still remains on pipe */
						read_size = 0;
					}
				}
				read_size = 0;
				
				/* convert input raster data to output byte data. */
				{
					int copybyte = 1;
	
					if ( 1 == byte_par_pixel )
						copybyte = 3;
	
					if (x_mag == 1)
					{
	
						if ( 1 == byte_par_pixel )
						{
							int k;
	
							for ( j = 0; j < print_area_x; j++ )
							{
								for ( k = 0; k < copybyte ; k++ )
									line[j * 3 + k] = image_raw[j];
							}
						}
						else
							memcpy (line, image_raw, bytes_per_line);
						
					}
					else
					{
						x_count = 0;
						
						for (j = 0; j < print_area_x; j ++)
						{
							int k;
	
							while ( x_count == 0 || j > x_mag * x_count)
								x_count ++;
							
							for ( k = 0; k < copybyte ; k++ )
								memcpy (line + (j * 3 ) + k,
								image_raw + ((x_count - 1) * byte_par_pixel),
								byte_par_pixel);
						}
					}
				}
				
				band_line_count ++;
				
				if (band_line_count >= band_line)
				{
					printHeight = band_line_count;
					PrintBand (bandBmp.bits, bandBmp.widthBytes, &printHeight);
#if (HAVE_PPM)
					fp = fopen(ppmfilename, "a+");
					int i=0;	
					for(i=0; i<bytes_per_line/byte_par_pixel*3; i++){		
						fprintf(fp, "%u ", (unsigned char)bandBmp.Bits[i]);
					}
					fprintf(fp, "\n");
					fclose(fp);
#endif
					debug_msg("printHeight = %d\n", printHeight);
					debug_msg("widthByte = %d\n", bandBmp.widthBytes);
					band_line_count -= printHeight;
					bandBmp.bits += band_line_count;
				}
				
			}
			
			if (band_line_count > 0)
			{
			
				err = PrintBand (bandBmp.bits, bandBmp.widthBytes, &printHeight);
				debug_msg("printHeight = %d\n", printHeight);
				if(err)
					err_fatal ("Error occurred in \"OUT_FUNC\"."); /* exit */
#if (HAVE_PPM)
				fp = fopen(ppmfilename, "a+");
				int i=0;	
				for(i=0; i<bytes_per_line/byte_par_pixel*3; i++){		
					fprintf(fp, "%u ", (unsigned char)bandBmp.Bits[i]);
				}
				fprintf(fp, "\n");
				fclose(fp);
#endif
				band_line_count -= printHeight;
				bandBmp.bits += band_line_count;
			}

//			err = epsEndPage(page_num - 1);
			err = epsEndPage(0);

			if(err)
				err_fatal ("Error occurred in \"PEND_FUNC\".");	/* exit */
		}
	}
	
	DEDBUG_END;

/* 2004.04.15 for 'error' */	
quit:;
	if( cancel ){
		err = epsEndPage(FALSE);
	}
     
	err = epsEndJob();
	if(err)
		err_fatal ("Error occurred in \"END_FUNC\"."); /* exit */

	/* free alloced memory */
	mem_free(image_raw);
	mem_free(band);
	mem_free(paper);
	debug_msg("PRINT SUCCESS\n");

//	XFIFOClose(&context);

	return 0;
}


static int
set_pips_parameter (filter_option_t *filter_opt_p, EPS_OPT *printOpt)
{
	char *mediaType = NULL;
	char *quality = NULL;
	char *ink = NULL;
	char *duplex = NULL;
	char *inputslot = NULL;
	char *brightness = NULL;
	char *contrast = NULL;
	char *saturation = NULL;
	char *quietmode = NULL;

	/* Some model's ppd don't support duplex or inputslot option.*/
	if (strlen (filter_opt_p->media) == 0
	    || strlen (filter_opt_p->quality) == 0
	    || strlen (filter_opt_p->ink) == 0 )
		return 1;

	/* pickup MediaType & Quality from input */
	quality = strrchr(filter_opt_p->quality, '_');

	if(strlen(quality) == 0)
		return 1;

	mediaType = str_clone (filter_opt_p->quality, strlen(filter_opt_p->quality) - strlen(quality));

	/* Media Type ID */
	jobAttr.mediaTypeIdx = getMediaTypeID(mediaType);
	debug_msg(" mediaType = %s\n", mediaType);

	/* Print MediaType */
	jobAttr.printQuality = EPS_MQID_DRAFT;
	if(strcmp(quality, "_DRAFT") == 0 || strcmp(quality, "_SUPERDRAFT") == 0){
		jobAttr.printQuality = EPS_MQID_DRAFT; 
	}else if(strcmp(quality, "_NORMAL") == 0){
		jobAttr.printQuality = EPS_MQID_NORMAL; 
	}else{
		jobAttr.printQuality = EPS_MQID_HIGH;
	}

	/* Ink */
	ink = str_clone (filter_opt_p->ink, strlen (filter_opt_p->ink));
 	if (strcmp (ink, "COLOR") == 0){ 
 		jobAttr.colorMode =  EPS_CM_COLOR;
 	}
 	else{ 
 		jobAttr.colorMode =  EPS_CM_MONOCHROME; 
 	}

	/* Duplex */
	duplex = str_clone (filter_opt_p->duplex, strlen (filter_opt_p->duplex));
 	if (strcmp (duplex, "None") == 0){ 
		debug_msg("DUPLEX NONE\n");
 		jobAttr.duplex =  EPS_DUPLEX_NONE;
 	}
	if (strcmp (duplex, "DuplexNoTumble") == 0){ 
		debug_msg("DUPLEX LONG\n");
 		jobAttr.duplex =  EPS_DUPLEX_LONG;
 	}
 	if (strcmp (duplex, "DuplexTumble") == 0){ 
		debug_msg("DUPLEX SHORT\n");
 		jobAttr.duplex =  EPS_DUPLEX_SHORT;
 	}
 	
	/* InputSlot */
	inputslot = str_clone (filter_opt_p->inputslot, strlen (filter_opt_p->inputslot));
 	if (strcmp (inputslot, "RearPaperFeed") == 0){ 
		debug_msg("Rear Tray\n");
 		jobAttr.paperSource =  EPS_MPID_REAR;
 	}
 	else if(strcmp (inputslot, "Upper") == 0 || strcmp (inputslot, "Cassette1") == 0 || strcmp (inputslot, "Cassette") == 0){ 
		debug_msg("Upper Tray\n");
 		jobAttr.paperSource =  EPS_MPID_FRONT1;
 	}
 	else if(strcmp (inputslot, "Lower") == 0 || strcmp (inputslot, "Cassette2") == 0){ 
		debug_msg("Lower Tray\n");
 		jobAttr.paperSource =  EPS_MPID_FRONT2;
 	}
 	else if(strcmp (inputslot, "Cassette3") == 0){ 
		debug_msg("Lower Tray\n");
 		jobAttr.paperSource =  EPS_MPID_FRONT3;
 	}
 	else if(strcmp (inputslot, "Cassette4") == 0){ 
		debug_msg("Lower Tray\n");
 		jobAttr.paperSource =  EPS_MPID_FRONT4;
 	}
 	else if(strcmp (inputslot, "Rear") == 0 || strcmp (inputslot, "ManualFeed") == 0){ 
		debug_msg("Manual Feed Tray\n");
 		jobAttr.paperSource =  EPS_MPID_REARMANUAL;
 	}
 	else if(strcmp (inputslot, "DiskTray") == 0){ 
		debug_msg("CD/DVD Tray\n");
 		jobAttr.paperSource =  EPS_MPID_CDTRAY;
 	}
 	else{ 
		debug_msg("Auto Selection\n");
 		jobAttr.paperSource =  EPS_MPID_AUTO;
 	}
 
	/* Brightness */
	brightness = str_clone (filter_opt_p->brightness, strlen (filter_opt_p->brightness));
 	jobAttr.brightness =  atoi(brightness);

	/* Contrast */
	contrast = str_clone (filter_opt_p->contrast, strlen (filter_opt_p->contrast));
 	jobAttr.contrast =  atoi(contrast);

	/* Saturation */
	saturation = str_clone (filter_opt_p->saturation, strlen (filter_opt_p->saturation));
 	jobAttr.saturation =  atoi(saturation);

	/* Quiet Mode */
	quietmode = str_clone (filter_opt_p->quietmode, strlen (filter_opt_p->quietmode));
	if(strcasecmp(quietmode, "Off") == 0){ 	
		jobAttr.quietmode =  0;
	}else if(strcasecmp(quietmode, "On") == 0){
		jobAttr.quietmode =  1;
	}else if(strcasecmp(quietmode, "Low") == 0){
		jobAttr.quietmode =  2;
	}else if(strcasecmp(quietmode, "Printer") == 0){
		jobAttr.quietmode =  3;
	}

	/* Get number of pages */
	char page_num;
	read (STDIN_FILENO, &page_num, 1);

	/* Others */
	jobAttr.apfAutoCorrect = EPS_APF_ACT_STANDARD;
	jobAttr.sharpness = 0;
	jobAttr.colorPlane = EPS_CP_FULLCOLOR;
	jobAttr.paletteSize = 0;
	jobAttr.paletteData = NULL;
	jobAttr.copies = 1;
	jobAttr.feedDirection = EPS_FEEDDIR_PORTRAIT;      /* paper feed direction  hardcode */
	jobAttr.printDirection = 0;
	jobAttr.pageNum = page_num;
	jobAttr.version = 4;

	/* free alloced memory */
	mem_free(mediaType);
	mem_free(ink);
	mem_free(duplex);
	mem_free(inputslot);
	mem_free(brightness);
	mem_free(contrast);
	mem_free(saturation);
	mem_free(quietmode);
	
	return 0;
}


static int  getMediaTypeID(char *rsc_name)
{
  int j;
	debug_msg("mediatype name = %s\n", rsc_name);
  for(j = 0; mediaTypeData[j].value != END_ARRAY; j++)
    if(strcmp(mediaTypeData[j].rsc_name,rsc_name) == 0)
      {
		debug_msg("mediatype found, idx = %d\n", mediaTypeData[j].value);
      	return mediaTypeData[j].value;
	  }
  return 0;
}

EPS_INT32 print_spool_fnc(void* hParam, const EPS_UINT8* pBuf, EPS_UINT32 cbBuf) 
{
#if 1
	long int i;
	for (i = 0; i < cbBuf; i++)
		putc(*(pBuf + i), outfp);
#endif

//	fwrite (pBuf, cbBuf, 1, outfp);

//	XFIFOWrite(context, pBuf, cbBuf);

	return 1;
}

