// #include <alloc.h>
#include "HTUtils.h"
#include "LYCurses.h"
// #include "HTFTP.h"
#include "HTML.h"
#include "LYUtils.h"
#include "LYString.h"
#include "LYGlobalDefs.h"
#include "LYOption.h"
#include "LYSignal.h"
#include "LYClean.h"
#include "LYCharSets.h"
#include "LYKeymap.h"
#include "LYrcFile.h"
#include "HTAlert.h"

#include "LYLeaks.h"

#include <unistd.h>

#define ACCEPT_DATA "Hit return to accept entered data"

BOOLEAN term_options;
PRIVATE void terminate_options  PARAMS((int sig));
PUBLIC int boolean_choice PARAMS((int status, int line, char **choices));

#define MAXCHOICES 10

PRIVATE void option_statusline ARGS1(char *,text)
{
    char buffer[256];

    if(!text || text==NULL)
        return;

        /* don't print statusline messages if dumping to stdout
         */
    if(dump_output_immediately)
        return;

    /* make sure text is not longer than COLS */
    LYstrncpy(buffer,text,LYcols-1);

    move(LYlines-1,0);

    clrtoeol();
    if (text != NULL) {
        start_reverse();
        addstr(buffer);
        stop_reverse();
    }

    refresh();
}


PUBLIC void options ()
{
#ifdef ALLOW_USERS_TO_CHANGE_EXEC_WITHIN_OPTIONS
    int itmp;
#endif
    int response=0, ch;
    /* if the user changes the display I need memory to put it in */
    char display_option[256]; 
    static char putenv_command[138];
    char *choices[MAXCHOICES];
    char version[128];
    
    term_options = FALSE;
    signal(SIGINT, terminate_options);

    clear(); 
    move(0,5);
    start_bold();
    printw("         Options Menu (%s Version %s)",
    	   			   LYNX_NAME, LYNX_VERSION);
    stop_bold();
    move(L_EDITOR,5);  
    printw("E)ditor                    : %s",(!editor ? "NONE" : editor));

    move(L_DISPLAY,5);
#ifndef DOS
    printw("D)ISPLAY variable          : %s",(display==NULL? "NONE":display));
#else
    printw("D)ISPLAY variable          : Not available on DOS");
#endif

    move(L_MAIL_ADDRESS,5);  
    printw("P)ersonal mail address     : %s",
		(!personal_mail_address ? "NONE": personal_mail_address));

    move(L_HOME,5);
    printw("B)ookmark file             : %s",(bookmark_page ? bookmark_page :
								"NONE"));

/*
    move(L_FTPSTYPE,5);
    printw("F)TP sort criteria         : %s",(HTfileSortMethod==FILE_BY_NAME ?
					"By Filename" :
					  (HTfileSortMethod==FILE_BY_SIZE ?
					    "By Size" :
					      (HTfileSortMethod==FILE_BY_TYPE ?
						"By Type" : "By Date"))));
*/
    move(L_SSEARCH,5);
    printw("S)earching type            : %s",(case_sensitive ?
					"CASE SENSITIVE" : "CASE INSENSITIVE"));

    move(L_CHARSET,5);
    printw("C)haracter set             : %s", LYchar_set_names[current_char_set]);

    move(L_VIKEYS,5);
    printw("V)I keys                   : %s", (vi_keys ? "ON" : "OFF"));

    move(L_EMACSKEYS,5);
    printw("e(M)acs keys               : %s", (emacs_keys ? "ON" : "OFF"));

    move(L_KEYPAD,5);
    addstr("K)eypad as arrows\n");
    addstr("     "); /* five spaces */
    printw("     or Numbered links     : %s",
					(keypad_mode == NUMBERS_AS_ARROWS ?
					"Numbers act as arrows" :
					"Links are numbered"));
    move(L_LANGUAGE,5);
    printw("preferred lan(G)uage       : %s",(!language ? "NONE" : language));

#ifdef DIRED_SUPPORT
    move(L_DIRED,5);
    printw("l(I)st directory style     : %s",
		     (dir_list_style == FILES_FIRST ? "Files first          " :
		     (dir_list_style == MIXED_STYLE ? "Mixed style          " :
						      "Directories first    ")));
#endif

    move(L_USER_MODE,5);
    printw("U)ser mode                 : %s",
			(user_mode == NOVICE_MODE ? "Novice" :
			(user_mode == INTERMEDIATE_MODE ? "Intermediate" :
							     "Advanced")));

#ifdef ALLOW_USERS_TO_CHANGE_EXEC_WITHIN_OPTIONS
    move(L_EXEC,5);
    printw("L)ocal execution links     : ");
#ifndef NEVER_ALLOW_REMOTE_EXEC
    addstr((local_exec ? "ALWAYS ON" :
		    (local_exec_on_local_files ? "FOR LOCAL FILES ONLY" :
							      "ALWAYS OFF")));
#else
    addstr(local_exec_on_local_files ? "FOR LOCAL FILES ONLY" :
							      "ALWAYS OFF");
#endif /* NEVER_ALLOW_REMOTE_EXEC */
#endif /* ALLOW_USERS_TO_CHANGE_EXEC_WITHIN_OPTIONS */


    move(20,2); addstr("Select capital letter of option line, '>' to save, or 'r' to return to Lynx.");


    while(TOUPPER(response) != 'R' && response != LTARROW &&
	  response != '>' && !term_options && response != 7 &&
					      response != 3) {

	   move(21,1); addstr("Command: ");

	   refresh();
	   response = LYgetch();
	   if (term_options || response == 7 || response == 3)
	       response = 'R';
	   switch(response) {
		case 'e':  /* change the editor */
		case 'E':
			if (!system_editor) {
			    if(!editor) {  /* clear the NONE */
				move(L_EDITOR,34);
				addstr("    ");
			    }
			    option_statusline(ACCEPT_DATA);
			    move(L_EDITOR,34);
				standout();
			    if(editor)
				strcpy(display_option, editor);
			    else
				*display_option = '\0';
			    ch = LYgetstr(display_option, VISIBLE);
				standend();
			    move(L_EDITOR,34);
			    if (!term_options && ch != -1 &&
						 *display_option != '\0')
				StrAllocCopy(editor,display_option);
			    addstr(editor ? editor : "NONE");
			} else {
			    option_statusline(
			  "You are not allowed to change which editor to use");
			    sleep(sleep_two);
			}
			break;
#ifndef DOS

		case 'd':  /* change the display */
		case 'D':
			if(display == NULL) {  /* clear the NONE */
			   move(L_DISPLAY,34);
			   addstr("    ");
			} else
			    strcpy(display_option,display);
			option_statusline(ACCEPT_DATA);
			move(L_DISPLAY,34);
			standout();
			ch = LYgetstr(display_option, VISIBLE);
			standend();
			move(L_DISPLAY,34);
			if (term_options || ch == -1 ||
					    *display_option == '\0')
			    addstr(display ? display : "NONE");
			else {
			    addstr(display_option);
			    sprintf(putenv_command,"DISPLAY=%s",
			    			   display_option);
			    putenv(putenv_command);
		            display = getenv("DISPLAY");
			}
			break;
#endif /* DOS */
		case 'b':  /* change the bookmark page location */
		case 'B':
			/* anonymous users should not be allowed to
			 * change the bookmark page
			 */
			if(!no_bookmark) {
			    if(!bookmark_page) {  /* clear the NONE */
			       move(L_HOME,34);
			       addstr("    ");
			    }
			    option_statusline(ACCEPT_DATA);
			    move(L_HOME,34); 
				standout();
			    if(bookmark_page)
			  	strcpy(display_option, bookmark_page);
			    else
				*display_option = '\0';
			    ch = LYgetstr(display_option, VISIBLE);
				standend();
			    move(L_HOME,34);
			    if (!term_options && ch != -1 &&
						 *display_option != '\0')
			        StrAllocCopy(bookmark_page,display_option);
			    addstr(bookmark_page ? bookmark_page : "NONE");
			} else { /* anonymous */
			   option_statusline("you are not allowed to change the bookmark file!");
			}
			    break;
    
		case 'p':  /* change personal mail address for From headers */
		case 'P':
		 	if(!personal_mail_address) {/* clear the NONE */
			   move(L_MAIL_ADDRESS,34);
			   addstr("    ");
			   *display_option = '\0';
			} else
			   strcpy(display_option,personal_mail_address);
			option_statusline(ACCEPT_DATA);
			move(L_MAIL_ADDRESS,34);
			standout();
			ch = LYgetstr(display_option, VISIBLE);
			standend();
			move(L_MAIL_ADDRESS,34);
			if (!term_options && ch != -1 &&
					     *display_option != '\0')
			    StrAllocCopy(personal_mail_address,display_option);
			addstr(personal_mail_address ?
			       personal_mail_address : "NONE");
			break;
#if 0
                case 'z':
                        {
                           struct farheapinfo heap;
                           long total = farcoreleft();
			   heap.ptr = NULL;
			   while(farheapwalk(&heap) != _HEAPEND)
			       if(!heap.in_use) total += heap.size;
			   printf("Heap left: %ld",total);
			   break;
			}
#endif
			/* copy strings into choice array */
/*
		case 'f':
		case 'F':
			choices[0] = (char *)0;
			StrAllocCopy(choices[0],"By Filename");
			choices[1] = (char *)0;
			StrAllocCopy(choices[1],"By Type    ");
			choices[2] = (char *)0;
			StrAllocCopy(choices[2],"By Size    ");
			choices[3] = (char *)0;
			StrAllocCopy(choices[3],"By Date    ");
			choices[4] = (char *)0;
			HTfileSortMethod = boolean_choice(HTfileSortMethod,
					       L_FTPSTYPE, choices);
			free(choices[0]);
			free(choices[1]);
			free(choices[2]);
			break;
*/
		case 's':
		case 'S':
			 /* copy strings into choice array */
			 choices[0] = (char *)0;
			 StrAllocCopy(choices[0],"CASE INSENSITIVE");
			 choices[1] = (char *)0;
			 StrAllocCopy(choices[1],"CASE SENSITIVE  ");
			 choices[2] = (char *)0;
			 case_sensitive = boolean_choice(case_sensitive,
						L_SSEARCH, choices);
			 free(choices[0]);
			 free(choices[1]);
			break;
#ifdef ALLOW_USERS_TO_CHANGE_EXEC_WITHIN_OPTIONS
		case 'l':  /* local exec */
		case 'L':
			if(!exec_frozen) {
#ifndef NEVER_ALLOW_REMOTE_EXEC
			   if(local_exec) {
				itmp=2;
			   } else {
#else
			  {
#endif /* NEVER_ALLOW_REMOTE_EXEC */
			  	if(local_exec_on_local_files)
				    	itmp=1;
				else
					itmp=0;
			   }
			   /* copy strings into choice array */
			   choices[0] = (char *)0;
			   StrAllocCopy(choices[0],"ALWAYS OFF          ");
			   choices[1] = (char *)0;
			   StrAllocCopy(choices[1],"FOR LOCAL FILES ONLY");
			   choices[2] = (char *)0;
#ifndef NEVER_ALLOW_REMOTE_EXEC
			   StrAllocCopy(choices[2],"ALWAYS ON           ");
			   choices[3] = (char *)0;
#endif /* NEVER_ALLOW_REMOTE_EXEC */
			   itmp = boolean_choice(itmp, L_EXEC, choices);
  
			   free(choices[0]);
			   free(choices[1]);
#ifndef NEVER_ALLOW_REMOTE_EXEC
			   free(choices[2]);
#endif /* NEVER_ALLOW_REMOTE_EXEC */
			   switch(itmp) {
			      case 0:
				  local_exec=FALSE;
				  local_exec_on_local_files=FALSE;
				  break;
			      case 1:
				  local_exec=FALSE;
				  local_exec_on_local_files=TRUE;
				  break;
#ifndef NEVER_ALLOW_REMOTE_EXEC
			      case 2:
				  local_exec=TRUE;
				  local_exec_on_local_files=FALSE;
				  break;
#endif /* NEVER_ALLOW_REMOTE_EXEC */
			  } /* end switch */
			} else {
			   option_statusline("You are not allowed to change this setting");
			   sleep(sleep_two); 
			}
			break;
#endif /* ALLOW_USERS_TO_CHANGE_EXEC_WITHIN_OPTIONS */

		case 'c':
		case 'C':
			current_char_set = boolean_choice(current_char_set,
			    		L_CHARSET, LYchar_set_names);
			/* set char set in HTML.c */
			HTMLUseCharacterSet(current_char_set);
			break;
		case 'v':
		case 'V':
			/* copy strings into choice array */
			choices[0] = (char *)0;
			StrAllocCopy(choices[0],"OFF");
			choices[1] = (char *)0;
			StrAllocCopy(choices[1],"ON ");
			choices[2] = (char *)0;
			vi_keys = boolean_choice(vi_keys, L_VIKEYS, choices);
			if (vi_keys)
                            set_vi_keys();
                        else
                            reset_vi_keys();
			free(choices[0]);
			free(choices[1]);
			break;
		case 'M':
		case 'm':
			/* copy strings into choice array */
			choices[0] = (char *)0;
			StrAllocCopy(choices[0],"OFF");
			choices[1] = (char *)0;
			StrAllocCopy(choices[1],"ON ");
			choices[2] = (char *)0;
			emacs_keys = boolean_choice(emacs_keys, L_EMACSKEYS, 
								      choices);
                        if (emacs_keys)
                            set_emacs_keys();
                        else
                            reset_emacs_keys();
			free(choices[0]);
			free(choices[1]);
			break;
		case 'k':
		case 'K':
			/* copy strings into choice array */
			choices[0] = (char *)0;
			StrAllocCopy(choices[0],"Numbers act as arrows");
			choices[1] = (char *)0;
			StrAllocCopy(choices[1],"Links are numbered   ");
			choices[2] = (char *)0;
			keypad_mode = boolean_choice(keypad_mode,
			       				L_KEYPAD+1, choices);
                        if (keypad_mode == NUMBERS_AS_ARROWS)
                            set_numbers_as_arrows();
                        else
                            reset_numbers_as_arrows();
			free(choices[0]);
			free(choices[1]);
			break;

		case 'g':  /* change language preference */
		case 'G':
			if(language == NULL) {  /* clear the NONE */
			   move(L_LANGUAGE,34);
			   addstr("    ");
			} else
			    strcpy(display_option,language);
			option_statusline(ACCEPT_DATA);
			move(L_LANGUAGE,34);
			standout();
			ch = LYgetstr(display_option, VISIBLE);
			standend();
			move(L_LANGUAGE,34);
			if (term_options || ch == -1 ||
					    *display_option == '\0')
			    addstr(language ? language : "NONE");
			else {
			    StrAllocCopy(language,display_option);
			    addstr(display_option);
			}
			break;

#ifdef DIRED_SUPPORT
		case 'i':
		case 'I':
			/* copy strings into choice array */
			choices[0] = (char *)0;
			StrAllocCopy(choices[0],"Directories first");
			choices[1] = (char *)0;
			StrAllocCopy(choices[1],"Files first      ");
			choices[2] = (char *)0;
			StrAllocCopy(choices[2],"Mixed style      ");
			choices[3] = (char *)0;
			dir_list_style = boolean_choice(dir_list_style,
							L_DIRED, choices);
			free(choices[0]);
			free(choices[1]);
			free(choices[2]);
			break;
#endif
		case 'u':
		case 'U':
			/* copy strings into choice array */
			choices[0] = (char *)0;
			StrAllocCopy(choices[0],"Novice      ");
			choices[1] = (char *)0;
			StrAllocCopy(choices[1],"Intermediate");
			choices[2] = (char *)0;
			StrAllocCopy(choices[2],"Advanced    ");
			choices[3] = (char *)0;
			user_mode = boolean_choice(user_mode,
							L_USER_MODE, choices);
			free(choices[0]);
			free(choices[1]);
			free(choices[2]);
			if(user_mode == NOVICE_MODE)
			   display_lines = LYlines-4;
			else
			   display_lines = LYlines-2;
			break;
		case '>':
                        if (!no_option_save) {
                            option_statusline("Saving...");
                            if(save_rc())
				option_statusline("Saved!");
			    else 
				HTAlert("Unable to save options");

                        } else {
                            option_statusline("Option saving disabled!");
                            sleep(sleep_two);
			    /* change response so that we don't exit
			     * the options menu 
			     */
			    response = ' ';
			} 
			break;
		case 'r':
		case 'R':
			break;
		default:
			option_statusline("'R' to return to lynx");
	    }  /* end switch */
     }  /* end while */

     term_options = FALSE;
     signal(SIGINT, cleanup_sig);
}


/* take a boolean status and prompt the user for a new status
 * and return it
 */

PUBLIC int boolean_choice ARGS3(int,status, int,line, char **,choices)
{
	int response=0;
	int number=0;
	
	for (; choices[number] != (char *) 0; number++)
	    ;  /* empty loop body */

	number--;

	option_statusline(ACCEPT_DATA);
	/* highlight the current selection */
	move(line,34);
	// standout();
	addstr(choices[status]);

	option_statusline("Hit any key to change value; RETURN to accept");

	// standout();  /* option_statusline might turn it off */

	while(1) {
	   move(line,34);
	   response = LYgetch();
	   if (term_options || response == 7 || response == 3)
		response = '\n';
	   if(response != '\n' && response != '\r') {
		if(status == number)
		    status = 0;  /* go over the top and around */
		else
		    status++;
		addstr(choices[status]);
	        refresh();
	    } else {
		option_statusline("Value accepted");

		/* unhighlight selection */
	        move(line,34);
	        standend();
	        addstr(choices[status]);
	 	return(status);
	    }
	}
}


PRIVATE void terminate_options ARGS1(int,sig)
{
	term_options=TRUE;
#ifdef VMS
	/* Reassert the AST */
	signal(SIGINT, terminate_options);
        /* refresh the screen to get rid of the "interrupt" message */
	if (!dump_output_immediately) {
	    clearok(curscr, TRUE);
	    refresh();
	}
#endif /* VMS */
}


