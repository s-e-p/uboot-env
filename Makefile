ifeq ($(OS),Windows_NT)
    CCFLAGS += -D WIN32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CCFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        CCFLAGS += -D OSX
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CCFLAGS += -mthumb
    endif
endif

all:		uboot-env

install:	uboot-env
	@cp uboot-env $(INSTALL)/sbin
	@cp uboot-env.conf $(INSTALL)/etc

uboot-env:	uboot-env.c
	@cc $(CCFLAGS) -Os -o uboot-env uboot-env.c
	@strip uboot-env

clean:
	@rm uboot-env
