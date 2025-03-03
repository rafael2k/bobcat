
#include <stdio.h>
#include <alloc.h>

#define NUM_PTRS  10
#define NUM_BYTES 16

void howmuchheap()
{
   struct farheapinfo heap;
   long total = farcoreleft();
   heap.ptr = NULL;
   while(farheapwalk(&heap) != _HEAPEND)
   if(!heap.in_use) total += heap.size;
   printf("\nHeap left: %ld\n",total);
}
