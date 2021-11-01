## 
##  (C) by Remo Dentato (rdentato@gmail.com)
##
## This software is distributed under the terms of the MIT license:
##  https://opensource.org/licenses/MIT
##

_EXE=.exe

ifeq "$(COMSPEC)" ""
_EXE=
endif

CC=gcc
CFLAGS=-O2 -Wall  -DUSE_LINENOISE
#CFLAGS=-g -Wall  -DUSE_LINENOISE

gerku$(_EXE): libs.o gerku.o
	gcc -o gerku$(_EXE) gerku.o libs.o

libs.o: libs.c

clean:
	rm -f gerku$(_EXE) *.o
