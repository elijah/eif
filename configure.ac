AC_INIT([eif], [1.3.4], [marcolz@stack.nl])
AC_CONFIG_SRCDIR([src/eif.c])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.

dnl Why doesn't configure do this by itself ? It might be necessary for
dnl header checks...

if test x$includedir != x
then
	CPPFLAGS="${CPPFLAGS} -I${includedir}"
fi

if test x$oldincludedir != x
then
	CPPFLAGS="${CPPFLAGS} -I${oldincludedir}"
fi

CPPFLAGS="${CPPFLAGS} -I/usr/local/include -I/usr/pkg/include"
LDFLAGS="${LDFLAGS} -L/usr/local/lib -L/usr/pkg/lib"

if test x$libdir != x
then
	LDFLAGS="${LDFLAGS} -L${libdir}"
fi

if test x$GCC != x
then
	CFLAGS="${CFLAGS} -Wall -W -Wshadow -Wbad-function-cast -Wcast-qual -Wwrite-strings"
fi


AC_PROG_CC
AC_PROG_LN_S
AC_PROG_GCC_TRADITIONAL
AC_PROG_RANLIB

dnl Checks for libraries.

AC_CHECK_LIB(bsd, bsd_ioctl)
AC_CHECK_LIB(curses, tgetent)

dnl AC_CHECK_LIB(dl, main)

AC_CHECK_LIB(readline, rl_unbind_key, ,AC_MSG_ERROR([You really need this one !.]))

AC_CHECK_LIB(termcap, main)

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h malloc.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/select.h sys/time.h termcap.h termio.h unistd.h readline/tilde.h)
AC_CHECK_HEADERS(readline/readline.h, , AC_MSG_ERROR([You really need this one !.]))

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(getaddrinfo getnameinfo getwd select setenv socket strdup unsetenv)

AC_DEFUN([EIF_FD_SET_NEEDS_STRUCT],[
	AC_CACHE_CHECK([whether fd_set needs 'struct'], [ac_cv_fd_set_needs_struct],
		[
		AC_TRY_COMPILE([
			#ifdef	HAVE_SYS_SELECT_H
			#include	<sys/select.h>
			#endif
			#ifdef	HAVE_SYS_TIME_H
			#include	<sys/time.h>
			#endif
			#include	<sys/types.h>
		],[
			fd_set		a;
		],[ac_cv_fd_set_needs_struct=no],AC_TRY_COMPILE([
			#ifdef	HAVE_SYS_SELECT_H
			#include	<sys/select.h>
			#endif
			#ifdef	HAVE_SYS_TIME_H
			#include	<sys/time.h>
			#endif
			#include	<sys/types.h>
		],[
			struct fd_set	a;
		],[ac_cv_fd_set_needs_struct=yes],
		[ac_cv_fd_set_needs_struct=no]))
	])
	if test "$ac_cv_fd_set_needs_struct" = "yes"; then
		AC_DEFINE(FD_SET_NEEDS_STRUCT, [struct])
	else
		AC_DEFINE(FD_SET_NEEDS_STRUCT,[])
	fi
])

AC_DEFUN([EIF_READLINE_HANDLES_CONST],[
	SAVE_CFLAGS="${SAVE_CFLAGS}"
	CFLAGS="${CFLAGS} -Werror"
	AC_CACHE_CHECK([whether readline handles 'const char *'], [ac_cv_readline_uses_const],
		[
		AC_TRY_COMPILE([
			#include	<readline/history.h>
		],[
			write_history("/dev/tty");
		],[ac_cv_readline_uses_const=yes],
		[ac_cv_readline_uses_const=no])
	])
	CFLAGS="${SAVE_CFLAGS}"
	if test "$ac_cv_readline_uses_const" = "yes"; then
		AC_DEFINE(READLINE_HANDLES_CONST, 1)
	fi
])

AC_DEFUN([EIF_READLINE_UNBIND_KEY],[
	SAVE_CFLAGS="${SAVE_CFLAGS}"
	CFLAGS="${CFLAGS} -Wimplicit-function-declaration -Werror"
	AC_CACHE_CHECK([whether readline.h prototypes 'rl_unbind_key'], ac_cv_readline_unbind_key,
		[
		AC_TRY_COMPILE([
			#include	<stdio.h>
			#include	<readline/readline.h>
		],[
			rl_unbind_key(1);
		],[ac_cv_readline_unbind_key=yes],
		[ac_cv_readline_unbind_key=no])
	])
	CFLAGS="${SAVE_CFLAGS}"
	if test "$ac_cv_readline_unbind_key" = "yes"; then
		AC_DEFINE(READLINE_UNBIND_KEY, 1)
	fi
])

dnl old acconfig.h
AH_TOP([#ifndef EIF_CONFIG_H
#define EIF_CONFIG_H
])

AH_TEMPLATE([FD_SET_NEEDS_STRUCT], [Some systems need 'struct fd_set', others just 'fd_set'])

AH_TEMPLATE([EIF_READLINE_HANDLES_CONST], [Readline is old code and does not always handle 'const char *'])

AH_TEMPLATE([EIF_READLINE_UNBIND_KEY], [OpenBSD uses a stripped version of readline's headerfiles])

AH_BOTTOM([#ifdef  HAVE_GETADDRINFO
#ifdef  HAVE_GETNAMEINFO
#define USE_IPv6
#endif  /* HAVE_GETNAMEINFO */
#endif  /* HAVE_GETADDRINFO */

#ifndef HAVE_READLINE_TILDE_H
#ifdef  READLINE_HANDLES_CONST
extern char *tilde_expand(const char *);
#else   /* READLINE_HANDLES_CONST */
extern char *tilde_expand(char *);
#endif  /* READLINE_HANDLES_CONST */
#endif  /* HAVE_READLINE_TILDE_H */

#ifndef READLINE_UNBIND_KEY
int     rl_unbind_key(int key);
#endif  /* READLINE_UNBIND_KEY */
#endif  /* EIF_CONFIG_H */
])

dnl Local additions from aclocal.m4
EIF_FD_SET_NEEDS_STRUCT
EIF_READLINE_HANDLES_CONST
EIF_READLINE_UNBIND_KEY

AC_SUBST(AM_CFLAGS)

AC_CONFIG_FILES([ \
	Makefile \
	src/Makefile \
	src/connect/Makefile \
	contrib/Makefile \
	doc/Makefile \
	])
AC_OUTPUT
