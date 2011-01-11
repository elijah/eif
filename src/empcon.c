/*************************************************************
 *  $Id: empcon.c,v 1.6 2004/02/06 16:15:19 marcolz Exp $
 *
 *  empcon.c
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
#include	<stdio.h>
#ifdef	HAVE_STRING_H
#include	<string.h>
#endif				/* HAVE_STRING_H */
#ifdef	HAVE_STRINGS_H
#include	<strings.h>
#endif				/* HAVE_STRINGS_H */
#include	<sys/types.h>
#ifdef	HAVE_UNISTD_H
#include	<unistd.h>
#endif				/* HAVE_UNISTD_H */

#include	"cntl.h"
#include	"data.h"
#include	"empcon.h"
#include	"games.h"
#include	"getcom.h"
#include	"print.h"

#include	"connect/connect.h"
#include	"connect/sock.h"

/* Local */
static		const char *
next_online(const char *cmd, char *where,
		unsigned int len, const char *prompt,
		const char *erm);

/******************************************
 * cmd_connect
 */
void
cmd_connect(buf, sub)
	const char	*buf;
	int		sub;
{
	int		kill_it = 0;
	int		sock;
	char		nameb[512];

	/* We are doing a kill, rather than a connect */
	if (sub == 1)
		kill_it = 1;

	/* Parse or ask for the game name */
	if (!next_online(buf, nameb, sizeof(nameb), "Game name? ",
			 "Bad game name."))
		return;

	/* Switch into that game's context. */
	if (!switch_to(nameb))
		return;

	/* If we aren't already connected */
	if (-1 == game_socket) {
		if ((sock = empire_connect(game_host, game_port, game_country, game_rep,
						kill_it)) != -1) {
			eprt("Connected!\n");
			game_socket = sock;
			game_time = 0;
			game_btus = 0;
			if (-1 == readsock(sock, stdout, 20, (char *)0, 0))
				game_died();
		} else {
			if (!kill_it)
				eprt("Connection failed.\n");
			game_died();
		}
	} else
		eprt("Reconnected.\n");
}

/******************************************
 * cmd_try
 */
void
cmd_try(buf, sub)
	const char	*buf;
	int		sub;
{
	int		kill_it = 0;
	int		sock;
	char		nameb[20];
	char		coun[20];
	char		rep[20];

	if (sub == 1)
		kill_it = 1;

	if (!(buf = next_online(buf, nameb, sizeof(nameb), "Game name? ",
				"Bad game name.")))
		return;

	if (!switch_to(nameb))
		return;

	if (!(buf = next_online(buf, coun, sizeof(coun), "Country? ",
				"Bad country.")))
		return;

	if (!(buf = next_online(buf, rep, sizeof(rep), "Rep? ",
				"Bad rep.")))
		return;

	/* Make sure close an existing connection */
	if (-1 != game_socket) {
		(void) close(game_socket);
		game_socket = -1;
	}
	/* Try it. */
	if ((sock = empire_connect(game_host, game_port, coun, rep, kill_it))
		!= -1) {
		eprt("Connected!\n");
		game_socket = sock;
		game_time = 0;
		game_btus = 0;
		if (-1 == readsock(sock, stdout, 20, (char *)0, 0))
			game_died();
	} else {
		if (!kill_it)
			eprt("Connection failed.\n");
		game_died();
	}
}

/******************************************
 * game_died
 */
void
game_died()
{
	if (-1 != game_socket) {
		eprt("Game died.\n");
		(void) close(game_socket);
		game_socket = -1;
		switch_gameout();
	}
}

/******************************************
 * next_online
 *
 * Parse a command line for an option, and if it
 * doesn't exist, ask the user for it.
 *
 * Allows quoted strings.
 */
static	const char *
next_online(cmd, where, len, prompt, erm)
	char		*where;
	const char	*cmd, *prompt, *erm;
	unsigned int	len;
{
	const char	*cp;
	unsigned int	clen = 0;

	while (isspace(*cmd))
		cmd++;
	if (*cmd) {
		cp = cmd;
		if ('"' == *cp) {
			cp++;
			while (*cmd && '"' != *cmd)
				cmd++;
			if ('"' != *cmd) {
				eprt("Missing end \".  %s\n", erm);
				return ((char *)0);
			}
		} else
			while (*cmd && !isspace(*cmd))
				cmd++;

		clen = cmd - cp;

		if (*cmd)
			cmd++;
		if (clen >= len) {
			eprt("Too long.  %s\n", erm);
			return (NULL);
		}
		strncpy(where, cp, clen);
		where[clen] = 0;
	} else {
		if (!get_input(where, len, prompt))
			return (NULL);
		if (!*where) {
			eprt("%s\n", erm);
			return (NULL);
		}
	}

	return (cmd);
}

/******************************************
 * cmd_addgame
 */
void
cmd_addgame(cmd, sub)
	const char	*cmd;
	int sub		__attribute__((unused));
{
	char		nameb[BUFSIZ], hostb[40], portb[8], counb[BUFSIZ], repb[BUFSIZ],
			datafb[BUFSIZ];
	int		xsize, ysize, proto, waitt;

	if (!(cmd = next_online(cmd, nameb, sizeof(nameb), "Game name? ",
				"Bad game name.")))
		return;
	if (!(cmd = next_online(cmd, hostb, sizeof(hostb), "Host name or address? ",
				"Bad host name.")))
		return;
	if (!(cmd = next_online(cmd, portb, sizeof(portb), "Game port? ",
				"Bad game port.")))
		return;
	if (!(cmd = next_online(cmd, counb, sizeof(counb), "Country name? ",
				"Bad country name.")))
		return;
	if (!(cmd = next_online(cmd, repb, sizeof(repb), "Representative? ",
				"Bad representative.")))
		return;
	if (!(cmd = next_online(cmd, datafb, sizeof(datafb), "Data file name? ",
				"Bad datafile name.")))
		return;

	/* Currently, the only useful option is waitt. */
	xsize = 64;
	ysize = 32;
	proto = 1;
	waitt = 120;

	if (add_game(nameb, hostb, portb, counb, repb, xsize, ysize, proto,
			waitt, datafb)) {
		eprt("Add of country failed.\n");
	} else
		prt("Game %s added.\n", nameb);
}

/******************************************
 * event_hook
 * readline calls this whenever it's waiting for input.
 * I've seen the idea in LAFE. Why should we abandon readline yet ?
 */
void
event_hook(void)
{
	FD_SET_NEEDS_STRUCT fd_set	checkfds;
	int				ret_val;

	/* Check if something came in from the game_socket. */

	if (-1 == game_socket)
		return;

	FD_ZERO(&checkfds);
	/* For now we just check the current game_socket. TODO: check
	 * all open sockets.
	 */
	FD_SET(0, &checkfds);		/* stdin */
	FD_SET(game_socket, &checkfds);	/* current game */
	/* Block until something needs to be done. */
	ret_val = select(FD_SETSIZE, &checkfds, NULL, NULL, NULL);

	if (ret_val > 0)
	{
		/* stdin is handled by readline */
		if (FD_ISSET(0, &checkfds))
			return;

		while ((ret_val = readsock(game_socket, output_curout(),
				0, NULL, 0)) > 0)
			;
		if (-1 == ret_val)
			game_died();
	}

	return;
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
