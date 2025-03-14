/*	A real style sheet for the Character Grid browser
**
**	The dimensions are all in characters!
*/

#include "HTUtils.h"
#include "HTStyle.h"
#include "HTFont.h"

#include "LYLeaks.h"

/*	Tab arrays:
*/
PRIVATE HTTabStop tabs_8[] = {
	{ 0, 8 }, {0, 16}, {0, 24}, {0, 32}, {0, 40},
	{ 0, 48 }, {0, 56}, {0, 64}, {0, 72}, {0, 80},
	{ 0, 88 }, {0, 96}, {0, 104}, {0, 112}, {0, 120},
	{ 0, 128 }, {0, 136}, {0, 144}, {0, 152}, {0, 160},
	{0, 168}, {0, 176},
	{0, 0 }		/* Terminate */
};

#ifdef NOT_USED
PRIVATE HTTabStop tabs_16[] = {
	{ 0, 16 }, {0, 32}, {0, 48}, {0, 64}, {0, 80},
	{0, 96}, {0, 112},
	{0, 0 }		/* Terminate */
};
#endif /* NOT_USED */

/* Template:
**	link to next, name, tag,
**	font, size, colour, 		superscript, anchor id,
**	indents: 1st, left, right, alignment	lineheight, descent,	tabs,
**	word wrap, free format, space: before, after, flags.
*/

PRIVATE HTStyle HTStyleNormal = { 
	0,  "Normal", "P",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	3, 3, 6, HT_LEFT,		1, 0,	tabs_8,
	YES, YES, 1, 0,			0 };	

PRIVATE HTStyle HTStyleBlockquote = { 
	&HTStyleNormal,  "Blockquote", "BLOCKQUOTE",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	5, 5, 6, HT_LEFT,		1, 0,	tabs_8,
	YES, YES, 1, 0,			0 };	

PRIVATE HTStyle HTStyleList = { 
	&HTStyleBlockquote,  "List", "UL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	3, 7, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0 };	

PRIVATE HTStyle HTStyleList1 = { 
	&HTStyleList,  "List1", "UL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	8, 12, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0 };	

PRIVATE HTStyle HTStyleList2 = { 
	&HTStyleList1,  "List2", "UL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	13, 17, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0 };	

PRIVATE HTStyle HTStyleList3 = { 
	&HTStyleList2,  "List3", "UL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	18, 22, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0 };	

PRIVATE HTStyle HTStyleList4 = { 
	&HTStyleList3,  "List4", "UL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	23, 27, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0 };	

PRIVATE HTStyle HTStyleList5 = { 
	&HTStyleList4,  "List5", "UL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	28, 32, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0 };	

PRIVATE HTStyle HTStyleList6 = { 
	&HTStyleList5,  "List6", "UL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	33, 37, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0 };	

PRIVATE HTStyle HTStyleMenu = {
	&HTStyleList6,  "Menu", "MENU",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	3, 7, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleMenu1 = {
	&HTStyleMenu,  "Menu1", "MENU",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	8, 12, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleMenu2= {
	&HTStyleMenu1,  "Menu2", "MENU",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	13, 17, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleMenu3= {
	&HTStyleMenu2,  "Menu3", "MENU",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	18, 22, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleMenu4= {
	&HTStyleMenu3,  "Menu4", "MENU",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	23, 27, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleMenu5= {
	&HTStyleMenu4,  "Menu5", "MENU",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	28, 33, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleMenu6= {
	&HTStyleMenu5,  "Menu6", "MENU",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	33, 38, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleGlossary = {
	&HTStyleMenu6,  "Glossary", "DL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	3, 10, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 1, 1,			0
};	

PRIVATE HTStyle HTStyleGlossary1 = {
	&HTStyleGlossary,  "Glossary1", "DL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	8, 16, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 1, 1,			0
};	

PRIVATE HTStyle HTStyleGlossary2 = {
	&HTStyleGlossary1,  "Glossary2", "DL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	14, 22, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 1, 1,			0
};	

PRIVATE HTStyle HTStyleGlossary3 = {
	&HTStyleGlossary2,  "Glossary3", "DL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	20, 28, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 1, 1,			0
};	

PRIVATE HTStyle HTStyleGlossary4 = {
	&HTStyleGlossary3,  "Glossary4", "DL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	26, 34, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 1, 1,			0
};	

PRIVATE HTStyle HTStyleGlossary5 = {
	&HTStyleGlossary4,  "Glossary5", "DL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	32, 40, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 1, 1,			0
};	

PRIVATE HTStyle HTStyleGlossary6 = {
	&HTStyleGlossary5,  "Glossary6", "DL",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	38, 46, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 1, 1,			0
};	

PRIVATE HTStyle HTStyleGlossaryCompact = {
	&HTStyleGlossary6,  "GlossaryCompact", "DLC",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	3, 10, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleGlossaryCompact1 = {
	&HTStyleGlossaryCompact,  "GlossaryCompact1", "DLC",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	8, 15, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleGlossaryCompact2 = {
	&HTStyleGlossaryCompact1,  "GlossaryCompact2", "DLC",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	13, 20, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleGlossaryCompact3 = {
	&HTStyleGlossaryCompact2,  "GlossaryCompact3", "DLC",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	18, 25, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleGlossaryCompact4 = {
	&HTStyleGlossaryCompact3,  "GlossaryCompact4", "DLC",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	23, 30, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleGlossaryCompact5 = {
	&HTStyleGlossaryCompact4,  "GlossaryCompact5", "DLC",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	28, 35, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleGlossaryCompact6 = {
	&HTStyleGlossaryCompact5,  "GlossaryCompact6", "DLC",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	33, 40, 6, HT_LEFT,		1, 0,	0, 
	YES, YES, 0, 0,			0
};

PRIVATE HTStyle HTStyleExample = {
	&HTStyleGlossaryCompact6,  "Example", "XMP",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	0, 0, 0, HT_LEFT,		1, 0,	tabs_8,
	NO, NO, 1, 1,			0
};	

PRIVATE HTStyle HTStylePreformatted = {
	&HTStyleExample,  	"Preformatted", "PRE",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	0, 0, 0, HT_LEFT,		1, 0,	tabs_8,
	NO, YES, 0, 0,			0
};	

PRIVATE HTStyle HTStyleListing =
	{ &HTStylePreformatted,  "Listing", "LISTING",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	0, 0, 0, HT_LEFT,		1, 0,	tabs_8,
	NO, NO, 1, 1,			0 };

PRIVATE HTStyle HTStyleAddress =
	{ &HTStyleListing,  "Address", "ADDRESS",
	HT_FONT, 1.0, HT_BLACK,		0, 0,
	4, 4, 6, HT_LEFT,		1, 0,	0,
	YES, YES, 2, 0,			0 };	

PRIVATE HTStyle HTStyleCenter =
        { &HTStyleAddress,  "Center", "CENTER",
        HT_FONT, 1.0, HT_BLACK,     0, 0,
        3, 3, 6, HT_CENTER,             1, 0,   tabs_8,
	YES, YES, 1, 1,                 0 };

PRIVATE HTStyle HTStyleHeading1 =
        { &HTStyleCenter,  "Heading1", "H1",
	HT_FONT+HT_CAPITALS+HT_BOLD, 1.0, HT_BLACK,	0, 0,
	0, 0, 0, HT_CENTER,		1, 0,	0,
	YES, YES, 1, 1,			0 };	

PRIVATE HTStyle HTStyleHeading2 =
	{ &HTStyleHeading1,  "Heading2", "H2",
	HT_FONT+HT_BOLD, 1.0, HT_BLACK,	0, 0,
	0, 0, 0, HT_LEFT,		1, 0,	0,
	YES, YES, 1, 1,			0 };

PRIVATE HTStyle HTStyleHeading3 = { 
	&HTStyleHeading2,  "Heading3", "H3",
	HT_FONT+HT_BOLD, 1.0, HT_BLACK,	0, 0,
	2, 2, 0, HT_LEFT,		1, 0,	0, 
	YES, YES, 1, 0,			0 };	

PRIVATE HTStyle HTStyleHeading4 = { 
	&HTStyleHeading3,  "Heading4", "H4",
	HT_FONT+HT_BOLD, 1.0, HT_BLACK,	0, 0,
	4, 4, 0, HT_LEFT,		1, 0,	0,
	YES, YES, 1, 0,			0 };

PRIVATE HTStyle HTStyleHeading5 = { 
	&HTStyleHeading4,  "Heading5", "H5",
	HT_FONT+HT_BOLD, 1.0, HT_BLACK,	0, 0,
	6, 6, 0, HT_LEFT,		1, 0,	0,
	YES, YES, 1, 0,			0 };	

PRIVATE HTStyle HTStyleHeading6 = { 
	&HTStyleHeading5,  "Heading6", "H6",
	HT_FONT+HT_BOLD, 1.0, HT_BLACK,	0, 0,
	8, 8, 0, HT_LEFT,		1, 0,	0,
	YES, YES, 1, 0,			0 };	

PRIVATE HTStyle HTStyleHeading7 = { 
	&HTStyleHeading6,  "Heading7", "H7",
	HT_FONT+HT_BOLD, 1.0, HT_BLACK,	0, 0,
	10, 10, 0, HT_LEFT,		1, 0,	0,
	YES, YES, 1, 0,			0 };	

/* Style sheet points to the last in the list:
*/
PRIVATE HTStyleSheet sheet = { "default.style", &HTStyleHeading7 }; /* sheet */

PUBLIC HTStyleSheet * styleSheet = &sheet;
 
