# libmdbx — agent guide (amalgamated package)

_Package: v0.15.0-722-g1b4977460 (2026-11-26T20:29:48Z)_

> **Who this file is for:** AI agents helping users of the **amalgamated**
> libmdbx package — embedding, integration, building and debugging scenarios.
> This is **not** the development repository: it contains an automatically
> generated amalgamated version of the sources, and development-related
> changes are not accepted here (see [CONTRIBUTING.md](CONTRIBUTING.md)).
> Development happens in the upstream **libmdbx-devel** repository; bug
> reports that apply to the amalgamated package are welcome via issues.

_libmdbx_ is an embedded transactional key-value database: compact, extremely
fast (memory-mapped, B+tree, O(log N)), ACID, crash-safe by default, no WAL,
no server process, safe for concurrent multi-process access.

## 1. What's inside

Flat single-file sources (package root):

| File | Purpose |
| --- | --- |
| `mdbx.h`, `mdbx.c` | C API and implementation |
| `mdbx.h++`, `mdbx.c++` | C++ API and implementation |
| `mdbx-internals.h` | internal definitions needed to build `mdbx.c` |
| `mdbx-wingetopt.h` | bundled getopt implementation used by the tools |
| `mdbx_chk.c`, `mdbx_copy.c`, `mdbx_drop.c`, `mdbx_dump.c`, `mdbx_load.c`, `mdbx_stat.c`, `mdbx_defrag.c` | standalone command-line tools |

Build files: `CMakeLists.txt`, `GNUmakefile`, `Makefile`, `config.h.in`,
`cmake/` (compiler/profile/utils/pch/mdbx-config), `conanfile.py`, plus
Windows-specific assembly sources (`windows-safeseh-*`) and `valgrind.supp`.

Documentation and metadata: `README.md` (full documentation),
`ChangeLog.md`, `LICENSE`, `NOTICE`, `COPYRIGHT`, `SECURITY.md`,
`FUNDING.json`, `CONTRIBUTING.md`, `VERSION.json`, `AGENTS.md` (this file).

Distributed documentation site content: `docs/en/` and `docs/ru/` —
guides, textbook ("deep dive" articles), overview and tooling pages; the
full C/C++ API reference is generated from the headers and published on
the website; `man1/` — man pages for every command-line tool.

Runnable examples: `examples/` — C and C++ sample programs with their own
CMake build.

## 2. Building

### CMake (recommended)

```sh
cmake -S . -B build        # add -DMDBX_BUILD_CXX=ON for the C++ API
cmake --build build
```

Useful options: `MDBX_BUILD_CXX` (enable the C++ API), `MDBX_BUILD_TOOLS`
(standalone tools), `MDBX_BUILD_SHARED_LIBRARY` (shared vs static),
`INTERPROCEDURAL_OPTIMIZATION`, and `MDBX_CONFIG_H` to point to a generated
configuration header.

> **Note:** `MDBX_ENABLE_TESTS` in this package does **not** add the unit
> test suite (the test framework lives only in the upstream development
> repository); it enables building `examples/`. The unit test framework is
> compiled only in the non-amalgamated development tree.

### GNU make

```sh
make mdbx-static.o     # static library object (or: make mdbx-dylib.o)
make mdbx_chk          # build a specific tool
```

The result is the library (`libmdbx.a`, `libmdbx.so`/`.dylib`/`.dll`) plus
the tools.

## 3. Testing in this package

The unit test suite is **not** part of this package — it resides in the
upstream development repository. What is available here:

- build and run the `examples/` programs (they double as smoke checks of
  basic operations);
- `mdbx_chk <dbfile>` — verify a database file after your integration
  scenarios;
- the C API status codes: every function returns `int` (`MDBX_result`);
  get a human-readable message with `mdbx_strerror(rc)`.

For real test coverage of a change, use the upstream
[libmdbx-devel](https://github.com/Mithril-mine/libmdbx-devel) repository.

## 4. Using the C API

Include `mdbx.h` and link the library. The model:

- **Environment** (`MDBX_env`) — one database file (or a directory of files).
- **DBI** (`MDBX_dbi`) — a named key/value map inside the environment.
- **Transactions** (`MDBX_txn`) — parallel readers never block a writer;
  writers are serialized. Open read transactions with `MDBX_TXN_RDONLY`.
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
- **Data integrity**: run `mdbx_chk` to verify a database; make consistent
  copies with `mdbx_copy` or the online backup API (`mdbx_env_copy2...`).
- **Environment size is dynamic** (auto-grow/shrink); set geometry explicitly
  via `mdbx_env_set_geometry()` when you need predictable sizing.
- **Keep read transactions short**: they prevent reclaiming of discarded
  pages, so long-lived read txns can grow the file.

## 5. Using the C++ API

Built with `MDBX_BUILD_CXX=ON`; include `<mdbx.h++>`. Everything lives in
namespace `mdbx` with RAII handles (`env`/`env_managed`, `txn`/`txn_managed`,
`cursor`/`cursor_managed`) and value helpers (`buffer`, `slice`). Run the
shipped `examples/example-mdbx.c++` for a working start.

Note: the C++ API reference documents the public surface; the internal
split into `decl-*.h++`/`impl-*.h++` headers exists only in the upstream
development repository — here it is amalgamated into the single
`mdbx.h++`.

## 6. Local documentation

The package ships the documentation site content:

- `docs/en/` — English guides, textbook, overview and tooling pages
  (`docs/en/reference/` links to the online C/C++ API reference; offline,
  the `mdbx.h`/`mdbx.h++` headers are self-documented); `docs/ru/` — the
  Russian counterpart.
- `man1/*.1` — man pages: `mdbx_stat`, `mdbx_copy`, `mdbx_dump`,
  `mdbx_load`, `mdbx_chk`, `mdbx_drop`, `mdbx_defrag`.
- `README.md` — characteristics, platform notes, durability/sync modes,
  gotchas, building.

## 7. Contributing & upstream

- This package is generated; **changes are not accepted here** except
  fixes that apply exclusively to the amalgamated form (e.g. clarifications
  of this document) — see [CONTRIBUTING.md](CONTRIBUTING.md).
- Report issues with reproduction steps against the amalgamated package via
  the issue tracker; fixes land in the upstream
  [libmdbx-devel](https://github.com/Mithril-mine/libmdbx-devel) repository
  and reach this package in the next release.
- For security vulnerabilities **do not** use the issue tracker — see
  [SECURITY.md](SECURITY.md) for how to report one.
- Never edit the generated `mdbx.c`/`mdbx.c++` in place — such edits are
  overwritten by the next amalgamation.

## 8. Further reading

- `docs/en/index.md` / the documentation site:
  <https://libmdbx.dqdkfa.ru/docs/en/index.html> — guides, textbook
  ("deep dive" articles on architecture and internals), "first steps".
- C and C++ API reference: <https://libmdbx.dqdkfa.ru/docs/en/reference/>.
- `README.md` — full documentation.
- `examples/example-mdbx.c` and `examples/example-mdbx.c++` — runnable
  examples.
- License: Apache-2.0 (see `LICENSE`/`NOTICE`/`COPYRIGHT`). Note that the
  test suite of the upstream repository is under a separate non-free
  license and is **not** part of this package.
