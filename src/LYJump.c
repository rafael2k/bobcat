#include "HTUtils.h"
#include "tcp.h"
#include "LYUtils.h"
#include "LYString.h"
#include "LYGlobal.h"
#include "LYJump.h"
#include "LYKeymap.h"
#include "LYSignal.h"

// #include <io.h>
// #include <dos.h>
#include <fcntl.h>

#include "LYLeaks.h"

#ifdef VMS
#include <fab.h>
#endif /* VMS */

typedef struct _JumpDatum {
    char *key;
    char *url;
} JumpDatum;

PRIVATE int LYCompare PARAMS ((CONST void *e1, CONST void *e2));
PRIVATE unsigned LYRead_Jumpfile PARAMS ((JumpDatum **tpp));

PUBLIC char *LYJump NOARGS
{
    static BOOL firsttime = TRUE;
    JumpDatum seeking;
    static JumpDatum *table;
    JumpDatum *found;
    static unsigned nel;
    static char buf[120];
    char *bp;

    if (firsttime) {
	nel = LYRead_Jumpfile(&table);
	firsttime = FALSE;
    }
    if (nel == 0) {
        if(jumpfile)
	    free(jumpfile);
	return (jumpfile=NULL);
    }

    statusline("Jump to (use '?' for list): ");
    if (!jump_buffer)
        *buf = '\0';
    if (LYgetstr(buf, VISIBLE) == -1)
        return NULL;
    bp = buf;
    if (toupper(*key_for_func(LYK_JUMP)) == 'G' && strncmp(buf, "o ", 2) == 0)
	bp++;
    while (isspace(*bp))
	bp++;
    if (*bp == '\0')
        return NULL;
    seeking.key = bp;
    found = (JumpDatum *)bsearch((char *)&seeking, (char *)table,
				 nel, sizeof(JumpDatum), LYCompare);
    if (!found) {
	char msg[120];

	sprintf(msg, "Unknown target `%s'", buf);
	statusline(msg);
	sleep(sleep_two);
    }

    return found ? found->url : NULL;
}

PRIVATE unsigned LYRead_Jumpfile ARGS1(JumpDatum **,tpp)
{
    struct stat st;
    unsigned int nel;
    char *mp;
    int fd;
    char *cp;
    int i;

    if (jumpfile == NULL || *jumpfile == '\0')
	return 0;

    if (stat(jumpfile, &st) < 0) {
	statusline("Cannot locate jump file");
	sleep(sleep_two);
	return 0;
    }

    /* allocate storage to read entire file */
    if ((mp=(char *)calloc(1, st.st_size + 1)) == NULL) {
	statusline("Out of memory reading jump file");
	sleep(sleep_two);
	return 0;
    }

    if ((fd=open(jumpfile, O_RDONLY | O_BINARY)) < 0) {
	statusline("Cannot open jump file");
	sleep(sleep_two);
	free(mp);
	return 0;
    }



    if (read(fd, mp, st.st_size) < st.st_size) {
	statusline("Error reading jump file");
//	printf("  size = %i",read(fd, mp, st.st_size));
	sleep(sleep_two);
	return 0;
    }

    mp[st.st_size] = '\0';
    close(fd);

    /* quick scan for approximate number of entries */
    nel = 0;
    cp = mp;
    while((cp = strchr(cp, '\n')) != NULL) {
	nel++;
	cp++;
    }

    *tpp = (JumpDatum *)malloc(nel * sizeof(JumpDatum));
    if (*tpp == NULL) {
	statusline("Out of memory for jump table");
	sleep(sleep_two);
	free(mp);
	return 0;
    }

    cp = mp;
    for (i = 0; i < nel; ) {
	if (strncmp(cp, "<!--", 4) == 0 || strncmp(cp, "<dl>", 4) == 0) {
	    cp = strchr(cp, '\r');
	    if (cp == NULL)
		cp = strchr(cp, '\n');
	    if (cp == NULL)
		break;
	    cp++;
	    continue;
	}
	cp = LYstrstr(cp, "<dt>");
	if (cp == NULL)
	    break;
	cp += 4;
	(*tpp)[i].key = cp;
	cp = LYstrstr(cp, "<dd>");
	if (cp == NULL)
	    break;
	*cp = '\0';
	cp += 4;
	cp = LYstrstr(cp, "href=\"");
	if (cp == NULL)
	    break;
	cp += 6;
	(*tpp)[i].url = cp;
	cp = strchr(cp, '"');
	if (cp == NULL)
	    break;
	*cp = '\0';
	cp++;
	cp = strchr(cp, '\r');
	if (cp == NULL)
	    cp = strchr(cp, '\n');
	if (cp == NULL)
	    break;
	cp++;
	i++;
	if (!cp)
	    break;
    }

    return i;
}

PRIVATE int LYCompare ARGS2 (CONST void *, e1, CONST void *, e2)
{
    return strcasecomp(((JumpDatum *)e1)->key, ((JumpDatum *)e2)->key);
}
