#ifndef NOICP
#ifndef NOCONN
char *connv = "   Connect, 4G(055)";
#endif /* ifndef NOCONN */
#endif /* ifndef NOICP */

/* C K U C O N -- Dumb terminal connection to remote system */

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

#include "ckcdeb.h"
#include "ckcker.h"
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <setjmp.h>

#ifndef SIGUSR1
#define SIGUSR1 16
#endif /* ifndef SIGUSR1 */

#ifdef __linux__
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif /* ifdef __linux__ */

#ifndef NOCONN
extern int local, speed, escape, duplex;
extern int parity, flow, seslog, mdmtyp;
extern int errno, cmask, fmask;
extern char ttname[], sesfil[];
extern CHAR dopar();
static int quitnow = 0;
static int dohangup = 0;

int i, active;                       /* Variables global to this module */
int io_retry = 0;
char *chstr();
char temp[50];

#ifdef MINBUF
#define LBUFL 48
#else /* ifdef MINBUF */
#define LBUFL 100                    /* Line buffer */
#endif /* ifdef MINBUF */
char lbuf[LBUFL];                    /* XXX(jhj): 200 stock lbuf */

static jmp_buf env_con;              /* Envir ptr for connect errors */

SIGTYP
conn_int()                           /* Modem read failure handler, */
{
  longjmp(env_con, 1);               /* notifies parent process to stop */
}

/* C O N E C T -- Perform terminal connection */

int
conect()
{
  int pid,                           /* process id of child: modem reader */
    parent_id,                       /* process id of parent: kbd. reader */
    n;
  int c;                             /* c is a character, but is a signed */
                                     /* int to pass thru -1, which is the */
                                     /* modem disconnection signal and is */
                                     /* different from the character 0377 */
  char errmsg[50], *erp;

  if (!local)
  {
    printf("'set line' first\n");
    return ( -2 );
  }

  if (speed < 0)
  {
    printf("'set speed' first\n");
    return ( -2 );
  }

  if (( escape < 0 ) || ( escape > 0177 ))
  {
    printf("not ASCII - %d\n", escape);
    return ( -2 );
  }

  if (ttopen(ttname, &local, mdmtyp) < 0)
  {
    erp = errmsg;
    sprintf(erp, "Can't open %s", ttname);
    perror(errmsg);
    return ( -2 );
  }

  dohangup = 0;
  printf(
    "[Connecting on %s, speed %d]\r\n",
      ttname, speed);
  printf(
    "[Escape character %s (%d) + '?' for help]\r\n",
      chstr(escape), escape);
  if (seslog)
  {
    printf("[Logging to %s]\r\n", sesfil);
  }

  /*
   * Condition console terminal
   * and communication line
   */

  if (conbin(escape) < 0)
  {
    printf("Can't condition terminal\n");
    return ( -2 );
  }

  if (ttvt(speed, flow) < 0)
  {
    conres();
    printf("Can't condition line\n");
    return ( -2 );
  }

  parent_id = getpid();              /* Get parent id for signalling */
  signal(SIGUSR1, SIG_IGN);          /* Don't kill parent */
  pid = fork();                      /* All ok, make a fork */
  if (pid == -1)
  {
    conres();                        /* Reset the console. */
    perror("kybrd fork failed");
    printf(
      "[Back Local]\n");
    return ( 0 );
  }

  io_retry = 0;
  if (pid)
  {
    active = 1;                      /* This fork reads, sends keystrokes */
    if (!setjmp(env_con))            /* comm error in child process */
    {
      signal(SIGUSR1, conn_int);     /* routine for child process exit */
      while (active)
      {
        c = coninc(0) & cmask;       /* Get character from keyboard */
        if (( c & 0177 ) == escape)  /* Look for escape char */
        {
          c = coninc(0) & 0177;      /* Got esc, get its arg */
          doesc(c);                  /* And process it */
        }
        else                         /* Ordinary character */
        {
          if (ttoc(dopar(c)) > -1)
          {
            if (duplex)              /* Half duplex? */
            {
              conoc(c);              /* Yes, also echo it. */
              if (seslog)            /* And maybe log it. */
              {
                if (zchout(ZSFILE, c) < 0)
                {
                  seslog = 0;
                }
              }
            }
          }
          else
          {
            perror("\r\nCan't send char");
            active = 0;
          }
        }
      }
    }                                /* Come here on death of child */

    kill(pid, 9);                    /* Done, kill inferior fork. */
    wait((int *)0);                  /* Wait till gone. */
    conres();                        /* Reset the console. */
    if (quitnow)
    {
      doexit(GOOD_EXIT);
    }

    if (dohangup)
    {
      tthang();
    }

    printf("\r[Back Local]\n");
    return ( 0 );
  }
  else                               /* Inferior reads, prints port input */
  {
    sleep(1);                        /* Wait for parent's handler setup */
    while (1)                        /* Fresh read, wait for a character */
    {
      if (( c = ttinc(0)) < 0)       /* Comm line hangup detected */
      {
        if (errno == 9999)           /* this value set by myread() */
        {
          printf("\r\nComm disconnect ");
        }
        else if (io_retry++ < 3)
        {
          tthang();
          continue;
        }

        if (errno != 9999)
        {
          perror("\r\nCan't get char");
        }

        kill(parent_id, SIGUSR1);    /* notify parent. */
        pause();                     /* Wait to be killed by parent. */
      }

      c &= cmask;                    /* Got a char, strip parity, etc */
      conoc(c);                      /* Put it on the screen. */
      if (seslog)
      {
        zchout(ZSFILE, c);           /* If logging, log it. */
      }

      while (( n = ttchk()) > 0)     /* Any more left in buffer? */
      {
        if (n > LBUFL)
        {
          n = LBUFL;                 /* Get them all at once. */
        }

        if (( n = ttxin(n, lbuf)) > 0)
        {
          for (i = 0; i < n; i++)
          {
            lbuf[i] &= cmask;        /* Strip */
          }

          conxo(n, lbuf);            /* Output */
          if (seslog)
          {
            zsoutx(ZSFILE, lbuf, n); /* Log */
          }
        }
      }
    }
  }
}

/* H C O N N E -- Give help message for connect */

char
hconne()
{
  int c;
  static char *hlpmsg[] = {
    "\r\n",
/*  "\r\n Escape character twice to send it,\r\n", */
    "\r\n c: close connection",
    "\r\n 0: send NULL",
    "\r\n b: send BREAK",
    "\r\n h: hangup",
    "\r\n q: hangup & quit",
    "\r\n s: status",
    "\r\n ?: help",
    "\r\n\r\n",
    ""
  };

  conola(hlpmsg);                    /* Print the help message. */
  conol(">");                        /* Prompt for command. */
  c = coninc(0) & 0177;              /* Get character, strip any parity. */
  conoc(c);                          /* Echo it. */
  conoll("");
  return ( c );                      /* Return it. */
}

/* C H S T R -- Make a printable string out of a character */

char *
chstr(c)
int c;
{
  static char s[8];
  char *cp = s;

  if (c < SP)
  {
    sprintf(cp, "CTRL-%c", ctl(c));
  }
  else
  {
    sprintf(cp, "'%c'\n", c);
  }

  cp = s;
  return ( cp );
}

/* D O E S C -- Process an escape character argument */

void
doesc(c)
char c;
{
  CHAR d;

  while (1)
  {
    if (c == escape)                 /* Send escape character */
    {
      d = dopar(c);
      ttoc(d);
      return;
    }
    else                             /* Or else look it up below. */
    if (isupper(c))
    {
      c = tolower(c);
    }

    switch (c)
    {
    case 'c':                        /* Close connection */
    case '\03':
      active = 0;
      conol("\r\n");
      return;

    case 'b':                        /* Send a BREAK signal */
    case '\02':
      ttsndb();
      return;

    case 'h':                        /* Hangup */
    case '\010':
      dohangup = 1;
      active = 0;
      conol("\r\n");
      return;

    case 'q':
      quitnow = 1;
      active = 0;
      conol("\r\n");
      return;

    case 's':                        /* Status */
      conol("\r\nConnected thru ");
      conol(ttname);
      if (speed >= 0)
      {
        sprintf(temp, ", speed %d", speed);
        conol(temp);
      }

      sprintf(temp, ", %d bits", ( cmask == 0177 ) ? 7 : 8);
      if (parity)
      {
        conol(", ");
        switch (parity)
        {
        case 'e':
          conol("even");
          break;

        case 'o':
          conol("odd");
          break;

        case 's':
          conol("space");
          break;

        case 'm':
          conol("mark");
          break;
        }
        conol(" parity");
      }

      if (seslog)
      {
        conol(", logging to ");
        conol(sesfil);
      }

      conoll("");
      return;

    case '?':                        /* Help */
      c = hconne();
      continue;

    case '0':                        /* Send a null */
      c = '\0';
      d = dopar(c);
      ttoc(d);
      return;

    case SP:                         /* Space, ignore */
      return;

    default:                         /* Other */
      conoc(BEL);
      return;                        /* Invalid esc arg, beep */
    }
  }
}
#endif /* ifndef NOCONN */
