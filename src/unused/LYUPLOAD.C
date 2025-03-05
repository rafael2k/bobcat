/* Routines to upload files to the local filesystem */
/* Created by: Rick Mallett, Carleton University */
/* Report problems to rmallett@ccs.carleton.ca */

#include "HTUtils.h"
#include "HTParse.h"
#include "LYCurses.h"
#include "LYUtils.h"
#include "LYGlobal.h"
#include "LYSignal.h"
#include "LYString.h"
#include "LYClean.h"
#include "LYGetFil.h"
#include "LYUpload.h"
#include "LYSystem.h"
#include "LYLocal.h"

#include "LYexit.h"
#include "LYLeaks.h"

/*
 *  LYUpload uploads a file to a given location using a 
 *  specified upload method.
 */

/* it parses an incoming link that looks like
 *
 *  LYNXDIRED://UPLOAD=<#>/TO=<STRING>
 */

PUBLIC void LYUpload ARGS1(char *,line) 
{
    char *method, *directory;
    int method_number;
    int count;
    char tmpbuf[256];
    char buffer[256];
    lynx_html_item_type *upload_command=0;
    char c;
    char *cp;
    FILE *fp;
#ifdef VMS
    extern BOOLEAN HadVMSInterrupt;
#endif /* VMS */

    /* parse out the Method and the Location */

    if((directory = (char *)strstr(line, "TO=")) == NULL)
	goto failed;
    *(directory-1) = '\0';
    /* go past "Directory=" */
    directory+=3;

    if((method = (char *)strstr(line, "UPLOAD=")) == NULL)
	goto failed;
    /* go past "Method=" */
    method+=7;
    method_number = atoi(method);

    _statusline("Enter a filename: ");
retry:
    *tmpbuf = '\0';
    if(LYgetstr(tmpbuf,0)==-1)
       goto cancelled;

    if(*tmpbuf=='\0')
       goto cancelled;

    if (strstr(tmpbuf,"../") != NULL) {
       _statusline("Illegal redirection \"../\" found! Request ignored. ");
       sleep(sleep_three);
       goto cancelled;
    } else if(strchr(tmpbuf,'/') != NULL) {
       _statusline("Illegal character \"/\" found! Request ignored. ");
       sleep(sleep_three);
       goto cancelled;
    } else if (tmpbuf[0] == '~') {
       _statusline("Illegal redirection using \"~\" found! Request ignored. ");
       sleep(sleep_three);
       goto cancelled;
    }
    sprintf(buffer,"%s/%s",directory,tmpbuf);

    if (nodotfiles) {
	if (*buffer == '.' ||
#ifdef VMS
	    ((cp = strrchr(buffer, ':')) && *(cp+1) == '.') ||
	    ((cp = strrchr(buffer, ']')) && *(cp+1) == '.') ||
#endif /* VMS */
	    ((cp = strrchr(buffer, '/')) && *(cp+1) == '.')) {
	    _statusline(
	    	"File name may not begin with dot. Enter a new filename: ");
	    goto retry;
	}
    }

	/* see if it already exists */
    if((fp = fopen(buffer,"r")) != NULL) {
        fclose(fp);

#ifdef VMS
	_statusline("File exists. Create higher version? (y/n)");
#else
	_statusline("File exists. Overwrite? (y/n)");
#endif /* VMS */
	c = 0;
	while(TOUPPER(c)!='Y' && TOUPPER(c)!='N' && c != 7 && c != 3)
	  c = LYgetch();
#ifdef VMS
	if(HadVMSInterrupt) {
	   HadVMSInterrupt = FALSE;
	   return;
	}
#endif /* VMS */

	if(c == 7 || c == 3) { /* Control-G or Control-C */
	    goto cancelled;
	}

	if(TOUPPER(c) == 'N') {
	   _statusline("Enter a filename: ");
	   goto retry;
	}
     }

     /* see if we can write to it */

    if((fp = fopen(buffer,"w")) != NULL) {
       fclose(fp);
#ifdef SCO
       unlink(buffer);
#else
       remove(buffer);
#endif /* SCO */
    } else {
       _statusline("Cannot write to file. Enter a new filename: ");
       goto retry;
    }

    /* use configured upload commands */
    
    for(count=0, upload_command=uploaders; count < method_number;
	count++, upload_command = upload_command->next)
      ; /* null body */

	/* commands have the form "command %s [etc]"
	 * where %s is the filename
	 */
        if(upload_command->command != NULL) {
#ifdef VMS
	   sprintf(tmpbuf,upload_command->command,buffer);
#else
	   cp = quote_pathname(buffer); /* to prevent spoofing of the shell */
	   sprintf(tmpbuf,upload_command->command,cp);
	   free(cp);
#endif /* VMS */
        } else {
            _statusline("ERROR! - upload command is misconfigured");
	    sleep(sleep_two);
	    goto failed;
        }

        stop_curses();
#ifndef VMS
//        signal(SIGINT, SIG_IGN);
#endif /* not VMS */
#ifdef DT
        if(TRACE)
            fprintf(stderr,"command: %s\n",tmpbuf);
#endif

        system(tmpbuf);
        fflush(stdout);
#ifndef VMS
        signal(SIGINT, cleanup_sig);
#endif /* not VMS */
        start_curses();
        /* don't remove(file); */

    return;

failed:

    _statusline("Unable to upload file");
    sleep(sleep_two);
    return;

cancelled:

    _statusline("Cancelling");
    return;

}	

/*
 * LYUpload_options writes out the current upload choices to a file
 * so that the user can select printers in the same way that
 * they select all other links 
 * upload links look like
 *  LYNXDIRED//UPLOAD=<#>/TO=<STRING>
 */

PUBLIC int LYUpload_options ARGS2 (char **,newfile, char *,directory)
{
    char upload_filename[256];
    static char *tempfile=NULL;
    FILE *fp0;
    lynx_html_item_type *cur_upload;
    char *last_slash;
    int count;
    static char curloc[256];
    char *cp;

    if(tempfile == NULL) {
	tempfile = (char *) malloc(127);
        tempname(tempfile,NEW_FILE);
#ifndef VMS
    } 
#else
    } else {
        remove(tempfile);   /* put VMS code to remove duplicates here */
    }
#endif /* VMS */

#ifdef VMS
    strcpy(curloc, "/sys$login");
#else
    cp = directory;
    if(!strncmp(cp,"file://localhost",16))
        cp += 16;
    else if(!strncmp(cp,"file:",5))
        cp += 5;
    strcpy(curloc,cp);
    HTUnEscape(curloc);
    if (curloc[strlen(curloc)-1] == '/')
        curloc[strlen(curloc)-1] = '\0';
#endif /* VMS */

    if((fp0 = fopen(tempfile,"w")) == NULL) {
        perror("Trying to open upload options file\n");
        exit(1);
    }

    /* make the file a URL now */

    sprintf(upload_filename,"file://localhost/%s",tempfile);

    StrAllocCopy(*newfile, upload_filename);

    fprintf(fp0,"<head>\n<title>%s</title>\n</head>\n<body>\n",
    		UPLOAD_OPTIONS_TITLE);

    fprintf(fp0,"<h1>Upload Options (%s Version %s)</h1>\n",
    				     LYNX_NAME, LYNX_VERSION);


    fputs("You have the following upload choices.<br>\n", fp0);
    fputs("Please select one:<br>\n<pre>\n", fp0);

    if(uploaders != NULL) {

       for(count=0, cur_upload=uploaders; cur_upload != NULL; 
	   cur_upload = cur_upload->next, count++) {

	  fprintf(fp0, "   <a href=\"LYNXDIRED://UPLOAD=%d/TO=%s\">",
	  	       count,curloc);

	  fprintf(fp0, (cur_upload->name ? 
			cur_upload->name : "No Name Given"));
	  fprintf(fp0, "</a>\n");
	}
    } else {
	fprintf(fp0, "\n   \
No other upload methods have been defined yet.  You may define\n   \
an unlimited number of upload methods using the lynx.cfg file.\n");

    }
    fprintf(fp0, "</pre>\n</body>\n");
    fclose(fp0);

    return(0);
}



