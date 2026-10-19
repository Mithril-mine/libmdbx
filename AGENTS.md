# libmdbx — agent guide

_Package: v0.15.0-139-g634a8449 (2026-10-19T08:59:22+03:00)_

_libmdbx_ is an embedded transactional key-value database: compact, extremely fast
(memory-mapped, B+tree, O(log N)), ACID, crash-safe by default, no WAL, no server
process, safe for concurrent multi-process access. This package is the
**amalgamated source code**, prepared for embedding and building — it contains no
test suite and no development internals.

## What's inside

Flat single-file sources (package root):

| File | Purpose |
| --- | --- |
| `mdbx.h`, `mdbx.c` | C API and implementation |
| `mdbx.h++`, `mdbx.c++` | C++ API and implementation |
| `mdbx-internals.h` | internal definitions needed to build `mdbx.c` |
| `mdbx_chk.c`, `mdbx_copy.c`, `mdbx_drop.c`, `mdbx_dump.c`, `mdbx_load.c`, `mdbx_stat.c`, `mdbx_defrag.c` | standalone command-line tools |

Build files and documentation: `CMakeLists.txt`, `GNUmakefile`, `Makefile`,
`config.h.in`, `cmake/*.cmake`, `README.md`, `ChangeLog.md`, `LICENSE`, `NOTICE`,
`COPYRIGHT`, `VERSION.json`, `conanfile.py`, `examples/`.

## Building

### CMake (recommended)

```sh
cmake -S . -B build        # add -DMDBX_BUILD_CXX=ON for the C++ API
cmake --build build
ctest --test-dir build     # smoke tests (if MDBX_ENABLE_TESTS)
```

Useful options: `MDBX_BUILD_CXX` (enable the C++ API), `MDBX_BUILD_TOOLS`
(standalone tools), `MDBX_BUILD_SHARED_LIBRARY` (shared vs static),
`MDBX_ENABLE_TESTS`, `INTERPROCEDURAL_OPTIMIZATION`, and `MDBX_CONFIG_H` to point
to a generated configuration header.

### GNU make

```sh
make mdbx-static.o     # static library object (or: make mdbx-dylib.o)
make check             # build + smoke checks
make mdbx_chk          # build a specific tool
```

The result is the library (`libmdbx.a`, `libmdbx.so`/`.dylib`/`.dll`) plus the tools.

## Using the C API

Include `mdbx.h` and link the library. Functions return an `int` status code
(`MDBX_result`); get a human-readable message with `mdbx_strerror(rc)`. The model:

- **Environment** (`MDBX_env`) — one database file (or a directory of files).
- **DBI** (`MDBX_dbi`) — a named key/value map inside the environment.
- **Transactions** (`MDBX_txn`) — parallel readers never block a writer; writers
  are serialized. Open read transactions with `MDBX_TXN_RDONLY`.
- **Cursors** (`MDBX_cursor`) — ordered iteration inside a transaction.

Minimal example:

```c
#include <stdio.h>
#include "mdbx.h"

int main(void) {
    MDBX_env *env = NULL;
    MDBX_txn *txn = NULL;
    MDBX_dbi dbi;
    MDBX_val key = {0}, data = {0};
    int rc;

    rc = mdbx_env_create(&env);
    if (rc) goto bail;
    mdbx_env_set_maxdbs(env, 4);            /* named maps need a DBI budget */
    rc = mdbx_env_open(env, "example.db", MDBX_SAFE_NOSYNC, 0644);
    if (rc) goto bail;

    /* write transaction: open a named map and put a key/value */
    rc = mdbx_txn_begin(env, NULL, 0, &txn);
    if (rc) goto bail;
    rc = mdbx_dbi_open(txn, "kv", MDBX_CREATE, &dbi);
    if (!rc) {
        key.iov_base = (void *)"key";   key.iov_len = 3;
        data.iov_base = (void *)"value"; data.iov_len = 5;
        rc = mdbx_put(txn, dbi, &key, &data, 0);
    }
    if (rc)
        mdbx_txn_abort(txn);
    else
        rc = mdbx_txn_commit(txn);
    if (rc) goto bail;

    /* read transaction: fetch the value back */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (!rc) {
        rc = mdbx_get(txn, dbi, &key, &data);
        if (!rc)
            printf("got: %.*s\n", (int)data.iov_len, (const char *)data.iov_base);
        mdbx_txn_abort(txn);
    }

bail:
    if (env) mdbx_env_close(env);
    return rc;
}
```

Key points for correct usage:

- **Crash safety is the default** (`MDBX_SYNC_DURABLE`). Trade durability for
  throughput with `MDBX_SAFE_NOSYNC` or `MDBX_NOMETASYNC`; never use
  `MDBX_UTTERLY_NOSYNC` for valuable data.
- **Keys and values are binary**: use `MDBX_val` (`iov_base`/`iov_len`); never
  assume NUL-termination.
- **Data integrity**: run `mdbx_chk` to verify a database; make consistent copies
  with `mdbx_copy` or the online backup API (`mdbx_env_copy2...`).
- **Environment size is dynamic** (auto-grow/shrink); set geometry explicitly via
  `mdbx_env_set_geometry()` when you need predictable sizing.
- **Keep read transactions short**: they prevent reclaiming of discarded pages,
  so long-lived read txns can grow the file.

## Using the C++ API

Built with `MDBX_BUILD_CXX=ON`; include `<mdbx.h++>`. Everything lives in
namespace `mdbx` with RAII handles (`env`/`env_managed`, `txn`/`txn_managed`,
`cursor`/`cursor_managed`) and value helpers (`buffer`, `slice`). Run the shipped
`examples/example-mdbx.c++` for a working start.

## Further reading

- `README.md` — full documentation (characteristics, platform notes, building).
- `examples/example-mdbx.c` and `examples/example-mdbx.c++` — runnable examples.
- Documentation site (guides, textbook, internals deep dive):
  <https://libmdbx.dqdkfa.ru/docs/en/index.html>.
- C and C++ API reference: <https://libmdbx.dqdkfa.ru/docs/en/reference/>.
- License: Apache-2.0 (see `LICENSE`/`NOTICE`/`COPYRIGHT`). Note that the test
  suite of the upstream repository is under a separate non-free license and is
  **not** part of this package.

