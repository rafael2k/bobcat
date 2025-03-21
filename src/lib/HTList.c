/*	A small List class					      HTList.c
**	==================
**
**	A list is represented as a sequence of linked nodes of type HTList.
**	The first node is a header which contains no object.
**	New nodes are inserted between the header and the rest of the list.
*/

#include "HTUtils.h"
#include "HTList.h"

/*#include <stdio.h> included by HTUtils.h -- FM *//* joe@athena, TBL 921019 */

#include "LYLeaks.h"

HTList * HTList_new NOARGS
{
  HTList *newList = (HTList *)calloc(sizeof(HTList), 1);
  if (newList == NULL) outofmem(__FILE__, "HTList_new");
  newList->object = NULL;
  newList->next = NULL;
  return newList;
}

void HTList_delete ARGS1(HTList *,me)
{
  HTList *current;
  while (current = me) {
    me = me->next;
    free (current);
  }
}

void HTList_addObject ARGS2(HTList *,me, void *,newObject)
{
  if (me) {
    HTList *newNode = (HTList *)calloc(sizeof(HTList), 1);
    if (newNode == NULL) outofmem(__FILE__, "HTList_addObject");
    newNode->object = newObject;
    newNode->next = me->next;
    me->next = newNode;
  }
#ifdef DT
  else
    if (TRACE) fprintf(stderr,
        "HTList: Trying to add object %p to a nonexisting list\n",
		       newObject);
#endif
}

BOOL HTList_removeObject ARGS2(HTList *,me, void *,oldObject)
{
  if (me) {
    HTList *previous;
    while (me->next) {
      previous = me;
      me = me->next;
      if (me && me->object == oldObject) {
	previous->next = me->next;
	free (me);
	return YES;  /* Success */
      }
    }
  }
  return NO;  /* object not found or NULL list */
}

PUBLIC void * HTList_removeObjectAt  ARGS2(
	HTList *,	me,
	int,		position)
{
    HTList * temp = me;
    HTList * prevNode;
    int pos = position;

    if (!temp || pos < 0)
	return NULL;

    prevNode = temp;
    while ((temp = temp->next)) {
	if (pos == 0) {
	    prevNode->next = temp->next;
	    prevNode = temp;
	    free(temp);
	    return prevNode->object;
	}
	prevNode = temp;
	pos--;
    }

    return NULL;  /* Reached the end of the list */
}

void * HTList_removeLastObject ARGS1 (HTList *,me)
{
  if (me && me->next) {
    HTList *lastNode = me->next;
    void * lastObject = lastNode->object;
    me->next = lastNode->next;
    free (lastNode);
    return lastObject;
  } else  /* Empty list */
    return NULL;
}

void * HTList_removeFirstObject ARGS1 (HTList *,me)
{
  if(!me)
    return NULL;

  if (me->next) {
    HTList * prevNode;
    void *firstObject;
    while (me->next) {
      prevNode = me;
      me = me->next;
    }
    firstObject = me->object;
    prevNode->next = NULL;
    free (me);
    return firstObject;
  } else  /* Empty list */
    return NULL;
}

int HTList_count ARGS1 (HTList *,me)
{
  int count = 0;
  if (me)
    while (me = me->next)
      count++;
  return count;
}

int HTList_indexOf ARGS2(HTList *,me, void *,object)
{
  if (me) {
    int position = 0;
    while (me = me->next) {
      if (me->object == object)
	return position;
      position++;
    }
  }
  return -1;  /* Object not in the list */
}

void * HTList_objectAt ARGS2 (HTList *,me, int,position)
{
  if (position < 0)
    return NULL;
  if (me) {
    while (me = me->next) {
      if (position == 0)
	return me->object;
      position--;
    }
  }
  return NULL;  /* Reached the end of the list */
}
