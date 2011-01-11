/*************************************************************
 * $Id: login.c,v 1.4 2003/09/25 15:54:28 marcolz Exp $
 *
 * login.c
 *
 * Shamelessly stolen from the original empire client.
 * With some minor modifications.
 */
/*
 * Log in to empire host
 * 
 * Dave Pare, 1989 */

#include	"config.h"

#include	"proto.h"

#include	<ctype.h>
#include	<stdio.h>
#ifdef	HAVE_STDLIB_H
#include	<stdlib.h>
#endif				/* HAVE_STDLIB_H */
#include	<sys/types.h>
#ifdef	HAVE_STRING_H
#include	<string.h>
#endif				/* HAVE_STRING_H */
#ifdef	HAVE_STRINGS_H
#include	<strings.h>
#endif				/* HAVE_STRINGS_H */

#include	"expect.h"
#include	"login.h"

#include	"../print.h"

int
login(s, uname, cname, cpass, kill_it)
	int		s;
	const char	*uname, *cname, *cpass;
	int		kill_it;
{
	char		buf[1024];
	char		*ptr;
	char		*p;

	if (!uname || !*uname || !cname || !*cname || !cpass || !*cpass)
		return (0);

	if (!expect(s, C_INIT, buf))
		return 0;
	(void)sendcmd(s, USER, uname);
	if (!expect(s, C_CMDOK, buf))
		return 0;
	(void)sendcmd(s, COUN, cname);
	if (!expect(s, C_CMDOK, buf)) {
		eprt("empire: no such country\n");
		return 0;
	}
	prt("\n");
	(void)sendcmd(s, PASS, cpass);
	if (!expect(s, C_CMDOK, buf)) {
		eprt("Bad password\n");
		return 0;
	}
	if (kill_it) {
		(void)sendcmd(s, KILL, (char *)0);
		prt("\n\t-=O=-\n");
		(void)expect(s, C_INIT, buf);
		eprt("%s\n", buf);
		return (1);
	}
	(void)sendcmd(s, PLAY, (char *)0);
	prt("\n\t-=O=-\n");
	if (!expect(s, C_INIT, buf)) {
		eprt("%s\n", buf);
		return 0;
	}
	for (ptr = buf; !isspace(*ptr); ptr++)
		;
	ptr++;
	p = index(ptr, '\n');
	if (p != 0)
		*p = 0;
	if (atoi(ptr) != CLIENTPROTO) {
		eprt("Empire client out of date; get new version!\n");
		eprt("   this version: %d, remote version: %d\n",
			CLIENTPROTO, atoi(ptr));
	}
	return 1;
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
