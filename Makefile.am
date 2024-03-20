AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libnebapi.la

# Setze subdir-objects nur für das Unterverzeichnis src
SUBDIRS = src
srcdir = $(top_srcdir)/src

# Setze die Quelldateien für die Bibliothek
libnebapi_la_SOURCES = nebapi.c


distclean-local: clean
	$(RM) -rf .deps/ Makefile Makefile.in aclocal.m4 autogen.sh autom4te.cache/ compile config.guess config.h config.h.in config.log config.status config.sub configure depcomp install-sh libtool ltmain.sh m4/ missing stamp-h1

