cc=gcc

all: spr

debug: d= -g
debug: spr

spr: *.c *.h
	$(cc) -o spr spr.c -Wall -lncurses -lm $(d)

run: all
	./spr $(name)

install: all
	cp spr /usr/bin

uninstall: all
	rm /usr/bin/spr

clean:
	rm spr
