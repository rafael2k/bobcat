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
#define	CURSES_LIBRARY	1
#include <curses.h>

/* undefine any macros for functions defined in this module */
#undef	touchwin
#undef	touchline
#undef	untouchwin
#undef	wtouchln
#undef	is_linetouched
#undef	is_wintouched

/* undefine any macros for functions called by this module if in debug mode */
#ifdef PDCDEBUG
#  undef	move
#  undef	wmove
#endif

#ifdef PDCDEBUG
char *rcsid_touch  = "$Id$";
#endif

/*man-start*********************************************************************

  Name:                                                         touch

  Synopsis:
  	int touchwin(WINDOW *win);
  	int touchline(WINDOW *win, int start,int count);
  	int untouchwin(WINDOW *win);
  	int wtouchln(WINDOW *win, int y, int n, int changed);
  	int is_linetouched(WINDOW *win,int line);
  	int is_wintouched(WINDOW *win);

  X/Open Description:
 	The touchwin() and touchline() functions throw away all optimisation 
 	information about which parts of the window have been touched, 
 	by pretending that the entire window has been drawn on.  
 	This is sometimes necessary when using overlapping
 	windows, since a change to one window will affect the other window,
 	but the records of which lines have been changed in the other
 	window will not reflect the change.

 	The untouchwin() routine marks all lines in the window as unchanged
 	since the last call to wrefresh().

 	The wtouchln() routine makes n lines in the window, starting at 
 	line y, look as if they have (changed=1) or have not (changed=0) 
 	been changed since the last call to wrefresh().

 	The is_linetouched() routine returns TRUE if the specified line 
 	in the specified window has been changed since the last call to 
 	wrefresh(). If the line has not changed, FALSE is returned.

 	The is_wintouched() routine returns TRUE if the specified window 
 	has been changed since the last call to wrefresh(). If the window 
 	has not changed, FALSE is returned.

  X/Open Return Value:
 	All functions return OK on success and ERR on error except
 	is_wintouched() and is_linetouched().

  X/Open Errors:
 	No errors are defined for this function.

  Portability                             X/Open    BSD    SYS V
                                          Dec '88
      touchwin                              Y        Y       Y
      touchline                             Y        -      3.0
      untouchwin                            -        -      4.0
      wtouchln                              Y        Y       Y
      is_linetouched                        -        -      4.0
      is_wintouched                         -        -      4.0

**man-end**********************************************************************/

/***********************************************************************/
int	touchwin(WINDOW *win)
/***********************************************************************/
{
	register int	i;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("touchwin() - called\n");
#endif

	if (win == (WINDOW *)NULL)
		return( ERR );

	for (i=0;i<win->_maxy;i++)
	{
		win->_firstch[i] = 0;
		win->_lastch[i] = win->_maxx-1;
	}
	return( OK );
}
/***********************************************************************/
int	touchline(WINDOW *win, int start,int count)
/***********************************************************************/
{
	register int i;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("touchline() - called: start %d count %d\n",start,count);
#endif

	if (win == (WINDOW *)NULL)
		return( ERR );

	if  (start > win->_maxy || start + count > win->_maxy)
		return( ERR );
	for(i=start;i<start+count;i++)
	   {
		win->_firstch[i] = 0;
		win->_lastch[i] = win->_maxx - 1;
	   }
	return( OK );
}
/***********************************************************************/
int	untouchwin(WINDOW *win)
/***********************************************************************/
{
	register int i;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("untouchwin() - called:");
#endif

	if (win == (WINDOW *)NULL)
		return( ERR );

	for (i=0;i<win->_maxy;i++)
	{
		win->_firstch[i] = _NO_CHANGE;
		win->_lastch[i] = _NO_CHANGE;
	}
	return(OK);
}
/***********************************************************************/
int	wtouchln(WINDOW *win, int y, int n, int changed)
/***********************************************************************/
{
	register int i;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("wtouchln() - called: y %d n %d changed %d\n",y,n,changed);
#endif

	if (win == (WINDOW *)NULL)
		return( ERR );

	if  (y > win->_maxy || y + n > win->_maxy)
		return( ERR );

	for (i=y;i<y+n;i++)
	{
		if ( changed ) 
		{
			win->_firstch[i] = 0;
			win->_lastch[i] = win->_maxx - 1;
		}
		else 
		{
			win->_firstch[i] = _NO_CHANGE;
			win->_lastch[i] = _NO_CHANGE;
		}
	}
	return( OK );
}
/***********************************************************************/
int	is_linetouched(WINDOW *win,int line)
/***********************************************************************/
{
	register int i;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("is_linetouched() - called\n");
#endif

	if (win == NULL)
		return(ERR);

	if (line > win->_maxy || line < 0)
		return(ERR);
	if (win->_firstch[line] != _NO_CHANGE) 
		return(TRUE);
	return(FALSE);
}
/***********************************************************************/
int	is_wintouched(WINDOW *win)
/***********************************************************************/
{
	register int i;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("is_wintouched() - called\n");
#endif

	if (win == NULL)
		return(ERR);

	for (i=0;i<win->_maxy;i++)
		if (win->_firstch[i] != _NO_CHANGE)
			return(TRUE);
	return(FALSE);
}
