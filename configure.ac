# $Id$
#
# configuration for uboot-env
# Copyright (C) 2013-2015 Peter Felecan peter@felecan.org
#
# This file is part of uboot-env
#

AC_INIT(uboot-env, 1.0)
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT(/usr)
AC_REVISION($Revision$)
# this is needed for platform specific actions; e.g. in Makefile.am
# $(build-os)
AC_CANONICAL_SYSTEM

# this must be after AC_INIT but before AC_PROG_CC
CFLAGS="-Os -mthumb"
# look first for the cross-compilers and lastly for the native one
# caveat: if the only available compiler is the native one, i.e. gcc,
# the configuration will succeed only on a ARM platform supporting
# Thumb mode.
AC_PROG_CC([arm-linux-gnueabihf-gcc arm-linux-gnueabi-gcc gcc])

AM_INIT_AUTOMAKE([-Wno-portability subdir-objects dist-xz no-dist-gzip])

AC_SUBST([ubootenvdir], $(eval echo ${datarootdir}/${PACKAGE}))

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
