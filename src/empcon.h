#ifndef	INC_EIF_EMPCON_H
#define	INC_EIF_EMPCON_H

void	cmd_connect(const char *buf, int sub);
void	cmd_try(const char *buf, int sub);
void	cmd_addgame(const char *buf, int sub);
void	game_died(void);
void	event_hook(void);

#endif	/* INC_EIF_EMPCON_H */

/* vim:ts=8:ai:sw=8:syntax=c
 */
