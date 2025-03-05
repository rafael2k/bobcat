#ifndef LYCURSES_H
#define LYCURSES_H

/*
 *	CR may be defined before the curses.h include occurs.
 *	There is a conflict between the termcap char *CR and the define.
 *	Assuming that the definition of CR will always be carriage return.
 *	06-09-94 Lynx 2-3-1 Garrett Arch Blythe
 */
#ifdef CR
#undef CR
#define REDEFINE_CR
#endif /* CR */

#include <curses.h>
#include <unikey.h>

#ifdef REDEFINE_CR
#define CR FROMASCII('\015')
#endif /* REDEFINE_CR */

extern int LYlines;  /* replaces LINES */
extern int LYcols;   /* replaces COLS */

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif /* HTUTILS_H */

extern void start_curses NOPARAMS;
extern void stop_curses NOPARAMS;
extern BOOLEAN setup PARAMS((char *terminal));

/* define curses functions */

#ifdef FIXME
#define start_bold()      attrset(A_BOLD)
#define stop_bold()       attroff(A_BOLD)
#define start_underline() attrset(A_UNDERLINE)
#define stop_underline()  attroff(A_UNDERLINE)
#define start_reverse()   attrset(A_REVERSE)
#define wstart_reverse(a) wattrset(a,A_REVERSE)
#define stop_reverse()    attroff(A_REVERSE)
#define wstop_reverse(a)  wattroff(a,A_REVERSE)
#endif

int start_bold();
int stop_bold();
int start_underline();
int stop_underline();
int start_reverse();
int wstart_reverse(WINDOW *a);
int stop_reverse();
int wstop_reverse(WINDOW *a);

#define LYGetYX(y, x)   getyx(stdscr, y, x)

#endif /* LYCURSES_H */
