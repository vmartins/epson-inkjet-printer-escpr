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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <errno.h>
#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* added 22-04-2004 */
#include <signal.h>

#include "libprtX.h"

#define WIDTH_BYTES(bits) (((bits) + 31) / 32 * 4)

#define PIPSLITE_WRAPPER_VERSION "* epson-escpr-wrapepr is a part of " PACKAGE_STRING

/* Static functions */
static int get_option_for_ppd (const char *, filter_option_t *);
static int get_option_for_arg (const char *, filter_option_t *);
/* Get value for PPD. */
static char * get_default_choice (ppd_file_t *, const char *);

/* added 22-04-2004 */
static void sig_set (void);
static void sigterm_handler (int sig);

int cancel_flg;

#define CUPS_READ_LINE	128
#define DEBUG_PATH "/tmp/eps_wrapper.txt"

#if (HAVE_DEBUG)
static FILE *debug_f = NULL;
#endif

#define HAVE_DEBUG 0
static void debug_msg(const char *fmt, ...){
#if (HAVE_DEBUG)	
	va_list ap;
	
	if(!debug_f){
		debug_f = fopen(DEBUG_PATH, "wb");
		if(debug_f == NULL){
			return;
		}
		fchmod (fileno (debug_f), 0777);
	}
	
	va_start (ap, fmt);
	vfprintf (debug_f, fmt, ap);
	fflush(debug_f);
	va_end (ap);
#endif
	return;
}

/*
 * $$$ CUPS Filter Options $$$
 *
 * printer  - The name of the printer queue
 *            (normally this is the name of the program being run)
 * job      - The numeric job ID for the job being printed
 * user     - The string from the originating-user-name attribute
 * title    - The string from the job-name attribute
 * copies   - The numeric value from the number-copies attribute
 * options  - String representations of the job template attributes, separated
 *            by spaces. Boolean attributes are provided as "name" for true
 *            values and "noname" for false values. All other attributes are
 *            provided as "name=value" for single-valued attributes and
 *            "name=value1,value2,...,valueN" for set attributes
 * filename - The request file
 */
int
main (int argc, char *argv[])
{
	int fd;			/* file descriptor */
	FILE *pfp;
	int i;			/* loop */
	cups_raster_t *ras;	/* raster stream for printing */
	cups_page_header_t header; /* page device dictionary header */
	filter_option_t fopt;

/* attach point */
#ifdef USE_DEBUGGER
	int flag = 1;
	while (flag) sleep (3);
#endif /* USE_DEBUGGER */

	cancel_flg = 0;
	memset (&fopt, 0, sizeof (filter_option_t));
	/* added 22-04-2004 */
	sig_set();

	if (argc < 6 || argc > 7)
	{
		for ( i = 1; i < argc; i++ ) {
			if ( (0 == strncmp(argv[i], "-v", (strlen("-v")+1)) )
				|| (0 == strncmp(argv[i], "--version", (strlen("--version")+1)) )
				) {
				fprintf(stderr, "%s\n", PIPSLITE_WRAPPER_VERSION);
				return 0;
			} else if ( (0 == strncmp(argv[i], "-h", (strlen("-h")+1)) )
				|| (0 == strncmp(argv[i], "--help", (strlen("--help")+1)) )
				) {
				fprintf(stderr, "%s\n", PIPSLITE_WRAPPER_VERSION);
				return 0;
			}
		}
		fprintf (stderr, "Insufficient options.\n");
		return 1;
	}

	if (argc == 7)
	{
		fd = open (argv[6], O_RDONLY);
		if (fd < 0)
		{
			perror ("open");
			return 1;
		}
	}
	else
	{
		fd = 0;
	}

	if (get_option_for_arg (argv[5], &fopt))
	{
		fprintf (stderr, "Cannot read filter option. Cannot get option of PIPS.");
		debug_msg("Cannot read filter option. Cannot get option of PIPS.");
		return 1;
	}
	if (get_option_for_ppd (argv[0], &fopt))
	{
		fprintf (stderr, "PPD file not found, or PPD file is broken. Cannot get option of PIPS.");
		debug_msg("PPD file not found, or PPD file is broken. Cannot get option of PIPS.");
		return 1;
	}

	/* Print start */
	ras = cupsRasterOpen (fd, CUPS_RASTER_READ);
	if (ras == NULL)
	{
		fprintf (stderr, "Can't open CUPS raster file.");
		debug_msg("Can't open CUPS raster file");
		return 1;
	}

	pfp = NULL;

	int total_read = 0;
	int total_read_cache = 0;
	char *page_raw;      //2ページ分のバッファ
	char *page_raw_cache;//2ページ分のキャッシュ

	static BOOL first_fwrite = TRUE;
	static BOOL cache_exist = FALSE;

	int pageNum = 0;
	while (1)
	{
		int ret;
		ret = cupsRasterReadHeader (ras, &header);

		if (ret == 0 || cancel_flg) //データをすべて読み終わった
		{

			if(cache_exist){//cacheがあればcacheを送信
				int pageNum_cache = 2;
				if(first_fwrite){//最初のfwriteだけ、ページ数を送信
					fwrite (&pageNum_cache, 1, 1, pfp);
					first_fwrite = FALSE;
				}	

				switch (pageNum){
				case 1:
					page_raw_cache[0] = 2;
					page_raw_cache[total_read_cache/2 + 1] = 1;
					break;
				case 2:
					page_raw_cache[0] = 99;
					page_raw_cache[total_read_cache/2 + 1] = 99;
					break;
				default:
					break;
				}				

				fwrite (page_raw_cache, total_read_cache + pageNum_cache, 1, pfp);				
				cache_exist = FALSE;
				total_read_cache = 0;
			}
			
			//バッファを送信
			switch (pageNum){
			case 1:
				page_raw[0] = 0;
				break;
			case 2:
				page_raw[0] = 1;
				page_raw[total_read/2 + 1] = 0;
				break;
			default:
				break;
			}

			if(first_fwrite){//最初のfwriteだけ、ページ数を送信
				fwrite (&pageNum, 1, 1, pfp);
				first_fwrite = FALSE;
			}	

			fwrite (page_raw, total_read + pageNum, 1, pfp);


			break;

    	}else{
			if(pageNum == 2){ //全部読み終わってないとき、２ページ分たまったら
				total_read_cache = total_read;

				if(cache_exist){//cacheがあればcacheを送信
					int pageNum_cache = 2;
					if(first_fwrite){//最初のfwriteだけ、ページ数を送信
						fwrite (&pageNum_cache, 1, 1, pfp);
						first_fwrite = FALSE;
					}	

					switch (pageNum){
					case 1:
						page_raw_cache[0] = 99;
						page_raw_cache[total_read_cache/2 + 1] = 99;
						break;
					case 2:
						page_raw_cache[0] = 2;
						page_raw_cache[total_read_cache/2 + 1] = 99;
						break;
					default:
						break;
					}				

					fwrite (page_raw_cache, total_read_cache + pageNum_cache, 1, pfp);				
					cache_exist = FALSE;
				}

				//バッファをキャッシュへコピー
				memcpy(page_raw_cache, page_raw, total_read_cache + pageNum + 1);
				cache_exist = TRUE;
			
				total_read = 0;
				pageNum = 0;
	
			}
		}

		int image_bytes;
		char *image_raw;

		if (pfp == NULL)
		{
			char tmpbuf[256];

			sprintf (tmpbuf, "%s/%s \"%s\" %d %d %d %s %s %s %s %s %s %s %s",
				 CUPS_FILTER_PATH,
				 CUPS_FILTER_NAME,
				 fopt.model,
				 header.cupsWidth,
				 header.cupsHeight,
				 header.HWResolution[0],
				 fopt.ink,
				 fopt.media,
				 fopt.quality,
				 fopt.duplex,
				 fopt.inputslot,
				 fopt.brightness,
				 fopt.contrast,
				 fopt.saturation);
		
			debug_msg("tmpbuf = [%s]\n", tmpbuf);

			pfp = popen (tmpbuf, "w");

			if (pfp == NULL)
			{
				debug_msg("popen error");
				perror ("popen");
				return 1;
			}

			// +1 .. left page num (2 / 1 / 0) , *2 .. 2page分のバッファ
			image_bytes = (WIDTH_BYTES(header.cupsBytesPerLine * 8 ) * header.cupsHeight + 1) * 2; 
			page_raw = (char *)calloc (sizeof (char), image_bytes);
			page_raw_cache = (char *)calloc (sizeof (char), image_bytes);

		}

		image_bytes = WIDTH_BYTES(header.cupsBytesPerLine * 8 ) * CUPS_READ_LINE;
		image_raw = (char *)calloc (sizeof (char), image_bytes);


		int left_lines = header.cupsHeight;	
		while(!cancel_flg)		
		{
			int cups_read_lines;
			if(left_lines >= CUPS_READ_LINE){
				cups_read_lines = CUPS_READ_LINE;
			}else{
				cups_read_lines = left_lines;
			}

			int readpixels;
			readpixels = cupsRasterReadPixels (ras, (unsigned char*)image_raw, header.cupsBytesPerLine * cups_read_lines);

			if (readpixels == 0)
			{	
				// 1ページ分読み込み完了
				break;
			}

			memcpy((page_raw + pageNum +1) + total_read, image_raw, readpixels);
			total_read += readpixels;
	
			left_lines -= CUPS_READ_LINE;

		}//while (pixel data)

			free (image_raw);

			pageNum++;

	}//while (cups header)

	free (page_raw);
	free (page_raw_cache);

	pclose (pfp);
	cupsRasterClose (ras);

	return 0;
}


static int
get_option_for_ppd (const char *printer, filter_option_t *filter_opt_p)
{
	char *ppd_path;		/* Path of PPD */
	ppd_file_t *ppd_p;	/* Struct of PPD */
	char *opt;		/* Temporary buffer (PPD option) */
	int i;			/* loop */

	/* Get option from PPD. */
	ppd_path = (char *) cupsGetPPD (printer);
	ppd_p = ppdOpenFile (ppd_path);
	if(NULL == ppd_p){
		return 1;
	}

	/* Make library file name */
	strcpy (filter_opt_p->model, ppd_p->modelname);
	for (i = 0; i < NAME_MAX && filter_opt_p->model[i] != '\0' ; i ++)
		filter_opt_p->model_low[i] = tolower (filter_opt_p->model[i]);
	filter_opt_p->model_low[i] = '\0';

	/* media */
	if (filter_opt_p->media[0] == '\0')
	{
		opt = get_default_choice (ppd_p, "PageSize");
		if (!opt)
			return 1;

		strcpy (filter_opt_p->media, opt);
	}

	/* ink */
	if (filter_opt_p->ink[0] == '\0')
	{
		opt = get_default_choice (ppd_p, "Ink");
		if (!opt)
			return 1;

		strcpy (filter_opt_p->ink, opt);
	}

	/* MediaType */
	if (filter_opt_p->quality[0] == '\0')
	{
		opt = get_default_choice (ppd_p, "MediaType");

		if (!opt){
			opt = get_default_choice (ppd_p, "Quality");	/* for old version */

			if (!opt)return 1;	
		}
		
		strcpy (filter_opt_p->quality, opt);
	}

	/* duplex */
	if (filter_opt_p->duplex[0] == '\0')
	{
		opt = get_default_choice (ppd_p, "Duplex");
		if (!opt)
		{
			debug_msg("can not get duplex\n");
			strcpy (filter_opt_p->duplex, "None");
			
		}
		else
		strcpy (filter_opt_p->duplex, opt);
	}
	
	/* inputslot */
	if (filter_opt_p->inputslot[0] == '\0')
	{
		opt = get_default_choice (ppd_p, "InputSlot");
		if (!opt)
		{
			debug_msg("can not get inputslot\n");
			strcpy (filter_opt_p->inputslot, "Unknown"); //AutoSelection
			
		}
		else
		strcpy (filter_opt_p->inputslot, opt);
	}

	/* brightness */
	if (filter_opt_p->brightness[0] == '\0')
	{
		opt = get_default_choice (ppd_p, "Brightness");
		if (!opt)
		{
			debug_msg("can not get brightness\n");
			strcpy (filter_opt_p->brightness, "0");
			
		}
		else
		strcpy (filter_opt_p->brightness, opt);
	}

	/* contrast */
	if (filter_opt_p->contrast[0] == '\0')
	{
		opt = get_default_choice (ppd_p, "Contrast");
		if (!opt)
		{
			debug_msg("can not get contrast\n");
			strcpy (filter_opt_p->contrast, "0");
			
		}
		else
		strcpy (filter_opt_p->contrast, opt);
	}

	/* saturation */
	if (filter_opt_p->saturation[0] == '\0')
	{
		opt = get_default_choice (ppd_p, "Saturation");
		if (!opt)
		{
			debug_msg("can not get saturation\n");
			strcpy (filter_opt_p->saturation, "0");
			
		}
		else
		strcpy (filter_opt_p->saturation, opt);
	}


#ifdef INK_CHANGE_SYSTEM
	/* inkset */
	if (filter_opt_p->inkset[0] == '\0')
	{
		opt = get_default_choice (ppd_p, "InkSet");
		if (!opt)
			return 1;

		strcpy (filter_opt_p->inkset, opt);
	}
#endif

	ppdClose (ppd_p);
	return 0;
}


static int
get_option_for_arg (const char *opt_str, filter_option_t *filter_opt_p)
{
	int opt_num;
	cups_option_t *option_p;
	const char *opt;
	
	const char *media_names[] = { "PageSize", "PageRegion",  "media", "" };
	int i;
	
	if (strlen (opt_str) == 0)
		return 0;

	opt_num = 0;
	option_p = NULL;
	opt_num = cupsParseOptions (opt_str, opt_num, &option_p);

	for (i = 0; *media_names[i] != '\0'; i ++)  /* chenged Wed Jan 28 2009 */
	{
		opt = cupsGetOption (media_names[i], opt_num, option_p);
		if (opt)
		{
			int num;

			num = strcspn (opt, ",");
			if (num >= PPD_MAX_NAME)
			{
				return 1;
			}

			strncpy (filter_opt_p->media, opt, num);
			filter_opt_p->media[num] = '\0';

			break;
		}
	}

	opt = cupsGetOption ("Ink", opt_num, option_p);
	if (opt)
		strcpy (filter_opt_p->ink, opt);

	opt = cupsGetOption ("MediaType", opt_num, option_p);
	if(!opt){
		opt = cupsGetOption ("Quality", opt_num, option_p); /* for old version */
	}

	if (opt)
	{
		strcpy (filter_opt_p->quality, opt);
	}

	opt = cupsGetOption ("Duplex", opt_num, option_p);
	if (opt)
		strcpy (filter_opt_p->duplex, opt);

	opt = cupsGetOption ("InputSlot", opt_num, option_p);
	if (opt)
		strcpy (filter_opt_p->inputslot, opt);

	opt = cupsGetOption ("Brightness", opt_num, option_p);
	if (opt)
		strcpy (filter_opt_p->brightness, opt);

	opt = cupsGetOption ("Contrast", opt_num, option_p);
	if (opt)
		strcpy (filter_opt_p->contrast, opt);

	opt = cupsGetOption ("Saturation", opt_num, option_p);
	if (opt)
		strcpy (filter_opt_p->saturation, opt);

	return 0;
}


/* Get value for PPD */
static char *
get_default_choice (ppd_file_t *ppd_p, const char *key)
{
	ppd_option_t *option;
	ppd_choice_t *choice;

	option = ppdFindOption (ppd_p, key);
	if (!option)
		return NULL;
	
	choice = ppdFindChoice (option, option->defchoice);
	if (!choice)
		return NULL;

	return choice->choice;
}

static void
sig_set (void)
{

#ifdef HAVE_SIGSET /* Use System V signals over POSIX to avoid bugs */
	sigset (SIGTERM, sigterm_handler);
#elif defined(HAVE_SIGACTION)
        {
          struct sigaction action;

          memset (&action, 0, sizeof(action));

          sigemptyset (&action.sa_mask);
          action.sa_handler = sigterm_handler;
          sigaction (SIGTERM, &action, NULL);
        }
#else
	signal (SIGTERM, sigterm_handler);
#endif /* HAVE_SIGSET */

  return;
}


static void
sigterm_handler (int sig)
{
  cancel_flg = 1;

  return;
}
