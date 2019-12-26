all: main

buffer.c.obj: buffer.c buffer.h
	cc -c buffer.c -o buffer.c.obj

hash.c.obj: hash.c hash.h
	cc -c hash.c -o hash.c.obj

hash.a: buffer.c.obj hash.c.obj
	ar rcs hash.a buffer.c.obj hash.c.obj

main.c.obj: main.c
	cc -c main.c -o main.c.obj

main: hash.a main.c.obj
	cc main.c.obj hash.a -o main

clean:
	rm buffer.c.obj hash.c.obj hash.a main.c.obj main
.PHONY: clean
