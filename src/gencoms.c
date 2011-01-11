/*************************************************************
 *  $Id: gencoms.c,v 1.3 2003/04/27 19:39:28 marcolz Exp $
 *
 *  gencoms.c
 *
 *  Some general commands.
 */
/*******************************************************
 *  Copyright (C) Doug Hay, 1991.
 *  Permission to use and abuse this code, as long
 *  as this copyright notice stays intact and with the
 *  code.  No warranty implied.  This code supplied as is.
 *******************************************************/

#include	"config.h"

#include	<ctype.h>
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

#include	"games.h"
#include	"gencoms.h"
#include	"print.h"

#ifdef	HAVE_READLINE_READLINE_H
#include	<readline/history.h>
#endif				/* HAVE_READLINE_READLINE_H */

char		readline_write_history_to[] = "/dev/tty";

/******************************************
 * cmd_echo
 *
 * The "echo" command.
 */
void
cmd_echo(buf, sub)
	const char	*buf;
	int sub		__attribute__((unused));
{
	while (isspace(*buf))
		buf++;
	prt("%s\n", buf);
}

/******************************************
 * cmd_beep
 *
 * Beep a bit.
 */
void
cmd_beep(buf, sub)
	const char	*buf __attribute__((unused));
	int sub		__attribute__((unused));
{
	eprt("");
}

/******************************************
 * cmd_games
 *
 * Show the configured games.
 */
void
cmd_games(buf, sub)
	const char	*buf;
	int sub		__attribute__((unused));
{
	char		*cp, cbuf[1024];

	while (isspace(*buf))
		buf++;

	strncpy(cbuf, buf, sizeof(cbuf));

	cp = cbuf;
	if (*cp) {
		while (*cp && !isspace(*cp))
			cp++;
		*cp = '\0';
	}
	list_games(cbuf);
}

/******************************************
 * cmd_history
 *
 */
void
cmd_history(buf, sub)
	const char	*buf __attribute__((unused));
	int sub		__attribute__((unused));
{
	/* Need some way of adding line numbers. */
	write_history(readline_write_history_to);
}

/******************************************
 * cmd_system
 *
 */
void
cmd_system(buf, sub)
	const char	*buf;
	int sub		__attribute__((unused));
{
	int		ret;

	ret = system(buf);
	if ((ret % 256) != 0)
		eprt("Problem in 'system' call, %d returned\n", (ret % 256));
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
