char *connv = "Connect Command for Unix, 4F(018) 19 Jun 89";

/*  C K U C O N  --  Dumb terminal connection to remote system, for Unix  */
/*
 Author: Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
 Columbia University Center for Computing Activities.
 First released January 1985.
 Copyright (C) 1985, 1989, Trustees of Columbia University in the City of New 
 York.  Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained. 
*/

#include <stdio.h>
#include <ctype.h>			/* Character types */
#include "ckcdeb.h"
#include "ckcker.h"
#include <signal.h>

#ifndef ZILOG
#include <setjmp.h>			/* Longjumps */
#else
#include <setret.h>
#endif

#ifndef SIGUSR1
#define SIGUSR1 16
#endif

extern int local, speed, escape, duplex, parity, flow, seslog, mdmtyp;
extern int errno, cmask, fmask;
extern char ttname[], sesfil[];
extern CHAR dopar();
static int quitnow = 0;
static int dohangup = 0;

int i, active;				/* Variables global to this module */
int io_retry = 0;
char *chstr();
char temp[50];

#define LBUFL 200			/* Line buffer */
char lbuf[LBUFL];

/* Connect state parent/child communication signal handlers */

static jmp_buf env_con;			/* Envir ptr for connect errors */

SIGTYP 
conn_int() {				/* Modem read failure handler, */
    longjmp(env_con,1);			/* notifies parent process to stop */
}

/*  C O N E C T  --  Perform terminal connection  */

conect() {
    int pid, 			/* process id of child (modem reader) */
	parent_id,		/* process id of parent (keyboard reader) */
	n;
    int c;			/* c is a character, but must be signed 
				   integer to pass thru -1, which is the
				   modem disconnection signal, and is
				   different from the character 0377 */
    char errmsg[50], *erp;

	if (!local) {
	    printf("Sorry, you must 'set line' first\n");
	    return(-2);
	}
	if (speed < 0) {
	    printf("Sorry, you must 'set speed' first\n");
	    return(-2);
        }
	if ((escape < 0) || (escape > 0177)) {
	    printf("Your escape character is not ASCII - %d\n",escape);
	    return(-2);
	}
	if (ttopen(ttname,&local,mdmtyp) < 0) {
	    erp = errmsg;
	    sprintf(erp,"Sorry, can't open %s",ttname);
	    perror(errmsg);
	    return(-2);
    	}
        dohangup = 0;
    	printf("Connecting thru %s, speed %d.\r\n",ttname,speed);
	printf("The escape character is %s (%d).\r\n",chstr(escape),escape);
	printf("Type the escape character followed by C to get back,\r\n");
	printf("or followed by ? to see other options.\r\n");
	if (seslog) printf("(Session logged to %s.)\r\n",sesfil);

/* Condition console terminal and communication line */	    

    	if (conbin(escape) < 0) {
	    printf("Sorry, can't condition console terminal\n");
	    return(-2);
    	}
	if (ttvt(speed,flow) < 0) {
	    conres();
	    printf("Sorry, Can't condition communication line\n");
	    return(-2);
    	}

/* cont'd... */

/* ...connect, cont'd */

	parent_id = getpid();		/* Get parent id for signalling */
        signal(SIGUSR1,SIG_IGN);	/* Don't kill parent */
	pid = fork();			/* All ok, make a fork */
	if (pid == -1) {
	    conres();			/* Reset the console. */
	    perror("Can't create keyboard fork");
	    printf("[Back at Local System]\n");
	    return(0);
	}
        io_retry = 0;
	if (pid) {			
	  active = 1;			/* This fork reads, sends keystrokes */
	  if (!setjmp(env_con)) {	/* comm error in child process */
	    signal(SIGUSR1,conn_int);	/* routine for child process exit */
	    while (active) {
		c = coninc(0) & cmask;	/* Get character from keyboard */
		if ((c & 0177) == escape) { /* Look for escape char */
		    c = coninc(0) & 0177;   /* Got esc, get its arg */
		    doesc(c);		    /* And process it */
		} else {		/* Ordinary character */
		    if (ttoc(dopar(c)) > -1) {
		    	if (duplex) {	    /* Half duplex? */
			    conoc(c);	    /* Yes, also echo it. */
			    if (seslog)     /* And maybe log it. */
			    	if (zchout(ZSFILE,c) < 0) seslog = 0;
			}
    	    	    } else {
			perror("\r\nCan't send character");
			active = 0;
		    }
		}
	      }
    	    }				/* Come here on death of child */
	    kill(pid,9);		/* Done, kill inferior fork. */
	    wait((int *)0);		/* Wait till gone. */
	    conres();			/* Reset the console. */
	    if (quitnow) doexit(GOOD_EXIT);
	    if (dohangup) tthang();
	    printf("\r[Back at Local System]\n");
	    return(0);

	} else {			/* Inferior reads, prints port input */

	    sleep(1);			/* Wait for parent's handler setup */
	    while (1) {			/* Fresh read, wait for a character */
		if ((c = ttinc(0)) < 0) { /* Comm line hangup detected */
		    if (errno == 9999)	/* this value set by myread() */
		      printf("\r\nCommunications disconnect ");
		    else if (io_retry++ < 3) {
			tthang();
			continue;
		    }
		    if (errno != 9999)
		      perror("\r\nCan't get character");
		    kill(parent_id,SIGUSR1); /* notify parent. */
		    pause();		/* Wait to be killed by parent. */
		}
		c &= cmask;		/* Got a char, strip parity, etc */
		conoc(c);		/* Put it on the screen. */
		if (seslog) zchout(ZSFILE,c);	/* If logging, log it. */
		while ((n = ttchk()) > 0) {	/* Any more left in buffer? */
		    if (n > LBUFL) n = LBUFL;   /* Get them all at once. */
		    if ((n = ttxin(n,lbuf)) > 0) {
			for (i = 0; i < n; i++) lbuf[i] &= cmask;  /* Strip */
			conxo(n,lbuf);	    	    	    	   /* Output */
			if (seslog) zsoutx(ZSFILE,lbuf,n);  	   /* Log */
		    }
	    	}
	    }
    	}
}

/*  H C O N N E  --  Give help message for connect.  */

hconne() {
    int c;
    static char *hlpmsg[] = {"\
\r\n  C to close the connection, or:",
"\r\n  0 (zero) to send a null",
"\r\n  B to send a BREAK",
"\r\n  H to hangup",
"\r\n  Q to hangup and quit Kermit",
"\r\n  S for status",
"\r\n  ? for help",
"\r\n escape character twice to send the escape character.\r\n\r\n",
"" };

    conola(hlpmsg);			/* Print the help message. */
    conol("Command>");			/* Prompt for command. */
    c = coninc(0) & 0177;		/* Get character, strip any parity. */
    conoc(c);				/* Echo it. */
    conoll("");
    return(c);				/* Return it. */
}


/*  C H S T R  --  Make a printable string out of a character  */

char *
chstr(c) int c; {
    static char s[8];
    char *cp = s;

    if (c < SP) {
	sprintf(cp,"CTRL-%c",ctl(c));
    } else sprintf(cp,"'%c'\n",c);
    cp = s;
    return(cp);
}

/*  D O E S C  --  Process an escape character argument  */

doesc(c) char c; {
    CHAR d;
  
    while (1) {
	if (c == escape) {		/* Send escape character */
	    d = dopar(c); ttoc(d); return;
    	} else				/* Or else look it up below. */
	    if (isupper(c)) c = tolower(c);

	switch (c) {

	case 'c':			/* Close connection */
	case '\03':
	    active = 0; conol("\r\n"); return;

	case 'b':			/* Send a BREAK signal */
	case '\02':
	    ttsndb(); return;

	case 'h':			/* Hangup */
	case '\010':
	    dohangup = 1; active = 0; conol("\r\n"); return;

	case 'q':
	    quitnow = 1; active = 0; conol("\r\n"); return;

	case 's':			/* Status */
	    conol("\r\nConnected thru ");
	    conol(ttname);
	    if (speed >= 0) {
		sprintf(temp,", speed %d",speed); conol(temp);
	    }
	    sprintf(temp,", %d bits",(cmask == 0177) ? 7 : 8);
	    if (parity) {
		conol(", ");
		switch (parity) {
		    case 'e': conol("even");  break;
		    case 'o': conol("odd");   break;
		    case 's': conol("space"); break;
		    case 'm': conol("mark");  break;
		}
		conol(" parity");
	    }
	    if (seslog) {
		conol(", logging to "); conol(sesfil);
            }
	    conoll(""); return;

	case '?':			/* Help */
	    c = hconne(); continue;

	case '0':			/* Send a null */
	    c = '\0'; d = dopar(c); ttoc(d); return;

	case SP:			/* Space, ignore */
	    return;

	default:			/* Other */
	    conoc(BEL); return; 	/* Invalid esc arg, beep */
    	}	    
    }
}    
