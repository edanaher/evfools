
NIX_LD_FLAGS=$(shell echo ${NIX_LDFLAGS} | sed 's/ \+/,/g')

all:
	gcc evmerge.c -o evmerge ${NIX_CFLAGS_COMPILE} -Wl,${NIX_LD_FLAGS} -levdev