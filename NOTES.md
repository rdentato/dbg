# Current Context

- Documented `dbgvrb(...) { ... }` as a marker for expected program stderr output.

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
- Worked on: Documenting the new one-argument `dbgchk(expr)` form.
- Completed: Updated `src/dbg.h` comments, `docs/project-description.md`, and durable knowledge.
- Pending: None.
- Notes: `test/t_test.c` now exercises `dbgchk(1>x)` without an explicit message.
- Worked on: Making the file/line message primitive internal-only.
- Completed: Replaced `dbgmsg` with internal `dbg_msg` in `src/dbg.h`, removed `dbgmsg` from docs, and updated tests to use public macros.
- Pending: None.
- Notes: External code should use `dbgprt`, `dbgerr`, `dbgwrn`, or `dbginf`; `dbg_msg` is reserved for internal macro implementation.
- Worked on: Documenting the new `dbgvrb(...) { ... }` block macro.
- Completed: Updated `src/dbg.h` comments and `docs/project-description.md` to describe `VRB` markers for expected program stderr output.
- Pending: None.
- Notes: `dbgvrb` is for output classification by later reporting tools, not for generating debug diagnostics itself.
