#include "HTUtils.h"
#include "LYUtils.h"
#include "LYString.h"
#include "LYBookma.h"
#include "LYGlobal.h"
#include "LYSignal.h"
#include "LYSystem.h"
#include "LYKeymap.h"

#include <time.h>
#include <curses.h>
// #include <dos.h>

#ifndef VMS
#include <errno.h>
#ifndef MSDOS
#include <unistd.h>
#endif /* msdos */
#endif /* VMS */

#ifdef VMS
#include <nam.h>
#endif /* VMS */

#include "LYLeaks.h"

PRIVATE BOOLEAN is_mosaic_hotlist=FALSE;
PRIVATE char * convert_mosaic_bookmark_file PARAMS((char *filename_buffer));

/* tries to open the bookmark file for reading.
 * if successful the file is closed and the filename
 * is returned and the URL is given in name
 */
PUBLIC char * get_bookmark_filename ARGS1(char **,URL)
{
    char URL_buffer[256];
    static char filename_buffer[256];
    char string_buffer[256];
    FILE *fp;

    if(!bookmark_page) {
	sprintf(string_buffer,
		"Bookmark file is not defined. Use %s to see options.",
		key_for_func(LYK_OPTIONS));
	_statusline(string_buffer);
	sleep(sleep_two);
	return(NULL);
    }

    /* see if it is in the home path */

    if (NULL != getenv("HOME"))
    {
        sprintf(filename_buffer,"%s/%s",getenv("HOME"), bookmark_page);
        if((fp = fopen(filename_buffer,"r")) != NULL) {
            goto success;
        }
    }

    /* check the start directory */

    {
        sprintf(filename_buffer,"%s/%s",cdirbuffer, bookmark_page);
        if((fp = fopen(filename_buffer,"r")) != NULL) {
            goto success;
        }
    }

#ifdef FIXME
    /* see if we can open it raw */
    if((fp = fopen(bookmark_page,"r")) != NULL) {
	strcpy(filename_buffer, bookmark_page);
	goto success;
    }
#endif

    /* failure */
    return(NULL);

success:
    /* we now have the file open.  Check if it is a mosaic
     * hotlist
     */
    if(fgets(string_buffer, 255, fp) &&
		!strncmp(string_buffer, "ncsa-xmosaic-hotlist-format-1", 29)) {
	char * newname;
	/* it is a mosaic hotlist file */
	is_mosaic_hotlist=TRUE;
	fclose(fp);
	newname = convert_mosaic_bookmark_file(filename_buffer);

    sprintf(URL_buffer,"file://localhost/%s", filename_buffer);

    } else {
	fclose(fp);
	is_mosaic_hotlist=FALSE;

    sprintf(URL_buffer,"file://localhost/%s", filename_buffer);
    }

    StrAllocCopy(*URL, URL_buffer);
    return(filename_buffer);  /* bookmark file exists */

} /* big end */

PRIVATE char * convert_mosaic_bookmark_file ARGS1(char *,filename_buffer)
{
    static char *newfile=NULL;
    FILE *fp, *nfp;
    char buf[BUFSIZ];
    int line= -2;
    char *endline;

    if(newfile == NULL) {
	newfile = (char *) malloc(128);
        tempname(newfile, NEW_FILE);
#ifndef VMS
    } /* otherwise reuse the existing tempfile */
#else
    } else {
        remove(newfile);   /* put VMS code to remove duplicates here */
    }
#endif /* VMS */


    if((nfp = fopen(newfile, "w")) == NULL) {
	_statusline("Unable to open tempfile for X Mosaic hotlist conversion.");
	sleep(sleep_two);
	return ("");
    }

    if((fp = fopen(filename_buffer, "r")) == NULL)
	return ("");  /* should always open */

    fprintf(nfp,"<head>\n<title>%s</title>\n</head>\n",MOSAIC_BOOKMARK_TITLE);
    fprintf(nfp,"\
     This file is an HTML representation of the X Mosaic hotlist file.\n\
     Outdated or invalid links may be removed by using the\n\
     remove bookmark command, it is usually the 'R' key but may have\n\
     been remapped by you or your system administrator.\n\n<p>\n<ol>\n");

    while (fgets(buf, sizeof(buf), fp) != NULL) {
	if(line >= 0) {
	    endline = &buf[strlen(buf)-1];
	    if(*endline == '\n')
		*endline = '\0';
	    if((line % 2) == 0) { /* even lines */
		if(*buf != '\0') {
		    strtok(buf," "); /* kill everything after the space */
	            fprintf(nfp,"<LI><a href=\"%s\">",buf); /* the URL */
		}
	    } else { /* odd lines */
	        fprintf(nfp,"%s</a>\n",buf);  /* the title */
	    }
	} 
	/* else - ignore the line (this gets rid of first two lines) */
	line++;
    }
    fclose(nfp);
    fclose(fp);
    return(newfile);
}

PUBLIC void save_bookmark_link ARGS2(char *,address, char *,title)
{
	FILE *fp;
	BOOLEAN first_time=FALSE;
	char * filename;
	char * bookmark_URL=NULL;
	char filename_buffer[256];
	char * Title=NULL;
	char *cp, *cp1;

	filename = get_bookmark_filename(&bookmark_URL);
	if(bookmark_URL)
	    free(bookmark_URL); /* don't need it */
	if(!bookmark_page)
	    return;

	if(filename == NULL) {
	    first_time= TRUE;
	    /* try in the home directory first */


    if (NULL != getenv("HOME"))
        sprintf(filename_buffer,"%s/%s",getenv("HOME"), bookmark_page);
    else
        sprintf(filename_buffer,"%s/%s", cdirbuffer, bookmark_page);

    	    if((fp = fopen(filename_buffer,"w")) == NULL) {
	            _statusline("ERROR - unable to open bookmark file.");
	            sleep(sleep_two);
	            return;
	    }

	} else {
	    if((fp = fopen(filename,"a+")) == NULL) {
	       _statusline("ERROR - unable to open bookmark file.");
	       sleep(sleep_two);
	       return;
	    }
	}

	if ((cp=strchr(title, '<')) != NULL) {
	    *cp = '\0';
	    StrAllocCopy(Title, title);
	    StrAllocCat(Title, "&lt;");
	    *cp = '<';
	    cp1 = (cp+1);
	    while ((cp=strchr(cp1, '<')) != NULL) {
	        *cp = '\0';
		StrAllocCat(Title, cp1);
		StrAllocCat(Title, "&lt;");
		*cp = '<';
		cp1 = (cp+1);
	    }
	    StrAllocCat(Title, cp1);
	} else {
	    StrAllocCopy(Title, title);
	}

	if(first_time) {
	    fprintf(fp,"<head>\n<title>%s</title>\n</head>\n",BOOKMARK_TITLE);
	    fprintf(fp,"\
     You can delete links using the new remove bookmark command.\n\
     it is usually the 'R' key but may have been remapped by you or\n\
     your system administrator.<br>\n\
     This file may also be edited with a standard text editor.\n\
     Outdated or invalid links may be removed by simply deleting\n\
     the line the link appears on in this file.\n\
     Please refer to the Lynx documentation or help files\n\
     for the HTML link syntax.\n\n<p>\n<ol>\n");
	}

	if(is_mosaic_hotlist) {
	    time_t NowTime = time (NULL);
	    char *TimeString = (char *)ctime (&NowTime);
		/* TimeString has a \n at the end */
	    fprintf(fp,"%s %s%s\n", address, TimeString, Title);
	} else {
	    fprintf(fp,"<LI><a href=\"%s\">%s</a>\n",address, Title);
	}

	fclose(fp);
	if (Title)
	    free(Title);

	_statusline("Done!");
	sleep(sleep_two);
}

PUBLIC void remove_bookmark_link ARGS1(int,cur)
    {
	FILE *fp, *nfp;
	char buf[BUFSIZ];
	int n;
#ifdef VMS
	char newfile[NAM$C_MAXRSS+12];
#else
	char newfile[128];
#endif /* VMS */
	char *filename;
	char *URL=0;

#ifdef DT
        if(TRACE)
	    fprintf(stderr,"remove_bookmark_link: deleting link number: %d\n",
									  cur);
#endif


	filename = get_bookmark_filename(&URL);
	if(URL)
	   free(URL); /* don't need it */
	if(!bookmark_page)
	    return;

	if((!filename) || (fp=fopen(filename, "r")) == NULL) {
		_statusline("Unable to open bookmark file for deletion of link.");
		sleep(sleep_two);
		return;
	}

	tempname(newfile, NEW_FILE);

	if((nfp = fopen(newfile, "w")) == NULL)	{
		fclose(fp);
		_statusline("Unable to open temporary file for deletion of link.");
		sleep(sleep_two);
		return;
	}

	if(is_mosaic_hotlist) {
	    int del_line = cur*2;  /* two lines per entry */
	    n = -3;  /* skip past cookie and name lines */
	    while (fgets(buf, sizeof(buf), fp) != NULL) {
		n++;
		if(n == del_line || n == del_line+1)
		    continue;  /* remove two lines */
		if (fputs(buf, nfp) == EOF)
			goto failure;
	    }

	} else {
	    n = -1;
	    while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (n < cur && LYstrstr(buf, "<a href=")) {
			if (++n == cur)
				continue;
		}
		if (fputs(buf, nfp) == EOF)
			goto failure;
	    }
	}

#ifdef DT
	if(TRACE)
	    fprintf(stderr,"remove_bookmark_link: files: %s %s\n",
							newfile, filename);
#endif


	fclose(fp);
	fp = NULL;
	fclose(nfp);
	nfp = NULL;

	unlink(filename);

{
   FILE *in, *out;
   unsigned char dc;
   in = fopen(newfile, "rb");

   out = fopen(filename, "wb");

   dc = fgetc(in);

   while (!feof(in))
   {
      fputc(dc, out);
      dc = fgetc(in);
   }

   fclose(in);
   fclose(out);
}

   return;


#ifdef FIXME
	if(rename(newfile, filename) != -1) {
		return;
	} else {

		_statusline("Error renaming temporary file.");
#ifdef DT
		if(TRACE)
			perror("renaming the file");
#endif

	    	sleep(sleep_two);
	}
#endif //FIXME
	   
      failure:
        _statusline("Bookmark deletion failed.");
        sleep(sleep_two);
	if(nfp != NULL)	{
		fclose(nfp);
	}
	if(fp != NULL)	{
		fclose(fp);
	}
#ifdef SCO
	unlink(newfile);
#else
	remove(newfile);
#endif /* SCO */
    }


int external2(char *b, int c, char *d, int e)
{

    char *command = NULL;
    int that=0;
    lynx_extern_item_type *auto_extern_type=0;

    for(auto_extern_type=auto_externs; (that == 0)&&(auto_extern_type != NULL);
	auto_extern_type=auto_extern_type->next)
    {
	that = !strnicmp(auto_extern_type->name,d,strlen(auto_extern_type->name));
    }

    if (!that) return(0);

    StrAllocCopy(command, "external ");
    StrAllocCat(command,d);

    if(command != NULL)
    {
	stop_curses();
	clrscr();
	system(command);
	start_curses();
    }

    return (1);

}

void external(char *b, int c, char *d, int e)
{

    char cc, *command = NULL;

	if(e)
	{
	    _statusline("Use current D)ocument, L)ink or C)ancel? (d,l,c): ");
	    cc = LYgetch();
	    if(TOUPPER(cc) == 'D')
	    {
		StrAllocCopy(command, "external ");
		StrAllocCat(command,b);
	    }
	    else if(TOUPPER(cc) == 'L')
		if(c != WWW_FORM_LINK_TYPE)
		{
		    StrAllocCopy(command, "external ");
		    StrAllocCat(command,d);
		}
		else
		{
		    _statusline("Cannot use form fields/links");
		    sleep(sleep_two);
		    return;
		}
	}
	else
	{
	    _statusline("Use current D)ocument or C)ancel? (d,c): ");
	    cc = LYgetch();
	    if(TOUPPER(cc) == 'D')
	    {
		StrAllocCopy(command, "external ");
		StrAllocCat(command,b);
	    }
	}

	if(command != NULL)
	{
	    stop_curses();
	    clrscr();
	    system(command);
	    start_curses();
	}

}

