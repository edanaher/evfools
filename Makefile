
NIX_LD_FLAGS=$(shell echo ${NIX_LDFLAGS} | sed 's/ \+/,/g')

all: evmerge dobuttons

evmerge: evmerge.c
	gcc evmerge.c -o evmerge ${NIX_CFLAGS_COMPILE} -Wl,${NIX_LD_FLAGS} -levdev

dobuttons: dobuttons.c
	gcc dobuttons.c -o dobuttons ${NIX_CFLAGS_COMPILE} -Wl,${NIX_LD_FLAGS} -levdev
