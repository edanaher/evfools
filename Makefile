
NIX_LD_FLAGS=$(shell echo ${NIX_LDFLAGS} | sed 's/ \+/,/g')

all: evmerge dobuttons evmorse

evmerge: evmerge.c
	gcc evmerge.c -o evmerge ${NIX_CFLAGS_COMPILE} -Wl,${NIX_LD_FLAGS} -levdev -ltoml

dobuttons: dobuttons.c
	gcc dobuttons.c -o dobuttons ${NIX_CFLAGS_COMPILE} -Wl,${NIX_LD_FLAGS} -levdev

evmorse: evmorse.c
	gcc -g evmorse.c -o evmorse ${NIX_CFLAGS_COMPILE} -Wl,${NIX_LD_FLAGS} -levdev
