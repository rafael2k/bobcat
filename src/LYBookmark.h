
#ifndef LYBOOKMARK_H
#define LYBOOKMARK_H

#ifndef LYSTRUCTS_H
#include "LYStruct.h"
#endif /* LYSTRUCTS_H */

extern char * get_bookmark_filename PARAMS((char **name));
extern void save_bookmark_link PARAMS((char *address, char *title));
extern void remove_bookmark_link PARAMS((int cur));
extern void external(char *b, int c, char *d, int e);
extern int external2(char *b, int c, char *d, int e);

#define BOOKMARK_TITLE "Bookmark file"
#define MOSAIC_BOOKMARK_TITLE "Converted Mosaic Hotlist"

#endif /* LYBOOKMARK_H */

