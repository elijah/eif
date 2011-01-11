#ifndef	INC_EIF_GETCOM_H
#define	INC_EIF_GETCOM_H

void	cmd_exec(const char *buf, int sub);
void	cmd_if(const char *buf, int sub);

int	get_input(char *retbuf, int len, const char *prompt);
int	get_main_input(char *retbuf, int len, const char *prompt);
int	unescape_backslashes(char *dst, const char *src);

#endif	/* INC_EIF_GETCOM_H */

/* vim:ts=8:ai:sw=8:syntax=c
 */
