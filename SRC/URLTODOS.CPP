//	Copyright (c) 1994, University of Kansas, All Rights Reserved
//
//	Class:		none
//	Include File:	urltodos.h
//	Purpose:	convert a url path to a dos path
//	Remarks/Portability/Dependencies/Restrictions:
//	Revision History:
//		01-18-94	created
#include"urltodos.h"
#include<string.h>
//#include<stdio.h>
//#include<dos.h>

extern "C" void urltodos(char *cp_url)	{
//	Purpose:	convert a url to a dos path
//	Arguments:	cp_url	the url to convert
//	Return Value:	void
//	Remarks/Portability/Dependencies/Restrictions:
//		Will overwrite the cp_url memory in order to provide the new
//			path.
//		Does not actually take a complete url, only the part passed
//			in by a routine in htfile.c
//	Revision History:
//		01-18-94	created


	//	Allocate space for the new name.
	//	Remember the address of the original url
	char *cp_newurl = new char[strlen(cp_url) + 1];
	char *cp_oldurl = cp_url;

	//	Skip the always leading '/'
	cp_url++;

	//	Next comes the drive letter.
	*cp_newurl = *cp_url;
	cp_url++;

	//	If the last character was a drive letter, this will be
	//	a '|', if not, the rest must be a file name.
	if(*cp_url == '|')	{
		*(cp_newurl + 1) = ':';
	}
	else	{
		//	Just copy over another character.
		*(cp_newurl + 1) = *cp_url;
	}

	//	Move past the '|'
	cp_url++;

	//	Copy the rest of the string over.
	strcpy(cp_newurl + 2, cp_url);

	//	Loop through the string, converting all '/' to '\\'
	for(cp_url = cp_newurl; *cp_url != '\0'; cp_url++)	{
		if(*cp_url == '/')	{
			*cp_url = '\\';
		}
	}

	//	Done, copy back over the new url, and release our memory.
	strcpy(cp_oldurl, cp_newurl);
	delete[](cp_newurl);

	//	Convert to lower case.
	strlwr(cp_oldurl);
}

extern void dostourl(char *cp_dest, const char *cp_dospath)	{
//	Purpose:	convert a dos path to a url
//	Arguments:	cp_dest		the buffer to put the url in
//			cp_dospath	the dos path to convert
//	Return Value:	void
//	Remarks/Portability/Dependencies/Restrictions:
//		Will return a full url.  What is returned is generally
//			not passable back to usltodos as the beginning
//			'file://' should be stripped off before passing.
//	Revision History:
//		01-19-94	created

	//	Copy the leading 'file:///' into cp_dest
	strcpy(cp_dest, "file:///");
	//	Append the dos part.
	strcat(cp_dest, cp_dospath);

	//	Go through cp_dest, changing '\\' to '/' and ':' to '|'
	for(int i_traverse = 8; *(cp_dest + i_traverse) != '\0';
		i_traverse++)	{
		switch(*(cp_dest + i_traverse))	{
		case '\\':
			*(cp_dest + i_traverse) = '/';
			break;
		case ':':
			*(cp_dest + i_traverse) = '|';
			break;
		}
	}

	//	convert to lower case.
	strlwr(cp_dest);

}


extern int URLisLocal(const char *cp_URL)	{
//	Purpose:	Determine if a URL is local.
//	Arguments:	cp_URL	the url in question
//	Return Value:	int	0	is not local
//				1	is local
//	Remarks/Portability/Dependencies/Restrictions:
//		Assuming a URL is local on dos only when "file:///" is the
//			prefix of the URL.
//	Revision History:
//		01-19-94	created

	return((strncmp(cp_URL, "file:///", 8) == 0) ? 1 : 0);
}
