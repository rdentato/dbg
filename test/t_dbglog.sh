#!/bin/sh

DBGLOG=${DBGLOG:-../src/dbglog}
SEP=$(printf '\017')

fail() {
  echo "t_dbglog: $*" >&2
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

# Digit-only pointer tokens are hex, not decimal.
out=$(printf 'M:malloc 16 FFF%sreg.c:1\nM?pointer 1000%sreg.c:2\n' "$SEP" "$SEP" | "$DBGLOG") || fail "digit-only hex pointer case failed"
contains "$out" 'PASS : pointer 0x1000'

# Failed realloc keeps the old allocation alive.
out=$(printf 'M:malloc 16 FFF%sreg.c:1\nM:realloc FFF 32 0%sreg.c:2\nM?pointer 1000%sreg.c:3\n' "$SEP" "$SEP" "$SEP" | "$DBGLOG") || fail "realloc failure case failed"
contains "$out" 'PASS : pointer 0x1000'

# realloc(ptr, 0) frees the old allocation in dbglog's model.
out=$(printf 'M:malloc 16 FFF%sreg.c:1\nM:realloc FFF 0 0%sreg.c:2\nM?pointer 1000%sreg.c:3\n' "$SEP" "$SEP" "$SEP" | "$DBGLOG") || fail "realloc zero case failed"
contains "$out" 'FAIL : pointer 0x1000'

# Pointer-range checks must not fail when base + size would overflow.
out=$(printf 'M:malloc 32 FFFFFFFFFFFFFFF0%sreg.c:1\nM?memset FFFFFFFFFFFFFFF8 4%sreg.c:2\n' "$SEP" "$SEP" | "$DBGLOG") || fail "pointer overflow case failed"
contains "$out" 'PASS : memset 0xFFFFFFFFFFFFFFF8 +4'

# Malformed and unknown M? records fail explicitly.
out=$(printf 'M?memset 1000%sreg.c:1\n' "$SEP" | "$DBGLOG") || fail "malformed M?memset case failed"
contains "$out" 'FAIL: malformed M?memset record'

out=$(printf 'M?bogus 1000 2%sreg.c:1\n' "$SEP" | "$DBGLOG") || fail "unknown M? case failed"
contains "$out" 'FAIL: unknown M? operation: bogus'

# Plain fallback text that starts with an event letter is not rewritten.
out=$(printf 'Wplain line\n' | "$DBGLOG") || fail "fallback event-like line case failed"
contains "$out" 'Wplain line'
not_contains "$out" 'WARN p'

# No-source M? records must not grow a fake :0 source location.
out=$(printf 'M?pointer 0\n' | "$DBGLOG") || fail "no-source M? case failed"
not_contains "$out" ':0'
contains "$out" 'PASS : pointer 0x0'

# -F applies to HTML mode too.
out=$(printf 'P:ok%sreg.c:1\nF:bad%sreg.c:2\n' "$SEP" "$SEP" | "$DBGLOG" -F -H) || fail "-F -H case failed"
contains "$out" 'FAIL : bad reg.c:2'
not_contains "$out" 'PASS : ok'

# Overlong lines are reported and make dbglog fail.
out=$( (awk 'BEGIN { for (i = 0; i < 9000; i++) printf "x"; printf "\n" }' | "$DBGLOG" >/dev/null) 2>&1 )
status=$?
[ "$status" -ne 0 ] || fail "overlong line case unexpectedly passed"
contains "$out" 'input line exceeds'

# dbgtrk expectation overflow inserts one FAIL line and ignores excess strings.
out=$(printf 'K["=e0", "=e1", "=e2", "=e3", "=e4", "=e5", "=e6", "=e7", "=e8"\nI:e0 e1 e2 e3 e4 e5 e6 e7 e8\nK]\n' | "$DBGLOG" -F) || fail "dbgtrk overflow case failed"
contains "$out" 'FAIL : (too many dbgtrk expectations; ignored 1 after max 8)'
not_contains "$out" '(=e8)'

# Allocation-table overflow is reported instead of silently disappearing.
out=$(awk 'BEGIN { for (i = 1; i <= 4097; i++) printf "M:malloc 1 %X\n", i * 16 }' | "$DBGLOG" -F 2>&1) || fail "allocation overflow case failed"
contains "$out" 'allocation table full'

# Multiple files keep concatenated-log state.
f1=t_dbglog_1.$$
f2=t_dbglog_2.$$
trap 'rm -f "$f1" "$f2"' EXIT HUP INT TERM
printf 'M:malloc 16 FFF\n' > "$f1"
printf 'M?pointer 1000\n' > "$f2"
out=$("$DBGLOG" "$f1" "$f2") || fail "multi-file state case failed"
contains "$out" 'PASS : pointer 0x1000'

# Unterminated blocks are reported at end of input.
out=$( (printf 'V[unterminated\nbody\n' | "$DBGLOG" >/dev/null) 2>&1 )
status=$?
[ "$status" -ne 0 ] || fail "unterminated verbatim case unexpectedly passed"
contains "$out" 'unterminated verbatim block'

exit 0
