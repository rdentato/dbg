# Current Branch

`master` (ahead of `origin/master` by several commits)

# Active Task

Embedded-simplification refactor of `dbg.h`: replace human-readable text prefixes with single-byte codes, emit raw values as 16-char hex strings, defer all formatting to `dbglog`. Planned but not started.

# Last Stop

Specification agreed:
- Event codes: `E`/`W`/`I`/`P`/`F`/`T`/`t`/`K`/`k`/`V`/`v`/`C`/`c`/`N`/`D`
- Format string emitted null-terminated, not printf-expanded
- Numeric args: 16-char uppercase hex
- `%s` args: null-terminated string content
- Source location: `\x0F <file>\0 <line-ascii>\n`
- `double` via union cast to uint64
- `DEBUG_ALLOC` excluded from this pass
- Implementation not yet started; no source files touched

# Open Questions

1. Should dbglog support both old and new formats during transition?
2. What does `DBG_PUTC` default to when `<stdio.h>` is not included?

# Last Decision

Agreed on 16-char hex encoding for all numeric args — avoids ABI assumptions in dbglog.

# Pointers

- `PLAN.md` — embedded refactor milestones and tasks
- `docs/manual.md` — user documentation (will need updating)
- `src/dbg.h` — main header to refactor
- `src/dbglog.c` — log formatter to rewrite
- `test/` — test suite (will need updates)
