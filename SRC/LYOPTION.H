
#ifndef LYOPTIONS_H
#define LYOPTIONS_H

extern void options NOPARAMS;

/* values for options */
#define L_EDITOR    2
#define L_DISPLAY   3
#define L_HOME      4
#define L_FTPSTYPE  5
#define L_MAIL_ADDRESS  6
#define L_SSEARCH   7
#define L_CHARSET   8
#define L_VIKEYS    9
#define L_EMACSKEYS 10
#define L_KEYPAD    11 
#define L_LANGUAGE  13

#ifdef DIRED_SUPPORT
#define L_DIRED     14
#define L_USER_MODE 15
#define L_EXEC      16
#else
#define L_USER_MODE 14
#define L_EXEC      15
#endif

#endif /* LYOPTIONS_H */
