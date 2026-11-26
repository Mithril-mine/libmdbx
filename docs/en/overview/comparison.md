# Comparison with other databases

> Related: [Overview](index.md) · [Restrictions & gotchas](restrictions.md) ·
> [Durability modes](../getting-started/durability-modes.md) ·
> [Improvements over LMDB](../deep-dive/improvements.md)

_libmdbx_ is an embedded transactional key-value database. This page puts it
next to the engines it is usually compared with: its closest relatives —
[LMDB](https://en.wikipedia.org/wiki/Lightning_Memory-Mapped_Database) and
[bbolt](https://github.com/etcd-io/bbolt) (BoltDB) — and the anchor
alternatives from other classes — [SQLite](https://www.sqlite.org/) and
[RocksDB](https://rocksdb.org/). The axes are the ones people actually choose
by: licensing, durability model, concurrency, limits, compactification,
tooling, platform support.

## Summary table

| | libmdbx | LMDB | bbolt | SQLite | RocksDB |
| --- | --- | --- | --- | --- | --- |
| License | Apache-2.0 | OpenLDAP (BSD-like) | MIT | public domain | Apache-2.0 or GPLv2 |
| Data model | key-value + multimap tables, B+tree | key-value, B+tree | key-value (buckets), B+tree | relational, SQL | key-value (column families), LSM |
| Storage | memory-mapped B+tree, shadow paging, no WAL | same | single-file B+tree, copy-on-write, no WAL | B-tree + rollback journal or WAL | LSM + WAL, background compactions |
| Durability | modes from `MDBX_DURABLE` to `MDBX_UTTERLY_NOSYNC`; integrity guaranteed in all but `UTTERLY_NOSYNC` | sync on every commit; `MDB_NOSYNC` is destructive on crash | sync on every commit; `NoSync` option | journaling modes incl. WAL; `PRAGMA synchronous` | configurable WAL syncing |
| Readers | non-blocking, snapshots, linear scaling across CPUs | non-blocking, snapshots | snapshots (read transactions) | snapshots in WAL mode | snapshots |
| Writers | one, fully serialized, deadlocks impossible | one, serialized | one, serialized | one; in WAL mode does not block readers | concurrent writing from several threads |
| Multi-process access | yes | yes | no (exclusive file lock) | yes (file locking) | no |
| Max key | ≈½ page: 2022 B (4 KiB) … 32,742 B (64 KiB) | ≈511 B (half the system page) | 32,768 B | no keys per se (BLOBs up to 10⁹ B) | not fixed |
| Max value | ≈2 GiB (`0x7FF00000`) | bounded by the mmap size | ≈2 GiB (2³¹−2) | 10⁹ B by default | not fixed |
| Max database size | ≈8 TiB (4 KiB) / ≈128 TiB (64 KiB); auto-grow on the fly | fixed at open (10 MiB by default), up to 128 TiB on 64-bit | auto-grow within the address space | up to 281 TiB | multi-terabyte class |
| Compactification | continuous zero-overhead + explicit defragmentation | only a compacted full copy | only rewriting via the `bbolt compact` tool | `VACUUM` (full rewrite) or `auto_vacuum` | background compactions |
| Tools | `mdbx_chk`, `mdbx_stat`, `mdbx_copy`, `mdbx_dump`, `mdbx_load`, `mdbx_defrag` | `mdb_stat`, `mdb_copy`, `mdb_dump`, `mdb_load` | CLI: `info`, `compact`, `sweep` | `sqlite3` CLI and a huge ecosystem | `ldb`, `sst_dump`, etc. |
| Platforms | Linux, Windows, macOS, iOS, Android, HarmonyOS, FreeBSD, Solaris, OpenBSD, NetBSD (POSIX.1-2008) | Unix-like, Windows, AIX, Solaris | Linux, macOS, Windows (Go) | virtually all | Linux, macOS, Windows |

The characteristic quantitative parameters of libmdbx are in the
[Overview](index.md#limitations-summary); behavioral limitations — in
[Restrictions & gotchas](restrictions.md).

## LMDB

The closest relative: historically _libmdbx_ is a deeply reworked descendant
of LMDB, today an independent codebase. The full catalogue of differences is
in [Improvements over LMDB](../deep-dive/improvements.md); in short, what
matters when choosing:

- LMDB requires the database size up front and never changes it (the default
  is a mere 10 MiB); libmdbx auto-grows on the fly with configurable geometry;
- longer keys: ≈½ of the page with a tunable page size up to 64 KiB,
  versus ≈511 B in LMDB;
- the libmdbx file format does not depend on word size or byte order
  (a file created on a 32-bit big-endian system reads fine on x86-64);
- the asynchronous `MDBX_SAFE_NOSYNC` mode keeps integrity after a crash
  (rollback to the last steady state), whereas LMDB's `MDB_NOSYNC` can destroy
  the database;
- continuous zero-overhead compactification and defragmentation instead of
  "pour it into a compacted copy" as in LMDB;
- stable cursor navigation over deletions — the cursor-positioning edge cases
  inherited from LMDB are fixed;
- diagnostics (`mdbx_chk`), stale-reader protection, a complete type-safe C++
  wrapper, and [dozens more improvements](../deep-dive/improvements.md).

The API is similar in spirit; migrating from LMDB usually does not change the
application logic.

## bbolt (BoltDB)

[bbolt](https://github.com/etcd-io/bbolt) carries the LMDB ideas to Go
(the etcd heritage). Same family — copy-on-write B+tree without WAL, one
writer, snapshot readers. The key differences from libmdbx:

- only one process per database: an exclusive file lock, no shared access by
  a swarm of processes;
- the file never shrinks by itself: compactification only via a separate tool;
- excluding disk I/O, libmdbx is roughly 3 times faster.

## SQLite

If you need SQL and the relational model, SQLite has no rival in this class.
But for plain key-value access it is heavier: SQL parsing on every query,
copying data between the application and the library instead of direct mmap
access. WAL mode gives concurrent reads with a single writer; compactification
is `VACUUM` — a full rewrite of the file (or incremental `auto_vacuum` at the
price of overhead).

## LevelDB / RocksDB

The LSM family (LevelDB, RocksDB, etc.) is optimized for write-intensive
workloads and large volumes on slow media: writes go to a memtable and a WAL,
background threads compact the data. The price is read/space amplification,
background threads and compaction threads, a noticeable amount of code and
dependencies, and no multi-process access. RocksDB allows concurrent writing
from several threads, but that does not make it lightweight.

libmdbx versus LSM is the classic reads-versus-writes trade: minimal read
overhead, no background threads, and the full set of transactional guarantees
— against the LSM advantages on write-heavy workloads. Note that without a
WAL the write volume of libmdbx is proportional to the changed pages, and on
fast NVMe the write gap behind LSM is usually small.

## Performance

Any numbers depend on the workload profile — measure your own. Verified
reference points: excluding disk I/O time, _libmdbx_ is roughly 3 times faster
than BoltDB, and in specific extreme cases — e.g. repeated reads of "hot" keys
through the [shared lock-free cache](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__crud.html) —
up to 10–100,000 times faster than both BoltDB and LMDB.

## How to choose

- SQL, relational model, complex queries needed — **SQLite**.
- Terabytes under write-intensive load on slow media, background compactions
  and a noticeable footprint acceptable — **RocksDB**.
- Maximum read speed, minimal overhead, access from several processes, no
  background threads — **libmdbx** (or LMDB, if its limitations — fixed size,
  ≈511 B keys, no compactification — are not critical for you).
- Already have LMDB-based code — _libmdbx_ will almost certainly fit without
  changing the logic, lifting some of the limitations.

---

Competitor characteristics are given per their official documentation at the
time of writing and change over time — verify with the primary sources when
choosing.
