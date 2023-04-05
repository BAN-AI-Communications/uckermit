/* C K C H D R -- Gneral purpose header file */

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

#include <stdio.h>

char *bldlen       (char *str, char *dest);
/* char *chstr     (int c); */
char  coninc       (int timo);
char *copy         (char *s);
/* CHAR dopar      (int ch); */
char  hconne       (void);
char  parser       (void);
char *rpar         (void);
char *tilde_expand (char *dirname);
char *whoami       (void);
char *zfcdat       (char *name);
char *zgtdir       (void);
char *zhome        (void);

int addaction     (int act, int state, int chr);
int addbuf        (char *cp);
int bcarcb        (long calsp);
int btest         (int x, int m);
int canned        (char *buf);
int chk1          (register CHAR *pkt);
int chkfn         (int n);
int chkint        (void);
int chkspd        (int x);
int chkwld        (char *s);
int ckdial        (char *telnbr);
int clrhash       (void);
int clsof         (int disp);
int cmcfm         (void);
int cmdini        (void);
int cmdir         (char *xhlp, char *xdef, char **xp);
int cmdlin        (void);
int cmfld         (char *xhlp, char *xdef, char **xp);
int cmifi         (char *xhlp, char *xdef, char **xp, int *wild);
/* int cmkey      (struct keytab table[], int n, char *xhlp, char *xdef); */
int cmnum         (char *xhlp, char *xdef, int radix, int *n);
int cmofi         (char *xhlp, char *xdef, char **xp);
int cmtxt         (char *xhlp, char *xdef, char **xp);
int conbin        (int esc);
int concb         (int esc);
int conchk        (void);
int conect        (void);
int congm         (void);
/* int conint     (SIGTYP (*f)(void)); */
int conola        (char *s[]);
int conol         (char *s);
int conoll        (char *s);
int conres        (void);
int conxo         (int x, char *s);
int copyact       (FILE *inp, FILE *outp, int actno);
int copyrest      (FILE *in, FILE *out);
int cwd           (char *vdir);
/* int decode     (register CHAR *buf, register int (*fn)(void)); */
int doarg         (int x);
int docmd         (int cx);
int doconect      (void);
int doexit        (int exitstat);
int dohlp         (int xx);
int dohrmt        (int xx);
int dohset        (int xx);
int dolog         (int x);
int doprm         (int xx);
int dormt         (int xx);
int dostat        (void);
int emptytbl      (void);
int enter         (char *name, int svalue);
int epilogue      (FILE *outfp);
int ermsg         (char *msg);
/* int faction    (Trans hd, int state, int chr); */
int fatal         (char *msg);
int fgen          (char *pat, char *resarry[], int len);
int fstats        (void);
int gattr         (char *s, struct zattr *yy);
int getpkt        (int bufmax);
int gettoken      (FILE *fp);
int gnfile        (void);
int gtimer        (void);
int gtword        (void);
int hash          (char *name);
int herald        (void);
int hmsga         (char *s[]);
int hmsg          (char *s);
int initattr      (struct zattr *yy);
int initial       (FILE *infp, FILE *outfp);
int isin          (char *s, int c);
int iswild        (char *str);
int isword        (int c);
int kinput        (void);
int lkup          (char *name);
int login         (char *cmdstr);
/* int lookup     (struct keytab table[], char *cmd, int n, int *x); */
int lower         (char *s);
int main          (int argc, char *argv[]);
int match         (char *pattern, char *string);
int msleep        (int m);
int opena         (char *f, struct zattr *zz);
int openi         (char *name);
int openo         (char *name);
int opent         (void);
int prolog        (FILE *outfp);
int putfil        (int c);
int putsrv        (int c);
int puttrm        (int c);
int rcvfil        (char *n);
int rdcmnt        (FILE *fp);
int rdigits       (char *s);
int rdstates      (FILE *fp, FILE *ofp);
int rdword        (FILE *fp, char *buf);
int reof          (struct zattr *yy);
int rfilop        (char *s, int t);
int rpack         (void);
int rsattr        (char *s);
int sattr         (int xp);
int sdata         (void);
int setatm        (char *cp);
int setcc         (int *prm, int x, int y);
int setgen        (int type, char *arg1, char *arg2, char *arg3);
int setnum        (int *prm, int x, int y, int max);
int seton         (int *prm);
int sfile         (int x);
int shopar        (void);
/* int sigint     (void); */
int sinit         (void);
int sndhlp        (void);
int spack         (int type, int n, register int len, register char *d);
/* int statelist  (FILE *fp, Trans t); */
int syscleanup    (void);
int syscmd        (char *prefix, char *suffix);
int sysinit       (void);
/* int teststate  (int state, Trans t); */
int transmit      (char *s, int t);
/* int trap       (void); */
int ttchk         (void);
int ttclos        (void);
int ttflui        (void);
int tthang        (void);
int ttinc         (int timo);
int ttinl         (CHAR *dest, int max, int timo, int eol);
int ttoc          (int c);
int ttol          (char *s, int n);
int ttopen        (char *ttname, int *lcl, int modem);
/* int ttpkt      (int speed, int flow, int parity); */
int ttres         (void);
int ttsndb        (void);
int ttvt          (int speed, int flow);
int ttxin         (int n, CHAR *buf);
int usage         (void);
int warray        (FILE *fp, char *nam, int cont[], int siz, char *typ);
int writetbl      (FILE *fp);
/* int wsetstate  (int state, Trans t); */
int zchdir        (char *dirnam);
int zchko         (char *name);
int zchout        (int n, int c);
int zclose        (int n);
int zclosf        (void);
int zinfill       (void);
int zkself        (void);
int zmail         (char *p, char *f);
int znext         (char *fn);
int zopeni        (int n, char *name);
int zopeno        (int n, char *name);
int zoutdump      (void);
int zprint        (char *p, char *f);
int zsattr        (struct zattr *xx);
int zsout         (int n, char *s);
int zsoutl        (int n, char *s);
int zsoutx        (int n, char *s, int x);
int zxcmd         (char *comand);
int zxpand        (char *fn);

long ttsspd (long speed);
long zchki  (char *name);

SIGTYP conn_int (void);
SIGTYP dialint  (void);
SIGTYP dialtime (void);
SIGTYP scrtime  (void);
SIGTYP timerh   (void);

struct path *splitpath(char *p);

/* Trans newtrans (void);                    */
/* Trans rdinput  (FILE *infp, FILE *outfp); */
/* Trans rdrules  (FILE *fp, FILE *out);     */

unsigned int chk2 (register CHAR *pkt);
unsigned int chk3 (register CHAR *pkt);

void ack1       (char *s);
void ack        (void);
void addhlp     (char *s);
void addresult  (char *str);
void clrhlp     (void);
void clsif      (void);
void cmini      (int d);
void cmres      (void);
void cmsavp     (char s[], int n);
void cmsetp     (char *s);
void connoi     (void);
void conoc      (int c);
void dmphlp     (void);
void doesc      (int c);
void encstr     (char *s);
/* void errpkt  (char *reason); */
void flushi     (void);
void intmsg     (long n);
void nack       (void);
void nxtpkt     (int *num);
void prompt     (void);
void rcalcpsz   (void);
void reot       (void);
void resend     (void);
void resetc     (void);
void rinit      (char *d);
void rtimer     (void);
void scmd       (int t, char *dat);
void screen     (int f, int c, long n, char *s);
void sdahead    (void);
void seof       (char *s);
void seot       (void);
void sipkt      (int c);
void spar       (char *s);
void srinit     (void);
/* void stptrap (void); */
void stripq     (char *s);
void tinit      (void);
void traverse   (struct path *pl, char *sofar, char *endcur);
void trtrap     (void);
void tstats     (void);
void xcpy       (register char *to, register char *from, register unsigned len);
void zdelet     (char *name);
void zltor      (char *name, char *name2);
void znewn      (char *fn, char **s);
void zrtol      (char *name, char *name2);
void ztime      (char **s);

int debopn( /* XXX(jhj): wtf */
#ifdef DEBUG
  char *s
#endif /* ifdef DEBUG */
);
