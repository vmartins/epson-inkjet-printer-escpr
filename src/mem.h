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

#ifndef MEM_H
#define MEM_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/* defines */
#ifdef __cplusplus
#define BEGIN_C extern "C" {
#define END_C }
#else
#define BEGIN_C
#define END_C
#endif /* __cplusplus */

typedef int bool_t;

/* If it is a success, an address is returned, and if it is failure, a message is outputted and it ends. */
#define mem_new(type, num) \
((type *) mem_malloc (sizeof (type) * (num), 1))
#define mem_new0(type, num) \
((type *) mem_calloc ((num), sizeof (type), 1))
#define mem_renew(type, pointer, num) \
((type *) mem_realloc (pointer, sizeof (type) * (num), 1))

/*
* bool value true Critical
* false Normal operation
*/
BEGIN_C

void * mem_malloc (unsigned int, bool_t);
void * mem_calloc (unsigned int, unsigned int, bool_t);
void * mem_realloc (void *, unsigned int, bool_t);
void mem_free (void *);

END_C

#endif /* MEM_H */
