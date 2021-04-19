/*  C K U U S 3 --  "User Interface" for Unix Kermit, part 3  */

/*
 Author: Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
 Columbia University Center for Computing Activities.
 First released January 1985.
 Copyright (C) 1985, 1989, Trustees of Columbia University in the City of New
 York.	Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained.
*/

/*  SET and REMOTE commands; screen, debug, interrupt, and logging functions */

/* Includes */

#include "ckcdeb.h"
#include <stdio.h>
#include <ctype.h>
#include "ckcker.h"
#include "ckucmd.h"
#include "ckuusr.h"
#ifdef UXIII
#include <termio.h>
#endif

#ifdef datageneral
extern int con_reads_mt,	    /* Flag if console read asynch is active */
	   conint_ch,		    /* The character read by asynch read */
	   conint_avl;		    /* Flag that char available */
#endif

/* Variables */

extern int size, spsiz, rpsiz, urpsiz, npad, timint, srvtim, rtimo, speed,
  local, server, lpcapr, fmask, cmask, backgrd, atcapr,
  flow, displa, binary, fncnv, delay, parity, deblog, escape, xargc,
  turn, duplex, cxseen, czseen, nfils, ckxech, pktlog, seslog, tralog, stdouf,
  turnch, bctr, bctu, dfloc, mdmtyp, keep, maxtry,
  rptflg, ebqflg, warn, quiet, cnflg, timef, spsizf, mypadn;

extern long filcnt, tlci, tlco, ffc, tfc, fsize;

extern char *versio, *protv, *ckxv, *ckzv, *fnsv, *connv, *dftty, *cmdv;
extern char *cmarg, *cmarg2, **xargv, **cmlist;
extern CHAR stchr, mystch, sstate, padch, mypadc, eol, seol, ctlq;
extern CHAR filnam[], ttname[];
char *strcpy();

/* Declarations from cmd package */

extern char cmdbuf[];			/* Command buffer */

/* From main ckuser module... */

extern char line[100], *lp;		/* Character buffer for anything */
extern char debfil[50],			/* Debugging log file name */
 pktfil[50],				/* Packet log file name */
 sesfil[50],				/* Session log file name */
 trafil[50];				/* Transaction log file name */

extern int tlevel;			/* Take Command file level */
extern FILE *tfile[];			/* Array of take command fd's */


/* Keyword tables for SET commands */


/* Block checks */

struct keytab blktab[] = {
    "1", 1, 0,
    "2", 2, 0,
    "3", 3, 0
};

/* Duplex keyword table */

struct keytab dpxtab[] = {
    "full",	 0, 0,
    "half",	 1, 0
};

struct keytab filtab[] = {
    "display", XYFILD, 0,
    "names",   XYFILN, 0,
    "type",    XYFILT, 0,
    "warning", XYFILW, 0
};
int nfilp = (sizeof(filtab) / sizeof(struct keytab));

/* Send/Receive Parameters */

struct keytab srtab[] = {
    "end-of-packet", XYEOL, 0,
    "packet-length", XYLEN, 0,
    "pad-character", XYPADC, 0,
    "padding", XYNPAD, 0,
    "start-of-packet", XYMARK, 0,
    "timeout", XYTIMO, 0
};
int nsrtab = (sizeof(srtab) / sizeof(struct keytab));

/* Flow Control */

struct keytab flotab[] = {
    "none",	0, 0,
    "xon/xoff", 1, 0
};
int nflo = (sizeof(flotab) / sizeof(struct keytab));

/*  Handshake characters  */

struct keytab hshtab[] = {
    "bell", 007, 0,
    "cr",   015, 0,
    "esc",  033, 0,
    "lf",   012, 0,
    "none", 999, 0,			/* (can't use negative numbers) */
    "xoff", 023, 0,
    "xon",  021, 0
};
int nhsh = (sizeof(hshtab) / sizeof(struct keytab));

struct keytab fntab[] = {		/* File naming */
    "converted", 1, 0,
    "literal",	 0, 0
};

struct keytab fttab[] = {		/* File types */
    "binary",	 1, 0,
    "text",	 0, 0
};

extern struct keytab mdmtab[] ;		/* Modem types (in module ckudia.c) */
extern int nmdm;

/* Parity keyword table */

struct keytab partab[] = {
    "even",    'e', 0,
    "mark",    'm', 0,
    "none",	 0, 0,
    "odd",     'o', 0,
    "space",   's', 0
};
int npar = (sizeof(partab) / sizeof(struct keytab));


/* On/Off table */

struct keytab onoff[] = {
    "off",	 0, 0,
    "on",	 1, 0
};

/* Incomplete File Disposition table */

struct keytab ifdtab[] = {
    "discard",	 0, 0,
    "keep",	 1, 0
};

/* Terminal parameters table */

struct keytab trmtab[] = {
    "bytesize",	 0, 0
};

/* Server parameters table */
struct keytab srvtab[] = {
    "timeout",	 0, 0
};

/*  D O P R M  --  Set a parameter.  */
/*
 Returns:
  -2: illegal input
  -1: reparse needed
   0: success
*/
doprm(xx) int xx; {
    int x, y, z;
    char *s;

switch (xx) {


case XYEOL:	/* These have all been moved to set send/receive... */
case XYLEN:	/* Let the user know what to do. */
case XYMARK:
case XYNPAD:
case XYPADC:
case XYTIMO:
    printf("...Use 'set send' or 'set receive' instead.\n");
    printf("Type 'help set send' or 'help set receive' for more info.\n");
    return(0);

case XYATTR:				/* File Attribute packets */
    return(seton(&atcapr));

case XYIFD:				/* Incomplete file disposition */
    if ((y = cmkey(ifdtab,2,"","discard")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    keep = y;
    return(0);

case XYLINE:
    if ((x = cmtxt("Device name",dftty,&s)) < 0) return(x);
    ttclos();				/* close old line, if any was open */

    x = strcmp(s,dftty) ? -1 : dfloc;	/* Maybe let ttopen figure it out... */
    if (ttopen(s,&x,mdmtyp) < 0 ) {	/* Can we open the new line? */
	perror("Sorry, can't open line");
	return(-2);			/* If not, give bad return */
    }
    if (x > -1) local = x;		/* Set local/remote status. */
    strcpy(ttname,s);			/* OK, copy name into real place. */
    if (!local) speed = -1;		/* If remote, say speed unknown. */
    debug(F111,"set line ",ttname,local);
    return(0);

case XYCHKT:
    if ((y = cmkey(blktab,3,"","1")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    bctr = y;
    return(0);

case XYDEBU:
    return(seton(&deblog));

case XYDELA:
    y = cmnum("Number of seconds before starting to send","5",10,&x);
    debug(F101,"XYDELA: y","",y);
    return(setnum(&delay,x,y,94));

case XYDUPL:
    if ((y = cmkey(dpxtab,2,"","full")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    duplex = y;
    return(0);

case XYESC:
    y = cmnum("Decimal ASCII code for escape character","",10,&x);
    return(setcc(&escape,x,y));

case XYFILE:
    if ((y = cmkey(filtab,nfilp,"File parameter","")) < 0) return(y);
    switch (y) {
	/* int z; */			/* Use exterior version of z, */
					/* to avoid OS-9 C Compiler bug. */
	case XYFILD:			/* Display */
	    y = seton(&z);
	    if (y < 0) return(y);
	    quiet = !z;
	    return(0);

	case XYFILN:			/* Names */
	    if ((x = cmkey(fntab,2,"how to handle filenames","converted")) < 0)
		return(x);
	    if ((z = cmcfm()) < 0) return(z);
	    fncnv = x;
	    return(0);

	case XYFILT:			/* Type */
	    if ((x = cmkey(fttab,2,"type of file","text")) < 0)
		return(x);
	    if ((y = cmnum("file byte size (7 or 8)","8",10,&z)) < 0)
		return(y);
	    if (z != 7 && z != 8) {
		printf("\n?The choices are 7 and 8\n");
		return(-2);
	    }
	    if ((y = cmcfm()) < 0) return(y);
	    binary = x;
	    if (z == 7) fmask = 0177;
	      else if (z == 8) fmask = 0377;
	    return(0);

	case XYFILW:			/* Warning/Write-Protect */
	    return(seton(&warn));

	default:
	    printf("?unexpected file parameter\n");
	    return(-2);
    }

case XYFLOW:				/* Flow control */
    if ((y = cmkey(flotab,nflo,"","xon/xoff")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    flow = y;
    return(0);

case XYHAND:				/* Handshake */
    if ((y = cmkey(hshtab,nhsh,"","none")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    turn = (y > 0127) ? 0 : 1 ;
    turnch = y;
    return(0);

case XYMODM:
    if ((x=cmkey(mdmtab,nmdm,"type of modem, direct means none","direct")) < 0)
	return(x);
    if ((z = cmcfm()) < 0) return(z);
    mdmtyp = x;
    return(0);
	
case XYPARI:				/* Parity */
    if ((y = cmkey(partab,npar,"","none")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);

/* If parity not none, then we also want 8th-bit prefixing */

    if (parity = y) ebqflg = 1; else ebqflg = 0;
    return(0);

case XYPROM:
    if ((x = cmtxt("Program's command prompt","C-Kermit>",&s)) < 0) return(x);
    if (*s == '\42') {			/* Quoted string? */
	x = strlen(s) - 1;		/* Yes, strip quotes. */
	if (*(s+x) == '\42')		/* This allows leading or trailing */
	    *(s+x) = '\0';		/* blanks. */
	    s++;
	}
    cmsetp(s);
    return(0);

case XYRETR:				/* Per-packet retry limit */
    y = cmnum("Maximum retries per packet","10",10,&x);
    return(setnum(&maxtry,x,y,94));

case XYSERV:				/* Server timeout */
    if ((y = cmkey(srvtab,1,"","timeout")) < 0) return(y);
    switch (y) {
      char tmp[20];
      case XYSERT:
	sprintf(tmp,"%d",DSRVTIM);
	if ((y = cmnum("interval for server NAKs, 0 = none",tmp,10,&x)) < 0)
	  return(y);
	if (x < 0) {
	    printf("\n?Specify a positive number, or 0 for no server NAKs\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	srvtim = x;			/* Set the server timeout variable */
	return(y);
      default:
	return(-2);
    }

case XYTERM:				/* Terminal parameters */
    if ((y = cmkey(trmtab,1,"","bytesize")) < 0) return(y);
    switch (y) {
      case 0:
	if ((y = cmnum("bytesize for terminal connection","8",10,&x)) < 0)
	  return(y);
	if (x != 7 && x != 8) {
	    printf("\n?The choices are 7 and 8\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	if (x == 7) cmask = 0177;
	else if (x == 8) cmask = 0377;
	return(y);
      default:	     /* Add more cases when we think of more parameters */
	return(-2);
    }

/* SET SEND/RECEIVE... */

case XYRECV:
case XYSEND:
    if (xx == XYRECV)
	strcpy(line,"Parameter for inbound packets");
    else
	strcpy(line,"Parameter for outbound packets");

    if ((y = cmkey(srtab,nsrtab,line,"")) < 0) return(y);
    switch (y) {

case XYEOL:
    y = cmnum("Decimal ASCII code for packet terminator","13",10,&x);
    if ((y = setcc(&z,x,y)) < 0) return(y);
    if (xx == XYRECV) eol = z; else seol = z;
    return(y);

case XYLEN:
    y = cmnum("Maximum number of characters in a packet","90",10,&x);
    if (xx == XYRECV) {			/* Receive... */
	if ((y = setnum(&z,x,y,MAXRP)) < 0)
	  return(y);
	urpsiz = z;
	rpsiz =	 (z > 94) ? 94 : z;
    } else {				/* Send... */
	if ((y = setnum(&z,x,y,MAXSP)) < 0)
	  return(y);
	spsiz = z;			/*   Set it and flag that it was set */
	spsizf = 1;			/*   to allow overriding Send-Init. */
    }
    if (z > 94 && !backgrd) printf("Extended-length packets requested\n");
    return(y);

case XYMARK:
    y = cmnum("Decimal ASCII code for packet-start character","1",10,&x);
    if ((y = setcc(&z,x,y)) < 0) return(y);
    if (xx == XYRECV) stchr = z; else mystch = z;
    return(y);

case XYNPAD:				/* Padding */
    y = cmnum("How many padding characters for inbound packets","0",10,&x);
    if ((y = setnum(&z,x,y,94)) < 0) return(y);
    if (xx == XYRECV) mypadn = z; else npad = z;
    return(y);

case XYPADC:				/* Pad character */
    y = cmnum("Decimal ASCII code for inbound pad character","0",10,&x);
    if ((y = setcc(&z,x,y)) < 0) return(y);
    if (xx == XYRECV) mypadc = z; else padch = z;
    return(y);

case XYTIMO:
    y = cmnum("Interpacket timeout interval","5",10,&x);
    if ((y = setnum(&z,x,y,94)) < 0) return(y);
    if (xx == XYRECV) {
	timef = 1;
	timint = z;
    } else rtimo = z;
    return(y);

    }

case XYSPEE:
    if (!local) {
	printf("\nSpeed setting can only be done on an external line\n");
	printf("You must 'set line' before issuing this command\n");
	return(0);
    }	
    lp = line;
    sprintf(lp,"Baud rate for %s",ttname);
    if ((y = cmnum(line,"",10,&x)) < 0) return(y);
    if ((y = cmcfm()) < 0) return(y);
    y = chkspd(x);
    if (y < 0)
	printf("?Unsupported line speed - %d\n",x);
    else {
	speed = y;
	if (!backgrd) printf("%s, %d baud\n",ttname,speed);
    }
    return(0);

default:
    if ((x = cmcfm()) < 0) return(x);
    printf("Not working yet - %s\n",cmdbuf);
    return(0);
    }
}

/*  C H K S P D	 --  Check if argument is a valid baud rate  */

chkspd(x) int x; {
    switch (x) {
#ifndef AMIGA
	case 0:
#endif
#ifndef datageneral
	case 110:
	case 150:
	case 300:
	case 600:
	case 1200:
#ifndef OS2
	case 1800:
#endif
	case 2400:
	case 4800:
	case 9600:
	case 19200:
#ifdef AMIGA
	case 38400:
	case 57600:
#endif
#endif

#ifdef datageneral
	case 50:
	case 75:
	case 134:
	case 3600:
	case 7200:
	case 19200:
	case 38400:
#endif
	    return(x);
	default:
#ifdef AMIGA
	    if (ttsspd(x) > 0)
	    {
		printf("Warning: non-standard baud rate - %d\n", x);
		return(x);
	    }
#endif
#ifdef VMS
	    if (ttsspd(x) > 0)
	    {
		printf("Warning: non-standard baud rate - %d\n", x);
		return(x);
	    }
#endif
	    return(-1);
      }
}

/*  S E T O N  --  Parse on/off (default on), set parameter to result  */

seton(prm) int *prm; {
    int x, y;
    if ((y = cmkey(onoff,2,"","on")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    *prm = y;
    return(0);
}

/*  S E T N U M	 --  Set parameter to result of cmnum() parse.	*/
/*
 Call with x - number from cnum parse, y - return code from cmnum
*/
setnum(prm,x,y,max) int x, y, *prm, max; {
    debug(F101,"setnum","",y);
    if (y < 0) return(y);
    if (x > max) {
	printf("\n?Sorry, %d is the maximum\n",max);
	return(-2);
    }
    if ((y = cmcfm()) < 0) return(y);
    *prm = x;
    return(0);
}

/*  S E T C C  --  Set parameter to an ASCII control character value.  */

setcc(prm,x,y) int x, y, *prm; {
    if (y < 0) return(y);
    if ((x > 037) && (x != 0177)) {
	printf("\n?Not in ASCII control range - %d\n",x);
	return(-2);
    }
    if ((y = cmcfm()) < 0) return(y);
    *prm = x;
    return(0);
}

/*  D O R M T  --  Do a remote command	*/

dormt(xx) int xx; {
    int x;
    char *s, sbuf[50], *s2;

    if (xx < 0) return(xx);
    switch (xx) {

case XZCWD:				/* CWD */
    if ((x = cmtxt("Remote directory name","",&s)) < 0) return(x);
    debug(F111,"XZCWD: ",s,x);
    *sbuf = NUL;
    s2 = sbuf;
    if (*s != NUL) {			/* If directory name given, */
					/* get password on separate line. */
	if (tlevel > -1) {		/* From take file... */

	    if (fgets(sbuf,50,tfile[tlevel]) == NULL)
		fatal("take file ends prematurely in 'remote cwd'");
	    debug(F110," pswd from take file",s2,0);
	    for (x = strlen(sbuf);
		 x > 0 && (sbuf[x-1] == NL || sbuf[x-1] == CR);
		 x--)
		sbuf[x-1] = '\0';

	} else {			/* From terminal... */

	    printf(" Password: ");		/* get a password */
#ifdef OS2
	    while (((x = isatty(0) ? coninc(0) :
		     getchar()) != NL) && (x != CR)) {	   /* with no echo */
#else
	    while (((x = getchar()) != NL) && (x != CR)) { /* with no echo */
#endif
		if ((x &= 0177) == '?') {
		    printf("? Password of remote directory\n Password: ");
		    s2 = sbuf;
		    *sbuf = NUL;
		}
		else if (x == ESC)	/* Mini command line editor... */
		    putchar(BEL);
		else if (x == BS || x == 0177)
		    s2--;
		else if (x == 025) {	/* Ctrl-U */
		    s2 = sbuf;
		    *sbuf = NUL;
		}
		else
		    *s2++ = x;
	    }
	    *s2 = NUL;
	    putchar('\n');
	}
	s2 = sbuf;
    } else s2 = "";
    debug(F110," password",s2,0);
    sstate = setgen('C',s,s2,"");
    return(0);

case XZDEL:				/* Delete */
    if ((x = cmtxt("Name of remote file(s) to delete","",&s)) < 0) return(x);
    return(sstate = rfilop(s,'E'));

case XZDIR:				/* Directory */
    if ((x = cmtxt("Remote directory or file specification","",&s)) < 0)
	return(x);
    return(sstate = setgen('D',s,"",""));

case XZHLP:				/* Help */
    if ((x = cmcfm()) < 0) return(x);
    sstate = setgen('H',"","","");
    return(0);

case XZHOS:				/* Host */
    if ((x = cmtxt("Command for remote system","",&cmarg)) < 0) return(x);
    return(sstate = 'c');

case XZPRI:				/* Print */
    if ((x = cmtxt("Remote file(s) to print on remote printer","",&s)) < 0)
	return(x);
    return(sstate = rfilop(s,'S'));

case XZSPA:				/* Space */
    if ((x = cmtxt("Confirm, or remote directory name","",&s)) < 0) return(x);
    return(sstate = setgen('U',s,"",""));

case XZTYP:				/* Type */
    if ((x = cmtxt("Remote file specification","",&s)) < 0) return(x);
    return(sstate = rfilop(s,'T'));

case XZWHO:
    if ((x = cmtxt("Remote user name, or carriage return","",&s)) < 0)
	return(x);
    return(sstate = setgen('W',s,"",""));

default:
    if ((x = cmcfm()) < 0) return(x);
    printf("not working yet - %s\n",cmdbuf);
    return(-2);
    }
}



/*  R F I L O P	 --  Remote File Operation  */

rfilop(s,t) char *s, t; {
    if (*s == NUL) {
	printf("?File specification required\n");
	return(-2);
    }
    debug(F111,"rfilop",s,t);
    return(setgen(t,s,"",""));
}

/*  S C R E E N	 --  Screen display function  */

/*  screen(f,c,n,s)
      f - argument descriptor
      c - a character or small integer
      n - a long integer
      s - a string.
 Fill in this routine with the appropriate display update for the system.
 This version is for a dumb tty.
*/
screen(f,c,n,s) int f; long n; char c; char *s; {
    static int p = 0;			/* Screen position */
    int len;				/* Length of string */
    char buf[80];			/* Output buffer */
    len = strlen(s);			/* Length of string */
    if ((f != SCR_WM) && (f != SCR_EM)) /* Always update warning & errors */
      if (!displa || quiet) return;	/* No update if display flag off */

    switch (f) {

case SCR_FN:				/* filename */
    conoll(""); conol(s); conoc(SP); p = len + 1; return;

case SCR_AN:				/* as-name */
    if (p + len > 75) { conoll(""); p = 0; }
    conol("=> "); conol(s); if ((p += (len + 3)) > 78) { conoll(""); p = 0; }
    return;

case SCR_FS:				/* file-size */
    sprintf(buf,", Size: %ld",n);  conoll(buf);	 p = 0; return;

case SCR_XD:				/* x-packet data */
    conoll(""); conoll(s); p = 0; return;

case SCR_ST:				/* File status */
    switch (c) {
	case ST_OK:			/*  Transferred OK */
	    if ((p += 5) > 78) { conoll(""); p = 0; }
	    conoll(" [OK]"); p += 5; return;

	case ST_DISC:			/*  Discarded */
	    if ((p += 12) > 78) { conoll(""); p = 0; }
	    conoll(" [discarded]"); p += 12; return;

	case ST_INT:			/*  Interrupted */
	    if ((p += 14) > 78) { conoll(""); p = 0; }
	    conoll(" [interrupted]"); p += 14; return;

	case ST_SKIP:			/*  Skipped */
	    conoll("");
	    conol("Skipping "); conoll(s); p = 0;
	    return;

	case ST_ERR:
	    conoll("");
	    conol("Error "); conoll(s); p = 0;
	    return;

	default:
	    conoll("*** screen() called with bad status ***"); p = 0; return;
    }

case SCR_PN:				/* Packet number */
    sprintf(buf,"%s: %ld",s,n); conol(buf); p += strlen(buf); return;

case SCR_PT:				/* Packet type or pseudotype */
    if (c == 'Y') return;		/* Don't bother with ACKs */
    if (c == 'D') {			/* Only show every 4th data packet */
	if (n % 4) return;
	c = '.';
    }
#ifndef AMIGA
    if (p++ > 78) {			/* If near right margin, */
	conoll("");			/* Start new line */
	p = 0;				/* and reset counter. */
    }
#endif
    conoc(c);				/* Display the character. */
#ifdef AMIGA
    if (c == 'G') conoll("");		/* new line after G packets */
#endif
    return;

case SCR_TC:				/* transaction complete */
    conoc(BEL); return;

case SCR_EM:				/* Error message */
    conoll(""); conoc('?'); conoll(s); p = 0; return;		/* +1	*/

case SCR_WM:				/* Warning message */
    conoll(""); conoll(s); p = 0; return;

case SCR_TU:				/* Undelimited text */
    if ((p += len) > 78) { conoll(""); p = len; }
    conol(s); return;

case SCR_TN:				/* Text delimited at beginning */
    conoll(""); conol(s); p = len; return;

case SCR_TZ:				/* Text delimited at end */
    if ((p += len) > 78) { conoll(""); p = len; }
    conoll(s); return;

case SCR_QE:				/* Quantity equals */
    sprintf(buf,"%s: %ld",s,n);
    conoll(buf); p = 0; return;

default:
    conoll("*** screen() called with bad object ***"); p = 0; return;
    }
}

/*  I N T M S G	 --  Issue message about terminal interrupts  */

intmsg(n) long n; {
    extern char *chstr();
    char buf[80];

    if ((!displa) || (quiet)) return;
#ifdef UXIII
    (void) conchk();	/* clear out pending escape-signals in ckxbsd.c */
#endif
    if (n == 1) {
#ifdef UXIII

#ifndef aegis
#ifndef datageneral
				/* we need to signal before kb input */
	sprintf(buf,"Type escape (%s) followed by:",chstr(escape));
	screen(SCR_TN,0,0l,buf);
#endif
#endif
#endif
 screen(SCR_TN,0,0l,"CTRL-F to cancel file,  CTRL-R to resend current packet");
 screen(SCR_TN,0,0l,"CTRL-B to cancel batch, CTRL-A for status report: ");
    }
    else screen(SCR_TU,0,0l," ");
}

/*  C H K I N T	 --  Check for console interrupts  */

/*** should rework not to destroy typeahead ***/

chkint() {
    int ch, cn;

    if ((!local) || (quiet)) return(0);	/* Only do this if local & not quiet */
#ifdef datageneral
    cn = (con_reads_mt) ? 1 : conchk();	/* Any input waiting? */
#else
    cn = conchk();			/* Any input waiting? */
#endif
    debug(F101,"conchk","",cn);

    while (cn > 0) {			/* Yes, read it. */
	cn--;
			/* give read 5 seconds for interrupt character */
#ifdef datageneral
	/* We must be careful to just print out one result for each character
	 * read.  The flag, conint_avl, controls duplication of characters.
	 * Only one character is handled at a time, which is a reasonable
	 * limit.  More complicated schemes could handle a buffer.
	 */
	if (con_reads_mt) {
	    if ((ch = conint_ch) <= 0) return(0);   /* I/O error, or no data */
	    else if (conint_avl == 0) return(0);    /* Char already read */
	    else conint_avl = 0;		    /* Flag character as read */
	}
	else { if ((ch = coninc(5)) < 0) return(0);  }
#else
	if ((ch = coninc(5)) < 0) return(0);
#endif
	switch (ch & 0177) {
	    case 0001:			/* CTRL-A */
		screen(SCR_TN,0,0l,"^A	Status report:");
		screen(SCR_TN,0,0l," file type: ");
		if (binary) screen(SCR_TZ,0,0l,"binary");
		    else    screen(SCR_TZ,0,0l,"text");
		screen(SCR_QE,0,(long)filcnt," file number");
		screen(SCR_QE,0,(long)ffc,   " characters ");
		screen(SCR_QE,0,(long)bctu,  " block check");
		screen(SCR_QE,0,(long)rptflg," compression");
		screen(SCR_QE,0,(long)ebqflg," 8th-bit prefixing");
		continue;
	    case 0002:			/* CTRL-B */
		screen(SCR_TN,0,0l,"^B - Cancelling Batch ");
		czseen = 1;
		continue;
	    case 0006:			/* CTRL-F */
		screen(SCR_TN,0,0l,"^F - Cancelling File ");
		cxseen = 1;
		continue;
	    case 0022:			/* CTRL-R */
		screen(SCR_TN,0,0l,"^R - Resending ");
		resend();
		return(1);
	    default:			/* Anything else, just ignore */
		screen(SCR_TU,0,0l," [Ignored] ");
		continue;
	}
    }
    return(0);
}

/*  D E B U G  --  Enter a record in the debugging log	*/

/*
 Call with a format, two strings, and a number:
   f  - Format, a bit string in range 0-7.
	If bit x is on, then argument number x is printed.
   s1 - String, argument number 1.  If selected, printed as is.
   s2 - String, argument number 2.  If selected, printed in brackets.
   n  - Int, argument 3.  If selected, printed preceded by equals sign.

   f=0 is special: print s1,s2, and interpret n as a char.
*/
#ifdef DEBUG
#define DBUFL 1200
debug(f,s1,s2,n) int f, n; char *s1, *s2; {
    static char s[DBUFL];
    char *sp = s;

    if (!deblog) return;		/* If no debug log, don't */
    switch (f) {
	case F000:			/* 0, print both strings, */
	    if (strlen(s1) + strlen(s2) + 3 > DBUFL)
	      sprintf(sp,"DEBUG string too long\n");
	    else sprintf(sp,"%s%s%c\n",s1,s2,n); /* interpret n as a char */
	    zsout(ZDFILE,s);
	    break;
	case F001:			/* 1, "=n" */
	    sprintf(sp,"=%d\n",n);
	    zsout(ZDFILE,s);
	    break;
	case F010:			/* 2, "[s2]" */
	    if (strlen(s2) + 4 > DBUFL)
	      sprintf(sp,"DEBUG string too long\n");
	    else sprintf(sp,"[%s]\n",s2);
	    zsout(ZDFILE,"");
	    break;
	case F011:			/* 3, "[s2]=n" */
	    if (strlen(s2) + 15 > DBUFL)
	      sprintf(sp,"DEBUG string too long\n");
	    else sprintf(sp,"[%s]=%d\n",s2,n);
	    zsout(ZDFILE,s);
	    break;
	case F100:			/* 4, "s1" */
	    zsoutl(ZDFILE,s1);
	    break;
	case F101:			/* 5, "s1=n" */
	    if (strlen(s1) + 15 > DBUFL)
	      sprintf(sp,"DEBUG string too long\n");
	    else sprintf(sp,"%s=%d\n",s1,n);
	    zsout(ZDFILE,s);
	    break;
	case F110:			/* 6, "s1[s2]" */
	    if (strlen(s1) + strlen(s2) + 4 > DBUFL)
	      sprintf(sp,"DEBUG string too long\n");
	    else sprintf(sp,"%s[%s]\n",s1,s2);
	    zsout(ZDFILE,s);
	    break;
	case F111:			/* 7, "s1[s2]=n" */
	    if (strlen(s1) + strlen(s2) + 15 > DBUFL)
	      sprintf(sp,"DEBUG string too long\n");
	    else sprintf(sp,"%s[%s]=%d\n",s1,s2,n);
	    zsout(ZDFILE,s);
	    break;
	default:
	    sprintf(sp,"\n?Invalid format for debug() - %d\n",n);
	    zsout(ZDFILE,s);
    }
}
#endif

#ifdef TLOG
#define TBUFL 300
/*  T L O G  --	 Log a record in the transaction file  */
/*
 Call with a format and 3 arguments: two strings and a number:
   f  - Format, a bit string in range 0-7, bit x is on, arg #x is printed.
   s1,s2 - String arguments 1 and 2.
   n  - Int, argument 3.
*/
tlog(f,s1,s2,n) int f; long n; char *s1, *s2; {
    static char s[TBUFL];
    char *sp = s; int x;

    if (!tralog) return;		/* If no transaction log, don't */
    switch (f) {
	case F000:			/* 0 (special) "s1 n s2"  */
	    if (strlen(s1) + strlen(s2) + 15 > TBUFL)
	      sprintf(sp,"?T-Log string too long\n");
	    else sprintf(sp,"%s %ld %s\n",s1,n,s2);
	    zsout(ZTFILE,s);
	    break;
	case F001:			/* 1, " n" */
	    sprintf(sp," %ld\n",n);
	    zsout(ZTFILE,s);
	    break;
	case F010:			/* 2, "[s2]" */
	    x = strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    if (x + 6 > TBUFL)
	      sprintf(sp,"?T-Log string too long\n");
	    else sprintf(sp,"[%s]\n",s2);
	    zsout(ZTFILE,"");
	    break;
	case F011:			/* 3, "[s2] n" */
	    x = strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    if (x + 6 > TBUFL)
	      sprintf(sp,"?T-Log string too long\n");
	    else sprintf(sp,"[%s] %ld\n",s2,n);
	    zsout(ZTFILE,s);
	    break;
	case F100:			/* 4, "s1" */
	    zsoutl(ZTFILE,s1);
	    break;
	case F101:			/* 5, "s1: n" */
	    if (strlen(s1) + 15 > TBUFL)
	      sprintf(sp,"?T-Log string too long\n");
	    else sprintf(sp,"%s: %ld\n",s1,n);
	    zsout(ZTFILE,s);
	    break;
	case F110:			/* 6, "s1 s2" */
	    x = strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    if (strlen(s1) + x + 4 > TBUFL)
	      sprintf(sp,"?T-Log string too long\n");
	    else sprintf(sp,"%s %s\n",s1,s2);
	    zsout(ZTFILE,s);
	    break;
	case F111:			/* 7, "s1 s2: n" */
	    x = strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    if (strlen(s1) + x + 15 > TBUFL)
	      sprintf(sp,"?T-Log string too long\n");
	    else sprintf(sp,"%s %s: %ld\n",s1,s2,n);
	    zsout(ZTFILE,s);
	    break;
	default:
	    sprintf(sp,"\n?Invalid format for tlog() - %ld\n",n);
	    zsout(ZTFILE,s);
    }
}
#endif
