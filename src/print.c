/*************************************************************
 *  $Id: print.c,v 1.5 2003/05/24 15:26:32 marcolz Exp $
 *
 *  print.c
 *
 */
/*******************************************************
 *  Copyright (C) Doug Hay, 1989,90,91.
 *  Permission to use and abuse this code, as long
 *  as this copyright notice stays intact and with the
 *  code.  No warranty implied.  This code supplied as is.
 *******************************************************/

#include	"config.h"

#include	<stdarg.h>
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
#ifdef	HAVE_TERMCAP_H
#include	<termcap.h>
#endif				/* HAVE_TERMCAP_H */

#include	"cntl.h"
#include	"data.h"
#include	"getcom.h"
#include	"print.h"

/* Prevent ^G from getting out. */
int		disable_bell_output = 0;

/* #define FUNNY_HIGH if highlighting doesn't work on your terminal */
/* #define FUNNY_HIGH */

static int	curout_isapipe;
static int	terminited = 0;
static char	*Terminal_SO, *Terminal_SE;

static void	stripprint(const char *str, FILE * out);
static void	terminit(void);
static void	hiprint(const char *str, FILE * out);

/*******************************************
 * stripprint
 *
 * Strip any EMPIRE hilighting bits from the
 * string, then print it to passed file.
 */
static void
stripprint(str, out)
	const char	*str;
	FILE		*out;
{
	char		buf[1024];
	char		*cp;

	strncpy(buf, str, sizeof(buf));
	cp = buf;
	if (disable_bell_output) {
		while (*cp) {
			if (*cp == 07) {
				if (!*(cp + 1)) {
					*cp = 0;
				} else
					*cp = ' ';
			}
			*cp++ &= 0x7f;
		}
	} else {
		while (*cp)
			*cp++ &= 0x7f;
	}
	fprintf(out, "%s", str);
}

/*******************************************
 * hiprint
 *
 * Find any characters in passed string with EMPIRE
 * highlighting bits set, and hilight those characters.
 */
static void
hiprint(str, out)
	const char	*str;
	FILE		*out;
{
	char		buf[1024];
	char		*cp;
	const char	*ws;
	int		hav_hi = 0;

	if (!terminited) {
		terminited = 1;
		terminit();
	}
	/*
	 * This routine uses "fprintf" instead of "fputs".  Why? Because of
	 * an amazing fputs bug in SunOS 4.0.2 on 386i's. And since I'm stuck
	 * with it, so are you.
	 */
	strncpy(buf, str, sizeof(buf));
	if (disable_bell_output) {
		for (cp = buf; *cp; cp++) {
			if (*cp & 0x80)
				hav_hi = 1;
			if (*cp == 07) {
				if (!*(cp + 1)) {
					*cp = 0;
				} else
					*cp = ' ';
			}
		}
	} else {
		for (ws = str; *ws; ws++) {
			if (*ws & 0x80)
				hav_hi = 1;
		}
	}
	if (!hav_hi || !Terminal_SO || !Terminal_SE || !*Terminal_SO
		|| !*Terminal_SE) {
		if (hav_hi)
			for (cp = buf; *cp; cp++)
				*cp &= 0x7f;
		fprintf(out, "%s", str);
	} else {
		for (cp = buf; *cp;) {
			if (*cp & 0x80) {
				fprintf(out, "%s", Terminal_SO);
				for (; (*cp & 0x80 && putc((*cp & 0x7f), out)); cp++)
					;
				fprintf(out, "%s", Terminal_SE);
			} else
				putc(*cp++, out);
		}
	}
}

/*******************************************
 * prt
 *
 * A printf equivalent.
 * Used to control printout levels, and
 * to copy output to files if necessary.
 */

/* VARARGS1 */
void
prt(const char *const format,...)
{
	va_list		ap;
	char		buf[1024];

	va_start(ap, format);
	if (vsnprintf(buf, sizeof(buf), format, ap) == sizeof(buf))
		*buf = 0;
	va_end(ap);

	if ((curout == stdout) || (curout == stderr)) {
		hiprint(buf, curout);
	} else if (curout) {
		stripprint(buf, curout);
	}
}

/*******************************************
 * put
 *
 * Print a single character to current output.
 */
void
put(c)
	char		c;
{
	if (curout)
		putc(c, curout);
}

/*******************************************
 * myfputs
 *
 * Put string to a file.
 */
void
myfputs(str, fp)
	const char	*str;
	FILE		*fp;
{
	if ((fp == stdout) || (fp == stderr)) {
		hiprint(str, fp);
	} else
		stripprint(str, fp);
}

/*******************************************
 * eprt
 *
 * Print string to stderr.
 * Echo to current output if not to the display.
 */
/* VARARGS1 */
void
eprt(const char *const format,...)
{
	va_list		ap;
	char		buf[1024];

	va_start(ap, format);
	if (vsnprintf(buf, sizeof(buf), format, ap) == sizeof(buf))
		*buf = 0;
	va_end(ap);

	hiprint(buf, stderr);

	if ((curout != stdout) && (curout != stderr) && curout)
		stripprint(buf, curout);
}

/*******************************************
 * notstdio_prt
 *
 * Print if current output is not to terminal.
 */
/* VARARGS1 */
void
notstdio_prt(const char *const format,...)
{
	va_list		ap;
	char		buf[1024];

	va_start(ap, format);
	if (vsnprintf(buf, sizeof(buf), format, ap) == sizeof(buf))
		*buf = 0;
	va_end(ap);

	if ((curout != stdout) && (curout != stderr) && curout)
		stripprint(buf, curout);
}

/*******************************************
 * output_curout
 *
 * Return the current output file, so it can
 * be used for something.
 */
FILE		*
output_curout()
{
	return (curout);
}

/*******************************************
 * output_file
 *
 * Close current output if not display, and
 * make passed file current output.
 *
 * Used to recover after a redirection.
 */
void
output_file(fp, ispipe)
	FILE		*fp;
	int		ispipe;
{
	if (fp && (fp == curout))
		return;

	if (curout && (curout != stdout) && (curout != stderr)) {
		if (curout_isapipe) {
			pclose(curout);
		} else
			fclose(curout);
	}
	curout = fp;
	curout_isapipe = ispipe;
}

/*******************************************
 * output_to
 *
 * Open file with given access, and make it current
 * output.
 * Return current output so that it may be recovered
 * when done with new file.
 */
FILE		*
output_to(name, xs, oldispipe)
	char		*name;
	int		xs;
	int		*oldispipe;
{
	FILE		*fp, *fp2;
	const char	*acc = "a";

	if (!xs) {
		if ((fp = fopen(name, "r"))) {
			fclose(fp);
			return ((FILE *) - 1);
		}
	} else if (0 > xs) {
		acc = "w";
	}
	if (!strcmp(name, "-")) {
		fp2 = curout;
		curout = stdout;
		*oldispipe = curout_isapipe;
		curout_isapipe = 0;
		return (fp2);
	} else if ((fp = fopen(name, acc))) {
		fp2 = curout;
		curout = fp;
		*oldispipe = curout_isapipe;
		curout_isapipe = 0;
		return (fp2);
	}
	return ((FILE *) 0);
}

/*******************************************
 * output_topipe
 *
 * Open a pipe to given command, and make it current
 * output.
 * Return current output so that it may be recovered
 * when done with new file.
 */
FILE		*
output_topipe(cmd, oldispipe)
	char		*cmd;
	int		*oldispipe;
{
	FILE		*fp, *fp2;
	char		pipe_cmd[1024];

	unescape_backslashes(pipe_cmd, cmd);

	if ((fp = popen(pipe_cmd, "w"))) {
		fp2 = curout;
		curout = fp;
		*oldispipe = curout_isapipe;
		curout_isapipe = 1;
		return (fp2);
	}
	return ((FILE *) 0);
}

/*******************************************
 * output_turnoff
 *
 * Disable any output.
 * Return current output so that it may be recovered
 * when done with new file.
 */
FILE		*
output_turnoff(oldispipe)
	int		*oldispipe;
{
	FILE		*fp2;

	fp2 = curout;
	curout = (FILE *) 0;
	*oldispipe = curout_isapipe;
	curout_isapipe = 0;
	return (fp2);
}


/*******************************************
 * terminit
 *
 * Go find the highlighting strings for the terminal.
 */

static void
terminit()
{
	char		*cp, *term;

	/*
	 * Ncurses 5.1 uses (NCURSES_CONST char *), which is empty on FreeBSD
	 * 4.x systems...
	 */
	char		ncurses_cast_qual_buf[4] = "so", *ncurses_cast_qual_buf_p = ncurses_cast_qual_buf;

	static char	nullstring[2], termbuf[1024], data[1024], *area;

	area = data;
	Terminal_SO = Terminal_SE = nullstring;

	term = getenv("TERM");
	if (!term) {
		fprintf(stderr, "Unable to get TERM environment variable.\n");
		return;
	}
	if (tgetent(termbuf, term) == -1) {
		fprintf(stderr, "terminit tgetent failed\n");
		return;
	}
	cp = area;
	if (tgetstr(ncurses_cast_qual_buf_p, &area) == NULL) {
#if 0
		fprintf(stderr, "terminit, no so\n");
#endif
	} else
		Terminal_SO = cp;

	cp = area;
	ncurses_cast_qual_buf_p[1] = 'e';
	if (tgetstr(ncurses_cast_qual_buf_p, &area) == NULL) {
#if 0
		fprintf(stderr, "terminit, no se\n");
#endif
	} else
		Terminal_SE = cp;

#ifdef FUNNY_HIGH
	++Terminal_SO;
	++Terminal_SE;
#endif
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
