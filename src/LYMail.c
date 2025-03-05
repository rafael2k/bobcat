// #include "spawno.h"
#include "HTUtils.h"
#include "HTParse.h"
#include "LYCurses.h"
#include "LYSignal.h"
#include "LYUtils.h"
#include "LYClean.h"
#include "LYString.h"
#include "GridText.h"
#include "LYSystem.h"
#include "LYGlobalDefs.h"
#include "LYMail.h"
#include "LYLeaks.h"

#include <unistd.h>

/*
**  mailform() sends form content to the mailto address(es). - FM
*/
PUBLIC void mailform ARGS3(char *,mailto_address, char *,mailto_subject,
			   char *,mailto_content)
{
    FILE *fd;
    char *address = NULL;
    char cmd[512], *cp, *cp0, *cp1;
    int len, i, ch, recall;
    char subject[80];
    char tmpfile[256];
    int result = 1;

    if(smtp_server == NULL) {
	_statusline("You need to define a SMTP_SERVER in lynx.cfg.");
        sleep(sleep_three);
	return;
    }

    if (!(personal_mail_address && *personal_mail_address))
    {
	_statusline("You need to define a Personal mail address in (O)ptions.");
        sleep(sleep_three);
	return;
    }

    if (!mailto_address || !mailto_content) {
	_statusline("BAD_FORM_MAILTO");
	return;
    }

    if ((cp = (char *)strchr(mailto_address,'\n')) != NULL)
	*cp = '\0';
    StrAllocCopy(address, mailto_address);

    /*
     *  Check for a ?subject=foo. - FM
     */
    subject[0] = '\0';
    if ((cp = strchr(address, '?')) != NULL &&
	strcasecomp(cp+1, "subject=")) {
	*cp = '\0';
	cp += 9;
	strncpy(subject, cp, 70);
	subject[70] = '\0';
	HTUnEscape(subject);
    }

    /*
     *  Unescape any hex escaped pounds. - FM
     */
    while ((cp1 = strstr(address, "%23")) != NULL) {
	*cp1 = '#';
	cp0 = (cp1 + 1);
	cp1 = (cp0 + 2);
	for (i = 0; cp1[i]; i++) {
	    cp0[i] = cp1[i];
	}
	cp0[i] = '\0';
    }

    /*
     *  Unescape any hex escaped percents. - FM
     */
    while ((cp1 = strstr(address, "%25")) != NULL) {
	cp0 = (cp1 + 1);
	cp1 = (cp0 + 2);
	for (i = 0; cp1[i]; i++) {
	    cp0[i] = cp1[i];
	}
	cp0[i] = '\0';
    }

    /*
     * Convert any Explorer semi-colon Internet address
     * separators to commas. - FM
     */
    cp = address;
    while ((cp1 = strchr(cp, '@')) != NULL) {
	cp1++;
	if ((cp0 = strchr(cp1, ';')) != NULL) {
	    *cp0 = ',';
	    cp1 = cp0 + 1;
	}
	cp = cp1;
    }

    /*
     *  Allow user to edit the default Subject. - FM
     */
    if (subject[0] == '\0') {
	if (mailto_subject && *mailto_subject) {
	    strncpy(subject, mailto_subject, 70);
	} else {
	    strcpy(subject, "mailto:");
	    strncpy((char*)&subject[7], address, 63);
	}
	subject[70] = '\0';
    }
    recall = 0;
    _statusline("Subject: ");
    if ((ch = LYgetstr(subject, VISIBLE, 71, recall)) < 0) {
	/*
	 * User cancelled via ^G. - FM
	 */
	_statusline("FORM_MAILTO_CANCELLED");
	sleep(sleep_three);
	free(address);
	return;
    }

	tempname(tmpfile,NEW_FILE);
	if((fd = fopen(tmpfile,"wt")) == NULL){
	    _statusline("Form Mailto failed to open tempfile.");
	    free(address);
	    return;
	}


    fprintf(fd,"To: %s\n", address);
    if (personal_mail_address && *personal_mail_address)
	fprintf(fd,"From: %s\n", personal_mail_address);
    fprintf(fd,"Subject: %.70s\n\n", subject);
//    _statusline("SENDING_FORM_CONTENT");

    /*
     *  Break up the content into lines with a maximimum length of 78.
     *  If the ENCTYPE was text/plain, we have physical newlines and
     *  should take them into account.  Otherwise, the actual newline
     *  characters in the content are hex escaped. - FM
     */
    while((cp = strchr(mailto_content, '\n')) != NULL) {
	*cp = '\0';
	i = 0;
	len = strlen(mailto_content);
	while (len > 78) {
	    strncpy(cmd, (char *)&mailto_content[i], 78);
	    cmd[78] = '\0';
	    fprintf(fd, "%s\n", cmd);
	    i += 78;
	    len = strlen((char *)&mailto_content[i]);
	}
	fprintf(fd, "%s\n", (char *)&mailto_content[i]);
	mailto_content = (cp+1);
    }
    i = 0;
    len = strlen(mailto_content);
    while (len > 78) {
	strncpy(cmd, (char *)&mailto_content[i], 78);
	cmd[78] = '\0';
	fprintf(fd, "%s\n", cmd);
	i += 78;
	len = strlen((char *)&mailto_content[i]);
    }
    if (len)
	fprintf(fd, "%s\n", (char *)&mailto_content[i]);

    fclose(fd);

    _statusline("Sending Form via mail.");

	fd = fopen(tmpfile, "r");
	if (fd == NULL) {
	    _statusline("Temp file failed.");
	    sleep(sleep_one);
	    goto cleanup;
	}
	fclose(fd);

    result = sendmail(tmpfile);

cleanup:

    if(result > 0) remove(tmpfile);

    free(address);
    return;
}


/* global variable for async i/o */
BOOLEAN term_letter;
PRIVATE void terminate_letter  PARAMS((int sig));
PRIVATE void remove_tildes PARAMS((char *string));


PUBLIC void reply_by_mail ARGS2(char *,mail_address, char *,filename)
{
	char user_input[1000];
	FILE *fd;
	int c;  /* user input */
	char tmpfile[100];
	char cmd[256];
	static char * personal_name=NULL;
	int n,result = 1;

	if(smtp_server == NULL) {
		_statusline("You need to define a SMTP_SERVER in lynx.cfg.");
                sleep(sleep_three);
		return;
	}

	term_letter=FALSE;

	clear();
	move(2,0);

	tempname(tmpfile,NEW_FILE);
	if((fd = fopen(tmpfile,"wt")) == NULL)
	    return;

	addstr("You are now sending a comment to:");
	addstr("\n	");
	if(*mail_address != '\0') {
	    addstr(mail_address);
	}
	addstr("\n\nUse Ctrl-G to cancel if you do not want to send a message\n\n");

	/* Use ^G to cancel mailing of comment */
	/* and don't let sigints exit lynx     */
	signal(SIGINT, terminate_letter);


	addstr(" Please enter your email address\n");
	if (personal_mail_address)
	    addstr(" Use Control-U to erase the default.\n");
	addstr("From: ");
	/* add the mail address if there is one */
	sprintf(user_input,"%s", (personal_mail_address ?
					personal_mail_address : ""));

	if (LYgetstr(user_input, VISIBLE) < 0 || term_letter) {
	    _statusline("Comment request cancelled!!!");
	    sleep(sleep_one);
	    fclose(fd);  /* close the temp file */
	    goto cleanup;
	}
	remove_tildes(user_input);

	fprintf(fd,"From: %s\n",user_input);

	/* put the to: line in the temp file */
	fprintf(fd,"To: %s\n", mail_address);

	fprintf(fd,"X-URL: %s%s\n",*filename ? filename : "mailto:",
				    *filename ? "" : mail_address);
	fprintf(fd,"X-Mailer: Lynx, Version %s\n",LYNX_VERSION);

	addstr("\n\n Please enter your name (Not required)\n");
	if(personal_name == NULL)
	    *user_input = '\0';
	else {
	    addstr(" Use Control-U to erase the default.\n");
	    strcpy(user_input, personal_name);
	}
	addstr("Personal Name: ");
	if (LYgetstr(user_input, VISIBLE) < 0 || term_letter) {
	    _statusline("Comment request cancelled!!!");
	    fclose(fd);  /* close the temp file */
	    sleep(sleep_one);
	    goto cleanup;
	}

	remove_tildes(user_input);
	StrAllocCopy(personal_name, user_input);

	term_letter=FALSE;
	fprintf(fd,"X-Personal_name: %s\n",user_input);

	addstr("\n\n Please enter a subject line.\n");
	addstr(" Use Control-U to erase the default.\n");
	addstr("Subject: ");
	sprintf(user_input, "%.70s%.63s", *filename ? filename : "mailto:",
					  *filename ? "" : mail_address);
	if (LYgetstr(user_input, VISIBLE) < 0 || term_letter) {
	    _statusline("Comment request cancelled!!!");
	    sleep(sleep_one);
	    fclose(fd);  /* close the temp file */
	    goto cleanup;
	}
	remove_tildes(user_input);

	fprintf(fd,"Subject: %s\n",user_input);
	fprintf(fd,"X-User-Message: Do not edit above this line!\n\n");

	if(!no_editor && editor && *editor != '\0' &&
					strcmp(HTLoadedDocumentURL(),"")) {
	    char *editor_arg = "";

	    /* ask if the user wants to include the original message */
	    _statusline("Do you wish to include the original message? (y/n) ");
	    c = 0;
	    while(TOUPPER(c) != 'Y' && TOUPPER(c) != 'N' &&
		  !term_letter && c != 7   && c != 3)
		c = LYgetch();
	    if(TOUPPER(c) == 'Y')
		/* the 1 will add the reply ">" in front of every line */
		print_wwwfile_to_fd(fd,1);

	    fclose(fd);

	    if (term_letter || c == 7 || c == 3)
		goto cleanup;

	    /* spawn the users editor on the mail file */
	    if (strstr(editor, "pico")) {
		editor_arg = " -t"; /* No prompt for filename to use */
	    }
	    sprintf(user_input,"%s%s %s",editor,editor_arg,tmpfile);
	    _statusline("Spawning your selected editor to edit mail message");
	    stop_curses();
	    if(system(user_input)) {
		start_curses();
		_statusline("Error spawning editor, check your editor definition in the options menu");
		sleep(sleep_two);
	    } else {
		start_curses();
	    }

	} else {

	    addstr("\n\n Please enter your message below.");
	    addstr("\n When you are done, press enter and put a single period (.)");
	    addstr("\n on a line and press enter again.");
	    addstr("\n\n");
	    scrollok(stdscr,TRUE);
	    refresh();
	    *user_input = '\0';
	    if (LYgetstr(user_input, VISIBLE) < 0 || term_letter) {
		_statusline("Comment request cancelled!!!");
		sleep(sleep_one);
		fclose(fd);  /* close the temp file */
		goto cleanup;
	    }


	    while(!STREQ(user_input,".") && !term_letter) {
	       addch('\n');
	       remove_tildes(user_input);
	       fprintf(fd,"%s\n",user_input);
	       *user_input = '\0';
	       if (LYgetstr(user_input, VISIBLE) < 0) {
		  _statusline("Comment request cancelled!!!");
		  sleep(sleep_one);
		  fclose(fd);  /* close the temp file */
		  goto cleanup;
	       }
	    }

	    fclose(fd);  /* close the temp file */
	    scrollok(stdscr,FALSE);  /* stop scrolling */
	}

	_statusline("Send this comment? (y/n) ");
	c = 0;
	while(TOUPPER(c) != 'Y' && TOUPPER(c) != 'N' &&
	      !term_letter && c != 7   && c != 3)
	    c = LYgetch();

	clear();  /* clear the screen */

	if(TOUPPER(c) != 'Y') {
	   goto cleanup;
	}

	/* send the tmpfile into sendmail.
	 */
	_statusline("Sending your message....");

	fd = fopen(tmpfile, "r");
	if (fd == NULL) {
	    _statusline("Comment request cancelled!!!");
	    sleep(sleep_one);
	    goto cleanup;
	}
	fclose(fd);

	result = sendmail(tmpfile);

	/* come here to cleanup and exit */
cleanup:
cleandown:
	term_letter = FALSE;

	scrollok(stdscr,FALSE);  /* stop scrolling */
	if(result > 0) remove(tmpfile);
}


PRIVATE void terminate_letter ARGS1(int,sig)
{
	term_letter=TRUE;
}

PRIVATE void remove_tildes ARGS1(char *,string)
{
       /* change the first character to a space if
	* it is a '~'
	*/
    if(*string == '~')
	*string = ' ';
}
