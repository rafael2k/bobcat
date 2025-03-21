#include "HTUtils.h"
#include "LYCurses.h"
#include "LYUtils.h"
#include "LYGlobalDefs.h"
#include "LYSignal.h"
#include "LYClean.h"
#include "tcp.h"

#include "LYexit.h"
#include "LYLeaks.h"

#include <stdio.h>

/*
 * These are routines to start and stop curses and to cleanup
 * the screen at the end
 *
 */

BOOLEAN use_color = FALSE;

int normal_fore = COLOR_WHITE;
int normal_back = COLOR_BLACK;
unsigned long normal_bold = A_NORMAL;

int underline_fore = COLOR_CYAN;
int underline_back = COLOR_BLACK;
unsigned long underline_bold = A_BOLD;

int reverse_fore = COLOR_BLACK;
int reverse_back = COLOR_WHITE;
unsigned long reverse_bold = A_NORMAL;

int bold_fore = COLOR_WHITE;
int bold_back = COLOR_BLACK;
unsigned long bold_bold = A_BOLD;

PRIVATE int dumbterm PARAMS((char *terminal));
BOOLEAN LYCursesON = FALSE;

PRIVATE int startup = 0;


/*
#  define COLOR_BLACK           0
#  define COLOR_BLUE            1
#  define COLOR_GREEN           2
#  define COLOR_CYAN            3
#  define COLOR_RED             4
#  define COLOR_MAGENTA         5
#  define COLOR_YELLOW          6
#  define COLOR_WHITE           7
*/
#ifdef FIXME
#define start_bold()
#define stop_bold()       attroff(A_BOLD)
#define start_underline() attrset(A_UNDERLINE)
#define stop_underline()  attroff(A_UNDERLINE)
#define start_reverse()   attrset(A_REVERSE)
#define wstart_reverse(a) wattrset(a,A_REVERSE)
#define stop_reverse()    attroff(A_REVERSE)
#define wstop_reverse(a)  wattroff(a,A_REVERSE)
#endif

int start_bold()
{
    if (use_color)
	attrset(COLOR_PAIR(4) | bold_bold);
    else
	attrset(A_BOLD);
    return(1);
}

int stop_bold()
{
    if (use_color)
	attrset(COLOR_PAIR(1) | normal_bold);
    else
	attroff(A_BOLD);
    return(1);
}

int start_underline()
{
    if (use_color)
	attrset(COLOR_PAIR(2) | underline_bold);
    else
	attrset(A_UNDERLINE);

    return(1);
}

int stop_underline()
{
    if (use_color)
	attrset(COLOR_PAIR(1) | normal_bold);
    else
	attroff(A_UNDERLINE);

    return(1);
}

int start_reverse()
{
    if (use_color)
	attrset(COLOR_PAIR(3) | reverse_bold);
    else
	attrset(A_REVERSE);
    return(1);
}

int stop_reverse()
{
    if (use_color)
	attrset(COLOR_PAIR(1) | normal_bold);
    else
	attroff(A_REVERSE);
    return(1);
}

int wstart_reverse(WINDOW *a)
{
    if (use_color)
	wattrset(a,COLOR_PAIR(3) | reverse_bold);
    else
	wattrset(a,A_REVERSE);
    return(1);
}

int wstop_reverse(WINDOW *a)
{
    if (use_color)
	wattrset(a,COLOR_PAIR(1) | normal_bold);
    else
	wattroff(a,A_REVERSE);
    return(1);
}

void
start_curses()
{
    static BOOLEAN first_time = TRUE;

    if(first_time)
    {
	initscr();	/* start curses */
	first_time = FALSE;
	cbreak();
	noecho();

//	if(!LYShowCursor) curs_set (0);

	/* nonl();   *//* seems to slow things down */
//	cursoff();
	keypad(stdscr, TRUE);
	fflush(stdin);
	fflush(stdout);

	if(!has_colors()) use_color = FALSE;

	if (use_color)
	{
	  start_color();

	  init_pair(1,normal_fore,normal_back); /* normal */
	  init_pair(2,underline_fore,underline_back); /* underline */
	  init_pair(3,reverse_fore,reverse_back); /* reverse */
	  init_pair(4,bold_fore,bold_back); /* bold */

	  // bkgd(COLOR_PAIR(1) | normal_bold);
	  erase();

	  attrset(COLOR_PAIR(1) | normal_bold);
	}
    }

    LYCursesON = TRUE;
    clear();
    // if(!firsttime) sock_init();
}

void
stop_curses()
{
//	extern char firsttime;
//	endwin();	/* stop curses */
//	curs_set (1);
//	curson();
	clear();
	refresh();
//	fflush(stdout);

    // if(!firsttime) sock_exit();

	LYCursesON = FALSE;
}

/*
 * check terminal type, start curses & setup terminal
 */
PUBLIC BOOLEAN setup ARGS1(char *,terminal)
{
    static char term_putenv[120];
    char term[120];
    char buffer[120];

 /* if the display was not set by a command line option then see 
  * if it is available from the environment 
  */
    display = getenv("DISPLAY");

    if(terminal != NULL) {
	sprintf(term_putenv,"TERM=%s",terminal);
	(void) putenv(term_putenv);
    }

	/* query the terminal type */
    if(dumbterm(getenv("TERM"))) {
	printf("\n\n  Your Terminal type is unknown!\n\n");
	printf("  Enter a terminal type: [vt100] ");
	gets(buffer);

	if(strlen(buffer) == 0)
	    strcpy(buffer,"vt100");

	sprintf(term_putenv,"TERM=%s", buffer);
	putenv(term_putenv);  /* */
	printf("\nTERMINAL TYPE IS SET TO %s\n",getenv("TERM"));
    }


    start_curses();

    LYlines = LINES;
    LYcols = COLS;

    return(1);
}

PRIVATE int dumbterm ARGS1(char *,terminal)
{
    int dumb = FALSE;

    /*
     *	Began checking for terminal == NULL in case that TERM environemnt
     *	variable is not set.  Thanks to Dick Wesseling (ftu@fi.ruu.nl)
     */
    if (terminal == NULL ||
	!strcasecomp(terminal, "network") ||
	!strcasecomp(terminal, "unknown") ||
	!strcasecomp(terminal, "dialup")  ||
	!strcasecomp(terminal, "dumb")    ||
	!strcasecomp(terminal, "switch")  ||
	!strcasecomp(terminal, "ethernet")  )
	dumb = TRUE;
    return(dumb);
}

