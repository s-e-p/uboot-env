all:		uboot-env

install:	uboot-env
	cp uboot-env $INSTALL/sbin
	cp uboot-env.conf $INSTALL/etc
