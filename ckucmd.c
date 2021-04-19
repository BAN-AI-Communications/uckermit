char *cmdv = "Unix cmd package V2(025), 19 Jul 89";
 
/*  C K U C M D  --  Interactive command package for Unix  */

/*
 Author: Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
 Columbia University Center for Computing Activities.
 First released January 1985.
 Copyright (C) 1985, 1989, Trustees of Columbia University in the City of New 
 York.  Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained. 
*/

/*
 Modelled after the DECSYSTEM-20 command parser (the COMND JSYS)
 
 Features:
 . parses and verifies keywords, filenames, text strings, numbers, other data
 . displays appropriate menu or help message when user types "?"
 . does keyword and filename completion when user types ESC or TAB
 . accepts any unique abbreviation for a keyword
 . allows keywords to have attributes, like "invisible"
 . can supply defaults for fields omitted by user
 . provides command line editing (character, word, and line deletion)
 . accepts input from keyboard, command files, or redirected stdin
 . allows for full or half duplex operation, character or line input
 . settable prompt, protected from deletion
 
 Functions:
  cmsetp - Set prompt (cmprom is prompt string, cmerrp is error msg prefix)
  cmsavp - Save current prompt
  prompt - Issue prompt 
  cmini  - Clear the command buffer (before parsing a new command)
  cmres  - Reset command buffer pointers (before reparsing)
  cmkey  - Parse a keyword
  cmnum  - Parse a number
  cmifi  - Parse an input file name
  cmofi  - Parse an output file name
  cmdir  - Parse a directory name (UNIX only)
  cmfld  - Parse an arbitrary field
  cmtxt  - Parse a text string
  cmcfm  - Parse command confirmation (end of line)
  stripq - Strip out backslash quotes from a string.
 
 Return codes:
  -3: no input provided when required
  -2: input was invalid
  -1: reparse required (user deleted into a preceding field)
   0 or greater: success
  See individual functions for greater detail.
 
 Before using these routines, the caller should #include ckucmd.h, and
 set the program's prompt by calling cmsetp().  If the file parsing
 functions cmifi and cmofi are to be used, this module must be linked
 with a ck?fio file system support module for the appropriate system,
 e.g. ckufio for Unix.  If the caller puts the terminal in
 character wakeup ("cbreak") mode with no echo, then these functions will
 provide line editing -- character, word, and line deletion, as well as
 keyword and filename completion upon ESC and help strings, keyword, or
 file menus upon '?'.  If the caller puts the terminal into character
 wakeup/noecho mode, care should be taken to restore it before exit from
 or interruption of the program.  If the character wakeup mode is not
 set, the system's own line editor may be used.
*/

/* Includes */
 
#include <stdio.h>                      /* Standard C I/O package */
#include <ctype.h>                      /* Character types */
#include "ckucmd.h"                     /* Command parsing definitions */
#include "ckcdeb.h"                     /* Formats for debug(), etc. */
#ifdef OS2
#define INCL_SUB
#include <os2.h>
#endif /* OS2 */

#ifdef OSK
#define cc ccount			/* OS-9/68K compiler bug */
#endif

/* Local variables */
 
int psetf = 0,                          /* Flag that prompt has been set */
    cc = 0,                             /* Character count */
    dpx = 0;                            /* Duplex (0 = full) */
 
int hw = HLPLW,                         /* Help line width */
    hc = HLPCW,                         /* Help line column width */
    hh,                                 /* Current help column number */
    hx;                                 /* Current help line position */
 
#define PROML 60                        /* Maximum length for prompt */
 
char cmprom[PROML+1];                   /* Program's prompt */
char *dfprom = "Command? ";             /* Default prompt */
 
char cmerrp[PROML+1];                   /* Program's error message prefix */
 
int cmflgs;                             /* Command flags */
 
char cmdbuf[CMDBL+4];                   /* Command buffer */
char hlpbuf[HLPBL+4];                   /* Help string buffer */
char atmbuf[ATMBL+4];                   /* Atom buffer */
char filbuf[ATMBL+4];                   /* File name buffer */
 
/* Command buffer pointers */
 
static char *bp,                        /* Current command buffer position */
    *pp,                                /* Start of current field */
    *np;                                /* Start of next field */
 
long zchki();                           /* From ck?fio.c. */

 
/*  C M S E T P  --  Set the program prompt.  */
 
cmsetp(s) char *s; {
    char *sx, *sy, *strncpy();
    psetf = 1;                          /* Flag that prompt has been set. */
    strncpy(cmprom,s,PROML - 1);        /* Copy the string. */
    cmprom[PROML] = NUL;                /* Ensure null terminator. */
    sx = cmprom; sy = cmerrp;           /* Also use as error message prefix. */
    while (*sy++ = *sx++) ;             /* Copy. */
    sy -= 2; if (*sy == '>') *sy = NUL; /* Delete any final '>'. */
}
/*  C M S A V P  --  Save a copy of the current prompt.  */
 
cmsavp(s,n) int n; char s[]; {
    extern char *strncpy();                                     /* +1   */
    strncpy(s,cmprom,n-1);
    s[n] = NUL;
}
 
/*  P R O M P T  --  Issue the program prompt.  */
 
prompt() {
    if (psetf == 0) cmsetp(dfprom);     /* If no prompt set, set default. */
#ifdef OSK
    fputs(cmprom, stdout);
#else
    printf("\r%s",cmprom);              /* Print the prompt. */
#endif
}
 
 
/*  C M R E S  --  Reset pointers to beginning of command buffer.  */
 
cmres() {  
    cc = 0;                             /* Reset character counter. */
    pp = np = bp = cmdbuf;              /* Point to command buffer. */
    cmflgs = -5;                        /* Parse not yet started. */
}
 
 
/*  C M I N I  --  Clear the command and atom buffers, reset pointers.  */
 
/*
The argument specifies who is to echo the user's typein --
  1 means the cmd package echoes
  0 somebody else (system, front end, terminal) echoes
*/
cmini(d) int d; {
    for (bp = cmdbuf; bp < cmdbuf+CMDBL; bp++) *bp = NUL;
    *atmbuf = NUL;
    dpx = d;
    cmres();
}
 
stripq(s) char *s; {                    /* Function to strip '\' quotes */
    char *t;
    while (*s) {
        if (*s == '\\') {
            for (t = s; *t != '\0'; t++) *t = *(t+1);
        }
        s++;
    }
}

 
/*  C M N U M  --  Parse a number in the indicated radix  */
 
/*  For now, only works for positive numbers in base 10.  */
 
/*
 Returns
   -3 if no input present when required,
   -2 if user typed an illegal number,
   -1 if reparse needed,
    0 otherwise, with n set to number that was parsed
*/
cmnum(xhlp,xdef,radix,n) char *xhlp, *xdef; int radix, *n; {
    int x; char *s;
 
    if (radix != 10) {                  /* Just do base 10 for now */
        printf("cmnum: illegal radix - %d\n",radix);
        return(-1);
    }
 
    x = cmfld(xhlp,xdef,&s);
    debug(F101,"cmnum: cmfld","",x);
    debug(F111,"cmnum: atmbuf",atmbuf,cc);
    if (x < 0) return(x);		/* Parse a field */
 
    if (rdigits(atmbuf)) {               /* Convert to number */
        *n = atoi(atmbuf);
        return(x);
    } else {
        printf("\n?not a number - %s\n",s);
        return(-2);     
    }
}

 
/*  C M O F I  --  Parse the name of an output file  */
 
/*
 Depends on the external function zchko(); if zchko() not available, use
 cmfld() to parse output file names.
 
 Returns
   -3 if no input present when required,
   -2 if permission would be denied to create the file,
   -1 if reparse needed,
    0 or 1 otherwise, with xp pointing to name.
*/
cmofi(xhlp,xdef,xp) char *xhlp, *xdef, **xp; {
    int x; char *s;
#ifdef DTILDE
    char *tilde_expand(), *dirp;
#endif 

    if (*xhlp == NUL) xhlp = "Output file";
    *xp = "";
 
    if ((x = cmfld(xhlp,xdef,&s)) < 0) return(x);

#ifdef DTILDE
    dirp = tilde_expand(s);		/* Expand tilde, if any, */
    if (*dirp != '\0') setatm(dirp);	/* right in the atom buffer. */
#endif

    if (chkwld(s)) {
        printf("\n?Wildcards not allowed - %s\n",s);
        return(-2);
    }
    if (zchko(s) < 0) {
        printf("\n?Write permission denied - %s\n",s);
        return(-2);
    } else {
        *xp = s;
        return(x);
    }
}

 
/*  C M I F I  --  Parse the name of an existing file  */
 
/*
 This function depends on the external functions:
   zchki()  - Check if input file exists and is readable.
   zxpand() - Expand a wild file specification into a list.
   znext()  - Return next file name from list.
 If these functions aren't available, then use cmfld() to parse filenames.
*/
/*
 Returns
   -4 EOF
   -3 if no input present when required,
   -2 if file does not exist or is not readable,
   -1 if reparse needed,
    0 or 1 otherwise, with:
        xp pointing to name,
        wild = 1 if name contains '*' or '?', 0 otherwise.
*/
cmifi(xhlp,xdef,xp,wild) char *xhlp, *xdef, **xp; int *wild; {
    int i, x, xc; long y; char *sp;
#ifdef DTILDE
    char *tilde_expand(), *dirp;
#endif
 
    cc = xc = 0;                        /* Initialize counts & pointers */
    *xp = "";
    if ((x = cmflgs) != 1) {            /* Already confirmed? */
        x = gtword();                   /* No, get a word */
    } else {
        cc = setatm(xdef);              /* If so, use default, if any. */
    }
    *xp = atmbuf;                       /* Point to result. */
    *wild = chkwld(*xp);
 
    while (1) {
        xc += cc;                       /* Count the characters. */
        debug(F111,"cmifi: gtword",atmbuf,xc);
        switch (x) {
            case -4:                    /* EOF */
            case -2:                    /* Out of space. */
            case -1:                    /* Reparse needed */
                return(x);
 
/* cont'd... */

 
/* ...cmifi(), cont'd */
 
            case 0:                     /* SP or NL */
            case 1:
                if (xc == 0) *xp = xdef;     /* If no input, return default. */
                else *xp = atmbuf;
                if (**xp == NUL) return(-3); /* If field empty, return -3. */

#ifdef DTILDE
		dirp = tilde_expand(*xp);    /* Expand tilde, if any, */
		if (*dirp != '\0') setatm(dirp); /* right in atom buffer. */
#endif
                /* If filespec is wild, see if there are any matches */
 
                *wild = chkwld(*xp);
                debug(F101," *wild","",*wild);
                if (*wild != 0) {
                    y = zxpand(*xp);
                    if (y == 0) {
                        printf("\n?No files match - %s\n",*xp);
                        return(-2);
                    } else if (y < 0) {
                        printf("\n?Too many files match - %s\n",*xp);
                        return(-2);
                    } else return(x);
                }
 
                /* If not wild, see if it exists and is readable. */
 
                y = zchki(*xp);

                if (y == -3) {
                    printf("\n?Read permission denied - %s\n",*xp);
                    return(-2);
                } else if (y == -2) {
                    printf("\n?File not readable - %s\n",*xp);
                    return(-2);
                } else if (y < 0) {
                    printf("\n?File not found - %s\n",*xp);
                    return(-2);
                }
                return(x);
/* cont'd... */

/* ...cmifi(), cont'd */
 
 
            case 2:                     /* ESC */
                if (xc == 0) {
                    if (*xdef != '\0') {
                        printf("%s ",xdef); /* If at beginning of field, */
                        addbuf(xdef);   /* supply default. */
                        cc = setatm(xdef);
                    } else {            /* No default */
                        putchar(BEL);
                    }
                    break;
                } 
#ifdef DTILDE
		dirp = tilde_expand(*xp);    /* Expand tilde, if any, */
		if (*dirp != '\0') setatm(dirp); /* in the atom buffer. */
#endif
                if (*wild = chkwld(*xp)) {  /* No completion if wild */
                    putchar(BEL);
                    break;
                }
                sp = atmbuf + cc;
                *sp++ = '*';
                *sp-- = '\0';
                y = zxpand(atmbuf);     /* Add * and expand list. */
                *sp = '\0';             /* Remove *. */
 
                if (y == 0) {
                    printf("\n?No files match - %s\n",atmbuf);
                    return(-2);
                } else if (y < 0) {
                    printf("\n?Too many files match - %s\n",atmbuf);
                    return(-2);
                } else if (y > 1) {     /* Not unique, just beep. */
                    putchar(BEL);
                } else {                /* Unique, complete it.  */
                    znext(filbuf);      /* Get whole name of file. */
                    sp = filbuf + cc;   /* Point past what user typed. */
                    printf("%s ",sp);   /* Complete the name. */
                    addbuf(sp);         /* Add the characters to cmdbuf. */
                    setatm(pp);         /* And to atmbuf. */
                    *xp = atmbuf;       /* Return pointer to atmbuf. */
                    return(cmflgs = 0);
                }
                break;
 
/* cont'd... */

 
/* ...cmifi(), cont'd */
 
 
            case 3:                     /* Question mark */
                if (*xhlp == NUL)
                    printf(" Input file specification");
                else
                    printf(" %s",xhlp);
                if (xc > 0) {
#ifdef DTILDE
		    dirp = tilde_expand(*xp);    /* Expand tilde, if any */
		    if (*dirp != '\0') setatm(dirp);
#endif
                    sp = atmbuf + cc;   /* Insert "*" at end */
#ifdef datageneral
                    *sp++ = '+';        /* Insert +, the DG wild card */
#else
                    *sp++ = '*';
#endif
                    *sp-- = '\0';
                    y = zxpand(atmbuf);
                    *sp = '\0';
                    if (y == 0) {                   
                        printf("\n?No files match - %s\n",atmbuf);
                        return(-2);
                    } else if (y < 0) {
                        printf("\n?Too many file match - %s\n",atmbuf);
                        return(-2);
                    } else {
                        printf(", one of the following:\n");
                        clrhlp();
                        for (i = 0; i < y; i++) {
                            znext(filbuf);
                            addhlp(filbuf);
                        }
                        dmphlp();
                    }
                } else printf("\n");
                printf("%s%s",cmprom,cmdbuf);
                break;
        }
    x = gtword();
    }
}

/*  C M D I R  --  Parse a directory specification  */
 
/*
 This function depends on the external functions:
   zchki()  - Check if input file exists and is readable.
 If these functions aren't available, then use cmfld() to parse dir names.
 Note: this function quickly cobbled together, mainly by deleting lots of
 lines from cmifi().  It seems to work, but various services are missing,
 like completion, lists of matching directories on "?", etc.
*/
/*
 Returns
   -4 EOF
   -3 if no input present when required,
   -2 if out of space or other internal error,
   -1 if reparse needed,
    0 or 1, with xp pointing to name, if directory specified,
    2 if a wildcard was included.
*/
cmdir(xhlp,xdef,xp) char *xhlp, *xdef, **xp; {
    int i, x, xc; long y; char *sp;
#ifdef DTILDE
    char *tilde_expand(), *dirp;
#endif 

    cc = xc = 0;                        /* Initialize counts & pointers */
    *xp = "";
    if ((x = cmflgs) != 1) {            /* Already confirmed? */
        x = gtword();                   /* No, get a word */
    } else {
        cc = setatm(xdef);              /* If so, use default, if any. */
    }
    *xp = atmbuf;                       /* Point to result. */
 
    while (1) {
        xc += cc;                       /* Count the characters. */
        debug(F111,"cmifi: gtword",atmbuf,xc);
        switch (x) {
            case -4:                    /* EOF */
            case -2:                    /* Out of space. */
            case -1:                    /* Reparse needed */
                return(x);
            case 0:                     /* SP or NL */
            case 1:
                if (xc == 0) *xp = xdef;     /* If no input, return default. */
                else *xp = atmbuf;
                if (**xp == NUL) return(-3); /* If field empty, return -3. */
#ifdef DTILDE
		dirp = tilde_expand(*xp);    /* Expand tilde, if any, */
		if (*dirp != '\0') setatm(dirp); /* in the atom buffer. */
#endif
		if (chkwld(*xp) != 0)	/* If wildcard included... */
		  return(2);

                /* If not wild, see if it exists and is readable. */
 
                y = zchki(*xp);

                if (y == -3) {
                    printf("\n?Read permission denied - %s\n",*xp);
                    return(-2);
		} else if (y == -2) {	/* Probably a directory... */
		    return(x);
                } else if (y < 0) {
                    printf("\n?Not found - %s\n",*xp);
                    return(-2);
                }
                return(x);
            case 2:                     /* ESC */
		putchar(BEL);
		break;

            case 3:                     /* Question mark */
                if (*xhlp == NUL)
                    printf(" Directory name");
                else
                    printf(" %s",xhlp);
                printf("\n%s%s",cmprom,cmdbuf);
                break;
        }
    x = gtword();
    }
}
 
/*  C H K W L D  --  Check for wildcard characters '*' or '?'  */
 
chkwld(s) char *s; {
 
    for ( ; *s != '\0'; s++) {
#ifdef datageneral
        /* Valid DG wild cards are '-', '+', '#', or '*' */
        if ( (*s <= '-') && (*s >= '#') &&
            ((*s == '-') || (*s == '+') || (*s == '#') || (*s == '*')) )
#else
        if ((*s == '*') || (*s == '?'))
#endif
            return(1);
    }
    return(0);
}

 
/*  C M F L D  --  Parse an arbitrary field  */
/*
 Returns
   -3 if no input present when required,
   -2 if field too big for buffer,
   -1 if reparse needed,
    0 otherwise, xp pointing to string result.
*/
cmfld(xhlp,xdef,xp) char *xhlp, *xdef, **xp; {
    int x, xc;
 
    debug(F110,"cmfld: xdef",xdef,0);
    cc = xc = 0;                        /* Initialize counts & pointers */
    *xp = "";
    debug(F101,"cmfld: cmflgs","",cmflgs);
    if ((x = cmflgs) != 1) {            /* Already confirmed? */
        x = gtword();                   /* No, get a word */
    } else {
        cc = setatm(xdef);              /* If so, use default, if any. */
    }
    *xp = atmbuf;                       /* Point to result. */
 
    while (1) {
        xc += cc;                       /* Count the characters. */
        debug(F111,"cmfld: gtword",atmbuf,xc);
        debug(F101," x","",x);
        switch (x) {
            case -4:                    /* EOF */
            case -2:                    /* Out of space. */
            case -1:                    /* Reparse needed */
                return(x);
            case 0:                     /* SP or NL */
            case 1:
                if (xc == 0) 		/* If no input, return default. */
		  cc = setatm(xdef);
		*xp = atmbuf;
                if (**xp == NUL) x = -3; /* If field empty, return -3. */
                return(x);
            case 2:                     /* ESC */
                if (xc == 0) {
                    printf("%s ",xdef); /* If at beginning of field, */
                    addbuf(xdef);       /* supply default. */
                    cc = setatm(xdef);  /* Return as if whole field */
                    return(0);          /* typed, followed by space. */
                } else {
                    putchar(BEL);       /* Beep if already into field. */
                }                   
                break;
            case 3:                     /* Question mark */
                if (*xhlp == NUL)
                    printf(" Please complete this field");
                else
                    printf(" %s",xhlp);
                printf("\n%s%s",cmprom,cmdbuf);
                break;
        }
    x = gtword();
    }
}

 
/*  C M T X T  --  Get a text string, including confirmation  */
 
/*
  Print help message 'xhlp' if ? typed, supply default 'xdef' if null
  string typed.  Returns
 
   -1 if reparse needed or buffer overflows.
    1 otherwise.
 
  with cmflgs set to return code, and xp pointing to result string.
*/
 
cmtxt(xhlp,xdef,xp) char *xhlp; char *xdef; char **xp; {
 
    int x;
    static int xc;
 
    debug(F101,"cmtxt, cmflgs","",cmflgs);
    cc = 0;                             /* Start atmbuf counter off at 0 */
    if (cmflgs == -1) {                 /* If reparsing, */
        xc = strlen(*xp);               /* get back the total text length, */
    } else {                            /* otherwise, */
        *xp = "";                       /* start fresh. */
        xc = 0;
    }
    *atmbuf = NUL;                      /* And empty atom buffer. */
    if ((x = cmflgs) != 1) {
        x = gtword();                   /* Get first word. */
        *xp = pp;                       /* Save pointer to it. */
    }
    while (1) {
        xc += cc;                       /* Char count for all words. */
        debug(F111,"cmtxt: gtword",atmbuf,xc);
        debug(F101," x","",x);
        switch (x) {
            case -4:                    /* EOF */
            case -2:                    /* Overflow */
            case -1:                    /* Deletion */
                return(x);
            case 0:                     /* Space */
                xc++;                   /* Just count it */
                break;
            case 1:                     /* CR or LF */
                if (xc == 0) *xp = xdef;
                return(x);
            case 2:                     /* ESC */
                if (xc == 0) {
                    printf("%s ",xdef);
                    cc = addbuf(xdef);
                } else {
                    putchar(BEL);
                }
                break;
            case 3:                     /* Question Mark */
                if (*xhlp == NUL)
                    printf(" Text string");
                else
                    printf(" %s",xhlp);
                printf("\n%s%s",cmprom,cmdbuf);
                break;
            default:
                printf("\n?Unexpected return code from gtword() - %d\n",x);
                return(-2);
        }
        x = gtword();
    }
}

 
/*  C M K E Y  --  Parse a keyword  */
 
/*
 Call with:
   table    --  keyword table, in 'struct keytab' format;
   n        --  number of entries in table;
   xhlp     --  pointer to help string;
   xdef     --  pointer to default keyword;
 
 Returns:
   -3       --  no input supplied and no default available
   -2       --  input doesn't uniquely match a keyword in the table
   -1       --  user deleted too much, command reparse required
    n >= 0  --  value associated with keyword
*/
 
cmkey(table,n,xhlp,xdef) struct keytab table[]; int n; char *xhlp, *xdef; {
    int i, y, z, zz, xc;
    char *xp;
 
    xc = cc = 0;                        /* Clear character counters. */
 
    if ((zz = cmflgs) == 1)             /* Command already entered? */
        setatm(xdef);
    else zz = gtword(); 
 
debug(F101,"cmkey: table length","",n);
debug(F101," cmflgs","",cmflgs);
debug(F101," zz","",zz);
while (1) {
    xc += cc;
    debug(F111,"cmkey: gtword",atmbuf,xc);
 
    switch(zz) {
        case -4:                        /* EOF */
        case -2:                        /* Buffer overflow */
        case -1:                        /* Or user did some deleting. */
            return(zz);
 
        case 0:                         /* User terminated word with space */
        case 1:                         /* or newline */
            if (cc == 0) setatm(xdef);
            y = lookup(table,atmbuf,n,&z);
            switch (y) {
                case -2:
                    printf("\n?Ambiguous - %s\n",atmbuf);
                    return(cmflgs = -2);
                case -1:
                    printf("\n?Invalid - %s\n",atmbuf);
                    return(cmflgs = -2);
                default:
                    break;
            }
            return(y);
 
/* cont'd... */

 
/* ...cmkey(), cont'd */
 
        case 2:                         /* User terminated word with ESC */
            if (cc == 0) {
                if (*xdef != NUL) {     /* Nothing in atmbuf */
                    printf("%s ",xdef); /* Supply default if any */
                    addbuf(xdef);
                    cc = setatm(xdef);
                    debug(F111,"cmkey: default",atmbuf,cc);
                } else {
                    putchar(BEL);       /* No default, just beep */
                    break;
                }
            }
            y = lookup(table,atmbuf,n,&z); /* Something in atmbuf */
            debug(F111,"cmkey: esc",atmbuf,y);
            if (y == -2) {
                putchar(BEL);
                break;
            }
            if (y == -1) {
                printf("\n?Invalid - %s\n",atmbuf);
                return(cmflgs = -2);
            }
            xp = table[z].kwd + cc;
            printf("%s ",xp);
            addbuf(xp);
            debug(F110,"cmkey: addbuf",cmdbuf,0);
            return(y);
 
/* cont'd... */

 
/* ...cmkey(), cont'd */
 
        case 3:                         /* User terminated word with "?" */
            y = lookup(table,atmbuf,n,&z);
            if (y > -1) {
                printf(" %s\n%s%s",table[z].kwd,cmprom,cmdbuf);
                break;
            } else if (y == -1) {
                printf("\n?Invalid\n");
                return(cmflgs = -2);
            }
 
            if (*xhlp == NUL)
                printf(" One of the following:\n");
            else
                printf(" %s, one of the following:\n",xhlp);
 
            clrhlp();
            for (i = 0; i < n; i++) {   
                if (!strncmp(table[i].kwd,atmbuf,cc)
                        && !test(table[i].flgs,CM_INV))
                    addhlp(table[i].kwd);
            }
            dmphlp();
            printf("%s%s", cmprom, cmdbuf);
            break;
 
        default:            
            printf("\n%d - Unexpected return code from gtword\n",zz);
            return(cmflgs = -2);
        }
        zz = gtword();
    }
}

 
/*  C M C F M  --  Parse command confirmation (end of line)  */
 
/*
 Returns
   -2: User typed anything but whitespace or newline
   -1: Reparse needed
    0: Confirmation was received
*/
 
cmcfm() {
    int x, xc;
 
    debug(F101,"cmcfm: cmflgs","",cmflgs);
 
    xc = cc = 0;
    if (cmflgs == 1) return(0);
 
    while (1) {
        x = gtword();
        xc += cc;
        debug(F111,"cmcfm: gtword",atmbuf,xc);
        switch (x) {
            case -4:                    /* EOF */
            case -2:
            case -1:
                return(x);
 
            case 0:                     /* Space */
                continue;
            case 1:                     /* End of line */
                if (xc > 0) {
                    printf("?Not confirmed - %s\n",atmbuf);
                    return(-2);
                } else return(0);                   
            case 2:
                putchar(BEL);
                continue;
 
            case 3:
                if (xc > 0) {
                    printf("\n?Not confirmed - %s\n",atmbuf);
                    return(-2);
                }
                printf("\n Type a carriage return to confirm the command\n");
                printf("%s%s",cmprom,cmdbuf);
                continue;
        }
    }
}

 
/* Keyword help routines */
 
 
/*  C L R H L P -- Initialize/Clear the help line buffer  */
 
clrhlp() {                              /* Clear the help buffer */
    hlpbuf[0] = NUL;
    hh = hx = 0;
}
 
 
/*  A D D H L P  --  Add a string to the help line buffer  */
 
addhlp(s) char *s; {                    /* Add a word to the help buffer */
    int j;
 
    hh++;                               /* Count this column */
 
    for (j = 0; (j < hc) && (*s != NUL); j++) { /* Fill the column */
        hlpbuf[hx++] = *s++;
    }
    if (*s != NUL)                      /* Still some chars left in string? */
        hlpbuf[hx-1] = '+';             /* Mark as too long for column. */
 
    if (hh < (hw / hc)) {               /* Pad col with spaces if necessary */
        for (; j < hc; j++) {
            hlpbuf[hx++] = SP;
        }
    } else {                            /* If last column, */
        hlpbuf[hx++] = NUL;             /* no spaces. */
        dmphlp();                       /* Print it. */
        return;
    }
}
 
 
/*  D M P H L P  --  Dump the help line buffer  */
 
dmphlp() {                              /* Print the help buffer */
    hlpbuf[hx++] = NUL;
    printf(" %s\n",hlpbuf);
    clrhlp();
}

 
/*  L O O K U P  --  Lookup the string in the given array of strings  */
 
/*
 Call this way:  v = lookup(table,word,n,&x);
 
   table - a 'struct keytab' table.
   word  - the target string to look up in the table.
   n     - the number of elements in the table.
   x     - address of an integer for returning the table array index.
 
 The keyword table must be arranged in ascending alphabetical order, and
 all letters must be lowercase.
 
 Returns the keyword's associated value ( zero or greater ) if found,
 with the variable x set to the array index, or:
 
  -3 if nothing to look up (target was null),
  -2 if ambiguous,
  -1 if not found.
 
 A match is successful if the target matches a keyword exactly, or if
 the target is a prefix of exactly one keyword.  It is ambiguous if the
 target matches two or more keywords from the table.
*/
 
lookup(table,cmd,n,x) char *cmd; struct keytab table[]; int n, *x; {
 
    int i, v, cmdlen;
 
/* Lowercase & get length of target, if it's null return code -3. */
 
    if ((((cmdlen = lower(cmd))) == 0) || (n < 1)) return(-3);
 
/* Not null, look it up */
 
    for (i = 0; i < n-1; i++) {
        if (!strcmp(table[i].kwd,cmd) ||
           ((v = !strncmp(table[i].kwd,cmd,cmdlen)) &&
             strncmp(table[i+1].kwd,cmd,cmdlen))) {
                *x = i;
                return(table[i].val);
             }
        if (v) return(-2);
    }   
 
/* Last (or only) element */
 
    if (!strncmp(table[n-1].kwd,cmd,cmdlen)) {
        *x = n-1;
        return(table[n-1].val);
    } else return(-1);
}

 
/*  G E T W D  --  Gets a "word" from the command input stream  */
 
/*
Usage: retcode = gtword();
 
Returns:
 -4 if end of file (e.g. pipe broken)
 -2 if command buffer overflows
 -1 if user did some deleting
  0 if word terminates with SP or tab
  1 if ... CR
  2 if ... ESC
  3 if ... ?
 
With:
  pp pointing to beginning of word in buffer
  bp pointing to after current position
  atmbuf containing a copy of the word
  cc containing the number of characters in the word copied to atmbuf
*/
gtword() {
 
    int c;                              /* Current char */
    static int inword = 0;              /* Flag for start of word found */
    int quote = 0;                      /* Flag for quote character */
    int echof = 0;                      /* Flag for whether to echo */
    int ignore;

#ifdef RTU
    extern int rtu_bug;
#endif

#ifdef datageneral
    extern int termtype;                /* DG terminal type flag */
    extern int con_reads_mt;            /* Console read asynch is active */
    if (con_reads_mt) connoi_mt();      /* Task would interfere w/cons read */
#endif 
 
    pp = np;                            /* Start of current field */
    debug(F101,"gtword: cmdbuf","",(int) cmdbuf);
    debug(F101," bp","",(int) bp);
    debug(F101," pp","",(int) pp);
    debug(F110," cmdbuf",cmdbuf,0);
 
    while (bp < cmdbuf+CMDBL) {         /* Loop */
 
        ignore = echof = 0;             /* Flag for whether to echo */
 
        if ((c = *bp) == NUL) {         /* Get next character */
            if (dpx) echof = 1;         /* from reparse buffer */
#ifdef datageneral
            {
               char ch;
               c = dgncinb(0,&ch,1);    /* -1 is EOF, -2 TO, 
                                         * -c is AOS/VS error */
               if (c == -2) {           /* timeout was enabled? */
                    resto(channel(0));  /* reset timeouts */
                    c = dgncinb(0,&ch,1); /* retry this now! */
               }
               if (c < 0) return(-4);    /* EOF or some error */
               else c = (int) ch & 0177; /* Get char without parity */
               echof = 1;
            }
#else
#ifdef OS2
	    c = isatty(0) ? coninc(0) : getchar();
	    if (c<0) return(-4);
#else
            c = getchar();              /* or from tty. */
#ifdef RTU
	    if (rtu_bug) {
		c = getchar();          /* RTU doesn't discard the ^Z */
		rtu_bug = 0;
	    }
#endif /* RTU */

            if (c == EOF) {
/***		perror("ckucmd getchar");  (just return silently) ***/
		return(-4);
	    }
	    c &= 127;			/* Strip any parity bit. */
#endif
#endif
        } else ignore = 1;
 
        if (quote == 0) {
 
            if (!ignore && (c == '\\')) { /* Quote character */
               quote = 1;
               continue;
            }
            if (c == FF) {              /* Formfeed. */
                c = NL;                 /* Replace with newline */
#ifdef aegis
                putchar(FF);
#else
#ifdef AMIGA
                putchar(FF);
#else
#ifdef OSK
                putchar(FF);
#else
#ifdef datageneral
                putchar(FF);
#else
#ifdef OS2
		{ char cell[2];
		cell[0] = ' ';
		cell[1] = 7;
		VioScrollUp(0,0,-1,-1,-1,cell,0);
		VioSetCurPos(0,0,0);
		}
#else
                system("clear");        /* and clear the screen. */
#endif
#endif
#endif
#endif
#endif
            }
 
            if (c == HT) c = ESC;        /* Substitute ESC for tab. */
 
/* cont'd... */

 
/* ...gtword(), cont'd */
 
            if (c == SP) {              /* If space */
                *bp++ = c;              /* deposit it in buffer. */
                if (echof) putchar(c);  /* echo it. */
                if (inword == 0) {      /* If leading, gobble it. */
                    pp++;
                    continue;
                } else {                /* If terminating, return. */
                    np = bp;
                    setatm(pp);
                    inword = 0;
                    return(cmflgs = 0);
                }
            }
            if (c == NL || c == CR) {   /* CR, LF */
                *bp = NUL;              /* End the string */
                if (echof) {            /* If echoing, */
                    putchar(c);         /* echo the typein */
#ifdef OS2
                    if (c == CR) putchar(NL);
#endif
#ifdef aegis
                    if (c == CR) putchar(NL);
#endif
#ifdef AMIGA
                    if (c == CR) putchar(NL);
#endif
#ifdef datageneral
                    if (c == CR) putchar(NL);
#endif
                }
                np = bp;                /* Where to start next field. */
                setatm(pp);             /* Copy this field to atom buffer. */
                inword = 0;
                return(cmflgs = 1);
            }
            if (!ignore && (c == '?')) { /* Question mark */
                putchar(c);
                *bp = NUL;
                setatm(pp);
                return(cmflgs = 3);
            }
            if (c == ESC) {             /* ESC */
                *bp = NUL;
                setatm(pp);
                return(cmflgs = 2);
            }
            if (c == BS || c == RUB) {  /* Character deletion */
                if (bp > cmdbuf) {      /* If still in buffer... */
#ifdef datageneral
                    /* DG '\b' is EM (^y or \031) */
                    if (termtype == 1)
                         /* Erase a character from non-DG screen, */
                         dgncoub(1,"\010 \010",3);
                    else
#endif
                    printf("\b \b");    /* erase character from screen, */
                    bp--;               /* point behind it, */
                    if (*bp == SP) inword = 0; /* Flag if current field gone */
                    *bp = NUL;          /* Erase character from buffer. */
                } else {                /* Otherwise, */
                    putchar(BEL);       /* beep, */
                    cmres();            /* and start parsing a new command. */
                }
                if (pp < bp) continue;
                else return(cmflgs = -1);
            }
            if (c == LDEL) {            /* ^U, line deletion */
#ifdef datageneral
                /* DG '\b' is EM (^y or \031) */
                if (termtype == 1)
                    /* Erase a character from a non-DG screen, */
                    while ((bp--) > cmdbuf) {
                         dgncoub(1,"\010 \010",3);
                         *bp = NUL;
                    }
                else
#endif /* datageneral */
                while ((bp--) > cmdbuf) {
                    printf("\b \b");
                    *bp = NUL;
                }
                cmres();                /* Restart the command. */
                inword = 0;
                return(cmflgs = -1);
            }
 
/* cont'd... */

 
/* ...gtword(), cont'd */
 
            if (c == WDEL) {            /* ^W, word deletion */
                if (bp <= cmdbuf) {     /* Beep if nothing to delete */
                    putchar(BEL);
                    cmres();
                    return(cmflgs = -1);
                }
                bp--;
#ifdef datageneral
                /* DG '\b' is EM (^y or \031) */
                if (termtype == 1) {
                    /* Erase a character from a non-DG screen, */
                    for ( ; (bp >= cmdbuf) && (*bp == SP) ; bp--) {
                         dgncoub(1,"\010 \010",3);
                         *bp = NUL;
                    }
                    for ( ; (bp >= cmdbuf) && (*bp != SP) ; bp--) {
                         dgncoub(1,"\010 \010",3);
                         *bp = NUL;
                    }
                }
                else {
#endif /* datageneral */
                for ( ; (bp >= cmdbuf) && (*bp == SP) ; bp--) {
                    printf("\b \b");
                    *bp = NUL;
                }
                for ( ; (bp >= cmdbuf) && (*bp != SP) ; bp--) {
                    printf("\b \b");
                    *bp = NUL;
                }
#ifdef datageneral
                }   /* Termtype == 1 */
#endif
                bp++;
                inword = 0;
                return(cmflgs = -1);
            }
            if (c == RDIS) {            /* ^R, redisplay */
                *bp = NUL;
                printf("\n%s%s",cmprom,cmdbuf);
                continue;
            }
        }
#ifdef OS2
        if (echof) {
        	putchar(c);          /* If tty input, echo. */
        	if (quote==1 && c==CR) putchar(NL);
        }
#else
        if (echof) putchar(c);          /* If tty input, echo. */
#endif
        inword = 1;                     /* Flag we're in a word. */
        if (quote == 0 || c != NL) *bp++ = c;   /* And deposit it. */
        quote = 0;                      /* Turn off quote. */
    }                                   /* end of big while */
    putchar(BEL);                       /* Get here if... */
    printf("\n?Buffer full\n");
    return(cmflgs = -2);
}

 
/* Utility functions */
 
/* A D D B U F  -- Add the string pointed to by cp to the command buffer  */
 
addbuf(cp) char *cp; {
    int len = 0;
    while ((*cp != NUL) && (bp < cmdbuf+CMDBL)) {
        *bp++ = *cp++;                  /* Copy and */
        len++;                          /* count the characters. */
    }   
    *bp++ = SP;                         /* Put a space at the end */
    *bp = NUL;                          /* Terminate with a null */
    np = bp;                            /* Update the next-field pointer */
    return(len);                        /* Return the length */
}
 
/*  S E T A T M  --  Deposit a token in the atom buffer.  */
/*  Break on space, newline, carriage return, or null. */
/*  Null-terminate the result. */
/*  If the source pointer is the atom buffer itself, do nothing. */
/*  Return length of token, and also set global "cc" to this length. */
 
setatm(cp) char *cp; {
    char *ap;
    cc = 0;
    ap = atmbuf;
    if (cp == ap) return(cc = strlen(ap));
    *ap = NUL;
    while (*cp == SP) cp++;
    while ((*cp != SP) && (*cp != NL) && (*cp != NUL) && (*cp != CR)) {
        *ap++ = *cp++;
        cc++;
    }
    *ap++ = NUL;
    return(cc);                         /* Return length */
}
 
/*  R D I G I T S  -- Verify that all the characters in line ARE DIGITS  */
 
rdigits(s) char *s; {
    while (*s) {
        if (!isdigit(*s)) return(0);
        s++;
    }
    return(1);
}
 
/*  L O W E R  --  Lowercase a string  */
 
lower(s) char *s; {
    int n = 0;
    while (*s) {
        if (isupper(*s)) *s = tolower(*s);
        s++, n++;
    }
    return(n);
}
 
/*  T E S T  --  Bit test  */
 
test(x,m) int x, m; { /*  Returns 1 if any bits from m are on in x, else 0  */
    return((x & m) ? 1 : 0);
}
