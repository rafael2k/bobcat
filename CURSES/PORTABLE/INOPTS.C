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

#ifdef UNIXx
#define NOTLIB
#include <defs.h>
#include <term.h>
#endif

/* undefine any macros for functions defined in this module */
#undef	cbreak
#undef	nocbreak
#undef	echo
#undef	noecho
#undef	halfdelay
#undef	intrflush
#undef	keypad
#undef	meta
#undef	nodelay
#undef	notimeout
#undef	raw
#undef	noraw
#undef	noqiflush
#undef	qiflush
#undef	timeout
#undef	wtimeout
#undef	typeahead

/* undefine any macros for functions called by this module if in debug mode */
#ifdef PDCDEBUG
#  undef	move
#  undef	wmove
#endif

#ifdef PDCDEBUG
char *rcsid_inopts  = "$Id$";
#endif

/*man-start*********************************************************************

  Name:                                                        inopts

  Synopsis:
  	int cbreak(void);
  	int nocbreak(void);
  	int echo(void);
  	int noecho(void);
  ***	int halfdelay(int tenths);
  	int intrflush(WINDOW *win, bool bf);
  	int keypad(WINDOW *win, bool bf);
  	int meta(WINDOW *win, bool bf);
  	int nodelay(WINDOW *win, bool bf);
  	int notimeout(WINDOW *win, bool bf);
  	int raw(void);
  	int noraw(void);
  ***	void noqiflush(void);
  ***	void qiflush(void);
  ***	int timeout(int delay);
  ***	int wtimeout(WINDOW *win, int delay);
  	int typeahead(int fildes);

  X/Open Description:
 	cbreak() and nocbreak() puts the terminal into and out of cbreak
 	mode. In cbreak mode, characters typed by the user are immediately
 	available to the program and erase/kill character processing is
 	not performed.  When out of cbreak mode, the terminal driver
 	will buffer characters typed until a newline or carriage return
 	is typed.  Interrupt and flow control characters are unaffected
 	by this mode.  Initially the terminal may or may not need be
 	in cbreak mode.

 	echo() and noecho() control whether characters typed by the user
 	are echoed by the input routine.  Initially, input characters
 	are echoed.  Subsequent calls to echo() and noecho() do not
 	flush type-ahead.

 	If the intrflush() option is enabled (bf is TRUE), and an interrupt
 	is pressed on the keyboard (INTR, BREAK, or QUIT) all output in
 	the terminal driver queue will be flushed, giving the effect
 	of faster response to the interrupt but causing curses to have
 	the wrong idea of what is on the screen.  Disabling the option
 	prevents the flush.  The default for the option is inherited
 	from the terminal driver settings.  The window argument is
 	ignored.

 	The keypad() function changes the keypad option of the user's terminal.
 	If enabled (bf is TRUE), the user can press a function key (such
 	as the left arrow key) and getch() will return a single value
 	that represents the KEY_LEFT function key.  (See Section 11.3.3,
 	Input Values.)  If disabled, curses will not treat function keys
 	as special keys and the program has to interpret the escape
 	sequences itself.  If the keypad is enabled, the terminal keypad
 	is turned on before input begins.

 	The meta() function forces the user's terminal to return 7 or 8
 	significant bits on input.  To force 8 bits to be returned,
 	invoke meta() with bf as TRUE.  To force 7 bits to be returned,
 	invoke meta() with bf as FALSE.
 	The window argument is always ignored, but it must still be a
 	valid window to avoid compiler errors.

 	The nodelay() function controls whether wgetch() is a non-blocking
 	call. If the option is enabled, and no input is ready, wgetch()
 	will return ERR. If disabled, wgetch() will hang until input
 	is ready.

 	While interpreting an input escape sequence, wgetch sets a timer while
 	waiting for the next character.  If notimeout(win,TRUE) is called, then
 	wgetch does not set a timer.  The purpose of the timeout is to
 	differentiate between sequences received from a function key and those
 	typed by a user.

 	With raw() and noraw(), the terminal in placed into or out of raw 
 	mode.  Raw mode is similar to cbreak mode, in that characters typed 
 	are immediately passed through to the user program.  The differences
 	are that in raw mode, the INTR, QUIT, SUSP, and STOP characters are 
 	passed through without being interpreted, and without generating a
 	signal.  The behaviour of the BREAK key depends on other
 	parameters of the terminal drive that are not set by curses.

 	The curses package does the "line-breakout optimisation" by
 	looking for type-ahead periodically while updating the screen.
 	If input is found, the current update will be postponed until
 	refresh() or doupdate() are called again.  This allows faster
 	response to commands typed in advance.  Normally, the input FILE
 	pointer passed to newterm(), or stdin in the case when initscr()
 	was called, will be used to do this type-ahead checking.  The
 	typeahead() routine specified that the file descriptor fd is to
 	be used to check for type-ahead instead.  If fd is -1, then no
 	type-ahead checking will be done.

  PDCurses Description:
 	The meta() function is provided for portability.  By default, 8 bits
 	are returned.

 	notimeout() is a no-op in PDCurses.

  X/Open Return Value:
 	All functions return OK on success and ERR on error.

  X/Open Errors:
 	No errors are defined for this function.

  Portability                             X/Open    BSD    SYS V
                                          Dec '88
      cbreak                                Y        Y       Y
      nocbreak                              Y        Y       Y
      echo                                  Y        Y       Y
      noecho                                Y        Y       Y
      halfdelay                             Y        Y       Y
      intrflush                             Y        Y       Y
      keypad                                Y        Y       Y
      meta                                  Y        Y       Y
      nodelay                               Y        Y       Y
      notimeout                             Y        Y       Y
      raw                                   Y        Y       Y
      noraw                                 Y        Y       Y
      noqiflush                             Y        Y       Y
      qiflush                               Y        Y       Y
      timeout                               Y        Y       Y
      wtimeout                              Y        Y       Y
      typeahead                             Y        Y       Y

**man-end**********************************************************************/

/***********************************************************************/
int	cbreak(void)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("cbreak() - called\n");
#endif

#ifdef UNIXx
#ifdef USE_TERMIO
	_CUR_TERM.prog_mode.c_lflag &= ~(ICANON);
	_CUR_TERM.prog_mode.c_iflag &= ~(ICRNL);
/*	_CUR_TERM.prog_mode.c_lflag |= ISIG;*/
	_CUR_TERM.prog_mode.c_cc[VMIN] = 1;
	_CUR_TERM.prog_mode.c_cc[VTIME] = 0;
	ioctl(_CUR_TERM.fd, TCSETAW, &_CUR_TERM.prog_mode);
#else
	_CUR_TERM.prog_mode.sg_flags |= CBREAK;
	ioctl(_CUR_TERM.fd, TIOCSETP, &_CUR_TERM.prog_mode);
#endif

#endif

	_cursvar.cbreak = TRUE;
	return( OK );
}
/***********************************************************************/
int	nocbreak(void)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("nocbreak() - called\n");
#endif

#ifdef UNIXx
#ifdef USE_TERMIO
	_CUR_TERM.prog_mode.c_lflag |= ICANON;
	ioctl(_CUR_TERM.fd, TCSETAW, &_CUR_TERM.prog_mode);
#else 
	_CUR_TERM->prog_mode.sg_flags &= ~CBREAK;
	ioctl(_CUR_TERM.fd, TIOCSETP,&_CUR_TERM.prog_mode);
#endif
#endif

	_cursvar.cbreak = FALSE;
	return( OK );
}
/***********************************************************************/
int	echo(void)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("echo() - called\n");
#endif

#ifdef UNIXx
#ifdef USE_TERMIO
	_CUR_TERM.prog_mode.c_lflag |= ECHOCTL|ECHOKE;
	ioctl(_CUR_TERM.fd, TCSETAW, &_CUR_TERM.prog_mode);
#else
	_CUR_TERM.prog_mode.sg_flags |= ECHO;
	ioctl(_CUR_TERM.fd, TIOCSETP, &_CUR_TERM.prog_mode);
#endif
#endif

	_cursvar.echo = TRUE;
	return( OK );
}
/***********************************************************************/
int	noecho(void)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("noecho() - called\n");
#endif

#ifdef UNIXx
#ifdef USE_TERMIO
	_CUR_TERM.prog_mode.c_lflag &= ~(ECHO|ECHOPRT);
	ioctl(_CUR_TERM.fd, TCSETAW, &_CUR_TERM.prog_mode);
#else
	_CUR_TERM.prog_mode.sg_flags &= ~ECHO;
	ioctl(_CUR_TERM.fd, TIOCSETP, &_CUR_TERM.prog_mode);
#endif
#endif

	_cursvar.echo = FALSE;
	return( OK );
}
/***********************************************************************/
int	intrflush( WINDOW *win, bool bf )
/***********************************************************************/
{
#ifdef	TC
#  pragma argsused
#endif
	int	y;
	int	maxy;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("intrflush() - called\n");
#endif

	if (win == (WINDOW *)NULL)
		return( ERR );

	maxy = win->_maxy - 1;

	for (y = 0; y <= maxy; y++)
	{
		win->_firstch[y] = _NO_CHANGE;
	}
	return( OK );
}
/***********************************************************************/
int	keypad( WINDOW *win, bool bf )
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("keypad() - called\n");
#endif

	win->_use_keypad = bf;
	return( OK );
}
/***********************************************************************/
int	meta( WINDOW *win, bool bf )
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("meta() - called\n");
#endif

#ifdef UNIXx
/* INCOMPLETE */
#endif

#ifdef	TC
# pragma argsused;
#endif
	_cursvar.raw_inp = bf;
	return( OK );
}
/***********************************************************************/
int	nodelay( WINDOW *win, bool flag )
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("nodelay() - called\n");
#endif

	win->_nodelay = flag;
	return( OK );
}
/***********************************************************************/
int	notimeout( WINDOW *win, bool flag )
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("notimeout() - called\n");
#endif

	return( OK );
}
/***********************************************************************/
int	raw(void)
/***********************************************************************/
{
#ifdef OS2
# ifndef EMXVIDEO
	KBDINFO KbdInfo;
# endif
#endif

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("raw() - called\n");
#endif

#ifdef OS2
# ifndef EMXVIDEO
	KbdGetStatus(&KbdInfo,0);
	KbdInfo.fsMask |= KEYBOARD_BINARY_MODE;
	KbdInfo.fsMask &= ~KEYBOARD_ASCII_MODE;
	KbdSetStatus(&KbdInfo,0);
# endif
#endif

#if defined( UNIXx )	/* || defined( EMXVIDEO )	NOT COMPLETED */
#ifdef USE_TERMIO
#if 0
	_CUR_TERM.prog_mode.c_lflag &= ~(ICANON|ISIG);
	_CUR_TERM.prog_mode.c_iflag &= ~(INPCK|ISTRIP|IXON);
	_CUR_TERM.prog_mode.c_oflag &= ~(OPOST);
	_CUR_TERM.prog_mode.c_cc[VMIN] = 1;
	_CUR_TERM.prog_mode.c_cc[VTIME] = 0;
	ioctl(_CUR_TERM.fd, TCSETAW, &_CUR_TERM.prog_mode);
#endif
	_CUR_TERM.prog_mode.c_lflag &= ~(ICANON|ISIG);
	_CUR_TERM.prog_mode.c_iflag &= ~(IXON);
	_CUR_TERM.prog_mode.c_iflag |= ICRNL;
	ioctl(_CUR_TERM.fd, TCSETAW, &_CUR_TERM.prog_mode);
#else
	_CUR_TERM.prog_mode.sg_flags |= RAW;
	ioctl(_CUR_TERM.fd, TIOCSETP, &_CUR_TERM.prog_mode);
#endif
#endif

	_cursvar.raw_inp = TRUE;
	PDC_set_ctrl_break(FALSE);      /* disallow ^BREAK on disk I/O */
/*	flushinp(); */
	return( OK );
}
/***********************************************************************/
int	noraw(void)
/***********************************************************************/
{
#ifdef OS2
# ifndef EMXVIDEO
	KBDINFO KbdInfo;
# endif
#endif

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("noraw() - called\n");
#endif

#ifdef OS2
# ifndef EMXVIDEO
	KbdGetStatus(&KbdInfo,0);
	KbdInfo.fsMask |= KEYBOARD_ASCII_MODE;
	KbdInfo.fsMask &= ~KEYBOARD_BINARY_MODE;
	KbdSetStatus(&KbdInfo,0);
# endif
#endif

#if defined( UNIXx ) /* || defined( EMXVIDEO ) NOT COMPLETE */
#ifdef USE_TERMIO
#if 0
	_CUR_TERM.prog_mode.c_lflag |= ISIG|ICANON;
	_CUR_TERM.prog_mode.c_iflag |= IXON|INPCK|ISTRIP;
	_CUR_TERM.prog_mode.c_oflag |= OPOST;
	_CUR_TERM.prog_mode.c_cc[VMIN] = _CUR_TERM.shell_mode.c_cc[VMIN];
	_CUR_TERM.prog_mode.c_cc[VTIME] = _CUR_TERM.shell_mode.c_cc[VTIME];
#endif
	_CUR_TERM.prog_mode.c_lflag |= ICANON;
	ioctl(_CUR_TERM.fd, TCSETAW, &_CUR_TERM.prog_mode);
#else
	_CUR_TERM.prog_mode.sg_flags &= ~RAW;
	ioctl(_CUR_TERM.fd, TIOCSETP, &_CUR_TERM.prog_mode);
#endif
#endif

	_cursvar.raw_inp = FALSE;
	PDC_set_ctrl_break(TRUE);
	return( OK );
}
/***********************************************************************/
int	typeahead( int fildes )
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("typeahead() - called\n");
#endif

	if (fildes < 0)
		_cursvar.refrbrk = FALSE;
	else
		_cursvar.refrbrk = TRUE;
	return(OK);
}
