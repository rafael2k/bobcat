#ifndef __CAPTUREALLOCATION_H
/*
 *	Avoid include redundancy
 */
#define __CAPTUREALLOCATION_H

#ifdef __cplusplus
extern "C"	{
#endif /* __cplusplus */

/*
 *	Copyright (c) 1994, University of Kansas, All Rights Reserved
 *
 *	Include File:	capalloc.h
 *	Purpose:	Provide a mechanism to take over the standard C
 *			dynamic memory allocation functions and channel
 *			them all through the common C++ new operator for
 *			enhanced memory management through the new operator
 *			and the set_new_handler C++ function.
 *	Remarks/Portability/Dependencies/Restrictions:
 *		All functions should be callable by both C and C++ code.
 *	Revision History:
 *		02-25-94	created
 */

/*
 *	Required includes
 */
#include<stddef.h>

/*
 *	Constant defines
 */
#define malloc capmalloc
#define calloc capcalloc
#define realloc caprealloc
#define free capfree

/*
 *	Data structures
 */

/*
 *	Global variable declarations
 */

/*
 *	Macros
 */

/*
 *	Function declarations
 */
extern void *capmalloc(size_t size);
extern void *capcalloc(size_t nitems, size_t size);
extern void *caprealloc(void *block, size_t size);
extern void capfree(void *block);

#ifdef __cplusplus
}; /* extern "C" */
#endif /* __cplusplus */

#endif /* __CAPTUREALLOCATION_H */
