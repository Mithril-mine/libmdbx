# libmdbx Textbook — Volume 0. Framework (table of contents)

> This is the table of contents and structure of the six-volume textbook: from
> basic concepts to bindings. Each volume is a separate file in this directory;
> all volumes are linked by cross-references and a single end-to-end project.

---

## The overall logic of progression

```
Basic concepts (Volume I) → Practical usage (Volume II) → Internal mechanisms (Volume III)
→ Performance and optimization (Volume IV) → Expert topics and edge cases (Volume V) → Bindings (Volume VI)
```

Each chapter follows the pattern "concept → mechanism → practice → nuance". A term is
defined at its first use. The end-to-end project (an application configurator) runs
from Volume I through Volume II; the exercises are the steps of this project.

---

## Volume I. Fundamentals (for beginners)

_Goal: a reader with no experience with embedded key-value databases understands libmdbx
and can build it and run the first operations. Length ~50–100 pages._

- **Ch. 1. What is libmdbx**
  - Embedded vs client-server databases; the key-value model; why mmap; place in the landscape
    (LMDB/BerkeleyDB/LevelDB/RocksDB/SQLite at the level of architectural properties); license, ecosystem,
    known users (Erigon, Reth, Isar, Monica Pass).
- **Ch. 2. Installation and first run**
  - Building (amalgamation vs full source), linking (CMake/pkg-config/static/dynamic), key build
    options; minimal open→put→get example; running the tests.
- **Ch. 3. Basic data model**
  - MDBX_val; the main table and dbi; MDBX_CREATE flags; constraints (key/value/number of tables);
    integer keys; zero keys/values (difference from LMDB).
- **Ch. 4. Basic CRUD operations**
  - put/get/del/replace; flags (NOOVERWRITE/NODUPDATA/CURRENT/ALLDUPS); reading with get vs cursor;
    full C examples; handling return codes.
- **Ch. 5. Transactions — first steps**
  - Why transactions; begin/commit/abort; read-only vs read-write; MVCC intuitively; the
    "one transaction — one thread" rule; examples.

## Volume II. Practical usage (for developers)

_Goal: the reader builds applications: cursors, multivalues, indexes, configuration, durability,
multithreading, error handling. Length ~100–200 pages._

- **Ch. 6. Cursors** — positioning (FIRST/LAST/NEXT/PREV/SET/SET_RANGE/LOWER/UPPER_BOUND),
  batch operations, UB after deletion, clone/bind, the safe-delete pattern in DUPSORT, examples.
- **Ch. 7. Multivalues and DUPSORT** — DUPFIXED/INTEGERDUP/REVERSEDUP, the nested value subtree,
  inverted "key→list of IDs" indexes, GET_BOTH/GET_BOTH_RANGE, secondary index.
- **Ch. 8. Secondary indexes** — main table + index tables; DUPSORT index vs separate table;
  maintaining consistency; composite keys; deletion by index; mini-ORM.
- **Ch. 9. Environment configuration** — geometry (lower/now/upper/growth_step/shrink_threshold),
  maxreaders/maxdbs/pagesize, flags, runtime options MDBX_opt_*, choosing a 4/8/64 KB page,
  env_info/env_stat.
- **Ch. 10. Durability modes** — SYNC_DURABLE/NOMETASYNC/SAFE_NOSYNC/UTTERLY_NOSYNC/WRITEMAP;
  when to use which; syncbytes/syncperiod; mdbx_env_sync_ex; platform nuances
  (F_FULLFSYNC, boot_id, LockFileEx).
- **Ch. 11. Multithreading** — sticky threads; NOSTICKYTHREADS (pools/coroutines); thread registration;
  TLS destructors; fork+resurrect; txn_clone; parking/eviction; multiple environments; MDBX_EXCLUSIVE.
- **Ch. 12. Error handling** — positive/negative codes; RESULT_TRUE/FALSE; strerror_r vs
  strerror; MAP_FULL/TXN_FULL; expected codes (KEYEXIST/NOTFOUND); BAD_TXN/BAD_DBI/EBADSIGN;
  WANNA_RECOVERY/MVCC_RETARDED; THREAD_MISMATCH/TXN_OVERLAPPING/BUSY; retry strategies.

## Volume III. Internal mechanisms (for the knowledgeable)

_Goal: the reader can explain the database's behavior through its architecture — a bridge between "how to
use" and "how to configure/debug". Length ~100–200 pages._

- **Ch. 13. Storage architecture: B+tree and mmap** — branches/leaves/overflow; why B+tree; page;
  mmap without the buffer cache; CoW pages; page states; the reachability invariant.
- **Ch. 14. MVCC and snapshots** — page versioning; the meta-page troika (Troika), two-phase commit,
  the finite state machine of 216 states; RLT; wait-free reading; safe64; the writer (mutex, front
  txnid, DPL); comparison with PostgreSQL/Oracle.
- **Ch. 15. Copy-on-Write and the commit pipeline** — touch→copy→DPL→spill→commit; DPL deduplication;
  spill (LRU, overflow priority, cursor protection, denominators); loose; refund; pipeline stages
  (GC→refund→spill→audit→meta→sync→unlock) and their impact on latency.
- **Ch. 16. GC** — why not a free-list; record format (key=txnid, value=PNL); the detent and its
  computation/caching; FIFO vs LIFO; BigFoot; recursive update; rp_augment_limit/
  gc_time_limit; early GC cleanup 2025; non-deferred cleanup (devel 0.14.x).
- **Ch. 17. Growth, shrinking, and defragmentation** — why it grows (readers/SAFE_NOSYNC/fragmentation);
  geometry; MAP_FULL and mitigation measures (steady-point/HSR/eviction); auto-compaction via refund
  and its limits; mdbx_env_defrag; hysteresis; capacity estimation.
- **Ch. 18. Nested transactions** — purpose; the CoW shadow of the parent, paired cursors; join vs undo;
  the fate of tables; the anti-pattern "a nested transaction in every function"; workaround for lack of
  space; compatibility with WRITEMAP.
- **Ch. 19. The lock file and interprocess synchronization** — why LCK; implementations (POSIX-2008/2001,
  SysV, WIN32FILES), why LockFileEx rather than named mutexes; LCK versioning;
  recovery after a crash; reader_check; the no-LCK mode.
- **Ch. 20. Durability and recovery** — two-phase meta (weak/steady); recovery without WAL;
  open for recovery (monotonic numbers, exclusive); boot_id and LXC; page cache incoherence
  (#269, FORCE_CHECK_MMAP_COHERENCY); mdbx_chk; what survives a power failure per mode.

## Volume IV. Performance and optimization (for experienced readers)

_Goal: the reader tunes libmdbx to their scenario, finds and eliminates bottlenecks. Length ~80–150 pages._

- **Ch. 21. WAF** — what and why; sources of amplification; batching as the main lever; tree height;
  the spill trade-off; prefer_waf_insteadof_balance/merge_threshold; calculating WAF with numbers.
- **Ch. 22. Choosing a configuration for your scenario** — 6 scenarios (write-heavy, read-heavy, mobile,
  analytics, multi-process reading, low latency): modes/flags/geometry/page/batching/numbers.
- **Ch. 23. Micro-optimizations** — SIMD search, branchless bsearch, radix-sort, atomics for weak memory
  models, minimizing syscalls, prefault+mincore, auto-appending split, merge with a dirty neighbor;
  when they matter.
- **Ch. 24. Cache lookup (get-cached)** — the concept, statuses HIT/CONFIRMED/REFRESHED/DIRTY/BEHIND/UNABLE/RACE;
  when it speeds things up; ABA; the multithreaded variant + NOSTICKYTHREADS; SingleThreaded vs multithreaded;
  history (Gabriel RABHI, Ghost Body Object).
- **Ch. 25. Bulk operations** — bunch_delete, get_batch, GET/PUT_MULTIPLE, estimate_range/distance/move,
  txn_clone, batching 100–10000 operations; when the "maximum transaction" is an anti-pattern.
- **Ch. 26. Benchmarks and measurements** — ioarena; what to measure; commit_latency by stages; PROFGC;
  typical benchmarking mistakes; "6–7× slower than LMDB" — what it means; reference points
  (200 TPS, 1–3M get/s, 20K–10M inserts/s); read scaling.

## Volume V. Expert topics and edge cases (for advanced readers)

_Goal: the reader knows the subtleties and pitfalls, can debug and make architectural decisions.
Length ~100–200 pages._

- **Ch. 27. Rules and advice** — proven practices and pitfalls, criticality context,
  counterexamples.
- **Ch. 28. Platform nuances** — Linux (boot_id, /proc, /dev/shm, tmpfs/ENOSPC, page cache, mincore);
  Windows (LockFileEx, LARGEADDRESSAWARE, 2G/4G, WSL1/2); macOS/iOS (F_FULLFSYNC,
  APPLE_SPEED_INSTEADOF_DURABILITY, F_PREALLOCATE); Android/bionic; containers (LXC/Docker);
  Wine; Linux <4.x; tmpfs.
- **Ch. 29. Handle-Slow-Readers (HSR)** — why; mdbx_env_set_hsr; parameters and returns
  (wait/kill/growth/MAP_FULL); typical implementations; HSR + SAFE_NOSYNC; a C example.
- **Ch. 30. Diagnostics and debugging** — tools (mdbx_chk options and
  exit codes, PROFGC and the semantics of gc_prof, commit_latency by stages, txn_info, reader_check/list,
  logging, ASAN/UBSAN/valgrind); step-by-step scenarios (database growth, commit slowdown, MAP_FULL,
  deadlock/BUSY, a long-running reader, corruption, slot leak, performance, fragmentation, fork,
  Windows slowdowns, containers); checklists before production.
- **Ch. 31. Migration from LMDB** — advantages; format compatibility (0.11.x ↔ 0.12.x);
  breaking changes (NOLOCK removed, NOTLS → NOSTICKYTHREADS); the mdb_* → mdbx_* mapping table;
  behavioral differences (meta troika, two-phase commit, checksums); migration checklist.
- **Ch. 32. Design patterns** — 8 patterns (auto-ID via sequence,
  DUPSORT secondary index, composite key+comparator, task queue table-as-queue, ring buffer,
  full-scan iterator, replication via per-table txnid, read-your-writes via clone/
  embark_read): problem → solution with code → trade-offs → alternatives.
- **Ch. 33. Roadmap and the future: MithrilDB** — the current state; a common API for several formats;
  amalgamation; replication (nonlinear GC); change subscription; encryption/compression; streaming BLOBs;
  SWIG.

## Volume VI. Bindings

_Goal: the reader uses libmdbx from their language with an understanding of the binding's specifics and
limitations. Length ~50–150 pages._

- **Ch. 34. Ecosystem overview** — the full list of officially tracked bindings; statuses
  (mature/experimental/deprecated); the "language → binding" map.
- **Ch. 35–46. Binding cards** (following a single template: overview/installation/basic example/
  threading/memory/nuances/typical errors/performance/gaps/references):
  35 Rust (mdbx-sys/mdbx-rs), 36 Go (mdbx-go), 37 Python (python-lmdbx/mdbx-py), 38 Node.js
  (node-mdbx), 39 .NET/C# (libmdbx-dotnet), 40 C++ (mdbx.h++), 41 Dart (mdbx-dart), 42 Nim, 43 Java,
  44 Haskell, 45 Ruby, 46 Scala. Zig (officially tracked) — a short reference in Ch. 34 (§34.2),
  without a separate card.
  - Mandatory nuances: Go+NOSTICKYTHREADS (deadlock scenarios, CGO); Rust+async (Send/Sync,
    use-after-free after commit, blocking calls); .NET+GC (SafeHandle, pinning, lifetime);
    Python+GIL (long reads, fork after import).
  - Honesty: when data is insufficient — "requires analysis of the binding's repository".
