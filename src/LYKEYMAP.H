
#ifndef LYKEYMAP_H
#define LYKEYMAP_H

extern BOOLEAN remap PARAMS((char *key, char *func));
extern void set_vms_keys NOPARAMS;
extern void set_vi_keys NOPARAMS;
extern void reset_vi_keys NOPARAMS;
extern void set_emacs_keys NOPARAMS;
extern void reset_emacs_keys NOPARAMS;
extern void set_numbers_as_arrows NOPARAMS;
extern void reset_numbers_as_arrows NOPARAMS;
extern void print_keymap PARAMS((char **newfile));
extern char *key_for_func PARAMS((int func));

extern char keymap[]; /* main keymap matrix */

#if defined(DIRED_SUPPORT) && defined(OK_OVERRIDE)
extern char override[];
#endif

#define       LYK_1     	1
#define       LYK_2     	2
#define       LYK_3     	3
#define       LYK_4     	4
#define       LYK_5     	5
#define       LYK_6     	6
#define       LYK_7     	7
#define       LYK_8     	8
#define       LYK_9     	9
#define       LYK_SOURCE        10
#define       LYK_RELOAD        11
#define       LYK_PIPE  	12
#define       LYK_QUIT  	13
#define       LYK_ABORT 	14
#define       LYK_NEXT_PAGE     15
#define       LYK_PREV_PAGE     16
#define       LYK_UP_TWO        17
#define       LYK_DOWN_TWO      18
#define       LYK_REFRESH       19
#define       LYK_HOME  	20
#define       LYK_END   	21
#define       LYK_PREV_LINK     22
#define       LYK_NEXT_LINK     23
#define       LYK_UP_LINK       24
#define       LYK_DOWN_LINK     25
#define       LYK_RIGHT_LINK    26
#define       LYK_LEFT_LINK     27
#define       LYK_HISTORY       28
#define       LYK_PREV_DOC      29
#define       LYK_ACTIVATE      30
#define       LYK_GOTO  	31
#define       LYK_HELP  	32
#define       LYK_INDEX 	33
#define       LYK_RESUBMIT      34
#define       LYK_INTERRUPT     35
#define       LYK_MAIN_MENU     36
#define       LYK_OPTIONS       37
#define       LYK_INDEX_SEARCH  38
#define       LYK_WHEREIS       39
#define       LYK_NEXT  	40
#define       LYK_COMMENT       41
#define       LYK_EDIT  	42
#define       LYK_INFO  	43
#define       LYK_PRINT 	44
#define       LYK_ADD_BOOKMARK  45
#define       LYK_DEL_BOOKMARK  46
#define       LYK_VIEW_BOOKMARK 47
#define       LYK_SHELL 	48
#define       LYK_DOWNLOAD      49
#define       LYK_TRACE_TOGGLE  50
#define       LYK_IMAGE_TOGGLE  51
#define       LYK_DO_NOTHING    52
#define       LYK_TOGGLE_HELP   53
#define       LYK_JUMP          54
#define       LYK_KEYMAP        55
#define       LYK_EXTERN        56

#ifdef DIRED_SUPPORT
#define       LYK_CREATE        56
#define       LYK_REMOVE        57
#define       LYK_MODIFY        58
#define       LYK_TAG_LINK      59
#define       LYK_DIRED_MENU    60
#define       LYK_UPLOAD        61
#define       LYK_INSTALL       62
#endif

#define       LYK_VERSION       63

#ifdef NOT_USED
#define       LYK_FORM_UP       64
#define       LYK_FORM_DOWN     65
#endif /* NOT_USED */

#endif /* LYKEYMAP_H */
