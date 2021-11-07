## 
##  (C) by Remo Dentato (rdentato@gmail.com)
##
## This software is distributed under the terms of the MIT license:
##  https://opensource.org/licenses/MIT
##

# Handle msys adding .exe to the program
_EXE=.exe

ifeq "$(COMSPEC)" ""
_EXE=
endif

# Force gcc as compiler
CC=gcc 
#CC=clang

# Comment the following if you want to use fgets based REPL
LINENOISE=-DUSE_LINENOISE

# Leave uncommented the desired level of debug
#DBG=-DNDEBUG
#DBG=-DDEBUG=DEBUG_ERROR
#DBG=-DDEBUG=DEBUG_WARN 
#DBG=-DDEBUG=DEBUG_INFO 
DBG=-DDEBUG=DEBUG_TEST 
#DBG=-DDEBUG=DEBUG_MEM  

# Optimization level (regular, gdb, valgrind)
OPT=-O2
#OPT=-g
#OPT=-ggdb3

# Uncomment  gdb read executable
CFLAGS= $(OPT) -Wall $(LINENOISE) $(DBG)

OBJS=libs.o dict.o eval.o repl.o gerku.o

LIBS=$(wildcard libs/*.h)

gerku$(_EXE): $(OBJS)
	$(CC) -o gerku$(_EXE) $(OBJS)

libs.o: $(LIBS)
	$(CC) $(CFLAGS) -c -o libs.o libs.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f gerku$(_EXE) *.o

