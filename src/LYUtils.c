#include "HTUtils.h"
#include "tcp.h"
#include "LYCurses.h"
#include "HTParse.h"
#include "HTAccess.h"
#include "LYUtils.h"
#include "LYString.h"
#include "LYGlobalDefs.h"
#include "LYSignal.h"
#include "GridText.h"
#ifndef MSDOS
#ifdef UNIX
#include <utmp.h>
#endif /* UNIX */
#endif /* msdos */

#include <time.h>
#include <unistd.h>

#ifdef DISP_PARTIAL
#include "LYKeymap.h"
#endif

// #include <process.h>

#include "LYLeaks.h"

#ifdef SVR4_BSDSELECT
extern int BSDselect PARAMS((int nfds, fd_set * readfds, fd_set * writefds,
	 		     fd_set * exceptfds, struct timeval * timeout));
#ifdef select
#undef select
#endif /* select */
#define select BSDselect
#ifdef SOCKS
#ifdef Rselect
#undef Rselect
#endif /* Rselect */
#define Rselect BSDselect
#endif /* SOCKS */
#endif /* SVR4_BSDSELECT */

#ifndef       FD_SETSIZE
#define       FD_SETSIZE   256
#endif

#ifndef UTMP_FILE
#if defined(__FreeBSD__) || defined(__bsdi__)
#define UTMP_FILE _PATH_UTMP
#else
#define UTMP_FILE "/etc/utmp"
#endif
#endif

/*
 * highlight (or unhighlight) a given link
 */
PUBLIC void highlight ARGS2(int,flag, int,cur)
{
    char buffer[200];
    int i;

    /* Bug in history code can cause -1 to be sent, which will yield
    ** an ACCVIO when LYstrncpy() is called with a nonsense pointer.
    ** This works around the bug, for now. -- FM
    */
    if (cur < 0)
        cur = 0;

    if (nlinks > 0) {
	move(links[cur].ly, links[cur].lx);
	if (flag == ON) { 
	   /* makes some terminals work wrong because
	    * they can't handle two attributes at the 
	    * same time
	    */
	   /* start_bold();  */
	   start_reverse();
      	 } else {
	   start_bold();
	 }

      if(links[cur].type == WWW_FORM_LINK_TYPE) {
	int len;
	int avail_space = (LYcols-links[cur].lx)-1;

        LYstrncpy(buffer,links[cur].hightext, 
			(avail_space > links[cur].form->size ? 
				links[cur].form->size : avail_space));
        addstr(buffer);  
  
	len = strlen(buffer);
	for(; len < links[cur].form->size && len < avail_space; len++)
	    addch('_');

      } else {
								
           /* copy into the buffer only what will fit within the
	    * width of the screen
	    */
	  LYstrncpy(buffer,links[cur].hightext, LYcols-links[cur].lx-1);
          addstr(buffer);  
      }

      /* display a second line as well */
      if(links[cur].hightext2 && links[cur].ly < display_lines) {
	  if (flag == ON)
	     stop_reverse();
	  else 
	     stop_bold();

	  addch('\n');
	  for(i=0; i < links[cur].hightext2_offset; i++)
	 	addch(' ');

	  if (flag == ON)
	     start_reverse();
	  else
	     start_bold();

	  for(i=0; links[cur].hightext2[i] != '\0' &&
			i+links[cur].hightext2_offset < LYcols; i++)
	      if(!IsSpecialAttrChar(links[cur].hightext2[i]))
	           addch((unsigned char)links[cur].hightext2[i]);
      }

      if (flag == ON) 
          stop_reverse();
      else
          stop_bold();

#ifdef FANCY_CURSES
      if(!LYShowCursor)
      {
	  move(LYlines-1,LYcols-1);  /* get cursor out of the way */
      }
      else
      {
#endif /* FANCY CURSES */
	  /* never hide the cursor if there's no FANCY CURSES */
	  move(links[cur].ly, links[cur].lx - 1);
      // }

      if(flag)
          refresh();
    }
    return;
}

/*
 * free_and_clear will free a pointer if it is non-zero and
 * then set it to zero
 */
PUBLIC void free_and_clear ARGS1(char **,pointer)
{
    if(*pointer) {
	free(*pointer);
        *pointer = 0;
    }
    return;
}

/*
 * Collapse (REMOVE) all spaces in the string. 
 */
PUBLIC void collapse_spaces ARGS1(char *,string)
{
    int i=0;
    int j=0;

    if (!string)
        return;

    for(;string[i] != '\0'; i++) 
	if(!isspace(string[i])) 
	    string[j++] = string[i];

    string[j] = '\0';  /* terminate */
    return;
}

/*
 * Convert single or serial newlines to single spaces throughout a string
 * (ignore newlines if the preceding character is a space) and convert
 * tabs to single spaces (but don't ignore any explicit tabs or spaces).
 */
PUBLIC void convert_to_spaces ARGS1(char *,string)
{
    char *s = string;
    char *ns = string;
    BOOL last_is_space = FALSE;

    if (!string)
        return;

    while (*s) {
	switch (*s) {
	    case ' ':
	    case '\t':
		*(ns++) = ' ';
		last_is_space = TRUE;
		break;

	    case '\r':
	    case '\n':
	        if (!last_is_space) {
		    *(ns++) = ' ';
		    last_is_space = TRUE;
		}
		break;

	    default:
		*(ns++) = *s;
		last_is_space = FALSE;
		break;
	}
	s++;
    }
    *ns = '\0';
    return;
}

/*
 * display (or hide) the status line
 */
BOOLEAN mustshow = FALSE;

PUBLIC void statusline ARGS1(char *,text)
{
    char buffer[256];
    extern BOOLEAN no_statusline;
    int max_length;

    if(!text || text==NULL)
	return;

	/* don't print statusline messages if dumping to stdout
	 */
    if(dump_output_immediately)
	return;

    /* Don't print status line if turned off. */
    if(mustshow != TRUE) {
	if(no_statusline == TRUE) {
	    return;
	}
    }
    mustshow = FALSE;

    /* deal with any newlines or tabs in the string */
    LYstrncpy(buffer, text, 255);
    convert_to_spaces(buffer);

    /* make sure text is not longer than the statusline window */
    max_length = ((LYcols - 2) < 256) ? (LYcols - 2) : 255;
    buffer[max_length] = '\0';

    if(user_mode == NOVICE_MODE)
        move(LYlines-3,0);
    else
        move(LYlines-1,0);
    clrtoeol();
    if (text != NULL) {
	start_reverse();
	addstr(buffer);
	stop_reverse();
    }

    refresh();
    return;
}

static char *novice_lines[] = {
#ifndef	NOVICE_LINE_TWO_A
#define	NOVICE_LINE_TWO_A	NOVICE_LINE_TWO
#define	NOVICE_LINE_TWO_B	""
#define	NOVICE_LINE_TWO_C	""
#endif
  NOVICE_LINE_TWO_A,
  NOVICE_LINE_TWO_B,
  NOVICE_LINE_TWO_C,
  ""
};
static int lineno = 0;

PUBLIC void toggle_novice_line NOARGS
{
	lineno++;
	if (*novice_lines[lineno] == '\0')
		lineno = 0;
	return;
}

PUBLIC void noviceline ARGS1(int,more)
{

    if(dump_output_immediately)
	return;

    move(LYlines-2,0);
    stop_reverse();
    clrtoeol();
    addstr(NOVICE_LINE_ONE);
    clrtoeol();

#if defined(DIRED_SUPPORT ) && defined(OK_OVERRIDE)
    if (lynx_edit_mode && !no_dired_support) 
       addstr(DIRED_NOVICELINE);
    else
#endif

    if (LYUseNoviceLineTwo)
        addstr(NOVICE_LINE_TWO);
    else
        addstr(novice_lines[lineno]);

#ifdef NOT
    if(is_www_index && more) {
        addstr("This is a searchable index.  Use ");
	addstr(key_for_func(LYK_INDEX_SEARCH));
	addstr(" to search:");
	stop_reverse();
	addstr("                ");
	start_reverse();
        addstr("space for more");

    } else if(is_www_index) {
        addstr("This is a searchable index.  Use ");
	addstr(key_for_func(LYK_INDEX_SEARCH));
	addstr(" to search:");
    } else {
        addstr("Type a command or ? for help:");                   

        if(more) {
	    stop_reverse();
	    addstr("                       ");
	    start_reverse();
            addstr("Press space for next page");
	}
    }

#endif /* NOT */

    refresh();
    return;
}

PRIVATE int fake_zap = 0;

PUBLIC void LYFakeZap ARGS1(BOOL,     set)
{
    if (set && fake_zap < 1) {
#ifdef DS
	if (TRACE) {
		fprintf(stderr, "\r *** Set simulated 'Z'");
		if (fake_zap)
			fprintf(stderr, ", %d pending", fake_zap);
			fprintf(stderr, " ***\n");
	}
#endif
	fake_zap++;
    } else if (!set && fake_zap) {
#ifdef DS
    if (TRACE) {
	fprintf(stderr, "\r *** Unset simulated 'Z'");
	fprintf(stderr, ", %d pending", fake_zap);
	fprintf(stderr, " ***\n");
    }
#endif
	fake_zap = 0;
    }
}

PUBLIC int HTCheckForInterrupt()
{

#ifdef DISP_PARTIAL
    extern int dp_dl_on, dp_newline ; /* flag to activate hack,
					 position in the document */
#endif

      if(dump_output_immediately)
	  return(FALSE);

      if (fake_zap > 0) {
	fake_zap--;
#ifdef DS
	if (TRACE) {
		fprintf(stderr, "\r *** Got simulated 'Z' ***\n");
		fflush(stderr);
		sleep(AlertSecs);
	}
#endif
	return((int)TRUE);
      }

#ifdef FIXME
      static struct timeval socket_timeout;
      BOOLEAN first=TRUE;
      static int ret=0;
      static fd_set readfds;

      if(first) {
	  socket_timeout.tv_sec = 0;
	  socket_timeout.tv_usec = 100;
      }

      FD_ZERO(&readfds);
      FD_SET(0, &readfds);
#ifdef __hpux
#ifdef SOCKS
      ret = Rselect(FD_SETSIZE, (int *)&readfds, NULL, NULL, &socket_timeout);
#else
      ret = select(FD_SETSIZE, (int *)&readfds, NULL, NULL, &socket_timeout);
#endif /* SOCKS */
#else
#ifdef SOCKS
      ret = Rselect(FD_SETSIZE, &readfds, NULL, NULL, &socket_timeout);
#else
      ret = select(FD_SETSIZE, &readfds, NULL, NULL, &socket_timeout);
#endif /* SOCKS */
#endif

      if(!FD_ISSET(0,&readfds)) {
	 return(FALSE);

      } else
#endif
      {
	 int c;
	 /** Keyboard 'Z' or 'z', or Control-G or Control-C **/
	 nodelay(stdscr,TRUE);
	 c = LYgetch();
	 nodelay(stdscr,FALSE);

	 if(TOUPPER(c) == 'Z' || c == 7 || c == 3)
	    return(TRUE);

#ifdef DISP_PARTIAL    /* hack to display partially downloaded doc. - ganesh */
    else if (dp_dl_on) /* yes, there's a document coming down the line */
      {
	switch (keymap[c+1])
	  {
	  case LYK_PREV_PAGE :
	    if (dp_newline > 1)
	      dp_newline -= display_lines ;
	    break ;
	  case LYK_NEXT_PAGE :
	    if (HText_canScrollDown())
	      dp_newline += display_lines ;
	    break ;
	  case LYK_UP_TWO :
	    if (dp_newline > 1)
	      dp_newline -= 2 ;
	    break ;
	  case LYK_DOWN_TWO :
	    if (HText_canScrollDown())
	      dp_newline += 2 ;
	    break ;
	  case LYK_REFRESH :
	    break ;
	  default :
	    return ((int)FALSE) ;
	  }
	HText_pageDisplay (dp_newline, "") ;
      }
#endif /* DISP_PARTIAL */

	    return(FALSE);
      }

}

/* A file URL for a remote host is an obsolete ftp URL.
 * Return YES only if we're certain it's a local file.
 */
PUBLIC BOOLEAN LYisLocalFile ARGS1(char *,filename)
{
    char *host=NULL;
    char *access=NULL;
    char *cp;

    if (!filename || !(host = HTParse(filename, "", PARSE_HOST)))
	return NO;

    if (!*host) {
        free(host);
	return NO;
    }

    if ((cp=strchr(host, ':')) != NULL)
        *cp = '\0';

    if((access = HTParse(filename, "", PARSE_ACCESS))) {
        if (0==strcmp("file", access) &&
	    (0==strcmp(host, "localhost") ||
#ifdef VMS
             0==strcasecomp(host, HTHostName())))
#else
             0==strcmp(host, HTHostName())))
#endif /* VMS */
        {
	    free(host);
	    free(access);
	    return YES;
	}
    }

    free(host);
    free(access);
    return NO;
}

/* Utility for checking URLs with a host field.
 * Return YES only if we're certain it's the local host.
 */
PUBLIC BOOLEAN LYisLocalHost ARGS1(char *,filename)
{
    char *host=NULL;
    char *cp;

    if (!filename || !(host = HTParse(filename, "", PARSE_HOST)))
        return NO;

    if (!*host) {
        free(host);
	return NO;
    }

    if ((cp=strchr(host, ':')) != NULL)
        *cp = '\0';

    if ((0==strcmp(host, "localhost") ||
#ifdef VMS
	 0==strcasecomp(host, HTHostName()))) {
#else
         0==strcmp(host, HTHostName()))) {
#endif /* VMS */
	    free(host);
	    return YES;
    }

    free(host);
    return NO;
}

/* must recognize a URL and return the type.
 */
PUBLIC int is_url ARGS1(char *,filename)
{
    char *cp=filename;
    char *cp2;

    /* don't crash on an empty argument */
    if (cp == NULL || *cp == '\0')
        return(0);

    /* kill beginning spaces */
    while(isspace(*cp)) cp++;

    if(!strncmp(cp,"mailto:",7)) {
	return(MAILTO_URL_TYPE);

	/* special internal lynx type */
    } else if(!strncmp(cp,"LYNXPRINT:",9)) {
	return(LYNXPRINT_URL_TYPE);


	/* special internal lynx type */
    } else if(!strncmp(cp,"LYNXDOWNLOAD:",9)) {
	return(LYNXDOWNLOAD_URL_TYPE);

#ifdef DIRED_SUPPORT
	/* special internal lynx type */
    } else if(!strncmp(cp,"LYNXDIRED:",9)) {
	return(LYNXDIRED_URL_TYPE);
#endif
	/* special internal lynx type */
    } else if(!strncmp(cp,"LYNXHIST:",9)) {
	return(LYNXHIST_URL_TYPE);

	/* special internal lynx type to handle exec links :(((((((*/
    } else if(!strncmp(cp,"lynxexec:",9)) {
	return(LYNXEXEC_URL_TYPE);

	/* special internal lynx type to handle cgi scripts */
    } else if(!strncmp(cp,"lynxcgi:",8)) {
	return(LYNXCGI_URL_TYPE);

	/* if it doesn't contain ":/" then it can't be a url 
	 * except for the ones above here
	 */
    } else if(!strstr(cp+3,":/")) {  
	return(0);

    } else if(!strncmp(cp,"http",4)) {
	return(HTTP_URL_TYPE);

    } else if(!strncmp(cp,"file",4)) {
        /*
	 *  We won't expend the overhead here of
	 *  determining whether it's really an
	 *  ftp URL unless we are restricting
	 *  ftp access, in which case getfile()
	 *  needs to know in order to issue an
	 *  appropriate statusline message and
	 *  and return NULLFILE.
	 */
        if ((ftp_ok) || LYisLocalFile(cp))
	    return(FILE_URL_TYPE);
	else
	    return(FTP_URL_TYPE);

    } else if(!strncmp(cp,"gopher",6)) {
	if((cp2 = strchr(cp+11,'/')) != NULL) {

	    if(TOUPPER(*(cp2+1)) == 'H' || *(cp2+1) == 'w')
		/* if this is a gopher html type */
	        return(HTML_GOPHER_URL_TYPE);
	    else if(*(cp2+1) == 'T' || *(cp2+1) == '8')
	        return(TELNET_GOPHER_URL_TYPE);
	    else if(*(cp2+1) == '7')
	        return(INDEX_GOPHER_URL_TYPE);
	    else
	        return(GOPHER_URL_TYPE);
	} else {
	    return(GOPHER_URL_TYPE);
	}

    } else if(!strncmp(cp,"ftp",3)) {
	return(FTP_URL_TYPE);

    } else if(!strncmp(cp,"wais",4)) {
	return(WAIS_URL_TYPE);

    } else if(!strncmp(cp,"telnet",6)) {
	return(TELNET_URL_TYPE);

    } else if(!strncmp(cp,"tn3270",6)) {
	return(TN3270_URL_TYPE);

    } else if(!strncmp(cp,"rlogin",6)) {
	return(RLOGIN_URL_TYPE);

    } else if(!strncmp(cp,"afs",3)) {
	return(AFS_URL_TYPE);

    } else if(!strncmp(cp,"prospero",8)) {
	return(PROSPERO_URL_TYPE);

    } else {
	return(0);
    }
}

/*
 * remove backslashes from any string
 */

PUBLIC void remove_backslashes ARGS1(char *,buf)
{
    char *cp;

    for (cp=buf; *cp != '\0' ; cp++) {

	if(*cp != '\\')  /* don't print slashes */
	   *buf = *cp, 
	   buf++;
	else if(*cp == '\\' &&  *(cp+1) == '\\') /*print one slash if there*/
	   *buf = *cp,                        /* are two in a row */
	   buf++;
    }
    *buf = '\0';
    return;
}

/* Quote the path to make it safe for shell command processing. */
/* We use a simple technique which involves quoting the entire
   string using single quotes, escaping the real single quotes
   with double quotes. This may be gross but it seems to work. */

   /* not for DOS we don't */

PUBLIC char * quote_pathname ARGS1 (char *, pathname)
{
   int i,n=0;
   char * result;

   for (i=0; i<strlen(pathname); ++i)
     if (pathname[i] == '\'') ++n;

   result = (char *) malloc(strlen(pathname) + 5*n + 3);
   if (result == NULL) outofmem(__FILE__, "quote_pathname");

   result[0] = '\"';
   for (i=0,n=1; i<strlen(pathname); i++)
     if (pathname[i] == '\"') {
	n++;
/*
	result[n++] = '\'';
	result[n++] = '"';
	result[n++] = '\'';
	result[n++] = '"';
	result[n++] = '\'';
*/
     } else
	result[n++] = pathname[i];
   result[n++] = '\"';
   result[n] = '\0';
   return result;
}

/*
 * checks to see if the current process is attached via a terminal in the
 * local domain
 *
 */
#if defined(VMS) || defined(SGI) || defined(SCO) || defined(MSDOS) || defined(__ELKS__) || defined(__linux__)
#ifndef NO_UTMP
#define NO_UTMP
#endif /* NO_UTMP */
#endif /* VMS || SGI || SCO */

PUBLIC BOOLEAN inlocaldomain ()
{
#ifdef NO_UTMP
    return(TRUE);
#else
    int n;
    FILE *fp;
    struct utmp me;
    char *cp, *mytty=NULL;
    char *ttyname();

    if ((cp=ttyname(0)))
	mytty = strrchr(cp, '/');

    if (mytty && (fp=fopen(UTMP_FILE, "r")) != NULL) {
	    mytty++;
	    do {
		n = fread((char *) &me, sizeof(struct utmp), 1, fp);
	    } while (n>0 && !STREQ(me.ut_line,mytty));
	    (void) fclose(fp);

	    if (n > 0 &&
	        strlen(me.ut_host) > strlen(LOCAL_DOMAIN) &&
	        STREQ(LOCAL_DOMAIN,
		  me.ut_host+strlen(me.ut_host)-strlen(LOCAL_DOMAIN)) )
		return(TRUE);
#ifdef LINUX
/* Linux fix to check for local user. J.Cullen 11Jul94          */
		if((n > 0) && (strlen(me.ut_host) == 0))
			return(TRUE);
#endif /* LINUX */

    }
#ifdef DT
    else {
	if(TRACE)
	   fprintf(stderr,"Could not get ttyname or open UTMP file");
    }
#endif

    return(FALSE);
#endif /* NO_UTMP */
}

/**************
** This bit of code catches window size change signals
**/

#ifdef VMS
#define NO_SIZECHANGE
#endif
#ifdef SNAKE
#define NO_SIZECHANGE
#endif

#if !defined(VMS) && !defined(ISC) && !defined(MSDOS)
#include <sys/ioctl.h>
#endif


PUBLIC void size_change ARGS1(int,sig)
{
#ifndef NO_SIZECHANGE
#ifdef TIOCGSIZE
        struct ttysize win;
#else
#  ifdef TIOCGWINSZ
        struct winsize win;
#  endif
#endif

#ifdef TIOCGSIZE
        if (ioctl (0, TIOCGSIZE, &win) == 0) {
                if (win.ts_lines != 0) {
                        LYlines = win.ts_lines - 1;
                }
                if (win.ts_cols != 0) {
                        LYcols = win.ts_cols;
                }
        }
#else
#  ifdef TIOCGWINSZ
        if (ioctl (0, TIOCGWINSZ, &win) == 0) {
                if (win.ws_row != 0) {
                        LYlines = win.ws_row - 1;
                }
                if (win.ws_col != 0) {
                        LYcols = win.ws_col;
                }
        }
#  endif
#endif

#endif /* NO_SIZECHANGE */
     recent_sizechange=TRUE; 
     return;
}

/*
 *  CHANGE_SUG_FILENAME -- Foteos Macrides 29-Dec-1993
 *	Upgraded for use with Lynx2.2 - FM 17-Jan-1994
 */

PUBLIC void change_sug_filename ARGS1(char *,fname)
{
     char    *cp, *cp1, *end;
#ifdef VMS
     char *dot;
     int j,k;
#endif /* VMS */

     /*** establish the current end of fname ***/
     end = fname + strlen(fname);

     /*** unescape fname ***/
     HTUnEscape(fname);

     /*** remove everything up the the last_slash if there is one ***/
     if((cp = strrchr(fname,'/')) != NULL && strlen(cp) > 1) {
	 cp1=fname;
	 cp++; /* go past the slash */
	 for(; *cp != '\0'; cp++, cp1++)
	    *cp1 = *cp;

	 *cp1 = '\0'; /* terminate */
     }

     /*** Trim off date-size suffix, if present ***/
     if ((*(end - 1) == ']') && ((cp = strrchr(fname, '[')) != NULL) &&
         (cp > fname) && *(--cp) == ' ')
	  while (*cp == ' ')
	       *(cp--) = '\0';

     /*** Trim off VMS device and/or directory specs, if present ***/
     if ((cp=strchr(fname,'[')) != NULL &&
         (cp1=strrchr(cp,']')) != NULL && strlen(cp1) > 1) {
	  cp1++;
	  for (cp=fname; *cp1 != '\0'; cp1++)
	       *(cp++) = *cp1;
	  *cp = '\0';
     }

#ifdef VMS
     /*** Replace illegal or problem characters ***/
     dot = fname + strlen(fname);
     for (cp = fname; cp < dot; cp++) {

	  /** Replace with underscores **/
	  if (*cp == ' ' || *cp == '/' || *cp == ':' ||
	      *cp == '[' || *cp == ']')
	       *cp = '_';

	  /** Replace with dashes **/
	  else if (*cp == '!' || *cp == '?' || *cp == '\'' || 
	           *cp == ',' || *cp == ':' || *cp == '\"' ||
	           *cp == '+' || *cp == '@' || *cp == '\\' ||
	           *cp == '(' || *cp == ')' || *cp == '=' ||
	           *cp == '<' || *cp == '>' || *cp == '#' ||
	           *cp == '%' || *cp == '*' || *cp == '`' ||
	           *cp == '~' || *cp == '^' || *cp == '|')
	       *cp = '-';
     }

     /** Collapse any serial underscores **/
     cp = fname + 1;
     j = 0;
     while (cp < dot) {
	  if (fname[j] == '_' && *cp == '_')
	       cp++;
	  else
	       fname[++j] = *cp++;
     }
     fname[++j] = '\0';

     /** Collapse any serial dashes **/
     dot = fname + (strlen(fname));
     cp = fname + 1;
     j = 0;
     while (cp < dot) {
          if (fname[j] == '-' && *cp == '-')
	       cp++;
	  else
	       fname[++j] = *cp++;
     }
     fname[++j] = '\0';

     /** Trim any trailing or leading **/
     /** underscrores or dashes       **/
     cp = fname + (strlen(fname)) - 1;
     while (*cp == '_' || *cp == '-')
          *cp-- = '\0';
     if (fname[0] == '_' || fname[0] == '-') {
          dot = fname + (strlen(fname));
          cp = fname;
          while ((*cp == '_' || *cp == '-') && cp < dot)
	       cp++;
	  j = 0;
          while (cp < dot)
	       fname[j++] = *cp++;
	  fname[j] = '\0';
     }

     /** Replace all but the last period with _'s, or second **/
     /** to last if last is followed by a terminal Z or z,   **/
     /** e.g., convert foo.tar.Z to                          **/
     /**               foo.tar_Z                             **/
     j = strlen(fname) - 1;
     if ((dot = strrchr(fname, '.')) != NULL) {
	  if (((fname[j] == 'Z' || fname[j] == 'z') && fname[j-1] == '.') &&
	      (((cp = strchr(fname, '.')) != NULL) && cp < dot)) {
	       *dot = '_';
	       dot = strrchr(fname, '.');
	  }
	  cp = fname;
	  while ((cp = strchr(cp, '.')) != NULL && cp < dot)
	       *cp = '_';

          /** But if the root is > 39 characters, move **/
          /** the period appropriately to the left     **/
	  while (dot - fname > 39) {
	       *dot = '\0';
	       if ((cp = strrchr(fname, '_')) != NULL) {
		    *cp  = '.';
		    *dot = '_';
	       } 
	       else if ((cp = strrchr(fname, '-')) != NULL) {
		    *cp  = '.';
		    *dot = '_';
	       }
	       else {
		    *dot = '_';
		    j = strlen(fname);
		    fname[j+1] = '\0';
		    while (j > 39)
			 fname[j--] = fname[j];
		    fname[j] = '.';
	       }
               dot = strrchr(fname, '.');
	  }

          /** Make sure the extension is < 40 characters **/
          if ((fname + strlen(fname) - dot) > 39)
	       *(dot+40) = '\0';

	  /** Trim trailing dashes or underscores **/
	  j = strlen(fname) - 1;
	  while (fname[j] == '_' || fname[j] == '-')
	       fname[j--] = '\0';
     }
     else {
	  /** No period, so put one on the end, or after   **/
	  /** the 39th character, trimming trailing dashes **/
	  /** or underscrores                              **/
	  if (strlen(fname) > 39)
	       fname[39] = '\0';
	  j = strlen(fname) - 1;
	  while ((fname[j] == '_') || (fname[j] == '-'))
	       j--;
	  fname[++j] = '.';
	  fname[++j] = '\0';
     }

#else /* not VMS  (UNIX) */
     /*** Replace problem characters ***/
     for (cp = fname; *cp != '\0'; cp++) {
	  switch (*cp) {
	  case '\'':
	  case '\"':
	  case '/':
	  case ' ':
	       *cp = '-';
	  }
     }
#endif /* VMS  (UNIX) */

     /** Make sure the rest of the original string in nulled. **/
     cp = fname + strlen(fname);
     while (cp < end)
          *cp++ = '\0';

    return;
}

/*
 *	To create standard temporary file names
 */
PUBLIC void tempname ARGS2(char *,namebuffer, int,action)
{
	static int counter = 0;


	if(action == REMOVE_FILES) { /* REMOVE ALL FILES */ 
	    for(; counter > 0; counter--) {
                sprintf(namebuffer, "%sL%d%u.htm", TEMP_SPACE, getpid(), 
								counter-1);
#ifdef SCO
		unlink(namebuffer);
#else
		remove(namebuffer);
#endif /* SCO */
	    }
        } else if(action == 2) { /* No ext */

	/*
	 * 	Create name
	 */
            sprintf(namebuffer, "%sL%d%u", TEMP_SPACE, getpid(), 
								counter++);
	} else /* add a file */ {
	/*
	 * 	Create name
	 */
            sprintf(namebuffer, "%sL%d%u.htm", TEMP_SPACE, getpid(), 
								counter++);
	}
	return;
}

/* convert 4, 6, 2, 8 to left, right, down, up, etc. */
PUBLIC int number2arrows ARGS1(int,number)
{
      switch(number) {
            case '1':
                number=END;
                  break;
            case '2':
                number=DNARROW;
                  break;
            case '3':
                number=PGDOWN;
                  break;
            case '4':
                number=LTARROW;
                  break;
	    case '5':
		number=DO_NOTHING;
		break;
            case '6':
                number=RTARROW;
                  break;
            case '7':
                number=HOME;
                  break;
 	    case '8':
                number=UPARROW;
                  break;
            case '9':
                number=PGUP;
                  break;
      }

      return(number);
}

/*
 * parse_restrictions takes a string of comma-separated restrictions
 * and sets the corresponding flags to restrict the facilities available
 */
PRIVATE char *restrict_name[] = {
       "inside_telnet" ,
       "outside_telnet",
       "telnet_port"   ,
       "inside_ftp"    ,
       "outside_ftp"   ,
       "inside_rlogin" ,
       "outside_rlogin",
       "suspend"       ,
       "editor"        ,
       "shell"         ,
       "bookmark"      ,
       "option_save"   ,
       "print"         ,
       "download"      ,
       "disk_save"     ,
       "exec"          ,
       "lynxcgi"       ,
       "exec_frozen"   ,
       "bookmark_exec" ,
       "goto"          ,
       "jump"          ,
       "file_url"      ,
       "mail"          ,
#ifdef DIRED_SUPPORT
       "dired_support" ,
#endif
       (char *) 0     };

	/* restrict_name and restrict_flag structure order
	 * must be maintained exactly!
	 */

PRIVATE BOOLEAN *restrict_flag[] = {
       &no_inside_telnet,
       &no_outside_telnet,
       &no_telnet_port,
       &no_inside_ftp,
       &no_outside_ftp,
       &no_inside_rlogin,
       &no_outside_rlogin,
       &no_suspend  ,
       &no_editor   ,
       &no_shell    ,
       &no_bookmark ,
       &no_option_save,
       &no_print    ,
       &no_download ,
       &no_disk_save,
       &no_exec     ,
       &no_lynxcgi  ,
       &exec_frozen ,
       &no_bookmark_exec,
       &no_goto     ,
       &no_jump     ,
       &no_file_url ,
       &no_mail     ,
#ifdef DIRED_SUPPORT
       &no_dired_support,
#endif
       (BOOLEAN *) 0  };

PUBLIC void parse_restrictions ARGS1(char *,s)
{
      char *p;
      char *word;
      int i;

      if (STREQ("all", s)) {
	   /* set all restrictions */
          for(i=0; restrict_flag[i]; i++) 
              *restrict_flag[i] = TRUE;
          return;
      }

      if (STREQ("default", s)) {
	   /* set all restrictions */
          for(i=0; restrict_flag[i]; i++) 
              *restrict_flag[i] = TRUE;

	     /* reset these to defaults */
             no_inside_telnet = !(CAN_ANONYMOUS_INSIDE_DOMAIN_TELNET);
            no_outside_telnet = !(CAN_ANONYMOUS_OUTSIDE_DOMAIN_TELNET);
                     no_print = !(CAN_ANONYMOUS_PRINT);
                      no_mail = !(CAN_ANONYMOUS_MAIL);
		      no_goto = !(CAN_ANONYMOUS_GOTO);
		      no_jump = !(CAN_ANONYMOUS_JUMP);
#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
		      no_exec = LOCAL_EXECUTION_LINKS_ALWAYS_OFF_FOR_ANONYMOUS;
#endif
                no_inside_ftp = !(CAN_ANONYMOUS_INSIDE_DOMAIN_FTP);
               no_outside_ftp = !(CAN_ANONYMOUS_OUTSIDE_DOMAIN_FTP);
             no_inside_rlogin = !(CAN_ANONYMOUS_INSIDE_DOMAIN_RLOGIN);
            no_outside_rlogin = !(CAN_ANONYMOUS_OUTSIDE_DOMAIN_RLOGIN);
          return;
      }

      p = s;
      while (*p) {
          while (isspace(*p))
              p++;
          if (*p == '\0')
              break;
          word = p;
          while (*p != ',' && *p != '\0')
              p++;
          if (*p)
              *p++ = '\0';

	  for(i=0; restrict_name[i]; i++) 
             if(STREQ(word, restrict_name[i])) {
                *restrict_flag[i] = TRUE;
		break;
	     }
      }
      return;
}

#ifdef VMS
#include <jpidef.h>
#include <maildef.h>
#include <starlet.h>

typedef struct _VMSMailItemList
{
  short buffer_length;
  short item_code;
  void *buffer_address;
  long *return_length_address;
} VMSMailItemList;

PUBLIC int LYCheckMail NOARGS
{
    static BOOL firsttime = TRUE, failure = FALSE;
    static char user[13], dir[252];
    static long userlen = 0, dirlen;
    static time_t lastcheck = 0;
    time_t now;
    static short new, lastcount;
    long ucontext = 0, status;
    short flags = MAIL$M_NEWMSG;
    VMSMailItemList
      null_list[] = {{0,0,0,0}},
      jpi_list[]  = {{sizeof(user) - 1,JPI$_USERNAME,(void *)user,&userlen},
		     {0,0,0,0}},
      uilist[]    = {{0,MAIL$_USER_USERNAME,0,0},
    		     {0,0,0,0}},
      uolist[]    = {{sizeof(new),MAIL$_USER_NEW_MESSAGES,&new,0},
                     {sizeof(dir),MAIL$_USER_FULL_DIRECTORY,dir,&dirlen},
                     {0,0,0,0}};
    extern long mail$user_begin();
    extern long mail$user_get_info(); 
    extern long mail$user_end();

    if (failure)
        return 0;

    if (firsttime) {
        firsttime = FALSE;
        /* Get the username. */
        status = sys$getjpiw(0,0,0,jpi_list,0,0,0);
        if (!(status & 1)) {
            failure = TRUE;
            return 0;
        }
        user[userlen] = '\0';
        while(isspace(user[--userlen])) /* suck up trailing spaces */
            user[userlen] = '\0';
    }

    /* Minimum report interval is 60 sec. */
    time(&now);
    if (now - lastcheck < 60)
	return 0;
    lastcheck = now;

    /* Get the current newmail count. */
    status = mail$user_begin(&ucontext,null_list,null_list);
    if (!(status & 1)) {
        failure = TRUE;
        return 0;
    }
    uilist[0].buffer_length = strlen(user);
    uilist[0].buffer_address = user;
    status = mail$user_get_info(&ucontext,uilist,uolist);
    if (!(status & 1)) {
        failure = TRUE;
        return 0;
    }

    /* Should we report anything to the user? */
    if (new > 0) {
	if (lastcount == 0)
	    /* Have newmail at startup of Lynx. */
	    _statusline("*** You have unread mail. ***");
	else if (new > lastcount)
	    /* Have additional mail since last report. */
	    _statusline("*** You have new mail. ***");
        lastcount = new;
	return 1;
    }
    lastcount = new;

    /* Clear the context */
    mail$user_end((long *)&ucontext,null_list,null_list);
    return 0;
}
#else
PUBLIC int LYCheckMail NOARGS
{
    static BOOL firsttime = TRUE;
    static char *mf;
    static time_t lastcheck;
    static long lastsize;
    time_t now;
    struct stat st;

    if (firsttime) {
	mf = getenv("MAIL");
	firsttime = FALSE;
    }

    if (mf == NULL)
	return 0;

    time(&now);
    if (now - lastcheck < 60)
	return 0;
    lastcheck = now;

    if (stat(mf,&st) < 0) {
	mf = NULL;
	return 0;
    }

    if (st.st_size > 0) {
	if (st.st_mtime > st.st_atime ||
	    (lastsize && st.st_size > lastsize))
	    _statusline("*** You have new mail. ***");
	else if (lastsize == 0)
	    _statusline("*** You have mail. ***");
        lastsize = st.st_size;
	return 1;
    }
    lastsize = st.st_size;
    return 0;
}
#endif /* VMS */

#ifdef VMS
/*
 *  Define_VMSLogical -- Fote Macrides 04-Apr-1995
 *	Define VMS logicals in the process table.
 */
#include <descrip.h>
#include <libclidef.h>
#include <lib$routines.h>

PUBLIC void Define_VMSLogical ARGS2(char *, LogicalName, char *, LogicalValue)
{
    $DESCRIPTOR(lname, "");
    $DESCRIPTOR(lvalue, "");
    $DESCRIPTOR(ltable, "LNM$PROCESS");

    lname.dsc$w_length = strlen(LogicalName);
    lname.dsc$a_pointer = LogicalName;
    lvalue.dsc$w_length = strlen(LogicalValue);
    lvalue.dsc$a_pointer = LogicalValue;
    lib$set_logical(&lname, &lvalue, &ltable, 0, 0);
    return;
}
#endif /* VMS */

#ifdef NO_PUTENV
/* no putenv on the next so we use this code instead!
 */

/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can  redistribute it and/or
modify it under the terms of the GNU Library General  Public License as
published by the Free Software Foundation; either  version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it  will be useful,
but WITHOUT ANY WARRANTY; without even the implied  warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library  General Public
License along with the GNU C Library; see the file  COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675  Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sys/types.h>
#include <errno.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#else
extern int errno;
#endif

#if defined(STDC_HEADERS) || defined(USG)
#include <string.h>
#define index strchr
#define bcopy(s, d, n) memcpy((d), (s), (n))
#else /* not (STDC_HEADERS or USG) */
#include <strings.h>
#endif /* STDC_HEADERS or USG */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef NULL
#define NULL 0
#endif

#if !__STDC__
#define const
#endif

extern char **environ;

/* Put STRING, which is of the form "NAME=VALUE", in  the environment.  */
int
putenv (string)
     const char *string;
{
  char *name_end = index (string, '=');
  register size_t size;
  register char **ep;

  if (name_end == NULL)
    {
      /* Remove the variable from the environment.  */
      size = strlen (string);
      for (ep = environ; *ep != NULL; ++ep)
	if (!strncmp (*ep, string, size) && (*ep)[size]  == '=')
	  {
	    while (ep[1] != NULL)
	      {
		ep[0] = ep[1];
		++ep;
	      }
	    *ep = NULL;
	    return 0;
	  }
    }

  size = 0;
  for (ep = environ; *ep != NULL; ++ep)
    if (!strncmp (*ep, string, name_end - string) && (*ep)[name_end - string] == '=')
      break;
    else
      ++size;

  if (*ep == NULL)
    {
      static char **last_environ = NULL;
      char **new_environ = (char **) malloc ((size + 2)  * sizeof (char *));
      if (new_environ == NULL)
	return -1;
      (void) bcopy ((char *) environ, (char *)  new_environ, size * sizeof (char *));
      new_environ[size] = (char *) string;
      new_environ[size + 1] = NULL;
      if (last_environ != NULL)
	free ((char *) last_environ);
      last_environ = new_environ;
      environ = new_environ;
    }
  else
    *ep = (char *) string;

  return 0;
}
#endif /* NO Putenv */
