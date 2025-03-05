#ifdef MSDOS
#ifndef __MSDOSTCP_H
#define __MSDOSTCP_H
/*
 *	NOTE:	BIG HACK on WATTCP that you will have to do.
 *	Take care to change the wattcp *.c and *.h files so that the tokens
 *		#include<tcp.h>
 *		getsockname
 *		sockaddr
 *		sockaddr_in
 *		in_addr
 *	are now
 *		#include"c:\wattcp\include\tcp.h" or whatever the path is
 *		hack_getsockname
 *		hack_sockaddr
 *		hack_in_addr
 *
 *	By using #define statements at the start of important wattcp headers,
 *	it should be very easy to do.
 *	This is to avoid name clashes only with tokens that I will code
 *	for better WWW and Berkeley socket compatibility.
 *	As for the header file tcp.h, there is one in WWW and Wattcp.  To
 *	avoid confusion, make sure the WWW\LIBRARY\IMPLEMEN directory is
 *	first in the include path search, and in all the wattcp code,
 *	specify the full path for the tcp.h file as above.
 */

/*
 * 	Wattcp fails to implement sockets in a directly compatible manner.
 *	Must define some common things used in WWW and implement functions
 *	to do the same thing as open, socket, bind, etc... in unix.
 *	The compatible(?) implementation can be found in msdostcp.c
 */
#include<time.h>
#include<errno.h>
#include<sys\stat.h>
#include".\wattcp\include\tcp.h"
//#include"ttcp.h"
#include<fcntl.h>

extern char firsttime;

/*
 *	Range of ports assignable by the system.
 */
#define MIN_TCP_PORT	1024
#define MAX_TCP_PORT	5999

/*
 * 	Undefine those things that clash from wattcp.
 *	We will have to call wattcp code that uses these things by using
 *	the prefix hack_, except sockaddr_in, which we will substitute
 *	hack_sockaddr directly.  See above.
 */
#undef getsockname
#undef sockaddr
#undef sockaddr_in
#undef in_addr

#undef NETREAD
#undef NETWRITE
#undef NETCLOSE
#define NETREAD(s, b, l) s_read(s, b, l)
#define NETWRITE(s, b, l) s_write(s, b, l)
#define NETCLOSE(s) s_close(s)
#define NETCLOSE(s) s_close(s)

// #undef socketclose
// #define socketclose(s) s_close(s)
// #define recv(s, b, l, x) s_read(s, b, l)
// #define send(s, b, l, x) s_write(s, b, l)

#define bzero(x,y) memset(x,0,y)
#define bcopy(x,y,z) memcpy(y,x,z)

#define u_long unsigned long int
#define u_short unsigned short int


#define SOCK_STREAM 1
#define AF_INET 2
#define IPPROTO_TCP 6
#define INADDR_ANY (unsigned long)0x00000000
//#define SOCKET_ERRNO errno          /* normal socket errno */

#ifndef GLOBALREF
#define GLOBALDEF
#define GLOBALREF extern
#endif /* !GLOBALREF */

#define ENOTCONN -1
#define ECONNRESET -1
#define FIONBIO -1
#define EINPROGRESS -1
#define EALREADY -1
#define EISCONN -1

struct timeval {
   long    tv_sec;         /* seconds */
   long    tv_usec;        /* microseconds */
};

#define __FDSET_LONGS 8
#define __NFDBITS       (8 * sizeof(unsigned long))
#define __FD_SETSIZE    (__FDSET_LONGS*__NFDBITS)
#define FD_SETSIZE         __FD_SETSIZE
#define mode_t int

/*
typedef struct fd_set {
	unsigned long fds_bits [__FDSET_LONGS];
} fd_set;
*/

struct hostent	{
	char *h_name;
	char **h_aliases;
	int h_addrtype;
	int h_length;
	char **h_addr_list;
};

/* The passwd structure.  */
struct passwd
{
  char *pw_name;                /* Username.  */
  char *pw_passwd;              /* Password.  */
//  __uid_t pw_uid;               /* User ID.  */
//  __gid_t pw_gid;               /* Group ID.  */
  int  pw_uid;               /* User ID.  */
  int  pw_gid;               /* Group ID.  */
  char *pw_gecos;               /* Real name.  */
  char *pw_dir;                 /* Home directory.  */
  char *pw_shell;               /* Shell program.  */
};

#define h_addr h_addr_list[0]

struct servent {
	char *s_name;
	char **s_aliases;
	int s_port;
	char *s_proto;
};


struct sockaddr	{
	unsigned short sa_family;
	char sa_data[14];
};

struct in_addr	{
	unsigned long s_addr;
};

struct sockaddr_in	{
	short sin_family;
	unsigned short sin_port;
	struct in_addr sin_addr;
	char sin_zero[8];
};

static int SOCKET_ERRNO;

typedef struct sockaddr_in SockA;

int socket(int family, int type, int protocol);
int connect(int sockfd, struct sockaddr *servaddr, int addrlen);
struct hostent *gethostbyname(char *hostname);
struct servent *getservbyname(char *servname, char *protname);
int s_read(int fildes, char *buff, unsigned int nbytes);
int s_write(int fildes, char *buff, unsigned int nbytes);
int s_close(int fildes);
int accept(int sockfd, struct sockaddr *peer, int *addrlen);
int bind(int sockfd, struct sockaddr *myaddr, int addrlen);
int listen(int sockfd, int backlog);
/*
void start_curses();
void stop_curses();
*/

/*
 * 	Functions may not be implemented yet.
 *	Check msdostcp.c
 */

int getsockname(int sockfd, struct sockaddr *peer, int *addrlen);

/*
 *	Socket structure used in msdostcp
 */
typedef struct socket_info_tag	{
	int i_family, i_type, i_protocol;
	void *vp_sockfd;
	unsigned short int usi_EOF, usi_IO, usi_OPEN;
	unsigned long int uli_read, uli_written;
	unsigned long int uli_bindmyaddr;
	unsigned short int usi_bindmyport;
	unsigned char uc_used;
	signed short int ssi_backlog;
	signed short int *ssip_backlogfd;
}
socket_info;

/*
 *	MAXSOCKETS thought to be the limit of open sockets with WATTCP.
 *	Global variable for outside routines to recognize which socket
 *	is being referenced and the table to use the reference in.  -1 if
 *	invalid or not referencing anything.
 */
#define MAXSOCKETS 16
extern signed short int ssi_sockfd;
extern socket_info sock_table[MAXSOCKETS];

#ifdef WWW
/*
 * 	The following is to help in converting the WWW library to dos.
 *	The code can still be found in msdostcp.c although it has nothing
 *	to do with Berkeley sockets so to speak.
 */
char *crypt(char *key, char *salt);

/*
 *	The following is a function to be passed to wattcp's sock_yield
 *	function.
 *	Application specific;  source defined elsewhere.
 */
extern void yield(void);

#endif /* WWW */
#endif /* __MSDOSTCP_H */
#endif /* MSDOS */
