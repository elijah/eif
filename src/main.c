/*************************************************************
 *  $Id: main.c,v 1.5 2003/09/25 15:54:27 marcolz Exp $
 *
 *  main.c
 *
 *  The main for the empire interface program EIF.
 *
 *  Handles runtime command arguments.
 */
/*******************************************************
 *  Copyright (C) Doug Hay, 1989,90,91.
 *  Permission to use and abuse this code, as long
 *  as this copyright notice stays intact and with the
 *  code.  No warranty implied.  This code supplied as is.
 *******************************************************/

#define MAIN

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

#include	"cntl.h"
#include	"data.h"
#include	"eif.h"
#include	"init.h"
#include	"main.h"
#include	"print.h"
#include	"vars.h"

#ifndef ENVARG
#define ENVARG "EIFARGS"
#endif

FILE		*curout;

static void	usagequit(int typ);
static void	parse_env(const char *env);
static void	print_help(void);
static void	version_print(void);
static int	parse_args(int argc, char *argv[]);
static int	parse_string(char *buf);

int
main(argc, argv)
	int		argc;
	char		*argv[];
{
	int		t;

	curout = stdout;

	/* Set defaults before argument parsing */
	set_defaults();

	/* Parse any arguments in the environment variable */
	parse_env(ENVARG);

	/* Parse the runtime arguments */
	if ((t = parse_args(argc, argv)))
		usagequit(t);

	eif_away();
}

/*************************************
 * parse_env
 *
 * Check the given environment variable,
 * and if it is set, parse it as if it
 * were a command line option string.
 */

static void
parse_env(env)
	const char	*env;
{
	int		l;
	char		*val;
	char		buf[256];

	if ((val = getenv(env)) == NULL)
		return;

	/* Get length of variable, make sure not too long. */
	l = (signed) strlen(val);
	if (l >= (signed) sizeof(buf))
		l = (signed) sizeof(buf) - 1;

	/* Copy it into our buffer */
	strncpy(buf, val, l);
	buf[l] = '\0';

	if ((l = parse_string(buf))) {
		if (l != -2)
			fprintf(stderr, "Error in environment variable %s\n", env);
		usagequit(l);
	}
}

/*******************************************
 * parse_string
 *
 * Passed a string, make it look like an argv,
 * and then call parse_args on it.
 */
static int
parse_string(buf)
	char		*buf;
{
	int		n = 0, inone = 0;
	char		*cp;
	char		*pnts[100];

	pnts[n++] = buf;

	/* Find the start of all strings in it */
	cp = buf;
	while (*cp) {
		if (*cp == ' ') {
			*cp = '\0';
			inone = 0;
		} else {
			if (!inone) {
				pnts[n++] = cp;
				inone = 1;
			}
		}
		cp++;
	}
	return (parse_args(n, pnts));
}


/**********************************
 * parse_args
 *
 * Parse the command line type args.
 *
 * Returns 0 if error.
 * Otherwise, pass return code to usagequit for
 * interpretation and error printout.
 *
 *  -1 is a general parsing error, can't figure it out.
 *  -2 is a print_help return, just to exit program.
 *  anything else must be workout in usagequit.
 */

static int
parse_args(argc, argv)
	int		argc;
	char		*argv[];
{
	int		argi, optcont;
	char		*arg, *vname;
	const char	*vval;

	argi = 0;
	while (++argi < argc) {	/* While there are args to process */
		arg = argv[argi];
		if (*arg == '-') {	/* minus options */
			optcont = 1;
			while (*++arg && optcont) {	/* while option chars */
				switch (*arg) {	/* case on minus option chars */
				case 'h':	/* help */
					print_help();
					return (-2);
				case 'v':
					version_print();
					return (-2);
				case 'b':
					disable_bell_output = 1;
					break;
				case 'r':
					cntl.st.readstartup = 0;
					break;
				case 'D':
					optcont = 0;
					if (++argi < argc) {
						vname = argv[argi];
						if (++argi < argc) {
							vval = argv[argi];
						} else
							vval = " ";
						setvar_value(vname, vval);
					}
					break;
				default:
					fprintf(stderr, "Invalid option : '%c' in '%s'\n",
						*arg, argv[argi]);
					return (-1);
				}	/* switch */
				/* NOTREACHED */
			}	/* while */
		} else if (*arg == '+') {	/* plus options */
			while (*++arg) {	/* while option chars */
				switch (*arg) {	/* case on plus option chars */
				case 'h':	/* help */
					print_help();
					return (-2);
				case 'v':
					version_print();
					return (-2);
				case 'b':
					disable_bell_output = 0;
					break;
				default:
					fprintf(stderr, "Invalid option : '%c' in '%s'\n",
						*arg, argv[argi]);
					return (-1);
				}	/* switch */
				/* NOTREACHED */
			}	/* while */
		} else if (*arg == ' ') {
		} else {	/* Not an option */
			setvar_value("AUTOGAME", arg);
		}		/* end of if else sequence */
	}			/* end of while on arguments passed */
	return (0);
}

/*****************************************
 * usagequit
 *
 * An option was incorrect.  Tell what it should be,
 * and exit program.
 */

static void
usagequit(typ)
	int		typ;
{
	if (typ == -2) {
		exit(0);	/* Help quit. */
	} else {
		fprintf(stderr, "Usage: [+/-hv] <gamename>\n");
	}
	fprintf(stderr, "Try -h option for help.\n");
	exit(1);
}


static void
print_help()
{
	const char	help_list[] =
	{"\n"
		"This is the Empire Interface program EIF.\n"
		"\n"
		"  A Doug Hay Production.\n"
		"  Copyright Doug Hay, 1991.\n"
		"\n"
		" Options:  <too few>\n"
		"   +/-h   - print this minor help display.\n"
		"   +/-v   - print out the version info.\n"
		"   -r     - prevent reading of the ~/.eifrc file.\n"
		"   -D var val  - do a 'setenv var val'.\n"
		"   +/-b   - enable/disable ^G (bell).\n"
		"\n"
	};

	printf("%s\n", help_list);

	printf(" May set any options above, as a string, in the\n");
	printf(" environment variable %s\n", ENVARG);
	printf(" Command line options are parsed last.\n");
}

static void
version_print()
{
	const char	version_list[] =
#include	"version.h"
			;
	printf("%s\n", version_list);
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
