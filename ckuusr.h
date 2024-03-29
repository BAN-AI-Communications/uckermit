/* C K U U S R . H -- Symbol definitions for modules */

/* SPDX-License-Identifier: BSD-3-Clause */

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

#define KERMRC ".uckermrc"

#include <pwd.h>

/*
 * Values associated with top-level
 * commands, must be 0 or greater.
 */

#define XXBYE   0   /* BYE */
#define XXCLE   1   /* CLEAR */
#define XXCLO   2   /* CLOSE */
#define XXCON   3   /* CONNECT */
#define XXCPY   4   /* COPY */
#define XXCWD   5   /* CWD (Change Working Directory) */
#define XXDEF   6   /* DEFINE (a command macro) */
#define XXDEL   7   /* (Local) DELETE */
#define XXDIR   8   /* (Local) DIRECTORY */
#define XXDIS   9   /* DISCONNECT */
#define XXECH  10   /* ECHO */
#define XXEXI  11   /* EXIT */
#define XXFIN  12   /* FINISH */
#define XXGET  13   /* GET */
#define XXHLP  14   /* HELP */
#define XXINP  15   /* INPUT */
#define XXLOC  16   /* LOCAL */
#define XXLOG  17   /* LOG */
#define XXMAI  18   /* MAIL */
#define XXMOU  19   /* (Local) MOUNT */
#define XXMSG  20   /* (Local) MESSAGE */
#define XXOUT  21   /* OUTPUT */
#define XXPAU  22   /* PAUSE */
#define XXPRI  23   /* (Local) PRINT */
#define XXQUI  24   /* QUIT */
#define XXREC  25   /* RECEIVE */
#define XXREM  26   /* REMOTE */
#define XXREN  27   /* (Local) RENAME */
#define XXSEN  28   /* SEND */
#define XXSER  29   /* SERVER */
#define XXSET  30   /* SET */
#define XXSHE  31   /* Command for SHELL */
#define XXSHO  32   /* SHOW */
#define XXSPA  33   /* (Local) SPACE */
#define XXSTA  34   /* STATISTICS */
#define XXSUB  35   /* (Local) SUBMIT */
#define XXTAK  36   /* TAKE */
#define XXTRA  37   /* TRANSMIT */
#define XXTYP  38   /* (Local) TYPE */
#define XXWHO  39   /* (Local) WHO */
#define XXDIAL 40   /* (Local) DIAL */
#define XXLOGI 41   /* (Local) SCRIPT */
#define XXCOM  42   /* Comment */
#define XXHAN  43   /* HANGUP */

/*
 * SET
 * parameters
 */

#define XYBREA  0 /* BREAK simulation */
#define XYCHKT  1 /* Block check type */
#define XYDEBU  2 /* Debugging */
#define XYDELA  3 /* Delay */
#define XYDUPL  4 /* Duplex */
#define XYEOL   5 /* End-Of-Line (packet terminator) */
#define XYESC   6 /* Escape character */
#define XYFILE  7 /* File Parameters */
#define XYFILN  0 /*  Naming  */
#define XYFILT  1 /*  Type    */
#define XYFILW  2 /*  Warning */
#define XYFILD  3 /*  ...     */
#define XYFLOW  9 /* Flow Control */
#define XYHAND 10 /* Handshake */
#define XYIFD  11 /* Incomplete File Disposition */
#define XYIMAG 12 /* "Image Mode" */
#define XYINPU 13 /* INPUT command parameters */
#define XYLEN  14 /* Maximum packet length to send */
#define XYLINE 15 /* Communication line to use */
#define XYLOG  16 /* Log file */
#define XYMARK 17 /* Start of Packet mark */
#define XYNPAD 18 /* Amount of padding */
#define XYPADC 19 /* Pad character */
#define XYPARI 20 /* Parity */
#define XYPAUS 21 /* Interpacket pause */
#define XYPROM 22 /* Program prompt string */
#define XYQBIN 23 /* 8th-bit prefix */
#define XYQCTL 24 /* Control character prefix */
#define XYREPT 25 /* Repeat count prefix */
#define XYRETR 26 /* Retry limit */
#define XYSPEE 27 /* Line speed (baud rate) */
#define XYTACH 28 /* Character to be doubled */
#define XYTIMO 29 /* Timeout interval */
#define XYMODM 30 /* Modem type */
#define XYSEND 31 /* SEND parameters, used with some of the above */
#define XYRECV 32 /* RECEIVE parameters, ditto */
#define XYTERM 33 /* Terminal parameters */
#define XYATTR 34 /* Attribute packets */
#define XYSERV 35 /* Server parameters */
#define XYSERT  0 /*  Server timeout   */
#define XYWIND 36 /* Window size */

/*
 * REMOTE command
 * symbols
 */

#define XZCPY 0  /* Copy */
#define XZCWD 1  /* Change Working Directory */
#define XZDEL 2  /* Delete */
#define XZDIR 3  /* Directory */
#define XZHLP 4  /* Help */
#define XZHOS 5  /* Host */
#define XZKER 6  /* Kermit */
#define XZLGI 7  /* Login */
#define XZLGO 8  /* Logout */
#define XZMAI 9  /* Mail */
#define XZMOU 10 /* Mount */
#define XZMSG 11 /* Message */
#define XZPRI 12 /* Print */
#define XZREN 13 /* Rename */
#define XZSET 14 /* Set */
#define XZSPA 15 /* Space */
#define XZSUB 16 /* Submit */
#define XZTYP 17 /* Type */
#define XZWHO 18 /* Who */

/*
 * Symbols for
 * logs
 */

#define LOGD 0 /* Debugging */
#define LOGP 1 /* Packets */
#define LOGS 2 /* Session */
#define LOGT 3 /* Transaction */
