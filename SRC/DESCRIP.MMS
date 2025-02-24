!       Make LYNX hypertext browser under VMS
!       =====================================
!
! History:
!  1/1/93  creation at KU (Lou montulli@ukanaix.cc.ukans.edu). 
!  4/12/93 (seb@lns61.tn.cornell.edu)
!           modified to support either UCX or MULTINET
!  12/2/93 modified to support Lynx rewrite
!  12/13/93 (macrides@sci.wfeb.edu)
!	     Added conditional compilations for VAXC vs. DECC
!	     (dependencies not yet specified; this is just a
!	      "starter", should anyone want to do it well).
!  10/31/94 RLD Updated for Lynx v2.3.4-VMS, supporting OpenCMU
!               and TCPWare
!  11/11/94 RLD Updated for Lynx v2.3.5-VMS
!  11/18/94 FM Updated for SOCKETSHR/NETLIB
!  12/07/94 FM Updated for DECC/VAX, VAXC/VAX and DECC/AXP
!  05/03/95 FM Include /NoMember for DECC (not the default on AXP, and
!		the code assumes byte alignment).
!
! Instructions:
!       Use the correct command line for your TCP/IP implementation:
!
!	$ MMS                                   for VAXC - MultiNet
!	$ MMS /Macro = (MULTINET=1)		for VAXC - MultiNet
!	$ MMS /Macro = (WIN_TCP=1)              for VAXC - Wollongong TCP/IP
!	$ MMS /Macro = (UCX=1)			for VAXC - UCX
!	$ MMS /Macro = (CMU_TCP=1)		for VAXC - OpenCMU TCP/IP
!	$ MMS /Macro = (SOCKETSHR_TCP=1)	for VAXC - SOCKETSHR/NETLIB
!	$ MMS /Macro = (TCPWARE=1)		for VAXC - TCPWare TCP/IP
!	$ MMS /Macro = (DECNET=1)		for VAXC - socket emulation over DECnet
!	$ MMS /Macro = (MULTINET=1, DEC_C=1)	for DECC - MultiNet
!	$ MMS /Macro = (WIN_TCP=1, DEC_C=1)	for DECC - Wollongong TCP/IP
!	$ MMS /Macro = (UCX=1, DEC_C=1)		for DECC - UCX
!	$ MMS /Macro = (CMU_TCP=1, DEC_C=1)	for DEC C - OpenCMU TCP/IP
!	$ MMS /Macro = (SOCKETSHR_TCP=1,DEC_C=1) for DECC - SOCKETSHR/NETLIB
!	$ MMS /Macro = (TCPWARE=1, DEC_C=1)	for DEC C - OpenCMU TCP/IP
!	$ MMS /Macro = (DECNET=1, DEC_C=1)	for DECC - socket emulation over DECnet

OBJS = 	DefaultStyle.obj, GridText.obj, HTAlert.obj, HTFWriter.obj, -
	HTInit.obj, HTML.obj, LYBookmark.obj, LYClean.obj, LYCurses.obj, -
	LYDownload.obj, LYEdit.obj, LYExit.obj, LYForms.obj, -
	LYGetFile.obj, LYHistory.obj, LYJump.obj, LYKeymap.obj, LYLeaks.obj, -
	LYMail.obj, LYMain.obj, LYMainLoop.obj, LYNews.obj, -
	LYOptions.obj, LYPrint.obj, LYrcFile.obj, LYReadCFG.obj, -
	LYSearch.obj, LYShowInfo.obj, LYStrings.obj, LYUpload.obj, -
	LYUtils.obj

.ifdef WIN_TCP
TCP = WIN_TCP
TCPOPT = WIN_TCP
.endif

.ifdef CMU_TCP
TCP = CMU_TCP
TCPOPT = CMU_TCP
.endif

.ifdef SOCKETSHR_TCP
TCP = SOCKETSHR_TCP
TCPOPT = SOCKETSHR_TCP
.endif

.ifdef UCX
TCP = UCX
TCPOPT = UCX
.endif

.ifdef TCPWARE
TCP = TCPWARE
TCTOPT= TCPWARE
.endif

.ifdef MULTINET
TCP = MULTINET
TCPOPT = MULTINET
.endif

.ifdef DECnet
TCP = DECNET
TCPOPT = DECNET
.endif

.ifdef TCP
.else
TCP = MULTINET
TCPOPT = MULTINET
.endif

.ifdef DEC_C
COMPILER = DECC
.ifdef TCPWARE
TCPFLAGS = /Prefix=All/NoMember/Define=(DEBUG,ACCESS_AUTH,$(TCP),UCX,__VMS_CURSES)
.else
TCPFLAGS = /Prefix=All/NoMember/Define=(DEBUG,ACCESS_AUTH,$(TCP),__VMS_CURSES)
.endif
.else
COMPILER = VAXC
.ifdef TCPWARE
TCPFLAGS = /Define = (DEBUG, ACCESS_AUTH, $(TCP), UCX)
.else
TCPFLAGS = /Define = (DEBUG, ACCESS_AUTH, $(TCP))
.endif
.endif

OPT = []$(TCPOPT)_$(COMPILER).opt
WWWLIB = [-.WWW.Library.Implementation]WWWLib_$(TCP).olb
CFLAGS = $(CFLAGS) $(TCPFLAGS) /Include = ([-], [-.WWW.Library.Implementation])


lynx :	lynx.exe
	@ Continue

lynx.exe :   $(OBJS) $(WWWLIB)
	$(LINK) /Executable = Lynx.exe $(OBJS), $(WWWLIB)/lib, $(OPT)/opt

clean :
	- Set Protection = (Owner:RWED) *.*;-1
	- Purge /NoLog /NoConfirm
	- Delete /NoConfirm /NoLog *.obj;*
	- Delete /NoConfirm /NoLog *.exe;*
