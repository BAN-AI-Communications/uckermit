/* C K C D E B -- Logging, debugging, and shared definitions. */

/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * This file is included by all modules, including the modules
 * that aren't specific to uCKermit. It specifies format codes
 * for debug, tlog, & similar functions. It includes necessary
 * typedefs to be used by all uCKermit modules.
 */

/*
 * Copyright (c) 2021, 2022, 2023 Jeffrey H. Johnson <trnsz@pobox.com>
 *
 * Copyright (c) 1981-2011,
 *   Trustees of Columbia University in the City of New York.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *   - Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *   - Neither the name of Columbia University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DEBUG
#define debug(a, b, c, d)
#endif /* ifndef DEBUG */

#ifndef TLOG
#define tlog(a, b, c, d)
#endif /* ifndef TLOG */

#define F000 0
#define F001 1
#define F010 2
#define F011 3
#define F100 4
#define F101 5
#define F110 6
#define F111 7

/*
 * Structure definitions for Kermit file attributes.
 * All strings come as pointer and length combinations.
 * Empty string (or, for numeric variables, -1) = unused.
 */

struct zstr             /* String format */
{
  int  len;             /* Length */
  char *val;            /* Value  */
};

struct zattr            /* Kermit File Attribute structure */
{
  long        lengthk;  /* (!) File length in K                           */
  struct zstr type;     /* (") File type (text or binary)                 */
  struct zstr date;     /* (#) File creation date [yy]yymmdd[ hh:mm[:ss]] */
  struct zstr creator;  /* ($) File creator id                            */
  struct zstr account;  /* (%) File account                               */
  struct zstr area;     /* (&) Area (e.g. directory) for file             */
  struct zstr passwd;   /* (') Password for area                          */
  long        blksize;  /* (() File blocksize                             */
  struct zstr access;   /* ()) File access: new, supersede, append, warn  */
  struct zstr encoding; /* (*) Encoding (transfer syntax)                 */
#ifndef NODISP
  struct zstr disp;     /* (+) Disposition (mail, message, print, etc)    */
#endif /* ifndef NODISP */
  struct zstr lprotect; /* (,) Protection (local syntax)                  */
  struct zstr gprotect; /* (-) Protection (generic syntax)                */
  struct zstr systemid; /* (.) ID for system of origin                    */
  struct zstr recfm;    /* (/) Record format                              */
  struct zstr sysparam; /* (0) System-dependent parameter string          */
  long        length;   /* (1) Exact length (on system of origin)         */
};

#ifdef SVR3
typedef void SIGTYP;
#else  /* ifdef SVR3 */
#ifdef SUNOS4
typedef void SIGTYP;
#else  /* ifdef SUNOS4 */
#ifdef __linux__
typedef void SIGTYP;
#else  /* ifdef __linux__ */
typedef int SIGTYP;
#endif /* ifdef __linux__ */
#endif /* ifdef SUNOS4 */
#endif /* ifdef SVR3 */

#ifndef FTYPE
#define FTYPE int
#endif /* ifndef FTYPE */

#ifdef BSD4
#ifndef MINBUF
#define DTILDE
#endif /* ifndef MINBUF */
#endif /* ifdef BSD4 */
#ifdef UXIII
#ifndef MINBUF
#ifndef NOTILDE
#define DTILDE
#endif /* ifndef NOTILDE */
#endif /* ifndef MINBUF */
#endif /* ifdef UXIII */

#ifdef V7
typedef char CHAR;
typedef long LONG;
#else  /* ifdef V7 */
#ifdef BSD29
typedef char CHAR;
typedef long LONG;
#else  /* ifdef BSD29 */
#ifdef __linux__
typedef char CHAR;
typedef long LONG;
#else  /* ifdef __linux__ */
typedef unsigned char CHAR;
typedef long LONG;
#endif /* ifdef __linux__ */
#endif /* ifdef BSD29 */
#endif /* ifdef V7 */

#define NLCHAR 012
#define CTTNAM "/dev/tty"

#define GOOD_EXIT 0
#define BAD_EXIT  1

#include "ckchdr.h"
