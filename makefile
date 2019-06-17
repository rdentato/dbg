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

#CC=
AR=ar -ru
RM=rm -f
CP=cp 
MD=mkdir -p
#-static -static-libgcc 
CFLAGS= -DDEBUG -std=c99 -Wall

all: dbgstat$(_EXE) dbgtest$(_EXE)

dbgstat$(_EXE): dbg.h
	$(CP) dbg.h dbgstat.c
	$(CC) $(CFLAGS) -DDBGSTAT -O2 -o dbgstat dbgstat.c
	$(RM) dbgstat.c

dbgtest$(_EXE): dbg.h
	$(CP) dbg.h dbgtest.c
	$(CC) $(CFLAGS) -DDBGTEST -O0 -o dbgtest dbgtest.c
	$(RM) dbgtest.c

runtest: dbgtest$(_EXE) dbgstat$(_EXE)
	@echo "FAIL: 3 expected on next line"
	@./dbgtest 2> dbgtest.log
	@./dbgstat < dbgtest.log | grep "FAIL: 3" 

clean:
	$(RM) dbgtest$(_EXE) dbgstat$(_EXE)
	$(RM) dbgtest.c dbgstat.c
	$(RM) *.log

dbg_mini.h: dbg.h
	cat dbg.h | \
	tr -d '\r' | \
	while IFS='' read  -r a && [ "$$a" !=  '/*<<<>>>*/' ]; do \
	  echo "$$a" ; \
	done > dbg_mini.h
