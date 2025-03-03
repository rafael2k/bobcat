/* global variable definitions */

#ifndef LYGLOBALDEFS_H
#define LYGLOBALDEFS_H

#ifndef USERDEFS_H
#include "userdefs.h"
#endif /* USERDEFS_H */

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif /* HTUTILS_H */

#ifndef LYSTRUCTS_H
#include "LYStruct.h"
#endif /* LYSTRUCTS_H */

#ifndef LYCOPYRIGHT_H
#include "LYCopyRi.h"
#endif /* LYCOPYRIGHT_H */

extern char *TEMP_SPACE;

#define NUMBERS_AS_ARROWS 0
#define LINKS_ARE_NUMBERED 1

#define NOVICE_MODE 	  0
#define INTERMEDIATE_MODE 1
#define ADVANCED_MODE 	  2
extern BOOLEAN LYUseNoviceLineTwo;  /* True if TOGGLE_HELP is not mapped */

#define MAX_LINE        512     /* Hope that no widow is larger than this */
extern char star_string[MAX_LINE + 1]; /* from GridText.c */
#define STARS(n) (&star_string[(MAX_LINE-1) - (n)])
#define DIRNAMESIZE 256
extern BOOLEAN LYShowCursor;   /* show the cursor or hide it */
extern BOOLEAN LYCursesON;  /* start_curses()->TRUE, stop_curses()->FALSE */
extern BOOLEAN LYUserSpecifiedURL;  /* URL from a goto or document? */
extern BOOLEAN LYJumpFileURL;   /* URL from the jump file shorcuts? */
extern BOOLEAN jump_buffer;     /* TRUE to use recall buffer for shortcuts */
extern int more;  /* is there more document to display? */
extern int HTCacheSize;  /* the number of documents cached in memory */
#if defined(VMS) && defined(VAXC) && !defined(__DECC)
extern int HTVirtualMemorySize; /* bytes allocated and not yet freed  */
#endif /* VMS && VAXC && !__DECC */
extern int display_lines; /* number of lines in the display */
extern int www_search_result;
extern char *checked_box;  /* form boxes */
extern char *unchecked_box;  /* form boxes */
extern char *empty_string;
extern char *startfile;
extern char *helpfile;
extern char *display;
extern char *language;
// extern char *inews_path;
extern BOOLEAN LYforce_HTML_mode;
extern BOOLEAN LYforce_no_cache;
extern BOOLEAN user_mode; /* novice or advanced */
extern BOOLEAN is_www_index;
extern BOOLEAN dump_output_immediately;
extern BOOLEAN lynx_mode;
extern BOOLEAN bold_headers;
#ifdef IGNORE_CTRL_C
extern BOOLEAN sigint;
#endif /* IGNORE_CTRL_C */

#ifdef VMS
extern BOOLEAN UseFixedRecords; /* convert binary files to FIXED 512 records */
#endif /* VMS */

#ifdef DIRED_SUPPORT
extern BOOLEAN lynx_edit_mode;
extern BOOLEAN dir_list_style;
extern taglink *tagged;
#define FILES_FIRST 1
#define MIXED_STYLE 2
#endif

extern BOOLEAN recent_sizechange;
extern BOOLEAN telnet_ok;
// extern BOOLEAN news_ok;
extern BOOLEAN ftp_ok;
extern BOOLEAN rlogin_ok;
extern BOOLEAN no_print;    /* TRUE to disable printing */
extern BOOLEAN system_editor; /* True if locked-down editor */
#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
extern BOOLEAN local_exec;  /* TRUE to enable local program execution */
        /* TRUE to enable local program execution in local files only */
extern BOOLEAN local_exec_on_local_files; 
#endif /* defined(EXEC_LINKS) || defined(EXEC_SCRIPTS) */
extern BOOLEAN log_urls;	  /* TRUE to write urls to history.htm */
extern BOOLEAN child_lynx;	  /* TRUE to exit with an arrow */
extern BOOLEAN error_logging;     /* TRUE to mail error messages */
extern BOOLEAN check_mail;        /* TRUE to report unread/new mail messages */
extern BOOLEAN vi_keys;           /* TRUE to turn on vi-like key movement */
extern BOOLEAN emacs_keys;        /* TRUE to turn on emacs-like key movement */
extern BOOLEAN keypad_mode;       /* is set to either NUMBERS_AS_ARROWS or
				   * LINKS_ARE_NUMBERED 
				   */
extern BOOLEAN case_sensitive;    /* TRUE to turn on case sensitive search */

extern BOOLEAN no_inside_telnet;  /* this and following are restrictions */
extern BOOLEAN no_outside_telnet;
extern BOOLEAN no_telnet_port;
// extern BOOLEAN no_inside_news;  
// extern BOOLEAN no_outside_news;
extern BOOLEAN no_inside_ftp;
extern BOOLEAN no_outside_ftp;
extern BOOLEAN no_inside_rlogin;
extern BOOLEAN no_outside_rlogin;
extern BOOLEAN no_suspend;
extern BOOLEAN no_editor;
extern BOOLEAN no_shell;
extern BOOLEAN no_bookmark;
extern BOOLEAN no_option_save;
extern BOOLEAN no_print;
extern BOOLEAN no_download;
extern BOOLEAN no_disk_save;
extern BOOLEAN no_exec;
extern BOOLEAN no_lynxcgi;
extern BOOLEAN exec_frozen;
extern BOOLEAN no_bookmark_exec;
extern BOOLEAN no_goto;
extern BOOLEAN no_jump;
extern BOOLEAN no_file_url;
// extern BOOLEAN no_newspost;
extern BOOLEAN no_mail;
extern BOOLEAN nodotfiles;
extern BOOLEAN local_host_only;

#ifdef DIRED_SUPPORT
extern BOOLEAN no_dired_support;
#endif /* DIRED_SUPPORT */

extern char *indexfile;
extern char *personal_mail_address;
extern char *homepage;	      /* startfile or command line argument */
extern char *editor;          /* if non empty it enables edit mode with
				   * the editor that is named */
extern char *jumpfile;
extern char *bookmark_page;
extern char *personal_type_map;
extern char *global_type_map;
extern char *global_extension_map;
extern char *personal_extension_map;

extern BOOLEAN keep_mime_headers;    /* Return mime headers */
extern BOOLEAN no_url_redirection;   /* Don't follow URL redirections */
extern char *form_post_data;         /* User data for post form */
extern char *form_get_data;          /* User data for get form */
extern char *http_error_file;        /* Place HTTP status code in this file */
extern char *authentication_info[2]; /* Id:Password for protected forms */

#ifndef VMS
extern char *http_proxy_putenv_cmd;
extern char *ftp_proxy_putenv_cmd;
extern char *gopher_proxy_putenv_cmd;
// extern char *news_proxy_putenv_cmd;
extern char *wais_proxy_putenv_cmd;
extern char *no_proxy_putenv_cmd;
#endif /* !VMS */
extern char *smtp_server;
extern char *chomedir;
extern char *cdirbuffer;
extern BOOLEAN clickable_images;

extern BOOLEAN use_color;

extern int normal_fore;
extern int normal_back;
extern unsigned long normal_bold;

extern int underline_fore;
extern int underline_back;
extern unsigned long underline_bold;

extern int reverse_fore;
extern int reverse_back;
extern unsigned long reverse_bold;

extern int bold_fore;
extern int bold_back;
extern unsigned long bold_bold;

extern char *incoming;
extern char *LYUserAgent;               /* Lynx User-Agent */
extern char *LYUserAgentFake;        /* Lynx Fake User-Agent */

extern sleep_one; /* variable sleep times */
extern sleep_two; /* variable sleep times */
extern sleep_three; /* variable sleep times */
#endif /* LYGLOBALDEFS_H */
