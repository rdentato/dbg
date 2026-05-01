# Objective

- Refactor `dbg.h` for embedded compatibility: minimize C library dependencies, emit raw data, let `dbglog` handle formatting.

# Status

- [~] Specification agreed; implementation not started.

# Milestones

1. [ ] Add `DBG_PUTC` macro and hex/string output primitives.
2. [ ] Replace `dbg_msg` + `fprintf` with `dbg_emit` using single-byte codes and 16-char hex args.
3. [ ] Rewrite block macros (`dbgtst`, `dbgtrk`, `dbgvrb`, `dbgclk`) for new format.
4. [ ] Rewrite `dbgchk`/`dbgmst` with `P`/`F` codes and `D` detail.
5. [ ] Rewrite `dbgnow` with `N` code and raw `time_t` as hex.
6. [ ] Make headers conditional; remove `<time.h>` dependency.
7. [ ] Rewrite `dbglog.c` to parse new format.
8. [ ] Update tests.
9. [ ] Update documentation.

# Format Specification

- Event codes: single ASCII byte (`E`, `W`, `I`, `P`, `F`, `T`/`t`, `K`/`k`, `V`/`v`, `C`/`c`, `N`, `D`)
- Payload: format string null-terminated (not printf-expanded)
- Numeric args: 16-char zero-padded uppercase hex
- `%s` args: null-terminated string content
- `double` args: union cast to uint64, emitted as 16-char hex
- Source location: `\x0F <file>\0 <line-decimal-ascii> \n`
- No spaces after `\0` delimiters

# Risks

- `dbglog` incompatible with old format; need full rewrite or dual-mode parser.
- `DEBUG_ALLOC` excluded from this pass (too many stdlib dependencies).
- Format specifier parsing in `dbg_emit` needs to handle full printf syntax without stdlib.

# Blockers

- None.

# Decision Log

- `DEBUG_ALLOC` excluded from embedded pass.
- 16-char hex chosen over raw binary to avoid ABI assumptions in dbglog.
- No commits until user gives explicit word.

# Next Step

- Begin implementation: `DBG_PUTC` + hex primitives + `dbg_emit` function in `src/dbg.h`.
