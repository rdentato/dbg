# Current Branch

`master` (tracking `origin/master`; local working tree has dbglog hardening changes)

# Active Task

dbglog hardening. Correctness/robustness fixes and regression tests implemented; internal parsed-event/rendering refactor remains.

# Last Stop

Implemented dbglog hardening milestones 1-6 and 8:
- pointer/size parsing fixed
- malformed/unknown `M?` failures explicit
- realloc and allocation-range handling fixed
- `-F -H`, no-source `M?`, HTML allocation failures fixed
- long-line, allocation-limit, and unterminated-block diagnostics added
- dbgtrk expectation overflow inserts one `FAIL` line and ignores excess strings
- `test/t_dbglog.sh` added and wired into `test/makefile`

Verified:
- `make -C src all`
- `make -C test runtest`

# Open Questions

1. Should the remaining internal parsed-event/rendering refactor be done now, or deferred?

# Last Decision

For dbglog hardening, `-F -H` emits only failure lines, multi-file input preserves concatenated-log state, and dbgtrk overflow ignores excess expectations while inserting a `FAIL` line.

# Pointers

- `PLAN.md` — dbglog hardening section
- `journal/2026-05-02-dbglog-hardening-plan.md`
- `src/dbglog.c`
- `test/t_dbglog.sh`
