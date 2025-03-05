#include "HTUtils.h"
#include "tcp.h"
#include "LYCurses.h"
#include "LYUtils.h"
#include "LYString.h"
#include "LYGlobalDefs.h"
#include "GridText.h"
#include "LYKeymap.h"
#include "LYSignal.h"
#include "LYClean.h"
// #include "LYMail.h"
#include "LYOption.h"

#include <ctype.h>

#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

extern BOOL HTPassHighCtrlRaw;

/*
 *  LYstrncpy() terminates strings with a null byte.
 *  Writes a null byte into the n+1 byte of dst.
 */
PUBLIC char *LYstrncpy ARGS3(
	char *,		dst,
	char *,		src,
	int,		n)
{
    char *val;
    int len=strlen(src);

    if (n < 0)
        n = 0;

    val = strncpy(dst, src, n);
    if (len < n)
        *(dst+len) = '\0';
    else
        *(dst+n) = '\0';
    return val;
}

#define IS_NEW_GLYPH(ch) (utf_flag && ((unsigned char)(ch)&0xc0) != 0x80)
#define IS_UTF_EXTRA(ch) (utf_flag && ((unsigned char)(ch)&0xc0) == 0x80)

/*
 *  LYmbcsstrlen() returns the printable length of a string
 *  that might contain IsSpecial or multibyte (CJK or UTF8)
 *  characters. - FM
 */
PUBLIC int LYmbcsstrlen ARGS2(
	char *,		str,
	BOOL,		utf_flag)
{
    int i, j, len = 0;

    if (!str && *str)
	return(len);

    for (i = 0; str[i] != '\0'; i++) {
	if (IsSpecialAttrChar(str[i])) {
	    continue;
	} else {
	    len++;
	}
	if (IS_NEW_GLYPH(str[i])) {
	    j = 0;
	    while (str[(i + 1)] != '\0' && 
	    	   !IsSpecialAttrChar(str[(i + 1)]) &&
		   j < 5 &&
		   IS_UTF_EXTRA(str[(i + 1)])) {
		i++;
		j++;
	    }
	} else if (!utf_flag && !isascii(str[i]) &&
		    str[(i + 1)] != '\0' &&
		    !IsSpecialAttrChar(str[(i + 1)])) {
	    i++;
	}
    }

    return(len);
}

#define GetChar() getch()

/*
 * LYgetch() translates some escape sequences and may fake noecho
 */
PUBLIC int LYgetch ()
{
    int a, b, c, d;

re_read:
    clearerr(stdin); /* needed here for ultrix and SOCKETSHR, but why? - FM */
    c = GetChar();

    if (feof(stdin) || ferror(stdin) || c == EOF) {
	if(recent_sizechange)
	    return(7); /* use ^G to cancel whatever called us. */
#ifdef IGNORE_CTRL_C
	if (sigint) {
	    sigint = FALSE;
	    /* clearerr(stdin);  don't need here if stays above - FM */
	    goto re_read;
	}
#endif /* IGNORE_CTRL_C */
	cleanup();
	exit(0);
    }

  if (c == 0) c = '/';
  if (c > 255) {      /* handle raw dos keys */
    switch (c)
    {
      case 464: c = '-'; break;  /* keypad minus*/
      case 465: c = '+'; break;  /* keypad plus*/
      case 459: c = 13; break;  /* keypad enter*/
      case 463: c = '*'; break;  /* keypad * */
//      case 440: c = 'Q'; break;  /* alt x */
//      default: break;
    }
    if (c < 256) return(c);
  }

    if (c == 27 || c == 155) {      /* handle escape sequence */
        b = GetChar();

        if (b == '[' || b == 'O') {
            a = GetChar();
        } else {
            a = b;
	}

        switch (a) {
        case 'A': c = UPARROW; break;
        case 'x': c = UPARROW; break;  /* keypad up on pc ncsa telnet */
        case 'B': c = DNARROW; break;
        case 'r': c = DNARROW; break; /* keypad down on pc ncsa telnet */
        case 'C': c = RTARROW; break;
        case 'v': c = RTARROW; break; /* keypad right on pc ncsa telnet */
        case 'D': c = LTARROW; break;
        case 't': c = LTARROW; break; /* keypad left on pc ncsa telnet */
        case 'y': c = PGUP; break;  /* keypad on pc ncsa telnet */
        case 's': c = PGDOWN; break;  /* keypad on pc ncsa telnet */
        case 'w': c = HOME; break;  /* keypad on pc ncsa telnet */
        case 'q': c = END; break;  /* keypad on pc ncsa telnet */
        case 'M': c = '\n'; break; /* kepad enter on pc ncsa telnet */

        case 'm':
#ifdef VMS
            if (b != 'O')
#endif /* VMS */
                c = '-';  /* keypad on pc ncsa telnet */
            break;
	case 'k':
	    if(b == 'O');
		c = '+';  /* keypad + on my xterminal :) */
	    break;
        case 'l':
#ifdef VMS
            if (b != 'O')
#endif /* VMS */
                c = '+';  /* keypad on pc ncsa telnet */
            break;
        case 'P':
#ifdef VMS
            if (b != 'O')
#endif /* VMS */
                c = F1;
            break;
        case 'u':
#ifdef VMS
            if (b != 'O')
#endif /* VMS */
                c = F1;  /* macintosh help button */
            break;
        case '1':                           /** VT300  Find  **/
            if ((b == '[' || c == 155) && GetChar() == '~')
                c = FIND_KEY;
            break;
	case '2':
	    if (b == '[' || c == 155) {
	        if ((d=GetChar())=='~')     /** VT300 Insert **/
	            c = INSERT_KEY;
	        else if ((d == '8' ||
			  d == '9') &&
			 GetChar() == '~')
	         {
		    if (d == '8')            /** VT300  Help **/
	                c = F1;
	            else if (d == '9')       /** VT300   Do  **/
	                c = DO_KEY;
		 }
	    }
	    break;
	case '3':			     /** VT300 Delete **/
	    if ((b == '[' || c == 155) && GetChar() == '~')
	        c = REMOVE_KEY;
	    break;
        case '4':                            /** VT300 Select **/
            if ((b == '[' || c == 155) && GetChar() == '~')
                c = SELECT_KEY;
            break;
        case '5':                            /** VT300 PrevScreen **/
            if ((b == '[' || c == 155) && GetChar() == '~')
                c = '-';
            break;
        case '6':                            /** VT300 NextScreen **/
            if ((b == '[' || c == 155) && GetChar() == '~')
                c = '+';
            break;
	default: {}
#ifdef DT
	   if(TRACE) {
		fprintf(stderr,"Unknown key sequence: %d:%d:%d\n",c,b,a);
		sleep(sleep_two);
	   }
#endif

        }
#if defined(NO_KEYPAD) || defined(VMS)
    }
#else
    } else {

	/* convert keypad() mode keys into Lynx defined keys
	 */

	switch(c) {
	case KEY_DOWN:	           /* The four arrow keys ... */
	   c=DNARROW;
	   break;
	case KEY_UP:	
	   c=UPARROW;
	   break;
	case KEY_LEFT:	
	   c=LTARROW;
	   break;
	case KEY_RIGHT:	           /* ... */
	   c=RTARROW;
	   break;
	case KEY_HOME:	           /* Home key (upward+left arrow) */
	   c=HOME;
	   break;
#if 0
	case KEY_CLEAR:	           /* Clear screen */
	   c=18; /* CTRL-R */
	   break;
#endif
	case KEY_NPAGE:	           /* Next page */
	   c=PGDOWN;
	   break;
	case KEY_PPAGE:	           /* Previous page */
	   c=PGUP;
	   break;
#if 0
	case KEY_LL:	           /* home down or bottom (lower left) */
	   c=END;
	   break;
#endif
	   /* The keypad is arranged like this:*/
                                        /*    a1    up    a3   */
                                        /*   left   b2  right  */
                                        /*    c1   down   c3   */
#if 0
	case KEY_A1:	           /* upper left of keypad */
	   c=HOME;
	   break;
	case KEY_A3:	           /* upper right of keypad */
	   c=PGUP;
	   break;
	case KEY_B2:	           /* center of keypad */
	   c=DO_NOTHING;
	   break;
	case KEY_C1:	           /* lower left of keypad */
	   c=END;
	   break;
	case KEY_C3:	           /* lower right of keypad */
	   c=PGDOWN;
	   break;
#endif
//#ifdef KEY_END
#if 1
	case KEY_END:	           /* end key           001 */
	   c=END;
	   break;
#endif /* KEY_END */
#ifdef KEY_HELP
	case KEY_HELP:	           /* help key          001 */
	   c=F1;
	   break;
#endif /* KEY_HELP */
	}
    }
#endif /* defined(NO_KEYPAD) || defined(VMS) */

    return(c);
}


#if defined(NCURSES)
/*
 * Workaround a bug in ncurses order-of-refresh by setting a pointer to
 * the topmost window that should be displayed.
 */
PRIVATE WINDOW *my_subwindow;

PUBLIC void LYsubwindow ARGS1(WINDOW *, param)
{
	my_subwindow = param;
}
#endif

/*
**  Display the current value of the string and allow the user
**  to edit it.
*/

#define EDREC    EditFieldData

/*
 *  Shorthand to get rid of all most of the "edit->suchandsos".
 */
#define Buf      edit->buffer
#define Pos      edit->pos
#define StrLen   edit->strlen
#define MaxLen   edit->maxlen
#define DspWdth  edit->dspwdth
#define DspStart edit->xpan
#define Margin   edit->margin

PUBLIC void LYSetupEdit ARGS4(
	EDREC *,	edit,
	char *,		old,
	int,		maxstr,
	int,		maxdsp)
{
    /*
     *  Initialize edit record
     */
    LYGetYX(edit->sy, edit->sx);
    edit->pad   = ' ';
    edit->dirty = TRUE;
    edit->panon = FALSE;

    StrLen  = strlen(old);
    MaxLen  = maxstr;
    DspWdth = maxdsp;
    Margin  = 0;
    Pos = strlen(old);
    DspStart = 0;

    if (maxstr > maxdsp) {  /* Need panning? */
        if (DspWdth > 4)    /* Else "{}" take up precious screen space */
	    edit->panon = TRUE;

	/*
	 *  Figure out margins.  If too big, we do a lot of unnecessary
	 *  scrolling.  If too small, user doesn't have sufficient
	 *  look-ahead.  Let's say 25% for each margin, upper bound is
	 *  10 columns.
	 */
	Margin = DspWdth/4;
	if (Margin > 10)
	    Margin = 10;
    }

    /*
     *  We expect the called function to pass us a default (old) value
     *  with a length that is less than or equal to maxstr, and to
     *  handle any messaging associated with actions to achieve that
     *  requirement.  However, in case the calling function screwed
     *  up, we'll check it here, and ensure that no buffer overrun can
     *  occur by loading only as much of the head as fits. - FM
     */
    if (strlen(old) >= maxstr) {
	strncpy(edit->buffer, old, maxstr);
	edit->buffer[maxstr] = '\0';
	StrLen = maxstr;
    } else {
	strcpy(edit->buffer, old);
    }
}

PUBLIC int LYEdit1 ARGS4(
	EDREC *,	edit,
	int,		ch,
	int,		action,
	BOOL,		maxMessage)
{   /* returns 0    character processed
     *         ch   otherwise
     */
    int i;
    int length;

    if (MaxLen <= 0)
        return(0); /* Be defensive */

    length=strlen(&Buf[0]);
    StrLen = length;

    switch (action) {
//    case LYE_AIX:
	/*
	 *  Hex 97.
	 *  Fall through as a character for CJK.
	 *  Otherwise, we treat this as LYE_ENTER.
	 */
//	 if (HTCJK == NOCJK)
//	     return(ch);
    case LYE_CHAR:
        /*
	 *  ch is printable or ISO-8859-1 escape character.
	 */
	if (Pos <= (MaxLen) && StrLen < (MaxLen)) {
	    for(i = length; i >= Pos; i--)    /* Make room */
		Buf[i+1] = Buf[i];
	    Buf[length+1]='\0';
	    Buf[Pos] = (unsigned char) ch;
	    Pos++;
	} else if (maxMessage) {
	    _statusline("MAXLEN_REACHED_DEL_OR_MOV");
	}
	break;

    case LYE_BACKW:
        /*
	 *  Backword.
	 *  Definition of word is very naive: 1 or more a/n characters.
	 */
	while (Pos && !isalnum(Buf[Pos-1]))
	    Pos--;
	while (Pos &&  isalnum(Buf[Pos-1]))
	    Pos--;
	break;

    case LYE_FORWW:
        /*
	 *  Word forward.
	 */
	while (isalnum(Buf[Pos]))
	    Pos++;   /* '\0' is not a/n */
	while (!isalnum(Buf[Pos]) && Buf[Pos])
	    Pos++ ;
	break;

    case LYE_ERASE:
        /*
	 *  Erase the line to start fresh.
	 */
	 Buf[0] = '\0';
	 /* fall through */

    case LYE_BOL:
        /*
	 *  Go to first column.
	 */
	Pos = 0;
	break;

    case LYE_EOL:
        /*
	 *  Go to last column.
	 */
	Pos = length;
	break;

    case LYE_DELNW:
        /*
	 *  Delete next word.
	 */
	{
	    int pos0 = Pos;
	    LYEdit1 (edit, 0, LYE_FORWW, FALSE);
	    while (Pos > pos0)
	        LYEdit1(edit, 0, LYE_DELP, FALSE);
	}
	break;

    case LYE_DELPW:
        /*
	 *  Delete previous word.
	 */
	{
	    int pos0 = Pos;
	    LYEdit1 (edit, 0, LYE_BACKW, FALSE);
	    pos0 -= Pos;
	    while (pos0--)
	        LYEdit1(edit, 0, LYE_DELN, FALSE);
	}
	break;

    case LYE_DELN:
        /*
	 *  Delete next character
	 */
	if (Pos >= length)
	    break;
	Pos++;
	/* fall through */

    case LYE_DELP:
        /*
	 *  Delete preceding character.
	 */
	if (length == 0 || Pos == 0)
	    break;
	Pos--;
	for (i = Pos; i < length; i++)
	    Buf[i] = Buf[i+1];
	i--;
	Buf[i] = 0;
	break;

    case LYE_DELC:
        /*
	 *  Delete current character.
	 */
        if (length == 0 || Pos == length)
	    break;
	for (i = Pos; i < length; i++)
	    Buf[i] = Buf[i+1];
	i--;
	Buf[i] = 0;
	break;

    case LYE_FORW:
        /*
	 *  Move cursor to the right.
	 */
	if (Pos < length)
	    Pos++;
	break;

    case LYE_BACK:
        /*
	 *  Left-arrow move cursor to the left.
	 */
	if (Pos > 0)
	    Pos--;
	break;

    case LYE_UPPER:
	for (i = 0; Buf[i]; i++)
	   Buf[i] = TOUPPER(Buf[i]);
	break;

    case LYE_LOWER:
	for (i = 0; Buf[i]; i++)
	   Buf[i] = TOLOWER(Buf[i]);
	break;

    default:
	return(ch);
    }
    edit->dirty = TRUE;
    StrLen = strlen(&Buf[0]);
    return(0);
}


PUBLIC void LYRefreshEdit ARGS1(
	EDREC *,	edit)
{
    int i;
    int length;
    int nrdisplayed;
    int padsize;
    char *str;
    char buffer[3];

    buffer[0] = buffer[1] = buffer[2] = '\0';
    if (!edit->dirty || (DspWdth == 0))
        return;
    edit->dirty = FALSE;

    length=strlen(&Buf[0]);
    edit->strlen = length;
/*
 *  Now we have:
 *                .--DspWdth---.
 *      +---------+=============+-----------+
 *      |         |M           M|           |   (M=margin)
 *      +---------+=============+-----------+
 *      0         DspStart                   length
 *
 *  Insertion point can be anywhere beween 0 and stringlength.
 *  Figure out new display starting point.
 *
 *   The first "if" below makes Lynx scroll several columns at a time when
 *   extending the string. Looks awful, but that way we can keep up with
 *   data entry at low baudrates.
 */
    if ((DspStart + DspWdth) <= length)
        if (Pos >= (DspStart + DspWdth) - Margin)
	    DspStart=(Pos - DspWdth) + Margin;

    if (Pos < DspStart + Margin) {
        DspStart = Pos - Margin;
	if (DspStart < 0)
	    DspStart = 0;
    }

    str = &Buf[DspStart];

    nrdisplayed = length-DspStart;
    if (nrdisplayed > DspWdth)
        nrdisplayed = DspWdth;

    move(edit->sy, edit->sx);
    if (edit->hidden) {
        for (i = 0; i < nrdisplayed; i++)
	    addch('*');
    } else {
        for (i = 0; i < nrdisplayed; i++)
	    if ((buffer[0] = str[i]) == 1 || buffer[0] == 2 ||
	        ((unsigned char)buffer[0] == 160 &&
		 !HTPassHighCtrlRaw)) {
	        addch(' ');
	    } else {
		/* For CJK strings, by Masanobu Kimura */
		if (!isascii(buffer[0])) {
		    if (i < (nrdisplayed - 1))
		        buffer[1] = str[++i];
		    addstr(buffer);
		    buffer[1] = '\0';
		} else {
		    addstr(buffer);
		}
	    }
    }

    /*
     *  Erase rest of input area.
     */
    padsize = DspWdth-nrdisplayed;
    while (padsize--)
        addch((unsigned char)edit->pad);

    /*
     *  Scrolling indicators.
     */
    if (edit->panon) {
        if ((DspStart + nrdisplayed) < length) {
	    move(edit->sy, edit->sx+nrdisplayed-1);
	    addch('}');
	}
	if (DspStart) {
	    move(edit->sy, edit->sx);
	    addch('{');
	}
    }

    move(edit->sy, edit->sx + Pos - DspStart);
    refresh();
}


PUBLIC int LYgetstr ARGS2(
	char *,		inputline,
	int,		hidden)
{
	int		bufsize = 0;
	int		recall = 0;

    int x, y, MaxStringSize;
    int ch;
    EditFieldData MyEdit;

    LYGetYX(y, x);		/* Use screen from cursor position to eol */
    MaxStringSize = (bufsize < sizeof(MyEdit.buffer)) ?
    		    (bufsize - 1) : (sizeof(MyEdit.buffer) - 1);
    LYSetupEdit(&MyEdit, inputline, MaxStringSize, (LYcols-1)-x);
    MyEdit.hidden = hidden ;

    for (;;) {
again:
	LYRefreshEdit(&MyEdit);
	ch = LYgetch();
#ifdef VMS
	if (term_letter || term_options || term_message || HadVMSInterrupt) {
	    HadVMSInterrupt = FALSE;
	    ch = 7;
	}
#else
//	if (term_letter || term_options || term_message)
//	    ch = 7;
#endif /* VMS */
	if (recall && (ch == UPARROW || ch == DNARROW)) {
	    strcpy(inputline, MyEdit.buffer);
	    return(ch);
	}
	if (keymap[ch + 1] == LYK_REFRESH)
	    goto again;
	switch (EditBinding(ch)) {
	case LYE_TAB:
	    ch = '\t';
	    /* fall through */
	case LYE_AIX:
	    /*
	     *  Hex 97.
	     *  Treat as a character for CJK.
	     *  Otherwise, we treat this as LYE_ENTER.
	     */
	    if (ch != '\t') {
	        LYLineEdit(&MyEdit,ch, FALSE);
		break;
	    }
	case LYE_ENTER:
	    /*
	     *  Terminate the string and return.
	     */
	    strcpy(inputline, MyEdit.buffer);
            return(ch);
	    break;

        case LYE_ABORT:
	    /*
	     *  Control-C or Control-G aborts.
	     */
	    inputline[0] = '\0';		   
	    return(-1);
            break;

        default:
            LYLineEdit(&MyEdit,ch, FALSE);
        }
    }
}

/*
 *  LYstrstr will find the first occurence of the string
 *  pointed to by tarptr in the string pointed to by chptr.  
 *  It is a case insensitive search.
 */
PUBLIC char * LYstrstr ARGS2(
	char *,		chptr,
	char *,		tarptr)
{
    register char *tmpchptr, *tmptarptr;

    for(; *chptr != '\0'; chptr++) {
	if(TOUPPER(*chptr) == TOUPPER(*tarptr)) {	
	    /* see if they line up */ 
	    for(tmpchptr = chptr+1, tmptarptr = tarptr+1;
	        (TOUPPER(*tmpchptr) == TOUPPER(*tmptarptr) &&
		 *tmptarptr != '\0' && *tmpchptr != '\0');
	        tmpchptr++, tmptarptr++)
		   ; /* null body */ 
	    if(*tmptarptr == '\0') 
	  	return(chptr);
	}
    } /* end for */

    return(NULL);
}	

/*
 *  LYno_attr_char_case_strstr will find the first occurence of the
 *  string pointed to by tarptr in the string pointed to by chptr.
 *  It ignores the characters: LY_UNDERLINE_START_CHAR and
 * 			       LY_UNDERLINE_END_CHAR
 * 			       LY_BOLD_START_CHAR
 * 			       LY_BOLD_END_CHAR
 *				LY_SOFT_HYPHEN
 *			       if present in chptr.
 *  It is a case insensitive search.
 */
PUBLIC char * LYno_attr_char_case_strstr ARGS2(
	char *,		chptr,
	char *,		tarptr)
{
    register char *tmpchptr, *tmptarptr;

    if (!chptr)
        return(NULL);

    while (IsSpecialAttrChar(*chptr) && *chptr != '\0')
        chptr++;

    for (; *chptr != '\0'; chptr++) {
        if (TOUPPER(*chptr) == TOUPPER(*tarptr)) {
            /*
	     *  See if they line up.
	     */
	    tmpchptr = chptr+1;
	    tmptarptr = tarptr+1;

	    if (*tmptarptr == '\0')  /* one char target */
		 return(chptr);

	    while (1) {
		 if (!IsSpecialAttrChar(*tmpchptr)) {
                    if (TOUPPER(*tmpchptr) != TOUPPER(*tmptarptr))
			break;
		    tmpchptr++;
		    tmptarptr++;
		 } else {
		    tmpchptr++;
		 }
                 if (*tmptarptr == '\0')
		     return(chptr);
		 if (*tmpchptr == '\0')
		     break;
	    }
        }
    } /* end for */

    return(NULL);
}

/*
 *  LYno_attr_char_strstr will find the first occurence of the
 *  string pointed to by tarptr in the string pointed to by chptr.
 *  It ignores the characters: LY_UNDERLINE_START_CHAR and
 *                             LY_UNDERLINE_END_CHAR
 *                             LY_BOLD_START_CHAR
 *                             LY_BOLD_END_CHAR
 *				LY_SOFT_HYPHEN
 *			       if present in chptr.
 *  It is a case sensitive search.
 */
PUBLIC char * LYno_attr_char_strstr ARGS2(
	char *,		chptr,
	char *,		tarptr)
{
    register char *tmpchptr, *tmptarptr;

    if (!chptr)
        return(NULL);

    while (IsSpecialAttrChar(*chptr) && *chptr != '\0')
        chptr++;

    for (; *chptr != '\0'; chptr++) {
        if ((*chptr) == (*tarptr)) {
            /*
	     *  See if they line up.
	     */
            tmpchptr = chptr + 1;
            tmptarptr = tarptr + 1;

	    if (*tmptarptr == '\0')  /* one char target */
		 return(chptr);

            while (1) {
		 if (!IsSpecialAttrChar(*tmpchptr)) {
                    if ((*tmpchptr) != (*tmptarptr))
                        break;
                    tmpchptr++;
                    tmptarptr++;
                 } else {
                    tmpchptr++;
                 }
                 if (*tmptarptr == '\0')
                     return(chptr);
                 if (*tmpchptr == '\0')
                     break;
            }
        }
    } /* end for */

    return(NULL);
}

/*
 * LYno_attr_mbcs_case_strstr will find the first occurence of the string 
 * pointed to by tarptr in the string pointed to by chptr.
 * It takes account of MultiByte Character Sequences (UTF8).
 * The physical lenght of the displayed string up to the end of the target
 * string is returned in *nendp if the search is successful.
 * It ignores the characters: LY_UNDERLINE_START_CHAR and
 * 			      LY_UNDERLINE_END_CHAR
 * 			      LY_BOLD_START_CHAR
 *				LY_BOLD_END_CHAR
 *				LY_SOFT_HYPHEN
 *			      if present in chptr.
 * It assumes UTF8 if utf_flag is set.
 *  It is a case insensitive search. - KW & FM
 */
PUBLIC char * LYno_attr_mbcs_case_strstr ARGS5(
	char *,		chptr,
	char *,		tarptr,
	BOOL,		utf_flag,
	int *,		nstartp,
	int *,		nendp)
{
    register char *tmpchptr, *tmptarptr;
    int len = 0;
    int offset;

    if (!(chptr && tarptr))
        return(NULL);

    /*
     *  Skip initial IsSpecial chars. - FM
     */
    while (IsSpecialAttrChar(*chptr) && *chptr != '\0')
        chptr++;

    /*
     *  Seek a first target match. - FM
     */
    for (; *chptr != '\0'; chptr++) {
	if ((!utf_flag && !isascii(*chptr) &&
	     *chptr == *tarptr &&
	     *(chptr + 1) != '\0' &&
	     !IsSpecialAttrChar(*(chptr + 1))) ||
	    TOUPPER(*chptr) == TOUPPER(*tarptr)) {
	    int tarlen = 0;
	    offset = len;
	    len++;

            /*
	     *  See if they line up.
	     */
	    tmpchptr = (chptr + 1);
	    tmptarptr = (tarptr + 1);

	    if (*tmptarptr == '\0') {
		/*
		 *  One char target.
		 */
		*nstartp = offset;
		*nendp = len;
		 return(chptr);
	    }
	    if (!utf_flag && !isascii(*chptr) &&
		 *chptr == *tarptr &&
		 *tmpchptr != '\0' &&
		 !IsSpecialAttrChar(*tmpchptr)) {
		/*
		 *  Check the CJK mutibyte. - FM
		 */
		if (*tmpchptr == *tmptarptr) {
		    /*
		     *  It's a match.  Advance to next char. - FM
		     */
		    tmpchptr++;
		    tmptarptr++;
		    if (*tmptarptr == '\0') {
		        /*
			 *  One character match. - FM
			 */
			*nstartp = offset;
			*nendp = len + tarlen;
			return(chptr);
		    }
		    tarlen++;
		} else {
		    /*
		     *  It's not a match, so go back to
		     *  seeking a first target match. - FM
		     */
		    chptr++;
		    continue;
		}
	    }
	    /*
	     *  See if the rest of the target matches. - FM
	     */
	    while (1) {
		 if (!IsSpecialAttrChar(*tmpchptr)) {
		    if (!utf_flag && !isascii(*tmpchptr)) {
			if (*tmpchptr == *tmptarptr &&
			    *(tmpchptr + 1) == *(tmptarptr + 1) &&
			    !IsSpecialAttrChar(*(tmpchptr + 1))) {
			    tmpchptr++;
			    tmptarptr++;
			} else {
			break;
			}
		    } else if (TOUPPER(*tmpchptr) != TOUPPER(*tmptarptr)) {
			break;
		    }

		    if (!IS_UTF_EXTRA(*tmptarptr)) {
			tarlen++;
		    }
		    tmpchptr++;
		    tmptarptr++;

		 } else {
		    tmpchptr++;
		 }

                 if (*tmptarptr == '\0') {
		    *nstartp = offset;
		     *nendp = len + tarlen;
		     return(chptr);
		 }
		if (*tmpchptr == '\0') {
		     break;
	    }
	    }
        } else if (!(IS_UTF_EXTRA(*chptr) ||
		      IsSpecialAttrChar(*chptr))) {
	    if (!utf_flag && !isascii(*chptr) &&
		*(chptr + 1) != '\0' &&
		!IsSpecialAttrChar(*(chptr + 1))) {
		chptr++;
	    }
	    len++;
	}
    } /* end for */

    return(NULL);
}

/*
 * LYno_attr_mbcs_strstr will find the first occurence of the string
 * pointed to by tarptr in the string pointed to by chptr.
 *  It takes account of CJK and MultiByte Character Sequences (UTF8).
 *  The physical lengths of the displayed string up to the start and
 *  end of the target string are returned in *nstartp and *nendp if
 *  the search is successful.
 * It ignores the characters: LY_UNDERLINE_START_CHAR and
 *                            LY_UNDERLINE_END_CHAR
 *                            LY_BOLD_START_CHAR
 *                            LY_BOLD_END_CHAR
 *				LY_SOFT_HYPHEN
 *			      if present in chptr.
 * It assumes UTF8 if utf_flag is set.
 *  It is a case sensitive search. - KW & FM
 */
PUBLIC char * LYno_attr_mbcs_strstr ARGS5(
	char *,		chptr,
	char *,		tarptr,
	BOOL,		utf_flag,
	int *,		nstartp,
	int *,		nendp)
{
    register char *tmpchptr, *tmptarptr;
    int len = 0;
    int offset;

    if (!(chptr && tarptr))
        return(NULL);

    /*
     *  Skip initial IsSpecial chars. - FM
     */
    while (IsSpecialAttrChar(*chptr) && *chptr != '\0')
        chptr++;

    /*
     *  Seek a first target match. - FM
     */
    for (; *chptr != '\0'; chptr++) {

        if ((*chptr) == (*tarptr)) {
	    int tarlen = 0;
	    offset = len;
	    len++;

            /*
	     *  See if they line up.
	     */
            tmpchptr = (chptr + 1);
            tmptarptr = (tarptr + 1);

	    if (*tmptarptr == '\0') {
		/*
		 *  One char target.
		 */
		*nstartp = offset;
		*nendp = len + 1;
		 return(chptr);
	    }
	    if (!utf_flag && !isascii(*chptr) &&
		 *tmpchptr != '\0' &&
		 !IsSpecialAttrChar(*tmpchptr)) {
		/*
		 *  Check the CJK mutibyte. - FM
		 */
		if (*tmpchptr == *tmptarptr) {
		    /*
		     *  It's a match.  Advance to next char. - FM
		     */
		    tmpchptr++;
		    tmptarptr++;
		    if (*tmptarptr == '\0') {
		        /*
			 *  One character match. - FM
			 */
			*nstartp = offset;
			*nendp = len + tarlen;
			return(chptr);
		    }
		    tarlen++;
		} else {
		    /*
		     *  It's not a match, so go back to
		     *  seeking a first target match. - FM
		     */
		    chptr++;
		    continue;
		}
	    }
	    /*
	     *  See if the rest of the target matches. - FM
	     */
            while (1) {
		 if (!IsSpecialAttrChar(*tmpchptr)) {
		    if (!utf_flag && !isascii(*tmpchptr)) {
			if (*tmpchptr == *tmptarptr &&
			    *(tmpchptr + 1) == *(tmptarptr + 1) &&
			    !IsSpecialAttrChar(*(tmpchptr + 1))) {
			    tmpchptr++;
			    tmptarptr++;
			} else {
			    break;
			}
		    } else if ((*tmpchptr) != (*tmptarptr)) {
                        break;
		    }

		    if (!IS_UTF_EXTRA(*tmptarptr)) {
			tarlen++;
		    }
                    tmpchptr++;
                    tmptarptr++;

                 } else {
                    tmpchptr++;
                 }

                 if (*tmptarptr == '\0') {
		    *nstartp = offset;
		     *nendp = len + tarlen;
		     return(chptr);
		 }
		if (*tmpchptr == '\0') {
                     break;
            }
	    }
        } else if (!(IS_UTF_EXTRA(*chptr) ||
		      IsSpecialAttrChar(*chptr))) {
	    if (!utf_flag && !isascii(*chptr) &&
		*(chptr + 1) != '\0' &&
		!IsSpecialAttrChar(*(chptr + 1))) {
		chptr++;
	    }
	    len++;
        }
    } /* end for */

    return(NULL);
}

/*
 *  Allocate a new copy of a string, and returns it.
 */
PUBLIC char * SNACopy ARGS3(
	char **,	dest,
	CONST char *,	src,
	int,		n)
{
    FREE(*dest);
    if (src) {
	*dest = (char *)calloc(1, n + 1);
	if (*dest == NULL) {
	    if (TRACE)
	        fprintf(stderr, "Tried to calloc %d bytes\n", n);
	    outofmem(__FILE__, "SNACopy");
	}
	strncpy (*dest, src, n);
	*(*dest + n) = '\0'; /* terminate */
    }
    return *dest;
}

/*
 *  String Allocate and Concatenate.
 */
PUBLIC char * SNACat ARGS3(
	char **,	dest,
	CONST char *,	src,
	int,		n)
{
    if (src && *src) {
	if (*dest) {
	    int length = strlen(*dest);
	    *dest = (char *)realloc(*dest, length + n + 1);
	    if (*dest == NULL)
		outofmem(__FILE__, "SNACat");
	    strncpy(*dest + length, src, n);
	    *(*dest + length + n) = '\0'; /* terminate */
	} else {
	    *dest = (char *)calloc(1, strlen(src) + 1);
	    if (*dest == NULL)
		outofmem(__FILE__, "SNACat");
	    strncpy(*dest, src, n);
	    *dest[n] = '\0'; /* terminate */
	}
    }
    return *dest;
}
