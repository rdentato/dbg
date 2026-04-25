# Project Description: dbg

## Overview

`dbg` is a small C debugging and testing macro library. Its primary implementation is the header file `src/dbg.h`, which provides compile-time controlled macros for diagnostic printing, lightweight assertions, scoped test blocks, simple timing, debug grouping, and optional memory-call tracing. The `src/` directory also contains small log-processing tools.

The repository README describes the project as a "very minimal debug/testing macro" and points readers to `src/dbg.h` for full documentation. The code is distributed under the MIT License.

## Project Status

This repository appears to be a compact, early-stage or partially restored project:

- The usable library surface is concentrated in `src/dbg.h`.
- A minimal `src/dbglog.c` formatter is present for making raw `dbg` logs easier to read.

## Repository Layout

```text
.
|-- AGENTS.md
|-- LICENSE
|-- README.md
|-- docs/
|   |-- manual.md
|   `-- project-description.md
|-- src/
|   |-- dbg.h
|   |-- dbglog.c
|   `-- makefile
`-- test/
    |-- makefile
    |-- t_alloc.c
    |-- t_debug.c
    |-- t_levels.c
    |-- t_nodebug.c
    `-- t_test.c
```

## Main Components

### `src/dbg.h`

`src/dbg.h` is the main library file. It defines the project version and all public debug/test macros.

Current version identifiers:

```c
#define DBG_VERSION     0x0201000C
#define DBG_VERSION_STR "dbg 2.1.0-rc"
```

The header is designed so application code can include debug calls unconditionally. When `DEBUG` is not defined, most `dbg...` macros compile to no-op forms, allowing release builds to retain the same source code without emitting diagnostics.

`NDEBUG` has priority over `DEBUG` and `DEBUG_ALLOC`: if `NDEBUG` is defined, both switches are undefined by the header.

### `test/t_test.c`

`test/t_test.c` is a small executable sample for the test and profiling macros. It exercises:

- `dbginf`
- `dbgvrb`
- `dbgchk`
- `dbgtst`
- `dbgblk`
- `_dbgblk`
- `dbgclk`

It can be compiled directly with:

```sh
cc -DDEBUG=DBGLVL_TEST -Wall -O0 -Isrc -o /tmp/dbg_t_test test/t_test.c
```

Running it intentionally emits both passing and failing checks, making it a demonstration rather than a conventional all-green unit test.

### Additional `test/t_*.c` Coverage

The remaining test programs exercise focused scenarios:

- `test/t_alloc.c`: `DEBUG_ALLOC`, allocation wrappers, memory/string wrappers, and `dbgptr`.
- `test/t_debug.c`: enabled diagnostics, grouping, disabled underscore forms, `dbgblk`, `dbgmst`, and timing.
- `test/t_levels.c`: warning-level behavior below `DBGLVL_TEST`.
- `test/t_nodebug.c`: `NDEBUG` suppression and block macro behavior with debugging disabled.

## Debug Levels

Debug behavior is controlled by defining `DEBUG` before including `dbg.h`. The header defines these levels:

| Level | Value | Enabled Behavior |
|---|---:|---|
| `DBGLVL_ERROR` | `0` | `dbgerr`, `dbgvrb` |
| `DBGLVL_WARN` | `1` | Error-level macros plus `dbgwrn` |
| `DBGLVL_INFO` | `2` | Warning-level macros plus `dbginf` |
| `DBGLVL_TEST` | `3` | Test, profiling, and block macros except memory pointer tracing |

Example compile command:

```sh
cc -DDEBUG=DBGLVL_TEST -Isrc program.c
```

For a release-style build, omit `DEBUG` or define `NDEBUG`.

Memory allocation debugging is controlled separately with `DEBUG_ALLOC`. Defining `DEBUG_ALLOC` enables the memory wrappers and also implies `DEBUG` if `DEBUG` was not already defined. In that implied case, `DEBUG` defaults to error-level behavior rather than test-level behavior.

## Public Macro Groups

### Message Printing

The basic message macros write to `stderr`.

| Macro | Purpose |
|---|---|
| `dbgvrb(...) { ... }` | Mark enclosed program `stderr` output with `VRB` delimiters so it can be distinguished from dbg-generated diagnostics. |
| `dbgerr(...)` | Print a `FAIL:` message. |
| `dbgwrn(...)` | Print a `WARN:` message when `DEBUG >= DBGLVL_WARN`. |
| `dbginf(...)` | Print an `INFO:` message when `DEBUG >= DBGLVL_INFO`. |

`dbg_msg(...)` and `dbg_prt(...)` are internal primitives used by the public message macros. They are not part of the public API.

### Testing

Testing macros are enabled when `DEBUG >= DBGLVL_TEST`.

| Macro | Purpose |
|---|---|
| `dbgtst(...) { ... }` | Start a named test block. It emits test start/end markers and cannot be nested. |
| `dbgchk(expr)` | Evaluate an expression, emit `PASS` or `FAIL`, and set `errno` to `0` or `1`. |
| `dbgchk(expr, message, ...)` | Like `dbgchk(expr)`, but prints a formatted message when the expression fails. |
| `dbgmst(expr, ...)` | Like `dbgchk`, but aborts the program if the check fails. The failure message is optional. |
| `dbgblk { ... }` | Execute a block only when test-level debugging is enabled. |

### Profiling

`dbgclk(...) { ... }` measures elapsed CPU clock ticks for a scoped block and prints start/end timing messages. It reports elapsed time as a fraction of `CLOCKS_PER_SEC`.

Example:

```c
dbgclk("Testing count to %d", count) {
  for (int k = 0; k < count; k++) ;
}
```

### Grouping

`DBG_ON` and `DBG_OFF` allow related debug calls to be grouped behind project-specific macros.

Example:

```c
#define DBG_CHECK_INGEST DBG_ON

DBG_CHECK_INGEST(dbgchk(dataread > 0, "No data read: %d", dataread));
```

Changing the group definition to `DBG_OFF` compiles those grouped calls out without removing them from source.

### Temporarily Disabled Macros

For most public macros, `dbg.h` also provides an underscore-prefixed no-op counterpart:

- `_dbgtst`
- `_dbginf`
- `_dbgvrb`
- `_dbgwrn`
- `_dbgerr`
- `_dbgchk`
- `_dbgmst`
- `_dbgptr`
- `_dbgclk`
- `_dbgblk`

These are intended for temporarily disabling individual debug calls without deleting or commenting them out.

### Memory Tracing

When `DEBUG_ALLOC` is defined, `dbg.h` wraps selected allocation and memory/string functions to emit memory trace/check lines.

Wrapped allocation functions:

- `malloc`
- `calloc`
- `realloc`
- `free`
- `strdup`
- `strndup`

Wrapped memory/string operations:

- `strcpy`
- `strncpy`
- `strcat`
- `strncat`
- `memcpy`
- `memmove`
- `memset`

Additional macro:

- `dbgptr(p)`: emits a memory-check line for a pointer.

The memory tracing mode relies on `strdup` and `strndup`, which are POSIX-style functions rather than ISO C functions. The header requests C library extension declarations with `__STDC_WANT_LIB_EXT2__`, but portability depends on the compiler and C library.

Example compile command for allocation tracing only:

```sh
cc -DDEBUG_ALLOC -Isrc program.c
```

Example compile command for both test-level debugging and allocation tracing:

```sh
cc -DDEBUG=DBGLVL_TEST -DDEBUG_ALLOC -Isrc program.c
```

## Build And Usage

There is no root `Makefile`, `CMakeLists.txt`, package manifest, or checked-in root build script in the current repository. The visible code can be compiled directly with a C compiler.

Compile the main test sample:

```sh
cc -DDEBUG=DBGLVL_TEST -Wall -O0 -Isrc -o /tmp/dbg_t_test test/t_test.c
```

Use the library in another C file:

```c
#define DEBUG DBGLVL_TEST
#include "dbg.h"

int main(void) {
  dbgtst("basic arithmetic") {
    dbgchk(1 + 1 == 2);
    dbgchk(2 + 2 == 5, "unexpected arithmetic result");
  }
}
```

Then compile with:

```sh
cc -DDEBUG=DBGLVL_TEST -Isrc -o program program.c
```

## Verification Performed

The following checks were run while preparing this document:

```sh
make -f makefile clean all
make -f makefile runtest
```

Results:

- All `test/t_*.c` programs compiled and ran through `test/makefile`.
- `test/t_test.c` still intentionally emits failing checks as demonstration data.
- `src/dbglog` processed the generated `test.log`.

## Known Gaps And Risks

- There is no automated test runner or CI configuration in the current checkout, despite the README badge pointing to Travis CI.
- Memory tracing mode redefines standard/POSIX allocation and memory functions, which can be useful for diagnostics but may surprise consumers if enabled globally.

## License

The project is licensed under the MIT License. Copyright in the repository is attributed to Remo Dentato.
