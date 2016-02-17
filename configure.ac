#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(KB_PM, 0.1.0, [github.com/nladuo/KB_PM])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/box_drawing.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h float.h limits.h stdlib.h string.h sys/param.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([floor getcwd memset mkdir pow socket strchr strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
