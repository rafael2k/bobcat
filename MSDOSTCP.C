
#ifdef MSDOS

/*
 *	This file was created by Garrett Arch Blythe <blythe@stat1.cc.ukans.edu>
 *	for the sole purpose of porting the WWW library to dos using the
 *	Wattcp Library and Borland C++.  This code will of course aid any
 *	other unix code porting using Berkeley sockets, but it is not fully
 *	functional in the way you would, or should expect due to many Dos
 *	and Wattcp limitations.
 *	No wattcp headers are included to avoid name clashes with the
 *	unix socket inteface.
 */
#include"msdostcp.h"
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"htalert.h"
#include"lyleaks.h"

#ifdef WWW
/*
 *	A global determining if we will even allow any of these functions
 *	to execute.
 */
static int i_networked = 1;

/*
 *	Things to speed up gethostbyname in WWW.
 */
typedef struct	BeenResolved_tag	{
	char *cp_hostname;
	unsigned long int uli_address;
	struct BeenResolved_tag *BR_next;
}
BeenResolved;

BeenResolved *BR_first = NULL;

static int localportnum = MIN_TCP_PORT;

static void freeResolved(void);
/*
 *	To free memory at program exit.
 */
void freeResolved(void)	{
	BeenResolved *BR_old;

	while(BR_first != NULL)	{
		BR_old = BR_first;
		BR_first = BR_first->BR_next;
		free(BR_old->cp_hostname);
		free(BR_old);
	}
}

#endif /* WWW */

/*
 *	Doslynx tracing variable.
 */
#ifndef RELEASE
#define RELEASE
//extern char c_trace;
#endif /* RELEASE */

/* Rotate the local port so that servers like www.yahoo.com don't hang */
int get_local_port(sockfd)
{
//    (localportnum > MAX_TCP_PORT) ? (localportnum = MIN_TCP_PORT) : localportnum++;
//    return localportnum;

      return (2048 + (rand() % 30000));

}

/*
 *	The global socket_table, other functions may query this table for
 *		information regarding a socket, if called from withing
 *		the msdostcp routines, which set ssi_sockfd to the correct
 *		value of the socket being referenced.
 *	Also, of course, if outside functions have the sockfd returned from
 *		a socket call, they can access the table using its value.
 */
signed short int ssi_sockfd = -1;
socket_info sock_table[MAXSOCKETS];

/*
 *	Used for initialization only once.
 */
char firsttime = 1;

static void initialize(void);

/*
 *	Function:	initialize
 *	Include File:	none
 *	Purpose:	Correctly start the wattcp kernal, and initialize
 *			our socket interface.
 *	Syntax:		static void initialize(void);
 *	Arguments:	void
 *	Return Value:	void
 *	Remarks/Portability/Dependencies/Restrictions:
 *		This function only callable by functions in this file (static).
 *	Revision History:
 *		11-23-93	created
 */
void initialize(void)	{
	auto int i_counter;

	/*
	 *	Can't allow access if turned off.
	 */
	if(i_networked == 0)	{
		return;
	}

	/*
	 * 	Only do this once.
	 */
	if(firsttime == 0)
		return;
	firsttime = 0;

#ifndef RELEASE
	if(c_trace)	{
                printf("%s:%d: Init Waterloo TCP.\n", __FILE__,
			__LINE__);
	}
#endif /* RELEASE */

	/*
	 * 	Initialize Wattcp.
	 *	Put a yield function for background processing, defined by
	 *		programmer.
	 */
	sock_init();
#ifdef WWW
//      sock_yield(NULL, yield);
#endif /* WWW */

	/*
	 * 	Initialize our socket interface.
	 *	We will support MAXSOCKETS socket connections because
	 *	wattcp only allows that many, I think.
	 */
	for(i_counter = 0; i_counter < MAXSOCKETS; i_counter++)	{
		sock_table[i_counter].vp_sockfd = NULL;
		sock_table[i_counter].usi_EOF =
			sock_table[i_counter].usi_IO =
			sock_table[i_counter].usi_OPEN = 0U;
		sock_table[i_counter].uli_read =
			sock_table[i_counter].uli_written = 0UL;
		sock_table[i_counter].uli_bindmyaddr = 0UL;
		sock_table[i_counter].usi_bindmyport = 0U;
		sock_table[i_counter].uc_used = 0U;
		sock_table[i_counter].ssi_backlog = 0;
		sock_table[i_counter].ssip_backlogfd = NULL;
	}

	/*
	 *	Initialize the random number generator, used to select
	 *	port numbers in bind.
	 */
	randomize();
}


/*
 *	Function:	socket
 *	Include File:	msdostcp.h
 *	Purpose:	Specify the type of communication protocol desired.
 *	Syntax:		int socket(int family, int type, int protocol);
 *	Arguments:	family		Specifies an address family with
 *					which addresses specified in later
 *					socket operations should be
 *					interpreted.
 *			type		Specifies the communication semantics.
 *			protocol	Specifies the protocol to be used.
 *	Return Value:
 *			int	On success, return value is a descriptor to
 *				the socket.
 *				On faulure, returns -1, and places the
 *				specific error in the global variable errno.
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Made for Ms-Dos using the Wattcp library to better conform
 *		to Berkeley sockets in the DOSLYNX WWW project.
 *		This function in no way attempts to be fully compatible with
 *		it's unix namesake.  Many families, types, and protocols
 *		are not supported and are not #defined in the header file.
 *		MsDos does not support many errno values.  The one supported
 *		will always be returned regardless of the real reason.
 *	Revision History:
 *		11-23-93	created GB
 *				supports AF_INET, SOCK_STREAM, IPPROTO_TCP
 */
int socket(int family, int type, int protocol)	{
	auto int i_counter;

	/*
	 *	Can't allow access if turned off.
	 */
	if(i_networked == 0)	{
		errno = EMFILE;
		return(-1);
	}

	/*
	 * 	Is this the first time that any of the msdostcp functions
	 *	are being called?  If so, intialize wattcp and our socket
	 *	interface.
	 */
	if(firsttime)
		initialize();

#ifndef RELEASE
	if(c_trace)	{
		printf("%s:%d: socket(%d, %d, %d);\n", __FILE__, __LINE__,
			family, type, protocol);
	}
#endif /* RELEASE */

	/*
	 *	A call to socket only returns a socket descriptor
	 *	and makes no connection.  We will set up a table to hold
	 *	the protocol, along with a wattcp socket pointer, and
	 *	return an integer describing the location of the socket in
	 *	our array of socket descriptors.
	 *
	 *	First, find an open socket in our table.
	 */
	for(i_counter = 0; i_counter < MAXSOCKETS; i_counter++)
		if(sock_table[i_counter].uc_used == 0U)
			break;
	if(i_counter == MAXSOCKETS)	{
		errno = EMFILE;
		return(ssi_sockfd = -1);
	}

	/*
	 * 	Now all we do is assing the correct protocol in the
	 *	table and return it's integer reference.
	 *	Use else ifs to expand this code if further implementation
	 *	is required.
	 */
	if(family == AF_INET)	{
		sock_table[i_counter].i_family = AF_INET;
		if(type == SOCK_STREAM)	{
			sock_table[i_counter].i_type = SOCK_STREAM;
			if(protocol == IPPROTO_TCP || protocol == 0)	{
				sock_table[i_counter].i_protocol = IPPROTO_TCP;
				/*
				 *	Mark the socket as being in use.
				 */
				sock_table[i_counter].uc_used = 1U;
				return(ssi_sockfd = i_counter);
			}
		}
	}

	/*
	 *	Must not be implemented yet!
	 */
	errno = EMFILE;
	return(ssi_sockfd = -1);
}

/*
 *	Function:	connect
 *	Include File:	msdostcp.h
 *	Purpose:	establish a connection with a server.
 *	Syntax:		int connect(int sockfd, struct sockaddr *servaddr,
 *				int addrlen);
 *	Arguments:	sockfd		the socket descriptor returned by
 *					a call to socket
 *			servaddr	contains the address family and up
 *					to 14 bytes of protocol-specific
 *					address
 *			addrlen		the size of the protocol-specific
 *					address
 *	Return Value:	int	0 on success.
 *				-1 on failure, and sets errno.
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Made for Ms-Dos using the Wattcp library to better conform
 *		to Berkeley sockets in the DOSLYNX WWW project.
 *		This function in no way attempts to be fully compatible with
 *		it's unix namesake.  Many families, types, and protocols
 *		are not supported and are not #defined in the header file.
 *		MsDos does not support many errno values.  The ones supported
 *		will always be returned regardless of the real reason.
 *	Revision History:
 *		11-29-93	created
 */
int connect(int sockfd, struct sockaddr *servaddr, int addrlen)	{
	auto int sock_status = 0;

	/*
	 *	Can't allow access if turned off.
	 */
	if(i_networked == 0)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Set the global index
	 */
	ssi_sockfd = sockfd;

#ifndef RELEASE
	if(c_trace)	{
		printf("%s:%d: connect(%d, servaddr, %d);\n", __FILE__,
			__LINE__, sockfd, addrlen);
	}
#endif /* RELEASE */

	/*
	 *	Luckily, all sockaddr_* structures have the first 16 bytes
	 *	set to the family.
	 *	Check the family for validity with the socket descriptor.
	 *	Don't want our users mixing up their sockets....
	 */
	if(servaddr->sa_family != sock_table[sockfd].i_family)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 * 	Do the connection to the socket of appropriate type.
	 *	Add else ifs here to expand the implementation.
	 */
	if(sock_table[sockfd].i_family == AF_INET)	{
		if(sock_table[sockfd].i_type == SOCK_STREAM)	{
			if(sock_table[sockfd].i_protocol == IPPROTO_TCP)	{
				auto struct sockaddr_in *servaddr_in =
					(struct sockaddr_in *)servaddr;

				/*
				 * 	Allocate a tcp socket
				 */
				if(NULL == (sock_table[sockfd].vp_sockfd =
					malloc(sizeof(tcp_Socket))))	{
					errno = EBADF;
					return(-1);
				}

				/*
				 * 	Open the socket.
				 *	WWW does their code right, but
				 *	Wattcp may not; WWW calls htons
				 *	but Wattcp switches the bytes
				 *	even though it doesn't need to.
				 *	We simply will switch the bytes
				 *	again if WWW defined.
				 */
				if(tcp_open(sock_table[sockfd].vp_sockfd,
					get_local_port(sockfd),
					servaddr_in->sin_addr.s_addr,
#ifdef WWW
					ntohs(servaddr_in->sin_port),
#else
					servaddr_in->sin_port,
#endif /* WWW */
					NULL) == 0)	{
					errno = EBADF;
					return(-1);
				}

				/*
				 *	We will only use binary sockets
				 *	with a nagle algorithm.
				 */
				sock_mode(sock_table[sockfd].vp_sockfd,
					TCP_MODE_BINARY);
				sock_mode(sock_table[sockfd].vp_sockfd,
					TCP_MODE_NAGLE);

				/*
				 *	Set some of the socket members.
				 */
				sock_table[sockfd].usi_EOF =
					sock_table[sockfd].usi_IO =
					sock_table[sockfd].usi_OPEN = 0U;
				sock_table[sockfd].uli_read =
					sock_table[sockfd].uli_written = 0UL;
				sock_table[sockfd].uli_bindmyaddr = 0UL;
				sock_table[sockfd].usi_bindmyport = 0U;

				/*
				 * 	Wait for remote host verification.
				 *	This will goto sock_err on failure.
				 */
				sock_wait_established(
					sock_table[sockfd].vp_sockfd,
					sock_delay, NULL, &sock_status);

				/*
				 *	Mark that we have an open socket.
				 * 	We have a good tcp connection.
				 */
				sock_table[sockfd].usi_OPEN = 1;
				return(0);
			}
		}
	}

	/*
	 * 	Must not be implemented yet or goto sock_err!
	 *	Free the socket if allocated and correct the table.
	 */
sock_err:
	if(sock_status == -1 && sock_table[sockfd].vp_sockfd != NULL)
//                fprintf(stderr, "WATTCP:  %s\n", sockerr(sock_table[sockfd].vp_sockfd));
HTProgress(sockerr(sock_table[sockfd].vp_sockfd));
// sleep(0);
	if(sock_table[sockfd].vp_sockfd != NULL)	{
		free(sock_table[sockfd].vp_sockfd);
		sock_table[sockfd].vp_sockfd = NULL;
	}
	errno = EBADF;
	return(-1);
}

/*
 *	Function:	bind
 *	Include File:	msdostcp.h
 *	Purpose:	Binds a name to a socket.
 *	Syntax:		int bind(int sockfd, struct sockaddr *myaddr,
 *				int addrlen);
 *	Arguments:	sockfd	Specifies the socket descriptor of the
 *				socket to be bound.
 *			myaddr	Points to an address structure that specifies
 *				the address to which the socket should be
 *				bound.
 *			addrlen	Specifies the length of the socket address
 *				structure.
 *	Return Value:	int	0	success.
 *				-1	failure, errno is set.
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Made for Ms-Dos using the Wattcp library to better conform
 *		to Berkeley sockets in the DOSLYNX WWW project.
 *		This function in no way attempts to be fully compatible with
 *		it's unix namesake.  Many families, types, and protocols
 *		are not supported and are not #defined in the header file.
 *		MsDos does not support many errno values.  The ones supported
 *		will always be returned regardless of the real reason.
 *	Revision History:
 *		12-09-93	created, always fail
 *		03-17-94	Began modifications to implement bind, listen,
 *				and accept to work in a Berkeley fashion.
 *				No allocation of a wattcp socket will occur
 *				until the call to accept is issued.
 *		03-23-94	Modified so that a port request of zero means
 *				that we assign a valid port number.
 *		03-23-94	Sockets are now actually created in listen,
 *				not accept.
 */
int bind(int sockfd, struct sockaddr *myaddr, int addrlen)	{

	/*
	 *	Can't allow access if turned off.
	 */
	if(i_networked == 0)	{
		errno = EBADF;
		return(-1);
	}


	/*
	 *	set the global index of the current socket being referenced.
	 */
	ssi_sockfd = sockfd;

#ifndef RELEASE
	if(c_trace)	{
		printf("%s:%d: bind(%d, myaddr, %d);\n", __FILE__, __LINE__,
			sockfd, addrlen);
	}
#endif /* RELEASE */

	/*
	 *	Luckily, all sockaddr_* structures have the first 16 bytes
	 *	set to the family.
	 *	Check the family for validity with the socket descriptor.
	 *	Don't want our users mixing up their sockets....
	 */
	if(myaddr->sa_family != sock_table[sockfd].i_family)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Make sure that we support a bind of the requested type.
	 */
	if(sock_table[sockfd].i_family == AF_INET)	{
		if(sock_table[sockfd].i_type == SOCK_STREAM)	{
			if(sock_table[sockfd].i_protocol == IPPROTO_TCP)
			{
				auto struct sockaddr_in *bindaddr_in =
					(struct sockaddr_in *)myaddr;

				/*
				 *	Set some of the socket table members.
				 */
				sock_table[sockfd].usi_EOF =
					sock_table[sockfd].usi_IO =
					sock_table[sockfd].usi_OPEN = 0U;
				sock_table[sockfd].uli_read =
					sock_table[sockfd].uli_written = 0UL;

				/*
				 *	If the requested port to bind is 0,
				 *	then we take over and assign one.
				 */
				if(bindaddr_in->sin_port == 0)	{
					/*
					 *	Assign the port randomly.
					 */
/*
					bindaddr_in->sin_port = htons((rand()
						% (MAX_TCP_PORT -
						MIN_TCP_PORT)) +
						MIN_TCP_PORT);
*/
					htons(bindaddr_in->sin_port =
					    get_local_port(sockfd));
				}

				/*
				 *	Here, just copy of the requested
				 *	local bind address and port into
				 *	the socket table.
				 */
				sock_table[sockfd].uli_bindmyaddr =
					bindaddr_in->sin_addr.s_addr;
				sock_table[sockfd].usi_bindmyport =
					bindaddr_in->sin_port;

				/*
				 *	Successfully bound in a round
				 *	about manner.
				 */
				return(0);
			}
		}
	}

	/*
	 *	As of yet, not supported.
	 */
	errno = EBADF;
	return(-1);
}

/*
 *	Function:	listen
 *	Include File:	msdostcp.h
 *	Purpose:	Listens for socket connections and limits the
 *			backlog of incoming connections (perhaps none).
 *	Syntax:		int listen(int sockfd, int backlog);
 *	Arguments:	sockfd	Specifies the socket descriptor to listen on.
 *			backlog	Specifies the maximum number of outstanding
 *				connection requests.
 *	Return Value:
 *			int	0	success, socket is listening.
 *				-1	failure, errno is set
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Made for Ms-Dos using the Wattcp library to better conform
 *		to Berkeley sockets in the DOSLYNX WWW project.
 *		This function in no way attempts to be fully compatible with
 *		it's unix namesake.  Many families, types, and protocols
 *		are not supported and are not #defined in the header file.
 *		MsDos does not support many errno values.  The ones supported
 *		will always be returned regardless of the real reason.
 *	Revision History:
 *		12-09-93	created, always fail
 *		03-17-94	Modifed to succeed, NO BAKCLOG WILL
 *				BE IMPLEMENTED.  Allocation of an actual
 *				wattcp socket is done in accept rather than
 *				here.  The listening socket really needs no
 *				socket at all.
 *		03-23-94	Oddly enough, sometimes a request for a
 *				socket will come in before a call to accept
 *				is made.  In this case, we will need to
 *				create the listening sockets here in listen.
 *				Accept will return those it has ready, or
 *				will in busywait until a connection comes in.
 *				backlog will now be implemented, but the
 *				number requested can not be guaranteed due
 *				to possible memory failures.
 */
int listen(int sockfd, int backlog)	{
	auto int i_counter;

	/*
	 *	Can't allow access if turned off.
	 */
	if(i_networked == 0)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Set the global index.
	 */
	ssi_sockfd = sockfd;

#ifndef RELEASE
	if(c_trace)	{
		printf("%s:%d: listen(%d, %d);\n", __FILE__, __LINE__, sockfd,
			backlog);
	}
#endif /* RELEASE */

	/*
	 *	We can only listen using tcp with WATTCP.
	 *	Make sure this is the protocol.
	 */
	if(sock_table[sockfd].i_family != AF_INET || sock_table[sockfd].
		i_type != SOCK_STREAM || sock_table[sockfd].i_protocol !=
		IPPROTO_TCP)	{
		errno = EBADF;
		return(-1);
	}


	/*
	 *	Backlog is atleast one, never greater than 5.
	 */
	if(backlog < 1)	{
		backlog = 1;
	}
	else if(backlog > 5)	{
		backlog = 5;
	}

	/*
	 *	Assign the number of possible backlogs and allocate memory
	 *	to keep track of which sockets will be in the backlog queue.
	 */
	sock_table[sockfd].ssi_backlog = (signed short int)backlog;
	sock_table[sockfd].ssip_backlogfd = (signed short int *)malloc(
		sizeof(signed short int) * backlog);
	if(NULL == sock_table[sockfd].ssip_backlogfd)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Get our sockets, allocate them.  If unable to allocate one,
	 *	close it and put -1 for the it and the rest.
	 */
	for(i_counter = 0; i_counter < backlog; i_counter++)	{
		*(sock_table[sockfd].ssip_backlogfd + i_counter) = socket(
			sock_table[sockfd].i_family,
			sock_table[sockfd].i_type,
			sock_table[sockfd].i_protocol);

		if(*(sock_table[sockfd].ssip_backlogfd + i_counter) != -1)
		{
			/*
			 *	Allocate each valid socket its WATTCP socket.
			 */
			sock_table[*(sock_table[sockfd].ssip_backlogfd +
				i_counter)].vp_sockfd = malloc(sizeof(
				tcp_Socket));

			if(sock_table[*(sock_table[sockfd].ssip_backlogfd +
				i_counter)].vp_sockfd == NULL)	{
				s_close(*(sock_table[sockfd].ssip_backlogfd +
					i_counter));
				*(sock_table[sockfd].ssip_backlogfd +
					i_counter) = -1;
				continue;
			}

			/*
			 *	Have each one listen.
			 *	Do not check establishment.
			 *	That is done in accept.
			 */
			tcp_listen(sock_table[*(sock_table[sockfd].
				ssip_backlogfd + i_counter)].vp_sockfd,
#ifdef WWW
				htons(sock_table[sockfd].usi_bindmyport),
#else
				sock_tablep[sockfd].usi_bindmyport,
#endif /* WWW */
				0UL, 0U, NULL, 0U);
		}
	}

	/*
	 *	It just so happens that if even the very first backlog
	 *	socket is not allocated, then this function has failed.
	 *	Check.
	 */
	if(*(sock_table[sockfd].ssip_backlogfd) == -1)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Return success.
	 */
	return(0);
}

/*
 *	Function:	accept
 *	Include File:	msdostcp.h
 *	Purpose:	Accepts a connection on a socket to create a new
 *			socket.
 *	Syntax:		int accept(int sockfd, struct sockaddr *peer,
 *				int *addrlen);
 *	Arguments:
 *			sockfd	specifies a socket created with the socket
 *				subroutine, bound to an address with the
 *				bind subroutine, and that has issued a
 *				successful call to the listen subroutine.
 *			peer	Specifies a result parameter that is filled
 *				in with the address of the connecting entity
 *				as known to the communications layer.
 *			addrlen	Specifies a parameter that initially contains
 *				the amount of space pointed to by the peer
 *				parameter.  Upon return, the parameter contains
 *				the actual length (in bytes) of the address
 *				returned.
 *	Return Value:	int	On successful completion, returns the socket
 *				descriptor of the new socket.
 *				On failure, -1 and sets errno.
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Made for Ms-Dos using the Wattcp library to better conform
 *		to Berkeley sockets in the DOSLYNX WWW project.
 *		This function in no way attempts to be fully compatible with
 *		it's unix namesake.  Many families, types, and protocols
 *		are not supported and are not #defined in the header file.
 *		MsDos does not support many errno values.  The ones supported
 *		will always be returned regardless of the real reason.
 *	Revision History:
 *		12-09-93	created, always fail
 *		03-17-94	Modified to work.  What a slice of hell MSDOS
 *				has cut out for me.
 *		03-23-94	Allocation of the listening sockets moved to
 *				listen.
 */
int accept(int sockfd, struct sockaddr *peer, int *addrlen)	{
	auto int newsockfd = -1;
	auto short int newsock_status = 0;
	auto struct hack_sockaddr hackpeer;
	auto struct sockaddr_in *peer_in;
	auto int i_counter, i_replacefd;

	/*
	 *	Can't allow access if turned off.
	 */
	if(i_networked == 0)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Set the global index.
	 *	Will of course change when we create the new socket.
	 */
	ssi_sockfd = sockfd;

#ifndef RELEASE
	if(c_trace)	{
		printf("%s:%d: accept(%d, peer, %d);\n", __FILE__, __LINE__,
			sockfd, *addrlen);
	}
#endif /* RELEASE */

	/*
	 *	We can only listen using tcp with WATTCP.
	 *	Make sure this is the protocol.
	 */
	if(sock_table[sockfd].i_family == AF_INET)	{
		if(sock_table[sockfd].i_type == SOCK_STREAM)	{
			if(sock_table[sockfd].i_protocol == IPPROTO_TCP)
			{
				/*
				 *	first off, check to make sure that
				 *	we have sockets listening.
				 */
				for(i_counter = 0; i_counter < sock_table
					[sockfd].ssi_backlog; i_counter++)
				{
					if(*(sock_table[sockfd].ssip_backlogfd
						+ i_counter) != -1)	{
						break;
					}
				}

				/*
				 *	If no sockets listening, return
				 *	failure.
				 */
				if(i_counter == sock_table[sockfd].
					ssi_backlog)	{
					errno = EBADF;
					return(-1);
				}

				/*
				 *	Busy wait until a socket is connected.
				 */
				while(1)	{
					for(i_counter = 0; i_counter <
						sock_table[sockfd].
						ssi_backlog; i_counter++)
					{
						if(*(sock_table[sockfd].
							ssip_backlogfd +
							i_counter) == -1)
						{
							continue;
						}

						if(sock_established(sock_table
							[*(sock_table[sockfd].
							ssip_backlogfd +
							i_counter)].vp_sockfd)
							!= 0)	{
							break;
						}
					}

					/*
					 *	break loop, have established.
					 */
					if(i_counter != sock_table[sockfd].
						ssi_backlog)	{
						break;
					}
				}

				newsockfd = *(sock_table[sockfd].
					ssip_backlogfd + i_counter);

				/*
				 *	create a new socket to listen
				 *	in place of the established socket.
				 */
				i_replacefd = socket(
					sock_table[sockfd].i_family,
					sock_table[sockfd].i_type,
					sock_table[sockfd].i_protocol);

				/*
				 *	if we have a replacement, allocate
				 *	memory for it.
				 */
				if(i_replacefd != -1)	{
					if(NULL == (sock_table[i_replacefd].
						vp_sockfd =
						malloc(sizeof(tcp_Socket))))
					{
						s_close(i_replacefd);
						i_replacefd = -1;
					}
					else	{
						/*
						 *	Have the replacement
						 *	listen also.
						 */
						tcp_listen(sock_table[
							i_replacefd].
							vp_sockfd,
#ifdef WWW
							htons(
							sock_table
							[sockfd].
							usi_bindmyport),
#else
							sock_table[sockfd].
							usi_bindmyport,
#endif /* WWW */
							0UL,
							0U, NULL, 0U);
					}
				}

				/*
				 *	Put the replacement in the listen
				 *	queue.  May be -1.
				 */
				*(sock_table[sockfd].ssip_backlogfd +
					i_counter) = i_replacefd;

				/*
				 *	Call a wattcp function to get the
				 *	peer information.
				 */
				*addrlen = sizeof(struct hack_sockaddr);
				getpeername(sock_table[newsockfd].vp_sockfd,
					&hackpeer, addrlen);

				/*
				 *	Copy over the peer information.
				 *	Assume a sockaddr_in structure for
				 *	peer.
				 */
				peer_in = (struct sockaddr_in *)peer;
				peer_in->sin_port = hackpeer.s_port;
				peer_in->sin_addr.s_addr =
					hackpeer.s_ip;
				peer_in->sin_family = AF_INET;

				/*
				 *	Since doing a tcp socket, return
				 *	the size of the tcp sockaddr in
				 *	addrlen.
				 */
				*addrlen = sizeof(struct sockaddr_in);

				/*
				 *	Okay, return the established socket.
				 */
				return(newsockfd);
			}
		}
	}

	/*
	 *	Not supported.
	 */
	errno = EBADF;
	return(-1);

/*
 *	Timeout or some other error.
 */
sock_err:
	if(newsock_status == -1)	{
//                fprintf(stderr, "WATTCP:  %s\n", sockerr(sock_table[newsockfd].vp_sockfd));
HTProgress(sockerr(sock_table[newsockfd].vp_sockfd));
// sleep(0);
	}
	if(sock_table[newsockfd].vp_sockfd != NULL)	{
		free(sock_table[newsockfd].vp_sockfd);
		sock_table[newsockfd].vp_sockfd = NULL;
		s_close(newsockfd);
	}
	errno = EBADF;
	return(-1);

}

/*
 *	Function:	getsockname
 *	Include File:   msdostcp.h
 *	Purpose:	Retrieve the locally bound address of the specified
 *			socket.
 *	Syntax:		int getsockname(int sockfd, struct sockaddr *peer,
 *				int *addlen);
 *	Arguments:	sockfd	specifies the socket for which the local
 *				address is desired.
 *			peer	points to the structure containing the local
 *				address of the specified socket.
 *			addlen	specifies the size of the local address in
 *				bytes.
 *	Return Value:	int	0 success.  The addlen parameter points to
 *				the size of the socket address.
 *				-1 failure and errno set.
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Made for Ms-Dos using the Wattcp library to better conform
 *		to Berkeley sockets in the DOSLYNX WWW project.
 *		This function in no way attempts to be fully compatible with
 *		it's unix namesake.  Many families, types, and protocols
 *		are not supported and are not #defined in the header file.
 *		MsDos does not support many errno values.  The ones supported
 *		will always be returned regardless of the real reason.
 *	Revision History:
 *		12-01-93	created, always fail
 *		03-14-94	modifed to hopefully work in some fashion as
 *				to be expected by Berkeley sockets.
 *		03-14-94	Decided not to modify, Changed WWW library
 *				to not use getsockname by defining POLL_PORTS.
 *		03-17-94	Modified to work with tcp sockets only.
 *				Figured out how along with other function
 *				modifications.
 *		03-22-04	Passing in a bound socket proved to be very
 *				dangerous as we tried to kludge filling in
 *				the address.  A real socket is never created
 *				for a bound socket.  Redefining POLL_PORTS
 *				for HTFTP.c
 */
int getsockname(int sockfd, struct sockaddr *peer, int *addlen)	{
#ifdef WWW

	/*
	 *	Can't allow access if turned off.
	 */
	if(i_networked == 0)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Need some debugging information.
	 */
#ifndef RELEASE
	if(c_trace != 0)	{
		printf("%s:%d: getsockname(%d, peer, %d);\n", __FILE__,
			__LINE__, sockfd, *addlen);
	}
#endif /* RELEASE */
#endif /* WWW */

	/*
	 *	Set the global socket index
	 */
	ssi_sockfd = sockfd;

	/*
	 *	Do we have a wattcp socket allocated?
	 */
	if(sock_table[sockfd].uc_used == 0U)	{
		/*
		 *	Sorry, return error.
		 */
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Make sure the socket is of appropriate type.
	 */
	if(sock_table[sockfd].i_family == AF_INET)	{
		if(sock_table[sockfd].i_type == SOCK_STREAM)	{
			if(sock_table[sockfd].i_protocol == IPPROTO_TCP)
			{
				auto struct hack_sockaddr hackpeer;
				auto struct sockaddr_in *peer_in;

				/*
				 *	Have wattcp get the information or
				 *	kludge it if socket not really there.
				 */
				if(sock_table[sockfd].vp_sockfd != NULL)	{
					*addlen = sizeof(struct
						hack_sockaddr);
					hack_getsockname(sock_table[sockfd].
						vp_sockfd, &hackpeer, addlen);
				}
				else	{
					/*
					 *	Try to fill it in.
					 */
					hackpeer.s_ip = sock_table[sockfd].
						uli_bindmyaddr;
					hackpeer.s_port = sock_table[sockfd].
						usi_bindmyport;
				}

				/*
				 *	Copy the information over into peer.
				 *	Assume peer is of type sockaddr_in
				 */
				peer_in = (struct sockaddr_in *)peer;
				peer_in->sin_addr.s_addr = hackpeer.s_ip;
				peer_in->sin_port = hackpeer.s_port;
				peer_in->sin_family = AF_INET;

				/*
				 *	Set the address length.
				 */
				*addlen = sizeof(struct sockaddr_in);

				/*
				 *	Success
				 */
				return(0);
			}
		}
	}

	/*
	 *	Not supported.
	 */
	errno = EBADF;
	return(-1);
}

/*
 *	Function:	s_close
 *	Include File:   msdostcp.h
 *	Purpose:	Close a socket connection.
 *	Syntax:		int s_close(int sockfd);
 *	Arguments:	sockfd	The socket descriptor to close.
 *	Return Value:	int	0 on success
 *				-1 on failure and sets errno.
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Made for Ms-Dos using the Wattcp library to better conform
 *		to Berkeley sockets in the DOSLYNX WWW project.
 *		This function in no way attempts to be fully compatible with
 *		it's unix namesake.  Many families, types, and protocols
 *		are not supported and are not #defined in the header file.
 *		MsDos does not support many errno values.  The ones supported
 *		will always be returned regardless of the real reason.
 *	Revision History:
 *		11-30-93	created
 *		03-17-94	Modified to close also, a socket that isn't
 *				allocate as is the case with the bind, listen,
 *				accept trio with the listening socket.
 */
int s_close(int sockfd)	{
	auto int sock_status = 0;

	/*
	 *	Can't allow access if turned off.
	 */
	if(i_networked == 0)	{
		errno = EBADF;
		return(-1);
	}

// HTProgress("Closing socket");

	/*
	 *	Set the global index.
	 */
	ssi_sockfd = sockfd;

#ifndef RELEASE
	if(c_trace)	{
		printf("%s:%d: s_close(%d);\n", __FILE__, __LINE__, sockfd);
	}
#endif /* RELEASE */

	/*
	 *	First, check to see if this is a valid sockfd
	 */
	if(sock_table[sockfd].vp_sockfd != NULL)	{
		/*
		 * 	Have wattcp close the connection.
		 *	Account for server sending reset.
		 *	Initiate the close.
		 *	Wait for the close.
		 */
		sock_tick(sock_table[sockfd].vp_sockfd, &sock_status);
		sock_close(sock_table[sockfd].vp_sockfd);

//                sock_wait_closed(sock_table[sockfd].vp_sockfd, sock_delay,
// BAD			NULL, &sock_status);

		sock_wait_closed(sock_table[sockfd].vp_sockfd, 1, NULL, &sock_status);
	}

	/*
	 * 	Regardless if gotoed here by wattcp error, free the socket,
	 *	it's buffer, and return success.
	 */
sock_err:
	/*
	 *	Determine here if a socket has a backlog (if it was listening).
	 *	If so, we have to free all sockets that are not establised.
	 */
	if(sock_table[sockfd].ssi_backlog != 0U &&
		sock_table[sockfd].ssip_backlogfd != NULL)	{
		auto int i_back;

		for(i_back = 0; i_back < (int)(sock_table[sockfd].ssi_backlog)
			; i_back++)	{

			/*
			 *	If there is no socket there, continue on.
			 */
			if(*(sock_table[sockfd].ssip_backlogfd + i_back) ==
				-1)	{
				continue;
			}

			s_close(*(sock_table[sockfd].ssip_backlogfd +
				i_back));
		}

		/*
		 *	Free the backlog buffer.
		 */
		free(sock_table[sockfd].ssip_backlogfd);
	}
	sock_table[sockfd].ssi_backlog = 0;
	sock_table[sockfd].ssip_backlogfd = NULL;

	/*
	 *	Set the table to indicate a close.
	 */
	sock_table[sockfd].uc_used = 0U;
	sock_table[sockfd].usi_EOF =
		sock_table[sockfd].usi_IO =
		sock_table[sockfd].usi_OPEN = 0U;
	sock_table[sockfd].uli_read =
		sock_table[sockfd].uli_written = 0UL;
	sock_table[sockfd].uli_bindmyaddr = 0UL;
	sock_table[sockfd].usi_bindmyport = 0U;
	if(sock_status == -1 && sock_table[sockfd].vp_sockfd != NULL)
	{
//                fprintf(stderr, "WATTCP:  %s\n", sockerr(sock_table[sockfd].vp_sockfd));
		HTProgress(sockerr(sock_table[sockfd].vp_sockfd));
//		sleep(5);
	}
	if(sock_table[sockfd].vp_sockfd != NULL)	{
		free(sock_table[sockfd].vp_sockfd);
		sock_table[sockfd].vp_sockfd = NULL;
	}
//HTProgress("Done");
	return(0);
}

/*
 *	Function:	s_read
 *	Include File:   msdostcp.h
 *	Purpose:	Read from a socket connection.
 *	Syntax:		int s_read(int sockfd, char *buff,
 *				unsigned int nbytes);
 *	Arguments:	sockfd	The socket descriptor to read from.
 *			buff	The buffer to store the amount read.
 *			nbytes	The number of bytes to read.
 *	Return Value:	int     The number of bytes successfully read.
 *				0 on end of file
 *				-1 on failure and sets errno.
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Made for Ms-Dos using the Wattcp library to better conform
 *		to Berkeley sockets in the DOSLYNX WWW project.
 *		This function in no way attempts to be fully compatible with
 *		it's unix namesake.  Many families, types, and protocols
 *		are not supported and are not #defined in the header file.
 *		MsDos does not support many errno values.  The ones supported
 *		will always be returned regardless of the real reason.
 *	Revision History:
 *		11-30-93	created
 */
int s_read(int sockfd, char *buff, unsigned int nbytes)	{
	auto signed short int ssi_Status;

	/*
	 *	Can't allow access if turned off.
	 */
	if(i_networked == 0)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Set the global index.
	 */
	ssi_sockfd = sockfd;

#ifndef RELEASE
	if(c_trace)	{
		printf("%s:%d: s_read(%d, buff, %u);\n", __FILE__, __LINE__,
			sockfd, nbytes);
	}
#endif /* RELEASE */

	/*
	 * 	Make sure socket is valid.
	 */
	if(sock_table[sockfd].vp_sockfd == NULL)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Check usi_EOF to send eof, no more reads.
	 *	Provides a way to interrupt wattcp.
	 */
	if(sock_table[sockfd].usi_EOF)
		return(0);

	/*
	 *	Return the number of bytes read.
	 *	Mark that IO took place.  Record total IO.
	 */

	sock_wait_input(sock_table[sockfd].vp_sockfd, sock_delay, NULL,
		&ssi_Status);

	sock_table[sockfd].usi_IO = 1;
	{
	      short int retval = sock_fastread(sock_table[sockfd].vp_sockfd,
//	      short int retval = sock_read(sock_table[sockfd].vp_sockfd,
			buff, nbytes);
		buff[retval] = 0;
		if(retval > 0)
		{
			sock_table[sockfd].uli_read += retval;
			return((int)retval);
		}
		else
		{

			sock_wait_input(sock_table[sockfd].vp_sockfd, sock_delay, NULL,
				&ssi_Status);

			sock_table[sockfd].usi_IO = 1;

			retval = sock_read(sock_table[sockfd].vp_sockfd,
				buff, nbytes);
			buff[retval] = 0;
			if(retval > 0)
			{
				sock_table[sockfd].uli_read += retval;
		/*
		 *	Assume that sock_wait_input would produce any
		 *	error appropriate, and that a number below zero
		 *	should really be just 0
		 */
                              return(retval < 0 ? 0 : (int)retval);
//                                return((int)retval);
			}
			else
			{

				errno = EBADF;
				return(-1);
			}

		}
	}

sock_err:
	/*
	 *	Reaches here only if error occured while waiting for input.
	 */
	if(ssi_Status == -1 && sock_table[sockfd].vp_sockfd != NULL)
//                fprintf(stderr, "WATTCP:  %s\n", sockerr(sock_table[sockfd].vp_sockfd));
HTProgress(sockerr(sock_table[sockfd].vp_sockfd));
// sleep(0);
	return(-1);
}

/*
 *	Function:	s_write
 *	Include File:   msdostcp.h
 *	Purpose:	Write to a socket connection.
 *      Syntax:         int s_write(int sockfd, char *buff,
 *				unsigned int nbytes);
 *	Arguments:	sockfd	The socket descriptor to write to.
 *			buff	The buffer to write.
 *			nbytes	The number of bytes to write.
 *	Return Value:	int     The number of bytes successfully writen.
 *				-1 on failure and sets errno.
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Made for Ms-Dos using the Wattcp library to better conform
 *		to Berkeley sockets in the DOSLYNX WWW project.
 *		This function in no way attempts to be fully compatible with
 *		it's unix namesake.  Many families, types, and protocols
 *		are not supported and are not #defined in the header file.
 *		MsDos does not support many errno values.  The ones supported
 *		will always be returned regardless of the real reason.
 *	Revision History:
 *		11-30-93	created
 */
int s_write(int sockfd, char *buff, unsigned int nbytes)	{

	/*
	 *	Can't allow access if turned off.
	 */
	if(i_networked == 0)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Set the global index.
	 */
	ssi_sockfd = sockfd;

#ifndef RELEASE
	if(c_trace)	{
		printf("%s:%d: s_write(%d, buff, %u);\n", __FILE__, __LINE__,
			sockfd, nbytes);
	}
#endif /* RELEASE */

	/*
	 * 	Make sure socket is valid.
	 */
	if(sock_table[sockfd].vp_sockfd == NULL)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Check usi_EOF to send error.
	 *	Provides a way of interrupting wattcp.
	 */
	if(sock_table[sockfd].usi_EOF)	{
		errno = EBADF;
		return(-1);
	}

	/*
	 *	Mark that IO has taken place.  Record total IO.
	 *	Write to the socket, returning what wattcp gives us.
	 */
	sock_table[sockfd].usi_IO = 1;
	{
		short int retval = sock_write(sock_table[sockfd].vp_sockfd,
//		short int retval = sock_fastwrite(sock_table[sockfd].vp_sockfd,
			buff, nbytes);
		if(retval > 0)
			sock_table[sockfd].uli_written += retval;
		return((int)retval);
	}
}

/*
 *	Function:	gethostbyname
 *	Include File:   msdostcp.h
 *	Purpose:	Retrieves host address and name information using
 *			a host name as a search key.
 *	Syntax:		struct hostent *gethostbyname(char *hostname);
 *	Arguments:	hostname	the name of the host to retreive
 *					information on.
 *	Return Value:	struct hostent *	A static hostent structure
 *						that will be overwritten
 *						on subsequent calls but will
 *						hold current information or
 *						NULL, unable to resolve the
 *						hostname.
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Made for Ms-Dos using the Wattcp library to better conform
 *		to Berkeley sockets in the DOSLYNX WWW project.
 *		This function in no way attempts to be fully compatible with
 *		it's unix namesake.  Many families, types, and protocols
 *		are not supported and are not #defined in the header file.
 *		MsDos does not support many errno values.  The ones supported
 *		will always be returned regardless of the real reason.
 *		There is no h_errno value to be checked upon return, though
 *		it seems straightforward enough to implement it if need be.
 *	Revision History:
 *		11-30-93	created
 */
static struct hostent sh_return;

struct hostent *gethostbyname(char *hostname)	{
	static char *gethostfirsttime = (char *)1;
	void free_sh_return(void);
	unsigned long ul_address;

	/*
	 *	Can't allow access if turned off.
	 */
	if(i_networked == 0)	{
		return(NULL);
	}

#ifndef RELEASE
	if(c_trace)	{
		printf("%s:%d: gethostbyname(%s);\n", __FILE__, __LINE__,
			hostname);
	}
#endif /* RELEASE */

	/*
	 * 	Is this the first time that any of the msdostcp functions
	 *	are being called?  If so, intialize wattcp and our socket
	 *	interface.
	 */
	if(firsttime)
		initialize();

	/*
	 * 	Do this only once for proper initialization of our
	 *	static hostent structure
	 */
	if(gethostfirsttime != NULL)	{
		/*
		 *	These will always be the same for now.
		 */
		sh_return.h_addrtype = AF_INET;
		sh_return.h_length = sizeof(unsigned long);

		/*
		 *	Make sure we never do this again.
		 *	Make the name empty.
		 *	Set the aliases and addr_list to a NULL list.
		 */
		gethostfirsttime = NULL;
		sh_return.h_name = NULL;
		sh_return.h_aliases = &gethostfirsttime;
		sh_return.h_addr_list = &gethostfirsttime;

		/*
		 * 	Finally, at program exit, free all memory we have
		 *	taken.
		 */
		atexit(free_sh_return);
	}

	/*
	 * 	First, free all previous contents of our hostent structure.
	 */

/*	Putting this before a failure can cause a complete system lockup.
	It even completely and utterly locks up my Win95 machine!  WB

	free_sh_return();
*/

	/*
	 * 	Get the address resolution of the hostname.
	 *	If unable, return NULL.  Remember, no h_errno.
	 */

#ifdef WWW
	/*
	 *	To speed up things a bit, keep the return values from resolve
	 *	to help with a network intesive app like a WWW browser.
	 */
	{
		extern BeenResolved *BR_first;
		BeenResolved *BR = NULL;

		/*
		 *	if there are entries in the list, search for the
		 *	host name.
		 */
		if(BR_first != NULL)	{
			for(BR = BR_first; BR != NULL; BR = BR->BR_next)
			{
				if(!strcmp(BR->cp_hostname, hostname))
					break;
			}
		}

		/*
		 *	If we found a match, assign in the value.
		 */
		if(BR != NULL)	{
			ul_address = BR->uli_address;
		}
		/*
		 *	Otherwise, resolve normally.
		 */
		else	{
			BeenResolved *BR_new;

			if(0 == (ul_address = resolve(hostname)))	{
				return(NULL);
			}

			/*
			 *	Add to the list, if able.
			 *	Otherwise, forget it.
			 */
			if(NULL != (BR_new = (BeenResolved *)malloc(sizeof(
				BeenResolved))))	{
				if(BR_first == NULL)	{
					BR_first = BR_new;
					BR_first->BR_next = NULL;
					BR_first->uli_address = ul_address;
					BR_first->cp_hostname = (char *)
						malloc(strlen(hostname) + 1);
					if(BR_first->cp_hostname == NULL)
					{
						free(BR_first);
						BR_first = NULL;
					}
					else	{
						strcpy(BR_first->cp_hostname,
							hostname);
						/*
						 *	If firsttime, register
						 *	function to free list.
						 */
						if(BR_first->BR_next == NULL)
							atexit(freeResolved);
					}
				}
				else	{
					/*
					 *	Find last item.
					 */
					for(BR = BR_first; BR->BR_next !=
						NULL; BR = BR->BR_next)
						;

					BR->BR_next = BR_new;
					BR_new->BR_next = NULL;
					BR_new->uli_address = ul_address;
					BR_new->cp_hostname = (char *)
						malloc(strlen(hostname) + 1);
					if(BR_new->cp_hostname == NULL)
					{
						free(BR_new);
						BR->BR_next = NULL;
					}
					else	{
						strcpy(BR_new->cp_hostname,
							hostname);
					}
				}
			}
		}
	}
#else
	if(0 == (ul_address = resolve(hostname)))
		return(NULL);
#endif /* NOT WWW */

	/*
	 * 	First, free all previous contents of our hostent structure.
	 */

	free_sh_return();

	/*
	 * 	We have an address, assign it into h_addr_list.
	 *	Check for no memory.
	 *	Right now, we will only hold the one address resolve()
	 *	returns; though constructed for easier expansion if I ever
	 *	figure out how to correctly do this....
	 */
	if(NULL == (sh_return.h_addr_list = (char **)malloc(2 *
		sizeof(char *))))	{
		sh_return.h_addr_list = &gethostfirsttime;
		return(NULL);
	}
	if(NULL == (*(sh_return.h_addr_list) = (char *)malloc(
		sizeof(unsigned long))))	{
		free(sh_return.h_addr_list);
		sh_return.h_addr_list = &gethostfirsttime;
		return(NULL);
	}
	*((unsigned long *)(*(sh_return.h_addr_list))) = ul_address;
	*(sh_return.h_addr_list + 1) = NULL;

	/*
	 *      We must always assume that the hostname passed in is the
	 *	actual hostname since I am unable to implement this currently.
	 *	This means there we be no aliases, ever.  The code was made
	 *	to make this fairly easy to do if you ever need to try and
	 *	know how.
	 */
	if(NULL == (sh_return.h_name = (char *)malloc(strlen(hostname) + 1)))
		return(NULL);
	strcpy(sh_return.h_name, hostname);

	/*
	 * 	Done.
	 */
	return(&sh_return);
}

static void free_sh_return(void)	{
	/*
	 *	Can't allow access if turned off.
	 */
	if(i_networked == 0)	{
		return;
	}

	/*
	 * 	Free all previous contents of our hostent structure.
	 */
	if(sh_return.h_name != NULL)
		free(sh_return.h_name);
	if(*(sh_return.h_aliases) != NULL)	{
		auto int i_counter;

		for(i_counter = 0; *(sh_return.h_aliases + i_counter) != NULL;
			i_counter++)	{
			free(*(sh_return.h_aliases + i_counter));
		}
		free(sh_return.h_aliases);
	}
	if(*(sh_return.h_addr_list) != NULL)	{
		auto int i_counter;

		for(i_counter = 0; *(sh_return.h_addr_list + i_counter) != NULL;
			i_counter++)	{
			free(*(sh_return.h_addr_list + i_counter));
		}
		free(sh_return.h_addr_list);
	}
}

#ifdef WWW
/*
 * 	The following code has nothing to do with TCP but is included here
 *	in this file as a matter of convienience for use with the WWW
 *	DOSLYNX project.
 */

/*
 *	Function:	crypt
 *	Include File:	msdostcp.h
 *	Purpose:	Performs basic encryption of data.
 *	Syntax:		char *crypt(char *key, char *salt);
 *	Arguments:	key	Specifies an 8 character string which is used
 *				to change the encryption algorithm.
 *			salt	Specifies a 2 character string chosen from
 *				the set ["a-zA-Z0-9./"].  The salt parameter
 *				is used to vary the hashing algorithm in one
 *				of 4096 different ways.
 *	Return Value:	The crypt subroutine returns a pointer to the
 *			encrypted password.  The first two characters are the
 *			same as the salt parameter.
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Made for Ms-Dos to better conform to unix for the DOSLYNX
 *		WWW project.  This function in no way attempts to be fully
 *		compatible with it's unix namesake.  The return value points
 *		to static data that is overwritten by subsequent calls.
 *	Revision History:
 *		12-09-93	created, does nothing sortof to get code up
 */
char *crypt(char *key, char *salt)	{
	static char sc_buffer[11];

	strcpy(sc_buffer, salt);
	strcat(sc_buffer, key);
	return(sc_buffer);

#ifdef TODO
#error Complete crypt, source in \www\library\implemen\vms
#endif /* TODO */
}

/*
 *	The following variables are needed to help with DOS's segmented
 *	memory scheme and the Borland compiler in the large model, I think(?).
 *	Sometimes, when passing a pointer as a function argument, the
 *	validity of it's value is lost.  These variables, in conjunction
 *	with #ifdef statements help this problem; for now at least....
 *	If you wish to forgo the use of these variables, simply comment
 *	out the #define in msdostcp.h.  Using the variables will not cause
 *	any ill effects other than a minute amount of overhead.  Not
 *	using them may cause some of the hardest bugs to ever trace down.
 *	Lastly, all such errors as to be covered by the use of these
 *	variables are ones that didn't work on my compiler, with my
 *	setup, etc.  You may or may not have the same problem in different
 *	or the same functions, I suggest you use this fix if you can find
 *	these inconsistancies.
 */
void *vp_msdosmem;
void **vpp_msdosmem;

#endif /* WWW */
#endif /* MSDOS */
