# 2026-05-02 dbglog hardening plan

## Worked on

- Added a multi-session hardening plan for `src/dbglog.c` to `PLAN.md`.

## Decisions

- Treat dbglog hardening as separate from Phase 2/raw-emit work.
- Preserve current text log format and CLI unless explicitly changed.

## Pending

- Begin milestone 1: baseline current behavior and add focused regression tests.
- Decide `dbglog -F -H` behavior.
- Decide whether multiple input files share parser/allocation state.

## Progress

- Began dbglog hardening milestone 1: baseline and regression tests.
- Inspected source and test makefiles.

## Progress

- Implemented dbglog hardening fixes for pointer parsing, malformed M? records, realloc handling, allocation range overflow, -F HTML behavior, no-source M? formatting, HTML allocation failures, long-line diagnostics, limit diagnostics, unterminated block diagnostics, and portable printf attributes.
- Added `test/t_dbglog.sh` regression coverage and wired it into `test/makefile`.
- Verified with `make -C src clean all`, `make -C test clean runtest`, and dbglog text/-F/-H/-F -H passes over `test/test.log`.
- Left internal parsed-event/rendering refactor as the remaining dbglog hardening milestone.

## Update

- Changed dbgtrk overflow handling: excess expectations are ignored but produce one normal FAIL line in dbglog output instead of stderr diagnostics.
- Updated regression test to ensure the ninth expectation is ignored and the overflow FAIL appears under `-F`.
- Re-verified `make -C src all`, `make -C test runtest`, dbglog text/-F/-H/-F -H over `test/test.log`, and `git diff --check`.
