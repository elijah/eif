#ifndef	INC_EIF_CONNECT_SOCK_H
#define	INC_EIF_CONNECT_SOCK_H

void	writesock(int s, const char *cmd);

int	readsock(int sock, FILE *fp, int timeout, char *prompt, int prsize);

#endif	/* INC_EIF_CONNECT_SOCK_H */

/* vim:ts=8:ai:sw=8:syntax=c
 */
