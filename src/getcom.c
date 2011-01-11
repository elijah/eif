/*************************************************************
 *  $Id: getcom.c,v 1.10 2003/09/25 15:54:27 marcolz Exp $
 *
 *  getcom.c
 *
 *  Get a command, and do a bunch of things to it.
 *
 *  - convert aliases
 *  - replace variable strings
 *  - Check if we are in if-endif sequence
 *  - control where the command is coming from
 *
 *  And a zillion other little details.
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

#ifdef	HAVE_READLINE_READLINE_H
#include	<readline/history.h>
#include	<readline/readline.h>
#endif				/* HAVE_READLINE_READLINE_H */
#ifdef	HAVE_READLINE_TILDE_H
#include	<readline/tilde.h>
#endif				/* HAVE_READLINE_TILDE_H */

#include	"alias.h"
#include	"getcom.h"
#include	"print.h"
#include	"vars.h"

extern int	Interrupt;	/* True if we got a SIGINT */
extern int	Pipe_Interrupt;	/* True if we got a SIGPIPE */

static FILE	*current_input;
static int	current_input_isapipe;

#define FSTACKMAX	40
#define FSTK_USER	0
#define FSTK_EXEC	1
#define FSTK_REDIR	2
#define FSTK_CMDPART	3

typedef struct {
	int		type;
	int		depth;	/* Depth of if's */
	int		disabled;	/* True if commands being ignored due
					 * to if and the depth at which
					 * disabled. */
	union {
		struct {
			FILE		*oldin;
			int		ispipe;
		}		exec;
		struct {
			FILE		*oldout;
			int		ispipe;
		}		redir;
		struct {
			char		*cmd;
		}		cmdpart;
	}		un;
}		filesav_t;

static int		starwalk = 1;
static int		fsk_depth = -1;
static filesav_t	fstack[FSTACKMAX];

/* Local to this file */
static int		change_vars(char *cmd, char *buf);
static void		stuff_semis(char *cmd);
static int		replace_alias(char *cmd, char *alias, char *ret);
static int		find_aliasarg(int arg, char *cmd, char **retp,
				const char *condp, char **maxcp, char *alias);
static int		find_redirs(char *cmd, char *buf);
static filesav_t	*fst_add(int typ);
static int		get_nextbit(char *cmd, int cmdlen, const char *prompt,
				int sub, int dontstrip);

static filesav_t *
fst_add(typ)
	int		typ;
{
	filesav_t	*fs;

	if (fsk_depth >= FSTACKMAX - 1) {
		eprt("Error: Too many cmd stack layers, max %d\n", FSTACKMAX);
		return ((filesav_t *) 0);
	}
	fs = &fstack[++fsk_depth];
	fs->type = typ;
	return (fs);
}

static int
get_nextbit(cmd, cmdlen, prompt, sub, dontstrip)
	char		*cmd;
	const char	*prompt;
	int		cmdlen, dontstrip, sub;
{
	filesav_t	*fst;
	static char	lastcom[1024];
	char		*cp, *bp;
	int		cmdok;
	int		echoed = 0;

#ifndef	READLINE_HANDLES_CONST
	union {
		const char	*cchp;
		char		*chp;
	}		const_temp;

	const_temp.cchp = prompt;
#endif

	*cmd = '\0';
	do {
		cmdok = 1;
		fst = &fstack[fsk_depth];
		if (FSTK_CMDPART == fst->type) {
			cp = fst->un.cmdpart.cmd;
			if (!dontstrip)
				while (isspace(*cp))
					cp++;
			strcpy(cmd, cp);
			free(fst->un.cmdpart.cmd);
			fsk_depth--;
		} else if (FSTK_EXEC == fst->type) {
			if (!fgets(cmd, cmdlen, current_input)) {
				if (current_input_isapipe) {
					(void) pclose(current_input);
				} else
					(void) fclose(current_input);
				current_input = fst->un.exec.oldin;
				current_input_isapipe = fst->un.exec.ispipe;
				fsk_depth--;
				prt("End of exec\n");
				cmdok = 0;
				continue;
			}
			if (!dontstrip && isspace(*cmd)) {
				cp = cmd;
				while (isspace(*cp))
					cp++;
				strcpy(cmd, cp);
			}
			prt("%s", cmd);
			echoed = 1;
		} else if (FSTK_USER == fst->type) {
			if (sub) {
				if (prompt)
					eprt("%s", prompt);
				(void) fflush(stdout);
				(void) fflush(stderr);
				if (!fgets(cmd, cmdlen, stdin)) {
					strcpy(cmd, "ctld");
					clearerr(stdin);
				}
			} else {
				do {
					int		eofd = 0;

					/*
					 * Keep reading until we get a
					 * non-null input line.
					 */
					do {
#ifdef	READLINE_HANDLES_CONST
						if ((cp = readline(prompt))) {
#else				/* ! READLINE_HANDLES_CONST */
						/*
						 * Dirty trick to make it
						 * compile, don't try this at
						 * home. This is ok only
						 * because readline does not
						 * modify prompt...
						 */
						if ((cp = readline(const_temp.chp))) {
#endif				/* ! READLINE_HANDLES_CONST */
							if (!dontstrip)
								while (isspace(*cp))
									cp++;
							if (!*cp) {
								/*
								 * Return
								 * blank
								 * lines.
								 */
								*cmd = '\0';
								return (0);
							}
						} else {
							if (++eofd > 20) {
								eprt("Error: Input eof'd more than 20 times, dying.\n");
								exit(1);
							}
						}
					} while (!cp || !*cp);

					/*
					 * Now, if she input "!" or "^", we
					 * parse it here.
					 */
					if (cp && ('!' == *cp || '^' == *cp)) {
						if (-1 == history_expand(cp, &bp)) {
							eprt("Syntax: Error in history: %s\n", bp);
							cp = (char *)NULL;
						} else {
							cp = bp;
							/*
							 * Echo it to the
							 * user
							 */
							prt("%s\n", cp);
							echoed = 1;
						}
					}
				} while (!cp || !*cp);

				/*
				 * Only non-null, non-duplicate user input
				 * lines onto history
				 */
				if (cp && *cp) {
					if (*cp != lastcom[0] || (strcmp(cp, lastcom))) {
						add_history(cp);
						strcpy(lastcom, cp);
					}
				}
				strcpy(cmd, cp);
			}
		}
	} while (!cmdok);

	/* Remove any newline chars at end. */
	cp = cmd;
	while (*cp && '\n' != *cp)
		cp++;
	*cp = '\0';
	return (echoed);
}

/* unescapes only those characters that needed escaping. */
int
unescape_backslashes(dst, src)
	char		*dst;
	const char	*src;
{
	const char	*sp = src;
	char		*dp = dst;

	while (*sp)
	{
		if (*sp == '\\') {
			switch (sp[1])
			{
				case	'\0':
					eprt("Syntax: Error in unescape: trailing \\\n");
					return 1;
				case	'"':
				case	';':
				case	'$':
				case	'&':
				case	'|':
				case	'\\':
					/* skip */
					sp++;
					break;
				default:
					/* NOTHING */
					;
			}
		}
		*dp++ = *sp++;
	}
	*dp = 0;
	return 0;
}

/************************************
 * get_main_input
 *
 * Return the next line of user input.
 *
 * We must:
 *   Expand aliases.
 *   Parse seperate command lines, seperated by ";".
 *   Expand variables.
 */
int
get_main_input(retbuf, len, prompt)
	char		*retbuf;
	int len		__attribute__((unused));
	const char	*prompt;
{
	filesav_t	*fst;
	int		i, cmdok, echoed = 0;
	char		*cp, *bp, *ap;
	char		buf[1024], cmd[1024];

	/* Initialization */
	if (-1 == fsk_depth) {
		current_input = stdin;
		fsk_depth = 0;
		fstack[0].type = FSTK_USER;
	}
	if (Interrupt || Pipe_Interrupt) {
		int		stop = 0;
		while (fsk_depth > 0 && !stop) {
			fst = &fstack[fsk_depth];
			if (FSTK_REDIR == fst->type) {
				output_file(fst->un.redir.oldout, fst->un.redir.ispipe);
				if (fst->un.redir.ispipe && Pipe_Interrupt && !Interrupt)
					stop = 1;
				fsk_depth--;
			} else if (FSTK_CMDPART == fst->type) {
				free(fst->un.cmdpart.cmd);
				fsk_depth--;
			} else if (FSTK_EXEC == fst->type) {
				(void) fclose(current_input);
				current_input = fst->un.exec.oldin;
				fsk_depth--;
			} else {
				fsk_depth--;
			}
		}
		Interrupt = 0;
		Pipe_Interrupt = 0;
	}
	do {
		cmdok = 1;
		fst = &fstack[fsk_depth];
		/* pop redir */
		if (FSTK_REDIR == fst->type) {
			output_file(fst->un.redir.oldout, fst->un.redir.ispipe);
			fsk_depth--;
			cmdok = 0;
			continue;
		}
		echoed = get_nextbit(cmd, sizeof(cmd), prompt, 0, 0);
		fst = &fstack[fsk_depth];

		/* Is it a comment? */
		if ('#' == cmd[0]) {
			cmdok = 0;
			continue;
		}
		/* Is it a line marked for secondary input.  If so, ignore */
		if ('&' == cmd[0]) {
			stuff_semis(cmd);
			cmdok = 0;
			continue;
		}
		/* Expand any aliases */
		clear_alias_marks();
		do {
			cp = cmd;
			bp = buf;
			i = 0;
			while (cp && *cp && !isspace(*cp) && (++i < (signed) sizeof(buf)))
				*bp++ = *cp++;
			*bp = '\0';

			if ((ap = find_alias(buf))) {
				if (!replace_alias(cmd, ap, cmd))
					cmdok = 0;
				echoed = 0;
			}
		} while (ap && cmdok);
		if (!cmdok)
			continue;

		/* Any ";" commands to stuff into the stack? */
		stuff_semis(cmd);

		/* Any variables to replace? */
		if (!change_vars(cmd, buf)) {
			cmdok = 0;
			continue;
		}
		/*
		 * If we are disabled, we only let 'if', 'else', 'elseif',
		 * and 'endif' through.
		 */
		if (fst->disabled) {
			cp = cmd;
			while (isspace(*cp))
				cp++;
			if (!strncmp("if", cp, 2)) {
				cp += 2;
				if (*cp && !isspace(*cp))
					cmdok = 0;
			} else if (!strncmp("elseif", cp, 6)) {
				cp += 6;
				if (*cp && !isspace(*cp))
					cmdok = 0;
			} else if (!strncmp("else", cp, 4)) {
				cp += 4;
				if (*cp && !isspace(*cp))
					cmdok = 0;
			} else if (!strncmp("endif", cp, 5)) {
				cp += 5;
				if (*cp && !isspace(*cp))
					cmdok = 0;
			} else
				cmdok = 0;
			if (!cmdok)
				continue;
		}
		/* Any file redirections or pipes? */
		if (!(i = find_redirs(cmd, buf))) {
			cmdok = 0;
			continue;
		} else if (i < 0) {
			/*
			 * Redirected, so command was not echoed to redir
			 * file
			 */
			echoed = 0;
		}
	} while (!cmdok);

	unescape_backslashes(retbuf, cmd);
	return (echoed);
}

/************************************
 * get_input
 *
 * No history, vars, aliases.
 */
int
get_input(retbuf, len, prompt)
	char		*retbuf;
	int		len;
	const char	*prompt;
{
	filesav_t	*fst;
	const char	*wp;
	char		*cp;
	FILE		*fp;
	int		psav = 0, dontstrip = 0;

	/* Tel's and ann's prompts look like " 512 left: " */
	if (prompt) {
		wp = prompt;
		while (isspace(*wp))
			wp++;
		if (isdigit(*wp)) {
			while (isdigit(*wp))
				wp++;
			while (isspace(*wp))
				wp++;
			if (!strncmp("left:", wp, 5))
				dontstrip = 1;
		}
	}
	/* Must save redirections so output makes it to a file. */
	fp = (FILE *) 0;
	fst = &fstack[fsk_depth];
	if (fst->type == FSTK_REDIR) {
		fp = fst->un.redir.oldout;
		psav = fst->un.redir.ispipe;
		fsk_depth--;
	}
	/* SPECIAL for empire telegrams and announcements. */

	get_nextbit(retbuf, len, prompt, 1, dontstrip);

	if (fp) {
		fst = fst_add(FSTK_REDIR);
		fst->un.redir.oldout = fp;
		fst->un.redir.ispipe = psav;
	}
	/*
	 * To allow semicolons in ann & tele's, or not? This allows them.
	 */
	if (!dontstrip)
		stuff_semis(retbuf);

	cp = retbuf;

	if (!dontstrip) {
		while (isspace(*cp))
			cp++;
		/* If marked as an secondary input, remove marker */
		if ('&' == *cp) {
			cp++;
			while (isspace(*cp))
				cp++;
		}
	}
	if (cp != retbuf)
		strcpy(retbuf, cp);

	/* Remove trailing space */
	cp = retbuf;
	while (*cp)
		cp++;
	if (cp != retbuf) {
		while (isspace(*--cp) && (cp != retbuf));
		cp++;
		*cp = '\0';

	}
	if (fsk_depth)
		eprt("%s\n", retbuf);

	if (Interrupt) {
		return (0);
	} else
		return (1);
}

/************************************
 * stuff_semis
 *
 * Scan cmd for ";" not inside of quotes, and
 * stick them on the cmd stack.
 */
static void
stuff_semis(cmd)
	char		*cmd;
{
	filesav_t	*fst;
	char		*cp;

	cp = cmd;
	while (*cp) {
		if (('\\' == *cp) &&
			((';' == cp[1]) || ('"' == cp[1]))) {
			/* Skip next character */
			cp++;
		} else if ('\"' == *cp) {
			/* Ignore things inside of quotes. */
			cp ++;
			while (*cp && '\"' != *cp)
				cp++;
		} else if (';' == *cp) {
			*cp++ = '\0';
			if (*cp) {
				if ((fst = fst_add(FSTK_CMDPART))) {
					fst->un.cmdpart.cmd = malloc(strlen(cp) + 1);
					strcpy(fst->un.cmdpart.cmd, cp);
				}
			}
			return;
		}
		cp++;
	}
}

/************************************
 * change_vars
 *
 * Find and substitute in the $VARs in
 * the cmd line.
 *
 * Looking for:
 *   $varname
 *   ${varname}
 *
 * Return 0 if there is a problem.
 * Return 1 if ok.
 */
static int
change_vars(cmd, buf)
	char		*cmd, *buf;
{
	int		err;
	char		*bp, *cp, *dp;
	const char	*ap;

	/* Look for variables to expand */
	/* This should probably be in a procedure.... */
	for (cp = cmd; *cp; cp++) {
		if ('\\' == *cp) {
			cp ++;
		} else if ('"' == *cp) {
			cp++;
			while (*cp)
			{
				if ('\\' == *cp)
				{
					if (!*++cp)
						break;
				} else if ('"' == *cp)
					break;

				cp++;
			}
			if ('"' != *cp) {
				eprt("Syntax: Missing trailing \" in '%s'\n", cmd);
				return (0);
			}
		} else if ('$' == *cp) {
			dp = cp + 1;
			if (*dp && !isspace(*dp)) {
				bp = buf;
				if ('{' /* } */ == *dp) {
					dp++;
					while (*dp && /* { */ '}' != *dp)
						*bp++ = *dp++;
					*bp = '\0';
					if (!*dp) {
						eprt("Syntax: Bad variable syntax, '%s'\n", cp);
						return (0);
					}
					dp++;
				} else {
					while (*dp && !isspace(*dp) && ':' != *dp)
						*bp++ = *dp++;
					*bp = '\0';
				}
				if (!buf[0]) {
					*cp = '\0';
					strcpy(buf, cmd);
					strcat(buf, "$");
					cp = cmd + strlen(buf) - 1;
					strcat(buf, dp);
					strcpy(cmd, buf);
				} else if ((ap = find_var(buf, &err))) {
					*cp = '\0';
					strcpy(buf, cmd);
					strcat(buf, ap);
					cp = cmd + strlen(buf) - 1;
					strcat(buf, dp);
					strcpy(cmd, buf);
				} else {
					if (err)
						eprt("Error: Unable to find variable '%s'\n", buf);
					return (0);
				}
			}
		}
	}
	return (1);
}

/************************************
 * find_redirs
 *
 * Look for "|", ">", etc, in the command line.
 *
 * Let us be sleazy.
 *  If we find a "|", pipe it into a "sh -c" for the
 *  rest of it.
 *
 * Return 0 if there was a problem.
 * Return 1 if ok.
 * Return -1 if redirected.
 */
static int
find_redirs(cmd, buf)
	char		*cmd, *buf __attribute__((unused));
{
	filesav_t	*fst;
	char		*cp, *bp, *np;
	FILE		*fp;
	int		xs = 0;

	cp = cmd;
	while (*cp && ('|' != *cp) && ('>' != *cp)) {
		/* Ignore quoted strings. */
		if ('\"' == *cp) {
			cp++;
			while (*cp && '\"' != *cp)
				cp++;
			if (!*cp)
				break;
		}
		/* Empire has commands which contain '>'. */
		/* However, they all follow '?' */
		/* Thus, we skip over strings following '?'s */
		if ('?' == *cp) {
			while (*cp && !isspace(*cp))
				cp++;
			if (!*cp)
				break;
		}
		cp++;
	}
	if ('|' == *cp) {
		*cp++ = '\0';
		while (isspace(*cp))
			cp++;
		if (!*cp) {
			eprt("Syntax: Bad pipe.  Need a destination program.\n");
			return (0);
		}
		if (!(fst = fst_add(FSTK_REDIR)))
			return (0);

		/* Expand any ~'s in destination name */
		np = tilde_expand(cp);

		/* Set up the output pipe. */
		fp = output_topipe(np, &fst->un.redir.ispipe);

		/* tilde_expand returns an alloc'd string */
		free(np);

		fst->un.redir.oldout = fp;
		if (!fp) {
			output_file(fp, fst->un.redir.ispipe);
			fsk_depth--;
			eprt("Error: Unable to popen pipe. %d:%s\n", errno, cp);
			return (0);
		}
		return (-1);
	} else if ('>' == *cp) {
		*cp++ = '\0';
		if ('>' == *cp) {
			xs = 1;
			cp++;
		} else if ('!' == *cp) {
			xs = -1;
			cp++;
		}
		while (isspace(*cp))
			cp++;
		if (!*cp) {
			eprt("Syntax: Bad file redirection.  Need a destination file.\n");
			return (0);
		}
		bp = cp;
		while (*bp && !isspace(*bp) && ('>' != *bp) && ('!' != *bp))
			bp++;
		if (('>' == *bp) || ('!' == *bp)) {
			eprt("Syntax: Bad file redirection. Only one redirection allowed.\n");
			return (0);
		}
		if (*bp)
			*bp++ = '\0';
		while (isspace(*bp))
			bp++;
		if (*bp) {
			eprt("Syntax: Bad file redirection.  Redirection must be last.\n");
			return (0);
		}
		if (!(fst = fst_add(FSTK_REDIR)))
			return (0);

		/* Expand any ~'s in destination name */
		np = tilde_expand(cp);

		/* Set up the output file. */
		fp = output_to(np, xs, &fst->un.redir.ispipe);

		/* tilde_expand returns an alloc'd string */
		free(np);

		fst->un.redir.oldout = fp;
		if (!fp) {
			fsk_depth--;
			eprt("Error: Unable to redirect to file '%s', %d.\n", cp, errno);
			return (0);
		} else if ((FILE *) - 1 == fp) {
			fsk_depth--;
			eprt("Error: File %s exists, redirection failed.\n", cp);
			return (0);
		}
		return (-1);
	}
	return (1);
}

/******************************************
 * cmd_exec
 *
 * Read commands from a file.
 *
 * sub = 0, read the file in.
 * sub = 1, execute file, and read in output from it.
 * sub = 2, read the file in, but don't echo the output.
 */
void
cmd_exec(buf, sub)
	const char	*buf;
	int		sub;
{
	int		ispipe = 0;
	filesav_t	*fst;
	char		*cp, *nam = NULL;
	FILE		*fp;

#ifndef	READLINE_HANDLES_CONST
	union {
		const char	*cchp;
		char		*chp;
	}		const_temp;
#endif

	/* Initialization */
	if (-1 == fsk_depth) {
		current_input = stdin;
		fsk_depth = 0;
		fstack[0].type = FSTK_USER;
	}
	while (isspace(*buf))
		buf++;

	if (!*buf) {
		eprt("Syntax: Need a filename to execute.\n");
		return;
	}
	/* Expand any ~'s in filename */
	/* This is in the GNU readline stuff. */
#ifdef	READLINE_HANDLES_CONST
	nam = tilde_expand(buf);
#else				/* READLINE_HANDLES_CONST */
	const_temp.cchp = buf;

	nam = tilde_expand(const_temp.chp);
#endif				/* READLINE_HANDLES_CONST */

	if ((0 == sub) || (2 == sub)) {
		cp = nam;
		while (*cp && !isspace(*cp))
			cp++;
		if (*cp)
			*cp++ = '\0';
		while (isspace(*cp))
			cp++;
		if (*cp) {
			eprt("Syntax: Extraneous characters after filename, `%s'\n", cp);
			goto error_ret;
		}
		if (!(fp = fopen(nam, "r"))) {
			eprt("Error: Unable to exec file %s, %d\n", nam, errno);
			goto error_ret;
		}
		/* Disable output by stacking another filetype. */
		if (2 == sub) {
			if (!(fst = fst_add(FSTK_REDIR)))
				goto error_ret;
			fst->un.redir.oldout = output_turnoff(&fst->un.redir.ispipe);
		}
	} else if (1 == sub) {
		if (!(fp = popen(nam, "r"))) {
			eprt("Unable to execute command '%s', %d\n", nam, errno);
			goto error_ret;
		}
		ispipe = 1;
	} else {
		eprt("Internal Error, bad sub to cmd_exec\n");
		goto error_ret;
	}

	if (!(fst = fst_add(FSTK_EXEC))) {
		(void) fclose(fp);
		goto error_ret;
	}
	fst->un.exec.oldin = current_input;
	fst->un.exec.ispipe = current_input_isapipe;
	current_input = fp;
	current_input_isapipe = ispipe;

error_ret:
	if (nam)
		free(nam);
}

/******************************************
 * replace_alias
 *
 * Take a command and an alias, and make up the
 * new command.
 */
static int
replace_alias(cmd, alias, ret)
	char		*cmd, *alias, *ret;
{
	int		arg;
	char		*ap, *bp, *cp, *maxcp;
	const char	*sp;
	char		buf[1024];

	/* Find the end of the alias parameter. */
	maxcp = cmd;
	while (isspace(*maxcp))
		maxcp++;
	while (*maxcp && !isspace(*maxcp))
		maxcp++;

	starwalk = 1;

	for (ap = alias, bp = buf; *ap; ap++) {
		if ('\\' == *ap)
		{
			*bp++ = *ap++;
			if (!*ap)
				break;

			*bp++ = *ap;
		} else if ('"' == *ap) {
			/* Ignore quoted strings. */
			*bp++ = *ap++;
			while (*ap)
			{
				if ('\\' == *ap)
				{
					*bp++ = *ap++;
					if (!*ap)
						break;
				} else if ('"' == *ap)
					break;
				*bp++ = *ap++;
			}
			while (*ap && '"' != *ap)
				*bp++ = *ap++;
			if ('"' != *ap) {
				eprt("Syntax: Alias missing trailing \" in '%s'\n", alias);
				return (0);
			}
			*bp++ = *ap;
		} else if ('$' == *ap) {
			cp = ap;
			cp++;
			if ('{' /* '} */ == *cp) {
				if (!isdigit(*++cp)) {
					*bp++ = *ap;
					continue;
				}
				/* It is a ${N type variable. } */
				arg = atoi(cp);
				while (isdigit(*cp))
					cp++;
				sp = NULL;
				if (':' == *cp) {
					sp = ++cp;
					while (*cp && /* { */ '}' != *cp)
						cp++;
				}
				if (/* { */ '}' != *cp) {
					eprt( /* { */ "Syntax: Alias missing trailing '}' in '%s'\n",
						alias);
					return (0);
				}
				ap = cp;
				if (find_aliasarg(arg, cmd, &bp, sp, &maxcp, alias))
					return (0);
			} else if (isdigit(*cp)) {
				/* It is a numeric variable name */
				arg = atoi(cp);
				if (find_aliasarg(arg, cmd, &bp, (char *)0, &maxcp, alias))
					return (0);
				while (isdigit(*cp))
					ap = cp++;
			} else if ('*' == *cp) {
				/* All args except 0 */
				arg = starwalk;
				sp = "*";
				if (!find_aliasarg(arg, cmd, &bp, sp, &maxcp, alias)) {
					starwalk++;
					arg++;
					ap -= 2;
				} else {
					if (starwalk > 1)
						*(--bp) = 0;
					starwalk = 1;
					ap++;
				}
			} else {
				*bp++ = '$';
				/* Special case for $$ */
				if ('$' == *cp)
					*bp++ = *ap++;
			}
		} else {
			*bp++ = *ap;
		}
	}
	*bp = '\0';
	strcpy(bp, maxcp);
	strcpy(ret, buf);
	return (1);
}

/******************************************
 * find_aliasarg
 *
 * Find the alias argument, and add it to the
 * new command being built.
 *
 * Arguments are quoted strings or space delimited words,
 * which can contain any characters.
 */
static int
find_aliasarg(arg, cmd, retp, condp, maxcp, alias)
	int		arg;
	char		*cmd;
	char		**retp;
	const char	*condp;
	char		**maxcp;
	char		*alias;
{
	char		*cp;
	int		i;

	cp = cmd;
	i = arg;
	/* While characters, and not running into the next command. */
	while (*cp && (';' != *cp) && (i >= 0)) {
		while (isspace(*cp))
			cp++;
		/* Quoted string */
		if ('"' == *cp) {
			cp++;
			while (*cp && '"' != *cp) {
				if (i) {
					cp++;
				} else
					*(*retp)++ = *cp++;
			}
			if ('"' != *cp++) {
				eprt("Syntax: Arg %d missing trailing \" in '%s'\n", i, cmd);
				return (-1);
			}
			i--;
		} else if (*cp && ';' != *cp) {
			while (*cp && ';' != *cp && !isspace(*cp)) {
				if (i) {
					cp++;
				} else
					*(*retp)++ = *cp++;
			}
			i--;
		}
	}
	if (cp > *maxcp)
		*maxcp = cp;

	/* We didn't find the argument. */
	if (i >= 0) {
		if (condp) {
			if ('-' == *condp) {
				/* Use the following string */
				condp++;
				while (*condp && /* { */ '}' != *condp)
					*(*retp)++ = *condp++;
				if ( /* { */ '}' != *condp) {
					eprt(	/* { */
						"Syntax: Conditional variable missing '}' in '%s'\n",
						alias);
					return (-1);
				}
			} else if ('?' == *condp) {
				/* Print an error, and abort command. */
				char		*xp;
				char		buf[1024];

				xp = buf;
				condp++;
				while (*condp && /* { */ '}' != *condp)
					*xp++ = *condp++;
				if ( /* { */ '}' != *condp)
					eprt(	/* { */
						"Syntax: Conditional variable missing '}' in '%s'\n",
						alias);
				*xp = '\0';
				eprt("%s\n", buf);
				return (-1);
			} else if ('+' == *condp) {
				/* Substitute nothing. */
				while (*condp && /* { */ '}' != *condp)
					condp++;
				if ( /* { */ '}' != *condp) {
					eprt(	/* { */
						"Syntax: Conditional variable missing '}' in '%s'\n",
						alias);
					return (-1);
				}
			} else if (*condp == '*') {
				return (-1);
			} else {
				eprt("Syntax: Bad conditional variable syntax in alias '%s`\n",
					alias);
				return (-1);
			}
		} else {
			eprt("Missing alias parameter %d\n", arg);
			return (-1);
		}
	}
	return (0);
}

/******************************************
 * cmd_if
 *
 * sub:
 *   0 - if
 *   1 - elseif
 *   2 - else
 *   3 - endif
 */
void
cmd_if(buf, sub)
	const char	*buf;
	int		sub;
{
	filesav_t	*fst;
	int		ret;
	char		cmd[200];

	fst = &fstack[fsk_depth];
	if ((sub == 0) || (sub == 1)) {
		/* IF or ELSEIF */
		if (sub == 1) {
			/* ELSEIF */
			if (fst->depth <= 0) {
				eprt("elseif not inside an if list.\n");
				return;
			}
			if (fst->disabled) {
				/*
				 * Lower level, or previous elseif disabled
				 * us
				 */
				if (fst->depth > fst->disabled)
					return;
				/* This level disabled us, re-enable */
				fst->disabled = 0;
			} else {
				/* Disabled until endif */
				fst->disabled = fst->depth - 1;
				prt("Commands disabled til matching endif.\n");
			}
		} else
			fst->depth += 2;

		if (!fst->disabled) {
			while (isspace(*buf))
				buf++;

			sprintf(cmd, "/bin/test %s", buf);
			ret = system(cmd);
			if ((ret % 256) != 0) {
				eprt("Problem in system call of /bin/test, %d", (ret % 256));
				eprt("Command was: %s", cmd);
				ret = -1;
			} else {
				ret = (ret / 256) % 256;
				if (ret == 255) {
					eprt("Bad test conditons.\n");
					ret = -1;
				}
			}
			if (ret) {
				if (ret == -1) {
					if ((fsk_depth == 0) && (fst->depth == 2)) {
						/*
						 * Interactive command,
						 * ignore error.
						 */
						fst->depth = 0;
						fst->disabled = 0;
					} else {
						/*
						 * Abort until this if is
						 * done.
						 */
						fst->disabled = fst->depth - 1;
						prt("Commands disabled til matching endif.\n");
					}
				} else {
					fst->disabled = fst->depth;
					prt("Commands disabled.\n");
				}
			} else if (sub == 1)
				prt("Commands enabled.\n");
		}
	} else if (sub == 2) {
		if (fst->depth <= 0) {
			eprt("else not inside an if list.\n");
			return;
		}
		/* ELSE */
		if (fst->disabled) {
			if (fst->depth <= fst->disabled) {
				fst->disabled = 0;
				prt("Commands enabled.\n");
			}
		} else if (fst->depth > 0) {
			prt("Commands disabled til matching endif.\n");
			fst->disabled = fst->depth - 1;
		}
	} else if (sub == 3) {
		/* ENDIF */
		if (fst->depth <= 0) {
			fst->depth = 0;
			if (!fst->disabled)	/* So I'm paranoid */
				eprt("endif not inside an if list.\n");
		} else
			fst->depth -= 2;
		if (fst->disabled && (fst->depth <= fst->disabled)) {
			fst->disabled = 0;
			prt("Commands enabled.\n");
		}
		if (fst->depth <= 0) {
			fst->depth = 0;
			fst->disabled = 0;
		}
	}
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
