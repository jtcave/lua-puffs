LUACPATH ?= /usr/lib/lua/5.4

# compilation artifacts
CMOD = puffs.so
OBJS = puffs.o constants.o op_shims.o node.o pcn.o

LIBS = -llua -lpuffs
WARN = -Wall -Wextra

CFLAGS  = -g -fPIC $(WARN) $(INCDIR) $(DEFS)
LDFLAGS = -O -shared -fPIC

CC = gcc
LD = $(MYENV) gcc

.PHONY: all clean install none

all: $(CMOD)

install: $(CMOD)
	cp $(CMOD) $(LUACPATH)

uninstall:
	rm $(LUACPATH)/$(CMOD)

clean:
	rm -f $(OBJS) $(CMOD) lua.core

.c.o: luapuffs.h
	$(CC) -c $(CFLAGS) $(DEFS) $(INCDIR) -o $@ $<

$(CMOD): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
