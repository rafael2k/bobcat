/*                     /Net/dxcern/userd/timbl/hypertext/WWW/Library/Implementation/HTTP.html
                                HYPERTEXT TRANFER PROTOCOL
                                             
 */
#ifndef HTTP_H
#define HTTP_H

#include "HTAccess.h"

#if defined (GLOBALREF_IS_MACRO)
extern GLOBALREF (HTProtocol,HTTP);
extern GLOBALREF (HTProtocol,HTTPS);
#else
GLOBALREF HTProtocol HTTP;
GLOBALREF HTProtocol HTTPS;
#endif

#define URL_GET_METHOD  1
#define URL_POST_METHOD 2
#define URL_MAIL_METHOD 3

#endif /* HTTP_H */

/*

   end of HTTP module definition
   
    */
