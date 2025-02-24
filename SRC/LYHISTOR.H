
#ifndef LYHISTORY_H
#define LYHISTORY_H

#ifndef LYSTRUCTS_H
#include "LYStruct.h"
#endif /* LYSTRUCTS_H */

extern void push PARAMS((document *doc));
extern void pop PARAMS((document *doc));
extern void pop_num PARAMS((int number, document *doc));
extern void showhistory PARAMS((char **newfile));
extern void historytarget PARAMS((document *newdoc));

#define HISTORY_PAGE_TITLE  "Lynx History Page"

#endif /* LYHISTORY_H */
