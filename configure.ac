AC_INIT([drawtiming],[0.7.1])
AC_CONFIG_SRCDIR([src/main.cc])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS(config.h)
AC_PROG_CC
AC_PROG_CXX
AC_PROG_YACC
AC_PROG_LEX([noyywrap])
AC_PROG_INSTALL
AC_C_CONST
AC_CHECK_LIB(gnugetopt, getopt_long)
AC_CHECK_HEADERS(getopt.h)

AC_ARG_WITH([magick],
  [AS_HELP_STRING([--without-magick], [Build lite version without ImageMagick])],
  [], [with_magick=check])

AS_IF([test "x$with_magick" != "xno"],
  [PKG_CHECK_MODULES([MAGICKXX], [Magick++])],
  [AC_DEFINE([LITE],[1],[Build without ImageMagick])])

AC_CONFIG_FILES([Makefile drawtiming.spec src/Makefile doc/Makefile samples/Makefile])
AC_OUTPUT
