#  SPDX-FileCopyrightText: © 2026 Remo Dentato <rdentato@gmail.com>
#  SPDX-License-Identifier: MIT

_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

CFLAGS=-O2 -Wall $(STATIC)
LIBS=

TOOLS=dbglog$(_EXE)

all: $(TOOLS)

dbglog$(_EXE): dbglog.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

.PRECIOUS: %.o

clean:
	rm -f $(TOOLS) dbglog.exe dbglog.o
