/*		FILE WRITER				HTFWrite.h
**		===========
**
**	This version of the stream object just writes to a C file.
**	The file is assumed open and left open.
**
**	Bugs:
**		strings written must be less than buffer size.
*/

#include "HTUtils.h"
#include "LYCurses.h"
#include "HTFWrite.h"

#include "HTFormat.h"
#include "HTAlert.h"
#include "HTFile.h"
#include "HTPlain.h"

#include "LYString.h"
#include "LYUtils.h"
#include "LYGlobal.h"
#include "LYSignal.h"
#include "LYSystem.h"
#include "GridText.h"
#include "LYKeymap.h"
#include "LYexit.h"
#include "LYLeaks.h"

PUBLIC char * WWW_Download_File=0; /* contains the name of the temp file
				    * which is being downloaded into
				    */
PUBLIC char LYCancelDownload=FALSE;  /* exported to HTFormat.c in libWWW */

extern char dump_output_immediately; /* if true dump to stdout and quit */
#ifdef VMS
extern BOOLEAN HadVMSInterrupt;      /* flag from cleanup_sig()		*/
PRIVATE char * FIXED_RECORD_COMMAND = NULL;
#define FIXED_RECORD_COMMAND_MASK "@Lynx_Dir:FIXED512 %s"
#endif /* VMS */

/*		Stream Object
**		------------
*/

struct _HTStream {
	CONST HTStreamClass *	isa;
	
	FILE *			fp;
	char * 			end_command;
	char * 			remove_command;
};


/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
**  Bug:
**	All errors are ignored.
*/

/*	Character handling
**	------------------
*/

PRIVATE void HTFWriter_put_character ARGS2(HTStream *, me, char, c)
{
    putc(c, me->fp);
}



/*	String handling
**	---------------
**
**	Strings must be smaller than this buffer size.
*/
PRIVATE void HTFWriter_put_string ARGS2(HTStream *, me, CONST char*, s)
{
    fputs(s, me->fp);
}


/*	Buffer write.  Buffers can (and should!) be big.
**	------------
*/
PRIVATE void HTFWriter_write ARGS3(HTStream *, me, CONST char*, s, int, l)
{
    fwrite(s, 1, l, me->fp); 
}




/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created
**	object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/

PRIVATE void HTFWriter_free ARGS1(HTStream *, me)
{

    fflush(me->fp);
    if (me->end_command) {		/* Temp file */
    	fclose(me->fp);
#ifdef VMS
	if (0==strcmp(me->end_command, "SaveVMSBinaryFile")) {
	    system(FIXED_RECORD_COMMAND);
	    free(FIXED_RECORD_COMMAND);
	} else
#endif /* VMS */
	if (strcmp(me->end_command, "SaveToFile")) {
	    if(!dump_output_immediately) {
                _HTProgress(me->end_command);  /* Tell user what's happening */
	        stop_curses();
	    }
	    system(me->end_command);

            if (me->remove_command) {
	        /* NEVER REMOVE THE FILE unless during an abort!!!*/
	        /* system(me->remove_command); */
		free(me->remove_command);
	    }
	    if(!dump_output_immediately)
	        start_curses();
	}
	free (me->end_command);
    }

    free(me);

    if(dump_output_immediately)
       exit(0);
}

/*	Abort writing
*/

PRIVATE void HTFWriter_abort ARGS2(HTStream *, me, HTError, e)
{
#ifdef DT
    if(TRACE)
       fprintf(stderr,"HTFWriter_abort called\n");
#endif


    fclose(me->fp);
    if (me->end_command) {              /* Temp file */
#ifdef DT
        if (TRACE) fprintf(stderr,
                "HTFWriter: Aborting: file not executed.\n");
#endif

        free (me->end_command);
        if (me->remove_command) {
            free(me->remove_command);
        }
    }

    if(WWW_Download_File) { /* get rid of it */
        free(WWW_Download_File);
        WWW_Download_File=0;
    }

    free(me);
}


/*	Structured Object Class
**	-----------------------
*/
PRIVATE CONST HTStreamClass HTFWriter = /* As opposed to print etc */
{		
	"FileWriter",
	HTFWriter_free,
	HTFWriter_abort,
	HTFWriter_put_character, 	HTFWriter_put_string,
	HTFWriter_write
}; 


/*	Subclass-specific Methods
**	-------------------------
*/

PUBLIC HTStream* HTFWriter_new ARGS1(FILE *, fp)
{
    HTStream* me;
    
    if (!fp) return NULL;

    me = (HTStream*)calloc(sizeof(*me),1);
    if (me == NULL) outofmem(__FILE__, "HTFWriter_new");
    me->isa = &HTFWriter;       

    me->fp = fp;
    me->end_command = NULL;
    me->remove_command = NULL;

    return me;
}

/*	Make system command from template
**	---------------------------------
**
**	See mailcap spec for description of template.
*/
/* @@ to be written.  sprintfs will do for now.  */


#define REMOVE_COMMAND " "

/*	Take action using a system command
**	----------------------------------
**
**	originally from Ghostview handling by Marc Andreseen.
**	Creates temporary file, writes to it, executes system command
**	on end-document.  The suffix of the temp file can be given
**	in case the application is fussy, or so that a generic opener can
**	be used.
*/
PUBLIC HTStream* HTSaveAndExecute ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	/* Not used */
	HTStream *,		sink)	/* Not used */
{
    char *fnam;
    CONST char * suffix;
    HTStream* me;

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
    if(pres->quality == 999.0) { /* exec link */
        if (no_exec) {
            _statusline("Execution is disabled.");
            sleep(sleep_two);
            return HTPlainPresent(pres, anchor, sink);
        }
	if(!local_exec) 
	   if(local_exec_on_local_files &&
	      (LYJumpFileURL ||
	       !strncmp(anchor->address,"file://localhost",16))) {
		/* allow it to continue */
	   } else {
		char buf[512];

		sprintf(buf, "Execution is not enabled for this file. See the Options menu (use %s).", key_for_func(LYK_OPTIONS));
		_statusline(buf);
		sleep(sleep_two);
		return HTPlainPresent(pres, anchor, sink);
	   }
    }
#endif /* EXEC_LINKS || EXEC_SCRIPTS */
    
    me = (HTStream*)calloc(sizeof(*me),1);
    if (me == NULL) outofmem(__FILE__, "HTSaveAndExecute");
    me->isa = &HTFWriter;

    /* Save the file under a suitably suffixed name */

    suffix = HTFileSuffix(pres->rep);

    fnam = (char *)malloc(64);
    tempname (fnam, 2);  /* lynx routine to create a filename */

    if (suffix) strcat(fnam, suffix);

    me->fp = fopen (fnam, "wb");

    if (!me->fp) {
	HTAlert("Can't open temporary file!");
        free(fnam);
	free(me);
	return NULL;
    }

/*	Make command to process file
*/
    me->end_command = (char *)calloc (
    			(strlen (pres->command) + 10+ 3*strlen(fnam))
    			 * sizeof (char),1);
    if (me == NULL) outofmem(__FILE__, "HTSaveAndExecute");
    
    sprintf (me->end_command, pres->command, fnam);

/*	Make command to delete file
    me->remove_command = (char *)calloc (
			(strlen (REMOVE_COMMAND) + 10+ strlen(fnam))
			 * sizeof (char),1);
    if (me == NULL) outofmem(__FILE__, "HTSaveAndExecute");

    sprintf (me->remove_command, REMOVE_COMMAND, fnam);
*/

    free (fnam);
    return me;
}


/*	Format Converter using system command
**	-------------------------------------
*/

/* @@@@@@@@@@@@@@@@@@@@@@ */

/*      Save to a local file   LJM!!!
**      --------------------
**
**      usually a binary file that can't be displayed
**
**      originally from Ghostview handling by Marc Andreseen.
**      Asks the user if he wants to continue, creates a temporary
**      file, and writes to it.  In HTSaveToFile_Free
**      the user will see a list of choices for download
*/
PUBLIC HTStream* HTSaveToFile ARGS3(
        HTPresentation *,       pres,
        HTParentAnchor *,       anchor, /* Not used */
	HTStream *,             sink)   /* Not used */
{
    HTStream * ret_obj;
    char fnam[64];
    int c=0;
    BOOL IsBinary = TRUE;
    CONST char * suffix;

    ret_obj = (HTStream*)calloc(sizeof(* ret_obj),1);
    if (ret_obj == NULL) outofmem(__FILE__, "HTSaveToFile");
    ret_obj->isa = &HTFWriter;
    ret_obj->remove_command = NULL;
    ret_obj->end_command = NULL;

    if(dump_output_immediately) {
        ret_obj->fp = stdout; /* stdout*/
        return ret_obj;
    }

    LYCancelDownload = FALSE;
    if(HTOutputFormat != HTAtom_for("www/download")) {
	if (no_download && no_disk_save) {
            _statusline("This file cannot be displayed on this terminal.");
	    sleep(sleep_three);
	    LYCancelDownload = TRUE;
            free(ret_obj);
            return(NULL);
	}

	_statusline(
      "This file cannot be displayed on this terminal:  D)ownload, or C)ancel");

	while(TOUPPER(c)!='C' && TOUPPER(c)!='D' && c!=7) {
	    c=LYgetch();
	}

        /** Cancel on 'C', 'c' or Control-G or Control-C **/
        if(TOUPPER(c)=='C' || c==7 || c==3) {
            _statusline("Cancelling file.");
	    LYCancelDownload = TRUE;
	    free(ret_obj);
            return(NULL);
        }
    }

/*	Set up a 'D'ownload
*/
    suffix = HTFileSuffix(pres->rep);

    tempname (fnam, 2);  /* lynx routine to create a filename */

    if (suffix) strcat(fnam, suffix);

    if(0==strncasecomp(pres->rep->name, "text/", 5) ||
       0==strcasecomp(pres->rep->name, "application/postscript") ||
       0==strcasecomp(pres->rep->name, "application/x-RUNOFF-MANUAL"))
        /*
	 *  It's a text file requested via 'd'ownload.
	 *  Keep adding others to the above list, 'til
	 *  we add a configurable procedure. - FM
	 */
	IsBinary = FALSE;

    ret_obj->fp = fopen (fnam, "wb");

    if (!ret_obj->fp) {
	HTAlert("Can't open output file! Cancelling");
	free(ret_obj);
	return NULL;
    }

    /*
     *  Any "application/foo" or other non-"text/foo" types that
     *  are actually text but not checked, above, will be treated
     *  as binary, so show the type to help sort that out later.
     *  Unix folks don't need to know this, but we'll show it to
     *  them, too. - FM
     */
    user_message("Content-type: %s", pres->rep->name);

    StrAllocCopy(WWW_Download_File,fnam);

/*	Make command to delete file
*/
    ret_obj->remove_command = (char *)calloc (
    			(strlen (REMOVE_COMMAND) + 10+ strlen(fnam))
    			 * sizeof (char),1);
    if (ret_obj == NULL) outofmem(__FILE__, "HTSaveToFile");
    
    sprintf (ret_obj->remove_command, REMOVE_COMMAND, fnam);

    ret_obj->end_command = (char *)malloc (sizeof(char)*12);
    if (ret_obj == NULL) outofmem(__FILE__, "HTSaveToFile");
    sprintf(ret_obj->end_command, "SaveToFile");

    _statusline("Retrieving file.  - PLEASE WAIT -");

    return ret_obj;
}

/*      Dump output to stdout LJM!!!
**      ---------------------
**
*/
PUBLIC HTStream* HTDumpToStdout ARGS3(
        HTPresentation *,       pres,
        HTParentAnchor *,       anchor, /* Not used */
        HTStream *,             sink)   /* Not used */
{
    HTStream * ret_obj;
    ret_obj = (HTStream*)calloc(sizeof(* ret_obj),1);
    if (ret_obj == NULL) outofmem(__FILE__, "HTDumpToStdout");
    ret_obj->isa = &HTFWriter;
    ret_obj->remove_command = NULL;
    ret_obj->end_command = NULL;

    ret_obj->fp = stdout; /* stdout*/
    return ret_obj;
}
