#include <unistd.h>

/*  #include "spawno.h" */
#include "HTUtils.h"
#include "LYCurses.h"
#include "HTAccess.h"
#include "HTParse.h"
#include "GridText.h"
#include "LYGlobalDefs.h"
#include "LYUtils.h"
#include "GridText.h"
#include "LYString.h"
#include "LYOption.h"
#include "LYSignal.h"
#include "LYGetFil.h"
#include "HTForms.h"
#include "LYSearch.h"
#include "LYClean.h"
#include "LYHistor.h"
#include "LYPrint.h"
// #include "LYMail.h"
#include "LYEdit.h"
#include "LYShowIn.h"
#include "LYBookmark.h"
#include "LYSystem.h"
#include "LYKeymap.h"
#include "LYJump.h"
#include "LYDownlo.h"

#ifdef DIRED_SUPPORT
#include "LYLocal.h"
#include "LYUpload.h"
#endif /* DIRED_SUPPORT */

#include "LYexit.h"
#include "LYLeaks.h"

extern BOOL reloading;    /* For Flushing Cache on Proxy Server */

int already = 1;

PRIVATE int are_different PARAMS((document *doc1, document *doc2));

/*
 * here's where we do all the work
 * mainloop is basically just a big switch dependent on the users input
 * I have tried to offload most of the work done here to procedures to
 * make it more modular, but this procedure still does alot of variable
 * manipulation.  This need some work to make it neater.
 */

void mainloop NOARGS
{
    int tempx = 0;
    extern lynx_extern_item_type *auto_externs;

    int  c, real_c, old_c = 0, cmd, arrowup=FALSE, show_help=FALSE;
    int lines_in_file= -1;
    int newline;
    char prev_target[512];
    char user_input_buffer[1024];
    char *owner_address;  /* holds the responsible owner's address */
    document newdoc, curdoc;
    BOOLEAN first_file=TRUE;
    BOOLEAN refresh_screen=FALSE;
    BOOLEAN force_load = FALSE;

#ifdef DIRED_SUPPORT
   char *cp,*tp;
   char tmpbuf[1024];
   struct stat dir_info;
   taglink *t1,*t2;
#endif /* DIRED_SUPPORT */

/*
 *  curdoc.address contains the name of the file that is currently open
 *  newdoc.address contains the name of the file that will soon be
 *		   opened if it exits
 *  prev_target contains the last search string the user searched for
 *  newdoc.title contains the link name that the user last chose to get into
 *		   the current link (file)
 */
      /* initalize some variables*/
    nhist = 0;
    newdoc.address=0;
    curdoc.address=0;
    StrAllocCopy(newdoc.address, startfile);
    newdoc.title=0;
    curdoc.title=0;
    StrAllocCopy(newdoc.title, "Entry into main screen");
    newdoc.post_data=0;
    curdoc.post_data=0;
    newdoc.post_content_type=0;
    curdoc.post_content_type=0;
    newdoc.line=1;
    newdoc.link=0;
    *prev_target='\0';
    *user_input_buffer='\0';

#ifdef DTx
    if(TRACE)
	fprintf(stderr,"Entering mainloop, startfile=%s\n",startfile);
#endif


    if (form_post_data) {
	StrAllocCopy(newdoc.post_data, form_post_data);
	StrAllocCopy(newdoc.post_content_type, "application/x-www-form-urlencoded");
    } else if (form_get_data) {
	StrAllocCat(newdoc.address, form_get_data);
    }


    if(user_mode==NOVICE_MODE)
	display_lines = LYlines-4;
    else
	display_lines = LYlines-2;

    while (TRUE) {
	/* if newdoc.address is different then curdoc.address then we need
	 * to go out and find and load newdoc.address
	 */
	if (LYforce_no_cache ||force_load ||are_different(&curdoc, &newdoc)) {

		force_load = FALSE;  /* done */
try_again:

		/* push the old file onto the history stack
		 */
		if(curdoc.address && newdoc.address) {
		    push(&curdoc);

		} else if(!newdoc.address) {
		    /* if newdoc.address is empty then pop a file
		     * and load it
		     */
		    pop(&newdoc);
		}

		switch(getfile(&newdoc)) {


		case NOT_FOUND:

			   /* OK! can't find the file */
			   /* so it must not be around now.  Print an error */
		   /* statusline(LINK_NOT_FOUND); don't print error */
		   /* sleep(sleep_one);  dont pause for awhile */

#ifdef FIXME
		   if(error_logging && !first_file && owner_address)
			/* send an error message */
		      mailmsg(curdoc.link, owner_address,
			history[nhist-1].address, history[nhist-1].title);
#endif

			/* do the NULL stuff also and reload the old file */

		   /* the first file wasn't found or has gone missing */
		   if(!nhist) {
			/* if nhist = 0 then it must be the first file */
			if (!dump_output_immediately)
			    cleanup();
			printf("\nlynx: Can't access start file %s\n",
								  startfile);
			exit(1);
		    }

		case NULLFILE:  /* not supposed to return any file */

		    newdoc.address = 0; /* to pop last doc */
		    LYJumpFileURL = FALSE;
		    reloading = FALSE;
		   /* the first file wasn't found or has gone missing */
		   if(!nhist) {
		       /* if nhist = 0 then it must be the first file */
		       if (first_file && homepage &&
#ifdef VMS
			   strcasecomp(homepage, startfile) != 0) {
#else
			   strcmp(homepage, startfile) != 0) {
#endif /* VMS */
			   /*
			    * Couldn't return to the first file but there is a
			    * homepage we can use instead. Useful for when the
			    * first URL causes a program to be invoked. - GL
			    *
			    * But first make sure homepage is different from
			    * startfile (above), then make it the same (below)
			    * so we don't enter an infinite getfile() loop on
			    * on failures to find the files. - FM
			    */
			   StrAllocCopy(newdoc.address, homepage);
			   free_and_clear(&newdoc.post_data);
			   free_and_clear(&newdoc.post_content_type);
			   StrAllocCopy(startfile, homepage);
		       } else {
			   cleanup();
			   printf(
 "\nlynx: Start file could not be found or is not text/html or text/plain\n");
			   printf("      Exiting...\n");
			   exit(1);
		       }
		    }

		    goto try_again;
		    break;

		case NORMAL:
		    *prev_target = '\0'; /* Reset for this document. -FM */
#ifdef TRAVERSAL
		   /* during traversal build up a list of all links
		    * traversed.  Traversal mode is a special
		    * feature to traverse every link in the web
		    */
		   add_to_traverse_list(curdoc.address, curdoc.title);
#endif /* TRAVERSAL */
		   curdoc.line = -1;

		   /*
            	    * set newline to the saved line
            	    * savline contains the line the
            	    * user was on if he has been in
            	    * the file before or it is 1
            	    * if this is a new file
		    */
                   newline = newdoc.line;
			/* if we are going to a target line override
			 * the www_search line result
			 */
		   if(newline > 1)
			www_search_result = -1;

	  	   break;	
		}  /* end switch */

#ifdef DTx
	   if(TRACE)
	      sleep(sleep_two); /* allow me to look at the results */
#endif


	   /* set the files the same */
	   StrAllocCopy(curdoc.address, newdoc.address);
	   StrAllocCopy(curdoc.post_data, newdoc.post_data);
	   StrAllocCopy(curdoc.post_content_type, newdoc.post_content_type);

	   /* reset WWW present mode so that if we were getting
	    * the source, we get rendered HTML from now on
	    */
	   HTOutputFormat = WWW_PRESENT;
	   LYUserSpecifiedURL = FALSE;	/* only set for goto's */
	   LYJumpFileURL = FALSE;	/* only set for jump's */
	   reloading = FALSE;		/* only set for RELOAD and RESUBMIT */

  	} /* end if(STREQ(newdoc.address,curdoc.address) */

        if(dump_output_immediately) {
            print_wwwfile_to_fd(stdout,0);
	    return;
	}

#ifdef FIXME

	/* if the resent_sizechange variable is set to true
	   then the window size changed recently. 
	*/
	if(recent_sizechange) {
		stop_curses();
		start_curses();
		clear();
		refresh_screen = TRUE; /*to force a redraw */
		recent_sizechange=FALSE;
		display_lines = LYlines-2;
	}
#endif

        if(www_search_result != -1) {
	     /* This was a WWW search, set the line
              * to the result of the search
              */
             newline = www_search_result;
	     www_search_result = -1;  /* reset */
	     more = HText_canScrollDown();
        }


	/* if the curdoc.line is different than newline then there must
	 * have been a change since last update. Run showpage.
	 * showpage will put a fresh screen of text out.
	 * If this is a WWW document then use the
	 * WWW routine HText_pageDisplay to put the page on
	 * the screen
         */
	if (curdoc.line != newline) {

   	    refresh_screen = FALSE;

	    HText_pageDisplay(newline, prev_target);

#ifdef DIRED_SUPPORT
	    if(lynx_edit_mode && nlinks && tagged != NULL)
	      showtags(tagged);
#endif /* DIRED_SUPPORT */
	    /* if more equals true then there is more
	     * info below this page 
	     */
	    more = HText_canScrollDown();
	    curdoc.line = newline = HText_getTopOfScreen()+1;
            lines_in_file = HText_getNumOfLines();

	    if(HText_getTitle()) {
	        StrAllocCopy(curdoc.title, HText_getTitle());
	    } else {
	        StrAllocCopy(curdoc.title, newdoc.title);
	    }
	   owner_address = HText_getOwner();

	   if (arrowup) { 
		/* arrow up is set if we just came up from
		 * a page below 
		 */
	        curdoc.link = nlinks - 1;
		arrowup = FALSE;
	   } else {
	        curdoc.link = newdoc.link;
		if(curdoc.link >= nlinks)
	            curdoc.link = nlinks - 1;
	   }

	   show_help = FALSE; /* reset */
	   newdoc.link = 0;
	   newdoc.line = 1;
	   curdoc.line = newline; /* set */
	}

	/* refesh the screen if neccessary */
	if(refresh_screen) {
	    clear();
	    HText_pageDisplay(newline, prev_target);

#ifdef DIRED_SUPPORT
	    if(lynx_edit_mode && nlinks && tagged != NULL)
	      showtags(tagged);
#endif /* DIRED_SUPPORT */
	    if(user_mode == NOVICE_MODE)
		noviceline(more);  /* print help message */
	    refresh_screen=FALSE;

	}

	/* report unread or new mail, if appropriate */
	if (check_mail && !no_mail && LYCheckMail())
	    sleep(sleep_two);

	if(first_file == TRUE) { /* we can never again have the first file */
	    first_file = FALSE; 
	}

	/* if help is not on the screen 
	 * then put a message on the screen
	 * to tell the user other misc info
	 */
	if (!show_help) {
	    /* make sure form novice lines are replaced */
	    if(user_mode == NOVICE_MODE) {
		noviceline(more);
	    }

	    /* if we are in forms mode then explicitly
	     * tell the user what each kind of link is
	     */
	    if(HTisDocumentSource()) {
		/* currently displaying HTML source */
		_statusline(SOURCE_HELP);

	    } else if(lynx_mode==FORMS_LYNX_MODE) {
                if(links[curdoc.link].type == WWW_FORM_LINK_TYPE)
		    switch(links[curdoc.link].form->type) {
                    case F_PASSWORD_TYPE:
                        statusline(FORM_LINK_PASSWORD_MESSAGE);
		        break;
		    case F_OPTION_LIST_TYPE:
                        statusline(FORM_LINK_OPTION_LIST_MESSAGE);
			break;
                    case F_CHECKBOX_TYPE:
		    case F_RADIO_TYPE:
                        statusline(FORM_LINK_CHECKBOX_MESSAGE);
		        break;
                    case F_SUBMIT_TYPE:
                        statusline(FORM_LINK_SUBMIT_MESSAGE);
		        break;
                    case F_RESET_TYPE:
                        statusline(FORM_LINK_RESET_MESSAGE);
		        break;
                    case F_TEXT_TYPE:
		    case F_TEXTAREA_TYPE:
                        statusline(FORM_LINK_TEXT_MESSAGE);
		        break;
		    }
		else
	            statusline(NORMAL_LINK_MESSAGE);

		/* let them know if it's an index -- very rare*/
		if(is_www_index) {
		    move(LYlines-1,LYcols-8);
		    start_reverse();
		    addstr("-index-");
		    stop_reverse();
		}
			
	    } else if(user_mode == ADVANCED_MODE && nlinks>0) {
		/* show the URL */
		if(more)
		    if(is_www_index)
		        _user_message("-more- -index- %s",
						 links[curdoc.link].lname);
		    else
		        _user_message("-more- %s",links[curdoc.link].lname);
		else
		    if(is_www_index)
		        _user_message("-index- %s",links[curdoc.link].lname);
		    else
		        statusline(links[curdoc.link].lname);
	    } else if(is_www_index && more) {
		char buf[128];

		sprintf(buf, WWW_INDEX_MORE_MESSAGE, key_for_func(LYK_INDEX_SEARCH));
		_statusline(buf);
	    } else if(is_www_index) {
		char buf[128];

		sprintf(buf, WWW_INDEX_MESSAGE, key_for_func(LYK_INDEX_SEARCH));
		_statusline(buf);
	    } else if (more) {
		if(user_mode == NOVICE_MODE)
			_statusline(MORE);
		else
			_statusline(MOREHELP);
	    } else {
	       _statusline(HELP);
   	    }	     
	} else {
	   show_help = FALSE;
	}

	highlight(ON, curdoc.link);	/* highlight current link */

	if(nlinks > 0 && links[curdoc.link].type == WWW_FORM_LINK_TYPE &&
			(links[curdoc.link].form->type == F_TEXT_TYPE ||
			 links[curdoc.link].form->type == F_PASSWORD_TYPE ||
			 links[curdoc.link].form->type == F_TEXTAREA_TYPE))
	  {
	    /* replace novice lines if in NOVICE_MODE */
	    if(user_mode==NOVICE_MODE)
	     {
		move(LYlines-2,0); clrtoeol();
		addstr(FORM_NOVICELINE_ONE);
		move(LYlines-1,0); clrtoeol();
		addstr(FORM_NOVICELINE_TWO);
	     }
	    c=change_form_link(&links[curdoc.link],
			       FORM_UP, &newdoc, &refresh_screen,
			       links[curdoc.link].form->name,
			       links[curdoc.link].form->value);

	    if(c == '\n' || c == '\r')  /* make return act like tab */
		c = '\t';
	  }
	else	{
	    /* Get a keystroke from the user
	     * Save the last keystroke to avoid redundant error reporting.
	     */
	    real_c = c = LYgetch();   /* get user input */
	    if(old_c != real_c)	{
		old_c = 0;
	    }
	}

new_keyboard_input:  /* a goto point for new input without going
	   * back through the getch() loop
	   */

	cmd=keymap[c+1];  /* add 1 to map EOF to 0 */

#if defined(DIRED_SUPPORT) && defined(OK_OVERRIDE)
	if (lynx_edit_mode && override[c+1] && !no_dired_support)
	  cmd = override[c+1];
#endif /* DIRED_SUPPORT && OK_OVERRIDE */

new_cmd:  /* a goto point for new input without going
	   * back through the getch() loop
	   */

	switch(cmd) {
	case 0: /* unmapped character */
	default:
	    if (more)
		_statusline(MOREHELP);
	    else
		_statusline(HELP);
	    show_help = TRUE;

#ifdef DTx
	    if(TRACE)
		printw("%d", c);  /* show the user input */
#endif

	    break;

	case LYK_INTERRUPT:
	    /* No network transmission to interrupt - 'til we multithread */
	    break;

	case LYK_1:
	case LYK_2:
	case LYK_3:
	case LYK_4:
	case LYK_5:
	case LYK_6:
	case LYK_7:
	case LYK_8:
	case LYK_9:
	    /* get a number from the user and follow that link number */
	    switch(follow_link_number(c, &curdoc.link)) {
	    case DO_LINK_STUFF:
		    /* follow a normal link */
		StrAllocCopy(newdoc.address, links[curdoc.link].lname);
		/*
		 * Might be an anchor in the same doc from a POST
		 * form.  If so, don't free the content. -- FM
		 */
		if (are_different(&curdoc, &newdoc)) {
		    free_and_clear(&newdoc.post_data);
		    free_and_clear(&newdoc.post_content_type);
		}
		force_load = TRUE;  /* force MainLoop to reload */
		break;

	    case PRINT_ERROR:
		if(old_c == real_c)
			break;
		old_c = real_c;
		_statusline(BAD_LINK_NUM_ENTERED);
		sleep(sleep_two);
		break;
	    }
	    break;

	case LYK_SOURCE:  /* toggle view source mode */
	     if(HTisDocumentSource())
		 HTOutputFormat = WWW_PRESENT;
	     else
		 HTOutputFormat = WWW_SOURCE;
	     HTuncache_current_document();
	     free_and_clear(&curdoc.address); /* so it doesn't get pushed */
	     break;

	case LYK_RELOAD:  /* control-R to reload and refresh */
		/*	Check to see if should reload source, or load html
		 */
		if(HTisDocumentSource())	{
			HTOutputFormat = WWW_SOURCE;
		}
	     HTuncache_current_document();
	     /*
	      * Don't assume the reloaded document will be the same. - FM
	      *//***
	     newdoc.line=curdoc.line;
	     newdoc.link=curdoc.link;
	     ***/
	     newdoc.line=1;
	     newdoc.link=0;
	     free_and_clear(&curdoc.address); /* so it doesn't get pushed */
	     /*
	      * Reload should force a cache refresh on a proxy
	      *        -- Ari L. <luotonen@dxcern.cern.ch>
	      */
	     reloading = TRUE;
	     break;


	case LYK_QUIT:	/* quit */
	    _statusline(QUIT);
	    c = LYgetch();
	    if(TOUPPER(c) != 'N')
		return;
	    else {
		statusline("Excellent!");
		sleep(sleep_one);
	    }
	    break;

	case LYK_ABORT:
	    return;  /* dont ask the user about quitting */
	    break;

	case LYK_NEXT_PAGE:	/* next page */
	    if(more)
		   newline += display_lines;
	    else if(curdoc.link < nlinks-1)
		{
		   highlight(OFF,curdoc.link);
		   curdoc.link = nlinks-1;  /* put on last link */
	        } 
	    else if(old_c != real_c)
		{
		   old_c = real_c;
		   _statusline("You are already at the end of this document.");
                   sleep(already);
		}
	    break;

	case LYK_PREV_PAGE:  /* page up */
	   if(newline > 1) 
	           newline -= display_lines;
	   else if(curdoc.link > 0) 
		{
		   highlight(OFF,curdoc.link);
		   curdoc.link = 0;  /* put on last link */
		}
	    else if(old_c != real_c) 
		{
		   old_c = real_c;
		   _statusline("You are already at the beginning of this document.");
                   sleep(already);
		}
	   break;

	case  LYK_UP_TWO:
	    if(newline > 1) 
	        newline -= 2;
	    else if(old_c != real_c) 
	        {
		   old_c = real_c;
		   _statusline("You are already at the beginning of this document.");
                   sleep(already);
	        }
	    break;

	case  LYK_DOWN_TWO:
	    if(more)
	           newline += 2;
	    else if(old_c != real_c)
		{
		   old_c = real_c;
		   _statusline("You are already at the end of this document.");
                   sleep(already);
		}
	    break;

	case LYK_REFRESH:
	   refresh_screen=TRUE;
	   break;

	case LYK_HOME:
	    if(curdoc.line > 1)
		newline = 1;
	    else {
		cmd = LYK_PREV_PAGE;
		goto new_cmd;
	    }
	    break;

	case LYK_END:
	       newline = 32700; /* go to end of file */
	       arrowup = TRUE;  /* position on last link */
	    break;

	case LYK_PREV_LINK:
	    if (curdoc.link>0) {		/* previous link */
		highlight(OFF, curdoc.link);   /* unhighlight the current link */
		curdoc.link--;

	    } else if(!more && curdoc.link==0 && newline==1) { /* at the top of list */
		/* if there is only one page of data and the user
		 * goes off the top, then just move the cursor to
		 * last link on the page
		 */
		highlight(OFF,curdoc.link); /* unhighlight the current link */
		curdoc.link = nlinks-1;  /* the last link */

	    } else if (curdoc.line > 1) {	/* previous page */
		/* go back to the previous page */
		newline -= (display_lines);
		arrowup = TRUE;

	    } else if(old_c != real_c) {
		old_c = real_c;
		_statusline("You're already at the top of this document");
		sleep(already);
	    }
	    break;

	case LYK_NEXT_LINK:
	    if (curdoc.link<nlinks-1) {		/* next link */
		highlight(OFF, curdoc.link);
		curdoc.link++;
	    /* at the bottom of list and there is only one page
	     * move to the top link on the page
	     */
	    } else if(!more && newline==1 && curdoc.link==nlinks-1) {
		highlight(OFF,curdoc.link);
		curdoc.link = 0;

	    } else if (more) {  /* next page */
		 newline += (display_lines);

	    } else if(old_c != real_c) {
		old_c = real_c;
		_statusline("You're already at the end of this document");
		sleep(already);
	    }
	    break;

	case LYK_UP_LINK:
	    if (curdoc.link>0) {         /* more links? */
		int i, newlink= -1;
		for(i=curdoc.link; i>=0; i--)
		   if(links[i].ly < links[curdoc.link].ly) {
			newlink=i;
			break;
		   }

		if(newlink > -1) {
		    highlight(OFF, curdoc.link);
		    curdoc.link=newlink;
		} else if(!more && newline==1 && curdoc.link==0) {
		    highlight(OFF,curdoc.link);
		    curdoc.link = nlinks-1;
		} else if (more) {  /* next page */
			newline += (display_lines);
		}

	    /* at the bottom of list and there is only one page
	     * move to the top link on the page
	     */
	    } else if(!more && newline==1 && curdoc.link==nlinks-1) {
		highlight(OFF,curdoc.link);
		curdoc.link = 0;

	    } else if (curdoc.line > 1) {  /* next page */
		    newline -= (display_lines);

	    } else if(old_c != real_c) {
		old_c = real_c;
		_statusline("You're already at the top of this document");
		sleep(already);
	    }
	    break;

	case LYK_DOWN_LINK:
	    if (curdoc.link<nlinks-1) {         /* more links? */
		int i, newlink= -1;
		for(i=curdoc.link; i<nlinks; i++)
		   if(links[i].ly > links[curdoc.link].ly) {
			newlink=i;
			break;
		   }

		if(newlink > -1) {
		    highlight(OFF, curdoc.link);
		    curdoc.link=newlink;
		} else if(!more && newline==1 && curdoc.link==nlinks-1) {
		    highlight(OFF,curdoc.link);
		    curdoc.link = 0;
		} else if (more) {  /* next page */
			newline += (display_lines);
		}

	    /* at the bottom of list and there is only one page
	     * move to the top link on the page
	     */
	    } else if(!more && newline==1 && curdoc.link==nlinks-1) {
		highlight(OFF,curdoc.link);
		curdoc.link = 0;

	    } else if (more) {  /* next page */
		    newline += (display_lines);

	    } else if(old_c != real_c) {
		old_c = real_c;
		_statusline("You're already at the bottom of this document");
		sleep(already);
	    }
	    break;

	case LYK_RIGHT_LINK:
	    if (curdoc.link<nlinks-1 &&
			links[curdoc.link].ly == links[curdoc.link+1].ly) {
		highlight(OFF,curdoc.link);
		curdoc.link++;
	    }
	    break;

	case LYK_LEFT_LINK:
	    if (curdoc.link>0 &&
			links[curdoc.link].ly == links[curdoc.link-1].ly) {
		highlight(OFF,curdoc.link);
		curdoc.link--;
	    }
	    break;

	case LYK_HISTORY: 	/* show the history page */
	  if(strcmp(curdoc.title, HISTORY_PAGE_TITLE)) {
		/* don't do if already viewing history page */

		/* push current file so that the history
		 * list contains the current file for printing
		 * purposes.
		 * pop the file afterwards to prevent multiple copies
		 */
		push(&curdoc);

		/* print history options to file */
		showhistory(&newdoc.address);
		free_and_clear(&curdoc.address);  /* so it doesn't get pushed */
		free_and_clear(&newdoc.post_data);
		free_and_clear(&newdoc.post_content_type);

		pop(&curdoc);

		refresh_screen=TRUE;

		break;
	  } /* end if strncmp */

	/* dont put break here so that if the backspace key is pressed in
	 * the history page, then it acts like a left arrow
	 */

	case LYK_PREV_DOC:			/* back up a level */
		if (nhist>0) {  /* if there is anything to go back to */

		    /* set newdoc.address to empty to pop a file */
		    free_and_clear(&newdoc.address);
#ifdef DIRED_SUPPORT
		    if (lynx_edit_mode)
		      HTuncache_current_document();
#endif /* DIRED_SUPPORT */
		} else if(child_lynx==TRUE) {
		   return; /* exit on left arrow in main screen */

		} else if(old_c != real_c) {
		    old_c = real_c;
		    _statusline("Already at the first document");
		    sleep(already);
		}
	     break;

	case LYK_RESUBMIT:	/* Force submission of form to server */
	     if(nlinks > 0) {
		 if (links[curdoc.link].type != WWW_FORM_LINK_TYPE ||
		     links[curdoc.link].form->type != F_SUBMIT_TYPE) {
		     if(old_c == real_c)
			 break;
		     old_c = real_c;
		     _statusline("You are not on a form submission button.");
		     sleep(sleep_two);
		     break;
		 } else {
		     LYforce_no_cache = TRUE;
		     reloading = TRUE;
		 }
	     } /* fall through to LYK_ACTIVATE */

	case LYK_ACTIVATE:			/* follow a link */
	    if (nlinks > 0) {
		if(links[curdoc.link].type == WWW_FORM_LINK_TYPE) {
			/*
			 * Don't try to submit forms with bad actions. - FM
			 */
			if(links[curdoc.link].form->type == F_SUBMIT_TYPE) {
			   /*
			    * Make sure we have an action. - FM
			    */
			   if (!links[curdoc.link].form->submit_action ||
			       *links[curdoc.link].form->submit_action
								== '\0') {
			       _statusline(
				 "** Bad HTML!!  No form action defined. **");
			       sleep(sleep_two);
			       break;
			    }
			    /*
			     * Uncomment the check for no_mail and
			     * re-phrase the statusline message when
			     * the mailto form action is supported. - FM
			     */
			    else if (!strncmp(
				     links[curdoc.link].form->submit_action,
				     "mailto:",7) && no_mail) {
				_statusline(
				     "Mailto form disallowed.");
			       sleep(sleep_two);
			       break;
			    }
			}
			c = change_form_link(&links[curdoc.link],
					     FORM_UP, &newdoc, &refresh_screen,
					     links[curdoc.link].form->name,
					     links[curdoc.link].form->value);
			goto new_keyboard_input;

		} else {   /* Not a forms link */
		    /* follow a normal link or anchor */
		  tempx = 0;

		  if(auto_externs != NULL)
		  {
		    tempx = external2(curdoc.address,links[curdoc.link].type,
		       links[curdoc.link].lname,nlinks);
		  }
		  if(tempx) {
		    refresh_screen=TRUE;  /* for a showpage */
		  } else {

		    StrAllocCopy(newdoc.address, links[curdoc.link].lname);
		    StrAllocCopy(newdoc.title, links[curdoc.link].hightext);

		    /*  Might be an anchor in the same doc from a POST
			form.  If so, dont't free the content. -- FM */

		    if (are_different(&curdoc, &newdoc)) {
			free_and_clear(&newdoc.post_data);
			free_and_clear(&newdoc.post_content_type);
		    }
		    newdoc.link = 0;
		    /*  force MainLoop to reload */
		    force_load = TRUE;
		  }
		}
	    }
	    break;

	case LYK_GOTO:   /* 'g' to goto a random URL  */

	    if(no_goto) {
		if (old_c != real_c) {
		    old_c = real_c;
		    _statusline("Goto a random URL is disallowed!");
		    sleep(sleep_two);
		}
		break;
	    }

//        StrAllocCopy(temp, user_input_buffer);

//        if (!goto_buffer)
//             *user_input_buffer = '\0';

        _statusline("URL to open: ");

//        LYgetstr(user_input_buffer, VISIBLE); /* ask the user */

	if ((LYgetstr(user_input_buffer, VISIBLE) < 0 ) || (strlen(user_input_buffer)<1)) {

		/*
		 *  User cancelled the Goto via ^G.
		 *  Restore user_input_buffer and break. - FM
		 */
//                strcpy(user_input_buffer, temp);
//                free(temp);
		_statusline("Goto Cancelled!");
		sleep(sleep_two);
		break;
	}

    /* get rid of leading spaces (and any other spaces) */
    collapse_spaces(user_input_buffer);

    /*
     * If its not a URL then make it one -- a web one. WB
     */
    if(!is_url(user_input_buffer)) {
	    /* rewrite the file as a URL */
	   char *tmpurl = NULL;

	   StrAllocCopy(tmpurl,"http://");
	   StrAllocCat(tmpurl,user_input_buffer);

	   strcpy(user_input_buffer,tmpurl);

	   free(tmpurl);
	   tmpurl=NULL;
    }


	    if(no_file_url && !strncmp(user_input_buffer,"file:",5)) {
		 _statusline("You are not allowed to goto \"file:\" URL's");
		 sleep(sleep_two);

#ifdef EXEC_LINKS
	    } else if((no_shell || local_exec_on_local_files) &&
#else
	    } else if(no_shell &&
#endif
		      !strncmp(user_input_buffer, "lynxexec:",9)) {
		 _statusline("You are not allowed to goto \"lynxexec:\" URL's");
		 sleep(sleep_two);

	    } else if(no_shell && !strncmp(user_input_buffer, "lynxcgi:",8)) {
		 _statusline("You are not allowed to goto \"lynxcgi:\" URL's");
		 sleep(sleep_two);

	    } else if(*user_input_buffer != '\0') {

		/* make a name for this new URL */
		StrAllocCopy(newdoc.title, "A URL specified by the user");

		/*
		 * If it's a file URL and the host is defaulted,
		 * force in "//localhost".  We need this until
		 * all the other Lynx code which performs security
		 * checks based on the "localhost" string is changed
		 * to assume "//localhost" when a host field is not
		 * present in file URLs - FM
		 */
		if(!strncmp(user_input_buffer, "file:", 5)) {
		    char *temp=NULL;
		    if(user_input_buffer[5] == '\0') {
			strcat(user_input_buffer, "//localhost");
		    } else if(!strcmp(user_input_buffer, "file://")) {
			strcat(user_input_buffer, "localhost");
		    } else if(!strncmp(user_input_buffer, "file:///", 8)) {
			StrAllocCopy(temp, (user_input_buffer+7));
			strcpy(user_input_buffer, "file://localhost");
			strcat(user_input_buffer, temp);
			free(temp);
		    } else if(!strncmp(user_input_buffer, "file:/", 6) &&
			      user_input_buffer[6] != '/') {
			StrAllocCopy(temp, (user_input_buffer+5));
			strcpy(user_input_buffer, "file://localhost");
			strcat(user_input_buffer, temp);
			free(temp);
		    }
		}

		/*
		 * No path in a file://localhost URL means a
		 * directory listing for the current default. - FM
		 */
		if(!strcmp(user_input_buffer, "file://localhost")) {
		    char curdir[DIRNAMESIZE];
#ifdef NO_GETCWD
		    getwd (curdir);
#else
		    getcwd (curdir, DIRNAMESIZE);
#endif /* NO_GETCWD */
		    strcat(user_input_buffer, curdir);
		}
       tempx = 0;
       if(auto_externs != NULL)
	  tempx = external2(curdoc.address,links[curdoc.link].type,
	     user_input_buffer,nlinks);

       if (tempx)
	 refresh_screen=TRUE;  /* for a showpage */
       else {
	 StrAllocCopy(newdoc.address, user_input_buffer);
	 free_and_clear(&newdoc.post_data);
	 free_and_clear(&newdoc.post_content_type);
	 force_load = TRUE;
       }

#ifdef DIRED_SUPPORT
		if (lynx_edit_mode)
		  HTuncache_current_document();
#endif /* DIRED_SUPPORT */
	    }
	    break;

	case LYK_HELP:			/* show help file */
	    if(!STREQ(curdoc.address, helpfile)) {
		StrAllocCopy(newdoc.address, helpfile);  /* set the filename */
		/* make a name for this help file */
		StrAllocCopy(newdoc.title, "Help Screen");
		free_and_clear(&newdoc.post_data);
		free_and_clear(&newdoc.post_content_type);
	    }
	    break;

	case LYK_INDEX:  /* index file */
	    /* make sure we are not in the index already */
	    if(!STREQ(curdoc.address, indexfile)) {

		if(indexfile[0]=='\0') { /* no defined index */
			if(old_c != real_c)	{
				old_c = real_c;
				_statusline("No index is currently available");
				sleep(sleep_two);
			}

		} else {
		    StrAllocCopy(newdoc.address, indexfile);
		    StrAllocCopy(newdoc.title, "System Index"); /* name it */
		    free_and_clear(&newdoc.post_data);
		    free_and_clear(&newdoc.post_content_type);
		} /* end else */
	    }  /* end if */
	    break;

	case LYK_MAIN_MENU:	/* return to main screen */
	    /* if its already the homepage then don't reload it */
	    if(!STREQ(curdoc.address,homepage)) {

		_statusline("Do you really want to go to the Main screen? (y/n) [n] ");
		c = LYgetch();
		if(TOUPPER(c)=='Y') {
		    StrAllocCopy(newdoc.address, homepage);
		    StrAllocCopy(newdoc.title, "Entry into main screen");
		    free_and_clear(&newdoc.post_data);
		    free_and_clear(&newdoc.post_content_type);
		    highlight(OFF,curdoc.link);
#ifdef DIRED_SUPPORT
		    if (lynx_edit_mode)
		      HTuncache_current_document();
#endif /* DIRED_SUPPORT */
		}
#ifdef VMS
		if (HadVMSInterrupt)
		    HadVMSInterrupt = FALSE;
#endif /* VMS */
	    } else {
		if(old_c != real_c)	{
			old_c = real_c;
			_statusline("Already at main screen!");
			sleep(already);
		}
	    }
	    break;

	case LYK_OPTIONS:     /* options screen */

#ifdef DIRED_SUPPORT
	    c = dir_list_style;
#endif /* DIRED_SUPPORT */
	    options(); /* do the options stuff */

#ifdef DIRED_SUPPORT
	    if (c != dir_list_style) {
	       HTuncache_current_document();
	       newdoc.address = curdoc.address;
	       curdoc.address = NULL;
	    }
#endif /* DIRED_SUPPORT */
	    refresh_screen = TRUE; /* to repaint screen */
	    break;

	case LYK_INDEX_SEARCH: /* search for a user string */
	     if(is_www_index) {
	       /* perform a database search */

		/* do_www_search will try to go out and get the document
		 * if it returns yes a new document was returned and is
		 * named in the newdoc.address
		 */
		if(do_www_search(&newdoc) == NORMAL) {
		   /* Yah, the search succeeded. */
		   push(&curdoc);
		   /* Make the curdoc.address the newdoc.address so that
		    * getfile doesn't try to get the newdoc.address.
		    * Since we have already gotton it.
		    */
		   StrAllocCopy(curdoc.address, newdoc.address);
		   StrAllocCopy(newdoc.post_data, curdoc.post_data);
		   curdoc.line = -1;
		   newline = 0;
		   refresh_screen = TRUE; /* redisplay it */
		} else if (use_this_url_instead != NULL) {
		   /* Got back a redirecting URL. Check it out. */
		   _user_message("Using %s", use_this_url_instead);
		   /* Make a name for this URL */
		   StrAllocCopy(newdoc.title, "A URL specified by redirection");
		   StrAllocCopy(newdoc.address, use_this_url_instead);
		   free_and_clear(&newdoc.post_data);
		   free_and_clear(&newdoc.post_content_type);
		   free(use_this_url_instead);
		   use_this_url_instead = NULL;
		   force_load = TRUE;
		   break;
		} else {
		   /* Yuk, the search failed.  Restore the old file. */
		   StrAllocCopy(newdoc.address, curdoc.address);
		   StrAllocCopy(newdoc.post_data, curdoc.post_data);
		}
	     } else if(old_c != real_c)	{
		old_c = real_c;
		_statusline("Not a searchable indexed document -- press '/' to search for a text string");
		sleep(sleep_two);
	     }
	     break;

	case LYK_WHEREIS: /* search within the document */
	/* search for the next occurrence of the user string */
	case LYK_NEXT:
	    /* user search */
	    if(cmd != LYK_NEXT) {
		/*
		 * Reset prev_target to force prompting
		 * for a new search string and to turn
		 * off highlighting in no search string
		 * is entered by the user.
		 */
		*prev_target = '\0';
		textsearch(&curdoc, prev_target, FALSE);
	    } else {
		/*
		 * When the third argument is TRUE, the previous
		 * search string, if any, will be recalled from
		 * a buffer, loaded into prev_target, and used
		 * for the search without prompting for a new
		 * search string.  This allows the LYK_NEXT
		 * command to repeat a search in a new document,
		 * after prev_target was reset on fetch of that
		 * document.
		 */
		textsearch(&curdoc, prev_target, TRUE);
	    }

	    /*
	     * Force a redraw to ensure highlighting of hits
	     * even when found on the same page, or clearing
	     * of highlighting is the default search string
	     * was erased without replacement. - FM
	     */
	    refresh_screen = TRUE;
	    break;

	case LYK_COMMENT:  /* reply by mail */
	   if(!owner_address) {
		if(old_c != real_c)	{
			old_c = real_c;
			_statusline("No owner is defined for this file so you cannot send a comment");
			sleep(sleep_two);
		}
	   } else if (no_mail) {
	       if(old_c != real_c) {
		   old_c = real_c;
		   _statusline("Mail is disallowed so you cannot send a comment");
		   sleep(sleep_two);
	       }
	   } else {
		_statusline("Do you wish to send a comment? [N]");
		c = LYgetch();
		if(TOUPPER(c) == 'Y') {

		   if(is_url(owner_address) != MAILTO_URL_TYPE) {
			/* the address is a url */
			/* just follow the link */

		       StrAllocCopy(newdoc.address, owner_address);

		   } else {
#if 0
		     /* the owner_address is a mailto: url type */
		       if(strchr(owner_address,':')!=NULL)
			    /* send a reply. The address is after the colon */
			 reply_by_mail(strchr(owner_address,':')+1,curdoc.address);
		       else
			 reply_by_mail(owner_address,curdoc.address);
#endif
		       refresh_screen=TRUE;  /* to force a showpage */
		  }
	       }
	   }
	   break;

#ifdef DIRED_SUPPORT
	case LYK_TAG_LINK:  /* tag or untag the current link */
	   if(lynx_edit_mode && nlinks && !no_dired_support) {
	      if (dir_list_style == MIXED_STYLE) {
		 if (!strcmp(links[curdoc.link].hightext,"../"))
		   break;
	      } else if (!strncmp(links[curdoc.link].hightext,"Up to ",6))
		 break;
	      t1 = tagged;
	      while(t1 != NULL) {
		 if(!strcmp(links[curdoc.link].lname,t1->name)) {
		    if(t1==tagged)
		      tagged = t1->next;
		    else
		      t2->next = t1->next;
		    free(t1->name);
		    free(t1);
		    tagflag(OFF,curdoc.link);
		    break;
		 }
		 t2 = t1;
		 t1 = t1->next;
	      }
	      if(t1 == NULL) {
		 t1 = (taglink *) malloc(sizeof(taglink));
		 if (tagged == NULL)
		   tagged = t1;
		 else
		   t2->next = t1;
		 t1->next = NULL;
		 t1->name = NULL;
		 StrAllocCopy(t1->name,links[curdoc.link].lname);
		 tagflag(ON,curdoc.link);
	      }
	      if(curdoc.link < nlinks-1) {
		highlight(OFF, curdoc.link);
		curdoc.link++;
	      } else if(!more && newline==1 && curdoc.link==nlinks-1) {
		highlight(OFF,curdoc.link);
		curdoc.link = 0;
	      } else if (more) {  /* next page */
		newline += (display_lines);
	      }
	   }
	   break;

	case LYK_MODIFY:  /* rename a file or directory */
	   if(lynx_edit_mode && nlinks && !no_dired_support) {
	      int ret;

	      ret = local_modify(&curdoc, &newdoc.address);
	      if (ret == PERMIT_FORM_RESULT) {      /* Permit form thrown up */
		 refresh_screen=TRUE;
	      } else if (ret) {
		 HTuncache_current_document();
		 newdoc.address = curdoc.address;
		 curdoc.address = NULL;
		 newdoc.line = curdoc.line;
		 newdoc.link = curdoc.link;
		 clear();
	      }
	   }
	   break;

	case LYK_CREATE:  /* create a new file or directory */
	   if(lynx_edit_mode && !no_dired_support) {
	      if (local_create(&curdoc)) {
		 HTuncache_current_document();
		 newdoc.address = curdoc.address;
		 curdoc.address = NULL;
		 newdoc.line = curdoc.line;
		 newdoc.link = curdoc.link > -1 ? curdoc.link : 0;
		 clear();
	      }
	   }
	   break;
#endif /* DIRED_SUPPORT */

	case LYK_EDIT:  /* edit */

#ifdef DIRED_SUPPORT

/* Allow the user to edit the link rather than curdoc in edit mode */

	   if(lynx_edit_mode && editor && *editor != '\0' &&
			    !no_editor && !no_dired_support) {
	      if (nlinks > 0) {
		 cp = links[curdoc.link].lname;
		 if(is_url(cp) == FILE_URL_TYPE) {
		    tp = cp;
		    if(!strncmp(tp,"file://localhost",16))
		       tp += 16;
		    else if(!strncmp(tp,"file:",5))
		       tp += 5;
		    strcpy(tmpbuf,tp);
		    HTUnEscape(tmpbuf);
		    if(stat(tmpbuf,&dir_info) == -1) {
		       _statusline("System error - failure to get status. ");
		       sleep(sleep_three);
		    } else {
		       if (((dir_info.st_mode) & S_IFMT) == S_IFREG) {
			  strcpy(tmpbuf,cp);
			  HTUnEscape(tmpbuf);
			  if(edit_current_file(tmpbuf,curdoc.link,newline))
			  {
				HTuncache_current_document();
				newdoc.address = curdoc.address;
				curdoc.address = NULL;
				/*	Can't assume the below if a document was
			   	 *	actually modified.  Taking out. GAB
				 *	newdoc.line = curdoc.line;
			   	 *	newdoc.link = curdoc.link;
			   	 */
			  	newdoc.line = 0;
			  	newdoc.link = 0;
			  	clear();  /* clear the screen */
			  }
			}
		    }
		 }
	      }
	   } else
#endif /* DIRED_SUPPORT */
	   if(editor && *editor != '\0' && !no_editor) {
		if(edit_current_file(newdoc.address,curdoc.link,newline))
		{
	            	HTuncache_current_document();
			LYforce_no_cache = TRUE;  /*force the document to be reloaded*/
	        	free_and_clear(&curdoc.address); /* so it doesn't get pushed */
			/*	Can't assume if document actually modified.
			 *	Taking out.  GAB
			 *	newdoc.line = curdoc.line;
			 *	newdoc.link = curdoc.link;
			 */
			newdoc.line = 0;
			newdoc.link = 0;
			clear();  /* clear the screen */
		}

	   } else if(!editor || *editor == '\0') {
		if(old_c != real_c)	{
			old_c = real_c;
			_statusline("No editor is defined!");
			sleep(sleep_two);
		}
	   }
	   break;

	case LYK_DEL_BOOKMARK:  /* delete home page link */

#ifdef DIRED_SUPPORT
	case LYK_REMOVE:  /* remove files and directories */
	   if(lynx_edit_mode && nlinks && !no_dired_support)
		local_remove(&curdoc);
	    else
#endif /* DIRED_SUPPORT */

	    if(bookmark_page && (strstr(curdoc.address, bookmark_page) ||
			!strcmp(curdoc.title, MOSAIC_BOOKMARK_TITLE))) {
		_statusline("Do you really want to delete this link from your bookmark file? (y/n)");
		c = LYgetch();
		if(TOUPPER(c) != 'Y')
		    break;
		remove_bookmark_link(links[curdoc.link].anchor_number-1);
	    } else {	/* behave like REFRESH for backward compatability */
		refresh_screen=TRUE;
		break;
	    }
	    HTuncache_current_document();
	    newdoc.address = curdoc.address;
	    curdoc.address = NULL;
	    newdoc.line = curdoc.line;
	    if(curdoc.link == nlinks-1)
	       /* if we deleted the last link on the page */
	       newdoc.link=curdoc.link-1;
	    else
		newdoc.link=curdoc.link;
	    break;

#ifdef DIRED_SUPPORT
	case LYK_INSTALL:  /* install a file into system area */
	   if(lynx_edit_mode && nlinks && !no_dired_support)
	      local_install(NULL, links[curdoc.link].lname, &newdoc.address);
	   break;
#endif /* DIRED_SUPPORT */

	case LYK_INFO:  /* show document info */
	   /* don't do if already viewing info page */
	   if(strcmp(curdoc.title, SHOWINFO_TITLE)) {
		showinfo(&curdoc, lines_in_file, &newdoc, owner_address);
		LYforce_no_cache = TRUE;
	   } else {
		/* if already in info page; get out */
		cmd = LYK_PREV_DOC;
		goto new_cmd;
	   }
	   break;

	case LYK_PRINT:  /* print the file */
	   /* don't do if already viewing print options page */
	   if(strcmp(curdoc.title, PRINT_OPTIONS_TITLE)) {

		print_options(&newdoc.address, lines_in_file);
		refresh_screen=TRUE;  /* redisplay */
	   }
	   break;

#ifdef DIRED_SUPPORT
       case LYK_DIRED_MENU:  /* provide full file management menu */
	   /* don't do if not allowed or already viewing the menu */
	   if(lynx_edit_mode && !no_dired_support &&
	      strcmp(curdoc.title, DIRED_MENU_TITLE)) {
	      dired_options(&curdoc,&newdoc.address);
	      refresh_screen=TRUE;  /* redisplay */
	   }
	   break;
#endif /* DIRED_SUPPORT */

	case LYK_EXTERN:  /* use external program on url */
	   external(curdoc.address,links[curdoc.link].type,
	     links[curdoc.link].lname,nlinks);
	   refresh_screen=TRUE;  /* for a showpage */
	   break;

	case LYK_ADD_BOOKMARK:  /* a to add link to bookmark file */
	   if(strcmp(curdoc.title, HISTORY_PAGE_TITLE) &&
	      strcmp(curdoc.title, SHOWINFO_TITLE) &&
	      strcmp(curdoc.title, PRINT_OPTIONS_TITLE) &&
#ifdef DIRED_SUPPORT
	      strcmp(curdoc.title, DIRED_MENU_TITLE) &&
	      strcmp(curdoc.title, PERMIT_OPTIONS_TITLE) &&
	      strcmp(curdoc.title, UPLOAD_OPTIONS_TITLE) &&
#endif /* DIRED_SUPPORT */
	      strcmp(curdoc.title, DOWNLOAD_OPTIONS_TITLE)) {

		if(nlinks) {
		    _statusline("Save D)ocument or L)ink to bookmark file or C)ancel? (d,l,c): ");
		    c = LYgetch();
		    if(TOUPPER(c) == 'D')
			save_bookmark_link(curdoc.address, curdoc.title);
		    else if(TOUPPER(c) == 'L')
			if(links[curdoc.link].type != WWW_FORM_LINK_TYPE)
			    save_bookmark_link(links[curdoc.link].lname,
						links[curdoc.link].hightext);
			else
			  {
			    _statusline("Cannot save form fields/links");
			    sleep(sleep_two);
			  }
		} else {
		    _statusline("Save D)ocument to bookmark file or C)ancel? (d,c): ");
		    c = LYgetch();
		    if(TOUPPER(c) == 'D')
			save_bookmark_link(curdoc.address, curdoc.title);
		}
	   } else {
		if(old_c != real_c)	{
			old_c = real_c;
			_statusline("History, showinfo and menu files cannot be saved in the bookmark page.");
			sleep(sleep_two);
		}
	   }

	   break;

	case LYK_VIEW_BOOKMARK:   /* v to view home page */
	    /* see if a home page exists
	     * if it does replace newdoc.address with it's name
	     */
	    if(get_bookmark_filename(&newdoc.address) != NULL) {
		LYforce_HTML_mode = TRUE;  /* force HTML */
		LYforce_no_cache = TRUE;  /*force the document to be reloaded*/
		StrAllocCopy(newdoc.title, "Bookmark File");
		free_and_clear(&newdoc.post_data);
		free_and_clear(&newdoc.post_content_type);
	    } else {
		if(old_c != real_c)	{
			old_c = real_c;
			_statusline("Unable to open Home page, use 'a' to save a link first");
			sleep(sleep_two);
		}
	    }
	    break;

	/* shell escape */
	case LYK_SHELL:
	    if(!no_shell) {
		stop_curses();
		system("cls");
		system("echo Type EXIT to return to Lynx.");
		system(getenv("COMSPEC") == NULL ? "command.com" :
			getenv("COMSPEC"));
		start_curses();
		refresh_screen=TRUE;
	    } else {
		if(old_c != real_c)	{
			old_c = real_c;
			_statusline("The (!) command is currently disabled");
			sleep(sleep_two);
		}
	    }
	    break;

	case LYK_DOWNLOAD:
	    /* don't do if both download and disk_save are restricted */
	    if (no_download && no_disk_save) {
		if(old_c != real_c)	{
		    old_c = real_c;
		    _statusline(
			"The 'd'ownload command is currently disabled.");
		    sleep(sleep_two);
		}
		break;
	    }

	    /* don't do if already viewing download options page */
	    if(0==strcmp(curdoc.title, DOWNLOAD_OPTIONS_TITLE))
		break;

	    if (nlinks > 0) {
		if(links[curdoc.link].type == WWW_FORM_LINK_TYPE) {
		    if(old_c != real_c)	{
			old_c = real_c;
			_statusline("You cannot download a input field.");
			sleep(sleep_two);
		    }

		} else if (0==strcmp(curdoc.title, PRINT_OPTIONS_TITLE)) {
		    if(old_c != real_c)	{
			old_c = real_c;
			_statusline("You cannot download a printing option.");
			sleep(sleep_two);
		    }

#ifdef DIRED_SUPPORT
		} else if (0==strcmp(curdoc.title, UPLOAD_OPTIONS_TITLE)) {
		    if(old_c != real_c)	{
			old_c = real_c;
			_statusline("You cannot download an upload option.");
			sleep(sleep_two);
		    }

		} else if (0==strcmp(curdoc.title, PERMIT_OPTIONS_TITLE)) {
		    if(old_c != real_c)	{
			old_c = real_c;
			_statusline("You cannot download an permit option.");
			sleep(sleep_two);
		    }

		} else if (lynx_edit_mode && !no_dired_support) {
		    /* Don't bother making a /tmp copy of the local file */
                    StrAllocCopy(newdoc.address, links[curdoc.link].lname);
		    LYdownload_options(&newdoc.address,
		    		       links[curdoc.link].lname);
#endif /* DIRED_SUPPORT */

		} else if (0==strcmp(curdoc.title, HISTORY_PAGE_TITLE)) {
		    int number = atoi(links[curdoc.link].lname+9);
		    StrAllocCopy(newdoc.address, history[number].address);
                    StrAllocCopy(newdoc.title, links[curdoc.link].hightext);
		    free_and_clear(&newdoc.post_data);
		    free_and_clear(&newdoc.post_content_type);
		    if (history[number].post_data)
			StrAllocCopy(newdoc.post_data,
				     history[number].post_data);
		    if (history[number].post_content_type)
		        StrAllocCopy(newdoc.post_content_type,
				     history[number].post_content_type);
                    newdoc.link = 0;
		    HTOutputFormat = HTAtom_for("www/download");
		    /*force the document to be reloaded*/
		    LYforce_no_cache = TRUE;
		    force_load = TRUE;  /* force MainLoop to reload */
		    
		} else {   /* Not a forms, options or history link */
                    /*
		     * Follow a normal link or anchor.  Note that
		     * if it's an anchor within the same document,
		     * entire document will be downloaded
		     */
                    StrAllocCopy(newdoc.address, links[curdoc.link].lname);
                    StrAllocCopy(newdoc.title, links[curdoc.link].hightext);
		    /*
		     * Might be an anchor in the same doc from a POST
		     * form.  If so, don't free the content. -- FM
		     */
		    if (are_different(&curdoc, &newdoc)) {
			free_and_clear(&newdoc.post_data);
			free_and_clear(&newdoc.post_content_type);
		    }
                    newdoc.link = 0;
	            HTOutputFormat = HTAtom_for("www/download");
		    /*force the document to be reloaded*/
		    LYforce_no_cache = TRUE;
		    force_load = TRUE;  /* force MainLoop to reload */
		}
            } else if(old_c != real_c)	{
		old_c = real_c;
		_statusline("Nothing to download.");
		sleep(sleep_two);
	    }
	    break;

#ifdef DIRED_SUPPORT
	  case LYK_UPLOAD:
	    /* don't do if already viewing upload options page */	
	    if(0==strcmp(curdoc.title, UPLOAD_OPTIONS_TITLE))
	        break;

	    if (lynx_edit_mode && !no_dired_support) {
		LYUpload_options((char **)&newdoc.address,
				 (char *)curdoc.address);
	     }
	    break;
#endif /* DIRED_SUPPORT */

#ifdef DT
	case LYK_TRACE_TOGGLE:
	    if(user_mode == ADVANCED_MODE) {
		if(WWW_TraceFlag)
		    WWW_TraceFlag = FALSE;
		else
		    WWW_TraceFlag = TRUE;

		_statusline(WWW_TraceFlag ? "Trace ON!" : "Trace OFF!");
		sleep(sleep_one);
	    }
		break;
#endif

        case LYK_IMAGE_TOGGLE:
	    if (clickable_images)
		clickable_images = FALSE;
            else
		clickable_images = TRUE;
	    _statusline(clickable_images ?
		"Clickable Images ON.  Reloading." : "Clickable Images OFF.  Reloading.");
	    sleep(sleep_two);
            cmd = LYK_RELOAD;
            goto new_cmd;
            break;

	case LYK_DO_NOTHING:
	    /* pretty self explanitory */
	    break;

	case LYK_TOGGLE_HELP:
	    if(user_mode==NOVICE_MODE) {
		toggle_novice_line();
		noviceline(more);
	    }
	    break;

	case LYK_KEYMAP:
	    if(old_c != real_c) {
		old_c = real_c;
		print_keymap(&newdoc.address);
		LYforce_HTML_mode = TRUE;  /* force HTML */
		LYforce_no_cache = TRUE;  /*force the document to be reloaded*/
		StrAllocCopy(newdoc.title, "Current Keymap");
		free_and_clear(&newdoc.post_data);
		free_and_clear(&newdoc.post_content_type);
	    }
	    break;

#ifdef FIXME
	case LYK_JUMP:
	    {
	      char *ret;

	      if(no_jump || jumpfile == NULL || *jumpfile == '\0') {
		  if(old_c != real_c) {
		      old_c = real_c;
		      if (no_jump)
			  _statusline(
			      "Jumping to a shortcut URL is disallowed!");
		      else
			  _statusline(NO_JUMPFILE);
		      sleep(sleep_two);
		  }
	      } else if ((ret = LYJump()) != NULL) {
		  ret = HTParse(ret, startfile, PARSE_ALL);
		  StrAllocCopy(newdoc.address, ret);
		  free_and_clear(&newdoc.post_data);
		  free_and_clear(&newdoc.post_content_type);
		  free(ret);
		  LYJumpFileURL = TRUE;
		  LYUserSpecifiedURL = TRUE;
	      }
	      break;
	    }

	case LYK_VERSION:

	    if(!strcmp(LYUserAgent,LYUserAgentFake))
	    {
		StrAllocCopy(LYUserAgent, LYNX_NAME);
		StrAllocCat(LYUserAgent, "/");
		StrAllocCat(LYUserAgent, LYNX_VERSION);
	    } else
		StrAllocCopy(LYUserAgent ,LYUserAgentFake);

	    statusline(LYUserAgent);
	    sleep(sleep_three);
	    break;

#endif //fixme
	} /* end of BIG switch */
    }
}

PRIVATE int are_different ARGS2(document *,doc1, document *,doc2)
{
    char *cp1, *cp2;

    /*
     * Do we have two addresses?
     */
    if(!doc1->address || !doc2->address)
	return (TRUE);

    /*
     * See if the addresses are different, making sure
     * we're not tripped up by multiple anchors in the
     * the same document from a POST form. -- FM
     */
    if (cp1=strchr(doc1->address, '#'))
	*cp1 = '\0';
    if (cp2=strchr(doc2->address, '#'))
	*cp2 = '\0';
    /*
     * Are the base addresses different?
     */
    if(strcmp(doc1->address, doc2->address))
      {
        if (cp1)
	    *cp1 = '#';
	if (cp2)
	    *cp2 = '#';
	return(TRUE);
      }
    if (cp1)
	*cp1 = '#';
    if (cp2)
        *cp2 = '#';

    /*
     * Do the docs have different contents?
     */
    if(doc1->post_data)
      {
	if(doc2->post_data)
	  {
	    if(strcmp(doc1->post_data, doc2->post_data))
		return(TRUE);
	  }
	else
	    return(TRUE);
      }
    else
        if(doc2->post_data)
	    return(TRUE);

    /*
     * We'll assume the two documents in fact are the same.
     */
    return(FALSE);
}
