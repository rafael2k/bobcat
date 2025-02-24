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
#define	LOCAL_VAR
#include <curses.h>

#ifndef NO_MEMORY_H
#include <memory.h>
#endif

#ifdef UNIXx
#define NOTLIB
#include <defs.h>
#include <term.h>
#endif

/* undefine any macros for functions defined in this module */
#undef	initscr
#undef	endwin
#undef	isendwin
#undef	newterm
#undef	set_term
#undef	delscreen
#undef	resize_screen

/* undefine any macros for functions called by this module if in debug mode */
#ifdef PDCDEBUG
#  undef	move
#  undef	wmove
#  undef	resize_window
#  undef	touchwin
#  undef	wnoutrefresh
#endif

#ifdef UNIXx
#define NOTLIB
#include <defs.h>
#include <term.h>
/* following is to stop compilation problems with #define of lines */
#undef lines
#endif

#ifdef PDCDEBUG
char *rcsid_initscr  = "$Id$";
#else
char*	_curses_notice = "PDCurses 2.2 - Public Domain 1994";
#endif

SCREEN _cursvar = {0};		/* curses variables		*/

WINDOW*	curscr=NULL;			/* the current screen image	*/
WINDOW*	stdscr=NULL;			/* the default screen window	*/
int	_default_lines = 25;	/* default terminal height	*/
int	LINES=0;			/* current terminal height	*/
int	COLS=0;			/* current terminal width	*/

#if defined	DOS
Regs regs;
#endif

/*
 * Global definitions for charget routines
 */
int	c_pindex = 0;		/* putter index */
int	c_gindex = 1;		/* getter index */
int	c_ungind = 0;		/* wungetch() push index */
int	c_ungch[NUNGETCH];	/* array of ungotten chars */
WINDOW*	_getch_win_=NULL;

/*
 * Global definitions for setmode routines
 */
struct cttyset c_sh_tty = {0};	/* tty modes for def_shell_mode */
struct cttyset c_pr_tty = {0};	/* tty modes for def_prog_mode  */
struct cttyset c_save_tty = {0};
struct cttyset c_save_trm = {0};

/*
 * Global definitions for printscan routines
 */
char c_printscanbuf[513];	/* buffer used during I/O */

/*
 * Global definitions for strget routines
 */
char *c_strbeg;

#if	EMALLOC
void*	emalloc( size_t );
void*	ecalloc( size_t, size_t );
void	efree( void* );

extern	void*	emalloc();	/* user's emalloc(size)		*/
extern	void*	ecalloc();	/* user's ecalloc(num,size)	*/
extern	void	efree();	/* user's efree(ptr)		*/
#endif

#ifndef UNIXx
extern	void*	malloc();	/* runtime's malloc(size)	*/
extern	void*	calloc();	/* runtime's calloc(num,size)	*/
extern	void	free();		/* runtime's free(ptr)		*/
#endif

void*	(*mallc)();		/* ptr to some malloc(size)	*/
void*	(*callc)();		/* ptr to some ecalloc(num,size)*/
void	(*fre)();		/* ptr to some free(ptr)	*/
void*	(*reallc)();		/* ptr to some realloc(ptr,size)	*/

#ifdef CHTYPE_LONG
chtype *acs_map;
#endif

/*man-start*********************************************************************

  Name:                                                       initscr

  Synopsis:
  	WINDOW *initscr(void);
  	int endwin(void);
  ***	int isendwin(void);
  ***	SCREEN *newterm(char *type, FILE *outfd, FILE *infd);
  	SCREEN *set_term(SCREEN *new);
  ***	void delscreen(SCREEN *sp);

  ***	int resize_screen(int nlines);

  X/Open Description:
 	The first curses routine called should be initscr().  This will
 	determine the terminal type and initialize all curses data
 	structures.  The initscr() function also arranges that the
 	first call to refresh() will clear the screen.  If errors
 	occur, initscr() will write an appropriate error message to
 	standard error and exit.  If the program wants an indication
 	of error conditions, newterm() should be used instead of
 	initscr().

 	A program should always call endwin() before exiting or
 	escaping from curses mode temporarily.  This routine will
 	restore tty modes, move the cursor to the lower left corner
 	of the screen and reset the terminal into the proper non-visual
 	mode.  To resume curses after a temporary escape, refresh() or
 	doupdate() should be called.

 	A program which outputs to more than one terminal should use
 	newterm() for each terminal instead of initscr().  The newterm()
 	function should be called once for each terminal.  It returns a 
 	value of type SCREEN* which should be saved as a reference to that
 	terminal. The arguments are the type of of terminal to be used
 	in place of TERM (environment variable), a file pointer for output
 	to the terminal and another file pointer for input from the terminal.
 	The program must also call endwin() for each terminal no longer being
 	used.

 	This function is used to switch between different terminals.
 	The screen reference 'new' becomes the new current terminal.
 	The previous terminal is returned by the routine.  This is the
 	only routine which manipulates SCREEN pointers; all other routines
 	affect only the current terminal.

  PDCurses Description:
 	Due to the fact that newterm() does not yet exist in PDCurses,
	there is no way to recover from an error in initscr().

  X/Open Return Value:
 	All functions return NULL on error, except endwin(), which
 	returns ERR on error.

  X/Open Errors:
 	No errors are defined for this function.

  Portability                             X/Open    BSD    SYS V
                                          Dec '88
      initscr                               Y        Y       Y
      endwin                                Y        Y       Y
      isendwin                              -        -      3.0
      newterm                               -        -       Y
      set_term                              -        -       Y
      delscreen                             -        -      4.0

**man-end**********************************************************************/

/***********************************************************************/
WINDOW*	initscr(void)
/***********************************************************************/
{
#ifdef CHTYPE_LONG
register int i=0;
#endif
	if  (_cursvar.alive)
		return( NULL);
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("initscr() - called\n");
#endif

#ifdef EMXVIDEO
	v_init();
#endif

	if  (_cursvar.emalloc == EMALLOC_MAGIC)
	{
#if	EMALLOC
		memset(&_cursvar, 0, sizeof(SCREEN));
		_cursvar.emalloc = TRUE;
		mallc = emalloc;
		callc = ecalloc;
		fre   = efree;
		reallc = erealloc;
#endif
	}
	else
	{
		memset(&_cursvar, 0, sizeof(SCREEN));
		mallc = malloc;
		callc = calloc;
		fre   = free;
		reallc = realloc;
	}

#ifdef UNIXx
	setupterm((char *)0,1,(int *)0);
	if (enter_ca_mode != NULL)
		putp(enter_ca_mode);
#endif

#if defined (XCURSES)
	if (Xinitscr() == ERR)
		exit(7);
#endif

	PDC_scr_open(&_cursvar, 0);
	_cursvar.orig_cursor = _cursvar.cursor;
/*	_cursvar.orig_font = PDC_get_font();*/
	_cursvar.orig_font = _cursvar.font;
	_cursvar.orgcbr = PDC_get_ctrl_break();
	_cursvar.blank = ' ';
#ifdef	FLEXOS
	_flexos_16bitmode();
#endif
/*	savetty();*/
/*	LINES = PDC_get_rows();*/
/*	COLS = PDC_get_columns(); */
	LINES = _cursvar.lines;
	COLS = _cursvar.cols;
	if (LINES < 2 || COLS < 2)
	{
		fprintf( stderr, "initscr(): LINES=%d COLS=%d: too small.\n",LINES,COLS );
		exit( 4 );
	}

	if ((curscr = newwin(LINES, COLS, 0, 0)) == (WINDOW *) NULL)
	{
		fprintf( stderr, "initscr(): Unable to create curscr.\n" );
		exit( 2 );
	}
	if ((stdscr = newwin(LINES, COLS, 0, 0)) == (WINDOW *) NULL)
	{
		fprintf( stderr, "initscr(): Unable to create stdscr.\n" );
		exit( 1 );
	}
	curscr->_clear = FALSE;
#ifdef	REGISTERWINDOWS
	_cursvar.refreshall = FALSE;
	_inswin(stdscr, (WINDOW *)NULL);
#endif

#ifdef CHTYPE_LONG
	if ((acs_map = (chtype *)(*mallc)(128*sizeof(chtype))) == (chtype *)NULL)
	{
		fprintf( stderr, "initscr(): Unable to create acs_map.\n" );
		exit( 5 );
	}
	for (i=0;i<128;i++)
		acs_map[i] = i | A_ALTCHARSET;
	PDC_init_atrtab(); /* set up default (BLACK on WHITE colours */
#endif

#ifdef EMXVIDEO
	_cursvar.tahead = -1;
#endif

	_cursvar.alive = TRUE;

#ifdef UNIXx
	PDC_setup_keys();
#else
	def_shell_mode(); /* don't do this for UNIX as scropen has already done changed things */
#endif
	return( stdscr );
}
/***********************************************************************/
int	endwin(void)
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("endwin() - called\n");
#endif

#if defined (XCURSES)
    fprintf(stderr,"start of endwin()\n");
#endif
	PDC_scr_close();
/*	resetty();*/
	if (_cursvar.orig_font != _cursvar.font)  /* screen has not been resized */
		{
		PDC_set_font(_cursvar.orig_font);
		resize_screen(PDC_get_rows());
		}

	_cursvar.visible_cursor = FALSE;	/* Force the visible cursor */
	_cursvar.cursor = _cursvar.orig_cursor;
	PDC_cursor_on();
	/*
	 * Position cursor so that the screen will not scroll until they hit
	 * a carriage return. Do this BEFORE delwin(curscr) as PDC_gotoxy() uses
	 * curscr.
	 */
	PDC_gotoxy(PDC_get_rows() - 2, 0);
	delwin(stdscr);
	delwin(curscr);
	stdscr = (WINDOW *)NULL;
	curscr = (WINDOW *)NULL;
	_cursvar.alive = FALSE;


#ifdef	FLEXOS
	_flexos_8bitmode();
#endif
/*	PDC_fix_cursor(_cursvar.orig_emulation);*/

#ifdef UNIXx
	if (exit_ca_mode != NULL)
		putp(exit_ca_mode);
#endif

#if defined(DOS) || defined(OS2)
	if (_cursvar.orig_font != _cursvar.font)  /* screen has not been resized */
		reset_shell_mode();
#endif

#if defined (XCURSES)
	XCurses_instruct(CURSES_EXIT);
#endif

	return( OK );
}
#if 0
/***********************************************************************/
SCREEN*	newterm( char *type, FILE *outfd, FILE *infd )
/***********************************************************************/
{
#ifdef	TC
#  pragma argsused
#endif
extern	void*	mallc();	/* malloc(size)		*/
extern	void*	callc();	/* calloc(num,size)	*/
extern	void	fre();		/* free(ptr)		*/

extern	void*	malloc();
extern	void*	calloc();
extern	void	free();

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("newterm() - called\n");
#endif

	if  (_cursvar.alive)
		return( ERR );

	if  (_cursvar.emalloc == EMALLOC_MAGIC)
	{
#if	EMALLOC
		memset(&_cursvar, 0, sizeof(SCREEN));
		_cursvar.emalloc = TRUE;
		mallc = emalloc;
		callc = ecalloc;
		fre   = efree;
		reallc = erealloc;
#endif
	}
	else
	{
		memset(&_cursvar, 0, sizeof(SCREEN));
		mallc = malloc;
		callc = calloc;
		fre   = free;
		reallc = realloc;
	}
	PDC_scr_open(&_cursvar, 0);
	_cursvar.orig_cursor = _cursvar.cursor;
	_cursvar.orig_font = PDC_get_font();
	_cursvar.orgcbr = PDC_get_ctrl_break();
	_cursvar.blank = ' ';
#ifdef	FLEXOS
	_flexos_16bitmode();
#endif
	savetty();
	LINES = PDC_get_rows();
	COLS = PDC_get_columns();

	if ((curscr = newwin(LINES, COLS, 0, 0)) == (WINDOW *) ERR)
	{
		return( ERR );
	}
	if ((stdscr = newwin(LINES, COLS, 0, 0)) == (WINDOW *) ERR)
	{
		return( ERR );
	}
	curscr->_clear = FALSE;
#ifdef	REGISTERWINDOWS
	_cursvar.refreshall = FALSE;
	_inswin(stdscr, (WINDOW *)NULL);
#endif
	_cursvar.alive = TRUE;
	return( &_cursvar );
}
#endif
/***********************************************************************/
SCREEN*	set_term( SCREEN *new )
/***********************************************************************/
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("set_term() - called\n");
#endif

#ifdef	TC
#  pragma argsused
#endif
	return( &_cursvar );  /* We only have one screen supported right now */
}
/***********************************************************************/
int	resize_screen(int nlines)
/***********************************************************************/
{
	WINDOW*	tmp=NULL;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("resize_screen() - called: nlines %d\n",nlines);
#endif

	if (stdscr == (WINDOW *)NULL)
		return(ERR);

#ifdef	FLEXOS
	/*
	 * Under FlexOS, this is functionally equivalent to a recallable
	 * initscr() because FlexOS does not yet support determination of
	 * screen fonts and therefore font loading and therefore text mode
	 * screen resolution changes...
	 */
	return( ERR );
#endif

#if defined(DOS)
	switch (_cursvar.adapter)
	{
	case _EGACOLOR:
		if (nlines >= 43)		PDC_set_font(_FONT8);
		else				PDC_set_80x25();
		break;

	case _VGACOLOR:
		if	(nlines > 28)		PDC_set_font(_FONT8);
		else	if (nlines > 25)	PDC_set_font(_FONT14);
		else				PDC_set_80x25();
		break;

	default:
		break;
	}
#endif

#ifdef     OS2
	if (nlines >= 43)		PDC_set_font(_FONT8);
	else	if (nlines > 25)	PDC_set_font(_FONT14);
	else				PDC_set_80x25();
#endif

	_cursvar.lines = LINES = PDC_get_rows();
	_cursvar.cols  = COLS  = PDC_get_columns();

	if (curscr->_pmaxy > LINES)
	{
		PDC_scroll(0, 0, curscr->_pmaxy - 1, COLS - 1, 0, _cursvar.orig_attr);
	}
	else
	{
		PDC_scroll(0, 0, LINES - 1, COLS - 1, 0, _cursvar.orig_attr);
	}
	if ((tmp = resize_window(curscr, LINES, COLS)) != (WINDOW *) NULL)
	{
		curscr = tmp;
	}
	else
	{
		return (ERR);
	}
	if ((tmp = resize_window(stdscr, LINES, COLS)) != (WINDOW *) NULL)
	{
		stdscr = tmp;
		touchwin(stdscr);
		wnoutrefresh(stdscr);
	}
	else
	{
		return (ERR);
	}
	return (OK);
}
