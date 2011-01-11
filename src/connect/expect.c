/****************************************************************
 * $Id: expect.c,v 1.4 2003/09/25 15:54:28 marcolz Exp $
 *
 * expect.c
 *
 * Shamelessly thieved from the empire client code written
 * by Dave Pare.
 *
 * Modified as well.
 *
 */
#include	"config.h"

#include	<stdio.h>
#include	<ctype.h>
#ifdef	HAVE_STRING_H
#include	<string.h>
#endif				/* HAVE_STRING_H */
#ifdef	HAVE_STRINGS_H
#include	<strings.h>
#endif				/* HAVE_STRINGS_H */
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#ifdef	HAVE_UNISTD_H
#include	<unistd.h>
#endif				/* HAVE_UNISTD_H */

#include	"proto.h"
#include	"expect.h"

#include	"../print.h"

struct fn {
	int		(*func) (void);
	const char	*name;
	int		value;
};

struct fn	fnlist[] = {
	{NULL, "user", USER,},
	{NULL, "coun", COUN,},
	{NULL, "quit", QUIT,},
	{NULL, "pass", PASS,},
	{NULL, "play", PLAY,},
	{NULL, "list", LIST,},
	{NULL, "cmd", CMD,},
	{NULL, "ctld", CTLD,},
	{NULL, "wat", WAT,},
	{NULL, "kill", KILL,},
	{NULL, "", 0,},
};

int
expect(s, match, buf)
	int		s;
	int		match;
	char		*buf;
{
	size_t		size;
	char		*p;
	int		n;
	int		code;
	int		newline;
	char		*ptr;
	int		cc;

	size = 1024;
	/* This was 10.  A little too slow. */
	(void)alarm(60);
	ptr = buf;
	n = recv(s, ptr, size, MSG_PEEK);
	if (n <= 0) {
		eprt("Expecting code %d\n", match);
		perror("recv");
		return 0;
	}
	size -= (size_t) n;
	buf[n] = '\0';
	if ((p = index(ptr, '\n')) == 0) {
		do {
			cc = read(s, ptr, (size_t) n);
			if (cc < 0) {
				perror("expect: read");
				return 0;
			}
			if (cc != n) {
				eprt("expect: short read (%d not %d)\n",
					cc, n);
				return 0;
			}
			ptr += n;
			if ((n = recv(s, ptr, size, MSG_PEEK)) <= 0) {
				eprt("1Expecting %d, got %s\n",
					match, buf);
				return 0;
			}
			size -= (size_t) n;
			ptr[n] = '\0';
		} while ((p = index(ptr, '\n')) == 0);
		newline = 1 + p - buf;
		*p = 0;
	} else
		newline = 1 + p - ptr;
	cc = read(s, buf, (size_t) newline);
	if (cc < 0) {
		perror("expect: read #2");
		return 0;
	}
	if (cc != newline) {
		eprt("expect: short read #2 (%d not %d)\n",
			cc, newline);
		return 0;
	}
	buf[newline] = '\0';
	(void)alarm(0);
	if (!isxdigit(*buf)) {
		eprt("2Expecting %d, got %s\n", match, buf);
		return 0;
	}
	if (isdigit(*buf))
		code = *buf - '0';
	else {
		if (isupper(*buf))
			*buf = (char) tolower(*buf);
		code = 10 + *buf - 'a';
	}
	if (code == match)
		return 1;
	return 0;
}

int
sendcmd(s, cmd, arg)
	int		s;
	int		cmd;
	const char	*arg;
{
	char		buf[80];
	int		cc;
	unsigned int	len;

	(void)sprintf(buf, "%s %s\n", fnlist[cmd].name, arg != 0 ? arg : "");
	len = strlen(buf);
	cc = write(s, buf, len);
	if (cc < 0) {
		perror("sendcmd: write");
	}
	if ((size_t) cc != len) {
		eprt("sendcmd: short write (%d not %d)\n", cc, len);
	}
	return (0);
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
