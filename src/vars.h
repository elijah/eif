#ifndef	INC_EIF_VARS_H
#define	INC_EIF_VARS_H

void		cmd_setvar(const char *buf, int sub);
void		cmd_unsetvar(const char *buf, int sub);
void		setvar_value(const char *var, const char *what);
const char	*find_var(char *buf, int *err);

#endif	/* INC_EIF_VARS_H */

/* vim:ts=8:ai:sw=8:syntax=c
 */
