/* ckcker.h -- Symbol and macro definitions for uCKermit */

/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright (c) 2021, 2022, Jeffrey H. Johnson <trnsz@pobox.com>
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

#define NUL       000         /* ASCII NULL */
#define SOH       001         /* ASCII Start of Header */
#define BEL       007         /* ASCII BEL (Beep) */
#define BS        010         /* ASCII Backspace */
#define LF        012         /* ASCII Linefeed */
#define CR        015         /* ASCII Carriage Return */
#define XON       021         /* ASCII XON */
#define SP        040         /* ASCII Space */
#define DEL       0177        /* ASCII Delete (Rubout) */

#ifndef MINBUF
#define MAXSP     1200        /* Send packet buffer size  */
#define MAXRP     1200        /* Receive packet buffer size */
#else /* ifndef MINBUF */
#define MAXSP     90          /* MINBUF packet buffer, send */
#define MAXRP     90          /* MINBUF packet buffer, recv */
#endif /* ifndef MINBUF */
#define MAXWS     1           /* Maximum window size */

#define MAXPACK   94          /* Maximum unextended packet size */
#define CTLQ      '#'         /* Control char prefix I will use */
#define MYEBQ     '&'         /* 8th-Bit prefix char I will use */
#define MYRPTQ    '~'         /* Repeat count prefix I will use */

#define MAXTRY    12          /* Times to retry a packet */
#define MYPADN    0           /* How many padding chars I need */
#define MYPADC    '\0'        /* Which padding character I need */

#define DMYTIM    9           /* Default timeout interval to use. */
#define URTIME    10          /* Timeout interval to be used on me. */
#define DSRVTIM   40          /* Default server command wait timeout. */

#define DEFTRN    0           /* Default line turnaround handshake */
#define DEFPAR    0           /* Default parity */
#define MYEOL     CR          /* End-Of-Line character I need on packets */

#define DRPSIZ    90          /* Default incoming packet size. */
#define DSPSIZ    90          /* Default outbound packet size. */

#ifdef __linux__
#define DDELAY    3           /* Default delay. */
#define DAPEED    38400       /* Default line speed. */
#else /* ifdef __linux__ */
#define DDELAY    5           /* Default delay. */
#define DSPEED    9600        /* Default line speed. */
#endif /* ifdef __linux__ */

#define ZCTERM    0           /* Console terminal */
#define ZSTDIO    1           /* Standard input/output */
#define ZIFILE    2           /* Current input file */
#define ZOFILE    3           /* Current output file */
#define ZDFILE    4           /* Current debugging log file */
#define ZTFILE    5           /* Current transaction log file */
#define ZPFILE    6           /* Current packet log file */
#define ZSFILE    7           /* Current session log file */
#define ZSYSFN    8           /* Input from a system function */
#define ZNFILS    9           /* How many defined file numbers */

#ifdef MINBUF
#define INBUFSIZE 96         /* XXX(jhj): stock 512 */
#else /* ifdef MINBUF */
#define INBUFSIZE 256         /* Size of the input & output buffer */
#endif /* ifdef MINBUF */

#define zminchar() \
  ((( --zincnt ) >= 0 ) ? \
    ((int)( *zinptr++ ) & 0377 ) : \
      zinfill())

#define zmchout(c) \
  (( *zoutptr++ = (CHAR)( c )), \
    (( ++zoutcnt ) >= INBUFSIZE ) ? \
      zoutdump() : 0 )

#define SCR_FN  1             /* filename */
#define SCR_AN  2             /* as-name */
#define SCR_FS  3             /* file-size */
#define SCR_XD  4             /* x-packet data */
#define SCR_ST  5             /* File status: */
#define ST_OK   0             /*  Transferred OK */
#define ST_DISC 1             /*  Discarded */
#define ST_INT  2             /*  Interrupted */
#define ST_SKIP 3             /*  Skipped */
#define ST_ERR  4             /*  Fatal Error */
#define SCR_PN  6             /* packet number */
#define SCR_PT  7             /* packet type or pseudotype */
#define SCR_TC  8             /* transaction complete */
#define SCR_EM  9             /* error message */
#define SCR_WM 10             /* warning message */
#define SCR_TU 11             /* arbitrary undelimited text */
#define SCR_TN 12             /* arbitrary new text delimited at beginning */
#define SCR_TZ 13             /* arbitrary text, delimited at end */
#define SCR_QE 14             /* quantity equals (e.g. "foo: 7") */

#define tochar(ch) \
  (( ch ) + SP )              /* Number to character */
#define xunchar(ch) \
  (( ch ) - SP )              /* Character to number */
#define ctl(ch) \
  (( ch ) ^ 64 )              /* Controllify/Uncontrollify */
#define unpar(ch) \
  (( ch ) & 127 )             /* Clear parity bit */
