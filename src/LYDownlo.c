#include <unistd.h>

#include "HTUtils.h"
#include "LYCurses.h"
#include "LYUtils.h"
#include "LYGlobalDefs.h"
#include "LYSignal.h"
#include "LYString.h"
#include "LYClean.h"
#include "LYGetFil.h"
#include "LYDownlo.h"
#include "LYSystem.h"

#include "LYexit.h"
#include "LYLeaks.h"

/*
 *  LYDownload takes a URL and downloads it using a user selected
 *  download program
 */

/* it parses an incoming link that looks like
 *
 *  LYNXDOWNLOAD://Method=<#>/File=<STRING>/SugFile=<STRING>
 */

PUBLIC void LYDownload ARGS1(char *,line)
{
    char *method, *file, *sug_file=0;
    int method_number;
    int count;
    char buffer[256];
    char command[256];
    char *cp;
    lynx_html_item_type *download_command=0;
    char c;
    FILE *fp;
#ifdef VMS
    extern BOOLEAN HadVMSInterrupt;
#endif /* VMS */


    /* parse out the sug_file, Method and the File */
    if((sug_file = (char *)strstr(line, "SugFile=")) != NULL) {
	*(sug_file-1) = '\0';
	/* go past "SugFile=" */
        sug_file+=8;
    }
	

    if((file = (char *)strstr(line, "File=")) == NULL)
	goto failed;
    *(file-1) = '\0';
    /* go past "File=" */
    file+=5;

#ifdef DIRED_SUPPORT
    if(!strncmp(file,"file://localhost",16))
	file += 16;
    else if(!strncmp(file,"file:",5))
        file += 5;
    HTUnEscape(file);
#endif

    if((method = (char *)strstr(line, "Method=")) == NULL)
	goto failed;
    /* go past "Method=" */
    method+=7;
    method_number = atoi(method);


    if(method_number < 0) {
	/* write to local file */
retry:	
	_statusline("Enter a filename: ");
	
        if(incoming)
        {
           strcpy(buffer,incoming);
           if(sug_file)
              strcat(buffer,sug_file);
        } else

	if(sug_file)
	   strcpy(buffer,sug_file);
	else
	   *buffer = '\0';

	if(LYgetstr(buffer,0)==-1)
	   goto cancelled;

	if(*buffer=='\0')
	   goto cancelled;

	if (nodotfiles) {
	  if (*buffer == '.' ||
	      ((cp = strrchr(buffer, '/')) && *(cp+1) == '.')) {
		_statusline("File name may not begin with dot. Enter a new filename: ");
		goto retry;
	  }
	}

	/* see if it already exists */
	if((fp = fopen(buffer,"r")) != NULL) {
	    fclose(fp);

            _statusline("File exists. Overwrite? (y/n)  Ctrl-c to cancel.");
	    c = 0;
	    while(TOUPPER(c)!='Y' && TOUPPER(c)!='N' && c != 7 && c != 3)
		c = LYgetch();

	    if(c == 7 || c == 3) { /* Control-G or Control-C */
		goto cancelled;
	    }

	    if(TOUPPER(c) == 'N') {
		goto retry;
	    }
	}

	/* see if we can write to it */
	if((fp = fopen(buffer,"w")) != NULL) {
	    fclose(fp);
	    remove(buffer);
	} else {
	    _statusline("Cannot write to file. Enter a new filename: ");
	    goto retry;
	}

	_statusline("Saving.....");

{
   FILE *in, *out;
   unsigned char dc;
   in = fopen(file, "rb");

   out = fopen(buffer, "wb");

   dc = fgetc(in);

   while (!feof(in))
   {
      fputc(dc, out);
      dc = fgetc(in);
   }

   fclose(in);
   fclose(out);
}
//	cp = quote_pathname(buffer); /* to prevent spoofing of the shell */
//	sprintf(command,"%s %s %s", COPY_PATH,file,cp);
//	free(cp);

#ifdef DT
	if(TRACE)
	    fprintf(stderr,"command: %s\n",command);
#endif

//        system(command);

    } else {

	/* use configured download commands */

	buffer[0] = '\0';
	for(count=0, download_command=downloaders; count < method_number;
			    count++, download_command = download_command->next)
	    ; /* null body */

	/* commands have the form "command %s [etc]"
	 * where %s is the filename
	 */
	if(download_command->command != NULL) {

	    /* check for two '%s' and ask for the local filename if
	     * there is
	     */
	    char *first_s = strstr(download_command->command, "%s");
	    if(first_s && strstr(first_s+1, "%s")) {
		_statusline("Enter a filename: ");
	again:	strcpy(buffer,sug_file);
		if(LYgetstr(buffer,0)==-1 || *buffer == '\0')
		    goto failed;
		if (nodotfiles) {
		    if (*buffer == '.' ||
#ifdef VMS
		       ((cp = strrchr(buffer, ':')) && *(cp+1) == '.') ||
		       ((cp = strrchr(buffer, ']')) && *(cp+1) == '.') ||
#endif /* VMS */
		       ((cp = strrchr(buffer, '/')) && *(cp+1) == '.')) {
			statusline("File name may not begin with dot. Enter a new filename: ");
			goto again;
		    }
		}
	    }

	    /*  The following is considered a bug by the community.
	     *  If the command only takes one argument on the command
	     *  line, then the suggested file name is not used.
	     *  It actually is not a bug at all and does as it should,
	     *  putting both names on the command line.
	     */
#ifdef VMS
	    sprintf(command,download_command->command,file,buffer);
#else
	    cp = quote_pathname(buffer); /* to prevent spoofing of the shell */
	    sprintf(command,download_command->command,file,cp);
	    free(cp);
#endif /* VMS */

	} else {
	    _statusline("ERROR! - download command is misconfigured");
	    sleep(sleep_two);
	    goto failed;
	}

	stop_curses();
#ifndef VMS
//        signal(SIGINT, SIG_IGN);
#endif /* not VMS */
#ifdef DT
        if(TRACE)
            fprintf(stderr,"command: %s\n",command);
#endif

        system(command);
	fflush(stdout);
#ifndef VMS
//        signal(SIGINT, cleanup_sig);
#endif /* not VMS */
	start_curses();
	/* don't remove(file); */

    }

    return;

failed:
    _statusline("Unable to download file");
    sleep(sleep_two);
    return;

cancelled:

    _statusline("Cancelling");
    return;

}	

/*
 * LYdownload_options writes out the current download choices to a file
 * so that the user can select printers in the same way that
 * they select all other links 
 * download links look like
 *  LYNXDOWNLOAD://Method=<#>/File=<STRING>/SugFile=<STRING>
 */

PUBLIC int LYdownload_options ARGS2(char **,newfile, char *,data_file)
{
    char download_filename[256];
    static char *tempfile=0;
    char *sug_filename=0;
    FILE *fp0;
    lynx_html_item_type *cur_download;
    char *last_slash;
    int count;

    if(!tempfile) {
	tempfile = (char *) malloc(127);
        tempname(tempfile,NEW_FILE);
#ifndef VMS
    } 
#else
    } else {
        remove(tempfile);   /* put VMS code to remove duplicates here */
    }
#endif /* VMS */

    /* get a suggested filename */
    StrAllocCopy(sug_filename,*newfile);
    change_sug_filename(sug_filename);

    if((fp0 = fopen(tempfile,"w")) == NULL) {
	perror("Trying to open download options file\n");
        exit(1);
    }

    /* make the file a URL now */

    sprintf(download_filename,"file://localhost/%s",tempfile);

    StrAllocCopy(*newfile, download_filename);
    LYforce_no_cache = TRUE;  /* don't cache this doc */

    fprintf(fp0, "<head>\n<title>%s</title>\n</head>\n<body>\n",
    		 DOWNLOAD_OPTIONS_TITLE);

    fprintf(fp0,"<h1>Download Options (%s Version %s)</h1>\n",
    				       LYNX_NAME, LYNX_VERSION);


    fputs("You have the following download choices.<br>\n", fp0);
    fputs("Please select one:<br>\n<pre>\n", fp0);

    if(!no_disk_save)
#ifdef DIRED_SUPPORT
	/* disable save to disk option for local files */
	if (!lynx_edit_mode)
#endif /* DIRED_SUPPORT */
            fprintf(fp0,"   \
<a href=\"LYNXDOWNLOAD://Method=-1/File=%s/SugFile=%s\">Save to disk</a>\n",
						data_file, sug_filename);
#ifdef DIRED_SUPPORT
	else {}
#endif /* DIRED_SUPPORT */
    else
	fprintf(fp0,"   Save to disk disabled.\n");

    if(downloaders != NULL) {

        for(count=0, cur_download=downloaders; cur_download != NULL; 
			    cur_download = cur_download->next, count++) {

	    if(!no_download || cur_download->always_enabled) {
	        fprintf(fp0,"   \
<a href=\"LYNXDOWNLOAD://Method=%d/File=%s/SugFile=%s\">", 
				count,data_file,sug_filename);

		fprintf(fp0, (cur_download->name ? 
				cur_download->name : "No Name Given"));
		fprintf(fp0,"</a>\n");
	    }
	}
    } else {
	fprintf(fp0, "\n   \
No other download methods have been defined yet.  You may define\n   \
an unlimited number of download methods using the lynx.cfg file.\n");

    }
    fprintf(fp0, "</pre>\n</body>\n");
    fclose(fp0);

    /* free off temp copy */
    free(sug_filename);

    return(0);
}
