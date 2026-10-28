# libmdbx Overview

> Related: [Installation and building](../getting-started/install-build.md) ·
> [First steps](../getting-started/first-steps.md) · [Restrictions and gotchas](restrictions.md) ·
> [Architecture](../deep-dive/architecture.md) ·
> [Textbook: Volume I "Fundamentals"](../textbook/01-tom-i-osnovy.md) ·
> [Improvements](../deep-dive/improvements.md)

_libmdbx_ is an extremely fast, compact, powerful, embedded, transactional
[key-value database](https://en.wikipedia.org/wiki/Key-value_database) with the
[Apache-2.0 license](https://www.apache.org/licenses/LICENSE-2.0), focused on
building unique lightweight solutions.

1. Allows **a swarm of multi-threaded processes to
   [ACID](https://en.wikipedia.org/wiki/ACID)ly read and update** several
   key-value [maps](https://en.wikipedia.org/wiki/Associative_array) and
   [multimaps](https://en.wikipedia.org/wiki/Multimap) in a locally-shared
   database.

2. Provides **extraordinary performance** with minimal overhead through
   [memory mapping](https://en.wikipedia.org/wiki/Memory-mapped_file) and
   `O(log N)` operation costs by virtue of the
   [B+tree](https://en.wikipedia.org/wiki/B%2B_tree).

3. **Requires no maintenance and no crash recovery** since no WAL is used —
   which may be a caveat for write-intensive workloads with durability
   requirements (see [durability modes](../getting-started/durability-modes.md)).

4. Enforces [serializability](https://en.wikipedia.org/wiki/Serializability)
   for writers through a single mutex and affords
   [wait-free](https://en.wikipedia.org/wiki/Non-blocking_algorithm#Wait-freedom)
   reads for parallel readers, while **readers and writers do not block each
   other**.

5. **Guarantees data integrity after a crash** unless explicitly neglected in
   favor of write performance ([durability modes](../getting-started/durability-modes.md)).

6. Supports Linux, Windows, macOS, HarmonyOS, Android, iOS, FreeBSD, Solaris,
   NetBSD, OpenBSD and other systems compliant with **POSIX.1-2008**.

7. **Compact and friendly for full embedding** — a few flat source files, no
   internal threads and no server process; implements the core of the Berkeley
   DB API with many powerful extensions.

Historically, _libmdbx_ is a deeply revised descendant of the legendary
[LMDB](https://en.wikipedia.org/wiki/Lightning_Memory-Mapped_Database): it
inherits all LMDB benefits, resolves several of its issues and adds a large
set of [improvements](../deep-dive/improvements.md).

## Key features

- B+tree-based database management library, modeled loosely on the BerkeleyDB
  API, but much simplified: the entire database ("environment") is exposed in a
  memory map, and all data fetches return data directly from the mapped memory
  — **no `malloc`'s or `memcpy`'s occur during data fetches**. The library is
  extremely simple (no page-caching layer of its own), extremely high
  performance and memory-efficient.
- Fully thread-aware: supports concurrent read/write access from multiple
  processes and threads; **writes are fully serialized** — only one write
  transaction may be active at a time, which guarantees that writers can never
  deadlock.
- The memory map is **read-only by default**, providing total immunity to
  corruption from stray pointer writes; read-write mapping offers much higher
  write performance, but adds the possibility for stray application writes
  through pointers to silently corrupt the database.
- The database structure is multi-versioned, so readers run with no locks;
  writers cannot block readers, and readers do not block writers.
- Key-value data model, keys are always sorted; range lookups including range
  query volume estimation.
- Fully [ACID](https://en.wikipedia.org/wiki/ACID) through
  [MVCC](https://en.wikipedia.org/wiki/Multiversion_concurrency_control) and
  [copy-on-write](https://en.wikipedia.org/wiki/Copy-on-write).
- Multiple tables/sub-databases within a single datafile; ultra-efficient
  multimap (dupsort) support: values are sorted, searchable and iterable,
  keys are stored without duplication.
- Data is [memory-mapped](https://en.wikipedia.org/wiki/Memory-mapped_file)
  and accessible zero-copy: full scans are extremely fast; `get(key)` is
  accelerated by a shareable lock-free cache.
- Reads scale linearly across CPUs; reader and writer transactions do not
  block each other; no conflicts and no deadlocks (writes are serialized).
- Online hot backup; an append operation for bulk insertion of pre-sorted
  data.
- Automatic on-the-fly database size adjustment; continuous zero-overhead
  compactification; explicit defragmentation.
- Customizable page size; nested transactions; no WAL and no transaction
  journal — no maintenance required.

## Limitations (summary)

Full treatment — [Restrictions and gotchas](restrictions.md).

| Parameter | Value |
| --- | --- |
| Page size | a power of 2: `256`…`65536`, default `4096` |
| Key size | up to ≈½ pagesize (`2022` bytes for 4K pages, `32742` for 64K); zero length supported |
| Value size | up to `0x7FF00000` bytes for maps; ≈½ pagesize for multimaps |
| Write transaction size | up to `4.94` TiB (4K pages), `79.1` TiB (64K) |
| Database size | up to ≈`8.0` TiB (4K), ≈`128.0` TiB (64K) |
| Maximum tables | `32765` |
| Writers | at most one write transaction at a time |

## Gotchas (summary)

- **B+tree** → page access is mostly random: SSDs provide a significant boost
  for large databases.
- **Shadow paging instead of WAL** → syncing may be the bottleneck for
  write-intensive workloads.
- **MVCC + long-lived readers**: heavy data alteration while a long-lived read
  transaction is active increases the working set and may exhaust the free
  space. Avoid long reads; use
  [transaction parking](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__transactions.html)
  and the [Handle-Slow-Readers callback](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__err.html)
  — details in [restrictions](restrictions.md).
- Extraordinary speed requires care: a simple linear scan may beat complex
  indexes, and suboptimal code shows its cost only on large data.

## Comparison with other databases

_libmdbx_ is superior to the legendary _LMDB_ in features and reliability while
not being inferior in performance (the detailed catalog —
[Improvements](../deep-dive/improvements.md)). Briefly, versus the class of embedded
stores:

- a database can be shared by multiple processes without multi-process issues;
- no problems moving cursors after deletion;
- zero-overhead compactification: a database file can be shrunk/truncated;
- excluding disk I/O time — up to ≈3× faster than BoltDB and up to
  10–100,000× faster than BoltDB/LMDB in specific edge cases (reproducible
  with [ioArena](https://sourcecraft.dev/dqdkfa/ioarena), `make bench-quartet`,
  including RocksDB and WiredTiger comparisons);
- even more features compared to BoltDB and/or LMDB.

## When to choose libmdbx

✅ A good fit: a local embedded store with ACID guarantees; many readers with
moderate writes; zero-maintenance and out-of-the-box crash-safety; multi-process
access within one machine; huge multimaps and range queries.

⚠️ Think twice: network/distributed use (libmdbx is not a network database);
write-intensive workloads with strict durability demands on slow storage
(fsync is the bottleneck — see [durability modes](../getting-started/durability-modes.md));
long-lived read transactions alongside heavy updates (see
[restrictions](restrictions.md)).

## Bindings

The full list (several dozen) — the
[Bindings and Projects](https://libmdbx.dqdkfa.ru/#sec-projects) section of the
home site; the most demanded: Rust
([libmdbx-rs](https://github.com/vorot93/libmdbx-rs)),
Go ([mdbx-go](https://github.com/torquem-ch/mdbx-go)),
Python ([PyPi/libmdbx](https://pypi.org/project/libmdbx/)),
.NET ([libmdbx-dotnet](https://public.git.amsoft.spb.ru/libmdbx/libmdbx-dotnet)),
NodeJS ([mdbxmou](https://github.com/ikonopistsev/mdbxmou)) and others.

## Roadmap and the future

- **MithrilDB** — the next-generation engine that removes the architectural
  limitations inherited from LMDB: a common API for several storage formats,
  reworked long-value handling, fundamental GC/freelist improvements, and no
  more trouble with long-lived readers. See the [Textbook: Chapter 33 «Roadmap
  and the future: MithrilDB»](../textbook/05-tom-v-ekspertnye-temy.md) and the
  [Roadmap](../overview/restrictions.md#roadmap) section.
- **Roadmap presentation** — the annotated slide-deck
  «[_libmdbx_: successes, obstacles, goals and roadmap](https://libmdbx.dqdkfa.ru/release/libmdbx-roadmap-HNY2026-english.pdf)»
  with important explanations embedded as comments (a
  [Russian version](https://libmdbx.dqdkfa.ru/release/libmdbx-roadmap-HNY2026-russian.pdf)).
- **TODO** — the current open directions are tracked in
  [`TODO.md`](https://sourcecraft.dev/dqdkfa/libmdbx/browse/TODO.md): new APIs,
  WASM, bindings, tracing, and more.
- **SkyNET** (Mission «Concordia Intellectuum») — an experimental AI model, a
  coordinated hierarchical swarm/harness, used in the development and
  maintenance of _libmdbx_.
