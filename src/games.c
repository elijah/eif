/*************************************************************
 *  $Id: games.c,v 1.6 2003/09/25 15:54:27 marcolz Exp $
 *
 *  games.c
 *
 *  Manage the games configured.
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
#ifdef	HAVE_UNISTD_H
#include	<unistd.h>
#endif				/* HAVE_UNISTD_H */

#include	"cntl.h"
#include	"data.h"
#include	"games.h"
#include	"print.h"

extern int	errno;

#define MAX_GAMES 50

typedef struct {
	char		name[BUFSIZ];	/* The local name */
	char		host[40];	/* The host id, name or internet id. */
	char		port[8];	/* Ascii of the port */
	char		country[BUFSIZ];/* The name of the country */
	char		rep[10];	/* The password into the country */
	int		protocol;	/* Which version of the game */
	int		xsize, ysize;	/* World dimensions */
	int		waittime;	/* Waiting time is seconds for data */
	char		datafile[BUFSIZ];/* Where game data is stored. */
}		game_data_t;

typedef struct {
	game_data_t	dt;	/* The above structure */
	int		socket;	/* If <> 0 and <> -1, the open socket to game */
	int		time, btus;
}		game_t;

game_t		games[MAX_GAMES];

static game_t	*find_game_loc(const char *name, int *which);

/*************************************
 * list_games
 *
 * Telling the user what the games are configured.
 */
void
list_games(gam)
	const char	*gam;
{
	int		printed = 0;
	int		i;
	game_t		*gt;

	for (gt = games, i = 0; i < MAX_GAMES; gt++, i++) {
		if (gt->dt.name[0] && (!gam || !*gam || !strcmp(gam, gt->dt.name))) {
			printed++;
			if (game_number == i) {
				if (gt->socket > 0) {
					prt("Game '%s', Current game, ** Connected\n",
						gt->dt.name);
				} else
					prt("Game '%s', Current game\n", gt->dt.name);
			} else {
				if (gt->socket > 0) {
					prt("Game '%s', ** Connected\n",
						gt->dt.name);
				} else
					prt("Game '%s'\n", gt->dt.name);
			}
			prt("  Host: %-20s  Port: %-5s  Coun: %s\n",
				gt->dt.host, gt->dt.port, gt->dt.country);
			prt("  World is %d by %d,   Protocol: %d,   Waittime: %d\n",
				gt->dt.xsize, gt->dt.ysize, gt->dt.protocol,
				gt->dt.waittime);
			prt("  Datadirectory: %s\n", gt->dt.datafile);
		}
	}
	if (!printed)
		prt("No games configured.\n");
}

/*************************************
 * find_game_loc
 *
 * Find the game name in the list of games,
 * if it exists.
 * Return a pointer to the entry, and which
 * game number it is.
 */
static game_t	*
find_game_loc(name, which)
	const char	*name;
	int		*which;
{
	int		i;
	game_t		*gt;

	for (gt = games, i = 0; i < MAX_GAMES; gt++, i++) {
		if (gt->dt.name[0] && !strcmp(name, gt->dt.name)) {
			*which = i;
			return (gt);
		}
	}
	*which = -1;
	return ((game_t *) 0);
}

/*************************************
 * add_game
 */
int
add_game(name, host, port, coun, rep, xsize, ysize, proto, waitt, dataf)
	const char	*name, *host, *port, *coun, *rep, *dataf;
	int		xsize, ysize, proto, waitt;
{
	int		i;
	game_t		*gt;

	if (find_game_loc(name, &i)) {
		eprt("Game %s already exists\n", name);
		return (1);
	}

	for (gt = games, i = 0; ((i < MAX_GAMES) && (gt->dt.name[0])); gt++, i++)
		;

	if (MAX_GAMES <= i) {
		eprt("Too many games configured, %d\n", MAX_GAMES);
		return (1);
	}

	if (strlen(name) >= sizeof(gt->dt.name)) {
		eprt("Name too big, max len of %lu\n", (long unsigned)sizeof(gt->dt.name) - 1);
		return (1);
	}

	if (strlen(host) >= sizeof(gt->dt.host)) {
		eprt("Host too big, max len of %lu\n", (long unsigned)sizeof(gt->dt.host) - 1);
		return (1);
	}

	if (strlen(port) >= sizeof(gt->dt.port)) {
		eprt("Port too big, max len of %lu\n", (long unsigned)sizeof(gt->dt.port) - 1);
		return (1);
	}

	if (strlen(coun) >= sizeof(gt->dt.country)) {
		eprt("Country name too big, max len of %lu\n", (long unsigned)sizeof(gt->dt.country) - 1);
		return (1);
	}

	if (strlen(rep) >= sizeof(gt->dt.rep)) {
		eprt("Rep too big, max len of %lu\n", (long unsigned)sizeof(gt->dt.rep) - 1);
		return (1);
	}

	if (strlen(dataf) >= sizeof(gt->dt.datafile)) {
		eprt("Dataf too big, max len of %lu\n", (long unsigned)sizeof(gt->dt.datafile) - 1);
		return (1);
	}

	if ((xsize % 2) || (0 >= xsize)) {
		eprt("Xsize %d wrong.  Must be even and greater than 0\n", xsize);
		return (1);
	}

	if ((ysize % 2) || (0 >= ysize)) {
		eprt("Ysize %d wrong.  Must be even and greater than 0\n", ysize);
		return (1);
	}

	if (waitt < 5) {
		eprt("Wait time of %d invalid.  Should be at least 5 seconds.\n", waitt);
		return (1);
	}

	strcpy(gt->dt.name, name);
	strcpy(gt->dt.host, host);
	strcpy(gt->dt.port, port);
	strcpy(gt->dt.country, coun);
	strcpy(gt->dt.rep, rep);
	strcpy(gt->dt.datafile, dataf);
	gt->dt.protocol = proto;
	gt->dt.xsize = xsize;
	gt->dt.ysize = ysize;
	gt->dt.waittime = waitt;

	gt->socket = -1;
	gt->time = 0;
	gt->btus = 0;

	return (0);
}

void
switch_gameout()
{
	game_t		*gt;

	if ((0 <= game_number) && (MAX_GAMES > game_number)) {
		/* Save the data from the current game */
		gt = &games[game_number];

		gt->socket = game_socket;
		gt->time = game_time;
		gt->btus = game_btus;
	}
	game_socket = -1;
	if (cntl.usr.homedir)
		(void) chdir(cntl.usr.homedir);
	game_number = -1;
}

int
switch_to(game)
	char		*game;
{
	int		which;
	game_t		*gt;

	if (!(gt = find_game_loc(game, &which))) {
		eprt("No such game, '%s'.\n", game);
		return (0);
	}
	if (game_number == which) {
		eprt("Already connected to '%s'.\n", game);
	} else {

		switch_gameout();

		game_number = which;
		game_socket = gt->socket;
		game_time = gt->time;
		game_btus = gt->btus;
		strcpy(game_name, gt->dt.name);
		strcpy(game_host, gt->dt.host);
		strcpy(game_port, gt->dt.port);
		strcpy(game_country, gt->dt.country);
		strcpy(game_rep, gt->dt.rep);
		game_wait = gt->dt.waittime;
		game_xsize = gt->dt.xsize;
		game_ysize = gt->dt.ysize;
		game_protocol = gt->dt.protocol;
		strcpy(game_dataf, gt->dt.datafile);
	}

	/* Change to the data directory.  First make sure we are at home. */
	if (cntl.usr.homedir)
		(void) chdir(cntl.usr.homedir);
	if (chdir(game_dataf))
		eprt("%d:Unable to chdir to game directory: %s\n", errno, game_dataf);

	return (1);
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
