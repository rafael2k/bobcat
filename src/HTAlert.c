/*	Displaying messages and getting input for Lynx Browser
**	==========================================================
**
**	REPLACE THIS MODULE with a GUI version in a GUI environment!
**
** History:
**	   Jun 92 Created May 1992 By C.T. Barker
**	   Feb 93 Simplified, portablised TBL
**
*/

// #include <dos.h>
#include <unistd.h>
#include "HTUtils.h"
#include "HTAlert.h"
#include "LYString.h"
#include "LYUtils.h"
#include "LYSignal.h"
#include "GridText.h"
#include "LYGlobal.h"

#include "LYLeaks.h"


PUBLIC void HTAlert ARGS1(CONST char *, Msg)
{
#ifdef DT
    if(TRACE) {
        fprintf(stderr, "\nAlert!: %s\n\n", (char *)Msg);
	fflush(stderr);
    } else
#endif
        _user_message("Alert!:  %s", (char *)Msg);

    if(user_mode == ADVANCED_MODE)
	sleep(sleep_two);
    else
	sleep(sleep_three);
}


PUBLIC void HTProgress ARGS1(CONST char *, Msg)
{
#ifdef DT
    if(TRACE)
        fprintf(stderr, "%s\n", (char *)Msg);
    else
#endif
        statusline((char *)Msg);
}


PUBLIC BOOL HTConfirm ARGS1(CONST char *, Msg)
{
    if (dump_output_immediately) { /* Non-interactive, can't respond */
	return(NO);
    } else {
	int c;
#ifdef VMS
	extern BOOLEAN HadVMSInterrupt;
#endif /* VMS */
	
	_user_message("WWW: %s (y/n) ", (char *) Msg);
	
	while(1) {
	    c = LYgetch();
#ifdef VMS
	    if(HadVMSInterrupt) {
		HadVMSInterrupt = FALSE;
		c = 'N';
	    }
#endif /* VMS */
	    if(TOUPPER(c)=='Y')
		return(YES);
	    if(TOUPPER(c)=='N' || c == 7 || c == 3) /* ^G or ^C cancels */
		return(NO);
	}
    }
}

/*	Prompt for answer and get text back
*/
PUBLIC char * HTPrompt ARGS2(CONST char *, Msg, CONST char *, deflt)
{
    char * rep = 0;
    char Tmp[200];

    Tmp[0]='\0';

    _statusline((char *)Msg);
    if (deflt)
	strcpy(Tmp, deflt);

    LYgetstr(Tmp, VISIBLE);

    StrAllocCopy(rep, Tmp);

    return rep;
}


/*      Prompt for password without echoing the reply
*/
PUBLIC char * HTPromptPassword ARGS1(CONST char *,Msg)
{
    char * rep = 0;
    char Tmp[120];

    Tmp[0]='\0';

    _statusline(Msg ? (char *)Msg : "Password: ");

    LYgetstr(Tmp, HIDDEN); /* hidden */

    StrAllocCopy(rep, Tmp);

    return rep;
}


/*      Prompt both username and password       HTPromptUsernameAndPassword()
**      ---------------------------------
** On entry,
**      Msg             is the prompting message.
**      *username and
**      *password       are char pointers; they are changed
**                      to point to result strings.
**
**                      If *username is not NULL, it is taken
**                      to point to  a default value.
**                      Initial value of *password is
**                      completely discarded.
**
** On exit,
**      *username and *password point to newly allocated
**      strings -- original strings pointed to by them
**      are NOT freed.
**
*/
PUBLIC void HTPromptUsernameAndPassword ARGS3(CONST char *,     Msg,
                                              char **,          username,
                                              char **,          password)
{
    if (authentication_info[0]) { /* -auth parameter gives us the values to
 				     use */
 	StrAllocCopy(*username, authentication_info[0]);
	StrAllocCopy(*password, authentication_info[1]);
    } else {
	if (Msg) {
	    *username = HTPrompt(Msg, *username);
	} else {
	    *username = HTPrompt("Username: ", *username);
	}
	*password = HTPromptPassword("Password: ");
    }
}

