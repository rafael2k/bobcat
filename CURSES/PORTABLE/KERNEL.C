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

#ifdef UNIXx
#include <defs.h>
#include <term.h>
#endif

/* undefine any macros for functions defined in this module */
#undef	def_prog_mode
#undef	def_shell_mode
#undef	reset_prog_mode
#undef	reset_shell_mode
#undef	resetty
#undef	savetty
#undef	getsyx
#undef	setsyx
#undef	ripoffline
#undef	curs_set
#undef	napms

/* undefine any macros for functions called by this module if in debug mode */
#ifdef PDCDEBUG
#  undef	move
#  undef	wmove
#endif

#ifdef PDCDEBUG
char *rcsid_kernel  = "$Id$";
#endif

/*man-start*********************************************************************

  Name:                                                        kernel

  Synopsis:
  	int def_prog_mode(void);
  	int def_shell_mode(void);
  	int reset_prog_mode(void);
  	int reset_shell_mode(void);
  	int resetty(void);
  	int savetty(void);
  ***	int getsyx(int y, int x);
  ***	int setsyx(int y, int x);
  ***	int ripoffline(int line, init(*int)(WINDOW *,int));
  	int curs_set(int visibility);
  	int napms(int ms);

  X/Open Description:
 	The def_prog_mode() and def_shell_mode() functions save the 
 	current terminal modes as the "program" (in curses) or
 	"shell" (not in curses) state for use by the reset_prog_mode()
 	and reset_shell_mode() functions.  This is done automatically by
 	initscr().

 	The reset_prog_mode() and reset_shell_mode() functions restore 
 	the terminal to "program" (in curses) or "shell" (not in curses)
 	state.  These are done automatically by endwin()
 	and doupdate() after an endwin(), so they would normally not
 	be called before these functions.

 	The savetty() and resetty() routines save and restore the state of 
 	the terminal modes. The savetty() function saves the current state 
 	in a buffer and resetty() restores the state to what it was at the 
 	last call to savetty().

  PDCurses Description:
 	FYI: It is very unclear whether savetty() and resetty() functions
 	are a duplication of the reset_prog_mode() and reset_shell_mode() 
 	functions or whether this is a backing store type of operation.  
 	At this time, they are implemented similar to the reset_*_mode() 
 	routines.

 	The curs_set() routine is used to set the visibility of the cursor.
 	The cursor can be made invisible, normal or highly visible by setting
 	the parameter to 0, 1 or 2 respectively. If an invalid value is passed
 	the function will set the cursor to "normal".

  X/Open Return Value:
 	All functions return OK on success and ERR on error except curs_set()
 	which returns the previous visibility.

  X/Open Errors:
 	No errors are defined for this function.

  Portability                             X/Open    BSD    SYS V
                                          Dec '88
      def_prog_mode                         Y        Y       Y
      def_shell_mode                        Y        Y       Y
      reset_prog_mode                       Y        Y       Y
      reset_shell_mode                      Y        Y       Y
      resetty                               Y        Y       Y
      savetty                               Y        Y       Y
      getsyx                                -        -      3.0
      setsyx                                -        -      3.0
      ripoffline                            -        -      3.0
      curs_set                              -        -      3.0
      napms                                 Y        Y       Y

**man-end**********************************************************************/

#ifndef UNIXx
/***********************************************************************/
int	def_prog_mode(void)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("def_prog_mode() - called\n");
#endif

#ifdef	FLEXOS
	_flexos_16bitmode();
#endif
	c_pr_tty.been_set = TRUE;

	memcpy(&c_pr_tty.saved, &_cursvar, sizeof(SCREEN));

	return( OK );
}
#endif

#ifndef UNIXx
/***********************************************************************/
int	def_shell_mode(void)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("def_shell_mode() - called\n");
#endif

#ifdef	FLEXOS
	_flexos_8bitmode();
#endif
	c_sh_tty.been_set = TRUE;

	memcpy(&c_sh_tty.saved, &_cursvar, sizeof(SCREEN));

	return( OK );
}
#endif

#ifndef UNIXx
/***********************************************************************/
int	reset_prog_mode(void)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("reset_prog_mode() - called\n");
#endif

	if	(c_pr_tty.been_set == TRUE)
	{

		memcpy(&_cursvar, &c_pr_tty.saved, sizeof(SCREEN));

		mvcur(0, 0, c_pr_tty.saved.cursrow, c_pr_tty.saved.curscol);
		if (PDC_get_ctrl_break() != c_pr_tty.saved.orgcbr)
			PDC_set_ctrl_break(c_pr_tty.saved.orgcbr);
		if (c_pr_tty.saved.raw_out)
			raw();
		if (c_pr_tty.saved.visible_cursor)
			PDC_cursor_on();
		_cursvar.font = PDC_get_font();
		PDC_set_font(c_pr_tty.saved.font);
#if !defined (XCURSES)
#  ifndef EMXVIDEO
		if (!PDC_scrn_modes_equal (PDC_get_scrn_mode(),  c_pr_tty.saved.scrnmode))
			PDC_set_scrn_mode(c_pr_tty.saved.scrnmode);
#  endif
#endif
		PDC_set_rows(c_pr_tty.saved.lines);
	}
#ifdef	FLEXOS
	_flexos_16bitmode();
#endif
	return( OK );
}
#endif

#ifndef UNIXx
/***********************************************************************/
int	reset_shell_mode(void)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("reset_shell_mode() - called\n");
#endif

	if	(c_sh_tty.been_set == TRUE)
	{

		memcpy(&_cursvar, &c_sh_tty.saved, sizeof(SCREEN));

		mvcur(0, 0, c_sh_tty.saved.cursrow, c_sh_tty.saved.curscol);
		if (PDC_get_ctrl_break() != c_sh_tty.saved.orgcbr)
			PDC_set_ctrl_break(c_sh_tty.saved.orgcbr);
		if (c_sh_tty.saved.raw_out)
			raw();
		if (c_sh_tty.saved.visible_cursor)
			PDC_cursor_on();
		_cursvar.font = PDC_get_font();
		PDC_set_font(c_sh_tty.saved.font);
#if !defined (XCURSES)
#  ifndef EMXVIDEO
		if (!PDC_scrn_modes_equal (PDC_get_scrn_mode(),  c_sh_tty.saved.scrnmode))
			PDC_set_scrn_mode(c_sh_tty.saved.scrnmode);
#  endif
#endif
		PDC_set_rows(c_sh_tty.saved.lines);
	}
#ifdef	FLEXOS
	_flexos_8bitmode();
#endif
	return( OK );
}
#endif

/***********************************************************************/
int	resetty(void)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("resetty() - called\n");
#endif

#ifndef UNIXx
	if	(c_save_tty.been_set == TRUE)
	{
		memcpy(&_cursvar, &c_save_tty.saved, sizeof(SCREEN));

		mvcur(0, 0, c_save_tty.saved.cursrow, c_save_tty.saved.curscol);
		if (PDC_get_ctrl_break() != c_save_tty.saved.orgcbr)
			PDC_set_ctrl_break(c_save_tty.saved.orgcbr);
		if (c_save_tty.saved.raw_out)
			raw();
		if (c_save_tty.saved.visible_cursor)
			PDC_cursor_on();
		_cursvar.font = PDC_get_font();
		PDC_set_font(c_save_tty.saved.font);
#if !defined (XCURSES)
#  ifndef EMXVIDEO
		if (!PDC_scrn_modes_equal (PDC_get_scrn_mode(), c_save_tty.saved.scrnmode))
			PDC_set_scrn_mode(c_save_tty.saved.scrnmode);
#  endif
#endif
		PDC_set_rows(c_save_tty.saved.lines);
	}
#endif
	return( c_save_tty.been_set ? OK : ERR );
}
/***********************************************************************/
int	savetty(void)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("savetty() - called\n");
#endif

	c_save_tty.been_set = TRUE;
	memcpy(&c_save_tty.saved, &_cursvar, sizeof(SCREEN));
	return( OK );
}
/***********************************************************************/
int	curs_set(int visibility)
/***********************************************************************/
{
#ifdef OS2
# ifndef EMXVIDEO
 VIOCURSORINFO pvioCursorInfo;
# endif
#endif
 int start,end,hidden=0;
 int ret_vis;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("curs_set() - called: visibility=%d\n",visibility);
#endif

	ret_vis = _cursvar.visibility;
	_cursvar.visibility = visibility;

#ifdef UNIXx
	switch(visibility)
	{
		case 0:  /* invisible */
			if (cursor_invisible != NULL)
				putp(cursor_invisible);
			break;
		case 2:  /* highly visible */
			if (cursor_visible != NULL)
				putp(cursor_visible);
			break;
		default:  /* normal visibility */
			if (cursor_visible != NULL)
				putp(cursor_visible);
			break;
	}
	return(OK);
#endif

#if defined(DOS) || defined(OS2)
	switch(visibility)
	{
		case 0:  /* invisible */
# ifdef OS2
#  ifdef EMXVIDEO
			start = end = 0;
#  else
			start = _cursvar.font / 4;
			end = _cursvar.font;
#  endif
# else
			start = 32;
			end = 32;  /* was 33 */
# endif
			hidden = (-1);
			break;
		case 2:  /* highly visible */
			start = 2;   /* almost full-height block */
			end = _cursvar.font-1;
			break;
		default:  /* normal visibility */
			start = _cursvar.font - 4;
			end = _cursvar.font-1;
			break;
	}

# ifdef OS2
#  ifdef EMXVIDEO
	if (hidden)
		v_hidecursor();
	else
		v_ctype (start, end);
#  else
	pvioCursorInfo.yStart = (USHORT)start;
	pvioCursorInfo.cEnd = (USHORT)end;
	pvioCursorInfo.cx = (USHORT)1;
	pvioCursorInfo.attr = hidden;
	VioSetCurType((PVIOCURSORINFO)&pvioCursorInfo,0);
#  endif
# endif

# ifdef DOS
	regs.h.ah = 0x01;
	regs.h.al = (unsigned char)_cursvar.scrnmode;  /* if not set, some BIOSes hang */
	regs.h.ch = (unsigned char)start;
	regs.h.cl = (unsigned char)end;
	int86(0x10, &regs, &regs);
# endif
	return( ret_vis );
#endif

#if defined (XCURSES)
	XCurses_display_cursor(_cursvar.cursrow,_cursvar.curscol,
		curscr->_y[_cursvar.cursrow][_cursvar.curscol],
		_cursvar.cursrow,_cursvar.curscol,
		curscr->_y[_cursvar.cursrow][_cursvar.curscol]);
	return(ret_vis);
#endif

}
/***********************************************************************/
int	napms(int ms)
/***********************************************************************/
{
	return(delay_output(ms));
}
