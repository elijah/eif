#ifndef	INC_EIF_PRINT_H
#define	INC_EIF_PRINT_H

extern int	disable_bell_output;

void	put(char c);
void	prt(const char * const format, ...) __attribute__((format(printf, 1, 2)));
void	eprt(const char * const format, ...) __attribute__((format(printf, 1, 2)));
void	notstdio_prt(const char * const format, ...) __attribute__((format(printf, 1, 2)));
void	output_file(FILE *fp, int ispipe);
FILE	*output_curout(void);
FILE	*output_to(char *name, int xs, int *oldispipe);
FILE	*output_topipe(char *cmd, int *oldispipe);
FILE	*output_turnoff(int *oldispipe);
void	myfputs(const char *str, FILE *fp);

#endif	/* INC_EIF_PRINT_H */

/* vim:ts=8:ai:sw=8:syntax=c
 */
