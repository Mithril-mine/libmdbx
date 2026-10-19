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

## Storing long values

The data lives in a B+tree as "key → value" pairs. A value that does not fit
into a leaf page together with the key and the node header (practically —
longer than roughly half a page) is **not split into parts**: it is stored as
one unbroken piece on consecutive pages (these are the large/overflow pages),
and the leaf keeps only a pointer to the piece, `{pgno, npages}`. Reading such
a piece is direct, without copying and without overhead (maximum read speed).
Mechanics — [internals](../deep-dive/internals.md#7-large-values-and-overflow-pages).

The "unbroken piece" requirement is exactly what grows into the main
limitation:
- **writing** a long value requires finding a *continuous* range ("span") of
  free pages of the required length, not "any free page". The longer the
  value, the longer the needed span and the harder it is to find: with a
  fragmented B+tree the engine has to scan and merge many GC records (the
  "deep search"), may process the whole GC or append pages at the file end,
  and when neither the GC nor the geometry upper bound provides a sequence —
  `MDBX_MAP_FULL`;
- **rewriting** a long value is CoW of the whole span: allocate a new
  contiguous range, copy the data, release the old range. Frequent overwrites
  churn the free space, fragment the file and produce lots of GC records;
- **very long values bloat GC**: one GC record holds about 1000 page numbers
  (~4 MB with a 4 KB page); a value spanning hundreds of thousands of pages
  produces bigfoot chains of dozens/hundreds of records, growing the GC tree;
- **write amplification** grows proportionally to the value size: every commit
  rewrites all the pages of the span, not a single leaf.

**Already available**: the free-sequence search through the GC has been
substantially accelerated (SIMD: NEON/SSE2/AVX2/AVX512); spilling and refund
reduce fragmentation; the auto-growing geometry helps when sequences are
missing.

### Basic advice

Store values so that every value **fits into a single page together with its
key** — i.e. not longer than `mdbx_limits_valsize4page_max()`. Such values do
not create the "span problem" at all. Additionally:
- enlarge the page (up to 64 KB) when values are "on average" large;
- monitor the DB with `mdbx_env_stat()` (overflow page counts) and
  `mdbx_chk` histograms (`large_pages`, `val_len`) to detect churn;
- when the free space is fragmented, use **defragmentation**
  (`mdbx_env_defrag()` / `mdbx_defrag`);
- tune `rp_augment_limit`/`gc_time_limit` for the write pattern
  (uncontrolled file growth vs "stuck" commits);
- keep "heavy" data (images, binary objects) out of the DB, storing only
  references in values (e.g. content-addressed keys).

### Splitting long values into chunks

If a long value still has to live in the DB and does not fit into one page, it
can be **explicitly split into chunks** not exceeding
`mdbx_limits_valsize4page_max()` (see also `mdbx_limits_valsize_max()` for the
per-table-flag limits). Two common approaches:

1. **In a plain table** — the key `K` gets a suffix with the chunk number:
   `K#0`, `K#1`, `K#2`, … A prefix search over `K` finds all chunks of the
   value in their order.
2. **In a multivalue (DUPSORT) table** — chunks not longer than
   `mdbx_limits_valsize_max(MDBX_DUPSORT)`; each chunk's data is prefixed with
   the multivalue number (when several long values are stored under one key)
   and the chunk number. A search by key yields all multivalues and their
   chunks, and a prefix search with the multivalue number yields just its
   chunks.

> Both approaches are **manual splitting**, needed only until the streamed API
> for long values ("Streaming BLOBs") planned in the roadmap arrives and
> removes the need for such splitting.

### Roadmap

Besides reworking long-value storage in _MithrilDB_ (where the problem is
solved architecturally), the roadmap also includes a **streaming read/write
mode for long values** — "Streaming BLOBs": an API to read and write long
values in a streamed fashion, without manual chunking. See also
[Volume V, ch. 33](../textbook/05-tom-v-ekspertnye-temy.md) and
[improvements](../deep-dive/improvements.md).

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
