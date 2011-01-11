/*************************************************************
 *  $Id: connect.c,v 1.4 2003/09/25 15:54:27 marcolz Exp $
 *
 *  connect.c
 *
 *  Much of this shamelessly cloned from elsewhere.
 *
 */
/*******************************************************
 *  Copyright (C) Doug Hay, 1991.
 *  Permission to use and abuse this code, as long
 *  as this copyright notice stays intact and with the
 *  code.  No warranty implied.  This code supplied as is.
 *******************************************************/
#include	"config.h"

#include	<ctype.h>
#include	<errno.h>
#include	<netdb.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netinet/tcp.h>
#include	<stdio.h>
#ifdef	HAVE_STDLIB_H
#include	<stdlib.h>
#endif				/* HAVE_STDLIB_H */
#ifdef	HAVE_STRING_H
#include	<string.h>
#endif				/* HAVE_STRING_H */
#ifdef	HAVE_STRINGS_H
#include	<strings.h>
#endif				/* HAVE_STRINGS_H */
#ifdef	HAVE_UNISTD_H
#include	<unistd.h>
#endif				/* HAVE_UNISTD_H */

#include	"connect.h"
#include	"login.h"

#include	"../print.h"

#ifndef USE_IPv6
static int	get_hostaddr(char *host, struct sockaddr_in * addr);
static int	get_hostport(char *port, struct sockaddr_in * addr);
static int	hostconnect(struct sockaddr_in * addr);
#else
static int	hostconnect(struct addrinfo * addr);
#endif

int
empire_connect(host, port, country, rep, kill_it)
	char		*host, *port, *country, *rep;
	int		kill_it;
{
	int		sock;
	const char	*user;
#ifndef USE_IPv6
	struct sockaddr_in addr;

	if (!get_hostaddr(host, &addr)) {
		eprt("Unable to get address of %s\n", host);
		return (-1);
	}
	if (!get_hostport(port, &addr)) {
		eprt("Unable to convert port %s\n", port);
		return (-1);
	}
	sock = hostconnect(&addr);
#else
	struct addrinfo *ainfo, hints;
	int		i;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((i = getaddrinfo(host, port, &hints, &ainfo))) {
		eprt("%s\n", gai_strerror(i));
		return (-1);
	}
	sock = hostconnect(ainfo);
	freeaddrinfo(ainfo);
#endif
	if (-1 == sock) {
		eprt("Connection failed.\n");
		return (-1);
	}
	if (!(user = getenv("USER")))
		user = "unknown";

	if (!login(sock, user, country, rep, kill_it)) {
		eprt("Login failed.\n");
		(void) close(sock);
		return (-1);
	}
	if (kill_it) {
		(void) close(sock);
		return (-1);
	}
	return (sock);
}
#ifndef USE_IPv6
/******************************************
 * get_hostaddr
 *
 * Find the actual net address from the host name.
 * The host name can be an internet id, or a name.
 *
 * Returns 1 if ok, 0 if it failed.
 */
static int
get_hostaddr(host, addr)
	char		*host;
	struct sockaddr_in *addr;
{
	struct hostent *hptr;

	if (!host || !*host)
		return (0);

	if (isdigit(*host)) {
		/* Well, it could be an internet address. */
		/* Could also be a name starting with a numeric.  sick. */
		addr->sin_addr.s_addr = inet_addr(host);
		if ((unsigned)-1 != addr->sin_addr.s_addr) {
			/* Yeah, worked. */
			return (1);
		}
	}
	hptr = gethostbyname(host);
	if (!hptr)
		return (0);

	bcopy(hptr->h_addr, (char *)&addr->sin_addr, sizeof(addr->sin_addr));
	return (1);
}

/******************************************
 * get_hostport
 *
 * Convert the port string into the proper port id
 * in the socket address.
 * If "port" is a numeric string, add it in directly.
 * Otherwise, look it up in the services.
 *
 * Returns 1 if ok, 0 if it fails.
 */
static int
get_hostport(port, addr)
	char			*port;
	struct sockaddr_in	*addr;
{
	struct servent *serv;

	if (!port || !*port)
		return (0);

	if (isdigit(*port)) {
		addr->sin_port = htons((unsigned short)atol(port));
	} else {
		serv = getservbyname(port, "tcp");
		if (!serv)
			return (0);
		addr->sin_port = serv->s_port;
	}
	return (1);
}
#endif
/******************************************
 * hostconnect
 *
 * Returns -1 if terrible error occurs.
 * Returns socket number if succeeds.
 */
static int
hostconnect(addr)
#ifndef USE_IPv6
	struct sockaddr_in *addr;
#else
	struct addrinfo *addr;
#endif
{
	int		sock;

#ifndef USE_IPv6
	sock = socket(AF_INET, SOCK_STREAM, 0);
#else
	sock = socket(addr->ai_family, addr->ai_socktype, 0);
	/*
	 * XXX should work as long as AF_INET=PF_INET and AF_INET6=PF_INET6
	 * ai_family is PF_INET or PF_INET6 fix this
	 */
#endif
	if (-1 == sock) {
		perror("socket:");
		return (-1);
	}
#if 0
	/* This just prints some crap about the connection buffering */
	{
		int		t, dat, len;
		t = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&dat, &len);
		if (-1 == t) {
			eprt("getsockopt failed, %d\n", errno);
		} else {
			eprt("getsockopt, sndbuf, %d, %d\n", dat, len);
		}
		t = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&dat, &len);
		if (-1 == t) {
			eprt("getsockopt failed, %d\n", errno);
		} else {
			eprt("getsockopt, rcvbuf, %d, %d\n", dat, len);
		}
	}
#endif

#ifndef USE_IPv6
	addr->sin_family = AF_INET;
	if (connect(sock, (struct sockaddr *) addr, sizeof(*addr)) == -1) {
#else
	if (connect(sock, addr->ai_addr, (addr->ai_addrlen)) == -1) {
#endif
		switch (errno) {
		case ECONNREFUSED:	/* Connection refused */
			eprt("Connection refused.\n");
			eprt("Game is probably not up.\n");
			break;

		case ETIMEDOUT:/* Connection timed out. */
			eprt("Connection timed out.\n");
			eprt("Network problems?\n");
			break;

		case ENETUNREACH:	/* Network unreachable. */
			eprt("Network unreachable.\n");
			eprt("Good luck.\n");
			break;

		case EISCONN:	/* Already connected??!? */
			eprt("Already connected.\n");
			return (sock);

		default:
			perror("hostconnect:");
			break;
		}
		(void) close(sock);
		return (-1);
	}
#if 0
	/* This just prints some crap about the connection buffering */
	{
		int		t, dat, len;
		/* 6 is tcp's ip protocol number from /etc/protocols.. */
		t = getsockopt(sock, 6, TCP_NODELAY, (char *)&dat, &len);
		if (-1 == t) {
			eprt("getsockopt tcp failed, %d\n", errno);
		} else {
			eprt("getsockopt, nodelay, %d, %d\n", dat, len);
		}
		t = getsockopt(sock, 6, TCP_MAXSEG, (char *)&dat, &len);
		if (-1 == t) {
			eprt("getsockopt tcp failed, %d\n", errno);
		} else {
			eprt("getsockopt, maxseg, %d, %d\n", dat, len);
		}
	}
#endif
	return (sock);
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
