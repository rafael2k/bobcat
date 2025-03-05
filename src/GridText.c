/*		Character grid hypertext object
**		===============================
*/

#include "heap.h"
#include "HTUtils.h"
#include "tcp.h"

#include "LYCurses.h" /* lynx defined curses */

#include <assert.h>
#include <ctype.h>
#include "HTString.h"
#include "GridText.h"
#include "HTFont.h"
#include "HTAccess.h"
#include "HTAlert.h"
#include "HTParse.h"
#include "HTTP.h"

/* lynx specific defines */
#include "LYUtils.h"
#include "LYMail.h"
#include "LYString.h"
#include "LYStruct.h"
#include "LYGlobalDefs.h"
#include "LYGetFil.h"
#include "LYSignal.h"
#include "LYexit.h"
#include "LYLeaks.h"
#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
#include <syslog.h>
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

#include <unistd.h>

struct _HTStream {                      /* only know it as object */
    CONST HTStreamClass *       isa;
    /* ... */
};

#define TITLE_LINES  1


/*	From default style sheet:
*/
extern HTStyleSheet * styleSheet;	/* Default or overridden */

extern int display_lines; /* number of lines in display */
extern char HTML_Last_Char;

/*	Exports
*/ 
PUBLIC HText * HTMainText = 0;		/* Equivalent of main window */
PUBLIC HTParentAnchor * HTMainAnchor = 0;	/* Anchor for HTMainText */

PUBLIC char * HTAppName = LYNX_NAME;      /* Application name */
PUBLIC char * HTAppVersion = LYNX_VERSION;        /* Application version */

PUBLIC int HTFormNumber = 0;
PUBLIC char * HTCurSelectGroup=NULL;  /* form select group name */
PUBLIC int HTCurSelectGroupType = F_RADIO_TYPE; /* group type */
PUBLIC char * HTCurSelectGroupSize = NULL; /* length of select */

PUBLIC char * checked_radio = "(*)";
PUBLIC char * unchecked_radio = "( )";
PUBLIC char * checked_box = "[X]";
PUBLIC char * unchecked_box = "[ ]";

PUBLIC BOOLEAN underline_on = OFF;
PUBLIC BOOLEAN bold_on      = OFF;

typedef struct _line {
	struct _line	*next;
	struct _line	*prev;
	short unsigned	offset;		/* Implicit initial spaces */
	short unsigned	size;		/* Number of characters */
	BOOL	split_after;		/* Can we split after? */
	BOOL	bullet;			/* Do we bullet? */
	char	data[1];		/* Space for terminator at least! */
} HTLine;

#define LINE_SIZE(l) (sizeof(HTLine)+(l))	/* allow for terminator */

typedef struct _TextAnchor {
	struct _TextAnchor *	next;
	int			number;		/* For user interface */
	int			start;		/* Characters */
        int			line_pos;       /* position in text */
	int			extent;		/* Characters */
	int			line_num;       /* place in document */
	char *		        hightext;       /* the link text */
	char *		        hightext2;       /* a second line*/
        int 		    	hightext2offset; /* offset from left */
	int			link_type;	/* normal or form ? */
	FormInfo *		input_field;	/* info for form links */
	BOOL			show_anchor;    /* show the anchor? */
	HTChildAnchor *		anchor;
} TextAnchor;


/*	Notes on struct _Htext:
**	next_line is valid iff state is false.
**	top_of_screen line means the line at the top of the screen
**			or just under the title if there is one.
*/
struct _HText {
	HTParentAnchor *	node_anchor;
	HTLine * 		last_line;
	int			lines;		/* Number of them */
	int			chars;		/* Number of them */
	TextAnchor *		first_anchor;	/* Singly linked list */
	TextAnchor *		last_anchor;
	int			last_anchor_number;	/* user number */
	BOOL			source;		/* is the text source? */
/* For Internal use: */
	HTStyle *		style;			/* Current style */
	int			display_on_the_fly;	/* Lines left */
	int			top_of_screen;		/* Line number */
	HTLine *		top_of_screen_line;	/* Top */
	HTLine *		next_line;		/* Bottom + 1 */
	int			permissible_split;	/* in last line */
	BOOL			in_line_1;		/* of paragraph */
	BOOL			stale;			/* Must refresh */

	int                     halted;                 /* emergency halt */
	HTStream*               target;                 /* Output stream */
	HTStreamClass           targetClass;            /* Output routines */
};

/*	Boring static variable used for moving cursor across
*/

#define UNDERSCORES(n) (&underscore_string[(MAX_LINE-1) - (n)])

/*
 *	Memory leak fixed.
 *	05-29-94 Lynx 2-3-1 Garrett Arch Blythe
 *	Changed to arrays.
 */
PRIVATE char underscore_string[MAX_LINE + 1];
PUBLIC char star_string[MAX_LINE + 1];

PRIVATE int ctrl_chars_on_this_line=0;  /* num of ctrl chars in current line */

PRIVATE HTStyle default_style =
	{ 0,  "(Unstyled)", "",
	(HTFont)0, 1.0, HT_BLACK,		0, 0,
	0, 0, 0, HT_LEFT,		1, 0,	0, 
	NO, NO, 0, 0,			0 };	



PRIVATE HTList * loaded_texts = NULL;	  /* A list of all those in memory */
PRIVATE void free_all_texts NOARGS;

 /*
  *  text->halted = 1: have set fake 'Z' and output a message
  *                 2: next time when HText_appendCharacter is called
  *                  it will append *** MEMORY EXHAUSTED ***, then set
  *                  to 3.
  *               3: normal text output will be suppressed (but not anchors,
  *                  form fields etc.)
  */
 PRIVATE void HText_halt NOARGS
 {
     if (HTFormNumber > 0)
       HText_DisableCurrentForm();
     if (!HTMainText)
       return;
     if (HTMainText->halted < 2)
       HTMainText->halted = 2;
 }
 
 #define MIN_NEEDED_MEM 5000
 
 /*
  *  Check whether factor*min(bytes,MIN_NEEDED_MEM) is available,
  *  or bytes if factor is 0.
  *  MIN_NEEDED_MEM and factor together represent a security margin,
  *  to take account of all the memory allocations where we don't check
  *  and of buffers which may be emptied before HTCheckForInterupt()
  *  is (maybe) called and other things happening, with some chance of
  *  success.
  *  This just tries to malloc() the to-be-checked-for amount of memory,
  *  which might make the situation worse depending how allocation works.
  *  There should be a better way... - kw
  */
 PRIVATE BOOL mem_is_avail ARGS2(
     size_t,   factor,
     size_t,   bytes)
 {
     void *p;
     if (bytes < MIN_NEEDED_MEM && factor > 0)
       bytes = MIN_NEEDED_MEM;
     if (factor == 0)
       factor = 1;
     p = malloc(factor * bytes);
     if (p) {
       free(p);
       return YES;
     } else {
       return NO;
     }
 }
 
 /*
  *  Replacement for calloc which checks for "enough" free memory
  *  (with some security margins) and tries various recovery actions
  *  if deemed necessary. - kw
  */
 PRIVATE void * LY_check_calloc ARGS2(
     size_t,   nmemb,
     size_t,   size)
 {
     int i, n;
     if (mem_is_avail(4, nmemb * size)) {
       return (calloc(nmemb, size));
     }
     n = HTList_count(loaded_texts);
     for (i = n - 1; i > 0; i--) {
       HText * t = HTList_objectAt(loaded_texts, i);
       if (t == HTMainText)
           t = NULL;           /* shouldn't happen */
       HTList_removeObjectAt(loaded_texts, i);
	 HText_free(t);
       if (mem_is_avail(4, nmemb * size)) {
           return (calloc(nmemb, size));
       }
     }
     LYFakeZap(YES);
     if (!HTMainText || HTMainText->halted <= 1) {
       if (!mem_is_avail(2, nmemb * size)) {
           HText_halt();
           if (mem_is_avail(0, 700)) {
               HTAlert("Memory exhausted, display interrupted!");
           }
       } else {
           if ((!HTMainText || HTMainText->halted == 0) &&
               mem_is_avail(0, 700)) {
               HTAlert("Memory exhausted, will interrupt transfer!");
               if (HTMainText)
                   HTMainText->halted = 1;
           }
       }
     }
     return (calloc(nmemb, size));
 }
 
 #define LY_CALLOC LY_check_calloc








/*			Creation Method
**			---------------
*/
PUBLIC HText *	HText_new ARGS1(HTParentAnchor *,anchor)
{
#if defined(VMS) && defined(VAXC) && !defined(__DECC)
#include <lib$routines.h>
    int status, VMType=3, VMTotal;
#endif /* VMS && VAXC && !__DECC */
    HTLine * line = NULL;
    HText * self = (HText *) calloc(sizeof(*self),1);
    if (!self) return self;
    
    if (!loaded_texts)	{
	loaded_texts = HTList_new();
/*
 *  All allocated memory is freed when an image exits on VMS,
 *  and more efficiently than via C RTL free's, so on VMS
 *  only use this when debugging with LY_FIND_LEAKS defined.
 */
#if !defined(VMS) || defined(LY_FIND_LEAKS)
	atexit(free_all_texts);
#endif /* !VMS || LY_FIND_LEAKS */
    }

    /*
     * Links between anchors & documents are a 1-1 relationship. If
     * an anchor is already linked to a document we didn't call
     * HTuncache_current_document(), e.g., for the showinfo, options,
     * dowload, print, etc., temporary file URLs, so we'll check now
     * and free it before reloading. - Dick Wesseling (ftu@fi.ruu.nl)
     */
    if (anchor->document) {
       HTList_removeObject(loaded_texts, anchor->document);
       ((HText *)anchor->document)->node_anchor=0;
       HText_free((HText *)anchor->document);
       anchor->document = 0;
    }

    HTList_addObject(loaded_texts, self);
#if defined(VMS) && defined(VAXC) && !defined(__DECC)
    while (HTList_count(loaded_texts) > HTCacheSize &&
    	   VMTotal > HTVirtualMemorySize) {
#else
    if (HTList_count(loaded_texts) > HTCacheSize) {
#endif /* VMS && VAXC && !__DECC */

	HText_free((HText *)HTList_removeFirstObject(loaded_texts));
#if defined(VMS) && defined (VAXC) && !defined(__DECC)
        status = lib$stat_vm(&VMType, &VMTotal);

#endif /* VMS && VAXC && !__DECC */
    }
    
    line = self->last_line = (HTLine *)calloc(sizeof(char),LINE_SIZE(MAX_LINE));
    if (line == NULL) outofmem(__FILE__, "HText_New");
    line->next = line->prev = line;
    line->offset = line->size = 0;
    self->lines = self->chars = 0;
    self->first_anchor = self->last_anchor = 0;
    self->style = &default_style;
    self->top_of_screen = 0;
    self->node_anchor = anchor;
    self->last_anchor_number = 0;	/* Numbering of them for references */
    self->stale = YES;

    if(HTOutputFormat == WWW_SOURCE)
        self->source = YES;
    else
        self->source = NO;
    
    HTAnchor_setDocument(anchor, (HyperDoc *)self);

    HTFormNumber = 0;  /* no forms started yet */
    HTMainText = self;
    HTMainAnchor = anchor;
    self->display_on_the_fly = 0;
   
    /*
     *	Memory leak fixed.
     *  05-29-94 Lynx 2-3-1 Garrett Arch Blythe
     *	Check to see if our underline and star_string need initialization
     *		if the underline is not filled with dots.
     */ 
    if (underscore_string[0] != '.') { /* Make a line */

//        char *p;
//        for (p=underscore_string; p<underscore_string+(MAX_LINE-1); p++)
//            *p = '.';           /* Used for printfs later */
//        underscore_string[(MAX_LINE-1)] = '\0';
//        for (p=star_string; p<star_string+(LINESIZE-1); p++)
//            *p = '_';           /* Used for printfs later */
//        star_string[(LINESIZE-1)] = '\0';
    /*
     *  Create and array of dots for the UNDERSCORES macro. - FM
     */
    memset(underscore_string, '.', (MAX_LINE-1));
    underscore_string[(MAX_LINE-1)] = '\0';
    underscore_string[MAX_LINE] = '\0';
    /*
     *  Create and array of underscores for the STARS macro. - FM
     */
    memset(star_string, '_', (MAX_LINE-1));
    star_string[(MAX_LINE-1)] = '\0';
    star_string[MAX_LINE] = '\0';

    }

    underline_on = FALSE; /* reset */
    bold_on = FALSE;
    
    return self;
}

/*                      Creation Method 2
**                      ---------------
**
**      Stream is assumed open and left open.
*/
PUBLIC HText *  HText_new2 ARGS2(
                HTParentAnchor *,       anchor,
                HTStream*,              stream)

{
    HText * this = HText_new(anchor);

    if (stream) {
        this->target = stream;
        this->targetClass = *stream->isa;       /* copy action procedures */
    }
    return this;
}

/*	Free Entire Text
**	----------------
*/
PUBLIC void 	HText_free ARGS1(HText *,self)
{
    if (!self)
        return;

    HTAnchor_setDocument(self->node_anchor, (HyperDoc *)0);
    
    while(YES) {	/* Free off line array */
        HTLine * l = self->last_line;
	if(l) {
	    l->next->prev = l->prev;
	    l->prev->next = l->next;	/* Unlink l */
	    self->last_line = l->prev;
	    free(l);
	}
	if (l == self->last_line)	/* empty */
	    break;
    };
    
    while(self->first_anchor) {		/* Free off anchor array */
        TextAnchor * l = self->first_anchor;
	self->first_anchor = l->next;

	    /* free form fields */
	if(l->link_type == INPUT_ANCHOR && l->input_field) {

		/* free off option lists */
	    if(l->input_field->type == F_OPTION_LIST_TYPE) {
		OptionType *optptr=l->input_field->select_list;
		OptionType *tmp;
		while(optptr) {
		    tmp = optptr;
		    if(tmp->name)
		        free(tmp->name);
		    optptr = optptr->next;
		    free(tmp);
		}
	    }
	    else 
	      {
		/* don't free the value field on option
		 * lists since it points to a option value
		 * same for orig value
		 */
		if(l->input_field->value)
                    free(l->input_field->value);
    	        if(l->input_field->orig_value)
                    free(l->input_field->orig_value);
    	        if(l->input_field->cp_submit_value)
                    free(l->input_field->cp_submit_value);
    	        if(l->input_field->orig_submit_value)
                    free(l->input_field->orig_submit_value);
	      }
		
    	    if(l->input_field->submit_action)
                free(l->input_field->submit_action);
		
	    free(l->input_field);
	}

	if(l->hightext)
	    free(l->hightext);
	if(l->hightext2)
	    free(l->hightext2);
	   
	if (l)
	    free(l);
    }

    /*
     *	Memory leak fixed.
     *	05-27-94 Lynx 2-3-1 Garrett Arch Blythe
     *
     *  Causes ACCVIOs/CoreDumps due to problems with the hash
     *  table when anchors are deleted, so we are living with
     *  the leaks, for now. - FM
     *
     *  I looks like the problems that were causing ACCVIOs/CoreDumps are
     *  solved, so now we will delete them, but leave it easy to compile
     *  out. - FM
     */
/* #define LIVE_WITH_LEAKS */
#ifndef LIVE_WITH_LEAKS
    if (self->node_anchor)
        HTAnchor_delete(self->node_anchor);
#endif /* !LIVE_WITH_LEAKS */

    if (self)
        free(self);
}

/*		Display Methods
**		---------------
*/


/*	Output a line
**	-------------
*/
PRIVATE int display_line ARGS1(HTLine *,line)
{
      register int i,j;
      char buffer[2];

      buffer[1] = '\0';

      clrtoeol();
      /* make sure that we don't go over the COLS limit on the display! */

	/* add offset */
      for(i=0;i < (int)line->offset && i < (int)(LYcols-1); i++)
	 addch(' ');

	/* add data */
      for(j=0;i < (LYcols-1) && line->data[j] != '\0'; i++, j++) {
	 if(!dump_output_immediately &&
	    line->data[j] == LY_UNDERLINE_START_CHAR) {
	     start_underline();
	     i--;
	 } else if(!dump_output_immediately &&
		   line->data[j] == LY_UNDERLINE_END_CHAR) {
	     stop_underline();
	     i--;
	 } else if(line->data[j] == LY_UNDERLINE_START_CHAR
	           || line->data[j] == LY_UNDERLINE_END_CHAR) {
	     buffer[0] = '_';
	     addstr(buffer);
         } else if(line->data[j] == LY_BOLD_START_CHAR) {
	     start_bold();
	     i--;
	 } else if(line->data[j] == LY_BOLD_END_CHAR) {
	     stop_bold();
	     i--;
	 } else {
	     buffer[0] = line->data[j];
	     addstr(buffer);
	 } 
      }

     /* add the return */
     addch('\n');

     stop_underline();
     stop_bold();
     return(0);

}

/*	Output the title line
**	---------------------
*/
PRIVATE void display_title ARGS1(HText *,text)
{
    CONST char * title = HTAnchor_title(text->node_anchor);
    char percent[20], format[20];
    char *cp;

    /* there shouldn't be any \n in the title, but if there is
     * lets kill it now!
     */
    if(title && (cp = strchr(title,'\n')) != NULL)
	*cp = '\0';

    if ((text->lines + 1) > (display_lines)) {
	/*	In a small attempt to correct the number of pages counted....
	 *	GAB 07-14-94
	 *
	 *	In a bigger attempt (hope it holds up 8-)....
	 *	FM 02-08-95
	 */
	int total_pages =
	 	(((text->lines + 1) + (display_lines - 1))/(display_lines));
	int start_of_last_page =
		((text->lines + 1) < display_lines) ? 0 :
		((text->lines + 1) - display_lines);	

	sprintf(percent, " (p%d of %d)",
		((text->top_of_screen >= start_of_last_page) ?
		    total_pages :
	            ((text->top_of_screen + display_lines)/(display_lines))),
		total_pages);
    } else {
	percent[0] = 0;	/* Null string */
    }

    sprintf(format, "%%%d.%ds%%s\n",	/* Generate format string */
		    (LYcols-1)-strlen(percent),
		    (LYcols-1)-strlen(percent));

    move(0,0);
    printw(format, title ? title : "" , percent);
}



/*	Output a page
**	-------------
*/
PRIVATE void display_page ARGS3(HText *,text, int,line_number, char *, target)
{
    HTLine * line = NULL;
    int i;
    char *cp;
    int last_screen = text->lines - (display_lines-2);
    TextAnchor *Anchor_ptr = NULL;
    FormInfo *FormInfo_ptr;
    BOOL display_flag=FALSE;
    HTAnchor *link_dest;

    lynx_mode = NORMAL_LYNX_MODE;
 
    if(text == NULL) {
#ifdef REVERSE_CLEAR_SCREEN_PROBLEM
	/* hack to fix reverse clear screen problem */
	addch('*');
	refresh();
	clear();
#endif /* REVERSE_CLEAR_SCREEN_PROBLEM */
	addstr("\n\nError accessing document\nNo data available\n");
	refresh();
	nlinks = 0;  /* set number of links to 0 */
	return;
    }

    line = text->last_line->prev;

/*	Constrain the line number to be within the document
*/
    if (text->lines < (display_lines)) line_number = 0;
    else if (line_number>text->lines) line_number = last_screen;
    else if (line_number < 0) line_number = 0;
    
    for(i=0,  line = text->last_line->next;		/* Find line */
    	i<line_number && (line!=text->last_line);
      i++, line=line->next) /* Loop */ assert(line->next != NULL);

/*
 *	clear the screen.
 *	hack to fix reverse clear screen problem -- shf@access.digex.net
 */
#ifdef REVERSE_CLEAR_SCREEN_PROBLEM
	addch('*');
	refresh();
	clear();
#endif /* REVERSE_CLEAR_SCREEN_PROBLEM */

    text->top_of_screen = line_number;
    display_title(text);  /* will move cursor to top of screen */
    display_flag=TRUE;
    
 /*	print it
 */
    if (line) {
      for(i=0; i < (display_lines); i++)  {

        assert(line != NULL);
        display_line(line);

        /* if the target is on this line, underline it */
        if(strlen(target) > 0 &&
	    (case_sensitive ?  
	    (cp = LYno_attr_char_strstr(line->data, target)) != NULL : 
	    (cp = LYno_attr_char_case_strstr(line->data, target)) != NULL) &&
            ((int)(cp - (char *)line->data) +
	     (int)line->offset + strlen(target)) < LYcols) {

	    int itmp=0;
	    int written=0;
	    int x_pos=(int)line->offset + (int)(cp - line->data);
	    int len = strlen(target);

	    start_underline();
		/* underline string */
	    for(; written < len && line->data[itmp] != '\0'; itmp++)  {
		if(IsSpecialAttrChar(line->data[itmp])) {
		   /* ignore special characters */
		   x_pos--;

		} else if(cp == &line->data[itmp]) {
		  /* first character of target */
            	    move(i+1, x_pos);
		    addch((unsigned char)line->data[itmp]);
		    written++;

		} else if(&line->data[itmp] > cp) {
			/* print all the other target chars */
		    addch((unsigned char)line->data[itmp]);
		    written++;
		}
	    }

	    stop_underline();
	    move(i+2, 0);
	}

	/* stop if at the last line */
	if(line == text->last_line)  {
	    /* clr remaining lines of display */
	    for(i++; i < (display_lines); i++) {
		move(i+1,0);
		clrtoeol();
	    }
	    break;
	}

	display_flag=TRUE;
	line = line->next;
      }
    }

    text->next_line = line;	/* Line after screen */
    text->stale = NO;		/* Display is up-to-date */

    /* add the anchors to lynx structures */
    nlinks = 0;
    for(Anchor_ptr=text->first_anchor;  Anchor_ptr != NULL &&
		Anchor_ptr->line_num <= line_number+(display_lines);
					    Anchor_ptr = Anchor_ptr->next) {

	if(Anchor_ptr->line_num >= line_number &&
		Anchor_ptr->line_num < line_number+(display_lines)) {

		/* load normal hypertext anchors */
	    if(Anchor_ptr->show_anchor && Anchor_ptr->hightext && 
			strlen(Anchor_ptr->hightext)>0 && 
			Anchor_ptr->link_type == HYPERTEXT_ANCHOR) {

                links[nlinks].hightext  = Anchor_ptr->hightext;
                links[nlinks].hightext2 = Anchor_ptr->hightext2;
		links[nlinks].hightext2_offset = Anchor_ptr->hightext2offset;

                links[nlinks].anchor_number = Anchor_ptr->number;

		link_dest = HTAnchor_followMainLink(
					     (HTAnchor *)Anchor_ptr->anchor);
		{
			/*
			 *	Memory leak fixed 05-27-94
			 *	Garrett Arch Blythe
			 */
			auto char *cp_AnchorAddress = HTAnchor_address(
				link_dest);

			if(links[nlinks].lname)
				free(links[nlinks].lname);

			if(cp_AnchorAddress != NULL)	{
				links[nlinks].lname = cp_AnchorAddress;
			}
			else	{
				links[nlinks].lname = empty_string;
			}
		}

      	        links[nlinks].lx= Anchor_ptr->line_pos;
      	        links[nlinks].ly= (Anchor_ptr->line_num+1)-line_number;
		links[nlinks].type = WWW_LINK_TYPE;
		links[nlinks].target = empty_string;

	        nlinks++;
		display_flag = TRUE;

	    } else if(Anchor_ptr->link_type == INPUT_ANCHOR
			&& Anchor_ptr->input_field->type != F_HIDDEN_TYPE) {

		lynx_mode = FORMS_LYNX_MODE;

		FormInfo_ptr = Anchor_ptr->input_field;

                links[nlinks].anchor_number = Anchor_ptr->number;

	   	links[nlinks].form = FormInfo_ptr;
		links[nlinks].lx = Anchor_ptr->line_pos;
		links[nlinks].ly= (Anchor_ptr->line_num+1)-line_number;
		links[nlinks].type= WWW_FORM_LINK_TYPE;
		links[nlinks].target= empty_string;
		StrAllocCopy(links[nlinks].lname, empty_string);

                if(FormInfo_ptr->type==F_CHECKBOX_TYPE) {
		    if(FormInfo_ptr->num_value)
                        links[nlinks].hightext = checked_box;
		    else
                        links[nlinks].hightext = unchecked_box;

                } else if(FormInfo_ptr->type==F_RADIO_TYPE) {
		    if(FormInfo_ptr->num_value)
                        links[nlinks].hightext = checked_radio;
		    else
                        links[nlinks].hightext = unchecked_radio;

		} else if(FormInfo_ptr->type==F_PASSWORD_TYPE) {
		    links[nlinks].hightext = STARS(strlen(FormInfo_ptr->value));

		} else {  /* TEXT type */
		    links[nlinks].hightext = FormInfo_ptr->value;
		}

		/* never a second line on form types */
		links[nlinks].hightext2 = 0;

		nlinks++;
	         /* bold the link after incrementing nlinks */
		highlight(OFF,nlinks-1);
	
		display_flag = TRUE;

	    }
//            else { /* not showing anchor */ }
	} 

	if(Anchor_ptr == text->last_anchor)
	    break;
    }


    if(!display_flag) /* nothing on the page */
	addstr("\n     Document is empty");

    refresh();

}


/*			Object Building methods
**			-----------------------
**
**	These are used by a parser to build the text in an object
*/
PUBLIC void HText_beginAppend ARGS1(HText *,text)
{
    text->permissible_split = 0;
    text->in_line_1 = YES;

}


/*	Add a new line of text
**	----------------------
**
** On entry,
**
**	split	is zero for newline function, else number of characters
**		before split.
**	text->display_on_the_fly
**		may be set to indicate direct output of the finished line.
** On exit,
**		A new line has been made, justified according to the
**		current style. Text after the split (if split nonzero)
**		is taken over onto the next line.
**
**		If display_on_the_fly is set, then it is decremented and
**		the finished line is displayed.
*/
#define new_line(text) split_line(text, 0)

PRIVATE void split_line ARGS2(HText *,text, int,split)
{
    HTStyle * style = text->style;
//#if defined(AIX) || defined(ultrix)
    HTLine * temp; /* for realloc() substitute. */
//#endif /* AIX || ultrix */
    int spare;
    int indent = text->in_line_1 ? text->style->indent1st
				 : text->style->leftIndent;

/*	Make new line
*/
    HTLine * previous = text->last_line;
    int ctrl_chars_on_previous_line = 0;
    char * cp;

    HTLine * line = (HTLine *)LY_CALLOC(sizeof(char), LINE_SIZE(MAX_LINE));

#if 0
    if (line == NULL) {
	howmuchheap();
	outofmem(__FILE__, "split_line_1");
    }
#endif
    ctrl_chars_on_this_line = 0; /*reset since we are going to a new line*/
    HTML_Last_Char=' ';

    text->lines++;
    
    previous->next->prev = line;
    line->prev = previous;
    line->next = previous->next;
    previous->next = line;
    text->last_line = line;
    line->size = 0;
    line->offset = 0;
    text->permissible_split = 0;  /* 12/13/93 */
    line->data[0] = '\0';

    /*
     * If we are not splitting and need an underline char, add it now. - FM
     */
    if ((split < 1) &&
	!dump_output_immediately && underline_on) {
	line->data[line->size++] = LY_UNDERLINE_START_CHAR;
	line->data[line->size] = '\0';
	ctrl_chars_on_this_line++;
    }
    /*
     * If we are not splitting and need a bold char, add it now. - FM
     */
    if((split < 1) && bold_on) {
	line->data[line->size++] = LY_BOLD_START_CHAR;
	line->data[line->size] = '\0';
	ctrl_chars_on_this_line++;
    }

/*	Split at required point
*/    
    if (split > 0) {	/* Delete space at "split" splitting line */
        char *p, *prevdata = previous->data, *linedata = line->data;
        unsigned int plen;
	int i;

        /*
	 * Split the line. - FM
	 */
	prevdata[previous->size] = 0;
	previous->size = split;

	/*
	 * Trim any spaces from the beginning of our new line. - FM
	 */
	p = prevdata + split;
        while (*p == ' ')
	    p++;
        plen = strlen(p);

	/*
	 * Add underline char if needed. - FM
	 */
	if (!dump_output_immediately) {
	    /*
	     * Make sure our global flag is correct. - FM
	     */
	    underline_on = NO;
	    for (i = (split-1); i >= 0; i--) {
		if (prevdata[i] == LY_UNDERLINE_END_CHAR) {
		    break;
		}
		if (prevdata[i] == LY_UNDERLINE_START_CHAR) {
		    underline_on = YES;
		    break;
		}
	    }
	    /*
	     * Act on the global flag if set above. - FM
	     */
	    if (underline_on) {
	        linedata[line->size++] = LY_UNDERLINE_START_CHAR;
		linedata[line->size] = '\0';
		ctrl_chars_on_this_line++;
	    }
	    for (i = (plen - 1); i >= 0; i--) {
		if (p[i] == LY_UNDERLINE_START_CHAR) {
		    underline_on = YES;
		    break;
		}
		if (p[i] == LY_UNDERLINE_END_CHAR) {
		    underline_on = NO;
		    break;
		}
	    }
	    for (i = (plen - 1); i >= 0; i--) {
	        if (p[i] == LY_UNDERLINE_START_CHAR ||
		    p[i] == LY_UNDERLINE_END_CHAR) {
		    ctrl_chars_on_this_line++;
		}
	    }
	}

	/*
	 * Add bold char if needed, first making
	 * sure that our global flag is correct. - FM
	 */
	bold_on = NO;
	for (i = (split - 1); i >= 0; i--) {
	    if (prevdata[i] == LY_BOLD_END_CHAR) {
		break;
	    }
	    if (prevdata[i] == LY_BOLD_START_CHAR) {
	        bold_on = YES;
		break;
	    }
	}
	/*
	 * Act on the global flag if set above. - FM
	 */
	if (bold_on) {
	    linedata[line->size++] = LY_BOLD_START_CHAR;
	    linedata[line->size] = '\0';
	    ctrl_chars_on_this_line++;
	}
	for (i = (plen - 1); i >= 0; i--) {
	    if (p[i] == LY_BOLD_START_CHAR) {
		bold_on = YES;
		break;
	    }
	    if (p[i] == LY_BOLD_END_CHAR) {
		bold_on = NO;
		break;
	    }
	}
	for (i = (plen - 1); i >= 0; i--) {
	    if (p[i] == LY_BOLD_START_CHAR ||
		p[i] == LY_BOLD_END_CHAR) {
		ctrl_chars_on_this_line++;
	    }
	}

	/*
	 * Add the data to the new line. - FM
	 */
	strcat(linedata, p);
	line->size += plen;
    }

/*	Economize on space.
*/
    while ((previous->size > 0) &&
	(previous->data[previous->size-1] == ' '))	/* Strip trailers */
	previous->size--;

    /*
     * Use a substitute for realloc.
     */

    temp = (HTLine *)LY_CALLOC(1, LINE_SIZE(previous->size));
#if 0
    if (temp == NULL) {
	howmuchheap();
	outofmem(__FILE__, "split_line_2");
    }
#endif
    memcpy(temp, previous, LINE_SIZE(previous->size));
    free(previous);
    previous = temp;

    previous->prev->next = previous;	/* Link in new line */
    previous->next->prev = previous;	/* Could be same node of course */

/*	Terminate finished line for printing
*/
    previous->data[previous->size] = 0;
     
    
/*	Align left, right or center
*/

    for (cp = previous->data; *cp; cp++) {
        if (*cp == LY_UNDERLINE_START_CHAR ||
	    *cp == LY_UNDERLINE_END_CHAR ||
	    *cp == LY_BOLD_START_CHAR ||
	    *cp == LY_BOLD_END_CHAR)
	    ctrl_chars_on_previous_line++;
    }
    /* @@ first line indent */
    spare =  (LYcols-1) -
    		(int)style->rightIndent - indent +
    		ctrl_chars_on_previous_line - previous->size;

    switch (style->alignment) {
	case HT_CENTER :
	    previous->offset = previous->offset + indent + spare/2;
	    break;
	case HT_RIGHT :
	    previous->offset = previous->offset + indent + spare;
	    break;
	case HT_LEFT :
	case HT_JUSTIFY :		/* Not implemented */
	default:
	    previous->offset = previous->offset + indent;
	    break;
    } /* switch */

    text->chars = text->chars + previous->size + 1;	/* 1 for the line */
    text->in_line_1 = NO;		/* unless caller sets it otherwise */
    
} /* split_line */



/*	Allow vertical blank space
**	--------------------------
*/
PRIVATE void blank_lines ARGS2(HText *,text, int,newlines)
{
    if (text->last_line->size == 0) {	/* No text on current line */
	HTLine * line = text->last_line->prev;
	while ((line!=text->last_line) && (line->size == 0)) {
	    if (newlines==0) break;
	    newlines--;		/* Don't bother: already blank */
	    line = line->prev;
	}
    } else {
	newlines++;			/* Need also to finish this line */
    }

    for(;newlines;newlines--) {
	new_line(text);
    }
    text->in_line_1 = YES;
}


/*	New paragraph in current style
**	------------------------------
** See also: setStyle.
*/

PUBLIC void HText_appendParagraph ARGS1(HText *,text)
{
    int after = text->style->spaceAfter;
    int before = text->style->spaceBefore;
    blank_lines(text, after>before ? after : before);
}


/*	Set Style
**	---------
**
**	Does not filter unnecessary style changes.
*/
PUBLIC void HText_setStyle ARGS2(HText *,text, HTStyle *,style)
{
    int after, before;

    if (!style) return;				/* Safety */
    after = text->style->spaceAfter;
    before = style->spaceBefore;
    blank_lines (text, after>before ? after : before);

    text->style = style;
}


/*	Append a character to the text object
**	-------------------------------------
*/
PUBLIC void HText_appendCharacter ARGS2(HText *,text, char,ch)
{
    HTLine * line;
    HTStyle * style;
    int indent;

    if(!text || text==NULL)
	return;

    line = text->last_line;
    style = text->style;

    indent = text->in_line_1 ? (int)style->indent1st : (int)style->leftIndent;
    

    if(IsSpecialAttrChar(ch)) 
        if (ch == LY_UNDERLINE_START_CHAR) { 
            line->data[line->size++] = LY_UNDERLINE_START_CHAR;
	    line->data[line->size] = '\0';
	    underline_on = ON;
	    if (!dump_output_immediately)
		ctrl_chars_on_this_line++;
	    return;
        } else if (ch == LY_UNDERLINE_END_CHAR) {
            line->data[line->size++] = LY_UNDERLINE_END_CHAR;
	    line->data[line->size] = '\0';
	    underline_on = OFF;
	    if (!dump_output_immediately)
	    	ctrl_chars_on_this_line++;
	    return;
        } else if (ch == LY_BOLD_START_CHAR) {
            line->data[line->size++] = LY_BOLD_START_CHAR;
	    line->data[line->size] = '\0';
            bold_on = ON;
	    ctrl_chars_on_this_line++;
            return;
        } else if (ch == LY_BOLD_END_CHAR) {
            line->data[line->size++] = LY_BOLD_END_CHAR;
	    line->data[line->size] = '\0';
            bold_on = OFF;
	    ctrl_chars_on_this_line++;
            return;
        }

/*		New Line
*/
    if (ch == '\n') {
	    new_line(text);
	    text->in_line_1 = YES;	/* First line of new paragraph */
	    return;
    }

    /* convert EM_SPACE to a space here so that it doesn't get
     * collapsed
     */
    if (ch == HT_EM_SPACE)
	ch = ' ';

    /* I'm going to cheat here in a BIG way.  Since I know that all
     * \r's will be trapped by HTML_put_character I'm going to use
     * \r to mean go down a line but don't start a new paragraph.  
     * i.e. use the second line indenting.
     */
    if (ch == '\r') {
	new_line(text);
	text->in_line_1 = NO;
	return;
    }


/* 		Tabs
*/

    if (ch == '\t') {
        HTTabStop * tab;
	int target;	/* Where to tab to */
	int here;

	here = (((int)line->size + (int)line->offset) + indent)
		- ctrl_chars_on_this_line; /* Consider special chars GAB */
        if (style->tabs) {	/* Use tab table */
	    for (tab = style->tabs;
	    	tab->position <= here;
		tab++)
		if (!tab->position) {
		    new_line(text);
		    return;
		}
	    target = tab->position;
	} else if (text->in_line_1) {	/* Use 2nd indent */
	    if (here >= (int)style->leftIndent) {
	        new_line(text); /* wrap */
		return;
	    } else {
	        target = (int)style->leftIndent;
	    }
	} else {		/* Default tabs align with left indent mod 8 */
#ifdef DEFAULT_TABS_8
	    target = (((int)line->offset + (int)line->size + 8) & (-8))
	    		+ (int)style->leftIndent;
#else
	    new_line(text);
	    return;
#endif
	}

	if (target > (LYcols-1) - (int)style->rightIndent) {
	    new_line(text);
	    return;
	} else {
            text->permissible_split = (int)line->size;	/* Can split here */
	    if (line->size == 0)
	        line->offset = line->offset + target - here;
	    else for(; here<target; here++) {
                line->data[line->size++] = ' ';	/* Put character into line */
		line->data[line->size] = '\0';
	    }
	    return;
	}
	/*NOTREACHED*/
    } /* if tab */ 

    
    if (ch==' ') {
        text->permissible_split = (int)line->size;	/* Can split here */
    }

/*	Check for end of line
*/    
    if (((indent + (int)line->offset + (int)line->size) + 
	(int)style->rightIndent - ctrl_chars_on_this_line) >= (LYcols-1)) {

        if (style->wordWrap && HTOutputFormat!=WWW_SOURCE) {
	    split_line(text, text->permissible_split);
	    if (ch==' ') return;	/* Ignore space causing split */

	}  else if(HTOutputFormat==WWW_SOURCE) {
		 /* for source output we 
		  * dont want to wrap this stuff unless absolutely
		  * neccessary LJM 
		  * !
		  * If we don't wrap here we might get a segmentation fault.
		  * but let's see what happens
		  */
		if((int)line->size >= (int)(MAX_LINE-1))
		   new_line(text);  /* try not to linewrap */
	} else {
		/* for normal stuff like pre let's go ahead and
		 * wrap so the user can see all of the text
		 */
		new_line(text);  
	}
    }

/*	Insert normal characters
*/
    if (ch == HT_NON_BREAK_SPACE) {
        ch = ' ';
    }

    {
        HTLine * line = text->last_line;	/* May have changed */
        HTFont font = style->font;
        line->data[line->size++] =	/* Put character into line */
           font & HT_CAPITALS ? TOUPPER(ch) : ch;
	line->data[line->size] = '\0';
        if (font & HT_DOUBLE)		/* Do again if doubled */
            HText_appendCharacter(text, HT_NON_BREAK_SPACE);
	    /* NOT a permissible split */ 
    }
}

/*		Anchor handling
**		---------------
*/
/*	Start an anchor field
*/
PUBLIC void HText_beginAnchor ARGS2(HText *,text, HTChildAnchor *,anc)
{
    char marker[16];

    TextAnchor * a = (TextAnchor *) calloc(sizeof(*a),1);
    
    if (a == NULL) outofmem(__FILE__, "HText_beginAnchor");
    a->hightext  = 0;
    a->hightext2 = 0;
    a->start = text->chars + text->last_line->size;

    a->line_pos = text->last_line->size;
    if (text->last_anchor) {
        text->last_anchor->next = a;
    } else {
        text->first_anchor = a;
    }
    a->next = 0;
    a->anchor = anc;
    a->extent = 0;
    a->link_type = HYPERTEXT_ANCHOR;
    text->last_anchor = a;
    
    if (HTAnchor_followMainLink((HTAnchor*)anc)) {
        a->number = ++(text->last_anchor_number);
    } else {
        a->number = 0;
    }

    /* if we are doing link_numbering add the link number */
    if(keypad_mode == LINKS_ARE_NUMBERED && a->number > 0) {
	sprintf(marker,"[%d]", a->number);
        HText_appendText(text, marker);
	a->start += strlen(marker);
    }
}


PUBLIC void HText_endAnchor ARGS1(HText *,text)
{
    TextAnchor * a = text->last_anchor;
    if (a->number) {	 /* If it goes somewhere */
            a->extent += text->chars + text->last_line->size - a->start;
	    a->show_anchor = YES;
    } else {
	    a->show_anchor = NO;
	    a->extent = 0;
    }
 
}


PUBLIC void HText_appendText ARGS2(HText *,text, CONST char *,str)
{
    CONST char * p;

    if(str==NULL)
	return;

    if (text->halted == 3)
      return;

    for(p=str; *p; p++) {
        HText_appendCharacter(text, *p);
    }
}


PRIVATE void remove_special_attr_chars ARGS1(char *,buf)
{
    register char *cp;

    for (cp=buf; *cp != '\0' ; cp++) {
         /* don't print underline chars */
        if(!IsSpecialAttrChar(*cp)) {
           *buf = *cp, 
           buf++;
	}
    }
    *buf = '\0';
}


PUBLIC void HText_endAppend ARGS1(HText *,text)
{
    int cur_line,cur_char;
    TextAnchor *anchor_ptr;
    HTLine *line_ptr;

    if(!text)
	return;

   if (text->halted > 1) {
	if (text->halted == 2) {
		text->halted = 0;
//		text->kanji_buf = '\0';
		HText_appendText(text, " *** MEMORY EXHAUSTED ***");
	}
	text->halted = 3;
	return;
   }


    new_line(text);

    if (text->halted) {
      LYFakeZap(NO);
      text->halted = 0;
    }


    /* get the hightext from the text by finding the char
     * position, then
     * bring the anchors in line with the text by adding the
     * text offset to each of the anchors
     */
     /* get first line */
     line_ptr = text->last_line->next;
     cur_char = line_ptr->size;;
     cur_line = 0;

	/* remove the blank lines at the end of document */
     while(text->last_line->data[0]=='\0' && text->lines > 2) {
	HTLine *next_to_the_last_line;

	next_to_the_last_line = text->last_line->prev;

	/* line_ptr points to the first line */
	next_to_the_last_line->next = line_ptr;
	line_ptr->prev = next_to_the_last_line;
	if(text->last_line)
	    free(text->last_line);
	text->last_line = next_to_the_last_line;
	text->lines--;

     }


     for(anchor_ptr=text->first_anchor; anchor_ptr; 
				             anchor_ptr=anchor_ptr->next) {

re_parse:
	  /* find the right line */
	 for(;anchor_ptr->start >= cur_char; line_ptr = line_ptr->next, 
				    cur_char += line_ptr->size+1, cur_line++) 
		; /* null body */

	 if(anchor_ptr->start == cur_char)
	    anchor_ptr->line_pos = line_ptr->size;
	 else
	    anchor_ptr->line_pos = anchor_ptr->start-(cur_char-line_ptr->size);

	 if(anchor_ptr->line_pos < 0)
	     anchor_ptr->line_pos = 0;


	 /* strip off a spaces at the beginning if they exist
	  * but only on HYPERTEXT_ANCHORS
	  */
	 if(anchor_ptr->link_type == HYPERTEXT_ANCHOR)
             while(isspace(line_ptr->data[anchor_ptr->line_pos]) ||
		IsSpecialAttrChar(line_ptr->data[anchor_ptr->line_pos])) {
		    anchor_ptr->line_pos++;
		    anchor_ptr->extent--;
	     }

	 if(anchor_ptr->extent < 0)
	    anchor_ptr->extent = 0;

	  /* if the link begins with a end of line then start the
	   * highlighting on the next line
	   */
	 if(anchor_ptr->line_pos >= strlen(line_ptr->data))
	  {
	     anchor_ptr->start++;

	     goto re_parse;
	  }

	 /* copy the link name into the data structure */
	 if(line_ptr->data 
		&& anchor_ptr->extent > 0 
		    && anchor_ptr->line_pos >= 0) {

	     StrnAllocCopy(anchor_ptr->hightext, 
		&line_ptr->data[anchor_ptr->line_pos], anchor_ptr->extent);

	 } else {
	     StrAllocCopy(anchor_ptr->hightext,""); 
	 }

	/*  If true the anchor extends over two lines */
	if(anchor_ptr->extent > strlen(anchor_ptr->hightext)) {
            HTLine *line_ptr2 = line_ptr->next;
		/* double check! */
	    if(line_ptr) {
		StrnAllocCopy(anchor_ptr->hightext2, line_ptr2->data, 
			 (anchor_ptr->extent - strlen(anchor_ptr->hightext))-1);
	        anchor_ptr->hightext2offset = line_ptr2->offset;
	 	remove_special_attr_chars(anchor_ptr->hightext2);
	    }
	 }   

	 remove_special_attr_chars(anchor_ptr->hightext);

        /* subtract any formatting characters from the x position
         * of the link
         */
        if(anchor_ptr->line_pos > 0) {
            register int offset=0, i=0;
            for(; i < anchor_ptr->line_pos; i++)
                if(IsSpecialAttrChar(line_ptr->data[i]))
                    offset++;
            anchor_ptr->line_pos -= offset;
        }

        anchor_ptr->line_pos += line_ptr->offset;  /* add the offset */
        anchor_ptr->line_num  = cur_line;


 	 if(anchor_ptr == text->last_anchor)
	     break;
     }

}


/* 	Dump diagnostics to stderr
*/
PUBLIC void HText_dump ARGS1(HText *,text)
{
    fprintf(stderr, "HText: Dump called\n");
}
	

/*	Return the anchor associated with this node
*/
PUBLIC HTParentAnchor * HText_nodeAnchor ARGS1(HText *,text)
{
    return text->node_anchor;
}

/*				GridText specials
**				=================
*/
/*	Return the anchor with index N
**
**	The index corresponds to the number we print in the anchor.
*/
PUBLIC HTChildAnchor * HText_childNumber ARGS1(int,number)
{
    TextAnchor * a;
    for (a = HTMainText->first_anchor; a; a = a->next) {
        if (a->number == number) return(a->anchor);
    }
    return (HTChildAnchor *)0;	/* Fail */
}

/* HTGetLinkInfo returns some link info based on the number
 */
PUBLIC int HTGetLinkInfo ARGS3(int, number, char **, hightext, char **, lname)
{
    TextAnchor * a;
    HTAnchor *link_dest;

    for (a = HTMainText->first_anchor; a; a = a->next) {
        if (a->number == number) {
	    *hightext= a->hightext;
            link_dest = HTAnchor_followMainLink(
                                               (HTAnchor *)a->anchor);
	    {
		/*
		 *	Memory Leak fixed.
		 *	05-28-94 Lynx 2-3-1 Garrett Arch Blythe
	 	 */
		auto char *cp_freeme = HTAnchor_address(link_dest);
            	StrAllocCopy(*lname, cp_freeme);
		if (cp_freeme)
		    free(cp_freeme);
	    }
	    return(YES);
	}
    }
    return(NO);
}

/* HText_getNumOfLines returns the number of lines in the
 * current document
 */
PUBLIC int HText_getNumOfLines NOARGS
{
     return(HTMainText->lines);
}

/* HText_getTitle returns the title of the
 * current document
 */
PUBLIC char * HText_getTitle NOARGS
{
   return((char *) HTAnchor_title(HTMainText->node_anchor));
}

/* HText_pageDisplay displays a screen of text
 * starting from the line 'line_num'-1
 * this is the primary call for lynx
 */
extern char is_www_index;

PUBLIC void HText_pageDisplay ARGS2(int,line_num, char *, target)
{
    display_page(HTMainText, line_num-1, target);

    is_www_index = HTAnchor_isIndex(HTMainAnchor);
} 

PUBLIC void HText_setStale ARGS1(HText *,text)
{
    text->stale = YES;
}

PUBLIC void HText_refresh ARGS1(HText *,text)
{
    if (text->stale) display_page(text, text->top_of_screen, "");
}

PUBLIC int HText_sourceAnchors ARGS1(HText *,text)
{
    return text->last_anchor_number;
}

PUBLIC BOOL HText_canScrollUp ARGS1(HText *,text)
{
    return (text->top_of_screen != 0);
}

PUBLIC BOOL HText_canScrollDown NOARGS
{
    HText * text = HTMainText;

    return ((text->top_of_screen + display_lines) < text->lines+1);
}

/*		Scroll actions
*/
PUBLIC void HText_scrollTop ARGS1(HText *,text)
{
    display_page(text, 0, "");
}

PUBLIC void HText_scrollDown ARGS1(HText *,text)
{
    display_page(text, text->top_of_screen + display_lines, "");
}

PUBLIC void HText_scrollUp ARGS1(HText *,text)
{
    display_page(text, text->top_of_screen - display_lines, "");
}

PUBLIC void HText_scrollBottom ARGS1(HText *,text)
{
    display_page(text, text->lines - display_lines, "");
}


/*		Browsing functions
**		==================
*/

/* Bring to front and highlight it
*/

PRIVATE int line_for_char ARGS2(HText *,text, int,char_num)
{
    int line_number =0;
    int characters = 0;
    HTLine * line = text->last_line->next;
    for(;;) {
	if (line == text->last_line) return 0;	/* Invalid */
        characters = characters + line->size + 1;
	if (characters > char_num) return line_number;
	line_number ++;
	line = line->next;
    }
}

PUBLIC BOOL HText_select ARGS1(HText *,text)
{
    if (text != HTMainText) {
        HTMainText = text;
	HTMainAnchor = text->node_anchor;
	  /* let lynx do it */
	/* display_page(text, text->top_of_screen, ""); */
    }
    return YES;
}

PUBLIC BOOL HTFindPoundSelector ARGS1(char *,selector)
{
    TextAnchor * a;

    for(a=HTMainText->first_anchor; a; a=a->next) {

        if(a->anchor && a->anchor->tag)
            if(!strcmp(a->anchor->tag, selector)) {

                 www_search_result = a->line_num+1;

                 return(YES);
            }
    }

    return(NO);

}

PUBLIC BOOL HText_selectAnchor ARGS2(HText *,text, HTChildAnchor *,anchor)
{
    TextAnchor * a;

/* This is done later, hence HText_select is unused in GridText.c
   Should it be the contrary ? @@@
    if (text != HTMainText) {
        HText_select(text);
    }
*/

    for(a=text->first_anchor; a; a=a->next) {
        if (a->anchor == anchor) break;
    }
    if (!a) {
        return NO;
    }

    if (text != HTMainText) {		/* Comment out by ??? */
        HTMainText = text;		/* Put back in by tbl 921208 */
	HTMainAnchor = text->node_anchor;
    }

    {
	 int l = line_for_char(text, a->start);

	if ( !text->stale &&
	     (l >= text->top_of_screen) &&
	     ( l < text->top_of_screen + display_lines+1))
	         return YES;

	www_search_result = l - (display_lines/3); /* put in global variable */
    }
    
    return YES;
}
 

/*		Editing functions		- NOT IMPLEMENTED
**		=================
**
**	These are called from the application. There are many more functions
**	not included here from the orginal text object.
*/

/*	Style handling:
*/
/*	Apply this style to the selection
*/
PUBLIC void HText_applyStyle ARGS2(HText *, me, HTStyle *,style)
{
    
}


/*	Update all text with changed style.
*/
PUBLIC void HText_updateStyle ARGS2(HText *, me, HTStyle *,style)
{
    
}


/*	Return style of  selection
*/
PUBLIC HTStyle * HText_selectionStyle ARGS2(
	HText *,me,
	HTStyleSheet *,sheet)
{
    return 0;
}


/*	Paste in styled text
*/
PUBLIC void HText_replaceSel ARGS3(
	HText *,me,
	CONST char *,aString, 
	HTStyle *,aStyle)
{
}


/*	Apply this style to the selection and all similarly formatted text
**	(style recovery only)
*/
PUBLIC void HTextApplyToSimilar ARGS2(HText *,me, HTStyle *,style)
{
    
}

 
/*	Select the first unstyled run.
**	(style recovery only)
*/
PUBLIC void HTextSelectUnstyled ARGS2(HText *,me, HTStyleSheet *,sheet)
{
    
}


/*	Anchor handling:
*/
PUBLIC void		HText_unlinkSelection ARGS1(HText *,me)
{
    
}

PUBLIC HTAnchor *	HText_referenceSelected ARGS1(HText *,me)
{
     return 0;   
}


PUBLIC int HText_getTopOfScreen NOARGS
{
      HText * text = HTMainText;
      return text->top_of_screen;
}

PUBLIC int HText_getLines ARGS1(HText *,text)
{
      return text->lines;
}

PUBLIC HTAnchor *	HText_linkSelTo ARGS2(HText *,me, HTAnchor *,anchor)
{
    return 0;
}


PUBLIC int do_www_search ARGS1(document *,doc)
{
       char searchstring[256], temp[256], *cp, *tmpaddress=NULL;

       /*
        * Load the default query buffer
	*/
       if((cp=strchr(doc->address, '?')) != NULL) {
           /*
	    * This is an index from a previous search.
	    * Use its query as the default.
	    */
	   strcpy(searchstring, ++cp);
	   for (cp=searchstring; *cp; cp++)
	       if (*cp == '+')
	           *cp = ' ';
	   HTUnEscape(searchstring);
	   strcpy(temp, searchstring);
       } else {
           /*
	    * New search; no default.
	    */
           searchstring[0] = '\0';
	   temp[0] = '\0';
       }

       /*
        * Prompt for a query string.
	*/
       if(searchstring[0] == '\0') {
           if (HTMainAnchor->isIndexPrompt)
               _statusline(HTMainAnchor->isIndexPrompt);
	   else
               _statusline("Enter a database query: ");
       } else
           _statusline("Edit the current query: ");
       if(LYgetstr(searchstring, VISIBLE) < 0 || *searchstring == '\0')
           /*
	    * Search cancelled.
	    */
	   return(NULLFILE);

       /*
        * Don't resubmit the same query unintentionally.
	*/
       if(!LYforce_no_cache && 0==strcmp(temp, searchstring)) {
	   _statusline("Use Control-R to resubmit the current query.");
	   sleep(sleep_two);
	   return(NULLFILE);
       }

       /*
        * Show the URL with the new query.
	*/
       if((cp=strchr(doc->address, '?')) != NULL)
           *cp = '\0';
       StrAllocCopy(tmpaddress, doc->address);
       StrAllocCat(tmpaddress, "?");
       StrAllocCat(tmpaddress, searchstring);
       user_message(WWW_WAIT_MESSAGE, tmpaddress);
#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
       syslog("%s", tmpaddress);
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */
       free(tmpaddress);
       if(cp)
           *cp = '?';

       /*
        * OK, now we do the search.
	*/
       if (HTSearch(searchstring, HTMainAnchor)) {
	   /*
	    *	Memory leak fixed.
	    *	05-28-94 Lynx 2-3-1 Garrett Arch Blythe
	    */
	   auto char *cp_freeme = HTAnchor_address((HTAnchor *)HTMainAnchor);
           StrAllocCopy(doc->address, cp_freeme);
	   if (cp_freeme)
	       free(cp_freeme);

           /*
	    * Yah, the search succeeded.
	    */
	   return(NORMAL);
	}

       /*
        * Either the search failed (Yuk), or we got redirection.
	* If it's redirection, use_this_url_instead is set, and
	* mainloop() will deal with it such that security features
	* and restrictions are checked before acting on the URL, or
	* rejecting it. - FM
	*/
       return(NOT_FOUND);
}

/* print the contents of the file in HTMainText to
 * the file descripter fp.
 * if is_reply is true add ">" to the beginning of each
 * line to specify the file is a replied to message
 */
PUBLIC void print_wwwfile_to_fd ARGS2(FILE *,fp, int,is_reply)
{
      register int i;
      HTLine * line = HTMainText->last_line->next;
#ifdef VMS
      extern BOOLEAN HadVMSInterrupt;
#endif /* VMS */

      for(;; line = line->next) {

	  if(is_reply)
             fputc('>',fp);

            /* add offset */
          for(i=0;i < (int)line->offset; i++)
             fputc(' ',fp);

            /* add data */
          for(i=0;line->data[i] != '\0'; i++)
             if(!IsSpecialAttrChar(line->data[i]))
                fputc(line->data[i],fp);
	     else if (dump_output_immediately) {
		switch (line->data[i]) {
		    case LY_UNDERLINE_START_CHAR:
		    case LY_UNDERLINE_END_CHAR:
			fputc('_', fp);
			break;
		    case LY_BOLD_START_CHAR:
		    case LY_BOLD_END_CHAR:
			break;
		}
	     } 

         /* add the return */
         fputc('\n',fp);

	 if(line == HTMainText->last_line)
	    break;

#ifdef VMS
	if (HadVMSInterrupt)
	    break;
#endif /* VMS */
    }

}

PUBLIC void www_user_search ARGS2(int,start_line, char *,target)
{
    register HTLine * line = HTMainText->last_line->next;
    register int count;
    extern BOOLEAN case_sensitive;

	/* advance to the start line */
    for(count=1; count <= start_line; line=line->next, count++)
	; /* null */

    for(;;) {
	if(case_sensitive && LYno_attr_char_strstr(line->data, target)) {
	    www_search_result=count;
	    return;
	} else if(!case_sensitive && LYno_attr_char_case_strstr(line->data, target)) {
	    www_search_result=count;
	    return;
	} else if(line == HTMainText->last_line) {  /* next line */
	    break;
	} else {			/* end */
	    line = line->next;
	    count++;
	}
    }

	/* search from the beginning */
    line = HTMainText->last_line->next; /* set to first line */
    count = 1;

    for(;;) {
	    if(case_sensitive && LYno_attr_char_strstr(line->data, target)) {
	        www_search_result=count;
		return;
	    } else if(!case_sensitive && LYno_attr_char_case_strstr(line->data, target)) {
	        www_search_result=count;
		return;
	    } else if(count > start_line) {  /* next line */
    		_user_message("\"%s\" could not be found in this document",target);
    		sleep(sleep_two);
	        return;			/* end */
	    } else {
	        line = line->next;
		count++;
	    }
    }


}

PUBLIC  void  user_message ARGS2(char *,message, char *,argument) 
{
    char *temp = NULL;
    char temp_arg[256];

    if(message==NULL)
	return;

   /* make sure we don't overun any buffers */
    LYstrncpy(temp_arg, (temp_arg==NULL ? "" : argument), 255);
    temp = (char *)malloc(strlen(message) + strlen(temp_arg) + 1);
    sprintf(temp, message, temp_arg);

    statusline(temp);
   
    free(temp);
    return;
}

/* HText_getOwner returns the owner of the
 * current document
 */
PUBLIC char * HText_getOwner NOARGS
{
   return((char *)HTAnchor_owner(HTMainText->node_anchor));
}

PUBLIC void HTuncache_current_document NOARGS
{
    /* should remove current document from memory */
    HTList_removeObject(loaded_texts, HTMainText);
    HText_free(HTMainText);
    HTMainText=0;
}

PUBLIC int HTisDocumentSource NOARGS
{
   return(HTMainText->source);
}

PUBLIC char * HTLoadedDocumentURL NOARGS
{

   if(!HTMainText)
	return ("");

   if(HTMainText->node_anchor && HTMainText->node_anchor->address) 
       	return(HTMainText->node_anchor->address);
   else
	return ("");
}

PUBLIC char * HTLoadedDocumentPost_data NOARGS
{
   if(!HTMainText)
	return ("");

   if(HTMainText->node_anchor && HTMainText->node_anchor->post_data) 
       	return(HTMainText->node_anchor->post_data);
   else
	return ("");
}

PUBLIC char * HTLoadedDocumentTitle NOARGS
{
   if(!HTMainText)
	return ("");

   if(HTMainText->node_anchor && HTMainText->node_anchor->title) 
       	return(HTMainText->node_anchor->title);
   else
	return ("");
}

/*  Form methods
 *    These routines are used to build forms consisting
 *    of input fields 
 */

PRIVATE int HTFormMethod;
PRIVATE char * HTFormAction;
PRIVATE BOOLEAN HTFormDisabled = FALSE;

PUBLIC void HText_beginForm ARGS2(char *,action, char *,method)
{
    HTFormMethod = URL_GET_METHOD;
    HTFormNumber++;

    if(action!=NULL)	{
	if(!strncmp(action, "mailto:", 7))	{
		HTFormMethod = URL_MAIL_METHOD;
	}
        StrAllocCopy(HTFormAction, action);
    }
    else
	StrAllocCopy(HTFormAction, HTLoadedDocumentURL());
    
    if(method!=NULL)
	if(!strcasecomp(method,"post") && HTFormMethod != URL_MAIL_METHOD)
	   HTFormMethod = URL_POST_METHOD;

}

PUBLIC void HText_endForm NOARGS
{
    if (HTFormAction)
        free(HTFormAction);
    HTFormAction = 0;
}

PUBLIC void HText_beginSelect ARGS3(char *,name, BOOLEAN,multiple, char *, size)
{
   /* save the group name */
   HTCurSelectGroup = name;

   /* if multiple then all options are actually checkboxes */
   if(multiple)
      HTCurSelectGroupType = F_CHECKBOX_TYPE;
	/* if not multiple then all options are radio buttons */
   else
      HTCurSelectGroupType = F_RADIO_TYPE;

    /*	Length of an option list.
     */
    HTCurSelectGroupSize = size;

} 

/* we couln't set the value field for the previous option
 * tag so we have to do it now.  Assume that the last anchor
 * was the previous options tag
 */
PUBLIC char * HText_setLastOptionValue ARGS5(HText *, text, char *, value,
						char*, submit_value,
						int, order, BOOLEAN, checked)
{
   char *cp;
   static char * selectedOptionValue=0;
   int number=0;

   if(text->last_anchor->link_type != INPUT_ANCHOR)
	return NULL;

   /* strip end spaces, return is also whitespace. */
   cp = &value[strlen(value)-1];
   while(isspace(*cp) || IsSpecialAttrChar(*cp)) cp--;
   *(cp+1) = '\0';

   /* find first non space */
   cp = value;
   while(isspace(*cp) || IsSpecialAttrChar(*cp)) cp++;

   if(HTCurSelectGroupType == F_CHECKBOX_TYPE) {
       StrAllocCopy(text->last_anchor->input_field->value, cp);

       /* put the text on the screen as well */
       HText_appendText(text, cp);

   } else {
	/* create a linked list of option values */

	OptionType * op_ptr = text->last_anchor->input_field->select_list;
	OptionType * new_ptr=0;
	BOOLEAN first_option = FALSE;

	if(!op_ptr) {  /* no option items yet */
	    new_ptr = text->last_anchor->input_field->select_list = 
				(OptionType *) calloc(1,sizeof(OptionType));

	    first_option = TRUE;
	} else {
	    while(op_ptr->next) {
		number++;
		op_ptr=op_ptr->next;
	    }
	    number++;  /* add one more */

	    op_ptr->next = new_ptr =
	    			(OptionType *) calloc(1,sizeof(OptionType));
	}

	new_ptr->name = 0;
	new_ptr->cp_submit_value = 0;
	new_ptr->next = 0;
	StrAllocCopy(new_ptr->name, cp);
	StrAllocCopy(new_ptr->cp_submit_value,
	    		 submit_value ? submit_value : cp);

	if(first_option) {
	    StrAllocCopy(selectedOptionValue, cp);
	    text->last_anchor->input_field->num_value = 0;
	    text->last_anchor->input_field->value = 
		text->last_anchor->input_field->select_list->name;
	    text->last_anchor->input_field->orig_value = 
		text->last_anchor->input_field->select_list->name;
	    text->last_anchor->input_field->cp_submit_value = 
		text->last_anchor->input_field->select_list->cp_submit_value;
	    text->last_anchor->input_field->orig_submit_value = 
		text->last_anchor->input_field->select_list->cp_submit_value;
	} else {
	    int newlen = strlen(cp);
	    int curlen = strlen(selectedOptionValue);
		/* make the selected Option Value as long as the longest
		 * option
		 */
	    if(newlen > curlen)
		StrAllocCat(selectedOptionValue, UNDERSCORES(newlen-curlen));
	}

	if(checked) {
	    int curlen = strlen(new_ptr->name);
	    int newlen = strlen(selectedOptionValue);
		/* set the default option as this one */
	    text->last_anchor->input_field->num_value = number;
	    text->last_anchor->input_field->value = new_ptr->name;
	    text->last_anchor->input_field->orig_value = new_ptr->name;
	    text->last_anchor->input_field->cp_submit_value =
	    			   new_ptr->cp_submit_value;
	    text->last_anchor->input_field->orig_submit_value =
	    			   new_ptr->cp_submit_value;
	    StrAllocCopy(selectedOptionValue, new_ptr->name);
	    if(newlen > curlen)
		StrAllocCat(selectedOptionValue, UNDERSCORES(newlen-curlen));
	}
	     

	/* return the selected Option value to be sent to the screen */
	if(order == LAST_ORDER) {
		/* change the value */
	    text->last_anchor->input_field->size = strlen(selectedOptionValue); 
	    return(selectedOptionValue);
	} else 
	   return(NULL);
   }
	
   return(NULL);
}

/*
 *  Assign a form input anchor
 *  returns the number of charactors to leave blank
 *  so that the input field can fit
 */
PUBLIC int HText_beginInput ARGS2(HText *,text, InputFieldData *,I)
{
	
    TextAnchor * a = (TextAnchor *) calloc(sizeof(*a),1);
    FormInfo * f = (FormInfo *) calloc(sizeof(*f),1); 
    char *cp_option = NULL; 


    if (a == NULL || f == NULL) outofmem(__FILE__, "HText_beginInput");

    a->start = text->chars + text->last_line->size;
    a->line_pos = text->last_line->size;

    if (text->last_anchor) {
        text->last_anchor->next = a;
    } else {
        text->first_anchor = a;
    }
    a->next = 0;
    a->anchor = NULL;
    a->link_type = INPUT_ANCHOR;
    a->show_anchor = YES;

    a->hightext = NULL;
    a->extent = 2;

    a->input_field = f;

    f->select_list = 0;
    f->number = HTFormNumber;

    /* special case of option */
	/* set the values and let the parsing below do the work */
    if(I->type!=NULL && !strcmp(I->type,"OPTION")) {
	cp_option = I->type;
 	if(HTCurSelectGroupType==F_RADIO_TYPE)
	    I->type = "OPTION_LIST";
	else
	    I->type = "CHECKBOX";
	I->name = HTCurSelectGroup;

	/*	The option's size parameter actually gives the length and not
	 *		the width of the list.  Perform the conversion here
	 *		and get rid of the allocated HTCurSelect....
	 *	0 is ok as it means any length (arbitrary decision).
	 */
	if(HTCurSelectGroupSize != NULL)	{
		f->size_l = atoi(HTCurSelectGroupSize);
		free(HTCurSelectGroupSize);
		HTCurSelectGroupSize = NULL;
	}
    }

	/* set SIZE */
    if(I->size != NULL) {
	f->size = atoi(I->size);
	/*	Leave at zero for option lists.
	 */
	if(f->size == 0 && cp_option == NULL)	{
	   f->size = 20;  /* default */
	}
    } else {
	f->size = 20;  /* default */
    }
       if(f->size < 0) f->size = 1;

	/* set MAXLENGTH */
    if(I->maxlength != NULL) {
	f->maxlength = atoi(I->maxlength);

    } else {
	f->maxlength = 0;  /* 0 means infinite */
    }

	/* set CHECKED */
    /* num_value is only relevent to check and radio types */
    if(I->checked == TRUE)
 	f->num_value = 1; 
    else
 	f->num_value = 0;

	/* set TYPE */
    if(I->type != NULL) {
	if(!strcasecomp(I->type,"password")) {
	    f->type = F_PASSWORD_TYPE;
	} else if(!strcasecomp(I->type,"checkbox")) {
	    f->type = F_CHECKBOX_TYPE;
	} else if(!strcasecomp(I->type,"radio")) {
	    f->type = F_RADIO_TYPE;
	} else if(!strcasecomp(I->type,"submit")) {
	    f->type = F_SUBMIT_TYPE;
	} else if(!strcasecomp(I->type,"image")) {
	    /*
	     * Ugh, we have a clickable image submit button.
	     * Set the type to submit, and fake an ALT string.
	     * If the user activates it, we'll send a 0,0
	     * coordinate pair, which typically will return
	     * the image's default. FM
	     */
	    f->type = F_SUBMIT_TYPE;
	    StrAllocCopy(f->value, "[IMAGE]-Submit");
	} else if(!strcasecomp(I->type,"reset")) {
	    f->type = F_RESET_TYPE;
	} else if(!strcasecomp(I->type,"OPTION_LIST")) {
	    f->type = F_OPTION_LIST_TYPE;
	} else if(!strcasecomp(I->type,"hidden")) {
	   f->type = F_HIDDEN_TYPE;
	   f->size=0;
	} else if(!strcasecomp(I->type,"textarea")) {
	   f->type = F_TEXTAREA_TYPE;
	} else {
	    f->type = F_TEXT_TYPE; /* default */
	}
    } else {
	f->type = F_TEXT_TYPE;
    }


	/* set NAME */
    if(I->name!=NULL) {
        StrAllocCopy(f->name,I->name);

    } else {
	if(f->type==F_RESET_TYPE || f->type==F_SUBMIT_TYPE) {
		/* set name to empty string */
	    StrAllocCopy(f->name,"");
	} else {
		/* error! name must be present */

	    if(a)
	        free(a);
	    if(f)
	        free(f);
	    return(0);
	}
    }

	/* set VALUE */
    /* set the value if it exists */
    if(I->value != NULL)	{
	/*
	 *	OPTION VALUE is not actually the value to be seen but is to
	 *		be sent....
	 */
	if(f->type != F_OPTION_LIST_TYPE && f->type != F_CHECKBOX_TYPE)	{
		StrAllocCopy(f->value, I->value);
	}
	else	{
		/*
		 *	Fill both with the value.  The f->value may be
		 *	overwritten in HText_setLastOptionValue....
		 */
		if(!f->value || strcmp(f->value, "[IMAGE]-Submit"))
		    StrAllocCopy(f->value, I->value);
		StrAllocCopy(f->cp_submit_value, I->value);
	}
    }
    else
        if(!f->value || strcmp(f->value, "[IMAGE]-Submit"))
	    StrAllocCopy(f->value, "");

	/* run checks and fill in neccessary values */
    if(f->type==F_RESET_TYPE) {
	if(I->value!=NULL) {
	    f->size = strlen(I->value);
	} else {
	    StrAllocCopy(f->value, "Reset");
	    f->size = 5;
	}
    } else if(f->type==F_SUBMIT_TYPE) {
        if(f->value && !strcmp(f->value, "[IMAGE]-Submit")) {
	    f->size = strlen(f->value);
	} else if(I->value!=NULL) {
	    f->size = strlen(I->value);
	} else {
	    StrAllocCopy(f->value, "Submit");
	    f->size = 6;
	}
	f->submit_action = NULL;
	StrAllocCopy(f->submit_action, HTFormAction);
	f->submit_method = HTFormMethod;

    } else if(f->type==F_RADIO_TYPE || f->type==F_CHECKBOX_TYPE ) {
	f->size=3;
	if(I->value == NULL)
	   StrAllocCopy(f->value,"on");

    } 

    
    /* set original values */
    if(f->type==F_RADIO_TYPE || f->type==F_CHECKBOX_TYPE ) {
	if(f->num_value)
            StrAllocCopy(f->orig_value, "1");
	else
            StrAllocCopy(f->orig_value, "0");
    } else if(f->type==F_OPTION_LIST_TYPE) {
	f->orig_value=0;
    } else {
        StrAllocCopy(f->orig_value, f->value);
    }

    /* restrict size to maximum allowable size */
    if(f->size > LYcols-10)
	f->size = LYcols-10;  /* maximum */

    /* add this anchor to the anchor list */
    text->last_anchor = a;

	/* return the size of the input field */
    return(f->size);
}


PUBLIC void HText_SubmitForm ARGS4(FormInfo *,submit_item, document *,doc,
				   char *,link_name, char *, link_value)
{
   TextAnchor * anchor_ptr = HTMainText->first_anchor;
   int form_number = submit_item->number;
   FormInfo * form_ptr;
   int len;
   char *query=0;
   char *escaped1=NULL, *escaped2=NULL;
   int first_one=1;
   char * last_textarea_name=0;

   if(submit_item->submit_action)
        len = strlen(submit_item->submit_action) + 2048; /* plus breathing room */
   else
	return;

   /* go through list of anchors and get size first */
   while(1) {
        if(anchor_ptr->link_type == INPUT_ANCHOR) {
   	    if(anchor_ptr->input_field->number == form_number) {

	        form_ptr = anchor_ptr->input_field;
	
	        len += strlen(form_ptr->name)+10;
		/*
		 *	Calculate by the option submit value if present.
		 */
		if(form_ptr->cp_submit_value != NULL)	{
			len += strlen(form_ptr->cp_submit_value) + 10;
		}
		else	{
	        	len += strlen(form_ptr->value)+10;
		}
	        len += 32; /* plus and ampersand + safty net */

	    } else if(anchor_ptr->input_field->number > form_number) {
	        break;
	    }
	}

	if(anchor_ptr == HTMainText->last_anchor)
	    break;

	anchor_ptr = anchor_ptr->next;
   }

   /* get query ready */
   query = (char *)calloc (sizeof(char), len);

   if(submit_item->submit_method == URL_GET_METHOD) {
       	strcpy (query, submit_item->submit_action);
       	/* Clip out anchor. */
       	strtok (query, "#");
       	/* Clip out old query. */
       	strtok (query, "?");  
	strcat(query,"?");  /* add the question mark */
   } else {
	StrAllocCopy(doc->post_content_type, 
					"application/x-www-form-urlencoded");
        query[0] = '\0';
   }

   /* reset anchor->ptr */
   anchor_ptr = HTMainText->first_anchor;
   /* go through list of anchors and assemble URL query */
   while(1) {
        if(anchor_ptr->link_type == INPUT_ANCHOR) {
	    if(anchor_ptr->input_field->number == form_number) {

                form_ptr = anchor_ptr->input_field;

		switch(form_ptr->type) {

	        case F_RESET_TYPE:
		    break;

	        case F_SUBMIT_TYPE:
		    /*
		     *  If it has a non-zero length name (e.g., because
		     *  it's really a type="image" that's been converted
		     *  to SUBMIT_TYPE, or one of multiple submit buttons),
		     *  include the name=value pair. - FM
		     */
		    if((form_ptr->name && *form_ptr->name != '\0' &&
		        !strcmp(form_ptr->name, link_name)) &&
		       form_ptr->value && *form_ptr->value != '\0' &&
		       !strcmp(form_ptr->value, link_value)) {
		        if(first_one)
                            first_one=FALSE;
                        else
                            strcat(query,"&");

		        escaped1 = HTEscape(form_ptr->name,URL_XALPHAS);

			/*
		         * Be sure to actually look at the option submit value.
		         */
		        if(form_ptr->cp_submit_value != NULL)	{
		    	    escaped2 = HTEscape(form_ptr->cp_submit_value,
				    URL_XALPHAS);
		        }
		        else {
			    escaped2 = HTEscape(form_ptr->value, URL_XALPHAS);
		        }

			if (!strcmp(form_ptr->value, "[IMAGE]-Submit"))
			    /*
			     * It's a clickable image submit button.
			     * Fake a 0,0 coordinate pair, which
			     * typically returns the image's default. - FM
			     */
			    sprintf(&query[strlen(query)],
				    "%s.x=0&%s.y=0", escaped1, escaped1);
			else
			    /*
			     * It's a standard submit button.
			     * Use the name=value pair. = FM
			     */
			    sprintf(&query[strlen(query)],
				    "%s=%s", escaped1, escaped2);
			free(escaped1);
			free(escaped2);
		    }
		    break;

		case F_RADIO_TYPE:
		case F_CHECKBOX_TYPE:
		    /* only add if selected */
		    if(form_ptr->num_value) {
	                if(first_one)
		            first_one=FALSE;
	                else
		            strcat(query,"&");

		        escaped1 = HTEscape(form_ptr->name,URL_XALPHAS);
			/*
			 *	Be sure to use the submit option value.
			 */
			if(form_ptr->cp_submit_value != NULL) {
				escaped2 = HTEscape(form_ptr->cp_submit_value,
					URL_XALPHAS);
			}
			else	{
		        	escaped2 = HTEscape(form_ptr->value,
					URL_XALPHAS);
			}

                        sprintf(&query[strlen(query)], "%s=%s",
					        escaped1, escaped2);
			free(escaped1);
		        free(escaped2);
		    }
		    break;
		
		case F_TEXTAREA_TYPE:

                    escaped2 = HTEscape(form_ptr->value,URL_XALPHAS);

		    if(!last_textarea_name || 
			  strcmp(last_textarea_name, form_ptr->name))
		      {
			/* names are different so this is the first
			 * textarea or a different one from any before
			 * it.
			 */
		        if(first_one)
                            first_one=FALSE;
                        else
                            strcat(query,"&");
                        escaped1 = HTEscape(form_ptr->name,URL_XALPHAS);
                        sprintf(&query[strlen(query)], "%s=%s",
					    escaped1, escaped2);
                        free(escaped1);
			last_textarea_name = form_ptr->name;
		      }
		    else
		      {
			/* this is a continuation of a previous textarea
			 * add %0a (\n) and the escaped string
			 */
			if(escaped2[0] != '\0')
			    sprintf(&query[strlen(query)], "%%0a%s", escaped2);
		      }
                    free(escaped2);
                    break;

                case F_PASSWORD_TYPE:
	        case F_TEXT_TYPE:
		case F_OPTION_LIST_TYPE:
		case F_HIDDEN_TYPE:
	            if(first_one)
		        first_one=FALSE;
	            else
			strcat(query,"&");
    
		    escaped1 = HTEscape(form_ptr->name,URL_XALPHAS);

		    /*
		     *	Be sure to actually look at the option submit value.
		     */
		    if(form_ptr->cp_submit_value != NULL)	{
		    	escaped2 = HTEscape(form_ptr->cp_submit_value,
				URL_XALPHAS);
		    }
		    else	{
			escaped2 = HTEscape(form_ptr->value, URL_XALPHAS);
		    }

		    sprintf(&query[strlen(query)], "%s=%s",
					    escaped1, escaped2);
		    free(escaped1);
		    free(escaped2);
		    break;
	        }
	    } else if(anchor_ptr->input_field->number > form_number) {
	        break;
	    }
        }

        if(anchor_ptr == HTMainText->last_anchor)
            break;

	anchor_ptr = anchor_ptr->next;
   }

#if 0
    if (submit_item->submit_method == URL_MAIL_METHOD) {
	if (strncmp(submit_item->submit_action, "mailto:", 7)) {
	    HTAlert("BAD_FORM_MAILTO");
	    free(query);
	    return;
	}
	_user_message("submitting %s", submit_item->submit_action);

	sleep(sleep_three);
	mailform((submit_item->submit_action+7),
#ifdef FIXME
		 ((submit_item->submit_title &&
		   *submit_item->submit_title) ?
		   (submit_item->submit_title) :
#endif
			     (HText_getTitle() ?
			      HText_getTitle() : ""),query);
	free(query);
	return;
    } else {
	_statusline("SUBMITTING_FORM");
    }
#endif

   if(submit_item->submit_method == URL_POST_METHOD) {
       doc->post_data = query;

       StrAllocCopy(doc->address, submit_item->submit_action);
       return;
   } else { /* GET_METHOD */ 
       StrAllocCopy(doc->address, query);
       free_and_clear(&doc->post_data);
       free_and_clear(&doc->post_content_type);
       return;
   }

}

PUBLIC void HText_DisableCurrentForm NOARGS
{
    TextAnchor * anchor_ptr;

    HTFormDisabled = TRUE;
    if (!HTMainText)
	return;

    /*
     *  Go through list of anchors and set the disabled flag.
     */
    anchor_ptr = HTMainText->first_anchor;
    while (anchor_ptr) {
	if (anchor_ptr->link_type == INPUT_ANCHOR &&
	    anchor_ptr->input_field->number == HTFormNumber) {
	    anchor_ptr->input_field->disabled = TRUE;
	}

	if (anchor_ptr == HTMainText->last_anchor)
	    break;


	anchor_ptr = anchor_ptr->next;
    }

    return;
}


PUBLIC void HText_ResetForm ARGS1(FormInfo *,form)
{
    TextAnchor * anchor_ptr = HTMainText->first_anchor;

    _statusline("resetting form...");

   /* go through list of anchors and reset values */
   while(1) {
        if(anchor_ptr->link_type == INPUT_ANCHOR) {
            if(anchor_ptr->input_field->number == form->number) {

                 if(anchor_ptr->input_field->type == F_RADIO_TYPE ||
                      anchor_ptr->input_field->type == F_CHECKBOX_TYPE) {

		    if(anchor_ptr->input_field->orig_value[0]=='0')
		        anchor_ptr->input_field->num_value = 0;
		    else
		        anchor_ptr->input_field->num_value = 1;
		
		 } else if(anchor_ptr->input_field->type==F_OPTION_LIST_TYPE) {
		    anchor_ptr->input_field->value =
				anchor_ptr->input_field->orig_value;
		    
		    anchor_ptr->input_field->cp_submit_value =
		    		anchor_ptr->input_field->orig_submit_value;

	         } else {
		    StrAllocCopy(anchor_ptr->input_field->value,
					anchor_ptr->input_field->orig_value);
		 }
	     } else if(anchor_ptr->input_field->number > form->number) {
                 break;
	     }

        }

        if(anchor_ptr == HTMainText->last_anchor)
            break;


        anchor_ptr = anchor_ptr->next;
   }


}

PUBLIC void HText_activateRadioButton ARGS1(FormInfo *,form)
{
    TextAnchor * anchor_ptr = HTMainText->first_anchor;
    int form_number = form->number;

    while(1) {
        if(anchor_ptr->link_type == INPUT_ANCHOR &&
                anchor_ptr->input_field->type == F_RADIO_TYPE) {
                    
	    if(anchor_ptr->input_field->number == form_number) {

		    /* if it has the same name and its on */
	         if(!strcmp(anchor_ptr->input_field->name, form->name) &&
		    			anchor_ptr->input_field->num_value) {
		    anchor_ptr->input_field->num_value = 0;
		    break;
	         }
	    } else if(anchor_ptr->input_field->number > form_number) {
	            break;
	    }

        }

        if(anchor_ptr == HTMainText->last_anchor)
            break;

        anchor_ptr = anchor_ptr->next;
   }

   form->num_value = 1;
}

PUBLIC void HText_appendMHChar ARGS2(HText *, text, char, c)	{
	/*
	 *	Just return as not supported as of yet.
	 *	Also need code to submit the form correctly via the
	 *		mailto URL.
	 */
	return;
}

static void free_all_texts NOARGS	{
/*
 *	Purpose:	Free all currently loaded HText objects in memory.
 *	Arguments:	void
 *	Return Value:	void
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Usage of this function should really be limited to program
 *			termination.
 *	Revision History:
 *		05-27-94	created Lynx 2-3-1 Garrett Arch Blythe
 */

	if (!loaded_texts)
	    return;

	/*
	 *	Simply loop through the loaded texts list killing them off.
 	 */
	while(loaded_texts && !HTList_isEmpty(loaded_texts))	{
		HText_free((HText *)HTList_removeLastObject(loaded_texts));
	}

	/*
	 *	Get rid of the text list.
 	 */
	if(loaded_texts)	{
		HTList_delete(loaded_texts);
	}

	return;
}
