#include "HTUtils.h"
#include "HTFile.h"
#include "LYUtils.h"
#include "LYString.h"
#include "LYStruct.h"
#include "LYGlobalDefs.h"
#include "LYCharSets.h"
#include "LYKeymap.h"
#include "LYGetFil.h"
// #include "LYCgi.h"
#include "curses.h"

#include "LYLeaks.h"

PUBLIC BOOLEAN have_read_cfg=FALSE;
PUBLIC BOOLEAN LYUseNoviceLineTwo=TRUE;

#ifdef VMS
#define DISPLAY "DECW$DISPLAY"
#else
#define DISPLAY "DISPLAY"
#endif /* VMS */

PRIVATE int is_true ARGS1(char *,string)
{
   if(!strncasecomp(string,"TRUE",4))
	return(TRUE);
   else
	return(FALSE);
}

PRIVATE void add_item_to_list ARGS2(char *,buffer, lynx_html_item_type **,list_ptr)
{
   char *colon, *next_colon;
   lynx_html_item_type *cur_item, *prev_item;

   /* make a linked list */

   if(*list_ptr == NULL) {  /* first item */
      
      cur_item =(lynx_html_item_type *)calloc(sizeof(lynx_html_item_type),1);
      
      if(cur_item == NULL)
	perror("Out of memory in read_cfg");

      *list_ptr = cur_item;

   } else {
      
      /* find the last item */
      for(prev_item = *list_ptr; prev_item->next != NULL;
	  prev_item = prev_item->next)
	;  /* null body */
      
      cur_item = (lynx_html_item_type *)calloc(sizeof(lynx_html_item_type),1);
      
      if(cur_item == NULL)
	perror("Out of memory in read_cfg");
      else
	prev_item->next = cur_item;
   }
   
   cur_item->next = NULL;
   cur_item->name = NULL;
   cur_item->command = NULL;
   cur_item->always_enabled = FALSE;

   /* find first colon */
   colon = (char *)strchr(buffer,':');
   /* make sure it isn't escaped by a backslash */
   while(colon!=NULL && *(colon-1)=='\\')
     /* if it was escaped, try again */
     colon = (char *)strchr(colon+1,':');
   
   if(colon!=NULL) {

      cur_item->name = calloc((colon-buffer+1),sizeof(char));
      if(cur_item->name == NULL)
	perror("Out of memory in read_cfg");
      
      LYstrncpy(cur_item->name, buffer, colon-buffer);
      
      remove_backslashes(cur_item->name);

      next_colon = (char *)strchr(colon+1,':');
      /* make sure it isn't escaped by a backslash */
      while(next_colon!=NULL && *(next_colon-1)=='\\')
	/* if it was escaped, try again */
	next_colon = (char *)strchr(next_colon+1,':');

      if(next_colon!=NULL) {

	 cur_item->command = calloc(next_colon-colon,
				       sizeof(char));

	 if(cur_item->command == NULL)
	   perror("Out of memory in read_cfg");

	 LYstrncpy(cur_item->command, colon+1,
		   next_colon-(colon+1));

	 remove_backslashes(cur_item->command);

	 cur_item->always_enabled = is_true(next_colon+1);

      }
   }
}


PRIVATE void add_extern_to_list ARGS2(char *,buffer, lynx_extern_item_type **,list_ptr)
{
   char *colon, *next_colon;
   lynx_extern_item_type *cur_item, *prev_item;

   /* make a linked list */

   if(*list_ptr == NULL) {  /* first item */

      cur_item =(lynx_extern_item_type *)calloc(sizeof(lynx_extern_item_type),1);

      if(cur_item == NULL)
	perror("Out of memory in read_cfg");

      *list_ptr = cur_item;

   } else {

      /* find the last item */
      for(prev_item = *list_ptr; prev_item->next != NULL;
	  prev_item = prev_item->next)
	;  /* null body */

      cur_item = (lynx_extern_item_type *)calloc(sizeof(lynx_extern_item_type),1);

      if(cur_item == NULL)
	perror("Out of memory in read_cfg");
      else
	prev_item->next = cur_item;
   }

   cur_item->next = NULL;
   cur_item->name = NULL;

   if(buffer!=NULL) {

      StrAllocCopy(cur_item->name, buffer);
      StrAllocCat(cur_item->name, ":");

      if(cur_item->name == NULL)
	perror("Out of memory in read_cfg");

   }
}


PRIVATE void add_printer_to_list ARGS2(char *,buffer, lynx_printer_item_type **,list_ptr)
{
   char *colon, *next_colon;
   lynx_printer_item_type *cur_item, *prev_item;

   /* make a linked list */

   if(*list_ptr == NULL) {  /* first item */
      
      cur_item =(lynx_printer_item_type *)calloc(sizeof(lynx_printer_item_type),1);

      if(cur_item == NULL)
	perror("Out of memory in read_cfg");
      
      *list_ptr = cur_item;
      
   } else {
      
      /* find the last item */
      for(prev_item = *list_ptr; prev_item->next != NULL;
	  prev_item = prev_item->next)
	;  /* null body */
      
      cur_item = (lynx_printer_item_type *)calloc(sizeof(lynx_printer_item_type),1);
      
      if(cur_item == NULL)
	perror("Out of memory in read_cfg");
      else
	prev_item->next = cur_item;
   }

   cur_item->next = NULL;
   cur_item->name = NULL;
   cur_item->command = NULL;
   cur_item->always_enabled = FALSE;
   
   /* find first colon */
   colon = (char *)strchr(buffer,':');
   /* make sure it isn't escaped by a backslash */
   while(colon!=NULL && *(colon-1)=='\\')
     /* if it was escaped, try again */
     colon = (char *)strchr(colon+1,':');
   
   if(colon!=NULL) {

      cur_item->name = calloc((colon-buffer+1),sizeof(char));
      if(cur_item->name == NULL)
	perror("Out of memory in read_cfg");
      
      LYstrncpy(cur_item->name, buffer, colon-buffer);	

      remove_backslashes(cur_item->name);
      
      next_colon = (char *)strchr(colon+1,':');
      /* make sure it isn't escaped by a backslash */
      while(next_colon!=NULL && *(next_colon-1)=='\\')
	/* if it was escaped, try again */
	next_colon = (char *)strchr(next_colon+1,':');

      if(next_colon!=NULL) {
	 
	 cur_item->command = calloc(next_colon-colon, 
				       sizeof(char));
	 
	 if(cur_item->command == NULL)
	   perror("Out of memory in read_cfg");
	 
	 LYstrncpy(cur_item->command, colon+1, 
		   next_colon-(colon+1));
	 
	 remove_backslashes(cur_item->command);
	 
	 cur_item->always_enabled = is_true(next_colon+1);
      }
      next_colon = (char *)strchr(next_colon+1,':');

      if(next_colon!=NULL) {
        cur_item->pagelen = atoi(next_colon+1);
      } else {
        /* default to 66 lines */
        cur_item->pagelen = 66;
      }
   }
}

PUBLIC void read_cfg ARGS1(char *,cfg_filename)
{
    FILE *fp;
    char buffer[501];
    char temp[501];
    char *line_feed;
    int i;

    if (!cfg_filename || strlen(cfg_filename) == 0) {
#ifdef DT
	if(TRACE)
	    fprintf(stderr,"No filename following -cfg switch!\n");
#endif

	return;
    }

    if((fp = fopen(cfg_filename,"r")) == NULL) {
#ifdef DT
	if(TRACE)
	    fprintf(stderr,"lynx.cfg file not found as %s\n",cfg_filename);
#endif

	return;
    }

    have_read_cfg=TRUE;

    while(fgets(buffer, 500, fp) != NULL) {

	/* strip off \n at the end */
	if((line_feed = (char *)strchr(buffer,'\n')) != NULL)
	    *line_feed = '\0';

	/* strip off trailing white space */
	while (isspace(buffer[strlen(buffer)-1]))
	    buffer[strlen(buffer)-1] = '\0';
	  
	if(buffer[0] == '#') {
	    /* nothing */

	} else if(buffer[0] == '\0') {
	    /* nothing */

        } else if(!strncasecomp(buffer,"SUFFIX:",7)) {
	    char *extention;
	    char *mime_type;

	    if(strlen(buffer) > 9) {
	        extention = buffer + 7;
	        if((mime_type = strchr(extention, ':')) != NULL) 
		    *mime_type++ = '\0';
		    HTSetSuffix(extention, mime_type, "binary", 1.0);
	    }

        } else if(!strncasecomp(buffer,"VIEWER:",7)) {
	    char *mime_type;
	    char *viewer;
	    char *environment;

	    if(strlen(buffer) > 9) {
	        mime_type = buffer + 7;
	        if((viewer = strchr(mime_type, ':')) != NULL) 
		    *viewer++ = '\0';
		    if((environment = strchr(viewer, ':')) != NULL) {
			*environment++ = '\0';
			/* if environment equals xwindows then only
			 * assign the presentation if there is a display
			 * variable
			 */
			if(!strcasecomp(environment,"XWINDOWS")) {
			    if(getenv(DISPLAY)) 
				HTSetPresentation(mime_type, viewer, 1.0,
								      3.0, 0.0);
			} else if(!strcasecomp(environment,"NON_XWINDOWS")) {
			    if(!getenv(DISPLAY)) 
		      		HTSetPresentation(mime_type, viewer, 1.0, 	
								      3.0, 0.0);
			} else {
		            HTSetPresentation(mime_type, viewer, 1.0, 3.0, 0.0);
			}
		    } else {
		        HTSetPresentation(mime_type, viewer,  1.0, 3.0, 0.0);
		    }
	    }

        } else if(!strncasecomp(buffer,"KEYMAP:",7)) {
            char *key;
            char *func;
  
            key = buffer + 7;
            if ((func = strchr(key, ':')) != NULL)	{
		*func++ = '\0';
		/* Allow comments on the ends of key remapping lines. - DT */
            	if (!remap(key, strtok(func, " \t\n#")))
                    fprintf(stderr,
			    "key remapping of %s to %s failed\n",key,func);
		else if (!strcmp("TOGGLE_HELP", strtok(func, " \t\n#")))
		    LYUseNoviceLineTwo = FALSE;
	    }

	} else if(!strncasecomp(buffer,"TEXT_NORMAL:",12)) {

	    char temp[2];
	    temp[1] = 0;

	    temp[0] = buffer[12];
	    normal_fore = atoi(temp);
	    temp[0] = buffer[14];
	    normal_back = atoi(temp);

	    if(buffer[16] == '1')
		normal_bold = A_BOLD;
	    else
		normal_bold = A_NORMAL;

	} else if(!strncasecomp(buffer,"TEXT_BOLD:",10)) {

	    char temp[2];
	    temp[1] = 0;

	    temp[0] = buffer[10];
	    bold_fore = atoi(temp);
	    temp[0] = buffer[12];
	    bold_back = atoi(temp);

	    if(buffer[14] == '1')
		bold_bold = A_BOLD;
	    else
		bold_bold = A_NORMAL;

	} else if(!strncasecomp(buffer,"TEXT_UNDERLINE:",15)) {

	    char temp[2];
	    temp[1] = 0;

	    temp[0] = buffer[15];
	    underline_fore = atoi(temp);
	    temp[0] = buffer[17];
	    underline_back = atoi(temp);

	    if(buffer[19] == '1')
		underline_bold = A_BOLD;
	    else
		underline_bold = A_NORMAL;

	} else if(!strncasecomp(buffer,"TEXT_REVERSE:",13)) {

	    char temp[2];
	    temp[1] = 0;

	    temp[0] = buffer[13];
	    reverse_fore = atoi(temp);
	    temp[0] = buffer[15];
	    reverse_back = atoi(temp);

	    if(buffer[17] == '1')
		reverse_bold = A_BOLD;
	    else
		reverse_bold = A_NORMAL;

	} else if(!strncasecomp(buffer,"COLOR_SUPPORT:",14)) {

	    use_color = is_true(buffer+14);

	} else if(!strncasecomp(buffer,"FAKE_VERSION:",13)) {

            StrAllocCopy(LYUserAgentFake, buffer+13);

	} else if(!strncasecomp(buffer,"GLOBAL_MAILCAP:",15)) {

	    StrAllocCopy(global_type_map, buffer+15);

	} else if(!strncasecomp(buffer,"GLOBAL_EXTENSION_MAP:",21)) {

	    StrAllocCopy(global_extension_map, buffer+21);

	} else if(!strncasecomp(buffer,"PERSONAL_MAILCAP:",17)) {

            StrAllocCopy(personal_type_map, buffer+17);

        } else if(!strncasecomp(buffer,"PERSONAL_EXTENSION_MAP:",23)) {

            StrAllocCopy(personal_extension_map, buffer+23);

	} else if(!strncasecomp(buffer,"CHARACTER_SET:",14)) {
	    int i=0;
	    for(; LYchar_set_names[i]; i++)
		if(!strncmp(buffer+14,LYchar_set_names[i],strlen(buffer+14)))
		{
		    current_char_set=i;
		    break;
		}

        } else if(!strncasecomp(buffer,"INCOMING:",9)) {

            StrAllocCopy(incoming, buffer+9);
	    if(incoming[strlen(incoming)] != '\\')
		StrAllocCat(incoming, "\\");

	} else if(!strncasecomp(buffer,"STARTFILE:",10)) {

	    StrAllocCopy(startfile, buffer+10);

	} else if(!strncasecomp(buffer,"HELPFILE:",9)) {

	    StrAllocCopy(helpfile, buffer+9);

	} else if(!strncasecomp(buffer,"DEFAULT_INDEX_FILE:",19)) {
	    StrAllocCopy(indexfile, buffer+19);

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
	} else if(!strncasecomp(buffer,
				    "LOCAL_EXECUTION_LINKS_ALWAYS_ON:",32)) {
	    local_exec = is_true(buffer+32);

	} else if(!strncasecomp(buffer,
			    "LOCAL_EXECUTION_LINKS_ON_BUT_NOT_REMOTE:",40)) {
	    local_exec_on_local_files = is_true(buffer+40);
#endif /* defined(EXEC_LINKS) || defined(EXEC_SCRIPTS) */

	} else if(!strncasecomp(buffer,"MAIL_SYSTEM_ERROR_LOGGING:",26)) {
	    error_logging = is_true(buffer+26);

	} else if(!strncasecomp(buffer,"LOG_URLS:",8)) {
	    log_urls = is_true(buffer+8);

	} else if(!strncasecomp(buffer,"CHECKMAIL:",10)) {
	    check_mail = is_true(buffer+10);

#ifdef VMS
	} else if(!strncasecomp(buffer,"USE_FIXED_RECORDS:",18)) {
	    UseFixedRecords = is_true(buffer+18);
#endif /* VMS */

	} else if(!strncasecomp(buffer,"VI_KEYS_ALWAYS_ON:",18)) {
	    vi_keys = is_true(buffer+18);

	} else if(!strncasecomp(buffer,"EMACS_KEYS_ALWAYS_ON:",21)) {
	    emacs_keys = is_true(buffer+21);

	} else if(!strncasecomp(buffer,
			"DEFAULT_KEYPAD_MODE_IS_NUMBERS_AS_ARROWS:",41)) {
	    if(is_true(buffer+41))
		keypad_mode = NUMBERS_AS_ARROWS;
	    else
		keypad_mode = LINKS_ARE_NUMBERED;

	} else if(!strncasecomp(buffer,"CASE_SENSITIVE_ALWAYS_ON:",25)) {
	     case_sensitive = is_true(buffer+25);

	} else if(!strncasecomp(buffer,"DEFAULT_USER_MODE:",18)) {
		if(!strncasecomp(buffer+18,"NOVICE",5))
		   user_mode = NOVICE_MODE;
		else if(!strncasecomp(buffer+18,"INTER",5))
		   user_mode = INTERMEDIATE_MODE;
		else if(!strncasecomp(buffer+18,"ADVANCE",7))
		   user_mode = ADVANCED_MODE;

        } else if(!strncasecomp(buffer,"SMTP_SERVER:",12)) {
                StrAllocCopy(smtp_server,buffer+12);

	} else if(!strncasecomp(buffer,"DEFAULT_BOOKMARK_FILE:",22)) {
		StrAllocCopy(bookmark_page,buffer+22);

	} else if(!system_editor && 
		  !strncasecomp(buffer,"DEFAULT_EDITOR:",15)) {
		StrAllocCopy(editor,buffer+15);

	} else if(!strncasecomp(buffer,"JUMPFILE:",9)) {
		StrAllocCopy(jumpfile,buffer+9);

	} else if(!strncasecomp(buffer,"NO_DOT_FILES:",13)) {
	    nodotfiles = is_true(buffer+13);

	} else if(!strncasecomp(buffer,"JUMPBUFFER:",11)) {
		jump_buffer = is_true(buffer+11);

	} else if(!strncasecomp(buffer,"BOLD_HEADERS:",13)) {
		bold_headers = is_true(buffer+13);

 	} else if(!strncasecomp(buffer,"SYSTEM_EDITOR:",14)) {
		StrAllocCopy(editor,buffer+14);
 		system_editor = TRUE;

	} else if(!strncasecomp(buffer,"PREFERRED_LANGUAGE:",19)) {
		StrAllocCopy(language,buffer+19);

	} else if(!strncasecomp(buffer,"PRINTER:",8)) {
		add_printer_to_list (&buffer[8],&printers);

	} else if(!strncasecomp(buffer,"MAKE_LINKS_FOR_ALL_IMAGES:",26)) {
	    clickable_images = is_true(buffer+26);

	} else if(!strncasecomp(buffer,"AUTO_EXTERN:",12)) {
		add_extern_to_list(&buffer[12],&auto_externs);

	} else if(!strncasecomp(buffer,"DOWNLOADER:",11)) {
		add_item_to_list(&buffer[11],&downloaders);

	} else if (!strncasecomp(buffer, "SLEEP_ONE:", 10)) {
	    strcpy(temp, buffer+10);
	    for (i = 0; temp[i]; i++) {
	      if (!isdigit(temp[i])) {
		 temp[i] = '\0';
		 break;
	      }
	   }
	   if (temp[0])
	     sleep_one = atoi(temp);

	} else if (!strncasecomp(buffer, "SLEEP_TWO:", 10)) {
	    strcpy(temp, buffer+10);
	    for (i = 0; temp[i]; i++) {
	      if (!isdigit(temp[i])) {
		 temp[i] = '\0';
		 break;
	      }
	   }
	   if (temp[0])
	     sleep_two = atoi(temp);

	} else if (!strncasecomp(buffer, "SLEEP_THREE:", 12)) {
	    strcpy(temp, buffer+12);
            for (i = 0; temp[i]; i++) {
              if (!isdigit(temp[i])) {
                 temp[i] = '\0';
                 break;
              }
           }
           if (temp[0])
             sleep_three = atoi(temp);

        } else if (!strncasecomp(buffer, "SHOW_CURSOR:", 12)) {
            LYShowCursor = is_true(buffer+12);

	} else if(!strncasecomp(buffer,"http_proxy:",11)) {
	    if(getenv("http_proxy") == NULL) {
#ifdef VMS
		strcpy(temp, "http_proxy");
		Define_VMSLogical(temp, (char *)&buffer[11]);
#else
		strcpy(temp, "http_proxy=");
		StrAllocCopy(http_proxy_putenv_cmd, temp);
		StrAllocCat(http_proxy_putenv_cmd, (char *)&buffer[11]);
		putenv(http_proxy_putenv_cmd);
#endif /* VMS */
	    }

	} else if(!strncasecomp(buffer,"ftp_proxy:",10)) {
	    if(getenv("ftp_proxy") == NULL) {
#ifdef VMS
		strcpy(temp, "ftp_proxy");
		Define_VMSLogical(temp, (char *)&buffer[10]);
#else
		strcpy(temp, "ftp_proxy=");
		StrAllocCopy(ftp_proxy_putenv_cmd, temp);
		StrAllocCat(ftp_proxy_putenv_cmd, (char *)&buffer[10]);
		putenv(ftp_proxy_putenv_cmd);
#endif /* VMS */
	    }

	} else if(!strncasecomp(buffer,"gopher_proxy:",13)) {
	    if(getenv("gopher_proxy") == NULL) {
#ifdef VMS
		strcpy(temp, "gopher_proxy");
		Define_VMSLogical(temp, (char *)&buffer[13]);
#else
		strcpy(temp, "gopher_proxy=");
		StrAllocCopy(gopher_proxy_putenv_cmd, temp);
		StrAllocCat(gopher_proxy_putenv_cmd, (char *)&buffer[13]);
		putenv(gopher_proxy_putenv_cmd);
#endif /* VMS */
	    }

	} else if(!strncasecomp(buffer,"wais_proxy:",11)) {
	    if(getenv("wais_proxy") == NULL) {
#ifdef VMS
		strcpy(temp, "wais_proxy");
		Define_VMSLogical(temp, (char *)&buffer[11]);
#else
		strcpy(temp, "wais_proxy=");
		StrAllocCopy(wais_proxy_putenv_cmd, temp);
		StrAllocCat(wais_proxy_putenv_cmd, (char *)&buffer[11]);
		putenv(wais_proxy_putenv_cmd);
#endif /* VMS */
	    }

	} else if(!strncasecomp(buffer,"no_proxy:",9)) {
	    if(getenv("no_proxy") == NULL) {
#ifdef VMS
		strcpy(temp, "no_proxy");
		Define_VMSLogical(temp, (char *)&buffer[9]);
#else
		strcpy(temp, "no_proxy=");
		StrAllocCopy(no_proxy_putenv_cmd, temp);
		StrAllocCat(no_proxy_putenv_cmd, (char *)&buffer[9]);
		putenv(no_proxy_putenv_cmd);
#endif /* VMS */
	    }

#ifdef EXEC_LINKS
	} else if(!strncasecomp(buffer,"TRUSTED_EXEC:",13)) {
		add_trusted(&buffer[13], EXEC_PATH); /* Add exec path */
#endif
#ifdef LYNXCGI_LINKS
	} else if(!strncasecomp(buffer,"TRUSTED_LYNXCGI:",16)) {
		add_trusted(&buffer[16], CGI_PATH); /* Add CGI path */
	} else if(!strncasecomp(buffer,"LYNXCGI_ENVIRONMENT:",20)) {
		add_lynxcgi_environment(buffer+20);
#endif
#ifdef DIRED_SUPPORT
	} else if(!strncasecomp(buffer,"UPLOADER:",9)) {
	        add_item_to_list(&buffer[9],&uploaders);
#endif
        }  /* end of Huge if */
    } /* end of while */

    fclose(fp);

}




