#ifndef	INC_EIF_GAMES_H
#define	INC_EIF_GAMES_H

void	list_games(const char *gam);

int	add_game(const char *name, const char *host, const char *port,
		 const char *coun, const char *rep, int xsize, int ysize,
		 int proto, int waitt, const char *dataf);
int	switch_to(char *game);
void	switch_gameout(void);

#endif	/* INC_EIF_GAMES_H */

/* vim:ts=8:ai:sw=8:syntax=c
 */
