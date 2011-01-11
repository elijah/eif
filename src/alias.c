/*************************************************************
 *  $Id: alias.c,v 1.5 2003/05/24 15:26:32 marcolz Exp $
 *
 *  alias.c
 *
 *  Maintain the user aliases.
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

#ifdef	HAVE_STDLIB_H
#include	<stdlib.h>
#else				/* not HAVE_STDLIB_H */
#ifdef	HAVE_MALLOC_H
#include	<malloc.h>
#else				/* not HAVE_MALLOC_H */
char		*calloc();
#endif				/* not HAVE_MALLOC_H */
#endif				/* not HAVE_STDLIB_H */

#include	"alias.h"
#include	"print.h"

static alias_t *alias_list;
static int	numaliases = 0;
static int	maxaliases = 0;

static alias_t *find_alias_loc(const char *buf);

/***********************************
 * clear_alias_marks
 *
 * Clear the alias marks so they can be reused.
 */
void
clear_alias_marks()
{
	alias_t		*ap;
	int		i;

	for (ap = alias_list, i = 0; (ap && i < numaliases); ap++, i++)
		ap->mark = 0;
}

/***********************************
 * find_alias_loc
 *
 * Find the matching alias, if it exists.
 */
static alias_t *
find_alias_loc(buf)
	const char	*buf;
{
	alias_t		*ap;
	int		i;

	for (ap = alias_list, i = 0; ap && i < numaliases; ap++, i++) {
		if (ap->alias && !strcmp(ap->alias, buf))
			return (ap);
	}
	return ((alias_t *) 0);
}

/***********************************
 * find_alias
 *
 * Find the alias, if it exists, and return a
 * pointer to it's command, if it hasn't been
 * marked as being used before.
 */
char	*
find_alias(buf)
	const char	*buf;
{
	alias_t		*ap;

	if ((ap = find_alias_loc(buf)) && !ap->mark) {
		ap->mark = 1;
		return (ap->command);
	}
	return ((char *)0);
}


/***********************************
 * cmd_alias
 *
 * The alias command.
 */
void
cmd_alias(alias, sub)
	const char	*alias;
	int sub		__attribute__((unused));
{
	char		com_buf[1024];
	char		*command;
	char		*cp;
	alias_t		*ap;
	int		i;

	/* Skip over trailing space. */
	while (isspace(*alias))
		alias++;

	/* If no alias is specified, print all the aliases. */
	if (!*alias) {
		if (!numaliases)
			prt("No aliases defined.\n");
		else
			for (ap = alias_list, i = 0; ap && i < numaliases; ap++, i++)
				prt("alias %s \"%s\"\n", ap->alias, ap->command);
		return;
	}
	strncpy(com_buf, alias, sizeof(com_buf));
	command = com_buf;
	while (*command && !isspace(*command))
		command++;
	if (*command) {
		*command++ = '\0';
		while (isspace(*command))
			command++;
		/* String "'s from the command */
		if ('\"' == *command) {
			cp = ++command;
			while (*cp)
			{
				if ('\\' == *cp)	
				{
					if (!*++cp)
						break;
				} else if ('\"' == *cp)
					break;

				cp++;
			}
			if ('"' == *cp)
				*cp = '\0';
		}
	}
	/* We prevent certain keywords from being aliased. */
	/* Like, alias, unalias */
	if (!strcmp(com_buf, "alias") || !strcmp(com_buf, "unalias")) {
		eprt("Too dangerous to allow that\n");
		return;
	}
	/* If we find a matching alias, either print it or replace it. */
	if ((ap = find_alias_loc(com_buf))) {
		if (*command) {
			free(ap->command);
		} else {
			prt("alias %s \"%s\"\n", ap->alias, ap->command);
			return;
		}
	} else {
		if (!*command) {
			prt("%s undefined.\n", com_buf);
			return;
		}
		numaliases++;
		if (numaliases >= maxaliases) {
			alias_t         *tp;

			maxaliases += 10;
			if (!(tp = (alias_t *) calloc((unsigned)maxaliases,
							sizeof(alias_t))))
				exit(1);
			if (alias_list)
				bcopy((char *)alias_list, (char *)tp,
					((numaliases - 1) * sizeof(alias_t)));
			free((char *)alias_list);
			alias_list = tp;
		}
		ap = &alias_list[numaliases - 1];
		ap->alias = strdup(com_buf);
	}
	ap->command = strdup(command);
	return;
}

/***********************************
 * cmd_unalias
 *
 * The unalias command.
 */
void
cmd_unalias(buf, sub)
	const char	*buf;
	int sub		__attribute__((unused));
{
	char		*cp, a_buf[1024];
	char		*alias;
	alias_t		*ap, *bp;
	int		i;

	strncpy(a_buf, buf, sizeof(a_buf));
	alias = a_buf;

	/* Skip over trailing space. */
	while (isspace(*alias))
		alias++;

	if (!*alias) {
		eprt("Expecting alias to remove.\n");
		return;
	}
	cp = alias;
	while (!isspace(*cp))
		cp++;
	*cp = '\0';

	if ('*' == *alias && !*(alias + 1)) {
		for (ap = alias_list, i = 0; (ap && i < numaliases); ap++, i++) {
			free(ap->alias);
			free(ap->command);
			ap->alias = (char *)0;
		}
		numaliases = 0;
		prt("All aliases removed.\n");
		return;
	}
	if (!(ap = find_alias_loc(alias))) {
		eprt("Alias '%s' not found.\n", alias);
		return;
	}
	free(ap->alias);
	free(ap->command);
	bp = ap++;
	while (ap->alias) {
		bp->alias = ap->alias;
		bp->command = ap->command;
		bp = ap++;
	}
	bp->alias = (char *)0;

	prt("Alias '%s' deleted.\n", alias);
	numaliases--;
	return;
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
