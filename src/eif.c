/*************************************************************
 *  $Id: eif.c,v 1.7 2003/09/25 15:54:27 marcolz Exp $
 *
 *  eif.c
 *
 *  This is where things actually get done after runtime
 *  commandline parsing.
 *
 */
/*******************************************************
 *  Copyright (C) Doug Hay, 1991.
 *  Permission to use and abuse this code, as long
 *  as this copyright notice stays intact and with the
 *  code.  No warranty implied.  This code supplied as is.
 *******************************************************/

#include	"config.h"

#include	<stdio.h>
#include	<signal.h>
#ifdef	HAVE_STRING_H
#include	<string.h>
#endif				/* HAVE_STRING_H */
#ifdef	HAVE_STRINGS_H
#include	<strings.h>
#endif				/* HAVE_STRINGS_H */
#include	"cntl.h"
#include	"compick.h"
#include	"data.h"
#include	"eif.h"
#include	"empcon.h"
#include	"getcom.h"
#include	"print.h"
#include	"connect/sock.h"

#ifdef	HAVE_READLINE_READLINE_H
#include	<readline/history.h>
#include	<readline/readline.h>
#endif				/* HAVE_READLINE_READLINE_H */

static void	eif_sigdebug(int sig);
static void	eif_interrupt(int sig);
static void	eif_pipe(int sig);
static void	commandloop(void);
static void	send_command(char *cmd);

static char	cmdbuf[1024];
char		inform_buffer[BUFSIZ];

/*************************************
 * eif_away
 *
 * The main routine.
 */
void
eif_away()
{

	/* Stuff for the readline. */
	(void) rl_unbind_key(17);	/* Ctrl-Q */
	(void) rl_unbind_key(19);	/* Ctrl-S */

	/* initialize signal handling */
	(void) signal(SIGQUIT, eif_sigdebug);	/* Debugging */

	(void) signal(SIGALRM, SIG_IGN);
	(void) signal(SIGPIPE, eif_pipe);

	(void) signal(SIGINT, eif_interrupt);

	/* We want ^C to abort system calls. */
	/* siginterrupt(SIGINT, 1); */

	prt("\nWelcome to EIF\n");
	prt("Empire Interface Version " VERSION "\n\n");

	/*
	 * The problem with this is that the file is not actually executed
	 * here.  It is executed in the commandloop(). This just sets it up
	 * for input as an exec file.
	 */
	if (cntl.st.readstartup) {
		cmd_exec(cntl.st.startupf, 2);
	}
	/* Only allow up to 100 commands in history */
	stifle_history(100);

	/* Install readline event hook */
	rl_event_hook = (Function *)event_hook;

	while (1)
		commandloop();
}

static void
eif_sigdebug(sig)
	int sig		__attribute__((unused));
{
	eprt("Intr\n");
}

static void
eif_pipe(sig)
	int sig		__attribute__((unused));
{
	Pipe_Interrupt++;
}

static void
eif_interrupt(sig)
	int sig		__attribute__((unused));
{
	eprt("Intr\n");
	Interrupt++;
}

/*******************************************
 * commandloop
 */
static void
commandloop()
{
	char		prompt[BUFSIZ];
	int		echoed;

	while (1) {
		/* Design a prompt */
		if (-1 == game_socket) {
			strcpy(prompt, "Local% ");
		} else
			snprintf(prompt, BUFSIZ, "\n\r%s %s%s%s[%d,%d]%% ",
				game_name,
				(inform_buffer[0]) ? "(" : "",
				(inform_buffer[0]) ? inform_buffer : "",
				(inform_buffer[0]) ? ") " : "",
				game_time, game_btus);

		/* Get the next command. */
		echoed = get_main_input(cmdbuf, sizeof(cmdbuf), prompt);

		/* Is it a local command. */
		if (!direct_command(cmdbuf)) {
			/* Are we connected to a game? */
			if (-1 != game_socket) {
				/*
				 * redirected emp_player commands get the
				 * commandline as their first line.  Various
				 * tools depend on this.
				 */
				if (!echoed)
					notstdio_prt("%s\n", cmdbuf);
				send_command(cmdbuf);
			} else {
				if (cmdbuf[0]) {
					/* Well, should say something. */
					eprt("Not a local command, and not connected to a game.\nType 'cmdlist' for a list of commands.\n");
				}
			}
		}
	}
}

/*******************************************
 * send_command
 */
static void
send_command(cmd)
	char		*cmd;
{
	int		t;
	char		prompt[200];

	/* Are we connected to a game? */
	if (-1 == game_socket)
		return;

	/* Send command to game */
	writesock(game_socket, cmd);

	/* Read from game until a prompt returns. */
	while ((t = readsock(game_socket, output_curout(), game_wait,
				prompt, sizeof(prompt))) == 2) {
		if (get_input(cmdbuf, sizeof(cmdbuf), prompt)) {
			writesock(game_socket, cmdbuf);
		} else
			writesock(game_socket, "\naborted");
	}
	/* Damn, connection bit it. */
	if (-1 == t)
		game_died();
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
