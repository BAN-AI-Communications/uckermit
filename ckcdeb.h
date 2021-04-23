/* C K C D E B . H */

/*
 * This file is included by all uCKermit modules, including the modules
 * that aren't specific to Kermit. It specifies format codes for
 * debug(), tlog(), and similar functions, and includes any necessary
 * typedefs to be used by all uCKermit modules, and also includes some
 * feature selection compile-time switches.
 */

/*
 * Copyright (C) 1981-2011,
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

/*
 * Copyright (C) 1987, 1989, 
 *   Trustees of Columbia University in the City of New York.
 *
 * Permission is granted to any individual or institution to use, copy,
 *   or redistribute this software so long as it is not sold for profit,
 *   provided this copyright notice is retained.
 */

/*
 * DEBUG and TLOG should be defined in the Makefile if you want debugging
 * and transaction logs. Don't define them if you want to save the space
 * and overhead. (Note, in version 4F these definitions changed from "{}"
 * to the null string to avoid problems with semicolons after braces,
 * as in: "if (x) tlog(this); else tlog(that);"
 */

#ifndef DEBUG
#define debug(a, b, c, d)
#endif

#ifndef TLOG
#define tlog(a, b, c, d)
#endif

/*
 * Formats for
 * debug(), tlog(), etc
 */

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
 * Empty string (or for numeric variables, -1) = unused attribute.
 */

struct zstr {           /* String format */
  int  len;             /* Length */
  char *val;            /* Value */
};
struct zattr {          /* Kermit File Attribute structure */
  long lengthk;         /* (!) File length in K */
  struct zstr type;     /* (") File type (text or binary) */
  struct zstr date;     /* (#) File creation date [yy]yymmdd[ hh:mm[:ss]] */
  struct zstr creator;  /* ($) File creator id */
  struct zstr account;  /* (%) File account */
  struct zstr area;     /* (&) Area (e.g. directory) for file */
  struct zstr passwd;   /* (') Password for area */
  long   blksize;       /* (() File blocksize */
  struct zstr access;   /* ()) File access: new, supersede, append, warn */
  struct zstr encoding; /* (*) Encoding (transfer syntax) */
  struct zstr disp;     /* (+) Disposition (mail, message, print, etc) */
  struct zstr lprotect; /* (,) Protection (local syntax) */
  struct zstr gprotect; /* (-) Protection (generic syntax) */
  struct zstr systemid; /* (.) ID for system of origin */
  struct zstr recfm;    /* (/) Record format */
  struct zstr sysparam; /* (0) System-dependent parameter string */
  long   length;        /* (1) Exact length (on system of origin) */
};

/*
 * signal() type
 * void or int?
 */

#ifdef SVR3
typedef void SIGTYP;    /* System V R3 and later */
#else
#ifdef SUNOS4
typedef void SIGTYP;    /* SUNOS V 4.0 and later */
#else
#ifdef __linux__
typedef void SIGTYP;
#else
typedef int SIGTYP;
#endif
#endif
#endif

#ifndef FTYPE
#define FTYPE int
#endif

/* 
 * Systems that expand tilde at the
 * beginning of file or directory names
 */

#ifdef BSD4
#define DTILDE
#endif
#ifdef UXIII
#define DTILDE
#endif

/*
 * C Compiler
 * Dependencies
 */

#ifdef V7
typedef char CHAR;
typedef long LONG;
#else
#ifdef BSD29
typedef char CHAR;
typedef long LONG;
#else
#ifdef __linux__
typedef char CHAR;
typedef long LONG;
#else
typedef unsigned char CHAR;
typedef long LONG;
#endif
#endif
#endif

/*
 * Line delimiter
 * for text files
 */

/*
 * If the system uses a single character for text file line delimitation,
 * define NLCHAR to the value of that character. For text files, that
 * character will be converted to CRLF upon output, and CRLF will be
 * converted to that character on input during text-mode (default)
 * packet operations.
 */

#define NLCHAR 012

/*
 * At this point, if there's a system that uses ordinary CRLF line
 * delimitation AND the C compiler actually returns both the CR and
 * the LF when doing input from a file, then #undef NLCHAR.
 */

/*
 * The device name of a job's
 * controlling terminal.
 */

#define CTTNAM "/dev/tty"

/*
 * Program return codes
 */

#define GOOD_EXIT 0
#define BAD_EXIT  1
