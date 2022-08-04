#ifndef NOICP
char *userv = "   JSYS UI, 4G(125)";
#endif /* ifndef NOICP */

/* C K U U S R -- "User Interface" (Part 1) */

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

/*
 *  The ckuusr module contains the terminal input and output functions
 *  for UNIX Kermit.  It includes a simple UNIX-style command line
 *  parser as well as an interactive prompting keyword command parser.
 *  It depends on the existence of UNIX facilities like fopen, fgets,
 *  feof, (f)printf, argv/argc, etc.  Other functions that are likely
 *  to vary among UNIX implementations -- like setting terminal modes
 *  or interrupts -- are invoked via calls to functions that are defined
 *  in the system-dependent modules, ck?[ft]io.c.
 *
 *  The command line parser processes any arguments found on the command
 *  line, as passed to main() via argv/argc.  The interactive parser uses
 *  the facilities of the cmd package (developed for this program, but
 *  usable by any program).
 *
 *  Any command parser may be substituted for this one.  The only
 *  requirements for the Kermit command parser are these:
 *
 * 1. Set parameters via global variables like duplex, speed, ttname, etc.
 *  See ckmain.c for the declarations and descriptions of these variables.
 *
 * 2. If a command can be executed without the use of Kermit protocol, then
 *  execute the command directly and set the variable sstate to 0. Examples
 *  include 'set' commands, local directory listings, the 'connect' command.
 *
 * 3. If a command requires the Kermit protocol, set the following
 *  variables:
 *
 *  sstate                             string data
 *     'x' (enter server mode)            (none)
 *     'r' (send a 'get' command)         cmarg, cmarg2
 *     'v' (enter receive mode)           cmarg2
 *     'g' (send a generic command)       cmarg
 *     's' (send files)                   nfils, cmarg & cmarg2 OR cmlist
 *     'c' (send a remote host command)   cmarg
 *
 *   cmlist is an array of pointers to strings.
 *   cmarg, cmarg2 are pointers to strings.
 *   nfils is an integer.
 *
 *   cmarg can be a filename string (possibly wild), or
 *      a pointer to a prefabricated generic command string, or
 *      a pointer to a host command string.
 *   cmarg2 is the name to send a single file under, or
 *      the name under which to store an incoming file; must not be wild.
 *   cmlist is a list of nonwild filenames, such as passed via argv.
 *   nfils is an integer, interpreted as follows:
 *     -1: argument string is in cmarg, and should be expanded internally.
 *      0: stdin.
 *     >0: number of files to send, from cmlist.
 *
 *  The screen() function is used to update the screen during file transfer.
 *  The tlog() function maintains a transaction log.
 *  The debug() function maintains a debugging log.
 *  The intmsg() and chkint() functions provide the
 *    user i/o for interrupting file transfers.
 */

#include "ckcdeb.h"
#include <ctype.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include "ckcker.h"
#include "ckucmd.h"
#include "ckuusr.h"

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#endif /* ifdef __linux__ */

#ifndef NOSERVER
extern int server;
#endif /* ifndef NOSERVER */

extern int size, rpsiz, urpsiz, speed, local, binary;
#ifndef NOICP
extern int displa;
#endif /* ifndef NOICP */
extern int  parity, deblog, escape, xargc, flow, turn, duplex, nfils;
extern int  ckxech, pktlog, seslog, tralog, stdouf, turnch, dfloc, keep;
extern int  maxrps, warn, quiet, cnflg, tlevel, mdmtyp, zincnt;

extern char *versio;
#ifdef WART
extern char *protv;
#endif /* ifdef WART */
extern char *ckxv, *ckzv, *fnsv, *dftty, *cmdv;
#ifndef NOICP
extern char *connv;
#endif /* ifndef NOICP */
extern char *wartv;
#ifndef NOCKUDIA
extern char *dialv;
#endif /* ifndef NOCKUDIA */
#ifndef NOCKUSCR
extern char *loginv;
#endif /* ifndef NOCKUSCR */
extern char *ckxsys, *cmarg, *cmarg2;
extern char **xargv, **cmlist;
extern char *DIRCMD, *PWDCMD, cmerrp[];
extern CHAR sstate, ttname[];
extern CHAR *zinptr;
char *strcpy(), *getenv();

extern char cmdbuf[];          /* Command buffer */
#ifndef NOSPACE
extern char *SPACMD, *zhome(); /* Space command, home directory. */
#endif /* ifndef NOSPACE */
extern int backgrd;            /* Kermit executing in background */

/*
 * The background flag is set by ckutio.c
 * ( via conint() ) to note whether
 * this kermit is executing in background
 * ( '&' on shell command line ).
 */

#ifndef NOICP
char line[CMDBL + 10], *lp; /* Character buffer for anything */
#endif /* ifndef NOICP */
#ifndef NOLOGS
char debfil[30];            /* Debugging log file name */
char pktfil[30];            /* Packet log file name */
char sesfil[30];            /* Session log file name */
char trafil[30];            /* Transaction log file name */
#endif /* ifndef NOLOGS */

int cflg;                   /* Command-line connect cmd given */
int action;                 /* Action selected on command line*/
#ifndef NOICP
int repars;                 /* Reparse needed */
int cwdf = 0;               /* CWD has been done */

#define MAXTAKE 3           /* Maximum nesting of TAKE files */
FILE *tfile[MAXTAKE];       /* File pointers for TAKE command */

char *homdir;               /* Pointer to home directory string */
#endif /* ifndef NOICP */
#ifndef MINBUF
char cmdstr[100];           /* Place to build generic command */
#else /* ifndef MINBUF */
char cmdstr[48];
#endif /* ifndef MINBUF */

/* C M D L I N -- Get arguments from command line */

/*
 * Simple UNIX-style command line parser, conforming with
 *   'A Proposed Command Syntax Standard for UNIX Systems',
 *   Hemenway & Armitage, UNIX/World, Vol.1, No.3, 1984.
 */

int
cmdlin()
{
  char x;                   /* Local general-purpose int */

  cmarg = "";               /* Initialize globals */
  cmarg2 = "";
  action = cflg = 0;

  while (--xargc > 0)       /* Go through command line words */
  {
    xargv++;
    debug(F111, "xargv", *xargv, xargc);
    if (**xargv == '-')     /* Got an option (begins with dash) */
    {
      x = *( *xargv + 1 );  /* Get the option letter */
      if (doarg(x) < 0)
      {
        doexit(BAD_EXIT);   /* Go handle the option */
      }
    }
    else                    /* No dash where expected */
    {
      usage();
      doexit(BAD_EXIT);
    }
  }
  debug(F101, "action", "", action);
  if (!local)
  {
    if (
      ( action == 'g' ) || ( action == 'r' ) || ( action == 'c' ) ||
      ( cflg != 0 ))
    {
      fatal("-l and -b required");
    }
  }

  if (*cmarg2 != 0)
  {
    if (( action != 's' ) && ( action != 'r' ) && ( action != 'v' ))
    {
      fatal("-a without -s, -r, or -g");
    }
  }

  if (( action == 'v' ) && ( stdouf ) && ( !local ))
  {
    if (isatty(1))
    {
      fatal("unredirected -k only for local mode");
    }
  }

  if (( action == 's' ) || ( action == 'v' ) || ( action == 'r' ) ||
      ( action == 'x' ))
  {
    if (local)
    {
#ifndef NOICP
      displa = 1;
#endif /* ifndef NOICP */
    }

    if (stdouf)
    {
#ifndef NOICP
      displa = 0;
      quiet = 1;
#endif /* ifndef NOICP */
    }
  }

  if (quiet)
  {
#ifndef NOICP
    displa = 0; /* No display if quiet requested */
#endif /* ifndef NOICP */
  }

  if (cflg)
  {
#ifndef NOCONN
    conect(); /* Connect if requested */
#endif /* ifndef NOCONN */
    if (action == 0)
    {
      if (cnflg)
      {
#ifndef NOCONN
        conect();        /* And again if requested */
#endif /* if def NOCON  */
      }
      doexit(GOOD_EXIT); /* Then exit indicating success */
    }
  }

#ifndef NOICP
  if (displa)
  {
    concb(escape); /* (for console "interrupts") */
  }
#endif /* ifndef NOICP */

  return ( action );   /* Then do any requested protocol */
}

/* D O A R G -- Do a command-line argument */

int
doarg(x)
char x;
{
  int z;
  char *xp;

  xp = *xargv + 1; /* Pointer for bundled args */
  while (x)
  {
    switch (x)
    {
#ifndef NOSERVER
    case 'x': /* server */
      if (action)
      {
        fatal("bad request");
      }

      action = 'x';
      break;
#endif
    case 'f':
      if (action)
      {
        fatal("bad request");
      }

      action = setgen('F', "", "", "");
      break;

    case 'r': /* receive */
      if (action)
      {
        fatal("bad request");
      }

      action = 'v';
      break;

    case 'k': /* receive to stdout */
      if (action)
      {
        fatal("bad request");
      }

      stdouf = 1;
      action = 'v';
      break;

    case 's': /* send */
      if (action)
      {
        fatal("bad request");
      }

      if (*( xp + 1 ))
      {
        fatal("invalid arg");
      }

      z = nfils = 0;        /* Initialize file counter, flag */
      cmlist = xargv + 1;   /* Remember this pointer */
      while (--xargc > 0)   /* Traverse the list */
      {
        xargv++;
        if (**xargv == '-')   /* Check for sending stdin */
        {
          if (strcmp(*xargv, "-") != 0)
          {
            break;
          }

          z++;
        }

        nfils++; /* Bump file counter */
      }
      xargc++, xargv--; /* Adjust argv/argc */
      if (nfils < 1)
      {
        fatal("missing filename");
      }

      if (z > 1)
      {
        fatal("-s: too many -'s");
      }

      if (z == 1)
      {
        if (nfils == 1)
        {
          nfils = 0;
        }
        else
        {
          fatal("invalid mix of filenames and '-' in -s");
        }
      }

      if (nfils == 0)
      {
        if (isatty(0))
        {
          fatal("terminal input not allowed");
        }
      }

      debug(F101, *xargv, "", nfils);
      action = 's';
      break;

    case 'g': /* get */
      if (action)
      {
        fatal("bad request");
      }

      if (*( xp + 1 ))
      {
        fatal("invalid arg");
      }

      xargv++, xargc--;
      if (( xargc == 0 ) || ( **xargv == '-' ))
      {
        fatal("missing filename");
      }

      cmarg = *xargv;
      action = 'r';
      break;

#ifndef NOCONN
    case 'c': /* connect before */
      cflg = 1;
      break;

    case 'n': /* connect after */
      cnflg = 1;
      break;
#endif /* ifndef NOCONN */

#ifndef NODOHLP
    case 'h': /* help */
      usage();
      doexit(GOOD_EXIT);
#endif /* ifndef NODOHLP */

    case 'a': /* "as" */
      if (*( xp + 1 ))
      {
        fatal("invalid arg");
      }

      xargv++, xargc--;
      if (( xargc < 1 ) || ( **xargv == '-' ))
      {
        fatal("missing name");
      }

      cmarg2 = *xargv;
      break;

    case 'l': /* set line */
      if (*( xp + 1 ))
      {
        fatal("invalid arg");
      }

      xargv++, xargc--;
      if (( xargc < 1 ) || ( **xargv == '-' ))
      {
        fatal("missing name");
      }

      strcpy(ttname, *xargv);
      /*  if (strcmp(ttname,dftty) == 0) local = dfloc; else local = 1;  */
      local = ( strcmp(ttname, CTTNAM) != 0 ); /* (better than old way) */
      debug(F101, "local", "", local);
      ttopen(ttname, &local, 0);
      break;

    case 'b': /* set baud */
      if (*( xp + 1 ))
      {
        fatal("invalid arg");
      }

      xargv++, xargc--;
      if (( xargc < 1 ) || ( **xargv == '-' ))
      {
        fatal("missing baud");
      }

      z = atoi(*xargv); /* Convert to number */
      if (chkspd(z) > -1)
      {
        speed = z; /* Check it */
      }
      else
      {
        fatal("unsupported baud");
      }

      break;

    case 'e': /* Extended packet length */
      if (*( xp + 1 ))
      {
        fatal("invalid arg");
      }

      xargv++, xargc--;
      if (( xargc < 1 ) || ( **xargv == '-' ))
      {
        fatal("missing length");
      }

      z = atoi(*xargv); /* Convert to number */
      if (z > 10 && z < maxrps)
      {
        rpsiz = urpsiz = z;
        if (z > 94)
        {
          rpsiz = 94; /* Fallback if other Kermit can't */
        }
      }
      else
      {
        fatal("Unsupported length");
      }

      break;

    case 'i': /* Treat files as text */
      binary = 0;
      break;

    case 'w': /* File overwrite */
      warn = 0;
      break;

    case 'q': /* Quiet */
      quiet = 1;
      break;

    case 'd': /* debug */
#ifndef NOLOGS
#ifdef DEBUG
      debopn("debug.log");
#endif /* ifdef DEBUG */
#endif /* ifndef NOLOGS */
      break;

    case 'p': /* set parity */
      if (*( xp + 1 ))
      {
        fatal("invalid arg");
      }

      xargv++, xargc--;
      if (( xargc < 1 ) || ( **xargv == '-' ))
      {
        fatal("missing parity");
      }

      switch (x = **xargv)
      {
      case 'e':
      case 'o':
      case 'm':
      case 's':
        parity = x;
        break;

      case 'n':
        parity = 0;
        break;

      default:
        fatal("invalid parity");
      }
      break;

    case 't':
      turn = 1;     /* Line turnaround handshake */
      turnch = XON; /* XON is turnaround character */
      duplex = 1;   /* Half duplex */
      flow = 0;     /* No flow control */
      break;

    default:
      fatal("invalid arg, try '-h' for help");
    }

    x = *++xp; /* See if options are bundled */
  }
  return ( 0 );
}

/*
 * Misc
 */

int
fatal(msg)
char *msg;
{
  /* Fatal error message */
  fprintf(stderr, "\rFatal: %s\r\n", msg);
#ifndef NOLOGS
#ifdef TLOG
  tlog(F110, "Fatal:", msg, 0l);
#endif /* ifdef TLOG */
#endif /* ifndef NOLOGS */
  doexit(BAD_EXIT);   /* Exit indicating failure */
  return ( BAD_EXIT );
}

int
ermsg(msg)
char *msg;
{
  /* Print error message */
  if (!quiet)
  {
    fprintf(stderr, "\r\n%s - %s\n",
#ifndef NOICP
      cmerrp,
#else /* ifndef NOICP */
          "",
#endif /* ifndef NOICP */
        msg);
  }

#ifndef NOLOGS
#ifdef TLOG
  tlog(F110, "Error -", msg, 0l);
#endif /* ifdef TLOG */
#endif /* ifndef NOLOGS */
  return ( 0 );
}

/*
 * Interactive command
 * parser
 */

/*
 * Top-Level Keyword
 * Table
 */

#ifndef NOICP
struct keytab cmdtab[] = {
#ifndef NOPUSH
  "!",          XXSHE, 0,
#endif /* ifndef NOPUSH */
  "%",          XXCOM, CM_INV,
  "bye",        XXBYE, 0,
#ifndef NOCONN
  "c",          XXCON, CM_INV,
#endif /* ifndef NOCONN */
  "cd",         XXCWD, 0,
  "close",      XXCLO, 0,
#ifndef NOCONN
  "connect",    XXCON, 0,
#endif /* ifndef NOCONN */
  "cwd",        XXCWD, 0,
#ifndef NOCKUDIA
  "dial",       XXDIAL, 0,
#endif /* ifndef NOCKUDIA */
  "directory",  XXDIR, 0,
  "echo",       XXECH, 0,
  "exit",       XXEXI, 0,
  "finish",     XXFIN, 0,
  "get",        XXGET, 0,
#ifndef NOCKUDIA
  "hangup",     XXHAN, 0,
#endif /* ifndef NOCKUDIA */
#ifndef NODOHLP
  "help",       XXHLP, 0,
#endif /* ifndef NODOHLP */
#ifndef NOLOGS
  "log",        XXLOG, 0,
#endif /* ifndef NOLOGS */
  "quit",       XXQUI, 0,
/* "r",         XXREC,  CM_INV, */
  "receive",    XXREC, 0,
  "remote",     XXREM, 0,
/* "s",         XXSEN,  CM_INV, */
#ifndef NOCKUSCR
  "script",     XXLOGI, 0,
#endif /* ifndef NOCKUSCR */
  "send",       XXSEN, 0,
#ifndef NOSERVER
  "server",     XXSER, 0,
#endif /* ifndef NOSERVER */
  "set",        XXSET, 0,
  "show",       XXSHO, 0,
#ifndef NOSPACE
  "space",      XXSPA, 0,
#endif /* ifndef NOSPACE */
#ifndef NOSTATS
  "statistics", XXSTA, 0,
#endif /* ifndef NOSTATS */
  "take",       XXTAK, 0,
  "transmit",   XXTRA, 0
};

int ncmd = ( sizeof ( cmdtab ) / sizeof ( struct keytab ));

/*
 * Parameter keyword
 * table
 */

struct keytab prmtab[] = {
  "attributes", XYATTR, 0,
  "baud", XYSPEE, CM_INV,
  "block-check", XYCHKT, 0,
  "delay", XYDELA, 0,
  "duplex", XYDUPL, 0,
#ifdef COMMENT
  "end-of-packet", XYEOL, CM_INV,      /* moved to send/receive */
#endif /* ifdef COMMENT */
  "escape-character", XYESC, 0,
  "file", XYFILE, 0,
  "flow-control", XYFLOW, 0,
  "handshake", XYHAND, 0,
  "incomplete", XYIFD, 0,
  "line", XYLINE, 0,
#ifndef NOCKUDIA
  "modem-dialer", XYMODM, 0,
#endif /* ifndef NOCKUDIA */
#ifdef COMMENT
  "packet-length", XYLEN, CM_INV,      /* moved to send/receive */
  "pad-character", XYPADC, CM_INV,     /* moved to send/receive */
  "padding", XYNPAD, CM_INV,           /* moved to send/receive */
#endif /* ifdef COMMENT */
  "parity", XYPARI, 0,
  "prompt", XYPROM, 0,
  "receive", XYRECV, 0,
  "retry", XYRETR, 0,
  "send", XYSEND, 0,
#ifndef NOSERVER
  "server", XYSERV, 0,
#endif /* ifndef NOSERVER */
  "speed", XYSPEE, 0,
#ifdef COMMENT
  "start-of-packet", XYMARK, CM_INV,   /* moved to send/receive */
#endif /* ifndef COMMENT */
  "terminal", XYTERM, 0
/*  "timeout", XYTIMO, CM_INV */       /* moved to send/receive */
};

int nprm = \
  ( sizeof ( prmtab ) / sizeof ( struct keytab )); /* How many parameters */

/*
 * Remote Command
 * Table
 */

struct keytab remcmd[] = {
  "cd",        XZCWD,    CM_INV,
  "cwd",       XZCWD,    0,
  "delete",    XZDEL,    0,
  "directory", XZDIR,    0,
  "help",      XZHLP,    0,
  "host",      XZHOS,    0,
  "space",     XZSPA,    0,
  "type",      XZTYP,    0,
  "who",       XZWHO,    0
};

int nrmt = ( sizeof ( remcmd ) / sizeof ( struct keytab ));

#ifndef NOLOGS
struct keytab logtab[] = {
  "debugging",    LOGD, 0,
  "packets",      LOGP, 0,
  "session",      LOGS, 0,
  "transactions", LOGT, 0
};
#endif /* ifndef NOLOGS */

#ifndef NOLOGS
int nlog = ( sizeof ( logtab ) / sizeof ( struct keytab ));
#endif /* ifndef NOLOGS */
#endif /* ifndef NOICP */

/*
 * Show command
 * arguments
 */

#define SHPAR 0 /* Parameters */
#define SHVER 1 /* Versions */

#ifndef NOICP
struct keytab shotab[] = {
  "parameters", SHPAR, 0,
  "versions",   SHVER, 0
};
#endif /* ifndef NOICP */

/* C M D I N I -- Initialize the interactive command parser */

#ifndef NOICP
int
cmdini()
{
  tlevel = -1;         /* Take file level */
  cmsetp("uCKermit>"); /* Set default prompt */

  /*
   * Look for init file in
   * home or current directory.
   */

  homdir = zhome();
  lp = line;
  lp[0] = '\0';
  if (homdir)
  {
    strcpy(lp, homdir);
    if (lp[0] == '/')
    {
      strcat(lp, "/");
    }
  }

  strcat(lp, KERMRC);
  if (( tfile[0] = fopen(line, "r")) != NULL)
  {
    tlevel = 0;
    debug(F110, "init file", line, 0);
  }

  if (homdir && ( tlevel < 0 ))
  {
    strcpy(lp, KERMRC);
    if (( tfile[0] = fopen(line, "r")) != NULL)
    {
      tlevel = 0;
    }
  }

  congm(); /* Get console tty modes */
  return ( 0 );
}
#endif /* ifndef NOICP */

/*
 * Display version herald
 * and initial prompt
 */

#ifndef NOICP
int
herald()
{
  if (!backgrd)
  {
    printf("%s,%s\n", versio, ckxsys);
#ifndef NODOHLP
    printf("Interactive mode.  Type '?' for help.\n");
#endif /* ifndef NODOHLP */
  }
  return ( 0 );
}
#endif /* ifndef NOICP */

/* T R A P -- Terminal interrupt handler */

#ifndef NOICP
int
trap(
#ifdef DEBUG
  sig,
  code
#endif /* ifdef DEBUG */
  )
#ifdef DEBUG
int sig, code;
#endif /* ifdef DEBUG */
{
  fprintf(stderr, "\r^C...\n");
#ifdef DEBUG
  debug(F101, "trap() caught signal", "", sig);
  debug(F101, " code", "", code);
#endif /* ifdef DEBUG */
  doexit(GOOD_EXIT); /* Exit indicating success */
  return ( 0 );
}
#endif /* ifndef NOICP */

/* S T P T R A P -- Handle SIGTSTP signals */

#ifdef __linux__
void
#endif /* ifdef __linux__ */
stptrap(
#ifdef DEBUG
  sig,
  code
#endif /* ifdef DEBUG */
  )
#ifdef DEBUG
int sig, code;
#endif /* ifdef DEBUG */
{
#ifdef DEBUG
  debug(F101, "stptrap() caught signal", "", sig);
  debug(F101, " code", "", code);
#endif /* ifdef DEBUG */
  conres(); /* Reset the console */
#ifdef SIGTSTP
  kill(0, SIGSTOP);   /* If job control, suspend the job */
#else  /* ifdef SIGTSTP */
  doexit(GOOD_EXIT);   /* Probably won't happen otherwise */
#endif /* ifdef SIGTSTP */
  concb(escape); /* Put console back in Kermit mode */
  if (!backgrd)
  {
#ifndef NOICP
    prompt(); /* Reissue prompt when fg'd */
#endif /* ifndef NOICP */
  }
}

/* P A R S E R -- Top-level interactive command parser */
#ifndef NOICP
char
parser()
{
  int xx, cbn;
  char *cbp;

  concb(escape); /* Put console in cbreak mode. */
  conint(trap);  /* Turn on console terminal interrupts. */

  /*
   * sstate becomes nonzero when a command has been parsed that requires
   * some action from the protocol module.  Any non-protocol actions,
   * such as local directory listing or terminal emulation, are invoked
   * directly from below
   */

  if (local && !backgrd)
  {
    printf("\n");       /* Ensure prompt will be visible */
  }

  sstate = 0;           /* Start with no start state. */
  while (sstate == 0)   /* Parse cmds until action requested */
  {
    while (( tlevel > -1 ) && feof(tfile[tlevel])) /* If end of take */
    {
      fclose(tfile[tlevel--]);                     /* file, close it. */
      cmini(ckxech);    /* and clear the cmd buffer. */
      if (tlevel < 0)   /* Just popped out of cmd files? */
      {
        conint(trap);   /* Check background stuff again. */
        return ( 0 );   /* End of init file or whatever. */
      }
    }
    debug(F101, "tlevel", "", tlevel);
    if (tlevel > -1)   /* If in take file */
    {
      cbp = cmdbuf;    /* Get the next line. */
      cbn = CMDBL;

      /*
       * Loop to get next command line and
       * all continuation lines from take
       * file.
       */

again:
      if (fgets(line, cbn, tfile[tlevel]) == NULL)
      {
        continue;
      }

      lp = line; /* Got one, copy it. */
      while (( *cbp++ = *lp++ ))
      {
        if (--cbn < 1)
        {
          fatal("Command too long");
        }
      }
      if (*( cbp - 3 ) == '\\') /* Continued on next line? */
      {
        cbp -= 3;               /* If so, back up pointer, */
        goto again;             /* go back, get next line. */
      }

      stripq(cmdbuf); /* Strip any quotes from cmd buffer. */
    }
    else     /* No take file, get typein. */
    {
      if (!backgrd)
      {
        prompt(); /* Issue interactive prompt. */
      }

      cmini(ckxech);
    }
#ifndef NOICP
    repars = 1;
    displa = 0;
    while (repars)
    {
      cmres(); /* Reset buffer pointers. */
      xx = cmkey(cmdtab, ncmd, "Command", "");
      debug(F101, "top-level cmkey", "", xx);
      switch (docmd(xx))
      {
      case -4:             /* EOF */
        doexit(GOOD_EXIT); /* ...exit successfully */

      case -1:             /* Reparse needed */
        repars = 1;
        continue;

      case -2:       /* Invalid command given */
        if (backgrd) /* if in background, terminate */
        {
          fatal("error in background execution");
        }

        if (tlevel > -1)   /* If in take file, quit */
        {
          ermsg("Error: take file terminated.");
          fclose(tfile[tlevel]);
          tlevel--;
        }

        cmini(ckxech); /* (fall thru) */

      case -3:         /* Empty command OK at top level */
      default:         /* Anything else (fall thru) */
        repars = 0;    /* No reparse, get new command. */
        continue;
      }
    }
#endif /* ifndef NOICP */
  }

  /*
   * Got an action command; disable terminal
   * interrupts and return start state
   */

  if (!local)
  {
    connoi(); /* Interrupts off only if remote */
  }

  return ( sstate );
}
#endif /* ifndef NOICP */

/* D O E X I T -- Exit from the program. */

int
doexit(exitstat)
int exitstat;
{
  ttclos();                /* Close external line, if any */
  if (local)
  {
    strcpy(ttname, dftty); /* Restore default tty */
    local = dfloc;         /* And default remote/local status */
  }

  if (!quiet)
  {
    conres();              /* Restore console terminal. */
  }

  if (!quiet)
  {
    connoi();              /* Turn off console interrupt traps. */
  }

#ifndef NOLOGS
#ifdef DEBUG
  if (deblog)              /* Close any open logs. */
  {
    debug(F100, "Debug Log Closed", "", 0);
    *debfil = '\0';
    deblog = 0;
    zclose(ZDFILE);
  }
#endif /* ifdef DEBUG */

  if (pktlog)
  {
    *pktfil = '\0';
    pktlog = 0;
    zclose(ZPFILE);
  }

  if (seslog)
  {
    *sesfil = '\0';
    seslog = 0;
    zclose(ZSFILE);
  }

#ifdef TLOG
  if (tralog)
  {
    tlog(F100, "Transaction Log Closed", "", 0l);
    *trafil = '\0';
    tralog = 0;
    zclose(ZTFILE);
  }
#endif /* ifdef TLOG */
#endif/* ifndef NOLOGS */

  syscleanup();
  exit(exitstat); /* Exit from the program. */
  return ( exitstat );
}

/* B L D L E N -- Make length-encoded copy of string */

char *
bldlen(str, dest)
char *str, *dest;
{
  int len;
  len = strlen(str);
  *dest = tochar(len);
  strcpy(dest + 1, str);
  return ( dest + len + 1 );
}

/* S E T G E N -- Construct a generic command */

int
setgen(type, arg1, arg2, arg3)
char type, *arg1, *arg2, *arg3;
{
  char *upstr, *cp;

  cp = cmdstr;
  *cp++ = type;
  *cp = NUL;
  if (*arg1 != NUL)
  {
    upstr = bldlen(arg1, cp);
    if (*arg2 != NUL)
    {
      upstr = bldlen(arg2, upstr);
      if (*arg3 != NUL)
      {
        bldlen(arg3, upstr);
      }
    }
  }

  cmarg = cmdstr;
  debug(F110, "setgen", cmarg, 0);

  return ( 'g' );
}

/* D O C M D -- Do a command */

/*
 *  Returns:
 *  -2: user typed an illegal command
 *  -1: reparse needed
 *   0: parse was successful (even tho command may have failed).
 */

#ifndef NOICP
int
docmd(cx)
int cx;
{
  int x, y;
  char *s;

  switch (cx)
  {
  case -4: /* EOF */
    if (!quiet && !backgrd)
    {
      printf("\r\n");
    }

    doexit(GOOD_EXIT);

  case -3: /* Null command */
    return ( 0 );

  case -2: /* Error */
  case -1: /* Reparse needed */
    return ( cx );

  case XXBYE: /* bye */
    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    if (!local)
    {
      printf("'set line' first\n");
      return ( 0 );
    }

    sstate = setgen('L', "", "", "");
    return ( 0 );

  case XXCOM: /* comment */
    if (( x = cmtxt("comment", "", &s)) < 0)
    {
      return ( x );
    }

    return ( 0 );

  case XXCON: /* connect */
    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    return ( doconect());

  case XXCWD:
    if (( x = cmdir(
      "Name of dir, or return",
        homdir,
          &s))
            < 0)
    {
      return ( x );
    }

    if (x == 2)
    {
      printf("\n?Wildcards not allowed in dir name\n");
      return ( -2 );
    }

    if (!zchdir(s))
    {
      perror(s);
    }

    cwdf = 1;
    system(PWDCMD);
    return ( 0 );

#ifndef NOLOGS
  case XXCLO:
    x = cmkey(logtab, nlog, "Which log to close", "");
    if (x == -3)
    {
      printf("?Which log?\n");
      return ( -2 );
    }

    if (x < 0)
    {
      return ( x );
    }

    if (( y = cmcfm()) < 0)
    {
      return ( y );
    }

    switch (x)
    {
    case LOGD:
      if (deblog == 0)
      {
        printf("?Debugging log wasn't open\n");
        return ( 0 );
      }

      *debfil = '\0';
      deblog = 0;
      return ( zclose(ZDFILE) );

    case LOGP:
      if (pktlog == 0)
      {
        printf("?Packet log wasn't open\n");
        return ( 0 );
      }

      *pktfil = '\0';
      pktlog = 0;
      return ( zclose(ZPFILE) );

    case LOGS:
      if (seslog == 0)
      {
        printf("?Session log wasn't open\n");
        return ( 0 );
      }

      *sesfil = '\0';
      seslog = 0;
      return ( zclose(ZSFILE) );

    case LOGT:
      if (tralog == 0)
      {
        printf("?Transaction log wasn't open\n");
        return ( 0 );
      }

      *trafil = '\0';
      tralog = 0;
      return ( zclose(ZTFILE) );

    default:
      printf("\n?No log - %ld\n", (long)x);
      return ( 0 );
    }
#endif /* ifndef NOLOGS */

  case XXDIAL: /* dial number */
#ifndef NOCKUDIA
    if (( x = cmtxt("Number to dial", "", &s)) < 0)
    {
      return ( x );
    }

    debug(F110, "ckuusr calling ckdial", s, 0);
    return ( ckdial(s) );
#endif /* ifndef NOCKUDIA */

  case XXDIR: /* directory */
    if (( x = cmdir("Dir/file spec", "*", &s)) < 0)
    {
      return ( x );
    }

    lp = line;
    sprintf(lp, "%s %s", DIRCMD, s);
    system(line);
    return ( 0 );

  case XXECH: /* echo */
    if (( x = cmtxt("data to echo", "", &s)) < 0)
    {
      return ( x );
    }

    for (; *s; s++)
    {
      if (( x = *s ) == 0134) /* Convert octal escapes */
      {
        s++;                  /* up to 3 digits */
        for (x = y = 0;
          *s >= '0' && *s <= '7' && y < 3;
            s++, y++)
        {
          x = x * 8 + (int)*s - 48;
        }

        s--;
      }

      putchar(x);
    }

    printf("\n");
    return ( 0 );

  case XXQUI: /* quit, exit */
  case XXEXI:
    if (( x = cmcfm()) > -1)
    {
      doexit(GOOD_EXIT);
    }
    else
    {
      return ( x );
    }

  case XXFIN: /* finish */
    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    if (!local)
    {
      printf("'set line' first\n");
      return ( 0 );
    }

    sstate = setgen('F', "", "", "");
    return ( 0 );

  case XXGET: /* get */
    if (!local)
    {
      printf("\n'set line' first\n");
      return ( 0 );
    }

    x = cmtxt("Name of file(s), or return", "", &cmarg);
    if (( x == -2 ) || ( x == -1 ))
    {
      return ( x );
    }

    /*
     * If foreign file name omitted, get
     * foreign and local names separately
     */

    x = 0; /* For some reason cmtxt returns 1 */
    if (*cmarg == NUL)
    {
      if (tlevel > -1)   /* Input is from take file */
      {
        if (fgets(line, CMDBL, tfile[tlevel]) == NULL)
        {
          fatal("take file ends prematurely in 'get'");
        }

        debug(F110, "take-get 2nd line", line, 0);
        stripq(line);
        for (x = strlen(line);
             x > 0 && ( line[x - 1] == LF || line[x - 1] == CR ); x--)
        {
          line[x - 1] = '\0';
        }

        cmarg = line;
        if (fgets(cmdbuf, CMDBL, tfile[tlevel]) == NULL)
        {
          fatal("take file ends prematurely in 'get'");
        }

        stripq(cmdbuf);
        for (x = strlen(cmdbuf);
             x > 0 && ( cmdbuf[x - 1] == LF || cmdbuf[x - 1] == CR ); x--)
        {
          cmdbuf[x - 1] = '\0';
        }

        if (*cmdbuf == NUL)
        {
          cmarg2 = line;
        }
        else
        {
          cmarg2 = cmdbuf;
        }

        x = 0; /* Return code */
      }
      else     /* Input is from terminal */
      {
        char psave[40]; /* Save old prompt */
        cmsavp(psave, 40);
        cmsetp(" Remote file spec: "); /* Make new one */
        cmini(ckxech);
        x = -1;
        if (!backgrd)
        {
          prompt();
        }

        while (x == -1)   /* Prompt till they answer */
        {
          x = cmtxt("Name of remote file(s)", "", &cmarg);
          debug(F111, " cmtxt", cmarg, x);
        }
        if (x < 0)
        {
          cmsetp(psave);
          return ( x );
        }

        if (*cmarg == NUL)         /* If user types a bare CR, */
        {
          printf("(cancelled)\n"); /* Forget about this. */
          cmsetp(psave);           /* Restore old prompt, */
          return ( 0 );                /* and return. */
        }

        strcpy(line, cmarg); /* Make a safe copy */
        cmarg = line;
        cmsetp(" Local name to store it under: "); /* New prompt */
        cmini(ckxech);
        x = -1;
        if (!backgrd)
        {
          prompt();
        }

        while (x == -1) /* Again, parse till answered */
        {
          x = cmofi("Local file name", "", &cmarg2);
        }
        if (x == -3)               /* If bare CR, */
        {
          printf("(cancelled)\n"); /* escape from this... */
          cmsetp(psave);           /* Restore old prompt, */
          return ( 0 );                /* and return. */
        }
        else if (x < 0)
        {
          return ( x ); /* Handle parse errors. */
        }

        x = -1; /* Get confirmation. */
        while (x == -1)
        {
          x = cmcfm();
        }
        cmsetp(psave); /* Restore old prompt. */
      }
    }

    if (x == 0)     /* Good return from cmtxt or cmcfm, */
    {
      sstate = 'r'; /* set start state. */
      if (local)
      {
        displa = 1;
      }
    }

    return ( x );

#ifndef NODOHLP
  case XXHLP:   /* Help */
    x = cmkey(cmdtab, ncmd, "command", "help");
    return ( dohlp(x));
#endif /* ifndef NODOHLP */

#ifndef NOCKUDIA
  case XXHAN: /* Hangup */
    if (( x = cmcfm()) > -1)
    {
      return ( tthang());
    }
#endif /* ifndef NOCKUDIA */

#ifndef NOLOGS
  case XXLOG: /* Log */
    x = cmkey(logtab, nlog, "What to log", "");
    if (x == -3)
    {
      printf("?What is to be logged?\n");
      return ( -2 );
    }

    if (x < 0)
    {
      return ( x );
    }

    return ( dolog(x));
#endif /* ifndef NOLOGS */

#ifndef NOCKUSCR
  case XXLOGI:   /* Send script remote system */
    if (( x = cmtxt("Text of login script", "", &s)) < 0)
    {
      return ( x );
    }

    return ( login(s));   /* Return 0=completed, -2=failed */
#endif /* ifndef NOCKUSCR */

  case XXREC: /* Receive */
    cmarg2 = "";
    x = cmofi("Name under which to store the file, or CR", "", &cmarg2);
    if (( x == -1 ) || ( x == -2 ))
    {
      return ( x );
    }

    debug(F111, "cmofi cmarg2", cmarg2, x);
    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    sstate = 'v';
    if (local)
    {
      displa = 1;
    }

    return ( 0 );

  case XXREM: /* Remote */
    if (!local)
    {
      printf("\n'set line' first\n");
      return ( -2 );
    }

    x = cmkey(remcmd, nrmt, "Remote command", "");
    if (x == -3)
    {
      printf("?Command for remote?\n");
      return ( -2 );
    }

    return ( dormt(x));

  case XXSEN: /* Send */
    cmarg = cmarg2 = "";
    if (( x = cmifi("File(s) to send", "", &s, &y)) < 0)
    {
      if (x == -3)
      {
        printf("?File spec required\n");
        return ( -2 );
      }

      return ( x );
    }

    nfils = -1;      /* Files come from internal list. */
    strcpy(line, s); /* Save copy of string just parsed. */
    debug(F101, "Send: wild", "", y);
    if (y == 0)
    {
      if (( x = cmtxt("Name to send it with", "", &cmarg2)) < 0)
      {
        return ( x );
      }
    }
    else if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    cmarg = line; /* File to send */
#ifdef DEBUG
    debug(F110, "Sending:", cmarg, 0);
    if (*cmarg2 != '\0')
    {
      debug(F110, " as:", cmarg2, 0);
    }

#endif /* ifdef DEBUG */
    sstate = 's'; /* Set start state */
    if (local)
    {
      displa = 1;
    }

    return ( 0 );
#ifndef NOSERVER
  case XXSER: /* Server */
    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    sstate = 'x';
    if (local)
    {
      displa = 1;
    }
    return ( 0 );
#endif
  case XXSET: /* Set */
    x = cmkey(prmtab, nprm, "Parameter", "");
    if (x == -3)
    {
      printf("?Parameter to set?\n");
      return ( -2 );
    }

    if (x < 0)
    {
      return ( x );
    }

    return ( doprm(x));

#ifndef NOPUSH

  /*
   * XXSHE code by H. Fischer;
   *   Copyright rights assigned to Columbia Univ
   *
   * Adapted to use getpwuid to find login shell because
   * many systems do not have SHELL in environment, and
   * to use direct calling of shell rather than
   * intermediate system() call.
   *                                     -- H. Fischer
   */

  case XXSHE:   /* Local shell command */
  {
    int pid;
    if (cmtxt("command to execute", "", &s) < 0)
    {
      return ( -1 );
    }

    conres();   /* Make console normal  */

#ifdef MSDOS
    zxcmd(s);
#else  /* ifdef MSDOS */
    if (( pid = fork()) == 0)     /* Make child */
    {
#ifdef __linux__
      const
#endif /* ifdef __linux__ */
      char *shpath,
      *shname, *shptr;     /* For finding desired shell */
      struct passwd *p;
      extern struct passwd *getpwuid();
#ifndef __linux__
      extern int getuid();
#endif /* ifndef __linux__ */
      char *defShel = "/bin/sh";     /* Default */

      p = getpwuid(getuid());     /* Get login data */
      if (p == (struct passwd *)NULL || !*( p->pw_shell ))
      {
        shpath = defShel;
      }
      else
      {
        shpath = p->pw_shell;
      }

      shptr = shname = shpath;
      while (*shptr != '\0')
      {
        if (*shptr++ == '/')
        {
          shname = shptr;
        }
      }

      /*
       * Remove following uid calls
       * if they cause trouble
       */

#ifdef BSD4
#ifndef BSD41
      setegid(getgid());         /* Override 4.3BSD csh security */
      seteuid(getuid());         /*  checks. */
#endif /* ifndef BSD41 */
#endif /* ifdef BSD4 */
      if (*s == NUL)                          /* Interactive sh requested */
      {
        execl(shpath, shname, "-i", NULL);    /* Yes, do that */
      }
      else                                    /* Otherwise, */
      {
        execl(shpath, shname, "-c", s, NULL); /* exec the given command */
      }

      exit(BAD_EXIT);
    }                                     /* Just punt if it didn't work */
    else                                  /* Parent */
    {
      int wstat;                          /* Kermit must wait for child */
      SIGTYP ( *istat )(), ( *qstat )();

      istat = signal(SIGINT, SIG_IGN);     /* Let the fork handle keyboard */
      qstat = signal(SIGQUIT, SIG_IGN);    /* interrupts itself... */

      while ((( wstat = wait((int *)0)) != pid ) && ( wstat != -1 ))
      {
        ;                               /* Wait for fork */
      }
      signal(SIGINT, istat);            /* Restore interrupts */
      signal(SIGQUIT, qstat);
    }

#endif /* ifdef MSDOS */
    concb(escape);                      /* Console back in cbreak mode */
    return ( 0 );
  }
#endif /* ifndef NOPUSH */

  case XXSHO:                           /* Show */
    x = cmkey(shotab, 2, "", "parameters");
    if (x < 0)
    {
      return ( x );
    }

    if (( y = cmcfm()) < 0)
    {
      return ( y );
    }

    switch (x)
    {
    case SHPAR:
      shopar();
      break;

    case SHVER:
      printf("\n  %s,%s\n", versio, ckxsys);
#ifdef WART
      printf("%s\n",    protv);
#endif /* ifdef WART */
      printf("%s\n",    fnsv);
      printf("%s\n",    cmdv);
#ifndef NOICP
      printf("%s\n",    userv);
#endif /* ifndef NOICP */
      printf("%s\n",    ckxv);
      printf("%s\n",    ckzv);
#ifndef NOCONN
      printf("%s\n",    connv);
#endif /* ifndef NOCONN */
#ifndef NOCKUDIA
      printf("%s\n",    dialv);
#endif /* ifndef NOCKUDIA */
#ifndef NOCKUSCR
      printf("%s\n",    loginv);
#endif /* ifndef NOCKUSCR */
      printf("\n");
      break;

    default:
      printf("\nNothing to show.\n");
      break;
    }
    return ( 0 );

#ifndef NOSPACE
  case XXSPA: /* space */
    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    system(SPACMD);
    return ( 0 );
#endif /* ifndef NOSPACE */

#ifndef NOSTATS
  case XXSTA:   /* statistics */
    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    return ( dostat());
#endif /* ifndef NOSTATS */

  case XXTAK: /* take */
    if (tlevel > MAXTAKE - 1)
    {
      printf("?Take files nested too deeply\n");
      return ( -2 );
    }

    if (( y = cmifi("command file", "", &s, &x)) < 0)
    {
      if (y == -3)
      {
        printf("?A file spec is required\n");
        return ( -2 );
      }
      else
      {
        return ( y );
      }
    }

    if (x != 0)
    {
      printf("?Wildcards not allowed\n");
      return ( -2 );
    }

    strcpy(line, s); /* Make a safe copy of the string */
    if (( y = cmcfm()) < 0)
    {
      return ( y );
    }

    if (( tfile[++tlevel] = fopen(line, "r")) == NULL)
    {
      perror(line);
      debug(F110, "Failure to open", line, 0);
      tlevel--;
    }

    return ( 0 );

  case XXTRA: /* transmit */
    if (( x = cmifi("File to transmit", "", &s, &y)) < 0)
    {
      if (x == -3)
      {
        printf("?Name of an existing file\n");
        return ( -2 );
      }

      return ( x );
    }

    if (y != 0)
    {
      printf("?Only one file may be transmitted\n");
      return ( -2 );
    }

    strcpy(line, s); /* Save copy of string just parsed. */
    y = cmnum(
      "ASCII value of turnaround char",
      "10",
      10,
      &x);
#ifdef DEBUG
    debug(F101, "transmit parse turnaround", "", x);
#endif /* ifdef DEBUG */
    if (y < 0)
    {
      return ( y );
    }

    if (x < 0 || x > 127)
    {
      printf("?0 to 127\n");
      return ( -2 );
    }

    if (( y = cmcfm()) < 0)
    {
      return ( y ); /*
                     * Confirm the command
                     *
                     *   if (!local) {
                     *       printf("?Transmit requires prior SET LINE\n");
                     *       return(-2);
                     *   }
                     */
    }
#ifdef DEBUG
    debug(F110, "calling transmit", line, 0);
#endif /* ifdef DEBUG */
    return ( transmit(line, x)); /* Do the command */

  default:
    printf("??? - %s\n", cmdbuf);
    return ( -2 );
  }
}
#endif /* ifndef NOICP */

/* D O C O N E C T -- Do the connect command */

/*
 * Note, we don't call this directly from dial,
 * because we need to give the user a chance to
 * change parameters (e.g. parity) after the
 * connection is made.
 */

#ifndef NOICP
int
doconect()
{
  int x = 0;

  conres();          /* Put console back to normal */
#ifndef NOCONN
  x = conect();      /* Connect */
#endif
  concb(escape);     /* Put console into cbreak mode, */
  return ( x );      /* for more command parsing. */
}
#endif /* ifndef NOICP */

/* T R A N S M I T -- Raw upload */

/*
 * Obey current line, duplex, parity,
 * flow, text/binary settings.
 *
 * Returns 0 upon apparent success,
 * 1 on obvious failure.
 *
 * Things to add:
 *  - Make both text and binary mode obey set file bytesize.
 *  - Maybe allow user to specify terminators other than CR?
 *  - Maybe allow user to specify prompts other than single characters?
 */
#ifndef NOICP
int tr_int;                             /* Flag if TRANSMIT interrupted */

#ifdef __linux__
void
#endif /* ifdef __linux__ */
trtrap()
{                                       /* TRANSMIT interrupt trap */
  tr_int = 1;
#ifdef __linux__
#endif /* ifdef __linux__ */
}

int
transmit(s, t)
#ifdef __linux__
const
#endif /* ifdef __linux__ */
char *s;
char t;
{
#ifndef MINBUF
#define LINBUFSIZ 150
#else /* ifndef MINBUF */
#define LINBUFSIZ 85
#endif /* ifndef MINBUF */
  char linbuf[LINBUFSIZ + 2];           /* Line buffer */

  SIGTYP (*oldsig)();                   /* For saving old interrupt trap. */
  int z = 0;                            /* Return code. */
  int x = 0;                            /* Workers... */
  int c = 0;                            /* XXX(jhj): x/c/i/n = 0 */
  int i = 0;
  int n = 0;
                                        /* CHAR tt; */
  (void)n;
  CHAR dopar(char t);                   /* Turnaround char, with parity */

#ifdef DEBUG
  debug(F101, "transmit turnaround", "", t);
#endif /* ifdef DEBUG */

  if (zopeni(ZIFILE, s) == 0)           /* Open file to be transmitted */
  {
    printf("?Can't open %s\n", s);
    return ( 1 );
  }

  x = -1;                               /* Open the communication line */
  if (ttopen(ttname, &x, mdmtyp) < 0)   /* (does no harm if already open) */
  {
    printf("Can't open %s\n", ttname);
    return ( 1 );
  }

  x = x ? speed : -1;                   /* Put the line in "packet mode" */
  if (ttpkt(x, flow, parity) < 0)
  {
    printf("Can't condition line\n");
    return ( 1 );
  }

  i = 0;                                /* Beginning of buffer. */
  oldsig = signal(SIGINT, trtrap);      /* Save current interrupt trap. */
  tr_int = 0;                           /* Have not been interrupted yet */
  z = 0;                                /* Return code presumed good. */

  while (( c = zminchar()) != -1)       /* Loop on all characters in file */
  {
    if (tr_int)                         /* Interrupted? */
    {
      fprintf(stderr, "^C...\n");       /* Print message */
      z = 1;
      break;
    }

    if (duplex)
    {
      conoc(c);                         /* Echo character on screen */
    }

    if (binary)                         /* If binary file */
    {
      if (ttoc(dopar(c)) < 0)           /* just try to send the character */
      {
        printf("?Can't transmit character\n");
        z = 1;
        break;
      }

      if (!duplex)
      {
        x = ttinc(1);                   /* Try to read back echo */
        if (x > -1)
        {
          conoc(x);
        }
      }
    }
    else                                /* Line at a time for text files */
    {
      if (c == '\n')                    /* Got a line */
      {
        if (linbuf[i - 1] != dopar('\r'))
        {
          linbuf[i++] = dopar('\r');    /* Terminate it with CR */
        }

        if (ttol(linbuf, i) < 0)        /* try to send it */
        {
          printf("?Can't transmit line\n");
          z = 1;
          break;
        }

        i = 0;                          /* Reset the buffer pointer */
        if (t)                          /* Got a turnaround character? */
        {
          x = 0; /* wait for it */
          while (( x != -1 ) && ( x != t ))
          {
            x = ttinc(1);
            if (!duplex)
            {
              conoc(x);                 /* also echo any echoes */
            }
          }
        }
      }
      else                              /* Not a NL, regular character */
      {
        linbuf[i++] = dopar(c);         /* Put it in line buffer. */
        if (i == LINBUFSIZ)             /* If buffer full, */
        {
          if (ttol(linbuf, i) < 0)      /* try to send it. */
          {
            printf("Can't send buffer\n");
            z = 1;
            break;
          }                             /* Don't wait for turnaround */

          i = 0;                        /* Reset buffer pointer */
        }
      }
    }
  }
  signal(SIGINT, oldsig);               /* put old signal action back. */
  ttres();                              /* Done, restore tty, */
  zclose(ZIFILE);                       /* close file, */
  return ( z );                         /* and return successfully. */
}
#endif /* ifndef NOICP */
