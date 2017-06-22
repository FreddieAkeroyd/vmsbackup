##########
#
# Make file for VMS backup program
#
# assumes GNU make and a Unix box with bash or bourne.
# If you're working on OpenVMS, use build.com instead.
#
#
# 26-Feb-2011, Stephen Hoffman, HoffmanLabs LLC
# tweaked the version to 4.4.3, and added vmsbackup.pdf 
# into the cleanup
#
# 31-Mar-2008, Stephen Hoffman, HoffmanLabs LLC
# full re-write of the original makefile.
#
#

# ensure we use bourne or bash
#
SHELL = /bin/sh 

##########
#
# the vmsbackup version number.
# also change the version over in build.com
#

CURRVER = 4.4.3hb

##########
HAVE_REMOTE =
#
# Choose line depending if you want to have remote tape support
#
ifeq ($(HAVE_REMOTE),yes)
REMOTE=-D REMOTE
else
REMOTE=
endif

##########
HAVE_GETOPTLONG = yes
#
# Set this if you want to use long option names
#
ifeq ($(HAVE_GETOPTLONG),yes)
LONGOPT=-D HAVE_GETOPTLONG
else
LONGOPT=
endif

###########
#
# Housekeeping and related functions
#
OWNER=unknown                      # user for remote tape access
MODE=4755
BINDIR=/usr/local/bin
MANSEC=l
MANDIR=/usr/man/man$(MANSEC)
DISTFILES=README vmsbackup.1 Makefile vmsbackup.c vmsconstants.h match.c NEWS build.com dclmain.c getoptmain.c vmsbackup.cld vmsbackup.h AAAREADME.TXT


CFLAGS = $(LONGOPT) # -g -O0
LDFLAGS=#-g

.SUFFIXES: .c .1 .o .ps .pdf .txt

.1.txt: ; groff -man -Tascii $*.1 > $*.txt

.1.ps: ;  groff -man -Tps $*.1 > $*.ps
ifneq ($(SHELL which ps2pdf),"")
.1.pdf: ; ps2pdf $*.ps $*.pdf
else
.1.pdf: ; pstopdf $*.ps -o $*.pdf
endif

SRCS = vmsbackup.c match.c getoptmain.c
OBJS = ${SRCS:.c=.o}
DOCS = vmsbackup.txt vmsbackup.ps  vmsbackup.pdf

##########
#
# Build it all
#
all : vmsbackup  vmsbackup.info

vmsbackupver.h :
	echo "#define VMSBACKUPVERSION \"$(CURRVER)\"" > $@

getoptmain.o: getoptmain.c vmsbackupver.h

vmsbackup : $(OBJS)

vmsbackup.info : $(DOCS)

##########
#
#  Install the bits locally
#
.PHONY : install
install :
	install -m $(MODE) -o $(OWNER) -s vmsbackup $(BINDIR)
	cp vmsbackup.1 $(MANDIR)/vmsbackup.$(MANSEC)

##########
#
#  Clean up any residual mess
#
.PHONY : clean
clean :
	rm -f vmsbackup vmsbackupver.h *.o core
	rm -f vmsbackup.txt vmsbackup.ps vmsbackup.pdf
	rm -f vmsbackup-src_*.tar

##########
#
#  build a source distribution using shell archive
#
.PHONY : shar
shar :
	shar -a $(DISTFILES) > vmsbackup.shar


##########
#
#  Build a source distribution using tar
#
.PHONY : dist 
dist :
	rm -rf vmsbackup-$(CURRVER)
	mkdir vmsbackup-$(CURRVER)
	for i in $(DISTFILES); do ln $${i} vmsbackup-$(CURRVER);  done
	tar chf vmsbackup-$(CURRVER).tar vmsbackup-$(CURRVER)

.PHONY : zip
zip :
	rm -rf vmsbackup-$(CURRVER)
	mkdir vmsbackup-$(CURRVER)
	for i in $(DISTFILES); do ln $${i} vmsbackup-$(CURRVER);  done
	zip vmsbackup-$(CURRVER).zip vmsbackup-$(CURRVER)/*


