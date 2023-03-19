cc=gcc

all: spr

spr: *.c *.h
	$(cc) -o spr spr.c -Wall -lncurses -lm

run: all
	./spr $(name)

clean:
	rm spr
