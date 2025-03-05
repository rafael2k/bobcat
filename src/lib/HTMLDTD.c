/*		Our Static DTD for HTML
**		-----------------------
*/

/* Implements:
*/

#include "HTUtils.h"
#include "HTMLDTD.h"
#include "LYLeaks.h"
#include "HTML.h"

/* 	Entity Names
**	------------
**
**	This table must be matched exactly with ALL the translation tables
*/
static CONST char* entities[] = {
  "AElig",	/* capital AE diphthong (ligature) */ 
  "Aacute",	/* capital A, acute accent */ 
  "Acirc",	/* capital A, circumflex accent */ 
  "Agrave",	/* capital A, grave accent */ 
  "Aring",	/* capital A, ring */ 
  "Atilde",	/* capital A, tilde */ 
  "Auml",	/* capital A, dieresis or umlaut mark */ 
  "Ccedil",	/* capital C, cedilla */ 
  "Dstrok",	/* capital Eth, Icelandic */ 
  "ETH",	/* capital Eth, Icelandic */ 
  "Eacute",	/* capital E, acute accent */ 
  "Ecirc",	/* capital E, circumflex accent */ 
  "Egrave",	/* capital E, grave accent */ 
  "Euml",	/* capital E, dieresis or umlaut mark */ 
  "Iacute",	/* capital I, acute accent */ 
  "Icirc",	/* capital I, circumflex accent */ 
  "Igrave",	/* capital I, grave accent */ 
  "Iuml",	/* capital I, dieresis or umlaut mark */ 
  "Ntilde",	/* capital N, tilde */ 
  "Oacute",	/* capital O, acute accent */ 
  "Ocirc",	/* capital O, circumflex accent */ 
  "Ograve",	/* capital O, grave accent */ 
  "Oslash",	/* capital O, slash */ 
  "Otilde",	/* capital O, tilde */ 
  "Ouml",	/* capital O, dieresis or umlaut mark */ 
  "THORN",	/* capital THORN, Icelandic */ 
  "Uacute",	/* capital U, acute accent */ 
  "Ucirc",	/* capital U, circumflex accent */ 
  "Ugrave",	/* capital U, grave accent */ 
  "Uuml",	/* capital U, dieresis or umlaut mark */ 
  "Yacute",	/* capital Y, acute accent */ 
  "aacute",	/* small a, acute accent */ 
  "acirc",	/* small a, circumflex accent */ 
  "acute",	/* spacing acute */
  "aelig",	/* small ae diphthong (ligature) */ 
  "agrave",	/* small a, grave accent */ 
  "amp",	/* ampersand */ 
  "aring",	/* small a, ring */ 
  "atilde",	/* small a, tilde */ 
  "auml",	/* small a, dieresis or umlaut mark */ 
  "brkbar",	/* broken vertical bar */
  "brvbar",	/* broken vertical bar */
  "ccedil",	/* small c, cedilla */ 
  "cedil",	/* spacing cedilla */
  "cent",	/* cent sign */
  "copy",	/* copyright sign */
  "curren",	/* currency sign */
  "deg",	/* degree sign */
  "divide",	/* division sign */
  "eacute",	/* small e, acute accent */ 
  "ecirc",	/* small e, circumflex accent */ 
  "egrave",	/* small e, grave accent */ 
  "emdash",	/* dash the width of emsp */
  "emsp",	/* em space - not collapsed */
  "endash",	/* dash the width of ensp */
  "ensp",	/* en space - not collapsed */
  "eth",	/* small eth, Icelandic */ 
  "euml",	/* small e, dieresis or umlaut mark */ 
  "frac12",	/* fraction 1/2 */
  "frac14",	/* fraction 1/4 */
  "frac34",	/* fraction 3/4 */
  "gt",		/* greater than */ 
  "hibar",	/* spacing macron */
  "iacute",	/* small i, acute accent */ 
  "icirc",	/* small i, circumflex accent */ 
  "iexcl",	/* inverted exclamation mark */
  "igrave",	/* small i, grave accent */ 
  "iquest",	/* inverted question mark */
  "iuml",	/* small i, dieresis or umlaut mark */ 
  "laquo",	/* angle quotation mark, left */
  "lt",		/* less than */ 
  "mdash",	/* dash the width of emsp */
  "micro",	/* micro sign */
  "middot",	/* middle dot */
  "nbsp",       /* non breaking space */
  "ndash",	/* dash the width of ensp */
  "not",	/* negation sign */
  "ntilde",	/* small n, tilde */ 
  "oacute",	/* small o, acute accent */ 
  "ocirc",	/* small o, circumflex accent */ 
  "ograve",	/* small o, grave accent */ 
  "ordf",	/* feminine ordinal indicator */
  "ordm",	/* masculine ordinal indicator */
  "oslash",	/* small o, slash */ 
  "otilde",	/* small o, tilde */ 
  "ouml",	/* small o, dieresis or umlaut mark */ 
  "para",	/* paragraph sign */
  "plusmn",	/* plus-or-minus sign */
  "pound",	/* pound sign */
  "quot",	/* quot '"' */
  "raquo",	/* angle quotation mark, right */
  "reg",	/* circled R registered sign */
  "sect",	/* section sign */
  "shy",	/* soft hyphen */
  "sup1",	/* superscript 1 */
  "sup2",	/* superscript 2 */
  "sup3",	/* superscript 3 */
  "szlig",	/* small sharp s, German (sz ligature) */ 
  "thorn",	/* small thorn, Icelandic */ 
  "times",	/* multiplication sign */ 
  "uacute",	/* small u, acute accent */ 
  "ucirc",	/* small u, circumflex accent */ 
  "ugrave",	/* small u, grave accent */ 
  "uml",	/* spacing diaresis */
  "uuml",	/* small u, dieresis or umlaut mark */ 
  "yacute",	/* small y, acute accent */ 
  "yen",	/* yen sign */
  "yuml",	/* small y, dieresis or umlaut mark */ 
};

#define HTML_ENTITIES 108



/*		Attribute Lists
**		---------------
**
**	Lists must be in alphatbetical order by attribute name
**	The tag elements contain the number of attributes
*/
static attr no_attr[] = 
	{{ 0 }};

static attr a_attr[] = {			/* Anchor attributes */
	{ "HREF"},
	{ "NAME" },				/* Should be ID */
	{ "TITLE" },
	{ "TYPE" },
	{ "URN" },
	{ 0 }	/* Terminate list */
};	

static attr base_attr[] = {			/* BASE attributes */
	{ "HREF"},
	{ 0 }	/* Terminate list */
};	

static attr img_attr[] = {			/* IMG attributes */
	{ "ALT"},
	{ "ISMAP"},
	{ "SRC"},
	{ 0 }	/* Terminate list */
};

static attr frame_attr[HTML_FRAME_ATTRIBUTES+1] = { /* frame attributes */
	{ "NAME" },
	{ "SRC" },
	{ 0 }	/* Terminate list */
};

static attr area_attr[HTML_AREA_ATTRIBUTES+1] = { /* Area attributes */
	{ "ALT" },
	{ "HREF" },
	{ 0 }	/* Terminate list */
};

static attr isindex_attr[] = {			/* ISINDEX attributes */
	{ "ACTION"},	/* Not in spec.  Lynx treats it as HREF. - FM */
	{ "HREF"},	/* HTML 3.0 attritute for search action. - FM */ 
	{ "PROMPT"},	/* HTML 3.0 attribute for prompt string. - FM */
	{ 0 }	/* Terminate list */
};	

static attr link_attr[] = {
	{ "HREF"},
	{ "REL"},
	{ "REV"},
	{ 0 }	/* Terminate list */
};

static attr form_attr[] = {
	{ "ACTION"},
	{ "ENCTYPE"},
	{ "METHOD"},
	{ 0 }	/* Terminate list */
};

static attr select_attr[] = {
	{ "MULTIPLE" },
	{ "NAME" },
	{ "SIZE" },
	{ 0 }	/* Terminate list */
};

static attr option_attr[] = {
	{ "SELECTED"},
	{ "VALUE"},
	{ 0 }	/* Terminate list */
};

static attr textarea_attr[] = {
	{ "COLS"},
	{ "NAME"},
	{ "ROWS"},
	{ 0 }	/* Terminate list */
};

static attr input_attr[] = {
	{ "CHECKED"},
	{ "MAXLENGTH"},
	{ "NAME"},
	{ "SIZE"},
	{ "TYPE"},
	{ "VALUE"},
	{ 0 } /* Terminate list */
};

static attr list_attr[] = {
	{ "COMPACT"},
	{ 0 }	/* Terminate list */
};

static attr glossary_attr[] = {
	{ "COMPACT" },
	{ 0 }	/* Terminate list */
};

static attr nextid_attr[] = {
	{ "N" }
};

static attr mh_attr[]	=	{
	{ "HIDDEN" },
	{ 0 }	/* Terminate list */
};

/*	Elements
**	--------
**
**	Must match definitions in HTMLDTD.html!
**	Must be in alphabetical order.
**
**    Name, 	Attributes, 		content
*/
static HTTag tags[HTML_ELEMENTS] = {
    { "A"	, a_attr,	HTML_A_ATTRIBUTES,	SGML_MIXED },
    { "ADDRESS"	, no_attr,	0,		SGML_MIXED },
    { "APPLET"  , no_attr,      0,              SGML_MIXED },
    { "AREA"    , area_attr,    HTML_AREA_ATTRIBUTES,    SGML_EMPTY },
    { "B"	, no_attr,	0,		SGML_MIXED },
    { "BASE"	, base_attr,	HTML_BASE_ATTRIBUTES,	SGML_EMPTY },
    { "BLOCKQUOTE", no_attr,	0,		SGML_MIXED },
    { "BODY"	, no_attr,	0,		SGML_MIXED },
    { "BR"	, no_attr,	0,		SGML_EMPTY },
    { "CENTER"  , no_attr,      0,              SGML_MIXED },
    { "CITE"	, no_attr,	0,		SGML_MIXED },
    { "CODE"	, no_attr,	0,		SGML_MIXED },
    { "COMMENT",  no_attr,	0,		SGML_MIXED },
    { "DD"	, no_attr,	0,		SGML_EMPTY },
    { "DFN"	, no_attr,	0,		SGML_MIXED },
    { "DIR"	, list_attr,	1,		SGML_MIXED },
    { "DL"	, glossary_attr,1,		SGML_MIXED },
    { "DLC"	, glossary_attr,1,		SGML_MIXED },
    { "DT"	, no_attr,	0,		SGML_EMPTY },
    { "EM"	, no_attr,	0,		SGML_MIXED },
    { "FORM"	, form_attr,	HTML_FORM_ATTRIBUTES,	SGML_MIXED },
    { "FRAME"   , frame_attr,   HTML_FRAME_ATTRIBUTES,  SGML_EMPTY },
    { "H1"	, no_attr,	0,		SGML_MIXED },
    { "H2"	, no_attr,	0,		SGML_MIXED },
    { "H3"	, no_attr,	0,		SGML_MIXED },
    { "H4"	, no_attr,	0,		SGML_MIXED },
    { "H5"	, no_attr,	0,		SGML_MIXED },
    { "H6"	, no_attr,	0,		SGML_MIXED },
    { "H7"	, no_attr,	0,		SGML_MIXED },
    { "HEAD"	, no_attr,	0,		SGML_MIXED },
    { "HR"	, no_attr,	0,		SGML_EMPTY },
    { "HTML"	, no_attr,	0,		SGML_MIXED },
    { "I"	, no_attr,	0,		SGML_MIXED },
    { "IMG"     , img_attr,	HTML_IMG_ATTRIBUTES,	SGML_EMPTY },
    { "INPUT"   , input_attr,	HTML_INPUT_ATTRIBUTES,	SGML_EMPTY },
    { "ISINDEX" , isindex_attr,	HTML_ISINDEX_ATTRIBUTES,SGML_EMPTY },
    { "KBD"	, no_attr,	0,		SGML_MIXED },
    { "LI"	, list_attr,	1,		SGML_EMPTY },
    { "LINK"	, link_attr,	HTML_LINK_ATTRIBUTES,	SGML_EMPTY },
    { "LISTING"	, no_attr,	0,		SGML_LITTERAL },
    { "MAP"     , no_attr,      0,              SGML_EMPTY },
    { "MENU"	, list_attr,	1,		SGML_MIXED },
    { "MH"	, mh_attr,	HTML_MH_ATTRIBUTES,	SGML_LITTERAL },
    { "NEXTID"  , nextid_attr,	1,		SGML_EMPTY },
    { "OL"	, list_attr,	1,		SGML_MIXED },
    { "OPTION"	, option_attr,	HTML_OPTION_ATTRIBUTES,	SGML_EMPTY },
    { "P"	, no_attr,	0,		SGML_EMPTY },
    { "PLAINTEXT", no_attr,	0,		SGML_LITTERAL },
    { "PRE"	, no_attr,	0,		SGML_MIXED },
    { "SAMP"	, no_attr,	0,		SGML_MIXED },
    { "SCRIPT"  , no_attr,      0,              SGML_MIXED },
    { "SELECT"	, select_attr,	HTML_SELECT_ATTRIBUTES,	SGML_MIXED },
    { "STRONG"	, no_attr,	0,		SGML_MIXED },
    { "STYLE"   , no_attr,      0,              SGML_MIXED },
    { "TD"      , no_attr,      0,              SGML_EMPTY },
    { "TEXTAREA", textarea_attr,HTML_TEXTAREA_ATTRIBUTES, SGML_MIXED },
    { "TH"      , no_attr,      0,              SGML_EMPTY },
    { "TITLE", 	  no_attr,	0,		SGML_RCDATA },
    { "TR"      , no_attr,      0,              SGML_EMPTY },
    { "TT"	, no_attr,	0,		SGML_MIXED },
    { "U"	, no_attr,	0,		SGML_MIXED },
    { "UL"	, list_attr,	1,		SGML_MIXED },
    { "VAR"	, no_attr,	0,		SGML_MIXED },
    { "XMP"	, no_attr,	0,		SGML_LITTERAL },
};


PUBLIC CONST SGML_dtd HTML_dtd = {
	tags,
	HTML_ELEMENTS,
	entities,
	sizeof(entities)/sizeof(char**)
};

/*	Utility Routine: useful for people building HTML objects */

/*	Start anchor element
**	--------------------
**
**	It is kinda convenient to have a particulr routine for
**	starting an anchor element, as everything else for HTML is
**	simple anyway.
*/
struct _HTStructured {
    HTStructuredClass * isa;
	/* ... */
};

PUBLIC void HTStartAnchor ARGS3(HTStructured *, obj,
		CONST char *,  name,
		CONST char *,  href)
{
    BOOL		present[HTML_A_ATTRIBUTES];
    CONST char * 	value[HTML_A_ATTRIBUTES];
    
    {
    	int i;
    	for(i=0; i<HTML_A_ATTRIBUTES; i++)
	    present[i] = NO;
    }
    if (name) {
    	present[HTML_A_NAME] = YES;
	value[HTML_A_NAME] = (CONST char *)name;
    }
    if (href) {
        present[HTML_A_HREF] = YES;
        value[HTML_A_HREF] = (CONST char *)href;
    }

    (*obj->isa->start_element)(obj, HTML_A , present, value);

}

