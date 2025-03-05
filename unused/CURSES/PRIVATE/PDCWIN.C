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

#ifndef NO_MEMORY_H
#include <memory.h>
#endif

#ifdef PDCDEBUG
char *rcsid_PDCwin  = "$Id$";
#endif

/*man-start*********************************************************************

  PDC_copy_win()	- Common routine for copywin(), overlay() and overwrite()
 	functions.

  PDCurses Description:
 	This function copies the region of the source window specified
 	over the specified region of the destination window. All validation
 	of limits are done by the calling function.

 	Thanks to Andreas Otte (venn@uni-paderborn.de) for the code changes.

  PDCurses Errors:
 	ERR is returned if either src or dst windows are NULL;

  Portability:
 	PDCurses	int	PDC_copy_win( WINDOW* src_w, WINDOW* dst_w
 			int src_tr,int src_tc,int src_br,int src_bc,
 			int dst_tr,int dst_tc,int dst_br,int dst_bc,bool overlay);

**man-end**********************************************************************/

int	PDC_copy_win(WINDOW *src_w, WINDOW *dst_w,
 			int src_tr,int src_tc,int src_br,int src_bc,
 			int dst_tr,int dst_tc,int dst_br,int dst_bc,bool overlay)
{
	int*	minchng=0;
	int*	maxchng=0;
	chtype*	w1ptr=NULL;
	chtype*	w2ptr=NULL;
	int	col=0;
	int	line=0;
	int	xdiff = src_bc - src_tc;
	int	ydiff = src_br - src_tr;
	int	y1=0;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("PDC_copy_win() - called\n");
#endif

	if (src_w == (WINDOW *)NULL)	return( ERR );
	if (dst_w == (WINDOW *)NULL)	return( ERR );

	minchng    = dst_w->_firstch;
	maxchng    = dst_w->_lastch;


	for (y1 = 0; y1 < dst_tr; y1++)
		{
		minchng++;
		maxchng++;
		}

	for (line = 0; line < ydiff; line++)
	{
		register int fc;
		register int lc;

		w1ptr = src_w->_y[line+src_tr]+src_tc;
		w2ptr = dst_w->_y[line+dst_tr]+dst_tc;
		fc    = _NO_CHANGE;

		for (col = 0; col < xdiff; col++)
		{
			if ((*w1ptr) != (*w2ptr)
			&&  !((*w1ptr & A_CHARTEXT) == src_w->_blank && overlay))
			{
				*w2ptr = *w1ptr;
				if (fc == _NO_CHANGE)
				{
					fc = col+dst_tc;
				}
				lc = col+dst_tc;
			}
			w1ptr++;
			w2ptr++;
		}

		if (*minchng == _NO_CHANGE)
		{
			*minchng = fc;
			*maxchng = lc;
		}
		else	if (fc != _NO_CHANGE)
		{
			if (fc < *minchng)	*minchng = fc;
			if (lc > *maxchng)	*maxchng = lc;
		}
		minchng++;
		maxchng++;
	}
	return( OK );
}

/*man-start*********************************************************************

  PDC_makenew()	- Create a WINDOW* (sans line allocation)

  PDCurses Description:
 	This is a private PDCurses routine.

 	Allocates all data for a new WINDOW* except the actual lines
 	themselves.

  PDCurses Return Value:
 	This function returns a valid WINDOW* on success and NULL on error.

  PDCurses Errors:
 	If PDC_makenew() is unable to allocate memory for the window
 	structure, it will free all allocated memory and return
 	a NULL pointer.

  Portability:
 	PDCurses	WINDOW* PDC_makenew( int num_lines, int num_columns,
 					 int begy, int begx );

**man-end**********************************************************************/

WINDOW*	PDC_makenew(int num_lines, int num_columns, int begy, int begx)
{
extern	void*	(*mallc)( size_t );
extern	void*	(*callc)( size_t, size_t );
extern	void	(*fre)( void* );

	short	i=0;
	WINDOW *win=NULL;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("PDC_makenew() - called: lines %d cols %d begy %d begx %d\n",num_lines,num_columns,begy,begx);
#endif

	/*
	*	Use the standard runtime malloc/calloc package or use
	*	the user's emalloc/ecalloc package.
	*
	*	Allocate the window structure itself
	*/
	if ((win = (*mallc)(sizeof(WINDOW))) == (WINDOW *)NULL)
	{
		return( win );
	}

	/*
	* allocate the line pointer array
	*/
	if ((win->_y = (*callc)(num_lines, sizeof(chtype *))) == NULL)
	{
		(*fre)(win);
		return( (WINDOW *)NULL );
	}

	/*
	* allocate the minchng and maxchng arrays
	*/
	if ((win->_firstch = (*callc)(num_lines, sizeof(int))) == NULL)
	{
		(*fre)(win->_y);
		(*fre)(win);
		return( (WINDOW *)NULL );
	}
	if ((win->_lastch = (*callc)(num_lines, sizeof(int))) == NULL)
	{
		(*fre)(win->_firstch);
		(*fre)(win->_y);
		(*fre)(win);
		return( (WINDOW *)NULL );
	}

	/*
	* initialize window variables
	*/
	win->_curx = 0;
	win->_cury = 0;
	win->_maxy = num_lines;		/* real max screen size */
	win->_maxx = num_columns;	/* real max screen size */
	win->_pmaxy = num_lines;	/* real max window size */
	win->_pmaxx = num_columns;	/* real max window size */
	win->_begy = begy;
	win->_begx = begx;
	win->_lastpy = 0;
	win->_lastpx = 0;
	win->_lastsy1 = 0;
	win->_lastsx1 = 0;
	win->_lastsy2 = LINES-1;
	win->_lastsx2 = COLS-1;
	win->_flags = 0;
	win->_attrs = 0;		/* No attributes */
	win->_tabsize = 8;
	win->_clear = (bool) ((num_lines == LINES) && (num_columns == COLS));
	win->_leave = FALSE;
	win->_scroll = FALSE;
	win->_nodelay = FALSE;
	win->_use_keypad = FALSE;
	win->_immed = FALSE;
	win->_use_idl = FALSE;
	win->_use_idc = TRUE;
	win->_tmarg = 0;
	win->_bmarg = num_lines - 1;
	win->_title = NULL;
	win->_title_ofs = 1;
	win->_title_attr = win->_attrs;
	win->_blank = ' ';
	win->_parx = win->_pary = -1;
	win->_parent = NULL;
/* wrs 4/10/93 -- initialize background to blank */
	win->_bkgd = ' ';
	/*
	* init to say window unchanged
	*/
	for (i = 0; i < num_lines; i++)
	{
		win->_firstch[i] = 0;
		win->_lastch[i] = num_columns - 1;
	}

	/*
	* set flags for window properties
	*/
	if ((begy + num_lines) == LINES)
	{
		win->_flags |= _ENDLINE;
		if ((begx == 0) &&
		    (num_columns == COLS) &&
		    (begy == 0))
		{
			win->_flags |= _FULLWIN;
		}
	}

	if (((begy + num_lines) == LINES) &&
	    ((begx + num_columns) == COLS))
	{
		win->_flags |= _SCROLLWIN;
	}
	return( win );
}
