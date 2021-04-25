#ifndef NOCKUSCR
char *loginv = "Script Processor, 4G(025), 2021-APR-24";

/* C K U S C R -- Login script for logging onto remote system */

/*
 * Copyright (C) 2021, Jeffrey H. Johnson <trnsz@pobox.com>
 *
 * Copyright (C) 1985, Herman Fischer, Encino CA
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

/*
 * This module should work under all versions of UNIX.
 * It calls externally defined system-depended functions for i/o.
 * The module expects a login string of the expect send [expect send] ...
 * format. It is intended to operate similarly to the way the common
 * uucp "L.sys" login entries work. Conditional responses are supported
 * expect[-send-expect[...]] as with uucp. The send keyword EOT sends a
 * control-d, and the keyword BREAK sends a break.  Letters prefixed
 * by '~' are '~b' backspace, '~s' space, '~n' linefeed, '~r' return,
 * '~x' xon, '~t' tab, '~q' ? (not allowed on kermit command lines),
 * '~' ~, '~'', '~"', '~c' don't append return, and '~o[o[o]]' octal
 * character. As with some UUCP systems, sent strings are followed by
 * ~r (not ~n) unless they end with ~c. Null expect strings (e.g., ~0
 * or --) cause a short delay, and are useful for sending sequences
 * requiring slight pauses.
 *
 */

#include "ckcdeb.h"
#include "ckcker.h"
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>

#ifdef __linux__
#include <string.h>
#endif /* ifdef __linux__ */

extern int local, speed, flow, seslog, mdmtyp, quiet, duplex;
extern char ttname[];
extern CHAR dopar();
static char *chstr();

static int EXP_ALRM = 15;           /* Time to wait for expect string */
#define SND_ALRM 15                 /* Time to allow for sending string */
#define NULL_EXP 2                  /* Time to pause on null expect strg*/
#define DEL_MSEC 300                /* milliseconds to pause on ~d */

#define SBUFL 512
static char seq_buf[SBUFL + 1], *s; /* Login Sequence buffer */
static char fls_buf[SBUFL + 1];     /* Flush buffer */
static int got_it, no_cr;

/*
 * Connect state parent/child
 * communication signal handlers
 */

static jmp_buf alrmRng;             /* Envir ptr for connect errors */

SIGTYP
scrtime()                           /* Modem read failure handler */
{
  longjmp(alrmRng, 1);              /* Notifies parent process to stop */
}

/*
 * Sequence interpreter -- pick up next sequence from
 * command string, decode escapes and place into seq_buf.
 * If string contains a ~d (delay) then sequenc returns a
 * 1 expecting to be called again after the ~d executes.
 */

static
sequenc()
{
  int i;
  char c, oct_char;

  no_cr = 0;                                      /* needs cr appended */

  for (i = 0; i < SBUFL;)
  {
    if (*s == '\0' || *s == '-' || isspace(*s))   /* done */
    {
      seq_buf[i] = '\0';
      return ( 0 );
    }

    if (*s == '~')                                /* escape character */
    {
      switch (c = *( ++s ))
      {
      case 'n':
        seq_buf[i++] = LF;
        break;

      case 'r':
        seq_buf[i++] = CR;
        break;

      case 't':
        seq_buf[i++] = '\t';
        break;

      case 'b':
        seq_buf[i++] = '\b';
        break;

      case 'q':
        seq_buf[i++] = '?';
        break;

      case '~':
        seq_buf[i++] = '~';
        break;

      case '\'':
        seq_buf[i++] = '\'';
        break;

      case '\"':
        seq_buf[i++] = '\"';
        break;

      case 's':
        seq_buf[i++] = ' ';
        break;

      case 'x':
        seq_buf[i++] = '\021';
        break;

      case 'c':
        no_cr = 1;
        break;

      case 'd': {                            /* send what we have & then */
        seq_buf[i] = '\0';                   /* expect to send rest after */
        no_cr = 1;                           /* sender delays a little */
        s++;
        return ( 1 );
      }

      case 'w': {                            /* wait count */
        EXP_ALRM = 15;                       /* default to 15 sec */
        if (isdigit(*( s + 1 )))
        {
          EXP_ALRM = ( *( ++s )) & 15;
          if (isdigit(*( s + 1 )))
          {
            EXP_ALRM = EXP_ALRM * 10 + (( *( ++s )) & 15 );
          }
        }

        break;
      }

      default:
        if (isdigit(c))                      /* octal character */
        {
          oct_char = ( c & 7 );              /* most significant digit */
          if (isdigit(*( s + 1 )))
          {
            oct_char = ( oct_char << 3 ) | (( *( ++s )) & 7 );
            if (isdigit(*( s + 1 )))
            {
              oct_char = ( oct_char << 3 ) | (( *( ++s )) & 7 );
            }
          }

          seq_buf[i++] = oct_char;
          break;
        }
      }
    }
    else
    {
      seq_buf[i++] = *s;                     /* plain old character */
    }

    s++;
  }

  seq_buf[i] = '\0';
  return ( 0 );                              /* end of space, still return */
}

/*
 * Receive sequence -- see if expected
 * response comes return success
 * (or failure) in got_it
 */

static
recvSeq()
{
  char *e, got[7], trace[SBUFL];
  int i, l;

  sequenc();
  l = strlen(
    e = seq_buf);                            /* not >7 chars allowed */
  if (l > 7)
  {
    e += l - 7;
    l = 7;
  }

  tlog(F111, "expecting sequence", e, (long)l);
  if (l == 0)                                /* null sequence, delay */
  {
    sleep(NULL_EXP);
    got_it = 1;
    tlog(F100, "got it (null sequence)", "", 0l);
    return;
  }

  *trace = '\0';
  for (i = 0; i < 7; i++)
  {
    got[i] = '\0';
  }

  signal(SIGALRM, scrtime);                  /* did we get it? */
  if (!setjmp(alrmRng))                      /* not timed out yet */
  {
    alarm(EXP_ALRM);
    while (!got_it)
    {
      for (i = 0; i < ( l - 1 ); i++)
      {
        got[i] = got[i + 1];                 /* shift over one */
      }

      got[l - 1] = ttinc(0) & 0177;          /* next char */
      if (seslog)                            /* Log in session log */
      {
        zchout(ZSFILE, got[l - 1]);
      }

      if (strlen(trace) < sizeof ( trace ) - 2)
      {
        strcat(trace, chstr(got[l - 1]));
      }

      got_it = ( !strncmp(seq_buf, got, l));
    }
  }
  else
  {
    got_it = 0;                              /* timed out here */
  }

  alarm(0);
  signal(SIGALRM, SIG_IGN);
  tlog(F110, "received sequence: ", trace, 0l);
  tlog(F101, "returning with got-it code", "", (long)got_it);
  return;
}

/*
 * Output a sequence starting at
 * pointer s, return 0 if okay, 1
 * if failed to read (modem hangup,
 * or whatever)
 */

static int
outSeq()
{
  char *sb;
  int l;
  int delay;
  int retCode = 0;

  while (1)
  {
    delay = sequenc();
    l = strlen(seq_buf);
    tlog(F111, "sending sequence ", seq_buf, (long)l);
    signal(SIGALRM, scrtime);
    if (!setjmp(alrmRng))
    {
      alarm(SND_ALRM);
      if (!strcmp(seq_buf, "EOT"))
      {
        ttoc(dopar('\004'));
        if (seslog && duplex)
        {
          zsout(ZSFILE, "{EOT}");
        }
      }
      else if (!strcmp(seq_buf, "BREAK"))
      {
        ttsndb();
        zsout(ZSFILE, "{BREAK}");
      }
      else
      {
        if (l > 0)
        {
          for (sb = seq_buf; *sb; sb++)
          {
            *sb = dopar(*sb);
          }

          ttol(seq_buf, l);                  /* with parity */
          if (seslog && duplex)
          {
            zsout(ZSFILE, seq_buf);
          }
        }

        if (!no_cr)
        {
          ttoc(dopar('\r'));
          if (seslog && duplex)
          {
            zchout(ZSFILE, dopar('\r'));
          }
        }
      }
    }
    else
    {
      retCode |= -1;                         /* else -- alarm rang */
    }

    alarm(0);
    signal(SIGALRM, SIG_IGN);
    if (!delay)
    {
      return ( retCode );
    }

    msleep(DEL_MSEC);                        /* delay/loop to next to send */
  }
}

/* L O G I N -- Login to remote system */

login(cmdstr)
char *cmdstr;
{
  SIGTYP (*saveAlm)();                    /* save incoming alarm function */
  char *e;

  s = cmdstr;                             /* make global to ckuscr.c */

  tlog(F100, loginv, "", 0l);
  if (!local)
  {
    printf("You must 'set line' first\n");
    return ( -2 );
  }

  if (speed < 0)
  {
    printf("You must 'set speed' first\n");
    return ( -2 );
  }

  if (ttopen(ttname, &local, mdmtyp) < 0)
  {
    sprintf(seq_buf, "Can't open %s", ttname);
    perror(seq_buf);
    return ( -2 );
  }

  if (!quiet)
  {
    printf("Executing script thru %s, speed %d.\n", ttname, speed);
  }

  *seq_buf = 0;
  for (e = s; *e; e++)
  {
    strcat(seq_buf, chstr(*e));
  }

  if (!quiet)
  {
    printf("Script string: %s\n", seq_buf);
  }

  tlog(F110, "Script string: ", seq_buf, 0l);

  /*
   * Condition console terminal
   * and communication line
   */

  if (ttvt(speed, flow) < 0)
  {
    printf("Can't condition communication line\n");
    return ( -2 );
  }

  saveAlm = signal(SIGALRM, SIG_IGN); /* save timer interrupt value */
  flushi();                           /* flush stale input */

  /*
   * Start expect,
   * Send sequence
   */

  while (*s)                          /* while not done with buffer */
  {
    while (*s && isspace(*s))
    {
      s++;                            /* skip over separating whitespaces */
    }
    got_it = 0;                       /* gather up expect sequence */
    recvSeq();

    while (!got_it)                   /* no, is there a conditional send */
    {
      if (*s++ != '-')
      {
        goto failRet;                 /* no -- return failure */
      }

      /*
       * start of
       * conditional send
       */

      flushi();                       /* flush out input buffer */
      if (outSeq())
      {
        goto failRet;                 /* if unable to send! */
      }

      if (*s++ != '-')
      {
        goto failRet;                 /* must have condit respon.*/
      }

      recvSeq();
    }                                 /* loop back and check got_it */

    while (*s && !isspace(*s++))
    {
      ;                               /* Skip over conditionals */
    }
    while (*s && isspace(*s))
    {
      s++;                            /* Skip over separating whitespaces */
    }
    flushi();                         /* Flush */
    if (*s)
    {
      if (outSeq())
      {
        goto failRet;                 /* If any */
      }
    }
  }
  signal(SIGALRM, saveAlm);
  if (!quiet)
  {
    printf("Script successful.\n");
  }

  tlog(F100, "Script successful.", "", 0l);
  return ( 0 );

failRet:
  signal(SIGALRM, saveAlm);
  printf("Script failed\n");
  tlog(F100, "Script failed", "", 0l);
  return ( -2 );
}

/* C H S T R -- Make printable string from a character */

static char *
chstr(c)
char c;
{
  static char sc[4];

  if (c < SP)
  {
    sprintf(sc, "^%c", ctl(c));
  }
  else
  {
    sprintf(sc, "%c", c);
  }

  return ( sc );
}

/* F L U S H I -- Flush, but log, input buffer */

flushi()
{
  int n;

  if (seslog)                         /* Logging session? */
  {
    n = ttchk();                      /* Yes, anything in buffer? */
    if (n > 0)                        /* If so, */
    {
      if (n > SBUFL)
      {
        n = SBUFL;                    /* make sure not too much, */
      }

      n = ttxin(n, fls_buf);          /* then read it, */
      zsout(ZSFILE, fls_buf);         /* and log it. */
    }
  }
  else
  {
    ttflui();                         /* Otherwise just flush. */
  }
}
#endif /* ifndef NOCKUSCR */
