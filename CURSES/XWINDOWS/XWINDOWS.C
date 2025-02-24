/*
 * Copyright 1994 - Mark Hessling
 */

#if defined(XCURSES)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#ifdef SYSVR4
#include <sys/utsname.h>
#endif
#include <fcntl.h>

#include "curses.h"

#define ICON_WIDTH 64
#define ICON_HEIGHT 64

static unsigned char icon_bitmap_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
   0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x00, 0xc0, 0x00, 0x00,
   0x00, 0x00, 0xfc, 0x03, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x07,
   0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x07, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x00, 0xf0, 0x0f, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1f,
   0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x06, 0x00, 0x00,
   0x00, 0x00, 0xc0, 0x3f, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x80, 0x7f,
   0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xfe, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe,
   0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x33, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xf8, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0,
   0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x0e, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x60, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0,
   0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0x7f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x98, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c,
   0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xfe, 0x01, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x03, 0xfc, 0x03, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01,
   0xfc, 0x03, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xf8, 0x07, 0x00, 0x00,
   0x00, 0x00, 0xc0, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00,
   0xe0, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0xe0, 0x1f, 0x00, 0x00,
   0x00, 0x00, 0x18, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00,
   0x80, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0xff, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x23, 0x50, 0x1e,
   0x7c, 0xf0, 0xe0, 0x03, 0x60, 0x26, 0x50, 0x33, 0xc6, 0x98, 0x31, 0x06,
   0x30, 0x2c, 0xd0, 0x61, 0x83, 0x0d, 0x1b, 0x0c, 0x10, 0x28, 0xd0, 0x40,
   0x01, 0x05, 0x0a, 0x08, 0x10, 0x20, 0x50, 0x00, 0x01, 0x05, 0x0a, 0x08,
   0x10, 0x20, 0x50, 0x00, 0x03, 0x04, 0x1a, 0x00, 0x10, 0x20, 0x50, 0x00,
   0x06, 0x04, 0x32, 0x00, 0x10, 0x20, 0x50, 0x00, 0x7c, 0xfc, 0xe3, 0x03,
   0x10, 0x20, 0x50, 0x00, 0xc0, 0x04, 0x00, 0x06, 0x10, 0x20, 0x50, 0x00,
   0x80, 0x05, 0x00, 0x0c, 0x10, 0x20, 0x50, 0x00, 0x01, 0x05, 0x0a, 0x08,
   0x10, 0x28, 0x50, 0x00, 0x01, 0x05, 0x0a, 0x08, 0x30, 0x6c, 0x58, 0x00,
   0x83, 0x0d, 0x1b, 0x0c, 0x60, 0xc6, 0x4c, 0x00, 0xc6, 0x98, 0x31, 0x06,
   0xc0, 0x83, 0x47, 0x00, 0x7c, 0xf0, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static XrmDatabase commandlineDB, rDB;

struct XCursesKey
{
 char *xname;
 int keycode;
};
typedef struct XCursesKey XCURSESKEY;

XCURSESKEY XCursesKeys[] =
{
 {"Left",KEY_LEFT},
 {"Right",KEY_RIGHT},
 {"Up",KEY_UP},
 {"Down",KEY_DOWN},
 {"Home",KEY_HOME},
 {"End",KEY_END},
 {"Prior",KEY_PPAGE},
 {"Next",KEY_NPAGE},
 {"Insert",KEY_IC},
 {NULL,0},
};

#define BITMAPDEPTH 1
#define MAX_COLORS 8

/* Display and screen_num are used as arguments to nearly every Xlib routine, 
 * so it simplifies routine calls to declare them global.  If there were 
 * additional source files, these variables would be declared extern in
 * them. */
static Display *display;
static int screen_num;
static Screen *screen_ptr;

/* pixel values */
unsigned long foreground_pixel, background_pixel, border_pixel;

/* values for window_size in main, is window big enough to be useful? */
#define SMALL 0

static int colors[MAX_COLORS];
static char *color_names[MAX_COLORS];
static char *default_color_names[MAX_COLORS] = 
{"Black", 
 "Red", 
 "Green", 
 "Yellow",
 "Blue", 
 "Magenta", 
 "Cyan", 
 "White"};

 extern char *XCursesProgramName;
 static Window win;
 static Window iconwin;
 static GC normal_gc,bold_gc,cursor_gc;
 static XFontStruct *XCursesFontInfo;
 static XFontStruct *XCursesBoldFontInfo;
 static char *display_name = NULL;
 static char *DefaultFont = "9x15";
 static char *DefaultBoldFont = "9x15bold";
 static char XCursesFontName[50];
 static char XCursesBoldFontName[50];
 static int XCursesFontHeight,XCursesFontWidth;
 static int XCursesWindowWidth,XCursesWindowHeight;
 static int XCursesMinWindowWidth,XCursesMinWindowHeight;
 static int window_size = 0;
 static int XCursesLINES=24;
 static int XCursesCOLS=80;
 static char *bitmap_file=NULL;
 static int cursor_colour=COLOR_RED;

 Window focus_window;
 int revert_to;

 static        unsigned int width, height, x, y;     /* window size and position */
 static        unsigned int borderwidth = 4;         /* four pixels */

 static int icon_bitmap_width;
 static int icon_bitmap_height;

 static        Pixmap icon_pixmap;
 static int             screen_number;
 static Visual          *visual;
 static Colormap        colormap;
 static XFontStruct     *theFont;
 static Cursor          XCursesCursor;

 static Bool after_first_curses_request = False;
 static Bool after_first_expose_event   = False;

 static int display_sockets[2];
 static int display_sock;
 static int key_sockets[2];
 static int key_sock;
 fd_set readfds;
 fd_set writefds;
 struct timeval timeout;

#ifdef PROTO
static void get_GC(Window, GC *, XFontStruct *,int ,int );
static void place_text(Window, GC , XFontStruct *);
static void place_graphics(Window , GC , int, int , int );
static void TooSmall(Window , GC ,XFontStruct *);
static void makeXY(int ,int ,int ,int ,int *,int *);
static int get_colors(void);
int Xinitscr(void);
static void mergeDatabases(void);
static char *getHomeDir(char *);
static int extractOpts(void);
int Xendwin(void);
void start_event_handler(void);
int XCursesTransformLine(int ,int ,chtype ,int ,int,int,char *);
#else    
static void get_GC();
static void place_text();
static void place_graphics();
static void TooSmall();
static void makeXY();
static int get_colors();
int Xinitscr();
static void mergeDatabases();
static char *getHomeDir();
static int extractOpts();
int Xendwin();
void start_event_handler(void);
int XCursesTransformLine();
#endif


int xerror();

void say(char *msg)
{
/*
 fprintf(stderr,"%s",msg);
*/
}

/*********************************************************************/
/*           Child process functions...                              */
/*********************************************************************/
void exit_child(char *msg)
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("exit_child() - called: %s\n",msg);
#endif
 endwin();
 shutdown(display_sock,2);
 close(display_sock);
 shutdown(key_sock,2);
 close(key_sock);
 _exit(1);
}
/***********************************************************************/
#ifdef PROTO
int XCurses_redraw_curscr(void)
#else
int XCurses_redraw_curscr()
#endif
/***********************************************************************/
{
 int i;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("XCurses_redraw_curscr() - called\n");
#endif
#ifdef PDCDEBUG
 say("child refreshing screen\n");
#endif
 for (i=0;i<curscr->_maxy;i++)
    XCurses_transform_line(curscr->_y[i],i,0,curscr->_maxx);
 return(OK);
}
/***********************************************************************/
#ifdef PROTO
int XCurses_display_cursor(int oldrow,int oldcol,chtype oldchar,int newrow,int newcol,chtype newchar)
#else
int XCurses_display_cursor(oldrow,oldcol,oldchar,newrow,newcol,newchar)
int oldrow,oldcol;
chtype oldchar;
int newrow,newcol;
chtype newchar;
#endif
/***********************************************************************/
{
 char buf[30];
 short fore,back;
 int idx,colour,new_attr,pos;
 int pair_num;
 chtype attr;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("XCurses_display_cursor() - called: OLD row %d col %d char %x NEW row %d col %d char %x\n",oldrow,oldcol,oldchar,newrow,newcol,newchar);
#endif

/* redisplay character at old cursor position */
 XCurses_transform_line(&oldchar,oldrow,oldcol,1);

 if (_cursvar.visibility == 0) /* cursor not displayed, exit */
    return(OK);

 idx = CURSES_CURSOR;
 memcpy(buf,(char *)&idx,sizeof(int));
 idx = sizeof(int);
 memcpy(buf+idx,(char *)&_cursvar.visibility,sizeof(int));
 idx += sizeof(int);

 if ((pair_num = PAIR_NUMBER(newchar)) != 0)
   {
    if (pair_content(pair_num,&fore,&back) == ERR)
       return(ERR);
   }
 else
   {
    fore = COLOR_WHITE;
    back = COLOR_BLACK;
   }
 colour = fore + (back << 8);
 pos = newrow + (newcol << 8);
 attr = (int)((newchar & A_ATTRIBUTES) >> 16);
 memcpy(buf+idx,(char *)&colour,sizeof(int));
 idx += sizeof(int);
 memcpy(buf+idx,(char *)&pos,sizeof(int));
 idx += sizeof(int);
 memcpy(buf+idx,(char *)&attr,sizeof(int));
 idx += sizeof(int);
 buf[idx++] = (char)newchar & A_CHARTEXT;

 if (write_socket(display_sock,buf,idx) < 0)
    exit_child("exitting from XCurses_transform_line\n");

 return(OK);
}
/***********************************************************************/
#ifdef PROTO
int XCurses_rawgetch(void)
#else
int XCurses_rawgetch()
#endif
/***********************************************************************/
{
 int key,s;
 char buf[10]; /* big enough for 1 key - plenty */

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("XCurses_rawgetch() - called\n");
#endif
 while(1)
   {
       if (read_socket(key_sock,buf,sizeof(int)) < 0)
          exit_child("exitting from XCurses_rawchar\n"); /* what else ?? */
       memcpy((char *)&key,buf,sizeof(int));
       if (key != CURSES_REFRESH) /* normal key */
          break;
    /* X requires refresh */
       XCurses_redraw_curscr();
   }

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("XCurses_rawgetch() - key %d returned\n",key);
#endif
 return(key);
}
/***********************************************************************/
#ifdef PROTO
bool XCurses_kbhit(void)
#else
bool XCurses_kbhit()
#endif
/***********************************************************************/
{
 int s;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("XCurses_kbhit() - called\n");
#endif
/*---------------------------------------------------------------------*/
/* Is something ready to be read on the socket ? Must be a key.        */
/*---------------------------------------------------------------------*/
 FD_ZERO( &readfds );
 FD_SET( key_sock, &readfds );
 if ( ( s = select ( FD_SETSIZE, &readfds, NULL, NULL, &timeout ) ) < 0 )
    exit_child("child - exiting from XCurses_kbhit select failed\n");
            
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("XCurses_kbhit() - returning %s\n",(s == 0) ? "FALSE" : "TRUE");
#endif
 if ( s == 0 )
    return(FALSE);
 return(TRUE);
}
/***********************************************************************/
#ifdef PROTO
int XCurses_instruct(int flag)
#else
int XCurses_instruct(flag)
int flag;
#endif
/***********************************************************************/
{
 char buf[10];

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("XCurses_instruct() - called\n");
#endif
/*---------------------------------------------------------------------*/
/* Send a request to Xwindows ...                                      */
/*---------------------------------------------------------------------*/
 memcpy(buf,(char *)&flag,sizeof(int));
 if (write_socket(display_sock,buf,sizeof(int)) < 0)
    exit_child("exitting from XCurses_instruct\n");
 return(OK);
}
/***********************************************************************/
#ifdef PROTO
int XCurses_wait_for_display(void)
#else
int XCurses_wait_for_display()
int flag;
#endif
/***********************************************************************/
{
 char buf[10];
 int result;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("XCurses_wait_for_display() - called\n");
#endif
/*---------------------------------------------------------------------*/
/* Tell Xwindows we want to wait...                                    */
/*---------------------------------------------------------------------*/
 XCurses_instruct(CURSES_CONTINUE);
/*---------------------------------------------------------------------*/
/* ... wait for Xwindows to say the display has occurred.              */
/*---------------------------------------------------------------------*/
 if (read_socket(display_sock,buf,sizeof(int)) < 0)
    exit_child("exitting from XCurses_transform_line\n");
 memcpy((char *)&result,buf,sizeof(int));
 if (result != CURSES_CONTINUE)
   exit_child("exitting from XCurses_transform_line - synchronization error\n");
 return(OK);
}
/***********************************************************************/
#ifdef PROTO
int XCurses_transform_line(long *ch, int row, int x, int num_cols)
#else
int XCurses_transform_line(ch, row, x, num_cols)
long *ch;
int row,x,num_cols;
#endif
/***********************************************************************/
{
 char buf[300];
 char text[300];
 bool new_packet=FALSE;
 short fore,back;
 int idx,colour,new_attr,pos;
 int original_x,pair_num,i,j,result;
 chtype old_attr,save_ch,attr;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("XCurses_transform_line() - called: %s Row: %d X: %d NumCols: %d\n",(ch == NULL) ? "SHUTDOWN" : "", row,x,num_cols);
#endif
 if (num_cols == 0)
    return(OK);
#ifdef PDCDEBUG
 if (trace_on)
   {
    for (i=0;i<num_cols;i++)
       text[i] = *(ch+i) & A_CHARTEXT;
    text[i] = '\0';
    PDC_debug("XCurses_transform_line() - row: %d col: %d num_cols: %d text:<%s>\n",row,x,num_cols,text);
   }
#endif

 old_attr = *ch & A_ATTRIBUTES;
 original_x = x;
 for (i=0,j=0; j<num_cols; x++,j++)
   {
    attr = *(ch+j) & A_ATTRIBUTES;
    if (attr != old_attr)
       new_packet = TRUE;
    if (new_packet)
      {
       if ((pair_num = PAIR_NUMBER(save_ch)) != 0)
         {
          if (pair_content(pair_num,&fore,&back) == ERR)
             return(ERR);
         }
       else
         {
          fore = COLOR_WHITE;
          back = COLOR_BLACK;
         }
       text[i] = '\0';
       colour = fore + (back << 8);
       pos = row + (original_x << 8);
       new_attr = (int)(old_attr >> 16);

       memcpy(buf,(char *)&i,sizeof(int));
       idx = sizeof(int);
       memcpy(buf+idx,(char *)&colour,sizeof(int));
       idx += sizeof(int);
       memcpy(buf+idx,(char *)&pos,sizeof(int));
       idx += sizeof(int);
       memcpy(buf+idx,(char *)&new_attr,sizeof(int));
       idx += sizeof(int);
       memcpy(buf+idx,text,i);
       idx += i;
       if (write_socket(display_sock,buf,idx) < 0)
          exit_child("exitting from XCurses_transform_line\n");

       new_packet = FALSE;
       old_attr = attr;
       original_x = x;
       i = 0;
      }
    text[i++] = *(ch+j) & A_CHARTEXT;
    save_ch = *(ch+j);
   }
 if ((pair_num = PAIR_NUMBER(save_ch)) != 0)
   {
    if (pair_content(pair_num,&fore,&back) == ERR)
       return(ERR);
   }
 else
   {
    fore = COLOR_WHITE;
    back = COLOR_BLACK;
   }
 text[i] = '\0';

 colour = fore + (back << 8);
 pos = row + (original_x << 8);
 new_attr = (int)(old_attr >> 16);

 memcpy(buf,(char *)&i,sizeof(int));
 idx = sizeof(int);
 memcpy(buf+idx,(char *)&colour,sizeof(int));
 idx += sizeof(int);
 memcpy(buf+idx,(char *)&pos,sizeof(int));
 idx += sizeof(int);
 memcpy(buf+idx,(char *)&new_attr,sizeof(int));
 idx += sizeof(int);
 memcpy(buf+idx,text,i);
 idx += i;
 if (write_socket(display_sock,buf,idx) < 0)
    exit_child("exitting from XCurses_transform_line\n");

 return(OK);
}
/*********************************************************************/
/*          Parent process functions...                              */
/*********************************************************************/
void exit_parent(char *msg)
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("exit_parent() - called: %s\n",msg);
#endif
 Xendwin();
 shutdown(display_sock,2);
 close(display_sock);
 shutdown(key_sock,2);
 close(key_sock);
 _exit(1);
}
/***********************************************************************/
#ifdef PROTO
static void get_GC(Window win, GC *gc, XFontStruct *font_info, int fore, int back)
#else
static void get_GC(win, gc, font_info,fore,back)
Window win;
GC *gc;
XFontStruct *font_info;
int fore,back;
#endif
/***********************************************************************/
{
 XGCValues values;

        /* Create default Graphics Context */
 *gc = XCreateGC(display, win, 0L, &values);

        /* specify font */
 XSetFont(display, *gc, font_info->fid);

        /* specify black foreground since default may be white on white */
 XSetForeground(display, *gc, colors[fore]);
 XSetBackground(display, *gc, colors[back]);

}
/***********************************************************************/
#ifdef PROTO
static void makeXY(int x,int y,int fontwidth,int fontheight,int *xpos,int *ypos)
#else
static void makeXY(x,y,fontwidth,fontheight,xpos,ypos)
int x,y,fontwidth,fontheight,*xpos,*ypos;
#endif
/***********************************************************************/
{   
 *xpos = x * fontwidth;
 *ypos = XCursesFontInfo->ascent + (y * fontheight);
}

/***********************************************************************/
#ifdef PROTO
static int get_colors(void)
#else
static int get_colors()
#endif
/***********************************************************************/
{
#ifdef PDCDEBUG
 static char *visual_class[] = 
   {
    "StaticGray",
    "GrayScale",
    "StaticColor",
    "PseudoColor",
    "TrueColor",
    "DirectColor"
   };
 int ncolors = 0;
#endif

 int default_depth;
 Visual *default_visual;
 XColor exact_def;
 Colormap default_cmap;
 int i = 5;
 XVisualInfo visual_info;
        
        /* Try to allocate colors for PseudoColor, TrueColor, 
         * DirectColor, and StaticColor.  Use black and white
         * for StaticGray and GrayScale */
#ifdef PDCDEBUG
 say("in get_colors\n");
#endif

 default_depth = DefaultDepth(display, screen_num);
 default_visual = DefaultVisual(display, screen_num);
 default_cmap   = DefaultColormap(display, screen_num);
 if (default_depth == 1) 
   {
                /* must be StaticGray, use black and white */
    border_pixel = BlackPixel(display, screen_num);
    background_pixel = WhitePixel(display, screen_num);
    foreground_pixel = BlackPixel(display, screen_num);
    return(OK);
   }

 while (!XMatchVisualInfo(display, screen_num, default_depth, i--, &visual_info))
   ;
#if 0
 fprintf(stderr,"found a %s class visual at default_depth.\n", visual_class[++i]);
#endif
        
 if (i < 2) 
   {
                /* No color visual available at default_depth.
                 * Some applications might call XMatchVisualInfo
                 * here to try for a GrayScale visual 
                 * if they can use gray to advantage, before 
                 * giving up and using black and white.
                 */
    border_pixel = BlackPixel(display, screen_num);
    background_pixel = WhitePixel(display, screen_num);
    foreground_pixel = BlackPixel(display, screen_num);
    return(OK);
   }

        /* otherwise, got a color visual at default_depth */

        /* The visual we found is not necessarily the 
         * default visual, and therefore it is not necessarily
         * the one we used to create our window.  However,
         * we now know for sure that color is supported, so the
         * following code will work (or fail in a controlled way).
         * Let's check just out of curiosity: */
#if 0
 if (visual_info.visual != default_visual)
    fprintf(stderr,"PseudoColor visual at default depth is not default visual!\nContinuing anyway...\n");
#endif

 for (i=0;i<MAX_COLORS;i++) 
   {
#if 0
    fprintf(stderr,"allocating %s\n", color_names[i]);
#endif
    if (!XParseColor (display, default_cmap, color_names[i], &exact_def))
      {
       fprintf(stderr, "color name %s not in database",  color_names[i]);
       return(ERR);
      }
#if 0
    fprintf(stderr,"The RGB values from the database are %d, %d, %d\n", exact_def.red, exact_def.green, exact_def.blue);
#endif
    if (!XAllocColor(display, default_cmap, &exact_def)) 
      {
       fprintf(stderr, "can't allocate color: all colorcells allocated and no matching cell found.\n");
       return(ERR);
      }
#if 0
    fprintf(stderr,"The RGB values actually allocated are %d, %d, %d\n", exact_def.red, exact_def.green, exact_def.blue);
    ncolors++;
#endif
    colors[i] = exact_def.pixel;
   }
#if 0
 fprintf(stderr,"allocated %d read-only color cells\n", ncolors);
#endif

 border_pixel = colors[COLOR_BLACK];
 background_pixel = colors[COLOR_BLACK];
 foreground_pixel = colors[COLOR_WHITE];
 return(OK);
}
xerror(d, ev)
Display *d;
XErrorEvent *ev;
{
 return(1);
}
xioerror(dpy)
Display *dpy;
{
 return(1);
}
/***********************************************************************/
#ifdef PROTO
static void mergeDatabases(void)
#else
static void mergeDatabases()
#endif
/***********************************************************************/
{
#ifdef SYSVR4
 struct utsname hostid;
#endif
 XrmDatabase homeDB, serverDB, applicationDB;
 char filenamebuf[1024];
 char *filename = &filenamebuf[0];
 char *environment;
 char *classname = "XCurses";
 char name[255];
/*---------------------------------------------------------------------*/
/* Get application default values first...                             */
/*---------------------------------------------------------------------*/
 (void)strcpy(name, "/usr/lib/X11/app-defaults/");
 (void)strcat(name, classname);
#if 0
 fprintf(stderr,"getting resources from %s\n",name);
#endif
 applicationDB = XrmGetFileDatabase(name);
 (void)XrmMergeDatabases(applicationDB, &rDB);
/*---------------------------------------------------------------------*/
/* Get server default values...                                        */
/*---------------------------------------------------------------------*/
 if (XResourceManagerString(display) != NULL) 
    serverDB = XrmGetStringDatabase(XResourceManagerString(display));
 else
/*---------------------------------------------------------------------*/
/* ... or application defaults from home directory...                  */
/*---------------------------------------------------------------------*/
   {
    (void)getHomeDir(filename);
    (void) strcat(filename, "/.Xdefaults");
#if 0
    fprintf(stderr,"getting resources from %s\n",name);
#endif
    serverDB = XrmGetFileDatabase(filename);
   }
 XrmMergeDatabases(serverDB, &rDB);
/*---------------------------------------------------------------------*/
/* Get defaults specified for XENVIRONMENT...                          */
/*---------------------------------------------------------------------*/
 if ((environment = (char *)getenv("XENVIRONMENT")) == NULL)
   {
    int len;
    environment = getHomeDir(filename);
    (void)strcat(environment, "/.Xdefaults-");
    len = strlen(environment);
#ifdef SYSVR4
    (void)uname(&hostid);
    strcat(environment + len, hostid.nodename);
#else
    (void)gethostname(environment + len, 1024 - len);
#endif
   }
#if 0
 fprintf(stderr,"getting resources from %s\n",environment);
#endif
 homeDB = XrmGetFileDatabase(environment);
 XrmMergeDatabases(homeDB, &rDB);
/*---------------------------------------------------------------------*/
/* Last, command line options take precedence over all others.         */
/* Not implemented, yet...                                             */
/*---------------------------------------------------------------------*/
 /* XrmMergeDatabases(commandlineDB, &rDB); */
}
/***********************************************************************/
#ifdef PROTO
static char *getHomeDir(char *dest)
#else
static char *getHomeDir(dest)
char *dest;
#endif
/***********************************************************************/
{
 int uid;
 struct passwd *pw;
 register char *ptr;

 if ((ptr = (char *)getenv("HOME")) != NULL)
    (void)strcpy(dest, ptr);
 else
   {
    if ((ptr = (char *)getenv("USER")) != NULL)
       pw = getpwnam(ptr);
    else
      {
       uid = getuid();
       pw = getpwuid(uid);
      }
    if (pw)
       (void) strcpy(dest, pw->pw_dir);
    else
       *dest = '\0';
   }
 return dest;
}
/***********************************************************************/
#ifdef PROTO
static int extractOpts(void)
#else
static int extractOpts()
#endif
/***********************************************************************/
{
 extern char *XCursesProgramName;
 char resource_buffer[100];
 char class_buffer[50];
 char *str_type[20];
 char buffer[20];
 XrmValue value;
 int i;
/*---------------------------------------------------------------------*/
/* Get the specific resources out of the merged databases that we want */
/*---------------------------------------------------------------------*/
 sprintf(resource_buffer,"Xcurses.%s.lines",XCursesProgramName);
 if (XrmGetResource(rDB,resource_buffer,"Xcurses*Lines",str_type,&value) == True)
   {
    (void)strncpy(buffer, value.addr,(int)value.size);
    buffer[value.size] = '\0';
    XCursesLINES = atoi(buffer);
   }

 sprintf(resource_buffer,"Xcurses.%s.cols",XCursesProgramName);
 if (XrmGetResource(rDB,resource_buffer,"Xcurses*Cols",str_type,&value) == True)
   {
    (void)strncpy(buffer, value.addr,(int)value.size);
    buffer[value.size] = '\0';
    XCursesCOLS = atoi(buffer);
   }

 sprintf(resource_buffer,"Xcurses.%s.font",XCursesProgramName);
 if (XrmGetResource(rDB,resource_buffer,"Xcurses*Font",str_type,&value) == True)
   {
    (void)strncpy(XCursesFontName,value.addr,(int)value.size);
    *(XCursesFontName+(int)value.size) = '\0';
   }
 else
    (void)strcpy(XCursesFontName,DefaultFont);

 sprintf(resource_buffer,"Xcurses.%s.boldfont",XCursesProgramName);
 if (XrmGetResource(rDB,resource_buffer,"Xcurses*BoldFont",str_type,&value) == True)
   {
    (void)strncpy(XCursesBoldFontName,value.addr,(int)value.size);
    *(XCursesBoldFontName+(int)value.size) = '\0';
   }
 else
    (void)strcpy(XCursesBoldFontName,DefaultBoldFont);

 sprintf(resource_buffer,"Xcurses.%s.icon",XCursesProgramName);
 if (XrmGetResource(rDB,resource_buffer,"Xcurses*Icon",str_type,&value) == True)
   {
    if ((bitmap_file = (char *)malloc(value.size+1)) == NULL)
      {
       fprintf(stderr,"Unable to allocate %d bytes for bitmap file\n",value.size+1);
       return(ERR);
      }
    (void)strncpy(bitmap_file,value.addr,(int)value.size);
    *(bitmap_file+(int)value.size) = '\0';
   }

 for (i=0;i<MAX_COLORS;i++)
   {
    sprintf(resource_buffer,"Xcurses.%s.color%d",XCursesProgramName,i);
    sprintf(class_buffer,"Xcurses*Color%d",i);
    if (XrmGetResource(rDB,resource_buffer,class_buffer,str_type,&value) == True)
      {
       if ((color_names[i] = (char *)malloc(value.size+1)) == NULL)
         {
          fprintf(stderr,"Unable to allocate %d bytes for color%d\n",value.size+1,i);
          return(ERR);
         }
       (void)strncpy(color_names[i],value.addr,(int)value.size);
       *(color_names[i]+(int)value.size) = '\0';
      }
    else
      {
       if ((color_names[i] = (char *)malloc(strlen(default_color_names[i])+1)) == NULL)
         {
          fprintf(stderr,"Unable to allocate %d bytes for color%d\n",strlen(default_color_names[i])+1,i);
          return(ERR);
         }
       (void)strcpy(color_names[i],default_color_names[i]);
      }
   }
 return(OK);
}
/***********************************************************************/
#ifdef PROTO
int Xendwin(void)
#else
int Xendwin()
#endif
/***********************************************************************/
{
 int i;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("Xendwin() - called\n");
#endif
 if (bitmap_file != NULL)
   {
    XFreePixmap(display,icon_pixmap);
    free(bitmap_file);
   }
 for (i=0;i<MAX_COLORS;i++)
    free(color_names[i]);
 XUnloadFont(display, XCursesFontInfo->fid);
 XUnloadFont(display, XCursesBoldFontInfo->fid);
 XFreeGC(display, normal_gc);
 XFreeGC(display, bold_gc);
 XFreeGC(display, cursor_gc);
 XCloseDisplay(display);
 return(0);
}
/***********************************************************************/
#ifdef PROTO
void start_event_handler(void)
#else
void start_event_handler()
#endif
/***********************************************************************/
{
 XEvent event;
 XKeyEvent key_event;
 KeySym keysym;
 XComposeStatus compose;
 int count,key;
 char buffer[120];
 int buflen=40;
 char *ptr;
 int i;
#ifdef MAIN
 int stdscr;
#endif

/*---------------------------------------------------------------------*/
/* Process trapped events and return with KEY pressed (or mouse)...    */
/*---------------------------------------------------------------------*/
#ifdef PDCDEBUG
 say("start of start_event_handler\n");
#endif
 while (1)  
   {
    if (XPending(display) > 0)
      {
       XNextEvent(display, &event);
       switch (event.type)
         {
          case Expose:
#ifdef PDCDEBUG
               say("Expose\n");
#endif
            /* get all other Expose events on the queue */
               while (XCheckTypedEvent(display, Expose, &event));

/*
 * should send refresh request to curses
 */
               if (after_first_curses_request
               &&  after_first_expose_event)
                  XCursesSendKeyToCurses(CURSES_REFRESH);
               after_first_expose_event = True;
               break;
          case ConfigureNotify:
#ifdef PDCDEBUG
               say("ConfigureNotify\n");
#endif
            /* window has been resized, change width and
             * height to send to place_text and place_graphics
             * in next Expose */
               XCursesWindowWidth = event.xconfigure.width;
               XCursesWindowHeight = event.xconfigure.height;
               break;
          case ButtonPress:
#ifdef PDCDEBUG
               say("ButtonPress\n");
#endif
#ifdef MAIN
               (void)display_button();
#else
               XCursesSendKeyToCurses(KEY_MOUSE);
#endif
               break;
          case KeyPress:
               buffer[0] = '\0';
               count = XLookupString((XKeyEvent *)&event,buffer,buflen,&keysym, &compose);
            /* translate keysym into curses key code */
               key = 0;
               if (keysym >= XK_F1 && keysym <= XK_F35)
                  key = KEY_F(keysym - XK_F1 + 1);
               if (buffer[0] != 0
               && count == 1)
                  key = (int)buffer[0];
               if (key == 0)
                 {
                  ptr = XKeysymToString(keysym);
                  for (i=0;XCursesKeys[i].keycode != 0;i++)
                    {
                     if (strcmp(ptr,XCursesKeys[i].xname) == 0)
                       {
                        key = XCursesKeys[i].keycode;
                        break;
                       }
                    }
                 }
#if 0
               fprintf(stderr,"Key: %s pressed - %x\n",XKeysymToString(keysym),key);
#endif
               XCursesSendKeyToCurses(key);
               break;
          default:
            /* all events selected by StructureNotifyMask
             * except ConfigureNotify are thrown away here,
             * since nothing is done with them */
               fprintf(stderr,"Event not handled: %d\n",event.type);
               break;
         } /* end switch */
      }
    else
       XCursesProcessRequestsFromCurses();
   } /* end while */
#ifdef PDCDEBUG
 say("end of start_event_handler\n");
#endif
}
/***********************************************************************/
#ifdef PROTO
int XCursesSendKeyToCurses(int key)
#else
int XCursesSendKeyToCurses(key)
int key;
#endif
/***********************************************************************/
{
 char buf[10];

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("XCursesSendKeyToCurses() - called: sending %d\n",key);
#endif
 memcpy(buf,(char *)&key,sizeof(int));
 if (write_socket(key_sock,buf,sizeof(int)) < 0)
    exit_parent("exiting from XCursesSendKeyToCurses\n");
 return(0);
}
/***********************************************************************/
#ifdef PROTO
int XCursesProcessRequestsFromCurses(void)
#else
int XCursesProcessRequestsFromCurses()
#endif
/***********************************************************************/
{
 int s,idx,visibility;
 int fore,back,row,x;
 char buf[300]; /* big enough for 1 line of the screen */
 int pos,attr,colour,num_cols;
 chtype new_attr;

 FD_ZERO ( &readfds );
 FD_SET ( display_sock, &readfds );

 if ( ( s = select ( FD_SETSIZE, &readfds, NULL, NULL, &timeout ) ) < 0 )
    exit_parent("exiting from XCursesProcessRequestsFromCurses - select failed\n");
            
 if ( s == 0 )
    return(0);
            
 if ( FD_ISSET ( display_sock, &readfds ) )
   {
/* read first integer to determine total message has been received */
    if (read_socket(display_sock,buf,sizeof(int)) < 0)
       exit_parent("exitting from XCursesProcessRequestsFromCurses - first read\n");
    memcpy((char *)&num_cols,buf,sizeof(int));
    after_first_curses_request = True;
    switch(num_cols)
      {
       case CURSES_EXIT: /* request from curses to stop */
            exit_parent("parent requested to exit by child\n");
            break;
       case CURSES_BELL: /* request from curses to beep */
            XBell(display,50);
            break;
       case CURSES_CLEAR: /* request from curses to clear window */
            XClearWindow(display,win);
            break;
       case CURSES_FLASH: /* request from curses to beep */
            fprintf(stderr,"flash the screen\n");
            XBell(display,100);
            break;
       case CURSES_CONTINUE: /* request from curses to confirm completion of display */
            x = CURSES_CONTINUE;
            memcpy(buf,(char *)&x,sizeof(int));
            if (write_socket(display_sock,buf,sizeof(int)) < 0)
               exit_parent("exitting from XCursesProcessRequestsFromCurses\n");
            break;
       case CURSES_CURSOR: /* display cursor */
            if (read_socket(display_sock,buf,(sizeof(int)*4)+1) < 0)
               exit_parent("exitting from CURSES_CURSOR XCursesProcessRequestsFromCurses\n");
            memcpy((char *)&visibility,buf,sizeof(int));
            idx = sizeof(int);
            memcpy((char *)&colour,buf,sizeof(int));
            idx += sizeof(int);
            memcpy((char *)&pos,buf+idx,sizeof(int));
            idx += sizeof(int);
            memcpy((char *)&attr,buf+idx,sizeof(int));
            idx += sizeof(int);

            fore = colour & 0xFF;
            back = colour >> 8;
            row = pos & 0xFF;
            x = pos >> 8;
            new_attr = (chtype)attr << 16;
            if (visibility == 1)
              {
               XDrawRectangle(display,win,cursor_gc,(x*XCursesFontWidth)+1,(row*XCursesFontHeight)+1,XCursesFontWidth-2,XCursesFontHeight-2);
              }
            else
              {
               XCursesTransformLine(back,cursor_colour,new_attr,row,x,1,buf+idx);
              }
            break;
       default:
            if (read_socket(display_sock,buf,(sizeof(int)*3)+num_cols) < 0)
               exit_parent("exitting from XCursesProcessRequestsFromCurses\n");

            memcpy((char *)&colour,buf,sizeof(int));
            idx = sizeof(int);

            memcpy((char *)&pos,buf+idx,sizeof(int));
            idx += sizeof(int);

            memcpy((char *)&attr,buf+idx,sizeof(int));
            idx += sizeof(int);

            fore = colour & 0xFF;
            back = colour >> 8;
            row = pos & 0xFF;
            x = pos >> 8;
            new_attr = (chtype)attr << 16;
            buf[num_cols+idx] = '\0';
            XCursesTransformLine(fore,back,new_attr,row,x,num_cols,buf+idx);
            break;
      }
   }
 return(0);
}
/***********************************************************************/
#ifdef PROTO
int XCursesTransformLine(int fore,int back,chtype attr,int row,int x, int num_cols,char *text)
#else
int XCursesTransformLine(fore,back,attr,row,x,num_cols,text)
int fore,back;
chtype attr;
int row,x,num_cols;
char *text;
#endif
/***********************************************************************/
{
 int xpos,ypos;
 GC gc;

 if (attr & A_BOLD)
    gc = bold_gc;
 else
    gc = normal_gc;

 if (attr & A_REVERSE)
   {
    XSetForeground(display, gc, colors[COLOR_BLACK]);
    XSetBackground(display, gc, colors[COLOR_WHITE]);
   }
 else
   {
    XSetForeground(display, gc, colors[fore]);
    XSetBackground(display, gc, colors[back]);
   }

 makeXY(x,row,XCursesFontWidth,XCursesFontHeight,&xpos,&ypos);
#if 0
 fprintf(stderr,"ROW: %d COL: %d FORE: %d BACK: %d ATTR: %x\n",row,x,fore,back,attr);
 fprintf(stderr,"     TEXT: <%s>\n",text);
#endif
 XDrawImageString(display,win,gc,xpos,ypos,text,num_cols);
 return(OK);
}
/*********************************************************************/
/*          Shared process functions...                              */
/*********************************************************************/
/***********************************************************************/
#ifdef PROTO
int write_socket(int sock_num,char *buf,int len)
#else
int write_socket(sock_num,buf,len)
int sock_num;
char *buf;
int len;
#endif
/***********************************************************************/
{
 int start=0,length=len,rc;
 while(1)
   {
    rc = write(sock_num,buf+start,length);
    if (rc < 0
    ||  rc == length)
       return(rc);
    length -= rc;
    start = rc;
#ifdef PDCDEBUG
    say("having to write again\n");
#endif
   }
 return(len);
}
/***********************************************************************/
#ifdef PROTO
int read_socket(int sock_num,char *buf,int len)
#else
int read_socket(sock_num,buf,len)
int sock_num;
char *buf;
int len;
#endif
/***********************************************************************/
{
 int start=0,length=len,rc;
 while(1)
   {
    rc = read(sock_num,buf+start,length);
    if (rc < 0
    ||  rc == length)
       return(rc);
    length -= rc;
    start = rc;
#ifdef PDCDEBUG
    say("having to read again\n");
#endif
   }
 return(len);
}
/***********************************************************************/
#ifdef PROTO
int Xinitscr(void)
#else
int Xinitscr()
#endif
/***********************************************************************/
{
 XClassHint classhints;
 XSizeHints sizehints;
 XSizeHints iconsizehints;
 XWMHints wmhints;
 XTextProperty windowName, iconName;
 XSetWindowAttributes attributes;
 int pid,x_hot,y_hot,rc;
 int fileflags;
 char wait_buf[5];
 int wait_value;

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("Xinitscr() - called\n");
#endif
 memset ( &timeout, '\0', sizeof ( timeout ) );
            
 if ( socketpair ( AF_UNIX, SOCK_STREAM, 0, display_sockets ) < 0 )
   {
    fprintf(stderr, "ERROR: cannot create display socketpair\n");
    return(ERR);
    }
            
 if ( socketpair ( AF_UNIX, SOCK_STREAM, 0, key_sockets ) < 0 )
   {
    fprintf(stderr, "ERROR: cannot create key socketpair\n");
    return(ERR);
    }
 pid = fork();
 switch(pid)
   {
   case (-1):
         fprintf(stderr,"ERROR: cannot fork()\n");
         return(ERR);
         break;
    case 0: /* child */
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("Xinitscr() - child started\n");
         say ("child waiting...\n");
#endif
         close ( display_sockets[1] );
         close ( key_sockets[1] );
         display_sock = display_sockets[0];
         key_sock = key_sockets[0];
         FD_ZERO ( &readfds );
         FD_ZERO ( &writefds );
         read_socket(display_sock,wait_buf,sizeof(int));
         memcpy((char *)&wait_value,wait_buf,sizeof(int));
         if (wait_value != CURSES_CHILD)
            return(ERR);
         read_socket(display_sock,wait_buf,sizeof(int));
         memcpy((char *)&XCursesLINES,wait_buf,sizeof(int));
         read_socket(display_sock,wait_buf,sizeof(int));
         memcpy((char *)&XCursesCOLS,wait_buf,sizeof(int));
         say ("child started\n");
         return(OK);
         break;
    default: /* parent */
         break;
   }

/*
 (void)XSetErrorHandler(xerror);
 (void)XSetIOErrorHandler(xioerror);
*/
/*---------------------------------------------------------------------*/
/* Initialise Resource Manager...                                      */
/*---------------------------------------------------------------------*/
#ifdef PDCDEBUG
  say("before XrmInitialize\n");
#endif
 XrmInitialize();
/*---------------------------------------------------------------------*/
/* Connect to the X server...                                          */
/*---------------------------------------------------------------------*/
#ifdef PDCDEBUG
  say("before XOpenDisplay\n");
#endif
 if ((display=XOpenDisplay(display_name)) == NULL)
    return(ERR);
 XSynchronize(display,1);
/*---------------------------------------------------------------------*/
/* Get screen dimensions for server...                                 */
/*---------------------------------------------------------------------*/
 screen_num = DefaultScreen(display);
 screen_ptr = DefaultScreenOfDisplay(display);
/*---------------------------------------------------------------------*/
/* Get application defaults...                                         */
/*---------------------------------------------------------------------*/
#ifdef PDCDEBUG
  say("before mergeDatabases\n");
#endif
 (void)mergeDatabases();
/*---------------------------------------------------------------------*/
/* Extract required options...                                         */
/*---------------------------------------------------------------------*/
#ifdef PDCDEBUG
  say("before extractOpts\n");
#endif
 if (extractOpts() == ERR)
    return(ERR);
/*---------------------------------------------------------------------*/
/* Load the supplied or default font...                                */
/*---------------------------------------------------------------------*/
#ifdef PDCDEBUG
  say("before XLoadQueryFont\n");
#endif

#if 0
  fprintf(stderr, "name %s COLS %d LINES %d wwidth %d wheight %d fwidth %d fheight %d\n",
                    XCursesFontName, XCursesCOLS,XCursesLINES, XCursesWindowWidth, XCursesWindowHeight,
                    XCursesFontWidth, XCursesFontHeight);
#endif
 if ((XCursesFontInfo = XLoadQueryFont(display,XCursesFontName)) == NULL)
   {
    fprintf(stderr,"Error loading font %s\n",XCursesFontName);
    return(ERR);
   }
 XCursesFontWidth = XCursesFontInfo->max_bounds.rbearing - XCursesFontInfo->min_bounds.lbearing;
 XCursesFontHeight = XCursesFontInfo->max_bounds.ascent + XCursesFontInfo->max_bounds.descent;
/*---------------------------------------------------------------------*/
/* Load the bold font...                                               */
/*---------------------------------------------------------------------*/
 if ((XCursesBoldFontInfo = XLoadQueryFont(display,XCursesBoldFontName)) == NULL)
   {
    fprintf(stderr,"Error loading bold font %s\n",XCursesBoldFontName);
    return(ERR);
   }
/*---------------------------------------------------------------------*/
/* Check that the bold font and normal fonts are the same size...      */
/*---------------------------------------------------------------------*/
 if (XCursesFontWidth != XCursesBoldFontInfo->max_bounds.rbearing - XCursesBoldFontInfo->min_bounds.lbearing
 ||  XCursesFontHeight != XCursesBoldFontInfo->max_bounds.ascent + XCursesBoldFontInfo->max_bounds.descent)
   {
    fprintf(stderr,"Error: normal font (%s) and bold font (%s) different sizes\n",XCursesFontName,XCursesBoldFontName);
    return(ERR);
   }
/*---------------------------------------------------------------------*/
/* Calculate size of display window...                                 */
/*---------------------------------------------------------------------*/
 XCursesWindowWidth = XCursesFontWidth * XCursesCOLS;
 XCursesWindowHeight = XCursesFontHeight * XCursesLINES;
#if 0
  fprintf(stderr, "COLS %d LINES %d wwidth %d wheight %d fwidth %d fheight %d\n",
                    XCursesCOLS,XCursesLINES, XCursesWindowWidth, XCursesWindowHeight,
                    XCursesFontWidth, XCursesFontHeight);
  say("before get_colors\n");
#endif
/*---------------------------------------------------------------------*/
/* Process the supplied colors...                                      */
/*---------------------------------------------------------------------*/
 if (get_colors() == ERR)
    return(ERR);
/*---------------------------------------------------------------------*/
/* Define the cursor...                                                */
/*---------------------------------------------------------------------*/
 XCursesCursor = XCreateFontCursor(display, XC_xterm);
/*---------------------------------------------------------------------*/
/* Create the default X window for the curses display...               */
/*---------------------------------------------------------------------*/
#ifdef PDCDEBUG
  say("before XcreateSimpleWindow\n");
#endif

 win = XCreateSimpleWindow(display, RootWindow(display,screen_num), x, y, 
                        XCursesWindowWidth, XCursesWindowHeight, borderwidth, border_pixel,
                        colors[COLOR_BLACK]);
/*---------------------------------------------------------------------*/
/* Generate the pixmap for the window...                               */
/*---------------------------------------------------------------------*/
#ifdef PDCDEBUG
  say("before XcreateBitMapFromData\n");
#endif
 icon_bitmap_width = ICON_WIDTH;
 icon_bitmap_height = ICON_HEIGHT;
 if (bitmap_file == NULL)
    icon_pixmap = XCreateBitmapFromData(display, win, icon_bitmap_bits, 
                        icon_bitmap_width, icon_bitmap_height);
 else
   {
    icon_bitmap_width = 64;
    icon_bitmap_height = 64;
    rc = XReadBitmapFile(display,RootWindow(display,screen_num),
         bitmap_file,&icon_bitmap_width,&icon_bitmap_height,&icon_pixmap,
         &x_hot,&y_hot);
    if (rc != BitmapSuccess)
      {
       fprintf(stderr,"bitmap file %s invalid - using default\n",bitmap_file);
       icon_bitmap_width = ICON_WIDTH;
       icon_bitmap_height = ICON_HEIGHT;
       icon_pixmap = XCreateBitmapFromData(display, win, icon_bitmap_bits, 
                        icon_bitmap_width, icon_bitmap_height);
      }
   }
/*---------------------------------------------------------------------*/
/* Assign the cursor to the window.                                    */
/*---------------------------------------------------------------------*/
 XDefineCursor(display,win,XCursesCursor);
/*---------------------------------------------------------------------*/
/* Set various hints...                                                */
/*---------------------------------------------------------------------*/
 wmhints.flags = InputHint | StateHint | IconPixmapHint;
 wmhints.input = True;
 wmhints.initial_state = NormalState;
 wmhints.icon_pixmap = icon_pixmap;
 wmhints.icon_window = 0;
 wmhints.icon_x = 150;
 wmhints.icon_y = 0;
 wmhints.icon_mask = 0;

 sizehints.flags = PMinSize | PPosition | PSize | PResizeInc | PBaseSize;
 sizehints.x = 100;
 sizehints.y = 100;
 sizehints.width = XCursesWindowWidth;
 sizehints.height = XCursesWindowHeight;
 sizehints.min_width = XCursesMinWindowWidth = 350;
 sizehints.min_height = XCursesMinWindowHeight = 250;
 sizehints.width_inc = XCursesFontWidth;
 sizehints.height_inc = XCursesFontHeight;
 sizehints.base_width = XCursesWindowWidth;
 sizehints.base_height = XCursesWindowHeight;

 classhints.res_name = XCursesProgramName;
 classhints.res_class = "Xcurses";

 iconsizehints.flags = PMinSize | PMaxSize | PPosition | PSize;
 iconsizehints.x = 150;
 iconsizehints.y = 2;
 iconsizehints.width = icon_bitmap_width;
 iconsizehints.height = icon_bitmap_height;
 iconsizehints.min_width = icon_bitmap_width;
 iconsizehints.max_height = icon_bitmap_height;
 iconsizehints.width_inc = 0;
 iconsizehints.height_inc = 0;
/*---------------------------------------------------------------------*/
/* Prepare for setting properties for the window and icon...           */
/*---------------------------------------------------------------------*/
#ifdef PDCDEBUG
  say("before XStringListToTextProperty\n");
#endif
 if (XStringListToTextProperty(&XCursesProgramName, 1, &windowName) == 0)
   {
    (void) fprintf( stderr, "structure allocation for windowName failed.\n");
    return(ERR);
   }
 if (XStringListToTextProperty(&XCursesProgramName, 1, &iconName) == 0)
   {
    (void) fprintf( stderr, "structure allocation for iconName failed.\n");
    return(ERR);
   }
/*---------------------------------------------------------------------*/
/* Set properties for window, icon and class...                        */
/*---------------------------------------------------------------------*/
#ifdef PDCDEBUG
  say("before XSetWMProperties\n");
#endif
 XSetWMProperties(display,win,&windowName,&iconName,NULL,0,&sizehints,
                  &wmhints,&classhints);
/*---------------------------------------------------------------------*/
/* Select the types of events we want to capture...                    */
/*---------------------------------------------------------------------*/
#ifdef PDCDEBUG
  say("before XSelectInput\n");
#endif
 XSelectInput(display, win, ExposureMask | KeyPressMask | 
              ButtonPressMask | StructureNotifyMask);
/*---------------------------------------------------------------------*/
/* Create the Graphics Context for drawing...                          */
/*---------------------------------------------------------------------*/
#ifdef PDCDEBUG
  say("before get_GC\n");
#endif
 get_GC(win,&normal_gc,XCursesFontInfo,COLOR_WHITE,COLOR_BLACK);
 get_GC(win,&bold_gc,XCursesBoldFontInfo,COLOR_WHITE,COLOR_BLACK);
 get_GC(win,&cursor_gc,XCursesFontInfo,cursor_colour,COLOR_BLACK);
 XSetLineAttributes(display,cursor_gc,2,LineSolid,CapButt,JoinMiter);
#ifdef PDCDEBUG
  say("before XMapWindow\n");
#endif
  XMapWindow(display, win);

#ifdef PDCDEBUG
	if (trace_on) PDC_debug("Xinitscr() - parent started\n");
#endif
 close ( display_sockets[0] );
 display_sock = display_sockets[1];
 close ( key_sockets[0] );
 key_sock = key_sockets[1];

 wait_value = CURSES_CHILD;
 memcpy(wait_buf,(char *)&wait_value,sizeof(int));
 (void)write_socket(display_sock,wait_buf,sizeof(int));
 memcpy(wait_buf,(char *)&XCursesLINES,sizeof(int));
 (void)write_socket(display_sock,wait_buf,sizeof(int));
 memcpy(wait_buf,(char *)&XCursesCOLS,sizeof(int));
 (void)write_socket(display_sock,wait_buf,sizeof(int));
/*---------------------------------------------------------------------*/
/* Fork off the child process to setup and handle the X-windows events.*/
/*---------------------------------------------------------------------*/
 start_event_handler();
 return(OK);
}


#ifndef MAIN
int XCurses_get_rows(void)
{
 return(XCursesLINES);
}
int XCurses_get_cols(void)
{
 return(XCursesCOLS);
}
#endif


#ifdef MAIN
void display_button(void)
{
 char string[20];

 sprintf(string,"Button is: ??");
 XDrawImageString(display,win,normal_gc,100,120,string,strlen(string));
 return;
}
void display_key(char *key)
{
 char string[20];

 sprintf(string,"Key is: %s",key);

 XDrawImageString(display,win,bold_gc,100,100,string,strlen(string));
 return;
}
/***********************************************************************/
#ifdef PROTO
static void place_text(Window win, GC gc, XFontStruct *font_info)
#else
static void place_text(win, gc, font_info)
Window win;
GC gc;
XFontStruct *font_info;
#endif
/***********************************************************************/
{
        int y = 20;     /* offset from corner of window*/
        char *string1 = "Hi! I'm a window, who are you?";
        char *string2 = "To terminate program; Press any key";
        char *string3 = "or button while in this window.";
        char *string4 = "Screen Dimensions:";
        int len1, len2, len3, len4;
        int width1, width2, width3;
        char cd_height[80], cd_width[80], cd_depth[80];
        int font_height,win_width,win_height;
        int initial_y_offset, x_offset;
        XWindowAttributes win_attrs;

        XGetWindowAttributes(display,win,&win_attrs);
        win_height = win_attrs.height;
        win_width = win_attrs.width;

        /* need length for both XTextWidth and XDrawString */
        len1 = strlen(string1);
        len2 = strlen(string2);
        len3 = strlen(string3);

#ifdef PDCDEBUG
 say("before XTextWidth\n");
#endif
        /* get string widths for centering */
        width1 = XTextWidth(font_info, string1, len1);
        width2 = XTextWidth(font_info, string2, len2);
        width3 = XTextWidth(font_info, string3, len3);

#ifdef PDCDEBUG
  say("before XDrawImageString\n");
#endif
        /* output text, centered on each line */
        XDrawImageString(display,win,gc,(win_width - width1)/2,y,string1,len1);
        XDrawImageString(display,win,gc,(win_width - width2)/2,
                        (int)(win_height - 35),string2,len2);
        XDrawImageString(display,win,gc,(win_width - width3)/2,
                        (int)(win_height - 15),string3,len3);

        /* copy numbers into string variables */
        (void) sprintf(cd_height, " Height - %d/%d pixels",
                        DisplayHeight(display,screen_num),win_height);
        (void) sprintf(cd_width, " Width  - %d/%d pixels",
                        DisplayWidth(display,screen_num),win_width);
        (void) sprintf(cd_depth, " Depth  - %d plane(s)", 
                        DefaultDepth(display, screen_num));

        /* reuse these for same purpose */
        len4 = strlen(string4);
        len1 = strlen(cd_height);
        len2 = strlen(cd_width);
        len3 = strlen(cd_depth);

        font_height = font_info->max_bounds.ascent + 
                        font_info->max_bounds.descent;

        /* To center strings vertically, we place the first string
         * so that the top of it is two font_heights above the center
         * of the window.  Since the baseline of the string is what we
         * need to locate for XDrawString, and the baseline is one
         * font_info->max_bounds.ascent below the top of the chacter,
         * the final offset of the origin up from the center of the 
         * window is one font_height + one descent. */

        initial_y_offset = win_height/2 - font_height - 
                        font_info->max_bounds.descent;
        x_offset = (int) win_width/4;
        XDrawString(display, win, gc, x_offset, (int) initial_y_offset, 
                        string4,len4);

#ifdef PDCDEBUG
  say("before XSetForeGround\n");
#endif
        XSetForeground(display, gc, colors[COLOR_RED]);
        XSetBackground(display, gc, colors[COLOR_BLUE]);
        XDrawImageString(display, win, gc, x_offset, (int) initial_y_offset +
                        font_height,cd_height,len1);
        XSetForeground(display, gc, colors[COLOR_MAGENTA]);
        XSetBackground(display, gc, colors[COLOR_CYAN]);
        XDrawImageString(display, win, gc, x_offset, (int) initial_y_offset +
                        2 * font_height,cd_width,len2);
        XSetForeground(display, gc, colors[COLOR_BLACK]);
        XSetBackground(display, gc, colors[COLOR_GREEN]);
        XDrawImageString(display, win, gc, x_offset, (int) initial_y_offset +
                        3 * font_height,cd_depth,len3);
}

/***********************************************************************/
#ifdef PROTO
static void place_graphics(Window win, GC gc, int rows, 
                           int fontwidth, int fontheight)
#else
static void place_graphics(win, gc, rows, fontwidth, fontheight)
Window win;
GC gc;
int rows;
int fontwidth, fontheight;
#endif
/***********************************************************************/
{
        int i,j;
        int xpos,ypos;
        char buffer[400];

        for (i=0;i<256;i++)
           buffer[i] = i;
        XSetForeground(display, gc, colors[COLOR_WHITE]);
        XSetBackground(display, gc, colors[COLOR_BLUE]);
        for (i=0;i<rows;i++)
          {
           for (j=0;j<10;j++)
             {
              if ((i*10)+j < 256)
                {
                 makeXY(j,i,fontwidth,fontheight,&xpos,&ypos);
                 if (i==2)
                    XDrawImageString(display,win,gc,xpos+1,ypos,buffer+(i*10)+j,1);
                    XDrawImageString(display,win,gc,xpos,ypos,buffer+(i*10)+j,1);
                }
             }
          }
}
/***********************************************************************/
#ifdef PROTO
static void TooSmall(Window win, GC gc,XFontStruct *font_info)
#else
TooSmall(win, gc, font_info)
Window win;
GC gc;
XFontStruct *font_info;
#endif
/***********************************************************************/
{
        char *string1 = "Too Small";
        int y_offset, x_offset;

        y_offset = font_info->max_bounds.ascent + 2;
        x_offset = 2;

        /* output text, centered on each line */
        XDrawString(display, win, gc, x_offset, y_offset, string1, 
                        strlen(string1));
}
#endif

#ifdef MAIN
int main(argc, argv)
int argc;
char **argv;
{
#ifdef PDCDEBUG
  say("before Xinitscr\n");
#endif
  Xinitscr();

#ifdef PDCDEBUG
  say("before Xendwin\n");
#endif
  Xendwin();

  return(0);
}
#endif
#endif
