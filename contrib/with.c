/* Copyright (C) 1999 by Marc Olzheim */

/*
 * # with description do commands with \[\]
 * # p.e. with \# \?food\<400 do mov f 1,1 50 \[\]
 * # to move 50 food from 1,1 to all sectors with less than 400 food.
 * alias with "runfeed /usr/local/lib/eif/${0}"
 * alias lwith "runfeed /usr/local/lib/eif/${0}"
 * alias nwith "runfeed /usr/local/lib/eif/${0}"
 * alias pwith "runfeed /usr/local/lib/eif/${0}"
 * alias swith "runfeed /usr/local/lib/eif/${0}"
 */

/* Yeah, I know.... It sux. But it works, kind of... */

/* with <sectors> do <command with []> */

#include	"../config.h"

#include	<ctype.h>
#include	<err.h>
#include	<stdio.h>
#ifdef	HAVE_STDLIB_H
#include	<stdlib.h>
#endif	/* HAVE_STDLIB_H */
#ifdef	HAVE_STRING_H
#include	<string.h>
#endif	/* HAVE_STRING_H */
#ifdef	HAVE_STRINGS_H
#include	<strings.h>
#endif	/* HAVE_STRINGS_H */
#include	<unistd.h>

char		*progname = NULL;

static void usage(void)
{
	fprintf(stderr, "Usage: %s <sectors> do <command with `[]' in it>\n",
		progname);
	fprintf(stderr, "       %s -f <dump_file>\n", progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
	char		**argw, *walk0, *walk1;
	char		buffer[4096], buffer2[4096], filename[BUFSIZ];
	char		type = 0;
	FILE		*fpr;
	unsigned int	i, has_params = 0;

	buffer[0] = ' ';
	progname = argv[0];

	if (((i = strlen(progname)) > 4) && (progname[i - 5] != '/'))
		type = progname[i - 5];

	(void) snprintf(filename, sizeof(filename), "tmp/%i.dump", getpid());

	if ((argc < 5) && (argc != 3))
		usage();

	if (argc == 3)
	{
		if (strcmp(argv[1], "-f"))
			usage();
		if (!(fpr = fopen(argv[2], "r")))
			err(1, "Could not open dumpfile `%s' for reading",
				filename);

		if (!fgets(buffer + 1, sizeof(buffer) - 1, fpr))
			err(1, "File empty ?!");

		walk0 = buffer + strlen(buffer);
		while (walk0 >= buffer)
		{
			if (isspace(*walk0))
			{
				if (strcmp(walk0 + 1, "[]"))
					sprintf(buffer2 + strlen(buffer2),
						"%s ", walk0 + 1);
				else
				{
					if (!type)
						sprintf(buffer2 +
							strlen(buffer2),
							"%%s,%%s ");
					else
						sprintf(buffer2 +
							strlen(buffer2),
							"%%s ");
					has_params++;
				}
				*walk0-- = 0;
			}
			walk0--;
		}
		buffer2[strlen(buffer2) + 1] = 0;
		buffer2[strlen(buffer2)] = '\n';

		while (fgets(buffer + 1, sizeof(buffer) - 1, fpr))
		{
			if (!isdigit(buffer[1]) && (buffer[1] != '-'))
				continue;

			walk0 = walk1 = buffer + 1;
			while (*walk1 && !isspace(*walk1))
				walk1++;


			while (isspace(*walk1))
				*walk1++ = 0;

			if (!isdigit(*walk1) && (*walk1 != '-'))
				continue;

			walk0 = walk1;

			while (*walk1 && !isspace(*walk1))
				walk1++;
			if (*walk1)
				*walk1++ = 0;

			switch(has_params)
			{
			case	0:
				printf(buffer2);
				break;
			case	1:
				if (type)
					printf(buffer2, buffer + 1);
				else
					printf(buffer2, buffer + 1, walk0);
				break;
			case	2:
				if (type)
					printf(buffer2, buffer + 1, buffer + 1);
				else
					printf(buffer2, buffer + 1, walk0,
							buffer + 1, walk0);
				break;
			case	3:
				if (type)
					printf(buffer2, buffer + 1, buffer + 1,
							buffer + 1);
				else
					printf(buffer2, buffer + 1, walk0,
							buffer + 1, walk0,
							buffer + 1, walk0);
				break;
			case	4:
				if (type)
					printf(buffer2, buffer + 1, buffer + 1,
							buffer + 1, buffer + 1);
				else
					printf(buffer2, buffer + 1, walk0,
							buffer + 1, walk0,
							buffer + 1, walk0,
							buffer + 1, walk0);
				break;
			default:
				printf("echo Too many variable occurences\n");
			}
		}

		fclose(fpr);
	} else
	{
		printf("echo ");
		/* Print command first */
		/* Skip 'do' */
		argw = argv + argc - 1;

		i = 0;

		while ((strcmp(*argw, "do")) && (argw >= argv))
		{
			printf("%s%s", (i++) ? " " : "", *argw);
			argw--;
		}

		if (argw == argv)
			usage();

		printf(" >! %s\n", filename);

		/* Skip 'with' */
		argw = argv + 1;

		if (type)
			(void) putchar(type);
		if (**(argw + 1) == '?')
			printf("dump %s %s %s >> %s\n", *argw, *(argw + 1),
				(type) ? "x" : "civ", filename);
		else /* *(argw + 1) == "do" */
			printf("dump %s %s >> %s\n",
				*argw, (type) ? "x" : "civ", filename);

		if (type)
			(void) putchar(type);
		printf("with -f %s\n", filename);
	}

	return(0);
}

/* vim:ts=8:ai:syntax=c
 */
