all: ant4c

argument_parser.c: argument_parser.h
argument_parser.c: buffer.h
argument_parser.c: common.h
argument_parser.c: conversion.h
argument_parser.c: load_file.h
argument_parser.c: math_unit.h
argument_parser.c: property.h
argument_parser.c: range.h
argument_parser.c: string_unit.h
argument_parser.c: text_encoding.h

buffer.c: buffer.h

common.c: common.h
common.c: conversion.h
common.c: buffer.h
common.c: range.h
common.c: string_unit.h

choose_task.c: choose_task.h

conversion.c: conversion.h
conversion.c: buffer.h
conversion.c: common.h
conversion.c: range.h

copy_move.c: copy_move.h
copy_move.c: buffer.h
copy_move.c: common.h
copy_move.c: conversion.h
copy_move.c: file_system.h
copy_move.c: load_file.h
copy_move.c: path.h
copy_move.c: project.h
copy_move.c: range.h
copy_move.c: string_unit.h
copy_move.c: text_encoding.h

date_time.c: date_time.h
date_time.c: buffer.h
date_time.c: common.h
date_time.c: conversion.h
date_time.c: math_unit.h
date_time.c: range.h

default_listener.c: default_listener.h

echo.c: echo.h
echo.c: buffer.h
echo.c: common.h
echo.c: conversion.h
echo.c: file_system.h
echo.c: range.h
echo.c: string_unit.h
echo.c: text_encoding.h

environment.c: environment.h
environment.c: buffer.h
environment.c: common.h
environment.c: conversion.h
environment.c: operating_system.h
environment.c: range.h
environment.c: text_encoding.h

exec.c: exec.h
exec.c: argument_parser.h
exec.c: buffer.h
exec.c: common.h
exec.c: conversion.h
exec.c: date_time.h
exec.c: echo.h
exec.c: file_system.h
exec.c: path.h
exec.c: project.h
exec.c: property.h
exec.c: range.h
exec.c: string_unit.h
exec.c: text_encoding.h
exec.c: xml.h

fail_task.c: fail_task.h

file_system.c: file_system.h
file_system.c: buffer.h
file_system.c: common.h
file_system.c: conversion.h
file_system.c: date_time.h
file_system.c: hash.h
file_system.c: path.h
file_system.c: project.h
file_system.c: property.h
file_system.c: range.h
file_system.c: string_unit.h
file_system.c: text_encoding.h

for_each.c: for_each.h
for_each.c: buffer.h
for_each.c: common.h
for_each.c: file_system.h
for_each.c: interpreter.h
for_each.c: path.h
for_each.c: project.h
for_each.c: property.h
for_each.c: range.h
for_each.c: string_unit.h
for_each.c: xml.h

hash.blake2.c: hash.h
hash.blake2.c: buffer.h
hash.blake2.c: range.h

hash.blake3.c: hash.h
hash.blake3.c: buffer.h

hash.c: hash.h
hash.c: buffer.h
hash.c: common.h
hash.c: file_system.h
hash.c: range.h

hash.crc32.c: hash.h
hash.crc32.c: common.h

hash.sha3.c: hash.h
hash.sha3.c: buffer.h

if_task.c: if_task.h

interpreter.c: interpreter.h
interpreter.c: buffer.h
interpreter.c: common.h
interpreter.c: conversion.h
interpreter.c: copy_move.h
interpreter.c: date_time.h
interpreter.c: echo.h
interpreter.c: environment.h
interpreter.c: exec.h
interpreter.c: file_system.h
interpreter.c: for_each.h
interpreter.c: hash.h
interpreter.c: load_file.h
interpreter.c: load_tasks.h
interpreter.c: math_unit.h
interpreter.c: operating_system.h
interpreter.c: path.h
interpreter.c: project.h
interpreter.c: property.h
interpreter.c: range.h
interpreter.c: sleep_unit.h
interpreter.c: string_unit.h
interpreter.c: target.h
interpreter.c: task.h
interpreter.c: text_encoding.h
interpreter.c: try_catch.h
interpreter.c: version.h
interpreter.c: xml.h

listener.c: listener.h

load_file.c: load_file.h
load_file.c: buffer.h
load_file.c: common.h
load_file.c: file_system.h
load_file.c: project.h
load_file.c: property.h
load_file.c: range.h
load_file.c: string_unit.h
load_file.c: text_encoding.h

load_tasks.c: load_tasks.h

main.c: argument_parser.h
main.c: buffer.h
main.c: common.h
main.c: conversion.h
main.c: date_time.h
main.c: echo.h
main.c: environment.h
main.c: exec.h
main.c: file_system.h
main.c: hash.h
main.c: interpreter.h
main.c: load_file.h
main.c: load_tasks.h
main.c: math_unit.h
main.c: operating_system.h
main.c: path.h
main.c: project.h
main.c: property.h
main.c: range.h
main.c: sleep_unit.h
main.c: string_unit.h
main.c: target.h
main.c: text_encoding.h
main.c: version.h
main.c: xml.h

math_unit.c: math_unit.h
math_unit.c: buffer.h
math_unit.c: common.h
math_unit.c: conversion.h
math_unit.c: range.h
math_unit.c: string_unit.h

operating_system.c: operating_system.h
operating_system.c: buffer.h
operating_system.c: common.h
operating_system.c: conversion.h
operating_system.c: environment.h
operating_system.c: range.h
operating_system.c: string_unit.h

operating_system.h: version.h

path.c: path.h
path.c: buffer.h
path.c: common.h
path.c: conversion.h
path.c: environment.h
path.c: file_system.h
path.c: project.h
path.c: range.h
path.c: string_unit.h
path.c: text_encoding.h

project.c: project.h
project.c: buffer.h
project.c: common.h
project.c: conversion.h
project.c: file_system.h
project.c: interpreter.h
project.c: load_file.h
project.c: load_tasks.h
project.c: path.h
project.c: property.h
project.c: range.h
project.c: string_unit.h
project.c: target.h
project.c: text_encoding.h
project.c: version.h
project.c: xml.h

property.c: property.h
property.c: buffer.h
property.c: common.h
property.c: conversion.h
property.c: file_system.h
property.c: load_file.h
property.c: project.h
property.c: range.h
property.c: string_unit.h
property.c: text_encoding.h

range.c: range.h
range.c: buffer.h
range.c: common.h

shared_object.c: shared_object.h

sleep_unit.c: sleep_unit.h
sleep_unit.c: buffer.h
sleep_unit.c: common.h
sleep_unit.c: conversion.h
sleep_unit.c: date_time.h

string_unit.c: string_unit.h
string_unit.c: buffer.h
string_unit.c: common.h
string_unit.c: conversion.h
string_unit.c: range.h
string_unit.c: text_encoding.h

target.c: target.h
target.c: buffer.h
target.c: common.h
target.c: conversion.h
target.c: echo.h
target.c: interpreter.h
target.c: project.h
target.c: property.h
target.c: range.h
target.c: string_unit.h
target.c: text_encoding.h
target.c: xml.h

task.c: task.h
task.c: common.h
task.c: conversion.h
task.c: interpreter.h
task.c: project.h
task.c: range.h

text_encoding.c: text_encoding.h
text_encoding.c: buffer.h
text_encoding.c: common.h

try_catch.c: try_catch.h
try_catch.c: buffer.h
try_catch.c: common.h
try_catch.c: file_system.h
try_catch.c: interpreter.h
try_catch.c: path.h
try_catch.c: project.h
try_catch.c: property.h
try_catch.c: range.h
try_catch.c: xml.h

version.c: version.h
version.c: buffer.h
version.c: common.h
version.c: conversion.h
version.c: range.h

xml.c: xml.h
xml.c: buffer.h
xml.c: common.h
xml.c: range.h
xml.c: string_unit.h
xml.c: text_encoding.h

argument_parser.obj: argument_parser.c
	$(CC) $(CFLAGS) -c argument_parser.c -o $@

buffer.obj: buffer.c
	$(CC) $(CFLAGS) -c buffer.c -o $@

choose_task.obj: choose_task.c
	$(CC) $(CFLAGS) -c choose_task.c -o $@

common.obj: common.c
	$(CC) $(CFLAGS) -c common.c -o $@

conversion.obj: conversion.c
	$(CC) $(CFLAGS) -c conversion.c -o $@

copy_move.obj: copy_move.c
	$(CC) $(CFLAGS) -c copy_move.c -o $@

date_time.obj: date_time.c
	$(CC) $(CFLAGS) -c date_time.c -o $@

default_listener.obj: default_listener.c
	${CC} ${CFLAGS} -c -fPIC default_listener.c -o $@

echo.obj: echo.c
	$(CC) $(CFLAGS) -c echo.c -o $@

environment.obj: environment.c
	$(CC) $(CFLAGS) -c environment.c -o $@

exec.obj: exec.c
	$(CC) $(CFLAGS) -c exec.c -o $@

fail_task.obj: fail_task.c
	$(CC) $(CFLAGS) -c fail_task.c -o $@

file_system.obj: file_system.c
	$(CC) $(CFLAGS) -c file_system.c -o $@

for_each.obj: for_each.c
	$(CC) $(CFLAGS) -c for_each.c -o $@

hash.blake2.obj: hash.blake2.c
	$(CC) $(CFLAGS) -c hash.blake2.c -o $@

hash.blake3.obj: hash.blake3.c
	$(CC) $(CFLAGS) -c hash.blake3.c -o $@

hash.obj: hash.c
	$(CC) $(CFLAGS) -c hash.c -o $@

hash.crc32.obj: hash.crc32.c
	$(CC) $(CFLAGS) -c hash.crc32.c -o $@

hash.sha3.obj: hash.sha3.c
	$(CC) $(CFLAGS) -c hash.sha3.c -o $@

if_task.obj: if_task.c
	$(CC) $(CFLAGS) -c if_task.c -o $@

interpreter.obj: interpreter.c
	$(CC) $(CFLAGS) -c interpreter.c -o $@

listener.obj: listener.c
	$(CC) $(CFLAGS) -c listener.c -o $@

load_file.obj: load_file.c
	$(CC) $(CFLAGS) -c load_file.c -o $@

load_tasks.obj: load_tasks.c
	$(CC) $(CFLAGS) -c load_tasks.c -o $@

main.obj: main.c
	$(CC) $(CFLAGS) -c main.c -o $@

math_unit.obj: math_unit.c
	$(CC) $(CFLAGS) -c math_unit.c -o $@

operating_system.obj: operating_system.c
	$(CC) $(CFLAGS) -c operating_system.c -o $@

path.obj: path.c
	$(CC) $(CFLAGS) -c path.c -o $@

project.obj: project.c
	$(CC) $(CFLAGS) -c project.c -o $@

property.obj: property.c
	$(CC) $(CFLAGS) -c property.c -o $@

range.obj: range.c
	$(CC) $(CFLAGS) -c range.c -o $@

shared_object.obj: shared_object.c
	$(CC) $(CFLAGS) -c shared_object.c -o $@

sleep_unit.obj: sleep_unit.c
	$(CC) $(CFLAGS) -c sleep_unit.c -o $@

string_unit.obj: string_unit.c
	$(CC) $(CFLAGS) -c string_unit.c -o $@

target.obj: target.c
	$(CC) $(CFLAGS) -c target.c -o $@

task.obj: task.c
	$(CC) $(CFLAGS) -c task.c -o $@

text_encoding.obj: text_encoding.c
	$(CC) $(CFLAGS) -c text_encoding.c -o $@

try_catch.obj: try_catch.c
	$(CC) $(CFLAGS) -c try_catch.c -o $@

version.obj: version.c
	$(CC) $(CFLAGS) -c version.c -o $@

xml.obj: xml.c
	$(CC) $(CFLAGS) -c xml.c -o $@

libant4c.a: argument_parser.obj
libant4c.a: buffer.obj
libant4c.a: choose_task.obj
libant4c.a: common.obj
libant4c.a: conversion.obj
libant4c.a: copy_move.obj
libant4c.a: date_time.obj
libant4c.a: echo.obj
libant4c.a: environment.obj
libant4c.a: exec.obj
libant4c.a: fail_task.obj
libant4c.a: file_system.obj
libant4c.a: for_each.obj
libant4c.a: hash.blake2.obj
libant4c.a: hash.blake3.obj
libant4c.a: hash.crc32.obj
libant4c.a: hash.obj
libant4c.a: hash.sha3.obj
libant4c.a: if_task.obj
libant4c.a: interpreter.obj
libant4c.a: listener.obj
libant4c.a: load_file.obj
libant4c.a: load_tasks.obj
libant4c.a: math_unit.obj
libant4c.a: operating_system.obj
libant4c.a: path.obj
libant4c.a: project.obj
libant4c.a: property.obj
libant4c.a: range.obj
libant4c.a: shared_object.obj
libant4c.a: sleep_unit.obj
libant4c.a: string_unit.obj
libant4c.a: target.obj
libant4c.a: task.obj
libant4c.a: text_encoding.obj
libant4c.a: try_catch.obj
libant4c.a: version.obj
libant4c.a: xml.obj
	ar qc $@ argument_parser.obj buffer.obj choose_task.obj common.obj conversion.obj copy_move.obj date_time.obj echo.obj environment.obj exec.obj fail_task.obj file_system.obj for_each.obj hash.blake2.obj hash.blake3.obj hash.obj hash.crc32.obj hash.sha3.obj if_task.obj interpreter.obj listener.obj load_file.obj load_tasks.obj math_unit.obj operating_system.obj path.obj project.obj property.obj range.obj shared_object.obj sleep_unit.obj string_unit.obj target.obj task.obj text_encoding.obj try_catch.obj version.obj xml.obj

ant4c: libant4c.a
ant4c: main.obj
ant4c: libdefault_listener.so
	$(CC) main.obj -o $@ libant4c.a -lm $(LDCFLAGS)

libdefault_listener.so: default_listener.obj
	$(CC) -shared -o $@ default_listener.obj

install: ant4c

clean:
	-rm ant4c libant4c.a main.obj libdefault_listener.so argument_parser.obj buffer.obj choose_task.obj common.obj conversion.obj copy_move.obj date_time.obj default_listener.obj echo.obj environment.obj exec.obj fail_task.obj file_system.obj for_each.obj hash.blake2.obj hash.blake3.obj hash.obj hash.crc32.obj hash.sha3.obj if_task.obj interpreter.obj listener.obj load_file.obj load_tasks.obj math_unit.obj operating_system.obj path.obj project.obj property.obj range.obj shared_object.obj sleep_unit.obj string_unit.obj target.obj task.obj text_encoding.obj try_catch.obj version.obj xml.obj
.PHONY: ant4c clean
