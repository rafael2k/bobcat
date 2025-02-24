/* Routines to manipulate the local filesystem. */
/* Written by: Rick Mallett, Carleton University */
/* Report problems to rmallett@ccs.carleton.ca */

#ifdef DIRED_SUPPORT

#include "HTUtils.h"
#include "LYCurses.h"
#include "LYGlobal.h"
#include "LYUtils.h"
#include "LYString.h"
#include "LYStruct.h"
#include "LYGetFil.h"
#include "LYLocal.h"
#include "LYSystem.h"

#include <sys/wait.h>
#include <errno.h>
#include <grp.h>

#include "LYLeaks.h"

PRIVATE void clear_tags PARAMS((void));
PRIVATE int my_spawn PARAMS((char *path, char **argv, char *msg));
PRIVATE char *filename PARAMS((char *prompt, char *buf));
PRIVATE BOOLEAN permit_location PARAMS((char * destpath, char * srcpath, char ** newpath));

/* Remove all tagged files and directories. */

PRIVATE BOOLEAN remove_tagged ()
{ 
   int c, ans;
   char *cp,*tp;
   char tmpbuf[1024];
   char testpath[512];
   struct stat dir_info;
   int count,i;
   taglink *tag;
   char *args[5];

   if (tagged == NULL) return 0; /* should never happen */

   _statusline("Remove all tagged files and directories (y or n): ");
   c = LYgetch();
   ans=TOUPPER(c);

   count = 0;
   tag = tagged;
   while(ans == 'Y' && tag != NULL) {
      cp = tag->name;
      if(is_url(cp) == FILE_URL_TYPE) { /* unecessary check */
	 tp = cp;
	 if(!strncmp(tp,"file://localhost",16))
	   tp += 16;
	 else if(!strncmp(tp,"file:",5))
	   tp += 5;
	 strcpy(testpath,tp);
	 HTUnEscape(testpath);
	 if((i = strlen(testpath)) && testpath[i-1] == '/')
	   testpath[i-1] = '\0';

/* check the current status of the path to be deleted */

	 if (stat(testpath,&dir_info) == -1) {
	    sprintf(tmpbuf,"System error - failed to get status of %s ",testpath);
	    _statusline(tmpbuf);
	    sleep(sleep_three);
	    return count;
	 } else {
	    args[0] = "rm";
	    args[1] = "-rf";
	    args[2] = testpath;
	    args[3] = (char *) 0;
	    sprintf(tmpbuf, "remove %s", testpath);
	    if (my_spawn(RM_PATH, args, tmpbuf) <= 0)
		return count;
	    ++count;
	 }
      }
      tag = tag->next;
   }
   clear_tags();
   return count;
}

/* Move all tagged files and directories to a new location. */
/* Input is current directory. */

PRIVATE BOOLEAN modify_tagged ARGS1 (char *,testpath)
{
   int ans;
   char *cp;
   dev_t dev;
   ino_t inode;
   uid_t owner;
   char tmpbuf[1024];
   char savepath[512];
   struct stat dir_info;
   char *args[5];
   int count;
   taglink *tag;

   if (tagged == NULL) return 0; /* should never happen */

   _statusline("Enter new location for tagged items: ");

   tmpbuf[0] = '\0';
   LYgetstr(tmpbuf,VISIBLE);
   if (strlen(tmpbuf)) {

/* determine the ownership of the current location */

      cp = testpath;
      if (!strncmp(cp,"file://localhost",16))
	cp += 16;
      else if (!strncmp(cp,"file:",5))
	cp += 5;
      strcpy(savepath,cp);
      HTUnEscape(savepath);
      if (stat(savepath,&dir_info) == -1) {
	 sprintf(tmpbuf,"Unable to get status of %s ",savepath);
	 _statusline(tmpbuf);
	 sleep(sleep_three);
	 return 0;
      } 

/* save the owner of the current location for later use */
/* also save the device and inode for location checking */

      dev = dir_info.st_dev;
      inode = dir_info.st_ino;
      owner = dir_info.st_uid;

/* replace ~/ references to the home directory */

      if (!strncmp(tmpbuf,"~/",2)) {
	 cp = getenv("HOME");
	 strcpy(testpath,cp);
	 strcat(testpath,tmpbuf+1);
	 strcpy(tmpbuf,testpath);
      }

/* if path is relative prefix it with current location */

      if (tmpbuf[0] != '/') {
	 if (savepath[strlen(savepath)-1] != '/')
	   strcat(savepath,"/");
	 strcat(savepath,tmpbuf);
      } else {
	 strcpy(savepath,tmpbuf);
      }

/* stat the target location to determine type and ownership */

      if (stat(savepath,&dir_info) == -1) {
	 sprintf(tmpbuf,"Unable to get status of %s ",savepath);
	 _statusline(tmpbuf);
	 sleep(sleep_three);
	 return 0;
      }

/* make sure the source and target locations are not the same place */

      if (dev == dir_info.st_dev && inode == dir_info.st_ino) {
	 _statusline("Source and destination are the same location - request ignored!");
	 sleep(sleep_three);
	 return 0;
      }

/* make sure the target location is a directory which is owned */
/* by the same uid as the owner of the current location */

      if((dir_info.st_mode & S_IFMT) == S_IFDIR) {
	 if(dir_info.st_uid == owner) {
	    count = 0;
	    tag = tagged;

/* move all tagged items to the target location */

	    while (tag != NULL) {
	       cp = tag->name;
	       if(!strncmp(cp,"file://localhost",16))
		 cp += 16;
	       else if(!strncmp(cp,"file:",5))
		 cp += 5;
	       strcpy(testpath,cp);
	       HTUnEscape(testpath);

	       sprintf(tmpbuf,"move %s to %s",testpath,savepath);
	       args[0] = "mv";
	       args[1] = testpath;
	       args[2] = savepath;
	       args[3] = (char *) 0;
	       if (my_spawn(MV_PATH, args, tmpbuf) <= 0)
		  break;
	       tag = tag->next;
	       ++count;
	    }
	    clear_tags();
	    return count;
	 } else {
	    _statusline("Destination has different owner! Request denied. ");
	    sleep(sleep_three);
	    return 0;
	 }
      } else {
	 _statusline("Destination is not a valid directory! Request denied. ");
	 sleep(sleep_three);
	 return 0;
      }
   }
}

/* Modify the name of the specified item. */

PRIVATE BOOLEAN modify_name ARGS1 (char *,testpath)

{
   char *cp;
   uid_t owner;
   char tmpbuf[512];
   char newpath[512];
   char savepath[512];
   struct stat dir_info;
   char *args[5];

/* Determine the status of the selected item. */

   testpath = strip_trailing_slash(testpath);

   if (stat(testpath,&dir_info) == -1) {
      sprintf(tmpbuf,"Unable to get status of %s ",testpath);
      _statusline(tmpbuf);
      sleep(sleep_three);
   } else {

/* Change the name of the file or directory. */

      if ((dir_info.st_mode & S_IFMT) == S_IFDIR) {
	 cp = "Enter new name for directory: ";
      } else if ((dir_info.st_mode & S_IFMT) == S_IFREG) {
	 cp = "Enter new name for file: ";
      } else {
	 _statusline("The selected item is not a file or a directory! Request ignored. ");
	 sleep(sleep_three);
	 return 0;
      }
      if (filename(cp, tmpbuf) == NULL)
	 return 0;

/* Do not allow the user to also change the location at this time */

      if(strchr(tmpbuf,'/') != NULL) {
	 _statusline("Illegal character \"/\" found! Request ignored. ");
	 sleep(sleep_three);
      } else if(strlen(tmpbuf) && (cp = strrchr(testpath,'/')) != NULL) {
	 strcpy(savepath,testpath);
	 *++cp = '\0';
	 strcpy(newpath,testpath);
	 strcat(newpath,tmpbuf);

/* Make sure the destination does not already exist. */

	 if (stat(newpath,&dir_info) == -1) {
	    if (errno != ENOENT) {
	       sprintf(tmpbuf,"Unable to determine status of %s ",newpath);
	       _statusline(tmpbuf);
	       sleep(sleep_three);
	    } else {
	       sprintf(tmpbuf,"move %s to %s",savepath,newpath);
	       args[0] = "mv";
	       args[1] = savepath;
	       args[2] = newpath;
	       args[3] = (char *) 0;
	       if (my_spawn(MV_PATH, args, tmpbuf) <= 0)
		  return 0;
	       return 1; 
	    }
	 } else if ((dir_info.st_mode & S_IFMT) == S_IFDIR) {
	    _statusline("There is already a directory with that name! Request ignored. ");
	    sleep(sleep_three);
	 } else if ((dir_info.st_mode & S_IFMT) == S_IFREG) {
	    _statusline("There is already a file with that name! Request ignored. ");
	    sleep(sleep_three);
	 } else {
	    _statusline("The specified name is already in use! Request ignored. ");
	    sleep(sleep_three);
	 }
      }
   }
   return 0;
}

/* Change the location of a file or directory. */

PRIVATE BOOLEAN modify_location ARGS1 (char *,testpath)
{
   int mode;
   char *cp;
   dev_t dev;
   ino_t inode;
   uid_t owner;
   char tmpbuf[1024];
   char newpath[512];
   char savepath[512];
   struct stat dir_info;
   char *args[5];

/* Determine the status of the selected item. */

   testpath = strip_trailing_slash(testpath);

   if (stat(testpath,&dir_info) == -1) {
      sprintf(tmpbuf,"Unable to get status of %s ",testpath);
      _statusline(tmpbuf);
      sleep(sleep_three);
      return 0;
   } 

/* Change the location of the file or directory */

   if ((dir_info.st_mode & S_IFMT) == S_IFDIR) {
      cp = "Enter new location for directory: ";
   } else if ((dir_info.st_mode & S_IFMT) == S_IFREG) {
      cp = "Enter new location for file: ";
   } else {
      _statusline("The specified item is not a file or a directory - request ignored.");
      sleep(sleep_three);
      return 0;
   }
   if (filename(cp, tmpbuf) == NULL)
      return 0;
   if (strlen(tmpbuf)) {
      strcpy(savepath,testpath);
      strcpy(newpath,testpath);

/* Allow ~/ references to the home directory. */

      if (!strncmp(tmpbuf,"~/",2)) {
	 cp = getenv("HOME");
	 strcpy(newpath,cp);
	 strcat(newpath,tmpbuf+1);
	 strcpy(tmpbuf,newpath);
      }
      if (tmpbuf[0] != '/') {
	 if ((cp = strrchr(newpath,'/')) != NULL) {
	    *++cp = '\0';
	    strcat(newpath,tmpbuf);
	 } else {
	    _statusline("Unexpected failure - unable to find trailing \"/\"");
	    sleep(sleep_three);
	    return 0;
	 }
      } else {
	 strcpy(newpath,tmpbuf);
      }
      
/* Make sure the source and target have the same owner (uid) */

      dev = dir_info.st_dev;
      mode = dir_info.st_mode;
      inode = dir_info.st_ino;
      owner = dir_info.st_uid;  
      if (stat(newpath,&dir_info) == -1) {
	 sprintf(tmpbuf,"Unable to get status of %s ",newpath);
	 _statusline(tmpbuf);
	 sleep(sleep_three);
	 return 0;
      }
      if ((dir_info.st_mode & S_IFMT) != S_IFDIR) {
	 _statusline("Destination is not a valid directory! Request denied. ");
	 sleep(sleep_three);
	 return 0;
      }

/* make sure the source and target are not the same location */

      if (dev == dir_info.st_dev && inode == dir_info.st_ino) {
	 _statusline("Source and destination are the same location! Request ignored!");
	 sleep(sleep_three);
	 return 0;
      }
      if(dir_info.st_uid == owner) {
	 sprintf(tmpbuf,"move %s to %s",savepath,newpath);
	 args[0] = "mv";
	 args[1] = savepath;
	 args[2] = newpath;
	 args[3] = (char *) 0;
	 if (my_spawn(MV_PATH, args, tmpbuf) <= 0)
	    return 0;
	 return 1;
      } else {
	 _statusline("Destination has different owner! Request denied. ");
	 sleep(sleep_three);
	 return 0;
      }
   }
}   

/* Modify name or location of a file or directory on localhost. */

PUBLIC BOOLEAN local_modify ARGS2 (document *,doc, char **, newpath)
{
   int c, ans;
   char *cp;
   char testpath[512]; /* a bit ridiculous */
   int count;

   if (tagged != NULL) {
      cp = doc->address;
      if (!strncmp(cp,"file://localhost",16))
	cp += 16;
      else if (!strncmp(cp,"file:",5))
	cp += 5;
      strcpy(testpath,cp);
      HTUnEscape(testpath);
      count = modify_tagged(testpath);

      if (doc->link > (nlinks-count-1)) doc->link = nlinks-count-1;
      doc->link = doc->link < 0 ? 0 : doc->link; 

      return count;
   } else if (doc->link < 0 || doc->link > nlinks) /* added protection */
      return 0;

/* Do not allow simultaneous change of name and location as in Unix */
/* This reduces functionality but reduces difficulty for the novice */

   _statusline("Modify name, location, or permission (n, l, or p): ");
   c = LYgetch();
   ans=TOUPPER(c);

   if (strchr("NLP",ans) != NULL) {
      cp = links[doc->link].lname;
      if(!strncmp(cp,"file://localhost",16))
	cp += 16;
      else if(!strncmp(cp,"file:",5))
	cp += 5;
      strcpy(testpath,cp);
      HTUnEscape(testpath);

      if (ans == 'N') {

	 return(modify_name(testpath));

      } else if (ans == 'L') {

	 if (modify_location(testpath)) {

	   if (doc->link == (nlinks-1)) --doc->link;

	   return 1;
	}
      } else if (ans == 'P') {
	  return(permit_location(NULL, testpath, newpath));

      } else {

/* code for changing ownership needed here */

	 _statusline("This feature not yet implemented! ");
	 sleep(sleep_three);
      }
   }
   return 0;
}

/* Create a new empty file in the current directory. */

PRIVATE BOOLEAN create_file ARGS1 (char *,current_location)
{
   char tmpbuf[512];
   char testpath[512];
   struct stat dir_info;
   char *args[5];

   if (filename("Enter name of file to create: ", tmpbuf) == NULL)
      return 0;

   if(strstr(tmpbuf,"//") != NULL) {
      _statusline("Illegal redirection \"//\" found! Request ignored.");
      sleep(sleep_three);
   } else if(strlen(tmpbuf) && strchr(".~/",tmpbuf[0]) == NULL) {
      strcpy(testpath,current_location);
      if(testpath[strlen(testpath)-1] != '/')
	strcat(testpath,"/");

/* append the target filename to the current location */

      strcat(testpath,tmpbuf);

/* make sure the target does not already exist */

      if (stat(testpath,&dir_info) == -1) {
	 if (errno != ENOENT) {
	    sprintf(tmpbuf,"Unable to determine status of %s ",testpath);
	    _statusline(tmpbuf);
	    sleep(sleep_three);
	    return 0;
	 } 
	 sprintf(tmpbuf,"create %s",testpath);
	 args[0] = "touch";
	 args[1] = testpath;
	 args[2] = (char *) 0;
	 if (my_spawn(TOUCH_PATH, args, tmpbuf) <= 0)
	    return 0;
	 return 1;
      } else if ((dir_info.st_mode & S_IFMT) == S_IFDIR) {
	 _statusline("There is already a directory with that name! Request ignored. ");
	 sleep(sleep_three);
      } else if ((dir_info.st_mode & S_IFMT) == S_IFREG) {
	 _statusline("There is already a file with that name! Request ignored. ");
	 sleep(sleep_three);
      } else {
	 _statusline("The specified name is already in use! Request ignored. ");
	 sleep(sleep_three);
      }
   }
   return 0;
}

/* Create a new directory in the current directory. */

PRIVATE BOOLEAN create_directory ARGS1 (char *,current_location)
{
   char *cp;
   char tmpbuf[512];
   char testpath[512];
   struct stat dir_info;
   char *args[5];

   if (filename("Enter name for new directory: ", tmpbuf) == NULL)
      return 0;

   if(strstr(tmpbuf,"//") != NULL) {
      _statusline("Illegal redirection \"//\" found! Request ignored.");
      sleep(sleep_three);
   } else if(strlen(tmpbuf) && strchr(".~/",tmpbuf[0]) == NULL) {
      strcpy(testpath,current_location);

      if(testpath[strlen(testpath)-1] != '/')
	strcat(testpath,"/");

      strcat(testpath,tmpbuf);

/* make sure the target does not already exist */

      if (stat(testpath,&dir_info) == -1) {
	 if (errno != ENOENT) {
	    sprintf(tmpbuf,"Unable to determine status of %s ",testpath);
	    _statusline(tmpbuf);
	    sleep(sleep_three);
	    return 0;
	 } 
	 sprintf(tmpbuf,"make directory %s",testpath);
	 args[0] = "mkdir";
	 args[1] = testpath;
	 args[2] = (char *) 0;
	 if (my_spawn(MKDIR_PATH, args, tmpbuf) <= 0)
	    return 0;
	 return 1;
      } else if ((dir_info.st_mode & S_IFMT) == S_IFDIR) {
	 _statusline("There is already a directory with that name! Request ignored. ");
	 sleep(sleep_three);
      } else if ((dir_info.st_mode & S_IFMT) == S_IFREG) {
	 _statusline("There is already a file with that name! Request ignored. ");
	 sleep(sleep_three);
      } else {
	 _statusline("The specified name is already in use! Request ignored. ");
	 sleep(sleep_three);
      }
   }
   return 0;
}

/* Create a file or a directory at the current location. */

PUBLIC BOOLEAN local_create ARGS1 (document *,doc)
{
   int c, ans;
   char *cp;
   char testpath[512];

   _statusline("Create file or directory (f or d): ");
   c = LYgetch();
   ans = TOUPPER(c);

   cp = doc->address;
   if(!strncmp(cp,"file://localhost",16))
     cp += 16;
   else if(!strncmp(cp,"file:",5))
     cp += 5;
   strcpy(testpath,cp);
   HTUnEscape(testpath);
   
   if (ans == 'F') 
     return(create_file(testpath));
   else if (ans == 'D') 
     return(create_directory(testpath));
   else return 0;

}

/* Remove a single file or directory. */

PRIVATE BOOLEAN remove_single ARGS1 (char *,testpath) 
{
   int c, ans;
   char *cp;
   char tmpbuf[1024];
   struct stat dir_info;
   int i;
   char *args[5];

/* lstat first in case its a symbolic link */

   if (lstat(testpath,&dir_info) == -1 && stat(testpath,&dir_info) == -1) {
      sprintf(tmpbuf,"System error - failed to get status of %s. ",testpath);
      _statusline(tmpbuf);
      sleep(sleep_three);
      return 0;
   } 

/* locate the filename portion of the path */

   if ((cp = strrchr(testpath,'/')) != NULL) {
      ++cp;
   } else {
      cp = testpath;
   }
   if ((dir_info.st_mode & S_IFMT) == S_IFDIR) {
      if(strlen(cp) < 37)
	sprintf(tmpbuf,"Remove %s and all of its contents (y or n): ",cp);
      else
	sprintf(tmpbuf,"Remove directory and all of its contents (y or n): ");
   } else if ((dir_info.st_mode & S_IFMT) == S_IFREG) {
      if(strlen(cp) < 60)
	sprintf(tmpbuf,"Remove file %s (y or n): ",cp);
      else 
	sprintf(tmpbuf,"Remove file (y or n): ");
   } else if ((dir_info.st_mode & S_IFMT) == S_IFLNK) {
      if(strlen(cp) < 50)
	sprintf(tmpbuf,"Remove symbolic link %s (y or n): ",cp);
      else 
	sprintf(tmpbuf,"Remove symbolic link (y or n): ");
   } else {
      sprintf(tmpbuf,"Unable to determine status of %s. ",testpath);
      _statusline(tmpbuf);
      sleep(sleep_three);
      return 0;
   }
   _statusline(tmpbuf);

   c = LYgetch();
   if(TOUPPER(c) == 'Y') {
      sprintf(tmpbuf,"remove %s",testpath);
      args[0] = "rm";
      args[1] = "-rf";
      args[2] = testpath;
      args[3] = (char *) 0;
      if (my_spawn(RM_PATH, args, tmpbuf) <= 0)
	  return 0;
      return 1;
   }
   return 0;
}

/* Remove a file or a directory. */

PUBLIC BOOLEAN local_remove ARGS1 (document *,doc)
{  
   char *cp,*tp;
   char testpath[512];
   int count,i;

   if (tagged != NULL) {

      count = remove_tagged();

      if (doc->link > (nlinks-count-1)) doc->link = nlinks-count-1;
      doc->link = doc->link < 0 ? 0 : doc->link; 

      return count;
   } else if (doc->link < 0 || doc->link > nlinks)
      return 0;
   cp = links[doc->link].lname;
   if(is_url(cp) == FILE_URL_TYPE) {
      tp = cp;
      if(!strncmp(tp,"file://localhost",16))
	tp += 16;
      else if(!strncmp(tp,"file:",5))
	tp += 5;
      strcpy(testpath,tp);
      HTUnEscape(testpath);
      if((i = strlen(testpath)) && testpath[i-1] == '/')
	testpath[i-1] = '\0';

      if (remove_single(testpath)) {

	 if (doc->link == (nlinks-1)) --doc->link;

	 return 1;
      }
   }
   return 0;
}

/* Table of permission strings and chmod values. Makes the code a bit cleaner */
static struct {
    char *string_mode;	/* Key for  value below */
    long permit_bits;	/* Value for chmod/whatever */
} permissions[] = {
    {"IRUSR", S_IRUSR},
    {"IWUSR", S_IWUSR},
    {"IXUSR", S_IXUSR},
    {"IRGRP", S_IRGRP},
    {"IWGRP", S_IWGRP},
    {"IXGRP", S_IXGRP},
    {"IROTH", S_IROTH},
    {"IWOTH", S_IWOTH},
    {"IXOTH", S_IXOTH},
    {NULL, 0}			/* Don't include setuid and friends,
				   use shell access for that */
};

#ifndef S_ISDIR
#  define S_ISDIR(mode)   ((mode&0xF000) == 0x4000)
#endif
    
PRIVATE BOOLEAN permit_location ARGS3 (char *, destpath, char *, srcpath, char **, newpath)
{

#ifndef UNIX
    _statusline("Sorry, don't know how to permit non-UNIX files yet.");
    sleep(sleep_two);
    return(0);
#else
    char *cp;
    char tmpbuf[LINESIZE];
    struct stat dir_info;

    if (srcpath) {                      /* Create form */
	FILE *fp0;
	static char * tempfile=NULL;
	char * print_filename=NULL;
	char * user_filename;
	struct group * grp;
	char * group_name;

	/* A couple of sanity tests */

	srcpath = strip_trailing_slash(srcpath);
	if(strncmp(srcpath,"file://localhost",16) == 0)
	    srcpath += 16;
	
	if (lstat(srcpath,&dir_info) == -1) {
	    sprintf(tmpbuf,"Unable to get status of %s ",srcpath);
	    _statusline(tmpbuf);
	    sleep(sleep_three);
	    return 0;
	} else if ((dir_info.st_mode & S_IFMT) != S_IFDIR &&
	    (dir_info.st_mode & S_IFMT) != S_IFREG) {
	    _statusline("The specified item is not a file nor a directory - request ignored.");
	    sleep(sleep_three);
	    return(0);
	}
	
	user_filename = srcpath;
	cp = strrchr(srcpath, '/');
	if (cp != NULL) {
	    user_filename = cp + 1;
	}
	
	if(tempfile == NULL) {
	    tempfile = (char *) malloc(127);
	    tempname(tempfile,NEW_FILE);
	    /* make the file a URL now */
	}
	StrAllocCopy(print_filename,"file://localhost");
	StrAllocCat(print_filename,tempfile);
	
	if((fp0 = fopen(tempfile,"w")) == NULL) {
	    _statusline("Unable to open permit options file");
	    sleep(sleep_two);
	    return(0);
	}
	
	StrAllocCopy(*newpath, print_filename);
	
	grp = getgrgid(dir_info.st_gid);
	if (grp == NULL) {
	    group_name = "";
	} else {
	    group_name = grp->gr_name;
	}

	fprintf(fp0, "<Html><Head>\n<Title>%s</Title>\n</Head>\n<Body>\n",
		PERMIT_OPTIONS_TITLE);
	fprintf(fp0,"<H1>Permissions for %s</H1>\n", user_filename);
	fprintf(fp0, "<Form Action=\"LYNXDIRED://PERMIT_LOCATION%s\">\n",
		srcpath);
	
	fprintf(fp0, "<Ol><Li>Specify permissions below:<Br><Br>\n");
	fprintf(fp0, "Owner:<Br>\n");
	fprintf(fp0,
		"<Input Type=\"checkbox\" Name=\"mode\" Value=\"IRUSR\" %s> Read<Br>\n",
		dir_info.st_mode & S_IRUSR ? "checked" : "");
	fprintf(fp0,
		"<Input Type=\"checkbox\" Name=\"mode\" Value=\"IWUSR\" %s> Write<Br>\n",
		dir_info.st_mode & S_IWUSR ? "checked" : "");
	fprintf(fp0,
		"<Input Type=\"checkbox\" Name=\"mode\" Value=\"IXUSR\" %s> %s<Br>\n",
		dir_info.st_mode & S_IXUSR ? "checked" : "",
		S_ISDIR(dir_info.st_mode) ? "Search" : "Execute");
	
	fprintf(fp0, "Group %s:<Br>\n", group_name);
	fprintf(fp0,
		"<Input Type=\"checkbox\" Name=\"mode\" Value=\"IRGRP\" %s> Read<Br>\n",
		dir_info.st_mode & S_IRGRP ? "checked" : "");
	fprintf(fp0,
		"<Input Type=\"checkbox\" Name=\"mode\" Value=\"IWGRP\" %s> Write<Br>\n",
		dir_info.st_mode & S_IWGRP ? "checked" : "");
	fprintf(fp0,
		"<Input Type=\"checkbox\" Name=\"mode\" Value=\"IXGRP\" %s> %s<Br>\n",
		dir_info.st_mode & S_IXGRP ? "checked" : "",
		S_ISDIR(dir_info.st_mode) ? "Search" : "Execute");
	
	fprintf(fp0, "Others:<Br>\n");
	fprintf(fp0,
		"<Input Type=\"checkbox\" Name=\"mode\" Value=\"IROTH\" %s> Read<Br>\n",
		dir_info.st_mode & S_IROTH ? "checked" : "");
	fprintf(fp0,
		"<Input Type=\"checkbox\" Name=\"mode\" Value=\"IWOTH\" %s> Write<Br>\n",
		dir_info.st_mode & S_IWOTH ? "checked" : "");
	fprintf(fp0,
		"<Input Type=\"checkbox\" Name=\"mode\" Value=\"IXOTH\" %s> %s<Br>\n",
		dir_info.st_mode & S_IXOTH ? "checked" : "",
		S_ISDIR(dir_info.st_mode) ? "Search" : "Execute");
	
	fprintf(fp0, "<Br>\n<Li><Input Type=\"submit\" Value=\"Submit\"> form to permit %s %s.\n</Ol>\n</Form>\n",
		(dir_info.st_mode & S_IFMT) == S_IFDIR ? "directory" : "file",
		user_filename);
	fprintf(fp0, "</Body></Html>");
	fclose(fp0);
	
	++LYforce_no_cache;
	return(PERMIT_FORM_RESULT);      /* Special flag for LYMainLoop */

    } else {                             /* The form being activated */
	mode_t new_mode = 0;
	char *args[5];
	char amode[10];
	
	HTUnEscape(destpath);
	cp = destpath;
	while (*cp != '\0' && *cp != '?') { /* Find filename */
	    cp++;
	}
	if (*cp == '\0') {
	    return(0);      /* Nothing to permit */
	}
	*cp++ = '\0';       /* Null terminate file name and start working on the masks */
	
	/* A couple of sanity tests */
	destpath = strip_trailing_slash(destpath);
	if (stat(destpath,&dir_info) == -1) {
	    sprintf(tmpbuf,"Unable to get status of %s ",destpath);
	    _statusline(tmpbuf);
	    sleep(sleep_three);
	    return 0;
	} else if ((dir_info.st_mode & S_IFMT) != S_IFDIR &&
	    (dir_info.st_mode & S_IFMT) != S_IFREG) {
	    _statusline("The specified item is not a file nor a directory - request ignored.");
	    sleep(sleep_three);
	    return 0;
	}
	
	/* Cycle over permission strings */
	while(*cp != '\0') {
	    char *cr = cp;
	    
	    while(*cr != '\0' && *cr != '&') { /* GET data split by '&' */
		cr++;
	    }
	    if (*cr != '\0') {
		*cr++ = '\0';
	    }
	    if (strncmp(cp, "mode=", 5) == 0) {	/* Magic string */
		int i;

		for(i = 0; permissions[i].string_mode != NULL; i++) {
		    if (strcmp(permissions[i].string_mode, cp+5) == 0) {
			new_mode |= permissions[i].permit_bits;
			break;
		    }
		}
		if (permissions[i].string_mode == NULL) {
		    _statusline("Invalid mode format.");
		    sleep(sleep_three);
		    return 0;
		}
	    } else {
		_statusline("Invalid syntax format.");
		sleep(sleep_three);
		return 0;
	    }
	    
	    cp = cr;
	}
	
	/* Call chmod */
	sprintf(tmpbuf,"chmod %.4o %s", new_mode, destpath);
	sprintf(amode, "%.4o", new_mode);
	args[0] = "chmod";
	args[1] = amode;
	args[2] = destpath;
	args[3] = (char *) 0;
	if (my_spawn(CHMOD_PATH, args, tmpbuf) <= 0) {
	    return 0;
	}
	++LYforce_no_cache;         /* Force update of dired listing */
	return 1;
    }
#endif
}

PUBLIC BOOLEAN is_a_file ARGS1 (char *,testname)
{ 
   char *cp;
   char testpath[512];
   struct stat dir_info;

   cp = testname;
   if(!strncmp(cp,"file://localhost",16))
     cp += 16;
   else if(!strncmp(cp,"file:",5))
     cp += 5;
   strcpy(testpath,cp);
   HTUnEscape(testpath);
   if (stat(testpath,&dir_info) == -1) 
      return -1; 
   else
     if (((dir_info.st_mode) & S_IFMT) == S_IFREG)
       return 1;
     else
       return 0;
}

/* display or remove a tag from a given link */

PUBLIC void tagflag ARGS2(int,flag, int,cur)
{
    if (nlinks > 0 /*&& links[cur].lx == 3*/) {
       move(links[cur].ly,2 /*links[cur].lx-2*/);
       stop_reverse();
       if (flag == ON)
	  addch('+');
       else
	  addch(' ');

#ifdef FANCY_CURSES
      if(!LYShowCursor)
          move(LYlines-1,LYcols-1);  /* get cursor out of the way */
      else
#endif /* FANCY CURSES */
	  /* never hide the cursor if there's no FANCY CURSES */
          move(links[cur].ly, links[cur].lx);

      if(flag)
          refresh();
    }
}

PUBLIC void showtags ARGS1 (taglink *, t)
{
    int i;
    taglink *s;
    
    for(i=0;i<nlinks;i++) {
      s = t;
      while(s != NULL) {
	 if(!strcmp(links[i].lname,s->name)) {
	    tagflag(ON,i);
	    break;
	 } else
	    s = s->next;
      }
   }
}

PUBLIC char * strip_trailing_slash ARGS1 (char *, dirname)
{
   int i;

   i = strlen(dirname) - 1;
   while (i && dirname[i] == '/') dirname[i--] = '\0';
   return dirname;
}

/* Perform file management operations for LYNXDIRED URL's */

PUBLIC int local_dired ARGS1(document *,doc)
{
   char *line;
   char *cp,*tp,*qp;
   char tmpbuf[256];
   char buffer[512];
   char *p1,*p2;

   DocAddress WWWDoc;  /* a WWW absolute doc address struct */

   line = doc->address;
   HTUnEscape(line);

   /* This cuases a SIGSEGV later when StrAllocCopy tries to free tp
    * let's make it point to NULL
   tp = tmpbuf;
   */
   tp = NULL;
   if (!strncmp(line,"LYNXDIRED://NEW_FILE",20)) {
      if (create_file(&line[20])) ++LYforce_no_cache;
   } else if (!strncmp(line,"LYNXDIRED://NEW_FOLDER",22)) {
      if (create_directory(&line[22])) ++LYforce_no_cache;
   } else if (!strncmp(line,"LYNXDIRED://INSTALL_SRC",23)) {
      local_install(NULL, &line[23], &tp);
      StrAllocCopy(doc->address, tp);
      return 0;
   } else if (!strncmp(line,"LYNXDIRED://INSTALL_DEST",24)) {
      local_install(&line[24], NULL, &tp);
      pop(doc);
   } else if (!strncmp(line,"LYNXDIRED://MODIFY_NAME",23)) {
      if (modify_name(&line[23])) ++LYforce_no_cache;
   } else if (!strncmp(line,"LYNXDIRED://MODIFY_LOCATION",27)) {
      if (modify_location(&line[27])) ++LYforce_no_cache;
   } else if (!strncmp(line,"LYNXDIRED://MOVE_TAGGED",23)) {
      if (modify_tagged(&line[23])) ++LYforce_no_cache;
   } else if (!strncmp(line,"LYNXDIRED://PERMIT_SRC",22)) {
      permit_location(NULL, &line[22], &tp);
      StrAllocCopy(doc->address, tp);
      return 0;
   } else if (!strncmp(line,"LYNXDIRED://PERMIT_LOCATION",27)) {
       permit_location(&line[27], NULL, &tp);
   } else if (!strncmp(line,"LYNXDIRED://REMOVE_SINGLE",25)) {
      if (remove_single(&line[25])) ++LYforce_no_cache;
   } else if (!strncmp(line,"LYNXDIRED://REMOVE_TAGGED",25)) {
      if (remove_tagged()) ++LYforce_no_cache;
   } else if (!strncmp(line,"LYNXDIRED://UPLOAD",18)) {
     if (LYUpload(line)) ++LYforce_no_cache;
   } else {
      if (line[strlen(line)-1] == '/')
	line[strlen(line)-1] = '\0';
      if ((cp = strrchr(line,'/')) == NULL)
	return 0;

/* Construct the appropriate system command taking care to escape all
   path references to avoid spoofing the shell. */

      *buffer = '\0';
      if (!strncmp(line,"LYNXDIRED://DECOMPRESS",22)) {
	tp = quote_pathname(line+22); 
	sprintf(buffer,"%s %s", UNCOMPRESS_PATH, tp);
	free(tp);
      } else if (!strncmp(line,"LYNXDIRED://UUDECODE",20)) {
	tp = quote_pathname(line+20); 
	sprintf(buffer,"%s %s", UUDECODE_PATH, tp);
	_statusline("Warning! UUDecoded file will exist in the directory you started Lynx.");
	sleep(sleep_three);
	free(tp);
#ifdef OK_TAR
      } else if (!strncmp(line,"LYNXDIRED://UNTAR_Z",19)) {
	tp = quote_pathname(line+19);
	*cp++ = '\0';
	cp = quote_pathname(line+19);
	sprintf(buffer,"%s %s | (cd %s; %s -xfe -)", ZCAT_PATH, tp,cp,TAR_PATH);
	free(cp);
	free(tp);
      } else if (!strncmp(line,"LYNXDIRED://TAR_Z",17)) {
	*cp++ = '\0';
        cp = quote_pathname(cp);
	tp = quote_pathname(line+17);
	sprintf(buffer,"(cd %s; %s -cfe - %s) | %s >%s/%s.tar.Z",tp,TAR_PATH, cp,COMPRESS_PATH, tp,cp);
	free(cp);
	free(tp);
#endif
#ifdef OK_GZIP
      } else if (!strncmp(line,"LYNXDIRED://GZIP",16)) {
	tp = quote_pathname(line+16);
	sprintf(buffer,"%s -q %s", GZIP_PATH, tp);
	free(tp);
      } else if (!strncmp(line,"LYNXDIRED://UNGZIP",18)) {
	tp = quote_pathname(line+18);
	sprintf(buffer,"%s -d %s", GZIP_PATH, tp);
	free(tp);
#endif
#ifdef OK_ZIP
      } else if (!strncmp(line,"LYNXDIRED://ZIP",15)) {
	tp = quote_pathname(line+15);
	sprintf(buffer,"%s -rq %s.zip %s", ZIP_PATH, tp, tp);
	free(tp);
      } else if (!strncmp(line,"LYNXDIRED://UNZIP",17)) {
	tp = quote_pathname(line+17);
	sprintf(buffer,"%s -q %s", UNZIP_PATH, tp);
	free(tp);
#endif
#if defined(OK_TAR) && defined(OK_GZIP)
      } else if (!strncmp(line,"LYNXDIRED://UNTAR_GZ",20)) {
	tp = quote_pathname(line+20);
	*cp++ = '\0';
	cp = quote_pathname(line+20);
	sprintf(buffer,"%s -qdc %s | (cd %s; %s -xfe -)",GZIP_PATH, tp,cp, TAR_PATH);
	free(cp);
	free(tp);
      } else if (!strncmp(line,"LYNXDIRED://TAR_GZ",18)) {
	*cp++ = '\0';
	cp = quote_pathname(cp);
	tp = quote_pathname(line+18);
	sprintf(buffer,"(cd %s; %s -cfe - %s) | %s -qc >%s/%s.tar.gz",tp, TAR_PATH, cp, GZIP_PATH, tp,cp);
	free(cp);
	free(tp);
#endif
      } else if (!strncmp(line,"LYNXDIRED://COMPRESS",20)) {
	tp = quote_pathname(line+20);
	sprintf(buffer,"%s %s",COMPRESS_PATH, tp);
	free(tp);
      }
      if (strlen(buffer)) {
	 if (strlen(buffer) < 60) 
	    sprintf(tmpbuf,"Executing %s ",buffer);
	 else
	    sprintf(tmpbuf,
		    "Executing system command. This might take a while.");
	 _statusline(tmpbuf);
	 stop_curses();
	 printf("%s\n", tmpbuf);
	 fflush(stdout);
	 system(buffer);
#ifdef VMS
	 extern BOOLEAN HadVMSInterrupt
	 HadVMSInterrupt = FALSE;
#endif
	 start_curses();
	 ++LYforce_no_cache;
      }
   }

   pop(doc);
}

/* Provide a menu of file management options. */

PUBLIC int dired_options ARGS2 (document *,doc, char **,newfile)
{
    static char * tempfile=NULL;
    static char * dired_filename=0;
    char testpath[512],curloc[512]; /* much too large */
    char tmpbuf[LINESIZE];
    lynx_html_item_type *nxt;
    struct stat dir_info;
    FILE *fp0;
    char *cp,*tp = NULL;
    char *escaped;
    int count;

    if(tempfile == NULL) {
	tempfile = (char *) malloc(127);
        tempname(tempfile,NEW_FILE);
        /* make the file a URL now */
	StrAllocCopy(dired_filename,"file://localhost");
	StrAllocCat(dired_filename,tempfile);
    }

    if((fp0 = fopen(tempfile,"w")) == NULL) {
       _statusline("Unable to open file management menu file");
       sleep(sleep_two);
       return(0);
    }
    
    StrAllocCopy(*newfile, dired_filename);
    
    cp = doc->address;
    if(!strncmp(cp,"file://localhost",16))
      cp += 16;
    else if(!strncmp(cp,"file:",5))
      cp += 5;
    strcpy(curloc,cp);
    HTUnEscape(curloc);
    if (curloc[strlen(curloc)-1] == '/')
      curloc[strlen(curloc)-1] = '\0';

    if (doc->link > -1 && doc->link < (nlinks+1)) {
       cp = links[doc->link].lname;
       if(!strncmp(cp,"file://localhost",16))
	 cp += 16;
       else if(!strncmp(cp,"file:",5))
	 cp += 5;
       strcpy(testpath,cp);
       HTUnEscape(testpath);
       if (testpath[strlen(testpath)-1] == '/')
	  testpath[strlen(testpath)-1] = '\0';

       if (lstat(testpath,&dir_info) == -1 && stat(testpath,&dir_info) == -1) {
	  sprintf(tmpbuf,"Unable to get status of %s ",testpath);
	  _statusline(tmpbuf);
	  sleep(sleep_three);
	  return 0;
       } 

       if ((cp = strrchr(testpath,'.')) != NULL && strlen(testpath) > strlen(cp)) {
	  *cp = '\0';
	  tp = strrchr(testpath,'.');
	  *cp = '.';
       }
    } else testpath[0] = '\0';

    escaped = (char *) HTEscape(testpath,(unsigned char) 4);

    fprintf(fp0,"<head>\n<title>%s</title></head>\n<body>\n",DIRED_MENU_TITLE);

    fprintf(fp0,"\n<h1>File Management Options (%s Version %s)</h1>",
    						LYNX_NAME, LYNX_VERSION);

    fprintf(fp0,"Current directory is %s <br>\n",curloc);

    if (tagged == NULL)
       if (strlen(testpath))
          fprintf(fp0,"Current selection is %s <p>\n",testpath);
       else
          fprintf(fp0,"Nothing currently selected. <p>\n");
    else 
       fprintf(fp0,"Current selection is all tagged items. <p>\n");

    fprintf(fp0,"<a href=\"LYNXDIRED://NEW_FILE%s\"> ",curloc);
    fprintf(fp0,"New File </a> (in current directory)<br>\n");

    fprintf(fp0,"<a href=\"LYNXDIRED://NEW_FOLDER%s\"> ",curloc);
    fprintf(fp0,"New Directory </a> (in current directory)<br>\n");

    if (tagged == NULL && strlen(testpath)) {

       fprintf(fp0,"<a href=\"LYNXDIRED://INSTALL_SRC%s\"> ",escaped);
       fprintf(fp0,"Install</a> (of current selection)<br>\n");

       fprintf(fp0,"<a href=\"LYNXDIRED://MODIFY_NAME%s\"> ",escaped);
       fprintf(fp0,"Modify Name </a> (of current selection)<br>\n");

       fprintf(fp0,"<a href=\"LYNXDIRED://PERMIT_SRC%s\"> ",escaped);
       fprintf(fp0,"Permit Name </a> (of current selection)<br>\n");

       fprintf(fp0,"<a href=\"LYNXDIRED://MODIFY_LOCATION%s\"> ",escaped);
       fprintf(fp0,"Change Location </a> (of current selection)<br>\n");

       fprintf(fp0,"<a href=\"LYNXDIRED://REMOVE_SINGLE%s\"> ",escaped);
       fprintf(fp0,"Remove </a> (current selection)<br>\n");

#ifdef OK_UUDECODE
       fprintf(fp0,"<a href=\"LYNXDIRED://UUDECODE%s\"> ",escaped);
       fprintf(fp0,"UUDecode </a> (current selection) <br>\n");
#endif
	  
       if (tp != NULL && !strcmp(tp,".tar.Z")) {
#ifdef OK_TAR
	  fprintf(fp0,"<a href=\"LYNXDIRED://UNTAR_Z%s\"> ",escaped);
	  fprintf(fp0,"Expand </a> (current selection)<br>\n");
#endif
       } else if (tp != NULL && !strcmp(tp,".tar.gz")) {
#if defined(OK_TAR) && defined(OK_GZIP)
	  fprintf(fp0,"<a href=\"LYNXDIRED://UNTAR_GZ%s\"> ",escaped);
	  fprintf(fp0,"Expand </a> (current selection)<br>\n");
#endif
       } else if (cp != NULL && !strcmp(cp,".Z")) {
	  
	  fprintf(fp0,"<a href=\"LYNXDIRED://DECOMPRESS%s\"> ",escaped);
	  fprintf(fp0,"Uncompress </a> (current selection) <br>\n");
	  
       } else if (cp != NULL && !strcmp(cp,".gz")) {
#ifdef OK_GZIP
	  fprintf(fp0,"<a href=\"LYNXDIRED://UNGZIP%s\"> ",escaped);
	  fprintf(fp0,"Uncompress </a> (current selection) <br>\n");
#endif	  
       } else if (cp != NULL && !strcmp(cp,".zip")) {
#ifdef OK_ZIP	  
	  fprintf(fp0,"<a href=\"LYNXDIRED://UNZIP%s\"> ",escaped);
	  fprintf(fp0,"Expand </a> zip Archive<br>\n");
#endif	  
       } else {

	  if ((dir_info.st_mode & S_IFMT) == S_IFDIR) {
#ifdef OK_TAR
	     fprintf(fp0,"<a href=\"LYNXDIRED://TAR_Z%s\"> ",escaped);
	     fprintf(fp0,"Tar and compress </a> (using Unix compress)<br>\n");
#endif
#if defined(OK_TAR) && defined(OK_GZIP)
	     fprintf(fp0,"<a href=\"LYNXDIRED://TAR_GZ%s\"> ",escaped);
	     fprintf(fp0,"Tar and compress </a> (using GNU gzip)<br>\n");
#endif
#ifdef OK_ZIP
	     fprintf(fp0,"<a href=\"LYNXDIRED://ZIP%s\"> ",escaped);
	     fprintf(fp0,"Package and compress </a> (using zip)<br>\n");
#endif
	  } else if ((dir_info.st_mode & S_IFMT) == S_IFREG) {

	     fprintf(fp0,"<a href=\"LYNXDIRED://COMPRESS%s\"> ",escaped);
	     fprintf(fp0,"Compress </a> (using Unix compress)<br>\n");
#ifdef OK_GZIP	  
	     fprintf(fp0,"<a href=\"LYNXDIRED://GZIP%s\"> ",escaped);
	     fprintf(fp0,"Compress </a> (using gzip) <br>\n");
#endif
#ifdef OK_ZIP	  
	     fprintf(fp0,"<a href=\"LYNXDIRED://ZIP%s\"> ",escaped);
	     fprintf(fp0,"Compress </a> (using zip) <br>\n");
#endif
	  }
	  
       }
             
    } else if (tagged != NULL) {

       fprintf(fp0,"<a href=\"LYNXDIRED://MOVE_TAGGED\"> ");
       fprintf(fp0,"Move all tagged items to another location.</a><br>\n");

       fprintf(fp0,"<a href=\"LYNXDIRED://REMOVE_TAGGED\"> ");
       fprintf(fp0,"Remove all tagged files and directories.</a><br>\n");
    }

    if (uploaders != NULL) {
       fprintf(fp0, "<p>Upload to current directory:<p>\n");
       for (count=0, nxt = uploaders; nxt != NULL; nxt = nxt->next, count++) {
	  fprintf(fp0,"<a href=\"LYNXDIRED://UPLOAD=%d/TO=%s\"> %s </a><br>\n",count,curloc,nxt->name);
       }
    }

    fprintf(fp0,"</body>\n");
    fclose(fp0);

    free(escaped);

    LYforce_no_cache = 1;

    return(0);
}

PRIVATE int my_spawn ARGS3(char *,path, char **,argv, char *,msg)
{
    int pid, status, rc;
    char tmpbuf[512];

    rc = 1;                 /* It will work */
    stop_curses();
    pid = fork(); /* fork and execute rm */
    switch (pid) {
      case -1:
	sprintf(tmpbuf, "Unable to %s due to system error!", msg);
	_statusline(tmpbuf);
	sleep(sleep_three);
	rc = 0;
      case 0:  /* child */
	execv(path, argv);
	exit(1);    /* execv failed, give wait() something to look at */
      default:  /* parent */
	while(wait(&status) != pid) /* do nothing */ ;
	    if(WEXITSTATUS(status) != 0 ||
	       WTERMSIG(status) != 0)  { /* error return */
		sprintf(tmpbuf, "Probable failure to %s due to system error!",
		    msg);
		_statusline(tmpbuf);
		sleep(sleep_three);
		rc = 0;
	    }
    }
#ifdef VMS
    {
	extern BOOLEAN HadVMSInterrupt;
	HadVMSInterrupt = FALSE;
    }
#endif /* VMS */
    start_curses();

    return(rc);
}

PRIVATE char *filename ARGS2 (char *,prompt, char *,buf)
{
   char *cp;

   _statusline(prompt);

   *buf = '\0';
   LYgetstr(buf,VISIBLE);
   if(strstr(buf,"../") != NULL) {
       _statusline("Illegal filename; request ignored.");
       sleep(sleep_three);
       return NULL;
   }
#ifdef NO_DOT_FILES
   cp = strrchr(buf, '/');	/* find last slash */
   if (cp) 
      cp += 1;
   else
      cp = buf;
   if (*cp == '.') {
       _statusline("Illegal filename; request ignored.");
       sleep(sleep_three);
       return NULL;
   }
   return buf;
#endif
}

/* Install the specified file or directory. */

BOOLEAN local_install ARGS3 (char *, destpath, char *, srcpath, char **, newpath)

{
   char *cp;
   uid_t owner;
   char tmpbuf[512];
   static char savepath[512]; /* this will be the link that is to be installed */
   struct stat dir_info;
   char *args[5];
   taglink *tag;
   int count = 0;

/* Determine the status of the selected item. */

   if (srcpath) {
      srcpath = strip_trailing_slash(srcpath);

      if(strncmp(srcpath,"file://localhost",16) == 0)
	 srcpath += 16;
      if (stat(srcpath,&dir_info) == -1) {
         sprintf(tmpbuf,"Unable to get status of %s ",srcpath);
         _statusline(tmpbuf);
         sleep(sleep_three);
         return 0;
      } else if ((dir_info.st_mode & S_IFMT) != S_IFDIR && 
                 (dir_info.st_mode & S_IFMT) != S_IFREG) {
         _statusline("The selected item is not a file or a directory! Request ignored. ");
         sleep(sleep_three);
         return 0;
      }
      strcpy(savepath, srcpath);
      ++LYforce_no_cache;
      strcpy(tmpbuf, "file://localhost");
      strcat(tmpbuf, getenv("HOME"));
      strcat(tmpbuf, "/.installdirs.html");
      StrAllocCopy(*newpath, tmpbuf);
      return 0;
   }

      destpath = strip_trailing_slash(destpath);

      if (stat(destpath,&dir_info) == -1) {
         sprintf(tmpbuf,"Unable to get status of %s ",destpath);
         _statusline(tmpbuf);
         sleep(sleep_three);
         return 0;
      } else if ((dir_info.st_mode & S_IFMT) != S_IFDIR) {
         _statusline("The selected item is not a directory! Request ignored. ");
         sleep(sleep_three);
         return 0;
      } else if (0 /*directory not writeable*/) {
         _statusline("Install in the selected directory not permitted.");
         sleep(sleep_three);
         return 0;
      }

   args[0] = "install";
   args[2] = destpath;
   args[3] = (char *) 0;
   sprintf(tmpbuf, "install %s", destpath);
   tag = tagged;
   for (;;) {
      if (tagged) {
	 args[1] = tag->name;
	 if (strncmp("file://localhost", args[1], 16) == 0)
	    args[1] = tag->name + 16;
      } else
         args[1] = savepath;

      if (my_spawn(INSTALL_PATH, args, tmpbuf) <= 0)
         return count;
      count++;
      if (!tagged)
	 break;
      tag = tag->next;
      if (!tag)
	break;
   }
   if (tagged)
      clear_tags();
   return count;
}

PRIVATE void clear_tags NOARGS
{
    taglink *t1;

    while((t1=tagged) != NULL) { 
	tagged = tagged->next;
	free(t1->name);
	free(t1);
    }
    tagged = NULL;
}
#endif /* DIRED_SUPPORT */
