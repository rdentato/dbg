#  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
#  SPDX-License-Identifier: MIT

SRC=../src

_EXE=.exe
STATIC=-static
SRCDIR=../src

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

CFLAGS=-O2 -Wall -DDEBUG=DBGLVL_TEST -I $(SRC) -I. $(STATIC)  -pedantic
LIBS=

TESTS_SRC_C=$(wildcard t_*.c)
TESTS_C=$(TESTS_SRC_C:.c=)
TESTS_SH=$(wildcard t_*.sh)

TESTS_SRC=$(TST_SRC_C)
TESTS=$(TESTS_C:%=%$(_EXE))
DBGLOG=../src/dbglog$(_EXE)

# targets
all: $(TESTS)

%$(_EXE): %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

runtest: all $(DBGLOG)
	@rm -f test.log
	@set -e; for test in $(TESTS); do \
		./$$test 2>>test.log; \
	done
	@set -e; for test in $(TESTS_SH); do \
		DBGLOG=$(DBGLOG) sh ./$$test; \
	done

$(DBGLOG): ../src/dbglog.c
	$(MAKE) -C ../src all

.PRECIOUS: %.o


clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) test.log
