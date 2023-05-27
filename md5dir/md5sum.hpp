#pragma once


/* md5sum.c - Compute MD5 checksum of files or strings according to the
 *            definition of MD5 in RFC 1321 from April 1992.
 * Copyright (C) 1995-1999 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Written by Ulrich Drepper <drepper@gnu.ai.mit.edu> */
/* Hacked to work with BusyBox by Alfred M. Szmidt <ams@trillian.itslinux.org> */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <getopt.h>

// #include "busybox.h"

/* For some silly reason, this file uses backwards TRUE and FALSE conventions */
#undef TRUE
#undef FALSE
#define FALSE   ((int) 1)
#define TRUE    ((int) 0)

//----------------------------------------------------------------------------
//--------md5.c
//----------------------------------------------------------------------------

/* md5.c - Functions to compute MD5 message digest of files or memory blocks
 *         according to the definition of MD5 in RFC 1321 from April 1992.
 * Copyright (C) 1995, 1996 Free Software Foundation, Inc.
 *
 * NOTE: The canonical source of this file is maintained with the GNU C
 * Library.  Bugs can be reported to bug-glibc@prep.ai.mit.edu.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.  */

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
// #include <endian.h>

// #include "busybox.h"
//----------------------------------------------------------------------------
//--------md5.h
//----------------------------------------------------------------------------

/* md5.h - Declaration of functions and data types used for MD5 sum
   computing library functions.
   Copyright (C) 1995, 1996 Free Software Foundation, Inc.
   NOTE: The canonical source of this file is maintained with the GNU C
   Library.  Bugs can be reported to bug-glibc@prep.ai.mit.edu.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _MD5_H
static const int _MD5_H = 1;

#include <stdio.h>

#if defined HAVE_LIMITS_H || defined _LIBC
# include <limits.h>
#endif

/* The following contortions are an attempt to use the C preprocessor
   to determine an unsigned integral type that is 32 bits wide.  An
   alternative approach is to use autoconf's AC_CHECK_SIZEOF macro, but
   doing that would require that the configure script compile and *run*
   the resulting executable.  Locally running cross-compiled executables
   is usually not possible.  */

# include <sys/types.h>
typedef u_int32_t md5_uint32;

/* Structure to save state of computation between the single steps.  */
struct md5_ctx
{
  md5_uint32 A;
  md5_uint32 B;
  md5_uint32 C;
  md5_uint32 D;

  md5_uint32 total[2];
  md5_uint32 buflen;
  char buffer[128];
};

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The following three functions are build up the low level used in
 * the functions `md5_stream' and `md5_buffer'.
 */

/* Initialize structure containing state of computation.
   (RFC 1321, 3.3: Step 3)  */
extern void md5_init_ctx __P ((struct md5_ctx *ctx));

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is necessary that LEN is a multiple of 64!!! */
extern void md5_process_block __P ((const void *buffer, size_t len,
				    struct md5_ctx *ctx));

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is NOT required that LEN is a multiple of 64.  */
extern void md5_process_bytes __P ((const void *buffer, size_t len,
				    struct md5_ctx *ctx));

/* Process the remaining bytes in the buffer and put result from CTX
   in first 16 bytes following RESBUF.  The result is always in little
   endian byte order, so that a byte-wise output yields to the wanted
   ASCII representation of the message digest.

   IMPORTANT: On some systems it is required that RESBUF is correctly
   aligned for a 32 bits value.  */
extern void *md5_finish_ctx __P ((struct md5_ctx *ctx, void *resbuf));


/* Put result from CTX in first 16 bytes following RESBUF.  The result is
   always in little endian byte order, so that a byte-wise output yields
   to the wanted ASCII representation of the message digest.

   IMPORTANT: On some systems it is required that RESBUF is correctly
   aligned for a 32 bits value.  */
extern void *md5_read_ctx __P ((const struct md5_ctx *ctx, void *resbuf));


/* Compute MD5 message digest for bytes read from STREAM.  The
   resulting message digest number will be written into the 16 bytes
   beginning at RESBLOCK.  */
extern int md5_stream __P ((FILE *stream, void *resblock));

/* Compute MD5 message digest for LEN bytes beginning at BUFFER.  The
   result is always in little endian byte order, so that a byte-wise
   output yields to the wanted ASCII representation of the message
   digest.  */
extern void *md5_buffer __P ((const char *buffer, size_t len, void *resblock));

#endif

//----------------------------------------------------------------------------
//--------end of md5.h
//----------------------------------------------------------------------------

extern int md5_stream __P ((FILE *stream, void *resblock));

#ifdef __cplusplus
}
#endif