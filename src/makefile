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

# Comment the following if you want to use fgets based REPL
LINENOISE=-DUSE_LINENOISE

# Uncomment the second line to get gdb read executable
CFLAGS=-O2 -Wall $(LINENOISE)
#CFLAGS=-g -Wall $(LINENOISE)

OBJS=libs.o dict.o eval.o gerku.o

gerku$(_EXE): $(OBJS)
	$(CC) -o gerku$(_EXE) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f gerku$(_EXE) *.o
