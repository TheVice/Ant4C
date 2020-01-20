all: main

hash.blake3.c.obj: hash.blake3.c hash.h
	cc -c hash.blake3.c -o hash.blake3.c.obj

hash.a: hash.blake3.c.obj
	ar rcs hash.a hash.blake3.c.obj

main.c.obj: main.c
	cc -c main.c -o main.c.obj $(CFLAGS)
.PHONY: main.c.obj

main: hash.a main.c.obj
	cc main.c.obj hash.a -lm -o main

clean:
	rm hash.blake3.c.obj main.c.obj hash.a main
.PHONY: clean
