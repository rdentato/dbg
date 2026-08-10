#!/bin/sh

DBGLOG=${DBGLOG:-../src/dbglog}
CC=${CC:-cc}

fail() {
  echo "t_dbg_h: $*" >&2
  exit 1
}

contains() {
  case "$1" in
    *"$2"*) : ;;
    *) fail "expected output to contain: $2\n--- output ---\n$1" ;;
  esac
}

not_contains() {
  case "$1" in
    *"$2"*) fail "expected output not to contain: $2\n--- output ---\n$1" ;;
    *) : ;;
  esac
}

src=t_dbg_h_probe.$$.c
exe=t_dbg_h_probe.$$
log=t_dbg_h_probe.$$.log
trap 'rm -f "$src" "$exe" "$log"' EXIT HUP INT TERM

cat > "$src" <<'SRC'
#include "dbg.h"

int main(void) {
  char *p = strndup("ab", 100);
  if (!p) return 1;
  dbgptr((void *)((uintptr_t)p + 50));
  free(p);
  return 0;
}
SRC

$CC -Wall -pedantic -DDEBUG_ALLOC -I../src -o "$exe" "$src" || fail "compile DEBUG_ALLOC strndup probe failed"
./"$exe" > /dev/null 2> "$log" || fail "run DEBUG_ALLOC strndup probe failed"
out=$("$DBGLOG" < "$log") || fail "dbglog DEBUG_ALLOC strndup probe failed"
contains "$out" 'FAIL : pointer'

cat > "$src" <<'SRC'
#include <stdio.h>
#include "dbg.h"

int main(void) {
  dbgvrb("x") {
    fprintf(stderr, "out\n");
  }
  return 0;
}
SRC

$CC -Wall -pedantic -DDEBUG=DBGLVL_ERROR -I../src -o "$exe" "$src" || fail "compile DBGLVL_ERROR dbgvrb probe failed"
out=$(./"$exe" 2>&1 >/dev/null) || fail "run DBGLVL_ERROR dbgvrb probe failed"
contains "$out" 'out'
not_contains "$out" 'V['
not_contains "$out" 'V]'

$CC -Wall -pedantic -DDEBUG=DBGLVL_TEST -I../src -o "$exe" "$src" || fail "compile DBGLVL_TEST dbgvrb probe failed"
out=$(./"$exe" 2>&1 >/dev/null) || fail "run DBGLVL_TEST dbgvrb probe failed"
contains "$out" 'V['
contains "$out" 'out'
contains "$out" 'V]'

exit 0
