/*              Structured stream to Rich hypertext converter
**		============================================
**
**	This generates of a hypertext object.  It converts from the
**	structured stream interface fro HTMl events into the style-
**	oriented iunterface of the HText.h interface.  This module is
**	only used in clients and shouldnot be linked into servers.
**
**	Override this module if making a new GUI browser.
**
**   Being Overidden
**
*/
#include <unistd.h>

#include "HTUtils.h"

#include "HTML.h"

/* #define CAREFUL		 Check nesting here notreally necessary */

#include <ctype.h>

//#include <dir.h>
#include <stdio.h>  /* included by HTUtils.h -- FM */

#include "HTAtom.h"
#include "HTChunk.h"
#include "HText.h"
#include "HTStyle.h"

#include "HTAlert.h"
#include "HTMLGen.h"
#include "HTParse.h"

#include "SGML.h"

#include "HTNested.h"
#include "HTForms.h"

#include "GridText.h"

#include "HTFont.h"

#ifdef VMS
#include "LYCurses.h"
#include "HTVMSUtils.h"
#endif

#include "LYGlobalDefs.h"

#include "LYSignal.h"
#include "LYUtils.h"

#include "LYexit.h"

#include "LYCharSets.c"

#include "LYLeaks.h"

static char *LastOptionValue=NULL;

/* from Curses.h */
extern int LYcols;

extern HTStyleSheet * styleSheet;	/* Application-wide */
PUBLIC char HTML_Last_Char='\0';

/*	Module-wide style cache
*/
PRIVATE int got_styles = 0;
PRIVATE HTStyle *styles[HTML_ELEMENTS+25];  /* adding 24 nested list styles */
PRIVATE HTStyle *default_style;
//PRIVATE char HTML_Last_Char='\0'; /* the last character put on the screen */
PRIVATE char *textarea_name=0;
PRIVATE char *textarea_cols=0;
PRIVATE int textarea_rows=4;
PRIVATE BOOLEAN LastOptionChecked=FALSE;
PRIVATE BOOLEAN B_hide_mail_header=FALSE;
PRIVATE char *base_href=NULL;

/*      Track if we are in an anchor, paragraph, address, base.
 */
PRIVATE BOOLEAN B_inA = FALSE;
PRIVATE BOOLEAN B_inBoldH = FALSE;
PRIVATE BOOLEAN B_inBoldA = FALSE;
PRIVATE BOOLEAN B_inUnderline = FALSE;
PRIVATE BOOLEAN B_inP = FALSE;
PRIVATE BOOLEAN B_inADDRESS = FALSE;
PRIVATE BOOLEAN B_inBASE = FALSE;
PRIVATE BOOLEAN B_inFORM = FALSE;
PRIVATE BOOLEAN B_inSELECT = FALSE;
PRIVATE BOOLEAN B_inTEXTAREA = FALSE;

/* used for nested lists */
PRIVATE int List_Nesting_Level= -1;  /* counter for list nesting level */

/* used to turn off a style if the HTML author forgot to
PRIVATE int i_prior_style = -1;
 */

/*		HTML Object
**		-----------
*/
#define MAX_NESTING 800		/* Should be checked by parser */

/*
 *	Private function....
 */
PRIVATE void HTML_end_element PARAMS((HTStructured *me, int this_int_here));
PRIVATE void HTML_put_entity PARAMS((HTStructured *me, int entity_number));

typedef struct _stack_element {
        HTStyle *	style;
	int		tag_number;
} stack_element;

struct _HTStructured {
    CONST HTStructuredClass * 	isa;
    HTParentAnchor * 		node_anchor;
    HText * 			text;

    HTStream*			target;			/* Output stream */
    HTStreamClass		targetClass;		/* Output routines */

    HTChunk 			title;		/* Grow by 128 */
    HTChunk			option;		/* Grow by 128 */
    HTChunk			textarea;	/* Grow by 128 */

    char *			comment_start;	/* for literate programming */
    char *			comment_end;

    HTTag *			current_tag;
    BOOL			style_change;
    HTStyle *			new_style;
    HTStyle *			old_style;
    BOOL			in_word;  /* Have just had a non-white char */
    stack_element 	stack[MAX_NESTING];
    stack_element 	*sp;		/* Style stack pointer */
};

struct _HTStream {
    CONST HTStreamClass *	isa;
    /* .... */
};

/*		Forward declarations of routines
*/
PRIVATE void get_styles NOPARAMS;


PRIVATE void actually_set_style PARAMS((HTStructured * me));
PRIVATE void change_paragraph_style PARAMS((HTStructured * me, HTStyle * style));

/*	Style buffering avoids dummy paragraph begin/ends.
*/
#define UPDATE_STYLE if (me->style_change) { actually_set_style(me); }

/* include the character sets 
 * There may be an unlimited number of sets
 * The sets will be referenced using the
 * LYCharSets array
 */

//#include "LYCharSe.c"

/*		Set character set
**		----------------
*/

//PRIVATE char** p_entity_values = ISO_Latin1;	/* Pointer to translation */

PUBLIC char * LYUnEscapeEntities PARAMS((char *str));

PUBLIC void HTMLUseCharacterSet ARGS1(int,i)
{
     p_entity_values = LYCharSets[i];
}


/*		Flattening the style structure
**		------------------------------
**
On the NeXT, and on any read-only browser, it is simpler for the text to have
a sequence of styles, rather than a nested tree of styles. In this
case we have to flatten the structure as it arrives from SGML tags into
a sequence of styles.
*/

/*		If style really needs to be set, call this
*/
PRIVATE void actually_set_style ARGS1(HTStructured *, me)
{
    if (!me->text) {			/* First time through */
	    me->text = HText_new2(me->node_anchor, me->target);
	    HText_beginAppend(me->text);
	    HText_setStyle(me->text, me->new_style);
	    me->in_word = NO;
    } else {
	    HText_setStyle(me->text, me->new_style);
    }

    me->old_style = me->new_style;
    me->style_change = NO;

}

/*      If you THINK you need to change style, call this
*/

PRIVATE void change_paragraph_style ARGS2(HTStructured *, me, HTStyle *,style)
{
    if (me->new_style!=style) {
    	me->style_change = YES;
	me->new_style = style;
    }
    me->in_word = NO;
}

/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/

PRIVATE void HTML_put_character ARGS2(HTStructured *, me, char, c)
{

    switch (me->sp[0].tag_number) {

    case HTML_COMMENT:
	return;					/* Do Nothing */
    case HTML_STYLE:
	return;					/* Do Nothing */
    case HTML_SCRIPT:
	return;					/* Do Nothing */
    case HTML_APPLET:
	return;					/* Do Nothing */

	case HTML_MH:   {
		/*
                 *      Save the mail header into the form,
                 *      And possibly display.
                 */
		HText_appendMHChar(me->text, c);
		if(B_hide_mail_header == FALSE)	{
			HText_appendCharacter(me->text, c);
		}
		break;
        }

    case HTML_TITLE:
	if(c != '\n' && c != '\t')
	    HTChunkPutc(&me->title, c);
	else
    	    HTChunkPutc(&me->title, ' ');
	break;

    case HTML_SELECT:	
	if(c != '\n' && c != '\t')
            HTChunkPutc(&me->option, c);
        else
	    HTChunkPutc(&me->option, ' ');
	break;

    case HTML_TEXTAREA:	
    	HTChunkPutc(&me->textarea, c);
	break;

    case HTML_LISTING:				/* Litteral text */
    case HTML_XMP:
    case HTML_PLAINTEXT:
    case HTML_PRE:
/*	We guarrantee that the style is up-to-date in begin_litteral
	But we still want to strip \r's
*/
	if(c != '\r')	{
	    B_inP = TRUE; 
	    HText_appendCharacter(me->text, c);
	}
	break;
	
    default:					/* Free format text */
	if(!strcmp(me->sp->style->name,"Preformatted") ||
		!strcmp(me->sp->style->name,"Listing") ||
		!strcmp(me->sp->style->name,"Example")) {

	    if(c != '\r')	{
		B_inP = TRUE; 
		HText_appendCharacter(me->text, c);
	    }
	
	} else {

	    if (me->style_change) {
	        if ((c=='\n') || (c==' ')) return;	/* Ignore it */
	        UPDATE_STYLE;
	    }
	    if (c=='\n') {
	        if (me->in_word) {
	            if(HTML_Last_Char != ' ')	{
			B_inP = TRUE;
		        HText_appendCharacter(me->text, ' ');
		    }
		    me->in_word = NO;
	        }

	    } else if (c==' ' || c=='\t') {
	        if(HTML_Last_Char != ' ')	{
		    B_inP = TRUE; 
	            HText_appendCharacter(me->text, ' ');
		}

	    } else if (c=='\r') {
	       /* ignore */
	    } else {
		B_inP = TRUE;
	        HText_appendCharacter(me->text, c);
	        me->in_word = YES;
	    }
	}
    } /* end switch */

    if(c=='\n' || c=='\t') {
     	HTML_Last_Char=' '; /* set it to a generic seperater */

	/* \r's are ignored.  In order to keep collapsing spaces
	 * correctly we must default back to the previous
	 * seperater if there was one
	 */
    } else if(c=='\r' && HTML_Last_Char == ' ') {
     	    HTML_Last_Char=' '; /* set it to a generic seperater */
    } else {
     	HTML_Last_Char = c;
    }
}



/*	String handling
**	---------------
**
**	This is written separately from put_character becuase the loop can
**	in some cases be promoted to a higher function call level for speed.
*/
PRIVATE void HTML_put_string ARGS2(HTStructured *, me, CONST char*, s)
{
   if(s==NULL)
      return;

    switch (me->sp[0].tag_number) {

    case HTML_COMMENT:
	break;					/* Do Nothing */
    case HTML_STYLE:
	break;					/* Do Nothing */
    case HTML_SCRIPT:
	break;					/* Do Nothing */
    case HTML_APPLET:
	break;					/* Do Nothing */

	case HTML_MH:	{
		/*
		 *	Save the mail header into the form,
		 *	And possibly display, just feed to HTML_put_character.
		 */
		char *cp_s = (char *)s;
		while(*cp_s != '\0')	{
			HTML_put_character(me, *cp_s);
			cp_s++;
		}
		break;
	}
    case HTML_TITLE:	
    	HTChunkPuts(&me->title, s);
	break;

    case HTML_SELECT:	
	HTChunkPuts(&me->option, s);
	break;
	
    case HTML_TEXTAREA:	
    	HTChunkPuts(&me->textarea, s);
	break;

    case HTML_LISTING:				/* Litteral text */
    case HTML_XMP:
    case HTML_PLAINTEXT:
    case HTML_PRE:

/*	We guarrantee that the style is up-to-date in begin_litteral
*/
    	HText_appendText(me->text, s);
	break;
	
    default:					/* Free format text */
        {
	    CONST char *p = s;
	    if (me->style_change) {
		/* Ignore leaders */
		for (; *p && ((*p=='\n') || (*p==' ') || (*p=='\t')); p++);
		if (!*p) return;
		UPDATE_STYLE;
	    }
	    for(; *p; p++) {
		if (me->style_change) {
		    if ((*p=='\n') || (*p==' ') || (*p=='\t'))
			continue;  /* Ignore it */
		    UPDATE_STYLE;
		}
		if (*p=='\n') {
		    if (me->in_word) {
		        if(HTML_Last_Char!=' ')
			    HText_appendCharacter(me->text, ' ');
			me->in_word = NO;
		    }

		} else if (*p==' ' || *p=='\t') {
		   if(HTML_Last_Char!=' ')
			HText_appendCharacter(me->text, ' ');
			
		} else if (*p=='\r') {
			/* ignore */
		} else {
		    HText_appendCharacter(me->text, *p);
		    me->in_word = YES;
		}

		/* set the Last Character */
    		if(*p=='\n' || *p=='\t') {
        	    HTML_Last_Char=' '; /* set it to a generic seperater */

        		/* \r's are ignored.  In order to keep 
			 * collapsing spaces
         		 * correctly we must default back to the previous
         		 * seperater if there was one
         		 */
    		} else if(*p=='\r' && HTML_Last_Char == ' ') {
		    HTML_Last_Char=' '; /* set it to a generic seperater */
    		} else {
       		    HTML_Last_Char = *p;
    		}

	    } /* for */
	}
    } /* end switch */
}


/*	Buffer write
**	------------
*/
PRIVATE void HTML_write ARGS3(HTStructured *, me, CONST char*, s, int, l)
{
    CONST char* p;
    CONST char* e = s+l;
    for (p=s; s<e; p++) HTML_put_character(me, *p);
}


/*	Start Element
**	-------------
*/

// extern BOOLEAN HT_Is_Gopher_URL;

PRIVATE void HTML_start_element ARGS4(
	HTStructured *, 	me,
	int,		element_number,
	CONST BOOL*,	 	present,
	CONST char **,	value)
{
    static int OL_Counter[7];		   /* counter for ordered lists */
    static BOOLEAN first_option=TRUE;	   /* is this the first option tag? */
    static HTChildAnchor *B_CurrentA=NULL; /* current HTML_A anchor */

    char *alt_string = NULL;
//    char *id_string = NULL;
    char *href = NULL;
//    char *map_href = NULL;
//    char *title = NULL;
    char *temp = NULL;
    HTParentAnchor *dest = NULL;	     /* the anchor's destination */
    BOOL dest_ismap = FALSE;	     	     /* is dest an image map script? */
    HTChildAnchor *B_ID_A = NULL;	     /* HTML_foo_ID anchor */
    int url_type;


    switch (element_number) {

    case HTML_BASE:
	if (present[HTML_BASE_HREF]) {
	    char *related=NULL, *temp=NULL, *cp;

	    /*
	     *  Get parent's address for defaulted fields.
	     */
	    StrAllocCopy(related, me->node_anchor->address);

	    /* 
	     *  Create the access field.
	     */
	    if ((temp = HTParse(value[HTML_BASE_HREF], related,
				PARSE_ACCESS+PARSE_PUNCTUATION))
		&& *temp != '\0')
	        StrAllocCopy(base_href, temp);
	    else
	        StrAllocCopy(base_href, (temp=HTParse(related, "",
					 PARSE_ACCESS+PARSE_PUNCTUATION)));

	    /*
	     *  Create the host[:port] field.
	     */
	    if ((temp = HTParse(value[HTML_BASE_HREF], "",
				PARSE_HOST+PARSE_PUNCTUATION)) &&
		!strncmp(temp, "//", 2)) {
	        StrAllocCat(base_href, temp);
		if (!strcmp(base_href, "file://"))
		    StrAllocCat(base_href, "localhost");
	    } else {
		if (!strcmp(base_href, "file:"))
		    StrAllocCat(base_href, "//localhost");
		else
	            StrAllocCat(base_href, (temp=HTParse(related, "",
					    PARSE_HOST+PARSE_PUNCTUATION)));
	    }

	    /*
	     *  Create the path field.
	     */
	    if ((temp = HTParse(value[HTML_BASE_HREF], "",
				PARSE_PATH+PARSE_PUNCTUATION))
	    	&& *temp != '\0')
		StrAllocCat(base_href, temp);
	    else
		StrAllocCat(base_href, "/");

            B_inBASE = TRUE;

	    free(related);
	    free(temp);
	}
	break;

    case HTML_TR:
	UPDATE_STYLE;
	if(B_inA)	{
		UPDATE_STYLE;
		HText_endAnchor(me->text);
		B_inA = FALSE;
	}
//	if (HTML_Last_Char != '\r')
		HText_appendCharacter(me->text, '\r');
	me->in_word = NO;
	break;

    case HTML_TD:
    case HTML_TH:
	UPDATE_STYLE;
	if(B_inA)	{
		UPDATE_STYLE;
		HText_endAnchor(me->text);
		B_inA = FALSE;
	}
	if (HTML_Last_Char != ' ')
		HText_appendCharacter(me->text, ' ');
	me->in_word = NO;
	break;

    case HTML_FRAME:      {               /* frame url view hack */

     if(present[HTML_FRAME_SRC]) {
	BOOL B_anchor = FALSE;

	/*
	 *	First off, are we inside of an anchor?  Cant have that!
	 */
	if(B_inA)    {
	     me->in_word = YES;
	     UPDATE_STYLE;
	     HText_endAnchor(me->text);
	}

	B_inA = FALSE;

//	if (HTML_Last_Char != '\r')
//		HText_appendCharacter(me->text, '\r');

	HTML_put_string(me, "Frame: ");

	{

		HTChildAnchor * source;
		char * href = NULL;

		if(present[HTML_FRAME_SRC])       {
			StrAllocCopy(href, value[HTML_FRAME_SRC]);
			HTSimplify(href);
		}

		source = HTAnchor_findChildAndLink(me->node_anchor,
			NULL, present[HTML_FRAME_SRC] ? href : NULL, NULL);

		if(href) {
			free(href);
		}

		UPDATE_STYLE;
		HText_beginAnchor(me->text, source);
		if (B_inBoldH == FALSE){
		   HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
		   B_inBoldH = TRUE;
		}


		/*
		 * 	Decide how to name the anchor.
		 */
		if((present[HTML_FRAME_NAME]) &&
		  (value[HTML_FRAME_NAME] != NULL) &&
		  (*(value[HTML_FRAME_NAME]) != '\0'))
		{
			/*
			 *      Use the NAME text.
			 */
			HTML_put_string(me,
				LYUnEscapeEntities(
					value[HTML_FRAME_NAME]));
		}
		else    {
			/*
			 *      Use the SRC URL
			 */
			HTML_put_string(me, value[HTML_FRAME_SRC]);
		}

		HText_endAnchor(me->text);
		HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		B_inBoldH = FALSE;
/* wsb */
        if (B_inUnderline == TRUE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
//		HTML_Last_Char = ' ';  /* absorb white space */
		HText_appendCharacter(me->text,'\r');
		me->in_word = NO;
//		crcheck = 1;
	}

     }
     break;
    }

    case HTML_AREA:      {               /* client side map hack */

	BOOL B_anchor = FALSE;

	/*
	 *	First off, are we inside of an anchor?  Cant have that!
	 */
	if(B_inA)    {
	     me->in_word = YES;
	     UPDATE_STYLE;
	     HText_endAnchor(me->text);
	}

	B_inA = FALSE;


//	if (HTML_Last_Char != '\r')
//		HText_appendCharacter(me->text, '\r');

	HTML_put_string(me, "Imagemap: ");

	{

		HTChildAnchor * source;
		char * href = NULL;


		if(present[HTML_AREA_HREF])       {
			StrAllocCopy(href, value[HTML_AREA_HREF]);
			HTSimplify(href);
		}

		source = HTAnchor_findChildAndLink(
			me->node_anchor,
			NULL, present[HTML_AREA_HREF] ? href : NULL, NULL);

		if(href) {
			free(href);
		}

		UPDATE_STYLE;
		HText_beginAnchor(me->text, source);

	    if (B_inBoldH == FALSE){
		HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
		B_inBoldH = TRUE;
		}


		/*
		 * 	Decide how to name the anchor.
		 */
		if((present[HTML_AREA_ALT]) &&
		  (value[HTML_AREA_ALT] != NULL) &&
		  (*(value[HTML_AREA_ALT]) != '\0'))
		{
			/*
			 *      Use the ALT text.
			 */
			HTML_put_string(me,
				LYUnEscapeEntities(
					value[HTML_AREA_ALT]));
		}
		else    {
			/*
			 *      Use the SRC URL
			 */
			HTML_put_string(me, value[HTML_AREA_HREF]);
		}

		HText_endAnchor(me->text);
		HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		B_inBoldH = FALSE;
/* wsb */
        if (B_inUnderline == TRUE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
		HText_appendCharacter(me->text,'\r');
		me->in_word = NO;
//		HTML_Last_Char = ' ';  /* absorb white space */
//		crcheck = 1;
	}
	break;
    }

    case HTML_A:
	{

	    char * href = NULL;
	    char * temp = NULL;

	    /*	Set to know we are in an anchor.
	     */
	    B_inA = TRUE;

	    if (present[HTML_A_HREF]) {

		/*
		 * Check whether a base tag is in effect.
		 */
		if ((B_inBASE) &&
		    (temp = HTParse(value[HTML_A_HREF], base_href,
					    PARSE_ALL)) && *temp != '\0')
		    /*
		     *  Use reference related to the base.
		     */
		    StrAllocCopy(href, temp);
		else
		    /*
		     *  Use anchor's reference as is.
		     */
		    StrAllocCopy(href, value[HTML_A_HREF] ?
				       value[HTML_A_HREF] : 0);
//                                       value[HTML_A_HREF] : "");

		/*
		 * If it's a file URL and the host is defaulted,
		 * force in "//localhost".  We need this until
		 * all the other Lynx code which performs security
		 * checks based on the "localhost" string is changed
		 * to assume "//localhost" when a host field is not
		 * present in file URLs - FM
		 */
		if(!strcmp(href, "//") || !strncmp(href, "///", 3)) {
		    if((B_inBASE && !strncmp(base_href, "file:", 5)) ||
		       (!B_inBASE &&
			!strncmp(me->node_anchor->address, "file:", 5))) {
			StrAllocCopy(temp, "file:");
			StrAllocCat(temp, href);
			StrAllocCopy(href, temp);
		    }
		}
		if (!strncmp(href, "file:", 5)) {
		    if (href[5] == '\0') {
			StrAllocCat(href, "//localhost");
		    } else if(!strcmp(href, "file://")) {
			StrAllocCat(href, "localhost");
		    } else if(!strncmp(href, "file:///", 8)) {
		        StrAllocCopy(temp, (href+7));
			StrAllocCopy(href, "file://localhost");
			StrAllocCat(href, temp);
		    } else if(!strncmp(href, "file:/", 6) && href[6] != '/') {
			StrAllocCopy(temp, (href+5));
			StrAllocCopy(href, "file://localhost");
		        StrAllocCat(href, temp);
		    }
		}

		/*
		 * No path in a file://localhost URL means a
		 * directory listing for the current default. - FM
		 */
		if(!strcmp(href, "file://localhost")) {
#ifdef VMS
		    StrAllocCat(href, HTVMS_wwwName(getenv("PATH")));
#else
		    char curdir[DIRNAMESIZE];
#ifdef NO_GETCWD
		    getwd (curdir);
#else
		    getcwd (curdir, DIRNAMESIZE);
#endif /* NO_GETCWD */
		    StrAllocCat(href, curdir);
#endif /* VMS */
		}

#ifdef VMS
		/*
		 * On VMS, a file://localhost/ URL means
		 * a listing for the login directory. - FM
		 */
		if(!strcmp(href, "file://localhost/"))
		    StrAllocCat(href, (HTVMS_wwwName(getenv("HOME"))+1));
#endif /* VMS */

		if (temp)
		    free(temp);

		/*
		 *  Don't simplify gopher url's.
		 */
//		if(!HT_Is_Gopher_URL)
		    HTSimplify(href);
//		else
//		    HT_Is_Gopher_URL= FALSE;
	    }

	    B_CurrentA = HTAnchor_findChildAndLink(
		me->node_anchor,				/* parent */
		present[HTML_A_NAME] ? value[HTML_A_NAME] : NULL,	/* Tag */
		present[HTML_A_HREF] ? href : NULL,		/* Addresss */
		present[HTML_A_TYPE] && value[HTML_A_TYPE] ?
			(HTLinkType*)HTAtom_for(value[HTML_A_TYPE])
						: NULL);

	    /*
	     *	Get rid of href since no longer needed.
	     *	Memory leak fixed
	     *	06-16-94 Lynx 2-3-1 Garrett Arch Blythe
	     */
	    if (href)
		free(href);

	    if (present[HTML_A_TITLE] && value[HTML_A_TITLE]) {
		HTParentAnchor * dest =
		    HTAnchor_parent(
			HTAnchor_followMainLink((HTAnchor*)B_CurrentA)
				    );
		if (!HTAnchor_title(dest))
			HTAnchor_setTitle(dest, value[HTML_A_TITLE]);
	    }
	    UPDATE_STYLE;
	    HText_beginAnchor(me->text, B_CurrentA);
	    if (B_inBoldH == FALSE)
		HText_appendCharacter(me->text,LY_BOLD_START_CHAR);

	}
    	break;

    case HTML_FORM:
	{
	    char * action = NULL;
	    char * method = NULL;
	    char * temp   = NULL;
	    HTChildAnchor * source;
	    HTAnchor *link_dest;

	    /*	Set to know we are in a form.
	     */
	    B_inFORM = TRUE;

	    if (present[HTML_FORM_ACTION])  {
		/*
		 * Check whether a base tag is in effect.
		 */
		if ((B_inBASE) &&
		    (temp = HTParse(value[HTML_FORM_ACTION], base_href,
					    PARSE_ALL)) && *temp != '\0')
		    /*
		     *  Use action related to the base.
		     */
		    StrAllocCopy(action, temp);
		else
		    /*
		     *  Use form's action as is.
		     */
		    StrAllocCopy(action, value[HTML_FORM_ACTION] ?
					 value[HTML_FORM_ACTION] : "");
		if (temp)
		    free(temp);

		/*
		 *  Don't simplify gopher url's as actions/
		 */
#ifdef USEGOPHER
		if(strncmp(action,"gopher",6))
#endif
		    HTSimplify(action);

		source = HTAnchor_findChildAndLink(me->node_anchor,
								NULL, action, NULL);
		link_dest = HTAnchor_followMainLink((HTAnchor *)source);
		{
			/*
			 *	Memory leak fixed.
			 *	05-28-94 Lynx 2-3-1 Garrett Arch Blythe
			 */
			auto char *cp_freeme = HTAnchor_address(link_dest);
			if(cp_freeme != NULL)	{
				StrAllocCopy(action, cp_freeme);
				free(cp_freeme);
			}
			else	{
				StrAllocCopy(action, "");
			}
		}
	    }
	    if (present[HTML_FORM_METHOD])
		StrAllocCopy(method, value[HTML_FORM_METHOD] ?
				     value[HTML_FORM_METHOD] : "GET");

	    HText_beginForm(action, method);

	    if (action)
	        free(action);
	    if(method)
		free(method);
	}
	break;

	case HTML_MH:	{
		/*
		 *	Check to see if should hide the mail header.
		 *	The actual mail header will be captured and put into
		 *		the form in HTML_put* functions.
		 *	Non hidden mail headers will be displayed also.
		 */
		if(present[HTML_MH_HIDDEN])	{
			B_hide_mail_header = TRUE;
		}
		else	{
			B_hide_mail_header = FALSE;
		}
#ifdef DT
		if(TRACE)	{
			fprintf(stderr, "MH encountered %d\n",
				B_hide_mail_header);
		}
#endif

		break;
	}
    case HTML_INPUT:
	{
	    InputFieldData I;
	    int chars;

	    /* Make sure we're in a form. */
	    if (!B_inFORM) {
#ifdef DT
	        if (TRACE) {
		    fprintf(stderr, "HTML: INPUT tag not within FORM tag\n");
		} else
		{
		    _statusline("** Bad HTML!!  Use -trace to diagnose. **");
		    sleep(sleep_two);
		}
#endif

//                break;
	    }

	    /* Check for unclosed TEXTAREA */
	    if (B_inTEXTAREA) {
#ifdef DT
		if (TRACE) {
		    fprintf(stderr, "HTML: Missing TEXTAREA end tag\n");
		} else

		{
		    _statusline("** Bad HTML!!  Use -trace to diagnose. **");
		    sleep(sleep_two);
		}
#endif
	    }

	    /* init */
	    I.name=NULL; I.type=NULL; I.value=NULL;
	    I.checked=NO; I.size=NULL; I.maxlength=NULL;

	    /* before any input field add a space if necessary */
	    UPDATE_STYLE;
	    HTML_put_character(me,' ');

	    if (present[HTML_INPUT_NAME])  
		I.name = (char *)value[HTML_INPUT_NAME];
	    else
	        I.name = "";
	    if (present[HTML_INPUT_TYPE]) 
		I.type = (char *)value[HTML_INPUT_TYPE];
	    if (present[HTML_INPUT_VALUE]) {
		/*
		 * Convert any HTML entities or decimal escaping. - FM
		 */
		LYUnEscapeEntities((char *)value[HTML_INPUT_VALUE]);
		I.value = (char *)value[HTML_INPUT_VALUE];
	    }
	    if (present[HTML_INPUT_CHECKED])
		I.checked = YES;
	    if (present[HTML_INPUT_SIZE])
		I.size = (char *)value[HTML_INPUT_SIZE];
	    if (present[HTML_INPUT_MAXLENGTH])
		I.maxlength = (char *)value[HTML_INPUT_MAXLENGTH];

	    chars = HText_beginInput(me->text, &I);
	    for(; chars>0; chars--)
		HTML_put_character(me, '_');
	}
	break;

    case HTML_TEXTAREA:
	/* Make sure we're in a form. */
	if (!B_inFORM) {
#ifdef DT
	    if (TRACE) {
		fprintf(stderr,
			"HTML: TEXTAREA start tag not within FORM tag\n");
	    } else
	    {
		_statusline("** Bad HTML!!  Use -trace to diagnose. **");
		sleep(sleep_two);
	    }
#endif
	    break;
	}

	/* Set to know we are in a textarea.
	 */
	B_inTEXTAREA = TRUE;

	    /* Get ready for the value */
        HTChunkClear(&me->textarea);
	if (present[HTML_TEXTAREA_NAME] && value[HTML_TEXTAREA_NAME])
	    StrAllocCopy(textarea_name, value[HTML_TEXTAREA_NAME]);
	else
	    textarea_name = "";

	if (present[HTML_TEXTAREA_COLS] && *value[HTML_TEXTAREA_COLS])
	    StrAllocCopy(textarea_cols, value[HTML_TEXTAREA_COLS]);
	else
	    StrAllocCopy(textarea_cols, "60");

	if (present[HTML_TEXTAREA_ROWS] && *value[HTML_TEXTAREA_ROWS])  
	    textarea_rows = atoi(value[HTML_TEXTAREA_ROWS]);
	else
	    textarea_rows = 4;
	break;


    case HTML_SELECT:
	{
	    char *name = NULL;
	    BOOLEAN multiple=NO;
	    char *size = NULL;

	    /* Make sure we're in a form. */
	    if (!B_inFORM) {
#ifdef DT
	        if (TRACE) {
		    fprintf(stderr,
			    "HTML: SELECT start tag not within FORM tag\n");
		} else
		{
		    _statusline("** Bad HTML!!  Use -trace to diagnose. **");
		    sleep(sleep_two);
		}
#endif

		break;
	    }

	    /* Check for unclosed TEXTAREA */
	    if (B_inTEXTAREA) {
#ifdef DT
	        if (TRACE) {
		    fprintf(stderr, "HTML: Missing TEXTAREA end tag\n");
		} else
		{
		    _statusline("** Bad HTML!!  Use -trace to diagnose. **");
		    sleep(sleep_two);
		}
#endif
	    }

	    /*	Set to know we are in a select tag.
	     */
	    B_inSELECT = TRUE;

	    if (present[HTML_SELECT_NAME])  
		StrAllocCopy(name, value[HTML_SELECT_NAME] ?
				   value[HTML_SELECT_NAME] : "");
	    if (present[HTML_SELECT_MULTIPLE])
		multiple=YES;
	    if(present[HTML_SELECT_SIZE] && *value[HTML_SELECT_SIZE]) {
		StrAllocCopy(size, value[HTML_SELECT_SIZE]);
	    }

	    if (B_inBoldH == TRUE && multiple == NO) {
		HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		B_inBoldH = FALSE;
	    }
	    if (B_inUnderline == TRUE && multiple == NO) {
	        HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
		B_inUnderline = FALSE;
	    }
	    HText_beginSelect(name, multiple, size);

	    first_option = TRUE;
	}
	break;

    case HTML_OPTION:
	{
	    /* an option is a special case of an input field */
	    InputFieldData I;
	    int i;


	    /* Make sure we're in a select tag. */
	    if (!B_inSELECT) {
#ifdef DT
	        if (TRACE) {
		    fprintf(stderr,
			    "HTML: OPTION tag not within SELECT tag\n");
		} else
		{
		    _statusline("** Bad HTML!!  Use -trace to diagnose. **");
		    sleep(sleep_two);
		}
#endif
		break;
	    }

	    /* needed in case we have a <form><select><option>
	     * to begin a doc so that we don't crash when we try
	     * to use a nonexistent HText - CL
	     */
	    UPDATE_STYLE;

	    if(!first_option) {
		/* finish the data off */
       	        HTChunkTerminate(&me->option);
		/* finish the previous option @@@@@ */
	        HText_setLastOptionValue(me->text,
					 me->option.data, LastOptionValue,
				         MIDDLE_ORDER, LastOptionChecked);
	    }

	    /* if its not a multiple option list then don't
	     * use the checkbox method, and don't put
	     * anything on the screen yet.
	     */
	    if(first_option || HTCurSelectGroupType == F_CHECKBOX_TYPE) {


		if(HTCurSelectGroupType == F_CHECKBOX_TYPE)
	            /* start a newline before each option */
		    HText_appendCharacter(me->text,'\r');
		else {
		    /* add option list designation character */
		    HText_appendCharacter(me->text,'[');
		}

                /* init */
                I.name=NULL; I.type=NULL; I.value=NULL;
                I.checked=NO; I.size=NULL; I.maxlength=NULL;

		I.type = "OPTION";
    
	        if(present[HTML_OPTION_SELECTED])
		    I.checked=YES;

		if(present[HTML_OPTION_VALUE])	{
		    /*
		     * Convert any HTML entities or decimal escaping. - FM
		     */
		    LYUnEscapeEntities((char *)value[HTML_OPTION_VALUE]);
		    I.value = (char *)value[HTML_OPTION_VALUE];
		}

		HText_beginInput(me->text, &I);
    
	        first_option = FALSE;

		if(HTCurSelectGroupType == F_CHECKBOX_TYPE) {
	        /* put 3 underscores and one space before each option */
                    for(i=0; i<3; i++)
			HText_appendCharacter(me->text, '_');
	            HText_appendCharacter(me->text, ' ');
		}
	    }

	    /* Get ready for the next value */
	    HTChunkClear(&me->option);
	    if(present[HTML_OPTION_SELECTED])
		LastOptionChecked=YES;
	    else
		LastOptionChecked=NO;

	    if (present[HTML_OPTION_VALUE] && value[HTML_OPTION_VALUE])
		StrAllocCopy(LastOptionValue, value[HTML_OPTION_VALUE]);
	    else
	        StrAllocCopy(LastOptionValue, me->option.data);
	}
	break;

    case HTML_LINK:
	if (present[HTML_LINK_HREF]) {
	    char * href = NULL;

	    StrAllocCopy(href, value[HTML_A_HREF] ?
	    		       value[HTML_A_HREF] : "");
	    HTSimplify(href);

	    if(present[HTML_LINK_REV]) {
	        if(!strcasecomp("made", value[HTML_LINK_REV]) ||
		   !strcasecomp("owner", value[HTML_LINK_REV]))
		    HTAnchor_setOwner(me->node_anchor, href);

#ifdef DT
		if(TRACE)
		   fprintf(stderr,"HTML.c: DOC OWNER found\n");
#endif

	    }
	    free(href);
	}
	break;
	
    case HTML_TITLE:
        HTChunkClear(&me->title);
	List_Nesting_Level = -1;
	break;

    case HTML_NEXTID:
    	/* if (present[NEXTID_N] && value[NEXTID_N])
		HText_setNextId(me->text, atoi(value[NEXTID_N])); */
	break;
	
    case HTML_ISINDEX:
	if(present && (present[HTML_ISINDEX_HREF] ||
		       present[HTML_ISINDEX_ACTION])) {
	    char * action = NULL;
	    char * isindex_href = NULL;

	    /*
	     *  Lynx was supporting ACTION, which never made it into
	     *  the HTTP 2.0 specs.  HTTP 3.0 uses HREF, so we'll
	     *  use that too, but allow use of ACTION as an alternate
	     *  until people have fully switched over. - FM
	     */
	    if (present[HTML_ISINDEX_HREF])
	        StrAllocCopy(isindex_href, value[HTML_ISINDEX_HREF]);
	    else
	        StrAllocCopy(isindex_href, value[HTML_ISINDEX_ACTION]);

	    /*
	     * Check whether a base tag is in effect.
	     */
	    if (B_inBASE)
		action = HTParse((isindex_href ? isindex_href : ""),
				 base_href, PARSE_ALL);
	    else {
	        char *related=NULL;

		StrAllocCopy(related, me->node_anchor->address);
	        action = HTParse((isindex_href ? isindex_href : ""),
				related, PARSE_ALL);
		free(related);
	    }

	    /*
	     *  Don't simplify gopher url's as actions.
	     */
//	    if(strncmp(action,"gopher",6))
		HTSimplify(action);

	    HTAnchor_setIndex(me->node_anchor, action);
	    if (isindex_href)
		free(isindex_href);
	    if (action)
		free(action);

	} else {
	    if (B_inBASE)
	        /*
	         *  Use base.
		 */
   	        HTAnchor_setIndex(me->node_anchor, base_href);
	    else
	        /*
	         *  Use index's address.
	         */
		HTAnchor_setIndex(me->node_anchor, me->node_anchor->address);
	}
	/*
	 *  Support HTML 3.0 PROMPT attribute. - FM
	 */
	if (present && present[HTML_ISINDEX_PROMPT]) {
	    int last;
	    HTAnchor_setPrompt(me->node_anchor,
	    		       (char *)value[HTML_ISINDEX_PROMPT]);
	    last = (strlen(me->node_anchor->isIndexPrompt) - 1);
	    if (me->node_anchor->isIndexPrompt[last] != ' ')
	        StrAllocCat(me->node_anchor->isIndexPrompt, " ");
	} else {
	    HTAnchor_setPrompt(me->node_anchor, "Enter a database query: ");
	}
	break;
	
    case HTML_MAP:
    case HTML_P:
	/*	Mark that we are no longer in a paragraph.
	 */
	B_inP = FALSE;

	UPDATE_STYLE;
	/* HText_appendParagraph(me->text); */
	/* since everyone seems to use the paragraph tag to mean
	 * insert two linebreaks, I am changing its meaning now
	 * to mean two line breaks :(  It's so sad....
	 */
	if (HTML_Last_Char != '\r')
		HText_appendCharacter(me->text, '\r');

	/*	If we're in an address, only do one \r
	 */
	if(B_inADDRESS != TRUE) {
		HText_appendCharacter(me->text, '\r');
	}
	me->in_word = NO;
	break;

    case HTML_DL:
	List_Nesting_Level++;  /* increment the List nesting level */
	if(List_Nesting_Level <= 0) {
	    change_paragraph_style(me, present && present[DL_COMPACT]
    			              ? styles[HTML_DLC] : styles[HTML_DL]);

	} else if(List_Nesting_Level >= 6) {
	    change_paragraph_style(me, present && present[DL_COMPACT]
    			              ? styles[HTML_DLC6] : styles[HTML_DL6]);

	} else {
            change_paragraph_style(me, present && present[DL_COMPACT]
    		 ? styles[(HTML_DLC1 - 1) + List_Nesting_Level] 
		 : styles[(HTML_DL1 - 1) + List_Nesting_Level]);
	}
	break;
	
    case HTML_DLC:
        List_Nesting_Level++;  /* increment the List nesting level */
        if(List_Nesting_Level <= 0) {
	    change_paragraph_style(me, styles[HTML_DLC]);

        } else if(List_Nesting_Level >= 6) {
	    change_paragraph_style(me, styles[HTML_DLC6]);

        } else {
            change_paragraph_style(me, 
			       styles[(HTML_DLC1 - 1) + List_Nesting_Level]);
        }
        break;

    case HTML_DT:
        if (!me->style_change) {
	    HText_appendParagraph(me->text);
	    me->in_word = NO;
	}
	break;
	
    case HTML_DD:
	HTML_Last_Char = ' ';  /* absorb white space */
	if (!me->style_change)
	    HText_appendCharacter(me->text, '\r');
	else 
	  {
            UPDATE_STYLE;
	    HText_appendCharacter(me->text, '\t');
	  }
	me->in_word = NO;
	break;

    case HTML_BR:
        UPDATE_STYLE;
	HTML_Last_Char = ' ';  /* absorb white space */
	HText_appendCharacter(me->text, '\r');
	break;

    case HTML_HR:
        UPDATE_STYLE;
	{
	    register int i;
	    HText_appendCharacter(me->text, ' '); /* dummy white space */
	    HText_appendCharacter(me->text, '\r');
	    HText_appendCharacter(me->text, ' ');
	    HText_appendCharacter(me->text, ' ');

	    for(i=me->new_style->leftIndent+2;
			i < LYcols-(me->new_style->rightIndent+4);  i++)
		HTML_put_character(me, '_');

	    HTML_put_character(me, ' '); /* dummy white space */

	    HText_appendCharacter(me->text, '\r');
	    HText_appendCharacter(me->text, '\r');
	}
	break;
	 

    case HTML_OL:
	OL_Counter[(List_Nesting_Level < 5 ? List_Nesting_Level+1 : 6)] = 1;
    case HTML_UL:
    case HTML_MENU:
    case HTML_DIR:
	List_Nesting_Level++;

	if(List_Nesting_Level <= 0) {
       	    change_paragraph_style(me, styles[element_number]);

	} else if(List_Nesting_Level >= 6) {
	    if(element_number==HTML_UL || element_number==HTML_OL) 
       	        change_paragraph_style(me, styles[HTML_OL6]);
	    else
		change_paragraph_style(me, styles[HTML_MENU6]);

	} else {
	    if(element_number==HTML_UL || element_number==HTML_OL)
                change_paragraph_style(me, 
		          styles[HTML_OL1 + List_Nesting_Level - 1]);
	    else 
                change_paragraph_style(me, 
		          styles[HTML_MENU1 + List_Nesting_Level - 1]);
	}
	break;
	
    case HTML_LI:
	UPDATE_STYLE;  /* update to the new style */
	HText_appendParagraph(me->text);
	if (me->sp[0].tag_number == HTML_OL) {
	    char number_string[20];
	    register int count=0;
	    sprintf(number_string, "%2d.",
		OL_Counter[(List_Nesting_Level<6 ? List_Nesting_Level : 6)]++);
	    /* hack, because there is no append string! */
	    for(;number_string[count]!='\0';count++)
		if(number_string[count] == ' ')
		    HText_appendCharacter(me->text, number_string[count]);
		else
	    	    HTML_put_character(me, number_string[count]);

	    /* use HTML_put_character so that any other spaces
	     * comming through will be collapsed
	     */
	    HTML_put_character(me, ' ');  /* the spacing charactor */	

	} else if(me->sp[0].tag_number == HTML_UL ||
			   me->sp[0].tag_number == HTML_MENU ) {
	    /* hack, because there is no append string! */
	    HText_appendCharacter(me->text, ' ');
	    HText_appendCharacter(me->text, ' ');
	    /* use HTML_put_character so that any other spaces
	     * comeing through will be collapsed
	     */
	    switch(List_Nesting_Level % 7) {
		case 0:
	    	    HTML_put_character(me, '*');
		    break;
		case 1:
	    	    HTML_put_character(me, '+');
		    break;
		case 2:
	    	    HTML_put_character(me, 'o');
		    break;
		case 3:
	    	    HTML_put_character(me, '#');
		    break;
		case 4:
	    	    HTML_put_character(me, '@');
		    break;
		case 5:
	    	    HTML_put_character(me, '-');
		    break;
		case 6:
		    HTML_put_character(me, '=');
		    break;

	    }
	    HTML_put_character(me, ' ');
	}
	me->in_word = NO;
	break;

    case HTML_LISTING:				/* Litteral text */
    case HTML_XMP:
    case HTML_PLAINTEXT:
    case HTML_PRE:
	change_paragraph_style(me, styles[element_number]);
	UPDATE_STYLE;
	if (me->comment_end)
	    HText_appendText(me->text, me->comment_end);
	break;

    case HTML_IMG:			/* Images -- put ALT text */
	UPDATE_STYLE;
	/*
	 *  If it's a clickable image for the current anchor,
	 *  Fake a 0,0 coordinate pair, which typically
	 *  returns the image's default. - FM
	 */
	if (B_inA && present[HTML_IMG_ISMAP] && B_CurrentA) {
		HTParentAnchor *dest;
		dest = HTAnchor_parent(
				HTAnchor_followMainLink((HTAnchor*)B_CurrentA)
				      );
		dest->isISMAPScript = TRUE;
#ifdef DT
		if (TRACE)
		    fprintf(stderr,
			    "HTML: Designating '%s' as an ISMAP script\n",
			    dest->address);
#endif
	}


	if (present[HTML_IMG_ALT] &&
	   !(clickable_images && *value[HTML_IMG_ALT] == '\0')) {

	    HTML_put_character(me, ' ');  /* space char may be ignored */
	    StrAllocCopy(alt_string, value[HTML_IMG_ALT] ?
				     value[HTML_IMG_ALT] : "");

	     /* Convert any HTML entities or decimal escaping. - FM
	     */
	    LYUnEscapeEntities(alt_string);

//	    free(alt_string);

	} else {   /* put [IMAGE] string */
		char *cp_image = " [INLINE]";

		if(present[HTML_IMG_ISMAP])	{
			cp_image = " [ISMAP]";
		}
		else if(B_inA == TRUE)	{
			cp_image = " [LINK]";
		}
		else if(B_inP == FALSE)	{
			cp_image = " [IMAGE]";
		}

		StrAllocCopy(alt_string, cp_image);
//		HTML_put_string(me, cp_image);
	}


	if(!clickable_images){
	    HTML_put_string(me, alt_string);
	    HTML_put_character(me, ' ');  /* space char may be ignored */
	}

	/*
	 *  Create links to the SRC for all images, if desired. - FM
	 */
	if (clickable_images && present[HTML_IMG_SRC] &&
	    value[HTML_IMG_SRC] && *value[HTML_IMG_SRC] != '\0') {

		if ((B_inBASE) &&
		    (temp = HTParse(value[HTML_IMG_SRC], base_href,
					    PARSE_ALL)) && *temp != '\0')
		    /*
		     *  Use reference related to the base.
		     */
		    StrAllocCopy(href, temp);
		else
		    /*
		     *  Use anchor's reference as is.
		     */
		    StrAllocCopy(href, value[HTML_IMG_SRC] ?
				       value[HTML_IMG_SRC] : "");

		/*
		 *  Don't simplify gopher url's.
		 */

//		if(!HT_Is_Gopher_URL)
		    HTSimplify(href);
//		else
//		    HT_Is_Gopher_URL= FALSE;

	    /*
	     *  If it's an ISMAP and/or USEMAP, or graphic for an
	     *  anchor, end that anchor and start one for the SRC. - FM
	     */
	    if (B_inA) {

		HTML_put_string(me, alt_string);

		if (B_inBoldA == TRUE && B_inBoldH == FALSE) {
		    HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		}
		B_inBoldA = FALSE;
		HText_endAnchor(me->text);
		HText_appendCharacter(me->text, '-');
		StrAllocCopy(alt_string, "[IMAGE]");

	    }

	    /*
	     *  Create the link to the SRC. - FM
	     */
	    B_CurrentA = HTAnchor_findChildAndLink(
			me->node_anchor,		/* Parent */
			NULL,				/* Tag */
			href ? href : NULL,		/* Addresss */
			NULL);				/* Type */

	    HText_beginAnchor(me->text, B_CurrentA);
	    if (B_inBoldH == FALSE)
		HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
	    HTML_put_string(me, alt_string);

	    if (!B_inA) {
		if (B_inBoldH == FALSE)
		    HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		HText_endAnchor(me->text);
		HTML_put_character(me, ' ');  /* space char may be ignored */
		me->in_word = NO;
	    } else {
		B_inBoldA = TRUE;
	    }
	}
	if (href) free(href);
	if (alt_string) free(alt_string);
	if (temp) free(temp);
	dest = NULL;
	dest_ismap = FALSE;
	break;


    case HTML_HTML:
	List_Nesting_Level = -1;
	if (base_href != NULL) {
	   free(base_href);
	   base_href = NULL;
	}
	B_inBASE = FALSE;
	break;
    
    case HTML_HEAD:
	List_Nesting_Level = -1;
	if (base_href != NULL) {
	   free(base_href);
	   base_href = NULL;
	}
        B_inBASE = FALSE;
	break;

    case HTML_BODY:
	List_Nesting_Level = -1;
	break;

    case HTML_B:			 /* Physical character highlighting */
    case HTML_I:
    case HTML_U:
    
    case HTML_EM:			/* Logical character highlighting */
    case HTML_STRONG:
        UPDATE_STYLE;
	/*	Ignore this if inside of an anchor or bold header.
	 *	Can't display both underline and bold at same time.
	 */
	if(B_inA == TRUE || B_inBoldH == TRUE)	{
		break;
	}
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	B_inUnderline = TRUE;
#ifdef DT
	if(TRACE)
	   fprintf(stderr,"Beginning underline\n");
#endif

    	break;
	
    case HTML_TT:			/* Physical character highlighting */
    case HTML_CODE:
    case HTML_SAMP:
    case HTML_KBD:
    case HTML_VAR:
    case HTML_DFN:
    case HTML_CITE:
	break; /* ignore */

    case HTML_ADDRESS:
	B_inADDRESS = TRUE;
    case HTML_H1:			/* paragraph styles */
    case HTML_H2:
    case HTML_H3:
    case HTML_H4:
    case HTML_H5:
    case HTML_H6:
    case HTML_H7:
    case HTML_CENTER:
    case HTML_BLOCKQUOTE:
	/*
	 *	Close the previous style if not done by HTML doc.
	 *	Added to get rid of core dumps in BAD HTML on the net.
	 *		GAB 07-07-94
	 *	But then again, these are actually allowed to nest.  I guess
	 *	I have to depend on the HTML writers correct style.
	 *		GAB 07-12-94
	if(i_prior_style != -1)	{
		HTML_end_element(me, i_prior_style);
	}
	i_prior_style = element_number;
	 */

	change_paragraph_style(me, styles[element_number]);
	UPDATE_STYLE;
	if (bold_headers == TRUE && (styles[element_number]->font&HT_BOLD)) {
	    if (B_inA == FALSE && B_inBoldH == FALSE) {
		HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
	    }
	    B_inBoldH = TRUE;
	}
	HTML_Last_Char = '\r';
	break;

    } /* end switch */

    if (HTML_dtd.tags[element_number].contents!= SGML_EMPTY) {
	if (me->sp == me->stack) {
            fprintf(stderr, 
		"HTML: ****** Maximum nesting of %d tags exceeded!\n",
            	MAX_NESTING);
            return;
        }

    	--(me->sp);
	me->sp[0].style = me->new_style;	/* Stack new style */
	me->sp[0].tag_number = element_number;

#ifdef DT
	if(TRACE)
	    fprintf(stderr,"HTML:begin_element: adding style to stack - %s\n",
							me->new_style->name);
#endif

    }	
}


/*		End Element
**		-----------
**
*/
/*	When we end an element, the style must be returned to that
**	in effect before that element.  Note that anchors (etc?)
**	don't have an associated style, so that we must scan down the
**	stack for an element with a defined style. (In fact, the styles
**	should be linked to the whole stack not just the top one.)
**	TBL 921119
**
**	We don't turn on "CAREFUL" check because the parser produces
**	(internal code errors apart) good nesting. The parser checks
**	incoming code errors, not this module.
*/
PRIVATE void HTML_end_element ARGS2(HTStructured *, me, int , element_number)
{
#ifdef CAREFUL			/* parser assumed to produce good nesting */
    if (element_number != me->sp[0].tag_number) {
        fprintf(stderr, "HTMLText: end of element %s when expecting end of %s\n",
		HTML_dtd.tags[element_number].name,
		HTML_dtd.tags[me->sp->tag_number].name);
		/* panic */
    }
#endif
    
    if(me->sp < me->stack + MAX_NESTING+1) {
        me->sp++;				/* Pop state off stack */
#ifdef DT
        if(TRACE)
	    fprintf(stderr,"HTML:end_element: Popped style off stack - %s\n",me->sp->style->name);
    } else {
	if(TRACE)
	    fprintf(stderr,"Stack underflow error!  Tried to pop off more styles than exist in stack\n");
#endif
    }
    
    /* Check for unclosed TEXTAREA */
#ifdef DT
    if (B_inTEXTAREA && element_number != HTML_TEXTAREA)
	if (TRACE) {
	    fprintf(stderr, "HTML: Missing TEXTAREA end tag\n");
	} else
	{
	    _statusline("** Bad HTML!!  Use -trace to diagnose. **");
	    sleep(sleep_two);
	}
#endif

    switch(element_number) {


#ifdef NOT_DEFINED /* BASE will be a container in HTML+ */
    case HTML_BASE:
	if (base_href != NULL) {
	    free(base_href);
	    base_href = NULL;
	}
	B_inBASE = FALSE;
	break;
#endif /* NOT_DEFINED */

    case HTML_A:
	/*	Set to know that we are no longer in an anchor.
	 */
	B_inA = FALSE;

	UPDATE_STYLE;
	if (B_inBoldH == FALSE)
	    HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
	HText_endAnchor(me->text);
/* wsb */
        if (B_inUnderline == TRUE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	break;

    case HTML_FORM:
	/* Make sure we had a form start tag. */
	if (!B_inFORM) {
#ifdef DT
	    if (TRACE) {
		fprintf(stderr, "HTML: Unmatched FORM end tag\n");
	    } else
	    {
		_statusline("** Bad HTML!!  Use -trace to diagnose. **");
		sleep(sleep_two);
	    }
#endif
	    break;
	}

	/* Set to know that we are no longer in an form.
	 */
	B_inFORM = FALSE;

	UPDATE_STYLE;
	HText_endForm();
	HText_appendCharacter(me->text,'\r'); 
	break;

    	case HTML_MH:	{
		UPDATE_STYLE;

		/*
		 *	Do nothing for now.
		 */
		break;
	}

    case HTML_SELECT:
	{
	    char *ptr;

	    /* Make sure we had a select start tag. */
	    if (!B_inSELECT) {
#ifdef DT
	        if (TRACE) {
		    fprintf(stderr, "HTML: Unmatched SELECT end tag\n");
		} else
		{
		    _statusline("** Bad HTML!!  Use -trace to diagnose. **");
		    sleep(sleep_two);
		}
#endif
		break;
	    }

	    /* Set to know that we are no longer in a select tag.
	     */
	    B_inSELECT = FALSE;

	    /* finish the data off */
       	    HTChunkTerminate(&me->option);
	    /* finish the previous option @@@@@ */
	    ptr = HText_setLastOptionValue(me->text,
	    				   me->option.data, LastOptionValue,
					   LAST_ORDER, LastOptionChecked);
	    if (LastOptionValue) {
	        free(LastOptionValue);
		LastOptionValue = NULL;
	    }

	    LastOptionChecked = FALSE;

	    if(HTCurSelectGroupType != F_CHECKBOX_TYPE) {
	        /* output to screen but use non breaking spaces for output */
	        for(; *ptr != '\0'; ptr++)
		    if(*ptr == ' ')
	    	        HText_appendCharacter(me->text,HT_NON_BREAK_SPACE); 
		    else
	    	        HText_appendCharacter(me->text,*ptr); 
	         /* add end option character */
	        HText_appendCharacter(me->text,']'); 
		HText_appendCharacter(me->text,' ');
	    }
    	   HTChunkClear(&me->option);
	}
	break;

    case HTML_TEXTAREA:
        {
            InputFieldData I;
            int chars;
 	    char *cp=0;
	    int i;

	    /* Make sure we had a textarea start tag. */
	    if (!B_inTEXTAREA) {
#ifdef DT
	        if (TRACE) {
		    fprintf(stderr, "HTML: Unmatched TEXTAREA end tag\n");
		} else
		{
		    _statusline("** Bad HTML!!  Use -trace to diagnose. **");
		    sleep(sleep_two);
		}
#endif
		break;
	    }

	    /* Set to know that we are no longer in a textarea tag.
	     */
	    B_inTEXTAREA = FALSE;

            /* init */
            I.name=NULL; I.type=NULL; I.value=NULL;
            I.checked=NO; I.size=NULL; I.maxlength=NULL;

            UPDATE_STYLE;
            /* before any input field add a space if necessary */
	    HTML_put_character(me,' ');
	    /* add a return */
	    HText_appendCharacter(me->text,'\r');

	    /* finish the data off */
            HTChunkTerminate(&me->textarea);

	    I.type = "textarea";
	    I.value = cp;  /* may be null */
	    I.size = textarea_cols;
	    I.name = textarea_name;

	    cp = strtok(me->textarea.data, "\n");
	    for(i=0; i < textarea_rows; i++)
	      {
		I.value = cp;

                chars = HText_beginInput(me->text, &I);
	        for(; chars>0; chars--)
	    	    HTML_put_character(me, '_');
		HText_appendCharacter(me->text,'\r');
	
		cp = strtok(NULL, "\n");
	      }

	    /* check for more data lines than the rows attribute 
   	     */
	    while(cp)
	      {
		I.value = cp;

                chars = HText_beginInput(me->text, &I);
                for(chars=atoi(textarea_cols); chars>0; chars--)
                    HTML_put_character(me, '_');
                HText_appendCharacter(me->text,'\r');
        
                cp = strtok(NULL, "\n");
              }

	    break;
	}

    case HTML_TITLE:
        HTChunkTerminate(&me->title);
    	HTAnchor_setTitle(me->node_anchor, me->title.data);
        HTChunkClear(&me->title);
	break;
	
    case HTML_LISTING:				/* Litteral text */
    case HTML_XMP:
    case HTML_PLAINTEXT:
    case HTML_PRE:
    	if (me->comment_start)
    	    HText_appendText(me->text, me->comment_start);
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	HTML_Last_Char = '\r';
	break;

    case HTML_OL:
    case HTML_DL:
    case HTML_UL:
    case HTML_MENU:
    case HTML_DIR:
	List_Nesting_Level--;
#ifdef DT
	if(TRACE)
	    fprintf(stderr,"Reducing List Nesting Level to %d\n",
						    List_Nesting_Level);
#endif

	change_paragraph_style(me, me->sp->style);

	break;

    case HTML_HTML:
	List_Nesting_Level = -1;
	if (base_href != NULL) {
	   free(base_href);
	   base_href = NULL;
	}
	B_inBASE = FALSE;
#ifdef DT
	if (B_inA || B_inFORM || B_inSELECT || B_inTEXTAREA)
	    if (TRACE)
	        fprintf(stderr,
			"HTML: Something not closed before HTML close-tag\n");
	    else
		_statusline("** Bad HTML!!  Use -trace to diagnose. **");
#endif
	break;

    case HTML_HEAD:
        List_Nesting_Level = -1;
	break;

    case HTML_BODY:
	List_Nesting_Level = -1;
	if (base_href != NULL) {
	   free(base_href);
	   base_href = NULL;
	}
        B_inBASE = FALSE;
#ifdef DT
	if (B_inA || B_inFORM || B_inSELECT || B_inTEXTAREA)
	    if (TRACE)
	        fprintf(stderr,
			"HTML: Something not closed before BODY close-tag\n");
	    else
		_statusline("** Bad HTML!!  Use -trace to diagnose. **");
#endif

	break;

    case HTML_B:
    case HTML_I:
    case HTML_U:
    
    case HTML_EM:			/* Logical character highlighting */
    case HTML_STRONG:
	/*	If in an anchor or bold header, ignore these.
	 *	Can't display both underline and bold at the same time.
	 */
	if(B_inA == TRUE || B_inBoldH == TRUE)	{
		break;
	}
	if (B_inUnderline == TRUE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	B_inUnderline = FALSE;
#ifdef DT
	if(TRACE)
	   fprintf(stderr,"Ending underline\n");
#endif

    	break;

    case HTML_TT:			/* Physical character highlighting */
    case HTML_CODE:
    case HTML_SAMP:
    case HTML_KBD:
    case HTML_VAR:
    case HTML_DFN:
    case HTML_CITE:
	break;

    case HTML_ADDRESS:
	B_inADDRESS = FALSE;
    case HTML_H1:                       /* paragraph styles */
    case HTML_H2:
    case HTML_H3:
    case HTML_H4:
    case HTML_H5:
    case HTML_H6:
    case HTML_H7:
    case HTML_CENTER:
    case HTML_BLOCKQUOTE:
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	if (styles[element_number]->font & HT_BOLD) {
	    if (B_inA == FALSE && B_inBoldH == TRUE) {
		HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
	    }
	    B_inBoldH = FALSE;
	}
	HTML_Last_Char = '\r';
	break;
		/*
		 *	Set flag to know that style has ended.
		 *	Fall through.
		i_prior_style = -1;
		 */
    default:
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
#if FIXME
	if (styles[element_number]->font & HT_BOLD) {
	    if (B_inA == FALSE && B_inBoldH == TRUE) {
		HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
	    }
	    B_inBoldH = FALSE;
	}
#endif
	break;

    } /* switch */
}


/*		Expanding entities
**		------------------
*/
/*	(In fact, they all shrink!)
*/

PRIVATE void HTML_put_entity ARGS2(HTStructured *, me, int, entity_number)
{
    HTML_put_string(me, p_entity_values[entity_number]);
}



/*	Free an HTML object
**	-------------------
**
** If the document is empty, the text object will not yet exist.
   So we could in fact abandon creating the document and return
   an error code.  In fact an empty document is an important type
   of document, so we don't.
**
**	If non-interactive, everything is freed off.   No: crashes -listrefs
**	Otherwise, the interactive object is left.	
*/
PUBLIC void HTML_free ARGS1(HTStructured *, me)
{
    UPDATE_STYLE;		/* Creates empty document here! */
    if (me->comment_end)
		HTML_put_string(me,me->comment_end);
    HText_endAppend(me->text);

    if (me->target) {
        (*me->targetClass._free)(me->target);
    }
    if (me)
        free(me);
}


PRIVATE void HTML_abort ARGS2(HTStructured *, me, HTError, e)

{			
    List_Nesting_Level = -1;

    if(me->text)
	HText_endAppend(me->text);

    if (me->target) {
        (*me->targetClass._abort)(me->target, e);
    }
    if (me)
        free(me);
}


/*	Get Styles from style sheet
**	---------------------------
*/
PRIVATE void get_styles NOARGS
{
    got_styles = YES;

    default_style =		HTStyleNamed(styleSheet, "Normal");

    styles[HTML_H1] =		HTStyleNamed(styleSheet, "Heading1");
    styles[HTML_H2] =		HTStyleNamed(styleSheet, "Heading2");
    styles[HTML_H3] =		HTStyleNamed(styleSheet, "Heading3");
    styles[HTML_H4] =		HTStyleNamed(styleSheet, "Heading4");
    styles[HTML_H5] =		HTStyleNamed(styleSheet, "Heading5");
    styles[HTML_H6] =		HTStyleNamed(styleSheet, "Heading6");
    styles[HTML_H7] =		HTStyleNamed(styleSheet, "Heading7");

    styles[HTML_CENTER] =       HTStyleNamed(styleSheet, "Center");

    styles[HTML_DL] =		HTStyleNamed(styleSheet, "Glossary");
	/* nested list styles */
    styles[HTML_DL1] =		HTStyleNamed(styleSheet, "Glossary1");
    styles[HTML_DL2] =		HTStyleNamed(styleSheet, "Glossary2");
    styles[HTML_DL3] =		HTStyleNamed(styleSheet, "Glossary3");
    styles[HTML_DL4] =		HTStyleNamed(styleSheet, "Glossary4");
    styles[HTML_DL5] =		HTStyleNamed(styleSheet, "Glossary5");
    styles[HTML_DL6] =		HTStyleNamed(styleSheet, "Glossary6");

    styles[HTML_UL] =
    styles[HTML_OL] =		HTStyleNamed(styleSheet, "List");
	/* nested list styles */
    styles[HTML_OL1] =		HTStyleNamed(styleSheet, "List1");
    styles[HTML_OL2] =		HTStyleNamed(styleSheet, "List2");
    styles[HTML_OL3] =		HTStyleNamed(styleSheet, "List3");
    styles[HTML_OL4] =		HTStyleNamed(styleSheet, "List4");
    styles[HTML_OL5] =		HTStyleNamed(styleSheet, "List5");
    styles[HTML_OL6] =		HTStyleNamed(styleSheet, "List6");

    styles[HTML_MENU] =
    styles[HTML_DIR] =		HTStyleNamed(styleSheet, "Menu");    
	/* nested list styles */
    styles[HTML_MENU1] =	HTStyleNamed(styleSheet, "Menu1");    
    styles[HTML_MENU2] =	HTStyleNamed(styleSheet, "Menu2");    
    styles[HTML_MENU3] =	HTStyleNamed(styleSheet, "Menu3");    
    styles[HTML_MENU4] =	HTStyleNamed(styleSheet, "Menu4");    
    styles[HTML_MENU5] =	HTStyleNamed(styleSheet, "Menu5");    
    styles[HTML_MENU6] =	HTStyleNamed(styleSheet, "Menu6");    

    styles[HTML_DLC] =		HTStyleNamed(styleSheet, "GlossaryCompact");
	/* nested list styles */
    styles[HTML_DLC1] =		HTStyleNamed(styleSheet, "GlossaryCompact1");
    styles[HTML_DLC2] =		HTStyleNamed(styleSheet, "GlossaryCompact2");
    styles[HTML_DLC3] =		HTStyleNamed(styleSheet, "GlossaryCompact3");
    styles[HTML_DLC4] =		HTStyleNamed(styleSheet, "GlossaryCompact4");
    styles[HTML_DLC5] =		HTStyleNamed(styleSheet, "GlossaryCompact5");
    styles[HTML_DLC6] =		HTStyleNamed(styleSheet, "GlossaryCompact6");

    styles[HTML_ADDRESS] =	HTStyleNamed(styleSheet, "Address");
    styles[HTML_BLOCKQUOTE] =	HTStyleNamed(styleSheet, "Blockquote");
    styles[HTML_PLAINTEXT] =
    styles[HTML_XMP] =		HTStyleNamed(styleSheet, "Example");
    styles[HTML_PRE] =		HTStyleNamed(styleSheet, "Preformatted");
    styles[HTML_LISTING] =	HTStyleNamed(styleSheet, "Listing");
}
/*				P U B L I C
*/

/*	Structured Object Class
**	-----------------------
*/
PUBLIC CONST HTStructuredClass HTMLPresentation = /* As opposed to print etc */
{		
	"text/html",
	HTML_free,
	HTML_abort,
	HTML_put_character, 	HTML_put_string,  HTML_write,
	HTML_start_element, 	HTML_end_element,
	HTML_put_entity
}; 


/*		New Structured Text object
**		--------------------------
**
**	The strutcured stream can generate either presentation,
**	or plain text, or HTML.
*/
PUBLIC HTStructured* HTML_new ARGS3(
	HTParentAnchor *, 	anchor,
	HTFormat,		format_out,
	HTStream*,		stream)
{

    HTStructured * me;


	/*  Reset to know that we aren't in an anchor, bold header or paragraph.
	 */
	B_inA = FALSE;
	B_inBoldH = FALSE;
	B_inUnderline = FALSE;
	B_inP = FALSE;
 
    if (format_out != WWW_PLAINTEXT && format_out != WWW_PRESENT) {
        HTStream * intermediate = HTStreamStack(WWW_HTML, format_out,
		stream, anchor);
	if (intermediate) return HTMLGenerator(intermediate);
        fprintf(stderr, "** Internal error: can't parse HTML to %s\n",
       		HTAtom_name(format_out));
      
	exit (-99);
    }

    me = (HTStructured*) malloc(sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "HTML_new");

    if (!got_styles) get_styles();

    me->isa = &HTMLPresentation;
    me->node_anchor =  anchor;
    me->title.size = 0;
    me->title.growby = 128;
    me->title.allocated = 0;
    me->title.data = 0;
    me->option.size = 0;
    me->option.growby = 128;
    me->option.allocated = 0;
    me->option.data = 0;
    me->textarea.size = 0;
    me->textarea.growby = 128;
    me->textarea.allocated = 0;
    me->textarea.data = 0;
    me->text = 0;
    me->style_change = YES; /* Force check leading to text creation */
    me->new_style = default_style;
    me->old_style = 0;
    me->sp = me->stack + MAX_NESTING - 1;
    me->sp->tag_number = -1;				/* INVALID */
    me->sp->style = default_style;			/* INVALID */
    
    me->comment_start = NULL;
    me->comment_end = NULL;
    me->target = stream;
    if (stream) me->targetClass = *stream->isa;	/* Copy pointers */
    
    return (HTStructured*) me;
}


/*	HTConverter for HTML to plain text
**	----------------------------------
**
**	This will convert from HTML to presentation or plain text.
*/
PUBLIC HTStream* HTMLToPlain ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink)
{
    return SGML_new(&HTML_dtd, HTML_new(anchor, pres->rep_out, sink));
}


/*	HTConverter for HTML to C code
**	------------------------------
**
**	C copde is like plain text but all non-preformatted code
**	is commented out.
**	This will convert from HTML to presentation or plain text.
*/
PUBLIC HTStream* HTMLToC ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink)
{
    
    HTStructured * html;
    
    (*sink->isa->put_string)(sink, "/* ");	/* Before even title */
    html = HTML_new(anchor, WWW_PLAINTEXT, sink);
    html->comment_start = "/* ";
    html->comment_end = " */\n";	/* Must start in col 1 for cpp */
/*    HTML_put_string(html,html->comment_start); */
    return SGML_new(&HTML_dtd, html);
}


/*	Presenter for HTML
**	------------------
**
**	This will convert from HTML to presentation or plain text.
**
**	Override this if you have a windows version
*/
#ifndef GUI
PUBLIC HTStream* HTMLPresent ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink)
{
    return SGML_new(&HTML_dtd, HTML_new(anchor, WWW_PRESENT, NULL));
}
#endif


/*	Record error message as a hypertext object
**	------------------------------------------
**
**	The error message should be marked as an error so that
**	it can be reloaded later.
**	This implementation just throws up an error message
**	and leaves the document unloaded.
**	A smarter implementation would load an error document,
**	marking at such so that it is retried on reload.
**
** On entry,
**	sink 	is a stream to the output device if any
**	number	is the HTTP error number
**	message	is the human readable message.
**
** On exit,
**	returns	a negative number to indicate lack of success in the load.
*/

PUBLIC int HTLoadError ARGS3(
	HTStream *, 	sink,
	int,		number,
	CONST char *,	message)
{
    HTAlert(message);		/* @@@@@@@@@@@@@@@@@@@ */
    return -number;
} 

/*
**  This function converts HTML entities within a string to
**  to their translations in the active LYCharSets array.
**  It also converts decimal escaped characters to their
**  numeric values.  The string is converted in place, on
**  the assumption that the conversion strings are not
**  longer than the entity strings, such that the overall
**  string will never grow.  This assumption always will be
**  valid for decimal escaped characters.  Make sure it stays
**  true for the LYCharSets arrays. - FM
*/

PUBLIC char * LYUnEscapeEntities ARGS1(
	char *,	str)
{
    char * p = str;
    char * q = str;
    char * cp;

    if (str == NULL)
       return str;

    while(*p) {
        if (*p == '&') {
	    p++;
	    if (*p == '#' && strlen(p) > 4 && *(p+4) == ';' &&
		    isalnum(*(p+1)) && isalnum(*(p+2)) && isalnum(*(p+3))) {
		    /*
		     * It's decimal escaped.
		     */
		    p++;
		    *(p+3) = '\0';
		    *q++ = (unsigned char)atoi(p);
		    p += 4;

		} else if ((cp=strchr(p, ';')) != NULL) {
		    /*
		     * It's an HTML entity.
		     */
		    int len, high, low, diff, i;
		    len =  (cp - p);
		    for(low=0, high = HTML_dtd.number_of_entities;
			high > low ;
		        diff < 0 ? (low = i+1) : (high = i)) {
			/* Binary search */
		        i = (low + (high-low)/2);
			diff = strncmp(HTML_dtd.entity_names[i], p, len);
			if (diff==0) {
			    /*
			     * Found the entity.  Assume that the length
			     * of the value does not exceed the length of
			     * the raw entity, so that the overall string
			     * does not need to grow.  Make sure this stays
			     * true in the LYCharSets arrays. - FM
			     */
			    int j;
			    for (j = 0; p_entity_values[i][j]; j++)
			        *q++ = (unsigned char)p_entity_values[i][j];
			    p = (cp+1);
			    break;
			}
		    }
		    if (diff != 0) {
		        /*
			 * Entity not found.
			 */
			*q++ = '&';
		    }

		} else {
		    /*
		     * It's a raw ampersand.
		     */
		    *q++ = '&';
		}
	} else
	    *q++ = *p++; 
    }
    
    *q++ = '\0';
    return str;
}
