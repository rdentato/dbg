# 2026-05-01

## Worked on

- Documented `dbgtrk` in `docs/manual.md`
- Fixed `dbgtrk` nesting guard (own `dbg_nested_track` variable) and stringified expectations
- Added `dbgnow` timestamp macro with test (`test/t_now.c`)
- Fixed zero-length format warning in `dbgnow` and `dbgclk`
- Added `_dbgnow` disabled form and no-DEBUG fallback
- Removed `errno` from `dbgchk`; extracted `dbg_chk` helper; updated tests and docs
- Planned embedded refactor: agreed on single-byte codes + 16-char hex format

## Completed

- Commits: `67515e5` through `beae91e` (7 commits on `master`)
- `dbgtrk` documented, fixed, and tested
- `dbgnow` implemented, tested, documented
- `errno` dependency removed from `dbgchk`

## Pending

- Embedded refactor: specification agreed, implementation not started
- `dbglog` rewrite for new format
- Test suite updates for new format

## Decisions

- `DEBUG_ALLOC` excluded from embedded pass
- 16-char hex encoding for numeric args (avoids ABI assumptions)
- Single-byte event codes replacing multi-byte prefixes
- No commits until explicit user approval

## Notes

- Current `dbg.h` format spec: `PLAN.md` § "Format Specification"
- Next step: implement `DBG_PUTC` + hex primitives + `dbg_emit` in `src/dbg.h`
