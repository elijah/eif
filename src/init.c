/*************************************************************
 *  $Id: init.c,v 1.4 2003/09/25 15:54:27 marcolz Exp $
 *
 *  init.c
 *
 *  Initialize any global data that need init'ing.
 */
/*******************************************************
 *  Copyright (C) Doug Hay, 1989,90,91.
 *  Permission to use and abuse this code, as long
 *  as this copyright notice stays intact and with the
 *  code.  No warranty implied.  This code supplied as is.
 *******************************************************/

#include	"config.h"

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
#include	<sys/types.h>
#ifdef	HAVE_UNISTD_H
#include	<unistd.h>
#endif				/* HAVE_UNISTD_H */

#include	"cntl.h"
#include	"data.h"
#include	"init.h"
#include	"print.h"

static int	showinit;
static void	set_user_info(void);
static int	string_env(char *whr, const char *env, const char *def,
			size_t max);

/****************************************
 * string_env
 *
 * Look for the specified environment var.
 * If it doesn't exist, use the default.
 * Return 0 if default used, 1 if env used.
 */
static int
string_env(whr, env, def, max)
	char		*whr;
	const char	*def, *env;
	size_t		max;
{
	unsigned int	l;
	char		*ep;

	if ((ep = getenv(env))) {
		l = strlen(ep);
		if (l >= max)
			l = max - 1;
		strncpy(whr, ep, l);
	} else {
		l = strlen(def);
		if (l >= max)
			l = max - 1;
		strcpy(whr, def);
	}
	if (showinit)
		prt(" def: %s   env: %s   set: %s\n",
		    def, env, whr);
	return ((ep != NULL));
}

void
set_defaults()
{
	set_user_info();
	cntl.pid = getpid();

	if (getenv("EIFSHOWINIT"))
		showinit = 1;


	if (!string_env(cntl.st.startupf, "EIFSTARTF", "",
			sizeof(cntl.st.startupf))) {
		if (cntl.usr.homedir &&
			(strlen(cntl.usr.homedir) + 8) <=
			sizeof(cntl.st.startupf)) {
			strcpy(cntl.st.startupf, cntl.usr.homedir);
			strcat(cntl.st.startupf, "/.eifrc");
			cntl.st.readstartup = 1;
		} else
			cntl.st.readstartup = 0;
	} else
		cntl.st.readstartup = 1;
}

#include <pwd.h>
static void
set_user_info()
{
	struct passwd	*pwd;

	cntl.usr.uid = getuid();
	pwd = getpwuid(cntl.usr.uid);
	if (pwd) {
		cntl.usr.name = malloc((unsigned)strlen(pwd->pw_name) + 1);
		strcpy(cntl.usr.name, pwd->pw_name);
		cntl.usr.homedir = malloc((unsigned)strlen(pwd->pw_dir) + 1);
		strcpy(cntl.usr.homedir, pwd->pw_dir);
		cntl.usr.shell = malloc((unsigned)strlen(pwd->pw_shell) + 1);
		strcpy(cntl.usr.shell, pwd->pw_shell);
	}
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
