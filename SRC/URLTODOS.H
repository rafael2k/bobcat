#ifndef __URLTODOS_H
//	Avoid include redundancy
#define __URLTODOS_H

//	Copyright (c) 1994, University of Kansas, All Rights Reserved
//
//	Include File:	urltodos.h
//	Purpose:	convert a url local path to a dos path
//	Remarks/Portability/Dependencies/Restrictions:
//	Revision History:
//		01-18-94	created

//	Constant defines

//	Required includes

//	Class declarations
extern "C" void urltodos(char *cp_url);
extern void dostourl(char *cp_dest, const char *cp_dospath);
extern int URLisLocal(const char *cp_URL);

//	Global variable declarations

//	Macros

#endif // __URLTODOS_H
