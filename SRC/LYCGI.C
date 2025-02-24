/*                   Lynx CGI support                              LYCgi.c
**                   ================
**
** Authors
**          GL      George Lindholm <George.Lindholm@ubc.ca>
**
** History
**      15 Jun 95   Created as way to provide a lynx based service with
**                  dynamic pages without the need for a http daemon. GL
**      27 Jun 95   Added <index> (command line) support. Various cleanup
**                  and bug fixes. GL
**
** Bugs
**      If the called scripts aborts before sending the mime headers then
**      lynx hangs.
**
**      Should do something about SIGPIPE, (but then it should never happen)
**
**      No support for redirection. Or mime-types.
**
**      Should try and parse for a HTTP 1.1 header in case we are "calling" a
**      nph- script.
*/ 

#include "HTUtils.h"
#include "HTTP.h"
#include "LYUtils.h"

#include "HTParse.h"
#include "LYGlobal.h"
#include "LYGetFil.h"
#include "LYBookma.h"
#include "GridText.h"
#include "HTTCP.h"
#include "HTFormat.h"
#include "HTFile.h"
#include <ctype.h>
// #include <dos.h>
#include "HTAlert.h"
#include "HTMIME.h"
#include "HTML.h"
#include "HTInit.h"
#include "HTAABrow.h"
#include "LYCgi.h"
#ifndef MSDOS
#include <unistd.h>
#endif /* msdos */

#include "LYLeaks.h"

struct _HTStream 
{
  HTStreamClass * isa;
};

PRIVATE char **env = NULL;  /* Environment variables */
PRIVATE int envc_size = 0;  /* Slots in environment array */
PRIVATE envc = 0;	    /* Slots used so far */
PRIVATE char user_agent[64];
PRIVATE char server_software[64];

PRIVATE add_environment_value PARAMS((char *env_value));


/*
 * Simple routine for expanding the environment array and adding a value to
 * it
 */
PRIVATE add_environment_value ARGS1 (
				     char *, env_value)
{
    if (envc == envc_size) {   /* Need some more slots */
	envc_size += 10;
	env = realloc(env, sizeof(env[0]) * (envc_size + 2));/* + terminator 
							      and base 0 */
	if (env == NULL) {
	    outofmem(__FILE__, "LYCgi");
	}
    }

    env[envc++] = env_value;
    env[envc] = NULL;      /* Make sure it is always properly terminated */
}
    
/*
 * Add the value of an existing environment variable to those passed on to the
 * lynxcgi script.
 */
PUBLIC void add_lynxcgi_environment ARGS1 (
					   CONST char *, variable_name)
{
    char *env_value;

    env_value = getenv(variable_name);
    if (env_value != NULL) {
	char *add_value = NULL;

	add_value = malloc(strlen(variable_name) + strlen(env_value) + 2);
	if (add_value == NULL) {
	    outofmem(__FILE__, "LYCgi");
	}
	strcpy(add_value, variable_name);
	strcat(add_value, "=");
	strcat(add_value, env_value);
	add_environment_value(add_value);
    }
}

PUBLIC int LYLoadCGI ARGS4 (
	CONST char *, 		arg,
	HTParentAnchor *,	anAnchor,
	HTFormat,		format_out,
	HTStream*,		sink)
{
    int status;
#ifdef LYNXCGI_LINKS
#ifndef VMS
    char *cp;
    struct stat stat_buf;
    char *pgm = NULL;		        /* executable */
    char *pgm_args = NULL;	        /* and its argument(s) */

    if (arg) {
	if (strncmp(arg, "lynxcgi://localhost", 19) == 0) {
	    StrAllocCopy(pgm, arg+19);
	} else {
	    StrAllocCopy(pgm, arg+8);
	}
	if ((cp=strchr(pgm, '?')) != NULL) { /* Need to terminate executable */
	    *cp++ = '\0';
	    pgm_args = cp;
	}
    }

    if (!arg || !*arg) {
	_statusline("Bad request.");
	sleep(sleep_two);
	status = -2;

    } else if (stat(pgm, &stat_buf) < 0) {
	_statusline("Unable to access cgi script");
	sleep(sleep_two);
#ifdef DT
	if (TRACE) {
	    perror("LYNXCGI: stat() failed");
	}
#endif

	status = -4;

    } else if (!(S_ISREG(stat_buf.st_mode) &&
		 stat_buf.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))) {
	/* Not a runnable file, See if we can load it using file: code */
	char *temp = NULL;
	char *new_arg = NULL;

	StrAllocCopy(temp, pgm);
	StrAllocCopy(new_arg, "file://localhost");
	StrAllocCat(new_arg, temp);

#ifdef DT
	if (TRACE) {
	    fprintf(stderr,
		    "%s is not an executable file, passing the buck.\n", arg);
	}
#endif

	status = HTLoadFile(new_arg, anAnchor, format_out, sink);
	free(new_arg);

    } else if (no_lynxcgi) {
	_statusline("cgi support has been disabled by system administrator.");
	sleep(sleep_two);
	status = HT_NOT_LOADED;

    } else if (no_bookmark_exec && bookmark_page &&
	       (strstr(HTLoadedDocumentURL(), bookmark_page) ||
		!strcmp(HTLoadedDocumentTitle(),
			MOSAIC_BOOKMARK_TITLE))) {
	_statusline("Execution via bookmarks is disabled.");
	sleep(sleep_two);
	status = HT_NOT_LOADED;

    } else if (!exec_ok(HTLoadedDocumentURL(), pgm,
			CGI_PATH)) { /* exec_ok gives out msg. */
	status = HT_NOT_LOADED;

    } else {
	HTFormat format_in;
	HTStream *target  = NULL;		/* Unconverted data */
	int fd1[2], fd2[2];
	pid_t pid;
	char buf[1024];

	/* Decode full HTTP response */
	format_in = HTAtom_for("www/mime");
		
	target = HTStreamStack(format_in,
			       format_out,
			       sink, anAnchor);
		
	if (!target || target == NULL) {
	    sprintf(buf, "Sorry, no known way of converting %s to %s.",
		    HTAtom_name(format_in), HTAtom_name(format_out));
	    _statusline (buf);
	    sleep(sleep_two);
	    status = HT_NOT_LOADED;

	} else if (anAnchor->post_data && pipe(fd1) < 0) {
	    _statusline("Unable to set up connection");
	    sleep(sleep_two);
#ifdef DT
	    if (TRACE) {
		perror("LYNXCGI: pipe() failed");
	    }
#endif

	    status = -3;
	    
	} else if (pipe(fd2) < 0) {
	    _statusline("Unable to set up connection");
	    sleep(sleep_two);
#ifdef DT
	    if (TRACE) {
		perror("LYNXCGI: pipe() failed");
	    }
#endif

	    close(fd1[0]);
	    close(fd1[1]);
	    status = -3;
	    
	} else {	
	    static BOOL first_time = TRUE;      /* One time setup flag */

	    if (first_time) {	/* Set up static environment variables */
		first_time = FALSE;	/* Only once */
		
		add_environment_value("REMOTE_HOST=localhost");
		add_environment_value("REMOTE_ADDR=127.0.0.1");
		
		sprintf(user_agent, "HTTP_USER_AGENT=%s/%s libwww/%s",
			LYNX_NAME, LYNX_VERSION, HTLibraryVersion);
		add_environment_value(user_agent);
		
		sprintf(server_software, "SERVER_SOFTWARE=%s/%s",
			LYNX_NAME, LYNX_VERSION);
		add_environment_value(server_software);
	    }
	    
	    if ((pid = fork()) > 0) { /* The good, */
		long chars, total_chars;
		
		close(fd2[1]);
		
		if (anAnchor->post_data) {
		    close(fd1[0]);

		    /* We have form data to push across the pipe */
#ifdef DT
		    if (TRACE) {
			fprintf(stderr, "LYNXCGI: Doing post, content-type '%s'\n",
				anAnchor->post_content_type);
			fprintf(stderr,
				"LYNXCGI: Writing:\n%s----------------------------------\n",
				anAnchor->post_data);			
		    }
#endif

		    write(fd1[1], anAnchor->post_data,
			  strlen(anAnchor->post_data));
		}
		
		total_chars = 0;
		while((chars = read(fd2[0], buf, sizeof(buf))) > 0) {
		    char line[40];
		    
		    total_chars += chars;
		    sprintf (line, "Read %ld bytes of data.", total_chars);
		    HTProgress(line);
#ifdef DT
		    if (TRACE) {
			fprintf(stderr, "LYNXCGI: Rx: %s\n", buf);
		    }
#endif

		    (*target->isa->put_block)(target, buf, chars);
		}
		waitpid(pid, &status, 0);
		if (anAnchor->post_data) {
		    close(fd1[1]);
		}
		close(fd2[0]);
		status = HT_LOADED;
		
	    } else if(pid == 0) { /* The Bad, */
		char **argv = NULL;
		char post_len[32];
		int argv_cnt = 3; /* name, one arg and terminator */
		char **cur_argv = NULL;

		/* Set up output pipe */
		close(fd2[0]);
		dup2(fd2[1], STDOUT_FILENO); /* Should check success code */
		dup2(fd2[1], STDERR_FILENO);
		close(fd2[1]);
		
		
		if (anAnchor->post_data) { /* post script, read stdin */
		    close(fd1[1]);
		    dup2(fd1[0], STDIN_FILENO);
		    close(fd1[0]);

		    /* Build environment variables */

		    add_environment_value("REQUEST_METHOD=POST");

		    sprintf(post_len, "CONTENT_LENGTH=%d",
			    strlen(anAnchor->post_data));
		    add_environment_value(post_len);
		} else {
		    close(STDIN_FILENO);
		}

		/* 
		 * Set up argument line, mainly for <index> scripts
		 */
		if (pgm_args != NULL) {
		    for (cp = pgm_args; *cp != '\0'; cp++) {
			if (*cp == '+') {
			    argv_cnt++;
			}
		    }
		}

		argv = (char**)malloc(argv_cnt * sizeof(char*));
		if (argv == NULL) {
		    outofmem(__FILE__, "LYCgi");
		}
		cur_argv = argv + 1;		/* For argv[0] */
		if (pgm_args != NULL) {		
		    char *cr;

		    /* Data for a get/search form */
		    if (is_www_index) {
			add_environment_value("REQUEST_METHOD=SEARCH");
		    } else {
			add_environment_value("REQUEST_METHOD=GET");
		    }
		    
		    cp = NULL;
		    StrAllocCopy(cp, "QUERY_STRING=");
		    StrAllocCat(cp, pgm_args);
		    add_environment_value(cp);

		    /*
		     * Split up arguments into argv array
		     */
		    cp = pgm_args;
		    cr = cp;
		    while(1) {
			if (*cp == '\0') {
			    *(cur_argv++) = HTUnEscape(cr);
			    break;
			    
			} else if (*cp == '+') {
			    *cp++ = '\0';
			    *(cur_argv++) = HTUnEscape(cr);
			    cr = cp;
			}
			cp++;
		    }
		}
		*cur_argv = NULL;	/* Terminate argv */		
		argv[0] = pgm;

		execve(argv[0], argv, env);
#ifdef DT
		if (TRACE) {
		    perror("LYNXCGI: execve failed");
		}
#endif

	    } else {	/* and the Ugly */
		_statusline ("Unable to make connection");
		sleep(sleep_two);
#ifdef DT
		if (TRACE) {
		    perror("LYNXCGI: fork() failed");
		}
#endif

		status = HT_NO_DATA;
		close(fd1[0]);
		close(fd1[1]);
		close(fd2[0]);
		close(fd2[1]);
		status = -1;
	    }

	}
	if (target != NULL) {
	    (*target->isa->_free)(target);
	}
    }
    if (pgm != NULL) {
	free(pgm);
    }
#else  /* VMS */
    _statusline("Sorry, don't know how to run a cgi script under VMS.");
    sleep(sleep_two);
    status = -1;
#endif /* VMS */
#else /* LYNXCGI_LINKS */
    _statusline("Lynxcgi capabilities are not compiled into this version.");
    sleep(sleep_two);
    status = HT_NOT_LOADED;
#endif /* LYNXCGI_LINKS */
    return(status);
}

GLOBALDEF PUBLIC HTProtocol LYLynxCGI = { "lynxcgi", LYLoadCGI, 0 };




