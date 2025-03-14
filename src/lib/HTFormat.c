
/*		Manage different file formats			HTFormat.c
**		=============================
**
** Bugs:
**	Not reentrant.
**
**	Assumes the incoming stream is ASCII, rather than a local file
**	format, and so ALWAYS converts from ASCII on non-ASCII machines.
**	Therefore, non-ASCII machines can't read local files.
**
*/

#include <unistd.h>

#include "HTUtils.h"
#include "tcp.h"

/* Implements:
*/
#include "HTFormat.h"

PUBLIC long HTMaxSecs = 1e10;          /* No effective limit */
PUBLIC long HTMaxLength = 1e10;        /* No effective limit */

PUBLIC long loading_length= -1;

#ifdef unix
#ifdef NeXT
#define PRESENT_POSTSCRIPT "open %s; /bin/rm -f %s\n"
#else
#define PRESENT_POSTSCRIPT "(ghostview %s ; /bin/rm -f %s)&\n"	
	/* Full pathname would be better! */
#endif
#endif


#include "HTML.h"
#include "HTMLDTD.h"
#include "HText.h"
#include "HTAlert.h"
#include "HTList.h"
#include "HTInit.h"
/*	Streams and structured streams which we use:
*/
#include "HTFWrite.h"
#include "HTPlain.h"
#include "SGML.h"
#include "HTML.h"
#include "HTMLGen.h"

#ifdef MSDOS
#include "LYUtils.h"
#endif

#include "LYexit.h"
#include "LYLeaks.h"

extern int HTCheckForInterrupt NOPARAMS;

PUBLIC	BOOL HTOutputSource = NO;	/* Flag: shortcut parser to stdout */
/* extern  BOOL interactive; LJM */

#ifdef ORIGINAL
struct _HTStream {
      CONST HTStreamClass*	isa;
      /* ... */
};
#endif

/* this version used by the NetToText stream */
struct _HTStream {
	CONST HTStreamClass *		isa;
	BOOL			had_cr;
	HTStream * 		sink;
};


/*	Presentation methods
**	--------------------
*/

PUBLIC  HTList * HTPresentations = 0;
PUBLIC  HTPresentation* default_presentation = 0;

/*
 *	To free off the presentation list.
 */
static void free_presentations NOPARAMS;

/*	Define a presentation system command for a content-type
**	-------------------------------------------------------
*/
PUBLIC void HTSetPresentation ARGS5(
	CONST char *, representation,
	CONST char *, command,
        long,  quality,
        long,  secs, 
        long,  secs_per_byte
){

    HTPresentation * pres = (HTPresentation *)malloc(sizeof(HTPresentation));
    if (pres == NULL) outofmem(__FILE__, "HTSetPresentation");
    
    pres->rep = HTAtom_for(representation);
    pres->rep_out = WWW_PRESENT;		/* Fixed for now ... :-) */
    pres->converter = HTSaveAndExecute;		/* Fixed for now ...     */
    pres->quality = quality;
    pres->secs = secs;
    pres->secs_per_byte = secs_per_byte;
    pres->rep = HTAtom_for(representation);
    pres->command = 0;
    StrAllocCopy(pres->command, command);
   
    /*
     *	Memory leak fixed.
     *  05-28-94 Lynx 2-3-1 Garrett Arch Blythe
     */ 
    if (!HTPresentations)	{
	HTPresentations = HTList_new();
	atexit(free_presentations);
    }
    
    if (strcmp(representation, "*")==0) {
        if (default_presentation) free(default_presentation);
	default_presentation = pres;
    } else {
        HTList_addObject(HTPresentations, pres);
    }
}


/*	Define a built-in function for a content-type
**	---------------------------------------------
*/
PUBLIC void HTSetConversion ARGS6(
	CONST char *, representation_in,
	CONST char *, representation_out,
	HTConverter*,	converter,
        long,  quality,
        long,  secs, 
        long,  secs_per_byte
){

    HTPresentation * pres = (HTPresentation *)malloc(sizeof(HTPresentation));
    if (pres == NULL) outofmem(__FILE__, "HTSetConversion");
    
    pres->rep = HTAtom_for(representation_in);
    pres->rep_out = HTAtom_for(representation_out);
    pres->converter = converter;
    pres->command = NULL;		/* Fixed */
    pres->quality = quality;
    pres->secs = secs;
    pres->secs_per_byte = secs_per_byte;
    pres->command = 0;
   
    /*
     *	Memory Leak fixed.
     *	05-28-94 Lynx 2-3-1 Garrett Arch Blythe
     */ 
    if (!HTPresentations)	{
	HTPresentations = HTList_new();
	atexit(free_presentations);
    }
    
    HTList_addObject(HTPresentations, pres);
}

static void free_presentations NOARGS	{
/*
 *	Purpose:	Free the presentation list.
 *	Arguments:	void
 *	Return Value:	void
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Made to clean up Lynx's bad leakage.
 *	Revision History:
 *		05-28-94	created Lynx 2-3-1 Garrett Arch Blythe
 */

	/*
	 *	Loop through the list.
	 */
	while(!HTList_isEmpty(HTPresentations))	{
		/*
		 *	Free off each item.
		 *	May also need to free off it's items, but not sure
		 *	as of yet.
		 */
		free(HTList_removeLastObject(HTPresentations));
	}
	/*
	 *	Free the list itself.
	 */
	HTList_delete(HTPresentations);
}


/*	File buffering
**	--------------
**
**	The input file is read using the macro which can read from
**	a socket or a file.
**	The input buffer size, if large will give greater efficiency and
**	release the server faster, and if small will save space on PCs etc.
*/
#define INPUT_BUFFER_SIZE 256		/* Tradeoff */
PRIVATE char input_buffer[INPUT_BUFFER_SIZE];
PRIVATE char * input_pointer;
PRIVATE char * input_limit;
PRIVATE int input_file_number;


/*	Set up the buffering
**
**	These routines are public because they are in fact needed by
**	many parsers, and on PCs and Macs we should not duplicate
**	the static buffer area.
*/
PUBLIC void HTInitInput ARGS1 (int,file_number)
{
    input_file_number = file_number;
    input_pointer = input_limit = input_buffer;
}

PUBLIC int interrupted_in_htgetcharacter = 0;
PUBLIC char HTGetCharacter NOARGS
{
    char ch;
    interrupted_in_htgetcharacter = 0;
    do {
	if (input_pointer >= input_limit) {
	    int status = NETREAD(
		    input_file_number, input_buffer, INPUT_BUFFER_SIZE);
	    if (status <= 0) {
		if (status == 0) return (char)EOF;
#ifdef FIXME
		if (status == HT_INTERRUPTED)
		{
#ifdef DT
		  if (TRACE)
		    fprintf (stderr, "HTFormat: Interrupted in HTGetCharacter\n");
#endif

		  interrupted_in_htgetcharacter = 1;
		  return (char)EOF;
		}
#endif
#ifdef DT
		if (TRACE) fprintf(stderr,
		    "HTFormat: File read error %d\n", status);
#endif

		return (char)EOF; /* -1 is returned by UCX at end of HTTP link */
	    }
	    input_pointer = input_buffer;
	    input_limit = input_buffer + status;
	}
	ch = *input_pointer++;
    } while (ch == (char) 13); /* Ignore ASCII carriage return */
    
    return FROMASCII(ch);
}

/*	Stream the data to an ouput file as binary
*/
PUBLIC int HTOutputBinary ARGS2( int, 		input,
				  FILE *, 	output)
{
    do {
	    int status = NETREAD(
		    input, input_buffer, INPUT_BUFFER_SIZE);
	    if (status <= 0) {
		if (status == 0) return 0;
#ifdef DT
		if (TRACE) fprintf(stderr,
		    "HTFormat: File read error %d\n", status);
#endif

		return 2;			/* Error */
	    }
	    fwrite(input_buffer, sizeof(char), status, output);
    } while (YES);
}

/* match maintype/* to  any MIME type starting with maintype
 *  for example   image/gif should match image/*
 */
PRIVATE int half_match ARGS2(char *,trial_type, char *,target)
{
    char *cp=strchr(trial_type,'/');

    /* if no '/' or no '*' */
    if(!cp || *(cp+1) != '*')
	return 0;

#ifdef DT
    if(TRACE)
	fprintf(stderr,"HTFormat: comparing %s and %s for half match\n",
						      trial_type, target);
#endif

	/* main type matches */
    if(!strncmp(trial_type, target, (cp-trial_type)-1)) 
	return 1;

    return 0;
}


/*		Create a filter stack
**		---------------------
**
**	If a wildcard match is made, a temporary HTPresentation
**	structure is made to hold the destination format while the
**	new stack is generated. This is just to pass the out format to
**	MIME so far.  Storing the format of a stream in the stream might
**	be a lot neater.
**
*/
PUBLIC HTStream * HTStreamStack ARGS4(
	HTFormat,		rep_in,
	HTFormat,		rep_out,
	HTStream*,		sink,
	HTParentAnchor*,	anchor)
{
    HTAtom * wildcard = HTAtom_for("*");
    HTFormat source = WWW_SOURCE;
#ifdef DT
    if (TRACE) fprintf(stderr,
    	"HTFormat: Constructing stream stack for %s to %s\n",
	HTAtom_name(rep_in),	
	HTAtom_name(rep_out));
#endif

       /* don't return on WWW_SOURCE some people might like
	* to make use of the source!!!!  LJM
        */
     /* if (rep_out == WWW_SOURCE ||
       		rep_out == rep_in) return sink;  LJM */

     if(rep_out == rep_in) return sink;

	/* don't do anymore do it in the Lynx code at startup LJM */
    /* if (!HTPresentations) HTFormatInit(); */	/* set up the list */

    {
	int n = HTList_count(HTPresentations);
	int i;
	HTPresentation * pres, *match,
			*strong_wildcard_match=0,
			*weak_wildcard_match=0,
			*last_default_match=0,
			*strong_subtype_wildcard_match=0;

	for(i=0; i<n; i++) {
	    pres = HTList_objectAt(HTPresentations, i);
	    if (pres->rep == rep_in) {
	        if (pres->rep_out == rep_out) {
#ifdef DT
		    if(TRACE)
			fprintf(stderr,"StreamStack: found exact match: %s\n",HTAtom_name(pres->rep));
#endif

	    	    return (*pres->converter)(pres, anchor, sink);

		} else if (pres->rep_out == wildcard) {
		    if(!strong_wildcard_match)
		        strong_wildcard_match = pres;
		    /* otherwise use the first one */
#ifdef DT
		    if(TRACE)
			fprintf(stderr,"StreamStack: found strong wildcard match: %s\n",HTAtom_name(pres->rep));
#endif

		}

	    } else if(half_match(HTAtom_name(pres->rep),
						HTAtom_name(rep_in))) {
		
	        if (pres->rep_out == rep_out) {
		    if(!strong_subtype_wildcard_match)
		       strong_subtype_wildcard_match = pres;
		    /* otherwise use the first one */
#ifdef DT
		    if(TRACE)
			fprintf(stderr,"StreamStack: found strong subtype wildcard match: %s\n",HTAtom_name(pres->rep));
#endif

		}
	    }

	    if (pres->rep == WWW_SOURCE) {
		if(pres->rep_out == rep_out) {
		    if(!weak_wildcard_match)
		        weak_wildcard_match = pres;
		    /* otherwise use the first one */
#ifdef DT
		    if(TRACE)
			fprintf(stderr,"StreamStack: found weak wildcard match: %s\n",HTAtom_name(pres->rep_out));
#endif

		}
		if(pres->rep_out == wildcard) {
		    if(!last_default_match)
		        last_default_match = pres;
		    /* otherwise use the first one */
		}
	    }
	}

	match = strong_subtype_wildcard_match ? strong_subtype_wildcard_match :
		strong_wildcard_match ?	strong_wildcard_match : 
		weak_wildcard_match ? weak_wildcard_match : 
		last_default_match;
	
	if (match) {
		HTPresentation temp;
		temp = *match;			/* Specific instance */
		temp.rep = rep_in;		/* yuk */
		temp.rep_out = rep_out;		/* yuk */
#ifdef DT
		if(TRACE)
		    fprintf(stderr,"StreamStack: Using %s\n",HTAtom_name(temp.rep_out));
#endif

		return (*match->converter)(&temp, anchor, sink);
        }
    }

    return NULL;
}
	

/*		Find the cost of a filter stack
**		-------------------------------
**
**	Must return the cost of the same stack which StreamStack would set up.
**
** On entry,
**	length	The size of the data to be converted
*/
PUBLIC long HTStackValue ARGS4(
	HTFormat,		rep_in,
	HTFormat,		rep_out,
        long,                  initial_value,
	long int,		length)
{
    HTAtom * wildcard = HTAtom_for("*");

#ifdef DT
    if (TRACE) fprintf(stderr,
    	"HTFormat: Evaluating stream stack for %s worth %.3f to %s\n",
	HTAtom_name(rep_in),	initial_value,
	HTAtom_name(rep_out));
#endif

		
    if (rep_out == WWW_SOURCE ||
    	rep_out == rep_in) return 0.0;

	/* don't do anymore do it in the Lynx code at startup LJM */
    /* if (!HTPresentations) HTFormatInit(); */	/* set up the list */
    
    {
	int n = HTList_count(HTPresentations);
	int i;
	HTPresentation * pres;
	for(i=0; i<n; i++) {
	    pres = HTList_objectAt(HTPresentations, i);
	    if (pres->rep == rep_in && (
	    		pres->rep_out == rep_out ||
			pres->rep_out == wildcard)) {
                long value = initial_value * pres->quality;
		if (HTMaxSecs != 0.0)
		value = value - (length*pres->secs_per_byte + pres->secs)
			                 /HTMaxSecs;
		return value;
	    }
	}
    }
    
    return -2147483646L;		/* Really bad */

}
	

/*	Push data from a socket down a stream
**	-------------------------------------
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**   The file number given is assumed to be a TELNET stream ie containing
**   CRLF at the end of lines which need to be stripped to LF for unix
**   when the format is textual.
**
*/

PUBLIC int HTCopy ARGS2(
	int,			file_number,
	HTStream*,		sink)
{
    HTStreamClass targetClass;    
    char line[256];
    long bytes=0;
    int rv = 0;
    char * msg;

    if (loading_length == -1)
	msg = "Read %ld bytes of data.";
    else
	/* We have a loading_length. */
	msg = "Read %ld of %ld bytes of data.";


/*	Push the data down the stream
**
*/
    targetClass = *(sink->isa);	/* Copy pointers to procedures */
    
    /*	Push binary from socket down sink
    **
    **		This operation could be put into a main event loop
    */
    for(;;) {
        int status;
	extern char LYCancelDownload;

	if (LYCancelDownload) {
	    LYCancelDownload = FALSE;
	    (*targetClass._abort)(sink, NULL);
	    rv = -1;
	    goto finished;
	}

#if 0
        if (HTCheckForInterrupt())
          {
              _HTProgress ("Data transfer interrupted.");
            (*targetClass._abort)(sink, NULL);
	    if(bytes)
                rv = HT_INTERRUPTED;
	    else
		rv = -1;
	    goto finished;
          }
#endif
        status = NETREAD(file_number, input_buffer, INPUT_BUFFER_SIZE);

        if (status == 0)
        {
            break;
        }
        if (status < 0) {

            if (status == HT_INTERRUPTED)
              {
                _HTProgress ("Data transfer interrupted.");
                (*targetClass._abort)(sink, NULL);
		if(bytes)
                    rv = HT_INTERRUPTED;
	        else
		    rv = -1;
		goto finished;
              }
//            else if (SOCKET_ERRNO == ENOTCONN || SOCKET_ERRNO == ECONNRESET || SOCKET_ERRNO == EPIPE)
#if 0
            else if (SOCKET_ERRNO == EBADF)
              {
                /* Arrrrgh, HTTP 0/1 compability problem, maybe. */
		rv = -2;
	        goto finished;
              }
#endif
	    break;
	}

#ifdef NOT_ASCII
	{
	    char * p;
	    for(p = input_buffer; p < input_buffer+status; p++) {
		*p = FROMASCII(*p);
	    }
	}
#endif

	(*targetClass.put_block)(sink, input_buffer, status);

	bytes += status;
	sprintf(line, msg, bytes, loading_length);
        HTProgress(line);
    } /* next bufferload */

    // _HTProgress("Data transfer complete");
    NETCLOSE(file_number);
    rv = HT_LOADED;

finished:
    loading_length = -1;
    return(rv);

}



/*	Push data from a file pointer down a stream
**	-------------------------------------
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**
*/
PUBLIC void HTFileCopy ARGS2(
	FILE *,			fp,
	HTStream*,		sink)
{
    HTStreamClass targetClass;    

/*	Push the data down the stream
**
*/
    targetClass = *(sink->isa);	/* Copy pointers to procedures */
    
    /*	Push binary from socket down sink
    */
    for(;;) {
	int status = fread(
	       input_buffer, 1, INPUT_BUFFER_SIZE, fp);
	if (status == 0) { /* EOF or error */
	    if (ferror(fp) == 0) break;
#ifdef DT
	    if (TRACE) fprintf(stderr,
		"HTFormat: Read error, read returns %d\n", ferror(fp));
#endif

	    break;
	}
	(*targetClass.put_block)(sink, input_buffer, status);
    } /* next bufferload */
	
}




/*	Push data from a socket down a stream STRIPPING CR
**	--------------------------------------------------
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the socket.
**
**   The file number given is assumed to be a TELNET stream ie containing
**   CRLF at the end of lines which need to be stripped to LF for unix
**   when the format is textual.
**
*/
PUBLIC void HTCopyNoCR ARGS2(
	int,			file_number,
	HTStream*,		sink)
{
    HTStreamClass targetClass;    
    
/*	Push the data, ignoring CRLF, down the stream
**
*/
    targetClass = *(sink->isa);	/* Copy pointers to procedures */

/*	Push text from telnet socket down sink
**
**	@@@@@ To push strings could be faster? (especially is we
**	cheat and don't ignore CR! :-}
*/  
    HTInitInput(file_number);
    for(;;) {
	char character;
	character = HTGetCharacter();
	if (character == (char)EOF) break;
	(*targetClass.put_character)(sink, character);           
    }
}



/*	Parse a socket given format and file number
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**   The file number given is assumed to be a TELNET stream ie containing
**   CRLF at the end of lines which need to be stripped to LF for unix
**   when the format is textual.
**
*/
PUBLIC int HTParseSocket ARGS5(
	HTFormat,		rep_in,
	HTFormat,		format_out,
	HTParentAnchor *,	anchor,
	int,			file_number,
	HTStream*,		sink)
{
    HTStream * stream;
    HTStreamClass targetClass;
    int rv;
    extern char LYCancelDownload;

    stream = HTStreamStack(rep_in,
			format_out,
	 		sink , anchor);
    
    if (!stream) {
		char buffer[128];	/* @@@@@@@@ */
        if (LYCancelDownload) {
			LYCancelDownload = FALSE;
			return -1;
		}
		sprintf(buffer, "Sorry, can't convert from %s to %s.",
				HTAtom_name(rep_in), HTAtom_name(format_out));
#ifdef DT
		if (TRACE) fprintf(stderr, "HTFormat: %s\n", buffer);
#endif

        return HTLoadError(sink, 501, buffer); /* returns -501 */
    }
    
/*
**	Push the data, don't worry about CRLF we can strip them later.
*/
    targetClass = *(stream->isa);	/* Copy pointers to procedures */
    rv = HTCopy(file_number, stream);
    if (rv != -1 && rv != HT_INTERRUPTED) (*targetClass._free)(stream);
    
    return rv; /* full: HT_LOADED;  partial: HT_INTERRUPTED;  no bytes: -1 */
}



/*	Parse a file given format and file pointer
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**   The file number given is assumed to be a TELNET stream ie containing
**   CRLF at the end of lines which need to be stripped to \n for unix
**   when the format is textual.
**
*/
PUBLIC int HTParseFile ARGS5(
	HTFormat,		rep_in,
	HTFormat,		format_out,
	HTParentAnchor *,	anchor,
	FILE *,			fp,
	HTStream*,		sink)
{
    HTStream * stream;
    HTStreamClass targetClass;    

    stream = HTStreamStack(rep_in,
			format_out,
	 		sink , anchor);
    
    if (!stream) {
        char buffer[128];	/* @@@@@@@@ */
	extern char LYCancelDownload;
        if (LYCancelDownload) {
	    LYCancelDownload = FALSE;
	    return -1;
	}
	sprintf(buffer, "Sorry, can't convert from %s to %s.",
		HTAtom_name(rep_in), HTAtom_name(format_out));
#ifdef DT
	if (TRACE) fprintf(stderr, "HTFormat(in HTParseFile): %s\n", buffer);
#endif

        return HTLoadError(sink, 501, buffer);
    }
    
/*	Push the data down the stream
**
**
**   @@  Bug:  This decision ought to be made based on "encoding"
**   rather than on content-type.  @@@  When we handle encoding.
**   The current method smells anyway.
*/
    targetClass = *(stream->isa);	/* Copy pointers to procedures */
    HTFileCopy(fp, stream);
    (*targetClass._free)(stream);
    
    return HT_LOADED;
}


/*	Converter stream: Network Telnet to internal character text
**	-----------------------------------------------------------
**
**	The input is assumed to be in ASCII, with lines delimited
**	by (13,10) pairs, These pairs are converted into (CR,LF)
**	pairs in the local representation.  The (CR,LF) sequence
**	when found is changed to a '\n' character, the internal
**	C representation of a new line.
*/


PRIVATE void NetToText_put_character ARGS2(HTStream *, me, char, net_char)
{
    char c = FROMASCII(net_char);
    if (me->had_cr) {
        if (c==LF) {
	    me->sink->isa->put_character(me->sink, '\n');	/* Newline */
	    me->had_cr = NO;
	    return;
        } else {
	    me->sink->isa->put_character(me->sink, CR);	/* leftover */
	}
    }
    me->had_cr = (c==CR);
    if (!me->had_cr)
	me->sink->isa->put_character(me->sink, c);		/* normal */
}

PRIVATE void NetToText_put_string ARGS2(HTStream *, me, CONST char *, s)
{
    CONST char * p;
    for(p=s; *p; p++) NetToText_put_character(me, *p);
}

PRIVATE void NetToText_put_block ARGS3(HTStream *, me, CONST char*, s, int, l)
{
    CONST char * p;
    for(p=s; p<(s+l); p++) NetToText_put_character(me, *p);
}

PRIVATE void NetToText_free ARGS1(HTStream *, me)
{
    (me->sink->isa->_free)(me->sink);		/* Close rest of pipe */
    free(me);
}

PRIVATE void NetToText_abort ARGS2(HTStream *, me, HTError, e)
{
    me->sink->isa->_abort(me->sink,e);		/* Abort rest of pipe */
    free(me);
}

/*	The class structure
*/
PRIVATE HTStreamClass NetToTextClass = {
    "NetToText",
    NetToText_free,
    NetToText_abort,
    NetToText_put_character,
    NetToText_put_string,
    NetToText_put_block
};

/*	The creation method
*/
PUBLIC HTStream * HTNetToText ARGS1(HTStream *, sink)
{
    HTStream* me = (HTStream*)malloc(sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "NetToText");
    me->isa = &NetToTextClass;
    
    me->had_cr = NO;
    me->sink = sink;
    return me;
}


