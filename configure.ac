#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([dfu-util-qda],[1.1.0 beta (dfu-util 0.8)],,,])
AC_CONFIG_AUX_DIR(m4)
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([config.h])

# Test for new silent rules and enable only if they are available
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
LIBS="$LIBS"
CFLAGS="$CFLAGS"

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([usbpath.h windows.h sysexits.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([ftruncate getpagesize nanosleep err])

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT
