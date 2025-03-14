#ifndef LYSTRINGS_H
#define LYSTRINGS_H

#include <string.h>
#ifdef __STRICT_BSD__
extern char *strstr();
#endif /* __STRICT_BSD__ */

extern int get_mouse_link NOPARAMS;
extern char * LYstrncpy PARAMS((char *dst, char *src, int n));
extern int LYgetch NOPARAMS;
extern int LYgetstr PARAMS((char *inputline, int hidden));
			    /* int bufsize, int recall)); */
extern char * LYstrstr PARAMS((char *chptr, char *tarptr));
extern char * LYno_attr_char_strstr PARAMS((char *chptr, char *tarptr));
extern char * LYno_attr_char_case_strstr PARAMS((char *chptr, char *tarptr));

#ifdef EXP_CHARTRANS
extern char * LYno_attr_mbcs_strstr PARAMS((
	char *		chptr,
	char *		tarptr,
	BOOL		utf_flag,
	int *		nstartp,
	int *		nendp));
extern char * LYno_attr_mbcs_case_strstr PARAMS((
	char *		chptr,
	char *		tarptr,
	BOOL		utf_flag,
	int *		nstartp,
	int *		nendp));
extern int LYmbcsstrlen PARAMS((
	char *		str,
	BOOL		utf_flag));
extern char * LYmbcsstrncpy PARAMS((
	char *		dst,
	char *		src,
	int		n_bytes,
	int		n_glyphs,
	int		enc));
#else
#define LYmbcsstrlen(str,utf_flag) strlen(str)
#define LYmbcsstrncpy(dest,src,n,n_glyphs,enc) LYstrncpy(dest, src, n)
#endif

extern char * SNACopy PARAMS((char **dest, CONST char *src, int n));
extern char * SNACat PARAMS((char **dest, CONST char *src, int n));

#define StrnAllocCopy(dest, src, n)  SNACopy (&(dest), src, n)
#define StrnAllocCat(dest, src, n)   SNACat  (&(dest), src, n)

#define printable(c) (((c)>31 && (c)<=255) || (c)==9 || (c)==10 || (c)<0 )

#ifdef NEWLYNX
/* values for LYgetch */
#define UPARROW		256	/* 0x100 */
#define DNARROW		257	/* 0x101 */
#define RTARROW		258	/* 0x102 */
#define LTARROW		259	/* 0x103 */
#define PGDOWN		260	/* 0x104 */
#define PGUP		261	/* 0x105 */
#define HOME		262	/* 0x106 */
#define END		263	/* 0x107 */
#define F1		264	/* 0x108 */
#define DO_KEY		265	/* 0x109 */
#define FIND_KEY	266	/* 0x10A */
#define SELECT_KEY	267	/* 0x10B */
#define INSERT_KEY	268	/* 0x10C */
#define REMOVE_KEY	269	/* 0x10D */
#define DO_NOTHING	270	/* 0x00E */
#else
#define UPARROW 128
#define DNARROW 129
#define RTARROW 130
#define LTARROW 131
#define PGDOWN  132
#define PGUP    133
#define HOME    134
#define END     135
#define F1      136
#define DO_KEY     137
#define FIND_KEY   138
#define SELECT_KEY 139
#define INSERT_KEY 140
#define REMOVE_KEY 141
#define DO_NOTHING 142
#endif

#define VISIBLE  0
#define HIDDEN   1
#define NORECALL 0
#define RECALL   1

/* EditFieldData preserves state between calls to LYEdit1
 */
typedef struct _EditFieldData {
	
        int  sx;        /* Origin of editfield                       */
        int  sy;
        int  dspwdth;   /* Screen real estate for editting           */

        int  strlen;    /* Current size of string.                   */
        int  maxlen;    /* Max size of string, excluding zero at end */
        char pad;       /* Right padding  typically ' ' or '_'       */
        BOOL hidden;    /* Masked password entry flag                */

        BOOL dirty;     /* accumulate refresh requests               */
        BOOL panon;     /* Need horizontal scroll indicator          */
        int  xpan;      /* Horizontal scroll offset                  */
        int  pos;       /* Insertion point in string                 */
        int  margin;    /* Number of columns look-ahead/look-back    */

        char buffer[512]; /* String buffer                          */

} EditFieldData;

/* line-edit action encoding */

#define LYE_NOP 0		  /* Do Nothing            */
#define LYE_CHAR  (LYE_NOP   +1)  /* Insert printable char */
#define LYE_ENTER (LYE_CHAR  +1)  /* Input complete, return char */
#define LYE_TAB   (LYE_ENTER +1)  /* Input complete, return TAB  */
#define LYE_ABORT (LYE_TAB   +1)  /* Input cancelled       */

#define LYE_DELN  (LYE_ABORT +1)  /* Delete next    char   */
#define LYE_DELC  (LYE_DELN  +1)  /* Delete current char   */
#define LYE_DELP  (LYE_DELC  +1)  /* Delete prev    char   */
#define LYE_DELNW (LYE_DELP  +1)  /* Delete next word      */
#define LYE_DELPW (LYE_DELNW +1)  /* Delete prev word      */

#define LYE_ERASE (LYE_DELPW +1)  /* Erase the line        */

#define LYE_BOL   (LYE_ERASE +1)  /* Go to begin of line   */
#define LYE_EOL   (LYE_BOL   +1)  /* Go to end   of line   */
#define LYE_FORW  (LYE_EOL   +1)  /* Cursor forwards       */
#define LYE_BACK  (LYE_FORW  +1)  /* Cursor backwards      */
#define LYE_FORWW (LYE_BACK  +1)  /* Word forward          */
#define LYE_BACKW (LYE_FORWW +1)  /* Word back             */

#define LYE_LOWER (LYE_BACKW +1)  /* Lower case the line   */
#define LYE_UPPER (LYE_LOWER +1)  /* Upper case the line   */

#define LYE_AIX   (LYE_UPPER +1)  /* Hex 97		   */

extern void LYSetupEdit   PARAMS((EditFieldData *edit, char *old, int maxstr,
				  int maxdsp));
extern void LYRefreshEdit PARAMS((EditFieldData *edit));
extern int  LYEdit1       PARAMS((EditFieldData *edit, int ch,
				  int action, BOOL maxMessage));


extern int current_lineedit;
extern char * LYLineeditNames[];
extern char * LYLineEditors[];

/* Push a chacter through the linedit machinery */
#define EditBinding(c) (LYLineEditors[current_lineedit][c])
#define LYLineEdit(e,c,m) LYEdit1(e,c,EditBinding(c),m)

/* Dummy initializer for LYEditmap.c */
extern int LYEditmapDeclared NOPARAMS;

#endif /* LYSTRINGS_H */
