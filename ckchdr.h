#include <stdio.h>

int input(void);
/* CHAR dopar(int ch); */
int spack(int type, int n, register int len, register char *d);
int chk1(register CHAR *pkt);
unsigned int chk2(register CHAR *pkt);
unsigned int chk3(register CHAR *pkt);
void ack(void);
void ack1(char *s);
void nack(void);
void rcalcpsz(void);
void resend(void);
/* void errpkt(char *reason); */
void scmd(int t, char *dat);
void srinit(void);
void nxtpkt(int *num);
/* int sigint(void); */
int rpack(void);
int sattr(int xp);
int rsattr(char *s);
int gattr(char *s, struct zattr *yy);
int initattr(struct zattr *yy);
void encstr(char *s);
int putsrv(int c);
int puttrm(int c);
int putfil(int c);
/* int decode(register CHAR *buf, register int (*fn)(void)); */
int getpkt(int bufmax);
int canned(char *buf);
void resetc(void);
void tinit(void);
void rinit(char *d);
int sinit(void);
void sipkt(int c);
int rcvfil(char *n);
int opena(char *f, struct zattr *zz);
int reof(struct zattr *yy);
void reot(void);
int sfile(int x);
void sdahead(void);
int sdata(void);
void seof(char *s);
void seot(void);
CHAR *rpar(void);
void spar(char *s);
int gnfile(void);
int openi(char *name);
int openo(char *name);
int opent(void);
void clsif(void);
int clsof(int disp);
int sndhlp(void);
int cwd(char *vdir);
int syscmd(char *prefix, char *suffix);
void cmsetp(char *s);
void cmsavp(char s[], int n);
void prompt(void);
void cmres(void);
void cmini(int d);
void stripq(char *s);
int cmnum(char *xhlp, char *xdef, int radix, int *n);
int cmofi(char *xhlp, char *xdef, char **xp);
int cmifi(char *xhlp, char *xdef, char **xp, int *wild);
int cmdir(char *xhlp, char *xdef, char **xp);
int chkwld(char *s);
int cmfld(char *xhlp, char *xdef, char **xp);
int cmtxt(char *xhlp, char *xdef, char **xp);
/* int cmkey(struct keytab table[], int n, char *xhlp, char *xdef); */
int cmcfm(void);
void clrhlp(void);
void addhlp(char *s);
void dmphlp(void);
/* int lookup(struct keytab table[], char *cmd, int n, int *x); */
int gtword(void);
int addbuf(char *cp);
int setatm(char *cp);
int rdigits(char *s);
int lower(char *s);
int btest(int x, int m);
SIGTYP conn_int(void);
int conect(void);
char hconne(void);
/* char *chstr(int c); */
void doesc(int c);
void xcpy(register char *to, register char *from, register unsigned len);
SIGTYP dialtime(void);
SIGTYP dialint(void);
int ckdial(char *telnbr);
int zkself(void);
int zopeni(int n, char *name);
int zopeno(int n, char *name);
int zclose(int n);
int zinfill(void);
int zsout(int n, char *s);
int zsoutl(int n, char *s);
int zsoutx(int n, char *s, int x);
int zchout(int n, int c);
int zoutdump(void);
int chkfn(int n);
long zchki(char *name);
int zchko(char *name);
void zdelet(char *name);
void zrtol(char *name, char *name2);
void zltor(char *name, char *name2);
int zchdir(char *dirnam);
char *zhome(void);
char *zgtdir(void);
int zxcmd(char *comand);
int zclosf(void);
int zxpand(char *fn);
int znext(char *fn);
void znewn(char *fn, char **s);
int zsattr(struct zattr *xx);
char *zfcdat(char *name);
int zmail(char *p, char *f);
int zprint(char *p, char *f);
struct path *splitpath(char *p);
int fgen(char *pat, char *resarry[], int len);
void traverse(struct path *pl, char *sofar, char *endcur);
void addresult(char *str);
int iswild(char *str);
int match(char *pattern, char *string);
char *whoami(void);
char *tilde_expand(char *dirname);
SIGTYP scrtime(void);
int login(char *cmdstr);
void flushi(void);
int sysinit(void);
int syscleanup(void);
int ttopen(char *ttname, int *lcl, int modem);
int ttclos(void);
int tthang(void);
int ttres(void);
/* int ttpkt(int speed, int flow, int parity); */
int ttvt(int speed, int flow);
long ttsspd(long speed);
int ttflui(void);
SIGTYP timerh(void);
/* int conint(SIGTYP (*f)(void)); */
void connoi(void);
int ttchk(void);
int ttxin(int n, CHAR *buf);
int ttol(char *s, int n);
int ttoc(int c);
int ttinl(CHAR *dest, int max, int timo, int eol);
int ttinc(int timo);
int ttsndb(void);
int msleep(int m);
void rtimer(void);
int gtimer(void);
void ztime(char **s);
int congm(void);
int concb(int esc);
int conbin(int esc);
int conres(void);
void conoc(int c);
int conxo(int x, char *s);
int conol(char *s);
int conola(char *s[]);
int conoll(char *s);
int conchk(void);
char coninc(int timo);
int usage(void);
int dohlp(int xx);
int hmsg(char *s);
int hmsga(char *s[]);
int bcarcb(long calsp);
int dohset(int xx);
int dohrmt(int xx);
int dolog(int x);

int debopn(
#ifdef DEBUG
  char *s
#endif /* ifdef DEBUG */
  );

int shopar(void);
int dostat(void);
int fstats(void);
void tstats(void);
int doprm(int xx);
int chkspd(int x);
int seton(int *prm);
int setnum(int *prm, int x, int y, int max);
int setcc(int *prm, int x, int y);
int dormt(int xx);
int rfilop(char *s, int t);
void screen(int f, int c, long n, char *s);
void intmsg(long n);
int chkint(void);
int cmdlin(void);
int doarg(int x);
int fatal(char *msg);
int ermsg(char *msg);
int cmdini(void);
int herald(void);
/*int trap(void); */
/* void stptrap(void); */
char parser(void);
int doexit(int exitstat);
char *bldlen(char *str, char *dest);
int setgen(int type, char *arg1, char *arg2, char *arg3);
int docmd(int cx);
int doconect(void);
void trtrap(void);
int transmit(const char *s, int t);
/* int wsetstate(int state, Trans t); */
/* int teststate(int state, Trans t); */
/* Trans rdinput(FILE *infp, FILE *outfp); */
int initial(FILE *infp, FILE *outfp);
int isin(char *s, int c);
int isword(int c);
int rdword(FILE *fp, char *buf);
int rdstates(FILE *fp, FILE *ofp);
/* Trans newtrans(void); */
/* Trans rdrules(FILE *fp, FILE *out); */
/* int statelist(FILE *fp, Trans t); */
int copyact(FILE *inp, FILE *outp, int actno);
/* int faction(Trans hd, int state, int chr); */
int emptytbl(void);
int addaction(int act, int state, int chr);
int writetbl(FILE *fp);
int warray(FILE *fp, char *nam, int cont[], int siz, char *typ);
int main(int argc, char *argv[]);
int fatal(char *msg);
int prolog(FILE *outfp);
int epilogue(FILE *outfp);
int copyrest(FILE *in, FILE *out);
int gettoken(FILE *fp);
int rdcmnt(FILE *fp);
int clrhash(void);
int hash(char *name);
char *copy(char *s);
int enter(char *name, int svalue);
int lkup(char *name);
