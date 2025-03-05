/*	HyperText Tranfer Protocol	- Client implementation		HTTP.c
**	==========================
** Modified:
** 27 Jan 1994  PDM  Added Ari Luotonen's Fix for Reload when using proxy
**                   servers.
*/

#include "HTUtils.h"
#include "tcp.h"

#include "HTTP.h"

#define HTTP_VERSION	"HTTP/1.0"

#define INIT_LINE_SIZE		1024	/* Start with line buffer this big */
#define LINE_EXTEND_THRESH	256	/* Minimum read size */
#define VERSION_LENGTH 		20	/* for returned protocol version */

#include "HTParse.h"
#include "HTTCP.h"
#include "HTFormat.h"
#include "HTFile.h"
#include <ctype.h>
#include "HTAlert.h"
#include "HTMIME.h"
#include "HTML.h"
#include "HTInit.h"
#include "HTAABrow.h"

#include "gridtext.h"

#ifdef MSDOS
#include "LYUtils.h"
#endif

#include "LYLeaks.h"

#ifdef NO_BCOPY
#define bcopy(s, d, n) memcpy((d), (s), (n))
#endif /* NO_BCOPY */

#ifdef DISP_PARTIAL
int dp_dl_on = 0 ; /* global which activates the paging in HTCheckForInterrupt
	      when a file is being downloaded */
int dp_newline = 1 ; /* newline for paging during downloads */
#endif

struct _HTStream
{
  HTStreamClass * isa;
};

extern char * LYUserAgent;      /* Lynx User-Agent string */   
extern char * HTAppName;	/* Application name: please supply */
extern char * HTAppVersion;	/* Application version: please supply */
extern char * personal_mail_address;	/* User's name/email address */

extern BOOL using_proxy;	/* Are we using an HTTP gateway? */
PUBLIC BOOL reloading = FALSE;	/* Reloading => send no-cache pragma to proxy */

extern char LYUserSpecifiedURL; /* Is the URL a goto? */

extern BOOL keep_mime_headers;   /* Return mime headers */
extern BOOL no_url_redirection;  /* Don't follow Location: URL for */
extern char *http_error_file;    /* Store HTTP status code in this file */

BOOL do_head = FALSE;		/* Whether or not we should do a head */

/*		Load Document from HTTP Server			HTLoadHTTP()
**		==============================
**
**	Given a hypertext address, this routine loads a document.
**
**
** On entry,
**	arg	is the hypertext reference of the article to be loaded.
**
** On exit,
**	returns	>=0	If no error, a good socket number
**		<0	Error.
**
**	The socket must be closed by the caller after the document has been
**	read.
**
*/
PUBLIC int HTLoadHTTP ARGS4 (
	CONST char *, 		arg,
	HTParentAnchor *,	anAnchor,
	HTFormat,		format_out,
	HTStream*,		sink)
{
  int s;				/* Socket number for returned data */
  char *command;			/* The whole command */
  char *eol;			/* End of line if found */
  char *start_of_data;		/* Start of body of reply */
  int status;				/* tcp return */
  long bytes_already_read;
  char crlf[3];			/* A CR LF equivalent string */
  HTStream *target;		/* Unconverted data */
  HTFormat format_in;			/* Format arriving in the message */
  int do_post = 0;		/* ARE WE posting ? */

  BOOL had_header;		/* Have we had at least one header? */
  char *line_buffer;
  char *line_kept_clean;
  BOOL extensions;		/* Assume good HTTP server */
  int compressed;
  char line[256];
  
  int length, doing_redirect, rv;
  int already_retrying = 0;

  if(anAnchor->post_data)
      do_post=TRUE;
  
  if (!arg)
    {
      status = -3;
      _HTProgress ("Bad request.");
      goto done;
    }
  if (!*arg)
    {
      status = -2;
      _HTProgress ("Bad request.");
      goto done;
    }
  
  sprintf(crlf, "%c%c", CR, LF);

  /* At this point, we're talking HTTP/1.0. */
  extensions = YES;

 try_again:
  /* All initializations are moved down here from up above,
     so we can start over here... */
  eol = 0;
  bytes_already_read = 0;
  had_header = NO;
  length = 0;
  doing_redirect = 0;
  compressed = 0;
  target = NULL;
  line_buffer = NULL;
  line_kept_clean = NULL;

  status = HTDoConnect (arg, "HTTP", TCP_PORT, &s);
  if (status == HT_INTERRUPTED)
    {
      /* Interrupt cleanly. */
#ifdef DT
      if (TRACE)
        fprintf (stderr,
                 "HTTP: Interrupted on connect; recovering cleanly.\n");
#endif

      _HTProgress ("Connection interrupted.");
      /* status already == HT_INTERRUPTED */
      goto done;
    }
  if (status < 0) 
    {
#ifdef DT
      if (TRACE) 
        fprintf(stderr, 
                "HTTP: Unable to connect to remote host for `%s' (errno = %d).\n", arg, SOCKET_ERRNO);
#endif

      HTAlert("Unable to connect to remote host.");
      status = HT_NO_DATA;
      goto done;
    }
  
  /*	Ask that node for the document,
   **	omitting the host name & anchor
   */        
  {
    char * p1 = HTParse(arg, "", PARSE_PATH|PARSE_PUNCTUATION);
    command = malloc(5 + strlen(p1)+ 2 + 31 + 
		/* Referer: field */
		(LYUserSpecifiedURL ? 0 : 
		    (strlen((char *)HTLoadedDocumentURL()) + 10)));

    if (do_post)
      strcpy(command, "POST ");
    else if(do_head)
	strcpy(command, "HEAD ");
    else
      strcpy(command, "GET ");

    /* if we are using a proxy gateway don't copy in the first slash
     * of say: /gopher://a;lkdjfl;ajdf;lkj/;aldk/adflj
     * so that just gopher://.... is sent.
     */
    if(using_proxy)
        strcat(command, p1+1);
    else
        strcat(command, p1);
    free(p1);
  }
  if (extensions) 
    {
      strcat(command, " ");
      strcat(command, HTTP_VERSION);
    }
  
  strcat(command, crlf);	/* CR LF, as in rfc 977 */
  
  if (extensions) 
    {
      int n, i;
      extern char * language; /* Lynx's preferred language - FM */
      
        /* send host:port for virtual domains WB */

    {
	char *colon=NULL;
	char *hostname=NULL;
	int portnumber;
	char *targ=NULL;

	if(using_proxy == TRUE)	{
		targ = strstr(arg,"/");
		targ++;
		targ++;
		targ = strstr(targ,"/");
		targ++;
	}
	else targ = arg;

	hostname = HTParse(targ, "", PARSE_HOST);
	if (hostname &&
	    NULL != (colon = strchr(hostname, ':'))) {
	    *(colon++) = '\0';	/* Chop off port number */
	    portnumber = atoi(colon);
	}
	else portnumber = 80;

	sprintf(line, "Host: %s:%i%c%c", hostname, portnumber, CR, LF);

	StrAllocCat(command, line);

	FREE(hostname);
    }


      if (!HTPresentations) HTFormatInit();
      n = HTList_count(HTPresentations);
      
      for(i=0; i<n; i++) 
        {
	  HTPresentation * pres = HTList_objectAt(HTPresentations, i);
          if (pres->rep_out == WWW_PRESENT)
            {
	      if(pres->rep == WWW_SOURCE) 
		  sprintf(line, "Accept: */*%c%c", CR, LF);
  	      else
		  sprintf(line, "Accept: %s%c%c",
					HTAtom_name(pres->rep), CR, LF);
              StrAllocCat(command, line);
            }
	}

      if (language) {
	  sprintf(line, "Accept-Language: %s; q=1%c%c", language, CR, LF);
	  StrAllocCat(command, line);
	  sprintf(line, "Accept-Language: *; q=0.1%c%c", CR, LF);
	  StrAllocCat(command, line);
      }

      /*
       * When reloading give no-cache pragma to proxy server to make
       * it refresh its cache. -- Ari L. <luotonen@dxcern.cern.ch>
       */
      if (reloading) {
          sprintf(line, "Pragma: no-cache%c%c", CR, LF);
          StrAllocCat(command, line);
      }
      reloading = FALSE;           /* Now turn it off again if on */

      if (LYUserAgent && *LYUserAgent) {
          sprintf(line, "User-Agent: %s%c%c", LYUserAgent, CR, LF);
      } else {
          sprintf(line, "User-Agent:  %s/%s  libwww/%s%c%c",
	      HTAppName ? HTAppName : "unknown",
              HTAppVersion ? HTAppVersion : "0.0",
	      HTLibraryVersion, CR, LF);
      }
      StrAllocCat(command, line);

/*    I don't like giving out my personal mail address
      to every single site I browse.

      if(personal_mail_address) {
	  sprintf(line, "From:  %s%c%c", personal_mail_address, CR,LF);
	  StrAllocCat(command, line);
      }
*/

      if(!LYUserSpecifiedURL) {
	  StrAllocCat(command, "Referer:  ");
	  StrAllocCat(command, (char *)HTLoadedDocumentURL());
          sprintf(line, "%c%c", CR, LF);
          StrAllocCat(command, line);
      }
      
      {
        char *docname;
	char *hostname;
	char *colon;
	int portnumber;
	char *auth;

	docname = HTParse(arg, "", PARSE_PATH);
        hostname = HTParse(arg, "", PARSE_HOST);
        if (hostname &&
            NULL != (colon = strchr(hostname, ':'))) 
	  {
            *(colon++) = '\0';	/* Chop off port number */
            portnumber = atoi(colon);
          }
        else portnumber = 80;
        
        if (NULL!=(auth=HTAA_composeAuth(hostname, portnumber, docname))) 
	  {
            sprintf(line, "%s%c%c", auth, CR, LF);
            StrAllocCat(command, line);
          }
#ifdef DT
        if (TRACE) 
	  {
            if (auth)
              fprintf(stderr, "HTTP: Sending authorization: %s\n", auth);
	    else
              fprintf(stderr, "HTTP: Not sending authorization (yet)\n");
          }
#endif

        FREE(hostname);
        FREE(docname);
      }
    }

  if (do_post)
    {
#ifdef DT
      if (TRACE)
        fprintf (stderr, "HTTP: Doing post, content-type '%s'\n",
                 anAnchor->post_content_type);
#endif

      sprintf (line, "Content-type: %s%c%c",
               anAnchor->post_content_type ? anAnchor->post_content_type 
							: "lose", CR, LF);
      StrAllocCat(command, line);
      {
	long content_length;
	if (!anAnchor->post_data)
	  content_length = 4; /* 4 == "lose" :-) */
	else
	  content_length = strlen (anAnchor->post_data);
	sprintf (line, "Content-length: %ld%c%c",
		 content_length, CR, LF);
	StrAllocCat(command, line);
      }

      StrAllocCat(command, crlf);	/* Blank line means "end" */

      StrAllocCat(command, anAnchor->post_data);
    }

  StrAllocCat(command, crlf);	/* Blank line means "end" */
  
#ifdef DT
  if (TRACE)
    fprintf (stderr, "Writing:\n%s----------------------------------\n",
             command);
#endif

  
  _HTProgress ("Sending HTTP request.");

  status = NETWRITE(s, command, (int)strlen(command));
  free (command);
  if (status <= 0) 
    {
      if (status == 0)
        {
#ifdef DT
          if (TRACE)
            fprintf (stderr, "HTTP: Got status 0 in initial write\n");
#endif

          /* Do nothing. */
        }
      else if 
	((SOCKET_ERRNO == ENOTCONN || SOCKET_ERRNO == ECONNRESET
	 || SOCKET_ERRNO == EPIPE
	) && !already_retrying &&
         /* Don't retry if we're posting. */ !do_post)
          {
            /* Arrrrgh, HTTP 0/1 compability problem, maybe. */
#ifdef DT
            if (TRACE)
              fprintf 
                (stderr, 
                 "HTTP: BONZO ON WRITE Trying again with HTTP0 request.\n");
#endif

            _HTProgress ("Retrying as HTTP0 request.");
            NETCLOSE(s);
            extensions = NO;
            already_retrying = 1;
            goto try_again;
          }
      else
        {
#ifdef DT
          if (TRACE)
            fprintf (stderr, "HTTP: Hit unexpected network WRITE error; aborting connection.\n");
#endif

          NETCLOSE (s);
          status = -1;
          HTAlert("Unexpected network write error; connection aborted.");
          goto done;
        }
    }
  
#ifdef DT
  if (TRACE)
    fprintf (stderr, "HTTP: WRITE delivered OK\n");
#endif

  _HTProgress ("HTTP request sent; waiting for response.");

  /*	Read the first line of the response
   **	-----------------------------------
   */
  
  {
    /* Get numeric status etc */
    BOOL end_of_file = NO;
    HTAtom * encoding = HTAtom_for("8bit");
    int buffer_length = INIT_LINE_SIZE;
    
    line_buffer = (char *) malloc(buffer_length * sizeof(char));
    
    do 
      {	/* Loop to read in the first line */
        /* Extend line buffer if necessary for those crazy WAIS URLs ;-) */
        if (buffer_length - length < LINE_EXTEND_THRESH) 
          {
            buffer_length = buffer_length + buffer_length;
            line_buffer = 
              (char *) realloc(line_buffer, buffer_length * sizeof(char));
          }
#ifdef DT
        if (TRACE)
          fprintf (stderr, "HTTP: Trying to read %d\n",
                   buffer_length - length - 1);
#endif

        status = NETREAD(s, line_buffer + length,
                         buffer_length - length - 1);
#ifdef DT
        if (TRACE)
          fprintf (stderr, "HTTP: Read %d\n", status);
#endif

        if (status <= 0) 
          {
            /* Retry if we get nothing back too; 
               bomb out if we get nothing twice. */
            if (status == HT_INTERRUPTED)
              {
#ifdef DT
                if (TRACE)
		  fprintf (stderr, "HTTP: Interrupted initial read.\n");
#endif

                _HTProgress ("Connection interrupted.");
                status = HT_INTERRUPTED;
                goto clean_up;
              }
            else if 
              (status < 0 &&
	       (SOCKET_ERRNO == ENOTCONN || SOCKET_ERRNO == ECONNRESET
		|| SOCKET_ERRNO == EPIPE
		     ) && !already_retrying && !do_post)
              {
                /* Arrrrgh, HTTP 0/1 compability problem, maybe. */
#ifdef DT
                if (TRACE)
                  fprintf (stderr, "HTTP: BONZO Trying again with HTTP0 request.\n");
#endif

                NETCLOSE(s);
                if (line_buffer) 
                  free(line_buffer);
                if (line_kept_clean) 
                  free(line_kept_clean);
                
                extensions = NO;
                already_retrying = 1;
                _HTProgress ("Retrying as HTTP0 request.");
                goto try_again;
              }
            else
              {
#ifdef DT
                if (TRACE)
                  fprintf (stderr, "HTTP: Hit unexpected network read error; aborting connection; status %d.\n", status);
#endif

                HTAlert("Unexpected network read error; connection aborted.");

                NETCLOSE (s);
                status = -1;
                goto clean_up;
              }
          }

        bytes_already_read += status;
        {
          char line[256];
	  sprintf (line, "Read %ld bytes of data.", bytes_already_read);
          HTProgress (line);
        }
        
#ifdef UCX  /* UCX returns -1 on EOF */
        if (status == 0 || status == -1) 
#else
        if (status == 0)
#endif
          {
            end_of_file = YES;
            break;
          }
        line_buffer[length+status] = 0;
        
        if (line_buffer)
          {
            if (line_kept_clean)
              free (line_kept_clean);
            line_kept_clean = (char *)malloc (buffer_length * sizeof (char));
            bcopy (line_buffer, line_kept_clean, buffer_length);
          }
        
        eol = strchr(line_buffer + length, LF);
        /* Do we *really* want to do this? */
        if (eol && eol != line_buffer && *(eol-1) == CR) 
          *(eol-1) = ' '; 
        
        length = length + status;

        /* Do we really want to do *this*? */
        if (eol) 
          *eol = 0;		/* Terminate the line */
      }
    /* All we need is the first line of the response.  If it's a HTTP/1.0
       response, then the first line will be absurdly short and therefore
       we can safely gate the number of bytes read through this code
       (as opposed to below) to ~1000. */
    /* Well, let's try 100. */
    while (!eol && !end_of_file && bytes_already_read < 100);
  } /* Scope of loop variables */
    
    
  /*	We now have a terminated unfolded line. Parse it.
   **	-------------------------------------------------
   */
#ifdef DT
  if (TRACE)
    fprintf(stderr, "HTTP: Rx: %s\n", line_buffer);
#endif

  
/* Kludge to work with old buggy servers and the VMS Help gateway.
** They can't handle the third word, so we try again without it.
*/
  if (extensions &&       /* Old buggy server or Help gateway? */
      (0==strncmp(line_buffer,"<TITLE>Bad File Request</TITLE>",31) ||
       0==strncmp(line_buffer,"Address should begin with",25) ||
       0==strncmp(line_buffer,"<TITLE>Help ",12) ||
       0==strcmp(line_buffer,
       		 "Document address invalid or access not authorised"))) {
      if (line_buffer)
	  free(line_buffer);
      if (line_kept_clean) 
          free(line_kept_clean);
      extensions = NO;
      already_retrying = 1;
#ifdef DT
      if (TRACE) fprintf(stderr,
			 "HTTP: close socket %d to retry with HTTP0\n", s);
#endif

      NETCLOSE(s);
      /* print a progress message */
      _HTProgress ("Retrying as HTTP0 request.");
      goto try_again;
  }


  {
    int fields;
    char server_version[VERSION_LENGTH+1];
    int server_status;

    server_version[0] = 0;
    
    fields = sscanf(line_buffer, "%20s %d",
                    server_version,
                    &server_status);
    
#ifdef DT
    if (TRACE)
      fprintf (stderr, "HTTP: Scanned %d fields from line_buffer\n", fields);
#endif

    
    if (http_error_file) {     /* Make the status code externally available */
	FILE *error_file;
	error_file = fopen(http_error_file, "w");
	if (error_file) {		/* Managed to open the file */
	    fprintf(error_file, "error=%d\n", server_status);
	    fclose(error_file);
	}
    }

    /* Rule out HTTP/1.0 reply as best we can. */
    if (fields < 2 || !server_version[0] || server_version[0] != 'H' ||
        server_version[1] != 'T' || server_version[2] != 'T' ||
        server_version[3] != 'P' || server_version[4] != '/' ||
        server_version[6] != '.') 
      {			/* HTTP0 reply */
        HTAtom * encoding;

#ifdef DT
        if (TRACE)
          fprintf (stderr, "--- Talking HTTP0.\n");
#endif

        
        format_in = HTFileFormat(arg, &encoding);
	/* treat all plain text as HTML.
         * this sucks but its the only solution without
         * looking at content.
         */
        if(!strncmp(HTAtom_name(format_in), "text/plain",10)) {
#ifdef DT
            if(TRACE)
                fprintf(stderr,
                           "HTTP:  format_in being changed to text/HTML\n");
#endif

            format_in = WWW_HTML;
        }

        start_of_data = line_kept_clean;
      } 
    else 
      {
        /* Decode full HTTP response */
        format_in = HTAtom_for("www/mime");
        /* We set start_of_data to "" when !eol here because there
           will be a put_block done below; we do *not* use the value
           of start_of_data (as a pointer) in the computation of
           length or anything else in this situation. */
        start_of_data = eol ? eol + 1 : "";
        length = eol ? length - (start_of_data - line_buffer) : 0;
        
#ifdef DT
        if (TRACE)
          fprintf (stderr, "--- Talking HTTP1.\n");
#endif

        
        switch (server_status / 100) 
          {
          case 3:		/* Various forms of redirection */
 
           if (no_url_redirection) { /* We want to see Location: */
	       break;
	   }


            /*
	     * We do not load the file, but read the header for
	     * its "Location: " and go get that URL.  If that's
	     * another redirecting URL, we'll repeat the fetches
	     * until we reach the actual URL. - FM
	     */
	   {
	    char *cp = line_kept_clean;
            doing_redirect = 1;
#ifdef DT
	    if(TRACE)
		fprintf(stderr,"Got Redirect code\n");
#endif

once_again:
	    /*
	     * Look for the "Location: " in the header. - FM
	     */
	    while (*cp) {
	        if(*cp != 'l' && *cp != 'L')
		    cp++;
	        else if (!strncasecomp(cp, "Location: ", 10)) {
	            extern char *redirecting_url;
	            char *cp1=NULL, *cp2=NULL;
	            cp += 10;
		    if((cp1=strchr(cp, LF)) != NULL) {
			int status1;
		        *cp1 = '\0';
		        if((cp2=strchr(cp, CR)) != NULL)
		            *cp2 = '\0';
			/*
			 * Load the new URL into redirecting_url,
			 * which will serve as a flag for seeking
			 * the document at that location. - FM
			 */
		        StrAllocCopy(redirecting_url, cp);
#ifdef DT
                        if (TRACE)
                            fprintf(stderr,
                                "[HTLoadHTTP] Picked up location '%s'\n", redirecting_url);
#endif

		        if(cp2)
		            *cp2 = CR;
		        *cp1 = LF;
		        status = HT_REDIRECTING;
			/*
			 * Finish reading the redirecting message if
			 * we don't have it all yet, then make sure
			 * the socket is closed and goto clean_up.
			 * Then, we'll check out the redirecting_url
			 * and if it's acceptible (e.g., not a telnet
			 * URL when we have that disabled) we'll fetch
			 * or act on it.
			 */

                        status1=1;

			while (status1 != -1) {
			    status1=NETREAD(s, line_buffer, INIT_LINE_SIZE);

			    if (status1 == HT_INTERRUPTED) {
#ifdef DT
			        if (TRACE)
				    fprintf (stderr,
				        "HTTP: Interrupted followup read.\n");
#endif

				_HTProgress ("Connection interrupted.");
				status = HT_INTERRUPTED;
				NETCLOSE(s);
				goto clean_up;
		             }
			}

#ifdef DT
                        if (TRACE)
                            fprintf(stderr, "HTTP: Going to cleanup.");
#endif


		        NETCLOSE(s);
		        goto clean_up;
		    }
	            break;
	        }
	        else {
	           cp++;
	        }
	    }
	    /*
	     * If we get to here, we didn't find the "Location: " yet.
	     * Get more of the header from the server, and goto
	     * "once_again:" to resume searching for it.  We should
	     * change this, someday, to get the entire header unfolded
	     * at the outset, but this works fine, for now. - FM
	     */
	    status = NETREAD(s, line_buffer, INIT_LINE_SIZE);
            if (status <= 0) {
                if (status == HT_INTERRUPTED) {
		    /*
		     * Impatient user. - FM
		     */
#ifdef DT
                    if (TRACE)
                        fprintf (stderr, "HTTP: Interrupted followup read.\n");
#endif

                    _HTProgress ("Connection interrupted.");
                    status = HT_INTERRUPTED;
		    NETCLOSE(s);
                    goto clean_up;
                }
                else {
		    /*
		     *  Hmm, no more data, and we didn't find the
		     *  redirecting_url.  Better luck next time,
		     *  but we'll give up on this request. - FM
		     */
#ifdef DT
                    if (TRACE)
                      fprintf (stderr,
  "HTTP: Hit unexpected network read error; aborting connection; status %d.\n",
 			       status);
#endif

                    HTAlert(
		         "Unexpected network read error; connection aborted.");
                    NETCLOSE (s);
                    status = HT_NO_DATA;
                    goto clean_up;
                }
            }
	    /*
	     *  We got more from the server, so go back and
	     *  once_again search for the redirecting_url. - FM
	     */
	    StrAllocCopy(line_kept_clean, line_buffer);
	    cp = line_kept_clean;
	    goto once_again;
            break;
	   }
            
          case 4:		/* "I think I goofed" */
            switch (server_status) 
              {
              case 403:
                /* 403 is "forbidden"; display returned text. */
                /* format_in = HTAtom_for("text/html"); */
                break;

              case 401:
                /* length -= start_of_data - text_buffer; */
                if (HTAA_shouldRetryWithAuth(start_of_data, length, s)) 
                  {
                    extern BOOLEAN dump_output_immediately;
 		    extern char *authentication_info[2];

                    (void)NETCLOSE(s);
                    if (line_buffer) 
                      free(line_buffer);
                    if (line_kept_clean) 
                      free(line_kept_clean);
                    if(dump_output_immediately && !authentication_info[0])	{
                      fprintf(stderr, "HTTP:  Access authorization required.\n");
                      fprintf(stderr, "       Use the -auth=id:pw parameter.\n");
                      status = HT_NO_DATA;
                      goto clean_up;
                    }

#ifdef DT
                    if (TRACE) 
                      fprintf(stderr, "%s %d %s\n",
                              "HTTP: close socket", s,
                              "to retry with Access Authorization");
#endif

                    
                    _HTProgress ("Retrying with access authorization information.");
                    goto try_again;
                    break;
                  }
                else 
                  {
                    /* Fall through. */
                  }

              default:
                break;
#if 0
                char *p1 = HTParse(arg, "", PARSE_HOST);
                char *message;
                
                message = (char*)malloc(strlen(text_buffer) +
                                        strlen(p1) + 100);
                sprintf(message,
                        "HTTP server at %s replies:\n%s\n\n%s\n",
                        p1, text_buffer,
                        ((server_status == 401) 
                         ? "Access Authorization package giving up.\n"
                         : ""));
                free(message);
                free(p1);
#endif
#if 0
                _HTProgress ("Could not load requested document.");
                status = -1;
                goto clean_up;
#endif
              } /* case 4 switch */
            break;

          case 5:		/* I think you goofed */
            break;
            
          case 2:		/* Good: Got MIME object */
	    if(server_status == 204) {
	        status = HT_NO_DATA;
	        goto done;
	    }
            break;
            
          default:		/* bad number */
            HTAlert("Unknown status reply from server!");
            break;
          } /* Switch on server_status/100 */
        
      }	/* Full HTTP reply */
  } /* scope of fields */

  /* Set up the stream stack to handle the body of the message */
  if (keep_mime_headers) { /* We want to see all the mime lines*/
      format_in = HTAtom_for("www/source");
  }

  target = HTStreamStack(format_in,
                         format_out,
                         sink, anAnchor);
  
  if (!target || target == NULL) 
    {
      char buffer[1024];	/* @@@@@@@@ */
      sprintf(buffer, "Sorry, no known way of converting %s to %s.",
              HTAtom_name(format_in), HTAtom_name(format_out));
      _HTProgress (buffer);
      status = -1;
      goto clean_up;
    }

  /* Recycle the first chunk of data, in all cases. */
  (*target->isa->put_block)(target, start_of_data, length);

#ifdef DISP_PARTIAL
  dp_dl_on = dp_newline = 1 ;
#endif
  /* Go pull the bulk of the data down. */
  rv = HTCopy(s, target);

#ifdef DISP_PARTIAL
  dp_dl_on = 0 ;
#endif

  if (rv == -1)
    {
      /* Intentional interrupt before data were received, not an error */
      /* (*target->isa->_abort)(target, NULL);/* already done in HTCopy */
      status = HT_INTERRUPTED;
      NETCLOSE(s);
      goto clean_up;
    }
  if (rv == -2 && !already_retrying && !do_post)
    { 
      /* Aw hell, a REAL error, maybe cuz it's a dumb HTTP0 server */
#ifdef DT
      if (TRACE)
        fprintf (stderr, "HTTP: Trying again with HTTP0 request.\n");
#endif

      /* May as well consider it an interrupt -- right? */
      (*target->isa->_abort)(target, NULL);
      NETCLOSE(s);
      if (line_buffer) 
        free(line_buffer);
      if (line_kept_clean) 
        free(line_kept_clean);

      extensions = NO;
      already_retrying = 1;
      _HTProgress ("Retrying as HTTP0 request.");
      goto try_again;
    }

  /* 
   * Close socket if partial transmission (was freed on abort)
   * Free if complete transmission (socket was closed before return)
   */
  if (rv == HT_INTERRUPTED)
      NETCLOSE(s);
  else
      (*target->isa->_free)(target);

  if (doing_redirect)
    /*
     * We already jumped over all this if the "case 3:" code worked
     * above, but we'll check here as a backup in case it fails. - FM
     */
    {
      /* Lou's old comment:  - FM */
      /* OK, now we've got the redirection URL temporarily stored
         in external variable redirecting_url, exported from HTMIME.c,
         since there's no straightforward way to do this in the library
         currently.  Do the right thing. */
      status = HT_REDIRECTING;
    }
  else
    {
      /* If any data were received, treat as a complete transmission */
      status = HT_LOADED;
    }

  /*	Clean up
   */
  
clean_up: 
  if (line_buffer) 
    free(line_buffer);
  if (line_kept_clean) 
    free(line_kept_clean);

 done:
  /* Clear out on exit, just in case. */
  do_post = 0;
  return status;
}


/*	Protocol descriptor
*/

#if defined (GLOBALDEF_IS_MACRO)
#define _HTTP_C_GLOBALDEF_1_INIT { "http", HTLoadHTTP, 0}
GLOBALDEF (HTProtocol,HTTP,_HTTP_C_GLOBALDEF_1_INIT);
#define _HTTP_C_GLOBALDEF_2_INIT { "https", HTLoadHTTP, 0}
GLOBALDEF (HTProtocol,HTTPS,_HTTP_C_GLOBALDEF_2_INIT);
#else
GLOBALDEF PUBLIC HTProtocol HTTP = { "http", HTLoadHTTP, 0 };
GLOBALDEF PUBLIC HTProtocol HTTPS = { "https", HTLoadHTTP, 0 };
#endif
