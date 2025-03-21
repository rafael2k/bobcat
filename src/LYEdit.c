#include "HTUtils.h"
#include "HTParse.h"
#include "LYCurses.h"
#include "LYSignal.h"
#include "LYUtils.h"
#include "LYClean.h"
#include "LYGlobalDefs.h"
#include "LYEdit.h"
#include "LYString.h"
#include "LYSystem.h"
#ifdef VMS
#include <unixio.h>
#include "HTVMSUtils.h"
#endif /* VMS */

#include <unistd.h>

#include "LYLeaks.h"

/*
 *  in edit mode invoke either emacs, vi, pico  or the default editor
 *  to display and edit the current file
 *  Both vi, emacs, and pico will open the file to the same line that the
 *  screen cursor is on when editing is invoked
 *  returns FALSE if file uneditable
 */

PUBLIC int edit_current_file ARGS3(char *,newfile, int,cur, int,lineno)
{

	char command[512];
        char *filename = NULL;
	char *colon, *number_sign;
	FILE *fp;
	int url_type = is_url(newfile);

	/*
	 * If its a remote file then we can't edit it.
	 */
	if(!LYisLocalFile(newfile)) {
	    _statusline("Lynx cannot currently (E)dit remote WWW files");
	    sleep(sleep_two);
	    return FALSE;
	}

	number_sign = strchr(newfile,'#');
	if(number_sign)
	    *number_sign = '\0';
	   
	 /*
	  * On Unix, first try to open it as a completely referenced file,
	  * then via the path alone.
	  *
	  * On VMS, only try the path.
	  */

	    filename = HTParse(newfile,"",PARSE_PATH+PARSE_PUNCTUATION);
	    HTUnEscape(filename);

	{
	    int i;
	    for(i=0;i<strlen(filename);i++)
		if(filename[i] == '/') filename[i] = '\\';
	}

	    if ((fp = fopen(filename+1,"r")) == NULL) {

		_statusline("Could not access file.");
		sleep(sleep_two);
		free(filename);
		goto failure;
	    }

	fclose(fp);


#ifdef VMS
	/*
	 * Don't allow editing if user lacks append access.
	 */
#ifdef VMS
	if ((fp = fopen(HTVMS_name("",filename),"a")) == NULL) {
#else
	if ((fp = fopen(filename,"a")) == NULL) {
#endif /* VMS */
		_statusline("You are not authorized to edit this file.");
		sleep(sleep_two);
		goto failure;
	}
	fclose(fp);
#endif /* VMS || CANT_EDIT_UNWRITABLE_FILES */

	if(strstr(editor,"emacs") || strstr(editor,"vi") ||
	   strstr(editor, "pico") || strstr(editor,"jove")) 
	    sprintf(command,"%s +%d \"%s\"",editor, lineno+links[cur].ly, 
								filename+1);
	else
            sprintf(command,"%s %s",editor, filename+1);

	free(filename);

	stop_curses();

	system(command);

	start_curses();

	if(number_sign)
	    *number_sign = '#';
	return TRUE;

failure:
	if(number_sign)
	    *number_sign = '#';
	return FALSE;
}
