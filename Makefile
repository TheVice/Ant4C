all: ant4c

argument_parser.c: argument_parser.h
argument_parser.c: buffer.h
argument_parser.c: common.h
argument_parser.c: conversion.h
argument_parser.c: math_unit.h
argument_parser.c: property.h
argument_parser.c: range.h
argument_parser.c: string_unit.h

buffer.c: buffer.h

common.c: common.h
common.c: conversion.h
common.c: buffer.h
common.c: range.h
common.c: string_unit.h

conversion.c: conversion.h
conversion.c: buffer.h
conversion.c: common.h
conversion.c: range.h

date_time.c: date_time.h
date_time.c: buffer.h
date_time.c: common.h
date_time.c: conversion.h
date_time.c: math_unit.h
date_time.c: range.h

echo.c: echo.h
echo.c: buffer.h
echo.c: common.h
echo.c: conversion.h
echo.c: file_system.h
echo.c: range.h
echo.c: string_unit.h

environment.c: environment.h
environment.c: buffer.h
environment.c: common.h
environment.c: conversion.h
environment.c: operating_system.h
environment.c: range.h

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
exec.c: xml.h

file_system.c: file_system.h
file_system.c: buffer.h
file_system.c: common.h
file_system.c: conversion.h
file_system.c: date_time.h
file_system.c: path.h
file_system.c: project.h
file_system.c: property.h
file_system.c: range.h
file_system.c: string_unit.h

interpreter.c: interpreter.h
interpreter.c: buffer.h
interpreter.c: common.h
interpreter.c: conversion.h
interpreter.c: date_time.h
interpreter.c: echo.h
interpreter.c: environment.h
interpreter.c: exec.h
interpreter.c: file_system.h
interpreter.c: math_unit.h
interpreter.c: operating_system.h
interpreter.c: path.h
interpreter.c: project.h
interpreter.c: property.h
interpreter.c: range.h
interpreter.c: string_unit.h
interpreter.c: target.h
interpreter.c: version.h
interpreter.c: xml.h

main.c: argument_parser.h
main.c: buffer.h
main.c: common.h
main.c: conversion.h
main.c: date_time.h
main.c: echo.h
main.c: environment.h
main.c: exec.h
main.c: file_system.h
main.c: interpreter.h
main.c: math_unit.h
main.c: operating_system.h
main.c: path.h
main.c: project.h
main.c: property.h
main.c: range.h
main.c: string_unit.h
main.c: target.h
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

project.c: project.h
project.c: buffer.h
project.c: common.h
project.c: conversion.h
project.c: file_system.h
project.c: interpreter.h
project.c: path.h
project.c: property.h
project.c: range.h
project.c: string_unit.h
project.c: target.h
project.c: version.h
project.c: xml.h

property.c: property.h
property.c: buffer.h
property.c: common.h
property.c: conversion.h
property.c: file_system.h
property.c: project.h
property.c: range.h
property.c: string_unit.h

range.c: range.h
range.c: buffer.h
range.c: common.h

string_unit.c: string_unit.h
string_unit.c: buffer.h
string_unit.c: common.h
string_unit.c: conversion.h
string_unit.c: range.h

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
target.c: xml.h

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

argument_parser.obj: argument_parser.c
	$(CC) $(CFLAGS) -c argument_parser.c -o $@

buffer.obj: buffer.c
	$(CC) $(CFLAGS) -c buffer.c -o $@

common.obj: common.c
	$(CC) $(CFLAGS) -c common.c -o $@

conversion.obj: conversion.c
	$(CC) $(CFLAGS) -c conversion.c -o $@

date_time.obj: date_time.c
	$(CC) $(CFLAGS) -c date_time.c -o $@

echo.obj: echo.c
	$(CC) $(CFLAGS) -c echo.c -o $@

environment.obj: environment.c
	$(CC) $(CFLAGS) -c environment.c -o $@

exec.obj: exec.c
	$(CC) $(CFLAGS) -c exec.c -o $@

file_system.obj: file_system.c
	$(CC) $(CFLAGS) -c file_system.c -o $@

interpreter.obj: interpreter.c
	$(CC) $(CFLAGS) -c interpreter.c -o $@

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

string_unit.obj: string_unit.c
	$(CC) $(CFLAGS) -c string_unit.c -o $@

target.obj: target.c
	$(CC) $(CFLAGS) -c target.c -o $@

version.obj: version.c
	$(CC) $(CFLAGS) -c version.c -o $@

xml.obj: xml.c
	$(CC) $(CFLAGS) -c xml.c -o $@

libant4c.a: argument_parser.obj
libant4c.a: buffer.obj
libant4c.a: common.obj
libant4c.a: conversion.obj
libant4c.a: date_time.obj
libant4c.a: echo.obj
libant4c.a: environment.obj
libant4c.a: exec.obj
libant4c.a: file_system.obj
libant4c.a: interpreter.obj
libant4c.a: math_unit.obj
libant4c.a: operating_system.obj
libant4c.a: path.obj
libant4c.a: project.obj
libant4c.a: property.obj
libant4c.a: range.obj
libant4c.a: string_unit.obj
libant4c.a: target.obj
libant4c.a: version.obj
libant4c.a: xml.obj
	ar qc $@ argument_parser.obj buffer.obj common.obj conversion.obj date_time.obj echo.obj environment.obj exec.obj file_system.obj interpreter.obj math_unit.obj operating_system.obj path.obj project.obj property.obj range.obj string_unit.obj target.obj version.obj xml.obj

ant4c: libant4c.a
ant4c: main.obj
	$(CC) main.obj -o $@ libant4c.a -lm $(LDCFLAGS)

install: ant4c

clean:
	-rm ant4c libant4c.a main.obj argument_parser.obj buffer.obj common.obj conversion.obj date_time.obj echo.obj environment.obj exec.obj file_system.obj interpreter.obj math_unit.obj operating_system.obj path.obj project.obj property.obj range.obj string_unit.obj target.obj version.obj xml.obj
.PHONY: ant4c clean
