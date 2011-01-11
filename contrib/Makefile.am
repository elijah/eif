## Process this file with automake to produce Makefile.in
EXTRA_DIST = buildcost.pl cmap.pl comcount.sh decay.pl edu1.sh edu2.pl genconf.sector genconf.version hap1.sh hap2.pl lev0.sh luse.sh mob.sh pop.sh eifrc.include.in

MOSTLYCLEANFILES = eifrc.include

contribdir = $(pkglibdir)
contrib_PROGRAMS = ex mov with
contrib_DATA = eifrc.include
contrib_SCRIPTS = buildcost.pl cmap.pl comcount.sh decay.pl edu1.sh edu2.pl genconf.sector genconf.version hap1.sh hap2.pl lev0.sh luse.sh mob.sh pop.sh

ex_SOURCES = ex.c

mov_SOURCES = mov.c

with_SOURCES = with.c

edit = sed \
	-e 's,@pkglibdir\@,$(pkglibdir),g' \
	-e 's,@prefix\@,$(prefix),g'

AM_CFLAGS += -Wall -W -Wshadow -Wbad-function-cast -Wcast-qual -Wwrite-strings
AM_CFLAGS += -ansi -pedantic -Wstrict-prototypes -Wmissing-prototypes
AM_CFLAGS += -Wpointer-arith -Wmissing-declarations -Waggregate-return
#AM_CFLAGS += -Werror

eifrc.include: $(srcdir)/eifrc.include.in
	$(edit) $(srcdir)/eifrc.include.in > eifrc.include

install-data-hook:
	( \
		cd $(pkglibdir) || exit 1 ;\
		chmod u+w . ;\
		rm -f lwith nwith pwith swith ;\
		$(LN_S) with lwith ;\
		$(LN_S) with nwith ;\
		$(LN_S) with pwith ;\
		$(LN_S) with swith ;\
	)

uninstall-hook:
	( \
		cd $(pkglibdir) || exit 1 ;\
		rm -f lwith nwith pwith swith ;\
	)


LIBS =
