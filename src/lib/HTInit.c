/*		Configuration-specific Initialialization	HTInit.c
**		----------------------------------------
*/

/*	Define a basic set of suffixes and presentations
**	------------------------------------------------
*/

// #include "spawno.h"
#include "HTUtils.h"

/* Implements:
*/
#include "HTInit.h"

#include "HTML.h"
#include "HTPlain.h"
#include "HTMLGen.h"
#include "HTFile.h"
#include "HTFormat.h"
#include "HTMIME.h"
#include "HTWSRC.h"

#include "HTSaveTo.h"  /* LJM */
#include "userdefs.h"
#include "LYUtils.h"
#include "LYGlobalDefs.h"
#include "LYSystem.h"

#include "LYexit.h"
#include "LYLeaks.h"

#ifdef VMS
#define DISPLAY "DECW$DISPLAY"
#else
#define DISPLAY "DISPLAY"
#endif /* VMS */

PRIVATE int HTLoadTypesConfigFile PARAMS((char *fn));
PRIVATE int HTLoadExtensionsConfigFile PARAMS((char *fn));

PUBLIC void HTFormatInit NOARGS
{
 FILE *fp;
 extern char * personal_type_map, * global_type_map;

#ifdef NeXT
  HTSetPresentation("application/postscript",   "open %s",      1.0, 2.0, 0.0);
  HTSetPresentation("image/x-tiff",             "open %s",      1.0, 2.0, 0.0);
  HTSetPresentation("audio/basic",              "open %s",      1.0, 2.0, 0.0);
  HTSetPresentation("*",                        "open %s",      1.0, 0.0, 0.0);
#else
 if (getenv(DISPLAY)) {       /* Must have X11 */
  HTSetPresentation("application/postscript", "ghostview %s&",  1.0, 3.0, 0.0);
  HTSetPresentation("image/gif",        XLOADIMAGE_COMMAND,     1.0, 3.0, 0.0);
  HTSetPresentation("image/x-xbm",      XLOADIMAGE_COMMAND,     1.0, 3.0, 0.0);
  HTSetPresentation("image/x-rgb",      XLOADIMAGE_COMMAND,     1.0, 3.0, 0.0);
  HTSetPresentation("image/x-tiff",     XLOADIMAGE_COMMAND,     1.0, 3.0, 0.0);
  HTSetPresentation("image/jpeg",       XLOADIMAGE_COMMAND,     1.0, 3.0, 0.0);
  HTSetPresentation("video/mpeg",       "mpeg_play %s &",1.0, 3.0, 0.0);

 }
#endif

#ifdef EXEC_SCRIPTS
 /* set quality to 999.0 for protected exec applications */
#ifndef VMS
 HTSetPresentation("application/x-csh",	"csh %s", 999.0, 3.0, 0.0);
 HTSetPresentation("application/x-sh",	"sh %s",  999.0, 3.0, 0.0);
 HTSetPresentation("application/x-ksh",	"ksh %s", 999.0, 3.0, 0.0);
#else
 HTSetPresentation("application/x-VMS_script",	"@%s", 999.0, 3.0, 0.0);
#endif /* not VMS */
#endif /* EXEC_SCRIPTS */

 HTSetConversion("www/mime",  "www/present",  HTMIMEConvert, 	1.0, 0.0, 0.0);
 HTSetConversion("www/mime",  "www/download", HTMIMEConvert, 	1.0, 0.0, 0.0);
 HTSetConversion("www/mime",  "www/source",   HTMIMEConvert, 	1.0, 0.0, 0.0);
 HTSetConversion("www/mime",  "www/dump",     HTMIMEConvert, 	1.0, 0.0, 0.0);

 HTSetConversion("text/html", "text/x-c",    HTMLToC, 	        0.5, 0.0, 0.0);
 HTSetConversion("text/html", "text/plain",  HTMLToPlain, 	0.5, 0.0, 0.0);
 HTSetConversion("text/html", "www/present", HTMLPresent, 	1.0, 0.0, 0.0);
 HTSetConversion("text/html", "www/source",  HTPlainPresent, 	1.0, 0.0, 0.0);
 HTSetConversion("text/plain","www/present", HTPlainPresent,	1.0, 0.0, 0.0);

 /*
  *	Added the following to support some content types beginning to
  *	surface.
  */
 HTSetConversion("application/html", "text/x-c", HTMLToC, 0.5, 0.0, 0.0);
 HTSetConversion("application/html", "text/plain", HTMLToPlain, 0.5, 0.0, 0.0);
 HTSetConversion("application/html", "www/present", HTMLPresent, 1.0, 0.0, 0.0);
 HTSetConversion("application/html", "www/source", HTPlainPresent, 1.0, 0.0, 0.0);

 HTSetConversion("application/x-wais-source", "www/source",  	     
					HTPlainPresent, 	1.0, 0.0, 0.0);
 HTSetConversion("application/x-wais-source", "www/present",  	     
				        HTWSRCConvert,		1.0, 0.0, 0.0);
 HTSetConversion("application/x-wais-source", "www/download",  	     
					HTWSRCConvert, 		1.0, 0.0, 0.0);
 HTSetConversion("application/x-wais-source", "www/dump",  	     
					HTWSRCConvert, 		1.0, 0.0, 0.0);

  /* save all unknown mime types to disk */
 HTSetConversion("www/source",  "www/present",  HTSaveToFile ,  1.0, 3.0, 0.0);
 HTSetConversion("www/source",  "www/source",   HTSaveToFile ,  1.0, 3.0, 0.0);
 HTSetConversion("www/source",  "www/download", HTSaveToFile ,  1.0, 3.0, 0.0);
 HTSetConversion("www/source",  "*", 	        HTSaveToFile ,  1.0, 3.0, 0.0);

 /* output all www/dump presentations to stdout */
 HTSetConversion("www/source",  "www/dump",     HTDumpToStdout, 1.0, 3.0, 0.0);

  /* These should override the default types as necessary. */
 HTLoadTypesConfigFile (global_type_map);

  /* load the local maps */
 if((fp = fopen(personal_type_map,"r")) != NULL) {
    fclose(fp);
    /* These should override everything else. */
    HTLoadTypesConfigFile (personal_type_map);
 } 

}

/* Some of the following is taken from: */

/*
Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore)

Permission to use, copy, modify, and distribute this material 
for any purpose and without fee is hereby granted, provided 
that the above copyright notice and this permission notice 
appear in all copies, and that the name of Bellcore not be 
used in advertising or publicity pertaining to this 
material without the specific, prior written permission 
of an authorized representative of Bellcore.  BELLCORE 
MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY 
OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS", 
WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
*/
/****************************************************** 
    Metamail -- A tool to help diverse mail readers 
                cope with diverse multimedia mail formats.

    Author:  Nathaniel S. Borenstein, Bellcore

 ******************************************************* */

struct MailcapEntry {
    char *contenttype;
    char *command;
    char *testcommand;
    int needsterminal;
    int copiousoutput;
    int needtofree;
    char *label;
    char *printcommand;
};


PRIVATE int ExitWithError PARAMS((char *txt));
PRIVATE int PassesTest PARAMS((struct MailcapEntry *mc));

#define LINE_BUF_SIZE       2000
#define TMPFILE_NAME_SIZE 127

PRIVATE char *GetCommand ARGS2(char *,s, char **,t)
{
    char *s2;
    int quoted = 0;

    /* marca -- added + 1 for error case -- oct 24, 1993. */
    s2 = malloc(strlen(s)*2 + 1); /* absolute max, if all % signs */

    *t = s2;
    while (s && *s) {
	if (quoted) {
            if (*s == '%') *s2++ = '%'; /* Quote through next level, ugh! */

            *s2++ = *s++;
	    quoted = 0;
	} else {
	    if (*s == ';') {
                *s2 = 0;
		return(++s);
	    }
	    if (*s == '\\') {
		quoted = 1;
		++s;
	    } else {
		*s2++ = *s++;
	    }
	}
    }
    *s2 = 0;
    return(NULL);
}	

/* no leading or trailing space, all lower case */
PRIVATE char *Cleanse ARGS1(char *,s) 
{
    char *tmp, *news;
    
    /* strip leading white space */
    while (*s && isspace((unsigned char) *s)) ++s;
    news = s;
    /* put in lower case */
    for (tmp=s; *tmp; ++tmp) {
      *tmp = TOLOWER ((unsigned char)*tmp);
    }
    /* strip trailing white space */
    while (*--tmp && isspace((unsigned char) *tmp)) *tmp = 0;
    return(news);
}

PRIVATE int ProcessMailcapEntry ARGS2(FILE *,fp, struct MailcapEntry *,mc)
{
    int rawentryalloc = 2000, len;
    char *rawentry, *s, *t, *LineBuf;

    LineBuf = malloc(LINE_BUF_SIZE);
    if (!LineBuf) 
      ExitWithError("Out of memory");
    rawentry = malloc(1 + rawentryalloc);
    if (!rawentry) 
      ExitWithError("Out of memory");
    *rawentry = 0;
    while (fgets(LineBuf, LINE_BUF_SIZE, fp)) {
	if (LineBuf[0] == '#') 
	  continue;
        len = strlen(LineBuf);
        if (len == 0) 
	  continue;
        if (LineBuf[len-1] == '\n') 
	  LineBuf[--len] = 0;
	if ((len + strlen(rawentry)) > rawentryalloc) {
	    rawentryalloc += 2000;
	    rawentry = realloc(rawentry, rawentryalloc+1);
	    if (!rawentry) ExitWithError("Out of memory");
	}
	if (len > 0 && LineBuf[len-1] == '\\') {
            LineBuf[len-1] = 0;
	    strcat(rawentry, LineBuf);
	} else {
	    strcat(rawentry, LineBuf);
	    break;
	}
    }

    free(LineBuf);

    for (s=rawentry; *s && isspace((unsigned char) *s); ++s) ;
    if (!*s) {
	/* totally blank entry -- quietly ignore */
	free(rawentry);
	return(0);
    }
    s = strchr(rawentry, ';');
    if (s == NULL) {
#ifdef DT
	if(TRACE)	{
		fprintf(stderr, "metamail: Ignoring invalid mailcap entry: %s\n", rawentry);
	}
#endif

	free(rawentry);
	return(0);
    }
    *s++ = 0;
    mc->needsterminal = 0;
    mc->copiousoutput = 0;
    mc->needtofree = 1;
    mc->testcommand = 0;
    mc->label = NULL;
    mc->printcommand = NULL;
    mc->contenttype = malloc(1+strlen(rawentry));
    if (!mc->contenttype) ExitWithError("Out of memory");
    strcpy(mc->contenttype, rawentry);
    t = GetCommand(s, &mc->command);
    if (!t) {
        goto assign_presentation;
    }
    s = t;
    while (s && *s && isspace((unsigned char) *s)) ++s;
    while (s) {
	char *arg, *eq, *mallocd_string;

        t = GetCommand(s, &mallocd_string);
	arg = mallocd_string;
        eq = strchr(arg, '=');
        if (eq) *eq++ = 0;
	if (arg && *arg) {
	    arg = Cleanse(arg);
	    if (!strcmp(arg, "needsterminal")) {
		mc->needsterminal = 1;
	    } else if (!strcmp(arg, "copiousoutput")) {
		mc->copiousoutput = 1;
	    } else if (eq && !strcmp(arg, "test")) {
		mc->testcommand = 0;
		StrAllocCopy(mc->testcommand, eq);
#ifdef DT
		if(TRACE)
		    fprintf(stderr,"[HTInit]: found testcommand:%s\n",mc->testcommand);
#endif

	    } else if (eq && !strcmp(arg, "description")) {
		mc->label = eq;
	    } else if (eq && !strcmp(arg, "label")) { 
		mc->label = eq; /* bogus old name for description */
	    } else if (eq && !strcmp(arg, "print")) {
		mc->printcommand = eq;
	    } else if (eq && !strcmp(arg, "textualnewlines")) {
		/* no support for now.  What does this do anyways? */
		/* ExceptionalNewline(mc->contenttype, atoi(eq)); */
	    }
#ifdef DT
            else if (strcmp(arg, "notes")) { /* IGNORE notes field */
		if (*arg && TRACE) fprintf(stderr, "metamail: Ignoring mailcap flag: %s\n", arg);
	    }
#endif


	}
      if(mallocd_string);
      	free(mallocd_string);
      s = t;
    }

assign_presentation:
    free(rawentry);

    if(PassesTest(mc)) {
#ifdef DT
	if(TRACE)
	    fprintf(stderr,"[HTInit] Setting up conversion %s : %s\n",
						mc->contenttype, mc->command);
#endif

	HTSetPresentation(mc->contenttype, mc->command, 1.0, 3.0, 0.0);
    }
    if(mc->command)
	free(mc->command);
    if(mc->contenttype)
    	free(mc->contenttype);

    return(1);
}

PRIVATE void BuildCommand ARGS3(char *,Buf, char *,controlstring, 
							char *,TmpFileName)
{
    char *from, *to;
    int prefixed = 0; 

    for (from=controlstring, to=Buf; *from != '\0'; from++) {
        if (prefixed) {
	    prefixed = 0;
            switch(*from) {
                case '%':
                    *to++ = '%';
                    break;
		case 'n':
		case 'F':
#ifdef DT
			if(TRACE)	{
		     		fprintf(stderr, "metamail: Bad mailcap \"test\" clause: %s\n", controlstring);
			}
#endif

		case 's':
		    if (TmpFileName) {
                        strcpy(to, TmpFileName);
                        to += strlen(TmpFileName);
                    }
                    break;
                default:
#ifdef DT
			if(TRACE)	{
                    		fprintf(stderr, "Ignoring unrecognized format code in mailcap file: %%%c\n", *from);
			}
#endif

                    break;
            }
	} else if (*from == '%') {
            prefixed = 1;
        } else {
            *to++ = *from;
        }
    }
    *to = 0;
}

PRIVATE int PassesTest ARGS1(struct MailcapEntry *,mc)
{
    int result;
    char *cmd, TmpFileName[TMPFILE_NAME_SIZE];

    /*
     * Make sure we have a command
     */
    if (!mc->testcommand) return(1);

    /*
     * Save overhead of system() calls by faking these. - FM
     */
    if (0==strcasecomp(mc->testcommand, "test -n \"$DISPLAY\"")) {
#ifdef DT
        if (TRACE) fprintf(stderr,"Testing for XWINDOWS environment.\n");
#endif

    	if(getenv(DISPLAY) != NULL) {
#ifdef DT
	    if(TRACE)
	        fprintf(stderr,"[HTInit] Test passed!\n");
#endif

	    return(0==0);
	} else {
#ifdef DT
	    if(TRACE)
	        fprintf(stderr,"[HTInit] Test failed!\n");
#endif

	    return(-1==0);
	}
    }
    if (0==strcasecomp(mc->testcommand, "test -z \"$DISPLAY\"")) {
#ifdef DT
        if (TRACE) fprintf(stderr,"Testing for NON_XWINDOWS environment.\n");
#endif

    	if(getenv(DISPLAY) == NULL) {
#ifdef DT
	    if(TRACE)
	        fprintf(stderr,"[HTInit] Test passed!\n");
#endif

	    return(0==0);
	} else {
#ifdef DT
	    if(TRACE)
	        fprintf(stderr,"[HTInit] Test failed!\n");
#endif

	    return(-1==0);
	}
    }

    /*
     * Why do anything but return success for this one! - FM
     */
    if (0==strcasecomp(mc->testcommand, "test -n \"$LYNX_VERSION\"")){
#ifdef DT
        if (TRACE) {
	    fprintf(stderr,"Testing for LYNX environment.\n");
	    fprintf(stderr,"[HTInit] Test passed!\n");
	}
#endif

	return(0==0);
    } else
    /*
     * ... or failure for this one! - FM
     */
    if (0==strcasecomp(mc->testcommand, "test -z \"$LYNX_VERSION\"")){
#ifdef DT
        if (TRACE) {
	    fprintf(stderr,"Testing for non-LYNX environment.\n");
	    fprintf(stderr,"[HTInit] Test failed!\n");
	}
#endif

	return(-1==0);
    }

    /*
     * Build the command and execute it.
     */
    tempname(TmpFileName, NEW_FILE);
    cmd = (char *)malloc(1024);
    if (!cmd) ExitWithError("Out of memory");
    BuildCommand(cmd, mc->testcommand, TmpFileName);
#ifdef DT
    if (TRACE) fprintf(stderr,"Executing test command: %s\n", cmd);
#endif

    result = system(cmd);
    free(cmd);

    /* free the test command as well since
     * we wont be needing it anymore
     */
    free(mc->testcommand);
    mc->testcommand = 0;

#ifdef DT
    if(TRACE && result)
	fprintf(stderr,"[HTInit] Test failed!\n");
    else if(TRACE)
	fprintf(stderr,"[HTInit] Test passed!\n");
#endif

    return(result==0);
}

PRIVATE int ProcessMailcapFile ARGS1(char *,file)
{
    struct MailcapEntry mc;
    FILE *fp;

#ifdef DT
    if (TRACE)
      fprintf (stderr, "Loading types config file '%s'\n",
               file);
#endif

    fp = fopen(file, "r");

//    if(fp) printf ("Processing '%s'\n", file);

    while (fp && !feof(fp)) {
        ProcessMailcapEntry(fp, &mc);
    }
    if (fp) fclose(fp);
    return(-1);
}

PRIVATE int ExitWithError ARGS1(char *,txt)
{
    if (txt) fprintf(stderr, "metamail: %s\n", txt);
    exit(-1);
    return(-1);
}


PRIVATE int HTLoadTypesConfigFile ARGS1(char *,fn)
{
  return ProcessMailcapFile (fn);
}




/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

/*	Define a basic set of suffixes
**	------------------------------
**
**	The LAST suffix for a type is that used for temporary files
**	of that type.
**	The quality is an apriori bias as to whether the file should be
**	used.  Not that different suffixes can be used to represent files
**	which are of the same format but are originals or regenerated,
**	with different values.
*/

PUBLIC void HTFileInit NOARGS
{
  FILE *fp;
  extern char *global_extension_map;
  extern char *personal_extension_map;

#ifdef DT
      if (TRACE)
        fprintf (stderr, "@@@ Using default extension map\n");
#endif

       /* default suffix interpretation */
      HTSetSuffix("*","text/plain", "7bit", 1.0);
      HTSetSuffix("*.*","text/plain", "7bit", 1.0);

#ifdef EXEC_SCRIPTS
     /* define these extentions for exec scripts
      */
        /* for csh exec links */
#ifndef VMS
      HTSetSuffix(".csh", "application/x-csh", "8bit", 0.8);
      HTSetSuffix(".sh",  "application/x-sh", "8bit", 0.8);
      HTSetSuffix(".ksh", "application/x-ksh", "8bit", 0.8);
#else
      HTSetSuffix(".com", "application/x-VMS_script", "8bit", 0.8);
#endif /* not VMS */
#endif /* EXEC_SCRIPTS */


      HTSetSuffix(".uu",	"application/x-UUencoded", "binary", 1.0); 
      HTSetSuffix(".saveme",	"application/x-Binary", "binary", 1.0);
      HTSetSuffix(".dump",	"application/x-Binary", "binary", 1.0);
      HTSetSuffix(".hqx",       "application/x-Binhex", "binary", 1.0);
      HTSetSuffix(".arc",       "application/x-Compressed", "binary", 1.0);
      HTSetSuffix(".o",         "application/x-Prog. Object", "binary", 1.0);
      HTSetSuffix(".a",         "application/x-Prog. Library", "binary", 1.0);
      HTSetSuffix(".bin",	"application/x-Binary", "binary", 1.0);
      HTSetSuffix(".exe",	"application/x-Executable", "binary", 1.0);
      HTSetSuffix(".exe.z","application/x-Comp. Executable", "binary", 1.0);
      
      HTSetSuffix(".z",	        "application/UNIX Compressed", "binary", 1.0);
      HTSetSuffix(".tar.z","application/x-Comp. Tar File","binary", 1.0);
      HTSetSuffix(".gz",	"application/GNU Compressed", "binary", 1.0);

      HTSetSuffix(".oda",       "application/ODA", "binary", 1.0);

      HTSetSuffix(".pdf",	"application/PDF", "binary", 1.0);

      HTSetSuffix(".eps",	"application/Postscript", "binary", 1.0);
      HTSetSuffix(".ai",	"application/Postscript", "binary", 1.0);
      HTSetSuffix(".ps",	"application/Postscript", "binary", 1.0);
      
      HTSetSuffix(".rtf",	"application/RTF", "binary", 1.0);

      HTSetSuffix(".dvi","application/x-DVI", "binary", 1.0);

      HTSetSuffix(".hdf","application/x-HDF", "binary", 1.0);
      
      HTSetSuffix(".latex", "application/x-Latex", "binary", 1.0);

      HTSetSuffix(".cdf","application/x-netcdf", "binary", 1.0);
      HTSetSuffix(".nc","application/x-netcdf", "binary", 1.0);

      HTSetSuffix(".tex",  "application/x-Tex", "binary", 1.0);
      
      HTSetSuffix(".texinfo",       "application/x-Texinfo",    "binary", 1.0);
      HTSetSuffix(".texi",          "application/x-Texinfo",    "binary", 1.0);

      HTSetSuffix(".t",    "application/x-Troff", "binary", 1.0);
      HTSetSuffix(".tr",   "application/x-Troff", "binary", 1.0);
      HTSetSuffix(".roff", "application/x-Troff", "binary", 1.0);
      HTSetSuffix(".man",  "application/x-Troff-man", "binary", 1.0);
      HTSetSuffix(".me",   "application/x-Troff-me", "binary", 1.0);
      HTSetSuffix(".ms",   "application/x-Troff-ms", "binary", 1.0);

      HTSetSuffix(".src",	"application/x-WAIS-source", "binary", 1.0);
      HTSetSuffix(".wsrc",	"application/x-WAIS-source", "binary", 1.0); /* xtra */

      HTSetSuffix(".zip", "application/x-Zip File", "binary", 1.0);

      HTSetSuffix(".bcpio",   "application/x-bcpio", "binary", 1.0);
      HTSetSuffix(".cpio",    "application/x-cpio", "binary", 1.0);
      HTSetSuffix(".gtar",    "application/x-gtar", "binary", 1.0);
      HTSetSuffix(".shar",    "application/x-shar", "binary", 1.0);
      HTSetSuffix(".sh",      "application/x-sh", "binary", 1.0); /* xtra */
      HTSetSuffix(".sv4cpio", "application/x-sv4cpio", "binary", 1.0);
      HTSetSuffix(".sv4crc",  "application/x-sv4crc", "binary", 1.0);
      HTSetSuffix(".tar",     "application/x-Tar File", "binary", 1.0);
      HTSetSuffix(".ustar",   "application/x-ustar", "binary", 1.0);

      HTSetSuffix(".snd",  "audio/basic", "binary", 1.0);
      HTSetSuffix(".au",   "audio/basic", "binary", 1.0);
      HTSetSuffix(".aifc", "audio/x-aiff", "binary", 1.0);
      HTSetSuffix(".aif",  "audio/x-aiff", "binary", 1.0);
      HTSetSuffix(".aiff", "audio/x-aiff", "binary", 1.0);
      HTSetSuffix(".wav",  "audio/x-wav", "binary", 1.0);
      
      HTSetSuffix(".gif", "image/gif", "binary", 1.0);

      HTSetSuffix(".ief", "image/ief", "binary", 1.0);

      HTSetSuffix(".jfif","image/jpeg", "binary", 1.0); /* xtra */
      HTSetSuffix(".jfif-tbnl","image/jpeg", "binary", 1.0); /* xtra */
      HTSetSuffix(".jpe", "image/jpeg", "binary", 1.0);
      HTSetSuffix(".jpg", "image/jpeg", "binary", 1.0);
      HTSetSuffix(".jpeg","image/jpeg", "binary", 1.0);
      
      HTSetSuffix(".tif", "image/tiff", "binary", 1.0);
      HTSetSuffix(".tiff","image/tiff", "binary", 1.0);
            
      HTSetSuffix(".ras", "image/x-cmu-rast", "binary", 1.0);
      HTSetSuffix(".pnm", "image/x-portable-anymap", "binary", 1.0);
      HTSetSuffix(".pbm", "image/x-portable-bitmap", "binary", 1.0);
      HTSetSuffix(".pgm", "image/x-portable-graymap", "binary", 1.0);
      HTSetSuffix(".ppm", "image/x-portable-pixmap", "binary", 1.0);
      HTSetSuffix(".rgb", "image/x-rgb", "binary", 1.0);
      HTSetSuffix(".xbm", "image/x-xbitmap", "binary", 1.0);
      HTSetSuffix(".xpm", "image/x-xpixmap", "binary", 1.0);
      HTSetSuffix(".xwd", "image/x-xwindowdump", "binary", 1.0);

      HTSetSuffix(".htm",   "text/html", "binary", 1.0);
      HTSetSuffix(".html",  "text/html", "binary", 1.0);

      HTSetSuffix(".text",     "text/plain", "binary", 1.0);
      HTSetSuffix(".c",	       "text/plain", "binary", 1.0);
      HTSetSuffix(".cc",       "text/plain", "binary", 1.0);
      HTSetSuffix(".c++",      "text/plain", "binary", 1.0);
      HTSetSuffix(".h",	       "text/plain", "binary", 1.0);
      HTSetSuffix(".pl",       "text/plain", "binary", 1.0);
      HTSetSuffix(".txt",      "text/plain", "binary", 1.0);
            
      HTSetSuffix(".rtx", "text/richtext", "binary", 1.0); /* MIME richtext */
      HTSetSuffix(".tsv", "text/tab-separated-values", "binary", 1.0);
      HTSetSuffix(".etx", "text/x-setext", "binary", 1.0);

      HTSetSuffix(".mpg",  "video/mpeg", "binary", 1.0);
      HTSetSuffix(".mpe",  "video/mpeg", "binary", 1.0);
      HTSetSuffix(".mpeg", "video/mpeg", "binary", 1.0);

      HTSetSuffix(".mov", "video/quicktime", "binary", 1.0);
      HTSetSuffix(".qt",  "video/quicktime", "binary", 1.0);

      HTSetSuffix(".avi", "video/x-msvideo", "binary", 1.0);

      HTSetSuffix(".movie", "video/x-sgi-movie", "binary", 1.0);
      HTSetSuffix(".mv",    "video/x-sgi-movie", "binary", 1.0);

      HTSetSuffix(".mime", "message/rfc822", "binary", 1.0);

  /* These should override the default extensions as necessary. */
  HTLoadExtensionsConfigFile (global_extension_map);
  
 if((fp = fopen(personal_extension_map,"r")) != NULL) {
    fclose(fp);
    /* These should override everything else. */
    HTLoadExtensionsConfigFile (personal_extension_map);
 } 
}



/* -------------------- Extension config file reading --------------------- */

/* The following is lifted from NCSA httpd 1.0a1, by Rob McCool;
   NCSA httpd is in the public domain, as is this code. */

#define MAX_STRING_LEN 256

PRIVATE int getline2 ARGS3(char *,s, int,n, FILE *,f)
{
  register int i=0;
  
  while(1) 
    {
      s[i] = (char)fgetc(f);
      
      if(s[i] == CR)
        s[i] = fgetc(f);
    
      if((s[i] == EOF) || (s[i] == LF) || (i == (n-1)))
	{
          s[i] = '\0';
          return (feof(f) ? 1 : 0);
        }
      ++i;
    }

  /* NOTREACHED */
}

PRIVATE void getword(char * word, char * line)
{
  int x = 0, y;

  for (x = 0; line[x] && line[x] != ' ' && line[x] != '\t'; x++)
    {
      word[x] = line[x];
    }

  word[x] = '\0';
  if (line[x])
    ++x;
  y=0;

  while ((line[y++] = line[x++]))
    ;

  return;
}

PRIVATE int HTLoadExtensionsConfigFile ARGS1(char *,fn)
{
  char l[MAX_STRING_LEN],w[MAX_STRING_LEN],*ct;
  FILE *f;
  int x, count = 0;

#ifdef DT
  if (TRACE)
    fprintf (stderr, "Loading extensions config file '%s'\n",
             fn);
#endif

//    printf ("Loading extensions config file '%s'\n",fn);
  

  if(!(f = fopen(fn,"r")))
    {
#ifdef DT
      if (TRACE)
        fprintf (stderr, "Could not open extensions config file '%s'\n",fn);
#endif

//    printf ("Could not open extensions config file '%s'\n",fn);
      return -1;
    }


  while(!(getline2(l,MAX_STRING_LEN,f)))
    {
      getword(w,l);
      if(l[0] == '\0' || w[0] == '#')
	continue;
      ct = (char *)malloc(sizeof(char) * (strlen(w) + 1));
      strcpy(ct,w);

      while(l[0])
	{
	  getword(w,l);
	  if(w[0] && (w[0] != ' '))
	    {
	      char *ext = (char *)malloc(sizeof(char) * (strlen(w)+1+1));

	      for(x=0; w[x]; x++)
		ext[x+1] = TOLOWER(w[x]);
	      ext[0] = '.';
	      ext[strlen(w)+1] = 0;

#ifdef DT
	      if (TRACE)
		fprintf (stderr, "SETTING SUFFIX '%s' to '%s'\n", ext, ct);
#endif

	      HTSetSuffix (ext, ct, "binary", 1.0);
	      count++;

	      free (ext);
	    }
	}
      free(ct);
    }

  fclose(f);

  return count;
}
