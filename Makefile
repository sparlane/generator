CFLAGS:=-Iinclude -std=c99 -pedantic `pkg-config --cflags --libs lua` -D_GNU_SOURCE -ggdb
CFILES=main.c generate.c process.c world.c module.c object.c

all: generator

generator: $(CFILES) include/* Makefile
	gcc -o $(@) $(CFILES) $(CFLAGS)
