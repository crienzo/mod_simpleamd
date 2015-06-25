AC_INIT([mod_simpleamd], [1.4.19], http://github.com/crienzo/mod_simpleamd/issues)

AM_INIT_AUTOMAKE

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
m4_ifdef([AS_VAR_COPY], [],
[m4_define([AS_VAR_COPY],
[AS_LITERAL_IF([$1[]$2],[$1=$$2],[eval $1=\$$2])])])
m4_ifdef([PKG_CHECK_VAR], [],
[AC_DEFUN([PKG_CHECK_VAR],
	[AC_REQUIRE([PKG_PROG_PKG_CONFIG])dnl
	 AC_ARG_VAR([$1], [value of $3 for $2, overriding pkg-config])dnl
	 _PKG_CONFIG([$1], [variable="][$3]["], [$2])
	 AS_VAR_COPY([$1], [pkg_cv_][$1])
	 AS_VAR_IF([$1], [""], [$5], [$4])dnl
	]) # PKG_CHECK_VAR
])

AM_PROG_CC_C_O

AC_PROG_LIBTOOL

PKG_CHECK_MODULES([FREESWITCH],[freeswitch],[],[])
PKG_CHECK_VAR([moddir],[freeswitch],[modulesdir])

AC_SUBST(FREESWITCH_CFLAGS)
AC_SUBST(FREESWITCH_LDFLAGS)
AC_SUBST(moddir)

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
