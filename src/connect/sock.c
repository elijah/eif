/*******************************************************
 * $Id: sock.c,v 1.7 2003/10/06 16:15:31 marcolz Exp $
 *
 * sock.c
 *
 * Send and receive information to/from emp_player.
 *
 * Doug Hay.
 *
 * Heavily modified from source by:
 *    Jon Sari, 1990
 */

#include	"config.h"

#include	<errno.h>
#include	<stdio.h>
#ifdef	HAVE_STRING_H
#include	<string.h>
#endif				/* HAVE_STRING_H */
#ifdef	HAVE_STRINGS_H
#include	<strings.h>
#endif				/* HAVE_STRINGS_H */
#include	<sys/time.h>
#include	<sys/types.h>
#include	<sys/uio.h>
#ifdef	HAVE_UNISTD_H
#include	<unistd.h>
#endif				/* HAVE_UNISTD_H */

#ifdef	HAVE_READLINE_READLINE_H
#include	<readline/readline.h>
#endif	/* HAVE_READLINE_READLINE_H */

#include	"proto.h"

#include	"sock.h"
#include	"../print.h"

static char	*grab_line(char *p, int *no, char *line);
static int	dreadable(int s, int waitt);

/* If this is set, we had an interrupt */
extern int	Interrupt;
/* This set if a pipe interrupt */
extern int	Pipe_Interrupt;

/* Parsed from prompts coming back. */
extern int	game_time, game_btus;

static char	sockbuf[4096];
extern char	inform_buffer[];

/*****************************************
 * readsock
 *
 *
 * Returns:
 *  -1 - when lost the connection.
 *   0 - not dreadable, data timeout?
 *   1 - a main prompt is sent.
 *   2 - when a flush is sent.
 */
int
readsock(sock, fp, timeout, prompt, prsize)
	int		sock;	/* Descriptor to read from */
	FILE		*fp;	/* Pointer to write to */
	int		timeout;/* Max time in seconds to wait for read */
	char		*prompt;
	int		prsize;
{
	int		sentabort = 0;	/* Whether we sent an abort or not. */
	static char	*ptr;	/* Current place in the display buffer */
	int		the_ret = 0;	/* Possible return value */
	char		*cp;
	int		cc;
	unsigned int	count;
	int		expected;
	char		display[4096];

	/* If there was an interrupt, let us try to abort. */
	if (Interrupt) {
		eprt("Sending abort\n");
		writesock(sock, "\naborted");
		Interrupt = 0;
		sentabort = 1;
	}
	ptr = NULL;
	expected = 0;

	/*
	 * While there is data to read, within the timeout we are to wait,
	 * read data.
	 */
	while (dreadable(sock, timeout)) {
		if (ptr && *ptr) {
			/* We have an incomplete line left in the buffer. */
			count = strlen(ptr);
			strncpy(sockbuf, ptr, sizeof(sockbuf) - (count + 1));
			sockbuf[sizeof(sockbuf) - 1] = 0;
		} else
			count = 0;

		ptr = sockbuf + count;

		/* Get more data from emp_player */
		if ((cc = read(sock, ptr, sizeof(sockbuf) - (count + 1))) < 0) {
			perror("socket read");
			return (-1);
		}
		if (cc == 0) {
			eprt("Server EOF\n");
			return (-1);
		}
		/* Make sure it is ended with a null */
		if (cc + count > sizeof(sockbuf) - 1)
			eprt("Overflow !\n");
		sockbuf[cc + count] = '\0';
		ptr = sockbuf;

		/* While there are complete lines left, look at them */
		while (index(ptr, '\n')) {
			/* Parse the line. */
			ptr = grab_line(ptr, &expected, display);

			if (-1 != the_ret)
				the_ret = 0;
			switch (expected) {
			case C_PROMPT:
				(void)sscanf(display, "%d %d", &game_time, &game_btus);
				if (-1 != the_ret)
					the_ret = 1;
				sentabort = 0;
				break;

			case C_DATA:
				/*
				 * XXXX We need to check for inverse
				 * characters.
				 */
				if (fp && !Pipe_Interrupt)
					myfputs(display, fp);
				break;

			case C_EXIT:
				/* Well, says he wants to quit. */
				/* Game over, man. */
				if (fp)
					fprintf(fp, "%s\n", display);
				the_ret = -1;
				break;

			case C_FLUSH:
				/* A prompt, but not a command-line one. */
				if ((cp = index(display, '\n')))
					*cp = '\0';
				if (prompt) {
					strncpy(prompt, display, prsize - 1);
				} else
					printf("%s", display);
				if ((-1 != the_ret) && !sentabort)
					the_ret = 2;
				break;

			case C_FLASH:
				printf("\n%s", display);
				break;
			case C_INFORM:
				printf("\n%s", display);

				(void) snprintf(inform_buffer, BUFSIZ, "%s",
						display);
				if ('\n' == inform_buffer[strlen(inform_buffer) - 1])
					inform_buffer[strlen(inform_buffer) - 1] = 0;

				/* If a message was printed, redraw. */
				if (inform_buffer[0])
					rl_forced_update_display();

				break;

			case C_CMDERR:
			case C_BADCMD:
				eprt("Error; %s\n", display);
				break;

			case C_PIPE:	/* Should not occur */
			case C_REDIR:	/* Should not occur */
				eprt("Damn, let a redirection command through.\n");
				break;

			case C_EXECUTE:	/* Should not occur */
				eprt("Damn, let an exec command through.\n");
				break;

			case C_CMDOK:
			case C_INIT:
			case C_NOECHO:
			case C_ABORT:
				eprt("Unexpected protocol, %d\n", expected);
				break;

			default:
				eprt("Unknown protocol code, %d\n", expected);
				break;
			}
		}
		if (!*ptr) {
			ptr = sockbuf;
			*ptr = '\0';
		}
		if (Interrupt) {
			eprt("Sending abort\n");
			Interrupt = 0;
			writesock(sock, "\naborted");
			sentabort = 1;
		}
		if (fp && !Pipe_Interrupt)
			(void) fflush(fp);
		if (the_ret) {
			/*
			 * If we had a return set  (a prompt of some form)
			 * and there is no more data to read, then return the
			 * prompt type.
			 */
			if (!dreadable(sock, 0) && !*ptr)
				return (the_ret);
		}
	}
	return (0);
}

/*
 * A neat-o routine to tell whether a descriptor has any data available.
 * 
 * Jon "Rob McKenna was a rain god . . . " Sari, 1990
 */
/*
 * Returns: 1 - if 's' has data ready to be read. 0 - if 's' has no data
 * ready to be read. -1 - if a problem occured.  Maybe an Interrupt.
 * 
 * 'waitt' is the time, in seconds, to wait for data. If 'waitt' is zero, then
 * this is a check to see if data is available.
 */

static int
dreadable(s, waitt)
	int		s, waitt;
{
	FD_SET_NEEDS_STRUCT fd_set read_fd;
	struct timeval	timeout;
	int		rval;

	timeout.tv_sec = waitt;
	timeout.tv_usec = 0;

	FD_ZERO(&read_fd);
	FD_SET(s, &read_fd);

	rval = select(s + 1, &read_fd, (fd_set *) 0, (fd_set *) 0, &timeout);
	if (rval < 0 && errno == EINTR)
		Interrupt++;

	return rval;
}

/*
 * A routine to write a string to a socket. Actually, pretty dull.
 * 
 * Jon "Fwuh!" Sari, 1990
 */

void
writesock(s, cmd)
	int		s;
	const char	*cmd;
{
	int		i = 0;

	while (*cmd)
		sockbuf[i++] = *cmd++;
	sockbuf[i++] = '\n';
	sockbuf[i] = 0;
	if (write(s, sockbuf, (size_t) i) < i)
		perror("socket write");
}

/*
 * A routine to replace sscanf("%x%*c%[^\n]", &no, line) which strips off
 * signed character data.  (It's faster, anyhow.)
 * 
 * Jon "Heh heh heh" Sari, 1990
 */

static char	*
grab_line(p, no, line)
	char		*p;
	int		*no;
	char		*line;
{
	*no = (*p <= '9') ? *p - '0' : ((*p < 'G') ? (*p - 'A') + 10 : (*p - 'a') + 10);
	p += 2;
	while (*p && *p != '\n')
		*line++ = *p++;
	if ('\n' == *p) {
		*line++ = *p++;
	} else
		*line++ = '\n';
	*line = '\0';
	return (p);
}

/* vim:ts=8:ai:sw=8:syntax=c
 */
