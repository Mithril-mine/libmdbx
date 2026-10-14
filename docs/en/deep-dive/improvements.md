# Improvements: what libmdbx adds over LMDB

> Public edition for the amalgamated package. The catalog of libmdbx improvements
> over LMDB: features, API extensions, engine mechanisms, fixes and
> optimizations. Related: [`architecture.en.md`](architecture.md),
> [`deep-dive.en.md`](internals.md).
> A systematic account of what _libmdbx_ (a deeply reworked descendant of LMDB, forked in 2015)
> has added, changed and improved compared with LMDB: features, API functions, mechanisms,
> bug fixes and optimizations — down to micro-optimizations (SIMD, branchless search, atomics),
> **without enumerating individual code sites or one-off edits**.
>
> Sources: current and historical `README.md`, documentation in `docs/`, `ChangeLog*.md` of all
> releases (0.09–0.14), the official website [libmdbx.dqdkfa.ru](https://libmdbx.dqdkfa.ru/),
> and the full list of runtime options (`MDBX_opt_*`). Versions are rough "first appeared / became
> noticeable" markers; much of the functionality evolved gradually.

---

## 1. Summary map

| Category | In short | Key points |
| --- | --- | --- |
| A. Reliability | Defects still living in LMDB are fixed | §2 |
| B. Data model & format | Longer keys, zero length, architecture-independent format | §3 |
| C. Transactions & concurrency | Parking/ousting, cloning, fork, HSR | §4 |
| D. GC & DB size | Big Foot, LIFO/FIFO, auto-compaction, defragmentation | §5 |
| E. Durability | SAFE_NOSYNC, steady points, meta troika, recovery | §6 |
| F. B+tree engine | Smart splits, merge tactics, multivalues, estimates | §7 |
| G. C API | Extended operations, information, options, cache | §8 |
| H. C++ API | Full type-safe layer | §9 |
| I. Utilities & diagnostics | chk/defrag/copy/dump, statistics, UUID | §10 |
| J. Optimizations | SIMD, branchless, prefault, spill, sorting | §11 |
| K. Portability | Platforms, locking, TLS, endianness | §12 |
| L. Build/distribution | Amalgamation, CMake/Make, LTO, license, Conan | §13 |
| M. Ecosystem | Hundreds of projects, bindings, awards, MithrilDB | §14 |

The version timeline is in §15.

---

## 2. Reliability: defects inherited from LMDB

A large part of the changes consists in fixing defects that have lived in LMDB for years or decades
and reproduce only under specific combinations of conditions, which makes them especially insidious.
The most significant ones are listed below.

**Known and publicly documented:**

- **Database page leaks and incorrect table/sub-DB statistics.** Two consequences: file space is lost
  (pages fall out of circulation) and growth metrics become unreliable — an application sees the
  database "inflate" when it actually does not.
- **Segfaults in several conditions:** corrupted databases, cursor edge cases, use of the `0` table
  handle in read transactions, tree walks after partial corruption. Fixed via strict boundary checks.
- **Suboptimal page-merge strategy on deletion.** LMDB always merged an emptied leaf with its least
  populated neighbor; corrected to the "prefer an already modified (dirty) page" tactic — see
  §5/§7. Up to 50% savings on mass deletions.
- **Updating an existing record with a changed data size** (including multimaps). In certain cases
  LMDB can silently lose data with large values; this path is reworked so that the value is either
  correctly relocated or an error is returned.
- **Database corruption in `DUPFIXED` mode** with long or odd-length multivalues (LEAF2 pages) —
  caused by reserving space without accounting for possible page overflow. The bug had been present
  in LMDB for more than 11 years and was inherited; fixed in libmdbx (0.12.10).
- **"Reincarnation" of a deleted sub-DB** and implicit deletion of tables via operations on `@MAIN`.
  Previously a deleted table could "resurrect" in a new transaction; now the table lifecycle is
  tracked explicitly.
- **Races when opening DBI handles** and when starting a transaction concurrently with descriptor
  creation. Resolved by synchronizing handle import into the transaction.
- **GC update looping at commit** (divergence, when reprocessing does not converge). Fixed via a
  corrective feedback loop in the update cycle.
- **Spurious `MDBX_CORRUPTED` on unaligned access to 64-bit fields** — including on ARM and after
  `#pragma pack`; closed with careful alignment handling.
- **Database copy errors on NFS/CIFS/SMB** (`fcntl`/`flock` conflicts, `EAGAIN`/`EWOULDBLOCK`);
  the copy function now works correctly on network file systems.
- **Loss of table contents on abort of a nested transaction that deleted the table** (0.14.x):
  the table state is now restored along with the rest of the data.
- **Incorrect closing of a DBI descriptor of a modified table** — could create a table with an empty
  name, leak pages or corrupt the tree-root link; an error is returned instead.
- **"Resurrection" of closed cursors of nested transactions** — led to memory leaks and
  use-after-free; cursor lifetimes are tracked more strictly.

**Systemic reliability guarantees:**

- **Integrity under asynchronous unordered writes (`SAFE_NOSYNC`).** Unlike `MDB_NOSYNC`, the
  database is not corrupted on a system crash: a rollback to the last steady commit is performed.
  For behavior exactly matching LMDB, a dedicated `MDBX_UTTERLY_NOSYNC` mode exists — the choice is
  explicit.
- **Boot-id based decisions** when discarding weak meta-pages — including inside an LXC container.
- **Protection against unified page/buffer cache incoherence** (Linux, issue #269): a full workaround,
  landed in the 0.11.5–0.11.6 series.
- **File-system consistency checks.** For unsuitable file systems the library either fails with an
  explicit error or warns; exclusive mode on network shares is allowed, cooperative read-only too.
- **Reaping "stuck" readers** at open and before database growth, plus **auto-reaping of "stuck"
  writers** (a sign of a stale registration).
- **`MDBX_EMULTIVAL` on ambiguous update/delete** — an explicit "several values" error instead of
  silent (and therefore dangerous) behavior.
- **Protection against double-opening one DB within a process**: tracking and POSIX-lock restoration
  after `fork`; a legacy-compatible mode is available.
- **`ENOLCK` on WSL1**, where operation is impossible in principle: an explicit refusal is preferable
  to silent data corruption.

## 3. Data model and format

- **Keys more than twice as long as in LMDB**: up to ~½ of a page (2022 bytes at a 4 KB page, 32742
  at 64 KB) versus 511 bytes. Limits are exposed via the API (`mdbx_limits_*`,
  `mdbx_env_get_maxkeysize()` and others). In practice, for many schemas there is no longer a need
  to hash long keys or introduce separate lookup tables.
- **Zero-length keys and values.** In LMDB a key must be non-empty. Storing "empty" entities
  simplifies binary protocols and sparse structures.
- **The database format is identical for 32- and 64-bit builds** — it depends only on the platform
  endianness. A file created on a 64-bit machine opens on a 32-bit one and vice versa; this matters
  for moving databases between environments.
- **Multivalues.** Three storage forms are supported: dense fixed-length dupfix pages, nested trees
  and sub-pages; the choice is automatic based on the number and size of values. Limits and nesting
  depth are available via the API (`dupsort_depthmask`, `count_ex`).
- **Sequences** and **three persistent 64-bit markers (vector-clock)** — a built-in tool for
  monotonic numbers and "who changed what and when" stamps.
- **Database UUID** in the `mi_dxbid` field: distinguishes different copies of a database, not just
  format-signature equality.
- **Refined internal node sizes**: in several cases fewer overflow pages and higher key limits for
  the same page size.
- **Non-printable and empty table names** are handled correctly (up to binary names).
- **Last-modifying transaction number per table** (per-subDB last-update txnid) — the basis for
  delta-synchronization and replication schemes layered on the database.

## 4. Transactions and concurrency

- **Parking of read transactions** (`mdbx_txn_park`/`unpark`) with flags `PARKED`/`AUTOUNPARK`/
  `OUSTED` and auto-restart of ousted ones (`restart_if_ousted`). A parked transaction releases its
  reader slot: its snapshot stops holding page reuse, and on resume the transaction continues from
  the same place.
- **Ousting parked readers.** When a writer lacks space, it atomically moves the slot from `PARKED`
  to `OUSTED`; on the next access the reader either restarts on a fresh snapshot or gets an explicit
  result code.
- **Handle-Slow-Readers (HSR)** — a callback for resolving overflow caused by long readers (an
  evolution of the old `mdbx_env_set_oomfunc`). Together with parking it forms a complete toolkit
  for dealing with "stuck" snapshots.
- **Cloning of read transactions** (`mdbx_txn_clone`): several positions on one snapshot.
- **`mdbx_env_resurrect_after_fork()`** — safe reopening of an environment in a child process after
  `fork()`, without full close/reopen.
- **Nested transactions radically reworked:** a fast path for "pure" nested transactions, lazy cursor
  shadowing, correct fate of created/deleted tables, C++ API support.
- **Nested read-only transactions** and "fake" read-only ones — for API uniformity.
- **Extended transaction operations:** `mdbx_txn_refresh()`, `mdbx_txn_checkpoint()` (commit without
  releasing the lock), `mdbx_txn_commit_embark_read()` (commit immediately continuing as a read
  transaction), `mdbx_txn_amend()` (writing from a read snapshot), `mdbx_txn_rollback()` (abort +
  restart without losing the lock).
- **`mdbx_txn_break()`** — explicit marking of a transaction as broken.
- **NOSTICKYTHREADS instead of LMDB's `MDBX_NOTLS`.** A transaction can be passed between threads;
  discipline violations are detected and returned as explicit codes (`MDBX_THREAD_MISMATCH`,
  `MDBX_TXN_OVERLAPPING`, `MDBX_BAD_RSLOT`, `MDBX_BUSY`). Important for coroutines and thread pools,
  where "sticky" transactions are unacceptable.
- **Explicit reader-thread registration/deregistration** (`mdbx_thread_register/unregister`).
- **Main-lock management** lock/unlock/upgrade/downgrade for complex writer-coordination scenarios.
- **Cursor discipline:** all cursors can be reused and must be closed explicitly — eliminating a
  whole class of use-after-free and double-frees. Related operations: `mdbx_cursor_create/bind/unbind`,
  `mdbx_txn_release_all_cursors[_ex]`.
- **Cursor positioning with `<`, `<=`, `==`, `>=`, `>`** (including key-value pairs);
  `MDBX_SET_LOWERBOUND`, `MDBX_SET_UPPERBOUND`, `mdbx_cursor_compare` (the `<=>` analog),
  `mdbx_cursor_scan[_from]`, `on_first_dup`/`on_last_dup`.
- **Cursors on one table in different read transactions** are allowed (multi-cursor API).
- **`mdbx_preopen_snapinfo()`** — getting database information without opening it.

## 5. Page reuse (GC), database growth and size

- **Big Foot (0.12.1)** — splitting large retired-page lists into chains of GC records. Removes the
  requirement to find and allocate long contiguous free-page sequences for storing the lists
  themselves; critical for transactions retiring millions of pages (e.g., Ethereum-ecosystem loads).
- **Early GC Cleanup (0.14.x)** — reclaimed GC records are deleted as early as possible rather than
  only at commit; opens the way to defragmentation and non-linear GC processing.
- **LIFO reuse policy (`MDBX_LIFORECLAIM`)** with FIFO by default. LIFO takes the freshest frees:
  a page circulates over a minimally short loop and does not fall out of the disk write-back cache.
  On systems with such a cache, write speed noticeably improves.
- **Automatic on-the-fly database-size adjustment** — growth and shrinkage — via geometry parameters
  `lower/now/upper/growth_step/shrink_threshold`.
- **Continuous zero-overhead compaction**: returning the free tail to the unallocated space (refund)
  on every commit + file truncation when it accumulates (implicit shrink, `stockpile_gap`,
  `madvise(MADV_DONTNEED/REMOVE)`). Runs alongside ordinary commits, no separate pass.
- **Explicit defragmentation** (`mdbx_env_defrag`, the `mdbx_defrag` utility) with control of goals,
  time and acceptable "backlash".
- **Dynamic limits:** `rp_augment_limit` (auto-adjusted to the DB size), `gc_time_limit` (time limit
  for sequence search), dirty-page limits (`txn_dp_limit` — auto from RAM size) and spill
  denominators.
- **Auto-merging of GC records** and an optimized `pnl_merge`; GC profiling (`MDBX_ENABLE_PROFGC`).
- **GC diagnostics:** `mdbx_gc_info()` (state, histograms, record iteration) and the `gcrtime`
  counter — time spent searching and reclaiming.
- **Refund/loose pages:** returning freed pages within the current transaction without touching GC
  (options `MDBX_ENABLE_REFUND`, `MDBX_opt_loose_limit`, `dp_reserve_limit`).
- **RKL** — tracking GC-record identifiers (intervals + lists) with "weighted-reserve" allocation:
  one-pass GC update with O(1)..O(log N) complexity.
- **Sparse and lock-free DBI handle sets** (`MDBX_ENABLE_DBI_SPARSE`, `MDBX_ENABLE_DBI_LOCKFREE`) —
  lower overhead with many tables.
- **Space counters in `mdbx_txn_info`:** dirty/leftover/retired/limit for read and write
  transactions — a working tool for diagnosing the impact of "long readers".

## 6. Durability and synchronization

- **Three meta-pages and the "Troika" commit method (0.12.1).** Two durable snapshots plus a tail
  slot for rewriting; two-phase update with minimal memory barriers, comparisons and branches.
  LMDB has two meta-pages, and their update is a source of subtle races.
- **Explicit sync modes:** `MDBX_SYNC_DURABLE`, `MDBX_NOMETASYNC`, `MDBX_SAFE_NOSYNC`
  (renamed from `MDBX_NOSYNC`), `MDBX_UTTERLY_NOSYNC`; `MDBX_MAPASYNC` is deprecated. Mode names
  reflect the risk level — reducing accidental choice of a "non-durable" mode.
- **Steady and weak meta-pages.** The steady point is formalized; when space runs low the library
  automatically forms a new steady point.
- **Automatic sync by thresholds and/or timeout** with cheap polling: `mdbx_env_set_syncbytes`/
  `syncperiod`, `MDBX_opt_sync_bytes/period`, `presync_threshold`, async `mdbx_env_sync[_ex|_poll]`
  with pre-check and retry.
- **Dynamic write-path selection** (`MDBX_opt_writethrough_threshold`): write-through (`O_DSYNC`)
  versus write + `fdatasync()`, depending on the number of pages and storage latency.
- **Prefault write** (`MDBX_opt_prefault_write_enable`) — pre-writing pages allocated for `WRITEMAP`
  to eliminate page faults and disk reads on first access.
- **Recovery without WAL**: selecting the last intact meta-page, `open_for_recovery` mode and
  switching to a given meta-page; since 0.12.7 recovery checks do not modify the database.
- **Diagnostic codes** `MDBX_WANNA_RECOVERY` / `MDBX_MVCC_RETARDED`.
- **Unified page-cache incoherence control** (see §2).
- **On macOS/iOS, `fcntl(F_FULLFSYNC)` is used by default** — the only way to guarantee durability
  on power failure; `MDBX_APPLE_SPEED_INSTEADOF_DURABILITY` trades durability for speed.

## 7. The B+tree engine and pages

- **Split with "auto-appending" (0.10.0)** — inserting ordered key sequences fills pages more
  densely; **"split at middle"** keeps the tree balanced on average rather than skewed toward
  inserts.
- **Merge tactic on deletion.** When a leaf empties, merging prefers an already modified (dirty)
  page; if statuses are equal — the least populated one. Reduces WAF, up to 50% savings on mass
  deletions. Controlled by `prefer_waf_insteadof_balance` and `merge_threshold` (33% by default
  since 0.14.x).
- **Transparent spill (0.10.0)** — dirty pages move to a "ready to evict" state without later
  changes; LRU policy with priority for overflow pages; spill accounts for large/overflow page
  sizes; `MDBX_TXN_NIPPED` allows pausing spill during GC processing.
- **Bulk operations.** "Bunch" deletion (`mdbx_cursor_bunch_delete`, `delete_range`) cuts whole
  pages and branches instead of item-by-item deletion; batch reads (`mdbx_cursor_get_batch`);
  batched multivalue work (`MDBX_GET/PUT_MULTIPLE`, `SEEK_AND_GET_MULTIPLE`, put/seek samelength,
  batch put).
- **Range query estimation** (`mdbx_estimate_range/distance/move`, `MDBX_EPSILON`): estimates from
  pages common to cursor stacks, without scanning data.
- **On-the-fly page checks:** improved online validation; the `MDBX_VALIDATION` option for working
  with corrupted or untrusted databases; corruption detection via the `parent-page-txnid` field.
- **Density parameters:** `MDBX_opt_subpage_*` (sub-page limit and reserve) and `split_reserve`.
- **Cursor tracking in write transactions** (repoint/shadow) and `mdbx_is_dirty()` for avoiding
  copies from dirty pages.
- **Minimized leaf-page reads when deleting tables and nested trees.**

## 8. C API: extensions

- **Unified options API** `mdbx_env_set_option/get_option` with the `MDBX_opt_*` set: `max_db`,
  `max_readers`, `sync_bytes`, `sync_period`, `rp_augment_limit`, `loose_limit`,
  `dp_reserve_limit`, `txn_dp_limit`, `txn_dp_initial`, `spill_max_denominator`,
  `spill_min_denominator`, `spill_parent4child_denominator`, `merge_threshold`,
  `prefer_waf_insteadof_balance`, `writethrough_threshold`, `prefault_write_enable`,
  `gc_time_limit`, `split_reserve`, `subpage_limit`, `subpage_room_threshold`,
  `subpage_reserve_prereq`, `subpage_reserve_limit`, `presync_threshold`.
- **Extended CRUD**: put with returning the previous value; updating/deleting a specific multivalue;
  `MDBX_UPSERT`, `MDBX_ALLDUPS`, `MDBX_APPEND/DUP`, `MDBX_NOOVERWRITE`; upsert of all duplicates;
  `reserve`; `empty-data` in `MDBX_MULTIPLE`.
- **"get-cached"**: `mdbx_cache_get[_SingleThreaded]` — lazy search by page version stamps with
  HIT/CONFIRMED/REFRESHED/DIRTY/BEHIND/UNABLE/RACE statuses; shared lock-free. Re-reading a "hot"
  key turns from a full tree descent into a couple of comparisons.
- **Geometry and limits**: `mdbx_env_set_geometry`, `mdbx_limits_*` (keysize/valsize/pairsize
  `...4page_max`, minimum bounds), `mdbx_default_pagesize`, `mdbx_get_sysraminfo`.
- **Information**: `mdbx_env_info_ex` (geometry, metas, meta[3], UUID, page-operation statistics),
  `mdbx_txn_info` (including `scan_rlt`), `mdbx_dbi_flags_ex` (DIRTY/STALE/FRESH/CREAT state),
  `mdbx_enumerate_tables`, `mdbx_cursor_count_ex`.
- **Transactions**: the full §4 set; `mdbx_txn_commit_ex` with latency metrics;
  `mdbx_txn_copy2pathname/fd` (copy from a transaction).
- **Tables**: `mdbx_dbi_rename[_2]`, `mdbx_dbi_sequence`, canary markers `mdbx_canary_*`,
  the `mdbx_drop` API, deferred invalidation of dropped-table handles (0.14.3+), `MDBX_DB_ACCEDE`.
- **Environment**: `mdbx_env_delete` (multiprocess deletion), `mdbx_env_warmup`, `mdbx_env_chk`
  (checking from within the library), `mdbx_env_defrag`, `mdbx_env_sync_poll`,
  exclusive/read-only/no-LCK modes, `mdbx_env_get_path[_W]`, `mdbx_set_panic`, user context for
  transactions and cursors.
- **Key transforms**: value-to-key for numbers (int32/int64, float/double, JSON-integer),
  `mdbx_key_from_*` — recommended instead of custom comparators so the database remains checkable
  with the standard `mdbx_chk`.
- **Logging**: a callback without `vprintf` (handy for language bindings), `mdbx_assert_fail` in the
  public API, level configuration via environment variables, `MDBX_LOG_DEBUG/TRACE` levels for API
  errors.
- **Runtime diagnostics**: `mdbx_get_sysraminfo`, page-op statistics via `mdbx_env_info_ex`,
  `MDBX_ENABLE_PGET_STAT` (page-access counter).

## 9. C++ API

- **Full type-safe RAII layer**: `env`/`env_managed`, `txn`/`txn_managed`, `cursor`/`cursor_managed`,
  `map_handle`, `slice`/`buffer<>` (polymorphic C++17 allocators, ownership policies,
  `inplace_storage_size_rounding`).
- **Exception hierarchy** mapped onto C error codes; `make_broken()`; `[[nodiscard]]` conventions;
  C++20 concepts when available.
- **Typed map operations**, `mdbx::pair`, key/value translation (including value2key).
- **Encoders**: hex/base58/base64 (a high-performance base58 per the RFC draft), `is_printable`,
  UTF-8 validation, safe middle/reservation.
- **`mdbx::comparator`, `default_comparator`, `estimate_result`, `extra_runtime_option`**, geometry
  with fluent setters, commit latency metrics.
- **Nested write transactions**, buffer append/reserve, `get_/set_context`, move/copy assignment for
  managed classes, `withdraw_handle`.
- **Explicit template instantiation inside the library** — faster consumer builds.

## 10. Utilities and diagnostics

- **`mdbx_chk`**: deep checking (meta/troika, trees, key order, GC, scopes, page and multivalue
  fill histograms), check against a given meta-page, meta-page switching, `-u/-U` warmup options,
  verbosity, "survival" with corrupted trees (bypassing damaged branches instead of crashing).
- **`mdbx_copy`**: hot backup (including to a pipe), copy-with-compaction (zeroing unused gaps),
  `-d/-p/-f` options, overwrite.
- **`mdbx_defrag`**: explicit defragmentation with goals and limits.
- **`mdbx_dump`/`mdbx_load`**: full attribute support, compact mode `-c` (single-shot keys), purge
  `-p`, batch-insert `-b`, limits `-L`, density `-d`, geometry `-G`.
- **`mdbx_drop`, `mdbx_stat`** (including `-p` page-op statistics and all counters), **`mdbx_test`**
  (stochastic scenarios, `--geometry-jitter`, `--numa`, `--pagesize`, `--loglevel`),
  `stochastic.sh`, `battery-tmux.sh`.
- **Version/build**: `VERSION.json`, `SOURCE_DATE_EPOCH`, `MDBX_BUILD_TIMESTAMP`,
  `MDBX_BUILD_METADATA`, the `options:` output for host/container compatibility.

## 11. Performance optimizations

### 11.1. Engine micro-optimizations

- **SIMD search for free-page sequences**: `scan4seq_*` kernels for SSE2/NEON/AVX2/AVX512 — ×4/×8/×16
  speedups (0.12.1); runtime kernel selection (`scan4seq_resolver`), so one build works on both old
  x86 and modern AVX-512 servers; NEON fixed for ARM64-Windows.
- **Branchless binary search** (bsearch/lower_bound via CMOV) with a workaround for a CLANG x86 bug;
  later — **inlining of built-in/default comparators** (0.14.2): numeric-key search reduces to a
  sequence of unconditional instructions.
- **Sorts**: adaptive binary-search sort, radix sort (LSB-first, 2×16-bit digits), sorting networks
  for n=3..8, branch-free compare-swap; fast page-list sorts (PNL/DPL); the radix threshold
  (`MDBX_RADIXSORT_THRESHOLD`).
- **Lazily sorted dirty-page list (DPL)** with sort-on-demand; optimized `dpl_append`.
- **C11 atomics for weak memory models** (ARM/AArch64/PPC/MIPS/RISC-V): acquire/release, CAS, the
  safe64 protocol for 64-bit reads; atomicity checked at build time.
- **Rational branching and function markup**: `pure`/`const`, `__cold`/`__hot`, `__always_inline`,
  `likely/unlikely` (thousands of placed hints in the source).
- **`__builtin_cpu_supports`** for SIMD dispatch (`MDBX_HAVE_BUILTIN_CPU_SUPPORTS`).
- **Minimized system calls**: dropping `pwritev` for single writes, merging write regions, avoiding
  unnecessary `msync`, dropping `copy_file_range` on defective kernels, `fallocate` against SIGBUS,
  `fcntl64` for locks.
- **Prefault write** and **mincore**-based resident-page tracking to prevent page faults in
  `WRITEMAP`.
- **No floating-point operations and no `libm` dependency** (0.14.x); 16.16 fixed-point
  representation for time and thresholds.
- **TLS attributes** (`tls_model("local-dynamic")`) and careful TLS-destructor handling.
- **`-fno-semantic-interposition`** — lower call overhead for internal functions.

### 11.2. Algorithmic improvements

- Transparent spill + LRU (see §7), refund/loose, auto-merging of GC records, one-pass GC update via
  RKL (see §5).
- Accelerated GC update for huge transactions (Ethereum/Erigon scenarios) — 0.11.3.
- Auto-appending split and "split at middle"; merge with a dirty neighbor (see §7).
- Dynamic heuristics: auto-tuned `dp_limit` from RAM size, `rp_augment_limit` from DB size, default
  page size and geometry auto-selection.

### 11.3. Synchronization / parallelism

- Wait-free readers without atomics on the read path; lock-free reader-table scans; oldest-reader
  caching.
- OFD locks (with fallback to POSIX `fcntl64`), timed waits, SysV/semaphore variants, no-LCK-file
  mode.
- **File locks on Windows — a deliberate choice instead of LMDB's named mutexes.** Reasons and
  consequences:
  - `LockFileEx()` allows placing the database on network drives (named mutexes are local entities
    and useless for network shares);
  - file locks protect against incompetent user actions (together with exclusive open) — protection
    from a class of errors that corrupt the DB;
  - the price is performance: Windows kernel file locks are poorly implemented, so in naive
    benchmarks with many small transactions libmdbx may lag behind LMDB (stated in the project FAQ
    and ChangeLog 0.12.3);
  - a conscious "reliability and portability vs speed" trade-off.
- **Overlapped/async writes on Windows** (`WriteGather`, unbuffered I/O) — reduced OS-interaction
  overhead.

## 12. Portability and platforms

- **Platforms**: Linux, Windows, macOS/iOS, Android, Harmony OS, Haiku, FreeBSD, NetBSD, OpenBSD,
  DragonFly, Solaris/OpenIndiana/OpenSolaris, Plan 9/9P (exclusive mode), WSL2 (and a correct refusal
  on WSL1); toolchains — GNU Make + CMake + MinGW + MSVC + CLANG + GCC + Elbrus/LCC.
- **Weak memory models** (see §11.1); **big-endian** and non-standard page sizes; **large databases
  (>4 GB) from 32-bit code**.
- **OS-specific handling**: workarounds for Wine, DrvFs, NFS/CIFS/SMB, CDROM, `F_FULLFSYNC`,
  `GetExitCodeThread`, `boot_id` on Windows and in LXC.
- **Protection against pid/tid reuse**; reader liveness checks; safe `fork`.

## 13. Build, distribution, license

- **Amalgamated single-file distribution** (in the SQLite manner) with removable dev markers;
  `make dist`, packages, a Conan recipe.
- **CMake** (including as a subproject), **GNU Make**, a broad set of build options
  (`MDBX_WITHOUT_MSVC_CRT`, `MDBX_CHECKING`, `MDBX_VALIDATION`, `MDBX_ENABLE_*`, `MDBX_AVOID_MSYNC`,
  `MDBX_BUILD_TOOLS`, `MDBX_USE_OFDLOCKS`).
- **LTO (Link-Time Optimization) support — a separate build subsystem**:
  - in CMake, LTO availability is auto-detected for GCC, CLANG and MSVC
    (`GCC_LTO_AVAILABLE`/`CLANG_LTO_AVAILABLE`/`MSVC_LTO_AVAILABLE`) with version checks
    (GCC ≥ 7, CLANG ≥ 5, MSVC ≥ 19);
  - enabled via the standard `INTERPROCEDURAL_OPTIMIZATION` option;
  - correct builds require selecting `ar`/`nm`/`ranlib` with the LTO plugin (or `lld`/`ld` for
    CLANG) — a non-trivial part of the scripts;
  - GNU Make provides dedicated `.static-lto` targets compiling utilities with `-flto`;
  - the change history reflects integration specifics: GCC tool search for LTO, CLANG-LTO detection
    for Android, suppressing spurious `-Wno-lto-type-mismatch` warnings for old GCC in LTO builds.
- **Reproducible builds** (`SOURCE_DATE_EPOCH`/`MDBX_BUILD_TIMESTAMP`), ASAN/UBSAN/MSAN support
  (Valgrind/ASAN), `ctest`, atomicity checks.
- **Apache-2.0 license** (since 0.13), explanations in COPYRIGHT.
- **Separate config files** for GNU Make and CMake; limited internal-symbol leakage.

## 14. Ecosystem

**Data from the official website (libmdbx.dqdkfa.ru, as of September 1, 2026):**
- **1109 open projects with 50881 stars** found, using libmdbx as storage (excluding forks, copies
  and clones); information is collected from GitHub, SourceCraft, SourceGraph, Codeberg, GitVerse,
  GitFlic, mos.hub, market.dev and Yandex Cloud Search.
- Largest clusters (top-42): **Ethereum infrastructure** — Reth, Erigon, optimism, taiko-mono,
  ethrex, sequencer (Starknet), citrea (Bitcoin ZK rollup), rbuilder (MEV), rindexer
  (EVM indexers); **L2/blockchain** — tempo, base, Nimiq core-rs-albatross, irys; **applications** —
  mangayomi, Isar (NoSQL for Flutter), miranda-ng (messenger), qiqqa (research), Monica Pass
  (password manager), nzbget (Usenet), Cobalt (WhatsApp API), endee (vector DB).
- **Officially tracked bindings**: Rust, Go, Node.js, Zig, Python, .NET (C#), C++, Dart, Nim, Java,
  Haskell, Ruby, Scala.
- The project is a winner of the **Yandex Open Source** contest among open-source projects; the code
  stays open with free support.
- **The strategic direction is MithrilDB** (announced in late 2025): a common API for supporting
  several database formats. Planned: txnid-cached search, streaming BLOBs, optional mmap,
  engine-level encryption and compression, SWIG, replication, and cross-language C/C++↔Rust
  interaction. Old formats and databases will be supported while users need them.

## 15. Version timeline (key milestones)

| Version (year) | Key improvements |
| --- | --- |
| 2015–2017 (ReOpenLDAP/early) | Fork; basic reliability, longer keys, zero-length keys |
| ~0.9.x (2019–2021) | Options API; dynamic lists; reworked spill; refund; C11 atomics; C++ API (0.9.1); `env_delete`, `commit_ex`, `SET_LOWERBOUND` |
| 0.10.x (2021) | `set/get_option`; transparent spill + LRU; auto-appending split; `get_sysraminfo`; `DISABLE_PAGECHECKS`; page-op statistics |
| 0.11.x (2021–2022) | `cursor_get_batch`, `SET_UPPERBOUND`; GC speedup for huge transactions; incoherent page-cache fix (#269); post-GitHub removal relocation; wchar API; C++ finalized |
| 0.12.x (2022–2023) | **Big Foot**; **Troika**; SIMD search (AVX2/AVX512/SSE2/NEON); branchless bsearch; prefault-write; writethrough; merge tactic; `warmup`; Windows overlapped I/O; LCK v2; `VALIDATION` |
| 0.13.x (2023–2025) | Apache-2.0; **parking/ousting**; HSR; DBI sparse/lockfree; `gc_time_limit`; `env_chk` in the library; rename; cursor scan/compare; `resurrect_after_fork`; `NOSTICKYTHREADS`; subpage options; `prefer_waf_insteadof_balance`; UUID; a large series of API extensions |
| 0.14.x (2025–2026) | **Early GC cleanup**; **explicit defragmentation** + `mdbx_defrag`; `bunch_delete`/`delete_range`; `cache_get`; `txn_clone/refresh/checkpoint/amend/rollback/embark_read`; nested read-only; `gc_info`; `split_reserve`; distance/scroll/distribute; Harmony OS/Haiku; no float/`libm`; `MDBX_CHECKING`; branchless + inlined comparators; `presync_threshold`; deferred DBI invalidation; stabilization of 0.14.x (0.14.3) |

## 16. Status and next steps

This catalog is updated against the official website data (ecosystem, Windows locks, LTO).
Further steps:

1. Cross-check items against the code for categories where it is critical (format, options, meta).
2. Add links to [`deep-dive.en.md`](internals.md) sections where the
   mechanisms are described in detail.
3. If needed, a separate document "what differs in default behavior".

*This document is a functional description and deliberately does not reference source-code files or
modules. For module-level mapping and refactoring invariants see [`architecture.md`](architecture.md)
and `structure.md`.*