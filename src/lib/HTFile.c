/*                      File Access                             HTFile.c
**			===========
**
**	This is unix-specific code in general, with some VMS bits.
**	These are routines for file access used by browsers.
**
** History:
**	   Feb 91	Written Tim Berners-Lee CERN/CN
**	   Apr 91	vms-vms access included using DECnet syntax
**	26 Jun 92 (JFG) When running over DECnet, suppressed FTP.
**			Fixed access bug for relative names on VMS.
**	   Sep 93 (MD)  Access to VMS files allows sharing.
**	15 Nov 93 (MD)	Moved HTVMSname to HTVMSUTILS.C
**      27 Dec 93 (FM)  FTP now works with VMS hosts.
**			FTP path must be Unix-style and cannot include
**			 the device or top directory.
*/

#ifndef VMS
//#define LONG_LIST  /* Define this for long style unix listings (ls -l) */
/* #define NO_PARENT_DIR_REFERENCE  /* Define this for no parent links */
#endif /* !VMS */

#include "HTAccess.h"
#include "HTUtils.h"
#include "HTFile.h"		/* Implemented here */

#define INFINITY 512		/* file name length @@ FIXME */
#define MULTI_SUFFIX ".multi"   /* Extension for scanning formats */

#define HT_EM_SPACE ((char)2)

#define FREE(x) if (x) {free(x); x = NULL;}

#include "HTParse.h"
#include "tcp.h"
#include "HTTCP.h"
// #include "HTFTP.h"
//#include "HTGOPHER.h"
#include "HTAnchor.h"
#include "HTAtom.h"
#include "HTWriter.h"
#include "HTFWrite.h"
#include "HTInit.h"
#include "HTBTree.h"
#include <dirent.h>

typedef struct _HTSuffix {
	char *		suffix;
	HTAtom *	rep;
	HTAtom *	encoding;
        long           quality;
} HTSuffix;

#ifndef NGROUPS
#ifdef NGROUPS_MAX
#define NGROUPS NGROUPS_MAX
#else
#define NGROUPS 32
#endif /* NGROUPS_MAX */
#endif /* NGROUPS */


#define GOT_READ_DIR 1
#define STRUCT_DIRENT struct dirent

#include "HTML.h"		/* For directory object building */

#include "LYexit.h"
#include "LYLeaks.h"

#define PUTC(c) (*target->isa->put_character)(target, c)
#define PUTS(s) (*target->isa->put_string)(target, s)
#define START(e) (*target->isa->start_element)(target, e, 0, 0)
#define END(e) (*target->isa->end_element)(target, e)
#define FREE_TARGET (*target->isa->_free)(target)
struct _HTStructured {
	CONST HTStructuredClass *	isa;
	/* ... */
};


/*                   Controlling globals
**
*/

PUBLIC int HTDirAccess = HT_DIR_OK;

PUBLIC int HTDirReadme = HT_DIR_README_TOP;

PRIVATE char *HTMountRoot = "/Net/";		/* Where to find mounts */
PRIVATE char *HTCacheRoot = "/tmp/W3_Cache_";   /* Where to cache things */

/* PRIVATE char *HTSaveRoot  = "$(HOME)/WWW/";*/    /* Where to save things */


/*	Suffix registration
*/

PRIVATE HTList * HTSuffixes = 0;
PRIVATE HTSuffix no_suffix = { "*", NULL, NULL, 1.0 };
PRIVATE HTSuffix unknown_suffix = { "*.*", NULL, NULL, 1.0};

/*
 *	To free up the suffixes at program exit.
 */
static void free_suffixes NOPARAMS;

/*	Define the representation associated with a file suffix
**	-------------------------------------------------------
**
**	Calling this with suffix set to "*" will set the default
**	representation.
**	Calling this with suffix set to "*.*" will set the default
**	representation for unknown suffix files which contain a ".".
**
**	If filename suffix is already defined its previous
**	definition is overridden.
*/
PUBLIC void HTSetSuffix ARGS4(
	CONST char *,	suffix,
	CONST char *,	representation,
	CONST char *,	encoding,
        long,          value)
{
    
    HTSuffix * suff;

    if (strcmp(suffix, "*")==0) suff = &no_suffix;
    else if (strcmp(suffix, "*.*")==0) suff = &unknown_suffix;
    else {
	HTList *cur = HTSuffixes;

	while (NULL != (suff = (HTSuffix*)HTList_nextObject(cur))) {
	    if (suff->suffix && 0==strcmp(suff->suffix, suffix))
		break;
	}
	if (!suff) { /* Not found -- create a new node */
	    suff = (HTSuffix*) calloc(1, sizeof(HTSuffix));
	    if (suff == NULL) outofmem(__FILE__, "HTSetSuffix");

	    /*
             *	Memory leak fixed.
	     *	05-28-94 Lynx 2-3-1 Garrett Arch Blythe
	     */	
	    if (!HTSuffixes)	{
		HTSuffixes = HTList_new();
		atexit(free_suffixes);
	    }

	    HTList_addObject(HTSuffixes, suff);
	
	    StrAllocCopy(suff->suffix, suffix);
	}
    }

    suff->rep = HTAtom_for(representation);
   
    /*
     *	Memory leak fixed.
     *	05-28-94 Lynx 2-3-1 Garrett Arch Blythe
     *	Invariant code removed.
     */
    suff->encoding = HTAtom_for(encoding);
    
    suff->quality = value;
}

static void free_suffixes NOARGS	{
/*
 *	Purpose:	Free all added suffixes.
 *	Arguments:	void
 *	Return Value:	void
 *	Remarks/Portability/Dependencies/Restrictions:
 *		To be used at program exit.
 *	Revision History:
 *		05-28-94	created Lynx 2-3-1 Garrett Arch Blythe
 */

	/*
	 *	Loop through all suffixes.
	 */
	while(!HTList_isEmpty(HTSuffixes))	{
		/*
		 *	Free off each item and its members if need be.
		 */
		free(HTList_removeLastObject(HTSuffixes));
	}
	/*
	 *	Free off the list itself.
 	 */
	HTList_delete(HTSuffixes);
}


/*	Send README file
**
**  If a README file exists, then it is inserted into the document here.
*/

#ifdef GOT_READ_DIR
PRIVATE void do_readme ARGS2(HTStructured *, target, CONST char *, localname)
{ 
    FILE * fp;
    char * readme_file_name = 
	malloc(strlen(localname)+ 1 + strlen(HT_DIR_README_FILE) + 1);
    strcpy(readme_file_name, localname);
    strcat(readme_file_name, "/");
    strcat(readme_file_name, HT_DIR_README_FILE);
    
    fp = fopen(readme_file_name,  "r");
    
    if (fp) {
	HTStructuredClass targetClass;
	
	targetClass =  *target->isa;	/* (Can't init agregate in K&R) */

	START(HTML_PRE);
	for(;;){
	    char c = fgetc(fp);
	    if (c == (char)EOF) break;
	    switch (c) {
	    	case '&':
		case '<':
		case '>':
			PUTC('&');
			PUTC('#');
			PUTC((char)(c / 10));
			PUTC((char) (c % 10));
			PUTC(';');
			break;
/*	    	case '\n':
			PUTC('\r');    
Bug removed thanks to joe@athena.mit.edu */			
		default:
			PUTC(c);
	    }
	}
	END(HTML_PRE);
	fclose(fp);
    } 
}
#endif /* GOT_READ_DIR */


#ifdef FIXME
/*	Make the cache file name for a W3 document
**	------------------------------------------
**	Make up a suitable name for saving the node in
**
**	E.g.	/tmp/WWW_Cache_news/1234@cernvax.cern.ch
**		/tmp/WWW_Cache_http/crnvmc/FIND/xx.xxx.xx
**
** On exit,
**	returns	a malloc'ed string which must be freed by the caller.
*/
PUBLIC char * HTCacheFileName ARGS1(CONST char *,name)
{
    char * access = HTParse(name, "", PARSE_ACCESS);
    char * host = HTParse(name, "", PARSE_HOST);
    char * path = HTParse(name, "", PARSE_PATH+PARSE_PUNCTUATION);
    
    char * result;
    result = (char *)malloc(
	    strlen(HTCacheRoot)+strlen(access)
	    +strlen(host)+strlen(path)+6+1);
    if (result == NULL) outofmem(__FILE__, "HTCacheFileName");
    sprintf(result, "%s/WWW/%s/%s%s", HTCacheRoot, access, host, path);
    FREE(path);
    FREE(access);
    FREE(host);
    return result;
}
#endif //fixme

/*	Open a file for write, creating the path
**	----------------------------------------
*/
#ifdef NOT_IMPLEMENTED
PRIVATE int HTCreatePath ARGS1(CONST char *,path)
{
    return -1;
}
#endif /* NOT_IMPLEMENTED */

/*	Convert filenames between local and WWW formats
**	-----------------------------------------------
**	Make up a suitable name for saving the node in
**
**	E.g.	$(HOME)/WWW/news/1234@cernvax.cern.ch
**		$(HOME)/WWW/http/crnvmc/FIND/xx.xxx.xx
**
** On exit,
**	returns	a malloc'ed string which must be freed by the caller.
*/
PUBLIC char * HTLocalName ARGS1(CONST char *,name)
{
    char * access = HTParse(name, "", PARSE_ACCESS);
    char * host = HTParse(name, "", PARSE_HOST);
    char * path = HTParse(name, "", PARSE_PATH+PARSE_PUNCTUATION);
    
    HTUnEscape(path);	/* Interpret % signs */
    
    if (0==strcmp(access, "file")) { /* local file */
        FREE(access);	
	if ((0==strcasecomp(host, HTHostName())) ||
	    (0==strcasecomp(host, "localhost")) || !*host) {
	    FREE(host);
#ifdef DT
	    if (TRACE) fprintf(stderr, "Node `%s' means path `%s'\n", name, path);
#endif
	    return(path);
	} else {
	    char * result = (char *)malloc(
	    			strlen("/Net/")+strlen(host)+strlen(path)+1);
              if (result == NULL) outofmem(__FILE__, "HTLocalName");
	    sprintf(result, "%s%s%s", "/Net/", host, path);
	    FREE(host);
            FREE(path);
#ifdef DT
	    if (TRACE) fprintf(stderr, "Node `%s' means file `%s'\n", name, result);
#endif

	    return result;
	}
    } else {  /* other access */
	char * result;
#ifdef VMS
        char * home =  getenv("HOME");
	if (!home) 
	    home = HTCacheRoot; 
	else
   	    home = HTVMS_wwwName(home);
#else
        CONST char * home =  (CONST char*)getenv("HOME");
	if (!home) home = "/tmp"; 
#endif /* VMS */
	result = (char *)malloc(
		strlen(home)+strlen(access)+strlen(host)+strlen(path)+6+1);
      if (result == NULL) outofmem(__FILE__, "HTLocalName");
	sprintf(result, "%s/WWW/%s/%s%s", home, access, host, path);
	FREE(path);
	FREE(access);
	FREE(host);
	return result;
    }
}


/*	Make a WWW name from a full local path name
**
** Bugs:
**	At present, only the names of two network root nodes are hand-coded
**	in and valid for the NeXT only. This should be configurable in
**	the general case.
*/

PUBLIC char * WWW_nameOfFile ARGS1 (CONST char *,name)
{
    char * result;
#ifdef NeXT
    if (0==strncmp("/private/Net/", name, 13)) {
	result = (char *)malloc(7+strlen(name+13)+1);
	if (result == NULL) outofmem(__FILE__, "WWW_nameOfFile");
	sprintf(result, "file://%s", name+13);
    } else
#endif /* NeXT */
    if (0==strncmp(HTMountRoot, name, 5)) {
	result = (char *)malloc(7+strlen(name+5)+1);
	if (result == NULL) outofmem(__FILE__, "WWW_nameOfFile");
	sprintf(result, "file://%s", name+5);
    } else {
        result = (char *)malloc(7+strlen(HTHostName())+strlen(name)+1);
	if (result == NULL) outofmem(__FILE__, "WWW_nameOfFile");
	sprintf(result, "file://%s%s", HTHostName(), name);
    }
#ifdef DT
    if (TRACE) fprintf(stderr, "File `%s'\n\tmeans node `%s'\n", name, result);
#endif

    return result;
}


/*	Determine a suitable suffix, given the representation
**	-----------------------------------------------------
**
** On entry,
**	rep	is the atomized MIME style representation
**
** On exit,
**	returns	a pointer to a suitable suffix string if one has been
**		found, else "".
*/
PUBLIC CONST char * HTFileSuffix ARGS1(HTAtom*, rep)
{
    HTSuffix * suff;
    int n;
    int i;

#define NO_INIT  /* dont init anymore since I do it in Lynx at startup */
#ifndef NO_INIT    
    if (!HTSuffixes) HTFileInit();
#endif /* !NO_INIT */
    n = HTList_count(HTSuffixes);
    for(i=0; i<n; i++) {
	suff = HTList_objectAt(HTSuffixes, i);
	if (suff->rep == rep) {
	    return suff->suffix;		/* OK -- found */
	}
    }
    return "";		/* Dunno */
}


/*	Determine file format from file name
**	------------------------------------
**
**	This version will return the representation and also set
**	a variable for the encoding.
**
**	It will handle for example  x.txt, x.txt,Z, x.Z
*/

PUBLIC HTFormat HTFileFormat ARGS2 (
			CONST char *,	filename,
			HTAtom **,	pencoding)

{
    HTSuffix * suff;
    int n;
    int i;
    int lf;
#ifdef VMS
    char *semicolon;
#endif /* VMS */
    extern char LYforce_HTML_mode;

    if(LYforce_HTML_mode) {
        LYforce_HTML_mode = FALSE;
	return WWW_HTML;
    }

#ifdef VMS
    /*
     * Trim at semicolon if a version number was
     * included, so it doesn't interfere with the
     * code for getting the MIME type. - FM
     */
    if ((semicolon=strchr(filename, ';')) != NULL)
        *semicolon = '\0';
#endif /* VMS */

#ifndef NO_INIT    
    if (!HTSuffixes) HTFileInit();
#endif /* !NO_INIT */
    *pencoding = NULL;
    lf  = strlen(filename);
    n = HTList_count(HTSuffixes);
    for(i=0; i<n; i++) {
        int ls;
	suff = HTList_objectAt(HTSuffixes, i);
	ls = strlen(suff->suffix);
	if ((ls <= lf) && 0==strcasecomp(suff->suffix, filename + lf - ls)) {
	    int j;
	    *pencoding = suff->encoding;
	    if (suff->rep) {
#ifdef VMS
		if (semicolon != NULL)
		    *semicolon = ';';
#endif /* VMS */
	        return suff->rep;		/* OK -- found */
	    }
	    for(j=0; j<n; j++) {  /* Got encoding, need representation */
		int ls2;
		suff = HTList_objectAt(HTSuffixes, j);
		ls2 = strlen(suff->suffix);
		if ((ls <= lf) && 0==strncasecomp(
			suff->suffix, filename + lf - ls -ls2, ls2)) {
		    if (suff->rep) {
#ifdef VMS
			if (semicolon != NULL)
			    *semicolon = ';';
#endif /* VMS */
		        return suff->rep;
		    }
		}
	    }

	}
    }
    
    /* defaults tree */
    
    suff = strchr(filename, '.') ? 	/* Unknown suffix */
    	 ( unknown_suffix.rep ? &unknown_suffix : &no_suffix)
	 : &no_suffix;
	 
    /* set default encoding unless found with suffix already */
    if (!*pencoding) *pencoding = suff->encoding ? suff->encoding
				    : HTAtom_for("binary");
#ifdef VMS
    if (semicolon != NULL)
        *semicolon = ';';
#endif /* VMS */
    return suff->rep ? suff->rep : WWW_BINARY;
}


/*	Determine value from file name
**	------------------------------
**
*/

PUBLIC long HTFileValue ARGS1 (CONST char *,filename)

{
    HTSuffix * suff;
    int n;
    int i;
    int lf = strlen(filename);

#ifndef NO_INIT    
    if (!HTSuffixes) HTFileInit();
#endif /* !NO_INIT */
    n = HTList_count(HTSuffixes);
    for(i=0; i<n; i++) {
        int ls;
	suff = HTList_objectAt(HTSuffixes, i);
	ls = strlen(suff->suffix);
	if ((ls <= lf) && 0==strcmp(suff->suffix, filename + lf - ls)) {
#ifdef DT
	    if (TRACE) fprintf(stderr, "File: Value of %s is %.3f\n",
			       filename, suff->quality);
#endif

	    return suff->quality;		/* OK -- found */
	}
    }
    return 0.3;		/* Dunno! */
}


/*	Determine write access to a file
**	--------------------------------
**
** On exit,
**	return value	YES if file can be accessed and can be written to.
**
** Bugs:
**	1.	No code for non-unix systems.
**	2.	Isn't there a quicker way?
*/

#ifdef VMS
#define NO_GROUPS
#endif /* VMS */
#ifdef NO_UNIX_IO
#define NO_GROUPS
#endif /* NO_UNIX_IO */
#ifdef PCNFS
#define NO_GROUPS
#endif /* PCNFS */
#ifdef MSDOS
#define NO_GROUPS
#endif /* MSDOS */

#define NO_GROUPS


PUBLIC BOOL HTEditable ARGS1 (CONST char *,filename)
{
#ifdef NO_GROUPS
    return NO;		/* Safe answer till we find the correct algorithm */
#else
    int 	groups[NGROUPS];
    uid_t	myUid;
    int		ngroups;			/* The number of groups  */
    struct stat	fileStatus;
    int		i;
        
    if (stat(filename, &fileStatus))		/* Get details of filename */
	return NO;				/* Can't even access file! */

    ngroups = getgroups(NGROUPS, groups);	/* Groups to which I belong  */
    myUid = geteuid();				/* Get my user identifier */

#ifdef DT
    if (TRACE) {
        int i;
	fprintf(stderr, 
	    "File mode is 0%o, uid=%d, gid=%d. My uid=%d, %d groups (",
    	    (unsigned int) fileStatus.st_mode, fileStatus.st_uid,
	    fileStatus.st_gid,
	    myUid, ngroups);
	for (i=0; i<ngroups; i++) fprintf(stderr, " %d", groups[i]);
	fprintf(stderr, ")\n");
    }
#endif

    if (fileStatus.st_mode & 0002)		/* I can write anyway? */
    	return YES;

    if ((fileStatus.st_mode & 0200)		/* I can write my own file? */
     && (fileStatus.st_uid == myUid))
    	return YES;

    if (fileStatus.st_mode & 0020)		/* Group I am in can write? */
    {
   	for (i=0; i<ngroups; i++) {
            if (groups[i] == fileStatus.st_gid)
		return YES;
	}
    }
#ifdef DT
    if (TRACE) fprintf(stderr, "\tFile is not editable.\n");
#endif

    return NO;					/* If no excuse, can't do */
#endif /* NO_GROUPS */
}


/*	Make a save stream
**	------------------
**
**	The stream must be used for writing back the file.
**	@@@ no backup done
*/
PUBLIC HTStream * HTFileSaveStream ARGS1(HTParentAnchor *, anchor)
{

    CONST char * addr = HTAnchor_address((HTAnchor*)anchor);
    char *  localname = HTLocalName(addr);
    
    FILE* fp = fopen(localname, "wb");
    if (!fp) return NULL;
    
    return HTFWriter_new(fp);
    
}

/*      Output one directory entry
**
*/
PUBLIC void HTDirEntry ARGS3(HTStructured *, target,
		 CONST char * , tail,
		 CONST char *,  entry)
{
    char * relative;
    char * escaped;

    escaped = HTEscape(entry, URL_XPALPHAS);

	/* handle extra slash at end of path */
    if(*tail == '\0') {
	HTStartAnchor(target, NULL, escaped);
    } else {
	/* If empty tail, gives absolute ref below */
	relative = (char*) malloc(strlen(tail) + strlen(escaped)+2);
	if (relative == NULL) outofmem(__FILE__, "DirRead");
	sprintf(relative, "%s/%s", tail, escaped);
	HTStartAnchor(target, NULL, relative);
	FREE(relative);
    }
    FREE(escaped);
}

/*      Output parent directory entry
**
**    This gives the TITLE and H1 header, and also a link
**    to the parent directory if appropriate.
*/
PUBLIC void HTDirTitles ARGS2(HTStructured *, target,
		 HTAnchor * , anchor)

{

    char * logical = HTAnchor_address(anchor);
    char * path = HTParse(logical, "", PARSE_PATH + PARSE_PUNCTUATION);
    char * current;

//printf("\n\n%s %s\n\n",path,current);

    current = strrchr(path, '/');	/* last part or "" */

    {
      char * printable = NULL;

      StrAllocCopy(printable, (current + 1));


      START(HTML_HTML);
      START(HTML_HEAD);
      PUTS("\n");
      HTUnEscape(printable);

      START(HTML_TITLE);
      PUTS(*printable ? printable : "Welcome ");
      PUTS(" directory");
      END(HTML_TITLE);
      PUTS("\n");
      END(HTML_HEAD);
      PUTS("\n");

      START(HTML_H1);
      PUTS(*printable ? printable : "Welcome");
      END(HTML_H1);
      PUTS("\n");
      FREE(printable);
    }

#ifndef NO_PARENT_DIR_REFERENCE
    /*  Make link back to parent directory
     */

    if (strlen(current) > 1) {   /* was a slash AND something else too */
	char * parent;
	char * relative;
	*current++ = 0;
	parent = strrchr(path, '/');  /* penultimate slash */

	relative = (char*) malloc(strlen(current) + 4);
	if (relative == NULL) outofmem(__FILE__, "DirRead");

	if (strlen(parent) < 4)
	    sprintf(relative, "%s/.././", current);
	else
	    sprintf(relative, "%s/..", current);

#ifndef VMS
	{
	    /*
	     *  On Unix, if it's not ftp and the directory cannot
	     *  be read, don't put out a link.
	     *
	     *  On VMS, this problem is dealt with internally by
	     *  HTVMSBrowseDir().
	     */
	    extern BOOLEAN LYisLocalFile PARAMS((char *logical));
	    DIR  * dp=NULL;

	    if (LYisLocalFile(logical)) {
		if ((dp = opendir(relative)) == NULL) {
		    FREE(logical);
		    FREE(relative);
		    FREE(path);
		    return;
		}
		if (dp)
		    closedir(dp);
	    }
	}
#endif /* !VMS */


	HTStartAnchor(target, NULL, relative);
	FREE(relative);

#ifdef DIRED_SUPPORT
	if (dir_list_style != MIXED_STYLE)
#endif /* DIRED_SUPPORT */

	    PUTS("Up to ");

	if (parent) {

#ifdef DIRED_SUPPORT
	   if (dir_list_style == MIXED_STYLE) {
	      PUTS("../");
	   } else {
#else
	   {
#endif /* DIRED_SUPPORT */
	      char * printable = NULL;
	      StrAllocCopy(printable, parent + 1);
	      HTUnEscape(printable);
	      PUTS(printable);
	      FREE(printable);
	   }
	}
//	  else {
//	  PUTS("/");
//	}

	END(HTML_A);
    }
#endif /* NO_PARENT_DIR_REFERENCE */

    FREE(logical);
    FREE(path);
}



/*	Load a document
**	---------------
**
** On entry,
**	addr		must point to the fully qualified hypertext reference.
**			This is the physsical address of the file
**
** On exit,
**	returns		<0		Error has occured.
**			HTLOADED	OK 
**
*/
PUBLIC int HTLoadFile ARGS4 (
	CONST char *,		addr,
	HTParentAnchor *,	anchor,
	HTFormat,		format_out,
	HTStream *,		sink
)
{
    char * filename;
    char * access;
    HTFormat format;
    char * nodename=NULL;
    char * newname=NULL; /* Simplified name of file */
    HTAtom * encoding;	 /* @@ not used yet */
#ifdef VMS
    struct stat stat_info;
#endif /* VMS */


    FREE(nodename);	/* From prev call - Leak fixed AL 6 Feb 1994 */

/*	Reduce the filename to a basic form (hopefully unique!)
*/
    StrAllocCopy(newname, addr);
    filename=HTParse(newname, "", PARSE_PATH|PARSE_PUNCTUATION);
    nodename=HTParse(newname, "", PARSE_HOST);

    /* If access is ftp, or file is on another host, invoke ftp now */
    access = HTParse(newname, "", PARSE_ACCESS);
    if(strcmp("ftp", access) == 0 ||
       (strcmp("localhost", nodename) != 0 &&
#ifdef VMS
	strcasecomp(nodename, HTHostName()) != 0))
#else
	strcmp(nodename, HTHostName()) != 0))
#endif /* VMS */
    {
	FREE(newname);
	FREE(access);
	FREE(filename);
	return HTLoadError(sink, 500,
		"FTP disabled.");
//	return HTFTPLoad(addr, anchor, format_out, sink);
    } else {
	FREE(newname);
	FREE(access);
    }

    format = HTFileFormat(filename, &encoding);

    FREE(filename);

/*	For unix, we try to translate the name into the name of a transparently
**	mounted file.
**
**	Not allowed in secure (HTClienntHost) situations TBL 921019
*/
#ifndef NO_UNIX_IO
    /*  Need protection here for telnet server but not httpd server */

    if (!HTSecure) {		/* try local file system */
	char * localname = HTLocalName(addr);
	struct stat dir_info;
#ifdef LONG_LIST
	char buf[80], type, *datestr;
	struct stat st;
	struct passwd *p;
	struct group *g;
	time_t now;
	static char *pbits[] = { "---", "--x", "-w-", "-wx",
		"r--", "r-x", "rw-", "rwx", 0 };
	static char *psbits[] = { "--S", "--s", "-wS", "-ws",
		"r-S", "r-s", "rwS", "rws", 0 };
#define SEC_PER_YEAR	(60 * 60 * 24 * 365)
#define PBIT(a, n, s)  (s) ? psbits[((a) >> (n)) & 0x7] : \
	pbits[((a) >> (n)) & 0x7]
#endif /* LONG_LIST */

    if(strlen(localname)==3)
	StrAllocCat(localname,"foo\\..");

//printf("\n\n%s\n\n",localname);

#ifdef GOT_READ_DIR

/*			  Multiformat handling
**
**	If needed, scan directory to find a good file.
**  Bug:  we don't stat the file to find the length
*/
	if ( (strlen(localname) > strlen(MULTI_SUFFIX))
	   && (0==strcmp(localname + strlen(localname) - strlen(MULTI_SUFFIX),
			  MULTI_SUFFIX))) {
	    DIR *dp;

	    STRUCT_DIRENT * dirbuf;
            long best = NO_VALUE_FOUND;        /* So far best is bad */
	    HTFormat best_rep = NULL;	/* Set when rep found */
	    STRUCT_DIRENT best_dirbuf;	/* Best dir entry so far */

	    char * base = strrchr(localname, '/');
	    int baselen;

	    if (!base || base == localname) goto forget_multi;
	    *base++ = 0;		/* Just got directory name */
	    baselen = strlen(base)- strlen(MULTI_SUFFIX);
	    base[baselen] = 0;	/* Chop off suffix */

	    dp = opendir(localname);
	    if (!dp) {
forget_multi:
		FREE(localname);
		return HTLoadError(sink, 500,
			"Multiformat: directory scan failed.");
	    }

	    while ((dirbuf = readdir(dp))!=0) {
		/* while there are directory entries to be read */
//		if (dirbuf->d_ino == 0)
//		    continue;	/* if the entry is not being used, skip it */

		if (
#if !defined(SVR4) && !defined(ISC) && !defined(SCO)
//                    (int)dirbuf->d_namlen > baselen &&      /* Match? */
#endif /* !SVR4 && !ISC && !SCO */
		    !strncmp(dirbuf->d_name, base, baselen)) {
		    HTFormat rep = HTFileFormat(dirbuf->d_name, &encoding);
                    long value = HTStackValue(rep, format_out,
						HTFileValue(dirbuf->d_name),
						0.0  /* @@@@@@ */);
		    if (value != NO_VALUE_FOUND) {
#ifdef DT
			if (TRACE) fprintf(stderr,
				"HTFile: value of presenting %s is %f\n",
				HTAtom_name(rep), value);
#endif

			if  (value > best) {
			    best_rep = rep;
			    best = value;
			    best_dirbuf = *dirbuf;
		       }
		    }	/* if best so far */
		 } /* if match */

	    } /* end while directory entries left to read */
	    closedir(dp);

	    if (best_rep) {
		format = best_rep;
		base[-1] = '/';		/* Restore directory name */
		base[0] = 0;
		StrAllocCat(localname, best_dirbuf.d_name);
		goto open_file;

	    } else { 			/* If not found suitable file */
		FREE(localname);
		return HTLoadError(sink, 403,	/* List formats? */
		   "Could not find suitable representation for transmission.");
	    }
	    /*NOTREACHED*/
	} /* if multi suffix */


/*
**	Check to see if the 'localname' is in fact a directory.  If it is
**	create a new hypertext object containing a list of files and
**	subdirectories contained in the directory.  All of these are links
**      to the directories or files listed.
**      NB This assumes the existance of a type 'STRUCT_DIRENT', which will
**      hold the directory entry, and a type 'DIR' which is used to point to
**      the current directory being read.
*/


	if (stat(localname,&dir_info) == -1) {     /* get file information */
				       /* if can't read file information */
#ifdef DT
	    if (TRACE) fprintf(stderr, "HTFile: can't stat %s\n", localname);
#endif


	}  else {		/* Stat was OK */


	    if (((dir_info.st_mode) & S_IFMT) == S_IFDIR) {
		/* if localname is a directory */

		HTStructured* target;		/* HTML object */
		HTStructuredClass targetClass;

		DIR *dp;
		STRUCT_DIRENT * dirbuf;

		char * logical=0;
		char * pathname=0;
		char * tail=0;

		BOOL present[HTML_A_ATTRIBUTES];

		char * tmpfilename = NULL;
		struct stat file_info;

#ifdef DT
		if (TRACE)
		    fprintf(stderr,"%s is a directory\n",localname);
#endif


/*	Check directory access.
**	Selective access means only those directories containing a
**	marker file can be browsed
*/
		if (HTDirAccess == HT_DIR_FORBID) {
		    FREE(localname);
		    return HTLoadError(sink, 403,
		    "Directory browsing is not allowed.");
		}


		if (HTDirAccess == HT_DIR_SELECTIVE) {
		    char * enable_file_name =
			malloc(strlen(localname)+ 1 +
			 strlen(HT_DIR_ENABLE_FILE) + 1);
		    strcpy(enable_file_name, localname);
		    strcat(enable_file_name, "/");
		    strcat(enable_file_name, HT_DIR_ENABLE_FILE);
		    if (stat(enable_file_name, &file_info) != 0) {
			FREE(localname);
			return HTLoadError(sink, 403,
			"Selective access is not enabled for this directory");
		    }
		}


		dp = opendir(localname);
		if (!dp) {
		    FREE(localname);
		    return HTLoadError(sink, 403, "This directory is not readable.");
		}


 /*	Directory access is allowed and possible
 */
		logical = HTAnchor_address((HTAnchor*)anchor);

		pathname = HTParse(logical, "",
					PARSE_PATH + PARSE_PUNCTUATION);
//		if(pathname[strlen(pathname)-1] == '/')
//		    pathname[strlen(pathname)-1] = '\0';

		{
		  char * p = strrchr(pathname, '/');  /* find lastslash */
		  StrAllocCopy(tail, p+1); /* take slash off the beginning */
		}

		FREE(pathname);

		target = HTML_new(anchor, format_out, sink);
		targetClass = *target->isa;	/* Copy routine entry points */

		{ int i;
			for(i=0; i<HTML_A_ATTRIBUTES; i++)
				present[i] = (i==HTML_A_HREF);
		}

		HTDirTitles(target, (HTAnchor *)anchor);
/*
		if (HTDirReadme == HT_DIR_README_TOP)
		    do_readme(target, localname);
*/
		{
		    HTBTree * bt = HTBTree_new((HTComparer)strcasecomp);
		    while ((dirbuf = readdir(dp))!=0)
		    {
			/* while there are directory entries to be read */
			HTBTElement * dirname = NULL;

//		        if (dirbuf->d_ino == 0)
			    /* if the entry is not being used, skip it */
//			    continue;

			if ((*(dirbuf->d_name)=='.') ||
				(*(dirbuf->d_name)==','))
			    /* skip those files whose name begins
			     * with '.' or ',' */
			    continue;

			dirname = (HTBTElement *)malloc(
					strlen(dirbuf->d_name) + 4);
			if (dirname == NULL) outofmem(__FILE__,"DirRead");
			StrAllocCopy(tmpfilename,localname);

			if (strcmp(localname,"/"))
			    /* if filename is not root directory */
			    StrAllocCat(tmpfilename,"/");

			StrAllocCat(tmpfilename,dirbuf->d_name);
			stat(tmpfilename, &file_info);
			if (((file_info.st_mode) & S_IFMT) == S_IFDIR)
			    sprintf((char *)dirname,"D%s",dirbuf->d_name);
			else
			    sprintf((char *)dirname,"F%s",dirbuf->d_name);
			    /* D & F to have first directories, then files */
			HTBTree_add(bt,dirname); /* Sort dirname in the tree bt */
		    }

		    /*    Run through tree printing out in order
		     */
		    {
			HTBTElement * next_element = HTBTree_next(bt,NULL);
			    /* pick up the first element of the list */
			char state;
			    /* I for initial (.. file),
			       D for directory file,
			       F for file */

			state = 'I';

			while (next_element != NULL)
			{
			    char *entry, *file_extra;

			    StrAllocCopy(tmpfilename,localname);
			    if (strcmp(localname,"/"))

					/* if filename is not root directory */
				StrAllocCat(tmpfilename,"/");

			    StrAllocCat(tmpfilename,
					(char *)HTBTree_object(next_element)+1);
			    /* append the current entry's filename to the path */
			    HTSimplify(tmpfilename);
			    /* Output the directory entry */
			    if (strcmp((char *)
					     (HTBTree_object(next_element)),"D.."))
			    {
				if (state != *(char *)(HTBTree_object(next_element)))
				{
				    if (state == 'D')
					END(HTML_DIR);
				    state = *(char *)
					(HTBTree_object(next_element))=='D'?'D':'F';
				    START(HTML_H2);
				    PUTS(state == 'D'?"Subdirectories:":"Files");
				    END(HTML_H2);
				    START(HTML_DIR);
				}
				START(HTML_LI);
			    }
			    entry = (char*)HTBTree_object(next_element)+1;
			    file_extra = NULL;
#ifdef LONG_LIST
			    START(HTML_PRE);
//			    if(lstat(tmpfilename, &st) != -1) {
			    if(stat(tmpfilename, &st) != -1) {
			      char link_name[1025];
			      int file_len;

			      /* mode (permissions) and link count */
			      switch(st.st_mode & S_IFMT) {
			      case S_IFIFO: type = 'p'; break;
			      case S_IFCHR: type = 'c'; break;
			      case S_IFDIR: type = 'd'; break;
			      case S_IFBLK: type = 'b'; break;
			      case S_IFREG: type = '-'; break;
/*
			      case S_IFLNK:
				type = 'l';
				if ((file_len = readlink(tmpfilename,
							 link_name,
							 sizeof(link_name) - 1)
				     ) >=0) {
				    StrAllocCopy(file_extra, " -> ");
				    link_name[file_len] = '\0';
				    StrAllocCat(file_extra, link_name);
				}
				break;
*/
//			      case S_IFSOCK: type = 's'; break;
			      default: type = '?'; break;
			      }
			      sprintf(buf, "    %c%s  %3d ", type,
//			      sprintf(buf, "    %c%s%s%s  %3d ", type,
//        			PBIT(st.st_mode, 6, st.st_mode & S_ISUID),
//        			PBIT(st.st_mode, 3, st.st_mode & S_ISGID),
				PBIT(st.st_mode, 0, 0),
				st.st_nlink);
			      PUTS(buf);

			      /* user */
//			      p = getpwuid(st.st_uid);
//			      if(p)
//				sprintf(buf, "%-8.8s ", p->pw_name);
//			      else
				sprintf(buf, "%-8d ", st.st_uid);
			      PUTS(buf);

			      /* group */
//			      g = getgrgid(st.st_gid);
//			      if(g)
//				sprintf(buf, "%-8.8s ", g->gr_name);
//			      else
				sprintf(buf, "%-8d ", st.st_gid);
			      PUTS(buf);

			      /* size */
			      sprintf(buf, "%7d ", st.st_size);
			      PUTS(buf);

			      /* date */
			      now = time(0);
			      datestr = ctime(&st.st_mtime);
			      if((now - st.st_mtime) < SEC_PER_YEAR/2) 
				/* MMM DD HH:MM */
				sprintf(buf, "%.12s ", datestr + 4);
			      else 
				/* MMM DD  YYYY */
				sprintf(buf, "%.7s %.4s ", datestr + 4, 
				  datestr + 20);
			      PUTS(buf);
			    }
#endif /* LONG_LIST */

			    HTDirEntry(target, tail, entry);
/* dir here */			    PUTS(entry);
    			    END(HTML_A);
			    if (file_extra) {
				PUTS(file_extra);
				FREE(file_extra);
			    }
#ifdef LONG_LIST
  			    END(HTML_PRE);
#endif /* LONG_LIST */

			    next_element = HTBTree_next(bt,next_element);
				/* pick up the next element of the list;
				 if none, return NULL*/
			}
			if (state == 'I')
			{
			    START(HTML_P);
			    PUTS("Empty Directory");
			}
			else
			    END(HTML_DIR);
		    }

			/* end while directory entries left to read */
		    closedir(dp);
		    FREE(logical);
		    FREE(tmpfilename);
		    FREE(tail);
		    HTBTreeAndObject_free(bt);
/*
		    if (HTDirReadme == HT_DIR_README_BOTTOM)
			  do_readme(target, localname);
*/
		    FREE_TARGET;
		    FREE(localname);
		    return HT_LOADED;	/* document loaded */
		}

	    } /* end if localname is directory */
	
	} /* end if file stat worked */
	
/* End of directory reading section
*/
#endif /* GOT_READ_DIR */
open_file:
	{
            FILE * fp = fopen(localname,"rb");

#ifdef DT
	    if(TRACE) fprintf (stderr, "HTFile: Opening `%s' gives %p\n",
				localname, (void*)fp);
#endif

	    if (fp) {		/* Good! */
		if (HTEditable(localname)) {
		    HTAtom * put = HTAtom_for("PUT");
		    HTList * methods = HTAnchor_methods(anchor);
		    if (HTList_indexOf(methods, put) == (-1)) {
			HTList_addObject(methods, put);
		    }
		}
		FREE(localname);
		HTParseFile(format, format_out, anchor, fp, sink);
		fclose(fp);
		return HT_LOADED;
	    }  /* If succesfull open */
	}    /* scope of fp */
    }  /* local unix file system */    
#endif /* !NO_UNIX_IO */

#ifndef DECNET
/*	Now, as transparently mounted access has failed, we try FTP.
*/
    {
	/** Deal with case-sensitivity differences on VMS verus Unix **/
#ifdef VMS
        if (strcasecomp(nodename, HTHostName())!=0)
#else
	if (strcmp(nodename, HTHostName())!=0)
#endif /* VMS */
//	    if(!strncmp(addr,"file://localhost",16))
		return -1;  /* never go to ftp site when URL
			     * is file://localhost
			     */
//	    else
//	        return HTFTPLoad(addr, anchor, format_out, sink);
    }
#endif /* !DECNET */

/*	All attempts have failed.
*/
    {
#ifdef DT
    	if (TRACE)
	    fprintf(stderr, "Can't open `%s', errno=%d\n", addr, SOCKET_ERRNO);
#endif


	return HTLoadError(sink, 403, "Can't access requested file.");
    }
    
 
}

/*		Protocol descriptors
*/
#if defined (GLOBALDEF_IS_MACRO)
#define _HTFILE_C_1_INIT { "ftp", HTLoadFile, 0 }
GLOBALDEF (HTProtocol,HTFTP,_HTFILE_C_1_INIT);
#define _HTFILE_C_2_INIT { "file", HTLoadFile, HTFileSaveStream }
GLOBALDEF (HTProtocol,HTFile,_HTFILE_C_2_INIT);
#else
GLOBALDEF PUBLIC HTProtocol HTFTP  = { "ftp", HTLoadFile, 0 };
GLOBALDEF PUBLIC HTProtocol HTFile = { "file", HTLoadFile, HTFileSaveStream };
#endif
