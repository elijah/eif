## Process this file with automake to produce Makefile.in
bin_PROGRAMS = eif
eif_SOURCES = alias.c compick.c eif.c empcon.c games.c gencoms.c getcom.c \
		init.c main.c print.c vars.c alias.h cntl.h compick.h data.h \
		eif.h empcon.h games.h gencoms.h getcom.h init.h main.h \
		print.h vars.h version.h
eif_LDADD = connect/libconnect.a

SUBDIRS = connect

AM_CFLAGS += -Wall -W -Wshadow -Wbad-function-cast -Wcast-qual -Wwrite-strings
#AM_CFLAGS += -Werror
AM_CFLAGS += -g
