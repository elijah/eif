#ifndef	INC_EIF_CONNECT_EXPECT_H
#define	INC_EIF_CONNECT_EXPECT_H

int	expect(int s, int match, char *buf);
int	sendcmd(int s, int cmd, const char *arg);

#endif	/* INC_EIF_CONNECT_EXPECT_H */

/* vim:ts=8:ai:sw=8:syntax=c
 */
