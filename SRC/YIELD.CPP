//	Copyright (c) 1994, University of Kansas, All Rights Reserved
//
//	Include File:	none
//	Purpose:	'C' function to do background stuff while wattcp
//			is running.
//	Remarks/Portability/Dependencies/Restrictions:
//	Revision History:
//		01-14-94	created
#define Uses_TKeys
#include"tdoslynx.h"
#include"tcapture.h"
#include"trace.h"
extern "C"	{
#include"msdostcp.h"
};

extern "C" void yield(void)	{
//	Purpose:	Do idle processing while wattcp waiting for input.
//	Arguments:	void
//	Return Value:	void
//	Remarks/Portability/Dependencies/Restrictions:
//		Requires that this function is registered via a wattcp
//		routine in code elsewhere.
//		This function is called by 'C' code only.
//	Revision History:
//		01-14-94	created

	TEvent TE;

	//	Call our application's getEvent function.
	TDosLynx::application->getEvent(TE);

#ifdef TODO
#error yield:  Change status bar to only handle hot keys for cancel and handle here.
#endif // TODO

	//	Abort transfer on ESC
	if(TE.what & evKeyDown)	{
		if(TE.keyDown.keyCode == kbEsc)	{
#ifndef zRELEASE
			trace("Attempt to abort socket IO.");
#endif // RELEASE
			//	Clear the event.
			TDosLynx::application->clearEvent(TE);
			//	Find out if the socket is valid
			if(ssi_sockfd > -1)	{
				//	Find out if the socket is open
				if(sock_table[ssi_sockfd].usi_OPEN)	{
					//	Find out if the socket had IO
					if(sock_table[ssi_sockfd].usi_IO)
					{
						//	Set the socket to EOF
						//	As an interrupt to IO
						sock_table[ssi_sockfd].usi_EOF
							= 1;
						doslynxmessage("Interrup"
							"ting....  please"
							" wait.");
					}
					else	{
						doslynxmessage("Unable to "
							"interrupt.... no io "
							"has occured.");
					}
				}
				else	{
					doslynxmessage("Unable to "
						"interrupt....  socket not "
						"opened.");
				}
			}
			else	{
				doslynxmessage("Unable to interrupt.... "
					"not a valid socket, try CTRL-C");
			}
		}
	}
	//	Print message on any event except a mouse move and mouse
	//		button release.
	if(TE.what != evNothing && !(TE.what & evMouseMove) && !(TE.what &
		evMouseUp))	{
		doslynxmessage("Networking.... press ESC to interrupt.");
	}

	//	Clear the event message.
	TDosLynx::application->clearEvent(TE);
}
