###########################################################################
#                                                                         #
#                   Makefile, version 2.79, 2021-APR-29                   #
#                                                                         #
###########################################################################
#                                                                         #
# Makefile to build uCKermit (microkermit) for UNIX and UNIX-like systems #
#                                                                         #
###########################################################################
#                                                                         #
# for Alliant FX/8 with Concentrix 4.1, "make bsdlck"                     #
# for Amdahl UTSV IBM 370 series & compatible mainframes, "make sys3"     #
# for Apollo DOMAIN/IX, "make bsd" or "make sys3", for desired universe   #
# for Apollo with SR10.0 BSD or later, "make sr10-bsd"                    #
# for Apple Macintosh II with A/UX, "make aux"                            #
# for AT&T 3B2, 3B20 systems, "make att3bx"                               #
# for AT&T generic System III/System V, "make sys3" or "make sys3nid"     #
# for AT&T System V R3, use "make sys5r3". This is different from above.  #
# for Bell UNIX Version 7 (aka 7th Edition), "make v7" (but see below)    #
# for Berkeley UNIX 4.1, "make bsd41"                                     #
# for Berkeley UNIX 4.2, "make bsd" (tested with 4.2 and 4.3)             #
# for Berkeley UNIX 4.3, "make bsd43" (uses acucntrl program)             #
# for Berkeley UNIX 4.3 w/o acucntrl program, "make bsdlck" or "make bsd" #
# for Berkeley UNIX 4.2 or 4.3 with HoneyDanBer UUCP, "make bsdhdb"       #
# for Berkeley UNIX 2.9 (DEC PDP-11 or Pro-3xx), "make bsd29"             #
# for Berkeley UNIX 2.10, "make bsd210"                                   #
# for Charles River Data Systems 680x0 systems with Unos, "make sys3nid"  #
# for Convex C1, "make convex"                                            #
# for DEC Ultrix on VAX or DECstation, "make bsd"                         #
# for DEC Pro-350 with Pro/Venix V2 (SysV), "make sys3nid"                #
# for DEC Pro-380 with Pro/Venix V2 (SysV), "make sys3" or "make sys3nid" #
# for Encore Multimax 310, 510 with UMAX 4.2, "make bsd"                  #
# for Encore Multimax 310, 510 with UMAX 2.2, use Berkeley cc, "make bsd" #
# for HP-9000 Series with HP-UX, "make hpux"                              #
# for IBM PS/2 with PS/2 AIX, "make ps2aix"                               #
# for IBM RT PC with AIX 2.1, "make sys3"                                 #
# for IBM RT PC with ACIS, "make bsd"                                     #
# for Intel Xenix, "make sco286"                                          #
# for Interactive Sys III on other systems, "make is3"                    #
# for Linux (modern, with GCC or clang), "make linux"                     #
# for Linux (modern, with GCC or clang), tiny build, "make linux-small"   #
# for Microport System V, "make mpsysv"                                   #
# for Microsoft/IBM Xenix (286, AT, etc.), "make xenix" or "make sco286"  #
# for NeXTSTEP or OPENSTEP, "make next"                                   #
# for SCO Xenix 2.2.1 with development system 2.2 on 8086/8 "make sco86"  #
# for SCO Xenix/286 2.2.1 w/ development system 2.2 on 286, "make sco286" #
# for SCO Xenix/386 2.2.2, "make sco386"                                  #
# for Sequent Balance 8000 or B8 with DYNIX 3.0.14, "make bsdlck"         #
# for Sequent Symmetry S/81 running DYNIX 3.0.12, "make bsdlck"           #
# for Sun with SunOS 4.0 or later, "make sunos4" (uses BSD environment)   #
# for Sun with SunOS 4.0 or later, "make sys5r3" (uses AT&T environment)  #
#                                                                         # 
###########################################################################
#                                                                         #
# The result should be a runnable program called "wermit" in the current  #
# directory.  After satisfactory testing, rename "wermit" to "uckermit"   #
# and put it where users can access it.                                   #
#                                                                         #
###########################################################################
#                                                                         #
# To remove compiled output, intermediate, and object files, "make clean" #
#                                                                         #
###########################################################################
#                                                                         #
#  In many cases, the -O (optimize) compiler switch is omitted. Feel      #
#  free to add it if you trust the optimizer. The ckuus2.c module, in     #
#  particular, tends to make optimizers blow up.                          #
#                                                                         #
###########################################################################
#                                                                         #
#  "make bsd" should make a working uCKermit for 4.1, 4.2, & 4.3bsd on    #
#  VAX, SUN-3, SUN-4, Pyramid, and other 4.x systems and also VAX/Ultrix  #
#                                                                         #
###########################################################################
#                                                                         #
#  Either "make sys3" or "make sys3nid" tends to make a working version   #
#  on any ATT System III or System V R2 or earlier system, including      #
#  Motorola, Four Phase, Callan, Unistar, Cadmus, NCR Tower, HP9836       #
#  Series 200, Plexus, Heurikon, etc etc (for exceptions, see below;      #
#  some AT&T 3Bx systems have their own entry). As far as uCKermit goes,  #
#  there is no functional difference between ATT System III and System V  #
#  R2, so there is no need for a separate "make sys5" entry (but there    #
#  is one anyway; it merely invokes "make sys3"). But for ATT System V    #
#  R3, use "make sys5r3". This is different from the above because of     #
#  the redefinition of signal().                                          #
#                                                                         #
###########################################################################
#                                                                         #
#  "make sys3nid" is equivalent to "make sys3" but leaves out the -i      #
#  option, which is used indicate that separate instruction and data      #
#  (text) spaces are to be used, as on a PDP-11. Some systems don't       #
#  support this option, others may require it. If one of these options    #
#  doesn't work on your System III or System V system, try the other.     #
#                                                                         #
###########################################################################
#                                                                         #
#  For Xenix...  What's Xenix?  There are so many different products and  #
#  versions sold under this name, the name "xenix" is almost meaningless. #
#  IBM, SCO, Microsoft, etc, IBM Xenix 1.0 =(?) Microsoft Xenix 3.0 = ??? #
#  Nevertheless, try "make xenix" for IBM or Microsoft, or "make sco286"  #
#  or "make sco86" for for SCO Xenix. If these don't work, try the        #
#  following modifications to this Makefile:                              #
#                                                                         #
#    Change "CC= cc" to "CC = cc -LARGE"                                  #
#    In the "xenix:" make entry, add "-M2m -UM_I86" to the compiler       #
#    switches (for the IBM PC family), and "-Mm -lx" to the linker.       #
#                                                                         #
###########################################################################
#                                                                         #
#  For UNIX Version 7, several variables must be defined to the values    #
#  associated with your system. BOOTNAME=/edition7 is the kernel image on #
#  okstate's Perkin-Elmer 3230. Others will probably be /unix.            #
#  PROCNAME=proc is the name of the structure assigned to each process on #
#  okstate's system. This may be "_proc" or some other variation.         #
#  See <sys/proc.h> for more info on your systems name conventions.       #
#  NPROCNAME=nproc is the name of a Kernal variable that tells how many   #
#  "proc" structures there are. Again this may be different on your       #
#  system, but nproc will probably be somewhere. The variable NPTYPE is   #
#  the type of the nproc variable -- int, short, etc. which can probably  #
#  be gleaned from <sys/param.h>. The definition of DIRECT is a little    #
#  more complicated.  If nlist() returns, for "proc" only, the address of #
#  the array, then you should define DIRECT as it is below. If however,   #
#  nlist() returns the address of a pointer to the array, then you should #
#  give DIRECT a null definition (DIRECT= ).  The extern declaration      #
#  in <sys/proc.h> should clarify this for you.  If it is "extern struct  #
#  proc *proc", then you should NOT define DIRECT.  If it is "extern      #
#  struct proc proc[]", then you should probably define DIRECT as it is   #
#  below.  See ckuv7.hlp for further information.                         #
#                                                                         #
###########################################################################
#                                                                         #
#  For 2.9bsd, the Makefile may use pcc rather than cc for compiles;      #
#  that's what the CC and CC2 definitions are for (the current version    #
#  of the Makefile uses cc for both; this was tested and seems to work on #
#  the DEC Pro 380).  2.9 support basically follows the 4.1 path.         #
#  Some 2.9 systems use "dir.h" for the directory header file, others     #
#  will need to change this to "ndir.h".                                  #
#                                                                         #
###########################################################################
#                                                                         #
#  The v7 and 2.9bsd versions assume I&D space on a PDP-11. When building #
#  uCKermit for v7 on a PDP-11, you should probably add the -i option to  #
#  the link flags.  Without I&D space, overlays would probably have to be #
#  used (or code mapping a`la Pro/Venix if that's available).             #
#                                                                         #
###########################################################################
#                                                                         #
#  Other systems require some special treatment:                          #
#                                                                         # 
#  For Ridge32 (ROS3.2), use "make sys3", but                             #
#                                                                         #
#  1. Use "CFLAGS = -DUXIII -i -O" "LNKFLAGS = -i"                        #
#                                                                         #
#  2. Don't #include <sys/file.h> in cku[tf]io.c.                         #
#                                                                         #
###########################################################################
#                                                                         #
#  For Altos 986 with Xenix 3.0, use "make sys3", but                     #
#                                                                         #
#  1. Get rid of any "(void)"'s (they're only there for Lint anyway)      #
#                                                                         #
#  2. In ckcdeb.h, define CHAR to be "char" rather than "unsigned char".  #
#                                                                         #
###########################################################################
#                                                                         #
#  For Tandy 6000 running Sys III based Xenix (Xenix 3.xxx), use "make    #
#  sys3" but insert "#include <sys/types.h>" in any file that that uses   #
#  the "void" data type.                                                  #
#                                                                         #
###########################################################################
#                                                                         #
#  Other systems that are close to, but not quite, like Sys III or V, or  #
#  4.x BSD or V7 -- look at some of the tricks used below and see if you  #
#  can find a combination that works for you.                             #
#                                                                         #
###########################################################################
#                                                                         #
# V7-specific variables.  These are set up for Perkin-Elmer 3230 V7 UNIX: #
#                                                                         #
###########################################################################
#
PROC=proc
DIRECT=
NPROC=nproc
NPTYPE=int
BOOTFILE=/edition7
#
###########################################################################
#                                                                         #
#  For old Tandy TRS-80 Model 16A or 6000 V7-based Xenix, use PROC=_proc, #
#  DIRECT=-DDIRECT, NPROC=_Nproc, NPTYPE=short, BOOTFILE=/xenix           #
#                                                                         #
###########################################################################
#                                                                         #
#                       Compile and Link variables:                       #
#                                                                         #
###########################################################################
#
LNKFLAGS=
SHAREDLIB=
CC?= cc
CC2?= $(CC)
#
###########################################################################
#                                                                         #
#                          Dependencies Section:                          #
#                                                                         #
###########################################################################
#
make:
	@printf '%s\n' 'Error: You must specify a target to build uCKermit.'
	@false
#
###########################################################################
#
wermit: ckcmai.o ckucmd.o ckuusr.o ckuus2.o ckuus3.o ckcpro.o ckcfns.o \
		ckcfn2.o ckucon.o ckutio.o ckufio.o ckudia.o ckuscr.o
	$(CC2) $(LNKFLAGS) -o wermit \
		ckcmai.o ckutio.o ckufio.o ckcfns.o ckcfn2.o ckcpro.o ckucmd.o \
		ckuus2.o ckuus3.o ckuusr.o ckucon.o ckudia.o ckuscr.o \
		$(LIBS)
#
###########################################################################
#
ckcmai.o: ckcmai.c ckcker.h ckcdeb.h
#
###########################################################################
#
ckuusr.o: ckcpro.c ckuusr.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h
#
###########################################################################
#
ckuus2.o: ckuus2.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h
#
###########################################################################
#
ckuus3.o: ckuus3.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h
#
###########################################################################
#
ckucmd.o: ckucmd.c ckucmd.h ckcdeb.h
#
###########################################################################
#
ckcpro.o: ckcpro.c ckcker.h ckcdeb.h
#
###########################################################################
#
ckcpro.c: ckcpro.w wart
	./wart ckcpro.w ckcpro.c
#
###########################################################################
#
ckcfns.o: ckcfns.c ckcker.h ckcdeb.h
#
###########################################################################
#
ckcfn2.o: ckcfn2.c ckcker.h ckcdeb.h
#
###########################################################################
#
ckufio.o: ckufio.c ckcker.h ckcdeb.h
#
###########################################################################
#
ckutio.o: ckutio.c ckcdeb.h
#
###########################################################################
#
ckucon.o: ckucon.c ckcker.h ckcdeb.h
#
###########################################################################
#
wart: ckwart.o
	$(CC) $(LNKFLAGS) -o wart ckwart.o
#
###########################################################################
#
ckwart.o: ckwart.c
#
###########################################################################
#
ckudia.o: ckudia.c ckcker.h ckcdeb.h ckucmd.h
#
###########################################################################
#
ckuscr.o: ckuscr.c ckcker.h ckcdeb.h
#
###########################################################################
#                                                                         #
#                    Make commands for specific systems:                  #
#                                                                         #
###########################################################################
#Apple Mac II, A/UX
aux:
	make wermit "CFLAGS = -DAUX -DUXIII -DDEBUG -DTLOG -i -O" \
		"LNKFLAGS = -i"
#
###########################################################################
#Berkeley UNIX 4.1
bsd41:
	make wermit "CFLAGS= -DBSD4 -DBSD41 -DDEBUG -DTLOG" "LIBS = -ljobs"
#
###########################################################################
#Berkeley 4.2, 4.3, also Ultrix-32 1.x, 2.0, maybe 3.0, many others
bsd:
	make wermit "CFLAGS= -DBSD4 -DDEBUG -DTLOG"
#
###########################################################################
#Berkeley UNIX 4.2 or 4.3 with lock directory
#/usr/spool/uucp/LCK/LCK..tty??,
#but without acucntrl program
bsdlck:
	make wermit "CFLAGS= -DLCKDIR -DBSD4 -DDEBUG -DTLOG"
#
###########################################################################
#Berkeley UNIX with HoneyDanBer (HDB) UUCP
bsdhdb:
	make wermit "CFLAGS= -DHDBUUCP -DBSD4 -DDEBUG -DTLOG"
#
###########################################################################
#Berkeley UNIX 4.3 with acucntrl program
bsd43:
	make wermit "CFLAGS= -DBSD43 -DNEWUUCP -DBSD4 -DDEBUG -DTLOG -O"
#
###########################################################################
#Berkeley UNIX 2.8, 2.9 for PDP-11s with I&D space, maybe also Ultrix-11???
#If you have trouble with this, try removing "-l ndir".  If you still have
#trouble, remove "-DDEBUG -DTLOG".  Or try defining CC and/or CC2 as "pcc"
#instead of "cc".
bsd29:
	make wermit "CFLAGS= -DBSD29 -DDEBUG -DTLOG" \
		"LNKFLAGS= -i -lndir" "CC= cc " "CC2= cc"
#
###########################################################################
#Berkeley UNIX 2.10 (Stan Barber, sob@bcm.tmc.edu)
bsd210:
	make wermit "CFLAGS= -DBSD29 -DDEBUG -DTLOG" -DLCKDIR \
		"LNKFLAGS= -i " "CC= cc " "CC2= cc"
#
###########################################################################
#Convex C1 with Berkeley UNIX
convex:
	make wermit "CFLAGS= -DBSD4 -Dmsleep=mnap -DDEBUG -DTLOG"
#
###########################################################################
#NeXT
next:
	make wermit "CFLAGS= -fwritable-strings -DLCKDIR -DBSD4 -DDEBUG -DTLOG"
#
###########################################################################
#SUN OS version 4.0 or later (like Berkeley, but different signal() type)
sunos4:
	make wermit "CFLAGS= -DBSD4 -DSUNOS4 -DDEBUG -DTLOG"
#
###########################################################################
#Apollo with SR10.0 or later, BSD universe
sr10-bsd:
	make wermit "CFLAGS= -DBSD4 -DDEBUG -DTLOG -Uaegis"
#
###########################################################################
#Version 7 UNIX (see comments above)
v7:
	make wermit "CFLAGS=-DV7 -DDEBUG -DTLOG -DPROCNAME=\\\"$(PROC)\\\" \
		-DBOOTNAME=\\\"$(BOOTFILE)\\\" -DNPROCNAME=\\\"$(NPROC)\\\" \
		-DNPTYPE=$(NPTYPE) $(DIRECT)"
#
###########################################################################
#Modern Linux, GCC 10 (development)
linux:
	make wermit "CFLAGS = -DSYSVR3 -DUXIII -DBSD42 -DDEBUG -DTLOG -DUXIII \
		-DO_NDELAY -DTIOCFLUSH -DTIOCSINUSE -Wall -DSIGTSTP -DFIONREAD \
		-DTIMEZONE -Wall -Wno-return-type -Wno-unused-variable -Os \
		-Wno-implicit-int -fno-unwind-tables -fno-exceptions -DDIRENT \
		-Wno-implicit-function-declaration -Wno-implicit-fallthrough \
		-Wno-missing-braces -fno-math-errno -fdata-sections -DNOBTEST \
		-fno-asynchronous-unwind-tables -funsigned-char -ffast-math \
		-ffunction-sections -fmerge-all-constants -flto -ffast-math \
		-fdelete-null-pointer-checks -funsafe-math-optimizations -g" \
			"LNKFLAGS = -flto \
				-Wl,--gc-sections \
				-Wl,--print-gc-sections \
				-Wl,-z,relro,-z,now"
#
###########################################################################
#Linux WIP size-reduction, GCC 10 (development)
linux-small:
	make wermit "CFLAGS = -DSYSVR3 -DUXIII -DBSD42 -DUXIII -Wall -Os \
		-DO_NDELAY -DTIOCFLUSH -DFIONREAD -DDIRENT -DNOCKUSCR -DSIGTSTP \
		-DNOCKUDIA -Wno-return-type -Wno-missing-braces -DNODOHLP \
		-Wno-implicit-function-declaration -Wno-implicit-fallthrough \
		-fno-asynchronous-unwind-tables -fno-unwind-tables -fno-ident \
		-fno-exceptions -fdata-sections -ffunction-sections -ffast-math \
		-fno-math-errno -Wno-implicit-int -DNOSTATS -fmerge-all-constants \
		-funsigned-char -fdelete-null-pointer-checks -DNOBTEST -DNOICP \
		-DMINBUF -DNOATTR -funsafe-math-optimizations -g -flto -DNODISP" \
			"LNKFLAGS = -flto \
				-Wl,--gc-sections \
				-Wl,--print-gc-sections \
				-Wl,-z,relro,-z,now"
#
###########################################################################
#Linux WIP size-reduction target (alternate target name)
linux-tiny:
	make linux-small
#
###########################################################################
# strip binary aggressively (development)
strip:
	@test -f wermit
	@printf '\n%*s\n' "$${COLUMNS:-$$(tput cols)}" '' | tr ' ' - \
		2>/dev/null || true
	@printf '\n%s ' "File size: " "$$(du --apparent-size \
			-k wermit 2>/dev/null | \
		cut -f 1 -d "	" 2>/dev/null | \
		cut -f 1 -d " " 2>/dev/null || true)"K \
		"$$(/bin/ls -l wermit 2>/dev/null | \
		awk '{ print $$5 }' 2>/dev/null || true) bytes" || true
	@printf '\n%s\n' "" 2>/dev/null || true
	@cp -f wermit .wermit.old.1
	@~/src/bloaty/build/bloaty --domain=vm -n 0 -w -s vm \
		-d sections,symbols wermit \
		2>/dev/null || true
	@printf '\n%s\n\n' \
		"Change since $$(stat -c %y .wermit.old.1.saved)" \
		2>/dev/null || true
	@~/src/bloaty/build/bloaty --domain=vm -n 0 -w -s vm \
		-d sections,symbols wermit \
		-- .wermit.old.1.saved \
		2>/dev/null || true
	@strip wermit \
		2>/dev/null || true
	@strip -s wermit \
		2>/dev/null || true
	@strip --strip-all wermit \
		2>/dev/null || true
	@strip --strip-dwo \
		-R .note.ABI-tag \
		-R .comment \
		-R .note \
		-R .tm_clone_table \
		-R .got \
		-R .shstrtab \
		-R .note.gnu.build-id \
		-R .gnu.version \
		-R .gnu.hash \
		-R .gnu.build.attributes \
		-R .data.rel.ro \
		-R .rel.eh_frame \
		-R .eh_frame_hdr \
		-R .eh_frame \
		-R .rela.eh_frame \
		-R .note.gnu.gold-version \
		-R .jcr \
		-R .got.plt \
		wermit \
		2>/dev/null || true
	@sstrip -z wermit 2>/dev/null || true
	@printf '%s\n' "" 2>/dev/null || true
	@printf '%*s\n' "$${COLUMNS:-$$(tput cols)}" '' | tr ' ' - \
		2>/dev/null || true
	@printf '\n%s ' "Stripped Size: " "$$(du --apparent-size \
			-k wermit 2>/dev/null | \
		cut -f 1 -d "	" 2>/dev/null | \
		cut -f 1 -d " " 2>/dev/null || true)K" \
		"$$(/bin/ls -l wermit 2>/dev/null | \
		awk '{ print $$5 }' 2>/dev/null || true) bytes" || true
	@printf '\n%s\n' 2>/dev/null || true
	@cp -f wermit .wermit.old.2
	@~/src/bloaty/build/bloaty --domain=vm -n 0 -w -s vm \
		-d sections,symbols wermit \
		2>/dev/null || true
	@printf '\n%s\n\n' \
		"Change since $$(stat -c %y .wermit.old.2.saved)" \
		2>/dev/null || true
	@~/src/bloaty/build/bloaty --domain=vm -n 0 -w -s vm \
        -d sections,symbols wermit \
        -- .wermit.old.2.saved \
        2>/dev/null || true
	@printf '\n%*s\n\n' "$${COLUMNS:-$$(tput cols)}" '' | tr ' ' - \
		2>/dev/null || true
#
###########################################################################
#System V R3, some things changed since System V R2...
sys5r3:
	make wermit "CFLAGS = -DSVR3 -DUXIII -DDEBUG -DTLOG -i -O" \
		"LNKFLAGS = -i"
#
###########################################################################
#In case of "make sys5"...
sys5:
	make sys3
#
###########################################################################
#Generic ATT System III or System V (with I&D space)
sys3:
	make wermit "CFLAGS = -DUXIII -DDEBUG -DTLOG -i -O" "LNKFLAGS = -i"
#
###########################################################################
#Generic ATT System III or System V (no I&D space)
sys3nid:
	make wermit "CFLAGS = -DUXIII -DDEBUG -DTLOG -O" "LNKFLAGS ="
#
###########################################################################
#AT&T 3B2, 3B20-series computers running System V
#  Only difference from sys3 is lock file stuff...
att3bx:
	make wermit "CFLAGS = -DUXIII -DATT3BX -DDEBUG -DTLOG -i -O" \
		"LNKFLAGS = -i"
#
###########################################################################
#IBM PS/2 with AIX 1.0 (currently in field test as F10A)
#  Reports indicate that -O switch must be omitted.
#  It is also possible that "made bsd" will work (reports welcome).
ps2aix:
	make wermit "CFLAGS = -DUXIII -DDEBUG -DTLOG -i" \
		"LNKFLAGS = -i"
#
###########################################################################
#HP 9000 series 300, 500, 800.
hpux:
	make wermit "CFLAGS = -DUXIII -DHPUX -DDEBUG -DTLOG -O" "LNKFLAGS ="
#
###########################################################################
#Microport System V for IBM PC/AT and clones
mpsysv:
	make wermit "CFLAGS= -O -DXENIX -Dunix -DUXIII -DTLOG -Ml -i" \
		"LNKFLAGS = -Ml -i"
#
###########################################################################
#Microsoft "Xenix/286" e.g. for IBM PC/AT
xenix:
	make wermit "CFLAGS= -DXENIX -Dunix -DUXIII -DDEBUG -DTLOG \
	-F 3000 -i" \
		"LNKFLAGS = -F 3000 -i"
#
###########################################################################
#SCO Xenix/286 2.2.1, e.g. for IBM PC/AT, PS/2 Model 50, etc.
sco286:
	make wermit "CFLAGS= -DXENIX  -Dunix -DUXIII -DDEBUG -DTLOG \
	-F 3000 -i -M2le" \
		"LNKFLAGS = -F 3000 -i -M2le"
#
###########################################################################
#SCO Xenix 2.2.1 for IBM PC, XT, PS2/30, or other 8088 or 8086 machine
sco86:
	make wermit "CFLAGS= -DXENIX  -Dunix -DUXIII -DDEBUG -DTLOG \
	-F 3000 -i -M0me" \
		"LNKFLAGS = -F 3000 -i -M0me"
#
###########################################################################
#SCO Xenix/386 2.2.2
sco386:
	make wermit "CFLAGS= -DXENIX -Dunix -DUXIII -DDEBUG -DTLOG \
	-Otcl  -i -M3e" \
		"LNKFLAGS = -i"
#
###########################################################################
#Interactive Corp System III port in general --
is3:
	make wermit \
		"CFLAGS = -DISIII -DUXIII -DDEBUG -DTLOG -Ddata=datax -O -i" \
		  "LNKFLAGS = -i"
#
###########################################################################
#Clean up intermediate, compiled executable, debug, log, and object files
clean:
	-rm -f ckcmai.o ckucmd.o ckuusr.o ckuus2.o ckuus3.o ckcpro.o ckcfns.o \
		   ckcfn2.o ckucon.o ckutio.o ckufio.o ckudia.o ckuscr.o ckwart.o
	-rm -f ckcpro.c
	-rm -f uckermit uermit ckermit kermit wermit a.out
	-rm -f ckwart wart
	-rm -f *.log
	-rm -f ck*.s
	-rm -f core* vgcore*
	-rm -f massif.out.*
	-rm -f .wermit.old.?
#
###########################################################################
