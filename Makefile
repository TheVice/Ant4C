all: main

buffer.c.obj: buffer.c buffer.h
	cc -c buffer.c -o buffer.c.obj

hash.crc32.c.obj: hash.crc32.c hash.h
	cc -c hash.crc32.c -o hash.crc32.c.obj

hash.blake2.c.obj: hash.blake2.c hash.h
	cc -c hash.blake2.c -o hash.blake2.c.obj

hash.sha3.c.obj: hash.sha3.c hash.h
	cc -c hash.sha3.c -o hash.sha3.c.obj

hash.c.obj: hash.c hash.h
	cc -c hash.c -o hash.c.obj

math_unit.c.obj: math_unit.c math_unit.h
	cc -c math_unit.c -o math_unit.c.obj

hash.a: buffer.c.obj hash.c.obj hash.crc32.c.obj hash.blake2.c.obj hash.sha3.c.obj math_unit.c.obj
	ar rcs hash.a buffer.c.obj hash.c.obj math_unit.c.obj hash.crc32.c.obj hash.blake2.c.obj hash.sha3.c.obj

main.c.obj: main.c
	cc -c main.c -o main.c.obj $(CFLAGS)
.PHONY: main.c.obj

main: hash.a main.c.obj
	cc main.c.obj hash.a -lm -o main

clean:
	rm buffer.c.obj hash.c.obj hash.crc32.c.obj hash.blake2.c.obj hash.sha3.c.obj hash.a math_unit.c.obj main.c.obj main
.PHONY: clean
