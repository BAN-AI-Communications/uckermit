#ifndef NOICP
char *fnsv = "   Library, 4G(106)";
#endif /* ifndef NOICP */

/* C K C F N S -- System-independent Kermit protocol support functions */

/*
 * Copyright (C) 2021, 2022, Jeffrey H. Johnson <trnsz@pobox.com>
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

#include "ckcdeb.h"       /* Debug formats, typedefs, etc. */
#include "ckcker.h"       /* Symbol definitions for Kermit */

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#endif /* ifdef __linux__ */

#ifndef NULL
#define NULL 0
#endif /* ifndef NULL */

extern int  spsiz,    spmax,     rpsiz,   timint;
extern int  srvtim,   rtimo,     npad,    ebq;
extern int  ebqflg,   rpt,       rptq,    rptflg;
extern int  capas,    keep,      pktnum,  prvpkt;
extern int  sndtyp,   bctr,      bctu,    fmask;
extern int  size,     maxsize,   spktl,   nfils;
extern int  stdouf,   warn,      timef,   spsizf;
extern int  parity,   speed,     turn,    turnch;
extern int  delay,    displa,    pktlog,  tralog;
extern int  seslog,   xflg,      mypadn,  tsecs;
extern int  deblog,   hcflg,     binary,  savmod;
extern int  fncnv,    local;

#ifndef NOSERVER
extern int  server;
#endif /* ifndef NOSERVER */

extern int  cxseen;
extern int  czseen,   nakstate,  rq,      rqf;
extern int  sq,       wslots,    urpsiz,  rln;

#ifndef NOATTR
extern int  atcapr;
extern int  atcapu;
#endif /* ifndef NOATTR */

extern int  atcapb,   lpcapr;
extern int  lpcapb,   lpcapu,    swcapr,  swcapb;
extern int  swcapu,   bsave,     bsavef,  numerrs;

extern long filcnt,   ffc,       fsize;

#ifndef NOSTATS
extern long flci,     flco,      tlci;
extern long tlco,     tfc;
#endif /* ifndef NOSTATS */

extern CHAR padch,    mypadc,    eol,       seol;
extern CHAR *srvptr,  ctlq,      myctlq,    sstate;
extern CHAR filnam[], sndpkt[],  recpkt[],  data[];
extern CHAR srvcmd[], stchr,     mystch;

extern char *cmarg,   *cmarg2,   *hlptxt;
extern char **cmlist, *rdatap;

void spar();
void sipkt();
long zchki();
char *strcpy();
CHAR *rpar();

void errpkt(char *reason);

extern void errpkt();

extern CHAR zinbuffer[], zoutbuffer[];
extern CHAR *zinptr,     *zoutptr;

extern int  zincnt,      zoutcnt;

/*
 * Variables local
 * to this module
 */

static char *memptr;         /* Pointer for memory strings */

static char cmdstr[100];     /* UNIX system command string */

static int  sndsrc;          /* Flag for where to send from: */
                             /*   -1: name in cmdata, */
                             /*    0: stdin, */
                             /*   >0: list in cmlist. */

static int  n_len;           /* Packet encode-ahead length and flag */
                             /* if < 0, no pre-encoded data. */

static int  memstr;          /* Flag for input from memory string */
static int  first;           /* Flag for first char from input */

static CHAR t;               /* Current character */
static CHAR next;            /* Next character */

/* E N C S T R -- Encode a string from memory. */

/*
 * Call this instead of getpkt() if source
 * is a string, rather than a file.
 */

void
encstr(s)
char *s;
{
  int m;
  char *p;

  m = memstr;
  p = memptr;                 /* Save these. */

  memptr = s;                 /* Point to the string. */
  memstr = 1;                 /* Flag memory string as source. */
  first  = 1;                 /* Initialize character lookahead. */

  getpkt(
    spsiz - bctu - 3);        /* Fill a packet from the string. */

  memstr = m;                 /* Restore memory string flag */
  memptr = p;                 /* and pointer */
  first  = 1;                 /* Put this back as we found it. */
}

/*
 * Output functions
 * passed to 'decode'
 */

int
putsrv(c)
register char c;
{                             /* Put character in server command buffer */
  *srvptr++ = c;
  *srvptr = '\0';             /* Make sure buffer is null-terminated */
  return ( 0 );
}

int
puttrm(c)
register char c;
{                             /* Output character to console */
  conoc(c);
  return ( 0 );
}

int
putfil(c)
register char c;
{                             /* Output char to file */
  if (zchout(
    ZOFILE, c & fmask) < 0)
  {
    czseen = 1;               /* If write error... */
    debug(F101,
      "putfil zchout write error, setting czseen", "", 1);
    return ( -1 );
  }

  return ( 0 );
}

/* D E C O D E -- Kermit packet decoding procedure */

/*
 * Call with string to be decoded,
 * and an output function. Returns
 * 0 on success, -1 on failure
 * (e.g. disk full)
 */

int
decode(buf, fn)
register CHAR *buf;
register int (*fn)();
{
  register unsigned int a, a7, b8;   /* Low order 7 bits, and the 8th bit */
  int x = 0;

  rpt = 0;                           /* Initialize repeat count. */

  while (( a = *buf++ ) != '\0')
  {
    if (rptflg)                      /* Repeat processing? */
    {
      if (
        (unsigned int)a == \
        (unsigned int)rptq)
      {                              /* Yes, got a repeat prefix? */
        rpt = xunchar(*buf++);       /* Yes, get the repeat count, */
        a = *buf++;                  /* and get the prefixed character. */
      }
    }

    b8 = 0;                          /* Check high order "8th" bit */
    if (ebqflg)                      /* 8th-bit prefixing? */
    {
      if (
        (unsigned int)a == \
        (unsigned int)ebq)
      {                              /* Yes, got an 8th-bit prefix? */
        b8 = 0200;                   /* Yes, remember this, */
        a = *buf++;                  /* and get the prefixed character. */
      }
    }

    if (
      (unsigned int)a == \
      (unsigned int)ctlq)
    {                                /* If control prefix, */
      a = *buf++;                    /* get its operand. */
      a7 = a & 0177;                 /* Only look at low 7 bits. */
      if (
        ( a7 >= 0100 && a7 <= 0137 ) \
          || a7 == '?')              /* Uncontrollify, */
      {
        a = ctl(a);                  /* if in control range */
      }
    }

    a |= b8;                         /* Or, in the 8th bit */
    if (rpt == 0)
    {
      rpt = 1;                       /* If no repeats, then one */
    }

#ifdef NLCHAR
    if (!binary)                     /* If in text mode, */
    {
      if (a == CR)
      {
        continue;                    /* discard carriage returns, */
      }

      if (a == LF)
      {
        a = NLCHAR;                  /* convert LF to system's newline. */
      }
    }

#endif /* ifdef NLCHAR */
    if (fn == putfil)                /* Speedup via buffering and a macro */
    {
      for (; rpt > 0; rpt--)         /* Output the char RPT times */
      {
        x = zmchout(a);
        if (x < 0)   /* zmchout is a macro */
        {
          debug(F101,
            "decode zmchout", "", x);
          return ( -1 );
        }

        ffc++;                       /* Count the character */
      }
    }
    else                             /* Not to the file */
    {
      for (; rpt > 0; rpt--)         /* Output the char RPT times */
      {
        if (( *fn )(a) < 0)
        {
          return ( -1 );             /* Send to output function. */
        }
      }
    }
  }
  return ( 0 );
}

/* G E T P K T -- Fill a packet data field */

/*
 * Gets characters from the current source -- file or memory string.
 * Encodes the data into the packet, filling the packet optimally.
 * Set first = 1 when calling for the first time on a given input stream
 * (string or file).
 *
 * Uses global variables:
 *   t     -- current character.
 *   first -- flag: 1 to start up, 0 for input in progress, -1 for EOF.
 *   next  -- next character.
 *   data  -- the packet data buffer.
 *   size  -- number of characters in the data buffer.
 *
 * Returns the size as value of the function, and also sets global size,
 * and fills (and NULL-terminates) the global data array.
 *
 * Returns 0 upon EOF.
 *
 * Rewritten by Paul W. Placeway (PWP) of Ohio State University, March '89.
 * Incorporates old getchx() and encode() inline to eliminate function
 * calls, uses buffered input for much-improved efficiency, and clears up
 * some confusion with line termination (CR+LF vs. LF vs. CR).
 */

int
getpkt(bufmax)
int bufmax;
{                                     /* Fill one packet buffer */
  register CHAR rt = t, rnext = next; /* register shadows of the globals */
  register CHAR *dp, *odp, *p1, *p2;  /* pointers... */
  register int x;                     /* Loop index. */
  register int a7;                    /* Low 7 bits of character */
  static   CHAR leftover[6] = {
    '\0', '\0', '\0',
    '\0', '\0', '\0'
  };

  if (first == 1)                     /* If first time thru...  */
  {
    first = 0;                        /* remember, */
    *leftover = '\0';                 /* discard interrupted leftovers, */
                                      /* get 1st character of file into t */
    if (memstr)                       /* watching out for null file */
    {
      if (( rt = *memptr++ ) == '\0') /* end of string ==> EOF. */
      {
        first = -1;
        size = 0;
        debug(F100,
          "getpkt: empty string", "", 0);
        return ( 0 );
      }
    }
    else
    {
      if (( x = zminchar()) == -1)    /* End of file */
      {
        first = -1;
        debug(F100,
          "getpkt: empty file", "", 0);
        size = 0;
        return ( 0 );
      }

      ffc++;                          /* Count a file character */
      rt = x;
    }

    rt &= fmask;                      /* Bytesize mask */
  }
  else if (( first == -1 ) && \
    ( *leftover == '\0' ))            /* EOF from last time? */
  {
    return ( size = 0 );
  }

  dp = data;
  for (
    p1 = leftover;
      ( *dp = *p1 ) != '\0';
        p1++, dp++)                   /* Copy leftovers */
  {
    ;
  }

  *leftover = '\0';
  if (first == -1)
  {
    return ( size = ( dp - data ));   /* Handle final leftovers */
  }

  /*
   * Now, fill up the
   * rest of the packet.
   */

  rpt = 0;                               /* Clear out old repeat count. */
  while (first > -1)                     /* Until EOF... */
  {
    if (memstr)                          /* get next character */
    {
      if (( rnext = *memptr++ ) == '\0') /* end of string ==> EOF */
      {
        first = -1;                      /* Flag eof for next time. */
      }
      else
      {
        rnext &= fmask;                  /* Bytesize mask. */
      }
    }
    else
    {
      if (( x = zminchar()) == -1)       /* End of file */
      {
        first = -1;                      /* Flag eof for next time. */
      }
      else
      {
        rnext = x & fmask;               /* Bytesize mask. */
        ffc++;                           /* Count it */
      }
    }

    odp = dp;                            /* Remember current position. */

    if (rptflg)                          /* Repeat processing? */
    {
      if (
#ifdef NLCHAR

        /*
         * If the next char is really CRLF, then we cannot
         * be doing a repeat (unless CR,CR,LF which becomes
         * "~ <n-1> CR CR LF", which is OK, but not most
         * efficient). The actual conversion from NL to CRLF
         * is done after the rptflg if...
         */

        ( binary || \
          ( rnext != NLCHAR )) &&
#endif /* ifdef NLCHAR */
            rt == rnext && \
              ( first == 0 ))            /* Got a run... */
      {
        if (++rpt < 94)                  /* Below max, just count */
        {
          continue;                      /* go back and get another */
        }
        else if (rpt == 94)              /* Reached max, must dump */
        {
          *dp++ = rptq;
          *dp++ = tochar(rpt);
          rpt = 0;
        }
      }
      else if (rpt == 1)                 /* Run broken, only 2? */
      {
      /* ... */                          /* XXX(jhj): Now empty */
      }
      else if (rpt > 1)                  /* More than two */
      {
        *dp++ = rptq;                    /* Insert the repeat prefix */
        *dp++ = tochar(++rpt);           /* and count. */
        rpt = 0;                         /* Reset repeat counter. */
      }
    }

#ifdef NLCHAR
    if (!binary && ( rt == NLCHAR ))
    {
      *dp++ = myctlq;                    /* Put in the encoding directly */
      *dp++ = 'M';                       /* == ctl(CR) */
      if (( dp - data ) <= maxsize)
      {
        odp = dp;                        /* check packet bounds */
      }

      rt = LF;
    }
#endif /* ifdef NLCHAR */

    a7 = rt & 0177;                      /* Low 7 bits of character */
    if (ebqflg && ( rt & 0200 ))         /* Do 8th bit prefix if needed */
    {
      *dp++ = ebq;
      rt = a7;
    }

    if (( a7 < SP ) || \
      ( a7 == DEL ))                     /* Do control prefix if needed */
    {
      *dp++ = myctlq;
      rt = ctl(rt);
    }

    if (a7 == myctlq)                    /* Prefix the control prefix */
    {
      *dp++ = myctlq;
    }

    if (( rptflg ) && ( a7 == rptq ))    /* If it's the repeat prefix, */
    {
      *dp++ = myctlq;                    /* Quote if doing repeat counts. */
    }

    if (( ebqflg ) && ( a7 == ebq ))     /* Prefix the 8th bit prefix */
    {
      *dp++ = myctlq;                    /* if doing 8th-bit prefixes */
    }

    *dp++ = rt;                          /* Finally, insert the character */

    if (rpt == 1)                        /* Exactly two copies? */
    {
      rpt = 0;
      p2 = dp;                           /* Save current size temporarily */
      for (p1 = odp; p1 < p2; p1++)      /* copy the old chars over again */
      {
        *dp++ = *p1;
      }

      if (( p2 - data ) <= maxsize)
      {
        odp = p2;                        /* check packet bounds */
      }
    }

    rt = rnext;                          /* Next is now current. */
    if (( dp - data ) >= bufmax)         /* If too big save some for next */
    {
      size = ( dp - data );
      *dp = '\0';                        /* mark (current) the end. */
      if (( dp - data ) > bufmax)        /* if packet is overfull */
      {
        for (p1 = leftover, p2 = odp;
              ( *p1 = *p2 ) != '\0';
                p1++, p2++)
        {
          ;
        }

        debug(F111,
              "getpkt leftover", leftover, size);
        debug(F101,
              " osize", "", ( odp - data ));
        size = ( odp - data );           /* Return truncated packet. */
        *odp = '\0';                     /* Mark real end */
      }
      else                               /* If the packet is exactly full */
      {
        debug(F101,
          "getpkt exact fit", "", size);
      }

      t = rt;
      next = rnext;                      /* save for next time */
      return ( size );
    }
  }                                      /* Otherwise, keep filling. */
  size = ( dp - data );
  *dp = '\0';                            /* mark (current) the end. */
  debug(F111,
    "getpkt eof/eot", data, size);       /* Fell thru before full, */
  return ( size );                       /* Return part filled last pkt */
}

/* C A N N E D -- Check if current file transfer cancelled */

int
canned(buf)
char *buf;
{
  if (*buf == 'X')
  {
    cxseen = 1;
  }

  if (*buf == 'Z')
  {
    czseen = 1;
  }

  debug(F101, "canned: cxseen", "", cxseen);
  debug(F101, " czseen", "", czseen);
  return (( czseen || cxseen ) ? 1 : 0 );
}

/* R E S E T C -- Reset per-transaction character counters */

#ifndef NOSTATS
void
resetc()
{
  flci = flco = 0;
  tfc = tlci = tlco = 0;        /* Total file chars, line chars in & out */
}
#endif /* ifndef NOSTATS */

/* T I N I T -- Initialize a transaction */
void
tinit()
{
  xflg            = 0;             /* Reset x-packet flag */
  rqf             = -1;            /* Reset 8th-bit-quote request flag */
  memstr          = 0;             /* Reset memory-string flag */
  memptr          = NULL;          /*  and pointer */
  n_len           = -1;            /* No encoded-ahead data */
  bctu            = 1;             /* Reset block check type to 1 */
  ebq = ebqflg    = 0;             /* Reset 8th-bit quoting stuff */

  if (savmod)                      /* If global file mode was saved, */
  {
    binary        = 1;             /*  restore it, */
    savmod        = 0;             /*  unsave it. */
  }

  prvpkt          = -1;            /* Reset packet number */
  pktnum          = 0;
  numerrs         = 0;             /* Transmission error counter */
  cxseen = czseen = 0;             /* Reset interrupt flags */

  *filnam         = '\0';          /* Clear file name */
  *sndpkt         = '\0';          /* Clear retransmission buffer */

  spktl           = 0;             /* And its length */
  nakstate        = 0;             /* Say we're not in a NAK'ing state */

#ifndef NOSERVER
  if (server)                      /* If acting as server, */
  {
    timint        = srvtim;        /* Use server timeout interval. */
  }
#endif /* ifndef NOSERVER */
}

/* R I N I T -- Respond to S or I packet */

void
rinit(d)
char *d;
{
  char *tp;
  ztime(&tp);
  tlog(F110,
    "Transaction begins", tp, 0l); /* Make transaction log entry */
  if (binary)
  {
    tlog(F100,
      "Global file mode = binary", "", 0l);
  }
  else
  {
    tlog(F100,
      "Global file mode = text", "", 0l);
  }

  filcnt = 0;                      /* Init file counter */
  spar(d);
  ack1(rpar());
}

/* S I N I T -- Make sure file exists, then send Send-Init packet */

int
sinit()
{
  int x;
  char *tp;

  filcnt = 0;
  sndsrc = nfils;                   /* Where to look for files to send */

  ztime(&tp);
  tlog(F110,
    "Transaction begins", tp, 0l);  /* Make transaction log entry */
  debug(F101,
    "sinit: sndsrc", "", sndsrc);
  if (sndsrc < 0)                   /* Must expand from 'send' command */
  {
#ifdef DTILDE
    char *tnam, *tilde_expand();    /* May have to expand tildes */
    tnam = tilde_expand(cmarg);     /* Try to expand tilde. */
    if (*tnam != '\0')
    {
      cmarg = tnam;
    }
#endif /* ifdef DTILDE */
    nfils = zxpand(cmarg);          /* Look up literal name. */
    if (nfils < 0)
    {
#ifndef NOSERVER
      if (server)
      {
        errpkt(
          "Too many files");
      }
      else
      {
#endif /* ifndef NOSERVER */
        screen(SCR_EM, 0, 0l,
          "Too many files");
#ifndef NOSERVER
      }
#endif /* ifndef NOSERVER */

      return ( 0 );
    }
    else if (nfils == 0)            /* If none found, */
    {
      char xname[100];              /* convert the name. */
      zrtol(cmarg, xname);
      nfils = zxpand(xname);        /* Look it up again. */
    }

    if (nfils < 1)                  /* If no match, report error. */
    {
#ifndef NOSERVER
      if (server)
      {
        errpkt("File not found");
      }
      else
      {
#endif /* ifndef NOSERVER */
        screen(SCR_EM, 0, 0l,
          "File not found");
#ifndef NOSERVER
      }
#endif /* ifndef NOSERVER */
      return ( 0 );
    }

    x = gnfile();                   /* Position to first file. */
    if (x < 1)
    {
#ifndef NOSERVER
      if (!server)
      {
#endif /* ifndef NOSERVER */
        screen(SCR_EM, 0, 0l,
          "No readable file to send");
#ifndef NOSERVER
      }
      else
      {
        errpkt(
          "No readable file to send");
      }
#endif /* ifndef NOSERVER */
      return ( 0 );
    }
  }
  else if (sndsrc > 0)              /* Command line arglist -- */
  {
    x = gnfile();                   /* Get the first file from it. */
    if (x < 1)
    {
      return ( 0 );                 /* (if any) */
    }
  }
  else if (sndsrc == 0)             /* stdin or memory always exist... */
  {
    if (( cmarg2 != NULL ) && \
      ( *cmarg2 ))
    {
      strcpy(filnam, cmarg2);       /* If F packet, "as" name is used */
      cmarg2 = "";                  /* if provided, */
    }
    else                            /* otherwise */
    {
      strcpy(filnam, "stdin");      /* just use this. */
    }
  }
#ifdef DEBUG
  debug(F101,
    "sinit: nfils", "", nfils);
  debug(F110,
    " filnam", filnam, 0);
  debug(F110,
    " cmdstr", cmdstr, 0);
#endif /* ifdef DEBUG */

  ttflui();                         /* Flush input buffer. */

  if (!local
#ifndef NOSERVER
    && !server
#endif /* ifndef NOSERVER */
      )
  {
    sleep(delay);
  }

  sipkt('S');                       /* Send the Send-Init packet. */
  return ( 1 );
}

void
sipkt(c)
char c;
{                                   /* Send S or I packet. */
  CHAR *rp;
  ttflui();                         /* Flush pending input. */
  rp = rpar();                      /* Get parameters. */
  spack(c, pktnum, strlen(rp), rp);
}

/* R C V F I L -- Receive a file */

/*
 * Incoming filename is in data field of F packet. This
 * function decodes it into the srvcmd buffer, substituting
 * an alternate "as-name", if one was given. Finally, it
 * does any requested transformations (like converting to
 * lowercase) then if a file of the same name already exists,
 * makes a new unique name.
 */

int
rcvfil(n)
char *n;
{
  char xname[100], *xp;                /* Buffer for constructing name */
#ifdef DTILDE
  char *dirp, *tilde_expand();
#endif /* ifdef DTILDE */

  srvptr = srvcmd;                     /* Decode file name from packet. */
  decode(rdatap, putsrv);
  if (*srvcmd == '\0')                 /* Watch out for null F packet. */
  {
    strcpy(srvcmd, "NONAME");
  }

#ifdef DTILDE
  dirp = tilde_expand(srvcmd);         /* Expand tilde, if any. */
  if (*dirp != '\0')
  {
    strcpy(srvcmd, dirp);
  }

#endif /* ifdef DTILDE */
  screen(SCR_FN, 0, 0l, srvcmd);       /* Put it on screen */
  tlog(F110, "Receiving", srvcmd, 0l); /* Transaction log entry */
  if (cmarg2 != NULL)                  /* Check for alternate name */
  {
    if (*cmarg2 != '\0')
    {
      strcpy(srvcmd, cmarg2);          /* Got one, use it. */
      *cmarg2 = '\0';
    }
  }

  xp = xname;                          /* OK to proceed. */
  if (fncnv)                           /* If desired, */
  {
    zrtol(srvcmd, xp);                 /* convert name to local form */
  }
  else                                 /* otherwise, */
  {
    strcpy(xname, srvcmd);             /* use it literally */
  }

  if (warn)                            /* File collision avoidance? */
  {
    if (zchki(xname) != -1)            /* Yes, file exists? */
    {
      znewn(xname, &xp);               /* Yes, make new name. */
      strcpy(xname, xp);
      debug(F110,
        " exists, new name ",
          xname, 0);
    }
  }

  debug(F110,
    "rcvfil: xname",
      xname, 0);
  strcpy(n, xname);                    /* Return pointer to actual name. */
  ffc = 0;                             /* Init file character counter */
  filcnt++;
  return ( 1 );                        /* Always succeeds */
}

/* O P E N A -- Open a file, with attributes */

/*
 * This function tries to open a new file to
 * put the arriving data in. The filename is
 * the one in the srvcmd buffer. If warning
 * is on and a file of that name already
 * exists, then a unique name is chosen.
 */

int
opena(f, zz)
char *f;
struct zattr *zz;
{
  int x;

#ifdef DEBUG
  adebu(f, zz);                     /* Write attributes to debug log */
#endif /* ifdef DEBUG */

  if (bsavef)                       /* If somehow file mode */
  {
    binary = bsave;                 /* was saved but not restored, */
    bsavef = 0;                     /* restore it. */
    debug(F101,
      "opena restoring binary",
        "", binary);
  }

  if (( x = openo(f)))              /* Try to open the file. */
  {
    tlog(F110, " as", f, 0l);       /* OK, open, record name. */
    if (zz->type.val[0] == 'A')     /* Check attributes */
    {
      bsave = binary;               /* Save global file type */
      bsavef = 1;                   /* ( restore it in clsof() ) */
      binary = 0;
      debug(F100,
        "opena attribute A = text",
          "", binary);
    }
    else if \
      (zz->type.val[0] == 'B')
    {
      bsave = binary;               /* Save global file type */
      bsavef = 1;
      binary = 1;
      debug(F100,
        "opena attribute B = binary",
          "", binary);
    }

    if (binary)                     /* Log file mode in transaction log */
    {
      tlog(F100,
        " mode: binary", "", 0l);
    }
    else
    {
      tlog(F100,
        " mode: text", "", 0l);
    }

    screen(SCR_AN, 0, 0l, f);
#ifndef NOICP
#ifndef NOCONN
    intmsg(filcnt);
#endif /* ifndef NOCONN */
#endif /* ifndef NOICP */
  }
  else                              /* Did not open file OK. */
  {
    tlog(F110,
      "Failure to open", f, 0l);
    screen(SCR_EM, 0, 0l,
      "Can't open file");
  }

  ffc = 0;                          /* Init file character counter */
  return ( x );                     /* Pass on return code from openo */
}

/* R E O F -- Receive End Of File */

int
reof(yy)
struct zattr *yy;
{
  int x;
#ifndef NODISP
  char *p;
  char c;
#endif /* ifndef NODISP */

  if (cxseen == 0)
  {
    cxseen = ( *rdatap == 'D' );       /* Got discard directive? */
  }

  x = clsof(cxseen | czseen);
  if (cxseen || czseen)
  {
    tlog(F100,
      " *** Discarding", "", 0l);
    cxseen = 0;
  }
  else
  {
#ifndef NOSTATS
    fstats();                          /* Close out file statistics */
#endif /* ifndef NOSTATS */
#ifndef NODISP
    if (yy->disp.len != 0)             /* Handle file disposition */
    {
      p = yy->disp.val;
      c = *p++;
      if (c == 'M')                    /* Mail to user. */
      {
        zmail(p, filnam);              /* Do the system's mail command */
        tlog(F110,
         "mailed", filnam, 0l);
        tlog(F110, " to", p, 0l);
        zdelet(filnam);                /* Delete the file */
      }
      else if (c == 'P')               /* Print the file. */
      {
        zprint(p, filnam);             /* Do the system's print command */
        tlog(F110,
          "printed", filnam, 0l);
        tlog(F110,
          " with options", p, 0l);
        zdelet(filnam);                /* Delete the file */
      }
    }
#endif /* ifndef NODISP */
  }

  *filnam = '\0';
  return ( x );
}

/* R E O T -- Receive End Of Transaction */

void
reot()
{
  cxseen = czseen = 0;                 /* Reset interruption flags */
#ifndef NOSTATS
  tstats();
#endif /* ifndef NOSTATS */
}

/* S F I L E -- Send File header or teXt header packet */

/*
 * Call with x nonzero for X packet,
 * zero for F packet. Returns 1 on
 * success, 0 on failure.
 */

int
sfile(x)
int x;
{
  char pktnam[100];                    /* Local copy of name */
  char *s;

  if (x == 0)                          /* F-Packet setup */
  {
    if (*cmarg2 != '\0')               /* If we have a send-as name, */
    {
      strcpy(pktnam, cmarg2);          /* copy it literally, */
      cmarg2 = "";                     /* and blank it out for next time. */
    }
    else                               /* Otherwise use actual file name: */
    {
      if (fncnv)                       /* If converting names, */
      {
        zltor(filnam, pktnam);         /* convert it to common form, */
      }
      else                             /* otherwise, */
      {
        strcpy(pktnam, filnam);        /* copy it literally. */
      }
    }
#ifdef DEBUG
    debug(F110,
      "sfile", filnam, 0);             /* Log debugging info */
    debug(F110,
      " pktnam", pktnam, 0);
#endif /* ifdef DEBUG */
    if (openi(filnam) == 0)            /* Try to open the file */
    {
      return ( 0 );
    }

    s = pktnam;                        /* Name for packet data field */
  }
  else                                 /* X-packet setup */
  {
    debug(F110, "sxpack", cmdstr, 0);  /* Log debugging info */
    s = cmdstr;                        /* Name for data field */
  }

  encstr(s);                           /* Encode the name into data[]. */
  nxtpkt(&pktnum);                     /* Increment the packet number */
  spack(
    x ? 'X' : 'F',
      pktnum, size, data);             /* Send the F or X packet */

  if (x == 0)                          /* Display for F packet */
  {
    if (displa)                        /* Screen */
    {
      screen(SCR_FN, 'F',
        (long)pktnum, filnam);
      screen(SCR_AN, 0,
        0l, pktnam);
      screen(SCR_FS, 0,
        (long)fsize, "");
    }

    tlog(F110,
      "Sending", filnam, 0l);          /* Transaction log entry */
    tlog(F110,
      " as", pktnam, 0l);
  }
  else                                 /* Display for X-packet */
  {
    screen(SCR_XD,
      'X', (long)pktnum, cmdstr);      /* Screen */
    tlog(F110,
      "Sending from:", cmdstr, 0l);    /* Transaction log */
  }

#ifndef NOICP
#ifndef NOCONN
  intmsg(++filcnt);             /* Count file, give interrupt msg */
#endif /* ifndef NOICP */
#endif /* ifndef NOCONN */
  first = 1;                    /* Init file character lookahead. */
  n_len = -1;                   /* Init the packet encode-ahead length */
  ffc = 0;                      /* Init file character counter. */
  return ( 1 );
}

/* S D A H E A D -- (PWP) Encode the next data packet to send */

void
sdahead()
{
  if (spsiz > MAXPACK)                 /* S logic in ckcfn2.c, spack() */
  {
    n_len = getpkt(spsiz - bctu - 5);  /* long packet size */
  }
  else
  {
    n_len = getpkt(spsiz - bctu - 2);  /* short packet size */
  }
}

/* S D A T A -- Send a data packet */

/*
 * Returns -1 if no data to send, else
 * sends packet and returns length.
 */

int
sdata()
{
  int len;

  if (cxseen || czseen)           /* If interrupted, done. */
  {
    return ( -1 );
  }

  if (n_len < 0)                  /* (PWP) if we haven't encoded the */
  {
    sdahead();                    /* packet yet, do it now */
  }

  len = n_len;
  if (len == 0)                   /* Done if no data. */
  {
    return ( -1 );
  }

  nxtpkt(&pktnum);                /* Increment the packet number */
  spack('D', pktnum, len, data);  /* Send the packet */

  /*
   * Comment out next statement,
   * if it causes problems...
   */

  if (len > 0)                    /* if we got data last time */
  {
    sdahead();                    /* encode more now */
  }

  return ( len );
}

/* S E O F -- Send an End-Of-File packet */

/*
 * Call with a string pointer to character
 * to put in the data field, or else a null
 * pointer, or "" for no data.
 */

void
seof(s)
char *s;
{
  nxtpkt(&pktnum);                /* Increment the packet number */
  if (( s != NULL ) && \
    ( *s != '\0' ))
  {
    spack('Z', pktnum, 1, s);
    tlog(F100,
      " *** interrupted, sending discard request", "", 0l);
  }
  else
  {
    spack('Z', pktnum, 0, "");
#ifndef NOSTATS
    fstats();
#endif /* ifndef NOSTATS */
  }
}

/* S E O T -- Send an End-Of-Transaction packet */

void
seot()
{
  nxtpkt(&pktnum);             /* Increment the packet number */
  spack('B', pktnum, 0, "");   /* Send the EOT packet */
  cxseen = czseen = 0;         /* Reset interruption flags */
#ifndef NOSTATS
  tstats();                    /* Log timing info */
#endif /* ifndef NOSTATS */
}

/* R P A R -- Fill the data array with my send-init parameters */

CHAR *
rpar()
{
  if (rpsiz > MAXPACK)         /* Biggest normal packet I want. */
  {
    data[1] = tochar(MAXPACK); /* If > 94, use 94, but specify */
  }
  else                         /* extended packet length below... */
  {
    data[1] = tochar(rpsiz);   /* else use what the user said. */
  }

  data[2] = tochar(rtimo);     /* When I want to be timed out */
  data[3] = tochar(mypadn);    /* How much padding I need (none) */
  data[4] = ctl(mypadc);       /* Padding character I want */
  data[5] = tochar(eol);       /* End-Of-Line character I want */
  data[6] = '#';               /* Control-Quote character I send */
  switch (rqf)                 /* 8th-bit prefix */
  {
  case -1:
  case 1:
    if (parity)
    {
      ebq = sq = '&';
    }

    break;

  case 0:
  case 2:
    break;
  }
  data[7] = sq;
  data[8] = bctr + '0';                 /* Block check type */
  if (rptflg)                           /* Run length encoding */
  {
    data[9] = rptq;                     /* If receiving, agree. */
  }
  else
  {
    data[9] = '~';
  }

#ifndef NOATTR
  data[10] = tochar(
    ( atcapr ? atcapb : 0 ) | \
      ( lpcapr ? lpcapb : 0 ) |
        ( swcapr ? swcapb : 0 ));
#endif /* ifndef NOATTR */
  data[capas + 1] = \
    tochar(swcapr ? wslots : 0);        /* Window size */

  rpsiz = urpsiz;                       /* Long packets ... */
  data[capas + 2] = tochar(rpsiz / 95); /* Long packet size, big part */
  data[capas + 3] = tochar(rpsiz % 95); /* Long packet size, little part */
  data[capas + 4] = '\0';               /* Terminate the init string */
#ifdef DEBUG
  if (deblog)
  {
    debug(F110, "rpar", data + 1, 0);
    rdebu(capas + 2);
  }
#endif /* ifdef DEBUG */
  return ( data + 1 );                  /* Return pointer to string. */
}

void
spar(s)
char *s;
{                                       /* Set parameters */
  int x, lpsiz;

  debug(F110,
    "entering spar", s, 0);
  s--;                                  /* Line up with field numbers */
                                        /* Limit size of outbound packets */
  x = ( rln >= 1 ) ? \
    xunchar(s[1]) : 80;
  lpsiz = spsiz;                        /* Remember what they SET. */
  if (spsizf)                           /* SET-command override? */
  {
    if (x < spsiz)
    {
      spsiz = x;                        /* Ignore LEN unless smaller */
    }
  }
  else                                  /* otherwise */
  {
    spsiz = ( x < 10 ) ? 80 : x;        /* believe them if reasonable */
  }

  if (!timef)                           /* Timeout on inbound packets */
  {                                     /* Only if not SET-cmd override */
    x = ( rln >= 2 ) ?\
      xunchar(s[2]) : 5;
    timint = ( x < 0 ) ? 5 : x;
  }

  /*
   * Outbound
   * Padding
   */

  npad = 0;
  padch = '\0';
  if (rln >= 3)
  {
    npad = xunchar(s[3]);
    if (rln >= 4)
    {
      padch = ctl(s[4]);
    }
    else
    {
      padch = 0;
    }
  }

  /*
   * Outbound Packet
   * Terminator
   */

  seol = ( rln >= 5 ) ? \
    xunchar(s[5]) : '\r';
  if (( seol < 2 ) || \
    ( seol > 31 ))
  {
    seol = '\r';
  }

  /*
   * Control
   * prefix
   */

  x = ( rln >= 6 ) ? \
    s[6] : '#';
  myctlq = (( x > 32 && x < 63 ) || \
    ( x > 95 && x < 127 )) ? x : '#';

  /*
   * 8th-bit
   * prefix
   */

  rq = ( rln >= 7 ) ? \
    s[7] : 0;
  if (rq == 'Y')
  {
    rqf = 1;
  }
  else if (( rq > 32 && rq < 63 ) || \
    ( rq > 95 && rq < 127 ))
  {
    rqf = 2;
  }
  else
  {
    rqf = 0;
  }

  switch (rqf)
  {
  case 0:
    ebqflg = 0;
    break;

  case 1:
    if (parity)
    {
      ebqflg = 1;
      ebq = '&';
    }

    break;

  case 2:
    if (( ebqflg = \
      ( ebq == sq || \
        sq == 'Y' )))
    {
      ebq = rq;
    }
  }

  /*
   * Block
   * check
   */

  x = 1;
  if (rln >= 8)
  {
    x = s[8] - '0';
    if (( x < 1 ) || \
        ( x > 3 ))
    {
      x = 1;
    }
  }

  bctr = x;

  /*
   * Repeat
   * prefix
   */

  if (rln >= 9)
  {
    rptq = s[9];
    rptflg = (
      ( rptq > 32 && \
        rptq < 63 ) || \
      ( rptq > 95 && \
        rptq < 127 ));
  }
  else
  {
    rptflg = 0;
  }

  /*
   * Capabilities
   */

#ifndef NOATTR
  atcapu = lpcapu = swcapu = 0;
  if (rln >= 10)
  {
    x = xunchar(s[10]);
    atcapu = ( x & atcapb ) && \
      atcapr;
    lpcapu = ( x & lpcapb ) && \
      lpcapr;
    swcapu = ( x & swcapb ) && \
      swcapb;
    for (
      capas = 10;
      ( xunchar(s[capas]) & 1 ) && \
        ( rln >= capas );
          capas++)
    {
      ;
    }
  }
#endif /* ifndef NOATTR */

  /*
   * Long
   * Packets
   */

  if (lpcapu)
  {
    if (rln > capas + 2)
    {
      x = xunchar(s[capas + 2]) * \
          95 + xunchar(s[capas + 3]);
      if (spsizf)                           /* If overriding negotiations */
      {
        spsiz = ( x < lpsiz ) ? x : lpsiz;  /* do this, */
      }
      else                                  /* otherwise */
      {
        spsiz = ( x > MAXSP ) ? MAXSP : x;  /* do this. */
      }

      if (spsiz < 10)
      {
        spsiz = 80;                         /* Be defensive... */
      }
    }
  }

  /*
   * Save current send packet size
   * for optimal packet size calcs
   */

  spmax = spsiz;
  numerrs = 0;

  /*
   * Sliding
   * Windows
   */

  if (swcapu)
  {
    if (rln > capas + 1)
    {
      x = xunchar(s[capas + 1]);
      wslots = x > MAXWS ? MAXWS : x;
    }
    else
    {
      wslots = 1;
    }
  }

#ifdef DEBUG
  if (deblog)
  {
    sdebu(rln);                        /* Record parameters in debug log */
  }

#endif /* ifdef DEBUG */
}

/* G N F I L E -- Get the next file name from a file group */

/*
 * Returns 1 if there's a
 * next file, 0 otherwise
 */

int
gnfile()
{
  int x;
  long y;

  /*
   * If file group interruption
   * (C-Z) occured, then fail.
   */

  debug(F101,
    "gnfile: czseen", "", czseen);

  if (czseen)
  {
    tlog(F100,
      "Transaction cancelled", "", 0l);
    return ( 0 );
  }

  /*
   * If input was stdin or memory
   * string, there is no next file.
   */

  if (sndsrc == 0)
  {
    return ( 0 );
  }

  /*
   * If file list comes from command
   * line argunents, then get the
   * next list element.
   */

  y = -1;
  while (y < 0)                       /* Keep trying till we get one... */
  {
    if (sndsrc > 0)
    {
      if (nfils-- > 0)
      {
        strcpy(filnam, *cmlist++);
        debug(F111,
          "gnfile: cmlist filnam", filnam, nfils);
      }
      else
      {
        *filnam = '\0';
        debug(F101,
          "gnfile cmlist: nfils", "", nfils);
        return ( 0 );
      }
    }

    /*
     * Otherwise, step to next
     * element of internal wildcard
     * expansion list.
     */

    if (sndsrc < 0)
    {
      x = znext(filnam);
      debug(F111, "gnfile znext: filnam", filnam, x);
      if (x == 0)
      {
        return ( 0 );
      }
    }

    /*
     * Get here with
     * a filename.
     */

    y = zchki(filnam);                         /* Check if file readable */
    if (y < 0)
    {
      debug(F110,
        "gnfile skipping:", filnam, 0);
      tlog(F111,
        filnam, "not sent, reason", (long)y);
      screen(SCR_ST, ST_SKIP, 0l, filnam);
    }
    else
    {
      fsize = y;
    }
  }
  return ( 1 );
}

/* O P E N I -- Open an existing file for input */

int
openi(name)
char *name;
{
  int x, filno;
  if (memstr)                                   /* Just return, */
  {
    return ( 1 );                               /* if file is memory. */
  }

  debug(F110, "openi", name, 0);
  debug(F101, " sndsrc", "", sndsrc);
  filno = ( sndsrc == 0 ) ? ZSTDIO : ZIFILE;
  debug(F101, " file number", "", filno);

  if (zopeni(filno, name))                      /* now, try to open it. */
  {
    debug(F110, " ok", name, 0);
    return ( 1 );
  }
  else                                          /* If not found, */
  {
    char xname[100];                            /* convert the name */
    zrtol(name, xname);                         /* to local form and then */
    x = zopeni(filno, xname);                   /* try opening it again. */
    debug(F101, " zopeni", "", x);
    if (x)
    {
      debug(F110, " ok", xname, 0);
      return ( 1 );                             /* It worked. */
    }
    else
    {
      screen(SCR_EM,
        0, 0l, "Can't open file");              /* It didn't work. */
      tlog(F110,
        xname, "could not be opened", 0l);
      debug(F110,
        " openi failed", xname, 0);
      return ( 0 );
    }
  }
}

/* O P E N O -- Open a new file for output */

/*
 * Returns actual name under which the
 * file was opened in string 'name2'.
 */

int
openo(name)
char *name;
{
  if (stdouf)                               /* Receiving to stdout? */
  {
    return ( zopeno(ZSTDIO, ""));
  }

  debug(F110, "openo: name", name, 0);

  if (cxseen || czseen)                     /* Interrupted, get out before */
  {
    debug(F100,
      " open cancelled", "", 0);            /* destroying existing file. */
    return ( 1 );                           /* Pretend to succeed. */
  }

  if (zopeno(ZOFILE, name) == 0)            /* Try to open the file */
  {
    debug(F110,
      "openo failed", name, 0);
    tlog(F110,
      "Failure to open", name, 0l);
    return ( 0 );
  }
  else
  {
    debug(F110,
      "openo ok, name", name, 0);
    return ( 1 );
  }
}

/* O P E N T -- Open the terminal for output, in place of a file */

int
opent()
{
  ffc = 0;
#ifndef NOSTATS
  tfc = 0;
#endif /* ifndef NOSTATS */
  return ( zopeno(ZCTERM, "") );
}

/* C L S I F -- Close the current input file */

void
clsif()
{
  if (memstr)                        /* If input was memory string, */
  {
    memstr = 0;                      /* indicate no more. */
  }
  else
  {
    zclose(ZIFILE);                  /* else close input file. */
  }

  if (czseen || cxseen)
  {
    screen(SCR_ST, ST_DISC, 0l, "");
  }
  else
  {
    screen(SCR_ST, ST_OK, 0l, "");
  }

  cxseen = hcflg = 0;                /* Reset flags, */
  *filnam = '\0';                    /* and current file name */
  n_len = -1;                        /* reinit packet encode-ahead length */
}

/* C L S O F -- Close an output file */

/*
 * Call with disp != 0 if file is to
 * be discarded. Returns -1 upon
 * failure to close, 0 or greater
 * on success.
 */

int
clsof(disp)
int disp;
{
  int x;

  if (bsavef)                        /* If we saved global file type */
  {
    debug(
      F101,
        "clsof restoring binary",
          "",
            binary);
    binary = bsave;                  /* restore it */
    bsavef = 0;                      /* only this once. */
  }

  if (( x = zclose(ZOFILE)) < 0)     /* Try to close the file */
  {
    tlog(
      F100,
        "Failure to close",
          filnam,
            0l);
    screen(SCR_ST, ST_ERR, 0l, "");
  }
  else if (disp && ( keep == 0 ))    /* Delete it if interrupted, */
  {
    if (*filnam)
    {
      zdelet(filnam);                /* don't keep incomplete files */
    }

    debug(F100, "Discarded", "", 0);
    tlog(F100, "Discarded", "", 0l);
    screen(SCR_ST, ST_DISC, 0l, "");
  }
  else                               /* Nothing wrong, just keep it */
  {
    debug(F100, "Closed", "", 0);    /* and give comforting messages. */
    screen(SCR_ST, ST_OK, 0l, "");
  }

  return ( x );                      /* Send zclose() return code. */
}

/* S N D H L P -- Routine to send builtin help */

int
sndhlp()
{
  nfils = 0;                             /* No files, no lists. */
  xflg = 1;                              /* Flag we must send X packet. */
  strcpy(cmdstr, "help text");           /* Data for X packet. */
  first = 1;                             /* Init getchx lookahead */
  memstr = 1;                            /* Just set the flag. */
  memptr = hlptxt;                       /* And the pointer. */
  if (binary)                            /* If file mode is binary, */
  {
    binary = 0;                          /* turn it back to text for this */
    savmod = 1;                          /* remember to restore it later. */
  }

  return ( sinit() );
}

/* C W D -- Change current working directory */

/*
 * String passed has first byte as length
 * of directory name, rest of string is
 * name. Fails if can't connect, else,
 * ACKs (with name) and succeeds.
 */

int
cwd(vdir)
char *vdir;
{
  char *cdd, *zgtdir();
  char *dirp;
#ifdef DTILDE
  char *tilde_expand();
#endif /* ifdef DTILDE */

  vdir[xunchar(*vdir) + 1] = '\0';       /* Terminate string with a null */

  dirp = vdir + 1;
  (void)dirp;
#ifdef DTILDE
  dirp = tilde_expand(vdir + 1);         /* Attempt to expand tilde */
  if (*dirp == '\0')
  {
    dirp = vdir + 1;                     /* in directory name. */
  }

#endif /* ifdef DTILDE */
  tlog(F110,
    "Directory requested: ", dirp, 01);
  if (zchdir(dirp))
  {
    cdd = zgtdir();                      /* Get new working directory. */
    tlog(F110,
      "Changed directory to ", cdd, 01);
    encstr(cdd);
    ack1(data);
    tlog(F110,
      "Changed directory to", cdd, 0l);
    return ( 1 );
  }
  else
  {
    tlog(F110,
      "Failed to change directory to", dirp, 0l);
    return ( 0 );
  }
}

/* S Y S C M D -- Do a system command */

/*
 * Command string is formed by
 * concatenating the two arguments
 */

int
syscmd(prefix, suffix)
char *prefix, *suffix;
{
  char *cp;

  if (prefix == NULL || \
    *prefix == '\0')
  {
    return ( 0 );
  }

  for (cp = cmdstr; *prefix != \
    '\0'; *cp++ = *prefix++)
  {
    ;
  }

  while (( *cp++ = *suffix++ ))
  {
    ;
  }

#ifndef NOPUSH
  debug(F110,
    "syscmd", cmdstr, 0);
  if (zopeni(ZSYSFN, cmdstr) > 0)
  {
    debug(F100,
      "syscmd zopeni ok", cmdstr, 0);
    nfils = sndsrc = 0;                 /* Flag that input is from stdin */
    xflg = hcflg = 1;                   /* And special flags for pipe */
    if (binary)                         /* If file mode is binary, */
    {
      binary = 0;                       /* turn it back to text for this, */
      savmod = 1;                       /* remember to restore it later. */
    }

    return ( sinit() );                 /* Send S packet */
  }
  else
  {
#endif /* ifndef NOPUSH */
  debug(F100, "syscmd zopeni failed", cmdstr, 0);
  return ( 0 );

#ifndef NOPUSH
}
#endif /* ifndef NOPUSH */
}

/* A D E B U -- Write attribute packet info to debug log */

#ifdef DEBUG
int
adebu(f, zz)
char *f;
struct zattr *zz;
{
  if (deblog == 0)
  {
    return ( 0 );
  }

  debug(F110, "Attributes for incoming file ", f, 0);
  debug(F101, " length in K", "", (int)zz->lengthk);
  debug(F111, " file type", zz->type.val, zz->type.len);
  debug(F111, " creation date", zz->date.val, zz->date.len);
  debug(F111, " creator", zz->creator.val, zz->creator.len);
  debug(F111, " account", zz->account.val, zz->account.len);
  debug(F111, " area", zz->area.val, zz->area.len);
  debug(F111, " password", zz->passwd.val, zz->passwd.len);
  debug(F101, " blksize", (int)zz->blksize, 0);
  debug(F111, " access", zz->access.val, zz->access.len);
  debug(F111, " encoding", zz->encoding.val, zz->encoding.len);
#ifndef NODISP
  debug(F111, " disposition", zz->disp.val, zz->disp.len);
#endif /* ifndef NODISP */
  debug(F111, " lprotection", zz->lprotect.val, zz->lprotect.len);
  debug(F111, " gprotection", zz->gprotect.val, zz->gprotect.len);
  debug(F111, " systemid", zz->systemid.val, zz->systemid.len);
  debug(F111, " recfm", zz->recfm.val, zz->recfm.len);
  debug(F111, " sysparam", zz->sysparam.val, zz->sysparam.len);
  debug(F101, " length", "", (int)zz->length);
  return ( 0 );
}
#endif /* ifdef DEBUG */
