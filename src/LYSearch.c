#include "HTUtils.h"
#include "LYUtils.h"
#include "LYString.h"
#include "LYSearch.h"
#include "LYGlobalDefs.h"
#include "GridText.h"

#include "LYLeaks.h"

/*
 * search for the target string inside of the links
 * that are currently displayed on the screen beginning
 * with the one after the currently selected one!
 * if found set cur to the new value and return true
 * in not found do not reset cur and return false.
 */

PUBLIC int check_for_target_in_links ARGS2(int *,cur, char *,new_target)
{
    int i = *cur+1;

    if(nlinks==0)
	return(FALSE);

    for(; i < nlinks; i++)
        if(case_sensitive) {
	    if(strstr(links[i].hightext,new_target) != NULL)
		break;
        } else {
	    if(LYstrstr(links[i].hightext,new_target) != NULL)
		break;
	}

    if (i == nlinks)
	return(FALSE);
 
    /* else */
        *cur = i;
        return(TRUE);
}

/*
 *  Textsearch checks the prev_target variable to see if it is empty.
 *  If it is then it requests a new search string.  It then searches 
 *  the current file for the next instance of the search string and
 *  finds the line number that the string is on
 * 
 *  This is the primary USER search engine and is case sensitive
 *  or case insensitive depending on the 'case_sensitive' global
 *  variable
 *
 */
		
PUBLIC void textsearch ARGS3(document *,cur_doc,
				 char *,prev_target, BOOL, next)
{
	int offset;
        int oldcur = cur_doc->link;
	static char prev_target_buffer[512]; /* Search string buffer */
	static BOOL first = TRUE;

       /*
        * Initialize the search string buffer. - FM
	*/
	if(first) {
	    *prev_target_buffer = '\0';
	    first = FALSE;
	}

	if(next)
	    /*
	     * LYK_NEXT was pressed, so copy the
	     * buffer into prev_target. - FM
	     */
	    strcpy(prev_target, prev_target_buffer);

	if(strlen(prev_target) == 0 ) {
	    /*
	     * This is a new WHEREIS search ('/'), or
	     * LYK_NEXT was pressed but there was no
	     * previous search, so we need to get a
	     * search string from the user. - FM
	     */
	        _statusline("Enter a search string: ");

	    if(LYgetstr(prev_target, VISIBLE) < 0) {
	        /*
		 * User cancelled the search via ^G.
		 * Restore prev_target and return. - FM
		 */
		strcpy(prev_target, prev_target_buffer);
	        return;
	    }
	}

	if(strlen(prev_target) == 0) {
	    /*
	     * No entry.  Simply return, retaining the current buffer.
	     * Because prev_target is now reset, highlighting of the
	     * previous search string will no longer occur, but it can
	     * be used again via LYK_NEXT.   - FM
	     */
	    return;
	}

        /*
	 * Replace the search string buffer with the new target. - FM
	 */
	strcpy(prev_target_buffer, prev_target);

        /*
	 * Search only links for the string,
	 * starting from the current link
	 */
	if(check_for_target_in_links(&cur_doc->link, prev_target)) {
	    /*
	     * Found in link, changed cur, we're done.
	     */
            highlight(OFF, oldcur);
	    return; 
	}
	
	/*
	 * We'll search the text starting from the
	 * link we are on, or the next page.
	 */
	if(nlinks == 0)
	    offset = display_lines+1;
	else
	    offset = links[cur_doc->link].ly;

	/*
	 * Resume search, this time for all text.
	 * Set www_search_result if string found,
	 * and position the hit at top of screen.
	 */
	www_user_search(cur_doc->line+offset, prev_target);
}
