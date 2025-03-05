#ifdef noway
/*			GOPHER ACCESS				HTGopher.c
**			=============
**
** History:
**	26 Sep 90	Adapted from other accesses (News, HTTP) TBL
**	29 Nov 91	Downgraded to C, for portable implementation.
*/

#include "HTUtils.h"		/* Coding convention macros */
#include "tcp.h"
#include "HTAlert.h"

/* Implements:
*/
#include "HTGopher.h"

#define HT_EM_SPACE ((char)2)           /* For now */

#define GOPHER_PORT 70		/* See protocol spec */
#define BIG 1024		/* Bug */
#define LINE_LENGTH 256		/* Bug */

/*	Gopher entity types:
*/
#define GOPHER_TEXT		'0'
#define GOPHER_MENU		'1'
#define GOPHER_CSO		'2'
#define GOPHER_ERROR		'3'
#define GOPHER_MACBINHEX	'4'
#define GOPHER_PCBINARY		'5'
#define GOPHER_UUENCODED	'6'
#define GOPHER_INDEX		'7'
#define GOPHER_TELNET		'8'
#define GOPHER_BINARY           '9'
#define GOPHER_GIF              'g'
#define GOPHER_HTML		'h'	        /* HTML */
#define GOPHER_CHTML		'H'	        /* HTML */
#define GOPHER_SOUND            's'
#define GOPHER_WWW		'w'		/* W3 address */
#define GOPHER_IMAGE            'I'
#define GOPHER_TN3270           'T'
#define GOPHER_INFO             'i'
#define GOPHER_DUPLICATE	'+'
#define GOPHER_PLUS_IMAGE	':'		/* Addition from Gopher Plus */
#define GOPHER_PLUS_MOVIE	';'
#define GOPHER_PLUS_SOUND	'<'
#define GOPHER_PLUS_PDF		'P'

#include <ctype.h>

#include "HTParse.h"
#include "HTFormat.h"
#include "HTTCP.h"

/* added for the hell of it :-( */

#include "HTFile.h"
#include "HTMIME.h"
#include "HTML.h"
#include "HTInit.h"
#include "HTAABrow.h"
#include "gridtext.h"

/*		Hypertext object building machinery
*/
#include "HTML.h"

#include "LYUtils.h"

#include "LYLeaks.h"
 
#define PUTC(c) (*targetClass.put_character)(target, c)
#define PUTS(s) (*targetClass.put_string)(target, s)
#define START(e) (*targetClass.start_element)(target, e, 0, 0)
#define END(e) (*targetClass.end_element)(target, e)
#define FREE_TARGET (*targetClass._free)(target)
struct _HTStructured {
	CONST HTStructuredClass *	isa;
	/* ... */
};

PRIVATE HTStructured *target;			/* the new hypertext */
PRIVATE HTStructuredClass targetClass;		/* Its action routines */


#define GOPHER_PROGRESS(foo) HTAlert(foo)


#define NEXT_CHAR HTGetCharacter()



/*	Module-wide variables
*/
PRIVATE int s;					/* Socket for GopherHost */



/*	Matrix of allowed characters in filenames
**	-----------------------------------------
*/

PRIVATE BOOL acceptable[256];
PRIVATE BOOL acceptable_inited = NO;

PRIVATE void init_acceptable NOARGS
{
    unsigned int i;
    char * good = 
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./-_$";
    for(i=0; i<256; i++) acceptable[i] = NO;
    for(;*good; good++) acceptable[(unsigned int)*good] = YES;
    acceptable_inited = YES;
}

PRIVATE CONST char hex[17] = "0123456789abcdef";

/*	Decdoe one hex character
*/

PRIVATE char from_hex ARGS1(char, c)
{
    return 		  (c>='0')&&(c<='9') ? c-'0'
			: (c>='A')&&(c<='F') ? c-'A'+10
			: (c>='a')&&(c<='f') ? c-'a'+10
			:		       0;
}



/*	Paste in an Anchor
**	------------------
**
**	The title of the destination is set, as there is no way
**	of knowing what the title is when we arrive.
**
** On entry,
**	HT 	is in append mode.
**	text 	points to the text to be put into the file, 0 terminated.
**	addr	points to the hypertext refernce address 0 terminated.
*/
PUBLIC BOOLEAN HT_Is_Gopher_URL=FALSE;

PRIVATE void write_anchor ARGS2(CONST char *,text, CONST char *,addr)
{



    BOOL present[HTML_A_ATTRIBUTES];
    CONST char * value[HTML_A_ATTRIBUTES];

    int i;

    for (i=0; i<HTML_A_ATTRIBUTES; i++) present[i]=0;
    present[HTML_A_HREF] = YES;
    ((CONST char **)value)[HTML_A_HREF] = addr;
    present[HTML_A_TITLE] = YES;
    ((CONST char **)value)[HTML_A_TITLE] = text;

#ifdef DT
    if(TRACE)
	fprintf(stderr,"HTGopher: adding URL: %s\n",addr);
#endif


    HT_Is_Gopher_URL = TRUE;  /* tell HTML.c that this is a Gopher URL */

{
	extern void *vp_msdosmem;
	extern void **vpp_msdosmem;
	vp_msdosmem = (void *)(present);
	vpp_msdosmem = (void **)(value);
}

    (*targetClass.start_element)(target, HTML_A, present,(CONST char **)value);

    PUTS(text);
    END(HTML_A);
}


/*	Parse a Gopher Menu document
**	============================
**
*/

PRIVATE void parse_menu ARGS2 (
	CONST char *,		arg,
	HTParentAnchor *,	anAnchor)
{
    char gtype;
    char ch;
    char line[BIG];
    char address[BIG];
    char *name, *selector;		/* Gopher menu fields */
    char *host;
    char *port;
    char *p = line;
    CONST char *title;
    int bytes=0;
    int BytesReported = 0;
    char buffer[128];
    extern int interrupted_in_htgetcharacter;

#define TAB 		'\t'
#define HEX_ESCAPE 	'%'

{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_HTML);
    PUTS("\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
	vpp_msdosmem = NULL;
}

    START(HTML_HEAD);
    PUTS("\n");
{
	extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_TITLE);
    if ((title = HTAnchor_title(anAnchor)))
	PUTS(title);
    else
        PUTS("Gopher Menu");
    END(HTML_TITLE);
    PUTS("\n");
    END(HTML_HEAD);
    PUTS("\n");

{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
	vpp_msdosmem = NULL;
}

    START(HTML_BODY);
    PUTS("\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_H1);
    if ((title = HTAnchor_title(anAnchor)))
	PUTS(title);
    else
	PUTS("Gopher Menu");
    END(HTML_H1);
    PUTS("\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_PRE);
    while ((ch=NEXT_CHAR) != (char)EOF) {

#ifdef FIXME
	if (interrupted_in_htgetcharacter) {
#ifdef DT
	    if (TRACE) fprintf (stderr,
		"Gopher: Interrupted in HTGetCharacter, apparently.\n");
#endif

	    goto end_html;
	}

#endif
        if (ch != LF) {
	    *p = ch;		/* Put character in line */
	    if (p< &line[BIG-1]) p++;
	    
	} else {
	    *p++ = 0;		/* Terminate line */
	    bytes += p-line;    /* add size */
	    p = line;		/* Scan it to parse it */
	    port = 0;		/* Flag "not parsed" */
#ifdef DT
	    if (TRACE) fprintf(stderr, "HTGopher: Menu item: %s\n", line);
#endif

	    gtype = *p++;

	    if (bytes > BytesReported + 1024) {
	        sprintf(buffer, "Transferred %d bytes", bytes);
                HTProgress(buffer);
		BytesReported = bytes;
	    }
	    
	    /* Break on line with a dot by itself */
	    if ((gtype=='.') && ((*p=='\r') || (*p==0))) break;

	    if (gtype && *p) {
		name = p;
		selector = strchr(name, TAB);
		if (selector) {
		    *selector++ = 0;	/* Terminate name */
		    /*
		     * Gopher+ Type=0+ objects can be binary, and will
		     * have 9 or 5 beginning their selector.  Make sure
		     * we don't trash the terminal by treating them as
		     * text. - FM
		     */
		    if (gtype == GOPHER_TEXT && (*selector == GOPHER_BINARY ||
		    				 *selector == GOPHER_PCBINARY))
		        gtype = *selector;
		    host = strchr(selector, TAB);
		    if (host) {
			*host++ = 0;	/* Terminate selector */
			port = strchr(host, TAB);
			if (port) {
			    char *junk;
			    port[0] = ':';	/* delimit host a la W3 */
			    junk = strchr(port, TAB);
			    if (junk) *junk++ = 0;	/* Chop port */
			    if ((port[1]=='0') && (!port[2]))
			        port[0] = 0;	/* 0 means none */
			} /* no port */
		    } /* host ok */
		} /* selector ok */
	    } /* gtype and name ok */
	    
	    /* Nameless files are a separator line */
	    if (gtype == GOPHER_TEXT) {
	    	int i = strlen(name)-1;
		while (name[i] == ' ' && i >= 0)
		    name[i--] = '\0';
		if (i < 0)
		    gtype = GOPHER_INFO;
	    }

	    if (gtype == GOPHER_WWW) {	/* Gopher pointer to W3 */
		PUTS("(HTML) ");
		write_anchor(name, selector);

	    } else if (gtype == GOPHER_INFO) {
	    /* Information or separator line */
		PUTS("       ");
		PUTS(name);

	    } else if (port) {		/* Other types need port */
		if (gtype == GOPHER_TELNET) {
		    PUTS(" (TEL) ");
		    if (*selector) sprintf(address, "telnet://%s@%s/",
					   selector, host);
		    else sprintf(address, "telnet://%s/", host);
		}
		else if (gtype == GOPHER_TN3270) 
		{
		    PUTS("(3270) ");
		    if (*selector) 
			sprintf(address, "tn3270://%s@%s/",
				selector, host);
		    else 
			sprintf(address, "tn3270://%s/", host);
		}
		else {			/* If parsed ok */
		    char *q;
		    char *p;

		    switch(gtype) {
               		case GOPHER_TEXT:
                   	    PUTS("(FILE) ");
                   	    break;
                	case GOPHER_MENU:
                    	    PUTS(" (DIR) ");
                    	    break;
                	case GOPHER_CSO:
                    	    PUTS(" (CSO) ");
                    	    break;
                	case GOPHER_PCBINARY:
			    PUTS(" (BIN) ");
                    	    break;
                	case GOPHER_UUENCODED:
                    	    PUTS(" (UUE) ");
                    	    break;
                	case GOPHER_INDEX:
                    	    PUTS("  (?)  ");
                    	    break;
                	case GOPHER_BINARY:
                    	    PUTS(" (BIN) ");
                    	    break;
                	case GOPHER_GIF:
                	case GOPHER_IMAGE:
			case GOPHER_PLUS_IMAGE:
                    	    PUTS(" (IMG) ");
                    	    break;
                	case GOPHER_SOUND:
                	case GOPHER_PLUS_SOUND:
                    	    PUTS(" (SND) ");
                    	    break;
                	case GOPHER_MACBINHEX:
                    	    PUTS(" (HQX) ");
                    	    break;
			case GOPHER_HTML:
			case GOPHER_CHTML:
                    	    PUTS("(HTML) ");
			    break;
                	case 'm':
                    	    PUTS("(MIME) ");
                    	    break;
                	case GOPHER_PLUS_MOVIE:
                    	    PUTS(" (MOV) ");
                    	    break;
                	case GOPHER_PLUS_PDF:
                    	    PUTS(" (PDF) ");
                    	    break;
                	default:
                    	    PUTS("(UNKN) ");
                    	    break;
		    }

		    sprintf(address, "//%s/%c", host, gtype);

		    q = address+ strlen(address);
		    for(p=selector; *p; p++) {	/* Encode selector string */
			if (acceptable[*p]) *q++ = *p;
			else {
			    *q++ = HEX_ESCAPE;	/* Means hex coming */
			    *q++ = hex[(TOASCII(*p)) >> 4];
			    *q++ = hex[(TOASCII(*p)) & 15];
			}
		    }

		    *q++ = 0;			/* terminate address */
		}
		/* Error response from Gopher doesn't deserve to
		   be a hyperlink. */
		if (strcmp (address, "gopher://error.host:1/0"))
		    write_anchor(name, address);
		else
		    PUTS(name);
	    } else { /* parse error */
#ifdef DT
	        if (TRACE) fprintf(stderr,
			"HTGopher: Bad menu item.\n");
#endif

		PUTS(line);

	    } /* parse error */
	    
	    PUTS("\n");
	    p = line;	/* Start again at beginning of line */
	    
	} /* if end of line */
	
    } /* Loop over characters */
	
end_html:
    END(HTML_PRE);
    PUTS("\n");
    END(HTML_BODY);
    PUTS("\n");
    END(HTML_HTML);
    PUTS("\n");
    FREE_TARGET;

    return;
}
/*	Parse a Gopher CSO document
 **	============================
 **
 **   Accepts an open socket to a CSO server waiting to send us
 **   data and puts it on the screen in a reasonable manner.
 **
 **   Perhaps this data can be automatically linked to some
 **   other source as well???
 **
 **  Taken from hacking by Lou Montulli@ukanaix.cc.ukans.edu
 **  on XMosaic-1.1, and put on libwww 2.11 by Arthur Secret, 
 **  secret@dxcern.cern.ch .
 */

PRIVATE void parse_cso ARGS2 (
			      CONST char *,	arg,
			      HTParentAnchor *,anAnchor)
{
    char ch;
    char line[BIG];
    char *p = line;
    char *second_colon, last_char='\0';
    CONST char *title;
    
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
	vpp_msdosmem = NULL;
}

    START(HTML_HEAD);
    PUTS("\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_TITLE);
    if ((title = HTAnchor_title(anAnchor)))
        PUTS(title);
    else
        PUTS("CSO Search Results");
    END(HTML_TITLE);
    PUTS("\n");
    END(HTML_HEAD);
    PUTS("\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_H1);
    if ((title = HTAnchor_title(anAnchor)))
        PUTS(title);
    else {
        PUTS(arg);
        PUTS(" Search Results");
    }
    END(HTML_H1);
    PUTS("\n");
{
        extern void *vp_msdosmem;
	extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_PRE);

    /* start grabbing chars from the network */
    while ((ch=NEXT_CHAR) != (char)EOF) 
	{
	    if (ch != '\n') 
		{
		    *p = ch;		/* Put character in line */
		    if (p< &line[BIG-1]) p++;
		} 
	    else 
		{
		    *p++ = 0;		/* Terminate line */
		    p = line;		/* Scan it to parse it */
		    
		    /* OK we now have a line in 'p' lets parse it and 
		       print it */
		    
		    /* Break on line that begins with a 2. It's the end of
		     * data.
		     */
		    if (*p == '2')
			break;
		    
		    /*  lines beginning with 5 are errors, 
		     *  print them and quit
		     */
		    if (*p == '5') {
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

			START(HTML_H2);
			PUTS(p+4);
			END(HTML_H2);
			break;
		    }
		    
		    if(*p == '-') {
			/*  data lines look like  -200:#:
			 *  where # is the search result number and can be  
			 *  multiple digits (infinate?)
			 *  find the second colon and check the digit to the
			 *  left of it to see if they are diferent
			 *  if they are then a different person is starting.
			 *  make this line an <h2>
			 */
			
			/* find the second_colon */
			second_colon = strchr( strchr(p,':')+1, ':');
			
			if(second_colon != NULL) {  /* error check */
			    
			    if (*(second_colon-1) != last_char)   
				/* print seperator */
			    {
				END(HTML_PRE);
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

				START(HTML_H2);
			    }
				
			    
			    /* right now the record appears with the alias 
			     * (first line)
			     * as the header and the rest as <pre> text
			     * It might look better with the name as the
			     * header and the rest as a <ul> with <li> tags
			     * I'm not sure whether the name field comes in any
			     * special order or if its even required in a 
			     * record,
			     * so for now the first line is the header no 
			     * matter
			     * what it is (it's almost always the alias)
			     * A <dl> with the first line as the <DT> and
			     * the rest as some form of <DD> might good also?
			     */
			    
			    /* print data */
			    PUTS(second_colon+1);
			    PUTS("\n");
			    
			    if (*(second_colon-1) != last_char)   
				/* end seperator */
			    {
				END(HTML_H2);
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

				START(HTML_PRE);
			    }
							    
			    /* save the char before the second colon
			     * for comparison on the next pass
			     */
			    last_char =  *(second_colon-1) ;
			    
			} /* end if second_colon */
		    } /* end if *p == '-' */
		} /* if end of line */

	} /* Loop over characters */
    
    /* end the text block */
    PUTS("\n");
    END(HTML_PRE);
    PUTS("\n");
    FREE_TARGET;

    return;  /* all done */
} /* end of procedure */

/*	Display a Gopher Index document
 **	-------------------------------
 */

PRIVATE void display_index ARGS2 (
				  CONST char *,	arg,
				  HTParentAnchor *,anAnchor)
{
    CONST char * title;
    
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
	vpp_msdosmem = NULL;
}

    START(HTML_HEAD);
    PUTS("\n");
    PUTS("\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_TITLE);
    if ((title = HTAnchor_title(anAnchor)))
	PUTS(title);
    else
        PUTS("Gopher index");
    END(HTML_TITLE);
    PUTS("\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_ISINDEX);
    PUTS("\n");
    END(HTML_HEAD);
    PUTS("\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_H1);
    if ((title = HTAnchor_title(anAnchor)))
	PUTS(title);
    else {
       PUTS(arg);
       PUTS(" index");
    }
    END(HTML_H1);
    PUTS("\nThis is a searchable Gopher index.\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_P);
    PUTS("\nPlease enter search keywords.\n");
    
    if (!HTAnchor_title(anAnchor))
    	HTAnchor_setTitle(anAnchor, arg);
    
    FREE_TARGET;

    return;
}


/*      Display a CSO index document
**      -------------------------------
*/

PRIVATE void display_cso ARGS2 (
        CONST char *,   arg,
        HTParentAnchor *,anAnchor)
{
    CONST char * title;

{
        extern void *vp_msdosmem;
	extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_HEAD);
    PUTS("\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_TITLE);
    if ((title = HTAnchor_title(anAnchor)))
	PUTS(title);
    else
        PUTS("CSO index");
    END(HTML_TITLE);
    PUTS("\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_ISINDEX);
    PUTS("\n");
    END(HTML_HEAD);
    PUTS("\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_H1);
    if ((title = HTAnchor_title(anAnchor)))
	PUTS(title);
    else {
       PUTS(arg);
       PUTS(" index");
    }
    END(HTML_H1);
    PUTS("\nThis is a searchable index of a CSO database.\n");
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
	vpp_msdosmem = NULL;
}

    START(HTML_P);
    PUTS("\nPress the 's' key and enter search keywords.\n"); 
{
        extern void *vp_msdosmem;
        extern void **vpp_msdosmem;
        vp_msdosmem = NULL;
        vpp_msdosmem = NULL;
}

    START(HTML_P);
    PUTS("\nThe keywords that you enter will allow you to search on a");
    PUTS(" person's name in the database.\n");

    if (!HTAnchor_title(anAnchor))
    	HTAnchor_setTitle(anAnchor, arg);
    
    FREE_TARGET;

    return;
}


/*		De-escape a selector into a command
**		-----------------------------------
**
**	The % hex escapes are converted. Otheriwse, the string is copied.
*/
PRIVATE void de_escape ARGS2(char *, command, CONST char *, selector)
{
    CONST char * p = selector;
    char * q = command;
	if (command == NULL) outofmem(__FILE__, "HTLoadGopher");
    while (*p) {		/* Decode hex */
	if (*p == HEX_ESCAPE) {
	    char c;
	    unsigned int b;
	    p++;
	    c = *p++;
	    b =   from_hex(c);
	    c = *p++;
	    if (!c) break;	/* Odd number of chars! */
	    *q++ = FROMASCII((b<<4) + from_hex(c));
	} else {
	    *q++ = *p++;	/* Record */
	}
    }
    *q++ = 0;	/* Terminate command */

}


/*		Load by name					HTLoadGopher
**		============
**
**	 Bug:	No decoding of strange data types as yet.
**
*/
PUBLIC int HTLoadGopher ARGS4(
	CONST char *,		arg,
	HTParentAnchor *,	anAnchor,
	HTFormat,		format_out,
	HTStream*,		sink)
{
    char *command;			/* The whole command */
    int status;				/* tcp return */
    char gtype;				/* Gopher Node type */
    char * selector;			/* Selector string */
 
    struct sockaddr_in soc_address;	/* Binary network address */
    struct sockaddr_in* sin = &soc_address;
    
    if (!acceptable_inited) init_acceptable();
    
    if (!arg) return -3;		/* Bad if no name sepcified	*/
    if (!*arg) return -2;		/* Bad if name had zero length	*/

#ifdef DT
    if (TRACE) fprintf(stderr, "HTGopher: Looking for %s\n", arg);
#endif

    
    
/*  Set up defaults:
*/
    sin->sin_family = AF_INET;	    		/* Family, host order  */
    sin->sin_port = htons(GOPHER_PORT);	    	/* Default: new port,  */

/* Get node name and optional port number:
*/
    {
	char *p1 = HTParse(arg, "", PARSE_HOST);
	int status = HTParseInet(sin, p1);
        free(p1);
        if (status) return status;   /* Bad */
    }
    
/* Get entity type, and selector string.
*/        
    {
	char * p1 = HTParse(arg, "", PARSE_PATH|PARSE_PUNCTUATION);
        gtype = '1';		/* Default = menu */
	selector = p1;
	if ((*selector++=='/') && (*selector)) {	/* Skip first slash */
	    gtype = *selector++;			/* Pick up gtype */
	}
	if (gtype == GOPHER_INDEX) {
	    char * query;
	    /* Search is allowed */
            HTAnchor_setIndex(anAnchor, anAnchor->address);	
	    query = strchr(selector, '?');	/* Look for search string */
	    if (!query || !query[1]) {		/* No search required */
		target = HTML_new(anAnchor, format_out, sink);
		targetClass = *target->isa;
		display_index(arg, anAnchor);	/* Display "cover page" */
		return HT_LOADED;		/* Local function only */
	    }
	    *query++ = 0;			/* Skip '?' 	*/
	    command = malloc(strlen(selector)+ 1 + strlen(query)+ 2 + 1);
              if (command == NULL) outofmem(__FILE__, "HTLoadGopher");
	      
	    de_escape(command, selector);	/* Bug fix TBL 921208 */

	    strcat(command, "\t");
	  
	    {					/* Remove plus signs 921006 */
	    	char *p;
		for (p=query; *p; p++) {
		    if (*p == '+') *p = ' ';
		}
	    }

	    de_escape(&command[strlen(command)], query);/* bug fix LJM 940415 */
        } else if (gtype == GOPHER_CSO) {
            char * query;
	    /* Search is allowed */
            HTAnchor_setIndex(anAnchor, anAnchor->address);	
            query = strchr(selector, '?');      /* Look for search string */
            if (!query || !query[1]) {          /* No search required */
		target = HTML_new(anAnchor, format_out, sink);
		targetClass = *target->isa;
		display_cso(arg, anAnchor);     /* Display "cover page" */
                return HT_LOADED;                 /* Local function only */
            }
            *query++ = 0;                       /* Skip '?'     */
            command = malloc(strlen("query")+ 1 + strlen(query)+ 2 + 1);
              if (command == NULL) outofmem(__FILE__, "HTLoadGopher");

            de_escape(command, selector);       /* Bug fix TBL 921208 */

            strcpy(command, "query ");

            {                                   /* Remove plus signs 921006 */
                char *p;
		for (p=query; *p; p++) {
                    if (*p == '+') *p = ' ';
                }
            }
	    de_escape(&command[strlen(command)], query);/* bug fix LJM 940415 */

	    
	} else {				/* Not index */
	    command = malloc(strlen(selector)+2+1);
	    de_escape(command, selector);
	}
	free(p1);
    }

    {
	char * p = command + strlen(command);
	*p++ = CR;		/* Macros to be correct on Mac */
	*p++ = LF;
	*p++ = 0;
	/* strcat(command, "\r\n");	*/	/* CR LF, as in rfc 977 */
    }

/*	Set up a socket to the server for the data:
*/      

  status = HTDoConnect (arg, "Gopher", 70, &s);
  if (status == HT_INTERRUPTED)
    {
      /* Interrupt cleanly. */
#ifdef DT
      if (TRACE)
        fprintf (stderr,
                 "Gopher: Interrupted on connect; recovering cleanly.\n");
#endif

      _HTProgress ("Connection interrupted.");
      return HT_INTERRUPTED;
    }
  if (status<0){
#ifdef DT
	if (TRACE) fprintf(stderr, "HTTPAccess: Unable to connect to remote host for `%s'.\n",
	    arg);
#endif

	free(command);
	return HTInetStatus("connect");
  }
    
    HTInitInput(s);		/* Set up input buffering */
    
#ifdef DT
    if (TRACE) fprintf(stderr, "HTGopher: Connected, writing command `%s' to socket %d\n", command, s);
#endif

#ifdef NOT_ASCII
    {
    	char * p;
	for(p = command; *p; p++) {
	    *p = TOASCII(*p);
	}
    }
#endif

    _HTProgress ("Sending Gopher request.");

    status = NETWRITE(s, command, (int)strlen(command));
    free(command);
    if (status<0){
#ifdef DT
	if (TRACE) fprintf(stderr, "HTGopher: Unable to send command.\n");
#endif

	    return HTInetStatus("send");
    }

    _HTProgress ("Gopher request sent; waiting for response.");

/*	Now read the data from the socket:
*/
    switch (gtype) {
    
    case GOPHER_TEXT :
     	HTParseSocket(WWW_PLAINTEXT, format_out, anAnchor, s, sink);
	break;

    case GOPHER_HTML :
    case GOPHER_CHTML :
    	HTParseSocket(WWW_HTML, format_out, anAnchor, s, sink);
	break;

    case GOPHER_GIF:
    case GOPHER_IMAGE:
    case GOPHER_PLUS_IMAGE:
    	HTParseSocket(HTAtom_for("image/gif"), 
			   format_out, anAnchor, s, sink);
  	break;

    case GOPHER_MENU :
    case GOPHER_INDEX :
	target = HTML_new(anAnchor, format_out, sink);
	targetClass = *target->isa;
        parse_menu(arg, anAnchor);
	break;
	 
    case GOPHER_CSO:
	target = HTML_new(anAnchor, format_out, sink);
	targetClass = *target->isa;
      	parse_cso(arg, anAnchor);
	break;
   	
    case GOPHER_SOUND :
    case GOPHER_PLUS_SOUND :
    	HTParseSocket(WWW_AUDIO, format_out, anAnchor, s, sink);
	break;
	
    case GOPHER_PLUS_MOVIE:
    	HTParseSocket(HTAtom_for("video/mpeg"), format_out, anAnchor, s, sink);
	break;

    case GOPHER_PLUS_PDF:
    	HTParseSocket(HTAtom_for("application/pdf"), format_out, anAnchor,
				  s, sink);
	break;

    case GOPHER_MACBINHEX:
    case GOPHER_PCBINARY:
    case GOPHER_UUENCODED:
    case GOPHER_BINARY:
    default:
        /* Specifying WWW_UNKNOWN forces dump to local disk. */
	HTParseSocket (WWW_UNKNOWN, format_out, anAnchor, s, sink);
	break;

    } /* switch(gtype) */

    NETCLOSE(s);
    return HT_LOADED;
}

#if defined (GLOBALDEF_IS_MACRO)
#define _HTGOPHER_C_1_INIT { "gopher", HTLoadGopher, NULL }
GLOBALDEF (HTProtocol, HTGopher, _HTGOPHER_C_1_INIT);
#else
GLOBALDEF PUBLIC HTProtocol HTGopher = { "gopher", HTLoadGopher, NULL };
#endif
#endif