all: main

buffer.obj: buffer.c buffer.h
	cc -c buffer.c -o buffer.obj

common.obj: common.c common.h
	cc -c common.c -o common.obj

conversion.obj: conversion.c conversion.h
	cc -c conversion.c -o conversion.obj

file_system.obj: file_system.c file_system.h
	cc -c file_system.c -o file_system.obj

hash.crc32.obj: hash.crc32.c hash.h
	cc -c hash.crc32.c -o hash.crc32.obj

hash.blake2.obj: hash.blake2.c hash.h
	cc -c hash.blake2.c -o hash.blake2.obj

hash.blake3.obj: hash.blake3.c hash.h
	cc -c hash.blake3.c -o hash.blake3.obj

hash.sha3.obj: hash.sha3.c hash.h
	cc -c hash.sha3.c -o hash.sha3.obj

path.obj: path.c path.h
	cc -c path.c -o path.obj

range.obj: range.c range.h
	cc -c range.c -o range.obj

hash.obj: hash.c hash.h
	cc -c hash.c -o hash.obj

hash.a: buffer.obj common.obj conversion.obj file_system.obj hash.crc32.obj hash.blake2.obj hash.blake3.obj hash.sha3.obj path.obj range.obj hash.obj
	ar rcs hash.a buffer.obj common.obj conversion.obj file_system.obj hash.crc32.obj hash.blake2.obj hash.blake3.obj hash.sha3.obj path.obj range.obj hash.obj

main.obj: main.c
	cc -c main.c -o main.obj

main: hash.a main.obj
	cc main.obj hash.a -o main

clean:
	rm buffer.obj common.obj conversion.obj file_system.obj hash.obj hash.crc32.obj hash.blake2.obj hash.blake3.obj hash.sha3.obj hash.a main.obj path.obj range.obj main
.PHONY: clean
