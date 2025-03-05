#include <unistd.h>

#include "HTUtils.h"
#include "LYCurses.h"
#include "LYString.h"
#include "LYUtils.h"
#include "LYStruct.h"
#include "LYGlobalDefs.h"
#include "LYShowIn.h"
#include "LYSignal.h"

#ifdef DIRED_SUPPORT
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "LYLocal.h"
#endif /* DIRED_SUPPORT */

#include "LYLeaks.h"

/* 
 * showinfo prints a page of info about the current file and the link
 * that the cursor is on
 */
	     
PUBLIC int showinfo ARGS4(document *,doc, int,size_of_file, document *,newdoc,
							char *,owner_address)
{
	static char * tempfile=NULL;
        char * print_url=0;
	int url_type;
	FILE *fp0;
	char *Title=NULL;
	char *cp, *cp1;

#ifdef DIRED_SUPPORT
	char temp[300];
	struct tm *tm;
	struct stat dir_info;
	struct passwd *pw;
	struct group *grp;
#endif
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


            StrAllocCopy(print_url,"file://localhost/");

            StrAllocCat(print_url,tempfile);

        if((fp0 = fopen(tempfile,"w")) == NULL) {
            _statusline("Unable to open print options file");
            sleep(sleep_two);
            return(0);
        }

	newdoc->address = print_url; /*point the address pointer at this Url */


	if(nlinks > 0 && links[doc->link].lname != NULL &&
	   (url_type = is_url(links[doc->link].lname)) != 0 &&
	   url_type == LYNXEXEC_URL_TYPE) {
	    char *last_slash = strrchr(links[doc->link].lname,'/');
	    if(last_slash-links[doc->link].lname ==
	    	   strlen(links[doc->link].lname)-1)
	        links[doc->link].lname[strlen(links[doc->link].lname)-1] = '\0';
	    }

	fprintf(fp0,"<head>\n<title>%s</title>\n</head>\n<body>\n",
		    SHOWINFO_TITLE);
	fprintf(fp0,"<h1>YOU HAVE REACHED THE INFORMATION PAGE</h1>\n");
	fprintf(fp0,"<h2>%s Version %s</h2>\n", LYNX_NAME, LYNX_VERSION);

#ifdef DIRED_SUPPORT	
	if (lynx_edit_mode) {
	   fprintf(fp0,
	   	   "<h2>Directory that you are currently viewing</h2>\n<pre>");

	   cp = doc->address;
	   if(!strncmp(cp,"file://localhost",16)) 
	      cp += 16;
	   else if(!strncmp(cp,"file:",5))
	      cp += 5;
	   strcpy(temp,cp);
	   HTUnEscape(temp);

	   fprintf(fp0,"   Name:  %s\n",temp);
	   fprintf(fp0,"    URL:  %s\n",doc->address);

	   cp = links[doc->link].lname;
	   if(!strncmp(cp,"file://localhost",16)) 
	     cp += 16;
	   else if(!strncmp(cp,"file:",5))
	     cp += 5;
	   strcpy(temp,cp);
	   HTUnEscape(temp);
	   if (lstat(temp,&dir_info) == -1) {
	      _statusline("Failed to obtain status of current link!");
	      sleep(sleep_two);
	   } else {
	      char modes[80];
	      if (((dir_info.st_mode) & S_IFMT) == S_IFDIR) {
		 fprintf(fp0,"\nDirectory that you have currently selected\n\n");
	      } else if (((dir_info.st_mode) & S_IFMT) == S_IFREG) {
		 fprintf(fp0,"\nFile that you have currently selected\n\n");
	      } else if (((dir_info.st_mode) & S_IFMT) == S_IFLNK) {
		 fprintf(fp0,"\nSymbolic link that you have currently selected\n\n");
	      } else {
		 fprintf(fp0,"\nItem that you have currently selected\n\n");
	      }
	      fprintf(fp0,"       Full name:  %s\n",temp);
	      if (((dir_info.st_mode) & S_IFMT) == S_IFLNK) {
		  char buf[1025];
		  int buf_size;

		  if ((buf_size = readlink(temp, buf, sizeof(buf)-1)) != -1) {
		      buf[buf_size] = '\0';
		  } else {
		      strcpy(buf, "Unable to follow link");
		  }
		  fprintf(fp0, "  Points to file:  %s\n", buf);
	      }
	      pw = getpwuid(dir_info.st_uid);
 	      fprintf(fp0,"   Name of owner:  %s\n",pw->pw_name);
	      grp = getgrgid(dir_info.st_gid);
	      fprintf(fp0,"      Group name:  %s\n",grp->gr_name);
	      if (((dir_info.st_mode) & S_IFMT) == S_IFREG) {
		 sprintf(temp,"       File size:  %d (bytes)\n",dir_info.st_size);
		 fprintf(fp0,"%s",temp);
	      }
/*
   Include date and time information
*/
	      cp = ctime(&dir_info.st_ctime);
	      fprintf(fp0,"   Creation date:  %s",cp);

	      cp = ctime(&dir_info.st_mtime);	      
	      fprintf(fp0,"   Last modified:  %s",cp);

	      cp = ctime(&dir_info.st_atime);
	      fprintf(fp0,"   Last accessed:  %s\n",cp);

	      fprintf(fp0,"   Access Permissions\n");
	      fprintf(fp0,"      Owner:  ");
	      modes[0] = '\0';
	      modes[1] = '\0';   /* In case there are no permissions */
	      modes[2] = '\0';
	      if ((dir_info.st_mode & S_IRUSR))
		strcat(modes, ", read");
	      if ((dir_info.st_mode & S_IWUSR))
		strcat(modes,", write");
	      if ((dir_info.st_mode & S_IXUSR)) {
		if (((dir_info.st_mode) & S_IFMT) == S_IFDIR) 
		  strcat(modes,", search");
	        else {
		  strcat(modes,", execute");
		  if ((dir_info.st_mode & S_ISUID))
		    strcat(modes,", setuid");
	        }
	      }
	      fprintf(fp0,"%s\n", (char *)&modes[2]); /* Skip leading ', ' */

	      fprintf(fp0,"      Group:  ");
	      modes[0] = '\0';
	      modes[1] = '\0';   /* In case there are no permissions */
	      modes[2] = '\0';
	      if ((dir_info.st_mode & S_IRGRP)) 
		strcat(modes,", read");
	      if ((dir_info.st_mode & S_IWGRP))
		strcat(modes,", write");
	      if ((dir_info.st_mode & S_IXGRP)) {
		if (((dir_info.st_mode) & S_IFMT) == S_IFDIR) 
		  strcat(modes,", search");
	        else {
		  strcat(modes,", execute");
		  if ((dir_info.st_mode & S_ISGID))
		    strcat(modes,", setgid");
	        }
	      }
	      fprintf(fp0,"%s\n", (char *)&modes[2]);  /* Skip leading ', ' */

	      fprintf(fp0,"      World:  ");
	      modes[0] = '\0';
	      modes[1] = '\0';   /* In case there are no permissions */
	      modes[2] = '\0';
	      if ((dir_info.st_mode & S_IROTH))
		strcat(modes,", read");
	      if ((dir_info.st_mode & S_IWOTH))
		strcat(modes,", write");
	      if ((dir_info.st_mode & S_IXOTH)) {
		if (((dir_info.st_mode) & S_IFMT) == S_IFDIR) 
		  strcat(modes,", search");
	        else {
		  strcat(modes,", execute");
		  if ((dir_info.st_mode & S_ISVTX))
		    strcat(modes,", sticky");
	        }
	      }
	      fprintf(fp0,"%s\n", (char *)&modes[2]);  /* Skip leading ', ' */
	   }
	   fprintf(fp0,"</pre>\n");
	} else {
#endif	

	fprintf(fp0,"<h2>File that you are currently viewing</h2>\n<dl compact>");

	if ((cp=strchr(doc->title, '<')) != NULL) {
	    *cp = '\0';
	    StrAllocCopy(Title, doc->title);
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
	    StrAllocCopy(Title, doc->title);
	}
	fprintf(fp0,"<dt>Linkname: %s\n",Title);

	fprintf(fp0,"<dt>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;URL: %s\n",doc->address);

	if(doc->post_data) {
	    fprintf(fp0,"<dt>Post Data: %s\n",doc->post_data);
	    fprintf(fp0,"<dt>Post Content Type: %s\n",doc->post_content_type);
	}
	fprintf(fp0,"<dt>Owner(s): %s\n",
	           (!owner_address ? "None" : owner_address));

	fprintf(fp0,"<dt>&nbsp;&nbsp;&nbsp;&nbsp;size: %d lines\n",size_of_file);

	fprintf(fp0,"<dt>&nbsp;&nbsp;&nbsp;&nbsp;mode: %s\n",
		   (lynx_mode == FORMS_LYNX_MODE ? "forms mode" : "normal"));

	fprintf(fp0,"</dl>\n");  /* end of list */

        if(nlinks > 0) {
	    fprintf(fp0,
	        "<h2>Link that you currently have selected</h2>\n<dl compact>");
	    if ((cp=strchr(links[doc->link].hightext, '<')) != NULL) {
	        *cp = '\0';
		StrAllocCopy(Title, links[doc->link].hightext);
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
	        StrAllocCopy(Title, links[doc->link].hightext);
	    }
	    fprintf(fp0,"<dt>Linkname: %s\n",Title);
	    fprintf(fp0,"<dt>Filename: %s\n",links[doc->link].lname ?
	    				     links[doc->link].lname : "");
	    fprintf(fp0,"</dl>\n");  /* end of list */

         } else
	    fprintf(fp0,"<h2>No Links on the current page</h2>");

#ifdef DIRED_SUPPORT
     }
#endif
	fprintf(fp0,"</body>\n");

        refresh();

        fclose(fp0);
	if (Title)
	    free(Title);

        return(1);
}
