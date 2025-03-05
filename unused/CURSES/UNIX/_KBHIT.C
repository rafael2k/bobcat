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
#define CURSES_LIBRARY	1
#include <curses.h>

#ifdef UNIX
#define NOTLIB
#include <defs.h>
#include <term.h>
#include <sys/time.h>
#include <sys/types.h>
#endif
#undef  PDC_kbhit

#ifdef PDCDEBUG
char *rcsid_PDC_kbhit  = "$Id$";
#endif



#ifdef UNIX

/*man-start*********************************************************************

  PDC_kbhit()	- Check if a character has been entered on keyboard.

  PDCurses Description:
 	This is a private PDCurses routine.

 	Outputs character 'chr' to screen in tty fashion. If a colour
 	mode is active, the character is written with colour 'colour'.

  PDCurses Return Value:
 	This function returns OK on success and ERR on error.

  PDCurses Errors:
 	No errors are defined for this function.

  Portability:
 	PDCurses	int PDC_putc( chtype character, chtype attr );

**man-end**********************************************************************/

int PDC_kbhit(void)
{
	fd_set readfds, writefds, exceptfds;
	struct timeval timeout;
	static struct termio    otty, ntty;
	int ret;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("PDC_kbhit() - called\n");
#endif

	/* Create proper environment for select() */
	FD_ZERO( &readfds );
	FD_ZERO( &writefds );
	FD_ZERO( &exceptfds );
	FD_SET( fileno(stdin), &readfds );

	/* We shall specify 0.5 sec as the waiting time */
	timeout.tv_sec  = 0;	/*   0 seconds */
	timeout.tv_usec = 500;	/* 500 microseconds */

	/* Put tty in raw mode */
	ioctl(_CUR_TERM.fd, TCGETA, &otty);
	ntty = otty;
	ntty.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL);
	ntty.c_lflag &= ~ICANON;
	ntty.c_lflag |= ISIG;
	ntty.c_cflag &= ~(CSIZE|PARENB);
	ntty.c_cflag |= CS8;
	ntty.c_iflag &= (ICRNL|ISTRIP);
	ntty.c_cc[VMIN] = 1;
	ntty.c_cc[VTIME] = 1;
	ioctl(_CUR_TERM.fd, TCSETAW, &ntty);

	/* Do a select */
	ret = select( 1, &readfds, &writefds, &exceptfds, &timeout );

	/* Reset the tty back to its original mode */
	ioctl(_CUR_TERM.fd, TCSETAW, &otty);

	return( ret );
}
#endif
