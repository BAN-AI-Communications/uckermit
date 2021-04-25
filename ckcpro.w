#ifndef NOICP
char *protv = "Kermit Protocol Module, 4G(057), 2021-APR-24";
#endif /* ifndef NOICP */

/* C K C P R O -- uCKermit Protocol Module in Wart preprocessor notation */

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

/*
 * Author: Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
 * Columbia University Center for Computing Activities.
 *
 * First released January 1985.
 *
 * Copyright (C) 1985, 1989,
 *   Trustees of Columbia University in the City of New York.
 *
 * Permission is granted to any individual or institution to use, copy,
 *   or redistribute this software so long as it is not sold for profit,
 *   provided this copyright notice is retained. 
 */

#include "ckcdeb.h"
#include "ckcker.h"

/*
 * Note: This file may also be preprocessed by the UNIX Lex program, but 
 * you must indent the above #include statements before using Lex, and
 * then restore them to the left margin in the resulting C program before
 * compilation. Also, the invocation of the "wart()" function below must
 * be replaced by an invocation of the "yylex()" function. It might also
 * be necessary to remove comments in the %%...%% section.
 */

 /*
  * State definitions
  * for Wart (or Lex)
  */

%states ipkt rfile rattr rdata ssinit ssfile ssattr ssdata sseof sseot
%states serve generic get rgen

 /*
  * External uCKermit 
  * variable declarations
  */

  extern char sstate, *versio, *srvtxt, *cmarg, *cmarg2, *rpar();
  extern char data[], filnam[], srvcmd[], ttname[], *srvptr;
  extern int pktnum, timint, nfils, hcflg, xflg, speed, flow, mdmtyp;
  extern int prvpkt, cxseen, czseen, local;
#ifndef NOSERVER
  extern int server;
#endif /* ifndef NOSERVER */
  extern int displa, bctu, bctr, quiet;
#ifndef NOSTATS
  extern int tsecs;
#endif /* ifndef NOSTATS */
  extern int parity, backgrd, nakstate, atcapu;
  extern int putsrv(), puttrm(), putfil(), errpkt();
  extern CHAR *rdatap, recpkt[];
  extern char *DIRCMD, *DELCMD, *TYPCMD, *SPACMD, *SPACM2, *WHOCMD;
  extern struct zattr iattr;

 /*
  * Local
  * variables
  */

  static char vstate = 0;               /* Saved State   */
  static char vcmd = 0;                 /* Saved Command */
  int x;                                /* General-purpose integer */
  char *s;                              /* General-purpose string pointer */

 /*
  * Macros - Note: BEGIN is predefined
  * by Wart (and Lex) as "state = ",
  * BEGIN is NOT a GOTO!
  */

#define SERVE  \
  tinit(); BEGIN serve
#ifndef NOSERVER
#define RESUME \
  if (server) { SERVE; } else \
    { sleep(2); \
          return; }
#else /* ifndef NOSERVER */
#define RESUME { sleep(2); return; }
#endif /* ifndef NOSERVER */
%%

 /*
  * Protocol entry points, one
  * for each start state (sstate)
  */

s { tinit();                                   /* Do Send command */
    if (sinit()) BEGIN ssinit;
       else RESUME; }

v { tinit(); BEGIN get; }                      /* Receive */
r { tinit(); vstate = get;  vcmd = 0;   \
    sipkt('I'); BEGIN ipkt; }                  /* Get */
c { tinit(); vstate = rgen; vcmd = 'C'; \
    sipkt('I'); BEGIN ipkt; }                  /* Host */
g { tinit(); vstate = rgen; vcmd = 'G'; \
    sipkt('I'); BEGIN ipkt; }                  /* Generic */

x { sleep(1); SERVE; }                         /* Be a Server */

a { errpkt("User cancelled transaction");      /* ABEND: Tell other side. */
    return(0); }                               /* Return from protocol. */

/*
 * Dynamic states:
 * <current-states>input-character { action }
 */

<rgen,get,serve>S {                 /* Receive Send-Init packet */
    rinit(rdatap);                  /* Set parameters */
    bctu = bctr;                    /* Switch to agreed-upon block check */
#ifndef NOSTATS
    resetc();                       /* Reset counters */
    rtimer();                       /* Reset timer */
#endif
    BEGIN rfile;                    /* Go into receive-file state */
}

/*
 * States in which we get replies back
 * from commands sent to a server.
 */

<ipkt>Y {                             /* Get ack for I-packet */
    spar(rdatap);                     /* Set parameters */
    if (vcmd) {                       /* If sending a generic command */
        scmd(vcmd,cmarg);             /* Do that */
        vcmd = 0;                     /* and then un-remember it. */
    }
    if (vstate == get)
          srinit();                   /* If sending GET command, do that */
    BEGIN vstate;                     /* Switch to desired state */
}

<ipkt>E {                             /* Ignore Error reply to I packet */
    if (vcmd) {                       /* in case other Kermit doesn't */
        scmd(vcmd,cmarg);             /* understand I-packets. */
        vcmd = 0;                     /* Otherwise act as above... */
    }
    if (vstate == get)
          srinit();
    BEGIN vstate;
}

<get>Y {                              /* Resend of previous I-pkt ACK, */
    srinit();                         /* same seq number! */
}

 /*
  * States in which we're
  * being a server
  */

<serve>I {                            /* Get I-packet */
    spar(rdatap);                     /* Set parameters from it */
    ack1(rpar());                     /* Respond with our own parameters */
    pktnum = 0;                       /* Reset packet sequence numbers */
    prvpkt = -1;                      /* Stay in server command wait */
}

<serve>R {                            /* Get Receive-Init */
    srvptr = srvcmd;                  /* Point to server command buffer */
    decode(rdatap,putsrv);            /* Decode the GET command into it */
    cmarg = srvcmd;
    nfils = -1;                       /* Initialize number of files */
    if (sinit())                      /* Send Send-Init */
      BEGIN ssinit;                   /* If successful, switch state */
    else { SERVE; }                   /* Else back to server command wait */
}

<serve>G {                            /* Generic server command */
    srvptr = srvcmd;                  /* Point to command buffer */
    decode(rdatap,putsrv);            /* Decode packet data into it */
    putsrv('\0');                     /* Insert a couple nulls */
    putsrv('\0');                     /* for termination */
    sstate = srvcmd[0];               /* Set requested start state */
    BEGIN generic;                    /* Switch to generic command state */
}

<serve>C {                            /* Receive Host command */
    srvptr = srvcmd;                  /* Point to command buffer */
    decode(rdatap,putsrv);            /* Decode command packet into it */
    putsrv('\0');                     /* Null-terminate */
    if (syscmd(srvcmd,""))            /* Try to execute the command */
      BEGIN ssinit;                   /* If OK, send back its output */
    else {                            /* Otherwise */
      errpkt(
        "Can't do system command");   /* report error */
          SERVE;                      /* & go back to server command wait */
    }
}

<serve>. {                            /* Any other command in this state */
    errpkt(
        "Unimplemented server function"); /* we don't know about */
    SERVE;                            /* back to server command wait */
}

<generic>C {                          /* Got REMOTE CWD command */
    if (!cwd(srvcmd+1))
          errpkt(
            "Can't change directory");    /* Try to do it */
    SERVE;                            /* Back to server command wait */
}

<generic>D {                          /* REMOTE DIRECTORY command */
    if (syscmd(DIRCMD,srvcmd+2))      /* If it can be done */
      BEGIN ssinit;                   /* send the results back */
    else {                            /* otherwise */
        errpkt(
                  "Can't list directory");    /* report failure */
        SERVE;                        /* & return to server command wait */
    }
}

<generic>E {                          /* REMOTE DELETE (Erase) command */
    if (syscmd(DELCMD,srvcmd+2))      /* Try to do it */
      BEGIN ssinit;                   /* If OK send results back */
    else {                            /* otherwise */
        errpkt(
                  "Can't remove file");       /* report failure */
        SERVE;                        /* & return to server command wait */
    }
}

<generic>F {                          /* FINISH */
    ack();                            /* Acknowledge */
    screen(SCR_TC,0,0l,"");           /* Display */
    return(0);                        /* Done */
}

<generic>L {                          /* BYE (LOGOUT) */
    ack();                            /* Acknowledge */
    ttres();                          /* Reset the terminal */
    screen(SCR_TC,0,0l,"");           /* Display */
    return(zkself());                 /* Try to log self out */
}

<generic>H {                          /* REMOTE HELP */
    if (sndhlp()) BEGIN ssinit;       /* Try to send it */
    else {                            /* If not ok, */
        errpkt(
                  "Can't send help");         /* send error message instead */
        SERVE;                        /* & return to server command wait */
    }
}

<generic>T {                          /* REMOTE TYPE */
    if (syscmd(TYPCMD,srvcmd+2))      /* Try */
      BEGIN ssinit;                   /* OK */
    else {                            /* not OK */
        errpkt(
                  "Can't type file");         /* give error message */
        SERVE;                        /* wait for next server command */
    }
}

<generic>U {                          /* REMOTE SPACE */
    x = *(srvcmd+1);                  /* Get area to check */
    x = ((x == '\0') || \
          (x == SP));
    x = (x ? syscmd(SPACMD,"") : \
          syscmd(SPACM2,srvcmd+2));
    if (x)                            /* If we got the info */
      BEGIN ssinit;                   /* send it */
    else {                            /* otherwise */
        errpkt(
                  "Can't check space");       /* send error message */
        SERVE;                        /* and await next server command */
    }
}

<generic>W {                          /* REMOTE WHO */
    if (syscmd(WHOCMD,srvcmd+2))      /* The now-familiar scenario... */
      BEGIN ssinit;
    else {
        errpkt(
                  "Can't do who command");
        SERVE;
    }
}

<generic>. {                          /* Anything else in this state ... */
    errpkt(
          "Unimplemented generic server function");              /* Complain */
    SERVE;                            /* & return to server command wait */
}

<rgen>Y {                             /* Short-Form reply */
    decode(rdatap,puttrm);            /* in ACK Data field */
    RESUME;
}

<rgen,rfile>F {                       /* File header */
    xflg = 0;                         /* Not screen data */
    rcvfil(filnam);                   /* Figure out local filename */
    encstr(filnam);                   /* Encode it */
    ack1(data);                       /* Send it back in ACK */
    initattr(&iattr);                 /* Clear file attribute structure */
    nakstate = 1;                     /* In this state we can send NAKs */
    BEGIN rattr;                      /* Now expect Attribute packets */
}

<rgen,rfile>X {                       /* X-packet instead of file header */
    xflg = 1;                         /* Screen data */
    ack();                            /* Acknowledge the X-packet */
    initattr(&iattr);                 /* Initialize attribute structure */
    nakstate = 1;                     /* Say that we can send NAKs */
    BEGIN rattr;                      /* Expect Attribute packets */
}

<rattr>A {                            /* Attribute packet */
    if (gattr(rdatap,&iattr) == 0)    /* Read into attribute structure */
      ack();                          /* If OK, acknowledge */
    else                              /* If not */
      ack1("N");                      /* refuse to accept the file */
}

<rattr>D {                            /* First data packet */
    if (xflg)                         /* If screen data */
      x = opent();                    /* "open" the screen */
    else                              /* otherwise */
      x = opena(filnam,&iattr);       /* open the file, with attributes */
    if (x) {                          /* If file was opened ok */
        if (
                 decode(
                   rdatap,putfil) < 0) {      /* decode first data packet */
          errpkt(
                    "Error writing data");
           RESUME;
        }
        ack();                        /* acknowledge it */
        BEGIN rdata;                  /* and switch to receive-data state */
    } else {                          /* otherwise */
        errpkt(
                  "Can't open file");         /* send error message */
        RESUME;                       /* and quit. */
    }
}

<rfile>B {                            /* EOT, no more files */
    ack();                            /* Acknowledge */
#ifndef NOSTATS
    tsecs = gtimer();                 /* Get timing for statistics */
#endif
    reot();                           /* Do EOT things */
    RESUME;                           /* and quit */
}

<rdata>D {                            /* Data packet */
    if (cxseen)                       /* If file interrupt */
      ack1("X");                      /* put "X" in ACK */
    else if (czseen)                  /* If file-group interrupt */
      ack1("Z");                      /* put "Z" in ACK */
    else if (
          decode(
            rdatap,putfil) < 0) {         /* Normal case, */
        errpkt(
                  "Error writing data");      /*   decode data to file */
        RESUME;                       /* Send ACK if data written */
    } else ack();                     /* to file OK. */
}

<rdata,rattr>Z {                      /* End Of File (EOF) Packet */
    if (reof(&iattr) < 0) {           /* Close & dispose of the file */
        errpkt(
                  "Can't close file");        /* If problem, send error message */
        RESUME;                       /* and quit */
    } else {                          /* otherwise */
        ack();                        /* acknowledge the EOF packet */
        BEGIN rfile;                  /* and await another file */
    }
}

<ssinit>Y {                           /* ACK for Send-Init */
    spar(rdatap);                     /* set parameters from it */
    bctu = bctr;                      /* switch to agreed-on block check */
    x = sfile(xflg);                  /* Send X or F header packet */
    if (x) {                          /* If the packet was sent OK */
#ifndef NOSTATS
        resetc();                     /* reset per-transaction counters */
        rtimer();                     /* reset timers */
#endif
        BEGIN ssfile;                 /* and switch to receive-file state */
    } else {                          /* otherwise send error msg & quit */
        s = xflg ? \
                  "Can't execute command" : \
                    "Can't open file";
        errpkt(s);
        RESUME;
    }
}

<ssfile>Y {                           /* ACK for F packet */
    srvptr = srvcmd;                  /* Point to string buffer */
    decode(rdatap,putsrv);            /* Decode data field, if any */
    putsrv('\0');                     /* Terminate with null */
    if (*srvcmd) {                    /* If remote name was recorded */
      tlog(
            F110," stored as",srvcmd,0);  /* Record it in transaction log. */
        }
    if (atcapu) {                     /* If attributes are to be used */
        if (sattr(xflg) < 0) {        /* set and send them */
            errpkt(
                        "Can't send attributes"); /* if problem, say so */
              RESUME;                 /* and quit */
        } else BEGIN ssattr;          /* if ok, switch to attribute state */
    } else if (sdata() < 0) {         /* No attributes, send data */
        clsif();                      /* If not ok, close input file, */
        seof("");                     /* send EOF packet */
        BEGIN sseof;                  /* and switch to EOF state. */
    } else BEGIN ssdata;              /* OK, switch to send-data state */
}

<ssattr>Y {                           /* Got ACK to A packet */
    if (rsattr(rdatap) < 0) {         /* Was the file refused? */
        clsif();                      /* yes, close it */
        seof("D");                    /* send EOF with "discard" code */
        BEGIN sseof;                  /* switch to send-EOF state */
    } else if (sdata() < 0) {         /* File accepted, send data */
        clsif();                      /* If problem, close input file */
        seof("");                     /* send EOF packet */
        BEGIN sseof;                  /* and switch to send-EOF state. */
    } else BEGIN ssdata;              /* All ok, enter send-data state. */
}

<ssdata>Y {                           /* Got ACK to Data packet */
    if (canned(rdatap)) {             /* If file transfer cancelled */
        clsif();                      /* close input file */
        seof("D");                    /* send EOF packet w/ Discard code */
        BEGIN sseof;                  /* switch to EOF state */
    } else if (sdata() < 0) {         /* Not cancelled, send next data */
        clsif();                      /* If there is a problem close file */
        seof("");                     /* Send EOF packet */
        BEGIN sseof;                  /* enter send-eof state */
    }
}

<sseof>Y {                            /* Got ACK to EOF */
    if (gnfile() > 0) {               /* Any more files to send? */
        if (sfile(xflg))              /* Yes try sending next file header */
          BEGIN ssfile;               /* if ok, enter send-file state */
        else {                        /* otherwise */
            errpkt(
                          "Can't open file");     /* send error message */
                RESUME;               /* and quit */
        }
    } else {                          /* No next file */
#ifndef NOSTATS
        tsecs = gtimer();             /* get statistics timers */
#endif
        seot();                       /* send EOT packet */
        BEGIN sseot;                  /* enter send-eot state */
    }
}

<sseot>Y {                            /* Got ACK to EOT */
    RESUME;                           /* All done, just quit */
}

E {                                   /* Got Error packet, in any state */
    ermsg(rdatap);                    /* Issue message. */
    x = quiet; quiet = 1;             /* Close files silently, */
    clsif(); clsof(1);                /* discarding any output file. */
#ifndef NOSTATS
    tsecs = gtimer();                 /* Get timers */
#endif
    quiet = x;                        /* restore quiet state */
    if (backgrd 
#ifndef NOSERVER
	&& !server
#endif
	)
          fatal("Protocol error");
    RESUME;
}

. {                                   /* Anything not accounted for above */
    errpkt("Unknown packet type");    /* Give error message */
    RESUME;                           /* and quit */
}
%%

/* P R O T O -- Protocol entry function */

proto()
{

    extern int sigint();
    int x;

    conint(sigint);                   /* Enable console interrupts */

 /*
  * Set up the communication
  * line for file transfer.
  */

    if (local && (speed < 0)) {
        screen(
                  SCR_EM,0,0l,"You must 'set speed' first");
        return;
    }

    x = -1;
    if (ttopen(ttname,&x,mdmtyp) < 0) {
        debug(
                  F111,"failed: proto ttopen local",ttname,local);
        screen(
                  SCR_EM,0,0l,"Can't open line");
        return;
    }
    if (x > -1) local = x;
    debug(
          F111,"proto ttopen local",ttname,local);

    x = (local) ? speed : -1;
    if (ttpkt(x,flow,parity) < 0) {   /* Put line in packet mode, */
        screen(
                  SCR_EM,0,0l,"Can't condition line");
        return;
    }
#ifndef NOSERVER
    if (sstate == 'x') {              /* If entering server mode, */
        server = 1;                   /* set flag, */
        if (!quiet) {
            if (!local)               /* and issue appropriate message. */
                conol(srvtxt);
            else {
                conol(
                                  "Entering server mode on ");
                conoll(ttname);
            }
        }
    } else { server = 0; }
#endif
    if (sstate == 'v' && !local && !quiet)
      conoll(
        "Escape back to your local Kermit and give a SEND command...");
    if (sstate == 's' && !local && !quiet)
      conoll(
        "Escape back to your local Kermit and give a RECEIVE command...");
    sleep(1);

 /*
  * The 'wart()' function is generated by the wart program. It gets a
  * character from the input() routine and then based on that character
  * and the current state, selects the appropriate action, according to
  * the state table above, which is transformed by the wart program into
  * a big case statement. The function is active for one transaction.
  */

    wart();                           /* Enter the state table switcher. */
#ifndef NOSERVER
    if (server) {                     /* Back from packet protocol. */
        server = 0;
        if (!quiet)                   /* Give appropriate message */
            conoll(
                          "uCKermit server done");
    }
#endif
    ttres();
    screen(
          SCR_TC,0,0l,"");                /* Transaction complete */
}
