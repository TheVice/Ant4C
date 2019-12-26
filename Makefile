all: hash
buffer.c.obj: buffer.c buffer.h
	cc -c buffer.c -o buffer.c.obj

hash.c.obj: hash.c hash.h
	cc -c hash.c -o hash.c.obj

hash: buffer.c.obj hash.c.obj
	ar rcs hash.a buffer.c.obj hash.c.obj

clean:
	rm buffer.c.obj hash.c.obj hash.a
.PHONY: clean
