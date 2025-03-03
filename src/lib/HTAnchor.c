/*	Hypertext "Anchor" Object				HTAnchor.c
**	==========================
**
** An anchor represents a region of a hypertext document which is linked to
** another anchor in the same or a different document.
**
** History
**
**         Nov 1990  Written in Objective-C for the NeXT browser (TBL)
**	24-Oct-1991 (JFG), written in C, browser-independant 
**	21-Nov-1991 (JFG), first complete version
**
**	(c) Copyright CERN 1991 - See Copyright.html
*/

#define HASH_SIZE 101U           /* Arbitrary prime. Memory/speed tradeoff */

#include "HTUtils.h"
#include "tcp.h"
#include <ctype.h>
#include "HTAnchor.h"
#include "HTParse.h"

#include "LYLeaks.h"

/*
 *	This is the hashing function used to determine which list in the
 *		adult_table a parent anchor should be put in.  This is a
 *		much simpler function than the original used.
 */
/*
#define HASH_FUNCTION(cp_address) ((unsigned short int)strlen(cp_address)
	(unsigned short int)TOUPPER(*cp_address) % HASH_SIZE)
*/

#ifndef FIXME
/*
 *	This is the original function.  We'll use it again. - FM
 */ 
PRIVATE int HASH_FUNCTION ARGS1 (CONST char *, cp_address)
{
    int hash;
    unsigned char *p;

    for(p=(unsigned char *)cp_address, hash=0; *p; p++)
    	hash = (int) (hash * 3 + (*(unsigned char*)p)) % HASH_SIZE;

    return hash;
}
#endif

typedef struct _HyperDoc Hyperdoc;

PRIVATE HTList **adult_table=NULL;  /* Point to table of lists of all parents */

/*				Creation Methods
**				================
**
**	Do not use "new" by itself outside this module. In order to enforce
**	consistency, we insist that you furnish more information about the
**	anchor you are creating : use newWithParent or newWithAddress.
*/

PRIVATE HTParentAnchor * HTParentAnchor_new
  NOARGS
{
  HTParentAnchor *newAnchor = 
    (HTParentAnchor *) calloc (1, sizeof (HTParentAnchor));  /* zero-filled */
  newAnchor->parent = newAnchor;
  newAnchor->isISMAPScript = FALSE; /* Lynx appends ?0,0 if TRUE. - FM */
  return newAnchor;
}

PRIVATE HTChildAnchor * HTChildAnchor_new
  NOARGS
{
  return (HTChildAnchor *) calloc (1, sizeof (HTChildAnchor));  /* zero-filled */
}


/*	Case insensitive string comparison
**	----------------------------------
** On entry,
**	s	Points to one string, null terminated
**	t	points to the other.
** On exit,
**	returns	YES if the strings are equivalent ignoring case
**		NO if they differ in more than  their case.
*/

PRIVATE BOOL equivalent
  ARGS2 (CONST char *,s, CONST char *,t)
{
  if (s && t) {  /* Make sure they point to something */
    for ( ; *s && *t ; s++, t++) {
        if (TOUPPER(*s) != TOUPPER(*t))
	  return NO;
    }
    return TOUPPER(*s) == TOUPPER(*t);
  } else
    return s == t;  /* Two NULLs are equivalent, aren't they ? */
}


/*	Case sensitive string comparison
**	----------------------------------
** On entry,
**	s	Points to one string, null terminated
**	t	points to the other.
** On exit,
**	returns	YES if the strings are identical or both NULL
**		NO if they differ.
*/

PRIVATE BOOL identical
  ARGS2 (CONST char *,s, CONST char *,t)
{
  if (s && t) {  /* Make sure they point to something */
    for ( ; *s && *t ; s++, t++) {
        if (*s != *t)
	  return NO;
    }
    return *s == *t;
  } else
    return s == t;  /* Two NULLs are identical, aren't they ? */
}


/*	Create new or find old sub-anchor
**	---------------------------------
**
**	Me one is for a new anchor being edited into an existing
**	document. The parent anchor must already exist.
*/

extern HTChildAnchor * HTAnchor_findChild
  (HTParentAnchor *parent, CONST char *tag)
{
  auto HTChildAnchor *child;
  auto HTList *kids;

  if (!parent) {
#ifdef DT
    if (TRACE) fprintf(stderr, "HTAnchor_findChild called with NULL parent.\n");
#endif
    return NULL;
  }
  if (kids = parent->children) {  /* parent has children : search them */
    if (tag && *tag) {		/* TBL */
        while (child=HTList_nextObject(kids)) {
	    if (identical(child->tag, tag)) { /* Case sensitive - FM */
#ifdef DT
		if (TRACE) fprintf (stderr,
	       "Child anchor %p of parent %p with name `%s' already exists.\n",
		    (void*)child, (void*)parent, tag);
#endif

		return child;
	    }
	}
     }  /*  end if tag is void */
  } else  /* parent doesn't have any children yet : create family */
    parent->children = HTList_new();

  child = HTChildAnchor_new();
#ifdef DT
  if (TRACE) fprintf(stderr, "new Anchor %p named `%s' is child of %p\n",
       (void*)child, (int)tag ? tag : (CONST char *)"" , (void*)parent); /* int for apollo */
#endif

  HTList_addObject (parent->children, child);
  child->parent = parent;
  StrAllocCopy(child->tag, tag);
  return (child);
}


/*	Create or find a child anchor with a possible link
**	--------------------------------------------------
**
**	Create new anchor with a given parent and possibly
**	a name, and possibly a link to a _relatively_ named anchor.
**	(Code originally in ParseHTML.h)
*/
extern HTChildAnchor * HTAnchor_findChildAndLink
(
       HTParentAnchor *parent, /* May not be 0 */
       CONST char *tag,        /* May be "" or 0 */
       CONST char *href,       /* May be "" or 0 */
       HTLinkType *ltype       /* May be 0 */
)
{
  auto HTChildAnchor * child = HTAnchor_findChild(parent, tag);

#ifdef DT
  if(TRACE)
      fprintf(stderr,"Entered HTAnchor_findChildAndLink Parent: %s Tag: %s Href: %s Ltype%s\n",
        parent,tag,href,ltype);
#endif

  if (href && *href) {
    auto char *relative_to = HTAnchor_address((HTAnchor *)parent);
    DocAddress parsed_doc;
    HTAnchor * dest;

    parsed_doc.address = HTParse(href, relative_to, PARSE_ALL);
    parsed_doc.post_data = 0;
    parsed_doc.post_content_type = 0;
    dest = HTAnchor_findAddress(&parsed_doc);

    HTAnchor_link((HTAnchor *) child, dest, ltype);
    free(parsed_doc.address);
    free(relative_to);
  }
  return child;
}


/*	Create new or find old named anchor
**	-----------------------------------
**
**	Me one is for a reference which is found in a document, and might
**	not be already loaded.
**	Note: You are not guaranteed a new anchor -- you might get an old one,
**	like with fonts.
*/

HTAnchor * HTAnchor_findAddress ARGS1 (CONST DocAddress *,newdoc)
{
  char *tag = HTParse (newdoc->address, "", PARSE_ANCHOR);  /* Anchor tag specified ? */

#ifdef DT
  if(TRACE)
	fprintf(stderr,"Entered HTAnchor_findAddress\n");
#endif

  /* If the address represents a sub-anchor, we recursively load its parent,
     then we create a child anchor within that document. */
  if (*tag) {
    DocAddress parsed_doc;
    HTParentAnchor * foundParent;
    HTChildAnchor * foundAnchor;

    parsed_doc.address = HTParse(newdoc->address, "", 
		PARSE_ACCESS | PARSE_HOST | PARSE_PATH | PARSE_PUNCTUATION);
    parsed_doc.post_data = newdoc->post_data;
    parsed_doc.post_content_type = newdoc->post_content_type;
    
    foundParent = (HTParentAnchor *) HTAnchor_findAddress (&parsed_doc);
    foundAnchor = HTAnchor_findChild (foundParent, tag);
    free (parsed_doc.address);
    free (tag);
    return (HTAnchor *) foundAnchor;
  }
  
  else { /* If the address has no anchor tag, 
	    check whether we have this node */
    int hash;
    CONST char *p;
    HTList * adults;
    HTList *grownups;
    HTParentAnchor * foundAnchor;

    free (tag);
    
    /* Select list from hash table */
    hash = HASH_FUNCTION(newdoc->address);
    if (!adult_table)
        adult_table = (HTList**) calloc(HASH_SIZE, sizeof(HTList*));
    if (!adult_table[hash])
        adult_table[hash] = HTList_new();
    adults = adult_table[hash];

    /* Search list for anchor */
    grownups = adults;
    while (NULL != (foundAnchor=(HTParentAnchor*)HTList_nextObject(grownups))) {
#ifdef NOT_DEFINED
       if (equivalent(foundAnchor->address, newdoc->address) &&
	   equivalent(foundAnchor->post_data, newdoc->post_data)) {
#endif /* NOT_DEFINED */
       if (identical(foundAnchor->address, newdoc->address) &&
	   identical(foundAnchor->post_data, newdoc->post_data)) {
#ifdef DT
	if (TRACE) fprintf(stderr, "Anchor %p with address `%s' already exists.\n",
			  (void*) foundAnchor, newdoc->address);
#endif

	return (HTAnchor *) foundAnchor;
      }
    }
    
    /* Node not found : create new anchor */
    foundAnchor = HTParentAnchor_new ();
#ifdef DT
    if (TRACE) fprintf(stderr, "New anchor %p has hash %d and address `%s'\n",
    	(void*)foundAnchor, hash, newdoc->address);
#endif

    StrAllocCopy(foundAnchor->address, newdoc->address);
    if(newdoc->post_data)
	StrAllocCopy(foundAnchor->post_data, newdoc->post_data);
    if(newdoc->post_content_type)
	StrAllocCopy(foundAnchor->post_content_type, newdoc->post_content_type);
    HTList_addObject (adults, foundAnchor);
    return (HTAnchor *) foundAnchor;
  }
}


/*	Delete an anchor and possibly related things (auto garbage collection)
**	--------------------------------------------
**
**	The anchor is only deleted if the corresponding document is not loaded.
**	All outgoing links from parent and children are deleted, and this anchor
**	is removed from the sources list of all its targets.
**	We also try to delete the targets whose documents are not loaded.
**	If this anchor's source list is empty, we delete it and its children.
*/

PRIVATE void deleteLinks
  ARGS1 (HTAnchor *,me)
{
	/*
	 *	Memory leaks fixed.
	 *	05-27-94 Lynx 2-3-1 Garrett Arch Blythe
	 */

	/*
	 *	Anchor is NULL, do nothing.
	 */
	if(!me)	{
		return;
	}

	/*
	 *	Unregister me with our mainLink destination anchor's parent.
	 */
	if(me->mainLink.dest)	{
		auto HTParentAnchor *parent = me->mainLink.dest->parent;

		/*
		 *	Remove me from the parent's sources so that the
		 *		parent knows one less anchor is it's dest.
		 */
		if(!HTList_isEmpty(parent->sources))	{
			/*
			 *	Really should only need to deregister once.
			 */
			HTList_removeObject(parent->sources, (void *)me);
		}

		/*
		 *	Test here to avoid calling overhead.
		 *	If the parent has no loaded document, then we should
	 	 *		tell it to attempt to delete itself.
		 *	Don't do this jass if the anchor passed in is the same
		 *		as the anchor to delete.
		 *	Also, don't do this if the destination parent is our
		 *		parent.
		 */
		if(!parent->document && parent != (HTParentAnchor *)me
			&& me->parent != parent)	{
			HTAnchor_delete(parent);
		}

		/*
		 *	At this point, we haven't a mainLink.  Set it to be
		 *		so.
		 *	Leave the HTAtom pointed to by type up to other code to
		 *		handle (reusable, near static).
		 */
		me->mainLink.dest = NULL;
		me->mainLink.type = NULL;
	}

	/*
	 *	Check for extra destinations in our links list.
	 */
	if(!HTList_isEmpty(me->links))	{
		auto HTLink *target;
		auto HTParentAnchor *parent;

		/*
		 *	Take out our extra non mainLinks one by one, calling
		 *		their parents to know that they are no longer
		 *		the destination of me's anchor.
	 	 */
		while(target = (HTLink *)HTList_removeLastObject(me->links))
		{
			parent = target->dest->parent;

			if(!HTList_isEmpty(parent->sources))	{
				/*
				 *	Only need to tell destination parent
				 *		anchor once.
				 */
				HTList_removeObject(parent->sources,
					(void *)me);
			}

			/*
			 *	Avoid calling overhead.
			 *	If the parent hasn't a loaded document, then
			 *		we will attempt to have the parent
			 *		delete itself.
			 *	Don't call twice if this is the same anchor
			 *		that we are trying to delete.
			 *	Also, don't do this if we are trying to delete
			 *		our parent.
			 */
			if(!parent->document && (HTParentAnchor *)me
				!= parent && me->parent != parent)	{
				HTAnchor_delete(parent);
			}
		}

		/*
		 *	At this point, me no longer has any destination in
		 *		the links list.  Get rid of it.
		 */
		if(me->links)	{
			HTList_delete(me->links);
			me->links = NULL;
		}
	}

	/*
	 *	Catch in case links list exists but nothing in it.
	 */
	if(me->links)	{
		HTList_delete(me->links);
		me->links = NULL;
	}
}

PUBLIC BOOL HTAnchor_delete
  ARGS1 (HTParentAnchor *,me)
{
	/*
	 *	Memory leaks fixed.
	 *	05-27-94 Lynx 2-3-1 Garrett Arch Blythe
	 */
	auto HTChildAnchor *child;

	/*
	 *	Do nothing if nothing to do.
	 */
	if(!me)	{
		return(NO);
	}

	/*
	 *	Don't delete if document is loaded or being loaded.
	 */
	if(me->document || me->underway) {
		return(NO);
	}

	/*
	 *	Recursively try to delete destination anchors of this parent.
	 *	In any event, this will tell all destination anchors that we
	 *		no longer consider them a destination.
	 */
	deleteLinks((HTAnchor *)me);

	/*
	 *	There are still incoming links to this one (we are the
	 *		destination of another anchor).
	 *	Don't actually delete this anchor, but children are OK to
	 *		delete their links.
	 */
	if(!HTList_isEmpty(me->sources))	{
		/*
		 *	Delete all outgoing links from children, do not
		 *		delete the children though.
		 */
		if(!HTList_isEmpty(me->children))	{
			while(child = (HTChildAnchor *)HTList_removeLastObject(
				me->children))	{
				if(child != NULL)	{
					deleteLinks((HTAnchor *)child);
				}
			}
		}

		/*
		 *	Can't delete parent, still have sources.
		 */
		return(NO);
	}

	/*
	 *	No more incoming links : kill everything
	 *	First, recursively delete children and their links.
	 */
	if(!HTList_isEmpty(me->children))	{
		while(child = (HTChildAnchor *)HTList_removeLastObject(me->
			children))	{
			if(child)	{
				deleteLinks((HTAnchor *)child);
				if(child->tag)	{
					free(child->tag);
				}
				free(child);
			}
		}
	}

	/*
	 *	Delete our empty list of children.
	 */
	if(me->children)	{
		HTList_delete(me->children);
	}
	/*
	 *	Delete our empty list of sources.
	 */
	if(me->sources)	{
		HTList_delete(me->sources);
	}
	/*
	 *	Delete the methods list.
	 */
	if(me->methods)	{
		/*
	 	 *	Leave what the methods point to up in memory for
		 *		other code (near static stuff).
		 */
		HTList_delete(me->methods);
	}

	/*
	 *	Free up all allocated members.
	 */
	if(me->isIndexAction)	{
		free(me->isIndexAction);
	}

	if(me->isIndexPrompt)	{
		free(me->isIndexPrompt);
	}

	if(me->title)	{
		free(me->title);
	}
	if(me->physical)	{
		free(me->physical);
	}
	if(me->post_data)	{
		free(me->post_data);
	}
	if(me->post_content_type)	{
		free(me->post_content_type);
	}
	if(me->owner)	{
		free(me->owner);
	}

	/*
	 *	Remove ourselves from the hash table's list.
	 */
	if(adult_table)	{
		auto unsigned short int usi_hash = HASH_FUNCTION(me->address);
		if(adult_table[usi_hash])	{
			HTList_removeObject(adult_table[usi_hash], (void *)me);
		}
	}

	/*
	 *	Original code wanted a way to clean out the HTFormat if no
	 *		longer needed (ref count?).  I'll leave it alone since
	 *		those HTAtom objects are a little harder to know where
	 *		they are being referenced all at one time. (near static)
 	 */

	/* free the address too */
	/* LJM 12-5-94 */
	if(me->address)
	    free(me->address);
	/*
	 *	Finally, kill the parent anchor passed in.
	 */
	free(me);
}


/*		Move an anchor to the head of the list of its siblings
**		------------------------------------------------------
**
**	This is to ensure that an anchor which might have already existed
**	is put in the correct order as we load the document.
*/

void HTAnchor_makeLastChild
  ARGS1(HTChildAnchor *,me)
{
  if (me->parent != (HTParentAnchor *) me) {  /* Make sure it's a child */
    HTList * siblings = me->parent->children;
    HTList_removeObject (siblings, me);
    HTList_addObject (siblings, me);
  }
}

/*	Data access functions
**	---------------------
*/

PUBLIC HTParentAnchor * HTAnchor_parent
  ARGS1 (HTAnchor *,me)
{
  return me ? me->parent : NULL;
}

void HTAnchor_setDocument
  ARGS2 (HTParentAnchor *,me, HyperDoc *,doc)
{
  if (me)
    me->document = doc;
}

HyperDoc * HTAnchor_document
  ARGS1 (HTParentAnchor *,me)
{
  return me ? me->document : NULL;
}


/* We don't want code to change an address after anchor creation... yet ?
void HTAnchor_setAddress
  ARGS2 (HTAnchor *,me, char *,addr)
{
  if (me)
    StrAllocCopy (me->parent->address, addr);
}
*/

char * HTAnchor_address
  ARGS1 (HTAnchor *,me)
{
  char *addr = NULL;
  if (me) {
    if (((HTParentAnchor *) me == me->parent) ||
    	!((HTChildAnchor *) me)->tag) {  /* it's an adult or no tag */
      StrAllocCopy (addr, me->parent->address);
    }
    else {  /* it's a named child */
      addr = malloc (2 + strlen (me->parent->address)
		     + strlen (((HTChildAnchor *) me)->tag));
      if (addr == NULL) outofmem(__FILE__, "HTAnchor_address");
      sprintf (addr, "%s#%s", me->parent->address,
	       ((HTChildAnchor *) me)->tag);
    }
  }
  return addr;
}



void HTAnchor_setFormat
  ARGS2 (HTParentAnchor *,me, HTFormat ,form)
{
  if (me)
    me->format = form;
}

HTFormat HTAnchor_format
  ARGS1 (HTParentAnchor *,me)
{
  return me ? me->format : NULL;
}



void HTAnchor_setIndex
  ARGS2 (HTParentAnchor *,me, char *,address)
{
  if (me) {
    me->isIndex = YES;
    StrAllocCopy(me->isIndexAction, address);
  }
}

void HTAnchor_setPrompt
  ARGS2 (HTParentAnchor *,me, char *,prompt)
{
  if (me) {
    StrAllocCopy(me->isIndexPrompt, prompt);
  }
}

BOOL HTAnchor_isIndex
  ARGS1 (HTParentAnchor *,me)
{
  return me ? me->isIndex : NO;
}



BOOL HTAnchor_hasChildren
  ARGS1 (HTParentAnchor *,me)
{
  return me ? ! HTList_isEmpty(me->children) : NO;
}

/*	Title handling
*/
CONST char * HTAnchor_title
  ARGS1 (HTParentAnchor *,me)
{
  return me ? me->title : 0;
}

void HTAnchor_setTitle
  ARGS2(HTParentAnchor *,me, CONST char *,title)
{
  StrAllocCopy(me->title, title);
}

void HTAnchor_appendTitle
  ARGS2(HTParentAnchor *,me, CONST char *,title)
{
  StrAllocCat(me->title, title);
}

/*	Owner handling
*/
CONST char * HTAnchor_owner
  ARGS1 (HTParentAnchor *,me)
{
  return (me ? me->owner : 0);
}

void HTAnchor_setOwner
  ARGS2(HTParentAnchor *,me, CONST char *,owner)
{
  StrAllocCopy(me->owner, owner);
}

/*	Link me Anchor to another given one
**	-------------------------------------
*/

BOOL HTAnchor_link
  ARGS3(HTAnchor *,source, HTAnchor *,destination, HTLinkType *,type)
{
  if (! (source && destination))
    return NO;  /* Can't link to/from non-existing anchor */
#ifdef DT
  if (TRACE) fprintf(stderr,
  		     "Linking anchor %p to anchor %p\n", source, destination);
#endif

  if (! source->mainLink.dest) {
    source->mainLink.dest = destination;
    source->mainLink.type = type;
  } else {
    HTLink * newLink = (HTLink *) calloc (1, sizeof (HTLink));
    if (newLink == NULL) outofmem(__FILE__, "HTAnchor_link");
    newLink->dest = destination;
    newLink->type = type;
    if (! source->links)
      source->links = HTList_new ();
    HTList_addObject (source->links, newLink);
  }
  if (!destination->parent->sources)
    destination->parent->sources = HTList_new ();
  HTList_addObject (destination->parent->sources, source);
  return YES;  /* Success */
}


/*	Manipulation of links
**	---------------------
*/

HTAnchor * HTAnchor_followMainLink
  ARGS1 (HTAnchor *,me)
{
  return me->mainLink.dest;
}

HTAnchor * HTAnchor_followTypedLink
  ARGS2 (HTAnchor *,me, HTLinkType *,type)
{
  if (me->mainLink.type == type)
    return me->mainLink.dest;
  if (me->links) {
    HTList *links = me->links;
    HTLink *link;
    while (NULL != (link=(HTLink*)HTList_nextObject(links)))
      if (link->type == type)
	return link->dest;
  }
  return NULL;  /* No link of me type */
}


/*	Make main link
*/
BOOL HTAnchor_makeMainLink
  ARGS2 (HTAnchor *,me, HTLink *,movingLink)
{
  /* Check that everything's OK */
  if (! (me && HTList_removeObject (me->links, movingLink)))
    return NO;  /* link not found or NULL anchor */
  else {
    /* First push current main link onto top of links list */
    HTLink *newLink = (HTLink*) calloc (1, sizeof (HTLink));
    if (newLink == NULL) outofmem(__FILE__, "HTAnchor_makeMainLink");
    memcpy ((void *)newLink, (CONST char *)&me->mainLink, sizeof (HTLink));
    HTList_addObject (me->links, newLink);

    /* Now make movingLink the new main link, and free it */
    memcpy ((void *)&me->mainLink, (CONST void *)movingLink, sizeof (HTLink));
    free (movingLink);
    return YES;
  }
}


/*	Methods List
**	------------
*/

PUBLIC HTList * HTAnchor_methods ARGS1(HTParentAnchor *, me)
{
    if (!me->methods) {
        me->methods = HTList_new();
    }
    return me->methods;
}

/*	Protocol
**	--------
*/

PUBLIC void * HTAnchor_protocol ARGS1(HTParentAnchor *, me)
{
    return me->protocol;
}

PUBLIC void HTAnchor_setProtocol ARGS2(HTParentAnchor *, me,
	void*,	protocol)
{
    me->protocol = protocol;
}

/*	Physical Address
**	----------------
*/

PUBLIC char * HTAnchor_physical ARGS1(HTParentAnchor *, me)
{
    return me->physical;
}

PUBLIC void HTAnchor_setPhysical ARGS2(HTParentAnchor *, me,
	char *,	physical)
{
    StrAllocCopy(me->physical, physical);
}
