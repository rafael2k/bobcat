#include "HTUtils.h"
#include "LYUtils.h"
#include "LYrcFile.h"
#include "LYString.h"
#include "LYGlobal.h"
#include "LYLeaks.h"

static char histfile[256];


PUBLIC void init_syslog NOARGS
{
    char *home;

    if(!log_urls) return;

    /* make a name */

    home = getenv("HOME");
    if (home != NULL)
	sprintf(histfile,"%s/history.htm",home);
    else
	sprintf(histfile,"%s/history.htm",cdirbuffer);

}

PUBLIC void syslog ARGS1(char *,z)
{
    FILE *fp;

    if(!log_urls) return;

    if((fp = fopen(histfile,"a")) == NULL) {
	return; /* FALSE; */
    }

    if(!strncasecomp(z,"file",4)) return;
    if(!strncasecomp(z,"LYNX",4)) return;

    fprintf(fp,"<a href=\"%s\">%s</a><BR>\n",z,z);

    fclose(fp);

    return; /* TRUE; */

}

PUBLIC void textlog ARGS1(char *,z)
{
    FILE *fp;

    if(!log_urls) return;

    if((fp = fopen(histfile,"a")) == NULL) {
	return; /* FALSE; */
    }

    fprintf(fp,"<P>%s<P>\n",z);

    fclose(fp);

    return; /* TRUE; */

}
