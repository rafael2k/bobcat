/*
***************************************************************************
* This file comprises part of PDCurses. PDCurses is Public Domain software.
* You may use this code for whatever purposes you desire. This software
* is provided AS IS with NO WARRANTY whatsoever.
* Should this software be used in another application, an acknowledgement
* that PDCurses code is used would be appreciated, but is not mandatory.
*
* Any changes which you make to this software which may improve or enhance
* it, should be forwarded to the current maintainer for the benefit of 
* other users.
*
* The only restriction placed on this code is that no distribution of
* modified PDCurses code be made under the PDCurses name, by anyone
* other than the current maintainer.
* 
* See the file maintain.er for details of the current maintainer.
***************************************************************************
*/
#include <stdarg.h>
#include <string.h>
#define	CURSES_LIBRARY	1
#include <curses.h>
#undef	PDC_debug

#ifdef PDCDEBUG
char *rcsid_PDCdebug  = "$Id$";
#endif

	bool trace_on = FALSE;

/*man-start*********************************************************************

  PDC_debug()	- Write debugging info to log file.

  PDCurses Description:
 	This is a private PDCurses routine.

  PDCurses Return Value:
 	No return value.

  PDCurses Errors:
 	No errors are defined for this function.

  Portability:
 	PDCurses	void PDC_debug( char *,... );

**man-end**********************************************************************/

void	PDC_debug( char *fmt, ... )
{
	va_list args;
FILE *dbfp=NULL;
char buffer[256]="";

/*
 * open debug log file append
*/
	if (!trace_on)
		return; 
	dbfp = fopen("trace","a");
	if (dbfp == NULL)
	{
		fprintf( stderr, "PDC_debug(): Unable to open debug log file\n" );
		exit( 5 );
	}

	va_start(args, fmt);
	vsprintf(buffer,fmt,args);
	fputs(buffer,dbfp);
	va_end(args);
	fclose(dbfp);
	return;
}
