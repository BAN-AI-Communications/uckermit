#ifndef NOICP
char *ckxv = "    TTY IO, 4G(089)";
#endif /* ifndef NOICP */

/* C K U T I O */

/* uCKermit interrupt, terminal control & I/O functions for UNIX systems */

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

#include <sys/types.h> /* Types */
#include <ctype.h>     /* Character types */
#include <sys/dir.h>   /* Directory */

#ifdef NULL
#undef NULL
#endif /* ifdef NULL */

#include <signal.h>    /* Interrupts */
#include <stdio.h>     /* UNIX Standard i/o */

#include <setjmp.h>    /* Longjumps */

#include "ckcdeb.h"    /* Typedefs, formats for debug() */

#ifdef __linux__
#include <string.h>
#endif /* ifdef __linux__ */

#ifndef DEVNAMLEN
#define DEVNAMLEN 25
#endif /* ifndef DEVNAMLEN */

#ifndef NOICP
#ifdef BSD4
#define ANYBSD
#ifdef MAXNAMLEN
#define BSD42
#ifdef SUNOS4
char *ckxsys = " SUNOS 4.x";
#else  /* ifdef SUNOS4 */
#ifdef ultrix
char *ckxsys = " VAX/Ultrix";
#else  /* ifdef ultrix */
#ifdef BSD43
char *ckxsys = " 4.3 BSD";
#else  /* ifdef BSD43 */
char *ckxsys = " 4.2 BSD";
#endif /* bsd43 */
#endif /* ultrix */
#endif /* sunos4 */
#else  /* ifdef MAXNAMLEN */
#define BSD41
char *ckxsys = " 4.1 BSD";
#endif /* maxnamlen */
#endif /* bsd4 */
#ifdef BSD29
#define ANYBSD
char *ckxsys = " 2.9 BSD";
#endif /* bsd29 */
#ifdef V7
char *ckxsys = " Version 7 UNIX (tm)";
#endif /* v7 */
#ifdef UXIII
#ifdef XENIX
#ifdef M_I386
char *ckxsys = " Xenix/386";
#else  /* ifdef M_I386 */
#ifdef M_I286
char *ckxsys = " Xenix/286";
#else  /* ifdef M_I286 */
char *ckxsys = " Xenix/86";
#endif /* ifdef M_I286 */
#endif /* ifdef M_I386 */
#else  /* ifdef XENIX */
#ifdef ISIII
char *ckxsys = " Interactive Systems Corp System III";
#else  /* ifdef ISIII */
#ifdef hpux
char *ckxsys = " HP 9000 Series HP-UX";
#else  /* ifdef hpux */
#ifdef __linux__
char *ckxsys = " GNU/Linux";
#else  /* ifdef __linux__ */
char *ckxsys = " AT&T System III/System V";
#endif /* linux */
#endif /* hpux  */
#endif /* isiii */
#endif /* xenix */
#endif /* uxiii */
#endif /* ifndef NOICP */

/*
 * Where is the
 * UUCP lock file?
 */

#ifdef NEWUUCP
#define LCKDIR
#endif /* ifdef NEWUUCP */

/*
 * Coordinate with
 * HoneyDanBer UUCP?
 */

#ifdef ATT3BX
#define HDBUUCP
#endif /* ifdef ATT3BX */

/*
 * (PWP) if LOCK_DIR is already
 * defined, we don't change it
 */

#ifndef LOCK_DIR
#ifdef ISIII
#define LOCK_DIR "/etc/locks";
#else  /* ifdef ISIII */
#ifdef HDBUUCP
#define LOCK_DIR "/usr/spool/locks";
#else  /* ifdef HDBUUCP */
#ifdef LCKDIR
#define LOCK_DIR "/usr/spool/uucp/LCK";
#else  /* ifdef LCKDIR */
#define LOCK_DIR "/usr/spool/uucp";
#endif /* LCKDIR */
#endif /* HDBUUCP */
#endif /* ISIII */
#endif /* !LOCK_DIR (outside ifndef) */

/*
 * Do own buffering, using
 * unbuffered read() calls...
 */

#ifdef UXIII
#define MYREAD
#endif /* uxiii */

#ifdef BSD42
#undef MYREAD
#include <errno.h>
#endif /* bsd42 */

/*
 * Variables available to outside world:
 *
 * dftty  -- Pointer to default tty name string, like "/dev/tty".
 * dfloc  -- 0 if dftty is console, 1 if external line.
 * dfprty -- Default parity
 * dfflow -- Default flow control
 * ckxech -- Flag for who echoes console typein:
 *  1 - The program (system echo is turned off)
 *  0 - The system (or front end, or terminal).
 *  functions that want to do their own echoing should check this flag
 *  before doing so.
 *
 * flfnam -- Name of lock file, including its path, e.g.,
 *              "/usr/spool/uucp/LCK..cul0" or "/etc/locks/tty77"
 * hasLock -- Flag set if this kermit established a uucp lock.
 * inbufc -- number of tty line rawmode unread characters
 *              (system III/V unixes)
 * backgrd -- Flag indicating program executing in background ( & on
 *              end of shell command). Used to ignore INT and QUIT signals.
 * rtu_bug -- Set by stptrap().  RTU treats ^Z as EOF.
 *              (but only when we handle SIGTSTP)
 *
 * Functions for assigned communication line
 *              (either external or console tty):
 *
 *  sysinit()               -- System dependent program initialization
 *  syscleanup()            -- System dependent program shutdown
 *  ttopen(ttname,local,mdmtyp) -- Open the named tty for exclusive access.
 *  ttclos()                -- Close & reset the tty,
 *                               releasing any access lock.
 *  ttpkt(speed,flow)       -- Put the tty in packet mode and set the speed.
 *  ttvt(speed,flow)        -- Put the tty in virtual terminal mode.
 *                               or in DIALING or CONNECTED modem
 *                                 control state.
 * ttinl(dest,max,timo)    -- Timed read line from the tty.
 * ttinc(timo)             -- Timed read character from tty.
 * myread()                -- System 3 raw mode bulk buffer read, gives
 *                              subsequent chars one at a time and simulates
 *                                FIONREAD!
 * myunrd(c)               -- Places c back in buffer to be read (one only)
 * ttchk()                 -- See how many characters in tty input buffer.
 * ttxin(n,buf)            -- Read n characters from tty (untimed).
 * ttol(string,length)     -- Write a string to the tty.
 * ttoc(c)                 -- Write a character to the tty.
 * ttflui()                -- Flush tty input buffer.
 *
 * ttlock(ttname)          -- Lock against uucp collisions (Sys III)
 * ttunlck()               -- Unlock "       "     "
 * look4lk(ttname)         -- Check if a lock file exists
 *
 *                                 For ATT7300/UNIX PC, Sys III, Sys V:
 * attdial(ttname,speed,telnbr) -- dials ATT7300/UNIX PC internal modem
 * atthang(ttname)         -- Hangs up internal modem for ATT7300's
 * offgetty(ttname)        -- Turns off getty(1m) for comms line
 * ongetty(ttname)         -- Restores getty() to comms line
 */

/*
 * Functions for console terminal:
 *
 * congm()    - Get console terminal modes.
 * concb(esc) - Put the console in single-character wakeup mode without echo
 * conbin(esc) - Put the console in binary (raw) mode.
 * conres()  - Restore the console to mode obtained by congm().
 * conoc(c)  - Unbuffered output, one character to console.
 * conol(s)  - Unbuffered output, null-terminated string to the console.
 * conola(s) - Unbuffered output, array of strings to the console.
 * conxo(n,s) - Unbuffered output, n characters to the console.
 * conchk()  - Check if characters available at console (bsd 4.2).
 *             Check if escape char (^\) typed at console (System III/V).
 * coninc(timo)  - Timed get a character from the console.
 * conint()  - Enable terminal interrupts on the console if not background.
 * connoi()  - Disable terminal interrupts on the console if not background.
 *
 * Time functions
 *
 * msleep(m) -- Millisecond sleep
 * ztime(&s) -- Return pointer to date/time string
 * rtimer() --  Reset timer
 * gtimer()  -- Get elapsed time since last call to rtimer()
 */

/*
 * Whether to include
 * <sys/file.h>...
 */

#ifndef XENIX
#ifndef unos
#include <sys/file.h>             /* File information */
#endif /* ifndef unos */
#endif /* ifndef XENIX */

#ifdef BSD4
#include <fcntl.h>
#include <sys/file.h>
#endif /* ifdef BSD4 */

/*
 * System III
 * or System V
 */

#ifdef UXIII
#include <termio.h>
#include <sys/ioctl.h>
#include <errno.h>                /* error numbers for system returns */
#include <fcntl.h>                /* directory reading for locking */
#endif /* ifdef UXIII */

#ifdef HPUX
#include <sys/modem.h>
#endif /* ifdef HPUX */

/*
 * Not System III
 * nor System V
 */

#ifndef UXIII
#include <sgtty.h>                /* Set/Get tty modes */
#ifndef V7
#ifndef BSD41
#include <sys/time.h>             /* Clock info (for break generation) */
#endif /* ifndef BSD41 */
#endif /* ifndef V7 */
#endif /* ifndef UXIII */

#ifdef BSD41
#include <sys/timeb.h>            /* BSD 4.1 ... ceb */
#endif /* ifdef BSD41 */

#ifdef BSD29
#include <sys/timeb.h>            /* BSD 2.9 (Vic Abell, Purdue) */
#endif /* ifdef BSD29 */

#ifdef ultrix
#include <sys/ioctl.h>
#endif /* ifdef ultrix */

/*
 * The following two conditional #defines
 * are catch-alls for those systems that
 * didn't have or couldn't find <file.h>
 */

#ifndef FREAD
#define FREAD 0x01
#endif /* ifndef FREAD */

#ifndef FWRITE
#define FWRITE 0x10
#endif /* ifndef FWRITE */

long time();                      /* All UNIXes should have this... */
extern int errno;                 /* System call error code. */

/*
 * Special stuff for V7
 * input buffer peeking
 */

#ifdef V7
int kmem[2] = {
  -1, -1
};
char *initrawq(), *qaddr[2] =
{
  0, 0
};
#define CON 0
#define TTY 1
#endif /* ifdef V7 */

/*
 * dftty is the device name of the default
 * device for file transfer. dfloc is 0 if
 * dftty is the user's console terminal,
 * and 1 if an external line
 */

char *dftty = CTTNAM;             /* Remote by default, use normal */
int dfloc = 0;                    /* controlling terminal name. */
int dfprty = 0;                   /* Default parity (0 = none) */
int ttprty = 0;                   /* Parity in use. */
int ttmdm = 0;                    /* Modem in use. */
int dfflow = 1;                   /* Xon/Xoff flow control */
int backgrd = 0;                  /* Assume in foreground (no '&' ) */
#ifdef ultrix
int iniflags = 0;                 /* fcntl flags for ttyfd */
#endif /* ifdef ultrix */
int ckxech = 0;                   /* 0 if system normally echoes */
                                  /* console characters, else 1 */
/*
 * Declarations of variables
 * global within this module
 */

#ifndef NOSTATS
static long tcount;               /* Elapsed time counter */
#endif /* ifndef NOSTATS */

static jmp_buf sjbuf, jjbuf;      /* Longjump buffer */
static int lkf = 0,               /* Line lock flag */
  conif = 0,                      /* Console interrupts on/off flag */
  cgmf = 0,                       /* Flag that console modes saved */
  xlocal = 0,                     /* Flag for tty local or remote */
  ttyfd = -1;                     /* TTY file descriptor */
static char escchr;               /* Escape or attn character */

#ifdef BSD42
#include <time.h>
#include <sys/time.h>
static struct timeval tv;         /* For getting time, from sys/time.h */
static struct timezone tz;
#endif /* ifdef BSD42 */

#ifdef BSD29
static long clock;                /* For getting time from sys/time.h */
static struct timeb ftp;          /* And from sys/timeb.h */
#endif /* ifdef BSD29 */

#ifdef BSD41
static long clock;                /* For getting time from sys/time.h */
static struct timeb ftp;          /* And from sys/timeb.h */
#endif /* ifdef BSD41 */

#ifdef V7
static long clock;
#endif /* ifdef V7 */

/*
 * sgtty/termio/termios
 * information...
 */

#ifdef UXIII
static struct termio ttold = {
  0
};                                /* Init'd for word alignment, */
static struct termio ttraw = {
  0
};                                /* which is important for some */
static struct termio tttvt = {
  0
};                                /* systems, like Zilog... */
static struct termio ccold = {
  0
};
static struct termio ccraw = {
  0
};
static struct termio cccbrk = {
  0
};
#else  /* ifdef UXIII */
static struct sgttyb                /* sgtty info... */
  ttold, ttraw, tttvt,  ttbuf,
  ccold, ccraw, cccbrk, vanilla;
#endif /* ifdef UXIII */

static char flfnam[80];             /* UUCP lock file path name */
static int hasLock = 0;             /* =1 if this kermit locked uucp */
static int inbufc  = 0;             /* stuff for efficient SIII raw line */
static int ungotn  = -1;            /* pushback to unread character */
static int conesc  = 0;             /* set to 1 if esc char (^\) typed */
static int ttlock();                /* definition of ttlock subprocedure */
static int ttunlck();               /* and unlock subprocedure */
static char ttnmsv[DEVNAMLEN + 1];  /* copy of open path for tthang */

/* S Y S I N I T -- System-dependent program initialization */

sysinit()
{
#ifdef ultrix
  gtty(0, &vanilla);                 /* Get sgtty info */
  iniflags = fcntl(0, F_GETFL, 0);   /* Get flags */
#else  /* ifdef ultrix */
#ifdef AUX
  set42sig();                        /* Don't ask! (hakanson@cs.orst.edu) */
#endif /* aux */
#endif /* ifdef ultrix */
  return ( 0 );
}

/* S Y S C L E A N U P -- System-dependent program cleanup */

syscleanup()
{
#ifdef ultrix
  stty(0, &vanilla);                 /* Get sgtty info */
  fcntl(0, F_SETFL, iniflags);       /* Restore flags */
#endif /* ifdef ultrix */
  return ( 0 );
}

/* T T O P E N -- Open a tty for exclusive access */

/*
 * Returns 0 on success, -1 on failure
 *
 *  If called with lcl < 0, sets value of lcl as follows:
 *  0: the terminal named by ttname is the job's controlling terminal.
 *  1: the terminal named by ttname is not the job's controlling terminal.
 *  But watch out: if a line is already open, or if requested line can't
 *  be opened, then lcl remains (and is returned as) -1.
 */

ttopen(ttname, lcl, modem)
char *ttname;
int *lcl, modem;
{
#ifdef UXIII
  char *ctermid();                   /* Wish they all had this! */
#endif /* ifdef UXIII */

  char *x;
  extern char *ttyname();
  char cname[DEVNAMLEN + 4];

  if (ttyfd > -1)                    /* if comms line already opened */
  {
    if (strncmp(
      ttname, ttnmsv, DEVNAMLEN))    /* are new & old names equal? */
    {
      ttclos();                      /* no, close old ttname, open new */
    }
    else                             /* else same, ignore this call */
    {
      return ( 0 );
    }
  }

  ttmdm = modem;                     /* Make this available to other fns */
  xlocal = *lcl;                     /* Make this available to other fns */
#ifdef NEWUUCP
  acucntrl("disable", ttname);       /* Open getty on line (4.3BSD) */
#endif /* ifdef NEWUUCP */

/*
 * In the following section, we open the tty device for read/write.
 * If a modem has been specified via "set modem" prior to "set line"
 * then the O_NDELAY parameter is used in the open, provided this symbol
 * is defined (e.g. in fcntl.h), so that the program does not hang waiting
 * for carrier (which in most cases won't be present because a connection
 * has not been dialed yet).  It would make more sense to first determine
 * if the line is local before doing this, but because ttyname() requires a
 * file descriptor, we have to open first.
 */

#ifdef UXIII
  ttyfd = open(ttname, O_RDWR | ( modem ? O_NDELAY : 0 ));
#else /* not uxiii */
#ifdef O_NDELAY
  ttyfd = open(ttname, O_RDWR | ( modem ? O_NDELAY : 0 ));
#else /* O_NDELAY not defined */
  ttyfd = open(ttname, 2);
#endif /* O_NDELAY */
#endif /* uxiii */
  debug(F111, "ttopen", "modem", modem);
  debug(F101, " ttyfd", "", ttyfd);

  if (ttyfd < 0)                     /* If couldn't open, fail. */
  {
    perror(ttname);
    return ( -1 );
  }

  strncpy(
    ttnmsv, ttname, DEVNAMLEN);      /* Open, keep copy of name locally. */

  /*
   * Caller wants us to figure out
   * if line is controlling tty
   */

  debug(F111, "ttopen ok", ttname, *lcl);
  if (*lcl != 0)
  {
    if (strcmp(ttname, CTTNAM) == 0) /* /dev/tty is always remote */
    {
      xlocal = 0;
      debug(F111, " ttname=CTTNAM", ttname, xlocal);
    }
    else if (isatty(0))              /* Else, if stdin not redirected */
    {
      x = ttyname(0);                /* then compare its device name */
      strncpy(cname, x, DEVNAMLEN);  /* (copy from internal static buf) */
#ifdef __linux__
      x = cname;
#else  /* ifdef __linux__ */
      x = ttyname(ttyfd);            /* ...with real name of ttname. */
#endif /* ifdef __linux__ */
      if (x == NULL)
      {
        return;
      }

#ifdef __linux__
      xlocal = 1;
#else  /* ifdef __linux__ */
      xlocal = ( strncmp(x, cname, DEVNAMLEN) == 0 ) ? 0 : 1;
#endif /* ifdef __linux__ */
      debug(F111, " ttyname", x, xlocal);
    }
    else                             /* Else, if stdin redirected... */
    {
#ifdef UXIII

      /*
       * Sys III/V provides nice ctermid() function
       * to get name of controlling tty
       */

      ctermid(cname);                /* Get name of controlling terminal */
      debug(F110, " ctermid", cname, 0);
      x = ttyname(ttyfd);            /* Compare with name of comm line. */
      xlocal = ( strncmp(x, cname, DEVNAMLEN) == 0 ) ? 0 : 1;
      debug(F111, " ttyname", x, xlocal);
#else  /* ifdef UXIII */

      /*
       * Just assume local, so "set speed"
       * and similar commands will work
       * If not really local, how could
       * it work anyway?...
       */

      xlocal = 1;
      debug(F101, " redirected stdin", "", xlocal);
#endif /* uxiii */
    }
  }

  /*
   * Note, the following code was added so that UNIX "idle-line" snoopers
   * would not think Kermit was idle when it was transferring files, and
   * maybe log people out.  But it had to be removed because it broke
   * one of UNIX Kermit's very useful features, sending from standard input,
   * as in "kermit -s - < file" or "command | kermit -s -".  Too bad...
   *
   * If we're not local, close the ttyfd and just use 0
   */

  /*
   *    if (xlocal == 0) {
   *      close(ttyfd);
   *      ttyfd = 0;
   *    }
   */

  /*
   * Now check if line is locked --
   * if so fail, else lock for ourselves
   */

  lkf = 0; /* Check lock */
  if (xlocal > 0)
  {
    if (ttlock(ttname) < 0)
    {
      fprintf(stderr, "Exclusive access to %s denied\n", ttname);
      close(ttyfd);
      ttyfd = -1;
      debug(F110, " Access denied by lock", ttname, 0);
      return ( -1 );                 /* Not if already locked */
    }
    else
    {
      lkf = 1;
    }
  }

  /*
   * Got the line, now set the
   * desired value for local.
   */

  if (*lcl != 0)
  {
    *lcl = xlocal;
  }

  /*
   * Some "special"
   * stuff for UNIX v7
   */

#ifdef V7
  if (kmem[TTY] < 0)                 /*  If open, then skip this.  */
  {
    qaddr[TTY] = initrawq(ttyfd);    /* Init the queue. */
    if (( kmem[TTY] = \
      open("/dev/kmem", 0)) < 0)
    {
      fprintf(
        stderr,
          "Can't read /dev/kmem in ttopen.\n");
      perror("/dev/kmem");
      exit(1);
    }
  }
#endif /* ifdef V7 */

  /*
   * no failure returns
   * after this point
   */

#ifndef XENIX
#ifndef unos
#ifdef TIOCEXCL
  if (xlocal)
  {
    if (ioctl(ttyfd, TIOCEXCL, NULL) < 0)
    {
#ifndef NODOHLP
      fprintf(
        stderr,
          "Warning, problem getting exclusive access\n");
#else /* NODOHLP */
      fprintf(
        stderr,
          "TIOCEXCL failed\n"); 
#endif /* NODOHLP */
    }
  }

#endif /* ifdef TIOCEXCL */
#endif /* ifndef unos */
#endif /* ifndef XENIX */

#ifdef ultrix
  if (xlocal)
  {
    int temp = 0;
#ifdef TIOCSINUSE
    if (ioctl(ttyfd, TIOCSINUSE, NULL) < 0)
    {
      fprintf(stderr, "Can't set in-use flag on modem.\n");
      perror("TIOCSINUSE");
    }

#endif /* ifdef TIOCSINUSE */
    if (modem)
    {
      ioctl(ttyfd, TIOCMODEM, &temp);
      ioctl(ttyfd, TIOCHPCL, 0);
    }
    else
    {
      ioctl(ttyfd, TIOCNMODEM, &temp);
    }
  }
#endif /* ifdef ultrix */

  /*
   * Get tty
   * device settings
   */

#ifndef UXIII
  gtty(ttyfd, &ttold);               /* Get sgtty info */
  if (xlocal)
  {
    ttold.sg_flags &= ~ECHO;         /* Turn off echo on local line */
  }

  gtty(ttyfd, &ttraw);               /* And a copy of it for packets */
  gtty(ttyfd, &tttvt);               /* And one for virtual tty service */
#else  /* ifndef UXIII */
  ioctl(ttyfd, TCGETA, &ttold);      /* Same deal for Sys III, Sys V */
  if (xlocal)
  {
    ttold.c_lflag &= ~ECHO;          /* Turn off echo on local line. */
  }

  ioctl(ttyfd, TCGETA, &ttraw);
  ioctl(ttyfd, TCGETA, &tttvt);
#endif /* ifndef UXIII */
#ifdef DEBUG
  debug(F101, "ttopen, ttyfd", "", ttyfd);
  debug(F101, " lcl", "", *lcl);
  debug(F111, " lock file", flfnam, lkf);
#endif /* ifdef DEBUG */
  return ( 0 );
}

/* T T C L O S -- Close the TTY, releasing any lock */

#ifdef COMMENT
SIGTYP
ttclosx()   /* To avoid pointer type mismatches */
{
  ttclos();
}
#endif /* ifdef COMMENT */

ttclos()
{
  debug(F101, "ttclos", "", ttyfd);
  if (ttyfd < 0)
  {
    return ( 0 );                    /* Wasn't open. */
  }

#ifdef ultrix
  if (xlocal)
  {
    ioctl(ttyfd, TIOCNCAR, NULL);
  }
#endif /* ifdef ultrix */
  if (xlocal)
  {
    if (ttunlck())                   /* Release UUCP-style lock */
    {
#ifndef NODOHLP
      fprintf(stderr, "Warning, problem releasing lock\r\n");
#else /* ifndef NODOHLP */
      fprintf(stderr, "ttunlck failed\r\n");
#endif /* ifndef NODOHLP */
        }

    if (tthang())                    /* Hang up phone line */
    {
#ifndef NODOHLP
      fprintf(stderr, "Warning, problem hanging up the phone\r\n");
#else /* ifndef NODOHLP */
      fprintf(stderr, "tthang failed\r\n");
#endif /* ifndef NODOHLP */
        }
  }

  ttres();                           /* Reset modes. */

  /*
   * Relinquish exclusive access
   * if we might have had it...
   */

#ifndef XENIX
#ifndef unos
#ifdef TIOCEXCL
#ifdef TIOCNXCL
  if (xlocal)
  {
    if (ioctl(ttyfd, TIOCNXCL, NULL) < 0)
    {
#ifndef NODOHLP
      fprintf(
        stderr,
        "Warning, problem relinquishing exclusive access\r\n");
#else /* ifndef NODOHLP */
      fprintf(
        stderr,
        "TIOCNXCL failed\r\n");
#endif /* ifndef NODOHLP */
        }
  }

#endif /* ifdef TIOCNXCL */
#endif /* ifdef TIOCEXCL */
#endif /* ifndef unos */
#endif /* ifndef XENIX */

  if (ttyfd > 0)
  {
    close(ttyfd);                    /* Close it if tthang didn't */
  }

  ttyfd = -1;                        /* Mark it as closed. */

#ifdef NEWUUCP
  acucntrl("enable", flfnam);        /* Close getty on line. */
#endif /* ifdef NEWUUCP */
  debug(F101, "ttclose exit", "", ttyfd);
  return ( 0 );
}

/* T T H A N G -- Hangup phone line */

tthang()
{
#ifdef UXIII
#ifdef HPUX
  unsigned long dtr_down = 00000000000, modem_rtn;
#else  /* ifdef HPUX */
  unsigned short ttc_save;
#endif /* ifdef HPUX */
#endif /* ifdef UXIII */
#ifdef ANYBSD
  int ttc_save;
#endif /* ifdef ANYBSD */

  if (ttyfd < 0)
  {
    return ( 0 );                             /* Not open. */
  }

  if (xlocal < 1)
  {
    return ( 0 );                             /* Don't do if not local */
  }

#ifdef ANYBSD
#ifdef BSD42
  ttc_save = fcntl(ttyfd, F_GETFL, 0);        /* Get flags */
#endif /* ifdef BSD42 */
  ioctl(ttyfd, TIOCCDTR, 0);                  /* Clear DTR */
  msleep(550);                                /* For about 1/2 sec */
  ioctl(ttyfd, TIOCSDTR, 0);                  /* Restore DTR */
#ifdef COMMENT
  close(ttyfd);                               /* Close/reopen fd */
  if (( ttyfd = open(ttnmsv, ttc_save)) < 0)
  {
    perror(ttnmsv);                           /* If can't, give message */
    ttunlck();                                /* and unlock the file */
    return ( -1 );
  }

#endif /* ifdef COMMENT */
#endif /* ifdef ANYBSD */

#ifdef UXIII
#ifdef HPUX                                   /* HP way of modem control */
  if (ioctl(ttyfd, MCSETAF, &dtr_down) < 0)
  {
    return ( -1 );                            /* lower DTR */
  }

  msleep(550);
  if (ioctl(ttyfd, MCGETA, &modem_rtn) < 0)
  {
    return ( -1 );                            /* get line status */
  }

  if (( modem_rtn & MDCD ) != 0)
  {
    return ( -1 );                            /* check if DCD is low */
  }

  modem_rtn = MRTS | MDTR;                    /* bits for RTS & DTR  */
  if (ioctl(ttyfd, MCSETAF, &modem_rtn) < 0)
  {
    return ( -1 );                            /*  set lines  */
  }

#else  /* ifdef HPUX */
  ttc_save = ttraw.c_cflag;
  ttraw.c_cflag &= ~CBAUD;                    /* baud rate to 0 to hangup */
  if (ioctl(ttyfd, TCSETAF, &ttraw) < 0)
  {
    return ( -1 );                            /* do it */
  }

  msleep(125);                                /* let things settle */
  ttraw.c_cflag = ttc_save;

  /*
   * NOTE - The following #ifndef...#endif
   * can be removed for SCO Xenix 2.1.3
   * or later, but must keep for earlier
   * versions, which can't do close/open.
   */

#ifndef XENIX
  ttc_save = fcntl(ttyfd, F_GETFL, 0);
  (void)ttc_save;
  close(ttyfd);                          /* close/reopen file descriptor */
  ttyfd = -1;                            /* in case reopen fails */
#ifndef UXIII
  if (( ttyfd = open(ttnmsv, ttc_save)) < 0)
  {
    return ( -1 );
  }

#else  /* ifndef UXIII */
  if (( ttyfd = open(ttnmsv, O_RDWR | O_NDELAY)) < 0)
  {
    return ( -1 );
  }

  /*
   * So Kermit hangup command works
   * even if there is no carrier
   */

#endif /* ifndef UXIII */
#endif /* ifndef XENIX */
  if (ioctl(ttyfd, TCSETAF, &ttraw) < 0)
  {
    return ( -1 );                          /* un-do it */
  }

#endif /* ifdef HPUX */
#endif /* ifdef UXIII */
  return ( 0 );
}

/* T T R E S -- Restore terminal to "normal" mode */

ttres()
{                                           /* Restore the tty to normal. */
  int x;

  (void)x;

  if (ttyfd < 0)
  {
    return ( -1 );                          /* Not open. */
  }

#ifndef UXIII                               /* except for sIII, */
  sleep(1);                                 /* Wait for i/o to finish. */
#endif                                      /* (sIII does wait in ioctls) */

#ifdef UXIII
  if (ioctl(ttyfd, TCSETAW, &ttold) < 0)
  {
    return ( -1 );                          /* restore termio stuff */
  }

  if (ttmdm)
  {
    if (fcntl(ttyfd, F_SETFL, fcntl(ttyfd, F_GETFL, 0) & \
      ~O_NDELAY) < 0)
    {
      return ( -1 );
    }
  }

  return ( 0 );

#else  /* ifdef UXIII */
#ifdef MYREAD
#ifdef FIONBIO
  x = 0;
  x = ioctl(ttyfd, FIONBIO, &x);
  if (x < 0)
  {
    perror("ttres ioctl");
    debug(F101, "ttres ioctl", "", x);
  }

#else  /* ifdef FIONBIO */
#ifdef FNDELAY
  x =
    ( fcntl(
        ttyfd,
        F_SETFL,
        fcntl(ttyfd, F_GETFL, 0) & ~FNDELAY) == -1 );
  debug(F101, "ttres ~FNDELAY fcntl", "", x);
  if (x < 0)
  {
    perror("fcntl");
  }

#endif /* ifdef FNDELAY */
#endif /* ifdef FIONBIO */
#endif /* ifdef MYREAD */
  x = stty(ttyfd, &ttold);                  /* Restore sgtty stuff */
  debug(F101, "ttres stty restore", "", x);
  if (x < 0)
  {
    perror("ttres stty");
  }

  return ( x );

#endif /* ifdef UXIII */
}

/*
 * Exclusive UUCP file
 * locking control
 */

static char *
xxlast(s, c)
char *s;
char c;
{                                           /* Equivalent to strrchr() */
  int i;
  for (i = strlen(s); i > 0; i--)
  {
    if (s[i - 1] == c)
    {
      return ( s + ( i - 1 ));
    }
  }

  return ( NULL );
}

static
look4lk(ttname)
char *ttname;
{
  extern char *strcat(), *strcpy();
  char *device, *devname;
  char lockfil[50];                    /* Max length for lock file name */

  char *lockdir = LOCK_DIR;
  device =
    (( devname = xxlast(ttname, '/')) != \
      NULL ? devname + 1 : ttname );

#ifdef ISIII
  (void)strcpy(lockfil, device);
#else  /* ifdef ISIII */
  strcat(strcpy(lockfil, "LCK.."), device);
#endif /* isiii */

  if (access(lockdir, 04) < 0)         /* read access denied on lock dir */
  {
#ifndef NODOHLP
    fprintf(
      stderr,
        "Warning, read access to lock directory \"%s\" denied\n",
          lockdir);
#else /* ifndef NODOHLP */
    fprintf (stderr, "read lock failed\n");
#endif /* ifndef NODOHLP */
    return ( 1 );                      /* cannot check or set lock file */
  }

  strcat( \
    strcat( \
      strcpy(flfnam, lockdir), "/"),
        lockfil);
  debug(F110, "look4lk", flfnam, 0);

  if (!access(flfnam, 00))             /* print out lock file entry */
  {
    char lckcmd[40];
    strcat( \
      strcpy(lckcmd, "ls -l "),
        flfnam);
    system(lckcmd);                    /* XXX(jhj): make conditional */
    if (( access(flfnam, 02) == 0 ) && \
      ( access(lockdir, 02) == 0 ))
    {
#ifndef NODOHLP
      printf(
        "(You may type \"! rm %s\" to remove this file)\n",
          flfnam);
#endif /* ifndef NODOHLP */
    }

    return ( -1 );
  }

  if (access(lockdir, 02) < 0)         /* lock file cannot be written */
  {
#ifdef NODOHLP
    fprintf(stderr, "write lock failed\n");
#else /* ifdef NODOHLP */
    fprintf(stderr, "Warning, write access to lock directory denied\n");
#endif /* ifdef NODOHLP */
    return ( 1 );
  }

  return ( 0 );                        /* okay to go ahead and lock */
}

/* T T L O C K */

static
ttlock(ttfd)
char *ttfd;
{                                      /* lock UUCP if possible */
  int lck_fil, l4l;
  int pid_buf = getpid();              /* pid to save in lock file */

  hasLock = 0;                         /* not locked yet */
  l4l = look4lk(ttfd);
  if (l4l < 0)
  {
    return ( -1 );                     /* already locked */
  }

  if (l4l == 1)
  {
    return ( 0 );                      /* can't read/write lock directory */
  }

  lck_fil = creat(flfnam, 0444);       /* create lock file ... */
  if (lck_fil < 0)
  {
    return ( -1 );                     /* create of lockfile failed */
  }

#ifdef HDBUUCP
  {
    char string[12];
    sprintf(string, "%10d\n", pid_buf);
    write(lck_fil, string, 11);
  }
#else  /* ifdef HDBUUCP */
  write(
    lck_fil,
      &pid_buf,
        sizeof ( pid_buf ));           /* UUCP expects int in file */
#endif /* HDBUUCP */
  close(lck_fil);
  chmod(flfnam, 0644);                 /* make it readable by UUCP */
  hasLock = 1;                         /* now is locked */
  return ( 0 );
}

/* T T U N L O C K */

static
ttunlck()
{                                      /* kill uucp lock if possible */
  if (hasLock)
  {
    return ( unlink(flfnam));
  }

  return ( 0 );
}

/*
 * New-style (4.3BSD) UUCP line
 * direction control (Stan Barber, Rice U)
 */

#ifdef NEWUUCP
acucntrl(flag, ttname)
char *flag, *ttname;
{
  char x[DEVNAMLEN + 32], *device, *devname;

  if (strcmp(ttname, CTTNAM) == 0 || xlocal == 0)   /* If not local, */
  {
    return;                                         /* just return. */
  }

  device =
    (( devname = xxlast(ttname, '/')) != NULL ? devname + 1 : ttname );
  if (strncmp(device, "LCK..", 4) == 0)
  {
    device += 5;
  }

  sprintf(x, "/usr/lib/uucp/acucntrl %s %s", flag, device);
  debug(F000, "called ", x, 0);
  system(x);
}
#endif /* newuucp */

/* T T P K T -- Condition the communication line for packets */
/*              or for modem dialing */

#define DIALING 4            /* flags (via flow) */
#define CONNECT 5            /* for modem handling */

/*
 * If called with speed > -1,
 * also set the speed.
 *
 * Returns 0 on success,
 * -1 on failure.
 */

ttpkt(speed, flow, parity)
int speed, flow, parity;
{
  int s = -1;
  int x;

  (void)x;

  if (ttyfd < 0)
  {
    return ( -1 );              /* Not open. */
  }

  ttprty = parity;              /* Let other tt functions see this. */
  debug(F101,
    "ttpkt setting ttprty",
      "", ttprty);
  if (xlocal)
  {
    s = ttsspd(speed);          /* Check the speed */
  }

#ifndef UXIII
  if (flow == 1)
  {
    ttraw.sg_flags |= TANDEM;   /* Use XON/XOFF if selected */
  }

  if (flow == 0)
  {
    ttraw.sg_flags &= ~TANDEM;
  }

  ttraw.sg_flags |= RAW;        /* Go into raw mode */
  ttraw.sg_flags &= \
    ~( ECHO | CRMOD );          /* Use CR for break character */
  if (xlocal)
  {
    if (s > -1)
    {
      ttraw.sg_ispeed = \
        ttraw.sg_ospeed = \
          s;                    /* Do the speed */
    }
  }

#ifdef BSD41
  {
    /*
     * For 4.1BSD only, force "old" tty driver, new one botches TANDEM.
     * Note, should really use TIOCGETD in ttopen to get current
     * discipline, and restore it in ttres().
     */

    int ldisc = OTTYDISC;
    ioctl(ttyfd, TIOCSETD, &ldisc);
  }
#endif /* ifdef BSD41 */
  if (stty(ttyfd, &ttraw) < 0)
  {
    return ( -1 );              /* Set the new modes. */
  }

#ifdef MYREAD
#ifdef BSD4

  /*
   * Try to make
   * reads nonblocking
   */

#ifdef FIONBIO
  x = 1;
  debug(F100, "ttpkt FIONBIO ioctl", "", 0);
  if (ioctl(ttyfd, FIONBIO, &x) < 0)
  {
    perror("ttpkt ioctl");
    return ( -1 );
  }

#else  /* ifdef FIONBIO */
#ifdef FNDELAY
  debug(F100, "ttpkt FNDELAY fcntl", "", 0);
  if (fcntl(
        ttyfd,
        F_SETFL,
        fcntl(ttyfd, F_GETFL, 0) | FNDELAY) == -1)
  {
    return ( -1 );
  }

#endif /* ifdef FNDELAY */
#endif /* ifdef FIONBIO */
  debug(F100,
    "ttpkt flushing (1)",
      "", 0);
  ttflui();                     /* Flush any pending input */
  debug(F100,
    "ttpkt returning (1)",
      "", 0);
  return ( 0 );

#endif /* ifdef BSD4 */
#else  /* ifdef MYREAD */
  debug(F100,
    "ttpkt flushing (2)",
      "", 0);
  ttflui();                     /* Flush any pending input */
  debug(F100,
    "ttpkt returning (2)",
      "", 0);
  return ( 0 );

#endif /* ifdef MYREAD */
#endif /* ifndef UXIII */

#ifdef UXIII
  if (flow == 1)
  {
    ttraw.c_iflag |= ( IXON | IXOFF );
  }

  if (flow == 0)
  {
    ttraw.c_iflag &= ~( IXON | IXOFF );
  }

  if (flow == DIALING)
  {
    ttraw.c_cflag |= CLOCAL | HUPCL;
  }

  if (flow == CONNECT)
  {
    ttraw.c_cflag &= ~CLOCAL;
  }

  ttraw.c_lflag &= \
    ~( ICANON | ECHO );
  ttraw.c_lflag |= \
    ISIG;                 /* do check for interrupt */
  ttraw.c_iflag |= \
    ( BRKINT | IGNPAR );
  ttraw.c_iflag &=
    ~( IGNBRK  | \
        INLCR  | \
        IGNCR  | \
        ICRNL  | \
        IUCLC  | \
        INPCK  | \
        ISTRIP | \
        IXANY );
  ttraw.c_oflag &= \
    ~OPOST;
  ttraw.c_cflag &= \
    ~( CSIZE | PARENB );
  ttraw.c_cflag |= \
    ( CS8 | CREAD );
#ifdef MYREAD
  ttraw.c_cc[4] = 1;      /* return max of this many characters */
  ttraw.c_cc[5] = 0;      /* or when this many secs/10 expire w/no input */
#else  /* ifdef MYREAD */
  ttraw.c_cc[4] = 1;      /* [VMIN]  Maybe should be bigger for all Sys V? */
  ttraw.c_cc[5] = 0;      /* [VTIME] Should be set high enough to ignore */
                          /* intercharacter spacing? */
  /*
   * But then we have to distinguish
   * between Sys III and Sys V..
   */

#endif /* ifdef MYREAD */
  if (xlocal && \
    ( s > -1 ))           /* set speed */
  {
    ttraw.c_cflag &= \
      ~CBAUD;
    ttraw.c_cflag |= \
      s;
  }

  if (ioctl(ttyfd, TCSETAW, &ttraw) < 0)
  {
    return ( -1 );        /* set new modes . */
  }

  if (flow == DIALING)
  {
    if (fcntl(ttyfd, F_SETFL, fcntl(ttyfd, F_GETFL, 0) & ~O_NDELAY) < 0)
    {
      return ( -1 );
    }

    close(
      open(ttnmsv, 2));   /* magic to force mode change!!! */
  }

  debug(F100, "ttpkt flushing (3)", "", 0);
  ttflui();
  debug(F100, "ttpkt returning (3)", "", 0);
  return ( 0 );

#endif /* uxiii */
}

/* T T V T -- Condition communication line for use as virtual terminal */

ttvt(speed, flow)
int speed, flow;
{
  int s = -1;
  if (ttyfd < 0)
  {
    return ( -1 );        /* Not open. */
  }

  s = ttsspd(speed);      /* Check the speed */

#ifndef UXIII
  if (flow == 1)
  {
    tttvt.sg_flags |= \
      TANDEM;             /* XON/XOFF if selected */
  }

  if (flow == 0)
  {
    tttvt.sg_flags &= ~TANDEM;
  }

  tttvt.sg_flags |= \
    RAW;                  /* Raw mode */
  tttvt.sg_flags &= \
    ~ECHO;                /* No echo */
  if (s > -1)
  {
    tttvt.sg_ispeed = \
      tttvt.sg_ospeed = \
        s;                /* Do the speed */
  }

  if (stty(ttyfd, &tttvt) < 0)
  {
    return ( -1 );
  }
#ifdef MYREAD
#ifdef BSD4

  /*
   * Make reads
   * nonblocking
   */

  if (fcntl(ttyfd, F_SETFL, fcntl(ttyfd, F_GETFL, 0) | FNDELAY) == -1)
  {
    return ( -1 );
  }
  else
  {
    return ( 0 );
  }
#endif /* ifdef BSD4 */
#endif /* ifdef MYREAD */
#else  /* ifndef UXIII */
  if (flow == 1)
  {
    tttvt.c_iflag |= ( IXON | IXOFF );
  }

  if (flow == 0)
  {
    tttvt.c_iflag &= ~( IXON | IXOFF );
  }

  if (flow == DIALING)
  {
    tttvt.c_cflag |= CLOCAL | HUPCL;
  }

  if (flow == CONNECT)
  {
    tttvt.c_cflag &= ~CLOCAL;
  }

  tttvt.c_lflag &= ~( ISIG | ICANON | ECHO );
  tttvt.c_iflag |= ( IGNBRK | IGNPAR );
  tttvt.c_iflag &=
    ~( INLCR | IGNCR | ICRNL | IUCLC | BRKINT | INPCK | ISTRIP | IXANY );
  tttvt.c_oflag &= ~OPOST;
  tttvt.c_cflag &= ~( CSIZE | PARENB );
  tttvt.c_cflag |= ( CS8 | CREAD );
  tttvt.c_cc[4] = 1;
  tttvt.c_cc[5] = 0;

  if (s > -1)             /* set speed */
  {
    tttvt.c_cflag &= ~CBAUD;
    tttvt.c_cflag |= s;
  }

  if (ioctl(ttyfd, TCSETAW, &tttvt) < 0)
  {
    return ( -1 );        /* set new modes . */
  }

  if (flow == DIALING)
  {
    if (fcntl(ttyfd, F_SETFL, fcntl(ttyfd, F_GETFL, 0) & ~O_NDELAY) < 0)
    {
      return ( -1 );
    }

    close(
      open(ttnmsv, 2));   /* magic to force mode change!!! */
  }

#endif /* ifndef UXIII */
  return ( 0 );
}

/* T T S S P D -- Return the internal baud rate code for 'speed' */

ttsspd(speed)
{
  int s, spdok;

  if (speed < 0)
  {
    return ( -1 );
  }

  spdok = 1;              /* Assume arg ok */
  switch (speed)
  {
  case 0:
    s = B0;
    break;

  case 110:
    s = B110;
    break;

  case 150:
    s = B150;
    break;

  case 300:
    s = B300;
    break;

  case 600:
    s = B600;
    break;

  case 1200:
    s = B1200;
    break;

  case 1800:
    s = B1800;
    break;

  case 2400:
    s = B2400;
    break;

  case 4800:
    s = B4800;
    break;

  case 9600:
    s = B9600;
    break;

#ifdef B19200
  case 19200:
    s = B19200;
    break;

#else  /* ifdef B19200 */
  case 19200:
    s = EXTA;
    break;

#endif /* ifdef B19200 */
#ifdef B38400
  case 38400:
    s = B38400;
    break;

#endif /* ifdef B38400 */
#ifdef B57600
  case 57600:
    s = B57600;
    break;

#endif /* ifdef B57600 */
#ifdef B115200
  case 115200:
    s = B115200;
    break;

#endif /* ifdef B115200 */
#ifdef B230400
  case 230400:
    s = B230400;
    break;

#endif /* ifdef B230400 */
#ifdef B460800
  case 460800:
    s = B460800;
    break;

#endif /* ifdef B460800 */
#ifdef B921600
  case 921600:
    s = B921600;
    break;

#endif /* ifdef B921600 */
#ifdef B1000000
  case 1000000:
    s = B1000000;
    break;

#endif /* ifdef B1000000 */
#ifdef B1152000
  case 1152000:
    s = B1152000;
    break;

#endif /* ifdef B1152000 */
#ifdef B1500000
  case 1500000:
    s = B1500000;
    break;

#endif /* ifdef B1500000 */
#ifdef B2000000
  case 2000000:
    s = B2000000;
    break;

#endif /* ifdef B2000000 */
#ifdef B2500000
  case 2500000:
    s = B2500000;
    break;

#endif /* ifdef B2500000 */
#ifdef B3000000
  case 3000000:
    s = B3000000;
    break;

#endif /* ifdef B3000000 */
#ifdef B3500000
  case 3500000:
    s = B3500000;
    break;

#endif /* ifdef B3500000 */
#ifdef B4000000
  case 4000000:
    s = B4000000;
    break;

#endif /* ifdef B4000000 */
  default:
    spdok = 0;
#ifndef NODOHLP
    fprintf(stderr, "Unsupported line speed - %d\n", speed);
    fprintf(stderr, "Current speed not changed\n");
#else /* ifndef NODOHLP */
    fprintf(stderr, "Bad speed - %d\n", speed);
#endif /* ifndef NODOHLP */
    break;
  }
  if (spdok)
  {
    return ( s );
  }
  else
  {
    return ( -1 );
  }
}

/* T T F L U I -- Flush tty input buffer */

ttflui()
{
#ifndef UXIII
  long n;
#endif /* ifndef UXIII */
  if (ttyfd < 0)
  {
    return ( -1 );        /* Not open. */
  }

  ungotn = -1;            /* Initialize myread() stuff */
  inbufc = 0;

#ifdef UXIII
  if (ioctl(ttyfd, TCFLSH, 0) < 0)
  {
    perror("flush failed");
  }

#else  /* ifdef UXIII */
#ifdef TIOCFLUSH
#ifdef ANYBSD
  n = FREAD;              /* Specify read queue */
  if (
    ioctl(ttyfd, TIOCFLUSH, &n) < 0)
  {
    perror("flush failed");
  }

#else  /* ifdef ANYBSD */
  if (ioctl(ttyfd, TIOCFLUSH, 0) < 0)
  {
    perror("flush failed");
  }

#endif /* ifdef ANYBSD */
#endif /* ifdef TIOCFLUSH */
#endif /* ifdef UXIII */
  return ( 0 );
}

/*
 * Timeout handler for communication
 * line input functions
 */

SIGTYP
timerh()
{
  longjmp(sjbuf, 1);
}

/*
 * Set up terminal interrupts
 * on console terminal
 */

#ifdef UXIII
SIGTYP
esctrp()                      /* trap console escapes (^\) */
{
  conesc = 1;
  signal(SIGQUIT, SIG_IGN);   /* ignore until trapped */
}
#endif /* ifdef UXIII */

#ifdef V7
esctrp()
{                             /* trap console escapes (^\) */
  conesc = 1;
  signal(SIGQUIT, SIG_IGN);   /* ignore until trapped */
}
#endif /* ifdef V7 */

/* C O N I N T -- Console Interrupt setter */

conint(f)
SIGTYP (*f)();
{                             /* Set an interrupt trap. */
  int x, y;
#ifndef BSD4

  /*
   * Fix bug where subsequent calls
   * to conint always set backgrd = 1
   */

  static int bgset = 0;
#endif /* ifndef BSD4 */
#ifdef SIGTSTP
#ifdef __linux__
  void
#else  /* ifdef __linux__ */
  int
#endif /* ifdef __linux__ */
  stptrap();                  /* Suspend trap */
#endif /* ifdef SIGTSTP */

  /*
   * Check for background operation, even
   * if not running on real tty, so that
   * background flag can be set correctly.
   */

#ifdef BSD4
  int mypgrp;                 /* In BSD, we can check whether */
  int ctpgrp;                 /* this process's group is the */
                              /* same as the controlling */
  mypgrp = getpgrp(0);        /* terminal's process group. */
  ioctl(1,
    TIOCGPGRP, &ctpgrp);
  x = ( mypgrp != ctpgrp );   /* If they differ, then background. */
  debug(F101,
    "conint process group test", "", x);
#else  /* ifdef BSD4 */
  x = ( signal(SIGINT, SIG_IGN) == SIG_IGN );
  debug(F101, "conint signal test", "", x);
#endif /* ifdef BSD4 */
  y = isatty(0);
  debug(F101, "conint isatty test", "", y);
#ifdef BSD29

  /*
   * For some reason the signal() test
   * doesn't work under 2.9 BSD...
   */

  backgrd = !y;
#else  /* ifdef BSD29 */
#ifndef BSD4
  if (bgset == 0)
  {
    bgset = 1;
  }

#endif /* ifndef BSD4 */
  backgrd = ( x || !y );
#endif /* ifdef BSD29 */
  debug(F101, "conint backgrd", "", backgrd);

  signal(SIGHUP, f);          /* Ensure lockfile cleared on hangup */
  signal(SIGTERM, f);         /* or soft kill. */

  /*
   * Check if invoked in background,
   * if so signals set to be ignored
   */

  if (backgrd)                /* In background, ignore signals */
  {
    debug(F100, "conint background ignoring signals", "", 0);
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_IGN); /* Keyboard stop */
#endif /* ifdef SIGTSTP */
    signal(SIGQUIT, SIG_IGN); /* Keyboard quit */
    signal(SIGINT, SIG_IGN);  /* Keyboard interrupt */
  }
  else
  {
    debug(F100, "conint foreground catching signals", "", 0);
    signal(SIGINT, f);        /* Catch terminal interrupt */
#ifdef SIGTSTP
    signal(SIGTSTP, stptrap); /* Keyboard stop */
#endif /* ifdef SIGTSTP */
#ifdef UXIII
    signal(SIGQUIT, esctrp);  /* Quit signal, Sys III/V. */
    if (conesc)
    {
      conesc = 0;             /* Clear out pending escapes */
    }

#else  /* ifdef UXIII */
#ifdef V7
    signal(SIGQUIT, esctrp);  /* V7 like Sys III/V */
    if (conesc)
    {
      conesc = 0;
    }

#else  /* ifdef V7 */
    signal(SIGQUIT, SIG_IGN); /* Others, ignore like 4D & earlier. */
#endif /* ifdef V7 */
#endif /* ifdef UXIII */
    conif = 1;                /* Flag console interrupts on. */
  }

  return;
}

/* C O N N O I -- Reset console terminal interrupts */

connoi()
{                             /* Console-no-interrupts */
  debug(F100,
    "connoi", "", 0);
#ifdef SIGTSTP
  signal(SIGTSTP, SIG_DFL);
#endif /* ifdef SIGTSTP */
  signal(SIGINT, SIG_DFL);
  signal(SIGHUP, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  conif = 0;                  /* Flag interrupt trapping off */
}

/*
 *  myread() - For use by systems that can do nonblocking read() calls
 *
 * Returns:
 *  -1  if no characters available, timer expired
 *  -2  upon error (such as disconnect),
 *  otherwise value of character (0 or greater)
 */

#ifdef MYREAD
myread()
{
  static int inbuf_item;
  static CHAR inbuf[257];
  CHAR readit;

  if (ungotn >= 0)
  {
    readit = ungotn;
    ungotn = -1;
  }
  else
  {
    if (inbufc > 0)
    {
      readit = inbuf[++inbuf_item];
    }
    else
    {
      inbufc = read(ttyfd, inbuf, 256);
      if (inbufc > 0)
      {
        inbuf[inbufc] = '\0';
        debug(F101, "myread read", "", inbufc);
      }

      if (inbufc == 0)
      {
        if (ttmdm)
        {
          debug(F101,
            "myread read=0",
              "ttmdm",
                "",
                  ttmdm);
          errno = 9999;    /* magic number for no carrier */
          return ( -2 );   /* end of file has no errno */
        }
        else
        {
          return ( -1 );   /* in sys 5 means no data available */
        }
      }

      if (inbufc < 0)      /* Real error */
      {
#ifdef EWOULDBLOCK
        if (errno == EWOULDBLOCK)
        {
          return ( -1 );
        }
        else
        {
          return ( -2 );
        }
#else  /* ifdef EWOULDBLOCK */
        return ( -2 );
#endif /* ifdef EWOULDBLOCK */
      }

      readit = inbuf[inbuf_item = 0];
    }

    inbufc--;
  }

  return (((int)readit ) & 255 );
}

myunrd(ch) CHAR ch;
{                          /* push back up to one character */
  ungotn = ch;
}
#endif /* ifdef MYREAD */

/* I N I T R A W Q -- Set up to read /DEV/KMEM for character count */

#ifdef V7
/*
 *  Used in Version 7 to simulate Berkeley's FIONREAD ioctl call.  This
 *  eliminates blocking on a read, because we can read /dev/kmem to get the
 *  number of characters available for raw input.  If your system can't
 *  or you won't let it read /dev/kmem (the world that is) then you must
 *  figure out a different way to do the counting of characters available,
 *  or else replace this by a dummy function that always returns 0.
 *
 *
 * Call this routine as: initrawq(tty)
 * where tty is the file descriptor of a terminal.  It will return
 * (as a char *) the kernel-mode memory address of the rawq character
 * count, which may then be read.  It has the side-effect of flushing
 * input on the terminal.
 *
 */

#include <a.out.h>
#include <sys/proc.h>

char *
initrawq(tty)
int tty;
{
#ifdef BSD29
  return ( 0 );

#else  /* ifdef BSD29 */
  long lseek();
  static struct nlist nl[] = { { PROCNAME }, { NPROCNAME }, { "" } };
  static struct proc *pp;
  char *malloc(), *qaddr, *p, c;
  int m, pid, me;
  NPTYPE xproc;            /* Its type is defined in Makefile. */
  int catch();

  me = getpid();
  if (( m = open("/dev/kmem", 0)) < 0)
  {
    err("kmem");
  }

  nlist(BOOTNAME, nl);
  if (nl[0].n_type == 0)
  {
    err("proc array");
  }

  if (nl[1].n_type == 0)
  {
    err("nproc");
  }

  lseek(m, (long)( nl[1].n_value ), 0);
  read(m, &xproc, sizeof ( xproc ));
  signal(SIGALRM, catch);
  if (( pid = fork()) == 0)
  {
    while (1)
    {
      read(tty, &c, 1);
    }
  }

  alarm(2);

  if (setjmp(jjbuf) == 0)
  {
    while (1)
    {
      read(tty, &c, 1);
    }
  }

  signal(SIGALRM, SIG_DFL);

#ifdef DIRECT
  pp = (struct proc *)nl[0].n_value;
#else  /* ifdef DIRECT */
  if (lseek(m, (long)( nl[0].n_value ), 0) < 0L)
  {
    err("seek");
  }

  if (read(m, &pp, sizeof ( pp )) != sizeof ( pp ))
  {
    err("no read of proc ptr");
  }

#endif /* ifdef DIRECT */
  lseek(m, (long)( nl[1].n_value ), 0);
  read(m, &xproc, sizeof ( xproc ));

  if (lseek(m, (long)pp, 0) < 0L)
  {
    err("Can't seek to proc");
  }

  if (( p = malloc(xproc * sizeof ( struct proc ))) == NULL)
  {
    err("malloc");
  }

  if (read(
        m,
        p,
        xproc * sizeof ( struct proc )) !=
      xproc * sizeof ( struct proc ))
  {
    err("read proc table");
  }

  for (pp = (struct proc *)p; xproc > 0; --xproc, ++pp)
  {
    if (pp->p_pid == (short)pid)
    {
      goto iout;
    }
  }

  err("no such proc");

iout:
  close(m);
  qaddr = (char *)( pp->p_wchan );
  free(p);
  kill(pid, SIGKILL);
  wait((int *)0);          /* Destroy the ZOMBIEs! */
  return ( qaddr );

#endif /* ifdef BSD29 */
}

/*
 * More V7-support
 * functions...
 */

static
err(s)
char *s;
{
  char buf[200];

  sprintf(buf, "fatal error in initrawq: %s", s);
  perror(buf);
  doexit(1);
}

static
catch()
{
  longjmp(jjbuf, -1);
}

/* G E N B R K -- Simulate a modem break */

#define BSPEED B150

genbrk(fn)
int fn;
{
  struct sgttyb ttbuf;
  int ret, sospeed;

  ret = ioctl(fn, TIOCGETP, &ttbuf);
  sospeed = ttbuf.sg_ospeed;
  ttbuf.sg_ospeed = BSPEED;
  ret = ioctl(fn, TIOCSETP, &ttbuf);
  ret = write(fn, "\0\0\0\0\0\0\0\0\0\0\0\0", 8);
  ttbuf.sg_ospeed = sospeed;
  ret = ioctl(fn, TIOCSETP, &ttbuf);
  ret = write(fn, "@", 1);
  return;
}
#endif /* ifdef V7 */

/* T T C H K -- Tell how many characters are waiting in tty input buffer */

ttchk()
{
  int x;
  long n;

#ifdef FIONREAD
  x = ioctl(ttyfd, FIONREAD, &n);        /* Berkeley and maybe some others */
  debug(F101, "ttchk", "", n);
  return (( x < 0 ) ? 0 : n );

#else  /* ifdef FIONREAD */
#ifdef V7
  lseek(kmem[TTY], (long)qaddr[TTY], 0); /* 7th Edition UNIX */
  x = read(kmem[TTY], &n, sizeof ( int ));
  return (( x == sizeof ( int )) ? n : 0 );

#else  /* ifdef V7 */
#ifdef UXIII
  return ( inbufc + ( ungotn >= 0 ));    /* Sys III, Sys V */

#else  /* ifdef UXIII */
  return ( 0 );

#endif /* ifdef UXIII */
#endif /* ifdef V7 */
#endif /* ifdef FIONREAD */
}

/* T T X I N -- Get n characters from tty input buffer */

/*
 * Returns number of characters actually gotten, or -1 on failure
 *
 * Intended for use only when it is known that n characters are actually
 * available in the input buffer.
 */

ttxin(n, buf)
int n;
CHAR *buf;
{
  int x;

#ifdef MYREAD
  for (x = 0; ( x > -1 ) && ( x < n ); buf[x++] = myread())
  {
    ;
  }
#else  /* ifdef MYREAD */
  debug(F101, "ttxin: n", "", n);
  x = read(ttyfd, buf, n);
  debug(F101, " x", "", x);
#endif /* ifdef MYREAD */
  if (x > 0)
  {
    buf[x] = '\0';
  }

  if (x < 0)
  {
    x = -1;
  }

  return ( x );
}

/* T T O L -- Similar to "ttinl", but for writing */

ttol(s, n)
int n;
char *s;
{
  int x;
  if (ttyfd < 0)
  {
    return ( -1 );                       /* Not open. */
  }

  x = write(ttyfd, s, n);
  debug(F111, "ttol", s, n);
  /* if (x < 0)
   *   debug(F101, "ttol failed", "", x); */
  return ( x );
}

/* T T O C -- Output a character to the communication line */

/*
 * This function should only used for interactive, character-mode
 * operations, like terminal connection, script execution,
 * dialer i/o, where the overhead of the signals and alarms does
 * not create a bottleneck.
 */

ttoc(c)
char c;
{
  int x;

  (void)jjbuf;

  if (ttyfd < 0)
  {
    return ( -1 );                       /* Check for not open. */
  }

  signal(SIGALRM, timerh);               /* Enable timer interrupt */
  alarm(2);                              /* for 2 seconds. */
  x = write(ttyfd, &c, 1);               /* Try to write the character. */
  if (setjmp(sjbuf))                     /* Timer went off? */
  {
    x = -1;                              /* Yes, set return for failure */
  }

  alarm(0);                              /* Turn off timers, etc. */
  signal(SIGALRM, SIG_DFL);
  return ( x );
}

/* T T I N L -- Read a record (up to break character) from comm line */

/*
 * If no break character encountered within "max", return "max" characters,
 * with disposition of any remaining characters undefined. Otherwise, return
 * the characters that were read, including the break character, in "dest"
 * and the number of characters read as the value of the function, or 0
 * upon end of file, or -1 if an error occurred. Times out & returns
 * error if not completed within "timo" seconds.
 */

#define CTRLC '\03'

ttinl(dest, max, timo, eol)
int max, timo;
CHAR *dest, eol;
{
  unsigned int ccn;
  int x = 0, c, i, j, m, n;                 /* local variables */

  if (ttyfd < 0)
  {
    return ( -1 );                          /* Not open. */
  }

  ccn = 0;
  m = ( ttprty ) ? 0177 : 0377;             /* Parity stripping mask. */
  *dest = '\0';                             /* Clear destination buffer */
  if (timo)
  {
    signal(SIGALRM, timerh);                /* Enable timer interrupt */
  }

  alarm(timo);                              /* Set it. */
  if (setjmp(sjbuf))                        /* Timer went off? */
  {
    x = -1;
  }
  else
  {
    i = 0;                                  /* Next char to process */
    j = 0;                                  /* Buffer position */
    while (1)
    {
      if (( n = ttchk()) > 0)               /* See how many chars arrived */
      {
        if (n > ( max - j ))
        {
          n = max - j;
        }

        if (( n = ttxin(n, dest + i)) < 0)  /* Get them all at once */
        {
          x = -1;
          break;
        }
      }
      else                                  /* Or else... */
      {
        n = 1;                              /* just wait for a char */
        if (( c = ttinc(0)) == -1)
        {
          x = -1;
          break;
        }

        dest[i] = c;                        /* Got one. */
      }

      j = i + n;                            /* Remember buffer position. */
      if (j >= max)
      {
        debug(F101, "ttinl buffer overflow", "", j);
        x = -1;
        break;
      }

      for (; i < j; i++)               /* Go thru all chars we just got */
      {
        dest[i] &= m;                  /* Strip any parity */
        if (dest[i] == eol)            /* Got eol? */
        {
          dest[++i] = '\0';            /* Yes, tie off string, */
          alarm(0);                    /* turn off timers, etc, */
          if (timo)
          {
            signal(SIGALRM, SIG_DFL);  /* and return length. */
          }

          return ( i );
        }
        else if (( dest[i] & 0177 ) \
          == CTRLC)                    /* Check for ^C^C */
        {
          if (++ccn > 1)               /* If we got 2 in a row, clean up */
          {
            alarm(0);                  /* and exit. */
            signal(SIGALRM, SIG_DFL);
            fprintf(stderr, "^C...");
            ttres();
            fprintf(stderr, "\n");
            return ( -2 );
          }
        }
        else
        {
          ccn = 0;                     /* Not ^C, so reset ^C counter, */
        }
      }
    }
  }

  /* debug(F100, "ttinl timout", "", 0); */ /* Get here on timeout. */
  /* debug(F111, " with", dest, i); */
  alarm(0);                                 /* Turn off timer */
  signal(SIGALRM, SIG_DFL);                 /* and interrupt, */
  return ( x );                             /* and return error code */
}

/* T T I N C -- Read a character from the communication line */

ttinc(timo)
int timo;
{
  int m, n = 0;
  CHAR ch = 0;

  m = ( ttprty ) ? 0177 : 0377;        /* Parity stripping mask. */
  if (ttyfd < 0)
  {
    return ( -1 );                     /* Not open. */
  }

  if (timo <= 0)                       /* Untimed. */
  {
#ifdef MYREAD
    /* 
     * myread comm line failure
     * returns -1 thru myread, so,
     * no &= 0377
     */

    while (( n = myread()) == -1)
    {
      ;                                /* Wait for a character... */
    }
    if (n == -2)
    {
      n++;
    }

    return (( n < 0 ) ? -1 : n & m );

#else  /* ifdef MYREAD */
    while (( n = read(ttyfd, &ch, 1)) == 0)
    {
      ;                                /* Wait for a character. */
    }
    return (( n < 0 ) ? -1 : ( ch & 0377 ));

#endif /* ifdef MYREAD */
  }

  signal(SIGALRM, timerh);             /* Timed, set up timer. */
  alarm(timo);
  if (setjmp(sjbuf))
  {
    n = -1;
  }
  else
  {
#ifdef MYREAD
    while (( n = myread()) == -1)
    {
      ;                                /* If managing own buffer... */
    }
    if (n == -2)
    {
      n++;
    }
    else
    {
      ch = n;
      n = 1;
    }

#else  /* ifdef MYREAD */
    n = read(ttyfd, &ch, 1);           /* Otherwise call the system. */
#endif /* ifdef MYREAD */
  }

  alarm(0);                            /* Turn off timer, */
  signal(SIGALRM, SIG_DFL);            /* and interrupt. */
  return (( n < 0 ) ? -1 : \
    ( ch & m ));                       /* Return char or -1. */
}

/* T T S N D B -- Send a BREAK signal */

ttsndb()
{
  int x;
  long n;
  char spd;

  (void)spd;
  (void)n;
  (void)x;

  if (ttyfd < 0)
  {
    return ( -1 );                     /* Not open. */
  }

#ifdef UXIII
  if (
    ioctl(ttyfd, TCSBRK,
      (char *)0) < 0)                  /* Send a BREAK */
  {
    perror("Can't send BREAK");
    return ( -1 );
  }

  return ( 0 );

#else  /* ifdef UXIII */
#ifdef ANYBSD
  n = FWRITE;                           /* Flush output queue. */
  ioctl(ttyfd, TIOCFLUSH, &n);          /* Ignore any errors.. */
  if (ioctl(ttyfd, TIOCSBRK,
    (char *)0) < 0)                     /* Turn on BREAK */
  {
    perror("Can't send BREAK");
    return ( -1 );
  }

  x = msleep(275);                     /* Sleep for so many milliseconds */
  if (ioctl(ttyfd, TIOCCBRK,
    (char *)0) < 0)                    /* Turn off BREAK */
  {
    perror("BREAK STUCK!!!");
    doexit(1);                         /* Get out, closing the line. */
                                       /*   with exit status = 1 */
  }

  return ( x );

#else  /* ifdef ANYBSD */
#ifdef V7
  genbrk(ttyfd);                       /* Simulate a BREAK */
  return ( x );

#endif /* ifdef V7 */
#endif /* ifdef ANYBSD */
#endif /* ifdef UXIII */
}

/* M S L E E P -- Millisecond version of sleep() */

/*
 * Intended only for small intervals.
 * For big ones, just use sleep().
 */

msleep(m)
int m;
{
  (void)tz;
  (void)tv;

#ifdef ANYBSD
  int t1, t3, t4;
  if (m <= 0)
  {
    return ( 0 );
  }

#ifndef BSD42

  /*
   * 2.9 and 4.1 BSD
   * do it this way
   */

  if (ftime(&ftp) < 0)
  {
    return ( -1 );                     /* Get current time. */
  }

  t1 = (( ftp.time & 0xff ) * 1000 ) + ftp.millitm;
  while (1)
  {
    ftime(&ftp);                       /* new time */
    t3 = ((( ftp.time & 0xff ) * 1000 ) + ftp.millitm ) - t1;
    if (t3 > m)
    {
      return ( t3 );
    }
  }
#else  /* ifndef BSD42 */

  /*
   * 4.2 & above can do
   * it with select()...
   */

  if (gettimeofday(&tv, &tz) < 0)
  {
    return ( -1 );                     /* Get current time. */
  }

  t1 = tv.tv_sec;                      /* Seconds */

  tv.tv_sec = 0;                       /* Use select() */
  tv.tv_usec = m * 1000L;
  return ( select(0, (int *)0, (int *)0, (int *)0, &tv));

#endif /* ifndef BSD42 */
#endif /* ifdef ANYBSD */

  /*
   * The clock-tick business is a pain.
   * Wm. E. Davidsen suggested:
   *
   *   #include <sys/param.h>
   *   #define CLOCK_TICK 1000/HZ
   *
   * But I don't see the symbol HZ in
   * this file on my VAX. Maybe just
   * for XENIX.
   */

#ifdef UXIII
#ifdef XENIX

  /*
   * Actually, watch out.
   *
   * It's 50 on the AT,
   * 20 on older PCs...
   */

#define CLOCK_TICK 50                  /* millisecs per clock tick */
#else  /* ifdef XENIX */
#ifndef XENIX
#define CLOCK_TICK 17                  /* 1/60 sec */
#endif /* ifndef XENIX */
#endif /* ifdef XENIX */

  extern long times();
  long t1, t2, tarray[4];
  int t3;

  /*
   * In SCO Xenix 2.1.3 or later,
   * you can use nap((long)m) to do this
   */

  if (m <= 0)
  {
    return ( 0 );
  }

  if (( t1 = times(tarray)) < 0)
  {
    return ( -1 );
  }

  while (1)
  {
    if (( t2 = times(tarray)) < 0)
    {
      return ( -1 );
    }

    t3 = ((int)( t2 - t1 )) * CLOCK_TICK;
    if (t3 > m)
    {
      return ( t3 );
    }
  }
#endif /* ifdef UXIII */
}

/* R T I M E R -- Reset elapsed time counter */

#ifndef NOSTATS
rtimer()
{
  tcount = time((long *)0);
}
#endif /* ifndef NOSTATS */

/* G T I M E R -- Get current value of elapsed time counter in seconds */

#ifndef NOSTATS
gtimer()
{
  int x;

  x = (int)( time((long *)0) - tcount );
  rtimer();
  return (( x < 0 ) ? 0 : x );
}
#endif /* ifndef NOSTATS */

/* Z T I M E -- Return date/time string */

ztime(s)
char **s;
{
#ifdef UXIII
  extern long time();                  /* Sys III and Sys5 way to do it */
  char *ctime();
  long clock_storage;

  clock_storage = time((long *)0);
  *s = ctime(&clock_storage);
#endif /* ifdef UXIII */

#ifdef ANYBSD
  char *asctime();                     /* The Berkeley way */
  struct tm *localtime();
  struct tm *tp;
#ifdef BSD42
  gettimeofday(&tv, &tz);              /* BSD 4.2 */
  time(&tv.tv_sec);
  tp = localtime(&tv.tv_sec);
#else  /* ifdef BSD42 */
  time(&clock);                        /* BSD 4.1, 2.9 ... */
  tp = localtime(&clock);
#endif /* ifdef BSD42 */
  *s = asctime(tp);
#endif /* ifdef ANYBSD */

#ifdef V7
  char *asctime();                     /* old V7 way */
  struct tm *localtime();
  struct tm *tp;

  time(&clock);
  tp = localtime(&clock);
  *s = asctime(tp);
#endif /* ifdef V7 */
}

/* C O N G M -- Get console terminal modes */

/*
 * Saves current console mode, and establishes variables
 * for switching between current (presumably normal)
 * mode and other modes.
 */

congm()
{
  if (!isatty(0))
  {
    return ( 0 );                      /* only for real ttys */
  }

  debug(F100, "congm", "", 0);
#ifndef UXIII
  gtty(0, &ccold);                     /* Structure for restoring */
  gtty(0, &cccbrk);                    /* For setting CBREAK mode */
  gtty(0, &ccraw);                     /* For setting RAW mode */
#else  /* ifndef UXIII */
  ioctl(0, TCGETA, &ccold);
  ioctl(0, TCGETA, &cccbrk);
  ioctl(0, TCGETA, &ccraw);
/*  ccold.c_cc[1] = 034; */            /** these changes were suggested **/
/*  ioctl(0,TCSETAW,&ccold); */        /** but may be dangerous **/
#endif /* ifndef UXIII */
  cgmf = 1;                            /* Flag that we got them. */
  return ( 0 );
}

/* C O N C B -- Put console in cbreak mode */

/*
 * Returns 0 if ok,
 * -1 if not
 */

concb(esc)
char esc;
{
  int x;
  if (!isatty(0))
  {
    return ( 0 );                      /* only for real ttys */
  }

  if (cgmf == 0)
  {
    congm();                           /* Get modes if necessary. */
  }

  escchr = esc;                        /* Make this available to other fns */
  ckxech = 1;                          /* Program can echo characters */
#ifndef UXIII
  cccbrk.sg_flags |= CBREAK;           /* Set to character wakeup, */
  cccbrk.sg_flags &= ~ECHO;            /* no echo. */
  x = stty(0, &cccbrk);
#else  /* ifndef UXIII */
  cccbrk.c_lflag &= ~( ICANON | ECHO );
  cccbrk.c_cc[0] = 003;                /* interrupt char is control-c */
  cccbrk.c_cc[1] = escchr;             /* escape during packet modes */
  cccbrk.c_cc[4] = 1;
  cccbrk.c_cc[5] = 1;
  x = ioctl(0, TCSETAW, &cccbrk);      /* set new modes . */
#endif /* ifndef UXIII */

  if (x > -1)
  {
    setbuf(stdout, NULL);              /* Make console unbuffered. */
  }

#ifdef V7
  if (kmem[CON] < 0)
  {
    qaddr[CON] = initrawq(0);
    if (( kmem[CON] = open("/dev/kmem", 0)) < 0)
    {
      fprintf(stderr, "Can't read /dev/kmem in concb.\n");
      perror("/dev/kmem");
      exit(1);
    }
  }

#endif /* ifdef V7 */
  return ( x );
}

/* C O N B I N -- Put console in binary mode */

/*
 * Returns 0 if ok
 * -1 if not
 */

conbin(esc)
char esc;
{
  if (!isatty(0))
  {
    return ( 0 );                      /* only for real ttys */
  }

  if (cgmf == 0)
  {
    congm();                           /* Get modes if necessary. */
  }

  debug(F100, "conbin", "", 0);
  escchr = esc;                        /* Make this available to other fns */
  ckxech = 1;                          /* Program can echo characters */
#ifndef UXIII
  ccraw.sg_flags |= ( RAW | TANDEM );  /* Set rawmode, XON/XOFF */
  ccraw.sg_flags &= ~( ECHO | CRMOD ); /* Set char wakeup, no echo */
  return ( stty(0, &ccraw));

#else  /* ifndef UXIII */
  ccraw.c_lflag &= ~( ISIG | ICANON | ECHO );
  ccraw.c_iflag |= ( BRKINT | IGNPAR );
  ccraw.c_iflag &=
    ~( IGNBRK | INLCR | IGNCR | ICRNL | IUCLC | IXON | IXANY |
       IXOFF | INPCK | ISTRIP );
  ccraw.c_oflag &= ~OPOST;

/*
 * Kermit used to put the console in 8-bit raw mode, but some users have
 * pointed out that this should not be done, since some sites actually
 * use terminals with parity settings on their UNIX systems.
 */

#ifdef __linux__
  ccraw.c_cflag &= ~( PARENB | CSIZE );
  ccraw.c_cflag |= ( CS8 | CREAD );
#endif /* ifdef __linux__ */

/*
 * Sys III/V sites that have trouble with this can restore these lines.
 */

  ccraw.c_cc[0] = 003;                 /* Interrupt char is Ctrl-C */
  ccraw.c_cc[1] = escchr;              /* Escape during packet mode */
  ccraw.c_cc[4] = 1;
  ccraw.c_cc[5] = 1;
  return ( ioctl(0, TCSETAW, &ccraw)); /* set new modes . */

#endif /* ifndef UXIII */
}

/* C O N R E S -- Restore the console terminal */

conres()
{
  debug(F100, "entering conres", "", 0);
  if (cgmf == 0)
  {
    return ( 0 );                      /* Don't do anything if modes */
  }

  if (!isatty(0))
  {
    return ( 0 );                      /* only for real ttys */
  }

#ifndef UXIII                          /* except for sIII, */
  sleep(1);                            /*  not known! */
#endif                                 /*  (sIII does wait in ioctls) */
  ckxech = 0;                          /* System should echo chars */
#ifndef UXIII
  debug(F100, "conres restoring stty", "", 0);
  return ( stty(0, &ccold));           /* Restore controlling tty */

#else  /* ifndef UXIII */
  return ( ioctl(0, TCSETAW, &ccold));

#endif /* ifndef UXIII */
}

/* C O N O C -- Output a character to the console terminal */

conoc(c)
char c;
{
  write(1, &c, 1);
}

/* C O N X O -- Write x characters to the console terminal */

int
conxo(x, s)
char *s;
int x;
{
  write(1, s, x);
  return ( 0 );
}

/* C O N O L -- Write a line to the console terminal */

int
conol(s)
char *s;
{
  int len;
  len = strlen(s);
  write(1, s, len);
  return ( 0 );
}

/* C O N O L A -- Write an array of lines to the console terminal */

int
conola(s)
char *s[];
{
  int i;
  for (i = 0; *s[i]; i++)
  {
    conol(s[i]);
  }
  return ( 0 );
}

/* C O N O L L -- Output a string followed by CRLF */

int
conoll(s)
char *s;
{
  conol(s);
  write(1, "\r\n", 2);
  return ( 0 );
}

/* C O N C H K -- Return how many characters available at console */

#ifndef NOICP
conchk()
{
  int x;
  long n;

  (void)x;
  (void)n;
#ifdef V7
  lseek(kmem[CON], (long)qaddr[CON], 0);
  x = read(kmem[CON], &n, sizeof ( int ));
  return (( x == sizeof ( int )) ? n : 0 );

#else  /* ifdef V7 */
#ifdef UXIII
  if (conesc)                          /* Escape typed */
  {
    conesc = 0;
    signal(SIGQUIT, esctrp);           /* Restore escape */
    return ( 1 );
  }

  return ( 0 );

#else  /* ifdef UXIII */
#ifdef FIONREAD
  x = ioctl(0, FIONREAD, &n);          /* BSD and maybe some others */
  return (( x < 0 ) ? 0 : n );

#else  /* ifdef FIONREAD */
  return ( 0 );                        /* Others can't do. */

#endif /* ifdef FIONREAD */
#endif /* ifdef UXIII */
#endif /* ifdef V7 */
}
#endif /* ifndef NOICP */

/* C O N I N C -- Get a character from the console */

coninc(timo)
int timo;
{
  int n = 0;
  char ch;
  if (timo <= 0)                       /* Untimed */
  {
    n = read(0, &ch, 1);               /* Read a character. */
    ch &= 0377;
    if (n > 0)
    {
      return ( ch );                   /* Return the char if read */
    }
    else
#ifdef UXIII
    if (n < 0 && errno == EINTR)       /* if read was interrupted by QUIT */
    {
      return ( escchr );               /* user entered escape character */
    }
    else                               /* can't be ^C sigint never returns */
#endif /* ifdef UXIII */
    {
      return ( -1 );                   /* Return the char, or -1. */
    }
  }

  signal(SIGALRM, timerh);             /* Timed read, so set up timer */
  alarm(timo);
  if (setjmp(sjbuf))
  {
    n = -2;
  }
  else
  {
    n = read(0, &ch, 1);
    ch &= 0377;
  }

  alarm(0);                            /* Stop timing we got our character */
  signal(SIGALRM, SIG_DFL);
  if (n > 0)
  {
    return ( ch );
  }
  else
#ifdef UXIII
  if (n == -1 && errno == EINTR)       /* If read interrupted by QUIT, */
  {
    return ( escchr );                 /* user entered escape character, */
  }
  else                                 /* can't be ^C sigint never returns */
#endif /* ifdef UXIII */
  {
    return ( -1 );
  }
}
