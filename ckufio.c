#ifndef NOICP
char *ckzv = "   File IO, 4G(095)";
#endif /* ifndef NOICP */

/* C K U F I O -- Kermit file system support for UNIX systems */

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

#include "ckcdeb.h"               /* Typedefs, debug formats, etc */
#include "ckcker.h"               /* Kermit definitions */
#include <ctype.h>                /* Character types */
#include <errno.h>                /* Error number information */
#include <pwd.h>                  /* Password file for shell name */
#include <stdio.h>                /* Standard i/o */
#include <sys/dir.h>              /* Directory structure */
#include <sys/types.h>            /* Data types */



/*
 * File date
 * material
 */

#ifdef BSD4
#define TIMESTAMP
#include <sys/timeb.h>
#include <time.h>
#ifdef TIMEZONE
static long timezone;
#endif /* ifdef TIMEZONE */
#endif /* ifdef BSD4 */

#ifdef UXIII
#define TIMESTAMP
#include <time.h>
#ifdef TIMEZONE
void tzset();
extern long timezone;
#endif /* ifdef TIMEZONE */
#endif /* ifdef UXIII */

#ifdef __linux__
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#endif /* ifdef __linux__ */

#define leap(y) \
  ((( y ) % 4 == 0 && ( y ) % 100 != 0 ) || ( y ) % 400 == 0 )

#define nleap(y) \
  ((( y ) - 1969 ) / 4 - (( y ) - 1901 ) / 100 + (( y ) - 1601 ) / 400 )

#ifdef TIMEZONE
static char monlens[] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
#endif /* ifdef TIMEZONE */

#include <sys/stat.h>       /* File status */

/*
 * Support for tilde-expansion
 * in file and directory names
 */

#ifdef BSD4
#define NAMEENV "USER"      /* Environment variable for tilde */
#endif /* ifdef BSD4 */
#ifdef UXIII
#define NAMEENV "LOGNAME"   /* Environment variable for tilde */
#endif /* ifdef UXIII */

#ifdef BSD4
#ifdef MAXNAMLEN
#define BSD42
#ifdef BSD43
char *ckzsys = " 4.3 BSD";
#else  /* ifdef BSD43 */
#ifdef SUNOS4
char *ckzsys = " SUNOS 4.x";
#else  /* ifdef SUNOS4 */
#ifdef ultrix
char *ckzsys = " VAX/Ultrix";
#else  /* ifdef ultrix */
char *ckzsys = " 4.2 BSD";
#endif /* ifdef ultrix */
#endif /* ifdef SUNOS4 */
#endif /* ifdef BSD43 */
#else  /* ifdef MAXNAMLEN */
#define BSD41
char *ckzsys = " 4.1 BSD";
#endif /* ifdef MAXNAMLEN */
#endif /* ifdef BSD4 */
#ifdef BSD29
char *ckzsys = " 2.9 BSD";
#endif /* ifdef BSD29 */
#ifdef V7
char *ckzsys = " Version 7 UNIX";
#endif /* ifdef V7 */
#ifdef UXIII
#ifdef XENIX
#ifdef M_I386
char *ckzsys = " Xenix/386";
#else  /* ifdef M_I386 */
#ifdef M_I286
char *ckzsys = " Xenix/286";
#else  /* ifdef M_I286 */
char *ckzsys = " Xenix/86";
#endif /* ifdef M_I286 */
#endif /* ifdef M_I386 */
#else  /* ifdef XENIX */
#ifdef ISIII
char *ckzsys = " ISC System III";
#else  /* ifdef ISIII */
#ifdef __linux__
char *ckzsys = " GNU/Linux";
#else  /* ifdef __linux__ */
char *ckzsys = " AT&T System III/System V";
#endif /* ifdef __linux__ */
#endif /* ifdef ISIII */
#endif /* ifdef XENIX */
#endif /* ifdef UXIII */

/*
 * Definitions for some
 * UNIX system commands
 */

char *DELCMD = "rm -f ";             /* For file deletion */
#ifndef NOICP
char *PWDCMD = "pwd ";               /* For saying where I am */
#endif /* ifndef NOICP */
char *TYPCMD = "cat ";               /* For typing a file */
char *DIRCMD = "ls -l ";             /* For directory listing */
#ifdef BSD4
char *SPACMD = "pwd ; quota ; df ."; /* Space/quota of current directory */
#else  /* ifdef BSD4 */
char *SPACMD = "df ";
#endif /* ifdef BSD4 */
char *SPACM2 = "df ";                /* For space in specified directory */
#ifdef BSD4
char *WHOCMD = "finger ";            /* For seeing who's logged in */
#else  /* ifdef BSD4 */
char *WHOCMD = "who ";               /* For seeing who's logged in */
#endif /* ifdef BSD4 */

/*
 * Functions (n is one of the predefined file numbers from ckermi.h):
 *
 * zopeni(n,name)  - Opens an existing file for input
 * zopeno(n,name)  - Opens a new file for output
 * zclose(n)       - Closes a file
 * zsout(n,s)      - Write a null-terminated string to output file, buffered
 * zsoutl(n,s)     - Like zsout, but appends a line terminator
 * zsoutx(n,s,x)   - Write x characters to output file, unbuffered
 * zchout(n,c)     - Add a character to an output file, unbuffered
 * zchki(name)     - Check if named file exists and is readable, return size
 * zchko(name)     - Check if named file can be created
 * znewn(name,s)   - Make a new unique file name based on the given name
 * zdelet(name)    - Delete the named file
 * zxpand(string)  - Expands the given wildcard string into a list of files
 * znext(string)   - Returns the next file from the list in "string"
 * zxcmd(cmd)      - Execute the command in a lower fork
 * zclosf()        - Close input file associated with zxcmd()'s lower fork
 * zrtol(n1,n2)    - Convert remote filename into local form
 * zltor(n1,n2)    - Convert local filename into remote form
 * zchdir(dirnaml) - Change working directory
 * zhome()         - Return pointer to home directory name string
 * zkself()        - Kill self, log out own job
 * zsattr(
 *  struc zattr *) - Return attributes for file which is being sent
 */

/*
 * Which systems include
 * <sys/file.h>...
 */

#ifndef XENIX
#ifndef unos

/*
 * Watch out, some versions of Xenix might
 * need to do this include, but reportedly
 * SCO Xenix 2.2 on an 80x86 system does not
 */

#include <sys/file.h>        /* File access */
#endif /* ifndef unos */
#endif /* ifndef XENIX */

/*
 * Some systems define these symbols in
 * include files, others don't...
 */

#ifndef R_OK
#define R_OK 4               /* For access */
#endif /* ifndef R_OK */

#ifndef W_OK
#define W_OK 2
#endif /* ifndef W_OK */

#ifdef UXIII
#include <fcntl.h>
#ifndef MAXNAMLEN
#define MAXNAMLEN DIRSIZ
#endif /* ifndef MAXNAMLEN */
#endif /* ifdef UXIII */

#ifndef O_RDONLY
#define O_RDONLY 000
#endif /* ifndef O_RDONLY */

#ifdef MINBUF
#undef MAXNAMLEN
#define MAXNAMLEN 99         /* 99 max filename length for minbuf */
#endif /* ifdef MINBUF */

#ifndef MAXNAMLEN
#ifdef __linux__
#define MAXNANLEN 255        /* Linux XXX(jhj): Find in header */
#else  /* ifdef __linux__ */
#define MAXNAMLEN 14         /* If still not defined... */
#endif /* ifdef __linux__ */
#endif /* ifndef MAXNAMLEN */

#ifdef BSD29
#define MAXWLD 50            /* Maximum wildcard filenames */
#else  /* ifdef BSD29 */
#ifdef MINBUF
#define MAXWLD 20            /* XXX(jhj): 20 wildcard files */
#else /* ifdef MINBUF */
#define MAXWLD 100           /* XXX(jhj): 100 wildcard files */
#endif /* ifdef MINBUF */
#endif /* ifdef BSD29 */

FILE *fp[ZNFILS] = {         /* File pointers */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

/*
 * External definitions of things
 * used in buffered file input and output
 */

extern CHAR zinbuffer[], zoutbuffer[];
extern CHAR *zinptr, *zoutptr;
extern int zincnt, zoutcnt;

static long iflen = -1;            /* Input file length */
/* static long oflen = -1; */      /* Output file length */

static int pid = 0;                /* pid of child fork */
static int fcount;                 /* Number of files in wild group */
static char nambuf[MAXNAMLEN + 2]; /* Buffer for a filename */
static char zmbuf[200];            /* For mail, remote print strings */
#ifndef __linux__
char *malloc();
#endif /* ifndef __linux__ */
char *getenv(), *strcpy();         /* System functions */
extern int errno;                  /* System error code */

static char *mtchs[MAXWLD],        /* Matches found for filename */
  **mtchptr;                       /* Pointer to current match */

/* Z K S E L F -- Kill Self: log out own job, if possible */

/*
 * Note: This should get current pid,
 * but if your system doesn't have
 * getppid(), just do kill(0,9)...
 */

int
zkself()
{                                  /* For "bye", but, no guarantees! */
#ifdef V7
  return ( kill(0, 9));

#else  /* ifdef V7 */
  return ( kill(getppid(), 1));

#endif /* ifdef V7 */
}

/* Z O P E N I -- Open an existing file for input */

int
zopeni(n, name)
int n;
char *name;
{
  debug(F111, " zopeni", name, n);
  /* debug(F101, "  fp", "", (int)fp[n]); */
  if (chkfn(n) != 0)
  {
    return ( 0 );
  }

  zincnt = 0;                      /* Reset input buffer */
#ifndef NOPUSH
  if (n == ZSYSFN)                 /* Input from a system function? */
  {
    debug(F110, " invoking zxcmd", name, 0);
    *nambuf = '\0';                /* No filename this time... */
    return ( zxcmd(name));         /* Try to fork the command */
  }

#endif /* ifndef NOPUSH */
  if (n == ZSTDIO)                 /* Standard input? */
  {
    if (isatty(0))
    {
#ifdef NODOHLP
      ermsg("Not on a tty");
#else /* ifdef NODOHLP */
      ermsg("Terminal input not allowed");
#endif /* ifdef NODOHLP */
      debug(F110, "zopeni: attempts input from unredirected stdin", "", 0);
      return ( 0 );
    }

    fp[ZIFILE] = stdin;
    return ( 1 );
  }

  fp[n] = fopen(name, "r");        /* Real file. */
  /* debug(F111, " zopeni", name, (int)fp[n]); */
  if (fp[n] == NULL)
  {
    perror("zopeni");
  }

  return (( fp[n] != NULL ) ? 1 : 0 );
}

/* Z O P E N O -- Open a new file for output */

int
zopeno(n, name)
int n;
char *name;
{
  /*
   * This suid stuff commented out probably
   * needs work, to apply to all the UNIX
   * variants supported by this program.
   * Maybe use setreuid()? The code shown
   * allegedly works in 4.xBSD, Ultrix, etc.
   */

  /* int uid, euid; */             /* suid variables... */

  /* if (n != ZDFILE)
   * debug(F111, " zopeno", name, n); */
  if (chkfn(n) != 0)
  {
    return ( 0 );
  }

  if (( n == ZCTERM ) || \
    ( n == ZSTDIO ))               /* Terminal or standard output */
  {
    fp[ZOFILE] = stdout;
    /* if (n != ZDFILE)
     * debug(F101, " fp[]=stdout", "", (int)fp[n]); */
    zoutcnt = 0;
    zoutptr = zoutbuffer;
    return ( 1 );
  }

  /* uid = getuid(); euid = geteuid(); */ /* In case running suid to uucp */
  /* seteuid(uid); */                     /* etc, get user's own id. */
  fp[n] = fopen(name, "w");               /* A real file, try to open. */
  /* seteuid(uid); */                     /* Put back program's suid. */
  if (fp[n] == NULL)
  {
    perror("zopeno can't open");
  }
  else
  {
    chown(name, getuid(), getgid());      /* In case set[gu]id */
    if (n == ZDFILE)
    {
      setbuf(fp[n], NULL);                /* Debugging file unbuffered */
    }
  }

  zoutcnt = 0;                            /* (PWP) reset output buffer */
  zoutptr = zoutbuffer;
  /* if (n != ZDFILE)
   * debug(F101, " fp[n]", "", (int)fp[n]); */
  return (( fp[n] != NULL ) ? 1 : 0 );
}

/* Z C L O S E -- Close the given file */

/*
 * Returns 0 if arg out of
 * range, 1 if successful,
 * -1 if close failed.
 */

int
zclose(n)
int n;
{
  int x, x2;
  if (chkfn(n) < 1)
  {
    return ( 0 );                             /* Check range of n */
  }

  if (( n == ZOFILE ) && ( zoutcnt > 0 ))     /* (PWP) output leftovers */
  {
    x2 = zoutdump();
  }
  else
  {
    x2 = 0;
  }

  x = 0;                                  /* Initialize return code */
#ifndef NOPUSH
  if (( n == ZIFILE ) && fp[ZSYSFN])      /* If system function */
  {
    x = zclosf();                         /* do it specially */
  }
  else
  {
#endif /* ifndef NOPUSH */
  if (( fp[n] != stdout ) && ( fp[n] != stdin ))
  {
    x = fclose(fp[n]);
  }

  fp[n] = NULL;
#ifndef NOPUSH
}
#endif /* ifndef NOPUSH */
  iflen = -1;                             /* Invalidate file length */
  if (x == EOF)                           /* if we got a close error */
  {
    return ( -1 );
  }
  else if (x2 < 0)                        /* or error flushing the buffer */
  {
    return ( -1 );                        /* then return an error */
  }
  else
  {
    return ( 1 );
  }
}

/* Z C H I N -- Get a character from the input file */

/*
 * Returns -1 if EOF, 0
 * otherwise with character
 * returned in argument
 */

#ifdef COMMENT
zchin(n, c)
int n;
char *c;
{
  int a;

  /*
   * (PWP) Just in case this gets
   * called when it shoudn't
   */

  if (n == ZIFILE)
  {
    return ( zminchar());
  }

  /* if (chkfn(n) < 1) return(-1); */
  a = getc(fp[n]);
  if (a == EOF)
  {
    return ( -1 );
  }

  *c = a & 0377;
  return ( 0 );
}
#endif /* ifdef COMMENT */

/*
 * (Re)fill the buffered input buffer with data.
 * All file input should go through this routine, usually
 * by calling the zminchar() macro.
 */

int
zinfill()
{
  zincnt = fread(zinbuffer, sizeof ( char ), INBUFSIZE, fp[ZIFILE]);
  if (zincnt == 0)
  {
    return ( -1 );         /* end of file */
  }

  zinptr = zinbuffer;      /* set pointer to beginning, (== &zinbuffer[0]) */
  zincnt--;                /* one less char in buffer */
  return \
    ((int)( *zinptr++ ) \
     & 0377 );             /* because we return the first */
}

/* Z S O U T -- Write a string out to the given file, buffered */

int
zsout(n, s)
int n;
char *s;
{
  if (chkfn(n) < 1)
  {
    return ( -1 );         /* Keep this here, prevents memory faults */
  }

  fputs(s, fp[n]);
  return ( 0 );
}

/* Z S O U T L -- Write string to file, with line terminator, buffered */

int
zsoutl(n, s)
int n;
char *s;
{
  /* if (chkfn(n) < 1) return(-1); */
  fputs(s, fp[n]);
  fputs("\n", fp[n]);
  return ( 0 );
}

/* Z S O U T X -- Write x characters to file, unbuffered */

int
zsoutx(n, s, x)
int n, x;
char *s;
{
  /* if (chkfn(n) < 1) return(-1); */
  /*  return(write(fp[n]->_file,s,x));  */
  return ( write(fileno(fp[n]), s, x));
}

/* Z C H O U T -- Add a character to the given file */

/*
 * Should return 0 or greater on success,
 * -1 on failures (e.g. disk full)
 */

int
zchout(n, c)
/* register */
int n;
char c;
{
  /* if (chkfn(n) < 1) return(-1); */
  if (n == ZSFILE)
  {
    return ( write(fileno(fp[n]),
      &c, 1));                           /* Use unbuffered for session log */
  }
  else                                   /* Buffered for everything else */
  {
    if (putc(c, fp[n]) == EOF)           /* If true maybe there's an error */
    {
      return ( ferror(fp[n]) ? -1 : 0 ); /* Check to make sure */
    }
    else                                 /* Otherwise... */
    {
      return ( 0 );                      /* There was no error. */
    }
  }
}

/*
 * Buffered character output
 * routine to speed up file IO
 */

int
zoutdump()
{
  int x;

  zoutptr = zoutbuffer;             /* reset buffer pointer in all cases */
  debug(F101, "zoutdump chars", "", zoutcnt);
  if (zoutcnt == 0)                 /* nothing to output */
  {
    return ( 0 );
  }
  else if (zoutcnt < 0)             /* unexpected negative value */
  {
    zoutcnt = 0;                    /* reset output buffer count */
    return ( -1 );                  /* and fail. */
  }

  x = fwrite(zoutbuffer, 1, zoutcnt, fp[ZOFILE]);
  if (x)
  {
    debug(F101, "zoutdump fwrite wrote", "", x);
    zoutcnt = 0;                    /* reset output buffer count */
    return ( 0 );                   /* things worked OK */
  }
  else
  {
    zoutcnt = 0;                    /* reset output buffer count */
    x = ferror(fp[ZOFILE]);         /* get error code */
    debug(F101, "zoutdump fwrite error", "", x);
    return ( x ? -1 : 0 );          /* return failure if error */
  }
}

/* C H K F N -- Internal function to verify file number is ok */

/*
 * Returns:
 * -1: File number n is out of range
 * 0: n is in range, but file is not open
 * 1: n in range and file is open
 */

int
chkfn(n)
int n;
{
  switch (n)
  {
  case ZCTERM:
  case ZSTDIO:
  case ZIFILE:
  case ZOFILE:
  case ZDFILE:
  case ZTFILE:
  case ZPFILE:
  case ZSFILE:
#ifndef NOPUSH
  case ZSYSFN:
    break;

#endif /* ifndef NOPUSH */
  default:
    debug(F101, "chkfn: file number out of range", "", n);
#ifndef NODOHLP
    fprintf(stderr, "?File number out of range - %d\n", n);
#else /* ifndef NODOHLP */
    fprintf(stderr, "?chkfn fail - %d\n", n);
#endif /* ifndef NODOHLP */
    return ( -1 );
  }
  return (( fp[n] == NULL ) ? 0 : 1 );
}

/* Z C H K I -- Check if input file exists and is readable */

/*
 * Returns:
 *   >= 0 if the file can be read (returns the size).
 *     -1 if file doesn't exist or can't be accessed,
 *     -2 if file exists but is not readable (e.g. a directory file).
 *     -3 if file exists but protected against read access.
 */

/*
 * For Berkeley UNIX, a file must be of type "regular" to be
 * readable. Directory files, special files, and symbolic
 * links are not readable.
 */

long
zchki(name)
char *name;
{
  struct stat buf;
  int x = 0;
  long y = 0;

  (void)y;
  x = stat(name, &buf);
  if (x < 0)
  {
    debug(F111, "zchki stat fails", name, errno);
    return ( -1 );
  }

  x = buf.st_mode & S_IFMT;                  /* Isolate file format field */
  if (( x != 0 ) && ( x != S_IFREG ))
  {
    debug(F111, "zchki skipping:", name, x);
    return ( -2 );
  }

  debug(F111, "zchki stat ok:", name, x);

  x = access(name, R_OK);
  if (x < 0)                                 /* Is the file accessible? */
  {
    debug(F111, " access failed:", name, x); /* No */
    return ( -3 );
  }
  else
  {
    iflen = buf.st_size;                     /* Yes, remember size */
    strncpy(nambuf, name, MAXNAMLEN);        /* and name globally. */
    debug(F111, " access ok:", name, (int)iflen);
    return (( iflen > -1 ) ? iflen : 0 );
  }
}

/* Z C H K O -- Check if output file can be created */

/*
 * Returns -1 if write permission for
 * the file would be denied, 0 otherwise.
 */

#ifndef NOICP
int
zchko(name)
char *name;
{
  int i, x;
  char s[50], *sp;

  sp = s;                                    /* Make a copy, get length */
  x = 0;
  while (( *sp++ = *name++ ) != '\0')
  {
    x++;
  }
  if (x == 0)
  {
    return ( -1 );                           /* If no filename, fail. */
  }

  debug(F101, " length", "", x);
  for (i = x; i > 0; i--)                    /* Strip filename. */
  {
    if (s[i - 1] == '/')
    {
      break;
    }
  }

  debug(F101, " i", "", i);
  if (i == 0)                                /* If no path, use current */
  {
    strcpy(s, "./");
  }
  else                                       /* Otherwise, use given one. */
  {
    s[i] = '\0';
  }

  x = access(s, W_OK);                       /* Check access of path. */
  if (x < 0)
  {
    debug(F111, "zchko access failed:", s, errno);
    return ( -1 );
  }
  else
  {
    debug(F111, "zchko access ok:", s, x);
    return ( 0 );
  }
}
#endif /* ifndef NOICP */

/* Z D E L E T -- Delete the named file */

void
zdelet(name)
char *name;
{
  unlink(name);
}

/* Z R T O L -- Convert remote filename into local form */

/*
 * For UNIX, this means changing
 * uppercase letters to lowercase.
 */

void
zrtol(name, name2)
char *name, *name2;
{
  for (; *name != '\0'; name++)
  {
    *name2++ = isupper(*name) ? tolower(*name) : *name;
  }

  *name2 = '\0';
  debug(F110, "zrtol:", name2, 0);
}

/* Z L T O R -- Local TO Remote */

/*
 * Convert filename from a local
 * format to common (remote) form
 */

void
zltor(name, name2)
char *name, *name2;
{
  char work[100], *cp, *pp;
  int dc = 0;

  debug(F110, "zltor", name, 0);
  pp = work;
  for (cp = name; *cp != '\0'; cp++)       /* strip path name */
  {
    if (*cp == '/')
    {
      dc = 0;
      pp = work;
    }
    else if (islower(*cp))
    {
      *pp++ = toupper(*cp);                /* Uppercase letters */
    }
    else if (*cp == '~')
    {
      *pp++ = 'X';                         /* Change tilde to 'X' */
    }
    else if (*cp == '#')
    {
      *pp++ = 'X';                         /* Change number sign to 'X' */
    }
    else if (( *cp == '.' ) && ( ++dc > 1 ))
    {
      *pp++ = 'X';                         /* & extra dots */
    }
    else
    {
      *pp++ = *cp;
    }
  }

  *pp = '\0';                              /* Tie it off. */
  cp = name2;                              /* If nothing before dot, */
  if (*work == '.')
  {
    *cp++ = 'X';                           /* insert 'X' */
  }

  strcpy(cp, work);
  debug(F110, " name2", name2, 0);
}

/* Z C H D I R -- Change directory */

int
zchdir(dirnam)
char *dirnam;
{
  char *hd;
  if (*dirnam == '\0')
  {
    hd = getenv("HOME");
  }
  else
  {
    hd = dirnam;
  }

  return (( chdir(hd) == 0 ) ? 1 : 0 );
}

/* Z H O M E -- Return pointer to user's home directory */

#ifndef NOICP
char *
zhome()
{
  return ( getenv("HOME"));
}
#endif /* ifndef NOICP */

/* Z G T D I R -- Return pointer to user's current directory */

char *
zgtdir()
{
#ifdef MAXPATHLEN
#define CWDBL MAXPATHLEN
#else  /* ifdef MAXPATHLEN */
#define CWDBL 100
#endif /* ifdef MAXPATHLEN */

#ifdef UXIII
  char cwdbuf[CWDBL + 1];
  char *buf;
  char *getcwd();
  buf = cwdbuf;
  return ( getcwd(buf, CWDBL));

#else  /* ifdef UXIII */
#ifdef BSD4
  char cwdbuf[CWDBL + 1];
  char *buf;
  char *getwd();
  buf = cwdbuf;
  return ( getwd(buf));

#else  /* ifdef BSD4 */
  return ( "(directory unknown)" );

#endif /* ifdef BSD4 */
#endif /* ifdef UXIII */
}

/* Z X C M D -- Run system command so its output can be read like a file */

#ifndef NOPUSH
int
zxcmd(comand)
char *comand;
{
  int pipes[2];
  if (pipe(pipes) != 0)
  {
    debug(F100, "zxcmd pipe failure", "", 0);
    return ( 0 );                              /* can't make pipe, fail */
  }

  if (( pid = fork()) == 0)                    /* child */
  /*
   * #if BSD4 Code from Dave Tweten@AMES-NAS,
   * readapted to use getpwuid to find login
   * shell, bu H. Fischer.
   */
  {
    char *shpath, *shname, *shptr;             /* to find desired shell */
    struct passwd *p;
    extern struct passwd *getpwuid();
#ifndef __linux__
    extern int getuid();
#endif /* ifndef __linux__ */
    char *defShel = "/bin/sh";                 /* default shell */

    close(pipes[0]);                           /* close in side of pipe */
    close(0);                                  /* close stdin */
    if (open("/dev/null", 0) < 0)
    {
      return ( 0 );                            /* replace input by null */
    }

#ifndef UXIII
    dup2(pipes[1], 1);                         /* replace stdout & stderr */
    dup2(pipes[1], 2);                         /* by the pipe */
#else  /* ifndef UXIII */
    close(1);                                  /* simulate dup2 */
    if (dup(pipes[1]) != 1)
    {
#ifdef NODOHLP
      conol("zxcmd(o) failed\n");
#else /* ifdef NODOHLP */
      conol("trouble duping stdout in routine zxcmd\n");
#endif /* ifdef NODOHLP */
    }

    close(2);                                  /* simulate dup2 */
    if (dup(pipes[1]) != 2)
    {
#ifdef NODOHLP
      conol("zxcmd(e) failed\n");
#else /* ifdef NODOHLP */
      conol("trouble duping stderr in routine zxcmd\n");
#endif /* ifdef NODOHLP */
    }

#endif /* ifndef UXIII */
    close(pipes[1]);                           /* get rid of this copy */
    shpath = getenv("SHELL");                  /* What shell? */
    if (shpath == NULL)
    {
      p = getpwuid(getuid());                  /* get login data */
      if (p == (struct passwd *)NULL || \
        !*( p->pw_shell ))
      {
        shpath = defShel;
      }
      else
      {
        shpath = p->pw_shell;
      }
    }

    shptr = shname = shpath;
    while (*shptr != '\0')
    {
      if (*shptr++ == '/')
      {
        shname = shptr;
      }
    }
    debug(F100, "zxcmd...", "", 0);
    debug(F110, shpath, shname, 0);

    /*
     * Remove the following uid
     * calls if they cause trouble
     */

#ifdef BSD4
#ifndef BSD41
    setegid(getgid());                         /* Override 4.3BSD csh */
    seteuid(getuid());                         /*  security checks */
#endif /* ifndef BSD41 */
#endif /* ifdef BSD4 */
    execl(
      shpath,
      shname,
      "-c",
      comand,
      (char *)NULL);                           /* Execute the cmd */
    exit(0);                                   /* just punt if it failed. */
  }
  else if (pid == -1)
  {
    debug(F100, "zxcmd fork failure", "", 0);
    return ( 0 );
  }

  close(pipes[1]);                             /* don't need the out end */
  fp[ZIFILE] = fdopen(pipes[0], "r");          /* open a stream for it */
  fp[ZSYSFN] = fp[ZIFILE];                     /* Remember it. */
  return ( 1 );
}

/* Z C L O S F -- wait for the child fork to terminate and close the pipe */

int
zclosf()
{
  int wstat;

  if (pid != 0)
  {
    debug(F101, "zclosf pid =", "", pid);
    kill(pid, 9);
    while (( wstat = wait((int *)0)) != pid && wstat != -1)
    {
      ;
    }
    pid = 0;
  }

  fclose(fp[ZIFILE]);
  fp[ZIFILE] = fp[ZSYSFN] = NULL;
  return ( 1 );
}
#endif /* ifndef NOPUSH */

/* Z X P A N D -- Expand a wildcard string into an array of strings */

/*
 * Returns the number of files that match fn1,
 * with data structures set up so that first
 * file (if any) will be returned by the
 * next znext() call.
 */

int
zxpand(fn)
char *fn;
{
  fcount = fgen(fn, mtchs, MAXWLD);            /* Look up the file. */
  if (fcount > 0)
  {
    mtchptr = mtchs;                           /* Save pointer for next */
  }

  return ( fcount );
}

/* Z N E X T -- Get name of next file from list created by zxpand() */

/*
 * Returns >0 if there's another file,
 * with its name copied into the arg
 * string, or 0 if no more files in list.
 */

int
znext(fn)
char *fn;
{
  if (fcount-- > 0)
  {
    strcpy(fn, *mtchptr++);
  }
  else
  {
    *fn = '\0';
  }

  debug(F111, "znext", fn, fcount + 1);
  return ( fcount + 1 );
}

/* Z N E W N -- Make a new name for the given file */

void
znewn(fn, s)
char *fn, **s;
{
#ifdef BSD4
  static char buf[256];
#else  /* ifdef BSD4 */
  static char buf[100];
#endif /* ifdef BSD4 */
  char *bp, *xp;
  int len = 0, n = 0, d = 0, t, i, power = 1;
#ifdef MAXNAMLEN
  int max = MAXNAMLEN;
#else  /* ifdef MAXNAMLEN */
  int max = 14;
#endif /* ifdef MAXNAMLEN */
  bp = buf;
  while (*fn)                               /* Copy name into buf */
  {
    *bp++ = *fn++;
    len++;
  }
  if (len > max - 2)                        /* Don't let it get too long */
  {
    bp = buf + max - 2;
    len = max - 2;
  }

  for (i = 1; i < 4; i++)                   /* Try up to 999 times */
  {
    power *= 10;
    *bp++ = '*';                            /* Put a star on the end */
    *bp-- = '\0';

    n = zxpand(buf);                        /* Expand the result */

    while (n-- > 0)                         /* Find any existing ~d files */
    {
      xp = *mtchptr++;
      xp += len;
      if (*xp == '~')
      {
        t = atoi(xp + 1);
        if (t > d)
        {
          d = t;                            /* Get maximum d */
        }
      }
    }
    if (d < power - 1)
    {
      sprintf(bp, "~%d", d + 1);            /* Make ~(d+1) */
      *s = buf;
      return;
    }

    bp--;
    len--;
  }

  /*
   * If we ever get here, we will
   * overwrite the xxx~100 file...
   */
}

/* Z S A T T R -- Fills attribute structure for the file to be sent */

/*
 * Returns 0 on success with the structure
 * filled in, or -1 on failure. If any string
 * member is null, then it should be ignored.
 * If any numeric member is -1, then it should
 * be ignored.
 */

int
zsattr(xx)
struct zattr *xx;
{
  long k;
  char *zfcdat();

  k = iflen % 1024L;                /* File length in K */
  if (k != 0L)
  {
    k = 1L;
  }

  xx->lengthk = ( iflen / 1024L ) + k;
  xx->type.len = 0;                 /* File type can't be filled in here */
  xx->type.val = "";
  debug(F110, "before calling zfcdat", nambuf, 0);
  if (*nambuf)                      /* XXX(jhj): correct? */
  {
    xx->date.val = zfcdat(nambuf);  /* File creation date */
    xx->date.len = strlen(xx->date.val);
  }
  else
  {
    xx->date.len = 0;
    xx->date.val = "";
  }

  debug(F111, "attr date", xx->date.val, xx->date.len);
  xx->creator.len = 0;              /* File creator */
  xx->creator.val = "";
  xx->account.len = 0;              /* File account */
  xx->account.val = "";
  xx->area.len = 0;                 /* File area */
  xx->area.val = "";
  xx->passwd.len = 0;               /* Area password */
  xx->passwd.val = "";
  xx->blksize = -1L;                /* File blocksize */
  xx->access.len = 0;               /* File access */
  xx->access.val = "";
  xx->encoding.len = 0;             /* Transfer syntax */
  xx->encoding.val = 0;
#ifndef NODISP
  xx->disp.len = 0;                 /* Disposition upon arrival */
  xx->disp.val = "";
#endif /* ifndef NODISP */
  xx->lprotect.len = 0;             /* Local protection */
  xx->lprotect.val = "";
  xx->gprotect.len = 0;             /* Generic protection */
  xx->gprotect.val = "";
  xx->systemid.len = 2;             /* System ID */
  xx->systemid.val = "U1";
  xx->recfm.len = 0;                /* Record format */
  xx->recfm.val = "";
  xx->sysparam.len = 0;             /* System-dependent parameters */
  xx->sysparam.val = "";
  xx->length = iflen;               /* Length */
  return ( 0 );
}

/* Z F C D A T -- Return a string containing the time stamp for a file */

char *
zfcdat(name)
char *name;
{
#ifdef TIMESTAMP
  struct stat buffer;
  struct tm *time_stamp, *localtime();
  static char datbuf[70];

  datbuf[0] = '\0';
  if (stat(name, &buffer) != 0)
  {
    debug(F110, "zcfdat stat failed", name, 0);
    return ( "" );
  }

  time_stamp = localtime(&( buffer.st_mtime ));
  if (time_stamp->tm_year < 1900)
  {
    time_stamp->tm_year += 1900;
  }

  sprintf(
    datbuf,
    "%-4.4d%2.2d%2.2d %2.2d:%2.2d:%2.2d",
    time_stamp->tm_year,
    time_stamp->tm_mon + 1,
    time_stamp->tm_mday,
    time_stamp->tm_hour,
    time_stamp->tm_min,
    time_stamp->tm_sec);
  debug(F111, "zcfdat", datbuf, strlen(datbuf));
  return ( datbuf );

#else  /* ifdef TIMESTAMP */
  return ( "" );

#endif /* ifdef TIMESTAMP */
}

#ifdef TIMEZONE

/*
 * Apparently, the definition of timezone is a big problem.
 * Every computer I looked at deals with it in a different way.
 * On the SUN with SUNOS 4.0, you have to declare it yourself.
 * On a VAX with Ultrix, it's declared in <time.h> as
 * "char * timezone()", or as "extern long timezone" for System V.
 * So to use this, it looks like we'll need an #ifdef per system,
 * OS version, etc.
 */

/*
 * ANSI C mktime for UNIX
 *   by David MacKenzie <edf@rocky2.rockefeller.edu>
 *   and Michael Haertel <mike@stolaf.edu> 08/09/89
 */

/*
 * Convert the exploded time structure `tm', containing a
 * local time and date, into the number of seconds past
 * Jan 1, 1970 GMT. Sets `tm->tm_yday' and `tm->tm_wday'
 * correctly, but doesn't set `tm->tm_isdst'. Doesn't
 * return -1 if passed invalid values.
 */

#ifdef TIMESTAMP
time_t
mktime(tm)
struct tm *tm;
{
  int years, months, days, hours, minutes, seconds;

#ifdef ANYBSD
  struct timeb *tbp;

  ftime(tbp);
  timezone = tbp->timezone * 60;
  if (tbp->dstflag)
  {
    timezone -= 3600;
  }

#endif /* ifdef ANYBSD */

#ifdef UXIII
  tzset();                             /* Set `timezone'. */
#endif /* ifdef UXIII */

  years = tm->tm_year + 1900;          /* year - 1900 -> year */
  months = tm->tm_mon;                 /* 0..11 */
  days = tm->tm_mday - 1;              /* 1..31 -> 0..30 */
  hours = tm->tm_hour;                 /* 0..23 */
  minutes = tm->tm_min;                /* 0..59 */
  seconds = tm->tm_sec;                /* 0..59 */

  /*
   * Set `days' to the number
   * of days into the year.
   */

  if (months > 1 && leap(years))
  {
    ++days;
  }

  while (months-- > 0)
  {
    days += monlens[months];
  }
  tm->tm_yday = days;

  /*
   * Now set `days' to the number
   * of days since Jan 1, 1970.
   */

  days += \
    365 * ( years - 1970 ) + \
    nleap(years);
  tm->tm_wday = \
    ( days + 4 ) % 7;                    /* Jan 1, 1970 was a Thursday. */

  return \
    ( 86400 * days + \
      3600 * hours + \
      60 * minutes + \
      seconds + \
      timezone );
}
#endif /* ifdef TIMESTAMP */
#endif /* ifdef TIMEZONE */

int
zmail(p, f)
char *p;
char *f;
{                                      /* E-mail file f to address p */
#ifdef BSD4

  /*
   * The idea is to use /usr/ucb/mail, rather
   * than regular mail, so that a subject line
   * can be included, with -s. Since we can't
   * depend on the user's path, we use the
   * convention that /usr/ucb/Mail = /usr/ucb/mail
   * and even if Mail has been moved to somewhere
   * else, this should still find it... But there
   * really should be a better way...
   */

  /*
   * Should also make some check
   * on zmbuf overflow...
   */

  sprintf(zmbuf, "Mail -s %cfile %s%c %s < %s", '"', f, '"', p, f);
  system(zmbuf);
#else  /* ifdef BSD4 */
#ifdef UXIII
  sprintf(zmbuf, "mail %s < %s", p, f);
  system(zmbuf);
#else  /* ifdef UXIII */
  *zmbuf = '\0';
#endif /* ifdef UXIII */
#endif /* ifdef BSD4 */
  return ( 0 );
}

int
zprint(p, f)
char *p;
char *f;
{                                      /* Print file f with options p */
#ifdef BSD4
  sprintf(zmbuf, "lpr %s %s", p, f);   /* Construct print command */
  system(zmbuf);
#else  /* ifdef BSD4 */
#ifdef UXIII
  sprintf(zmbuf, "lp %s %s", p, f);
  system(zmbuf);
#else  /* ifdef UXIII */
  *zmbuf = '\0';
#endif /* ifdef UXIII */
#endif /* ifdef BSD4 */
  return ( 0 );
}

/*
 * Directory Functions for UNIX, written
 * by Jeff Damens, CUCCA, 1984.
 *
 * The path structure is used to represent the name to match.
 * Each slash-separated segment of the name is kept in one
 * such structure, and they are linked together, to make
 * traversing the name easier.
 */

struct
path
{
  char npart[MAXNAMLEN];             /* name part of path segment */
  struct path *fwd;                  /* forward ptr */
};

void traverse(struct path *pl, char *sofar, char *endcur);
void addresult(char *str);

#ifdef BSD29
#define SSPACE 500
#else  /* ifdef BSD29 */
#ifndef MINBUF
#define SSPACE 768                   /* XXX 2000 size of string-generating buffer */
#else /* ifndef MINBUF */
#define SSPACE 88
#endif /* ifndef MINBUF */
#endif /* ifdef BSD29 */
static char sspace[SSPACE];          /* buffer to generate names in */
static char *freeptr, **resptr;      /* copies of caller's arguments */
static int remlen;                   /* remaining length in caller's array*/
static int numfnd;                   /* number of matches found */

/*
 * splitpath(p):
 *  takes a string, p, and splits the slash-
 *  separated portions into a list of path
 *  structures. Returns the head of the list.
 *  The structures are allocated by malloc,
 *  so they must be freed. splitpath is used
 *  internally by the filename generator.
 *
 * Input:
 *  A string, p.
 *
 * Returns:
 *  A linked list of the slash-separated
 *  segments of the input string p.
 */

struct path *
splitpath(p)
char *p;
{
  struct path *head, *cur, *prv;
  int i;
  head = prv = NULL;
  if (*p == '/')
  {
    p++;                               /* skip leading slash */
  }

  while (*p != '\0')
  {
    cur = (struct path *)malloc(sizeof ( struct path ));
    debug(F101, "splitpath malloc", "", cur);
    if (cur == NULL)
    {
#ifdef NODOHLP
      fprintf(stderr, "\rmalloc fail\r\n");
#else /* ifdef NODOHLP */
      fprintf(stderr, "\rFatal: malloc fails in splitpath()\r\n");
#endif /* ifdef NODOHLP */
      doexit(BAD_EXIT);
      /* fatal("malloc fails in splitpath()"); */
          exit(BAD_EXIT); /* Appease clang ccc-analyzer */
    }

    cur->fwd = NULL;
    if (head == NULL)
    {
      head = cur;
    }
    else
    {
      prv->fwd = cur;                  /* link into chain */
    }

    prv = cur;
    for (
      i = 0;
      i < MAXNAMLEN - 1 && *p != '/' && *p != '\0';
      i++)
    {
      cur->npart[i] = *p++;
    }

    cur->npart[i] = '\0';              /* end this segment */
    if (i >= MAXNAMLEN)
    {
      while (
        *p != '/' && *p != '\0')
      {
        p++;
      }
    }

    if (*p == '/')
    {
      p++;
    }
  }
  debug(F101, "splitpath head", "", head);
  return ( head );
}

/*
 * fgen:
 *   This is the actual name generator. It is passed a string,
 *   possibly containing wildcards, and an array of character
 *   pointers. It finds all the matching filenames and stores
 *   them into the array. The returned strings are allocated
 *   from a static buffer local to this module (so the caller
 *   doesn't have to worry about deallocating them); this means
 *   that successive calls to fgen will wipe out the results of
 *   previous calls. This isn't a problem here because we
 *   process one wildcard string at a time.
 *
 * Input:   a wildcard string, an array to write names to,
 *          the length of the array.
 *
 * Returns: the number of matches. The array is filled with filenames
 *          that matched the pattern. If there wasn't enough room in
 *          the array, -1 is returned.
 *
 * By:      Jeff Damens, CUCCA, 1984.
 */

int
fgen(pat, resarry, len)
char *pat, *resarry[];
int len;
{
  struct path *head;
  char scratch[100], *sptr;
  head = splitpath(pat);
  if (*pat == '/')
  {
    scratch[0] = '/';
    sptr = scratch + 1;
  }
  else
  {
    strcpy(scratch, "./");
    sptr = scratch + 2;
  }                                  /* init buffer correctly */

  numfnd = 0;                        /* none found yet */
  freeptr = sspace;                  /* this is where matches are copied */
  resptr = resarry;                  /* static copies of these so*/
  remlen = len;                      /* recursive calls can alter them */
  traverse(head, scratch, sptr);     /* go walk the directory tree */
  for (; head != NULL; head = head->fwd)
  {
    free(head);                      /* return the path segments */
  }
  return ( numfnd );                 /* and return the number of matches */
}

/*
 * traverse:
 *   Walks the directory tree looking for matches to its arguments.
 *     The algorithm is, briefly:
 *       If the current pattern segment contains no wildcards, that
 *       segment is added to what we already have.  If the name so far
 *       exists, we call ourselves recursively with the next segment
 *       in the pattern string; otherwise, we just return.
 *
 *  If the current pattern segment contains wildcards, we open the name
 *    we've accumulated so far (assuming it is really a directory), then
 *    read each filename in it, and, if it matches the wildcard pattern
 *    segment, add that filename to what we have so far and call ourselves
 *    recursively on the next segment.
 *
 *  Finally, when no more pattern segments remain, we add what's
 *    accumulated so far to the result array and increment the
 *    number of matches.
 *
 * Input: a pattern path list (as generated by splitpath), a string
 *        pointer that points to what we've traversed so far (this
 *        can be initialized to "/" to start the search at the root
 *        directory, or to "./" to start the search at the current
 *        directory), and a string pointer to the end of the string
 *        in the previous argument.
 *
 * Returns: nothing.
 */

void
traverse(pl, sofar, endcur)
struct path *pl;
char *sofar, *endcur;
{
#ifdef BSD42
  DIR *fd, *opendir();
  struct direct *dirbuf;
#else  /* ifdef BSD42 */
#ifdef BSD29
  DIR *fd, *opendir();
  struct direct *dirbuf;
#else  /* ifdef BSD29 */
  int fd;
#ifdef DIRENT
  struct dirent dir_entry;
  struct dirent *dirbuf = &dir_entry;
#else  /* ifdef DIRENT */
  struct direct dir_entry;
  struct direct *dirbuf = &dir_entry;
#endif /* ifdef DIRENT */
#endif /* ifdef BSD29 */
#endif /* ifdef BSD42 */
  struct stat statbuf;
  if (pl == NULL)
  {
    *--endcur = '\0';                 /* end string, overwrite trailing / */
    addresult(sofar);
    return;
  }

  if (!iswild(pl->npart))
  {
    strcpy(endcur, pl->npart);
    endcur += strlen(pl->npart);
    *endcur = '\0';                   /* end current string */
    if (stat(sofar, &statbuf) == 0)   /* if current piece exists */
    {
      *endcur++ = '/';                /* add slash to end */
      *endcur = '\0';                 /* and end the string */
      traverse(pl->fwd, sofar, endcur);
    }

    return;
  }

  /*
   * segment contains wildcards,
   * have to search directory
   */

  *endcur = '\0';                     /* end current string */
  if (stat(sofar, &statbuf) == -1)
  {
    return;                           /* doesn't exist, forget it */
  }

  if (( statbuf.st_mode & S_IFDIR ) == 0)
  {
    return;                           /* not a directory, skip */
  }

#ifdef BSD42
  if (( fd = opendir(sofar)) == NULL)
  {
    return;                           /* can't open, forget it */
  }

  while (( dirbuf = readdir(fd) ))
#else  /* ifdef BSD42 */
#ifdef BSD29
  if (( fd = opendir(sofar)) == NULL)
  {
    return;                           /* can't open, forget it */
  }

  while (dirbuf = readdir(fd))
#else  /* ifdef BSD29 */
  if (( fd = open(sofar, O_RDONLY)) < 0)
  {
    return;                           /* can't open, forget it */
  }

  while (read(fd, dirbuf, sizeof dir_entry))
#endif /* ifdef BSD29 */
#endif /* ifdef BSD42 */
  {
    strncpy(
      nambuf,
      dirbuf->d_name,
      MAXNAMLEN);                     /* Get a null terminated copy!!! */
    nambuf[MAXNAMLEN] = '\0';
#ifdef unos
    if (dirbuf->d_ino != -1 && match(pl->npart, nambuf))
    {
#else  /* ifdef unos */
    if (dirbuf->d_ino != 0 && match(pl->npart, nambuf))
    {
#endif /* ifdef unos */
      char *eos;
      strcpy(endcur, nambuf);
      eos = endcur + strlen(nambuf);
      *eos = '/';                     /* end this segment */
      traverse(pl->fwd, sofar, eos + 1);
    }
  }
#ifdef BSD42
  closedir(fd);
#else  /* ifdef BSD42 */
#ifdef BSD29
  closedir(fd);
#else  /* ifdef BSD29 */
  close(fd);
#endif /* ifdef BSD29 */
#endif /* ifdef BSD42 */
}

/*
 * addresult:
 *   Adds a result string to the result array.
 *   Increments the number of matches found,
 *   copies the found string into our string
 *   buffer, and puts a pointer to the buffer
 *   into the caller's result array. Our free
 *   buffer pointer is updated. If there is no
 *   more room in the caller's array, the number
 *   of matches is set to -1.
 *
 * Input: a result string.
 *
 * Returns: nothing.
 */

void
addresult(str)
char *str;
{
  int l;
  if (strncmp(str, "./", 2) == 0)
  {
    str += 2;
  }

  if (--remlen < 0)
  {
    numfnd = -1;
    return;
  }

  l = strlen(str) + 1;              /* size this will take up */
  if (( freeptr + l ) > &sspace[SSPACE - 1])
  {
    numfnd = -1;                    /* do not record if not enough space */
    return;
  }

  strcpy(freeptr, str);
  *resptr++ = freeptr;
  freeptr += l;
  numfnd++;
}

int
iswild(str)
char *str;
{
  char c;
  while (( c = *str++ ) != '\0')
  {
    if (c == '*' || c == '?')
    {
      return ( 1 );
    }
  }
  return ( 0 );
}

/*
 * match:
 *   A pattern matcher. Takes a string and a
 *   pattern possibly containing the wildcard
 *   characters '*' and '?'. Returns true if
 *   the pattern matches the string, and false
 *   otherwise.
 *
 * by: Jeff Damens, CUCCA
 *
 * Input: a string and a wildcard pattern.
 *
 * Returns: 1 if match, 0 if no match.
 */

int
match(pattern, string)
char *pattern, *string;
{
  char *psave, *ssave;                            /* back up pointers */
  psave = ssave = NULL;
  while (1)
  {
    for (;
      *pattern == *string;
        pattern++, string++)                      /* skip first */
    {
      if (*string == '\0')
      {
        return ( 1 );                             /* end of strings, OK */
      }
    }

    if (*string != '\0' && *pattern == '?')
    {
      pattern++;                                  /* '?', let it match */
      string++;
    }
    else if (*pattern == '*')                     /* '*' ... */
    {
      psave = ++pattern;                          /* remember it */
      ssave = string;                             /* let it match 0 chars */
    }
    else if (ssave != NULL && *ssave != '\0')     /* if not at end  */
                                                  /* ...have seen a star */
    {
      string = ++ssave;                           /* skip char in string */
      pattern = psave;                            /* and back up pattern */
    }
    else
    {
      return ( 0 );                               /* otherwise just fail */
    }
  }
}

/*
 * The following two functions are for
 * expanding tilde in filenames.
 * Contributed by Howie Kaye, CUCCA,
 * developed for CCMD package.
 */

/*
 * WHOAMI:
 * 1) Get real uid
 * 2) See if the $USER environment variable is set ($LOGNAME on AT&T)
 * 3) If $USER's uid is the same as realuid, realname is $USER
 * 4) Otherwise get logged in user's name
 * 5) If that name has the same uid as the real uid realname is loginname
 * 6) Otherwise, get a name for realuid from /etc/passwd
 */

char *
whoami()
{
#ifdef DTILDE
  static char realname[32];                        /* user's name */
  static int  realuid = -1;                        /* user's real uid */
  char        loginname[32], envname[32];          /* temp storage */
  char        *getlogin(),   *getenv(),   *c;
  struct      passwd *p,     *getpwnam(),
              *getpwuid(),   *getpwent();

  if (realuid != -1)
  {
    return ( realname );
  }

  realuid = getuid();                         /* get our uid */

  /*
   * How about ...
   * $USER or $LOGNAME?
   */

  if (( c = getenv(NAMEENV)) != NULL)         /* check the env variable */
  {
    strcpy(envname, c);
    p = getpwnam(envname);
    if (p->pw_uid == (unsigned int)realuid)   /* get passwd entry */
                                              /* for envname */
    {
      strcpy(realname, envname);              /* if the uid are the same */
      return ( realname );
    }
  }

  /*
   * Can we use ...
   * loginname()?
   */

  if (( c = getlogin()) != NULL)                /* name from utmp file */
  {
    strcpy(loginname, c);
    if (( p = getpwnam(loginname)) != NULL)     /* get passwd entry */
    {
      if (p->pw_uid == (unsigned int)realuid)   /* for loginname */
      {
        strcpy(realname, loginname);            /* if uid's are the same */
        return ( realname );
      }
    }
  }

  /*
   * Use first name we
   * get for realuid
   */

  if (( p = getpwuid(realuid)) == NULL)         /* name for uid */
  {
    realname[0] = '\0';                         /* no user name */
    realuid = -1;
    return ( NULL );
  }

  strcpy(realname, p->pw_name);
  return ( realname );

#else  /* ifdef DTILDE */
  return ( NULL );

#endif /* ifdef DTILDE */
}

/*
 * expand ~user to the
 * user's home directory.
 */

#define DIRSEP '/'
char *
tilde_expand(dirname)
char *dirname;
{
#define BUFLEN 256
#ifdef DTILDE
  struct passwd *user, *getpwuid(), *getpwnam();
  static char olddir[BUFLEN];
  static char oldrealdir[BUFLEN];
  static char temp[BUFLEN];
  int i, j;
  char *whoami();

  debug(F111, "tilde_expand dirname", dirname, dirname[0]);
  if (dirname[0] != '~')
  {
    return ( dirname );                   /* not a tilde...return param */
  }

  if (!strcmp(olddir, dirname))
  {
    return ( oldrealdir );                /* same as last time. */
  }
  else                                    /* so return old answer */
  {
    j = strlen(dirname);
    for (i = 0; i < j; i++)               /* find username part of string */
    {
      if (dirname[i] != DIRSEP)
      {
        temp[i] = dirname[i];
      }
      else
      {
        break;
      }
    }

    temp[i] = '\0';                       /* tie off with a NULL */
    if (i == 1)                           /* if just a "~" */
    {
      user = getpwnam(whoami());          /*  get info on current user */
    }
    else
    {
      user = getpwnam(&temp[1]);          /* otherwise the specified user */
    }
  }

  if (user != NULL)                       /* valid user? */
  {
    strcpy(olddir, dirname);              /* remember the directory */
    strcpy(oldrealdir, user->pw_dir);     /* and their home directory */
    strcat(oldrealdir, &dirname[i]);
    return ( oldrealdir );
  }
  else                                    /* invalid? */
  {
    strcpy(olddir, dirname);              /* remember for next time */
    strcpy(oldrealdir, dirname);
    return ( oldrealdir );
  }

#else  /* ifdef DTILDE */
  return ( NULL );

#endif /* ifdef DTILDE */
}
