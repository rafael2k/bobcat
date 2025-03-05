#include "HTUtils.h"
#include "LYCurses.h"
#include "GridText.h"
#include "LYUtils.h"
#include "LYStruct.h"  /* includes HTForms.h */
#include "LYString.h"
#include "LYGlobalDefs.h"
#include "LYKeymap.h"
#include "LYSignal.h"

#include "LYLeaks.h"

#include <unistd.h>

#ifndef BOXVERT
#define BOXVERT '*'	/* character for popup window vertical borders */
#endif
#ifndef BOXHORI
#define BOXHORI '*'	/* character for popup window horizontal borders */
#endif

PRIVATE int form_getstr PARAMS((struct link * form_link));
PRIVATE int popup_options PARAMS((int cur_selection, OptionType *list, 
						   int ly, int lx, int width,
						   int i_length));

PUBLIC int change_form_link ARGS6(struct link *, form_link, int, mode, 
				  document *,newdoc, BOOLEAN *,refresh_screen,
				  char *,link_name, char *,link_value)
{

    FormInfo *form = form_link->form;
    int c=DO_NOTHING;

	/*	If there is no form to perform action on, don't do anything.
	 */
	if(form == NULL)	{
		return(c);
	}

    /* move to the link position */
    move(form_link->ly, form_link->lx);

    switch(form->type) {
	case F_CHECKBOX_TYPE:
	    if(form->num_value) {
		form_link->hightext = unchecked_box;
		form->num_value = 0;
	    } else {
		form_link->hightext = checked_box;
		form->num_value = 1;
	    }
	    break;

	case F_OPTION_LIST_TYPE:
	    form->num_value = popup_options(form->num_value, form->select_list,
				form_link->ly, form_link->lx, form->size,
				form->size_l);

	    {
    	        OptionType * opt_ptr=form->select_list;
		int i;
    	        for(i=0; i<form->num_value; i++, opt_ptr = opt_ptr->next) 
		    ; /* null body */
		form->value = opt_ptr->name;   /* set the name */
		form->cp_submit_value = opt_ptr->cp_submit_value; /* set the value */
	    }
	    c = 12;  /* CTRL-R for repaint */
	    break;

	case F_RADIO_TYPE:
		/* radio buttons must have one and
		 * only one down at a time! 
		 */
	    if(form->num_value) {
		_statusline("One radio button must be checked at all times!");
		sleep(sleep_two);

	    } else {	
		int i;
		/* run though list of the links on the screen and
		 * unselect any that are selected. :) 
		 */
		start_bold();
		for(i=0; i < nlinks; i++)
		   if(links[i].type == WWW_FORM_LINK_TYPE &&
		       links[i].form->type == F_RADIO_TYPE &&
		        links[i].form->number == form->number &&
   		         /* if it has the same name and its on */
			  !strcmp(links[i].form->name, form->name) &&
                            links[i].form->num_value ) 
		     {
			move(links[i].ly, links[i].lx);
			addstr(unchecked_box);
			links[i].hightext = unchecked_box;
		     }
		stop_bold();
		/* will unselect other button and select this one */
		HText_activateRadioButton(form);
		/* now highlight this one */
		form_link->hightext = checked_box;
	    }
	    break;

	case F_TEXT_TYPE:
	case F_TEXTAREA_TYPE:
	case F_PASSWORD_TYPE:
	    c = form_getstr(form_link);
	    if(form->type == F_PASSWORD_TYPE)
        	form_link->hightext = STARS(strlen(form->value));
	    else
	    	form_link->hightext = form->value;
	    break;

	case F_RESET_TYPE:
	    HText_ResetForm(form);
            *refresh_screen = TRUE;
	    break;
	
	case F_SUBMIT_TYPE:
		/* returns new document URL */
	    HText_SubmitForm(form, newdoc, link_name, link_value);
	    newdoc->link = 0;
	    break;

    }

    return(c);

} 

#ifdef getyx
#define GetYX(y,x)   getyx(stdscr,y,x)
#else
#define GetYX(y,x)   y = stdscr->_cury, x = stdscr->_curx
#endif

PRIVATE int form_getstr ARGS1(struct link *, form_link)
{
     FormInfo *form = form_link->form;
     int pointer = 0, tmp_pointer=0;
     int ch, i;
     int left_margin=1;
//     int right_margin=2;
     int right_margin=0;
     int last_char, len;
     int max_length = (form->maxlength ? form->maxlength : 1024);
     char inputline[1024];
     int startcol, startline;
     int cur_col, far_col;
     BOOLEAN has_there_ever_been_data;  /* True if data was ever in string */
     BOOLEAN extend=TRUE; /* TRUE to enable line extention */
     BOOLEAN line_extended=FALSE;  /* TRUE if the line was moved to accomadate
				    * more text entry
				    */
#ifdef VMS
    extern BOOLEAN HadVMSInterrupt;/* Flag from cleanup_sig() AST       */
#endif

     /* get bigger margins if possible */
     if(form->size > 9)
	if(form->size > 19) {
	    left_margin=10;
	    right_margin=10;
	} else {
	    left_margin=5;
	    right_margin=5;
	}

     /* get the initial position of the cursor */
     GetYX(startline, startcol);

     /* clear the old string */
     for(i=0; i < form->size; i++) 
	addch('_');

     /* go back to the initial position */
     move(startline, startcol);

     if(startcol + form->size > LYcols-1)
	far_col = LYcols-1;
     else
	far_col = startcol + form->size;

	/* if there is enough room to fit the whole
	 * string then disable the moving text feature
	 */
     if(form->maxlength && ((far_col-startcol) >= form->maxlength))
	extend=FALSE;

     strcpy(inputline, form->value);

     /* find that last char in the string that is non-space */
     last_char = strlen(inputline)-1;
     while(last_char >= 0 && isspace(inputline[last_char])) last_char--; 
     inputline[last_char+1] = '\0';

     if(last_char==-1)
	has_there_ever_been_data=FALSE;
     else
	has_there_ever_been_data=TRUE;

top:
     /*** Check to see if there's something in the inputline already ***/
     len = strlen(inputline);
     if(extend && len+startcol+right_margin > far_col) {
	pointer = (len - (far_col - startcol)) + right_margin;
	line_extended = TRUE;
     } else {
	line_extended = FALSE;
     }
     if(pointer > len || pointer < 0)
	pointer = 0;
     
     cur_col = startcol;
     while (inputline[pointer] != '\0') {
	  if(form->type == F_PASSWORD_TYPE)
	     addch('*');
	  else
	     addch((unsigned char)inputline[pointer]);
	  pointer++;
	  cur_col++;
     }
     refresh();

     for (;;) {
	  ch = LYgetch();

	  switch (ch) {

#ifdef AIX
	  case '\227':
#endif
	  case '\n':
	  case '\r':
	  case '\t':
	  case DNARROW:
	  case UPARROW:
          case PGUP:
          case PGDOWN:
	       inputline[pointer] = '\0';
    	       StrAllocCopy(form->value, inputline);
		/* fill out the rest of the space with underscores */
	       for(;cur_col<far_col;cur_col++)
		  addch('_');
	       return(ch);
	       break;

          /* Control-C or Control-G aborts */
          case 3:
          case 7:
               return(-1);

	 	/* break */  /* implicit break */


	  /* erase all the junk on the line and start fresh */
	  case 21 :
		move(startline, startcol);
		/* clear the old string */
     		for(i=0; i <= cur_col-startcol; i++) 
		    addch('_');
		move(startline, startcol);  /* move back */
		pointer = 0;  /* clear inputline */
		cur_col = startcol;
		line_extended = FALSE;
		refresh();
		break;

	  /**  Backspace and delete **/

	  case '\010':
	  case '\177':
	  case LTARROW:
	       if (pointer > 0) {
		    addch('\010');
		    addch('_');
		    addch('\010');

		    pointer--;
		    cur_col--;

		    if(line_extended && (cur_col-left_margin< startcol)) {
     			if(pointer + startcol + right_margin > far_col) {
			    tmp_pointer = (pointer - (far_col - startcol)) 
							      + right_margin;
			    if(tmp_pointer < 0) tmp_pointer=0;
			} else {
			    tmp_pointer = 0;
			    line_extended = FALSE;
			}

			move(startline, startcol);
     			/* clear the old string */
			for(i=0; i <= cur_col-startcol; i++)
			    addch('_');
			move(startline, startcol);

			cur_col = startcol;

     			while (tmp_pointer < pointer) {
				if(form->type == F_PASSWORD_TYPE)
                                    addch('*');
                                else
                                     addch((unsigned char)inputline[tmp_pointer]);
	  			tmp_pointer ++;
	  			cur_col++;
     			}
			
		    }
		    refresh();

	       } else if(ch == LTARROW) {
		   char c='n';
		   if(has_there_ever_been_data) {
			_statusline("Do you want to go back to the previous document? [n]");
			c=LYgetch();
		   } else 
			c='Y';

		   if(TOUPPER(c) == 'Y') {
		        inputline[0] = '\0';
    		        StrAllocCopy(form->value, inputline);
		        return(ch);
		   } else {
		        _statusline("Enter text. Use arrows or tab to move off of field.");
		       /* move back to the string */
			move(startline, startcol);
			refresh();
		   }
	       }
	       break;

	  case RTARROW:  /* print error */
		{ 
		    int newx, newy;
     		    GetYX(newy, newx);
		    _statusline("Link already selected!");
		    sleep(sleep_two);
		    _statusline("Enter text. Use arrows or tab to move off of field.");
                       /* move back to the string */
                    move(newy, newx);
                    refresh();
		}
	        break;	    
	       
	  default:
	       if (printable(ch) && pointer < max_length) {
		    has_there_ever_been_data=TRUE; /* must be now */
		    inputline[pointer++]= ch;
		    cur_col++;

	    if(cur_col <= far_col) {
		    if(form->type == F_PASSWORD_TYPE)
			addch('*');
		    else
			addch(ch);
	    }

		    if(extend && cur_col > far_col) {
//		    if(extend && cur_col+2 > far_col) {
			tmp_pointer = (pointer - (far_col - startcol))
							    + right_margin;
			if(tmp_pointer > pointer)
			    tmp_pointer = 0;
			line_extended = TRUE;

			move(startline, startcol);
			/* clear the old string */
			for(i=0; i <= (cur_col-startcol)-2; i++)
			    addch('_');
			move(startline, startcol);

			cur_col = startcol;

			while (tmp_pointer != pointer) {
				if(form->type == F_PASSWORD_TYPE)
				    addch('*');
				else
				    addch((unsigned char)inputline[tmp_pointer]);
				tmp_pointer ++;
				cur_col++;
			}

		    } else if(pointer >= max_length) {
			_statusline("Maximum length reached!");
			inputline[pointer] = '\0';
			move(startline,startcol);
			pointer=0;
			/* go back to to top and print it out again */
			goto top;
		    }
		    refresh();
	       } else if(!printable(ch)) {
		  /* terminate the string and return the char */
		   inputline[pointer] = '\0';
		   StrAllocCopy(form->value, inputline);
		    /* fill out the rest of the space with underscores */
		   for(;cur_col<far_col;cur_col++)
		      addch('_');
		   return(ch);
	       }
	  }
     }

}


PRIVATE int popup_options ARGS6(int,cur_selection, OptionType *,list,
						   int, ly, int, lx, int,width,
						   int, i_length)
{
    /*
     * Revamped to handle within-tag VALUE's, if present,
     * and to position the popup window appropriately,
     * taking the user_mode setting into account. -- FM
     */
    int c=0, cmd=0, i=0;
    int orig_selection = cur_selection;
    WINDOW * form_window;
    int num_options=0, top, bottom, length= -1;
    OptionType * opt_ptr=list;
    int window_offset=0;
    int display_lines;

    /*
     * Set display_lines based on the user_mode global.
     */
    if(user_mode==NOVICE_MODE)
        display_lines = LYlines-4;
    else
	display_lines = LYlines-2;

    /*
     * Counting the number of options to be displayed.
     *   num_options ranges 0...n
     */
    for(; opt_ptr->next; num_options++, opt_ptr = opt_ptr->next)
         ; /* null body */

    /*
     * Let's assume for the sake of sanity that ly is the number
     *   corresponding to the line the selection box is on.
     * Let's also assume that cur_selection is the number of the
     *   item that should be initially selected, as 0 beign the
     *   first item.
     * So what we have, is the top equal to the current screen line
     *   subtracting the cur_selection + 1 (the one must be for the
     *   top line we will draw in a box).  If the top goes under 0,
     *   consider it 0.
     */
    top = ly - (cur_selection + 1);
    if(top < 0)
	top = 0;

    /*
     * Check and see if we need to put the i_length parameter up to
     *   the number of real options.
     */
    if(!i_length) {
        i_length = num_options;
    }
    else {
        /*
	 * Otherwise, it is really one number too high.
	 */
	i_length--;
    }

    /*
     * The bottom is the value of the top plus the number of options
     *   to view plus 3 (one for the top line, one for the bottom line,
     *   and one to offset the 0 counted in the num_options).
     */
    bottom = top + i_length + 3;

    /*
     * Hmm...  If the bottom goes beyond the number of lines available
     */
    if(bottom > display_lines) {
        /*
	 * Position the window at the top if we have more options
	 *   than will fit in the window.
	 */
	if(i_length+3 > display_lines) {
	    top = 0;
            bottom = top + i_length+3;
	    if(bottom > display_lines)
	        bottom = display_lines + 1;
	} else {
	    /*
	     * Try to position the window so that the selected option will
	     *    appear where the selecton box currently is positioned.
	     * It could end up too high, at this point, but we'll move it
	     *    down latter, if that has happened.
	     */
	    top = (display_lines + 1) - (i_length + 3);
	    bottom = (display_lines + 1);
	}
    }

    /*
     * This is really fun, when the length is 4, it means 0-4, or 5.
     */
    length = (bottom - top) - 2;

    /*
     * Move the window down if it's too high.
     */
    if (bottom < ly + 2) {
	bottom = ly + 2;
	if (bottom > display_lines + 1)
	    bottom = display_lines + 1;
	top = bottom - length - 2;
    }

    /*
     * Set up the overall window, including the boxing characters ('*').
     */
    form_window = newwin(bottom - top, width+4, top, lx - 1);
    scrollok(form_window, TRUE);
    wbkgd(form_window, getbkgd(stdscr));
    
    /*
     * Set up the window_offset for options.
     *   cur_selection ranges from 0...n
     *   length ranges from 0...m
     */
    if(cur_selection >= length)	{
        window_offset = cur_selection - length + 1;
    }

/*
 * OH!  I LOVE GOTOs! hack hack hack
 *        07-11-94 GAB
 *      MORE hack hack hack
 *        09-05-94 FM
 */
redraw:

    opt_ptr=list;

    /* display the boxed options */
    for(i = 0; i <= num_options; i++, opt_ptr = opt_ptr->next) {
        if(i >= window_offset && i - window_offset < length) {
	    wmove(form_window,(i+1)-window_offset,2);
	    wclrtoeol(form_window);
            waddstr(form_window,opt_ptr->name);
	}
    }
    box(form_window, BOXVERT, BOXHORI);
    wrefresh(form_window);
    opt_ptr=NULL;

    /* loop on user input */
    while(cmd != LYK_ACTIVATE) {

        /* unreverse cur selection */
	if(opt_ptr!=NULL) {
	    wmove(form_window,(i+1)-window_offset,2);
            waddstr(form_window,opt_ptr->name);
	}

        opt_ptr=list;

        for(i=0; i<cur_selection; i++, opt_ptr = opt_ptr->next) 
	    ; /* null body */

        wstart_reverse(form_window);
	wmove(form_window,(i+1)-window_offset,2);
        waddstr(form_window,opt_ptr->name);
             wmove(form_window,(i+1)-window_offset,2);
        wstop_reverse(form_window);
        wrefresh(form_window);

        c = LYgetch();
	cmd = keymap[c+1];

new_cmd:  /* jump here to skip user */

        switch(cmd) {
            case LYK_PREV_LINK:
	    case LYK_UP_LINK:
	
	    if(cur_selection)
                cur_selection--;

		/* scroll the window up if neccessary */
		if(cur_selection-window_offset < 0) {
		    wmove(form_window,1,2);
		    winsertln(form_window);
    		    box(form_window, BOXVERT, BOXHORI);
		    window_offset--;
		}
                break;
            case LYK_NEXT_LINK:
	    case LYK_DOWN_LINK:
		if(cur_selection < num_options)
                    cur_selection++;

		/* scroll the window down if neccessary */
		if(cur_selection-window_offset >= length) {
		    /* remove the bottom border befor scrolling */
		    wmove(form_window,length+1,1);
		    wclrtoeol(form_window);
		    scroll(form_window);
    		    box(form_window, BOXVERT, BOXHORI);
		    window_offset++;
		}
                break;

	    case LYK_NEXT_PAGE:
		/*
		 * Okay, are we on the last page of the list?
		 *   if not then,
		 */
		if(window_offset != num_options - length + 1) {
		    /*
		     * Modify the current selection to not be a
		     *   coordinate in the list, but a coordinate
		     *   on the item selected in the window.
		     */
		    cur_selection -= window_offset;

		    /*
		     * Page down the proper length for the list.
		     * If simply to far, back up.
		     */
		    window_offset += length;
		    if(window_offset > num_options - length) {
		        window_offset = num_options - length + 1;
		    }

		    /*
		     * Readjust the current selection to be a list
		     *   coordinate rather than window.
		     * Redraw this thing.
		     */
		    cur_selection += window_offset;
		    goto redraw;
		}
		else if(cur_selection < num_options) {
		    /*
		     * Already on last page of the list so just
		     *   redraw it with the last item selected.
		     */
		    cur_selection = num_options;
		}
		break;

	    case LYK_PREV_PAGE:
		/*
		 * Are we on the first page of the list?
		 *   if not then,
		 */
		if(window_offset != 0) {
		    /*
		     * Modify the current selection to not be a list
		     *   coordinate, but a window coordinate.
		     */
		    cur_selection -= window_offset;

		    /*
		     * Page up the proper length.
		     * If too far, back up.
		     */
		    window_offset -= length;
		    if(window_offset < 0) {
		        window_offset = 0;
		    }

		    /*
		     * Readjust the current selection.
		     */
		    cur_selection += window_offset;
		    goto redraw;
		}
		else if(cur_selection > 0) {
		    /*
		     * Already on the first page so just
		     *   back up to the first item.
		     */
		    cur_selection = 0;
		}
		break;

	    case LYK_QUIT:
	    case LYK_ABORT:
	    case LYK_PREV_DOC:
	    case 7:	/* Control-G */
	    case 3:	/* Control-C */
		cur_selection = orig_selection;
		cmd=LYK_ACTIVATE; /* to exit */
		break;
        }

    }
    delwin(form_window);
    refresh();

    return(cur_selection);
}
