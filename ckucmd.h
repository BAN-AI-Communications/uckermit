/* C K U C M D . H -- Header file for UNIX cmd package */

/*
 * Copyright (C) 2021, Jeffrey H. Johnson <trnsz@pobox.com>
 *
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

#define HLPLW  78                    /* Width of ?-help line */
#define HLPCW  19                    /* Width of ?-help column */
#define CMDBL 255                    /* Command buffer length */
#define HLPBL 100                    /* Help string buffer length */
#define ATMBL 256                    /* Command atom buffer length*/

#ifndef   NUL
#define   NUL '\0'                   /* NULL */
#endif /* ifndef NUL */

#define   HT '\t'                    /* Horizontal Tab */
#define   NL '\n'                    /* Newline */

#ifndef   CR
#define   CR '\r'
#endif /* ifndef CR */

#define   FF 0014                    /* Formfeed     (^L) */
#define RDIS 0022                    /* Redisplay    (^R) */
#define LDEL 0025                    /* Delete line  (^U) */
#define WDEL 0027                    /* Delete word  (^W) */
#define  ESC 0033                    /* Escape */
#define  RUB 0177                    /* Rubout */

#ifndef  BEL
#define  BEL 0007                    /* Bell */
#endif /* ifndef BEL */

#ifndef   BS
#define   BS 0010                    /* Backspace */
#endif // ifndef   BS

#ifndef   SP
#define   SP 0040                    /* Space */
#endif // ifndef   SP

/*
 * Keyword table
 * flags
 */

#define CM_INV 1                     /* Invisible keyword */

/*
 * Keyword Table
 * Template
 */

struct keytab                        /* Keyword table */
{
  char *kwd;                         /* Pointer to keyword string */
  int val;                           /* Associated value */
  int flgs;                          /* Flags (as defined above) */
};
