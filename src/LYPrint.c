//#include "spawno.h"
// #include "LYMail.h"
#include "HTUtils.h"
#include "LYCurses.h"
#include "HTAccess.h"
#include "GridText.h"
#include "LYUtils.h"
#include "LYPrint.h"
#include "LYGlobalDefs.h"
#include "LYSignal.h"
#include "LYString.h"
#include "LYClean.h"
#include "LYGetFil.h"
#include "LYHistor.h"
#include "LYSystem.h"

#include "LYLeaks.h"

#include <unistd.h>

/*
 *  printfile prints out the current file minus the links and targets 
 *  to a veriaty of places
 */

/* it parses an incoming link that looks like
 *
 *  LYNXPRINT://LOCAL_FILE/lines=##
 *  LYNXPRINT://MAIL_FILE/lines=##
 *  LYNXPRINT://TO_SCREEN/lines=##
 *  LYNXPRINT://PRINTER/lines=##/number=#
 */

#define TO_FILE   1
#define TO_SCREEN 2
#define MAIL      3
#define PRINTER   4

PRIVATE int remove_quotes PARAMS((char *string));

PUBLIC int printfile ARGS1(document *,newdoc) 
{
    char buffer[LINESIZE];
    int lines_in_file=0;
    int printer_number;
    int pages=0;
    char filename[256];
    char user_response[256];
    int type, c;
    FILE *outfile_fp;
    char *cp;
    lynx_printer_item_type *cur_printer;
    char *sug_filename = 0;
    char *link_info = 0;
    DocAddress WWWDoc;
    int pagelen=0;

    /* extract useful info from URL */
    StrAllocCopy(link_info,newdoc->address+12);

	/* reload the file we want to print into memory */
    pop(newdoc);
    WWWDoc.address = newdoc->address;
    WWWDoc.post_data = newdoc->post_data;
    WWWDoc.post_content_type = newdoc->post_content_type;
    if(!HTLoadAbsolute(&WWWDoc))
        return(NOT_FOUND);
  
    StrAllocCopy(sug_filename, newdoc->address); /* must be freed */

    /* get the number of lines in the file */
    if((cp = (char *)strstr(link_info, "lines=")) != NULL) {
	/* terminate prev string here */
	*cp = '\0';
        /* number of charters in lines=  */
	cp += 6;

        lines_in_file = atoi(cp);
	pages = lines_in_file/66;
    }

    /* determine the type */
    if(strstr(link_info, "LOCAL_FILE")) {
	type = TO_FILE;
    } else if(strstr(link_info, "TO_SCREEN")) {
	type = TO_SCREEN;
    } else if(strstr(link_info, "MAIL_FILE")) {
	type = MAIL;
    } else if(strstr(link_info, "PRINTER")) {
	type = PRINTER;

        if((cp = (char *)strstr(link_info, "number=")) != NULL) {
            /* number of charters in number=  */
            cp += 7;
            printer_number = atoi(cp);
        }
        if((cp = (char *)strstr(link_info, "pagelen=")) != NULL) {
            /* page length */
            cp += 8;
	    pagelen = atoi(cp);
        } else {
            /* default to 66 lines */
            pagelen = 66;
        }
    }


    switch(type) {
	case TO_FILE:
		_statusline("Please enter a file name: ");
	retry:	strcpy(filename, sug_filename);  /* add suggestion info */
		/* make the sug_filename conform to system specs */
		change_sug_filename(filename);
		if(LYgetstr(filename, VISIBLE) < 0) {
                     _statusline("Print request cancelled!!!");
                     sleep(sleep_two);
		     free(sug_filename);
		     free(link_info);
                     return (NORMAL);
		}

		if (nodotfiles) {
		    char *cp;

		    if (*filename == '.' ||
			((cp = strrchr(filename, '/')) && *(cp+1) == '.')) {
			_statusline("File name may not begin with dot. Enter a new file name: ");
			goto retry;
		    }
		}

	/* see if file will open in current directory where lynx was started */ 
		if(getenv("PWD"))
                    sprintf(buffer,"%s/%s",getenv("PWD"),filename);
		else
		    strcpy(buffer,filename);

                if((outfile_fp = fopen(buffer,"w")) == NULL) {
   	/* if it doesn't open, try to save it in the 'HOME' directory */
                  if(*filename != '/' && *filename != '\\') {
	              if(*filename == '~') {
	                  strcpy(buffer,getenv("HOME"));
	                  strcat(buffer,filename+1); /* skip over the ~ */
	                  strcpy(filename,buffer);
	              }
	              else {
	                  strcpy(buffer,getenv("HOME"));
			  strcat(buffer,"/");
	              } /* end if-else */
            
                      strcat(buffer,filename);
                      strcpy(filename,buffer);
                  }
            
                  if((outfile_fp = fopen(filename,"w")) == NULL) {
	              _statusline("Unable to open output file!!!");
	              sleep(sleep_two);
		      free(sug_filename);
		      free(link_info);
	              return (NORMAL);
		  }
		} /* end of if(outfile_fp ==NULL from way up top */

		print_wwwfile_to_fd(outfile_fp,0);

		fclose(outfile_fp);
		break;

#if 0
	case MAIL:
	{
		char tempfile[128];

		if(smtp_server == NULL) {
			_statusline("You need to define a SMTP_SERVER in lynx.cfg.");
                        sleep(sleep_three);
			free(sug_filename);
			free(link_info);
			return (NORMAL);
		}

		if (!(personal_mail_address && *personal_mail_address)) {

			_statusline("You need to define a Personal mail address in Options.");
                        sleep(sleep_three);
			free(sug_filename);
			free(link_info);
			return (NORMAL);
		}

		_statusline("Please enter a valid internet mail address: ");
		strcpy(user_response, (personal_mail_address ?
						personal_mail_address : ""));
		if(LYgetstr(user_response, VISIBLE) < 0
						|| *user_response == '\0') {
		    _statusline("Mail request cancelled!!!");
		    sleep(sleep_two);
		    free(sug_filename);
		    free(link_info);
		    return (NORMAL);
		}

		change_sug_filename(sug_filename);

		tempname(tempfile,NEW_FILE);

		if ((outfile_fp = fopen(tempfile, "wt")) == NULL) {
			_statusline("ERROR - Unable to mail file");
			sleep(sleep_two);
			free(sug_filename);
			free(link_info);
			return (NORMAL);
		}

                fprintf(outfile_fp,"From: %s\n", personal_mail_address);

		fprintf(outfile_fp,"To: %s\n", user_response);

		fprintf(outfile_fp,"X-within-URL: %s\n",newdoc->address);

		fprintf(outfile_fp,"X-Mailer: Lynx, Version %s\n",LYNX_VERSION);

		fprintf(outfile_fp,"Subject: %s\n\n", sug_filename);

		print_wwwfile_to_fd(outfile_fp,0);

		fclose(outfile_fp);

		if(sendmail(tempfile) > 0) remove(tempfile);

		break;
	}
#endif

	case TO_SCREEN:
		pages = lines_in_file/(LYlines+1);
		/* count fractional pages ! */
		if ((lines_in_file % (LYlines+1)) >0)  pages++; 
		if(pages > 4) {
		    sprintf(filename,"File is %d screens long.  Are you sure you want to print? [y]",pages);
 		    _statusline(filename);
		    c=LYgetch();
    		    if (c == RTARROW || c == 'y' || c== 'Y'
                         || c == '\n' || c == '\r') {
                        addstr("   Ok...");
		    } else {
		        free(sug_filename);
		        free(link_info);
			return (NORMAL);
		    }
		}

		_statusline("Press RETURN to begin: ");
		*filename = '\0';
		if (LYgetstr(filename, VISIBLE) <0) {
		      _statusline("Print request cancelled!!!");
	              sleep(sleep_two);
		      free(sug_filename);
		      free(link_info);
	              return (NORMAL);
                }

		outfile_fp = stdout;

                stop_curses();

		print_wwwfile_to_fd(outfile_fp,0);

#ifdef VMS
		if (HadVMSInterrupt) {
		     HadVMSInterrupt = FALSE;
		     start_curses();
		     break;
		}
#endif /* VMS */
		fprintf(outfile_fp,"\n\nPress RETURN to finish");

		fflush(stdout);  /* refresh to screen */
		LYgetch();  /* grab some user input to pause */
#ifdef VMS
		HadVMSInterrupt = FALSE;
#endif /* VMS */
                clear();
//                start_curses();
		break;
	
	case PRINTER:
		pages = lines_in_file/pagelen;
		/* count fractional pages ! */
		if ((lines_in_file % pagelen) >0)  pages++; 
		if(pages > 4) {
		    sprintf(filename,"File is %d pages long.  Are you sure you want to print? [y]",pages);
 		    _statusline(filename);
		    c=LYgetch();
    		    if (c == RTARROW || c == 'y' || c== 'Y'
                         || c == '\n' || c == '\r') {
                        addstr("   Ok...");
		    } else  {
			free(sug_filename);
		        free(link_info);
			return (NORMAL);
		    }
		}

		
		tempname(filename,NEW_FILE);

                if((outfile_fp = fopen(filename,"w")) == NULL) {
	            _statusline("ERROR - Unable to allocate file space!!!");
	            sleep(sleep_two);
		    free(sug_filename);
		    free(link_info);
	            return (NORMAL);
                }

		print_wwwfile_to_fd(outfile_fp,0);

		fclose(outfile_fp);

		/* find the right printer number */
		{
		    int count=0;
		    for(cur_printer = printers; count < printer_number;
				    count++, cur_printer = cur_printer->next)
			; /* null body */
		}

		/* commands have the form "command %s [%s] [etc]"
		 * where %s is the filename and the second optional
		 * %s is the suggested filename
		 */
		if(cur_printer->command != NULL) {

		    /* check for two '%s' and ask for the second filename
		     * argument if there is
		     */
		    char *first_s = strstr(cur_printer->command, "%s");
		    if(first_s && strstr(first_s+1, "%s")) {
			_statusline("Enter a filename: ");
		again:	strcpy(user_response, sug_filename);
			if(LYgetstr(user_response,0)==-1 ||
				    *user_response == '\0') {
			    _statusline("Print request cancelled!!!");
			    sleep(sleep_two);
			    free(sug_filename);
			    free(link_info);
			    return (NORMAL);
			}
		        if (nodotfiles) {
			    if (*user_response == '.' ||
#ifdef VMS
			       ((cp = strrchr(user_response, ':')) &&
			       			*(cp+1) == '.') ||
			       ((cp = strrchr(user_response, ']')) &&
			       			*(cp+1) == '.') ||
#endif /* VMS */
			       ((cp = strrchr(user_response, '/')) &&
			       			*(cp+1) == '.')) {
			        statusline("File name may not begin with dot. Enter a new filename: ");
			        goto again;
			    }
		        }
		    }
#ifdef VMS
		    sprintf(buffer,cur_printer->command,filename,user_response);
#else
		    cp = quote_pathname(user_response); /* to prevent spoofing of the shell */
		    sprintf(buffer,cur_printer->command,filename,cp);
		    free(cp);
#endif /* VMS */

		} else {
		    _statusline("ERROR! - printer is misconfigured");
		    sleep(sleep_two);
		    free(sug_filename);
		    free(link_info);
		    return (NORMAL);
		}

		/* move the cursor to the top of the screen so that
		 * output from system'd commands don't scroll up 
                 * the screen
		 */
		move(1,1);

		stop_curses();
#ifndef VMS
//                signal(SIGINT, SIG_IGN);
#endif /* not VMS */

#ifdef DT
		if(TRACE)
		    fprintf(stderr,"command: %s\n",buffer);
#endif

		printf("Printing file.  Please wait...");
		system(buffer);
		fflush(stdout);
#ifndef VMS
//                signal(SIGINT, cleanup_sig);
#endif /* not VMS */
                sleep(sleep_two);
		start_curses();
		/* don't remove(filename); */
	} /* end switch */

     free(link_info);
     free(sug_filename);
     return(NORMAL);
}	

PRIVATE int remove_quotes ARGS1(char *,string)
{
   int i;

   for(i=0;string[i]!='\0';i++)
	if(string[i]=='"')
	   string[i]=' ';
	else if(string[i]=='&')
	   string[i]=' ';
	else if(string[i]=='|')
	   string[i]=' ';

   return(0);
}

/*
 * print_options writes out the current printer choices to a file
 * so that the user can select printers in the same way that
 * they select all other links 
 * printer links look like
 *  LYNXPRINT://LOCAL_FILE/lines=#  	     print to a local file
 *  LYNXPRINT://TO_SCREEN/lines=#   	     print to the screen
 *  LYNXPRINT://MAIL_FILE/lines=#   	     mail the file
 *  LYNXPRINT://PRINTER/lines=#/number=#   print to printer number #
 */

PUBLIC int print_options ARGS2(char **,newfile, int,lines_in_file)
{
    static char * tempfile=0;
    char * print_filename=0;
    char buffer[LINESIZE];
    int count;
    int pages;
    int pagelen;
    FILE *fp0;
    lynx_printer_item_type *cur_printer;

    pages = lines_in_file/66 + 1;

    if(tempfile == NULL) {
	tempfile = (char *) malloc(127);
        tempname(tempfile,NEW_FILE);
        /* make the file a URL now */
#ifndef VMS
    }
#else
    } else {
        remove(tempfile);   /* put VMS code to remove duplicates here */
    }
#endif /* VMS */

	StrAllocCopy(print_filename,"file://localhost/");

	StrAllocCat(print_filename,tempfile);

    if((fp0 = fopen(tempfile,"w")) == NULL) {
        _statusline("Unable to open print options file");
        sleep(sleep_two);
	return(0);
    }

    StrAllocCopy(*newfile, print_filename);
    LYforce_no_cache = TRUE;

    fprintf(fp0, "<head>\n<title>%s</title>\n</head>\n<body>\n",
    		 PRINT_OPTIONS_TITLE);

    fprintf(fp0,"<h1>Printing Options (%s Version %s)</h1>\n",
    				       LYNX_NAME, LYNX_VERSION);

    pages = (lines_in_file+65)/66;
    sprintf(buffer,
    	    "There are %d lines, or approximately %d page%s, to print.<br>\n",
    	    lines_in_file, pages, (pages > 1 ? "s" : ""));
    fputs(buffer,fp0);

    if(no_print || child_lynx || no_mail)
	fputs("Some print functions have been disabled!!!<br>\n", fp0);

    fputs("You have the following print choices.<br>\n", fp0);
    fputs("Please select one:<br>\n<pre>\n", fp0);

    if(child_lynx==FALSE && no_print==FALSE)
         fprintf(fp0, "   <a href=\"LYNXPRINT://LOCAL_FILE/lines=%d\">Save to a local file</a>\n",
	 	      lines_in_file);
    if(child_lynx==FALSE && no_mail==FALSE)
         fprintf(fp0, "   <a href=\"LYNXPRINT://MAIL_FILE/lines=%d\">Mail the file</a>\n",
		      lines_in_file);
    fprintf(fp0, "   <a href=\"LYNXPRINT://TO_SCREEN/lines=%d\">Print to the screen</a>\n",
	 	      lines_in_file);

    for(count=0, cur_printer=printers; cur_printer != NULL; 
        cur_printer = cur_printer->next, count++)
    if(no_print==FALSE || cur_printer->always_enabled) {
        fprintf(fp0,
                "   <a href=\"LYNXPRINT://PRINTER/number=%d/pagelen=%d/lines=%d\">",
                count,cur_printer->pagelen,lines_in_file);
	fprintf(fp0, (cur_printer->name ? 
				cur_printer->name : "No Name Given"));
	fprintf(fp0, "</a>\n");
    }
    fprintf(fp0, "</pre>\n</body>\n");
    fclose(fp0);

    LYforce_no_cache=TRUE;
    return(0);
}
