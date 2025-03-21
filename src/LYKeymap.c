#include "HTUtils.h"
#include "LYUtils.h"
#include "LYKeymap.h"
#include "LYGlobalDefs.h"

#include "LYLeaks.h"


/* the character gets 1 added to it before lookup,
 * so that EOF maps to 0
 */
char keymap[] = {

0,
/* EOF */

0,                  LYK_EXTERN,     LYK_PREV_PAGE,    0,
/* nul */           /* ^A */        /* ^B */      /* ^C */

LYK_ABORT,              0,          LYK_NEXT_PAGE,    0,
/* ^D */            /* ^E */        /* ^F */      /* ^G */

LYK_HISTORY,      LYK_NEXT_LINK,    LYK_ACTIVATE,     0,
/* bs */            /* ht */        /* nl */      /* ^K */

LYK_REFRESH,      LYK_ACTIVATE,     LYK_DOWN_TWO,     0,
/* ^L */            /* cr */        /* ^N */      /* ^O */

LYK_UP_TWO,             0,          LYK_RELOAD,       0,
/* ^P */            /* XON */       /* ^R */      /* XOFF */

LYK_TRACE_TOGGLE,       0,          LYK_VERSION,  LYK_REFRESH,
/* ^T */            /* ^U */        /* ^V */      /* ^W */

0,                      0,              0,            0,
/* ^X */            /* ^Y */        /* ^Z */      /* ESC */

0,                      0,              0,            0,
/* ^\ */            /* ^] */        /* ^^ */      /* ^_ */

LYK_NEXT_PAGE,    LYK_SHELL,            0,            0,
/* sp */             /* ! */         /* " */       /* # */

0,                      0,              0,             0,
/* $ */              /* % */         /* & */        /* ' */

0,                      0,       LYK_IMAGE_TOGGLE,  LYK_NEXT_PAGE,
/* ( */              /* ) */         /* * */        /* + */

LYK_NEXT_PAGE,    LYK_PREV_PAGE,        0,          LYK_WHEREIS,
/* , */              /* - */         /* . */        /* / */

0,                LYK_1,          LYK_2,            LYK_3,
/* 0 */              /* 1 */         /* 2 */        /* 3 */

LYK_4,            LYK_5,          LYK_6,            LYK_7,
/* 4 */              /* 5 */         /* 6 */        /* 7 */

LYK_8,            LYK_9,                0,             0,
/* 8 */              /* 9 */         /* : */        /* ; */

0,                LYK_INFO,             0,          LYK_HELP,
/* < */              /* = */         /* > */        /* ? */

0,              LYK_ADD_BOOKMARK, LYK_PREV_PAGE,    LYK_COMMENT,
/* @ */              /* A */         /* B */        /* C */

LYK_DOWNLOAD,     LYK_EDIT,
/* D */              /* E */

0,
/* F */

LYK_GOTO,
/* G */

//LYK_HELP,         LYK_INDEX,      LYK_JUMP,         LYK_KEYMAP,
LYK_HELP,         LYK_INDEX,      0,         LYK_KEYMAP,
/* H */              /* I */         /* J */        /* K */

0,                LYK_MAIN_MENU,  LYK_NEXT,         LYK_OPTIONS,
/* L */              /* M */         /* N */        /* O */

LYK_PRINT,        LYK_ABORT,      LYK_DEL_BOOKMARK, LYK_INDEX_SEARCH,
/* P */              /* Q */         /* R */        /* S */

0,
/* T */

		  LYK_PREV_DOC,   LYK_VIEW_BOOKMARK,   0,
		     /* U */         /* V */        /* W */

LYK_RESUBMIT,           0,        LYK_INTERRUPT,       0,
/* X */              /* Y */         /* Z */        /* [ */

LYK_SOURCE,             0,              0,             0,
/* \ */              /* ] */         /* ^ */        /* _ */

0,             LYK_ADD_BOOKMARK,  LYK_PREV_PAGE,    LYK_COMMENT,
/* ` */              /* a */         /* b */        /* c */

LYK_DOWNLOAD,     LYK_EDIT,
/* d */              /* e */

0,
/* f */

LYK_GOTO,
/* g */

//LYK_HELP,         LYK_INDEX,      LYK_JUMP,         LYK_KEYMAP,
LYK_HELP,         LYK_INDEX,      0,         LYK_KEYMAP,
/* h */              /* i */         /* j */        /* k */

0,                LYK_MAIN_MENU,  LYK_NEXT,         LYK_OPTIONS,
/* l */              /* m */         /* n */        /* o */

LYK_PRINT,        LYK_QUIT,       LYK_DEL_BOOKMARK, LYK_INDEX_SEARCH,
/* p */              /* q */         /* r */        /* s */

0,
/* t */

		    LYK_PREV_DOC,   LYK_VIEW_BOOKMARK,   0,
		     /* u */         /* v */         /* w */

LYK_RESUBMIT,           0,          LYK_INTERRUPT,     0,
/* x */              /* y */          /* z */       /* { */

//LYK_PIPE,               0,              0,          LYK_HISTORY,
0,               0,              0,          LYK_HISTORY,
/* | */               /* } */         /* ~ */       /* del */

LYK_PREV_LINK,    LYK_NEXT_LINK,    LYK_ACTIVATE,   LYK_PREV_DOC,
/* UPARROW */     /* DNARROW */     /* RTARROW */   /* LTARROW */

LYK_NEXT_PAGE,    LYK_PREV_PAGE,    LYK_HOME,       LYK_END,
/* PGDOWN */      /* PGUP */        /* HOME */      /* END */

//LYK_HELP,         LYK_ACTIVATE,     LYK_HOME,       LYK_END,
0,         0,     0,       0,
/* F1*/ 	  /* Do key */      /* Find key */  /* Select key */

//LYK_UP_TWO,       LYK_DOWN_TWO,
0,       0,
/* Insert key */  /* Remove key */

LYK_DO_NOTHING,
/* DO_NOTHING*/
};

/* this table is used to override the standard keyboard assignments
 * when lynx_edit_mode is in effect and keyboard overrides have been
 * allowed at compile time.
 */

#if defined(DIRED_SUPPORT) && defined(OK_OVERRIDE)

char override[] = {

    0,
/* EOF */

    0,                  0,              0,            0,
/* nul */           /* ^A */        /* ^B */      /* ^C */

    0,                  0,              0,            0,
/* ^D */            /* ^E */        /* ^F */      /* ^G */

    0,                  0,              0,            0,
/* bs */            /* ht */        /* nl */      /* ^K */

    0,                  0,              0,            0,
/* ^L */            /* cr */        /* ^N */      /* ^O */

    0,                  0,              0,            0,
/* ^P */            /* XON */       /* ^R */      /* XOFF */

    0,            LYK_PREV_DOC,         0,            0,
/* ^T */            /* ^U */        /* ^V */      /* ^W */

    0,                  0,              0,            0,
/* ^X */            /* ^Y */        /* ^Z */      /* ESC */

    0,                  0,              0,            0,
/* ^\ */            /* ^] */        /* ^^ */      /* ^_ */

    0,                 0,              0,            0,
/* sp */            /* ! */         /* " */       /* # */

   0,                  0,              0,            0,
/* $ */             /* % */         /* & */       /* ' */

    0,                 0,              0,            0,
/* ( */             /* ) */         /* * */       /* + */

    0,                 0,         LYK_TAG_LINK,      0,
/* , */             /* - */         /* . */       /* / */

   0,                 0,               0,            0,
/* 0 */             /* 1 */        /* 2 */        /* 3 */

   0,                 0,               0,            0,
/* 4 */             /* 5 */        /* 6 */        /* 7 */

   0,                 0,              0,             0,
/* 8 */             /* 9 */        /* : */        /* ; */

   0,                 0,              0,             0,
/* < */             /* = */        /* > */        /* ? */

   0,                  0,              0,         LYK_CREATE,
/* @ */             /* A */         /* B */        /* C */

   0,                  0,        LYK_DIRED_MENU,       0,
/* D */             /* E */         /* F */        /* G */

   0,                  0,              0,             0,
/* H */             /* I */         /* J */        /* K */

   0,             LYK_MODIFY,          0,             0,
/* L */             /* M */         /* N */        /* O */

   0,                  0,         LYK_REMOVE,         0,
/* P */             /* Q */         /* R */        /* S */

LYK_TAG_LINK,     LYK_UPLOAD,          0,             0,
/* T */             /* U */         /* V */        /* W */

   0,                  0,              0,             0,
/* X */             /* Y */         /* Z */        /* [ */

   0,                  0,              0,             0,
/* \ */             /* ] */         /* ^ */        /* _ */

0,                     0,              0,         LYK_CREATE,
/* ` */             /* a */         /* b */        /* c */

   0,                  0,       LYK_DIRED_MENU,       0,
/* d */             /* e */         /* f */        /* g */

   0,                  0,              0,             0,
/* h */             /* i */         /* j */        /* k */

0,                LYK_MODIFY,          0,             0,
/* l */             /* m */         /* n */        /* o */

   0,                  0,          LYK_REMOVE,        0,
/* p */             /* q */         /* r */        /* s */

LYK_TAG_LINK,      LYK_UPLOAD,         0,             0,
/* t */             /* u */         /* v */         /* w */

   0,                  0,               0,            0,
/* x */             /* y */          /* z */       /* { */

   0,                   0,             0,              0,
/* | */              /* } */         /* ~ */       /* del */

   0,                   0,             0,              0,
/* UPARROW */     /* DNARROW */     /* RTARROW */   /* LTARROW */

   0,                  0,              0,              0,
/* PGDOWN */      /* PGUP */        /* HOME */      /* END */

   0,                  0,              0,              0,
/* F1*/ 	  /* Do key */      /* Find key */  /* Select key */

   0,                  0,
/* Insert key */  /* Remove key */

LYK_DO_NOTHING,
/* DO_NOTHING*/
};

#endif

struct rmap {
	char *name;
	char *doc;
};

PRIVATE struct rmap revmap[] = {
{ "UNMAPPED",		NULL },
{ "1",			NULL },
{ "2",			NULL },
{ "3",			NULL },
{ "4",			NULL },
{ "5",			NULL },
{ "6",			NULL },
{ "7",			NULL },
{ "8",			NULL },
{ "9",			NULL },
{ "SOURCE",		"toggle source/presentation for current document" },
{ "RELOAD",		"reload the current document" },
{ "PIPE",		"pipe the current document to an external command" },
{ "QUIT",		"quit the browser" },
{ "ABORT",		"quit the browser unconditionally" },
{ "NEXT_PAGE",		"view the next page of the document" },
{ "PREV_PAGE",		"view the previous page of the document" },
{ "UP_TWO",		"go back two lines in the document" },
{ "DOWN_TWO",		"go forward two lines in the document" },
{ "REFRESH",		"refresh the screen to clear garbled text" },
{ "HOME",		"go to the beginning of the current document" },
{ "END",		"go to the end of the current document" },
{ "PREV_LINK",		"make the previous link current" },
{ "NEXT_LINK",		"make the next link current" },
{ "UP_LINK",		"move up the page to a previous link" },
{ "DOWN_LINK",		"move down the page to another link" },
{ "RIGHT_LINK",		"move right to another link" },
{ "LEFT_LINK",		"move left to a previous link" },
{ "HISTORY",		"display a list of previously viewed documents" },
{ "PREV_DOC",		"go back to the previous document" },
{ "ACTIVATE",		"go to the document given by the current link" },
{ "GOTO",		"go to a document given as a URL" },
{ "HELP",		"display help on using the browser" },
{ "INDEX",		"display an index of potentially useful documents" },
{ "RESUBMIT",		"force resubmission of form if presently cached" },
{ "INTERRUPT",		"interrupt network transmission" },
{ "MAIN_MENU",		"return to the first screen" },
{ "OPTIONS",		"display and change option settings" },
{ "INDEX_SEARCH",	"allow searching of an index" },
{ "WHEREIS",		"search within the current document" },
{ "NEXT",		"search for the next occurence" },
{ "COMMENT",		"send a comment to the author of the current document" },
{ "EDIT",		"edit the current document" },
{ "INFO",		"display information on the current document and link" },
{ "PRINT",		"display choices for printing the current document" },
{ "ADD_BOOKMARK",	"add to your personal bookmark list" },
{ "DEL_BOOKMARK",	"delete from your personal bookmark list" },
{ "VIEW_BOOKMARK",	"view your personal bookmark list" },
{ "SHELL",		"escape from the browser to the system" },
{ "DOWNLOAD",		"download the current link to your computer" },
{ "TRACE_TOGGLE",	"toggle tracing of browser operations" },
{ "IMAGE_TOGGLE",       "toggle handling of all images as links" },
{ "DO_NOTHING",		NULL },
{ "TOGGLE_HELP",	"show other commands in the novice help menu" },
{ "JUMP",		"go directly to a target document or action" },
{ "KEYMAP",		"display the current key map" },
{ "EXTERN",             "use an external application" },
{ "DO_NOTHING",		NULL },
{ "DO_NOTHING",		NULL },
{ "DO_NOTHING",		NULL },
{ "DO_NOTHING",		NULL },
{ "DO_NOTHING",		NULL },
{ "DO_NOTHING",		NULL },
{ "VERSION",            "Toggle between real and fake User Agents" },
{ NULL,			"" }
};

PRIVATE char *funckey[] = {
  "Up  Arrow",
  "Down Arrow",
  "Right Arrow",
  "Left Arrow",
  "Page Down",
  "Page Up",
  "Home",
  "End",
  "F1",
  "Do key",
  "Find key",
  "Select key",
  "Insert key",
  "Remove key"
};

PRIVATE char *pretty ARGS1 (int, c)
{
	static char buf[30];

	if (c == '\t')
		sprintf(buf, "&lt;tab&gt;       ");
	else if (c == '\r')
		sprintf(buf, "&lt;return&gt;    ");
	else if (c == ' ')
		sprintf(buf, "&lt;space&gt;     ");
	else if (c == '<')
		sprintf(buf, "&lt;           ");
	else if (c == '>')
		sprintf(buf, "&gt;           ");
	else if (c > ' ' && c < 0177)
		sprintf(buf, "%c", c);
	else if ((c > 407)&&(c < 443))
		sprintf(buf, "Alt-%c", c|0100);
	else if (c == 0177)
//		sprintf(buf, "&lt;delete&gt;    ");
		sprintf(buf, "&lt;backspace&gt; ");
	else if (c < ' ')
		sprintf(buf, "^%c", c|0100);
	else
		sprintf(buf, "%s", funckey[c-0200]);

	return buf;
}


#define KEYTITLE "Current Key Map"

PRIVATE void print_binding ARGS2(FILE *, fp0, int, i)
{
	if (keymap[i] && revmap[keymap[i]].doc)
		fprintf(fp0, "%-12s%-14s%s\n", pretty(i-1),
		revmap[keymap[i]].name, revmap[keymap[i]].doc);
}

PUBLIC void print_keymap ARGS1(char **,newfile)
{
	int i;
	static char *tmpfile=NULL;
	static char kmap_filename[256];
	FILE *fp0;

	if(tmpfile == NULL) {
	    tmpfile = (char *) malloc(128);
	    tempname(tmpfile,NEW_FILE);
	}

	if((fp0 = fopen(tmpfile,"w")) == NULL) {
		perror("Trying to open keymap file\n");
		exit(1);
	}

	/* make the file a URL now */

	sprintf(kmap_filename,"file://localhost/%s",tmpfile);

	StrAllocCopy(*newfile, kmap_filename);

	fprintf(fp0,"<head>\n<title>%s</title>\n</head>\n<body>\n", KEYTITLE);

	fprintf(fp0,"<h1>%s (%s Version %s)</h1>\n<pre>",
			 KEYTITLE, LYNX_NAME, LYNX_VERSION);


	for (i = 'a'+1; i <= 'z'+1; i++) {
	    print_binding(fp0, i);
	    if (keymap[i - ' '] != keymap[i])
	       print_binding(fp0, i-' '); /* uppercase mapping is different */
	}

	for (i = 1; i < sizeof(keymap); i++) {
			      /* LYK_PIPE not implemented yet */
	    if (!isalpha(i-1) && strcmp(revmap[keymap[i]].name, "PIPE"))
		print_binding(fp0, i);
	}

	fprintf(fp0,"</pre>\n</body>\n");

	fclose(fp0);
}

 /*
  * install func as the mapping for key.
  * func must be present in the revmap table.
  * returns TRUE if the mapping was made, FALSE if not.
  */
PUBLIC BOOLEAN remap ARGS2(char *,key, char *,func)
{
       int i;
       struct rmap *mp;
       int c = 0;

       if (func == NULL)
	       return FALSE;
       if (strlen(key) == 1)
	       c = *key;
       else if (strlen(key) == 2 && *key == '^')
	       c = key[1] & 037;
       else if (strlen(key) >= 2 && isdigit(*key))
	       if (sscanf(key, "%i", &c) != 1)
		       return FALSE;
       for (i = 0, mp = revmap; (*mp).name != NULL; mp++, i++) {
	       if (strcmp((*mp).name, func) == 0) {
		       keymap[c+1] = i;
		       return TRUE;
	       }
       }
       return FALSE;
 }


PUBLIC void set_vms_keys NOARGS
{
      keymap[26+1] = LYK_ABORT;  /* control-Z */
      keymap['$'+1] = LYK_SHELL;  
} 

static char saved_vi_keys[4];
static BOOLEAN did_vi_keys;

PUBLIC void set_vi_keys NOARGS
{
      saved_vi_keys[0] = keymap['h'+1];
      keymap['h'+1] = LYK_PREV_DOC;
      saved_vi_keys[1] = keymap['j'+1];
      keymap['j'+1] = LYK_NEXT_LINK;
      saved_vi_keys[2] = keymap['k'+1];
      keymap['k'+1] = LYK_PREV_LINK;
      saved_vi_keys[3] = keymap['l'+1];
      keymap['l'+1] = LYK_ACTIVATE;

      did_vi_keys = TRUE;
}

PUBLIC void reset_vi_keys NOARGS
{
      if (!did_vi_keys)
              return;

      keymap['h'+1] = saved_vi_keys[0];
      keymap['j'+1] = saved_vi_keys[1];
      keymap['k'+1] = saved_vi_keys[2];
      keymap['l'+1] = saved_vi_keys[3];

      did_vi_keys = FALSE;
}

static char saved_emacs_keys[4];
static BOOLEAN did_emacs_keys;

PUBLIC void set_emacs_keys NOARGS
{
      saved_emacs_keys[0] = keymap[2+1];
      keymap[2+1] = LYK_PREV_DOC;       /* ^B */
      saved_emacs_keys[1] = keymap[14+1];
      keymap[14+1] = LYK_NEXT_LINK;     /* ^N */
      saved_emacs_keys[2] = keymap[16+1];
      keymap[16+1] = LYK_PREV_LINK;     /* ^P */
      saved_emacs_keys[3] = keymap[6+1];
      keymap[6+1] = LYK_ACTIVATE;         /* ^F */

      did_emacs_keys = TRUE;
}

PUBLIC void reset_emacs_keys NOARGS
{
      if (!did_emacs_keys)
              return;

      keymap[2+1] = saved_emacs_keys[0];
      keymap[14+1] = saved_emacs_keys[1];
      keymap[16+1] = saved_emacs_keys[2];
      keymap[6+1] = saved_emacs_keys[3];

      did_emacs_keys = FALSE;
}

static char saved_number_keys[9];
static BOOLEAN did_number_keys;

PUBLIC void set_numbers_as_arrows NOARGS
{
      saved_number_keys[0] = keymap['4'+1];
      keymap['4'+1] = LYK_PREV_DOC;
      saved_number_keys[1] = keymap['2'+1];
      keymap['2'+1] = LYK_NEXT_LINK;
      saved_number_keys[2] = keymap['8'+1];
      keymap['8'+1] = LYK_PREV_LINK;
      saved_number_keys[3] = keymap['6'+1];
      keymap['6'+1] = LYK_ACTIVATE;
      saved_number_keys[4] = keymap['7'+1];
      keymap['7'+1] = LYK_HOME;
      saved_number_keys[5] = keymap['1'+1];
      keymap['1'+1] = LYK_END;
      saved_number_keys[6] = keymap['9'+1];
      keymap['9'+1] = LYK_PREV_PAGE;
      saved_number_keys[7] = keymap['3'+1];
      keymap['3'+1] = LYK_NEXT_PAGE;

	/* disable the 5 */
      saved_number_keys[8] = keymap['5'+1];
      keymap['5'+1] = LYK_DO_NOTHING;

      did_number_keys = TRUE;
}

PUBLIC void reset_numbers_as_arrows NOARGS
{
      if (!did_number_keys)
              return;

      keymap['4'+1] = saved_number_keys[0];
      keymap['2'+1] = saved_number_keys[1];
      keymap['8'+1] = saved_number_keys[2];
      keymap['6'+1] = saved_number_keys[3];
      keymap['7'+1] = saved_number_keys[4];
      keymap['1'+1] = saved_number_keys[5];
      keymap['9'+1] = saved_number_keys[6];
      keymap['3'+1] = saved_number_keys[7];
      keymap['5'+1] = saved_number_keys[8];

      did_number_keys = FALSE;
}

PUBLIC char *key_for_func ARGS1 (int,func)
{
	static char buf[512];
	int i;

	buf[0] = '\0';
	for (i = 1; i < sizeof(keymap); i++) {
		if (keymap[i] == func) {
			if (*buf)
				strcat(buf, " or ");
			strcat(buf, pretty(i-1));
		}
	}
	return buf;
}
