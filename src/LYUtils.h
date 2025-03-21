
#ifndef LYUTILS_H
#define LYUTILS_H

#include <stdio.h>

/* for tempname */
#define NEW_FILE     0
#define REMOVE_FILES 1

extern void statusline PARAMS((char *text));
extern void noviceline PARAMS((int more));
extern void LYFakeZap PARAMS((BOOL set));
extern void toggle_novice_line NOPARAMS;
extern BOOLEAN LYisLocalFile PARAMS((char *filename));
extern BOOLEAN LYisLocalHost PARAMS((char *filename));
extern int is_url PARAMS((char *filename));
extern void remove_backslashes PARAMS((char *buf));
extern void collapse_spaces PARAMS((char *string));
extern void convert_to_spaces PARAMS((char *string));
extern BOOLEAN inlocaldomain NOPARAMS;
extern void size_change PARAMS((int sig));
extern void change_sug_filename PARAMS((char *fname));
extern void tempname PARAMS((char *namebuffer, int action));
extern int HTCheckForInterrupt NOPARAMS;
extern int number2arrows PARAMS((int number));
extern void highlight PARAMS((int flag, int cur));
extern void parse_restrictions PARAMS((char *s));
extern void free_and_clear PARAMS((char **obj));
extern char * quote_pathname PARAMS((char * pathname));
extern void checkmail NOPARAMS;
extern int LYCheckMail NOPARAMS;

#ifdef VMS
extern void Define_VMSLogical PARAMS((char *LogicalName, char *LogicalValue));
#endif /* VMS */

/*	Whether or not the status line must be shown.
 */
extern BOOLEAN mustshow;
#define _statusline(msg)	mustshow = TRUE, statusline(msg)

/* for is_url */
/* universal document id types */
#define HTTP_URL_TYPE     1
#define FILE_URL_TYPE     2
#define FTP_URL_TYPE      3
#define WAIS_URL_TYPE     4
#define PROSPERO_URL_TYPE 5
#define NEWS_URL_TYPE     6
#define TELNET_URL_TYPE   7
#define TN3270_URL_TYPE   8
#define GOPHER_URL_TYPE   9
#define HTML_GOPHER_URL_TYPE 10
#define TELNET_GOPHER_URL_TYPE 11
#define INDEX_GOPHER_URL_TYPE 12
#define AFS_URL_TYPE      13
#define MAILTO_URL_TYPE   14
#define RLOGIN_URL_TYPE   15

#define NEWSPOST_URL_TYPE      19
#define NEWSREPLY_URL_TYPE     20

#define LYNXPRINT_URL_TYPE     21
#define LYNXHIST_URL_TYPE      22
#define LYNXDOWNLOAD_URL_TYPE  23
#define LYNXEXEC_URL_TYPE      24
#define LYNXCGI_URL_TYPE       25

#ifdef DIRED_SUPPORT
#define LYNXDIRED_URL_TYPE     26
#endif /* DIRED_SUPPORT */

#define ON      1
#define OFF     0
#define STREQ(a,b) (strcmp(a,b) == 0)
#define STRNEQ(a,b,c) (strncmp(a,b,c) == 0)

#ifdef SOLARIS2
#define index strchr
#define rindex strrchr
#endif /* SOLARIS2 */

#endif /* LYUTILS_H */
