# Current Context

- `dbg.h` release-hardening is complete.
- Local release verification is the supported path for this release: `make -C src clean all` and `make -C test clean runtest`.
- CI is intentionally deferred for this small near-complete solo project.
- Explicitly excluded for now: `dbglog` handling of overlong input lines.
- Current untracked local/generated artifacts: `src/dbglog`, `src/test.htm`, `test/t_alloc`, `test/t_debug`, `test/t_levels`, `test/t_nodebug`, `test/t_test`, `test/test.log`.

# Open Questions

- None.

# Known Issues / Blockers

- None.

# Knowledge Candidates

- None.

# Session Log

## 2026-05-01

- Worked on: dbgtrk() trace-based testing — header macro, dbglog tracking, and test suite.
- Completed:
  - User added `dbgtrk("=must-exist","!must-not-exist"){...}` macro to `src/dbg.h`.
  - Created `test/t_trk.c` with 13 test cases covering all four expectation outcomes plus edge cases (empty block, no expectations, two blocks in sequence, substring matching, 8-expectation max, 9-expectation overflow, buffer overflow).
  - Implemented TRK[: / TRK]: tracking in `src/dbglog.c`: parse expectations from TRK[: line, scan each log line with strstr, check at TRK]:, emit PASS/FAIL integrated with TST[: counters.
  - Optimized through several iterations per user direction:
    * Removed line buffering — scan on the fly, only store expectations.
    * Removed `must_exist`/`seen` struct fields — encode type in prefix char (`=`/`!`) and seen flag in 0x80 high bit.
    * Removed heap allocation — single `static unsigned char trkbuf[256]` with 8 offset slots at indices 0–7 and packed null-terminated strings starting at index 8.
    * Simplified parse to copy prefix+content in one memcpy, no back-stepping in pointer arithmetic.
  - Committed as `a88687f`.
- Pending: None.
- Notes: Test uses only dbg functions (dbginf, dbgtrk, dbgtst), no raw fprintf.

## 2026-04-25

- Worked on: Refreshing `NOTES.md` to match the current repository state.
- Completed: Removed stale historical session clutter, synced the note with the current `knowledge/` range, and recorded the present untracked local/generated artifacts.
- Pending: None.
- Notes: `PLAN.md` still reports no active multi-session objective or plan.
- Worked on: Final release-readiness review.
- Completed: Verified default builds/tests pass, identified production-readiness gaps, and converted the accepted items into an active release-hardening plan.
- Pending: Implement the plan starting with strict-ISO portability for `dbgchk`/`dbgmst`.
- Notes: Agreed to ignore the current `dbglog` long-line robustness issue for this round.
- Worked on: Tightening `dbgchk`/`dbgmst` for strict ISO C compatibility.
- Completed: Switched the API to require an explicit message argument, updated the remaining one-argument test call sites, and verified strict C99 syntax for the exercised test files.
- Pending: Update any remaining examples outside the checked files and continue with wrapper const-correctness.
- Notes: The portability fix is an intentional API change: callers now pass `""` when no failure message is needed.
- Worked on: Adding direct `dbg.h` output verification.
- Completed: Added `test/t_output.c`, a self-checking test that captures representative stderr output from `dbginf`, `dbgwrn`, `dbgerr`, `dbgvrb`, `dbgtst`, and `dbgchk`, compares it to the expected bytes, and passes under strict C99.
- Pending: None.
- Notes: `dbglog` verification is no longer part of the active release-hardening scope.
- Worked on: Finalizing `dbg.h` release-hardening.
- Completed: Fixed `DEBUG_ALLOC` const-correctness warnings, updated docs for the required `dbgchk(..., "")` / `dbgmst(..., "")` API, tightened `test/makefile` to strict C99, and aligned planning notes with the decision to skip CI for this release.
- Pending: None.
- Notes: Versioning was updated manually by the user before commit.
