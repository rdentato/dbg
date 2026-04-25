# Current Context

- Added `src/dbglog.c`, a readable formatter for raw `dbg` logs.

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
- Pending: None.
- Notes: `dbg.h` should stay lightweight; richer reporting should live outside the header.
- Worked on: Adapting repository files to the manually changed variadic `dbgtrk(...)` API.
- Completed: Updated `examples/akkartik/foo.c`, `src/dbg.h` comments, and `docs/project-description.md`; recorded durable knowledge.
- Pending: None.
- Notes: `test/t_trace.c`, `test/t_test.c`, and both Akkartik examples compile with the new trace macro.
- Worked on: Removing stale references after the placeholder C tool source was deleted.
- Completed: Updated project documentation and recorded durable knowledge.
- Pending: None.
- Notes: The deleted file was intentionally useless; current library surface is `src/dbg.h`.
- Worked on: Documenting the new one-argument `dbgchk(expr)` form.
- Completed: Updated `src/dbg.h` comments, `docs/project-description.md`, and durable knowledge.
- Pending: None.
- Notes: `test/t_test.c` now exercises `dbgchk(1>x)` without an explicit message.
- Worked on: Making the file/line message primitive internal-only.
- Completed: Replaced `dbgmsg` with internal `dbg_msg` in `src/dbg.h`, removed `dbgmsg` from docs, and updated tests to use public macros.
- Pending: None.
- Notes: External code should use public diagnostics; `dbg_msg` is reserved for internal macro implementation.
- Worked on: Documenting the new `dbgvrb(...) { ... }` block macro.
- Completed: Updated `src/dbg.h` comments and `docs/project-description.md` to describe `VRB` markers for expected program stderr output.
- Pending: None.
- Notes: `dbgvrb` is for output classification by later reporting tools, not for generating debug diagnostics itself.
- Worked on: Writing a complete user manual for `dbg`.
- Completed: Added `docs/manual.md` with build modes, debug levels, message/test/trace/timing/grouping/memory APIs, examples, and usage guidance; committed and pushed as `a4a377e`.
- Pending: None.
- Notes: The manual documents the important block-macro nuance: disabled marker blocks may still execute their enclosed C code, while `dbgblk` is the debug-only block form.
- Worked on: Fixing `test/makefile` `runtest` behavior.
- Completed: `make runtest` now builds discovered `t_*.c` tests, clears `test.log`, and redirects test stderr into it; committed and pushed as `b12c755`.
- Pending: None.
- Notes: Remaining untracked files are local/generated or scratch: `src/x.c`, `test/t_test`, `test/t_trace`, and `test/test.log`. `CHANGES.md` is ignored and should not be committed.
- Worked on: Session checkpoint.
- Completed: Created `PLAN.md` because it was absent; no active multi-session plan exists.
- Pending: Await next user goal.
- Notes: Durable decisions are recorded in `knowledge/001` through `knowledge/007`.
- Worked on: Removing the abandoned external reporting path.
- Completed: Removed the planning/spec/tool files for the abandoned report utility, removed active references to it from source comments and docs, and kept `dbglog` as the remaining log helper.
- Pending: None.
- Notes: Reporting is no longer planned as part of this repository.
- Worked on: Removing the empty `examples/akkartik` directory.
- Completed: Removed the empty directory after deleting its tracked files.
- Pending: None.
- Notes: The parent `examples/` directory remains.
- Worked on: Creating `src/dbglog.c`.
- Completed: Added a readable log formatter that accepts stdin or log files, recognizes `0x0F` source locations and pasted `file:line` suffixes, prints `FILE` headers and line-numbered records, annotates `TST]:` with failed-check counts, and prints a final `RSLT`; added it to `src/makefile` and project description; recorded durable knowledge.
- Pending: None.
- Notes: Verified `make -f makefile clean all` in `src/`, regenerated `test/test.log`, matched the provided transformation example, and cleaned generated binaries/logs.
- Worked on: Expanding `dbg.h` test coverage.
- Completed: Added `test/t_alloc.c`, `test/t_debug.c`, `test/t_levels.c`, and `test/t_nodebug.c`; updated project description; fixed wrapper issues found while making the tests pass; recorded durable knowledge.
- Pending: None.
- Notes: Verified clean `test` build and `make runtest`, clean `src` tool build, and `dbglog` formatting; cleaned generated artifacts afterward.
- Worked on: Making the raw print primitive internal-only.
- Completed: Removed the public raw print macro and disabled counterpart, renamed the internal primitive to `dbg_prt`, updated `dbgclk`, tests, docs, and durable knowledge.
- Pending: None.
- Notes: Verified no old raw-print macro name remains in active text/code/docs/knowledge; rebuilt and ran tests, built tools, checked `dbglog`, then cleaned generated artifacts.
- Worked on: Refining `dbglog` file summaries.
- Completed: Changed `dbglog` to print `RSLT` at the end of each file section and reset counters when the source file changes; recorded durable knowledge.
- Pending: None.
- Notes: Verified on the multi-file `test.log`; each file now gets its own failed-checks summary, matching `TST]: failed / checks` semantics.
- Worked on: Adapting to the new non-nested `dbgtst` rule.
- Completed: Simplified `dbglog` to a single active test scope and fixed the updated `dbgtst` macro so it compiles while enforcing one active test at a time.
- Pending: None.
- Notes: Verified clean test rebuild/run and `dbglog` output on the regenerated log. The commented nested example in `test/t_test.c` still documents that nested `dbgtst` blocks are invalid.
- Worked on: Compacting `dbglog.c`.
- Completed: Removed small helper indirections and the tiny test-state struct, keeping the same output with simpler direct counters and file-switch handling.
- Pending: None.
- Notes: Rebuilt `dbglog`, regenerated the test log, compared output, and cleaned artifacts.
