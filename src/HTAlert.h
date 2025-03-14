/*  */

/*      Displaying messages and getting input for WWW Library
**      =====================================================
**
**         May 92 Created By C.T. Barker
**         Feb 93 Portablized etc TBL
*/

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif /* HTUTILS_H */
#include "tcp.h"

/*      Display a message and get the input
**
**      On entry,
**              Msg is the message.
**
**      On exit,
**              Return value is malloc'd string which must be freed.
*/
extern char * HTPrompt PARAMS((CONST char * Msg, CONST char * deflt));
extern char * HTPromptPassword PARAMS((CONST char * Msg));
extern void HTPromptUsernameAndPassword PARAMS((CONST char * Msg,
                                                char ** username,
                                                char ** password));


/*      Display a message, don't wait for input
**
**      On entry,
**              The input is a list of parameters for printf.
*/
extern void HTAlert PARAMS((CONST char * Msg));


/*      Display a progress message for information (and diagnostics) only
**
**      On entry,
**              The input is a list of parameters for printf.
*/
extern BOOLEAN mustshow;
extern void HTProgress PARAMS((CONST char * Msg));
#define _HTProgress(msg)	mustshow = TRUE, HTProgress(msg)


/*      Display a message, then wait for 'yes' or 'no'.
**
**      On entry,
**              Takes a list of parameters for printf.
**
**      On exit,
**              If the user enters 'YES', returns TRUE, returns FALSE
**              otherwise.
*/
extern BOOL HTConfirm PARAMS ((CONST char * Msg));
/*

    */
