#include "HTUtils.h"
#include "tcp.h"
#include "LYCurses.h"
#include "GridText.h"
#include "HTAnchor.h"       /* Anchor class */
#include "HTAccess.h"
#include "LYGlobalDefs.h"
#include "LYUtils.h"
#include "LYSignal.h"
#include "LYGetFil.h"
#include "LYPrint.h"
#include "LYHistor.h"
#include "LYString.h"
#include "LYClean.h"
#include "LYDownlo.h"
//#include "LYNews.h"
//#include "LYMail.h"
#include "LYSystem.h"
#include "LYKeymap.h"
#include "LYBookmark.h"

#include <unistd.h>

#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
#include <syslog.h>
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

#ifdef DIRED_SUPPORT
#include "LYLocal.h"

#include "LYLeaks.h"

PRIVATE char * LYSanctify ARGS1(char *, href) 
{
    int i;
    char *p,*cp,*tp;
    char address_buffer[MAXFNAME];

    i = strlen(href) - 1;
    while (i && href[i] == '/') href[i--] = '\0';

    if ((cp = (char *) strchr(href,'~')) != NULL) {
       if (!strncmp(href,"file://localhost/",17))
	 tp = href + 17;
       else 
	 tp = href + 5;
       if ((cp-tp) && *(cp-1) != '/')
	 return href;
       LYstrncpy(address_buffer,href,cp-href);
       if (address_buffer[strlen(address_buffer)-1] == '/')
	 address_buffer[strlen(address_buffer)-1] = '\0';
       p = getenv("HOME");
       strcat(address_buffer,p);
       if (strlen(++cp))
	 strcat(address_buffer,cp);
       if (strcmp(href,address_buffer))
	 StrAllocCopy(href,address_buffer);
    }
    return href;
}

#endif


PRIVATE int fix_http_urls PARAMS((document *doc));
extern char * WWW_Download_File;

PUBLIC BOOLEAN getfile ARGS1(document *,doc)
{
        int url_type;
	char *tmptr;
	DocAddress WWWDoc;  /* a WWW absolute doc address struct */

/*
 *  Reset fake 'Z' to prevent unwanted delayed effect. - kw
 */

 LYFakeZap(NO);



Try_Redirected_URL:
	/* load the WWWDoc struct in case we need to use it */
	WWWDoc.address = doc->address;
        WWWDoc.post_data = doc->post_data;
        WWWDoc.post_content_type = doc->post_content_type;

	/* reset WWW_Download_File just in case */
	free_and_clear(&WWW_Download_File);

#ifdef DT
	if(TRACE) {
	    fprintf(stderr,"LYGetFile: getting %s\n",doc->address);
	}
#endif

	/* check to see if this is a universal document ID
	 * that lib WWW wants to handle
 	 *
	 * some special URL's we handle ourselves :)
	 */

	 if((url_type = is_url(doc->address)) != 0) {
#ifdef TRAVERSAL
		/* only done for traversals IGNORE! */
		if(url_type == HTTP_URL_TYPE) {
		    if(!HTLoadAbsolute(&WWWDoc))
		        return(NOT_FOUND);
		} else {
		    return(NULLFILE);
		}
#else
#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
		syslog(doc->address);
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */
		if(url_type == LYNXPRINT_URL_TYPE) {
		    return(printfile(doc));

#ifdef FIXME
		} else if(url_type == NEWSPOST_URL_TYPE) {

		    if(no_newspost) {
			_statusline("News posting is disabled!");
			sleep(sleep_two);
			return(NULLFILE);
		    } else {
			BOOLEAN followup = FALSE;
			return(LYNewsPost(doc, followup));
		    }

		} else if(url_type == NEWSREPLY_URL_TYPE) {

		    if(no_newspost) {
			_statusline("News posting is disabled!");
			sleep(sleep_two);
			return(NULLFILE);
		    } else {
			BOOLEAN followup = TRUE;
			return(LYNewsPost(doc, followup));
		    }
#endif
		} else if(url_type == LYNXDOWNLOAD_URL_TYPE) {
		    LYDownload(doc->address);
		    return(NORMAL);
#ifdef DIRED_SUPPORT
		} else if(url_type == LYNXDIRED_URL_TYPE) {
		    if (no_dired_support) {
		       _statusline("File management support is disabled!");
		       sleep(sleep_two);
		       return(NULLFILE);
		    } else {
		       local_dired(doc);
		       WWWDoc.address = doc->address;
        	       WWWDoc.post_data = doc->post_data;
        	       WWWDoc.post_content_type = doc->post_content_type;

		       if(!HTLoadAbsolute(&WWWDoc))
		           return(NOT_FOUND);
		       return(NORMAL);
		    }
#endif
		} else if(url_type == LYNXHIST_URL_TYPE) {
			/* doc will change to the new file */
		    historytarget(doc);

			/* we changed it so reload */
		    WWWDoc.address = doc->address;
        	    WWWDoc.post_data = doc->post_data;
        	    WWWDoc.post_content_type = doc->post_content_type;

		    if(!HTLoadAbsolute(&WWWDoc))
		        return(NOT_FOUND);
		    return(NORMAL);

		} else if(url_type == LYNXEXEC_URL_TYPE) {
#ifdef EXEC_LINKS
        	    if (no_exec) {
            	        statusline("Execution has been disabled by system administrator.");
            		sleep(sleep_two);
		    } else if (no_bookmark_exec && bookmark_page &&
		    	       (strstr(HTLoadedDocumentURL(), bookmark_page) ||
			        !strcmp(HTLoadedDocumentTitle(),
					MOSAIC_BOOKMARK_TITLE))) {
			statusline("Execution via bookmarks is disabled.");
			sleep(sleep_two);
		    } else if(local_exec || (local_exec_on_local_files &&
			      exec_ok(HTLoadedDocumentURL(),
				      doc->address+9, EXEC_PATH))) {

			char *cp, *p, addressbuf[MAXFNAME];

			/* Bug puts slash on end if none is in the string */
			char *last_slash = strrchr(doc->address,'/');
			if(last_slash-doc->address==strlen(doc->address)-1)
			    doc->address[strlen(doc->address)-1] = '\0';

			p = doc->address;
#ifndef VMS
			/** On Unix, convert '~' to $HOME **/
			if ((cp = strchr(doc->address, '~'))) {
			    strncpy(addressbuf, doc->address, cp-doc->address);
			    addressbuf[cp - doc->address] = '\0';
			    p = getenv("HOME");
			    strcat(addressbuf, p);
			    strcat(addressbuf, cp+1);
			    p = addressbuf;
			}
#endif /* !VMS */
			/* Show URL before executing it */
			statusline(doc->address);
			sleep(sleep_one);
			stop_curses();
			/* run the command */
			if (strstr(p,"//") == p+9)
			    system(p+11);
			else
			    system(p+9);
			/* Make sure user gets to see screen output */
			printf("\n%s", RETURN_TO_LYNX);
			fflush(stdout);
			LYgetch();
#ifdef VMS
			{
			  extern BOOLEAN HadVMSInterrupt;
			  HadVMSInterrupt = FALSE;
			}
#endif /* VMS */
			start_curses();
			
		     } else {
			char buf[512];

                	sprintf(buf, "Execution is not enabled for this file. See the Options menu (use %s).", key_for_func(LYK_OPTIONS));
			_statusline(buf);
                	sleep(sleep_two);
		     }
#else /* no exec_links */
		     _statusline("Execution capabilities are not compiled into this version.");
		     sleep(sleep_two);
#endif /* EXEC_LINKS */
		     return(NULLFILE);

		} else if(url_type == MAILTO_URL_TYPE) {
                  if(no_mail) {
//                    if(1) {
			_statusline("Mail access is disabled!");
			sleep(sleep_two);
		    } else {
			tmptr = (char *)strchr(doc->address,':')+1;
                      //reply_by_mail(tmptr,"");
		    }
		    return(NULLFILE);

		  /* from here on we could have a remote host,
		   * so check if that's allowed.
		   */
		} else if (local_host_only &&
//			   url_type != NEWS_URL_TYPE &&
			   !LYisLocalHost(doc->address)) {
		    statusline("Only files and servers on the local host can be accessed.");
		    sleep(sleep_two);
		    return(NULLFILE);

		  /* disable www telnet access if not telnet_ok */
		} else if(url_type == TELNET_URL_TYPE || 
			      url_type == TN3270_URL_TYPE ||
			           url_type == TELNET_GOPHER_URL_TYPE) {

		    if(!telnet_ok) {
		    	_statusline("Telnet access is disabled!");
		    	sleep(sleep_two);
		    } else if (no_telnet_port && strchr(doc->address+7, ':')) {
			statusline("Telnet port specifications are disabled.");
			sleep(sleep_two);
		    } else {
			stop_curses();
                        HTLoadAbsolute(&WWWDoc);
			start_curses();
                        fflush(stdout);

		    }

		    return(NULLFILE);

#ifdef FIXME
		/* disable www news access if not news_ok */
		} else if(url_type == NEWS_URL_TYPE && !news_ok) {
		    _statusline("USENET news access is disabled!");
		    sleep(sleep_two);
		    return(NULLFILE);
#endif
		} else if(url_type == RLOGIN_URL_TYPE) {

		    if (!rlogin_ok) {
			statusline("Rlogin access is disabled!");
			sleep(sleep_two);
		    } else {
			stop_curses();
			HTLoadAbsolute(&WWWDoc);
			fflush(stdout);
			start_curses();
		    }
		    return(NULLFILE);

		/* if its a gopher index type and there isn't a search
		 * term already attached then do this.  Otherwise
   		 * just load it!
		 */
		} else if(url_type == INDEX_GOPHER_URL_TYPE &&
					strchr(doc->address,'?') == NULL) {
		    int status;
			/* load it because the do_www_search routine
			 * uses the base url of the currently loaded
			 * document :(
			 */
		    if(!HTLoadAbsolute(&WWWDoc))
			return(NOT_FOUND);
		    status = do_www_search(doc);
		    if(status == NULLFILE) {
			pop(doc);
			WWWDoc.address = doc->address;
			WWWDoc.post_data = doc->post_data;
			WWWDoc.post_content_type = doc->post_content_type;
		        status = HTLoadAbsolute(&WWWDoc);
		    }
		    return(status); 

		} else {

		    if (url_type == FTP_URL_TYPE && !ftp_ok) {
			statusline("Ftp access is disabled!");
			sleep(sleep_two);
			return(NULLFILE);
		    }

		    if(url_type == HTML_GOPHER_URL_TYPE) {
		        char *cp, *tmp=NULL;
		       /*
		        * If tuple's Path=GET%20/... convert to an http URL
		        */ 
		        if((cp=strchr(doc->address+9, '/')) != NULL &&
		           0==strncmp(++cp, "hGET%20/", 8)) {
			    StrAllocCopy(tmp, "http://");
#ifdef DT
			    if(TRACE)
			        fprintf(stderr,
					"LYGetFile: URL %s\nchanged to ",
					doc->address);
#endif

			    *cp = '\0';
			    StrAllocCat(tmp, doc->address+9);
			   /*
			    * If the port is defaulted, it should stay 70
			    */
			    if(strchr(tmp+6, ':') == NULL) {
			        StrAllocCat(tmp, "70/");
				tmp[strlen(tmp)-4] = ':';
			    }
			    if(strlen(cp+7) > 1)
			        StrAllocCat(tmp, cp+8);
			    StrAllocCopy(doc->address, tmp);
#ifdef DT
			    if(TRACE)
			        fprintf(stderr, "%s\n",doc->address);
#endif

			    free(tmp);
			    url_type == HTTP_URL_TYPE;
		        }
		    }

                    if(url_type == HTTP_URL_TYPE || url_type == FTP_URL_TYPE)
			fix_http_urls(doc);
		    WWWDoc.address = doc->address;  /* possible reload */

#ifdef DIRED_SUPPORT
		    lynx_edit_mode = FALSE;
		    if(url_type == FILE_URL_TYPE) {
		        doc->address = LYSanctify(doc->address);
		        WWWDoc.address = doc->address;
		    }
#endif
#ifdef DT
		    if(TRACE)
		        sleep(sleep_two);
#endif

		    user_message(WWW_WAIT_MESSAGE,doc->address);
#ifdef DT
		    if(TRACE)
		        fprintf(stderr,"\n");
#endif

		    if(!HTLoadAbsolute(&WWWDoc)) {
			/* check for redirection
			 */
			if(use_this_url_instead != NULL) {
#ifdef DT
			    if(TRACE)
			        sleep(sleep_two);
#endif

			    _user_message("Using %s", use_this_url_instead);
#ifdef DT
			    if(TRACE)
			        fprintf(stderr,"\n");
#endif

			    StrAllocCopy(doc->address,
					use_this_url_instead);
			    free(use_this_url_instead);
			    use_this_url_instead = NULL;
			    free_and_clear(&doc->post_data);
			    free_and_clear(&doc->post_content_type);
			    /*
			     * Go to top to check for URL's which get
			     * special handling and/or security checks
			     * in Lynx. - FM
			     */
			    goto Try_Redirected_URL;
			}
		        return(NOT_FOUND);
		    }

		    lynx_mode = NORMAL_LYNX_MODE;

		    /* some URL's don't actually return a document
		     * compare doc->address with the document that is 
		     * actually loaded and return NULL if not
		     * loaded.  If www_search_result is not -1
		     * then this is a reference to a named anchor
		     * within the same document.  Do NOT return
		     * NULL
		     */
                    {
                        char *pound;
                        /* check for #selector */
                        pound = (char *)strchr(doc->address, '#');

			/* check to see if there is a temp
			 * file waiting for us to download
			 */
			if(WWW_Download_File) {
				LYdownload_options(&doc->address,
							WWW_Download_File);

		    		WWWDoc.address = doc->address;
		    		free_and_clear(&WWWDoc.post_data);
		    		free_and_clear(&WWWDoc.post_content_type);
				HTOutputFormat = WWW_PRESENT;
				if(!HTLoadAbsolute(&WWWDoc)) 
                        	    return(NOT_FOUND);
				else 
				    return(NORMAL);

			} else if(pound == NULL &&
#ifdef NOT_DEFINED
				  /*
				   * Make this case-insensitive, as in the
				   * HTAnchor hash-table searches - FM
				   */
                                  strcasecomp(doc->address,
						HTLoadedDocumentURL()) ||
#endif /* NOT_DEFINED */
				  /*
				   * HTAnchor hash-table searches are now
				   * case-sensitive (hopefully, without
				   * anchor deletion problems), so this
				   * is too. - FM
				   */
                                  strcmp(doc->address,
						HTLoadedDocumentURL()) ||
				  /*
				   * Also check the post_data elements. - FM
				   */
				  strcmp(doc->post_data ? doc->post_data : "",
				    	 HTLoadedDocumentPost_data())) {
			    /* nothing needed to be shown */
			    return(NULLFILE);

			} else {
			/* may set www_search_result */
			    if(pound != NULL)
				HTFindPoundSelector(pound+1);
			    return(NORMAL);
			}
		    }
		}
#endif /* TRAVERSAL */
	  } else {
#ifdef DT
	      if(TRACE)
		  sleep(sleep_two);
#endif

	      if(strnicmp(doc->address,"news:", 5))
		      _user_message("Badly formed address %s",doc->address);
	      else
		      _user_message("Sorry, News not supported in this version :-( %s", " ");
#ifdef DT
	      if(TRACE)
	          fprintf(stderr,"\n");
#endif

	      sleep(sleep_two);
              return(NULLFILE);
	  }
}

/* the user wants to select a link by number
 * if follow_link_number returns DO_LINK_STUFF do_link will be
 * run immeditely following its execution.
 * if follow_link_number returns PRINT_ERROR an error message will
 * be given to the user, if follow_link_number returns DO_FORMS_STUFF
 * some forms stuff will be done, and if follow_link_number returns
 * DO_NOTHING nothing special will run after it.
 */

PUBLIC int follow_link_number ARGS2(int,c, int *,cur)
{
    char temp[120];
    int link_number;

    temp[0] = c;
    temp[1] = '\0';
    _statusline("follow link number: ");
    /* get the number from the user */
    if (LYgetstr(temp, VISIBLE) <0 || strlen(temp) == 0) {
        _statusline("Cancelled!!!");
        sleep(sleep_one);
        return(DO_NOTHING);
    }

    link_number = atoi(temp);

    if (link_number > 0)  {
              /* get the lname, and hightext,
               * direct from
               * www structures and add it to the cur link
               * so that we can pass it transparently on to
               * get_file()
               * this is done so that you may select a link
               * anywhere in the document, whether it is displayed
               * on the screen or not!
               */
	       if(HTGetLinkInfo(link_number, &links[*cur].hightext, 
					 &links[*cur].lname)) {
                   links[*cur].type = WWW_LINK_TYPE;

		   return(DO_LINK_STUFF);
       		} else {
		   return(PRINT_ERROR);
		}
    } else {
            return(PRINT_ERROR);
    }

}
#if defined(EXEC_LINKS) || defined(LYNXCGI_LINKS)

struct trust {
	char *src;
	char *path;
	int type;
	struct trust *next;
};

static struct trust trusted_exec_default = {
  "file://localhost/",		"",	EXEC_PATH, NULL
};
static struct trust trusted_cgi_default = {
  "",		"",	CGI_PATH, NULL
};

static struct trust *trusted_exec = &trusted_exec_default;
static struct trust *trusted_cgi = &trusted_cgi_default;

PUBLIC void add_trusted ARGS2 (char *,str, int, type)
{
	struct trust *tp;
	char *src, *path;

	src = str;
	path = strchr(src, '\t');
	if (path)
		*path++ = '\0';
	else
		path = "";

	tp = malloc(sizeof(*tp));
	if (tp == NULL) outofmem(__FILE__, "add_trusted");
	tp->src = NULL;
	tp->path = NULL;
	tp->type = type;
	StrAllocCopy(tp->src, src);
	StrAllocCopy(tp->path, path);
	if (type == EXEC_PATH) {
	    if (trusted_exec == &trusted_exec_default)
		tp->next = NULL;
	    else
		tp->next = trusted_exec;
	    trusted_exec = tp;
	} else {
	    if (trusted_cgi == &trusted_cgi_default)
		tp->next = NULL;
	    else
		tp->next = trusted_cgi;
	    trusted_cgi = tp;
	}
}

/*
 * Check to see if the supplied paths is allowed to be executed.
 */
PUBLIC BOOLEAN exec_ok ARGS3(CONST char *,source, CONST char *,link, int, type)
{
    struct trust *tp;
    char CONST *cp;
    
    if (LYJumpFileURL)
	return TRUE;
    
    if (type == EXEC_PATH) {
	tp = trusted_exec;
    } else {
	tp = trusted_cgi;
    }
// #ifndef VMS
#ifdef NOT_DEFINED
    /* security: reject on strange character */
    for (cp = link; *cp != '\0'; cp++) {
	if(!isalnum(*cp) && *cp != '_' && *cp != '-' &&
	   *cp != ' ' && *cp != ':' && *cp != '.' &&
	   *cp != '/' && *cp != '@' && *cp != '~' &&
	   *cp != '$' && *cp != '\t') {
	    char buf[128];

	    sprintf(buf,
		    "Executable link rejected due to `%c' character.",
		    *cp);
	    _statusline(buf);
	    sleep(sleep_two);
	    return FALSE;
	}
    }
// #endif /* !VMS */
#endif

    while (tp) {
	if (tp->type == type) {
	    char CONST *command = link;
	    
	    if (strstr(command,"//") == link) {
		command += 2;
	    }
#ifdef VMS
	    if (strncasecomp(source, tp->src, strlen(tp->src)) == 0 &&
		strncasecomp(command, tp->path, strlen(tp->path)) == 0)
#else
		if (strncmp(source, tp->src, strlen(tp->src)) == 0 &&
		    strncmp(command, tp->path, strlen(tp->path)) == 0)
#endif /* VMS */
		    return TRUE;
	}
	tp = tp->next;
    }
    _statusline("Executable link rejected due to location or path.");
    sleep(sleep_two);
    return FALSE;
}
#endif /* EXEC_LINKS || LYNXCGI_LINKS */

PRIVATE int fix_http_urls ARGS1(document *,doc)
{
   char *slash;

   /* if it's an ftp URL with a trailing slash, trim it off */
   if (!strncmp(doc->address, "ftp", 3) &&
       doc->address[strlen(doc->address)-1] == '/') {
#ifdef DT
           if(TRACE)
               fprintf(stderr,"LYGetFile: URL %s\n", doc->address);
#endif

           doc->address[strlen(doc->address)-1] = '\0';
#ifdef DT
           if(TRACE) {
               fprintf(stderr,"    changed to %s\n", doc->address);
	       sleep(sleep_two);
	   }
#endif

   }

   /* if there isn't a slash besides the two at the beginning, append one */
   if((slash = strrchr(doc->address, '/')) != NULL) 
	if(*(slash-1) != '/' || *(slash-2) != ':')
	     return(0);
#ifdef DT
   if(TRACE)
       fprintf(stderr,"LYGetFile: URL %s\n", doc->address);
#endif

   StrAllocCat(doc->address, "/");
#ifdef DT
   if(TRACE) {
       fprintf(stderr,"    changed to %s\n",doc->address);
       sleep(sleep_two);
   }
#endif

   return(1);
}
