AC_INIT([project_name], [1.0], [daniel@statusengine.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_MACRO_DIRS([m4])
AC_PROG_CC

CFLAGS=""

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_AR
LT_INIT

PKG_CHECK_MODULES([GLIB], [glib-2.0])
AM_CFLAGS=$GLIB_CFLAGS

CFLAGS="${CFLAGS} `pkg-config --cflags naemon`"
NAEMON_LIBS="`pkg-config --libs naemon`"

AC_CHECK_HEADERS([naemon/naemon.h],,AC_MSG_ERROR([Compiling requires naemon.h. Hint: export PKG_CONFIG_PATH=/opt/naemon/lib/pkgconfig/]))
AC_SUBST(NAEMON_LIBS)

# Checks for libraries.
# Example: AC_CHECK_LIB([library], [function])
# You can add library checks if your project depends on any external libraries.

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
