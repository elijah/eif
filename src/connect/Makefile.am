## Process this file with automake to produce Makefile.in
noinst_LIBRARIES = libconnect.a
libconnect_a_SOURCES = connect.c expect.c login.c sock.c connect.h expect.h \
			login.h proto.h sock.h

AM_CFLAGS += -Wall -W -Wshadow -Wbad-function-cast -Wcast-qual -Wwrite-strings
#AM_CFLAGS += -Werror
AM_CFLAGS += -g
# -Werror is not possible because of strange prototypes for readline functions.
