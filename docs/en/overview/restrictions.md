# Restrictions and gotchas

> Related: [Overview](index.md) · [First steps](../getting-started/first-steps.md) ·
> [Durability modes](../getting-started/durability-modes.md) ·
> [Textbook: Volume V "Expert topics"](../textbook/05-tom-v-ekspertnye-temy.md)

The quantitative parameter limits are listed in the
[Overview](index.md#limitations-summary); this page covers behavioral
limitations, operational risks and their mitigations.

## MVCC and "long-lived" readers

The most important operational nuance of libmdbx. The mechanics: every write
transaction builds a new version of the changed pages (MVCC on a copy-on-write
basis); readers work with snapshots and are never blocked. Pages freed by newer
write transactions cannot be reused while at least one active older snapshot
refers to them. Therefore **heavy data alteration in parallel with a
long-running read** grows the working set and may quickly exhaust the free
space (`MDBX_MAP_FULL`).

Typical sources of long readers: a hot backup to a slow destination, debugging
a client application with an active read transaction, suspended
(SUSPEND/ptrace) processes holding an open read.

Mitigations — **already available**:

- **Transaction parking** — `mdbx_txn_park()`/`mdbx_txn_unpark()`: a parked
  reader is marked with a pseudo-TID, stops affecting the reuse watermark and
  can be evicted (ousted) with an automatic restart upon access.
- **Handle-Slow-Readers** — `mdbx_env_set_hsr()`: the callback fires when the
  database is full because of lagging readers; it may wait, asynchronously
  abort the reader transaction (return 1) or acknowledge the process death
  (return 2+).
- **`MDBX_LIFORECLAIM`** — LIFO reuse shortens the page-circulation cycle and
  mitigates the consequences on write-back storage.
- **Automatic eviction of parked readers** before space exhaustion (together
  with establishing a new steady snapshot).
- Read secondary data from a secondary copy of the database.

**Roadmap**: the radical fix of the architectural limitation (page reuse
independent of old MVCC snapshots — nonsequential GC recycling) belongs to the
next engine generation (_MithrilDB_), where the long-lived readers problem is
solved completely; partial improvements are planned in the upcoming libmdbx
releases too.

Avoid: suspending/blocking threads performing reads (including debugging) and
aborting processes with active reads (libmdbx cleans the reader slots
automatically, but the check cannot be run too often without a performance
penalty).

## Large values

- Values up to `0x7FF00000` bytes are stored in chains of consecutive pages —
  reading is direct, without copying or overhead.
- The downside: **writing** a large value requires finding a sufficient
  sequence of free pages, which can be expensive or impossible with a
  fragmented B+tree (the engine may process the whole GC or grow the file).
  Maximum read speed at the cost of write speed.
- **Already available**: the free-sequence search through the GC has been
  substantially accelerated (SIMD: NEON/SSE2/AVX2/AVX512); spilling and refund
  reduce fragmentation; the auto-growing geometry helps when sequences are
  missing.
- **Roadmap**: the rework of large-value storage and fragmentation bypass —
  in _MithrilDB_ (where the problem is solved architecturally) and in the
  refinements of the upcoming libmdbx releases.

## Huge transactions

A similar situation with transactions freeing many pages: the list of retired
pages is itself a long value requiring a sequence of free pages.

**Already available**: the **"Big Foot"** feature (enabled by default) — large
PNL lists are split into chains of records with consecutive txnids, avoiding
the search, allocation and storage of long sequences; see
`MDBX_ENABLE_BIGFOOT`. Additionally, the LIFO policy and refund shrink the GC
bookkeeping volume.

**Roadmap**: addressed in _MithrilDB_ — where retired-page lists fit the
architecture without searching for free sequences.

## Address space reservation

A database configuration often reserves a considerable amount of **virtual**
address space and (potentially) file size for future growth — this consumes no
real memory or disk. But on 64-bit systems with relatively small RAM an
inadequately large `size_upper` via `mdbx_env_set_geometry()` may deplete the
system resources (`ENOMEM`). Do not set the upper bound "just in case".

## Remote filesystems

- Do not place databases on remote filesystems — even between processes of the
  same host: this breaks file locks on some platforms and memory-map
  synchronization.
- Exceptions: exclusive-mode operation over a network and cooperative
  read-only access to databases on read-only network shares.

## Child processes and fork()

- Do not use an opened environment in a child after `fork()`. The best way is
  to have no open libmdbx instances at the `fork()` moment; the `MDBX_ENV_CHECKPID`
  build option (ON by default on non-Windows) detects such errors earlier but
  gives no guarantees.
- Since v0.13.1, `mdbx_env_resurrect_after_fork()` is **already available** —
  reuse of an already opened environment in the child, strictly without
  inheriting any transactions from the parent.
- Never call `fork()` simultaneously from multiple threads.

## Read-only mode

There is no "pure" read-only mode: readers need write access to the LCK file
to be visible to the writer. libmdbx always tries to open/create the LCK in
read-write; on errors (`EROFS`, `EACCES`, `EPERM`) with `MDBX_RDONLY` requested
it switches to the without-LCK mode.

## One thread — one transaction

A thread can use only one transaction at a time (plus nested write transactions
outside WRITEMAP). Every transaction belongs to one thread; a write transaction
is finished in the thread that started it. Violations return
`MDBX_TXN_OVERLAPPING`, `MDBX_BAD_RSLOT`, `MDBX_BUSY`. The
`MDBX_NOSTICKYTHREADS` option allows transaction hand-off between threads —
only for those who know exactly what they are doing (deadlocks, reading
alien data).

## Do not open twice

Reopening the same database within one process is forbidden: without OFD
locks POSIX file locks are used, and they break when one process opens a file
multiple times (closing one instance drops all locks). libmdbx tracks and
prevents this (`MDBX_BUSY`); the LMDB compatibility escape hatch is
`mdbx_setup_debug(MDBX_DBG_LEGACY_MULTIOPEN, ...)` with lock recovery
(possible pauses).

## Troubleshooting the LCK file

The LCK file is always cleared when the first process starts working with the
database: for any LCK damage it is enough to close the database in all
processes (or stop them) and restart.

- **LCK corruption** is possible when a misbehaving application writes through
  pointers into the shared memory; there is no portable protection (the LCK is
  updated concurrently and lock-free — locking would cost too much).
- **Stale readers** (left by aborted programs) make the database grow quickly;
  libmdbx checks for them at environment open and before database growth.
- **Stale writers** are cleaned automatically (platform-dependent, via robust
  mutexes); no known issues on the supported platforms.
