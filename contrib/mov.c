/* Copyright (C) 1999 by Marc Olzheim
 *
 * alias room "dump ${5} ${6:-?des#.} >!tmp/tmp_dump ; runfeed /usr/local/lib/eif/mov ${1} ${2} ${3} ${4}"
 */

#include	"config.h"

#include	<ctype.h>
#include	<stdio.h>
#include	<stdlib.h>
#ifdef	HAVE_STRING_H
#include	<string.h>
#endif	/* HAVE_STRING_H */
#ifdef	HAVE_STRINGS_H
#include	<strings.h>
#endif	/* HAVE_STRINGS_H */

int
main(int argc, char * const argv[])
{
	char	*walk0, *walk1, *sect_x = NULL, *sect_y = NULL;
	char	buffer[4096], bufje[32];
	FILE	*fp;
	int	column, number, diff, i, max;

	if (argc != 5 || ((number = atoi(argv[3])) < 0) ||
	    ((max = atoi(argv[4])) < 0))
	{
		fprintf(stderr, "Usage: %s <commodity> <to_sector> <thresh_from> <max_mov>\n", argv[0]);
		fprintf(stderr, "where: commodity    - the commodity to be moved\n");
		fprintf(stderr, "       to_sector    - the sector to be moved to\n");
		fprintf(stderr, "       thresh_from  - the per sector threshold\n");
		fprintf(stderr, "       max_mov      - the per sector maximum to be moved out\n");
		return(1);
	}

	if (!(fp = fopen("tmp/tmp_dump", "r")))
	{
		perror("tmp/tmp_dump");
		exit(1);
	}

	(void) fgets(buffer, sizeof(buffer), fp);
	(void) fgets(buffer, sizeof(buffer), fp);
	(void) fgets(buffer, sizeof(buffer), fp);
	buffer[0] = 0;
	(void) fgets(buffer, sizeof(buffer), fp);
	if (!strlen(buffer))
	{
		perror("tmp/tmp_dump");
		exit(1);
	}

	column = 0;
	walk0 = walk1 = buffer;
	while (*walk0)
	{
		column++;

		while (*walk1 && !isspace(*walk1))
			walk1++;
		if (*walk1)
			*walk1++ = 0;

		if (!strcmp(walk0, argv[1]))
			break;

		walk0 = walk1;
	}
	if (!*walk0)
	{
		fprintf(stderr, "Unknown commodity `%s'\n", argv[1]);
		exit(0);
	}

	column--;

	while (fgets(buffer, sizeof(buffer), fp))
	{
		walk1 = walk0 = buffer;
		for (i = 0; i < column; i++)
		{
			while (*walk1 && !isspace(*walk1))
				walk1++;
			if (*walk1)
				*walk1++ = 0;
			if (i == 0)
				sect_x = walk0;
			else if (i == 1)
				sect_y = walk0;

			walk0 = walk1;
		}
		if (i < 3)
			break;
		while (*walk1 && !isspace(*walk1))
			walk1++;
		if (*walk1)
			*walk1 = 0;

		sprintf(bufje, "%s,%s", sect_x, sect_y);

		/* If over threshold and not to_sector, move */
		if (((diff = atoi(walk0) - number) > 0) &&
		    strcmp(bufje, argv[2]))
			printf("move %s %s,%s %d %s\n",
				argv[1], sect_x, sect_y,
				(!max || (diff <= max)) ? diff : max,
				argv[2]);
	}

	fclose(fp);

	return(0);
}

/* vim:ts=8:ai:syntax=c
 */
