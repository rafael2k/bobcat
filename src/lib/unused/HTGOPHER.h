/*                                                          Gopher protocol module for libwww
                                      GOPHER ACCESS
                                             
  HISTORY:
  
  8 Jan 92               Adapted from HTTP TBL
                         
 */


#ifndef HTGOPHER_H
#define HTGOPHER_H

#include "HTAccess.h"
#include "HTAnchor.h"

#if defined (GLOBALREF_IS_MACRO)
extern GLOBALREF (HTProtocol, HTGopher);
#else
GLOBALREF HTProtocol HTGopher;
#endif

#endif /* HTGOPHER_H */

/*

   end of gopher module */
