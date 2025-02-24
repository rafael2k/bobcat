#include "tcp.h"
#include "HTTCP.h"
#include <string.h>
#include "HTAlert.h"
#include "LYUtils.h"
#include "LYGlobal.h"
#include <io.h>
#include "LYLeaks.h"

//	CR/LF sequence used over the net.
char *cp_crlf = "\r\n";

int sendmail(char *tmpfile)  {
//      Purpose:        Allow user to send a mail message from msdog
//      Arguments:      Temp filename with mail in it
//      Return Value:   fail = -1, success = 1
//	Remarks/Portability/Dependencies/Restrictions:
//		Based on RFC 821, SMTP
//	Revision History:
//              04-04-94        created for doslynx's maildeveloper
//              11-16-96        mangled as a sendmail for bobcat

//    extern unsigned long my_ip_addr;
    char cp_buffer[4096];
    int s;				/* Socket number for returned data */
    int status;				/* tcp return */
    struct sockaddr_in soc_address;	/* Binary network address */
    struct sockaddr_in* sin = &soc_address;
    FILE *fd;

    sin->sin_family = AF_INET;
    sin->sin_port = htons(25);

    _HTProgress("Contacting SMTP server");

    status = HTParseInet(sin, smtp_server);
    if (status)
      {
	_HTProgress("Unable to lookup SMTP server.");
        sleep(sleep_two);
	return -1;
      }

  /* Now, let's get a socket set up from the server for the data: */
  s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

   if(HTCheckForInterrupt())
   {
#ifdef DL
      if (TRACE)
	fprintf (stderr, "*** INTERRUPTED in middle of connect.\n");
#endif
      status = HT_INTERRUPTED;
      SOCKET_ERRNO = EINTR;
      return -1;
   }

  status = connect(s, (struct sockaddr*)&soc_address, sizeof(soc_address));

  if(status < 0)	{
	NETCLOSE(s);
	_HTProgress("Unable to connect to SMTP server.");
	sleep(sleep_two);
	return -1;
  }


  _HTProgress("SMTP server contacted.  Sending message.");

  //	Since we are connected, start doing the SMTP thing.
  //	Check for 220 response.
  cp_buffer[0] = '\0';
  s_read(s, cp_buffer, 4095);

  if(strstr(cp_buffer, "220") == NULL)	{
	NETCLOSE(s);
	_HTProgress("SMTP host not responding.  "
		"No mail sent.");
	sleep(sleep_two);
	  return(-1);
  }

  //	Send HELO
  cp_buffer[0] = '\0';
  sprintf(cp_buffer, "HELO %s.%s%s", gethostname(NULL, 0) ==
		NULL ? "doslynx" : gethostname(NULL, 0),
		getdomainname(NULL, 0) == NULL ? "not.known.edu" :
		getdomainname(NULL, 0), cp_crlf);

  s_write(s, cp_buffer, strlen(cp_buffer));

  //	Check for 250 response.
  cp_buffer[0] = '\0';
  s_read(s, cp_buffer, 4095);

  if(strstr(cp_buffer, "250") == NULL)	{
	NETCLOSE(s);
	_HTProgress("SMTP host not responding.  No mail sent.");
	sleep(sleep_two);
	return(-1);
  }

  fd = fopen(tmpfile, "r");
  if (fd == NULL) {
	NETCLOSE(s);
	_HTProgress("SMTP temp file not found.");
	sleep(sleep_two);
	return(-1);
  }

  //	Send MAIL FROM
  cp_buffer[0] = '\0';
  fscanf(fd,"%s",cp_buffer);
  cp_buffer[0] = '\0';
  fscanf(fd,"%s",cp_buffer);
//  cp_buffer[strlen(cp_buffer)] = NULL;
//  cp_buffer[strlen(cp_buffer)] = NULL;

  s_write(s, "MAIL FROM:<", 11);
  s_write(s, cp_buffer, strlen(cp_buffer));
  s_write(s, ">", 1);
  s_write(s, cp_crlf,2);

  //	Check for 250 response.
  cp_buffer[0] = '\0';
  s_read(s, cp_buffer, 4095);
  if(strstr(cp_buffer, "250") == NULL)	{
	NETCLOSE(s);
	fclose(fd);
//	_HTProgress("SMTP host refusing to accept.  No mail sent.");
	_HTProgress(cp_buffer);
        sleep(sleep_three);
	return(-1);
  }

  //	Send RCPT TO
  cp_buffer[0] = '\0';
  fscanf(fd,"%s",cp_buffer);
  cp_buffer[0] = '\0';
  fscanf(fd,"%s",cp_buffer);
//  cp_buffer[strlen(cp_buffer)] = NULL;
//  cp_buffer[strlen(cp_buffer)] = NULL;

  s_write(s, "RCPT TO:<", 9);
  s_write(s, cp_buffer, strlen(cp_buffer));
  s_write(s, ">", 1);
  s_write(s, cp_crlf,2);

  //	Check for 250 or 251 response.
  cp_buffer[0] = '\0';
  s_read(s, cp_buffer, 4095);
  if(strstr(cp_buffer, "250") == NULL && strstr(cp_buffer,
		"251") == NULL)	{
	NETCLOSE(s);
	fclose(fd);
	_HTProgress(cp_buffer);
//	_HTProgress("SMTP host refusing to accept.  No mail sent.");
        sleep(sleep_three);
	return(-1);
  }


  rewind(fd);

  //	Send DATA
  cp_buffer[0] = '\0';
  sprintf(cp_buffer, "DATA%s", cp_crlf);
  s_write(s, cp_buffer, strlen(cp_buffer));


  //	Check for 354 response.
  cp_buffer[0] = '\0';
  s_read(s, cp_buffer, 4095);
  if(strstr(cp_buffer, "354") == NULL)	{
	NETCLOSE(s);
	fclose(fd);
	_HTProgress(cp_buffer);
//	_HTProgress("SMTP host refusing to accept.  No mail sent.");
	sleep(sleep_three);
	return(-1);
  }

  cp_buffer[0] = '\0';
  fgets(cp_buffer,4095,fd);

  while(!feof(fd)) {
// printf("-> %s\n",cp_buffer);
	if(cp_buffer[0] == '.') cp_buffer[0] = '+';
	if(cp_buffer[strlen(cp_buffer)-1] != '\r')
	{
		cp_buffer[strlen(cp_buffer)-1] = '\r';
		s_write(s, cp_buffer, strlen(cp_buffer));
		s_write(s, "\n",1);
	} else
		s_write(s, cp_buffer, strlen(cp_buffer));
	cp_buffer[0] = '\0';
	fgets(cp_buffer,4095,fd);
	}

  s_write(s, cp_crlf,2);
  s_write(s, cp_crlf,2);
  s_write(s, ".",1);
  s_write(s, cp_crlf,2);

  //	Check for 250 response
  cp_buffer[0] = '\0';
  s_read(s, cp_buffer, 4095);
  if(strstr(cp_buffer, "250") == NULL)	{
	NETCLOSE(s);
	fclose(fd);
	_HTProgress("SMTP host did not accept message.");
	sleep(sleep_two);
	return(-1);
  }

  //	Send QUIT
  cp_buffer[0] = '\0';
  sprintf(cp_buffer, "QUIT%s", cp_crlf);
  s_write(s, cp_buffer, strlen(cp_buffer));

#ifdef FIXME

  //	Check for 221 response.
  cp_buffer[0] = '\0';
  s_read(s, cp_buffer, 4095);
  if(strstr(cp_buffer, "221") == NULL)	{
	NETCLOSE(s);
	fclose(fd);
	_HTProgress("SMTP host wouldn't end session.");
	sleep(sleep_two);
	return(-1);
  }

  //	Close the socket.  We're done.

#endif //fixme

  NETCLOSE(s);
  fclose(fd);
  _HTProgress("Mail message was sent.");
  sleep(sleep_two);
  return(1);

}