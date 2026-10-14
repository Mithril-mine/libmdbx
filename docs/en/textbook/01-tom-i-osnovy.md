# Volume I. Fundamentals

> **Level:** for beginners — those who have never worked with embedded key-value databases.
> **Volume goal:** you understand what libmdbx is, can build it, open a database, and perform
> your first read and write operations in transactions.
> **End-to-end project:** across Volumes I–II we build a single application — an **application
> configurator** (a simple key-value parameter store that gradually gains indexes,
> multithreading, and optimizations).
>
> The volume's progression: concept → practice → mechanism → nuance. The example code is complete
> and compilable; the helper functions (`env_open` in C++ / `ex_env_open` in C, plus `check_rc`
> and `die`) are shared across the whole textbook (see `examples/common/`).

---

## Chapter 1. What is libmdbx

### 1.1. Embedded database

Databases come in two fundamentally different kinds: **client-server** and **embedded**.

A client-server database (PostgreSQL, MySQL) is a separate server process. Your application talks
to it over the network or a socket: it sends SQL and receives answers. The server owns the data
files and manages the cache, concurrent access, and the journal. You pay for this with deployment
complexity, a separate process, and network overhead.

An embedded database (SQLite, libmdbx) is a **library** that you link directly into your process.
There is no server and no network: your application calls functions that read and write the file
directly. The advantages are simplicity (one library), speed (no network layer), and easy
distribution (the application carries its own database). The price — you are responsible for
concurrent access and backups yourself.

**libmdbx** is an embedded transactional **key-value** database, a deeply reworked descendant of
the well-known LMDB library. "Transactional" means that a group of operations is executed
atomically: either all changes are applied, or none are.

> **Analogy.** SQLite is an "embedded PostgreSQL". libmdbx is an "embedded high-performance
> cache server": it does not know SQL or tables in the relational sense, but it can deliver data
> at memory-access speed.

### 1.2. Key-value data model

A key-value database stores simple pairs: **key** → **value**. Both parts are just byte sequences
whose meaning only your application knows.

```
"user:1001"  →  {"name": "Anna", "role": "admin"}
"user:1002"  →  {"name": "Boris", "role": "user"}
"theme"      →  "dark"
```

How libmdbx differs from an "ordinary" dictionary (std::map, HashMap):

- **Keys are always ordered.** Data is stored in a balanced tree (B+tree), so keys can be
  traversed in order, ranges searched, and the "nearest greater key" found. This is like an array
  with sorted keys rather than a hash table.
- **Values can be large** — up to ~2 GB (they go onto separate "overflow" pages).
- **Data lives in a memory-mapped file.** Reading a key is effectively dereferencing a pointer,
  without copying and without serialization.

### 1.3. Why mmap: memory as the data interface

Most databases maintain their own buffer cache: when reading they copy a page from disk into the
library's memory; when writing they accumulate changes in this cache and periodically flush them
to disk.

libmdbx takes a different approach: a **memory-mapped file** (mmap). The operating system maps
the database file directly into the process's address space. The data looks like an ordinary byte
array. Reading is an access by address; the kernel pulls in the page as needed (page cache).
The library has no separate buffer cache — the OS page cache plays that role.

Three important properties follow from this:

1. **Reading does not copy data.** You get a pointer straight into the mmap region — no
   deserialization. This is the source of the record-breaking read speed (typically millions of
   operations per second).
2. **Writes go through Copy-on-Write (CoW).** When changing a page, the library does not touch
   the old version (other transactions may be reading it) but creates a new copy. Details are in
   Volume III.
3. **The operating system itself decides which pages to keep in memory.** The library does not
   choose an eviction strategy — the kernel does.

### 1.4. libmdbx's place in the landscape

| Database           | Model                     | Architectural properties                                                                    |
| ------------------ | ------------------------- | ------------------------------------------------------------------------------------------- |
| **LMDB**           | KV, mmap, MVCC            | Progenitor of the approach; simpler, fewer features                                         |
| **libmdbx**        | KV, mmap, MVCC            | Reworked LMDB: more durability modes, GC instead of a free-list, stronger guarantees        |
| **BerkeleyDB**     | KV, classic               | Older generation; page-level locking, no mmap                                               |
| **LevelDB/RocksDB**| KV, LSM                   | Optimized for writes, background compaction, poor read predictability                       |
| **SQLite**         | Relational, embedded      | SQL, transactions; slower on KV workloads                                                   |

Here **MVCC** (multi-version concurrency control) is the technique that lets readers and writers
proceed simultaneously without blocking readers: every transaction sees an immutable snapshot of
the data (details in chapter 5 and Volume III). **LSM** (log-structured merge-tree) is the
alternative write-optimized architecture that trades away read predictability.

Comparison at the level of properties (not benchmarks): if you need an ordered key-value store
with transactions, lightning-fast reads, and predictable writes — choose libmdbx. If you need
SQL — SQLite. If writes dominate reads and you can tolerate background compaction — RocksDB.

### 1.5. License, ecosystem, who uses it

libmdbx is distributed under a permissive license: historically — the OpenLDAP Public License
(OLPL); on master — **entirely Apache-2.0** (re-licensed in 0.13.x). It permits commercial use
without opening your own code.

The library is used in high-load projects:

- **The Ethereum ecosystem**: Erigon, Akula, Silkworm (all fast Ethereum implementations run on
  libmdbx);
- **Isar** — the local database for Flutter applications;
- messengers, analytics, cache layers.

The binding ecosystem covers Rust, Go, Python, Node.js, .NET, C++, Zig, Dart, Nim, Java,
Haskell, Ruby, Scala (in detail — Volume VI).

### 1.6. Summary of chapter 1

- libmdbx is an embedded transactional key-value database (a library, not a server).
- Keys are ordered (B+tree); values are arbitrary bytes up to ~2 GB.
- Data is memory-mapped (mmap): copy-free reads, Copy-on-Write writes.
- Properties: wait-free readers, a single writer, transactions, no WAL (recovery via meta pages).
- Apache-2.0 (master)/OLPL (historically); used in Ethereum clients, Isar, and other projects.

### 1.7. Exercises

1. How does a client-server database differ from an embedded one? Give an example where the
   embedded one is preferable.
2. What does key ordering provide beyond an ordinary dictionary?
3. Why can reading through mmap be faster than through a database's own cache?

### 1.8. Chapter 1 checklist

- [ ] I can explain the difference between client-server and embedded databases and say when each is preferable;
- [ ] I understand that libmdbx is an ordered key-value engine (B+tree), not a hash table and not SQL;
- [ ] I can describe the role of mmap: copy-free reads, Copy-on-Write writes, page eviction handled by the kernel;
- [ ] I know the key properties: MVCC, wait-free readers, a single writer, no WAL;
- [ ] I remember the license (Apache-2.0 on master) and examples of libmdbx users (Erigon, Isar).

### 1.9. What's next

It is time to turn this understanding of "what libmdbx is" into working code. In chapter 2 we will
build the library two ways (amalgamation and CMake), link it into a project, and run the first full
"hello, libmdbx" example. For the end-to-end configurator project this step is mandatory: without a
built library and a verified open → put → get cycle there is no moving forward.

---

## Chapter 2. Installation and first run

### 2.1. Building from source

libmdbx can be built in two ways:

- **Amalgamated** (amalgamation): a single `mdbx.c` file plus the `mdbx.h` header, which you can
  simply drop into your project. Minimal dependencies, ideal for embedding.
- **Full source**: a directory with `CMakeLists.txt`, tests, examples, and utilities (`mdbx_stat`,
  `mdbx_chk`, `mdbx_copy`, `mdbx_dump`, `mdbx_load`, `mdbx_defrag`).

Amalgamation: download `mdbx.h` and `mdbx.c` and add `mdbx.c` to your project's build. Nothing
more is required — this is a true "single-file" engine.

### 2.2. Adding to a project

**CMake** (full source):

```cmake
add_subdirectory(libmdbx)
target_link_libraries(yourapp PRIVATE mdbx)
```

or **pkg-config**:

```sh
pkg-config --cflags --libs libmdbx
```

Building it yourself:

```sh
make -j
make install  # installs libmdbx.a / libmdbx.so, mdbx.h, .pc
```

**Static vs dynamic.** Static linking is easier to distribute (no .so versioning problems) and
gives more optimization opportunities. Dynamic linking lets you update the engine without
rebuilding the application.

### 2.3. Key build options

| Option                  | Meaning                                                                                                                   |
| ----------------------- | -------------------------------------------------------------------------------------------------------------------------- |
| `MDBX_LOCKING`          | Locking implementation: `POSIX2008` (Linux, default), `POSIX2001`, `SYSV` (macOS default), `WIN32FILES` (Windows)          |
| `MDBX_CHECKING`         | Level of internal checks/assertions (−1…3). `2`/`3` are very slow — for debugging only                                     |
| `MDBX_FORCE_ASSERTIONS` | Deprecated option (equivalent to `MDBX_CHECKING=2`); prefer `MDBX_CHECKING`                                                |
| `MDBX_ENABLE_BIGFOOT`   | Chains of GC records for very large frees; enabled by default on 64-bit builds                                             |
| `MDBX_DEBUG`            | Logging/assertions/audit (0 by default)                                                                                    |
| `MDBX_VALIDATION`       | Additional structural checks on every operation                                                                            |
| `MDBX_ENABLE_PGOP_STAT` | Page operation counters (for diagnostics)                                                                                  |

> **Tip:** for production, build with `MDBX_CHECKING=0` (or omit it entirely) and
> `NDEBUG`. A debug build with assertions can be several times slower and mask real timings.

### 2.4. Minimal example

The first full listing of the textbook. It opens a database, writes a key, and reads it back.

```c
#include <stdio.h>
#include <string.h>
#include <mdbx.h>

static void die(const char *what, int rc) {
    fprintf(stderr, "%s: %s (%d)\n", what, mdbx_strerror(rc), rc);
    exit(1);
}

int main(void) {
    MDBX_env *env = NULL;
    MDBX_txn *txn = NULL;
    MDBX_dbi dbi;
    MDBX_val key, data;
    int rc;

    /* 1. Create the environment */
    rc = mdbx_env_create(&env);
    if (rc != MDBX_SUCCESS) die("env_create", rc);

    /* 2. Open (or create) the database file; default geometry */
    rc = mdbx_env_open(env, "./demo.mdbx",
                       MDBX_NOSUBDIR, 0664);
    if (rc != MDBX_SUCCESS) die("env_open", rc);

    /* 3. Read-write transaction */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc != MDBX_SUCCESS) die("txn_begin", rc);

    /* 4. Open the main table (NULL name = main) */
    rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
    if (rc != MDBX_SUCCESS) die("dbi_open", rc);

    /* 5. Write "greeting" -> "hello, libmdbx" */
    const char *k = "greeting";
    const char *v = "hello, libmdbx";
    key.iov_len = strlen(k);
    key.iov_base = (void *)k;
    data.iov_len = strlen(v);
    data.iov_base = (void *)v;
    rc = mdbx_put(txn, dbi, &key, &data, 0);
    if (rc != MDBX_SUCCESS) die("put", rc);

    /* 6. Commit */
    rc = mdbx_txn_commit(txn);
    if (rc != MDBX_SUCCESS) die("txn_commit", rc);

    /* 7. Read in a new read-only transaction */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) die("txn_begin-ro", rc);
    rc = mdbx_get(txn, dbi, &key, &data);
    if (rc != MDBX_SUCCESS) die("get", rc);
    printf("Value: %.*s\n", (int)data.iov_len, (char *)data.iov_base);
    mdbx_txn_abort(txn);

    mdbx_env_close(env);
    return 0;
}
```

Let's walk through it step by step.

- `mdbx_env_create` creates an empty environment object — a "connection" to the future database.
- `mdbx_env_open` opens the database file; creation is controlled by the `mode` parameter: a
  non-zero `0664` means "create if the database does not exist yet". Do not pass `MDBX_CREATE`
  here — it is for `mdbx_dbi_open()`, and its bit coincides with `MDBX_NOMETASYNC`.
  `MDBX_NOSUBDIR` means the file name is given directly rather than as a prefix for the
  `data+lock` pair.

> **Important:** the `MDBX_CREATE` bit = `0x40000` is the same bit as `MDBX_NOMETASYNC`. If you
> pass `MDBX_CREATE` in the `flags` of `mdbx_env_open()`, the database will be created (because
> of the non-zero `mode`), but the application will **silently** get weakened durability
> (`NOMETASYNC`). Creation is enabled only via `mode != 0`.

- All changes happen **inside a transaction** — before commit they are not visible to other
  readers.
- `mdbx_dbi_open(txn, NULL, 0, &dbi)` opens the **main table** (in detail — chapter 3).
- `mdbx_put` stores a pair; `mdbx_get` reads it.
- `MDBX_val` is a structure `{ iov_base, iov_len }`: a pointer and the length of the byte window.

**Fragment from [`examples/c++/01-hello.c++`](examples/c++/01-hello.c++)** — the same open → put → get → close cycle on the C++ API: RAII objects (`env_managed`, transactions) close the environment themselves, and keys and values are passed as `mdbx::slice`:

```cpp
#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common/common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-01-hello.mdbx";
    mdbx::env::remove(path); // erase leftovers from the previous run

    // Creation: the constructor with create_parameters creates the DB if absent.
    mdbx::env_managed env(path, mdbx::env_managed::create_parameters(), mdbx::env::operate_parameters());
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr); // main table
      txn.insert(table, mdbx::slice("key"), mdbx::slice("hello, libmdbx"));
      txn.commit();
    }
    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::cout << "got: " << rtxn.get(table, mdbx::slice("key")).as_string() << "\n";
    }
```

Full code: [01-hello.c++](examples/c++/01-hello.c++) · [C version](examples/c/01-hello.c)

### 2.5. Building and running the tests

In the full source:

```sh
cmake -S . -B build -DMDBX_ENABLE_WERROR=ON
cmake --build build -j
ctest --test-dir build
```

The stochastic stress test `mdbx_test` with a fixed `--prng-seed` is useful — it is reproducible
and helps debug rare errors (in detail in Volume V, chapter 30).

### 2.6. Nuance: data and lock files

One "database" physically consists of **two files**: the data (`demo.mdbx`) and the lock file
(`demo.mdbx-lck`). The lock file contains the writer mutex and the reader table — interprocess
infrastructure. Do not delete `.lck` while the database is open; when copying a database,
remember the file pair (in detail — Volume III, chapter 19).

> **Examples for this chapter:** [`examples/c++/01-hello.c++`](examples/c++/01-hello.c++) · [C version](examples/c/01-hello.c);
> end-to-end project: [`config-store-02.c++`](examples/config-store/config-store-02.c++).

### 2.7. Summary of chapter 2

- Amalgamation (`mdbx.c`+`mdbx.h`) or full source with CMake.
- Key build options: `MDBX_LOCKING`, `MDBX_CHECKING`, `MDBX_ENABLE_BIGFOOT`.
- Minimal cycle: create → open → txn_begin → dbi_open → put/get → commit → close.
- The database is a file pair: data + locks.
- Enable debug checks deliberately — they slow things down.

### 2.8. Exercises

1. Build the minimal example and run it twice — make sure the value survives the restart.
2. Add a second key and read both in a single transaction.
3. What happens if you pass `mode = 0` on the first run (when the database does not exist yet)?

### 2.9. Chapter 2 checklist

- [ ] I can build the library two ways: as an amalgamation (`mdbx.c` + `mdbx.h`) and from the full source with CMake;
- [ ] I know the key build options (`MDBX_LOCKING`, `MDBX_CHECKING`, `MDBX_ENABLE_BIGFOOT`) and why production builds want `MDBX_CHECKING=0` and `NDEBUG`;
- [ ] I can build and run the minimal example and make sure the value survives a restart;
- [ ] I know the minimal cycle: create → open → txn_begin → dbi_open → put/get → commit → close;
- [ ] I remember that a database is a file pair (data + `.lck`) and never delete the lock file while the database is open.

### 2.10. What's next

The next chapter is about what you actually write and read: the basic data model of libmdbx. We will
cover `MDBX_val`, the main and named tables (`MDBX_dbi`), limits, integer keys, and zero-length
keys/values. These concepts define the schema of the end-to-end configurator project: which tables
to create and what to store in them.

---

## Chapter 3. Basic data model

### 3.1. MDBX_val: key and value as byte strings

All data in libmdbx is byte sequences. Neither type, nor encoding, nor structure interests the
engine. A window onto such data is `MDBX_val`:

```c
typedef struct MDBX_val {
    size_t iov_len;   /* length in bytes */
    void  *iov_base;  /* pointer to data */
} MDBX_val;
```

Important: `iov_base` usually points **directly into the mmap region** of the database.
Therefore:

- the value can be read without copying;
- but after the transaction ends the pointer may become invalid — copy the data if you intend to
  keep it longer than the transaction.

> **Nuance (C++): `mdbx::slice` and `mdbx::buffer`.** In the C++ API two types play the role of
> `MDBX_val`. `mdbx::slice` is a non-owning view (a `std::string_view`-like window over
> `MDBX_val`): it merely references bytes owned elsewhere and never copies them. The constructor
> from `std::string` is `explicit` for a reason: `mdbx::slice(std::to_string(k))` written inline
> in a call argument is safe, because the temporary string lives until the end of the
> full-expression (i.e. until the whole `insert()`/`get()` call — which copies the bytes into the
> database — completes), whereas storing such a slice in a variable and using it later is a
> dangling reference. `mdbx::buffer` is the opposite, an owning container: by default it
> **copies** the content (the `make_reference=true` overloads fall back to `slice` behavior), so
> `mdbx::buffer(std::to_string(k))` puts no lifetime requirements on the source.

### 3.2. Main table and named tables (dbi)

Inside a single database file live **several trees**. Each tree is a "table" (in API terms —
`MDBX_dbi`, map / sub-DB). There are:

- the **main table** — descriptors of all other tables (name → tree root, flags, counters);
- **named tables** — your data;
- service ones (for example, the GC tree for free pages — in detail in Volume III).

Opening a table in a transaction:

```c
/* main table */
mdbx_dbi_open(txn, NULL, 0, &dbi_main);
/* named table (created on first open with MDBX_CREATE) */
mdbx_dbi_open(txn, "users", MDBX_CREATE, &dbi_users);
```

`MDBX_dbi` is not a global object but a **binding to a table within a specific transaction**:
the handle is valid where it was opened and is inherited by nested transactions. Using the handle
from another transaction/thread is an error (`MDBX_BAD_DBI`).

> **Nuance: reopening a table.** For an already-existing table `mdbx_dbi_open()` requires its
> persistent flags (`MDBX_INTEGERKEY`, `MDBX_DUPSORT`, `MDBX_DUPFIXED`, …); a mismatch yields
> `MDBX_INCOMPATIBLE`. If the flags are unknown in advance (the table may have been created by
> other code), open it with the `MDBX_DB_ACCEDE` flag: the table opens with its actual flags, and you
> can discover them via `mdbx_dbi_flags()`. An analogous `MDBX_ACCEDE` flag exists for
> `mdbx_env_open()` — it opens a database already used by another process in an unknown mode without
> an `MDBX_INCOMPATIBLE` error (Volume II, chapter 9).

### 3.3. Limits

| Parameter | Value                                                                                       |
| --------- | -------------------------------------------------------------------------------------------- |
| Key       | up to ~half a page (depends on the page size and table mode); integer keys — 8 bytes         |
| Value     | up to `MDBX_MAXDATASIZE` ≈ 2 GB                                                              |
| Tables    | `MDBX_MAX_DBI` = 32765                                                                       |
| Readers   | configurable (`mdbx_env_set_maxreaders`), up to `MDBX_READERS_LIMIT` = 32767                 |

The page size is chosen when the database is created: 256…65536 bytes, 4096 by default, and it
**does not change** afterwards. The choice affects performance (Volume IV).

The engine reports the actual limits through the `mdbx_limits_*` family — they depend on the page
size and the build, so prefer functions over hardcoded numbers: `mdbx_limits_dbsize_min()`,
`mdbx_limits_dbsize_max()`, `mdbx_limits_keysize_min()`, `mdbx_limits_keysize_max()`,
`mdbx_limits_valsize_min()`, `mdbx_limits_valsize_max()`, `mdbx_limits_pgsize_min()`,
`mdbx_limits_pgsize_max()`, `mdbx_limits_txnsize_max()`, `mdbx_limits_pairsize4page_max()`,
`mdbx_limits_valsize4page_max()`. Handy aliases for individual values —
`mdbx_env_get_maxkeysize(_ex)` and `mdbx_env_get_maxvalsize_ex`.

### 3.4. Integer keys and native byte order

For numeric keys use the special types: `MDBX_INTEGERKEY` (a table with integer keys
`uint32_t`/`uint64_t` in **native byte order**) and `MDBX_INTEGERDUP` (integer multivalues).
Native byte order means: the key `42` in memory as a `uint64_t` — without byte swapping. This is
critical for comparisons and cursors.

A non-standard sort order is set by a **comparator** — a callback of type `MDBX_cmp_func`. Custom
key/value comparators are passed when opening a table in the extended variants
`mdbx_dbi_open_ex()`/`mdbx_dbi_open_ex2()` (the `keycmp`/`datacmp` arguments). The standard
comparators for binary keys/values are `mdbx_cmp()`/`mdbx_dcmp()`; pick the one suitable for a given
set of table flags via `mdbx_get_keycmp(flags)`/`mdbx_get_datacmp(flags)`.

### 3.5. Zero-length keys and values (difference from LMDB)

Unlike LMDB, libmdbx allows **empty keys** (length 0) and **empty values** (length 0). This is
convenient for markers and presence flags. An empty value is `iov_len == 0` (the pointer may be
NULL). Distinguish between "no key" (`MDBX_NOTFOUND`) and "the key exists with an empty value"
(`MDBX_SUCCESS` with `iov_len == 0`).

**Fragment from [`examples/c++/02-data-model.c++`](examples/c++/02-data-model.c++)** — integer keys (`key_mode::ordinal`) and a zero-length value (`mdbx::slice()`):

```cpp
    // Main table (name == nullptr): usual keys and values.
    auto main = txn.open_map(nullptr);
    txn.insert(main, mdbx::slice("key"), mdbx::slice("value"));
    std::cout << "main[key] = " << txn.get(main, mdbx::slice("key")).as_string() << "\n";

    // Named table: a separate key space within the same DB.
    auto named = txn.create_map("named", mdbx::key_mode::usual, mdbx::value_mode::single);
    txn.insert(named, mdbx::slice("config"), mdbx::slice("42"));
    std::cout << "named[config] = " << txn.get(named, mdbx::slice("config")).as_string() << "\n";

    // Integer keys: a table with key_mode::ordinal (MDBX_INTEGERKEY),
    // keys are uint64_t in native byte order.
    auto ord = txn.create_map("ordinal", mdbx::key_mode::ordinal, mdbx::value_mode::single);
    const uint64_t k1 = 1, k2 = 1000;
    txn.insert(ord, buffer::key_from_u64(k1), mdbx::slice("one"));
    txn.insert(ord, buffer::key_from_u64(k2), mdbx::slice("thousand"));
    std::cout << "ordinal[1000] = " << txn.get(ord, buffer::key_from_u64(k2)).as_string() << "\n";

    // An empty key (zero length) and an empty value.
    txn.insert(main, mdbx::slice(), mdbx::slice("empty-key-value"));
    txn.insert(named, mdbx::slice("empty"), mdbx::slice());
    std::cout << "empty value length = " << txn.get(named, mdbx::slice("empty")).size() << "\n";
```

Full code: [02-data-model.c++](examples/c++/02-data-model.c++) · [C version](examples/c/02-data-model.c)

> **Examples for this chapter:** [`examples/c++/02-data-model.c++`](examples/c++/02-data-model.c++) · [C version](examples/c/02-data-model.c).

### 3.6. Summary of chapter 3

- `MDBX_val` = a byte window `{len, ptr}`; the pointer may live in mmap.
- One file — many tables (trees); the main table stores the descriptors.
- `MDBX_dbi` — a binding to a table in a specific transaction.
- Limits: key ~½ page, value ~2 GB, tables 32765.
- Integer keys — native byte order; empty keys and values are allowed.

### 3.7. Exercises

1. Create two named tables ("settings", "counters") and put a key into each.
2. Try to read a key from a handle opened in another transaction — what does the API return?
3. Check the behavior with an empty value: put `iov_len=0`, read it back — how do you tell an
   "empty value" from "no key"?

### 3.8. Chapter 3 checklist

- [ ] I understand that `MDBX_val` is a byte window `{iov_len, iov_base}` with no type or encoding;
- [ ] I can open the main and named tables via `mdbx_dbi_open()` and know the role of `MDBX_CREATE`;
- [ ] I understand that `MDBX_dbi` is valid only in its own transaction, and remember `MDBX_BAD_DBI` and `MDBX_DB_ACCEDE`;
- [ ] I check limits through the `mdbx_limits_*` functions rather than hardcoded numbers;
- [ ] I know why `MDBX_INTEGERKEY` exists (native byte order) and how to tell an empty value (`iov_len == 0`) from a missing key (`MDBX_NOTFOUND`).

### 3.9. What's next

Now that the data model is clear, we can operate on it. Chapter 4 covers the basic CRUD set —
`mdbx_put`, `mdbx_get`, `mdbx_del`, `mdbx_replace` — with write flags, handling of expected codes,
and a full example. These operations form the core of `cfg_set`/`cfg_get` in the end-to-end
configurator.

---

## Chapter 4. Basic CRUD operations

### 4.1. mdbx_put — insert and update

```c
int mdbx_put(MDBX_txn *txn, MDBX_dbi dbi,
             const MDBX_val *key, const MDBX_val *data, unsigned flags);
```

Flags:

| Flag                | Meaning                                              |
| ------------------- | ---------------------------------------------------- |
| `0` / `MDBX_UPSERT` | Insert or replace the value                          |
| `MDBX_NOOVERWRITE`  | Insert only if the key is absent; otherwise `MDBX_KEYEXIST` |
| `MDBX_NODUPDATA`    | (DUPSORT) do not add a duplicate value               |
| `MDBX_CURRENT`      | Update the cursor's current value                    |

### 4.2. mdbx_get — read by key

```c
int mdbx_get(MDBX_txn *txn, MDBX_dbi dbi,
             const MDBX_val *key, MDBX_val *data);
```

Returns `MDBX_SUCCESS` (found) or `MDBX_NOTFOUND` (no such key). For DUPSORT tables you need
`MDBX_GET_BOTH` (to search for a specific value) — in detail in Volume II, chapter 7.

The extended variant `mdbx_get_ex(txn, dbi, &key, &data, &values_count)` additionally returns the
**number of values for the key** (1 for a regular table; for DUPSORT — how many values are associated
with the key; handy without a separate cursor and `mdbx_cursor_count()`).

### 4.3. mdbx_del — deletion

```c
int mdbx_del(MDBX_txn *txn, MDBX_dbi dbi,
             const MDBX_val *key, const MDBX_val *data);
```

If `data == NULL` — the whole key is deleted. If data is given and the table is DUPSORT — the
specific value is deleted.

**Fragment from [`examples/c++/03-crud.c++`](examples/c++/03-crud.c++)** — write flags on the C++ API: `insert` corresponds to `MDBX_NOOVERWRITE` (on a repeat it throws `mdbx::key_exists`), `upsert` — `MDBX_UPSERT`, `update` — `MDBX_CURRENT`. The flags `MDBX_NODUPDATA`/`MDBX_ALLDUPS` for multivalues are in Volume II, chapters 6–7:

```cpp
    // NOOVERWRITE: inserting a unique key.
    txn.insert(table, mdbx::slice("key"), mdbx::slice("value1"));

    // Re-inserting the same key -> key_exists (MDBX_KEYEXIST).
    try {
      txn.insert(table, mdbx::slice("key"), mdbx::slice("value2"));
      std::cerr << "FAIL: duplicate insert did not throw\n";
      return EXIT_FAILURE;
    } catch (const mdbx::key_exists &) {
      std::cout << "insert duplicate -> MDBX_KEYEXIST as expected\n";
    }

    // try_insert does not throw; it reports the result via the done flag.
    auto ins = txn.try_insert(table, mdbx::slice("key"), mdbx::slice("value2"));
    std::cout << "try_insert duplicate -> done=" << (ins.done ? 1 : 0) << "\n";

    // UPSERT: insert or overwrite.
    txn.upsert(table, mdbx::slice("key"), mdbx::slice("value2"));
    std::cout << "after upsert: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";

    // CURRENT (update): update only an existing key.
    txn.update(table, mdbx::slice("key"), mdbx::slice("value3"));
    std::cout << "update existing: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";
```

Full code: [03-crud.c++](examples/c++/03-crud.c++) · [C version](examples/c/03-crud.c)

### 4.4. mdbx_replace — conditional replacement

`mdbx_replace` combines put+get into a single operation: replace a value, learn the old one,
insert only if the current value matches. It is used for atomic "compare-and-swap" without extra
lookups.

```c
int mdbx_replace(MDBX_txn *txn, MDBX_dbi dbi,
                 const MDBX_val *key, const MDBX_val *new_data,
                 const MDBX_val *old_data /* may be NULL */,
                 MDBX_val *old_value /* may be NULL */, unsigned flags);
```

### 4.5. Reading: mdbx_get vs a cursor

`mdbx_get` is a point lookup by exact key. If you need to traverse data, find a range, or the
"nearest greater key" — use a cursor (Volume II, chapter 6). For exact reads `mdbx_get` is
enough.

### 4.6. Full example: CRUD

```c
#include <stdio.h>
#include <string.h>
#include <mdbx.h>

static void die(const char *what, int rc) {
    fprintf(stderr, "%s: %s (%d)\n", what, mdbx_strerror(rc), rc);
    exit(1);
}

int main(void) {
    MDBX_env *env = NULL; MDBX_txn *txn = NULL; MDBX_dbi dbi;
    MDBX_val key, data, out;
    int rc;

    rc = mdbx_env_create(&env);
    if (rc) die("create", rc);
    rc = mdbx_env_open(env, "./crud.mdbx", MDBX_NOSUBDIR, 0664);
    if (rc) die("open", rc);
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc) die("txn", rc);
    rc = mdbx_dbi_open(txn, "kv", MDBX_CREATE, &dbi);
    if (rc) die("dbi", rc);

    /* insert */
    const char *k1 = "alpha", *v1 = "1";
    key.iov_base = (void *)k1; key.iov_len = strlen(k1);
    data.iov_base = (void *)v1; data.iov_len = strlen(v1);
    rc = mdbx_put(txn, dbi, &key, &data, MDBX_NOOVERWRITE);
    if (rc != MDBX_SUCCESS && rc != MDBX_KEYEXIST) die("put", rc);

    /* update */
    const char *v2 = "2";
    data.iov_base = (void *)v2; data.iov_len = strlen(v2);
    rc = mdbx_put(txn, dbi, &key, &data, 0); /* UPSERT */
    if (rc) die("put2", rc);

    /* get */
    out.iov_base = NULL; out.iov_len = 0;
    rc = mdbx_get(txn, dbi, &key, &out);
    if (rc) die("get", rc);
    printf("alpha = %.*s\n", (int)out.iov_len, (char *)out.iov_base);

    /* replace: read the old value and write the new one */
    MDBX_val old = {0, NULL};
    const char *v3 = "3";
    data.iov_base = (void *)v3; data.iov_len = strlen(v3);
    rc = mdbx_replace(txn, dbi, &key, &data, NULL, &old, 0);
    if (rc) die("replace", rc);
    printf("old value: %.*s\n", (int)old.iov_len, (char *)old.iov_base);

    /* del */
    rc = mdbx_del(txn, dbi, &key, NULL);
    if (rc) die("del", rc);

    /* verify the deletion */
    rc = mdbx_get(txn, dbi, &key, &out);
    printf("get after deletion: %s (%d)\n",
           rc == MDBX_NOTFOUND ? "NOTFOUND (expected)" : "unexpected", rc);

    mdbx_txn_commit(txn);
    mdbx_env_close(env);
    return 0;
}
```

> **Warning:** `old_value` from `mdbx_replace` and `out` from `mdbx_get` point into mmap. Do
> not keep these pointers after `commit`/`abort` — the data may be reused.

### 4.7. Error handling

Return codes: `MDBX_SUCCESS` (0) — success; `MDBX_RESULT_TRUE`/`MDBX_RESULT_FALSE` — special
results of some operations; negative `MDBX_*` — errors.

Common "expected" codes (these are not failures but normal responses):

- `MDBX_KEYEXIST` — the key already exists (with `MDBX_NOOVERWRITE`);
- `MDBX_NOTFOUND` — the key (or value) was not found;
- `MDBX_BAD_DBI` — invalid/closed table handle.

String description: `mdbx_strerror(rc)`. There is also a thread-safe variant `mdbx_strerror_r()` —
use it in multithreaded code (Volume II, chapter 12).

> **Examples for this chapter:** [`examples/c++/03-crud.c++`](examples/c++/03-crud.c++) · [C version](examples/c/03-crud.c);
> end-to-end project: [`config-store-04.c++`](examples/config-store/config-store-04.c++).

### 4.8. Summary of chapter 4

- put/get/del/replace — basic operations; flags control behavior on conflicts.
- `mdbx_get` — point reads; for traversal/ranges — cursors.
- `MDBX_KEYEXIST`/`MDBX_NOTFOUND` are expected states, not errors.
- `mdbx_replace` — atomic "read old + write new" replacement in a single call.
- Pointers into mmap are valid only within a transaction.

### 4.9. Exercises

1. Add "expected error" handling: insert a key again with `NOOVERWRITE` and handle
   `MDBX_KEYEXIST` without exiting the program.
2. Write a loop that inserts 10,000 keys in a **single** transaction and measures the time.
3. What does `mdbx_del` do with the `data` argument in an ordinary (non-DUPSORT) table?

### 4.10. Chapter 4 checklist

- [ ] I can perform all the basic operations: `mdbx_put`, `mdbx_get`, `mdbx_del`, and `mdbx_replace`;
- [ ] I understand the difference between the write flags `MDBX_NOOVERWRITE`, `MDBX_UPSERT`, and `MDBX_CURRENT`, and choose them deliberately;
- [ ] I treat `MDBX_KEYEXIST` and `MDBX_NOTFOUND` as normal states, not failures;
- [ ] I understand that `mdbx_replace` is an atomic "compare and swap" in a single call;
- [ ] I remember that pointers into mmap (`out`, `old_value`) must not be used after commit/abort.

### 4.11. What's next

You can already perform single operations, but the real power of libmdbx is in transactions.
Chapter 5 introduces begin/commit/abort, the difference between read-only and read-write
transactions, and the MVCC intuition — why readers never block the writer. For the configurator
this is the foundation: atomic writes of groups of settings and safe concurrent access to the
database.

---

## Chapter 5. Transactions — first steps

### 5.1. What a transaction is and why you need it

A transaction is a group of operations executed **atomically** and **in isolation**:

- either all changes are applied, or none are (atomicity);
- until the transaction is committed, its changes are not visible to others (isolation).

Example: a money transfer — debit account A and credit account B. If you do this as two separate
operations, you can lose data between them on a failure. In a transaction — it is safe.

### 5.2. begin / commit / abort

```c
int mdbx_txn_begin(MDBX_env *env, MDBX_txn *parent,
                   unsigned flags, MDBX_txn **txn);
int mdbx_txn_commit(MDBX_txn *txn);
int mdbx_txn_abort(MDBX_txn *txn);
```

- `mdbx_txn_begin(env, NULL, flags, &txn)` — start a transaction. `flags == 0` (same as
  `MDBX_TXN_READWRITE`) — read-write; for a read-only transaction pass `MDBX_TXN_RDONLY`.
- `mdbx_txn_commit(txn)` — apply the changes (for write) or simply close (for read).
- `mdbx_txn_abort(txn)` — discard the changes and close.

> **Tip:** always close a transaction — either commit or abort. An unclosed read transaction can
> "freeze" page reclamation (Volume III, chapter 16).

### 5.3. Read-only vs read-write

**Read-only transactions** do not block each other and do not require the writer mutex. You can
have many of them at once, in any threads.

There is **one** **read-write transaction** in the database at any moment (the global writer
mutex). If two threads try to start a write transaction at the same time — one waits for the
other.

### 5.4. Why readers do not block each other (MVCC intuitively)

libmdbx uses **MVCC** (multiversion concurrency control). Simplified:

1. Every transaction sees a "snapshot" of the database — the state at the moment it started.
2. A write transaction does not modify existing pages but creates **new versions**
   (Copy-on-Write).
3. A reader that started earlier keeps seeing the old versions as long as its snapshot is alive.

Therefore readers never conflict either with each other or with the writer. The price — old page
versions take up space while old snapshots are alive (in detail — Volume III, chapters 14 and
16).

**Fragment from [`examples/c++/04-transactions.c++`](examples/c++/04-transactions.c++)** — a read-only observer in a separate thread sees a stable MVCC snapshot while a write transaction overwrites the key and then does an `abort` (rollback):

```cpp
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      txn.commit();
    }

    // Read-only observer in a separate thread: a fixed snapshot of the database.
    std::thread observer(observer_run, std::ref(env));
    while (stage.load() != 1)
      std::this_thread::yield();

    // The write transaction overwrites the key but does not commit yet.
    {
      auto wtxn = env.start_write();
      auto table = wtxn.open_map(nullptr);
      wtxn.upsert(table, mdbx::slice("key"), mdbx::slice("v2"));
      stage = 2; // the observer checks the snapshot with an uncommitted write
      while (stage.load() != 3)
        std::this_thread::yield();
      wtxn.abort(); // rollback: v2 is not saved
    }
```

Full code: [04-transactions.c++](examples/c++/04-transactions.c++) · [C version](examples/c/04-transactions.c)

### 5.5. Basic rule: one transaction — one thread

A transaction is bound to the thread that created it ("sticky thread"). You cannot use a
transaction object from another thread — that is a discipline violation (`MDBX_THREAD_MISMATCH`).
However, there is the `MDBX_NOSTICKYTHREADS` mode for thread pools and coroutines (Volume II,
chapter 11).

### 5.6. Full example with transactions

```c
#include <stdio.h>
#include <string.h>
#include <mdbx.h>

static void die(const char *what, int rc) {
    fprintf(stderr, "%s: %s (%d)\n", what, mdbx_strerror(rc), rc);
    exit(1);
}

int main(void) {
    MDBX_env *env = NULL; MDBX_txn *txn = NULL; MDBX_dbi dbi;
    MDBX_val key, data, out;
    int rc;

    rc = mdbx_env_create(&env);
    if (rc) die("create", rc);
    rc = mdbx_env_open(env, "./txn.mdbx", MDBX_NOSUBDIR, 0664);
    if (rc) die("open", rc);

    /* ---- Write transaction: two keys atomically ---- */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc) die("txn_w", rc);
    rc = mdbx_dbi_open(txn, "kv", MDBX_CREATE, &dbi);
    if (rc) die("dbi", rc);

    const char *ka = "a", *va = "1";
    const char *kb = "b", *vb = "2";
    key.iov_base = (void *)ka; key.iov_len = strlen(ka);
    data.iov_base = (void *)va; data.iov_len = strlen(va);
    mdbx_put(txn, dbi, &key, &data, 0);
    key.iov_base = (void *)kb; key.iov_len = strlen(kb);
    data.iov_base = (void *)vb; data.iov_len = strlen(vb);
    mdbx_put(txn, dbi, &key, &data, 0);
    rc = mdbx_txn_commit(txn);
    if (rc) die("commit", rc);

    /* ---- Read-only transaction ---- */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc) die("txn_r", rc);
    out.iov_base = NULL; out.iov_len = 0;
    key.iov_base = (void *)ka; key.iov_len = strlen(ka);
    rc = mdbx_get(txn, dbi, &key, &out);
    if (rc) die("get", rc);
    printf("a = %.*s\n", (int)out.iov_len, (char *)out.iov_base);
    mdbx_txn_abort(txn); /* read-only: just close it */

    /* ---- Abort: changes are discarded ---- */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc) die("txn_w2", rc);
    const char *kc = "c", *vc = "3";
    key.iov_base = (void *)kc; key.iov_len = strlen(kc);
    data.iov_base = (void *)vc; data.iov_len = strlen(vc);
    mdbx_put(txn, dbi, &key, &data, 0);
    mdbx_txn_abort(txn); /* c will not appear in the database */

    /* verify that "c" is absent */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc) die("txn_r2", rc);
    rc = mdbx_get(txn, dbi, &key, &out);
    printf("after abort: %s\n", rc == MDBX_NOTFOUND ? "no key (correct)" : "unexpected");
    mdbx_txn_abort(txn);

    mdbx_env_close(env);
    return 0;
}
```

> **Keep reading after commit — `mdbx_txn_commit_embark_read()`.** A commit in libmdbx
> "seats" the transaction on a fresh snapshot (the detent — a reader held on a committed
> snapshot; the mechanism is detailed in Volume III); the dedicated function
> `mdbx_txn_commit_embark_read()` does the same and immediately returns a **new read-only
> transaction on the already-committed state** — with no "commit → new begin" gap. Useful for
> read-your-writes patterns and queue processing (Volume V, chapter 32). The transaction identifier
> (the ordinal number in the database) is available via `mdbx_txn_id()`.

### 5.7. End-to-end project: the beginning

Our application configurator. The idea: store settings as "section → key → value" tables.

```c
/* config_store.h — the end-to-end project skeleton (chapters 2-5) */
#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H
#include <mdbx.h>

typedef struct {
    MDBX_env *env;
    MDBX_dbi  main;
} config_store_t;

int cfg_open(config_store_t *cs, const char *path);
int cfg_set(config_store_t *cs, const char *key, const char *value);
int cfg_get(config_store_t *cs, const char *key, char *buf, size_t buflen);
int cfg_close(config_store_t *cs);

#endif
```

The implementation comes in the following chapters; by the end of Volume II this will be a
full-fledged application with indexes and multithreading.

> **Examples for this chapter:** [`examples/c++/04-transactions.c++`](examples/c++/04-transactions.c++) · [C version](examples/c/04-transactions.c);
> end-to-end project: [`config-store-05.c++`](examples/config-store/config-store-05.c++).

### 5.8. Summary of chapter 5

- A transaction = an atomic, isolated group of operations.
- begin (0/`MDBX_TXN_READWRITE` — write; `MDBX_TXN_RDONLY` — read-only), commit, abort.
- MVCC: readers see a snapshot and do not block the writer.
- One transaction — one thread (by default).
- Unclosed read transactions "freeze" page reclamation.

### 5.9. Exercises

1. Write a `cfg_set` function that uses `MDBX_NOOVERWRITE` for creation and `0` for updates —
   how do you combine them in a single transaction?
2. Measure: how many write transactions per second does your disk sustain (without batching)?
3. Why is starting a read-write transaction "just in case" a bad idea (hint: the writer mutex)?

### 5.10. Chapter 5 checklist

- [ ] I understand the transaction properties: atomicity (all or nothing) and isolation;
- [ ] I can start and correctly close every kind of transaction: `MDBX_TXN_READWRITE`, `MDBX_TXN_RDONLY`, `mdbx_txn_abort()`;
- [ ] I understand why there is always a single write transaction and how MVCC lets readers proceed without blocking the writer;
- [ ] I know the rule "one transaction — one thread" and the `MDBX_THREAD_MISMATCH` code;
- [ ] I can demonstrate on an example that abort discards changes and that an unclosed read transaction stalls page reclamation.

### 5.11. What's next

Volume I is complete: you have a working configurator skeleton. Volume II turns it into a
full-fledged application: cursors for traversal and ranges (chapter 6), DUPSORT and secondary
indexes (chapters 7–8), environment configuration (chapter 9), durability modes (chapter 10),
multithreading (chapter 11), and error handling (chapter 12). We start with cursors — without them
you cannot traverse a table or search a range.

---

## Volume summary

You have learned the basic model of libmdbx: what it is, how to build it, how to open a database,
and how to perform CRUD in transactions. Your end-to-end "configurator" project works.

**What's next:** Volume II — cursors, multivalues and DUPSORT, secondary indexes, environment
configuration, durability modes, multithreading, and error handling.
