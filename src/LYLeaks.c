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
#include<malloc.h>


void *capmalloc(size_t size)	{
//	Purpose:	Mimic malloc with new.
//	Arguments:	size	The amount of memory to dynamically allocate.
//	Return Value:	void *	A pointer to the allocated memory or
//				NULL on failure.
//	Remarks/Portability/Dependencies/Restrictions:
//	Revision History:
//		02-25-94	created
	return malloc(size); // by rafael2k for elks - 2025
	//return((void *)new char[size]);
}

void *capcalloc(size_t nitems, size_t size)	{
    size_t total = nitems * size;
    void *ptr = malloc(total);

    memset(ptr, 0, total);
}

void capfree(void *block)	{
	free(block);
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

    if(size == 0)	
        return(NULL);

    void *newblock = malloc(size);
    
    if (block == NULL)
        return newblock;

    memcpy(newblock, block, size);

    free(block);

    return (newblock);

}
