char *versio = "uCKermit, 4G(172), 2022-FEB-26";

/* C K C M A I -- uCKermit Main program */

/*
 * Copyright (C) 2021-2022, Jeffrey H. Johnson <trnsz@pobox.com>
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

#include "ckcdeb.h"                 /* Debug & other symbols */
#include "ckcker.h"                 /* Kermit symbols */

#ifdef __linux__
#include <stdlib.h>
#include <string.h>                 /* String manipulation */
#include <stdio.h>
#endif /* ifdef __linux__ */

/*
 * Text message definitions, each
 * must be 256 chars long or less.
 */

#ifdef NODOHLP
char *hlptxt =
"HELP unavailable.\n\
\n\0";
#else  /* ifdef NODOHLP */
char *hlptxt =
"uCKermit Server REMOTE Commands:\n\
\n\
GET files  REMOTE CWD [dir]    REMOTE DIRECTORY [files]\n\
SEND files REMOTE SPACE [dir]  REMOTE HOST command\n\
MAIL files REMOTE DELETE files REMOTE WHO [user]\n\
BYE        REMOTE PRINT files  REMOTE TYPE files\n\
FINISH     REMOTE HELP\n\
\n\0";
#endif /* ifdef NODOHLP */

#ifndef NOSERVER
#ifdef NODOHLP
char *srvtxt = "\r\n\
Server starting.\n\r\n\0";
#else  /* ifdef NODOHLP */
char *srvtxt =
"\r\n\
uCKermit Server starting.  Return to your local machine by typing\r\n\
its escape sequence for closing the connection, and issue further\r\n\
commands from there.  To shut down the uCKermit server, issue the\r\n\
FINISH or BYE command and then reconnect.\n\
\r\n\0";
#endif /* ifdef NODOHLP */
#endif /* ifndef NOSERVER */

/*
 * Declarations for
 * Send-Init Parameters
 */

int         spsiz  = DSPSIZ,        /* curent packet size to send */
            spmax  = DSPSIZ,        /* Biggest packet size we can send */
            spsizf = 0,             /* Flag to override what you ask for */
            rpsiz  = DRPSIZ,        /* Biggest we want to receive */
            urpsiz = DRPSIZ,        /* User-requested rpsiz */
            maxrps = MAXRP;         /* Maximum incoming long packet size */
#ifndef NOICP
int         maxsps = MAXSP;         /* Maximum outbound l.p. size */
#endif /* ifndef NOICP */
int         maxtry = MAXTRY,        /* Maximum retries per packet */
            wslots = 1,             /* Window size */
            timint = DMYTIM,        /* Timeout interval I use */
            srvtim = DSRVTIM,       /* Server command wait timeout */
            rtimo  = URTIME,        /* Timeout I want you to use */
            timef  = 0,             /* Flag to override what you ask */
            npad   = MYPADN,        /* How much padding to send */
            mypadn = MYPADN,        /* How much padding to ask for */
            bctr   = 1,             /* Block check type requested */
            bctu   = 1,             /* Block check type used */
            ebq    = MYEBQ,         /* 8th bit prefix */
            ebqflg = 0,             /* 8th-bit quoting flag */
            rqf    = -1,            /* Flag used in 8bq negotiation */
            rq     = 0,             /* Received 8bq bid */
            sq     = 'Y',           /* Sent 8bq bid */
            rpt    = 0,             /* Repeat count */
            rptq   = MYRPTQ,        /* Repeat prefix */
            rptflg = 0;             /* Repeat processing flag */
int         capas  = 10,            /* Position of Capabilities */
#ifndef NOATTR
            atcapb = 8,             /* Attribute capability */
            atcapr = 1,             /*  requested */
            atcapu = 0,             /*  used */
#endif /* ifndef NOATTR */
            swcapb = 4,             /* Sliding Window capability */
            swcapr = 0,             /*  requested */
            swcapu = 0,             /*  used */
            lpcapb = 2,             /* Long Packet capability */
            lpcapr = 1,             /*  requested */
            lpcapu = 0;             /*  used */
CHAR        padch  = MYPADC,        /* Padding character to send */
            mypadc = MYPADC,        /* Padding character to ask for */
            seol   = MYEOL,         /* End-Of-Line character to send */
            eol    = MYEOL,         /* End-Of-Line character to look for */
            ctlq   = CTLQ,          /* Control prefix in incoming data */
            myctlq = CTLQ;          /* Outbound control character prefix */

struct      zattr iattr;            /* Incoming file attributes */

/*
 * Packet-related
 * variables
 */

int         pktnum = 0,             /* Current packet number */
            prvpkt = -1,            /* Previous packet number */
            sndtyp,                 /* Type of packet just sent */
            rsn,                    /* Received packet sequence number */
            rln,                    /* Received packet length */
            size,                   /* Current size of output pkt data */
            maxsize,                /* Max size for building data field */
            spktl  = 0;             /* Length packet being sent */
CHAR        sndpkt[MAXSP + 100],    /* Entire packet being sent */
            recpkt[MAXRP + 200],    /* Packet most recently received */
            *rdatap,                /* Pointer to received packet data */
            data[MAXSP   + 4],      /* Packet data buffer */
            srvcmd[MAXRP + 4],      /* Where to decode server command */
            *srvptr,                /* Pointer to above */
            mystch = SOH,           /* Outbound packet-start character */
            stchr  = SOH;           /* Incoming packet-start character */

/*
 * File-related
 * variables.
 */

CHAR        filnam[50];             /* Name of current file. */
int         nfils;                  /* Number of files in file group */
long        fsize;                  /* Size of current file */

/*
 * Communication
 * line variables
 */

CHAR        ttname[50];             /* Name of communication line. */
int         parity,                 /* Parity specified, 0, 'e', 'o', etc */
            flow,                   /* Flow control, 1 = xon/xoff */
            speed  = -1,            /* Line speed */
            turn   = 0,             /* Line turnaround handshake flag */
            turnch = XON,           /* Line turnaround character */
            duplex = 0,             /* Duplex, full by default */
            escape = 034,           /* Escape character for connect */
            delay  = DDELAY,        /* Initial delay before sending */
            mdmtyp = 0;             /* Modem type (initially none)  */
#ifndef NOICP
int         tlevel = -1;            /* Take-file command level */
#endif /* ifndef NOICP */

/*
 * Statistics
 * variables
 */

long        filcnt,                 /* Number of files in transaction */
            flci,                   /* Characters from line, current file */
            flco,                   /* Chars to line, current file  */
            tlci,                   /* Chars from line in transaction */
            tlco,                   /* Chars to line in transaction */
            ffc;                    /* Chars to/from current file */
#ifndef NOSTATS
long        tfc;                    /* Chars to/from files in transaction */
int         tsecs;                  /* Seconds for transaction */
#endif

/*
 * Flags
 */

#ifdef DEBUG
#ifndef NOLOGS
int         deblog   = 0;           /* Flag for debug logging */
#endif /* ifndef NOLOGS */
#endif /* ifdef DEBUG */
#ifndef NOLOGS
int         pktlog   = 0,           /* Flag for packet logging */
            seslog   = 0;           /* Session logging */
#endif /* ifndef NOLOGS */
#ifndef NOLOGS
#ifdef TLOG
int         tralog   = 0;           /* Transaction logging */
#endif /* ifdef TLOG */
#endif /* ifndef NOLOGS */
int         displa   = 0,           /* File transfer display on/off */
            stdouf   = 0,           /* Flag for output to stdout */
            xflg     = 0,           /* Flag for X instead of F packet */
            hcflg    = 0,           /* Doing Host command */
            fncnv    = 1,           /* Flag for file name conversion */
            binary   = 1,           /* Flag for binary file */
            savmod   = 0,           /* Saved file mode (whole session) */
            bsave    = 0,           /* Saved file mode (per file) */
            bsavef   = 0,           /* Flag if bsave was used. */
            cmask    = 0177,        /* Connect byte mask */
            fmask    = 0377,        /* File byte mask */
            warn     = 1,           /* Flag for file warning */
            quiet    = 0,           /* Be quiet during file transfer */
            local    = 0,           /* Flag for external tty vs stdout */
#ifndef NOSERVER
            server   = 0,           /* Flag for being a server */
#endif /* ifndef NOSERVER */
#ifndef NOCOCONN
            cnflg    = 0,           /* Connect after transaction */
#endif /* ifndef NOCONN */
            cxseen   = 0,           /* Flag for cancelling a file */
            czseen   = 0,           /* Flag for cancelling file group */
            keep     = 0,           /* Keep incomplete files */
            nakstate = 0;           /* In a state where we can send NAKs */

/*
 * Variables passed from command
 * parser to protocol module
 */

#ifndef NOICP
char        parser();               /* The parser itself */
#endif /* ifndef NOICP */
char        sstate  = 0;            /* Starting state for automaton */
char        *cmarg  = "";           /* Pointer to command data */
char        *cmarg2 = "";           /* Pointer to 2nd command data */
char        **cmlist;               /* Pointer to file list in argv */

/*
 * Miscellaneous
 */

char        **xargv;                /* Global copies of */
int         xargc;                  /* argv and argc */
extern char *dftty;                 /* Default tty name */
extern int  dfloc;                  /* Default location: remote/local */
extern int  dfprty;                 /* Default parity */
extern int  dfflow;                 /* Default flow control */
extern char *ckzsys;

/*
 * Input and output
 * buffers; see getpkt()
 */

CHAR        zinbuffer[INBUFSIZE];
CHAR        zoutbuffer[INBUFSIZE];
CHAR        *zinptr;
CHAR        *zoutptr;
int         zincnt;
int         zoutcnt;

/* M A I N -- uCKermit main program */

FTYPE
main(argc, argv)
int argc;
char **argv;
{
#ifndef __linux__
  char *strcpy();
#endif /* ifndef __linux__ */

  xargc  = argc;                    /* Make global copies of */
  xargv  = argv;                    /* argv and argc */
  sstate = 0;                       /* No default start state. */
  if (sysinit() < 0)
  {
	  
#ifndef NOICP
    doexit(BAD_EXIT);               /* System-dependent initialization. */
#else /* ifndef NOICP */
        exit(BAD_EXIT);
#endif /* ifndef NOICP */
  }

  strcpy(ttname, dftty);            /* Set up default tty name. */
  local  = dfloc;                   /* And whether it's local or remote. */
  parity = dfprty;                  /* Set initial parity, */
  flow   = dfflow;                  /*  and flow control. */

  /*
   * Attempt to take init file
   * before doing command line
   */

#ifndef NOICP
  cmdini();                         /* Sets tlevel */
  while (tlevel > -1)               /* Execute init file. */
  {
    sstate = parser();              /* Loop getting commands. */
    if (sstate)
    {
      proto();                      /* Enter protocol if requested. */
    }
  }
#endif /* ifndef NOICP */

  /*
   * Look for a UNIX-style
   * command line...
   */

  if (argc > 1)                     /* Command line arguments? */
  {
    sstate = cmdlin();              /* Yes, parse. */
    if (sstate)
    {
      proto();                      /* Take any requested action, then */
      if (!quiet)
      {
        conoll("");                 /* put cursor back at left margin, */
      }

#ifndef NOCONN
      if (cnflg)
      {
        conect();                   /* connect if requested, */
      }
#endif /* ifndef NOCONN */
#ifndef NOICP
      doexit(GOOD_EXIT);            /* and then exit with status 0. */
#else /* ifndef NOICP */
          return( GOOD_EXIT );
#endif /* ifndef NOICP */
    }
  }

  /*
   * If no action requested on command
   * line, enter interactive parser.
   */

#ifndef NOICP
  herald();                         /* Display program herald. */
  while (1)                         /* Loop getting commands. */
  {
    sstate = parser();
#endif /* ifndef NOICP */
    if (sstate)
    {
      proto();                      /* Enter protocol if requested. */
    }
#ifndef NOICP
  }
#endif /* ifndef NOICP */

  /*
   * If interactive mode
   * is not available...
   */

  printf("\r%s,%s", versio, ckzsys);
  printf("\r\nError: No action requested");
#ifdef NODOHLP
  printf(".\r\n");
#else /* ifdef NODOHLP */
  printf(", use -h for help\r\n");
#endif /* ifdef NODOHLP */

  return ( 1 );
}
