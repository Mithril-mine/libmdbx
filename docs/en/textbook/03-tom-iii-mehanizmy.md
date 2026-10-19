# Volume III. Internal Mechanisms

> **Level:** for those who understand architecture; a bridge between "how to use" and "how to
> configure and debug".
> **Goal of the volume:** you explain libmdbx's behavior through its internals: B+tree and mmap,
> MVCC, commit pipeline, GC, geometry, nested transactions, lock file, and recovery.

---

## Chapter 13. Storage Architecture: B+tree and mmap

### 13.1. B+tree: Branches, Leaves, Overflow

libmdbx stores data in a **B+tree** — a balanced tree where all values live in leaves, while
internal nodes (branch) contain only separator keys and pointers to child pages.

```
          [branch: M]
        /             \
 [branch: B]          [branch: V]
   /    \                /    \
[a..b] [c..m]        [n..r] [s..z]     <- leaves: key→value pairs
```

Properties:

- search — descent from root to leaf, O(log N);
- all leaves at the same level are linked — range traversal goes through neighbors;
- on leaf overflow — **split** (halving), recursively upward, including a new root;
- on emptying below the threshold — **rebalance/merge** with a neighbor (threshold `merge_threshold`).

Values that do not fit in a leaf (larger than roughly half a page) go to **overflow pages**: a
contiguous run of `P_LARGE` pages; the leaf keeps a pointer `{pgno, npages}`.

When the value does not fit even a single overflow page (`npages > 1`), the run must be a sequence
of **consecutive** pages of the needed length. This is the main practical point: allocating such a
run is costlier than allocating any single free page (the engine may scan and merge GC records or
append at the file end), and rewriting the value rewrites the whole run (CoW of all its pages,
fragmentation, GC churn). The problem can be softened by splitting the long value into chunks that
fit into a single page (`mdbx_limits_valsize4page_max()`); manual splitting is needed only until
the planned **streamed API for long values** ("Streaming BLOBs") arrives. Details and mitigations —
[Restrictions & gotchas](https://libmdbx.dqdkfa.ru/docs/en/overview/restrictions.html).

### 13.2. Why B+tree Rather than B-tree or LSM

- **B-tree** — data in both leaves and branches: a search may stop at any level, but traversal
  requires moving up/down. A B+tree with data only in leaves gives a predictable descent and
  linear traversal.
- **LSM** (LevelDB/RocksDB) is optimized for writes (append-only + background compaction), but
  reads suffer from multi-level lookups. A B+tree gives honest O(log N) for both reads and writes
  without background processes.

### 13.3. Page — the Unit of Storage and I/O

The page size is 256…65536 bytes (4096 by default), chosen when the database is created and then
fixed. Types:

| Type             | Purpose                                              |
| ---------------- | ---------------------------------------------------- |
| branch           | Internal node: separators and pointers               |
| leaf             | Leaf: key→value pairs                                |
| large / overflow | Contiguous run of large values                       |
| meta             | One of the three meta pages (database state snapshot) |
| dupfix / subpage | Dense packing of multivalues                         |

Every page carries the **transaction label** (`txnid`) of the transaction that created its current
version — this is the basis of CoW and MVCC.

A user-supplied **canary** (`MDBX_canary`, 32 bytes — four `uint64_t`: `x`, `y`, `z`, `v`; the `v`
field is always set to the transaction number) can be associated with the database:
`mdbx_canary_put(txn, &canary)` writes it into the meta, `mdbx_canary_get(txn, &canary)` reads it.
Updated values become visible to other processes only after the commit. This is not table data but an
arbitrary application marker (e.g., a schema version) that survives database opens and is available
even when mounted read-only.

### 13.4. mmap: Virtual Memory as a Window into the File

The file is mapped into the address space in its entirety. Reading is a pointer dereference; the
kernel (page cache) pulls in the page. There is no separate buffer cache in the library.

Consequences:

- copy-free reads (you get an address straight into mmap);
- the OS page cache manages residency;
- **database size is limited by the address space** (on 32-bit — practically ~1–3 GB);
- when the database ≫ RAM, PTE overhead in the kernel grows.

`madvise(MADV_NOHUGEPAGE)` (opting out of THP) is applied to the mmap region — huge pages interfere
with accurate residency estimation.

**Fragment from [`examples/c++/18-tree-height.c++`](examples/c++/18-tree-height.c++)** — tree
estimation via `mdbx_env_stat_ex()`: height, leaf/branch/overflow pages and the number of entries;
a value larger than the page size creates overflow pages:

```cpp
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 5000; ++i)
        txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("v"));
      // A value larger than the page size (4096) goes to overflow pages.
      const std::string big(64 * 1024, 'x');
      txn.insert(table, mdbx::slice("big"), mdbx::slice(big));
      txn.commit();
    }

    const auto stat = env.get_stat(); // mdbx_env_stat_ex()
    std::cout << "stat: depth=" << stat.ms_depth << " leaf=" << stat.ms_leaf_pages
              << " branch=" << stat.ms_branch_pages << " overflow=" << stat.ms_overflow_pages
              << " entries=" << stat.ms_entries << "\n";
```

Full code: [18-tree-height.c++](examples/c++/18-tree-height.c++) · [C version](examples/c/18-tree-height.c).

### 13.5. The Reachability Invariant

Every non-meta page up to the `first_unallocated` pointer is reachable **exactly once**: either it
belongs to some tree (an ordinary, the main, or the GC table), or it is listed in a GC record.
Integrity auditing and the database verification tool (`mdbx_chk`) are built on this invariant.

> **Examples for this chapter:** [`examples/c++/18-tree-height.c++`](examples/c++/18-tree-height.c++) · [C version](examples/c/18-tree-height.c).

### 13.6. Summary of Chapter 13

- B+tree: data in leaves, branches — separators; split/merge maintains the balance.
- Large values — overflow runs of pages.
- mmap instead of a buffer cache: copy-free reads, the cost — address space/PTE.
- The reachability invariant — the foundation of integrity.

### 13.7. Exercises

1. Explain why `mdbx_get` on 64-bit is practically independent of the database size.
2. What changes for a reader if the tree grows from height 3 to height 5?

### 13.8. Chapter 13 checklist

- [ ] I can explain why all values live in the leaves of the B+tree while branches hold only separators, and what this gives for search and range traversal.
- [ ] I know where a value that does not fit in a leaf goes (an overflow run) and what the leaf keeps in its place.
- [ ] I understand why a value spanning several overflow pages needs a contiguous run of pages, and what it costs for allocation and rewriting.
- [ ] I understand why libmdbx does not maintain its own buffer cache, and its consequences (copy-free reads, residency managed by the kernel, PTE growth when the DB ≫ RAM).
- [ ] I can name the address-space limit on database size and its consequence on 32-bit platforms.
- [ ] I know the purpose of the canary (`MDBX_canary`) and that the `v` field always holds the transaction number.
- [ ] I can state the reachability invariant and explain what is built on it (integrity auditing, `mdbx_chk`).

---

## Chapter 14. MVCC and Snapshots

### 14.1. Page Versioning, Reader Snapshot

Each page is marked with the number of the transaction that created its current version. On start,
a reader fixes a snapshot — the state of the database as of the moment the transaction began.
Everything it sees afterwards is a consistent picture of that moment, regardless of subsequent
commits.

### 14.2. Trio of Meta Pages and Two-Phase Commit

The database always has **three meta pages** (`NUM_METAS = 3`). This allows keeping two valid
snapshots ("fresh" and "steady" — with data guaranteed to be flushed to disk) and one trailing
slot for overwriting.

The meta update is **two-phase**: first one half is invalidated (writing the transaction number
while zeroing the second), then the fields are filled in and the second half is validated. A reader
sees either the old intact meta or the new one — never a "half-updated" one.

### 14.3. Finite State Machine of the Trio

The state of each meta is encoded independently; the full transition table covers **216 (6³)**
combinations and is exhaustively checked by internal validation. This protects against "impossible"
states after failures.

### 14.4. Reader Lock Table (RLT)

A reader registers its snapshot number in the **reader lock table** (in the lock file). The slot
contains: the snapshot number, the owner's pid/tid, and fields describing how many pages the reader
"pins".

The oldest active snapshot — the **detent** — is computed by scanning the RLT (lock-free, with
caching). The detent is the watershed for page reuse (chapter 16).

### 14.5. Wait-free Reads and safe64

On read operations the reader **takes no locks** — it simply walks the mmap. The only "writing"
action is registering a slot when the transaction starts.

64-bit fields (transaction numbers in the RLT and in the meta) are read through a special atomic
protocol, **safe64**: values are written "low word first, then high word", and reads are retried
when a "torn" value is detected. This is protection against torn 64-bit reads on weak memory
models.

### 14.6. The Writer: Global Mutex, Front Txnid, Dirty List

At any moment there is **one write transaction** (the global writer mutex in the lock file). On
start, the writer gets a **preliminary number** (`front txnid = txnid + 1`): new and CoW-copied
pages are marked with it and are invisible to readers until commit. Modified pages accumulate in
the **dirty list** (DPL).

**Fragment from [`examples/c++/19-readers-lag.c++`](examples/c++/19-readers-lag.c++)** — a reader
holds a snapshot; the writer's commits increase its "lag", and `mdbx_reader_list` shows reader
slots with txnid and lag:

```cpp
    // Commits after the snapshot was created increase the reader's lag.
    for (int i = 3; i < 8; ++i) {
      auto txn = env.start_write();
      auto wtable = txn.open_map(nullptr);
      txn.insert(wtable, mdbx::slice("k" + std::to_string(i)), mdbx::slice("v"));
      txn.commit();
    }

    // Enumerate the readers: a reader slot has lag > 0.
    struct visitor {
      int operator()(const mdbx::env::reader_info &ri, int) {
        std::cout << "reader slot: pid=" << ri.pid << " tid=" << ri.thread << " txnid=" << ri.transaction_id
                  << " lag=" << ri.transaction_lag << "\n";
        return mdbx::continue_loop;
      }
    } v;
    env.enumerate_readers(v);
```

Full code: [19-readers-lag.c++](examples/c++/19-readers-lag.c++).

> **Examples for this chapter:** [`examples/c++/19-readers-lag.c++`](examples/c++/19-readers-lag.c++).

### 14.7. Summary of Chapter 14

- MVCC: a page is marked with a txnid; a reader sees its own snapshot.
- The meta trio with a two-phase update and a finite state machine of 216 states.
- RLT and detent — the basis of safe reuse.
- Wait-free reads; safe64 against torn reads.
- One writer; front txnid hides uncommitted changes.

### 14.8. Exercises

1. Why exactly three meta pages, and not two?
2. What happens if the writer crashes in the middle of a two-phase meta update?

### 14.9. Chapter 14 checklist

- [ ] I can explain why a reader sees a consistent picture regardless of subsequent commits (the snapshot + the `txnid` label on pages).
- [ ] I know why exactly three meta pages are needed and which two valid snapshots they keep.
- [ ] I can describe the two-phase meta update and explain why a reader never sees a "half-updated" meta.
- [ ] I understand how the detent is computed by scanning the RLT and why it is the watershed for page reuse.
- [ ] I know that reads are wait-free and take no locks, and why the `safe64` protocol is needed.
- [ ] I can explain why the writer's uncommitted pages are invisible to readers (`front txnid`, the dirty list).

---

## Chapter 15. Copy-on-Write and the Commit Pipeline

### 15.1. Page Lifecycle on Write

The page state is determined by comparing its label `mp->txnid` with the transaction number:

| State      | Condition              | Meaning                                                            |
| ---------- | ---------------------- | ------------------------------------------------------------------ |
| frozen     | `txnid < txn->txnid`   | Old version, visible to readers; cannot be changed — only copied   |
| spilled    | `txnid == txn->txnid`  | Already written to disk by the current writer                      |
| shadowed   | `txnid > txn->txnid`   | A newer version exists; this one is obsolete                       |
| modifiable | `txnid == front txnid` | New version of the current writer; can be modified in place        |

The touch mechanism (CoW): if a page is "frozen" — we allocate a new one (from GC or the file
tail), copy the contents, mark it with `front txnid`, put it in the dirty list; the old one is
released. The parent that pointed to the old page also becomes "modifiable" — CoW propagates
upward to the root. Cursors that referred to the old version are moved to the new one.

### 15.2. Dirty Page List (DPL)

Within one transaction a page enters the dirty list **once**, no matter how many times it is
modified — this is the key factor holding back WAF (write amplification factor, the ratio of
bytes written to bytes of useful data; detailed in Volume IV, chapter 21).

### 15.3. Spill

If the volume of dirty pages exceeds the limit (`dp_limit`, default ≈ 1/42 of RAM), **spill** kicks
in: some pages are written to disk ahead of time. Pages referenced by cursors are not subject to
spill; the selection order is governed by divisor options (minimum/maximum share).

> **Nuance:** if a spilled page is then modified again — it will be written once more
> (additional amplification). This is a "memory vs WAF" trade-off.

### 15.4. Loose Pages and Refund

- **Loose pages** — a small cache of dirty pages freed in the current transaction (limit
  `loose_limit`, default 64) for fast reuse without consulting the GC.
- **Refund** — returning freed tail pages to unallocated space (rather than to the GC); reduces
  WAF and enables online shrinkage.

### 15.5. The Commit Pipeline

1. Close/verify cursors.
2. **GC processing**: freed pages are placed into GC-tree records.
3. **Refund**: tail frees are returned to unallocated space.
4. **Spill**: on dirty-list overflow — early write-out.
5. **Audit** (in enhanced checking modes): reconciliation of page sums.
6. **Two-phase meta update** + synchronization according to the durability mode.
7. Release the writer lock, update the detent cache, clean out dead readers.

Each stage is measurable via `mdbx_txn_commit_ex()` → `MDBX_commit_latency`
(`preparation/gc_wallclock/audit/write/sync/ending/whole`).

**Fragment from [`examples/c++/20-commit-latency.c++`](examples/c++/20-commit-latency.c++)** —
collecting `MDBX_commit_latency` by stage via `commit_get_latency()` for a series of commits and
averaging:

```cpp
    for (int i = 0; i < commits; ++i) {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("v"));
      const auto lat = txn.commit_get_latency(); // returns MDBX_commit_latency
      prep += lat.preparation;
      gc += lat.gc_wallclock;
      audit += lat.audit;
      write += lat.write;
      sync += lat.sync;
      ending += lat.ending;
      whole += lat.whole;
    }

    std::cout << "avg latency (us): preparation=" << (prep / commits) << " gc=" << (gc / commits)
              << " audit=" << (audit / commits) << " write=" << (write / commits) << " sync=" << (sync / commits)
              << " ending=" << (ending / commits) << " whole=" << (whole / commits) << "\n";
```

Full code: [20-commit-latency.c++](examples/c++/20-commit-latency.c++).

> **Examples for this chapter:** [`examples/c++/20-commit-latency.c++`](examples/c++/20-commit-latency.c++).

### 15.6. Summary of Chapter 15

- CoW: frozen → copy → modifiable; the path to the root is rewritten entirely.
- DPL deduplicates pages within a transaction — the main lever of WAF.
- Spill — a memory/WAF trade-off; loose and refund reduce writes.
- Commit pipeline: GC → refund → spill → audit → meta → sync → unlock.

### 15.7. Exercises

1. Why does changing a single byte in a leaf rewrite the whole path to the root?
2. How can `mdbx_txn_commit_ex` help see what "got stuck" — spill or sync?

### 15.8. Chapter 15 checklist

- [ ] Given a page's `mp->txnid` label, I can determine its state (frozen/spilled/shadowed/modifiable) and what may be done with it.
- [ ] I can explain why CoW rewrites the path to the root and how cursors are moved to new page versions.
- [ ] I know that a page enters the dirty list once per transaction, and why this holds back WAF.
- [ ] I can describe when spill kicks in (`dp_limit` ≈ 1/42 of RAM), which pages it never touches, and why a re-modified spilled page is bad.
- [ ] I distinguish loose pages (fast reuse, `loose_limit` = 64) from refund (returning tail pages to unallocated space).
- [ ] I can reproduce the order of the commit pipeline and name the stages visible in `MDBX_commit_latency`.

---

## Chapter 16. GC — the Garbage Collector Inside the Database

### 16.1. Why There Is No Free-List

libmdbx has no classic free-page list. Freed pages are accounted for **persistently, inside the
data file**, in a special **GC tree** (a separate table whose root is in the meta).

### 16.2. Format of a GC Record

- **Record key** — the number of the transaction that freed the pages.
- **Value** — a list of page numbers (compressed, as ranges).
- One record is physically limited (~1000 numbers at a 4 KB page).

The GC format is frozen and has not changed since v11.3 — an important compatibility guarantee.

### 16.3. Detent: Safety of Reuse

Pages from a GC record with key `T` can be reused only when `T ≤ detent` — the oldest active
snapshot. Above the detent, records stay "frozen" until the corresponding reader finishes.

The detent is computed by scanning the RLT (lock-free, with caching of the result).

### 16.4. FIFO vs LIFO

| Policy                   | Mechanism                                        | Effect                                                                                                                            |
| ------------------------ | ------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------- |
| **FIFO** (default)       | Pages are taken from the oldest suitable record  | The list lives longer, cools down on disk                                                                                         |
| **LIFO** (`MDBX_LIFORECLAIM`) | The newest frees come first                 | Minimally short circulation cycle; pages are still "warm"; on systems with a write-back cache — several-fold write performance growth |

> **Nuance:** `MDBX_LIFORECLAIM` gives almost no effect with `SAFE_NOSYNC`/`UTTERLY_NOSYNC` — there
> the cycle length is determined by the frequency of `env_sync()`. Vector (SIMD) kernels are used
> to find dense sequences: SSE2/AVX2/AVX512/NEON.

### 16.5. BigFoot

One GC record holds ~1000 page numbers (~4 MB at 4 KB). If a transaction frees more (for example,
when replacing a huge value), the records form a **chain over consecutive txnids** — the BigFoot
mode. Readers must be newer than the whole chain. The price — more records, a taller GC tree.

BigFoot is enabled by default in 64-bit builds (`MDBX_ENABLE_BIGFOOT`); it is fully compatible with
the database format.

### 16.6. Recursiveness of GC Updates

The GC is also a CoW tree: modifying the GC requires allocating pages, which affects the list of
pages that goes into the GC. That is why an operational reserve of free pages is formed before an
update: shortage → irrational database growth, surplus → overhead.

### 16.7. rp_augment_limit and gc_time_limit

Searching for dense sequences (for large values) can be expensive under fragmentation. Two cost
limiters:

- `rp_augment_limit` — the limit of list accumulation during search; exceeding it → it is cheaper
  to append new pages to the file tail;
- `gc_time_limit` — a time limit (in 1/65536 seconds) on the search during a writing transaction.

> **Rule:** decrease `rp_augment_limit` only together with `gc_time_limit`; too small an
> `rp_augment` does not cure GC growth but aggravates it for the sake of long-record insert speed.

**Fragment from [`examples/c++/22-gc-limits.c++`](examples/c++/22-gc-limits.c++)** — setting and
reading GC limits: `rp_augment_limit` (page reserve for reclamation) and `gc_time_limit` (1/65536
seconds per sequence search):

```cpp
    env.set_extra_option(opt::rp_augment_limit, 128 * 1024);
    // gc_time_limit is specified in 1/65536 fractions of a second (16.16 fixed point):
    // 2500 ≈ 38 ms for searching page sequences in the GC within a transaction.
    env.set_extra_option(opt::gc_time_limit, 2500);

    // A load with long values: makes the GC work harder.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 2000; ++i) {
        const auto key = "k" + std::to_string(i);
        const std::string value(4096 + (i % 7) * 512, 'v');
        txn.insert(table, mdbx::slice(key), mdbx::slice(value));
      }
      txn.commit();
      auto del = env.start_write();
      auto dtable = del.open_map(nullptr);
      for (int i = 0; i < 2000; i += 2)
        del.erase(dtable, mdbx::slice("k" + std::to_string(i)));
      del.commit();
    }

    std::cout << "rp_augment_limit = " << env.extra_option(opt::rp_augment_limit) << "\n";
    std::cout << "gc_time_limit = " << env.extra_option(opt::gc_time_limit) << "\n";
```

Full code: [22-gc-limits.c++](examples/c++/22-gc-limits.c++).

### 16.8. Early GC Cleanup (2025) and Non-Deferred Cleanup (devel)

- **Early cleanup (2025)**: recycled GC records start to be processed earlier.
- **Non-deferred cleanup (devel, 0.14.x)**: recycled GC records are removed immediately after
  reading rather than at commit; overhead becomes proportional to the volume of operations. This
  also opened the path to explicit copy-free defragmentation (0.14.2+).

> **Examples for this chapter:** [`examples/c++/21-gc-observe.c++`](examples/c++/21-gc-observe.c++);
> [`examples/c++/22-gc-limits.c++`](examples/c++/22-gc-limits.c++).

### 16.9. Summary of Chapter 16

- GC — a tree of "txnid → page list" records inside the file (not a free-list).
- Reuse only newer than the detent.
- FIFO by default; `MDBX_LIFORECLAIM` — LIFO.
- BigFoot — chains for large frees.
- `rp_augment_limit`/`gc_time_limit` — control over search cost.

### 16.10. Exercises

1. Explain why a long-lived reader "freezes" reclamation, using the concept of the detent.
2. When does LIFO give a win, and when not?

### 16.11. Chapter 16 checklist

- [ ] I can explain why libmdbx has no free-list and where freed pages are accounted for persistently.
- [ ] I describe the format of a GC record: key — the freeing txnid, value — a list of pages as ranges; the format is frozen since v11.3.
- [ ] I can explain the reuse condition via the detent and what happens to GC records above the detent.
- [ ] I know the difference between FIFO and LIFO and when `MDBX_LIFORECLAIM` gives a win and when it does not.
- [ ] I understand the BigFoot mechanism: chains of records over consecutive txnids and the requirement that readers be newer than the whole chain.
- [ ] I can explain why GC updates are recursive (the GC is itself a CoW tree) and why an operational reserve of pages is needed.
- [ ] I know the purpose of `rp_augment_limit` and `gc_time_limit` and the rule of decreasing them together.

---

## Chapter 17. Growth, Shrinkage, and Defragmentation of the Database

### 17.1. Why the Database Grows

- **Long-lived readers** freeze the detent → GC does not reuse pages → the writer takes new ones
  from the file tail. Even a put→del cycle "swells".
- **`MDBX_SAFE_NOSYNC`** — a permanent quasi-long-lived reader (the last steady commit).
- **Fragmentation** and a shortage of contiguous sequences for large values.
- **Forgotten readers** (thread crash, wrong TLS).

### 17.2. Geometry: growth_step, upper, shrink_threshold

The file grows automatically, in multiples of `growth_step`, up to `upper`. Truncation is possible
only down to the last used page; a single occupied page near the end can block shrinkage
indefinitely. Shrinkage requires hysteresis (shrink step > growth step), otherwise the file
"breathes".

### 17.3. MDBX_MAP_FULL and Resolution Mechanisms

When the GC is empty/frozen and the file has hit `upper`, the following fire, when possible: a new
steady point (detent shift) → the HSR callback (HSR — Handle-Slow-Readers, the mechanism for
evicting stuck readers; detailed in Volume V, ch. 29) → eviction of parked readers → and only then
`MDBX_MAP_FULL`.

**Fragment from [`examples/c++/23-map-full.c++`](examples/c++/23-map-full.c++)** — reproduction of
`MDBX_MAP_FULL` (in the example, a rigid geometry limits the file to 512 KB) and resolution by
increasing `size_upper`:

```cpp
    size_t inserted = 0;
    try {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (;;) {
        const auto key = "k" + std::to_string(inserted);
        const std::string value(1024, 'x');
        txn.insert(table, mdbx::slice(key), mdbx::slice(value));
        ++inserted;
      }
    } catch (const mdbx::db_full &ex) {
      std::cout << "MDBX_MAP_FULL after " << inserted << " inserts\n";
    }

    // Increase the upper bound — and continue from the same place.
    mdbx::env::geometry bigger;
    bigger.size_upper = 8 * 1024 * 1024;
    try {
      env.set_geometry(bigger);
    } catch (const mdbx::db_unable_extend &) {
      // On 32-bit Windows the address range right after the mapping may be
      // occupied: reopen the same database with the enlarged geometry.
      mdbx::env::geometry reopened = geo;
      reopened.size_upper = bigger.size_upper;
      env.close();
      env = mdbx::env_managed(path,
          mdbx::env_managed::create_parameters().set_geometry(reopened),
          mdbx::env::operate_parameters());
    }
```

Raising `upper` on a live database requires remapping: on most platforms this happens in place, but on
32-bit Windows there may be no free address range next to the current mapping — then
`MDBX_UNABLE_EXTEND_MAPSIZE` is returned, and the portable path is to close and reopen the database
with the enlarged geometry (the new mapping is created with the required `upper` right away).

Full code: [23-map-full.c++](examples/c++/23-map-full.c++).

### 17.4. Auto-Compaction (Implicit Shrink)

On every commit, freed tail pages are returned to unallocated space (refund). When the free tail
exceeds `shrink_threshold` (+ a small reserve), the commit additionally drops the "excess" range
via `madvise(MADV_DONTNEED/REMOVE)` and truncates the file.

Limitations: it does not work while long-lived readers hold the tail or while several processes use
the file; with `SAFE_NOSYNC` it is limited by the frequency of steady points.

### 17.5. Explicit Defragmentation

`mdbx_env_defrag()` / the `mdbx_defrag` utility (0.14.2+) — careful rearrangement of pages toward
the beginning of the file in cycles; each cycle is a separate committed transaction. Parameters:
`defrag_atleast`/`defrag_enough` (minimum/sufficient reduction), `time_atleast`/`time_limit`
(1/65536 s), `acceptable_backlash`, `preferred_batch`. The result code is
`MDBX_defrag_enough_threshold`.

Guaranteed file reduction — also `mdbx_copy -c` (compacted copy) or dump+load (`-a`).

### 17.6. Estimating Capacity

Dividing the file size by the item size **overestimates** the real capacity: page granularity, the
path to the root, GC, and hysteresis add overhead. Estimate for the worst case of reuse freezing.

> **Examples for this chapter:** [`examples/c++/23-map-full.c++`](examples/c++/23-map-full.c++);
> [`examples/c++/24-defrag.c++`](examples/c++/24-defrag.c++).

### 17.7. Summary of Chapter 17

- Growth = pages not being reused (readers/SAFE_NOSYNC/fragmentation).
- `upper` — the hard limit; MAP_FULL is resolved by steady/HSR/eviction.
- Implicit shrink — free compaction as part of commits.
- Explicit defragmentation in cycles; guaranteed shrinkage — copy -c / dump+load.

### 17.8. Exercises

1. Why is "10 GB in the database, so 10 GB of data" an estimation error?
2. Describe a scenario where auto-compaction will not work.

### 17.9. Chapter 17 checklist

- [ ] I name the causes of database growth: long-lived readers, `MDBX_SAFE_NOSYNC`, fragmentation, forgotten readers.
- [ ] I understand why truncation is possible only down to the last used page and why shrink-step hysteresis is needed.
- [ ] I can reproduce the chain that resolves `MDBX_MAP_FULL`: steady point → HSR callback → eviction of parked readers → the error.
- [ ] I know the limitations of auto-compaction (implicit shrink) and when it will not work.
- [ ] I can describe explicit cycle-based defragmentation and the guaranteed file-reduction paths (`mdbx_copy -c`, dump+load).
- [ ] I can explain why dividing the file size by the item size overestimates the real capacity.

---

## Chapter 18. Nested Transactions

### 18.1. Why They Are Needed

A nested transaction is a child write-transaction inside a parent one, allowing changes to be
grouped with the ability to **partially roll back**.

### 18.2. Implementation

A child transaction works on the same transaction number and front txnid. It inherits from the
parent: the page space and the dirty list (with a shadow of the parent's), retired lists (lists of
pages already freed but still held back from reuse — see 19.4), GC
buffers, table descriptors, and cursors (via the mechanism of "paired" cursors).

### 18.3. Commit (join) vs Abort (undo)

- **Commit**: the child's dirty pages are merged into the parent's list; GC counters are
  transferred. If the child is "clean" — a fast path without merging.
- **Abort**: all changes are discarded; retired pages are "un-retired"; handles and cursors are
  restored.

**Fragment from [`examples/c++/25-nested-txn.c++`](examples/c++/25-nested-txn.c++)** — a nested
transaction: `start_nested()` + `abort()` rolls back its changes (undo); the symmetric `commit()`
merges them into the parent (join):

```cpp
    // A nested transaction with a rollback (undo).
    {
      auto nested = txn.start_nested();
      auto ntable = nested.open_map(nullptr);
      nested.insert(ntable, mdbx::slice("k2"), mdbx::slice("v2"));
      nested.abort(); // roll back the nested one
    }
    std::cout << "parent sees k1 after nested abort: " << txn.get(table, mdbx::slice("k1")).as_string() << "\n";
    try {
      (void)txn.get(table, mdbx::slice("k2"));
      std::cerr << "FAIL: k2 survived nested abort\n";
      return EXIT_FAILURE;
    } catch (const mdbx::not_found &) {
      std::cout << "k2 rolled back by nested abort: absent\n";
    }
```

Full code: [25-nested-txn.c++](examples/c++/25-nested-txn.c++).

### 18.4. Fate of Tables

- A table **created** in a nested transaction: stays on commit, disappears on abort.
- A table **dropped** in a nested transaction: is deleted on commit, "comes back to life" on abort.

### 18.5. Anti-Pattern: a Nested Transaction in Every Function

Calling `mdbx_txn_begin(parent, ...)` in every small function is an anti-pattern: the meaning of
grouping is lost and overhead grows. Nest deliberately, along the boundaries of logical operations.

### 18.6. Workaround for Lack of Space

Nested transactions are a known workaround for lack-of-space scenarios (you can partially roll back
without losing previous changes).

> **Nuance:** nested transactions do not combine with `MDBX_WRITEMAP`.

> **Examples for this chapter:** [`examples/c++/25-nested-txn.c++`](examples/c++/25-nested-txn.c++).

### 18.7. Summary of Chapter 18

- A nested transaction = a child write-transaction with partial rollback.
- Commit — merging into the parent; Abort — full rollback.
- The fate of created/dropped tables follows the CoW rules.
- Do not nest "for every function"; do not combine with WRITEMAP.

### 18.8. Exercises

1. Write an example of "a group of updates with a rollback of the last step" via a nested
   transaction.
2. Check the fate of a table created and dropped in a nested transaction upon abort.

### 18.9. Chapter 18 checklist

- [ ] I can explain that a child transaction works on the same transaction number, and what it inherits from the parent.
- [ ] I distinguish commit (join): merging the child's dirty pages into the parent, from abort (undo): full rollback and "un-retiring" pages.
- [ ] I know the fate of tables created and dropped in a nested transaction, on commit and on abort.
- [ ] I can demonstrate a partial rollback via `start_nested()` + `abort()`.
- [ ] I can justify why a nested transaction "in every function" is an anti-pattern.
- [ ] I know that nested transactions are a workaround for lack of space, and that they do not combine with `MDBX_WRITEMAP`.

---

## Chapter 19. The Lock File and Interprocess Synchronization

### 19.1. Why a Separate File

Next to the data file lives the lock file (`.lck`): the global writer mutex, the reader table
(RLT), statistics, caches of the oldest snapshot. It is needed for coordination **between
processes**.

**Fragment from [`examples/c++/26-two-processes.c++`](examples/c++/26-two-processes.c++)** —
interprocess access: a writer process commits data, a reader process (a child via `fork()`) opens
the same database and reads it:

```cpp
    // Writer: the parent process commits data and closes the environment.
    {
      auto env = example::env_open(path);
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("k"), mdbx::slice("v1"));
      txn.commit();
      std::cout << "parent wrote: k=v1" << std::endl;
    }

    const pid_t pid = fork();
    if (pid < 0) {
      std::cerr << "fork failed\n";
      return EXIT_FAILURE;
    }

    if (pid == 0) {
      // Reader: the child opens the same database and reads the data.
      auto env = example::env_open(path);
      auto txn = env.start_read();
      auto table = txn.open_map(nullptr);
      std::cout << "child reads: " << txn.get(table, mdbx::slice("k")).as_string() << std::endl;
      txn.abort();
      _exit(EXIT_SUCCESS);
    }
```

Full code: [26-two-processes.c++](examples/c++/26-two-processes.c++).

### 19.2. Lock Implementations

Build option `MDBX_LOCKING`:

| Flavour      | Primitives                                  | LCK signature |
| ------------ | ------------------------------------------- | ------------- |
| `POSIX2008`  | Robust mutexes (Linux default)              | `0x8017`      |
| `POSIX2001`  | Shared mutexes                              | `0x8017`      |
| `SYSV`       | Semaphores (macOS default)                  | `0xF18D`      |
| `WIN32FILES` | Windows (`LockFileEx`)                      | `0xF10C`      |
| `POSIX1988`  | (legacy)                                    | `0xFC29`      |

> **Nuance: `POSIX2001` and `POSIX2008` share one signature — this is not a typo.** Both flavors
> store the same kind of interprocess primitive in the LCK — a `pthread_mutex_t`, so their
> on-disk formats are byte-identical and the signature is common. The difference lies only in the
> API features used: robust mutexes appeared in POSIX.1-2008, while the 2001 mode works with
> plain process-shared mutexes. Consequence: a database opened in one of these two modes can be
> continued in the other — but not with `SYSV`, `WIN32FILES`, or `POSIX1988` (their signatures
> differ, and an LCK of such a format is rejected).

The actual compatibility check at open time uses not the raw signature but the derived
`MDBX_LOCK_FORMAT` hash — it mixes the flavor's signature with `sizeof(reader_slot_t)` and the
offsets of the key `lck_t` fields, so the constants above are just the per-primitive-format
"base", while the full identity also accounts for the layout of the remaining fields.

The LCK format is versioned **separately** from the data format: a structure change breaks only
shared access to an already-open database, not the data file itself.

### 19.3. Why LockFileEx on Windows Instead of Named Mutexes

File locks (`LockFileEx`) were chosen deliberately: they work on network drives and protect against
incompetent actions. The price — in naive benchmarks with many small transactions libmdbx may lag
behind LMDB (acquiring/releasing a file lock — hundreds of microseconds).

### 19.4. Reader Slot (RLT)

```
txnid (atomic)          snapshot number (INVALID_TXNID = free)
tid                     owner; PARKED = UINT64_MAX, OUSTED = UINT64_MAX-1
pid                     owner process
snapshot_pages_used     first_unallocated at snapshot time
snapshot_pages_retired  how many pages it holds back from reuse
```

Slot liveness is checked using OS facilities (protection against pid/tid reuse). Stale slots are
not scanned one by one — the whole table is reinitialized when the LCK is opened by a single
process.

### 19.5. Recovering Locks After a Process Failure

After a process crash the OS releases its locks; dead reader slots are cleaned up by the liveness
mechanisms and `mdbx_reader_check()`.

### 19.6. Mode Without a Lock File

A "single process, exclusive" mode without `.lck` is possible (a build option) — when interprocess
access is not needed.

> **Examples for this chapter:** [`examples/c++/26-two-processes.c++`](examples/c++/26-two-processes.c++).

### 19.7. Summary of Chapter 19

- LCK: writer mutex + RLT + statistics; versioned separately.
- Implementations: POSIX/SysV/WIN32FILES; flavour signatures.
- LockFileEx — the choice for network drives; slower for small transactions.
- RLT slot: txnid/tid/pid + pinning fields; liveness via the OS.
- A mode without LCK is possible (single process).

### 19.8. Exercises

1. Why is deleting `.lck` while the database is open a mistake?
2. How is `mdbx_reader_check` different from "automatic" cleanup?

### 19.9. Chapter 19 checklist

- [ ] I can list what lives in the lock file and why it is needed for interprocess access.
- [ ] I know that the LCK format is versioned separately from the data format, and what that means for compatibility.
- [ ] I can explain why `LockFileEx` was chosen on Windows, and what we pay for it in benchmarks with small transactions.
- [ ] I describe an RLT reader slot and the fields recording how many pages the reader holds back from reuse.
- [ ] I understand how slot liveness is checked with OS facilities and what cleans dead slots after a process crash (`mdbx_reader_check()`).
- [ ] I know when the mode without a lock file is possible (single process, exclusive).

---

## Chapter 20. Durability and Recovery

### 20.1. Two-Phase Meta Update: weak vs steady

- **weak** — data is in the file but not guaranteed to be flushed to persistent media;
- **steady** — data is flushed to disk; the snapshot survives a system failure.

### 20.2. Recovery Without a WAL

There is deliberately no WAL. After a crash, on open the last **intact and valid** meta is selected
(a consistent trio, matching txnid halves, a checksum). "Half-written" transactions that did not
manage to publish the meta simply do not exist.

`mdbx_txn_checkpoint(txn, weakening_durability, &latency)` — a **commit variant**: it commits the
transaction and returns per-stage latencies (`MDBX_commit_latency`), and allows weakening durability
via `weakening_durability`. The header marks it as "may be changed in future releases" — use it for
diagnostics, not as a stable API. Forcing the steady point is done with
`mdbx_env_sync_ex(env, force=true, ...)` (Volume II, chapter 10), not with a checkpoint.

### 20.3. Open for Recovery

`mdbx_env_open_for_recovery()` opens the database with a choice of the target meta page
(`target_meta`) under an exclusive lock, to resolve a "stuck" trio. Since 0.12.7+ recovery checks
**do not modify the database**. The `MDBX_WANNA_RECOVERY` code is returned on a read-only open of a
database requiring recovery.

> **Warning:** the header marks this function as internal API of the `mdbx_chk` utility, "subject to
> change at any time" — it is not recommended in application code; the regular path is a read-write
> open or `mdbx_chk`.

**Fragment from [`examples/c++/27-recovery.c++`](examples/c++/27-recovery.c++)** — a writer crash
(`_exit` without commit/abort) is rolled back on reopen (steady), and `open_for_recovery()` opens a
specific meta page:

```cpp
    // Reopen: uncommitted changes were rolled back.
    {
      auto env = example::env_open(path);
      auto txn = env.start_read();
      auto table = txn.open_map(nullptr);
      const std::string value(txn.get(table, mdbx::slice("key")).as_string());
      std::cout << "crashed writer changes rolled back: key=" << value << ", newkey ";
      try {
        (void)txn.get(table, mdbx::slice("newkey"));
        std::cout << "present\n";
        return EXIT_FAILURE;
      } catch (const mdbx::not_found &) {
        std::cout << "absent\n";
      }
      txn.abort();
    }

    // Recovery mode: open a specific meta page (0..2).
    {
      auto recovery = mdbx::env_managed::open_for_recovery(path.c_str(), 0 /* target_meta */, true);
      std::cout << "open_for_recovery: ok\n";
    }
```

Full code: [27-recovery.c++](examples/c++/27-recovery.c++).

### 20.4. boot_id: Semantics and Behavior in LXC

`boot_id` — the OS boot identifier, taken into account when rolling back weak metas. In LXC, boot_id
may be absent/shared across containers — the rollback control for weak metas accounts for this. This
matters for failures inside containers.

### 20.5. Unified Page Cache Incoherence

A known issue, issue #269: incoherence of the unified page cache between processes. The protection —
`MDBX_FORCE_CHECK_MMAP_COHERENCY` (a build option, default 0); introduced in the 0.11.5–0.11.6
series; the trigger counter is visible in diagnostics.

### 20.6. mdbx_chk: Integrity Checking

`mdbx_chk` — deep validation: metas/trio, trees, ordering, the reachability invariant, GC. Modes:
`-w` (read-write, rollback to steady), `-d` (page-by-page traversal — without it you cannot find
lost/double-used pages), `-i` (ignore false ordering errors with custom comparators), `-0/-1/-2` (a
specific meta), `-vvvvv` (histograms). Exit code: 0 = clean.

### 20.7. What Survives a Power Failure — by Mode

| Mode                     | After a failure                              |
| ------------------------ | -------------------------------------------- |
| `MDBX_SYNC_DURABLE`      | All committed data is in place               |
| `MDBX_NOMETASYNC`        | Loss of the last commits (meta deferred)     |
| `MDBX_SAFE_NOSYNC`       | Rollback to the last steady                  |
| `MDBX_UTTERLY_NOSYNC`    | No guarantees; corruption possible           |

> **Examples for this chapter:** [`examples/c++/27-recovery.c++`](examples/c++/27-recovery.c++).

### 20.8. Summary of Chapter 20

- Two-phase meta: weak/steady; recovery — selecting the last intact meta.
- Open-for-recovery with target_meta; since 0.12.7 checks do not modify the database.
- boot_id matters in LXC; page-cache incoherence — #269 + FORCE_CHECK.
- `mdbx_chk` — the main verification tool; see the modes above.

### 20.9. Exercises

1. Describe what will happen to the database after kill -9 in the middle of a commit, in `DURABLE`
   mode.
2. Why is `mdbx_chk -d` needed to find lost-unused pages?

### 20.10. Chapter 20 checklist

- [ ] I distinguish weak and steady snapshots and know which one survives a system failure.
- [ ] I can explain how recovery works without a WAL: selecting the last intact and valid meta; "half-written" transactions simply do not exist.
- [ ] I know the purpose of `mdbx_env_open_for_recovery()` with `target_meta` and why it is not recommended in application code.
- [ ] I understand the role of `boot_id` when rolling back weak metas, and its specifics in LXC.
- [ ] I know about the unified page cache incoherence (#269) and the `MDBX_FORCE_CHECK_MMAP_COHERENCY` protection.
- [ ] I can pick the right `mdbx_chk` modes (`-w`, `-d`, `-i`, `-0/-1/-2`) for the task and understand why `-d` is needed.
- [ ] For each durability mode, I can say what will survive a power failure.

---

## Volume Conclusion

You understand the internals of libmdbx: from the page and B+tree to the meta, GC, and recovery.
Now you can explain any observed behavior through the architecture.

**What's next:** Volume IV — performance and optimization: WAF, choosing a configuration for the
scenario, micro-optimizations, get-cached, bulk operations, benchmarks.