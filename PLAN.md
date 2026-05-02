# Objective

Refactor `dbg.h` for embedded compatibility: minimize C library dependencies, emit raw data, let `dbglog` handle formatting.

# Status

- [x] Phase 1: Event-code renaming — single-byte codes replace multi-byte prefixes.
- [!] Phase 2: Raw-emit pipeline — on hold.
- [x] dbglog rewrite — unified text/HTML, 6-char markers, M? tracking, dbgtrk evaluation.
- [x] Test `t_alloc_fail.c` — 9 intentional M? violations, all correctly detected.

# Phase 1 — Event Codes (done)

Old multi-byte prefixes replaced with single-byte codes in working tree. Codes use `:` for simple messages, `[`/`]` for block open/close, uppercase for both.

| Macro | Old Prefix | New Code | Type |
|---|---|---|---|
| `dbgerr` | `EROR: ` | `E:` | simple |
| `dbgwrn` | `WARN: ` | `W:` | simple |
| `dbginf` | `INFO: ` | `I:` | simple |
| `dbgvrb` | `VRB[: ` … `VRB]:` | `V[` … `V]\x0E` | verbatim |
| `dbgtst` | `TST[: ` … `TST]:` | `T[` … `T]` | block |
| `dbgtrk` | `TRK[: ` … `TRK]:` | `K[` … `K]` | block |
| `dbgclk` | `CLK[` … `CLK]` | `C[` … `C]` | block |
| `dbgchk`/`dbgmst` | `PASS` / `FAIL` | `P` / `F` | simple (via `%c`) |
| `dbgchk` detail | ``     ` `` | `F=` | detail line |
| `DEBUG_ALLOC` allocs | `MTRK: ` | `M:` | simple |
| `DEBUG_ALLOC` checks + ptr | `MCHK: ` | `M?` | simple |
| `dbgnow` | `NOW=: ` | **removed** | — |

Internal primitives added: `dbg_prt`, `dbg_putc`, `dbg_puts` (thin wrappers over `fprintf`/`fputc`/`fputs` to `stderr`).

`dbg_msg` emits `\x0F<file>:<line>\n` (no leading space).

`dbgvrb` close emits `\nV]\x0E\n` (verbatim block). Other block closes emit `<code>]\n`.

`dbgclk` struct simplified to `{uint32_t millis; uint32_t loop}`, using `dbg_millis()` (clock_gettime or Arduino `millis()`).

`<stdint.h>` added unconditionally.

# Phase 2 — Raw-Emit Pipeline (not started)

Replace all `dbg_prt`/`fprintf` calls with `dbg_emit` that writes:

- **Format string:** null-terminated literal (not printf-expanded)
- **Numeric args:** 16-char zero-padded uppercase hex
- **String args (`%s`):** null-terminated string content
- **`double`:** union-cast to `uint64_t`, 16-char hex
- **Source location:** `\x0F <file>\0 <line-decimal-ascii> \n`
- **`DBG_PUTC`:** single-byte output primitive; defaults to `fputc(c, stderr)` unless overridden

## Milestones

1. [ ] Add `DBG_PUTC` macro (overridable single-byte output).
2. [ ] Implement `dbg_emit` family: `dbg_emit_hdr(code)`, `dbg_emit_str(s)`, `dbg_emit_hex(u64)`, `dbg_emit_loc(file,line)`, `dbg_emit_close(code)`.
3. [ ] Rewrite simple macros (`dbgerr`, `dbgwrn`, `dbginf`) to use `dbg_emit`.
4. [ ] Rewrite block macros (`dbgtst`, `dbgtrk`, `dbgvrb`, `dbgclk`) to use `dbg_emit`.
5. [ ] Rewrite `dbgchk`/`dbgmst` to use `dbg_emit` with `P`/`F` codes and `F=` detail.
6. [ ] Make `<stdio.h>`, `<time.h>`, `<stdlib.h>`, `<string.h>` conditional on `DBG_PUTC` / `DBG_STDIO`.
7. [ ] Remove `dbg_prt`, `dbg_putc`, `dbg_puts` (replaced by `dbg_emit` / `DBG_PUTC`).
8. [ ] Rewrite `dbglog.c` to parse new binary format.
9. [ ] Update tests.
10. [ ] Update documentation.

## Event-Code Register (final)

These codes are locked. Do not change.

| Code | Meaning | Used By | Delimiter |
|---|---|---|---|
| `E` | error | `dbgerr` | `:` |
| `W` | warning | `dbgwrn` | `:` |
| `I` | info | `dbginf` | `:` |
| `T` | test block | `dbgtst` | `[` / `]` |
| `K` | track | `dbgtrk` | `[` / `]` |
| `V` | verbatim (program output) | `dbgvrb` | `[` / `]` + `\x0E` |
| `C` | clock | `dbgclk` | `[` / `]` |
| `P` | check passed | `dbgchk`/`dbgmst` | `:` |
| `F` | check failed / detail | `dbgchk`/`dbgmst` | `:` / `=` |
| `M` | memory alloc tracking | `DEBUG_ALLOC` (excluded this pass) | `:` |
| `M?` | memory boundary checks + pointer check | `DEBUG_ALLOC` (excluded this pass) | `?` |

## Risks

- `dbglog` incompatible with old format; needs full rewrite or dual-mode parser.
- `DEBUG_ALLOC` excluded from this pass (too many stdlib dependencies).
- Format specifier parsing in `dbg_emit` needs to handle full printf syntax without stdlib.

## Holds

- [!] Phase 2 (Raw-Emit Pipeline) — all milestones on hold.
- [!] Arduino-specific functions (e.g. `DBG_ARDUINO` path in `dbg_millis`) — deferred.

## dbglog Rewrite (done)

```
dbglog [-F] [-H] [log ...]
```

- `-F` — show only failures
- `-H` — output as HTML
- Default: plain-text report with one line per event

Implemented:
1. Parses all current event codes.
2. **dbgtrk** — parses expectation strings from `K[`, scans buffered output, reports PASS/FAIL at `K]`.
3. **M? validation** — tracks allocations from `M:` events (malloc/calloc/realloc/free/strdup/strndup), validates `M?` bounds.
4. **Verbatim blocks** (`V[`…`V]\x0E`) passed through.
5. **Clock blocks** (`C[`…`C]`) passed through.
6. No FILE/RSLT statistics.
7. Source locations parsed from `\x0F` separator.

## Blockers

- None.

## Decision Log

- `DEBUG_ALLOC` excluded from embedded pass.
- 16-char hex chosen over raw binary to avoid ABI assumptions in dbglog.
- Event codes and delimiters are final and locked (Phase 1 complete).
- No commits until user gives explicit word.

## Next Step

- Begin Phase 2 implementation: `DBG_PUTC` macro + `dbg_emit` primitives in `src/dbg.h`.
