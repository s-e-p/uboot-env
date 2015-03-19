# $Id$
#
# Makefile for uboot-env
# Copyright (C) 2014 Steve Pelz
# Copyright (C) 2013-2015 Peter Felecan peter@felecan.org
#
# This file is part of uboot-env
#

ACLOCAL_AMFLAGS	=	-I m4

sbin_PROGRAMS	=	\
					uboot-env

uboot_env_sources	=	\
						uboot-env.c

sysconf_DATA	=	\
					uboot-env.conf

ubootenv_DATA	=	\
					default.txt \
					new_kern.txt \
					old_kern.txt \
					reset.txt \
					serialcon.txt \
					u-boot.txt \
					videocon.txt

doc_DATA		=	\
					README-custom \
					README-environments \
					README-uboot-env

EXTRA_DIST		=	\
					$(sysconf_DATA) \
					$(ubootenv_DATA) \
					$(doc_DATA)

clean-local:
	rm -rf	\
			debian/$(PACKAGE) \
			debian/$(PACKAGE).debhelper.log \
			debian/debhelper.log \
			debian/$(PACKAGE).postinst.debhelper \
			debian/$(PACKAGE).postrm.debhelper \
			debian/$(PACKAGE).prerm.debhelper \
			debian/$(PACKAGE).substvars \
			debian/substvars \
			debian/files \
			debian/tmp

# clean every generated file
MAINTAINERCLEANFILES = \
	aclocal.m4 \
	compile \
	COPYING \
	config.guess \
	config.sub \
	configure \
	depcomp \
	INSTALL \
	install-sh \
	ltmain.sh \
	missing \
	Makefile.in \
	$(distdir).tar.xz \
	*.o

package:
	dpkg-buildpackage -us -uc -a $(target_cpu)
