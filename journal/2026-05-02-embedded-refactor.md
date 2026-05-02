# 2026-05-02

## Worked on

- Re-read `dbg.h`; compared working tree vs HEAD to identify all user-applied event-code changes
- Documented old→new code correspondence (15 mappings). Key detail: `dbgvrb` close is `V]\x0E` — the `\x0E` byte is part of the closing marker, unique to verbose blocks
- Updated `PLAN.md` to reflect current state

## Completed

- Plan restructured into Phase 1 (done) and Phase 2 (not started)
- Event-code register finalized and locked — uppercase only, `[`/`]` for block delimiters, no lowercase close-codes
- `dbgnow` confirmed fully removed; `N` code struck from plan
- Milestones renumbered (11→10)

## Completed

- Plan restructured into Phase 1 (done) and Phase 2 (on hold)
- Event-code register finalized and locked
- `dbgnow` confirmed fully removed
- dbglog rewritten from scratch (~500 lines):
  - Text mode: direct fprintf (no unnecessary buffering)
  - -F mode: failures only
  - -H mode: HTML with CSS classes (buffered)
  - M? validation: tracks allocations, validates bounds, reports PASS/FAIL
  - dbgtrk: parses =/! expectations, scans output, reports PASS/FAIL
  - Clean compile: -Wall -Wextra -pedantic -Wconversion, zero warnings

## Pending

- Phase 2: on hold
- Tests: update test suite for new event codes and dbglog

## Decisions

- Current event codes are final; no lowercase close-codes
- Check failure detail uses `F=` prefix (no separate `D` code)
- `dbgnow` won't be re-implemented in this pass
- Arduino-specific functions deferred (on hold)
- `V` stands for verbatim (program output), not verbose
- dbglog spec finalised: `-F`/`-H` flags; track `M:` allocations for `M?` validation; verbatim blocks pass through; no FILE/RSLT statistics

## Notes

- Working tree unstaged changes vs HEAD:
  - `src/dbg.h` — Phase 1 event codes, dbg_millis, simplified dbgclk, M: and M? format changes
  - `src/dbglog.c` — complete rewrite (~700 lines)
  - `test/t_alloc_fail.c` — new test with 9 intentional M? failures

- dbglog features:
  - Single unified `process_line` for text and HTML
  - Event codes expanded to uniform 6-char markers
  - M? validation with allocation tracking
  - dbgtrk with expectation scanning, source from T[
  - -F (failures only), -H (HTML), default (full text)
  - Zero warnings with `-O2 -Wall` (make dbglog)

## Checkpoint

Session end. All Phase 1 work committed and pushed (2 commits: c8014af, 1c94534).

Delivered:
- Single-byte event codes in dbg.h
- dbglog.c rewritten from scratch (unified text/HTML, M? tracking, trk eval)
- New test t_alloc_fail.c (9 intentional M? violations)
- Manual updated for new codes and dbglog
- Full test suite passes (8 tests, 0 exit failures)

Next: Phase 2 (raw-emit pipeline) — on hold.
