# C86 Makefile for ELKS using OpenWatcom C

CC = gcc
LD = gcc
CLBASE = -std=gnu99
WARNINGS = -Wall
#INCLUDES += -I$(TOPDIR)/libc/include/watcom
INCLUDES += -Ilib -Icurses -I.
DEFINES = -Dunix -Wunreachable-code
CFLAGS = -O0 -g $(CLBASE) $(WARNINGS) $(INCLUDES) $(DEFINES) $(LOCALFLAGS)
LDBASE =
#LDBASE += -Wl,option -Wl,dosseg
#LDBASE += -Wl,option -Wl,start=_start
#LDBASE += -Wl,option -Wl,nodefaultlibs
#LDBASE += -Wl,option -Wl,stack=0x2200
#LDBASE += -Wl,option -Wl,heapsize=512
LDFLAGS = $(LDBASE)
#LDLIBS = -Wl,library -Wl,$(TOPDIR)/libc/libc.lib
LDLIBS=-lcrypt

OBJS = $(SRCS:.c=.obj)
%.obj: %.c
	$(CC) -c $(CFLAGS) -o $*.obj $<

############# End of Standard Section ##############

BINDIR = ../elks-bin
LOCALFLAGS = -DNDEBUG
PROG = bobcat

# dont include  "LYCharSets.c"
SRCS =  DefaultStyle.c  HTAlert.c  LYDownlo.c    LYHistor.c  LYLeaks.c   LYMainLoop.c  LYrcFile.c LYShowIn.c \
     GridText.c      HTML.c        LYClean.c     LYEdit.c    LYJump.c    LYOption.c    LYReadCF.c  LYString.c \
     LYBookmark.c  LYCurses.c    LYGetFil.c  LYKeymap.c  LYMain.c   LYPrint.c     LYSearch.c  LYUtils.c LYEditMap.c HTWrite.c HTForms.c \
     curses/curses2.c curses/curses3.c curses/curses.c curses/unikey.c curses/tty.c \
     lib/SGML.c    lib/HTAnchor.c  lib/HTChunk.c   lib/HTMIME.c   lib/HTPlain.c   lib/HTTP.c \
     lib/HTList.c \
     lib/HTAccess.c  lib/HTAuth.c    lib/HTLex.c    lib/HTParse.c   lib/HTStyle.c  \
     lib/HTBTree.c   lib/HTFWrite.c     lib/HTPasswd.c  lib/HTTCP.c \
     lib/HTAssoc.c   lib/HTFile.c    lib/HTMLDTD.c     lib/HTUU.c \
     lib/HTAtom.c    lib/HTFormat.c  lib/HTInit.c    lib/HTMLGen.c  lib/HTString.c  lib/HTWriter.c \
     lib/HTRules.c lib/HTAAUtil.c lib/HTAAProt.c lib/HTAABrow.c  lib/HTAAFile.c lib/HTAAServ.c lib/HTACL.c lib/HTGroup.c \

all: $(PROG)

elks: all

bobcat: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)


clean:
	rm -f $(PROG) *.obj lib/*.obj curses/*.obj tmp.h
