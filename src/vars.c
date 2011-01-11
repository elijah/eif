/*************************************************************
 *  $Id: vars.c,v 1.5 2003/09/25 15:54:27 marcolz Exp $
 *
 *  vars.c
 *
 *  Maintain the user variables.
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

#include	"getcom.h"
#include	"print.h"
#include	"vars.h"

typedef struct {
	char		*name, *value;
}		var_t;

/* Special vars. */
typedef struct {
	var_t		vt;
	int		type;
	int		setable;
}		intvar_t;

#define IV_PID		1
#define IV_INPUT	2

static var_t		*var_list;
static unsigned int	maxvars = 0;

/* For -Wwrite-strings compiles. */
char		EmptyString[] = "";
char		Dollar[] = "$";
char		InputFrom[] = "<";

intvar_t	intvar_list[] = {
	{{Dollar, EmptyString}, IV_PID, 0},
	{{InputFrom, EmptyString}, IV_INPUT, 0},
	{{EmptyString, EmptyString}, 0, 0}
};

static intvar_t	*find_var_int(const char *buf);
static var_t	*find_var_loc(const char *buf);

/***********************************
 * find_var_loc
 *
 * Find the matching var, if it exists.
 */
static var_t	*
find_var_loc(buf)
	const char	*buf;
{
	unsigned int	i;
	var_t		*vp;

	for (vp = var_list, i = 0; i < maxvars; vp++, i++) {
		if (vp->name && !strcmp(vp->name, buf))
			return (vp);
	}

	return ((var_t *) 0);
}

/***********************************
 * find_var_int
 *
 * Find the var, if it exists, and return a
 * pointer to it's value.
 */
static intvar_t *
find_var_int(buf)
	const char	*buf;
{
	intvar_t	*ivp;

	for (ivp = intvar_list; ivp->type; ivp++) {
		if (ivp->vt.name && !strcmp(ivp->vt.name, buf))
			return (ivp);
	}
	return ((intvar_t *) 0);
}

/***********************************
 * find_var
 *
 * Find the var, if it exists, and return a
 * pointer to it's value.
 *
 * ${varname}	- Try for variable.
 * ${%varname}	- Try for an external environment variable.
 * ${var:-word}	- If "var" is set and non-null, substitute its value,
 *		  otherwise use word.
 * ${var:=word}	- If "var" is not set, or null, set variable to word
 *		  and replace with word.
 * ${var:?word}	- If "var" is set and non-null, use it's value. Otherwise,
 *		  print "word" and abort the command.
 * ${var:+word}	- If "var" is set and non-null, substitute word. Otherwise,
 *		  substitute nothing.
 * All this stolen from "sh".
 * ${var:%word}	- Use "word" as a prompt if "var" == "<".  Ignored elsewhere.
 *
 *
 *
 * Note that by the time the variable string gets here, the
 * "$" and "{}"s have been stripped off.
 */
const char	*
find_var(buf, err)
	char		*buf;
	int		*err;
{
	intvar_t	*ivp;
	var_t		*vp;
	char		*bp;
	const char	*valp = NULL;
	int		mode = 0;
	static char	rbuf[100];

	*err = 1;
	bp = buf;
	while (*bp && ':' != *bp)
		bp++;
	if (':' == *bp) {
		*bp++ = '\0';
		switch (*bp++) {
		case '-':
			mode = 1;
			break;
		case '=':
			mode = 2;
			break;
		case '?':
			mode = 3;
			break;
		case '+':
			mode = 4;
			break;
		case '%':
			mode = 5;
			break;
		default:
			return ((char *)0);
		}
	}
	if ('%' == *buf) {
		if (*++buf)
			valp = getenv(buf);
	} else if ((ivp = find_var_int(buf))) {
		switch (ivp->type) {
		case IV_PID:
			sprintf(rbuf, "%d", getpid());
			valp = rbuf;
			break;
		case IV_INPUT:
			if (mode == 5) {
				get_input(rbuf, sizeof(rbuf), bp);
			} else
				get_input(rbuf, sizeof(rbuf), (char *)0);
			valp = rbuf;
			break;
		default:
			/* This ain't the user's fault... */
			strcpy(rbuf, "Bad internal variable type.");
			return (rbuf);
		}
	} else if ((vp = find_var_loc(buf))) {
		if (vp->value) {
			valp = vp->value;
		} else
			valp = "";
	}
	switch (mode) {
	case 0:		/* ${var} */
	case 5:		/* ${var:%word} */
		return (valp);
	case 1:		/* ${var:-word} */
		if (valp && *valp) {
			return (valp);
		} else {
			strcpy(rbuf, bp);
			return (rbuf);
		}
	case 2:		/* ${var:=word} */
		if (valp && *valp) {
			return (valp);
		} else {
			strcpy(rbuf, bp);
			setvar_value(buf, bp);
			/* Need to set variable value. */
			return (rbuf);
		}
	case 3:		/* ${var:?word} */
		if (valp && *valp) {
			return (valp);
		} else {
			if (*bp)
				eprt("%s\n", bp);
			*err = 0;
			return ((char *)0);
		}
	case 4:		/* ${var:+word} */
		if (valp && *valp) {
			return (bp);
		} else
			return ("");
	default:
		;
	}

	return (valp);
}

/***********************************
 * cmd_setvar
 *
 *  varname [= value]
 *  varname [= "value"]
 */
void
cmd_setvar(var, sub)
	const char	*var;
	int sub		__attribute__((unused));
{
	char		*what, *cp, vbuf[1024];

	while (isspace(*var))
		var++;

	if (!*var) {
		eprt("Need variable name.\n");
		return;
	}
	strncpy(vbuf, var, sizeof(vbuf));

	what = vbuf;
	while (*what && !isspace(*what))
		what++;
	if (*what)
		*what++ = '\0';
	while (isspace(*what))
		what++;
	if ('=' == *what) {
		what++;
		while (isspace(*what))
			what++;
	}
	if ('\"' == *what) {
		cp = ++what;
		while (*cp && '\"' != *cp)
			cp++;
		*cp = '\0';
	} else {
		cp = what;
		while (*cp && !isspace(*cp))
			cp++;
		*cp = '\0';
	}

	setvar_value(vbuf, what);
}

/***********************************
 * setvar_value
 *
 * Make the variable "var" be "what".
 */
void
setvar_value(var, what)
	const char	*var, *what;
{
	unsigned int	t;
	char		*cp;
	intvar_t	*ivp;
	var_t		*vt;

	if ('%' == *var) {
		var++;
		if (!*var) {
			eprt("External variable name incomplete.\n");
			return;
		}
#ifdef	HAVE_SETENV
		(void) setenv(var, what, 1);
#else	/* HAVE_SETENV */
		eprt("This OS does not support setting external variables.\n");
#endif	/* HAVE_SETENV */
	} else if ((ivp = find_var_int(var))) {
		if (!ivp->setable) {
			eprt("Sorry, special variable %s is not setable\n",
				ivp->vt.name);
			return;
		}
		eprt("Sorry, unimplemented.\n");
	} else if ((vt = find_var_loc(var))) {
		t = strlen(what);
		if (vt->value && (strlen(vt->value) >= t)) {
			strcpy(vt->value, what);
		} else {
			if (vt->value)
				free(vt->value);
			cp = malloc(t + 1);
			strcpy(cp, what);
			vt->value = cp;
		}
	} else {
		/* Try to find a free entry */
		for (vt = var_list, t = 0; ((t < maxvars) && vt->name); vt++, t++);
		if (t >= maxvars) {
			vt = (var_t *) calloc((unsigned)maxvars + 10, sizeof(*vt));
			if (var_list) {
				bcopy((char *)var_list, (char *)vt, (maxvars * sizeof(*vt)));
				free((char *)var_list);
			}
			var_list = vt;
			vt = &vt[maxvars];
			maxvars += 10;
		}
		cp = malloc((unsigned)(strlen(var) + 1));
		strcpy(cp, var);
		vt->name = cp;
		cp = malloc((unsigned)(strlen(what) + 1));
		strcpy(cp, what);
		vt->value = cp;
	}
}

/***********************************
 * cmd_unsetvar
 */
void
cmd_unsetvar(var, sub)
	const char	*var;
	int sub		__attribute__((unused));
{
	var_t		*vt;

	while (isspace(*var))
		var++;
	if (!*var) {
		eprt("Need variable name.\n");
		return;
	}
	if ('%' == *var) {
		var++;
		if (!*var) {
			eprt("External variable name incomplete.\n");
			return;
		}
#ifdef	HAVE_UNSETENV
		(void) unsetenv(var);
#else	/* HAVE_UNSETENV */
		eprt("This OS does not support unsetting external variables.\n");
#endif	/* HAVE_UNSETENV */
	} else if (find_var_int(var)) {
		eprt("Sorry, you cannot unset one of the special variables.\n");
	} else if ((vt = find_var_loc(var))) {
		free(vt->name);
		free(vt->value);
		vt->name = NULL;
		vt->value = NULL;
	} else
		eprt("%s not found.\n", var);
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
