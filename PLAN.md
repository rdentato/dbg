# Objective

- Complete the `dbg.h` release-hardening pass.

# Status

- [x] Release-hardening complete.

# Milestones

- [x] Resolve the strict ISO C portability issue by making `dbgchk`/`dbgmst` require an explicit message argument.
- [x] Remove public-header const-qualification warnings in `DEBUG_ALLOC` wrappers.
- [x] Add verification that asserts the public output contract for `dbg.h`.
- [x] Decide release versioning before shipping.

# Tasks

- [x] Resolve the `dbgchk`/`dbgmst` portability issue by requiring an explicit message argument and updating current test call sites.
- [x] Add strict compilation checks to validate the current macro form with existing tests.
- [x] Change internal wrapper signatures in `src/dbg.h` from mutable string pointers to const-correct ones where appropriate.
- [x] Re-run strict-warning compilation for `DEBUG_ALLOC` and ordinary debug builds to confirm warning cleanup.
- [x] Add a self-checking test for representative `dbg.h` stderr output.
- [x] Use the self-checking test approach instead of checked-in output fixtures.
- [x] Update version identifiers and release docs once the code and checks are green.

# Risks

- The `dbgchk`/`dbgmst` portability fix changes the public API by requiring an explicit message argument.
- Output-contract tests may need careful normalization if stderr formatting differs across toolchains.

# Blockers

- None currently.

# Decision Log

- Excluded from this release-hardening plan: improving `dbglog` handling of overlong input lines.
- Excluded from this release-hardening plan: adding further `dbglog` verification; the current focus is `dbg.h` production readiness.
- Decided not to add CI for this release; local makefile-driven verification is sufficient for this small near-complete solo project.

# Next Step

- Await the next user goal.
