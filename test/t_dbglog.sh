#!/bin/sh

DBGLOG=${DBGLOG:-../src/dbglog}
SEP=$(printf '\017')
EOV=$(printf '\016')

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
out=$(printf 'M:malloc 16 FFF%sreg.c:1\nM?pointer 1000%sreg.c:2\n' "$SEP" "$SEP" | "$DBGLOG" 2>/dev/null) || fail "digit-only hex pointer case failed"
contains "$out" 'PASS : pointer 0x1000'

# Failed realloc keeps the old allocation alive.
out=$(printf 'M:malloc 16 FFF%sreg.c:1\nM:realloc FFF 32 0%sreg.c:2\nM?pointer 1000%sreg.c:3\n' "$SEP" "$SEP" "$SEP" | "$DBGLOG" 2>/dev/null) || fail "realloc failure case failed"
contains "$out" 'PASS : pointer 0x1000'

# realloc(ptr, 0) frees the old allocation in dbglog's model.
out=$(printf 'M:malloc 16 FFF%sreg.c:1\nM:realloc FFF 0 0%sreg.c:2\nM?pointer 1000%sreg.c:3\n' "$SEP" "$SEP" "$SEP" | "$DBGLOG") || fail "realloc zero case failed"
contains "$out" 'FAIL : pointer 0x1000'

# Pointer-range checks must not fail when base + size would overflow.
out=$(printf 'M:malloc 32 FFFFFFFFFFFFFFF0%sreg.c:1\nM?memset FFFFFFFFFFFFFFF8 4%sreg.c:2\n' "$SEP" "$SEP" | "$DBGLOG" 2>/dev/null) || fail "pointer overflow case failed"
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

# Invalid source separators do not truncate the line.
out=$(printf 'hello%sworld no colon\n' "$SEP" | "$DBGLOG") || fail "invalid source separator case failed"
contains "$out" "hello${SEP}world no colon"

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

# dbgtrk scans lines but still processes and emits events inside the block.
out=$(printf 'K["=x"\nM:malloc 16 FFF\nK]\nM?pointer 1000\n' | "$DBGLOG" 2>/dev/null) || fail "dbgtrk inner allocation case failed"
contains "$out" 'PASS : pointer 0x1000'

out=$(printf 'K["=x"\nF:boom%sreg.c:9\nK]\n' "$SEP" | "$DBGLOG" -F) || fail "dbgtrk inner fail case failed"
contains "$out" 'FAIL : boom reg.c:9'

out=$(printf 'K["=needle"\nV[program output\nneedle\nV]%s\nK]\n' "$EOV" | "$DBGLOG") || fail "dbgtrk verbatim scan case failed"
contains "$out" 'PASS : (=needle)'
contains "$out" 'needle'

out=$(printf 'K["=needle"\nV[out\nK] fake close from program output\nneedle\nV]%s\nK]\n' "$EOV" | "$DBGLOG") || fail "dbgtrk verbatim fake close case failed"
contains "$out" 'PASS : (=needle)'
contains "$out" 'K] fake close from program output'
track_closes=$(printf '%s\n' "$out" | grep -c 'TRACK]')
[ "$track_closes" -eq 1 ] || fail "expected one TRACK] line, got $track_closes\n--- output ---\n$out"

# dbgtrk results use the K[ source location, not a stale T[ location.
out=$(printf 'T[t%sa.c:1\nT]\nK["=x"%sb.c:7\nx\nK]\n' "$SEP" "$SEP" | "$DBGLOG") || fail "dbgtrk source location case failed"
contains "$out" 'PASS : (=x) b.c:7'
not_contains "$out" 'PASS : (=x) a.c:1'

# Allocation-table overflow is reported instead of silently disappearing.
out=$(awk 'BEGIN { for (i = 1; i <= 4097; i++) printf "M:malloc 1 %X\n", i * 16 }' | "$DBGLOG" -F 2>&1) || fail "allocation overflow case failed"
contains "$out" 'allocation table full'

# Multiple files keep concatenated-log state.
f1=t_dbglog_1.$$
f2=t_dbglog_2.$$
trap 'rm -f "$f1" "$f2"' EXIT HUP INT TERM
printf 'M:malloc 16 FFF\n' > "$f1"
printf 'M?pointer 1000\n' > "$f2"
out=$("$DBGLOG" "$f1" "$f2" 2>/dev/null) || fail "multi-file state case failed"
contains "$out" 'PASS : pointer 0x1000'

# Verbatim close is recognized only inside a verbatim block and only at line start.
out=$(printf 'noise V]%s noise\n' "$EOV" | "$DBGLOG") || fail "verbatim close outside block case failed"
contains "$out" "noise V]${EOV} noise"
not_contains "$out" 'VERB ]'

out=$(printf 'V[block\nbody\nV]%s\n' "$EOV" | "$DBGLOG") || fail "verbatim normal block case failed"
contains "$out" 'VERB [ block'
contains "$out" 'body'
contains "$out" 'VERB ]'

# Untracked implicit memory-operation destinations are skipped, not failed.
out=$(printf 'M?strcpy ABC DEF 3\n' | "$DBGLOG") || fail "untracked strcpy skip case failed"
contains "$out" 'SKIP : strcpy 0xABC +3 (untracked)'

out=$(printf 'M?strcpy ABC DEF 3\n' | "$DBGLOG" -F) || fail "untracked strcpy -F case failed"
not_contains "$out" 'SKIP'
not_contains "$out" 'FAIL'

out=$(printf 'M:malloc 2 ABC\nM?strcpy ABC DEF 3\n' | "$DBGLOG" 2>/dev/null) || fail "tracked strcpy overrun case failed"
contains "$out" 'FAIL : strcpy 0xABC +3'

# Explicit pointer checks stay strict.
out=$(printf 'M?pointer ABC\n' | "$DBGLOG") || fail "strict pointer case failed"
contains "$out" 'FAIL : pointer 0xABC'

# Double free is reported; free(NULL) stays silent.
out=$(printf 'M:malloc 16 FFF\nM:free FFF\nM:free FFF\n' | "$DBGLOG") || fail "double free case failed"
contains "$out" 'FAIL : free 0xFFF (not allocated or already freed)'

out=$(printf 'M:free 0\n' | "$DBGLOG") || fail "free null case failed"
not_contains "$out" 'FAIL : free'

# Leaks are reported in the log output, not on stderr, without changing exit status.
out=$(printf 'M:malloc 16 FFF\n' | "$DBGLOG") || fail "leak report case failed"
contains "$out" 'LEAK : 1 allocation(s) never freed'
err=$( (printf 'M:malloc 16 FFF\n' | "$DBGLOG" >/dev/null) 2>&1 )
not_contains "$err" 'never freed'

# The leak line is a failure-class finding, so -F keeps it visible.
out=$(printf 'M:malloc 16 FFF\n' | "$DBGLOG" -F) || fail "leak report -F case failed"
contains "$out" 'LEAK : 1 allocation(s) never freed'

# Source overreads are checked when the source allocation is known.
out=$(printf 'M:malloc 4 A00\nM:malloc 64 B00\nM?memcpy B00 A00 16\n' | "$DBGLOG" 2>/dev/null) || fail "source overread case failed"
contains "$out" 'FAIL : memcpy source 0xA00 +16'

out=$(printf 'M:malloc 64 B00\nM?memcpy B00 A00 16\n' | "$DBGLOG" 2>/dev/null) || fail "untracked source case failed"
contains "$out" 'PASS : memcpy 0xB00 +16'

# Maximal final line at EOF is valid; maximal line with newline is valid; longer lines still fail.
out=$(awk 'BEGIN { for (i = 0; i < 8191; i++) printf "x" }' | "$DBGLOG") || fail "maximal final line case failed"
contains "$out" 'xxxxxxxxxxxxxxxx'

out=$( (awk 'BEGIN { for (i = 0; i < 8191; i++) printf "y"; printf "\n" }' | "$DBGLOG") 2>&1 ) || fail "maximal newline line case failed"
contains "$out" 'yyyyyyyyyyyyyyyy'
not_contains "$out" 'input line exceeds'

# Unterminated T[ is reported like other unterminated blocks.
out=$( (printf 'T[unterminated\n' | "$DBGLOG" >/dev/null) 2>&1 )
status=$?
[ "$status" -ne 0 ] || fail "unterminated test case unexpectedly passed"
contains "$out" 'unterminated test block'

# -- ends options and - reads stdin.
printf 'I:dash stdin\n' > "$f1"
out=$("$DBGLOG" -- - < "$f1") || fail "dash stdin case failed"
contains "$out" 'INFO : dash stdin'

# Unterminated blocks are reported at end of input.
out=$( (printf 'V[unterminated\nbody\n' | "$DBGLOG" >/dev/null) 2>&1 )
status=$?
[ "$status" -ne 0 ] || fail "unterminated verbatim case unexpectedly passed"
contains "$out" 'unterminated verbatim block'

# Unterminated blocks also leave a FAIL line in the report itself.
out=$(printf 'V[unterminated\nbody\n' | "$DBGLOG" 2>/dev/null)
contains "$out" 'FAIL : unterminated verbatim block at end of input'

out=$(printf 'T[unterminated\n' | "$DBGLOG" 2>/dev/null)
contains "$out" 'FAIL : unterminated test block at end of input'

out=$(printf 'K["=x"\nnothing\n' | "$DBGLOG" 2>/dev/null)
contains "$out" 'FAIL : unterminated dbgtrk block at end of input'
contains "$out" 'FAIL : (=x)'

exit 0
