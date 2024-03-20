AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libnebapi.la

# Setze subdir-objects nur für das Unterverzeichnis src
SUBDIRS = src
srcdir = $(top_srcdir)/src

# Setze die Quelldateien für die Bibliothek
libnebapi_la_SOURCES = nebapi.c


distclean-local: clean
	$(RM) -rf .deps/ Makefile.in aclocal.m4 autom4te.cache config.* configure depcomp install-sh missing *.gz

