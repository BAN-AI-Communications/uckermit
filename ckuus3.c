/* C K U U S 3 -- "User Interface" */

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

/*
 * SET and REMOTE commands; screen, debug,
 * interrupt, and logging functions
 */

#include "ckcdeb.h"
#include "ckcker.h"
#include "ckucmd.h"
#include "ckuusr.h"
#include <ctype.h>
#include <stdio.h>

#ifdef UXIII
#include <termio.h>
#endif /* ifdef UXIII */

#ifdef __linux__
#include <string.h>
#endif /* ifdef __linux__ */

#ifndef NOICP
extern int size, spsiz, rpsiz, urpsiz, npad, timint,
  srvtim, rtimo, speed, local, lpcapr;

#ifndef NOSERVER
extern int server;
#endif /* ifndef NOSERVER */

#ifndef NOATTR
extern int atcapr;
#endif /* ifndef NOATTR */

extern int fmask, cmask, backgrd, flow, displa,
  binary, fncnv, delay, parity, deblog, escape,
  xargc, turn, duplex, cxseen, czseen, nfils,
  ckxech, pktlog, seslog, tralog, stdouf, turnch,
  bctr, bctu, dfloc, mdmtyp, keep, maxtry,
  rptflg, ebqflg, warn, quiet, cnflg, timef,
  spsizf, mypadn;

extern long filcnt, tlci, tlco, ffc, tfc, fsize;

extern char *versio, *protv, *ckxv, *ckzv, *fnsv, *connv, *dftty, *cmdv;
extern char *cmarg, *cmarg2, **xargv, **cmlist;
extern CHAR stchr, mystch, sstate, padch, mypadc, eol, seol, ctlq;
extern CHAR filnam[], ttname[];
char *strcpy();

extern char cmdbuf[];       /* Command buffer */

extern char \
  line[CMDBL + 10], *lp;    /* Character buffer for anything */
extern char debfil[50],     /* Debugging log file name */
  pktfil[50],               /* Packet log file name */
  sesfil[50],               /* Session log file name */
  trafil[50];               /* Transaction log file name */

extern int tlevel;          /* Take Command file level */
extern FILE *tfile[];       /* Array of take command fd's */

char coninc(int timo);

/*
 * Keyword tables
 * for SET commands
 */

/*
 * Block
 * checks
 */

struct keytab blktab[] = {
    { "1", 1, 0, },
    { "2", 2, 0, },
    { "3", 3, 0  }
};

/*
 * Duplex keyword
 * table
 */

struct keytab dpxtab[] = {
    { "full", 0, 0, },
    { "half", 1, 0  }
};

struct keytab filtab[] = {
    { "display", XYFILD, 0, },
    { "names",   XYFILN, 0, },
    { "type",    XYFILT, 0, },
    { "rename",  XYFILW, 0  }
};

int nfilp = ( sizeof ( filtab ) / sizeof ( struct keytab ));

/*
 * Send/Receive
 * Parameters
 */

struct keytab srtab[] = {
    { "end-of-packet",   XYEOL,  0, },
    { "packet-length",   XYLEN,  0, },
    { "pad-character",   XYPADC, 0, },
    { "padding",         XYNPAD, 0, },
    { "start-of-packet", XYMARK, 0, },
    { "timeout",         XYTIMO, 0  }
};

int nsrtab = ( sizeof ( srtab ) / sizeof ( struct keytab ));

/*
 * Flow
 * Control
 */

struct keytab flotab[] = {
    { "none",     0, 0, },
    { "xon/xoff", 1, 0  }
};

int nflo = ( sizeof ( flotab ) / sizeof ( struct keytab ));

/*
 * Handshake
 * characters
 */

struct keytab hshtab[] = {
    { "bell", 007, 0, },
    { "cr",   015, 0, },
    { "esc",  033, 0, },
    { "lf",   012, 0, },
    { "none", 999, 0, },
    { "xoff", 023, 0, },
    { "xon",  021, 0  }
};

int nhsh = ( sizeof ( hshtab ) / sizeof ( struct keytab ));

struct keytab fntab[] = { /* File naming */
    { "converted", 1, 0, },
    { "literal",   0, 0  }
};

struct keytab fttab[] = { /* File types */
    { "binary", 1, 0, },
    { "text",   0, 0  }
};

#ifndef NOCKUDIA
extern struct keytab mdmtab[];   /* Modem types (in module ckudia.c) */
extern int nmdm;
#endif /* ifndef NOCKUDIA */

/*
 * Parity keyword
 * table
 */

struct keytab partab[] = {
    { "even",  'e', 0, },
    { "mark",  'm', 0, },
    { "none",  0,   0, },
    { "odd",   'o', 0, },
    { "space", 's', 0  }
};

int npar = ( sizeof ( partab ) / sizeof ( struct keytab ));

/*
 * On/Off
 * table
 */

struct keytab onoff[] = {
    { "off", 0, 0, },
    { "on",  1, 0  }
};

/*
 * Incomplete File
 * Disposition table
 */

struct keytab ifdtab[] = {
    { "discard", 0, 0, },
    { "keep",    1, 0  }
};

/*
 * Terminal parameters
 * table
 */

struct keytab trmtab[] = {
    { "bytesize", 0, 0 }
};

/*
 * Server parameters
 * table
 */

struct keytab srvtab[] = {
    { "timeout", 0, 0 }
};

/* D O P R M -- Set a parameter */

/*
 *  Returns:
 *   -2: illegal input
 *   -1: reparse needed
 *   0: success
 */

int
doprm(xx)
int xx;
{
  int x = 0;
  int y = 0;
  int z = 0;               /* XXX(jhj): init y & z to 0 */
  char *s;

  switch (xx)
  {
  case  XYEOL: /* These have all been moved to set send/receive... */
  case  XYLEN: /* Let the user know what to do. */
  case XYMARK:
  case XYNPAD:
  case XYPADC:
  case XYTIMO:
    printf("Use 'set send' or 'set receive'.\n");
#ifndef NODOHLP
    printf("Type 'help set send' or 'help set receive' for more info.\n");
#endif /* ifndef NODOHLP */
    return ( 0 );

#ifndef NOATTR
  case XYATTR: /* File Attribute packets */
    return ( seton(&atcapr));
#endif /* ifndef NOATTR */

  case XYIFD: /* Incomplete file disposition */
    if (( y = cmkey(ifdtab, 2, "", "discard")) < 0)
    {
      return ( y );
    }

    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    keep = y;
    return ( 0 );

  case XYLINE:
    if (( x = cmtxt("Device name", dftty, &s)) < 0)
    {
      return ( x );
    }

    ttclos(); /* close old line, if any was open */

    x = strcmp(s, dftty) ? -1 : dfloc; /* Maybe let ttopen figure it out */
    if (ttopen(s, &x, mdmtyp) < 0)     /* Can we open the new line? */
    {
      perror("Can't open line");
      return ( -2 ); /* If not, give bad return */
    }

    if (x > -1)
    {
      local = x;       /* Set local/remote status. */
    }

    strcpy(ttname, s); /* OK, copy name into real place. */
    if (!local)
    {
      speed = -1; /* If remote, say speed unknown. */
    }

    debug(F111, "set line ", ttname, local);
    return ( 0 );

  case XYCHKT:
    if (( y = cmkey(blktab, 3, "", "1")) < 0)
    {
      return ( y );
    }

    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    bctr = y;
    return ( 0 );

#ifndef NOLOGS
#ifdef DEBUG
  case XYDEBU:
    return ( seton(&deblog));
#endif /* ifdef DEBUG */
#endif /* ifndef NOLOGS */

  case XYDELA:
    y = cmnum("Seconds to delay send", "5", 10, &x);
    debug(F101, "XYDELA: y", "", y);
    return ( setnum(&delay, x, y, 94));

  case XYDUPL:
    if (( y = cmkey(dpxtab, 2, "", "full")) < 0)
    {
      return ( y );
    }

    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    duplex = y;
    return ( 0 );

  case XYESC:
    y = cmnum("ASCII code for escape character", "", 10, &x);
    return ( setcc(&escape, x, y));

  case XYFILE:
    if (( y = cmkey(filtab, nfilp, "File parameter", "")) < 0)
    {
      return ( y );
    }

    switch (y)
    {
    case XYFILD: /* Display */
      y = seton(&z);
      if (y < 0)
      {
        return ( y );
      }

      quiet = !z;
      return ( 0 );

    case XYFILN: /* Names */
      if (( x =
              cmkey(fntab, 2, "how to handle filenames", "converted")) < 0)
      {
        return ( x );
      }

      if (( z = cmcfm()) < 0)
      {
        return ( z );
      }

      fncnv = x;
      return ( 0 );

    case XYFILT: /* Type */
      if (( x = cmkey(fttab, 2, "type of file", "text")) < 0)
      {
        return ( x );
      }

      if (( y = cmnum("file byte size", "8", 10, &z)) < 0)
      {
        return ( y );
      }

      if (z != 7 && z != 8)
      {
        printf("\n?7 or 8\n");
        return ( -2 );
      }

      if (( y = cmcfm()) < 0)
      {
        return ( y );
      }

      binary = x;
      if (z == 7)
      {
        fmask = 0177;
      }
      else if (z == 8)
      {
        fmask = 0377;
      }

      return ( 0 );

    case XYFILW: /* Rename */
      return ( seton(&warn));

    default:
      printf("?unexpected parameter\n");
      return ( -2 );
    }

  case XYFLOW: /* Flow control */
    if (( y = cmkey(flotab, nflo, "", "xon/xoff")) < 0)
    {
      return ( y );
    }

    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    flow = y;
    return ( 0 );

  case XYHAND: /* Handshake */
    if (( y = cmkey(hshtab, nhsh, "", "none")) < 0)
    {
      return ( y );
    }

    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    turn = ( y > 0127 ) ? 0 : 1;
    turnch = y;
    return ( 0 );

  case XYMODM:
#ifndef NOCKUDIA
    if (( x = cmkey(
            mdmtab,
            nmdm,
            "type of modem, or direct",
            "direct")) < 0)
    {
      return ( x );
    }

#endif /* ifndef NOCKUDIA */
    if (( z = cmcfm()) < 0)
    {
      return ( z );
    }

    mdmtyp = x;
    return ( 0 );

  case XYPARI: /* Parity */
    if (( y = cmkey(partab, npar, "", "none")) < 0)
    {
      return ( y );
    }

    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    /*
     * If parity not none, then
     * we also want 8th-bit prefixing
     */

    if (( parity = y ))
    {
      ebqflg = 1;
    }
    else
    {
      ebqflg = 0;
    }

    return ( 0 );

  case XYPROM:
    if (( x = cmtxt("local prompt", "uCKermit>", &s)) < 0)
    {
      return ( x );
    }

    if (*s == '\42')         /* Quoted string? */
    {
      x = strlen(s) - 1;     /* Yes, strip quotes. */
      if (*( s + x ) == '\42') /* This allows leading or trailing */
      {
        *( s + x ) = '\0';     /* blanks. */
      }

      s++;
    }

    cmsetp(s);
    return ( 0 );

  case XYRETR: /* Per-packet retry limit */
    y = cmnum("retries per packet", "10", 10, &x);
    return ( setnum(&maxtry, x, y, 94));

  case XYSERV: /* Server timeout */
    if (( y = cmkey(srvtab, 1, "", "timeout")) < 0)
    {
      return ( y );
    }

    switch (y)
    {
      char tmp[20];

    case XYSERT:
      sprintf(tmp, "%d", DSRVTIM);
      if (( y = cmnum(
              "interval for server NAKs, 0 = none",
              tmp,
              10,
              &x)) < 0)
      {
        return ( y );
      }

      if (x < 0)
      {
        printf("\n?Positive number, 0 to disable.\n");
        return ( -2 );
      }

      if (( y = cmcfm()) < 0)
      {
        return ( y );
      }

      srvtim = x; /* Set the server timeout variable */
      return ( y );

    default:
      return ( -2 );
    }

  case XYTERM: /* Terminal parameters */
    if (( y = cmkey(trmtab, 1, "", "bytesize")) < 0)
    {
      return ( y );
    }

    switch (y)
    {
    case 0:
      if (( y = cmnum("bytesize for terminal", "8", 10, &x)) < 0)
      {
        return ( y );
      }

      if (x != 7 && x != 8)
      {
        printf("\n?Only 7 and 8\n");
        return ( -2 );
      }

      if (( y = cmcfm()) < 0)
      {
        return ( y );
      }

      if (x == 7)
      {
        cmask = 0177;
      }
      else if (x == 8)
      {
        cmask = 0377;
      }

      return ( y );

    default: /* Add more cases when we think of more parameters */
      return ( -2 );
    }

  /*
   * SET
   * SEND/RECEIVE
   */

  case XYRECV:
  case XYSEND:
    if (xx == XYRECV)
    {
      strcpy(line, "Parameter for inbound packets");
    }
    else
    {
      strcpy(line, "Parameter for outbound packets");
    }

    if (( y = cmkey(srtab, nsrtab, line, "")) < 0)
    {
      return ( y );
    }

    switch (y)
    {
    case XYEOL:
      y = cmnum("ASCII code for terminator", "13", 10, &x);
      if (( y = setcc(&z, x, y)) < 0)
      {
        return ( y );
      }

      if (xx == XYRECV)
      {
        eol = z;
      }
      else
      {
        seol = z;
      }

      return ( y );

    case XYLEN:
      y = cmnum("Max chars in packet", "90", 10, &x);
      if (xx == XYRECV)   /* Receive... */
      {
        if (( y = setnum(&z, x, y, MAXRP)) < 0)
        {
          return ( y );
        }

        urpsiz = z;
        rpsiz = ( z > 94 ) ? 94 : z;
      }
      else     /* Send... */
      {
        if (( y = setnum(&z, x, y, MAXSP)) < 0)
        {
          return ( y );
        }

        spsiz = z;  /*   Set it and flag that it was set */
        spsizf = 1; /*   to allow overriding Send-Init. */
      }

      return ( y );

    case XYMARK:
      y = cmnum(
        "ASCII code for start character",
        "1",
        10,
        &x);
      if (( y = setcc(&z, x, y)) < 0)
      {
        return ( y );
      }

      if (xx == XYRECV)
      {
        stchr = z;
      }
      else
      {
        mystch = z;
      }

      return ( y );

    case XYNPAD: /* Padding */
      y = cmnum(
        "Padding characters for inbound packets",
        "0",
        10,
        &x);
      if (( y = setnum(&z, x, y, 94)) < 0)
      {
        return ( y );
      }

      if (xx == XYRECV)
      {
        mypadn = z;
      }
      else
      {
        npad = z;
      }

      return ( y );

    case XYPADC: /* Pad character */
      y = cmnum(
        "ASCII code for pad character",
        "0",
        10,
        &x);
      if (( y = setcc(&z, x, y)) < 0)
      {
        return ( y );
      }

      if (xx == XYRECV)
      {
        mypadc = z;
      }
      else
      {
        padch = z;
      }

      return ( y );

    case XYTIMO:
      y = cmnum("Interpacket timeout", "5", 10, &x);
      if (( y = setnum(&z, x, y, 94)) < 0)
      {
        return ( y );
      }

      if (xx == XYRECV)
      {
        timef = 1;
        timint = z;
      }
      else
      {
        rtimo = z;
      }

      return ( y );
    }

  case XYSPEE:
    if (!local)
    {
      printf("\nSpeed setting only for external lines,\n");
        printf("You must 'set line' first\n");
      return ( 0 );
    }

    lp = line;
    sprintf(lp, "Baud for %s", ttname);
    if (( y = cmnum(line, "", 10, &x)) < 0)
    {
      return ( y );
    }

    if (( y = cmcfm()) < 0)
    {
      return ( y );
    }

    y = chkspd(x);
    if (y < 0)
    {
#ifndef NODOHLP
      printf("?Unsupported line speed - %d\n", x);
#else /* ifndef NODOHLP */
          printf("?Bad speed - %d\n", x);
#endif /* ifndef NODOHLP */
        }
    else
    {
      speed = y;
      if (!backgrd)
      {
        printf("%s, %d baud\n", ttname, speed);
      }
    }

    return ( 0 );

  default:
    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    printf("Not working yet - %s\n", cmdbuf);
    return ( 0 );
  }
}
#endif /* ifndef NOICP */

/* C H K S P D -- Check if argument is a valid baud rate */

int
chkspd(x)
int x;
{                       /* XXX(jhj): Needs fixin! */
  switch (x)
  {
  case 0:
  case 110:
  case 150:
  case 300:
  case 600:
  case 1200:
  case 1800:
  case 2400:
  case 4800:
  case 9600:
  case 19200:
#ifdef __linux__        /* XXX(jhj): nonononono */
  case 38400:
  case 57600:
  case 115200:
  case 230400:
  case 460800:
  case 921600:
  case 1000000:
  case 1152000:
  case 1500000:
  case 2000000:
  case 2500000:
  case 3000000:
  case 3500000:
  case 4000000:
#endif /* ifdef __linux__ */
    return ( x );

  default:
    return ( -1 );
  }
}

#ifndef NOICP
/* S E T O N -- Parse on/off (default on), set parameter to result */

int
seton(prm)
int *prm;
{
  int x, y;
  if (( y = cmkey(onoff, 2, "", "on")) < 0)
  {
    return ( y );
  }

  if (( x = cmcfm()) < 0)
  {
    return ( x );
  }

  *prm = y;
  return ( 0 );
}

/* S E T N U M -- Set parameter to result of cmnum() parse */

/*
 * Call with
 * x - number from cnum parse,
 * y - return code from cmnum
 */

int
setnum(prm, x, y, max)
int x, y, *prm, max;
{
  debug(F101, "setnum", "", y);
  if (y < 0)
  {
    return ( y );
  }

  if (x > max)
  {
    printf("\n?%d max\n", max);
    return ( -2 );
  }

  if (( y = cmcfm()) < 0)
  {
    return ( y );
  }

  *prm = x;
  return ( 0 );
}

/* S E T C C -- Set parameter to an ASCII control character value */

int
setcc(prm, x, y)
int x, y, *prm;
{
  if (y < 0)
  {
    return ( y );
  }

  if (( x > 037 ) && ( x != 0177 ))
  {
    printf("\n?Not a control char - %d\n", x);
    return ( -2 );
  }

  if (( y = cmcfm()) < 0)
  {
    return ( y );
  }

  *prm = x;
  return ( 0 );
}

/* D O R M T -- Do a remote command */

int
dormt(xx)
int xx;
{
  int x;
  char *s, sbuf[50], *s2;

  if (xx < 0)
  {
    return ( xx );
  }

  switch (xx)
  {
  case XZCWD: /* CWD */
    if (( x = cmtxt("Remote dir", "", &s)) < 0)
    {
      return ( x );
    }

    debug(F111, "XZCWD: ", s, x);
    *sbuf = NUL;
    s2 = sbuf;
    if (*s != NUL)       /* If directory name given, */
                         /* get password on separate line. */
    {
      if (tlevel > -1)   /* From take file... */
      {
        if (fgets(sbuf, 50, tfile[tlevel]) == NULL)
        {
          fatal("take file ended in 'remote cwd'");
        }
#ifdef DEBUG
        debug(F110, " pswd from take file", s2, 0);
#endif /* ifdef DEBUG */
        for (x = strlen(sbuf);
             x > 0 && ( sbuf[x - 1] == NL || sbuf[x - 1] == CR ); x--)
        {
          sbuf[x - 1] = '\0';
        }
      }
      else     /* From terminal... */
      {
        printf(" Password: "); /* get a password */
        while ((( x = getchar()) != NL ) && ( x != CR )) /* with no echo */
        {
#ifdef COMMENT
          if (( x &= 0177 ) == '?')
          {
            printf("? Password\n Password: ");
            s2 = sbuf;
            *sbuf = NUL;
          }
#endif /* ifdef COMMENT */
          if (x == ESC)   /* Mini command line editor... */
          {
            /* putchar(BEL); */
          }
          else if (x == BS || x == 0177)
          {
            s2--;
          }
          else if (x == 025)   /* Ctrl-U */
          {
            s2 = sbuf;
            *sbuf = NUL;
          }
          else
          {
            *s2++ = x;
          }
        }
        *s2 = NUL;
        putchar('\n');
      }

      s2 = sbuf;
    }
    else
    {
      s2 = "";
    }

    debug(F110, " password", s2, 0);
    sstate = setgen('C', s, s2, "");
    return ( 0 );

  case XZDEL: /* Delete */
    if (( x = cmtxt("Remote file(s) to delete", "", &s)) < 0)
    {
      return ( x );
    }

    return ( sstate = rfilop(s, 'E'));

  case XZDIR: /* Directory */
    if (( x = cmtxt("Remote dir or file spec", "", &s)) < 0)
    {
      return ( x );
    }

    return ( sstate = setgen('D', s, "", ""));

  case XZHLP: /* Help */
    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    sstate = setgen('H', "", "", "");
    return ( 0 );

  case XZHOS: /* Host */
    if (( x = cmtxt("Command for remote", "", &cmarg)) < 0)
    {
      return ( x );
    }

    return ( sstate = 'c' );

  case XZPRI: /* Print */
    if (( x = cmtxt(
            "Remote file(s) to print on remote",
            "",
            &s)) < 0)
    {
      return ( x );
    }

    return ( sstate = rfilop(s, 'S'));

  case XZSPA: /* Space */
    if (( x = cmtxt("Confirm, or remote dir name", "", &s)) < 0)
    {
      return ( x );
    }

    return ( sstate = setgen('U', s, "", ""));

  case XZTYP: /* Type */
    if (( x = cmtxt("Remote file spec", "", &s)) < 0)
    {
      return ( x );
    }

    return ( sstate = rfilop(s, 'T'));

  case XZWHO:
    if (( x = cmtxt("User name, or return", "", &s)) < 0)
    {
      return ( x );
    }

    return ( sstate = setgen('W', s, "", ""));

  default:
    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    printf("Not working yet - %s\n", cmdbuf);
    return ( -2 );
  }
}

/* R F I L O P -- Remote File Operation */

int
rfilop(s, t)
char *s, t;
{
  if (*s == NUL)
  {
    printf("?File spec required\n");
    return ( -2 );
  }

  debug(F111, "rfilop", s, t);
  return ( setgen(t, s, "", ""));
}
#endif /* ifndef NOICP */

/* S C R E E N -- Screen display function */

/*
 *  screen(f,c,n,s)
 *     f - argument descriptor
 *     c - a character or small integer
 *     n - a long integer
 *     s - a string.
 *  Fill in this routine with the appropriate display update for the system.
 *  This version is for a dumb tty.
 */

void
screen(f, c, n, s)
int f;
long n;
char c;
char *s;
{
  static int p = 0;                   /* Screen position */
  int len;                            /* Length of string */
  char buf[80];                       /* Output buffer */
  len = strlen(s);                    /* Length of string */
  if (( f != SCR_WM ) && \
    ( f != SCR_EM ))                  /* Always update warning & errors */
  {
#ifndef NOICP
    if (!displa || quiet)
    {
      return;                         /* No update if display flag off */
    }
#endif /* ifndef NOICP */
  }

  switch (f)
  {
  case SCR_FN: /* filename */
    conoll("");
    conol(s);
    conoc(SP);
    p = len + 1;
    return;

  case SCR_AN: /* as-name */
    if (p + len > 75)
    {
      conoll("");
      p = 0;
    }

    conol("=> ");
    conol(s);
    if (( p += ( len + 3 )) > 78)
    {
      conoll("");
      p = 0;
    }

    return;

  case SCR_FS: /* file-size */
    sprintf(buf, ", Size: %ld", n);
    conoll(buf);
    p = 0;
    return;

  case SCR_XD: /* x-packet data */
    conoll("");
    conoll(s);
    p = 0;
    return;

  case SCR_ST: /* File status */
    switch (c)
    {
    case ST_OK: /*  Transferred OK */
      if (( p += 5 ) > 78)
      {
        conoll("");
        p = 0;
      }

      conoll(" [OK]");
      p += 5;
      return;

    case ST_DISC: /*  Discarded */
      if (( p += 12 ) > 78)
      {
        conoll("");
        p = 0;
      }

      conoll(" [discarded]");
      p += 12;
      return;

    case ST_INT: /*  Interrupted */
      if (( p += 14 ) > 78)
      {
        conoll("");
        p = 0;
      }

      conoll(" [interrupted]");
      p += 14;
      return;

    case ST_SKIP: /*  Skipped */
      conoll("");
      conol("Skipping ");
      conoll(s);
      p = 0;
      return;

    case ST_ERR:
      conoll("");
      conol("Error ");
      conoll(s);
      p = 0;
      return;

    default:
#ifndef NODOHLP
      conoll("*** screen() called with bad status ***");
#endif /* ifndef NODOHLP */
      p = 0;
      return;
    }

  case SCR_PN: /* Packet number */
    sprintf(buf, "%s: %ld", s, n);
    conol(buf);
    p += strlen(buf);
    return;

  case SCR_PT: /* Packet type or pseudotype */
    if (c == 'Y')
    {
      return;       /* Don't bother with ACKs */
    }

    if (c == 'D')   /* Only show every 4th data packet */
    {
      if (n % 4)
      {
        return;
      }

      c = '.';
    }

    if (p++ > 78)   /* If near right margin, */
    {
      conoll("");   /* Start new line */
      p = 0;        /* and reset counter. */
    }

    conoc(c); /* Display the character. */
    return;

  case SCR_TC: /* transaction complete */
    /* conoc(BEL); */
    return;

  case SCR_EM: /* Error message */
    conoll("");
    conoc('?');
    conoll(s);
    p = 0;
    return;    /* +1 */

  case SCR_WM: /* Warning message */
    conoll("");
    conoll(s);
    p = 0;
    return;

  case SCR_TU: /* Undelimited text */
    if (( p += len ) > 78)
    {
      conoll("");
      p = len;
    }

    conol(s);
    return;

  case SCR_TN: /* Text delimited at beginning */
    conoll("");
    conol(s);
    p = len;
    return;

  case SCR_TZ: /* Text delimited at end */
    if (( p += len ) > 78)
    {
      conoll("");
      p = len;
    }

    conoll(s);
    return;

  case SCR_QE: /* Quantity equals */
    sprintf(buf, "%s: %ld", s, n);
    conoll(buf);
    p = 0;
    return;

  default:
#ifndef NODOHLP
    conoll("*** screen() called with bad object ***");
#endif /* ifndef NODOHLP */
    p = 0;
    return;
  }
}

/* I N T M S G -- Issue message about terminal interrupts */

#ifndef NOICP
void
intmsg(n)
long n;
{
  extern char *chstr();
  char buf[80];

  if (( !displa ) || ( quiet ))
  {
    return;
  }

#ifdef UXIII
  (void)conchk();   /* clear out pending escape-signals in ckxbsd.c */
#endif /* ifdef UXIII */
  if (n == 1)
  {
#ifdef UXIII

    /*
     * We need to signal
     * before kb input
     */

    sprintf(buf, "Escape character (%s) then:", chstr(escape));
    screen(SCR_TN, 0, 0l, buf);
#endif /* ifdef UXIII */
    screen(
      SCR_TN,
      0,
      0l,
      "CTRL-F to cancel file,  CTRL-R to resend packet");
    screen(
      SCR_TN,
      0,
      0l,
      "CTRL-B to cancel batch, CTRL-A for status report: ");
  }
  else
  {
    screen(SCR_TU, 0, 0l, " ");
  }
}

/* C H K I N T -- Check for console interrupts */

/*
 * Should rework not to
 * destroy typeahead
 */

int
chkint()
{
  int ch, cn;

  if (( !local ) || ( quiet ))
  {
    return ( 0 ); /* Only do this if local & not quiet */
  }

  cn = conchk(); /* Any input waiting? */
  debug(F101, "conchk", "", cn);

  while (cn > 0)   /* Yes, read it. */
  {
    cn--;

    /*
     * Give read 5 seconds
     * for interrupt character
     */

    if (( ch = coninc(5)) < 0)
    {
      return ( 0 );
    }

    switch (ch & 0177)
    {
    case 0001: /* CTRL-A */
      screen(SCR_TN, 0, 0l, "^A  Status report:");
      screen(SCR_TN, 0, 0l, " file type: ");
      if (binary)
      {
        screen(SCR_TZ, 0, 0l, "binary");
      }
      else
      {
        screen(SCR_TZ, 0, 0l, "text");
      }

      screen(SCR_QE, 0, (long)filcnt, " file number");
      screen(SCR_QE, 0, (long)ffc, " characters ");
      screen(SCR_QE, 0, (long)bctu, " block check");
      screen(SCR_QE, 0, (long)rptflg, " compression");
      screen(SCR_QE, 0, (long)ebqflg, " 8th-bit prefixing");
      continue;

    case 0002: /* CTRL-B */
      screen(SCR_TN, 0, 0l, "^B - Cancelling Batch ");
      czseen = 1;
      continue;

    case 0006: /* CTRL-F */
      screen(SCR_TN, 0, 0l, "^F - Cancelling File ");
      cxseen = 1;
      continue;

    case 0022: /* CTRL-R */
      screen(SCR_TN, 0, 0l, "^R - Resending ");
      resend();
      return ( 1 );

    default: /* Anything else, just ignore */
      screen(SCR_TU, 0, 0l, " [Ignored] ");
      continue;
    }
  }
  return ( 0 );
}

/* D E B U G -- Enter a record in the debugging log */

/*
 *  Call with a format, two strings, and a number:
 *  f  - Format, a bit string in range 0-7.
 *       If bit x is on, then argument number x is printed.
 *  s1 - String, argument number 1.  If selected, printed as is.
 *  s2 - String, argument number 2.  If selected, printed in brackets.
 *  n  - Int, argument 3.  If selected, printed preceded by equals sign.
 *
 *  f=0 is special: print s1,s2, and interpret n as a char.
 */

#ifndef NOLOGS
#ifdef DEBUG
#define DBUFL 1200
int
debug(f, s1, s2, n)
int f, n;
char *s1, *s2;
{
  static char s[DBUFL];
  char *sp = s;

  if (!deblog)
  {
    return ( 0 );   /* If no debug log, don't */
  }

  switch (f)
  {
  case F000:   /* 0, print both strings, */
    if (strlen(s1) + strlen(s2) + 3 > DBUFL)
    {
      sprintf(sp, "DEBUG string too long\n");
    }
    else
    {
      sprintf(sp, "%s%s%c\n", s1, s2, n);   /* interpret n as a char */
    }

    zsout(ZDFILE, s);
    break;

  case F001:   /* 1, "=n" */
    sprintf(sp, "=%d\n", n);
    zsout(ZDFILE, s);
    break;

  case F010:   /* 2, "[s2]" */
    if (strlen(s2) + 4 > DBUFL)
    {
      sprintf(sp, "DEBUG string too long\n");
    }
    else
    {
      sprintf(sp, "[%s]\n", s2);
    }

    zsout(ZDFILE, "");
    break;

  case F011:   /* 3, "[s2]=n" */
    if (strlen(s2) + 15 > DBUFL)
    {
      sprintf(sp, "DEBUG string too long\n");
    }
    else
    {
      sprintf(sp, "[%s]=%d\n", s2, n);
    }

    zsout(ZDFILE, s);
    break;

  case F100:   /* 4, "s1" */
    zsoutl(ZDFILE, s1);
    break;

  case F101:   /* 5, "s1=n" */
    if (strlen(s1) + 15 > DBUFL)
    {
      sprintf(sp, "DEBUG string too long\n");
    }
    else
    {
      sprintf(sp, "%s=%d\n", s1, n);
    }

    zsout(ZDFILE, s);
    break;

  case F110:   /* 6, "s1[s2]" */
    if (strlen(s1) + strlen(s2) + 4 > DBUFL)
    {
      sprintf(sp, "DEBUG string too long\n");
    }
    else
    {
      sprintf(sp, "%s[%s]\n", s1, s2);
    }

    zsout(ZDFILE, s);
    break;

  case F111:   /* 7, "s1[s2]=n" */
    if (strlen(s1) + strlen(s2) + 15 > DBUFL)
    {
      sprintf(sp, "DEBUG string too long\n");
    }
    else
    {
      sprintf(sp, "%s[%s]=%d\n", s1, s2, n);
    }

    zsout(ZDFILE, s);
    break;

  default:
    sprintf(sp, "\n?Invalid format for debug() - %d\n", n);
    zsout(ZDFILE, s);
  }
return ( 0 );
}
#endif /* ifdef DEBUG */
#endif /* ifndef NOLOGS */

#ifdef TLOG
#ifndef NOLOGS
#define TBUFL 300
#endif /* ifndef NOLOGS */

/* T L O G -- Log a record in the transaction file */

/*
 *  Call with a format and 3 arguments: two strings and a number:
 *  f  - Format, a bit string in range 0-7, bit x is on, arg #x is printed.
 *  s1,s2 - String arguments 1 and 2.
 *  n  - Int, argument 3.
 */

#ifndef NOLOGS
void
tlog(f, s1, s2, n)
int f;
long n;
char *s1, *s2;
{
  static char s[TBUFL];
  char *sp = s;
  int x;

  if (!tralog)
  {
    return;   /* If no transaction log, don't */
  }

  switch (f)
  {
  case F000:   /* 0 (special) "s1 n s2"  */
    if (strlen(s1) + strlen(s2) + 15 > TBUFL)
    {
      sprintf(sp, "?T-Log string too long\n");
    }
    else
    {
      sprintf(sp, "%s %ld %s\n", s1, n, s2);
    }

    zsout(ZTFILE, s);
    break;

  case F001:   /* 1, " n" */
    sprintf(sp, " %ld\n", n);
    zsout(ZTFILE, s);
    break;

  case F010:   /* 2, "[s2]" */
    x = strlen(s2);
    if (s2[x] == '\n')
    {
      s2[x] = '\0';
    }

    if (x + 6 > TBUFL)
    {
      sprintf(sp, "?T-Log string too long\n");
    }
    else
    {
      sprintf(sp, "[%s]\n", s2);
    }

    zsout(ZTFILE, "");
    break;

  case F011:   /* 3, "[s2] n" */
    x = strlen(s2);
    if (s2[x] == '\n')
    {
      s2[x] = '\0';
    }

    if (x + 6 > TBUFL)
    {
      sprintf(sp, "?T-Log string too long\n");
    }
    else
    {
      sprintf(sp, "[%s] %ld\n", s2, n);
    }

    zsout(ZTFILE, s);
    break;

  case F100:   /* 4, "s1" */
    zsoutl(ZTFILE, s1);
    break;

  case F101:   /* 5, "s1: n" */
    if (strlen(s1) + 15 > TBUFL)
    {
      sprintf(sp, "?T-Log string too long\n");
    }
    else
    {
      sprintf(sp, "%s: %ld\n", s1, n);
    }

    zsout(ZTFILE, s);
    break;

  case F110:   /* 6, "s1 s2" */
    x = strlen(s2);
    if (s2[x] == '\n')
    {
      s2[x] = '\0';
    }

    if (strlen(s1) + x + 4 > TBUFL)
    {
      sprintf(sp, "?T-Log string too long\n");
    }
    else
    {
      sprintf(sp, "%s %s\n", s1, s2);
    }

    zsout(ZTFILE, s);
    break;

  case F111:   /* 7, "s1 s2: n" */
    x = strlen(s2);
    if (s2[x] == '\n')
    {
      s2[x] = '\0';
    }

    if (strlen(s1) + x + 15 > TBUFL)
    {
      sprintf(sp, "?T-Log string too long\n");
    }
    else
    {
      sprintf(sp, "%s %s: %ld\n", s1, s2, n);
    }

    zsout(ZTFILE, s);
    break;

  default:
    sprintf(sp, "\n?Invalid format for tlog() - %ld\n", n);
    zsout(ZTFILE, s);
  }
}
#endif /* ifdef TLOG */
#endif /* ifndef NOLOGS */
#endif /* ifndef NOICP */
