uboot-env:	uboot-env.c
	cc -Os -mthumb -o uboot-env uboot-env.c
	strip uboot-env
