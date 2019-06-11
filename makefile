#   (C) by Remo Dentato (rdentato@gmail.com)
#  
#  This software is distributed under the terms of the MIT license:
#   https://opensource.org/licenses/MIT
#      
#  Makefile for Gcc tested on Linux and Msys
#

_EXE=.exe

ifeq "$(COMSPEC)" ""
_EXE=
endif

#CC=gcc
AR=ar -ru
RM=rm -f
CP=cp 
MD=mkdir -p

all: dbgstat dbgtest 

dbgstat$(_EXE): dbg.h
	$(CP) dbg.h dbgstat.c
	$(CC) -DDBGSTAT -O2 -std=c99 -Wall -o dbgstat dbgstat.c
	$(RM) dbgstat.c

dbgtest$(_EXE): dbg.h
	$(CP) dbg.h dbgtest.c
	$(CC) -DDEBUG -DDBGTEST -std=c99 -O2 -Wall -o dbgtest dbgtest.c
	$(RM) dbgtest.c

runtest: dbgtest$(_EXE) dbgstat$(_EXE)
	./dbgtest 2> dbgtest.log
	-./dbgstat < dbgtest.log

clean:
	$(RM) dbgtest$(_EXE) dbgstat$(_EXE)
	$(RM) *.log

dbg_mini.h: dbg.h
	cat dbg.h | \
	tr -d '\r' | \
	while IFS='' read  -r a && [ "$$a" !=  '/*<<<>>>*/' ]; do \
	  echo "$$a" ; \
	done > dbg_mini.h
