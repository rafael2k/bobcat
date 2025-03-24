/*
 * Lynx - Hypertext navigation system
 *
 *   (c) Copyright 1992, 1993, 1994 University of Kansas
 */

/*******************************************************************
 * There are three sections to this document
 *  Section 1.  Things you MUST change or verify
 *
 *  Section 2.  Things you should probably check!
 *
 *  Section 3.  Things you should only change after you have a good
 *              understanding of the program!
 *
 */

#ifndef USERDEFS_H
#define USERDEFS_H

/*******************************************************************
 * Things you must change
 *  Section 1. 
 */

/**************************
 * LYNX_CFG_FILE is the location and name of the default lynx
 * global configuration file.  It is sought and processed at
 * startup of Lynx, followed by a seek and processing of a
 * personal RC file (lynxrc in the user's HOME directory,
 * created if the user saves values in the 'o'ptions menu).
 * You also can define the location and name of the global
 * configuration file via an environment variable, "LYNX_CFG",
 * which will override the "LYNX_CFG_FILE" definition here.
 * The -cfg command line switch will override these definitions.
 *
 * Note that any SUFFIX or VIEWER mappings in the configuration
 * files will be overidden by any suffix or viewer mappings
 * that are established as defaults in src/HTInit.c.
 *
 */
#define LYNX_CFG_FILE "lynx.cfg"

/**************************
 * The EXTENSION_MAP file allows you to map file suffix's to 
 * mime types.
 * These global and personal files override anything in
 * lynx.cfg or src/HTInit.c
 */
#define GLOBAL_EXTENSION_MAP "mime.typ"
#define PERSONAL_EXTENSION_MAP "pmime.typ"

/**************************
 * The MAILCAP file allows you to map file MIME types to 
 * external viewers.
 * These global and personal files override anything in
 * lynx.cfg or src/HTInit.c
 */
#define GLOBAL_MAILCAP "mailcap"
#define PERSONAL_MAILCAP "pmailcap"

/*********************
 * LOCAL_DOMAIN is used to determine if a user is local
 * to your campus or organization
 *
 * Now a variable.
 *
 */

//#define LOCAL_DOMAIN "foo.com"                /* CHANGE THIS! */

/**************************
 * the full path and name of the telnet command
 */
#define TELNET_COMMAND "telnet"

/**************************
 * the full path and name of the tn3270 command
 */
#define TN3270_COMMAND "tn3270"

/**************************
 * the full path and name of the rlogin command
 */
#define RLOGIN_COMMAND "rlogin"

/*************************
 * This define will be used for a default in src/HTInit.c.
 * Make it the full path and name of the xloadimage command.
 * Put 'echo' or something like it here if you don't have it.
 * It can be anything that will handle GIF, TIFF and other
 * popular image formats (xv does).
 * You must also have a "%s" for the filename; "&" for
 * background is optional
 */
// #define XLOADIMAGE_COMMAND "xv %s &"
#define XLOADIMAGE_COMMAND "echo"

/*************************
 * The full path and name of the inews program
 *
 * A "mini" inews has been included in the utils directory.
 *
 * set empty or to "none" if you don't have or want it.
 */
#define INEWS ""

/**************************
 * A place to put temporary files, it's almost always "/tmp/" on
 * UNIX systems
 *
 * Now a variable.
 *
 */
//#define TEMP_SPACE "/tmp/"

/*****************************
 * STARTFILE is the default file if none is specified on the command line 
 * 
 * note: STARTFILE must be a URL.  See the Lynx online help for more
 *       information on URL's
 */
#define STARTFILE "http://64.227.13.248/"
//  "http://www.frogfind.com/http://frogfind.com/"
/* #define STARTFILE "http://www.w3.org/default.html" */
/* #define STARTFILE "http://kufacts.cc.ukans.edu/cwis/kufacts_start.html" */

/*****************************
 *
 * HELPFILE must be defined as a URL and must have a 
 * complete local path name if local 
 * (file://localhost/DIRECTORY/FILENAME
 *  replace DIRECTORY with the current directory path and
 *  FILENAME with the name of the file.
 *  file://localhost/dua#/DIRECTORY/FILENAME on VMS systems.)
 * the default HELPFILE is:
 * http://kufacts.cc.ukans.edu/lynx_help/lynx_help_main.html
 * This file will be updated as needed.
 */
#define HELPFILE "http://web.mit.edu/infoagents/src/lynx-2.7.1/lynx_help/Lynx_users_guide.html"

/*****************************
 * JUMPFILE is the local file checked for shortcut URL's when the
 * user presses the 'J' (JUMP) key.  The user will be prompted for
 * a shortcut entry (analogously to 'g'oto), and can enter one
 * or use '?' for a list of the shortcuts with associated links to
 * their actual URL's.  See the sample jumps files in the samples
 * subdirectory.  Make sure your jumps file includes a '?' shortcut
 * for a file://localhost URL to itself:
 *
 * <dt>?<dd><a href="file://localhost/path/jumps.html">This Shortcut List</a>
 *
 * If not defined here or in lynx.cfg, the JUMP command will invoke
 * the NO_JUMPFILE statusline message.
 *
 * On VMS, use Unix SHELL syntax (including a lead slash) to define it.
 *
 * Do not include "file://localhost" in the definition.
 */
/* #define JUMPFILE "/Lynx_Dir/jumps.html" */

/*****************************
 * DEFAULT_INDEX_FILE is the default file retrieved when the
 * user presses the 'I' key when viewing any document.
 * An index to your CWIS can be placed here or a document containing
 * pointers to lots of interesting places on the web.
 */
#define DEFAULT_INDEX_FILE "http://frogfind.com/"

/********************************
* The DEFAULT_CACHE_SIZE specifies the number of WWW documents to be
* cached in memory at one time.
*
* This so-called cache size (actually, number) may be modified with the
* command line argument -cache=NUMBER
*
*/
#define DEFAULT_CACHE_SIZE 3

/*****************************
 * PREFERRED_LANGUAGE is the language in MIME notation (e.g., "en",
 * "fr") which will be indicated by Lynx in its Accept-Language headers
 * as the preferred language.  If available, the document will be
 * transmitted in that language.  This definition can be overriden via
 * lynx.cfg.  Users also can change it via the 'o'ptions menu and save
 * that preference in their RC file.
 */
#define PREFERRED_LANGUAGE "en"

/****************************************************************
 *   Section 2.   Things that you probably want to change or review
 *
 */

/*****************************
 * Enter the name of your anonymous account if you have one
 * as ANONYMOUS_USER.  UNIX systems will use a cuserid
 * or get_login call to determine if the current user is
 * the ANONYMOUS_USER.
 *
 * VMS systems cannot use this feature, so they must specify
 * anonymous accounts using the "-anonymous" command line option.
 *
 * Other systems may use the "-anonymous" option for multiple
 * accounts or precautionary reasons as well.
 *
 * It is very important to have this correctly defined if you 
 * have an anonymous account.  If you do not you will be putting 
 * yourself at GREAT security risk!
 *
 * Later on in this file you can specify privileges for the
 * anonymous account.
 */
#define ANONYMOUS_USER "" 

/******************************
 * SHOW_CURSOR controls whether or not the cursor is hidden
 * or appears over the link.  This is just the default, it
 * can be turned on with the -show_cursor command line option.
 * Showing the cursor is handy if you have really stupid terminals
 * that can't do bold and reverse video at the same time or at all.
 */
#define SHOW_CURSOR FALSE

/******************************
 * BOXVERT and BOXHORI control the layout of popup menus.  Set to 0 if your
 * curses supports line-drawing characters, set to '*' or any other character
 * to not use line-drawing (e.g., '|' for vertical and '-' for horizontal).
 */
// #define BOXVERT '*'
#define BOXVERT 0
// #define BOXHORI '*'
#define BOXHORI 0

/*******************************
 * set to FALSE if you don't want users of your anonymous account
 * who are calling from inside your local domain 
 * to be able to telnet back out
 */
#define CAN_ANONYMOUS_INSIDE_DOMAIN_TELNET	  TRUE  

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account who are calling from outside your
 * local domain to be able to telnet back out
 */
#define CAN_ANONYMOUS_OUTSIDE_DOMAIN_TELNET      TRUE 

/*******************************
 * set to FALSE if you don't want users of your anonymous account
 * who are calling from inside your local domain
 * to be able to read news
 */
#define CAN_ANONYMOUS_INSIDE_DOMAIN_READ_NEWS     TRUE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account who are calling from outside your
 * local domain to be able to read news
 */
#define CAN_ANONYMOUS_OUTSIDE_DOMAIN_READ_NEWS    FALSE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to print
 */
#define CAN_ANONYMOUS_PRINT            TRUE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to mail
 */
#define CAN_ANONYMOUS_MAIL	       TRUE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to goto random URL's. (The 'g' command)
 */
#define CAN_ANONYMOUS_GOTO		TRUE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to jump to URL's (The 'J' command)
 * via the shortcut entries in your JUMPFILE.
 */
#define CAN_ANONYMOUS_JUMP		TRUE

/*******************************
 * set to FALSE if you don't want to recall the previous shortcut
 * when using the 'J'ump command (if TRUE, will recall as it does
 * for the 'g'oto command)
 */
#define JUMPBUFFER	  TRUE  

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account who are calling from inside your local domain
 * to be able to use ftp
 */
#define CAN_ANONYMOUS_INSIDE_DOMAIN_FTP	  TRUE  

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account who are calling from outside your local domain
 * to be able to use ftp
 */
#define CAN_ANONYMOUS_OUTSIDE_DOMAIN_FTP      TRUE 

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account who are calling from inside your local domain 
 * to be able to use rlogin
 */
#define CAN_ANONYMOUS_INSIDE_DOMAIN_RLOGIN	  TRUE  

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account who are calling from outside your local domain
 * to be able to use rlogin
 */
#define CAN_ANONYMOUS_OUTSIDE_DOMAIN_RLOGIN      TRUE 

/*******************************
 * Execution links/scripts configuration.
 *
 * Execution links and scripts allow you to run
 * local programs by activating links within Lynx.
 *
 * An execution link is of the form:
 *
 *     lynxexec:<COMMAND>
 * or:
 *     lynxexec://<COMMAND>
 *
 * where <COMMAND> is a command that Lynx will
 * run when the link is activated.
 * The double-slash should be included if the
 * command begins with an '@', as for executing
 * VMS command files.  Otherwise, the double-
 * slash can be omitted.
 *
 * Execution scripts take the form of a standard
 * URL.  Extension mapping or MIME typing is used
 * to decide if the file is a script and should be
 * executed.  The current extensions are:
 * .csh, .ksh, and .sh on UNIX systems and .com on
 * VMS systems.  Any time a file of this type is
 * accessed Lynx will look at the user's options
 * settings to decide if the script can be executed.
 * Current options include: Only exec files that
 * reside on the local machine and are referenced
 * with a "file://localhost" URL, All execution
 * off, and all execution on.
 *
 * The following definitions will add execution
 * capabilities to Lynx.  You may define none, one
 * or both.
 *
 * I strongly recommend that you define neither one
 * of these since execution links/scripts can represent
 * very serious security risk to your system and its
 * users.  If you do define these I suggest that
 * you only allow users to execute files/scripts
 * that reside on your local machine. 
 *
 * YOU HAVE BEEN WARNED!
 *
 * Note: if you are enabling execution scripts you should
 * also see src/HTInit.c to verify/change the execution
 * script extensions and/or commands.
 */
/* #define EXEC_LINKS  */ 
/* #define EXEC_SCRIPTS  */ 

/**********
 * CGI script support. Defining LYNXCGI_LINKS allows you to use the 
 *
 *   lynxcgi:path
 *
 * URL which allows lynx to access a cgi script directly without the need for
 * a http daemon. Redirection or mime support is not supported but just about
 * everything else is. If the path is not an executable file then the URL is
 * rewritten as file://localhost and passed to the file loader. This means that
 * if your http:html files are currently set up to use relative addressing, you
 * should be able to fire up your main page with lynxcgi:path and everything
 * should work as if you were talking to the http daemon.
 *
 * Note that you must use a LYNXCGI_PATH directive in your lynx.cfg file as
 * well in order for this to work.
 *
 * The cgi scripts are called with a fork()/execve() sequence so you don't
 * have to worry about people trying to abuse the code. :-)
 *
 *     George Lindholm <George.Lindholm@ubc.ca>
 */
/* #define LYNXCGI_LINKS */

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)

/**********
 * if ALLOW_USERS_TO_CHANGE_EXEC_WITHIN_OPTIONS 
 * is defined then the user will be able to change
 * the execution status within the options screen.
 */
/* #define ALLOW_USERS_TO_CHANGE_EXEC_WITHIN_OPTIONS */

/**********
 * if NEVER_ALLOW_REMOTE_EXEC is defined then local execution 
 * of scripts or lynxexec: URL's will only be implemented from
 * HTML files that were accessed via a "file://localhost/" URL,
 * and the options menu for "L)ocal executions links" will only
 * allow toggling between "ALWAYS OFF" and "FOR LOCAL FILES ONLY".
 */
/* #define NEVER_ALLOW_REMOTE_EXEC */

/*****************************
 * These are for executable shell scripts and links.
 * Set to FALSE unless you really know what you're
 * doing.
 *
 * This only applies if you are compiling with EXEC_LINKS or
 * EXEC_SCRIPTS defined.
 *
 * The first two settings:
 * LOCAL_EXECUTION_LINKS_ALWAYS_ON 
 * LOCAL_EXECUTION_LINKS_ON_BUT_NOT_REMOTE
 * specify the DEFAULT setting of the users execution link
 * options, but the user may still change those options.
 * If you do not wish the user to be able to change the
 * execution link settings you may wish to use the commandline option:
 *    -restrictions=exec_frozen
 *
 * LOCAL_EXECUTION_LINKS_ALWAYS_ON will be FALSE
 * if NEVER_ALLOW_REMOTE_EXEC has been defined.
 *
 * if LOCAL_EXECUTION_LINKS_ALWAYS_OFF_FOR_ANONYMOUS is 
 * true all execution links will be disabled when the
 * -anonymous command line option is used.  Anonymous
 * users are not allowed to change the execution options
 * from within the Lynx options menu so you might be able
 * to use this option to enable execution links and set
 * LOCAL_EXECUTION_LINKS_ON_BUT_NOT_REMOTE to TRUE to
 * give anonymous execution link capability without compromising
 * your system (see comments about TRUSTED_EXEC rules in
 * lynx.cfg for more information).
 *
 */
#define LOCAL_EXECUTION_LINKS_ALWAYS_ON          FALSE
#define LOCAL_EXECUTION_LINKS_ON_BUT_NOT_REMOTE  FALSE
#define LOCAL_EXECUTION_LINKS_ALWAYS_OFF_FOR_ANONYMOUS FALSE

#endif /*  defined(EXEC_LINKS) || defined(EXEC_SCRIPTS) */

/*********************************
 *  MAIL_SYSTEM_ERROR_LOGGING will send a message to the owner of 
 *  the information if there is one, every time
 *  that a document cannot be accessed!
 *
 *  NOTE: This can generate A LOT of mail, be warned.
 */
#define MAIL_SYSTEM_ERROR_LOGGING   FALSE  /*mail a message for every error?*/

/*********************************
 * If CHECKMAIL is set to TRUE, the user will be informed (via a statusline
 * message) about the existence of any unread mail at startup of Lynx, and
 * will get statusline messages if subsequent new mail arrives.  If a jumps
 * file with a lynxexec URL for invoking mail is available, or your html
 * pages include an mail launch file URL, the user thereby can access mail
 * and read the messages.  The checks and statusline reports will not be
 * performed if Lynx has been invoked with the -restrictions=mail switch.
 *
 *  VMS USERS !!!
 * New mail is normally broadcast as it arrives, via "unsolicitied screen
 * broadcasts", which can be "wiped" from the Lynx display via the Ctrl-W
 * command.  You may prefer to disable the broadcasts and use CHECKMAIL
 * instead (e.g., in a public account which will be used by people who
 * are ignorant about VMS).
 */
#define CHECKMAIL	FALSE	/* report unread and new mail messages */

/*********************************
 * VI_KEYS can be turned on by the user in the options
 * screen or the .lynxrc file.  This is just the default.
 */
#define VI_KEYS_ALWAYS_ON           FALSE /* familiar h,j,k, & l */

/*********************************
 * EMACS_KEYS can be turned on by the user in the options
 * screen or the .lynxrc file.  This is just the default.
 */
#define EMACS_KEYS_ALWAYS_ON           FALSE /* familiar ^N, ^P, ^F, ^B */

/*********************************
 * DEFAULT_KEYPAD_MODE specifies whether by default the user
 * has numbers that work like arrows or else numbered links
 * DEFAULT KEYPAD MODE may be set to 
 *	LINKS_ARE_NUMBERED  or
 *	NUMBERS_AS_ARROWS
 */
#define DEFAULT_KEYPAD_MODE	       NUMBERS_AS_ARROWS

/********************************
 * The default search.
 * This is a default that can be overridden by the user!
 */
#define CASE_SENSITIVE_ALWAYS_ON    FALSE /* case sensitive user search */

/********************************
 * If NO_DOT_FILES is TRUE, the user will not be allowed to specify files
 * beginning with a dot in reply to output filename prompts.
 *
 * On VMS, it also will stop inclusion of files beginning with a dot
 * (e.g., file:/localhost/device/directory/.lynxrc) in the directory
 * browser's listings (they are never included on Unix).
 */
#define NO_DOT_FILES    FALSE  /* disallow writing of dot files */


/* Don't let the user enter his/hers email address when sending a message.
 * Anonymous mail makes it far too easy for a user to spoof someone elses
 * email address.
 * This requires that your mailer agent put in the From: field for you.

 * The default should be to uncomment this line but there probably are too
 * many mail agents out there that won't do the right thing if there is no
 * From: line.
 */
/* #define NO_ANONYMOUS_EMAIL TRUE; */ /* Don't allow anonymous From: field */
  
/****************************************************************
 *   Section 3.   Things that you should not change until you
 *  		  have a good knowledge of the program
 */

#define LYNX_NAME "Lynx"
#define LYNX_VERSION "2-4-2 (Bobcat/0.8 [ELKS])"

/* text strings for certain actions */
/* changing these text strings is a way to customize 
 * your environment to better suit your tastes
 */
#define QUIT "Are you sure you want to quit? [Y] "
#define HELP \
 "Commands: Use arrow keys to move, '?' for help, 'q' to quit, '<-' to go back."
#define MOREHELP \
 "-- press space for more, use arrow keys to move, '?' for help, 'q' to quit."
#define MORE "-- press space for next page --"
#define FORM_LINK_TEXT_MESSAGE \
 "(Text entry field) Enter text. Use UP or DOWN arrows or tab to move off."
#define FORM_LINK_PASSWORD_MESSAGE \
 "(Password entry field) Enter text. Use UP or DOWN arrows or tab to move off."
#define FORM_LINK_CHECKBOX_MESSAGE \
 "(Checkbox Field)   Use right-arrow or <return> to toggle."
#define FORM_LINK_SUBMIT_MESSAGE \
 "(Form submit button) Use right-arrow or <return> to submit ('x' for no cache)."
#define FORM_LINK_RESET_MESSAGE \
 "(Form reset button)   Use right-arrow or <return> to reset form to defaults."
#define FORM_LINK_OPTION_LIST_MESSAGE \
 "(Option list) Hit return and use arrow keys and return to select option."
#define NORMAL_LINK_MESSAGE \
 "(NORMAL LINK)   Use right-arrow or <return> to activate."
#define LINK_NOT_FOUND "The resource requested is not available at this time."
#define WWW_WAIT_MESSAGE "Getting %s"
#define ADVANCED_URL_MESSAGE "URL: %s"
#define WWW_FAIL_MESSAGE "Unable to access WWW file!!!"
#define WWW_INDEX_MESSAGE "This is a searchable index.  Use %s to search."
#define WWW_INDEX_MORE_MESSAGE \
 "--More--  This is a searchable index.  Use %s to search."
#define BAD_LINK_NUM_ENTERED "You have entered an invalid link number."
#define SOURCE_HELP \
 "Currently viewing document source.  Press '\\' to return to rendered version."
#define NOVICE_LINE_ONE \
 "  Arrow keys: Up and Down to move. Right to follow a link; Left to go back.  \n"
#define NOVICE_LINE_TWO \
 " H)elp O)ptions P)rint G)o M)ain screen Q)uit /=search [delete]=history list \n"
#define NOVICE_LINE_TWO_A \
 "  O)ther cmds  H)elp  K)eymap  G)oto  P)rint  M)ain screen  o)ptions  Q)uit  \n"
#define NOVICE_LINE_TWO_B \
 "  O)ther cmds  B)ack  E)dit  D)ownload ^R)eload ^W)ipe screen  search doc: / \n"
#define NOVICE_LINE_TWO_C \
 "  O)ther cmds  C)omment  History: <delete>  Bookmarks: V)iew, A)dd, R)emove  \n"
#define FORM_NOVICELINE_ONE \
 "            Enter text into the field by typing on the keyboard              "
#define FORM_NOVICELINE_TWO \
"    Ctrl-U to delete all text in field, [Backspace] to delete a character    "
#define RETURN_TO_LYNX "Press <return> to return to Lynx."
#define NO_JUMPFILE "No jump file is currently available."

#ifdef DIRED_SUPPORT
#define DIRED_NOVICELINE \
 "  C)reate  D)ownload  E)dit  F)ull menu  M)odify  R)emove  T)ag  U)pload     \n"
#endif /* DIRED_SUPPORT */

#define MAXBASE 100       /* max length of base directory */
#define MAXHIGHLIGHT 160 /* max length of highlighted text */
#define MAXTARGET 130    /* max length of target string */
#define LINESIZE 256    /* max length of line to read from file*/
#define MAXFNAME 256	/* max filename length DDD/FILENAME.EXT */
#define MAXCOMMAND MAXFNAME /* max length of command should be the same */
#define MAXHIST  16	/* number of links we remember in history */
#define MAXLINKS 16	/* max links on one screen */
   /* traversal lookup table file, don't worry about it for now */
#define TRAVERSE_FILE "/homea/local/lynx2-4/traverse.file"
#define TRAVERSE_ERRORS "/homea/local/lynx2-4/traverse.errors"
#define TRAVERSE_FOUND_FILE "/homea/local/lynx2-4/traverse.found"

#ifndef VMS
/* Check these paths on Unix. */
#define COMPRESS_PATH   "compress"
#define UNCOMPRESS_PATH "uncompress"
#define UUDECODE_PATH   "uudecode"
#define ZCAT_PATH       "zcat"
#define GZIP_PATH       "gzip"
#define ZIP_PATH        "zip"
#define UNZIP_PATH      "unzip"
#define INSTALL_PATH    "install"
#define MKDIR_PATH      "mkdir"
#define MV_PATH         "mv"
#define RM_PATH         "rm"
#define TAR_PATH        "tar"
#define TOUCH_PATH      "touch"
#define COPY_PATH       "copy"
#define CHMOD_PATH      "chmod"
#endif /* !VMS */

#endif /* USERDEFS_H */
