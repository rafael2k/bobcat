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
/********************************* tui.c ************************************/
/*
 * File   : tui.c      'textual user interface'
 * Author : P.J. Kunst  (kunst@prl.philips.nl)
 * Date   : 25-02-93
 * Version: 1.02
 */

#include <ctype.h>
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tui.h"

#ifdef PDCDEBUG
char *rcsid_tui  = "$Id$";
#endif

#if defined(__unix) && !defined(GO32)
#define CPUACCOUNT                /* necessary if CPU cycles are to be paid! */
#endif

#ifdef A_COLOR
# define TITLECOLOR         1                   /* color pair indices */
# define MAINMENUCOLOR      (2 | A_BOLD)
# define MAINMENUREVCOLOR   (3 | A_BOLD | A_REVERSE)
# define SUBMENUCOLOR       (4 | A_BOLD)
# define SUBMENUREVCOLOR    (5 | A_BOLD | A_REVERSE)
# define BODYCOLOR          6
# define STATUSCOLOR        (7 | A_BOLD)
# define INPUTBOXCOLOR      8
# define EDITBOXCOLOR       (9 | A_BOLD | A_REVERSE)
#else
# define TITLECOLOR         0                   /* color pair indices */
# define MAINMENUCOLOR      (A_BOLD)
# define MAINMENUREVCOLOR   (A_BOLD | A_REVERSE)
# define SUBMENUCOLOR       (A_BOLD)
# define SUBMENUREVCOLOR    (A_BOLD | A_REVERSE)
# define BODYCOLOR          0
# define STATUSCOLOR        (A_BOLD)
# define INPUTBOXCOLOR      0
# define EDITBOXCOLOR       (A_BOLD | A_REVERSE)
#endif


#define  th   1                   /* title window height */
#define  mh   1                   /* main menu height */
#define  sh   2                   /* status window height */
#define  bh   (LINES-th-mh-sh-1)  /* body window height */
#define  bw   COLS                /* body window width */


/******************************* STATIC ************************************/

static WINDOW *wtitl, *wmain, *wbody, *wstat; /* title, menu, body, status win*/
static int nexty, nextx;
static int key = ERR, ch = ERR;
static bool quit = FALSE;
static bool incurses = FALSE;

#ifndef PDCURSES
static char wordchar (void) { return 0x17; }  /* ^W */ 
#endif

static char *padstr (char *s, int length)
{
  static char buf[MAXSTRLEN];
  char fmt[10];

  sprintf (fmt, strlen(s)>length ? "%%.%ds" : "%%-%ds", length);
  sprintf (buf, fmt, s);
  return buf;
}

static char *prepad (char *s, int length)
{
  int i;
  char *p = s;

  if (length > 0)
  {
    memmove ((void *)(s+length), (const void *)s, strlen(s)+1);
    for (i=0; i<length; i++) *p++ = ' ';
  }
  return s;
}

static void rmline (WINDOW *win, int nr)   /* keeps box lines intact */
{
  mvwaddstr (win, nr, 1, padstr(" ", bw-2));
  wrefresh (win);
}

static void initcolor (void)
{
#ifdef A_COLOR
  if (has_colors()) start_color();     /* foreground, background */
  init_pair (TITLECOLOR       & ~A_ATTR, COLOR_BLACK, COLOR_CYAN);      
  init_pair (MAINMENUCOLOR    & ~A_ATTR, COLOR_WHITE, COLOR_CYAN);    
  init_pair (MAINMENUREVCOLOR & ~A_ATTR, COLOR_WHITE, COLOR_BLACK);
  init_pair (SUBMENUCOLOR     & ~A_ATTR, COLOR_WHITE, COLOR_CYAN);    
  init_pair (SUBMENUREVCOLOR  & ~A_ATTR, COLOR_WHITE, COLOR_BLACK);   
  init_pair (BODYCOLOR        & ~A_ATTR, COLOR_WHITE, COLOR_BLUE);      
  init_pair (STATUSCOLOR      & ~A_ATTR, COLOR_WHITE, COLOR_CYAN);   
  init_pair (INPUTBOXCOLOR    & ~A_ATTR, COLOR_BLACK, COLOR_CYAN);
  init_pair (EDITBOXCOLOR     & ~A_ATTR, COLOR_WHITE, COLOR_BLACK);
#endif
}

static void setcolor (WINDOW *win, chtype color)
{
  chtype attr = color & A_ATTR;  /* extract Bold, Reverse, Blink bits */

#ifdef A_COLOR
  attr &= ~A_REVERSE;  /* ignore reverse, use colors instead! */
  wattrset (win, COLOR_PAIR(color & A_CHARTEXT) | attr);
#else
  attr &= ~A_BOLD;     /* ignore bold, gives messy display on HP-UX */
  wattrset (win, attr);
#endif
}

static void colorbox (WINDOW *win, chtype color, int hasbox)
{
  chtype attr = color & A_ATTR;  /* extract Bold, Reverse, Blink bits */

  setcolor (win, color);
#ifdef A_COLOR
  if (has_colors())
     wbkgd (win, COLOR_PAIR(color & A_CHARTEXT) | (attr & ~A_REVERSE));
  else
     wbkgd (win, color);
#else
  wbkgd (win, color);
#endif
  werase (win); 
  if (hasbox && win->_maxy > 2) box (win, 0, 0);
  touchwin (win); wrefresh (win);
}

static void idle (void)
{
  char buf[MAXSTRLEN];
  time_t t;
  struct tm *tp;

  if (time (&t) == -1) return;  /* time not available */
  tp = localtime(&t);
  sprintf (buf, " %.2d-%.2d-%.2d  %.2d:%.2d:%.2d", 
    tp->tm_mday, tp->tm_mon + 1, tp->tm_year,
    tp->tm_hour, tp->tm_min, tp->tm_sec);
  mvwaddstr (wtitl, 0, bw-strlen(buf)-2, buf);
  wrefresh (wtitl); 
#ifdef CPUACCOUNT
  sleep (1);    /* necessary if CPU cycles are to be paid ! */
#endif
}

static void menudim (menu *mp, int *lines, int *columns)
{
  int n, l, max = 0;

  for (n=0; mp->func; n++, mp++)
    if ((l = strlen(mp->name)) > max) max = l;
  *lines = n;
  *columns = max + 2;
}

static void setmenupos (int y, int x)
{
  nexty = y; nextx = x;
}

static void getmenupos (int *y, int *x)
{
  *y = nexty; *x = nextx;
}

static int hotkey (char *s)
{
  int c, c0 = *s;  /* if no upper case found, return first char */

  for (c = *s; c; s++) if (isupper(c)) break;
  return c ? c : c0;
}

static void repaintmenu (WINDOW *wmenu, menu *mp)
{
  int i;
  menu *p = mp;

  for (i=0; p->func; i++, p++)
    mvwaddstr (wmenu, i+1, 2, p->name);
  touchwin (wmenu); wrefresh (wmenu);
}

static void repaintmainmenu (int width, menu *mp)
{
  int i;
  menu *p = mp;

  for (i=0; p->func; i++, p++)
    mvwaddstr (wmain, 0, i*width, prepad (padstr(p->name, width-1), 1));
  touchwin (wmain); wrefresh (wmain);
}

static void mainhelp (void)
{
#ifdef ALT_X
  statusmsg ("Use arrow keys and Enter to select (Alt-X to quit)");
#else
  statusmsg ("Use arrow keys and Enter to select");
#endif
}

static void hidecursor (void)
{
#if defined(PDCURSES) || defined(SYSV)
  curs_set (0);
#endif
}

static void normalcursor (void)
{
#if defined(PDCURSES) || defined(SYSV)
  curs_set (1);
#endif
}

static void insertcursor (void)
{
#if defined(PDCURSES) || defined(SYSV)
  curs_set (2);
#endif
}

static void mainmenu (menu *mp)
{
  int nitems, barlen, old = -1, cur = 0, c, cur0;

  menudim (mp, &nitems, &barlen);
  repaintmainmenu (barlen, mp);

  while (!quit)
  {
    if (cur != old)
    {
      if (old != -1)
      {
        mvwaddstr (wmain, 0, old*barlen, 
          prepad (padstr(mp[old].name, barlen-1), 1));
        statusmsg (mp[cur].desc);
      }
      else mainhelp ();
      setcolor (wmain, MAINMENUREVCOLOR);
      mvwaddstr (wmain, 0, cur*barlen, 
        prepad (padstr(mp[cur].name, barlen-1), 1));
      setcolor (wmain, MAINMENUCOLOR);
      old = cur;
      wrefresh (wmain);
    }
    switch (c = (key != ERR ? key : waitforkey()))
    {
      case KEY_DOWN:
      case '\n':                              /* menu item selected ! */
        touchwin (wbody); wrefresh (wbody);
        rmerror ();
        setmenupos (th+mh, cur*barlen);
        normalcursor ();
        (mp[cur].func)();                     /* perform function */
        hidecursor ();
        switch (key)
        {
          case KEY_LEFT:
            cur = (cur+nitems-1) % nitems;
            key = '\n';
            break;

          case KEY_RIGHT:
            cur = (cur+1) % nitems;
            key = '\n';
            break;

          default:
            key = ERR;
            break;
        }
        repaintmainmenu (barlen, mp);
        old = -1;
        break;

      case KEY_LEFT:
        cur = (cur+nitems-1) % nitems;
        break;

      case KEY_RIGHT:
        cur = (cur+1) % nitems;
        break;

      case KEY_ESC:
        mainhelp ();
        break;

      default:
        cur0 = cur;
        do { cur = (cur+1) % nitems;
        } while ((cur != cur0) && (hotkey(mp[cur].name) != toupper(c)));
        if (hotkey(mp[cur].name) == toupper(c)) key = '\n';
        break;
    }
  }
  rmerror ();
  touchwin (wbody); wrefresh (wbody);
}

static void cleanup (void) /* cleanup curses settings */
{
  if (incurses)
  {
    delwin (wtitl);
    delwin (wmain);
    delwin (wbody);
    delwin (wstat);
    normalcursor ();
    endwin (); 
    incurses = FALSE;
  }
}


/******************************* EXTERNAL **********************************/

void clsbody (void)
{
  werase (wbody); wmove (wbody, 0, 0);
}

int bodylen (void)
{
  return wbody->_maxy;
}

WINDOW *bodywin (void)
{
  return wbody;
}

void rmerror (void)
{
  rmline (wstat, 0);
}

void rmstatus (void)
{
  rmline (wstat, 1);
}

void titlemsg (char *msg)
{
  mvwaddstr (wtitl, 0, 2, padstr(msg, bw-3));
  wrefresh (wtitl);
}

void bodymsg (char *msg)
{
  waddstr (wbody, msg);
  wrefresh (wbody);
}

void errormsg (char *msg)
{
  beep ();
  mvwaddstr (wstat, 0, 2, padstr(msg, bw-3));
  wrefresh (wstat);
}

void statusmsg (char *msg)
{
  mvwaddstr (wstat, 1, 2, padstr(msg, bw-3));
  wrefresh (wstat);
}

bool keypressed (void)
{
  if (ch == ERR) ch = wgetch(wbody);
  return ch != ERR;
}

int getkey (void)
{
  int c = ch;

  ch = ERR;
#ifdef ALT_X
  quit = (c == ALT_X);     /* PC only ! */
#endif
  return c;
}

void flushkeys (void)
{
  while (keypressed()) getkey();
}

int waitforkey (void)
{
  flushkeys ();
  while (!keypressed()) idle ();
  return getkey ();
}

void DoExit (void)   /* terminate program */
{
  quit = TRUE;
}

void domenu (menu *mp)
{
  int y, x, nitems, barlen, mheight, mw, old = -1, cur = 0, cur0;
  bool stop = FALSE;
  WINDOW *wmenu;

  hidecursor ();
  getmenupos (&y, &x);
  menudim (mp, &nitems, &barlen);
  mheight = nitems+2; mw = barlen+2;
  wmenu = newwin (mheight, mw, y, x);
  colorbox (wmenu, SUBMENUCOLOR, 1);
  repaintmenu (wmenu, mp);

  key = ERR;
  while (!stop && !quit)
  {
    if (cur != old)
    {
      if (old != -1)
        mvwaddstr (wmenu, old+1, 1, prepad (padstr(mp[old].name, barlen-1), 1));
      setcolor (wmenu, SUBMENUREVCOLOR);
      mvwaddstr (wmenu, cur+1, 1, prepad (padstr(mp[cur].name, barlen-1), 1));
      setcolor (wmenu, SUBMENUCOLOR);
      statusmsg (mp[cur].desc);
      old = cur;
      wrefresh (wmenu);
    }
    switch (key = ((key != ERR) ? key : waitforkey()))
    {
      case '\n':                              /* menu item selected ! */
        touchwin (wbody); wrefresh (wbody);
        setmenupos (y+1, x+1);
        rmerror ();
        key = ERR;
        normalcursor ();
        (mp[cur].func)();                     /* perform function */
        hidecursor ();
        repaintmenu (wmenu, mp);
        old = -1;
        break;

      case KEY_UP:
        cur = (cur+nitems-1) % nitems;
        key = ERR;
        break;

      case KEY_DOWN:
        cur = (cur+1) % nitems;
        key = ERR;
        break;

      case KEY_ESC:
      case KEY_LEFT:
      case KEY_RIGHT:
        if (key == KEY_ESC) key = ERR;  /* return to previous submenu */
        stop = TRUE;
        break;

      default:
        cur0 = cur;
        do { cur = (cur+1) % nitems;
        } while ((cur != cur0) && (hotkey(mp[cur].name) != toupper((int)key)));
        key = (hotkey(mp[cur].name) == toupper((int)key)) ? '\n' : ERR;
        break;
    }
  }
  rmerror ();
  delwin (wmenu);
  touchwin (wbody); wrefresh (wbody);
}

void startmenu (menu *mp, char *title)
{
  initscr ();
  incurses = TRUE;
  initcolor ();
/*  atexit (cleanup); */

  wtitl = subwin (stdscr, th,  bw,    0, 0);
  wmain = subwin (stdscr, mh,  bw,   th, 0);
  wbody = subwin (stdscr, bh,  bw,  th+mh, 0);
  wstat = subwin (stdscr, sh,  bw, th+mh+bh, 0);

  colorbox (wtitl, TITLECOLOR, 0);
  colorbox (wmain, MAINMENUCOLOR, 0);
  colorbox (wbody, BODYCOLOR, 0);
  colorbox (wstat, STATUSCOLOR, 0);

  if (title) titlemsg (title);

  cbreak ();                /* direct input (no newline required)... */
  noecho ();                /* ... without echoing */
  hidecursor ();            /* hide cursor (if possible) */
  nodelay (wbody, TRUE);    /* don't wait for input */
  keypad (wbody, TRUE);     /* enable cursor keys */
  scrollok (wbody, TRUE);   /* enable scrolling in main window */
  leaveok (stdscr, TRUE);
  leaveok (wtitl, TRUE);
  leaveok (wmain, TRUE);
  leaveok (wstat, TRUE);

  mainmenu (mp);

  cleanup ();
}


/*man-start*********************************************************************

  weditstr()     - edit string

  PDCurses Description:
        The initial value of 'str' with a maximum length of 'field'-1,
        which is supplied by the calling routine, is editted. The user's 
        erase (^H), kill (^U) and delete word (^W) chars are interpreted. 
        The PC insert or Tab keys toggle between insert and edit mode.
        Escape aborts the edit session, leaving 'str' unchanged.
        Enter, Up or Down Arrow are used to accept the changes to 'str'.

        NOTE: editstr(), mveditstr(), and mvweditstr() are macros.

  PDCurses Return Value:
        These functions return the input terminating character on 
        success (Escape, Enter, Up or Down Arrow) and ERR on error.

  PDCurses Errors:
        It is an error to call this function with a NULL window pointer.
        The length of the initial 'str' must not exceed 'field'-1.

  Portability:
        PDCurses        int weditstr( WINDOW* win, char* str, int field );
        X/Open Dec '88 
        BSD Curses    
        SYS V Curses 

**man-end**********************************************************************/


static void repainteditbox (WINDOW *win, int x, char *buf)
{
  werase (win); mvwprintw (win, 0, 0, "%s", padstr (buf, win->_maxx));
  wmove (win, 0, x); wrefresh (win); 
}

int weditstr (WINDOW *win, char *buf, int field)
{
  char org[MAXSTRLEN], *tp, *bp = buf;
  bool defdisp = TRUE, stop = FALSE, insert = FALSE;
  int cury, curx, begy, begx, oldattr;
  WINDOW *wedit;
  int c=0;

  if ((field>=MAXSTRLEN) || (buf==NULL) || (strlen(buf) > field-1)) return ERR;
  strcpy (org, buf);  /* save original */

  wrefresh (win);
  getyx (win, cury, curx); getbegyx (win, begy, begx);
  wedit = subwin (win, 1, field, begy+cury, begx+curx);  /* window relative */
  oldattr = wedit->_attrs;
  colorbox (wedit, EDITBOXCOLOR, 0);

  normalcursor ();
#ifdef CPUACCOUNT
  nodelay (win, FALSE);
  keypad (wedit, TRUE);
#endif
  while (!stop)
  {
    repainteditbox (wedit, bp-buf, buf);
#ifdef CPUACCOUNT
    switch (c = wgetch(wedit))
#else
    switch (c = waitforkey())
#endif
    {
      case KEY_ESC:
        strcpy (buf, org);  /* restore original */
        stop = TRUE;
        break;

      case '\n':  /* ENTER */
      case KEY_UP:
      case KEY_DOWN:
        stop = TRUE;
        break;

      case KEY_LEFT:
        if (bp > buf) bp--;
        break;

      case KEY_RIGHT:
        defdisp = FALSE;
        if (bp - buf < strlen(buf)) bp++;
        break;

      case '\t':     /* TAB, because Insert not properly handled on HP-UX ! */
      case KEY_IC:   /* enter insert mode */
      case KEY_EIC:  /* exit insert mode */
        defdisp = FALSE;
        insert = !insert;
        if (insert) insertcursor (); else normalcursor ();
        break;

      default:
        if (c == erasechar())  /* backspace, ^H */
        {
          if (bp > buf)
          {
            memmove ((void *)(bp-1), (const void *)bp, strlen(bp)+1);
            bp--;
          }
        }
        else if (c == killchar())  /* ^U */
        {
          bp = buf; *bp = '\0';
        }
        else if (c == wordchar())  /* ^W */
        {
          tp = bp;
          while ((bp > buf) && (*(bp-1) == ' ')) bp--;
          while ((bp > buf) && (*(bp-1) != ' ')) bp--;
          memmove ((void *)bp, (const void *)tp, strlen(tp)+1);
        }
        else if (isprint(c))
        {
          if (defdisp) { bp = buf; *bp = '\0'; defdisp = FALSE; }
          if (insert)
          {
            if (strlen(buf) < field-1)
            {
              memmove ((void *)(bp+1), (const void *)bp, strlen(bp)+1);
              *bp++ = c;
            }
          }
          else if (bp-buf < field-1)
          {
            if (!*bp) bp[1] = '\0'  /* append new string terminator */;
            *bp++ = c;
          }
        }
        break;
    }
  }
#ifdef CPUACCOUNT
  nodelay (win, TRUE);
#endif
  wattrset (wedit, oldattr);
  repainteditbox (wedit, bp-buf, buf);
  delwin (wedit);
  return c;
}

WINDOW *winputbox (WINDOW *win, int nlines, int ncols)
{
  int cury, curx, begy, begx;
  WINDOW *winp;

  getyx (win, cury, curx); getbegyx (win, begy, begx);
  winp = newwin (nlines, ncols, begy+cury, begx+curx); /* window relative */
  colorbox (winp, INPUTBOXCOLOR, 1);
  return winp;
}

int getstrings (char *desc[], char *buf[], int field)
{
  WINDOW *winput;
  int oldy, oldx, maxy, maxx, nlines, ncols, i, n, l, max = 0;
  int c=0;
  bool stop = FALSE;

  for (n=0; desc[n]; n++) if ((l = strlen(desc[n])) > max) max = l;
  nlines = n + 2; ncols = max + field + 4;
  getyx (wbody, oldy, oldx); getmaxyx (wbody, maxy, maxx);
  winput = mvwinputbox (wbody, (maxy-nlines)/2, (maxx-ncols)/2, nlines, ncols);
  for (i=0; i<n; i++) mvwprintw (winput, i+1, 2, "%s", desc[i]);
  i = 0;
  while (!stop)
  {
    switch (c = mvweditstr(winput, i+1, max+3, buf[i], field))
    {
      case KEY_ESC:
        stop = TRUE;
        break;

      case KEY_UP:
        i = (i+n-1)%n;
        break;

      case '\n':
      case '\t':
      case KEY_DOWN:
        if (++i == n) stop = TRUE;  /* all passed ? */
        break;
    }
  }
  delwin (winput);
  touchwin (wbody); wmove (wbody, oldy, oldx); wrefresh (wbody);
  return c;
}
