//	Copyright (c) 1994, University of Kansas, All Rights Reserved
//
//	Include File:	capalloc.h
//	Purpose:	Kludge the C memory functions to call new and delete.
//	Remarks/Portability/Dependencies/Restrictions:
//		Functions are callable by both C and C++ source.
//	Revision History:
//		02-25-94	created
//#include"capalloc.h"
#include<string.h>

extern "C"	{

void *capmalloc(size_t size)	{
//	Purpose:	Mimic malloc with new.
//	Arguments:	size	The amount of memory to dynamically allocate.
//	Return Value:	void *	A pointer to the allocated memory or
//				NULL on failure.
//	Remarks/Portability/Dependencies/Restrictions:
//	Revision History:
//		02-25-94	created

	return((void *)new char[size]);
}

void *capcalloc(size_t nitems, size_t size)	{
//	Purpose:	Mimic calloc with new.
//	Arguments:      nitems	The number of items of size to allocate.
//			size	The amount of memory to dynamically allocate
//				for each item.
//	Return Value:	void *	A pointer to the allocated memory or
//				NULL on failure.
//	Remarks/Portability/Dependencies/Restrictions:
//	Revision History:
//		02-25-94	created

	//	Allocate and initialize the block.
	char *cp_block = new char[nitems * size];
	if(cp_block == NULL)	{
		return(NULL);
	}

	for(size_t count = 0; count < nitems * size; count++)	{
		*(cp_block + count) = '\0';
	}

	return((void *)cp_block);
}

void capfree(void *block)	{
//	Purpose:	Mimic free with delete.
//	Arguments:	block	A pointer to the memory to be freed.
//	Return Value:	void
//	Remarks/Portability/Dependencies/Restrictions:
//		Memory to be freed is expected to be created using one of
//		the above functions.  If it is not, results, as I know of,
//		will be undefined.
//	Revision History:
//		02-25-94	created

	delete[]((char *)block);
}



void *caprealloc(void *block, size_t size)	{
//	Purpose:	Mimic realloc with new and delete.
//	Arguments:	block	A pointer to the memory to resize.
//			size	The new size of the block.
//	Return Value:	void *	A pointer to the new block of size, or
//				NULL if can't resize the block.
//	Remarks/Portability/Dependencies/Restrictions:
//		If block is NULL to begin with, acts just like malloc.
//		If size is 0, then will do nothing and return NULL.
//	Revision History:
//		02-25-94	created

	if(block == NULL)	{
		return(capmalloc(size));
	}
	else if(size == 0)	{
		return(NULL);
	}

	//	First allocate a new block, if can't return.
	void *newblock = capmalloc(size);
	if(newblock == NULL)
		return(NULL);

	//	Since really don't know size of old block, copy over size
	//	bytes from oldblock to new block.
	memcpy(newblock, block, size);

	//	Release the old block, return the new.
	capfree(block);
	return(newblock);
}

}; // extern "C"