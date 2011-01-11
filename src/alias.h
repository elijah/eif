#ifndef	INC_EIF_ALIAS_H
#define	INC_EIF_ALIAS_H

typedef struct {
	char	*alias, *command, mark;
} alias_t;

void	cmd_alias(const char *alias, int sub);
void	cmd_unalias(const char *buf, int sub);
void	clear_alias_marks(void);
char	*find_alias(const char* buf);

#endif	/* INC_EIF_ALIAS_H */

/* vim:ts=8:ai:sw=8:syntax=c
 */
