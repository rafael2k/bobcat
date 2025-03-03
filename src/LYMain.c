#include "spawno.h"
#include <dos.h>
#include <dir.h>
#include "HTUtils.h"
#include "HTInit.h"
#include "HTFile.h"
#include "LYCurses.h"
#include "HTML.h"
#include "HTAccess.h"
#include "LYUtils.h"
#include "LYGlobal.h"
#include "LYSignal.h"
#include "LYGetFil.h"
#include "LYString.h"
#include "LYClean.h"
#include "LYCharSe.h"
#include "LYReadCF.h"
#include "LYrcFile.h"
#include "LYKeymap.h"
#include "HTParse.h"

#ifdef LOCALE
#include<locale.h>
#endif /* LOCALE */

#include "LYexit.h"
#include "LYLeaks.h"

extern directvideo = 0;

extern unsigned _stklen = 543210U;

extern unsigned _ovrbuffer = 0x0C00U;

/* ahhhhhhhhhh!! Global variables :-< */
int sleep_one = 1; /* variable sleep times */
int sleep_two = 2; /* variable sleep times */
int sleep_three = 3; /* variable sleep times */
char *LYUserAgent=NULL;         /* Lynx User-Agent header */
char *LYUserAgentFake=NULL;  /* Lynx Fake User-Agent header  */
char *incoming = NULL;
char *chomedir = NULL;
char *cdirbuffer = NULL;
char *TEMP_SPACE = NULL;
int HTCacheSize = DEFAULT_CACHE_SIZE;  /* number of docs cached in memory */
#if defined(VMS) && defined(VAXC) && !defined(__DECC)
/* Don't dump doc cache unless this size is exceeded */
int HTVirtualMemorySize = DEFAULT_VIRTUAL_MEMORY_SIZE;
#endif /* VMS && VAXC && !_DECC */
char *empty_string = "\0";
int display_lines;  /* number of lines in display */
int www_search_result= -1;
lynx_printer_item_type *printers = NULL;    /* linked list of printers */
lynx_extern_item_type *auto_externs = NULL;   /* linked list of auto extern options*/
lynx_html_item_type *downloaders = NULL;    /* linked list of download options*/
lynx_html_item_type *uploaders = NULL;
char *log_file_name = NULL;  /* for WAIS log file name in libWWW */
int port_syntax = 1;

BOOLEAN LYShowCursor=SHOW_CURSOR; /* to show or not to show */
BOOLEAN LYforce_no_cache=FALSE;
BOOLEAN LYUserSpecifiedURL=TRUE;  /* always true the first time */
BOOLEAN LYJumpFileURL=FALSE;	  /* always false the first time */
BOOLEAN jump_buffer=JUMPBUFFER;   /* TRUE if recalling shortcuts */
BOOLEAN recent_sizechange=FALSE;  /* the window size changed recently? */
BOOLEAN user_mode=NOVICE_MODE;
BOOLEAN dump_output_immediately=FALSE;
BOOLEAN is_www_index=FALSE;
BOOLEAN lynx_mode=NORMAL_LYNX_MODE;
BOOLEAN bold_headers=FALSE;

#ifdef VMS
BOOLEAN UseFixedRecords = USE_FIXED_RECORDS;	/* create FIXED 512 binaries */
#endif /* VMS */

#ifdef DIRED_SUPPORT
BOOLEAN lynx_edit_mode = FALSE;
BOOLEAN dir_list_style = MIXED_STYLE;
taglink *tagged = NULL;
#endif /* DIRED_SUPPORT */
#ifdef IGNORE_CTRL_C
BOOLEAN sigint = FALSE;
#endif /* IGNORE_CTRL_C */

BOOLEAN child_lynx = FALSE;
#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
#ifndef NEVER_ALLOW_REMOTE_EXEC
BOOLEAN local_exec = LOCAL_EXECUTION_LINKS_ALWAYS_ON;
#else
BOOLEAN local_exec = FALSE;
#endif /* NEVER_ALLOW_REMOTE_EXEC */
BOOLEAN local_exec_on_local_files = LOCAL_EXECUTION_LINKS_ON_BUT_NOT_REMOTE;
#endif /* EXEC_LINKS || EXEC_SCRIPTS */
BOOLEAN error_logging = MAIL_SYSTEM_ERROR_LOGGING;
BOOLEAN log_urls = TRUE;
BOOLEAN check_mail = CHECKMAIL;
BOOLEAN vi_keys = VI_KEYS_ALWAYS_ON;
BOOLEAN emacs_keys = EMACS_KEYS_ALWAYS_ON;
BOOLEAN keypad_mode = DEFAULT_KEYPAD_MODE;
BOOLEAN case_sensitive = CASE_SENSITIVE_ALWAYS_ON;
BOOLEAN telnet_ok = TRUE;
// BOOLEAN news_ok = TRUE;
BOOLEAN rlogin_ok = TRUE;
BOOLEAN ftp_ok = TRUE;
BOOLEAN system_editor = FALSE;
BOOLEAN no_inside_telnet = FALSE;
BOOLEAN no_outside_telnet = FALSE;
BOOLEAN no_telnet_port = FALSE;
// BOOLEAN no_inside_news = FALSE;
// BOOLEAN no_outside_news = FALSE;
BOOLEAN no_inside_ftp = FALSE;
BOOLEAN no_outside_ftp = FALSE;
BOOLEAN no_inside_rlogin = FALSE;
BOOLEAN no_outside_rlogin = FALSE;
BOOLEAN no_suspend = FALSE;
BOOLEAN no_editor = FALSE;
BOOLEAN no_shell = FALSE;
BOOLEAN no_bookmark = FALSE;
BOOLEAN no_option_save = FALSE;
BOOLEAN no_print = FALSE;
BOOLEAN no_download = FALSE;
BOOLEAN no_disk_save = FALSE;
BOOLEAN no_exec = FALSE;
BOOLEAN no_lynxcgi = FALSE;
BOOLEAN exec_frozen = FALSE;
BOOLEAN no_bookmark_exec = FALSE;
BOOLEAN no_goto = FALSE;
BOOLEAN no_jump = FALSE;
BOOLEAN no_file_url = FALSE;
// BOOLEAN no_newspost = FALSE;
BOOLEAN no_mail = FALSE;
BOOLEAN nodotfiles = NO_DOT_FILES;
BOOLEAN no_statusline = FALSE;
BOOLEAN local_host_only = FALSE;

#ifdef DIRED_SUPPORT
BOOLEAN no_dired_support = FALSE;
#endif /* DIRED_SUPPORT */

BOOLEAN LYforce_HTML_mode=FALSE;
char *homepage = NULL;		    /* home page or main screen */
char *editor = NULL;  		    /* the name of the current editor */
char *jumpfile = NULL;		    /* the name of the file containing jumps */
char *bookmark_page = NULL;   	    /* the name of the current bookmark page */
char *startfile = NULL;		    /* the first file */
char *helpfile = NULL; 		    /* the help file */
char *indexfile = NULL;		    /* an index file if there is one */
char *personal_mail_address = NULL; /* the users mail address */
char *display=NULL;		    /* display environment variable */
char *personal_type_map = NULL;	    /* .mailcap */
char *global_type_map = NULL;	    /* global mailcap */
char *global_extension_map = NULL;  /* global mime.types */
char *personal_extension_map = NULL;/* .mime.types */
char *language = NULL;		    /* preferred language */
// char *inews_path = NULL;            /* the path for posting to news */
#ifndef VMS
char *http_proxy_putenv_cmd = NULL; /* lynx.cfg specified http_proxy */
char *ftp_proxy_putenv_cmd = NULL;  /* lynx.cfg specified ftp_proxy */
char *gopher_proxy_putenv_cmd=NULL; /* lynx.cfg specified gopher_proxy */
// char *news_proxy_putenv_cmd = NULL; /* lynx.cfg specified news_proxy */
char *wais_proxy_putenv_cmd = NULL; /* lynx.cfg specified wais_proxy */
char *no_proxy_putenv_cmd = NULL;   /* lynx.cfg specified no_proxy */
#ifdef SYSLOG_REQUESTED_URLS
char *syslog_txt = NULL;            /* syslog arb text for session */
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

char *smtp_server = NULL;   /* lynx.cfg specified smtp server */

BOOLEAN clickable_images = FALSE;
BOOLEAN keep_mime_headers = FALSE;   /* Return mime headers */
BOOLEAN no_url_redirection = FALSE;  /* Don't follow URL redirections */
char *form_post_data = NULL;         /* User data for post form */
char *form_get_data = NULL;          /* User data for get form */
char *http_error_file = NULL;        /* Place HTTP status code in this file */
char *authentication_info[2] = {NULL, NULL}; /* Id:Password for protected forms */


int LYlines = 24;
int LYcols = 80;

linkstruct links[MAXLINKS] = {NULL};

histstruct history[MAXHIST] = {NULL};

int nlinks = 0;  /* number of links in memory */
int nhist = 0;   /* number of history entries */
int more = FALSE; /* is there more text to display? */

PRIVATE void parse_arg PARAMS((char **arg, int *i, int argc));
PRIVATE void FatalProblem PARAMS((int sig));
extern void mainloop();

PRIVATE BOOLEAN restrictions_set=TRUE;
PRIVATE BOOLEAN stack_dump=FALSE;
PRIVATE char *terminal=NULL;
PRIVATE char *pgm;

PUBLIC int main ARGS2(int,argc, char **,argv)
{
    int  i;  /* indexing variable */
    char *lynx_cfg_file=NULL;
    char *lynx_version_putenv_command=NULL;
    char *cp;
    FILE *fp;

    pgm = argv[0];
    if ((cp = strrchr(pgm, '/')) != NULL) {
	pgm = cp + 1;
    }

    StrAllocCopy(cdirbuffer, pgm);

    {
	char *ptr;
	ptr = strrchr(cdirbuffer, '\\');
	if (ptr != NULL)
		*ptr = NULL;
    }

    if( cdirbuffer[strlen(cdirbuffer)-1] != '\\')
	 StrAllocCat(cdirbuffer, "\\");
    {
       char *ptrr;

       ptrr = getenv("HOME");

       if (ptrr) {
	    StrAllocCopy(chomedir,ptrr);
	    if( chomedir[strlen(chomedir)-1] != '\\')
		 StrAllocCat(chomedir, "\\");
       }
       else chomedir = cdirbuffer;

    }

    init_SPAWNO(chomedir,SWAP_ANY) ;

    if (_OvrInitEms(0L , 0L, 0L)) _OvrInitExt (0, 0);

/*
    {
	printf("No expanded memory found\n");
	if (_OvrInitExt (0, 0)) {
	    printf("No extended memory found\n");
	}
    }
*/
    {
	char *ccp;

	ccp=getenv("TEMP");
	if (ccp == NULL)
	    ccp=getenv("TMP");

	if (ccp != NULL)
	    StrAllocCopy(TEMP_SPACE, ccp);
	else
	    StrAllocCopy(TEMP_SPACE, chomedir);

	if( TEMP_SPACE[strlen(TEMP_SPACE)-1] != '\\')
	    StrAllocCat(TEMP_SPACE, "\\");

    }

    terminal = "vt100";

#ifdef SOCKS
    SOCKSinit(argv[0]);
#endif /* SOCKS */

#ifdef LY_FIND_LEAKS
    /*
     *	Register the final function to be executed when being exited.
     *	Will display memory leaks if LY_FIND_LEAKS is defined.
     */
//    atexit(LYLeaks);
#endif /* LY_FIND_LEAKS */

#ifdef LOCALE
    /* LOCALE support for international characters. */
    setlocale(LC_ALL, "");
#endif /* LOCALE */

    /* initialize some variables */
    /* zero the links struct array */
    memset((void *)links, 0, sizeof(linkstruct)*MAXLINKS);

    StrAllocCopy(helpfile, HELPFILE);
    StrAllocCopy(startfile, STARTFILE);
#ifdef JUMPFILE
    StrAllocCopy(jumpfile, JUMPFILE);
#endif /* JUMPFILE */
    StrAllocCopy(indexfile, DEFAULT_INDEX_FILE);
    StrAllocCopy(global_type_map, cdirbuffer);
    StrAllocCat(global_type_map, GLOBAL_MAILCAP);
    StrAllocCopy(personal_type_map, chomedir);
    StrAllocCat(personal_type_map, PERSONAL_MAILCAP);
    StrAllocCopy(global_extension_map, cdirbuffer);
    StrAllocCat(global_extension_map, GLOBAL_EXTENSION_MAP);
    StrAllocCopy(personal_extension_map, chomedir);
    StrAllocCat(personal_extension_map, PERSONAL_EXTENSION_MAP);
    StrAllocCopy(language, PREFERRED_LANGUAGE);
//    StrAllocCopy(inews_path, INEWS);


#ifdef UNIX
    StrAllocCopy(lynx_version_putenv_command,"LYNX_VERSION=");
    StrAllocCat(lynx_version_putenv_command,LYNX_VERSION);
    putenv(lynx_version_putenv_command);
#endif /* UNIX */

    StrAllocCopy(LYUserAgent, LYNX_NAME);
    StrAllocCat(LYUserAgent, "/");
    StrAllocCat(LYUserAgent, LYNX_VERSION);
    StrAllocCopy(LYUserAgentFake, LYUserAgent);

    /* set up trace, if requested, and an alternate
     * configuration file, if specified, now
     */
    for (i=1; i<argc; i++) {
	if (strncmp(argv[i], "-trace", 6) == 0)
	    WWW_TraceFlag = TRUE;
	else if (strncmp(argv[i], "-cfg", 4) == 0) {
	    if((cp=strchr(argv[i],'=')) != NULL)
		StrAllocCopy(lynx_cfg_file, cp+1);
	    else {
		StrAllocCopy(lynx_cfg_file, argv[i+1]);
		i++;
	    }
	}
    }

    /*
     * If no alternate configuration file was specified on
     * the command line, see if it's in the environment.
     */
    if (!lynx_cfg_file) {
	if (((cp=getenv("LYNX_CFG")) != NULL) ||
	    (cp=getenv("lynx_cfg")) != NULL)
	    StrAllocCopy(lynx_cfg_file, cp);
    }

    /*
     * If we still don't have a configuration file,
     * use the userdefs.h definition.
     */
    if (!lynx_cfg_file) {
	StrAllocCopy(lynx_cfg_file, cdirbuffer);
	StrAllocCat(lynx_cfg_file, LYNX_CFG_FILE);
    }

    /*
     * If the configuration file is not readable,
     * inform the user and exit.
     */
    if ((fp = fopen(lynx_cfg_file, "r")) == NULL) {
        fprintf(stderr, "\nConfiguration file %s is not readable.\nRun newuser.bat to install.\n\n",
			lynx_cfg_file);
	exit(1);
    }
    fclose(fp);

    /*
     * Process the configuration file.
     */
    read_cfg(lynx_cfg_file);
    free(lynx_cfg_file);

    /*
     * Set up the file extension and mime type maps from
     * src/HTInit.c and the global and personal mime.types
     * and mailcap files.  These will override any SUFFIX
     * or VIEWER maps in userdefs.h or the configuration
     * file, if they overlap.
     */
    HTFormatInit();
    HTFileInit();

    /*
     * Get WWW_HOME environment variable if it exists)
     */
    if((cp = getenv("WWW_HOME")) != NULL)
	StrAllocCopy(startfile, cp);

#ifdef SYSLOG_REQUESTED_URLS
      init_syslog();
#endif /* SYSLOG_REQUESTED_URLS */

    /*
     * Process arguments - with none, look for the database in STARTDIR,
     * starting with STARTFILE.
     *
     * If a pathname is given, use it as the starting point.  Split it
     * into directory and file components, 'cd' to the directory, and
     * view the file.
     */

     /*
      * If the only argument is '-' then we expect to see the arguments on
      * stdin, this is to allow for the potentially very long command line that
      * can be associated with post or get data.
      */
     if (argc == 2 && strcmp(argv[1], "-") == 0) {
	 char buf[513];
	 char *argv[2];
	 
	 argv[0] = buf;
	 argv[1] = NULL;
	 
	 while(fgets(buf, sizeof(buf) - 1, stdin)) {
	     int j;
	     
	     for(j = strlen(buf) - 1; j > 0 && 
		 (buf[j] == CR || buf[j] == LF); j--) {
		 buf[j] = '\0';
	     }
	     parse_arg(argv, NULL, -1);
	 }
     } else {
	 for (i=1; i<argc; i++) {
	     parse_arg(&argv[i], &i, argc);
	 }
     }
    
//    if(!dump_output_immediately)
//	printf("\nTo sleep, perchance to dream ...\n");

#ifdef SYSLOG_REQUESTED_URLS
        textlog("Session start");
#endif /* SYSLOG_REQUESTED_URLS */

    /* read the rc file */
    read_rc();
#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
#ifdef NEVER_ALLOW_REMOTE_EXEC
    if (local_exec) {
	local_exec = FALSE;
	local_exec_on_local_files=TRUE;
    }
#endif /* NEVER_ALLOW_REMOTE_EXEC */
#endif /* EXEC_LINKS || EXEC_SCRIPTS */


    if (emacs_keys)
        set_emacs_keys();
 
    if (vi_keys)
        set_vi_keys();
 
    if (keypad_mode == NUMBERS_AS_ARROWS)
	set_numbers_as_arrows();

    /* disable news posting if no posting command */
//    if (*inews_path == '\0' || !strcasecomp(inews_path,"none"))
//        no_newspost = TRUE;

#ifdef VMS
    set_vms_keys();
#endif /* VMS */


    /* trap interrupts */
//    if(!dump_output_immediately)
//        (void) signal (SIGHUP, cleanup_sig);
    (void) signal (SIGTERM, cleanup_sig);
#ifdef SIGWINCH
    (void) signal (SIGWINCH, size_change);
#endif /* SIGWINCH */
#ifndef VMS
    if(!TRACE && !dump_output_immediately && !stack_dump) {
	(void) signal (SIGINT, cleanup_sig);
#ifndef __linux__
#ifndef MSDOS
	(void) signal (SIGBUS, FatalProblem);
#endif
#endif /* !__linux__ */
        (void) signal (SIGSEGV, FatalProblem);
        (void) signal (SIGILL, FatalProblem);
        /*
	 * Since we're doing lots of TCP, just ignore SIGPIPE altogether.
	 *
	 * HTTCP.c should deal with a broken pipe for servers.
	 * Rick Mallet's check after c = GetChar() in LYStrings.c should
	 *  deal with a disconnected terminal.
	 * So the runaway CPU time problem on Unix should not occur any
	 *  more.
	 */
#ifndef MSDOS
	(void) signal (SIGPIPE, SIG_IGN);
#endif
    }
#endif /* !VMS */

    /* set up the proper character set */
    HTMLUseCharacterSet(current_char_set);

    /*
     * If startfile is a file URL and the host is defaulted,
     * force in "//localhost".  We need this until all the
     * other Lynx code which performs security checks based
     * on the "localhost" string is changed to assume
     * "//localhost" when a host field is not present in file
     * URLs - FM
     */
// #ifdef FIXME

    if(!strncmp(startfile, "file:", 5)) {
	char *temp=NULL;
	if (startfile[5] == '\0') {
	    StrAllocCat(startfile, "//localhost");
	} else if(!strcmp(startfile, "file://")) {
	    StrAllocCat(startfile, "localhost");
	} else if(!strncmp(startfile, "file:///", 8)) {
	    StrAllocCopy(temp, (startfile+7));
	    StrAllocCopy(startfile, "file://localhost");
	    StrAllocCat(startfile, temp);
	    free(temp);
	} else if (!strncmp(startfile, "file:/", 6) && startfile[6] != '/') {
	    StrAllocCopy(temp, (startfile+5));
	    StrAllocCopy(startfile, "file://localhost");
	    StrAllocCat(startfile, temp);
	    free(temp);
	}
    }

// #endif

    /*
     * No path in a file://localhost URL for startfile means
     * a directory listing for the current default. - FM
     */
    if(!strcmp(startfile, "file://localhost")) {
#ifdef VMS
	StrAllocCat(startfile, HTVMS_wwwName(getenv("PATH")));
#else
	char curdir[DIRNAMESIZE];
#ifdef NO_GETCWD
	getwd (curdir);
#else
	getcwd (curdir, DIRNAMESIZE);
#endif /* NO_GETCWD */
	StrAllocCat(startfile, curdir);
#endif /* VMS */
    }

#ifdef VMS
    /*
     * On VMS, a file://localhost/ URL for startfile
     * means a listing for the login directory. - FM
     */
    if(!strcmp(startfile, "file://localhost/"))
	StrAllocCat(startfile, (HTVMS_wwwName(getenv("HOME"))+1));
#endif /* VMS */

    /*
     * If its not a URL then make it one
     */
    if(!is_url(startfile)) {
	    /* rewrite the file as a URL */
	   char *old_startfile=startfile;

	   startfile = NULL;  /* so StrAllocCopy doesn't free it */
           StrAllocCopy(startfile,"http://");
#ifdef FIXME
	   StrAllocCopy(startfile,"file://localhost");
	   if(*old_startfile != '/') {
		char curdir[DIRNAMESIZE];
#ifdef NO_GETCWD
		getwd (curdir);
#else
		getcwd (curdir, DIRNAMESIZE);
#endif /* NO_GETCWD */
//		StrAllocCat(startfile,curdir);
		StrAllocCopy(startfile,curdir);

		StrAllocCat(startfile,"/");
	   }
#endif
	   StrAllocCat(startfile,old_startfile);

	   if(old_startfile)
	       free(old_startfile);
    }

    /*
     * If homepage is a file URL and the host is defaulted,
     * force in "//localhost".  We need this until all the
     * other Lynx code which performs security checks based
     * on the "localhost" string is changed to assume
     * "//localhost" when a host field is not present in file
     * URLs - FM
     */
    if(homepage && !strncmp(homepage, "file:", 5)) {
	char *temp=NULL;
        if (homepage[5] == '\0') {
            StrAllocCat(homepage, "//localhost");
        } else if(!strcmp(homepage, "file://")) {
            StrAllocCat(homepage, "localhost");
        } else if(!strncmp(homepage, "file:///", 8)) {
	    StrAllocCopy(temp, (homepage+7));
	    StrAllocCopy(homepage, "file://localhost");
	    StrAllocCat(homepage, temp);
	    free(temp);
        } else if (!strncmp(homepage, "file:/", 6) && homepage[6] != '/') {
	    StrAllocCopy(temp, (homepage+5));
	    StrAllocCopy(homepage, "file://localhost");
	    StrAllocCat(homepage, temp);
	    free(temp);
	}
    }

    /*
     * No path in a file://localhost URL for homepage means
     * a directory listing for the current default. - FM
     */
    if(homepage && !strcmp(homepage, "file://localhost")) {
#ifdef VMS
	StrAllocCat(homepage, HTVMS_wwwName(getenv("PATH")));
#else
        char curdir[DIRNAMESIZE];
#ifdef NO_GETCWD
	getwd (curdir);
#else
    	getcwd (curdir, DIRNAMESIZE);
#endif /* NO_GETCWD */
	StrAllocCat(homepage, curdir);
#endif /* VMS */
    }

#ifdef VMS
    /*
     * On VMS, a file://localhost/ URL for homepage
     * means a listing for the login directory. - FM
     */
    if(homepage && !strcmp(homepage, "file://localhost/"))
	StrAllocCat(homepage, (HTVMS_wwwName(getenv("HOME"))+1));
#endif /* VMS */

    if(homepage && !is_url(homepage)) {
	    /* rewrite the file as a URL */
	   char *old_homepage=homepage;

	   homepage = NULL;  /* so StrAllocCopy doesn't free it */
	   StrAllocCopy(homepage,"file://localhost");
	   if(*old_homepage != '/') {
#ifdef VMS
	      /*
	       *  Not a URL pathspec.  Get the full VMS spec and convert it.
	       */
	       char *cp, *cur_dir=NULL, *tmp_dir=NULL;
	       static char home_page[256], file_name[256];
	       int context = 0;
	       $DESCRIPTOR(home_page_dsc, home_page);
	       $DESCRIPTOR(file_name_dsc, file_name);
	       strcpy(home_page, old_homepage);
	       home_page_dsc.dsc$w_length = (short) strlen(old_homepage);
	       if (1&lib$find_file(&home_page_dsc, &file_name_dsc, &context,
    				   0, 0, 0, 0)) {
		  /*
		   *  We found the file.  Convert to a URL pathspec.
		   */
	           if ((cp=strchr(file_name, ';')) != NULL)
		       *cp = '\0';
		   for (cp = file_name; *cp; cp++)
			*cp = TOLOWER(*cp);
		   strcpy(home_page, HTVMS_wwwName(file_name));
		   if ((cp=strchr(old_homepage, ';')) != NULL)
		       strcat(home_page, cp);
	       } else if ((cur_dir=getcwd(cur_dir, 256, 0)) &&
	       		  0==chdir(old_homepage)) {
		   /*
		    * Probably a directory.  Try converting that.
		    */
		   tmp_dir = getcwd(tmp_dir, 256, 0);
		   if (tmp_dir) {
		       /*
		        * Yup, we got it!
			*/
		       strcpy(home_page, tmp_dir);
		       free(tmp_dir);
		   } else {
		       /*
		        *  Nope.  Use original pathspec for the
			*  error message that will result.
			*/
		       strcpy(home_page, "/");
		       strcat(home_page, old_homepage);
		   }
	       } else {
	          /*
		   *  File not found.  Use original pathspec for the
		   *  error message that will result.
		   */
	           strcpy(home_page, "/");
	           strcat(home_page, old_homepage);
	       }
	       lib$find_file_end(&context);
	       if (cur_dir) {
	           chdir(cur_dir);
		   free(cur_dir);
	       }
	       StrAllocCat(homepage, home_page);
	   } else {
	      /*
	       *  Use the original, URL pathspec.
	       */
	       StrAllocCat(homepage, old_homepage);
	   }
#else /* Unix: */
               	char curdir[256];
#ifdef NO_GETCWD
      		getwd (curdir);
#else
		getcwd (curdir, DIRNAMESIZE);
#endif /* NO_GETCWD */
		StrAllocCat(homepage,curdir);

		StrAllocCat(homepage,"/");
	   }
	   StrAllocCat(homepage,old_homepage);
#endif /* VMS */
	   if(old_homepage)
	       free(old_homepage);
    }
    if (!homepage)
	homepage = startfile;

    /*
     * anonymous may already be set above by command line options.
     * so this just sets the correct options.
     */
	if(!restrictions_set)
	    parse_restrictions("default");

    if(inlocaldomain()) {
#if defined(NO_UTMP) || defined(VMS) /* not selective */
        telnet_ok = !no_inside_telnet && !no_outside_telnet && telnet_ok;
//        news_ok = !no_inside_news && !no_outside_news && news_ok;
	ftp_ok = !no_inside_ftp && !no_outside_ftp && ftp_ok;
	rlogin_ok = !no_inside_rlogin && !no_outside_rlogin && rlogin_ok;
#else
#ifdef DT
	if(TRACE)
	   fprintf(stderr,"LYMain.c: User in Local domain\n");
#endif

        telnet_ok = !no_inside_telnet && telnet_ok;
//        news_ok = !no_inside_news && news_ok;
	ftp_ok = !no_inside_ftp && ftp_ok;
	rlogin_ok = !no_inside_rlogin && rlogin_ok;
#endif /* NO_UTMP || VMS */
    } else {
#ifdef DT
	if(TRACE)
	   fprintf(stderr,"LYMain.c: User in REMOTE domain\n");
#endif

        telnet_ok = !no_outside_telnet && telnet_ok;
//        news_ok = !no_outside_news && news_ok;
	ftp_ok = !no_outside_ftp && ftp_ok;
	rlogin_ok = !no_outside_rlogin && rlogin_ok;
    }

#ifdef SIGTSTP
    if(no_suspend)
	signal(SIGTSTP,SIG_IGN);
#endif /* SIGTSTP */

    /*
     * here's where we do all the work
     */
    if(dump_output_immediately) {
	mainloop();
//        (void) signal (SIGHUP, SIG_IGN);
	(void) signal (SIGTERM, SIG_IGN);
	(void) signal (SIGINT, SIG_IGN);
    } else {
	if (setup(terminal)) {
	    mainloop();
	    cleanup();
	}
    }

//    endwin();
    clrscr();
    printf("\n\n");

    exit(0);
    /* NOTREACHED */
}

/* Parse one argument, optionally picking up the next entry in argv (if
 * appropriate).
 */
PRIVATE void parse_arg ARGS3(char **, argv, int *, i, int, argc)
{
    char *cp;
 
    if (strncmp(argv[0], "-anonymous", 10) == 0) {
	if(!restrictions_set)
	    parse_restrictions("default");
	
    } else if(strncmp(argv[0], "-restrictions", 13) == 0) {
	if((cp=strchr(argv[0],'=')) != NULL)
	    parse_restrictions(cp+1);
	else 
	    {
		/* print help */
		printf("\n\
   USAGE: lynx -restrictions=[option][,option][,option]\n\
   List of Options:\n\
   all             restricts all options.\n\
   default         same as commandline option -anonymous.  Disables\n\
		   default services for anonymous users.  Currently set to,\n\
                   all restricted except for: inside_telnet, outside_telnet,\n\
                   inside_ftp, outside_ftp, inside_rlogin,\n\
		   outside_rlogin, goto, jump and mail.  Defaults settable\n\
		   within userdefs.h\n");
		printf("\
   bookmark        disallow changing the location of the bookmark file.\n\
   bookmark_exec   disallow execution links via the bookmark file\n");
#ifdef DIRED_SUPPORT
		printf("\
   dired_support   disallow local file management\n");
#endif /* DIRED_SUPPORT */
		printf("\
   disk_save       disallow saving binary files to disk in the download menu\n\
   download        disallow downloaders in the download menu\n\
   editor          disallow editing\n\
   exec            disable execution scripts\n\
   exec_frozen     disallow the user from changing the execution link\n\
   file_url        disallow using G)oto to go to file: URL's\n\
   goto            disable the 'g' (goto) command\n");
#if defined(NO_UTMP) || defined(VMS) /* not selective */
		printf("\
   inside_ftp      disallow ftps for people coming from inside your\n\
                   domain (utmp required for selectivity)\n\
   inside_rlogin   disallow rlogins for people coming from inside your\n\
                   domain (utmp required for selectivity)\n\
   inside_telnet   disallow telnets for people coming from inside your\n\
                   domain (utmp required for selectivity)\n");
#else
		printf("\
   inside_ftp      disallow ftps for people coming from inside your domain\n\
   inside_rlogin   disallow rlogins for people coming from inside your domain\n\
   inside_telnet   disallow telnets for people coming from inside your domain\n");
#endif /* NO_UTMP || VMS */
		printf("\
   jump            disable the 'j' (jump) command\n\
   mail            disallow mail\n\
   option_save     disallow saving options in lynxrc\n");
#if defined(NO_UTMP) || defined(VMS) /* not selective */
		printf("\
   outside_ftp     disallow ftps for people coming from outside your\n\
                   domain (utmp required for selectivity)\n\
   outside_rlogin  disallow rlogins for people coming from outside your\n\
		   domain (utmp required for selectivity)\n\
   outside_telnet  disallow telnets for people coming from outside your\n\
                   domain (utmp required for selectivity)\n");
#else
		printf("\
   outside_ftp     disallow ftps for people coming from outside your domain\n\
   outside_rlogin  disallow rlogins for people coming from outside your domain\n\
   outside_telnet  disallow telnets for people coming from outside your domain\n");
#endif /* NO_UTMP || VMS */
		printf("\
   print           disallow most print options\n\
   shell           disallow shell escapes, lynxexec, and lynxcgi G)oto's\n\
   suspend         disallow Control-Z suspends with escape to shell\n\
   telnet_port     disallow specifying a port in telnet G)oto's\n");
		exit(1);
	    }

    } else if(strncmp(argv[0], "-homepage", 9) == 0) {
	if((cp=strchr(argv[0],'=')) != NULL)
	    StrAllocCopy(homepage,cp+1);
	else if (argv[1]) {
	    StrAllocCopy(homepage,argv[1]);
	    *i++;
	}
	
    } else if(strncmp(argv[0], "-editor", 7) == 0) {
	if((cp=strchr(argv[0],'=')) != NULL)
	    StrAllocCopy(editor,cp+1);
	else if (argv[1]) {
	    StrAllocCopy(editor,argv[1]);
	    *i++;
	}
	system_editor = TRUE;

#ifdef SYSLOG_REQUESTED_URLSzzz
    } else if(strncmp(argv[0], "-syslog", 7) == 0) {
	if((cp=strchr(argv[0],'=')) != NULL)
	    StrAllocCopy(syslog_txt,cp+1);
	else if (argv[1]) {
	    StrAllocCopy(syslog_txt,argv[1]);
	    *i++;
	}
#endif /* SYSLOG_REQUESTED_URLS */
	
    } else if(strncmp(argv[0], "-display", 8) == 0) {
	
	char *putenv_command;
	
	if((cp=strchr(argv[0],'=')) != NULL)
	    display = cp+1;
	else if (argv[1]) {
	    display = argv[1];
	    *i++;
	}

#ifdef UNIX
	StrAllocCopy(putenv_command,"DISPLAY=");
	StrAllocCopy(putenv_command,display);
	putenv(putenv_command);
#endif /* UNIX */
	
    } else if(strncmp(argv[0], "-index", 6) == 0) {
	if((cp=strchr(argv[0],'=')) != NULL)
	    StrAllocCopy(indexfile, cp+1);
	else if (argv[1]) {
	    StrAllocCopy(indexfile, argv[1]);
	    *i++;
	}
	
    } else if(strncmp(argv[0], "-fake_agent", 11) == 0) {
        
        free(LYUserAgent);
        LYUserAgent = NULL;
        StrAllocCopy(LYUserAgent,LYUserAgentFake);

    } else if(strncmp(argv[0], "-cfg", 4) == 0) {
	/* already read the alternate configuration file 
	 * so just check whether we need to increment i
	 */
	if((cp=strchr(argv[0],'=')) == NULL)
	    *i++;
	
    } else if(strncmp(argv[0], "-stack_dump", 11) == 0) {
	stack_dump = TRUE;

    } else if(strncmp(argv[0], "-cache", 6) == 0) {
	if((cp=strchr(argv[0],'=')) != NULL)
	    HTCacheSize = atoi(cp+1);
	else if (argv[1]) {
	    HTCacheSize= atoi(argv[1]);
	    *i++;
	}
	
	/* limit size */
	if(HTCacheSize < 2) HTCacheSize = 2;
	
    } else if(strncmp(argv[0], "-vikeys", 7) == 0) {
	vi_keys = TRUE;
	
    } else if(strncmp(argv[0], "-emacskeys", 10) == 0) {
	emacs_keys = TRUE;


    } else if(strncmp(argv[0], "-version", 8) == 0) {

	printf(
"\n%s Version %s\n \
Portions copyrighted by:\n \
(c)1996,1997 Wayne Buttles (The Bobcat entity) \n \
(c)GNU General Public License (Lynx base code)\n \
(c)1990,1991,1992 Ralf Brown (SPAWNO v4.10)\n \
(c)1990,1991,1992,1993 Erick Engelke and others (WATTCP)\n \
Other copyrights held for the PDcurses, WATTCP and WWWLIB\n \
libraries can be found in the source distribution.\n \
<http://www.fdisk.com/doslynx/>\n \
Build: %s %s EST\n\n",
LYNX_NAME, LYNX_VERSION, __DATE__, __TIME__);

	exit(0);
	
    } else if(strncmp(argv[0], "-case", 5) == 0) {
	case_sensitive = TRUE;
	
    } else if(strncmp(argv[0], "-dump", 5) == 0) {
	dump_output_immediately = TRUE;
	LYcols=80;
	
    } else if(strncmp(argv[0], "-source", 7) == 0) {
	dump_output_immediately = TRUE;
	HTOutputFormat = HTAtom_for("www/dump");
	LYcols=999;
	
    } else if(strncmp(argv[0], "-force_html", 11) == 0) {
	LYforce_HTML_mode = TRUE;
	
    } else if (strncmp(argv[0], "-trace", 6) == 0) {
	WWW_TraceFlag = TRUE;
	
    } else if (strncmp(argv[0], "-linknums", 9) == 0) {
	keypad_mode = LINKS_ARE_NUMBERED;

    } else if (strncmp(argv[0], "-image_links", 12) == 0) {
	if (clickable_images)
	    clickable_images = FALSE;
        else
	    clickable_images = TRUE;

    } else if (strncmp(argv[0], "-localhost", 10) == 0) {
	local_host_only = TRUE;
	
    } else if(strncmp(argv[0], "-nobrowse", 9) == 0) {
	HTDirAccess = HT_DIR_FORBID;
	
    } else if(strncmp(argv[0], "-selective", 10) == 0) {
	HTDirAccess = HT_DIR_SELECTIVE;

    } else if (strncmp(argv[0], "-noprint", 8) == 0) {
	no_print=TRUE;
	
    } else if (strncmp(argv[0], "-print", 6) == 0) {
	no_print=FALSE;
	
#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
    } else if (strncmp(argv[0], "-exec", 5) == 0) {
#ifndef NEVER_ALLOW_REMOTE_EXEC
	local_exec=TRUE;
#else
	local_exec_on_local_files=TRUE;
#endif /* NEVER_ALLOW_REMOTE_EXEC */
	
    } else if (strncmp(argv[0], "-locexec", 8) == 0) {
	local_exec_on_local_files=TRUE;
	
    } else if (strncmp(argv[0], "-noexec", 7) == 0) {
	local_exec=FALSE;
#endif /* EXEC_LINKS || EXEC_SCRIPTS */
	
    } else if (strncmp(argv[0], "-child", 6) == 0) {
	child_lynx=TRUE;
	
    } else if (strncmp(argv[0], "-nolog", 6) == 0) {
	error_logging=TRUE;
	
    } else if(strncmp(argv[0], "-nostatus", 9) == 0)	{
	no_statusline = TRUE;
	
    } else if(strncmp(argv[0], "-show_cursor", 12) == 0) {
	LYShowCursor = TRUE;

    } else if (strncmp(argv[0], "-term", 5) == 0) {
	if((cp=strchr(argv[0],'=')) != NULL)
	    terminal = cp+1;
	else if (argv[1]) {
	    terminal = argv[1];
	    *i++;
	}
	
    } else if (strncmp(argv[0], "-telnet", 7) == 0) {
	telnet_ok=FALSE;
	
    } else if (strncmp(argv[0], "-ftp", 4) == 0) {
	ftp_ok=FALSE;
	
    } else if (strncmp(argv[0], "-rlogin", 7) == 0) {
	rlogin_ok=FALSE;
	
    } else if (strncmp(argv[0], "-fileversions", 13) == 0) {
#ifdef VMS
	HTVMSFileVersions=TRUE;
#else
	return;
#endif /* VMS */

    } else if (strcmp(argv[0], "-post_data") == 0 ||
 	       strcmp(argv[0], "-get_data") == 0) { 	/* User data for post
 							   or get form. */
	char **post_data;
	char buf[1024];

        /* -post_data and -get_data conflict with curses when interactive
          * so let's force them to dump.  - CL
          */
	dump_output_immediately = TRUE;
        LYcols = 80;
        
	if (strcmp(argv[0], "-get_data") == 0) {
	    StrAllocCopy(form_get_data, "?");   /* Prime the pump */
	    post_data = &form_get_data;
	} else {
	    post_data = &form_post_data;
	}

	/* Build post data for later. Stop reading when we see a line with
	 * "---" as its first three characters.
	 */
	while(fgets(buf, sizeof(buf), stdin) && strncmp(buf, "---", 3) != 0) {
	    int j;
	    
	    for(j = strlen(buf) - 1; j >= 0 && /* Strip line terminators */
		(buf[j] == CR || buf[j] == LF); j--) {
		buf[j] = '\0';
	    }
	    StrAllocCat(*post_data, buf);
	}
	
    } else if (strcmp(argv[0], "-mime_header") == 0) {/* Return mime headers */
	keep_mime_headers = TRUE;
	dump_output_immediately = TRUE;
	HTOutputFormat = HTAtom_for("www/dump");
	LYcols=999;

    } else if (strncmp(argv[0], "-error_file", 11) == 0) { /* Output return
							      (success/failure)
							      code of an HTTP
							      transaction */
	if((cp=strchr(argv[0],'=')) != NULL) {
	    http_error_file = cp+1;
	} else if (argv[1]) {
	    http_error_file = argv[1];
	    *i++;		/* Let master know we've stolen an argument */
	}
	
    } else if (strncmp(argv[0], "-auth", 5) == 0) { /* Authentication
						       information for
						       protected forms */
	char *auth_info = NULL;
	
	if((cp=strchr(argv[0],'=')) != NULL) {
	    StrAllocCopy(auth_info, (cp+1));
	    memset(argv[0], ' ', strlen(argv[0]));/* Let's not show too much */
	} else if (argv[1]) {
	    StrAllocCopy(auth_info, argv[1]);
	    memset(argv[1], ' ', strlen(argv[1]));/* Let's not show too much */
	    *i++;		/* Let master know we've stolen an argument */
	}
	if (auth_info != NULL)  {
	    if ((cp = strchr(auth_info, ':')) != NULL) { /* Pw */
		*cp++ = '\0';	/* Terminate ID */
		authentication_info[1] = HTUnEscape(cp);
	    }
	    authentication_info[0] = HTUnEscape(auth_info); /* Id */
	}
	
    } else if (strcmp(argv[0], "-noredir") == 0) { /* Don't follow URL
						      Redirections */
	no_url_redirection = TRUE;
	 
    } else if (strncmp(argv[0], "-", 1) == 0) {
	if(strncmp(argv[0], "-help", 5) != 0)
//		stop_curses();
//	endwin();
	printf("%s: Invalid Option: %s\n", pgm, argv[0]);
	printf("USAGE: %s [options] [file]\n",pgm);
	printf("Options are:\n");
	printf("    -anonymous       used to specify the anonymous account\n");
	printf("    -auth=id:pw      authentication information for protected forms\n");
	printf("    -case            enable case sensitive user searching\n");
	printf("    -cache=NUMBER    NUMBER of documents cached in memory. (default is %d\n",DEFAULT_CACHE_SIZE);
	printf("    -cfg=FILENAME    specifies a lynx.cfg file other than the default.\n");
	printf("    -display=DISPLAY set the display variable for X execed programs\n");
	printf("    -dump            dump the first file to stdout and exit\n");
	printf("    -editor=EDITOR   enable edit mode with specified editor\n");
	printf("    -emacskeys       enable emacs-like key movement\n");
	printf("    -error_file=file write the HTTP status code here\n");
#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
#ifndef NEVER_ALLOW_REMOTE_EXEC
	printf("    -exec            enable local program execution\n");
#endif /* !NEVER_ALLOW_REMOTE_EXEC */
	printf("    -locexec         enable local program execution from local files only\n");
	printf("    -noexec          disable local program execution (DEFAULT)\n");
#endif /* EXEC_LINKS || EXEC_SCRIPTS */
	printf("    -fake_agent      use the fake user agent string on startup\n");
	printf("    -fileversions    include all versions of files in local VMS directory\n\
		     listings\n");
	printf("    -force_html      forces the first document to be interpreted as HTML\n");
	printf("    -ftp             disable ftp access\n");
	printf("    -get_data        User data for get forms, read from stdin,\n");
	printf("                     terminated by '---' on a line\n");
	printf("    -help            print this usage message\n");
	printf("    -homepage=URL    set homepage separate from start page\n");
	printf("    -image_links     toggle selectable images\n");
	printf("    -index=URL       set the default index file to URL\n");
	printf("    -localhost       disable URLs that point to remote hosts\n");
	printf("    -mime_header     Show full mime header\n");
	printf("    -nobrowse        disable directory browsing\n");
	printf("    -noprint         disable print functions\n");
	printf("    -noredir         Don't follow Location: redirection\n");
	printf("    -nostatus        disable the miscellaneous information messages\n");
	printf("    -post_data       User data for post forms, read from stdin,\n");
	printf("                     terminated by '---' on a line\n");
	printf("    -print           enable print functions (DEFAULT)\n");
	printf("    -restrictions=[options]  use -restrictions to see list\n");
	printf("    -rlogin          disable rlogins\n");
	printf("    -selective       require .www_browsable files to browse directories\n");
	printf("    -show_cursor     don't hide the curser in the lower right corner\n");
	printf("    -source          dump the source of the first file to stdout and exit\n");
#ifdef SYSLOG_REQUESTED_URLSzzz
	printf("    -syslog=text     information for syslog call\n");
#endif /* SYSLOG_REQUESTED_URLS */
	printf("    -telnet          disable telnets\n");
	printf("    -term=TERM       set terminal type to TERM\n");
#ifdef DT
	printf("    -trace           turns on WWW trace mode\n");
#endif
	printf("    -version         print Lynx version information\n");
	printf("    -vikeys          enable vi-like key movement\n");
	exit(0);
    } else {	/* alternate database path */

	StrAllocCopy(startfile, argv[0]);
    }
}
#ifndef VMS

static void FatalProblem ARGS1(int,sig)
{
fprintf (stderr, "\r\nSorry, you have encountered a bug in %s Ver. %s\r\n",
	LYNX_NAME, LYNX_VERSION);

fprintf(stderr, "\r\nAmazingly, you made it to the FatalProblem \
Module!  Please send a concise mail message to\r\n\
doslynx-dev@raven.cc.ukans.edu describing what you were doing,\r\n\
the URL you were looking at or attempting to access,\r\n\
your operating system name with version number,\r\n\
the TCP/IP implementation that your system is using,\r\n\
and any other information you deem relevant.\r\n");


fprintf(stderr, "\r\nDo not mail the core file if one was generated.\r\n");

fprintf(stderr, "\r\nLynx now exiting with signal:  %d\r\n\n", sig);

    /* ignore further interrupts */     /*  mhc: 11/2/91 */
//    (void) signal (SIGHUP, SIG_IGN);
    (void) signal (SIGTERM, SIG_IGN);
#ifndef VMS  /* use ttclose() from cleanup() for VMS */
    (void) signal (SIGINT, SIG_IGN);
#endif /* !VMS */
    (void) signal (SIGSEGV, SIG_IGN);
    (void) signal (SIGILL, SIG_IGN);
    cleanup_sig(0);
    signal (SIGSEGV, 0);
    signal (SIGILL, 0);
    abort();  /* exit and dump core */
}

#endif /* !VMS */
