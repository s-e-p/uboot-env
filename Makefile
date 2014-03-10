all:		uboot-env

install:	uboot-env
	cp uboot-env $INSTALL/sbin
	cp uboot-env.conf $INSTALL/etc

uboot-env:	uboot-env.c
	cc -Os -mthumb -o uboot-env uboot-env.c
	strip uboot-env
