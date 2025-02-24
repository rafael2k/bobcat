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
#include <defs.h>
#include <term.h>
#undef lines
#endif

#ifdef PDCDEBUG
char *rcsid_PDCscrn  = "$Id$";
#endif

/*man-start*********************************************************************

  PDC_scr_close()	- Internal low-level binding to close the physical screen

  PDCurses Description:
 	This function provides a low-level binding for the Flexos
 	platform which must close the screen before writing to it.
 	This is a nop for the DOS platform.

 	This function is provided in order to reset the FlexOS 16 bit
 	character set for input rather than the limited input
 	character set associated with the VT52.

  PDCurses Return Value:
 	This function returns OK on success, otherwise an ERR is returned.

  PDCurses Errors:
 	The DOS platform will never fail.  The Flexos platform may fail
 	depending on the ability to close the current virtual console in
 	8 (as opposed to 16) bit mode.

  Portability:
 	PDCurses	int	PDC_scr_close( void );

**man-end**********************************************************************/

int	PDC_scr_close(void)
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("PDC_scr_close() - called\n");
#endif

#ifdef	FLEXOS
	_flexos_8bitmode();
	vir.vc_kbmode = kbmode;
	vir.vc_smode = smode;
	vir.vc_mode = cmode;
	retcode = s_set(T_VIRCON, 1L, (char *) &vir, (long) sizeof(vir));
	if  (retcode < 0L)
		return( ERR );
	return( OK );
#endif

	return( OK );
}

/*man-start*********************************************************************

  PDC_scrn_modes_equal()	- Decide if two screen modes are equal

  PDCurses Description:
 	Mainly required for OS/2. It decides if two screen modes
        (VIOMODEINFO structure) are equal. Under DOS it just compares
        two integers

  PDCurses Return Value:
 	This function returns TRUE if equal else FALSe.

  PDCurses Errors:
 	No errors are defined for this function.

  Portability:
 	PDCurses	int PDC_scrn_modes_equal( int mode1, int mode2 );
 	OS2 PDCurses	int PDC_scrn_modes_equal( VIOMODEINFO mode1, VIOMODEINFO mode2 );

**man-end**********************************************************************/

#if defined( OS2 ) && !defined( EMXVIDEO )
bool	PDC_scrn_modes_equal(VIOMODEINFO mode1, VIOMODEINFO mode2)
#else
bool	PDC_scrn_modes_equal(int mode1, int mode2)
#endif
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("PDC_scrn_modes_equal() - called\n");
#endif

#if defined( OS2 ) && !defined( EMXVIDEO )
	return (   (mode1.cb == mode2.cb) && (mode1.fbType == mode2.fbType)
            && (mode1.color == mode2.color) && (mode1.col == mode2.col)
            && (mode1.row == mode2.row) && (mode1.hres == mode2.vres)
            && (mode1.vres == mode2.vres) );
#else
	return (mode1 == mode2);
#endif
}

/*man-start*********************************************************************

  PDC_scr_open()	- Internal low-level binding to open the physical screen

  PDCurses Description:
 	This function provides a low-level binding for the Flexos
 	platform which must open the screen before writing to it.

 	This function is provided in order to access the FlexOS 16 bit
 	character set for input rather than the limited input
 	character set associated with the VT52.

  PDCurses Return Value:
 	This function returns OK on success, otherwise an ERR is returned.

  PDCurses Errors:
 	The DOS platform will never fail.  The Flexos platform may fail
 	depending on the ability to open the current virtual console in
 	8 (as opposed to 16) bit mode.

  Portability:
 	PDCurses	int	PDC_scr_open( SCREEN* internal, bool echo );

**man-end**********************************************************************/

int	PDC_scr_open(SCREEN *internal, bool echo)
{

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("PDC_scr_open() - called\n");
#endif

#ifdef	FLEXOS
	retcode = s_get(T_VIRCON, 0L, (char *) &vir, (long) sizeof(vir));
	if (retcode < 0L)
		return( ERR );

	kbmode = vir.vc_kbmode;
	cmode = vir.vc_mode;
	vir.vc_mode = 0;

	if (!echo)	vir.vc_kbmode |= VCKM_NECHO;
	else		vir.vc_kbmode &= ~VCKM_NECHO;

	smode = vir.vc_smode;
	retcode = s_set(T_VIRCON, 0L, (char *) &vir, (long) sizeof(vir));
	if  (retcode < 0L)
		return( ERR );

	if (vir.vc_type & 0x03)	internal->mono = TRUE;
	else			internal->mono = FALSE;

	internal->orig_attr = vir.vc_flcolor | vir.vc_blcolor;
	_flexos_16bitmode();
#endif

#ifdef	DOS
	internal->orig_attr	 = 0;
	internal->orig_emulation = getdosmembyte (0x487);
#endif

#ifdef	OS2
	internal->orig_attr	 = 0;
	internal->orig_emulation = 0;
#endif

#ifdef UNIXx
	PDC_get_cursor_pos(&internal->cursrow, &internal->curscol);
	internal->autocr	= FALSE;		/* lf -> crlf by default      */
	internal->raw_out	= FALSE;	/* tty I/O modes	      */
	internal->raw_inp	= FALSE;	/* tty I/O modes	      */
	internal->cbreak	= FALSE;
	internal->echo		= echo;
	internal->refrbrk	= FALSE;	/* no premature end of refresh*/
	internal->visible_cursor= TRUE;		/* Assume that it is visible  */
	internal->cursor	= 0;
	internal->font		= 0;
	internal->lines		= PDC_get_rows();
	internal->cols		= PDC_get_columns();
	internal->audible	= TRUE;
	internal->direct_video	= FALSE;		/* Assume that we can	      */
	internal->adapter	= PDC_query_adapter_type();
	_CUR_TERM.prog_mode.c_iflag &= ~(ICRNL);
	_CUR_TERM.prog_mode.c_oflag &= ~(ONLCR|OPOST);
	_CUR_TERM.prog_mode.c_lflag &= ~(ICANON|ECHO);
	_CUR_TERM.prog_mode.c_cc[VMIN] = 1;
	ioctl(_CUR_TERM.fd, TCSETA, &_CUR_TERM.prog_mode);
	return(OK);
#endif

#if defined(XCURSES)
	internal->cursrow = internal->curscol = 0;
	internal->direct_video	= FALSE;		/* Assume that we can't	      */
#else
	PDC_get_cursor_pos(&internal->cursrow, &internal->curscol);
	internal->direct_video	= TRUE;		/* Assume that we can	      */
#endif

	internal->autocr	= TRUE;		/* lf -> crlf by default      */
	internal->raw_out	= FALSE;	/* tty I/O modes	      */
	internal->raw_inp	= FALSE;	/* tty I/O modes	      */
	internal->cbreak	= TRUE;
	internal->echo		= echo;
/* under System V Curses, typeahead checking is enabled by default */
	internal->refrbrk	= TRUE;	/* allow premature end of refresh*/
#if !defined(OS2) && !defined(XCURSES)
	internal->video_seg	= 0xb000;	/* Base screen segment addr   */
	internal->video_ofs	= 0x0;		/* Base screen segment ofs    */
#endif
	internal->video_page	= 0;		/* Current Video Page	      */
	internal->visible_cursor= TRUE;		/* Assume that it is visible  */
	internal->cursor	= PDC_get_cursor_mode();
#ifdef EMXVIDEO
	internal->tahead	= -1;
#endif
	internal->adapter	= PDC_query_adapter_type();
	internal->font		= PDC_get_font();
#if !defined(EMXVIDEO) && !defined(XCURSES)
	internal->scrnmode	= PDC_get_scrn_mode();
#endif
	internal->lines		= PDC_get_rows();
	internal->cols		= PDC_get_columns();
	internal->audible	= TRUE;
	internal->visibility	= 1;
	return( OK );
}
