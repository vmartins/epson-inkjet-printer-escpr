/*
 * Epson Inkjet Printer Driver (ESC/P-R) for Linux
 * Copyright (C) Seiko Epson Corporation 2002-2013.
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

#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "mem.h"
#include "str.h"

char *
str_clone (char *str, size_t size)
{
	char *new;

	if (!str || !size)
		return NULL;	/* error */
	
	new = mem_new (char, size + 1);
	
	strncpy (new, str, size);
	new[size] = '\0';

	return new;
}

char *
str_delete (char *str)
{
	if (str)
		mem_free (str);

	str = 0;

	return 0;
}
