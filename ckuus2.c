/* C K U U S 2 -- "User Interface" STRINGS module */

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

#include "ckcdeb.h"
#include "ckcker.h"
#include "ckucmd.h"
#include "ckuusr.h"
#include <ctype.h>
#include <stdio.h>

#ifdef __linux__
#include <string.h>
#endif /* ifdef __linux__ */

#ifndef XENIX
#ifndef unos
#include <sys/file.h>             /* File information */
#endif /* ifndef unos */
#endif /* ifndef XENIX */

#ifdef BSD4
#include <fcntl.h>
#include <sys/file.h>
#endif /* ifdef BSD4 */

#ifdef UXIII
#include <termio.h>
#include <sys/ioctl.h>
#include <errno.h>                /* error numbers for system returns */
#include <fcntl.h>                /* directory reading for locking */
#endif /* ifdef UXIII */

#ifdef HPUX
#include <sys/modem.h>
#endif /* ifdef HPUX */

#ifndef UXIII
#include <sgtty.h>                /* Set/Get tty modes */
#ifndef V7
#ifndef BSD41
#include <sys/time.h>             /* Clock info (for break generation) */
#endif /* ifndef BSD41 */
#endif /* ifndef V7 */
#endif /* ifndef UXIII */

#ifdef BSD41
#include <sys/timeb.h>            /* BSD 4.1 */
#endif /* ifdef BSD41 */

#ifdef BSD29
#include <sys/timeb.h>            /* BSD 2.9 */
#endif /* ifdef BSD29 */

#ifdef ultrix
#include <sys/ioctl.h>
#endif /* ifdef ultrix */

extern CHAR mystch, stchr, eol, seol, padch, mypadc, ctlq;
extern CHAR data[], *rdatap, ttname[];
extern char cmdbuf[], line[], debfil[], pktfil[], sesfil[], trafil[];
extern int nrmt, nprm, dfloc, deblog, seslog, speed, local, parity, duplex;
extern int turn, turnch, pktlog, tralog, mdmtyp, flow, cmask, timef, spsizf;
extern int rtimo, timint, srvtim, npad, mypadn, bctr, delay;
extern int maxtry, spsiz, urpsiz, maxsps, maxrps, ebqflg, ebq;
extern int rptflg, rptq, fncnv, binary, pktlog, warn, quiet, fmask, keep;
extern int tsecs, bctu, len, lpcapu, swcapu, wslots, sq, rpsiz;
extern int capas;
#ifndef NOATTR
extern int atcapr;
extern int atcapu;
#endif /* ifndef NOATTR */
extern long filcnt, tfc, tlci, tlco, ffc, flci, flco;
extern char *dftty, *versio, *ckxsys;
extern struct keytab prmtab[];
extern struct keytab remcmd[];
#ifndef NODOHLP
int bcharc;
#endif /* ifndef NODOHLP */

static char *hlp1[] = {
  "\rUsage: [ -x arg [ -x arg ] [ -yy ] ] ]\n",
#ifndef NODOHLP
  "    ACTION -- (* options require setting '-l' and '-b')\n",
  "        -s file(s) send (use '-s -' for stdin)\n",
  "        -r         receive\n",
  "        -k         receive to console\n",
  "      * -g file(s) get remote file(s) from server (quote wildcards)\n",
  "        -a name    alternate name (for '-s', '-r', and '-g')\n",
#ifndef NOSERVER
  "        -x         start server mode\n",
#endif /* ifndef NOSERVER */
  "      * -f         send finish command to remote\n",
#ifndef NOCONN
  "      * -c         connect pre-transaction\n",
  "      * -n         connect post-transaction\n",
#endif /* ifndef NOCONN */
  "   SETTING --\n",
  "        -l line    line device (e.g. '/dev/ttyS1')\n",
  "        -b baud    baud (e.g. '9600')\n",
  "        -i         disable binary mode (do text conversion)\n",
  "        -p x       parity ('e', 'o', 'm', 's', or 'n')\n",
  "        -t         set line turnaround to XON (half duplex)\n",
  "        -w         overwrite existing files\n",
  "        -q         quiet mode (no status display)\n",
#ifndef NOLOGS
#ifdef DEBUG
  "        -d         debug mode (write debug.log)\n",
#endif /* ifdef DEBUG */
#endif /* ifndef NOLOGS */
  "        -e length  set extended receive packet length\n",
#ifndef NOICP
  "If no ACTION is specified, uCKermit enters interactive mode.\n",
#endif /* ifndef NOICP */
#endif /* ifndef NODOHLP */
  ""
};

/* U S A G E */

int
usage()
{
  conola(hlp1);
  return ( 0 );
}

/*
 * Help string
 * definitions
 */

#ifndef NODOHLP
static char *tophlp[] = {
  "\n\
Type '?' for a list of commands; Type 'help X' for any command 'X'.\n\
In interactive mode, the following characters have special meaning:\n\n\
 CTRL-H:  (or BACKSPACE, DELETE): Delete the most recent character.\n\
 CTRL-W:  Delete the most recent full word.\n",
"\
 CTRL-U:  Delete the current full line.\n\
 CTRL-R:  Redisplay the current full line.\n\
 CTRL-L:  Clear the display and then execute the current full line.\n\
      ?:  (QUESTION) Display help for the current command or field.\n\
 ESCAPE:  (or TAB) Attempt completing the current command or field.\n",
"\
      \\:  (BACKSLASH) Include, literally, the next input character.\n\n",
  ""
};
#endif /* ifndef NODOHLP */

#ifndef NODOHLP
static char *hmxxbye = "\
Shut down and log out a remote Kermit server";
#endif /* ifndef NODOHLP */

#ifndef NODOHLP
static char *hmxxclo =
  "\
Close one of the following logs:\n\
 session, transaction, packet, debugging -- 'help log' for further info.";
#endif /* ifndef NODOHLP */

#ifndef NODOHLP
static char *hmxxcon =
  "\
Connect to a remote system via the tty device given in the\n\
most recent 'set line' command";
#endif /* ifndef NODOHLP */

#ifndef NODOHLP
static char *hmxxget =
  "\
Format: 'get filespec'.  Tell the remote Kermit server to send the named\n\
files.  If filespec is omitted, then you are prompted for the remote and\n\
local filenames separately.";
#endif /* ifndef NODOHLP */

#ifndef NODOHLP
static char *hmxxlg[] = {
  "\
Record information in a log file:\n\n\
 debugging             Debugging information, to help track down\n\
  (default debug.log)  bugs in the uCKermit program.\n\n\
 packets               Kermit packets, to help track down protocol problems.\n\
  (packet.log)\n\n",

  " session               Terminal session, during CONNECT command.\n\
  (session.log)\n\n\
 transactions          Names and statistics about files transferred.\n\
  (transact.log)\n",
  ""
};
#endif /* ifndef NODOHLP */

#ifndef NOCKUSCR
#ifndef NODOHLP
static char *hmxxlogi[] = {
  "\
Syntax: script text\n\n",
  "Login to a remote system using the text provided.  The login script\n",
  "is intended to operate similarly to uucp \"L.sys\" entries.\n",
  "A login script is a sequence of the form:\n\n",
  "       expect send [expect send] . . .\n\n",
  "where 'expect' is a prompt or message to be issued by the remote site, "
  "and\n",
  "'send' is the names, numbers, etc, to return.  The send may also be the\n",
  "keyword EOT, to send control-d, or BREAK, to send a break.  Letters in\n",
  "send may be prefixed by ~ to send special characters.  These are:\n",
  "~b backspace, ~s space, ~q '?', ~n linefeed, ~r return, ~c don\'t\n",
  "append a return, and ~o[o[o]] for octal of a character.  As with some \n",
  "uucp systems, sent strings are followed by ~r unless they end with "
  "~c.\n\n",
  "Only the last 7 characters in each expect are matched.  A null expect,\n",
  "e.g. ~0 or two adjacent dashes, causes a short delay.  If you expect\n",
  "that a sequence might not arrive, as with uucp, conditional sequences\n",
  "may be expressed in the form:\n\n",
  "       -send-expect[-send-expect[...]]\n\n",
  "where dashed sequences are followed as long as previous expects fail.\n",
  ""
};
#endif /* ifndef NODOHLP */
#endif /* ifndef NOCKUSCR */

#ifndef NODOHLP
static char *hmxxrc[] = {
  "\
Format: 'receive [filespec]'.  Wait for a file to arrive from the other\n\
Kermit, which must be given a 'send' command.  If the optional filespec is\n",

  "given, the (first) incoming file will be stored under that name, otherwise\n\
it will be stored under the name it arrives with.",
  ""
};
#endif /* ifndef NODOHLP */

#ifndef NODOHLP
static char *hmxxsen =
  "\
Format: 'send file1 [file2]'.  File1 may contain wildcard characters '*' or\n\
'?'.  If no wildcards, then file2 may be used to specify the name file1 is\n\
sent under; if file2 is omitted, file1 is sent under its own name.";
#endif /* ifndef NODOHLP */

#ifndef NODOHLP
static char *hmxxser =
  "\
Enter server mode on the currently selected line.  All further commands\n\
will be taken in packet form from the other Kermit program.";
#endif /* ifndef NODOHLP */

#ifndef NODOHLP
static char *hmhset[] = {
  "\
The 'set' command is used to establish various communication or file\n",
  "parameters.  The 'show' command can be used to display the values of\n",
  "'set' parameters.  Help is available for each individual parameter;\n",
  "type 'help set ?' to see what's available.\n", ""
};
#endif  /* ifndef NODOHLP */

#ifndef NODOHLP
static char *hmxychkt[] = {
  "\
Type of packet block check to be used for error detection, 1, 2, or 3.\n",
  "Type 1 is standard, and catches most errors.  "
  "Types 2 and 3 specify more\n",
  "rigorous checking at the cost of higher overhead.  "
  "Not all Kermit programs\n",
  "support types 2 and 3.\n", ""
};
#endif /* ifndef NODOHLP */

#ifndef NODOHLP
static char *hmxyf[] = {
  "\
set file: names, type, overwrite, display.\n\n",
  "'names' are normally 'converted', which means file names are converted\n",
  "to 'common form' during transmission; 'literal' means use filenames\n",
  "literally (useful between like system types).\n\n",
  "'type' defaults to 'binary' with no conversion of local and remote\n",
  "newlines and CR-LF line delimiters; 'text' enables this conversion.\n",
  "Use 'binary' for executable programs or data, and 'text' otherwise.\n\n",
  "'overwrite' is 'on' or 'off', normally off.  When on, incoming files\n",
  "overwrite existing files of the same name.  When off, new names will be\n",
  "given to incoming files whose names are the same as existing files.\n",
  "\n\
'display' is normally 'on', causing file transfer progress to be displayed\n",
  "on your screen when in local mode.  'set display off' is useful for\n",
  "allowing file transfers to proceed in the background.\n\n",
  ""
};
#endif /* ifndef NODOHLP */

#ifndef NODOHLP
static char *hmhrmt[] = {
  "\
The 'remote' command is used to send file management instructions to a\n",
  "remote Kermit server.  There should already be a Kermit running in "
  "server\n",
  "mode on the other end of the currently selected line.  Type 'remote ?' "
  "to\n",
  "see a list of available remote commands.  Type 'help remote x' to get\n",
  "further information about a particular remote command 'x'.\n",
  ""
};
#endif /* ifndef NODOHLP */

/* D O H L P -- Give a help message */

#ifndef NODOHLP
int
dohlp(xx)
int xx;
{
  int x;
  int y;


  if (xx < 0)
  {
    return ( xx );
  }

  switch (xx)
  {
  case XXBYE:
    return ( hmsg(hmxxbye) );

  case XXCLO:
    return ( hmsg(hmxxclo) );

#ifndef NOCONN
  case XXCON:
    return ( hmsg(hmxxcon) );
#endif /* ifndef NOCONN */

  case XXCWD:
    return ( hmsg(
               "Change Working Directory") );

  case XXDEL:
    return ( hmsg("Delete a local file or files") );

#ifndef NOCKUDIA
  case XXDIAL:
    return ( hmsg("Dial a number using modem autodialer") );
#endif /* ifndef NOCKUDIA */

  case XXDIR:
    return ( hmsg("Display a directory of local files") );

  case XXECH:
    return ( hmsg(
               "Display the rest of the command on the terminal,\n\
useful in command files."));

  case XXEXI:
  case XXQUI:
    return ( hmsg("Exit the program, clesing any open logs.") );

  case XXFIN:
    return ( hmsg(
               "\
Tell the remote Kermit server to shut down without logging out.") );

  case XXGET:
    return ( hmsg(hmxxget) );

#ifndef NOCKUDIA
  case XXHAN:
    return ( hmsg("Hang up the phone.") );
#endif /* ifndef NOCKUDIA */

  case XXHLP:
    return ( hmsga(tophlp) );

#ifndef NOLOGS
  case XXLOG:
    return ( hmsga(hmxxlg) );
#endif /* ifndef NOLOGS */

#ifndef NOCKUSCR
  case XXLOGI:
    return ( hmsga(hmxxlogi) );
#endif /* ifndef NOCKUSCR */

  case XXREC:
    return ( hmsga(hmxxrc) );

  case XXREM:
    if (( y = cmkey(remcmd, nrmt, "Remote command", "")) == -2)
    {
      return ( y );
    }

    if (y == -1)
    {
      return ( y );
    }

    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    return ( dohrmt(y) );

  case XXSEN:
    return ( hmsg(hmxxsen) );

  case XXSER:
    return ( hmsg(hmxxser) );

  case XXSET:
    if (( y = cmkey(prmtab, nprm, "Parameter", "")) == -2)
    {
      return ( y );
    }

    if (y == -2)
    {
      return ( y );
    }

    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    return ( dohset(y) );

#ifndef NOPUSH
  case XXSHE:
    return ( hmsg(
               "\
Issue a command to the UNIX shell (space required after '!')") );

#endif /* ifndef NOPUSH */

  case XXSHO:
    return ( hmsg(
               "\
Display current values of 'set' parameters; 'show version' will display\n\
program version information for each of the uCKermit modules.") );

  case XXSPA:
    return ( hmsg("Display disk usage in current device, directory") );

#ifndef NOSTATS
  case XXSTA:
    return ( hmsg("Display statistics about most recent file transfer") );
#endif /* ifndef NOSTATS */

  case XXTAK:
    return ( hmsg(
               "\
Take Kermit commands from the named file.  Kermit command files may\n\
themselves contain 'take' commands, up to a reasonable depth of nesting."));

  case XXTRA:
    return ( hmsg(
               "\
Raw upload. Send a file, a line at a time (text) or a character at a time.\n\
For text, wait for turnaround character (default 10 = LF) after each line.\n\
Specify 0 for no waiting."));

  default:
    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }
    printf("Not available yet - %s\n", cmdbuf);
    break;
  }
  return ( 0 );
}
#endif /* ifndef NODOHLP */

/* H M S G -- Get confirmation, then print the given message */

#ifndef NODOHLP
int
hmsg(s)
char *s;
{
  int x;
  if (( x = cmcfm()) < 0)
  {
    return ( x );
  }

  puts(s);
  return ( 0 );
}

int
hmsga(s)
char *s[];
{ /* Same function, but for arrays */
  int x, i;
  if (( x = cmcfm()) < 0)
  {
    return ( x );
  }

  for (i = 0; *s[i]; i++)
  {
    fputs(s[i], stdout);
  }

  putc('\n', stdout);
  return ( 0 );
}
#endif /* ifndef NODOHLP */

/* B C A R C B -- Format helper for baud rates */

#ifndef NODOHLP
#ifndef NOICP
int
bcarcb(calsp)
long calsp;
{
#ifndef MAXBRATE
#ifdef B4000000
#define MAXBRATE 4000000
#endif /* ifdef B4000000 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B3500000
#define MAXBRATE 3500000
#endif /* ifdef B3500000 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B2500000
#define MAXBRATE 2500000
#endif /* ifdef B2500000 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B2000000
#define MAXBRATE 2000000
#endif /* ifdef B2000000 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B1152000
#define MAXBRATE 1152000
#endif /* ifdef B1152000 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B1000000
#define MAXBRATE 1000000
#endif /* ifdef B1000000 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B921600
#define MAXBRATE 921600
#endif /* ifdef B921600 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B576000
#define MAXBRATE 576000
#endif /* ifdef B576000 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B500000
#define MAXBRATE 500000
#endif /* ifdef B500000 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B460000
#define MAXBRATE 460000
#endif /* ifdef B460000 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B230000
#define MAXBRATE 230000
#endif /* ifdef B230000 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B115200
#define MAXBRATE 115200
#endif /* ifdef B115200 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B76800
#define MAXBRATE 76800
#endif /* ifdef B76800 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B57600
#define MAXBRATE 57500
#endif /* ifdef B57600 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B38400
#define MAXBRATE 38400
#endif /* ifdef B38400 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B19200
#define MAXBRATE 19200
#endif /* ifdef B19200 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B9600
#define MAXBRATE 9600
#endif /* ifdef B9600 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B4800
#define MAXBRATE 4800
#endif /* ifdef B4800 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B2400
#define MAXBRATE 2400
#endif /* ifdef B2400 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B1800
#define MAXBRATE 1800
#endif /* ifdef B1800 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B1200
#define MAXBRATE 1200
#endif /* ifdef B1200 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B600
#define MAXBRATE 600
#endif /* ifdef B600 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B300
#define MAXBRATE 300
#endif /* ifdef B300 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B200
#define MAXBRATE 200
#endif /* ifdef B200 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B150
#define MAXBRATE 150
#endif /* ifdef B150 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B134
#define MAXBRATE 134
#endif /* ifdef B134 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B110
#define MAXBRATE 110
#endif /* ifdef B110 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B75
#define MAXBRATE 75
#endif /* ifdef B75 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B50
#define MAXBRATE 50
#endif /* ifdef B50 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef B0
#define MAXBRATE 0
#endif /* ifdef B0 */
#endif /* ifndef MAXBRATE */

#ifndef MAXBRATE
#ifdef __linux__ /* ifdef __linux__ */
#define MAXBRATE 38400
#else /* ifdef __linux__ */
#define MAXBRATE 19200
#endif /* ifdef __linux__ */
#endif /* ifndef MAXBRATE */

  int brpln = 0;

  if (calsp < 0)
  {
    brpln = 4;
  }

  if (calsp > 10)
  {
    brpln = 5;
  }

  if (calsp > 100)
  {
    brpln = 6;
  }

  if (calsp > 1000)
  {
    brpln = 7;
  }

  if (calsp > 10000)
  {
    brpln = 8;
  }

  if (calsp > 100000)
  {
    brpln = 9;
  }

  if (calsp > 1000000)
  {
    brpln = 10;
  }

  if (calsp > 10000000)
  {
    brpln = 11;
  }

  printf(" %ld", calsp);

  if (calsp >= MAXBRATE)
  {
    printf(".\n");
  }
  else
  {
    printf(",");
  }

  bcharc = bcharc + brpln;

  if (bcharc >= 66)
  {
    printf("\n");
    bcharc = 0;
  }
  return ( 0 );
}
#endif /* ifndef NOICP */
#endif /* ifndef NODOHLP */

/* D O H S E T -- Give help for SET command */

#ifndef NODOHLP
int
dohset(xx)
int xx;
{
  if (xx == -3)
  {
    return ( hmsga(hmhset) );
  }

  if (xx < 0)
  {
    return ( xx );
  }

  switch (xx)
  {

#ifndef NOATTRIB
  case XYATTR:
    puts("Turn Attribute packet exchange off or on");
    return ( 0 );
#endif /* ifndef NOATTRIB */

  case XYIFD:
    puts("Discard or Keep incompletely received files, default is discard");
    return ( 0 );

  case XYCHKT:
    return ( hmsga(hmxychkt) );

  case XYDELA:
    puts(
      "\
Number of seconds to wait before sending first packet after 'send' command.");
    return ( 0 );

  case XYTERM:
    puts(
      "\
'set terminal bytesize 7 or 8' to use 7- or 8-bit terminal characters.");
    return ( 0 );

  case XYDUPL:
    puts(
      "\
During 'connect': 'full' means remote host echoes, 'half' means this program");
    puts("does its own echoing.");
    return ( 0 );

  case XYESC:
    printf(
      "%s",
      "\
Decimal ASCII value for escape character during 'connect', normally 28\n\
(Control-\\)\n");
    return ( 0 );

  case XYFILE:
    return ( hmsga(hmxyf) );

  case XYFLOW:
    puts(
      "\
Type of flow control to be used.  Choices are 'xon/xoff' and 'none'.");
    puts("normally xon/xoff.");
    return ( 0 );

  case XYHAND:
    puts(
      "\
Decimal ASCII value for character to use for half duplex line turnaround");
    puts("handshake.  Normally, handshaking is not done.");
    return ( 0 );

  case XYLINE:
    printf(
      "\
Device name of communication line to use.  Normally %s.\n",
      dftty);
    if (!dfloc)
    {
      printf(
        "\
If you set the line to other than %s, then Kermit\n",
        dftty);
      printf(
        "\
will be in 'local' mode; 'set line' will reset Kermit to remote mode.\n");
#ifndef NOCKUDIA
      puts(
        "\
If the line has a modem, and if the modem-dialer is set to direct, this");
      puts(
        "\
command causes waiting for a carrier detect (e.g. on a hayes type modem).");
      puts("\
This can be used to wait for incoming calls.");
      puts(
        "\
To use the modem to dial out, first set modem-dialer (e.g., to hayes), then");
      puts("set line, next issue the dial command, and finally connect.");
#endif /* ifndef NOCKUDIA */
    }
    return ( 0 );

#ifndef NOCKUDIA
  case XYMODM:
    puts(
      "\
Type of modem for dialing remote connections.  Needed to indicate modem can");
    puts(
      "\
be commanded to dial without 'carrier detect' from modem.  Many recently");
    puts(
      "\
manufactured modems use 'hayes' protocol.  Type 'set modem ?' to see what");
    puts("\
types of modems are supported by this program.");
    return ( 0 );
#endif /* ifndef NOCKUDIA */

  case XYPARI:
    puts("Parity to use during terminal connection and file transfer:");
    puts("even, odd, mark, space, or none.  Normally none.");
    return ( 0 );

  case XYPROM:
    puts("Prompt string for this program, normally 'uCKermit>'.");
    return ( 0 );

  case XYRETR:
    puts(
      "How many times to retransmit a packet before giving up");
    return ( 0 );

  case XYSPEE:
    puts(
      "Set speed of line specified by last 'set line' command to:");
#ifdef B0
    bcarcb(0);
#endif /* ifdef B0 */
#ifdef B50
    bcarcb(50);
#endif /* ifdef B50 */
#ifdef B75
    bcarcb(75);
#endif /* ifdef B75 */
#ifdef B110
    bcarcb(110);
#endif /* ifdef B110 */
#ifdef B134
    bcarcb(134);
#endif /* ifdef B134 */
#ifdef B150
    bcarcb(150);
#endif /* ifdef B150 */
#ifdef B200
    bcarcb(200);
#endif /* ifdef B200 */
#ifdef B300
    bcarcb(300);
#endif /* ifdef B300 */
#ifdef B600
    bcarcb(600);
#endif /* ifdef B600 */
#ifdef B1200
    bcarcb(1200);
#endif /* ifdef B1200 */
#ifdef B1800
    bcarcb(1800);
#endif /* ifdef B1800 */
#ifdef B2400
    bcarcb(2400);
#endif /* ifdef B2400 */
#ifdef B4800
    bcarcb(4800);
#endif /* ifdef B4800 */
#ifdef B9600
    bcarcb(9600);
#endif /* ifdef B9600 */
#ifdef B19200
    bcarcb(19200);
#endif /* ifdef B19200 */
#ifdef B38400
    bcarcb(38400);
#endif /* ifdef B38400 */
#ifdef B57600
    bcarcb(57600);
#endif /* ifdef B57600 */
#ifdef B115200
    bcarcb(115200);
#endif /* ifdef B115200 */
#ifdef B230400
    bcarcb(230400);
#endif /* ifdef B230400 */
#ifdef B460800
    bcarcb(460800);
#endif /* ifdef B460800 */
#ifdef B500000
    bcarcb(500000);
#endif /* ifdef B500000 */
#ifdef B921600
    bcarcb(921600);
#endif /* ifdef B921600 */
#ifdef B1000000
    bcarcb(1000000);
#endif /* ifdef B1000000 */
#ifdef B1152000
    bcarcb(1152000);
#endif /* ifdef B1152000 */
#ifdef B1500000
    bcarcb(1500000);
#endif /* ifdef B1500000 */
#ifdef B2000000
    bcarcb(2000000);
#endif /* ifdef B2000000 */
#ifdef B2500000
    bcarcb(2500000);
#endif /* ifdef B2500000 */
#ifdef B3000000
    bcarcb(3000000);
#endif /* ifdef B3000000 */
#ifdef B3500000
    bcarcb(3500000);
#endif /* ifdef B3500000 */
#ifdef B4000000
    bcarcb(4000000);
#endif /* ifdef B4000000 */
    return ( 0 );

  case XYRECV:
    puts("\
Specify parameters for inbound packets:");
    puts("\
End-Of-Packet (ASCII value), Packet-Length (1200 or less),");
    puts("\
Padding (amount, 94 or less), Pad-Character (ASCII value),");
    puts(
      "\
Start-Of-Packet (ASCII value), and Timeout (94 seconds or less),");
    puts("\
all specified as decimal numbers.");
    return ( 0 );

  case XYSEND:
    puts("\
Specify parameters for outbound packets:");
    puts("\
End-Of-Packet (ASCII value), Packet-Length (1200 or less),");
    puts("\
Padding (amount, 94 or less), Pad-Character (ASCII value),");
    puts(
      "\
Start-Of-Packet (ASCII value), and Timeout (94 seconds or less),");
    puts("\
all specified as decimal numbers.");
    return ( 0 );

#ifndef NOSERVER
  case XYSERV:
    puts("server timeout:");
    puts(
      "\
Server command wait timeout interval, how often the uCKermit server issues");
    puts(
      "\
a NAK while waiting for a command packet.  Specify 0 for no NAKs at all.");
    return ( 0 );
#endif /* ifndef NOSERVER */

  default:
    printf("Not available yet - %s\n", cmdbuf);
    return ( 0 );
  }
}
#endif /* ifndef NODOHLP */

/* D O H R M T -- Give help about REMOTE command */

#ifndef NODOHLP
int
dohrmt(xx)
int xx;
{
  int x;
  if (xx == -3)
  {
    return ( hmsga(hmhrmt) );
  }

  if (xx < 0)
  {
    return ( xx );
  }

  switch (xx)
  {
  case XZCWD:
    return ( hmsg(
               "\
Ask remote Kermit server to change its working directory."));

  case XZDEL:
    return ( hmsg(
               "\
Ask remote Kermit server to delete the named file(s)."));

  case XZDIR:
    return ( hmsg(
               "\
Ask remote Kermit server to provide directory listing of the named file(s)."));

  case XZHLP:
    return ( hmsg(
               "\
Ask remote Kermit server to tell you what services it provides."));

  case XZHOS:
    return ( hmsg(
               "\
Send a command to the remote system in its own command language\n\
through the remote Kermit server."));

  case XZSPA:
    return ( hmsg(
               "\
Ask the remote Kermit server to tell you about its disk space."));

  case XZTYP:
    return ( hmsg(
               "\
Ask the remote Kermit server to type the named file(s) on your screen."));

  case XZWHO:
    return ( hmsg(
               "\
Ask the remote Kermit server to list who's logged in, or to give information\n\
about the specified user."));

  default:
    if (( x = cmcfm()) < 0)
    {
      return ( x );
    }

    printf("Not working yet - %s\n", cmdbuf);
    return ( -2 );
  }
}
#endif /* ifndef NODOHLP */

/* D O L O G -- Do the log command */

#ifndef NOLOGS
#ifndef NOICP
int
dolog(x)
int x;
{
  int y;
  char *s;

  switch (x)
  {
  case LOGD:
#ifdef DEBUG
    y = cmofi("Name of debugging log file", "debug.log", &s);
#else  /* ifdef DEBUG */
    y = -2;
    s = "";
    printf("%s", "- Debug log not available\n");
#endif /* ifdef DEBUG */
    break;

  case LOGP:
    y = cmofi("Name of packet log file", "packet.log", &s);
    break;

  case LOGS:
    y = cmofi("Name of session log file", "session.log", &s);
    break;

  case LOGT:
#ifdef TLOG
    y = cmofi("Name of transaction log file", "transact.log", &s);
#else  /* ifdef TLOG */
    y = -2;
    s = "";
    printf("%s", "- Transaction log not available\n");
#endif /* ifdef TLOG */
    break;

  default:
    printf("\n?Unexpected log designator - %d\n", x);
    return ( -2 );
  }
  if (y < 0)
  {
    return ( y );
  }

  strcpy(line, s);
  s = line;
  if (( y = cmcfm()) < 0)
  {
    return ( y );
  }

  switch (x)
  {
  case LOGD:
    return ( deblog = debopn(s));

  case LOGP:
    zclose(ZPFILE);
    y = zopeno(ZPFILE, s);
    if (y > 0)
    {
      strcpy(pktfil, s);
    }
    else
    {
      *pktfil = '\0';
    }

    return ( pktlog = y );

  case LOGS:
    zclose(ZSFILE);
    y = zopeno(ZSFILE, s);
    if (y > 0)
    {
      strcpy(sesfil, s);
    }
    else
    {
      *sesfil = '\0';
    }

    return ( seslog = y );

  case LOGT:
    zclose(ZTFILE);
    tralog = zopeno(ZTFILE, s);
    if (tralog > 0)
    {
      strcpy(trafil, s);
      tlog(F110, "Transaction Log:", versio, 0l);
      tlog(F100, ckxsys, "", 0);
      ztime(&s);
      tlog(F100, s, "", 0l);
    }
    else
    {
      *trafil = '\0';
    }

    return ( tralog );

  default:
    return ( -2 );
  }
}
#endif /* ifndef NOICP */
#endif /* ifndef NOLOGS */

/* D E B O P N -- Open a debugging file */

#ifndef NOLOGS
int
debopn(
#ifdef DEBUG
  s
#endif /* ifdef DEBUG */
  )
#ifdef DEBUG
char *s;
#endif /* ifdef DEBUG */
{
#ifdef DEBUG
  char *tp;

  zclose(ZDFILE);
  deblog = zopeno(ZDFILE, s);
  if (deblog > 0)
  {
    strcpy(debfil, s);
    debug(F110, "Debug Log ", versio, 0);
    debug(F100, ckxsys, "", 0);
    ztime(&tp);
    debug(F100, tp, "", 0);
  }
  else
  {
    *debfil = '\0';
  }

  return ( deblog );

#else  /* ifdef DEBUG */
  return ( 0 );

#endif /* ifdef DEBUG */
}
#endif /* ifndef NOLOGS */

/* S H O P A R -- Show Parameters */

#ifndef NOICP
int
shopar()
{
#ifndef NOCKUDIA
  int i;
  extern struct keytab mdmtab[];
  extern int nmdm;
#endif /* ifndef NOCKUDIA */

  printf("\r\nLine: %s, speed: %d, mode: ", ttname, speed);
  if (local)
  {
    printf("local");
  }
  else
  {
    printf("remote");
  }

#ifndef NOCKUDIA
  for (i = 0; i < nmdm; i++)
  {
    if (mdmtab[i].val == mdmtyp)
    {
      printf(", modem-dialer: %s", mdmtab[i].kwd);
      break;
    }
  }

#endif /* ifndef NOCKUDIA */
  printf("\nBits: %d", ( parity ) ? 7 : 8);
  printf(", parity: ");
  switch (parity)
  {
  case 'e':
    printf("even");
    break;

  case 'o':
    printf("odd");
    break;

  case 'm':
    printf("mark");
    break;

  case 's':
    printf("space");
    break;

  case 0:
    printf("none");
    break;

  default:
    printf("invalid - %d", parity);
    break;
  }
  printf(", duplex: ");
  if (duplex)
  {
    printf("half, ");
  }
  else
  {
    printf("full, ");
  }

  printf("flow: ");
  if (flow == 1)
  {
    printf("XON/XOFF");
  }
  else if (flow == 0)
  {
    printf("none");
  }
  else
  {
    printf("%d", flow);
  }

  printf(", handshake: ");
  if (turn)
  {
    printf("%d\n", turnch);
  }
  else
  {
    printf("none\n");
  }

  printf("Terminal emulation: %d bits", ( cmask == 0177 ) ? 7 : 8);

#ifdef KERMRC
  printf(", Initialization file: %s", KERMRC);
#endif /* ifdef KERMRC */

  printf("\n\nProtocol Parameters:   Send    Receive");
  if (timef || spsizf)
  {
    printf("    (* = override)");
  }

  printf("\n Timeout:      %11d%9d", rtimo, timint);
  if (timef)
  {
    printf("*");
  }
  else
  {
    printf(" ");
  }

  printf("       Server timeout:%4d\n", srvtim);
  printf(" Padding:      %11d%9d", npad, mypadn);
  printf("        Block Check: %6d\n", bctr);
  printf(" Pad Character:%11d%9d", padch, mypadc);
  printf("        Delay:       %6d\n", delay);
  printf(" Packet Start: %11d%9d", mystch, stchr);
  printf("        Max Retries: %6d\n", maxtry);
  printf(" Packet End:   %11d%9d", seol, eol);
  if (ebqflg)
  {
    printf("        8th-Bit Prefix: '%c'", ebq);
  }

  printf("\n Packet Length:%11d", spsiz);
  printf(spsizf ? "*" : " ");
  printf("%8d", urpsiz);
  printf(( urpsiz > 94 ) ? " (94)" : "     ");
  if (rptflg)
  {
    printf("   Repeat Prefix:  '%c'", rptq);
  }

  printf("\n Length Limit: %11d%9d\n", maxsps, maxrps);
  printf("\nFile parameters:               Attributes:       ");
#ifndef NOATTR
  if (atcapr)
  {
    printf("on");
  }
  else
  {
    printf("off");
  }
#else /* ifndef NOATTR */
  printf("unavailable");
#endif /* ifndef NOATTR */

  printf("\n File Names:    ");
  if (fncnv)
  {
    printf("%-12s", "converted");
  }
  else
  {
    printf("%-12s", "literal");
  }

#ifndef NOLOGS
#ifdef DEBUG
  printf("   Debugging Log:    ");
  if (deblog)
  {
    printf("%s", debfil);
  }
  else
  {
    printf("none");
  }
#endif /* ifdef DEBUG */
#endif /* ifndef NOLOGS */

  printf("\n Transfer Mode: ");
  if (binary)
  {
    printf("%-12s", "binary");
  }
  else
  {
    printf("%-12s", "text");
  }

#ifndef NOLOGS
  printf("   Packet Log:       ");
  if (pktlog)
  {
    printf("%s", pktfil);
  }
  else
  {
    printf("none");
  }
#endif /* ifndef NOLOGS */

  printf("\n Overwrite:     ");
  if (warn)
  {
    printf("%-12s", "off");
  }
  else
  {
    printf("%-12s", "on");
  }

#ifndef NOLOGS
  printf("   Session Log:      ");
  if (seslog)
  {
    printf("%s", sesfil);
  }
  else
  {
    printf("none");
  }
#endif /* ifndef NOLOGS */

  printf("\n Info Display:  ");
  if (quiet)
  {
    printf("%-12s", "off");
  }
  else
  {
    printf("%-12s", "on");
  }

#ifdef TLOG
#ifndef NOLOGS
  printf("   Transaction Log:  ");
  if (tralog)
  {
    printf("%s", trafil);
  }
  else
  {
    printf("none");
  }
#endif /* ifndef NOLOGS */
#endif /* ifdef TLOG */
  printf("\n\nFile Byte Size: %d", ( fmask == 0177 ) ? 7 : 8);
  printf(", Incomplete File Disposition: ");
  if (keep)
  {
    printf("keep");
  }
  else
  {
    printf("discard");
  }
printf("\r\n\n");
return ( 0 );
}
#endif /* ifndef NOICP */

/* D O S T A T -- Display file transfer statistics */

#ifndef NOICP
#ifndef NOSTATS
#ifndef NOLOGS
int
dostat()
{
  printf("\r\n");
  printf(" Files: %ld\n", filcnt);
  printf(" Total file characters  : %ld\n", tfc);
  printf(" Communication line in  : %ld\n", tlci);
  printf(" Communication line out : %ld\n", tlco);
  printf(" Elapsed time           : %d sec\n", tsecs);
  if (filcnt > 0)
  {
    if (tsecs > 0)
    {
      long lx;
      lx = ( tfc * 10l ) / tsecs;
      printf(" Effective data rate    : %ld\n", lx);
      if (speed > 0)
      {
        lx = ( lx * 100l ) / speed;
        printf(" Efficiency             : %ld %%\n", lx);
      }
    }

    printf(
      " Packet length          : %d (Send), %d (Receive)\n",
      spsiz,
      urpsiz);
    printf(" Block check type used  : %d\n", bctu);
    printf(" Compression            : ");
    if (rptflg)
    {
      printf("Yes [%c]\n", rptq);
    }
    else
    {
      printf("No\n");
    }

    printf(" 8th bit prefixing      : ");
    if (ebqflg)
    {
      printf("Yes [%c]\n", ebq);
    }
    else
    {
      printf("No\n\n");
    }
  }
  else
  {
    printf("\r\n");
  }

  return ( 0 );
}

/* F S T A T S -- Record file statistics in transaction log */

int
fstats()
{
  tfc += ffc;
  tlog(F100, " end of file", "", 0l);
  tlog(F101, "  file characters        ", "", ffc);
  tlog(F101, "  communication line in  ", "", flci);
  tlog(F101, "  communication line out ", "", flco);
  return ( 0 );
}

/* T S T A T S -- Record statistics in transaction log */

void
tstats()
{
  char *tp;
  /* int x; */

  ztime(&tp);                               /* Get time stamp */
  tlog(F110, "End of transaction", tp, 0l);   /* Record it */

  if (filcnt < 1)
  {
    return;   /* If no files, done. */
  }

  /*
   * If multiple files, record
   * character totals for all files
   */

  if (filcnt > 1)
  {
    tlog(F101, " files", "", filcnt);
    tlog(F101, " total file characters   ", "", tfc);
    tlog(F101, " communication line in   ", "", tlci);
    tlog(F101, " communication line out  ", "", tlco);
  }

  /*
   * Record timing info
   * for one or more files
   */

  tlog(F101, " elapsed time (seconds)  ", "", (long)tsecs);
  if (tsecs > 0)
  {
    long lx;
    lx = ( tfc / tsecs ) * 10;
    tlog(F101, " effective baud rate     ", "", lx);
    if (speed > 0)
    {
      lx = ( lx * 100L ) / speed;
      tlog(F101, " efficiency (percent)    ", "", lx);
    }
  }

  tlog(F100, "", "", 0L);   /* Leave a blank line */
}
#endif /* ifndef NOLOGS */
#endif /* ifndef NOSTATS */
#endif /* ifndef NOICP */

/* S D E B U -- Record spar results in debugging log */

#ifdef DEBUG
#ifndef NOLOGS
int
sdebu(len)
int len;
{
  debug(F111, "spar: data", rdatap, len);
  debug(F101, " spsiz ", "", spsiz);
  debug(F101, " timint", "", timint);
  debug(F101, " npad  ", "", npad);
  debug(F101, " padch ", "", padch);
  debug(F101, " seol  ", "", seol);
  debug(F101, " ctlq  ", "", ctlq);
  debug(F101, " ebq   ", "", ebq);
  debug(F101, " ebqflg", "", ebqflg);
  debug(F101, " bctr  ", "", bctr);
  debug(F101, " rptq  ", "", rptq);
  debug(F101, " rptflg", "", rptflg);
#ifndef NOATTR
  debug(F101, " atcapu", "", atcapu);
#endif /* ifndef NOATTR */
  debug(F101, " lpcapu", "", lpcapu);
  debug(F101, " swcapu", "", swcapu);
  debug(F101, " wslots", "", wslots);
  return ( 0 );
}

/* R D E B U -- Debugging display of rpar() values */

int
rdebu(len)
int len;
{
  debug(F111, "rpar: data", data + 1, len);   /*** was rdatap ***/
  debug(F101, " rpsiz ", "", xunchar(data[1]));
  debug(F101, " rtimo ", "", rtimo);
  debug(F101, " mypadn", "", mypadn);
  debug(F101, " mypadc", "", mypadc);
  debug(F101, " eol   ", "", eol);
  debug(F101, " ctlq  ", "", ctlq);
  debug(F101, " sq    ", "", sq);
  debug(F101, " ebq   ", "", ebq);
  debug(F101, " ebqflg", "", ebqflg);
  debug(F101, " bctr  ", "", bctr);
  debug(F101, " rptq  ", "", data[9]);
  debug(F101, " rptflg", "", rptflg);
  debug(F101, " capas ", "", capas);
  debug(F101, " bits  ", "", data[capas]);
#ifndef NOATTR
  debug(F101, " atcapu", "", atcapu);
#endif /* ifndef NOATTR */
  debug(F101, " lpcapu", "", lpcapu);
  debug(F101, " swcapu", "", swcapu);
  debug(F101, " wslots", "", wslots);
  debug(F101, " rpsiz(extended)", "", rpsiz);
  return ( 0 );
}
#endif /* ifndef NOLOGS */
#endif /* ifdef DEBUG */
