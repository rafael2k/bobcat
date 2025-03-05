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
// #include <mem.h>

#define	CURSES_LIBRARY	1
#include <curses.h>

/* undefine any macros for functions defined in this module */
#undef	delch
#undef	wdelch
#undef	mvdelch
#undef	mvwdelch

/* undefine any macros for functions called by this module if in debug mode */
#ifdef PDCDEBUG
#  undef	move
#  undef	wmove
#endif

#ifdef PDCDEBUG
char *rcsid_delch  = "$Id$";
#endif

/*man-start*********************************************************************

  Name:                                                         delch

  Synopsis:
  	int delch(void);
  	int wdelch(WINDOW *win);
  	int mvdelch(int y, int x);
  	int mvwdelch(WINDOW *win, int y, int x);

  X/Open Description:
 	The character under the cursor in the window is deleted.  All
 	characters to the right on the same line are moved to the left
 	one position and the last character on the line is filled with
 	a blank.  The cursor position does not change (after moving to
 	y, x if coordinates are specified).

 	NOTE: delch(), mvdelch(), and mvwdelch() are implemented as macros.

  X/Open Return Value:
 	All functions return OK on success and ERR on error.

  X/Open Errors:
 	No errors are defined for this function.

  NOTE:
 	The behaviour of Unix curses is to display a blank in the last
 	column of the window with the A_NORMAL attribute. PDCurses
 	displays the blank with the window's current attributes 
 	(including current colour). To get the behaviour of PDCurses,
 	#define PDCURSES_WCLR in curses.h or add -DPDCURSES_WCLR to the 
 	compile switches.

  Portability                             X/Open    BSD    SYS V
                                          Dec '88
      delch                                 Y        Y       Y
      wdelch                                Y        Y       Y
      mvdelch                               Y        Y       Y
      mvwdelch                              Y        Y       Y

**man-end**********************************************************************/

/***********************************************************************/
int	delch(void)
/***********************************************************************/
{
	int		y;
	int		x;
	int		maxx;
	chtype*		temp1;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("delch() - called\n");
#endif

	if (stdscr == (WINDOW *)NULL)
		return (ERR);

	y	= stdscr->_cury;
	x	= stdscr->_curx;
	maxx	= stdscr->_maxx - 1;
	temp1	= &stdscr->_y[y][x];

	memmove( temp1, temp1 + 1, (maxx - x) * sizeof(chtype) );

#if defined(PDCURSES_WCLR)
	stdscr->_y[y][maxx]	= stdscr->_blank | stdscr->_attrs;
#else
/* wrs (4/10/93) account for window background */
	stdscr->_y[y][maxx]	= stdscr->_bkgd;
#endif

	stdscr->_lastch[y] = maxx;

	if ((stdscr->_firstch[y] == _NO_CHANGE) ||
	    (stdscr->_firstch[y] > x))
	{
		stdscr->_firstch[y] = x;
	}
	return (OK);
}
/***********************************************************************/
int	wdelch(WINDOW *win)
/***********************************************************************/
{
	int		y;
	int		x;
	int		maxx;
	chtype*		temp1;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("wdelch() - called\n");
#endif

	if (win == (WINDOW *)NULL)
		return (ERR);

	y	= win->_cury;
	x	= win->_curx;
	maxx	= win->_maxx - 1;
	temp1	= &win->_y[y][x];

	memmove( temp1, temp1 + 1, (maxx - x) * sizeof(chtype) );

#if defined(PDCURSES_WCLR)
	win->_y[y][maxx]	= win->_blank | win->_attrs;
#else
/* wrs (4/10/93) account for window background */
	win->_y[y][maxx]	= win->_bkgd;
#endif

	win->_lastch[y] = maxx;

	if ((win->_firstch[y] == _NO_CHANGE) ||
	    (win->_firstch[y] > x))
	{
		win->_firstch[y] = x;
	}
	return (OK);
}
/***********************************************************************/
int	mvdelch(int y, int x)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("mvdelch() - called\n");
#endif

	if (move(y,x) == ERR)
		return(ERR);
	return(delch());
}
/***********************************************************************/
int	mvwdelch(WINDOW *win, int y, int x)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("mvwdelch() - called\n");
#endif

	if (wmove(win,y,x) == ERR)
		return(ERR);
	return(wdelch(win));
}
