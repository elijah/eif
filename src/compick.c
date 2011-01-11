/*************************************************************
 *  $Id: compick.c,v 1.6 2003/09/25 15:54:27 marcolz Exp $
 *
 *  compick.c
 *
 *  Pick a command, any command.
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
#include	<string.h>
#endif				/* HAVE_STRINGS_H */

#include	"alias.h"
#include	"compick.h"
#include	"empcon.h"
#include	"getcom.h"
#include	"gencoms.h"
#include	"print.h"
#include	"vars.h"

/* Local to this file */
static void	cmd_list(const char *buf, int sub);

typedef struct {
	void		(*proc) (const char *buf, int sub);	/* The command procedure */
			const char *com;	/* The command string */
	int		minlen;	/* Min length to match, all if 0. */
	int		subcmdno;	/* Sub type to pass to procedure */
			const char *help;	/* Minor help string about
						 * command */
}		com_t;

#define CMD_NOP		((void (*)(const char *, int)) 0)
#define CMD_EOLIST	((void (*)(const char *, int)) -1)
#define CMD_QUIT	((void (*)(const char *, int)) -2)

static		const com_t coms[] = {
	{cmd_alias, "alias", 0, 0,
	"Set or show command aliases."},
	{cmd_unalias, "unalias", 0, 0,
	"Clear a single alias, or all aliases '*'."},
	{cmd_echo, "echo", 0, 0,
	"Echo some stuff."},
	{cmd_beep, "beep", 0, 0,
	"Send a Beep or ^G to the terminal."},
	{cmd_games, "games", 0, 0,
	"List the possible games."},
	{cmd_addgame, "addgame", 0, 0,
	"Add a game to the games list."},
	{cmd_connect, "connect", 4, 0,
	"Connect <game>"},
	{cmd_connect, "kill", 0, 1,
	"kill <game> - Try to kill a hung pid"},
	{cmd_try, "try", 0, 0,
	"Try <game> <cou> <rep> - Connect as another country."},
	{cmd_try, "trykill", 0, 1,
	"Trykill <game> <cou> <rep> - same as kill"},
#ifndef lint
	{CMD_QUIT, "exit", 0, 0,
	"Quit the program."},
#endif
	{cmd_list, "cmdlist", 0, 0,
	"List all the EIF commands."},
	{cmd_exec, "exec", 2, 0,
	"Read commands from a file."},
	{cmd_exec, "runfeed", 2, 1,
	"Execute a program, and use output as commands."},
	{cmd_history, "history", 0, 0,
	"Display the history."},
	{cmd_setvar, "setvar", 0, 0,
	"Set a variable"},
	{cmd_unsetvar, "unsetvar", 0, 0,
	"Unset a variable"},
	{cmd_system, "@", 0, 0,
	"Pass the command off to the system to execute"},
	{cmd_if, "if", 0, 0,
	"Uses /bin/test and stops at endif"},
	{cmd_if, "elseif", 0, 1,
	"Next case in an 'if' sequence."},
	{cmd_if, "else", 0, 2,
	"Last case in an 'if' sequence."},
	{cmd_if, "endif", 0, 3,
	"Ends an 'if' sequence."},
#ifndef lint
	{CMD_EOLIST, "", 0, 0, ""}
#endif
};

/************************************
 * direct_command
 *
 * Returns 1 if found and tried a command.
 * Returns 0 if couldn't find the command.
 */
int
direct_command(buf)
	char		*buf;
{
	const com_t	*ct, *ft;
	int		m = 0;
	unsigned int	l = 0;
	char		hold[100];

	while (isspace(*buf))
		buf++;
	if (*buf && !isalnum(*buf)) {
		hold[l++] = *buf++;
	} else {
		while (*buf && isalnum(*buf) && (l < (sizeof(hold) - 2)))
			hold[l++] = *buf++;
	}
	hold[l] = '\0';

	if (!l)
		return (0);

	ft = (com_t *) 0;
	for (ct = coms; ct->proc != CMD_EOLIST; ct++) {
		m = 0;
		if (!ct->minlen) {
			if (strlen(ct->com) == l)
				m = (strcmp(ct->com, hold) == 0);
		} else if (ct->minlen <= (signed) l)
			m = (strncmp(ct->com, hold, l) == 0);
		if (m && ft) {
			eprt("Command '%s' ambiguous.  More than one match.\n", hold);
			return (1);
		}
		if (m && !ft)
			ft = ct;
	}
	if (ft) {
		if (CMD_NOP == ft->proc) {
			prt("Sorry, command `%s' not implemented yet.\n", ft->com);
		} else if (CMD_QUIT == ft->proc) {
			eprt("Quitting....\n");
			exit(0);
		} else {
			(ft->proc) (buf, ft->subcmdno);
		}
		return (1);
	}
	return (0);
}

/************************************
 * cmd_list
 *
 * List the commands available.
 */
static void
cmd_list(buf, sub)
	const char	*buf __attribute__((unused));
	int sub		__attribute__((unused));
{
	const com_t	*ct;

	for (ct = coms; ct->proc != CMD_EOLIST; ct++) {
		prt("%10s : %s\n", ct->com, ct->help);
	}
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
