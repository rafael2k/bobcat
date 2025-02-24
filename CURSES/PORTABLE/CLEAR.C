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
#undef	clear
#undef	wclear
#undef	erase
#undef	werase
#undef	clrtobot
#undef	wclrtobot
#undef	clrtoeol
#undef	wclrtoeol

/* undefine any macros for functions called by this module if in debug mode */
#ifdef PDCDEBUG
#  undef	clearok
#endif

#ifdef PDCDEBUG
char *rcsid_clear  = "$Id$";
#endif

/*man-start*********************************************************************

  Name:                                                         clear

  Synopsis:
  	int clear(void);
  	int wclear(WINDOW *win);
  	int erase(void);
  	int werase(WINDOW *win);
  	int clrtobot(void);
  	int wclrtobot(WINDOW *win);
  	int clrtoeol(void);
  	int wclrtoeol(WINDOW *win);

  X/Open Description:
  	The erase() and werase() functions copy blanks to every position
  	of the window.

  	The clear() and wclear() functions are similar to erase() and
  	werase() except they also call clearok() to ensure that the
  	the screen is cleared on the next call to wrefresh() for that
  	window.

  	The clrtobot() and wclrtobot() functions clear the screen from
  	the current cursor position to the end of the current line and
  	all remaining lines in the window.

  	The clrtoeol() and wclrtoeol() functions clear the screen from
  	the current cursor position to the end of the current line only.

 	NOTE: clear(), wclear(), erase(), clrtobot(), and clrtoeol()
 	are implemented as macros

  PDCurses Description:

  X/Open Return Value:
 	All functions return OK on success and ERR on error.

  X/Open Errors:
 	No errors are defined for this function.

  NOTE:
 	The behaviour of Unix curses is to clear the line with a space
 	and attributes of A_NORMAL. PDCurses clears the line with the
 	window's current attributes (including current colour). To get
 	the behaviour of PDCurses, #define ODCURSES_WCLR in curses.h or
 	add -DPDCURSES_WCLR to the compile switches.

  Portability                             X/Open    BSD    SYS V
                                          Dec '88
      clear                                 Y        Y       Y
      wclear                                Y        Y       Y
      erase                                 Y        Y       Y
      werase                                Y        Y       Y
      clrtobot                              Y        Y       Y
      wclrtobot                             Y        Y       Y
      clrtoeol                              Y        Y       Y
      wclrtoeol                             Y        Y       Y

**man-end**********************************************************************/

/***********************************************************************/
int	clear(void)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("clear() - called\n");
#endif

	if  (stdscr == (WINDOW *)NULL)
		return(ERR);

	clearok(stdscr, TRUE);
	return(erase());
}
/***********************************************************************/
int	wclear( WINDOW *win )
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("wclear() - called\n");
#endif

	if  (win == (WINDOW *)NULL)
		return( ERR );

	clearok( win, TRUE );
	return( werase( win ) );
}
/***********************************************************************/
int	erase(void)
/***********************************************************************/
{
	chtype*	end;
	chtype*	start;
	int	y,x;
	chtype	blank;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("erase() - called\n");
#endif

	if (stdscr == (WINDOW *)NULL)
		return( ERR );

#if defined(PDCURSES_WCLR)
	blank	= stdscr->_blank | stdscr->_attrs;
#else
/* wrs (4/10/93) account for window background */
	blank	= stdscr->_bkgd;
#endif

	for (y = stdscr->_tmarg; y <= stdscr->_bmarg; y++)
	{
		start = stdscr->_y[y];
		end = &start[stdscr->_maxx - 1];
/* changed JGB 6/92 < to <= */
		while (start <= end)
		{
			*start++ = blank;
		}
		stdscr->_firstch[y] = 0;
		stdscr->_lastch[y] = stdscr->_maxx - 1;
	}
	stdscr->_cury = stdscr->_tmarg;
	stdscr->_curx = 0;

	return( OK );
}
/***********************************************************************/
int	werase(WINDOW *win)
/***********************************************************************/
{
	chtype*	end;
	chtype*	start;
	int	y,x;
	chtype	blank;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("werase() - called\n");
#endif

	if (win == (WINDOW *)NULL)
		return( ERR );

#if defined(PDCURSES_WCLR)
	blank	= win->_blank | win->_attrs;
#else
/* wrs (4/10/93) account for window background */
	blank	= win->_bkgd;
#endif

	for (y = win->_tmarg; y <= win->_bmarg; y++)
	{
		start = win->_y[y];
		end = &start[win->_maxx - 1];
/* changed JGB 6/92 < to <= */
		while (start <= end)
		{
			*start++ = blank;
		}
		win->_firstch[y] = 0;
		win->_lastch[y] = win->_maxx - 1;
	}
	win->_cury = win->_tmarg;
	win->_curx = 0;

	return( OK );
}
/***********************************************************************/
int	clrtobot(void)
/***********************************************************************/
{
	int	y;
	int	minx;
	int	startx;
	chtype	blank;
	chtype*	ptr;
	chtype*	end;
	chtype*	maxx;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("clrtobot() - called\n");
#endif

	if  (stdscr == (WINDOW *)NULL)
		return( ERR );

#if defined(PDCURSES_WCLR)
	blank	= stdscr->_blank | stdscr->_attrs;
#else
/* wrs (4/10/93) account for window background */
	blank	= stdscr->_bkgd;
#endif

	startx	= stdscr->_curx;

	for (y = stdscr->_cury; y < stdscr->_maxy; y++)
	{
		minx	= _NO_CHANGE;
		end	= &stdscr->_y[y][stdscr->_maxx - 1];
		for (ptr = &stdscr->_y[y][startx]; ptr <= end; ptr++)
		{
			if (*ptr != blank)
			{
				maxx = ptr;
				if (minx == _NO_CHANGE)
				{
					minx = (int) (ptr - stdscr->_y[y]);
				}
				*ptr = blank;
			}
		}
		if (minx != _NO_CHANGE)
		{
			if ((stdscr->_firstch[y] > minx) ||
			    (stdscr->_firstch[y] == _NO_CHANGE))
			{
				stdscr->_firstch[y] = minx;
				if (stdscr->_lastch[y] < maxx - stdscr->_y[y])
				{
					stdscr->_lastch[y] = (int) (maxx - stdscr->_y[y]);
				}
			}
		}
		startx = 0;
	}
	return( OK );
}
/***********************************************************************/
int	wclrtobot(WINDOW *win)
/***********************************************************************/
{
	int	y;
	int	minx;
	int	startx;
	chtype	blank;
	chtype*	ptr;
	chtype*	end;
	chtype*	maxx;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("wclrtobot() - called\n");
#endif

	if  (win == (WINDOW *)NULL)
		return( ERR );

#if defined(PDCURSES_WCLR)
	blank	= win->_blank | win->_attrs;
#else
/* wrs (4/10/93) account for window background */
	blank	= win->_bkgd;
#endif

	startx	= win->_curx;

	for (y = win->_cury; y < win->_maxy; y++)
	{
		minx	= _NO_CHANGE;
		end	= &win->_y[y][win->_maxx - 1];
		for (ptr = &win->_y[y][startx]; ptr <= end; ptr++)
		{
			if (*ptr != blank)
			{
				maxx = ptr;
				if (minx == _NO_CHANGE)
				{
					minx = (int) (ptr - win->_y[y]);
				}
				*ptr = blank;
			}
		}
		if (minx != _NO_CHANGE)
		{
			if ((win->_firstch[y] > minx) ||
			    (win->_firstch[y] == _NO_CHANGE))
			{
				win->_firstch[y] = minx;
				if (win->_lastch[y] < maxx - win->_y[y])
				{
					win->_lastch[y] = (int) (maxx - win->_y[y]);
				}
			}
		}
		startx = 0;
	}
	return( OK );
}
/***********************************************************************/
int	clrtoeol(void)
/***********************************************************************/
{
	int	y;
	int	x;
	int	minx;
	chtype	blank;
	chtype*	maxx;
	chtype*	ptr;
	chtype*	end;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("clrtoeol() - called\n");
#endif

	if (stdscr == (WINDOW *)NULL)
		return( ERR );

	y	= stdscr->_cury;
	x	= stdscr->_curx;

#if defined(PDCURSES_WCLR)
	blank	= stdscr->_blank | stdscr->_attrs;
#else
/* wrs (4/10/93) account for window background */
	blank	= stdscr->_bkgd;
#endif

	end	= &stdscr->_y[y][stdscr->_maxx - 1];
	minx	= _NO_CHANGE;
	maxx	= &stdscr->_y[y][x];

	for (ptr = maxx; ptr <= end; ptr++)
	{
		if (*ptr != blank)
		{
			maxx = ptr;
			if (minx == _NO_CHANGE)
			{
				minx = (int) (ptr - stdscr->_y[y]);
			}
			*ptr = blank;
		}
	}

	if (minx != _NO_CHANGE)
	{
		if ((stdscr->_firstch[y] > minx) ||
		    (stdscr->_firstch[y] == _NO_CHANGE))
		{
			stdscr->_firstch[y] = minx;
		}
		if (stdscr->_lastch[y] < maxx - stdscr->_y[y])
		{
			stdscr->_lastch[y] = (int) (maxx - stdscr->_y[y]);
		}
	}
	return( OK );
}
/***********************************************************************/
int	wclrtoeol(WINDOW *win)
/***********************************************************************/
{
	int	y;
	int	x;
	int	minx;
	chtype	blank;
	chtype*	maxx;
	chtype*	ptr;
	chtype*	end;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("wclrtoeol() - called\n");
#endif

	if (win == (WINDOW *)NULL)
		return( ERR );

	y	= win->_cury;
	x	= win->_curx;

#if defined(PDCURSES_WCLR)
	blank	= win->_blank | win->_attrs;
#else
/* wrs (4/10/93) account for window background */
	blank	= win->_bkgd;
#endif

	end	= &win->_y[y][win->_maxx - 1];
	minx	= _NO_CHANGE;
	maxx	= &win->_y[y][x];

	for (ptr = maxx; ptr <= end; ptr++)
	{
		if (*ptr != blank)
		{
			maxx = ptr;
			if (minx == _NO_CHANGE)
			{
				minx = (int) (ptr - win->_y[y]);
			}
			*ptr = blank;
		}
	}

	if (minx != _NO_CHANGE)
	{
		if ((win->_firstch[y] > minx) ||
		    (win->_firstch[y] == _NO_CHANGE))
		{
			win->_firstch[y] = minx;
		}
		if (win->_lastch[y] < maxx - win->_y[y])
		{
			win->_lastch[y] = (int) (maxx - win->_y[y]);
		}
	}
	return( OK );
}
