# Current Context

- Updated dependent files for variadic `dbgtrk(...)` trace patterns.

# Open Questions

- None.

# Known Issues / Blockers

- None.

# Knowledge Candidates

- None.

# Session Log

## 2026-04-25

- Worked on: Analyzing project structure and source to document project purpose, architecture, usage, and development details.
- Completed: Created session notes because `NOTES.md` was absent; wrote and verified `docs/project-description.md`.
- Pending: None.
- Notes: `PLAN.md` is absent; no multi-session plan appears necessary for this bounded documentation task. Initial verification found old `test/ut_test.c` and `examples/akkartik/foo.c` compile, while old `test/ut_trace.c`, `examples/akkartik/bar.c`, and the removed placeholder C tool did not compile as-is.
- Worked on: Decoupling memory allocation debugging from the `DEBUG` level ladder.
- Completed: Added `DEBUG_ALLOC`, made it imply `DEBUG` at error level when needed, made `NDEBUG` suppress both switches, updated project description, and recorded durable knowledge.
- Pending: None.
- Notes: `DEBUG_ALLOC` allocation-only and combined `DEBUG=DBGLVL_TEST` builds compile. A smoke test confirmed allocation tracing emits `MTRK`/`MCHK` lines, and `NDEBUG` suppresses allocation tracing.
- Worked on: Compared `../tst` test-framework features against `dbg`.
- Completed: Decided not to add result accounting/reporting APIs to `dbg.h`.
- Pending: Future `dbgstat.c` implementation for nicer reporting.
- Notes: `dbg.h` should stay lightweight; reporting belongs in a separate tool.
- Worked on: Adapting repository files to the manually changed variadic `dbgtrk(...)` API.
- Completed: Updated `examples/akkartik/foo.c`, `src/dbg.h` comments, and `docs/project-description.md`; recorded durable knowledge.
- Pending: None.
- Notes: `test/t_trace.c`, `test/t_test.c`, and both Akkartik examples compile with the new trace macro.
- Worked on: Removing stale references after the placeholder C tool source was deleted.
- Completed: Updated project documentation and recorded durable knowledge.
- Pending: Future `dbgstat.c` implementation for nicer reporting remains separate.
- Notes: The deleted file was intentionally useless; current library surface is `src/dbg.h`.
