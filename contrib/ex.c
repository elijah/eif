/* Copyright (C) 1998 by Marc Olzheim
 *
 * alias expp "runfeed /usr/local/lib/eif/ex ${1} ${2} ${3}"
 */

#include	"../config.h"

#include	<stdio.h>
#include	<stdlib.h>
#ifdef	HAVE_STRING_H
#include	<string.h>
#endif	/* HAVE_STRING_H */
#ifdef	HAVE_STRINGS_H
#include	<strings.h>
#endif	/* HAVE_STRINGS_H */

char		*prog_name;

static void usage(void)
{
		fprintf(stderr, "Usage: %s <from> <base_path> <expl_path>\n",
			prog_name);
		fprintf(stderr, "where: from      - the sector to explore from\n");
		fprintf(stderr, "       base_path - the path to move by before exploring\n");
		fprintf(stderr, "       expl_path - the path to explore\n");
		exit(1);
}

static void	chng_loc(int *x, int *y, char c)
{
	switch(c)
	{
	case 'b':
		(*x)--; (*y)++;
		break;
	case 'g':
		(*x) -= 2;
		break;
	case 'y':
		(*x)--; (*y)--;
		break;
	case 'u':
		(*x)++; (*y)--;
		break;
	case 'j':
		(*x) += 2;
		break;
	case 'n':
		(*x)++; (*y)++;
		break;
	default:
		fprintf(stderr, "Illegal path character `%c'\n", c);
		usage();
	}
}

int
main(int argc, char * const argv[])
{
	int		x_base, y_base, x, y;
	char		*base, *explore, *temp, *walk;
	unsigned int	i, len, max;

	prog_name = argv[0];

	if (argc != 4)
		usage();
	
	base = strdup(argv[2]);
	if (!base)
	{
		perror("Not enough mem (base)");
		return(1);
	}
	len = strlen(base);
	while (base[len - 1] == 'h')
		base[--len] = 0;

	explore = strdup(argv[3]);
	if (!explore)
	{
		perror("Not enough mem (explore)");
		return(1);
	}
	len = strlen(explore);
	while (explore[len - 1] == 'h')
		explore[--len] = 0;

	temp = strdup(argv[1]);
	if (!temp)
	{
		perror("Not enough mem (temp)");
		return(1);
	}
	walk = temp;
	while (*walk && *walk != ',')
		walk++;
	if (*walk)
		*walk++ = 0;

	x_base = atoi(temp);
	y_base = atoi(walk);
	if ((!x_base && (*temp != '0')) || (!y_base && (*walk != '0')))
	{
		fprintf(stderr, "Illegal 'from'\n");
		usage();
	}

	walk = base;
	while (*walk)
		chng_loc(&x_base, &y_base, *walk++);

	x = x_base; y = y_base;
	for (max = 1; max <= len; max++)
	{
		printf("explore civ %s 1 %s", argv[1], base);
		for (i = 0; i < max; i++)
			(void) putchar(explore[i]);

		chng_loc(&x, &y, explore[i - 1]);

		printf("h\ndesignate %d,%d +\n", x, y);
	}

	return(0);
}

/* vim:ts=8:ai:syntax=c
 */
