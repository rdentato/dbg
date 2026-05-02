# Current Branch

`master` (ahead of `origin/master` by several commits)

# Active Task

Phase 1 complete. dbglog rewritten. Phase 2 on hold.

# Last Stop

Session checkpoint. All work unstaged.

- `src/dbg.h` — Phase 1 event codes, dbg_millis, simplified dbgclk, M:/M? format
- `src/dbglog.c` — complete rewrite, single code path for text/HTML, zero warnings
- `test/t_alloc_fail.c` — new test, 9 intentional M? failures
- dbglog: uniform 6-char markers, M? allocation tracking, dbgtrk with T[ source

# Open Questions

1. Should -F affect HTML mode too? (currently HTML always shows full report)
2. What does `DBG_PUTC` default to when `<stdio.h>` is not included?

# Last Decision

dbglog unified text/HTML into single process_line. Event codes expanded to 6-char uniform width. Memory events: neutral color (same as clock).

# Pointers

- `PLAN.md` — status and milestones
- `journal/2026-05-02-embedded-refactor.md` — session record
- `src/dbg.h` — Phase 1 changes (unstaged)
- `src/dbglog.c` — complete rewrite (unstaged)
- `test/t_alloc_fail.c` — new test (unstaged)
