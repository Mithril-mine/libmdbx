# Volume V. Expert topics and edge cases

> **Level:** for advanced users.
> **Volume goal:** you know the subtleties and pitfalls, can debug problems, make architectural
> decisions, migrate from LMDB and build robust patterns.

---

## Chapter 27. Rules and tips from the knowledge base

The full list of rules and tips is in the knowledge base (`knowledge-base/rules-and-checklist/`,
`knowledge-base/tips/`). Here — grouping by topic with criticality accents.

### 27.1. Transactions and threads (critical)

- **One transaction — one thread.** Passing a transaction object to another thread is UB/an error
  (`MDBX_THREAD_MISMATCH`).
- **Do not pass a transaction between threads** even with `NOSTICKYTHREADS`, if another thread may
  synchronously call a write-function of the environment — deadlock.
- **Do not use transaction objects from two threads at the same time.**
- After `commit`/`abort` pointers to data in the mmap are invalid.

### 27.2. Lifecycle and integrity

- Do not open the database twice in one process; `fork()` — only with `resurrect_after_fork` in the child.
- An unclosed read-transaction "freezes" page reclamation — close/park it.
- Never copy the database file "on the fly" — use `mdbx_copy`.
- `MDBX_UTTERLY_NOSYNC` — only for non-critical data.

### 27.3. Space and growth

- Set the geometry once before open; do not lower `upper`.
- Lower `rp_augment_limit` — only together with `gc_time_limit`.
- Long values: increase the page size (up to 64 KB), split records.
- A value spanning several overflow pages needs a run of **consecutive** pages: when large values are
  rewritten often, watch fragmentation (`mdbx_chk` histograms, overflow page counts in
  `mdbx_env_stat`); for "heavy" data consider storing it outside the database.
- If a long value must be kept in the DB, split it into chunks that fit into a single page
  (`mdbx_limits_valsize4page_max()`): in a plain table — a chunk-number suffix in the key, in a
  DUPSORT table — a prefix with the multivalue number and the chunk number. Manual splitting is
  needed only until the planned **streamed API for long values** ("Streaming BLOBs") arrives.
  Details — [Restrictions & gotchas](https://libmdbx.dqdkfa.ru/docs/en/overview/restrictions.html).

### 27.4. Operations

- Do deletions before insertions in the same transaction; mass deletions — `bunch_delete`/`delete_range`.
- Do not build a `std::map` over the database for ordering — the gain is usually less than the cost.
- For DUPSORT deletion while iterating — use two cursors.

### 27.5. Typical counter-examples

- Violating "one transaction — one thread" → random `BAD_RSLOT`/overlaps.
- Opening the database twice → races and corrupted registrations.
- A forgotten reader → the file grows to `MDBX_MAP_FULL`.
- A debug build in production → multiple slowdowns.

**Fragment from [`examples/c++/34-rules-counter.c++`](examples/c++/34-rules-counter.c++)** — counter-example of a "forgotten" read snapshot: a thread opens a snapshot (`env.start_read()`) and holds it (`abort()` only via an external flag); while the reader is alive, the writer hits `MDBX_MAP_FULL` (GC is frozen, the file grows):

```cpp
    std::atomic<bool> reader_ready{false};
    std::atomic<bool> release_reader{false};
    std::thread holder([&] {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      (void)rtxn.get(table, mdbx::slice("k0"));
      reader_ready = true;
      while (!release_reader.load())
        std::this_thread::yield();
      rtxn.abort();
    });
    while (!reader_ready.load())
      std::this_thread::yield();
```

Full code: [34-rules-counter.c++](examples/c++/34-rules-counter.c++).

---

> **Examples for this chapter:** [`examples/c++/34-rules-counter.c++`](examples/c++/34-rules-counter.c++).

### 27.6. Summary of chapter 27

- One transaction — one thread: passing a transaction between threads is UB/an error (`MDBX_THREAD_MISMATCH`), even with `NOSTICKYTHREADS` a deadlock is possible.
- An unclosed read-transaction "freezes" page reclamation — the file grows to `MDBX_MAP_FULL`; close/park it.
- Do not open the database twice in one process; `fork()` — only with `resurrect_after_fork` in the child; copying — only `mdbx_copy`.
- Do not use `MDBX_UTTERLY_NOSYNC` in production.
- Set the geometry once before open, do not lower `upper`; for long values increase the page size (up to 64 KB), split into chunks that fit into one page, or store outside the DB; values that do not fit one page need a contiguous run and are sensitive to fragmentation — until the streaming API ("Streaming BLOBs") arrives.
- Do deletions before insertions; mass deletions — `bunch_delete`/`delete_range`; DUPSORT deletion while iterating — two cursors.
- Typical counter-examples: violating thread discipline (`BAD_RSLOT`), double opening, a forgotten reader, a debug build in production.

### 27.7. Chapter 27 checklist

- [ ] Ensure transactions are not passed between threads and are not used from two threads simultaneously.
- [ ] Close/park read-transactions immediately after use.
- [ ] Copy the database file only via `mdbx_copy`.
- [ ] Set the geometry before open and do not lower `upper`.
- [ ] Exclude `MDBX_UTTERLY_NOSYNC` from production configuration.
- [ ] Perform deletions before insertions; mass deletions — via `bunch_delete`/`delete_range`.
- [ ] For DUPSORT deletion while iterating use two cursors.

## Chapter 28. Platform-specific notes

### 28.1. Linux

- `boot_id` is taken into account when rolling back weak metas; in LXC — shared/missing.
- `/dev/shm` (tmpfs): create with headroom; `ENOSPC` from `fallocate()` can be ignored;
  since 0.13.8 fallocate protects against SIGBUS.
- Page cache: unified page cache incoherency (#269) — `MDBX_FORCE_CHECK_MMAP_COHERENCY`.
- `mincore`/`madvise(MADV_NOHUGEPAGE)` — opting out of THP.
- Linux < 4.x: `mdbx_env_create()` → `MDBX_INCOMPATIBLE`.

### 28.2. Windows

- `LockFileEx` is slower than named mutexes; small transactions are expensive.
- 32-bit: address-space limits; `/LARGEADDRESSAWARE` (+1 GB to `MAX_MAPSIZE32`).
- File compaction — only in a single-process scenario; extension via Native API;
  geometry changes suspend threads (SRWL — the Windows Slim Reader/Writer Lock,
  Windows' fine-grained locking primitive).
- WSL1: `ENOLCK` — operation is fundamentally impossible.

### 28.3. macOS / iOS

- `fcntl(F_FULLFSYNC)` by default (maximum durability);
  `MDBX_APPLE_SPEED_INSTEADOF_DURABILITY` — sacrifice durability for speed.
- SysV semaphores by default (flavour `SYSV`).
- `fcntl(F_PREALLOCATE)` reserves at the end — the DB grows at twice the standard rate.

### 28.4. Android / bionic

- TLS and atomics differ from glibc; 32-bit Android — a possible hang when rolling back a commit
  (historically; no problem since 0.11.7).

### 28.5. Containers

- LXC: boot_id; Docker — PID uniqueness.
- tmpfs: ENOSPC from fallocate (ignore).
- NFS/CIFS/SMB: only exclusive mode or cooperative read-only; copying works.

### 28.6. Wine

- Mechanisms of dynamic database resizing are not implemented — set a fixed
  sufficient size; `mdbx_module_handler(...)` at startup for static linking.

**Fragment from [`examples/c++/35-platform-notes.c++`](examples/c++/35-platform-notes.c++)** — a platform caveat in code: an "absurd" geometry request is handled via try/catch, because on tmpfs/`WSL1`/32-bit Windows `ENOSPC`/`ENOLCK` is a normal reaction, not a fatal failure:

```cpp
    // Checking geometry bounds must not crash: we request an "absurd"
    // upper bound and carefully handle the result.
    mdbx::env::geometry probe;
    probe.size_upper = intptr_t(8) * mdbx::env::geometry::TB;
    try {
      env.set_geometry(probe);
      std::cout << "geometry probe: accepted\n";
    } catch (const std::exception &ex) {
      // On tmpfs/WSL1/32-bit Windows this may fail —
      // for the example it matters that it is caught, not that it crashes the process.
      std::cout << "geometry probe: rejected (" << ex.what() << ")\n";
    }
```

Full code: [35-platform-notes.c++](examples/c++/35-platform-notes.c++).

> **Examples for this chapter:** [`examples/c++/35-platform-notes.c++`](examples/c++/35-platform-notes.c++).

### 28.7. Summary of chapter 28

- Platform differences concern locking, synchronization and address-space sizes.
- LXC/boot_id and page cache — container/Linux specifics.
- Windows LockFileEx and F_FULLFSYNC on macOS — the main platform pitfalls.

### 28.8. Chapter 28 checklist

- [ ] Linux: account for `boot_id` (LXC), tmpfs `ENOSPC` from fallocate, opt out of THP if needed.
- [ ] Windows: remember expensive small transactions and 32-bit limits (`/LARGEADDRESSAWARE`).
- [ ] macOS: consciously choose between `fcntl(F_FULLFSYNC)` and `MDBX_APPLE_SPEED_INSTEADOF_DURABILITY`.
- [ ] Android/bionic: verify behavior on 32-bit (the historical hang when rolling back a commit — before 0.11.7).
- [ ] Containers: LXC — boot_id, Docker — PID uniqueness; NFS/CIFS/SMB — only exclusive or cooperative read-only.
- [ ] Wine: set a fixed sufficient database size (dynamic resizing does not work).
- [ ] Treat `ENOSPC`/`ENOLCK` on tmpfs/WSL1/32-bit Windows as normal errors, not fatal failures.

---

## Chapter 29. Handle-Slow-Readers (HSR)

### 29.1. What HSR is and why it is needed

When the database is full **because of long readers** (GC is frozen), the library calls the HSR callback —
the only standard way for the application to influence the outcome of the "long reader versus
writer" conflict.

### 29.2. Signature and parameters

```c
typedef int (*MDBX_hsr_func)(const MDBX_env *env, const MDBX_txn *txn,
                             mdbx_pid_t pid, mdbx_tid_t tid,
                             uint64_t laggard, unsigned gap,
                             size_t space, int retry);
```

- `laggard` — the lag of the problematic reader;
- `gap` — the number of attempts;
- `space` — the volume that will be freed after the reader finishes;
- `retry` — the counter of repeated invocations.

### 29.3. Return values

| Value | Action                                                              |
| ----- | -------------------------------------------------------------------- |
| `0`   | Callback waited/decided; libmdbx re-scans RLT and retries            |
| `1`   | Reader transaction aborted asynchronously; clear the slot immediately |
| `2+`  | Reader process killed; libmdbx clears its registration               |

Called only when the database is full because of readers.

### 29.4. Typical implementations

- **Logging** + return 0 (wait).
- **Graceful shutdown**: set a flag for the reader, return 1.
- **Forced**: kill the reader process, return 2.
- **Agreement to grow**: if geometry allows — increase `upper` and return 0.

### 29.5. HSR and SAFE_NOSYNC

In `SAFE_NOSYNC` a "quasi-long reader" is a steady-commit, not a live slot: HSR does not
affect it. Growth must be managed via auto-sync (`syncbytes`/`syncperiod`).

**Fragment from [`examples/c++/36-hsr.c++`](examples/c++/36-hsr.c++)** — the HSR callback: reports the problematic reader (pid/lag/space), asks it to release the snapshot and waits for the actual release, then returns `MDBX_RESULT_TRUE`. Registration — `env.set_HandleSlowReaders(hsr_callback)`; together with `SAFE_NOSYNC` the callback remains the main tool against long readers:

```cpp
// The callback is invoked when the DB "hits" the limit because of readers
// holding old snapshots. Here: report the event, ask the reader to
// release the snapshot and wait for it, then return MDBX_RESULT_TRUE
// ("problem resolved" — the writer will continue).
int hsr_callback(const MDBX_env *, const MDBX_txn *, mdbx_pid_t pid, mdbx_tid_t tid, uint64_t laggard,
                 unsigned gap, size_t space, int retry) noexcept {
  std::cout << "hsr invoked: laggard pid=" << pid << " tid=" << tid << " lag=" << laggard << " gap=" << gap
            << " space=" << space << " retry=" << retry << "\n";
  hsr_called = true;
  hsr_release_reader = true;
  while (!reader_released.load())
    std::this_thread::yield(); // wait for the actual release
  return MDBX_RESULT_TRUE;
}
```

Full code: [36-hsr.c++](examples/c++/36-hsr.c++).

### 29.6. Example implementation

**Fragment (C, illustration):** the full compilable version is in
[`examples/c++/36-hsr.c++`](examples/c++/36-hsr.c++).

```c
static int my_hsr(const MDBX_env *env, const MDBX_txn *txn,
                  mdbx_pid_t pid, mdbx_tid_t tid,
                  uint64_t laggard, unsigned gap,
                  size_t space, int retry) {
    fprintf(stderr, "HSR: reader pid=%d lag=%llu space=%zu retry=%d\n",
            (int)pid, (unsigned long long)laggard, space, retry);
    if (retry > 3)
        return 2; /* kill the problematic reader */
    return 0;     /* wait more */
}
/* setup */
mdbx_env_set_hsr(env, my_hsr);
```

> **Examples for this chapter:** [`examples/c++/36-hsr.c++`](examples/c++/36-hsr.c++).

### 29.7. Summary of chapter 29

- HSR — a callback for resolving conflicts with long readers.
- Parameters: laggard/gap/space/retry; returns 0/1/2+.
- Typical strategies: wait/kill/grow.
- With SAFE_NOSYNC manage growth via auto-sync.

### 29.8. Chapter 29 checklist

- [ ] Install the HSR callback (`mdbx_env_set_hsr` / `set_HandleSlowReaders`).
- [ ] Define the policy per `retry`: logging, graceful shutdown, forced kill, agreement to grow.
- [ ] Use the `laggard`/`gap`/`space`/`retry` parameters to make the decision in the callback.
- [ ] Remember the return semantics: 0 — wait, 1 — abort the reader, 2+ — kill the reader process.
- [ ] Under `SAFE_NOSYNC` configure auto-sync (`syncbytes`/`syncperiod`) — HSR does not affect a steady-commit.

---

## Chapter 30. Diagnostics and debugging

### 30.1. Tools

**`mdbx_chk`** — integrity check. Key options:

| Option       | What it does                                                           |
| ------------ | ---------------------------------------------------------------------- |
| `-v…-vvvvv` | Output verbosity                                                        |
| `-q`         | Complete silence                                                        |
| `-c`         | Cooperative (non-exclusive) mode; full check — only exclusive           |
| `-w`         | Read-write: rollback to steady, check meta txnids                       |
| `-d`         | Page-by-page B+tree walk; without it lost/double-used pages cannot be found |
| `-i`         | Ignore false order errors (custom comparators)                          |
| `-s table`   | Check only the table                                                    |
| `-0/-1/-2`   | Specific meta; `-t`/`-T` — switch to it                                 |

Exit code: `0` = no errors.

**`MDBX_ENABLE_PROFGC`** — GC profile in `commit_latency.gc_prof`. Key fields:
`max_reader_lag`, `max_retained_pages`, `work_rtime_monotonic`/`work_xtime_cpu`,
`work_rsteps`/`work_xpages`, `work_majflt`, `self_*`, `wloops`, `flushes`, `kicks`.

**`MDBX_commit_latency`** — commit stages: `preparation`, `gc_wallclock`, `audit`, `write`, `sync`,
`ending`, `whole`, `gc_cputime`.

**`mdbx_txn_info()`** — `txn_reader_lag`, `txn_space_used/limit_soft/limit_hard/retired/leftover/dirty`.
For long readers `txn_space_retired` and `txn_space_leftover` matter.

**Debug build** — `MDBX_DEBUG`/`MDBX_CHECKING` (−1..3); `MDBX_FORCE_ASSERTIONS` deprecated.
Sanitizers: `ENABLE_ASAN`/`ENABLE_UBSAN`/`ENABLE_MEMCHECK`; TSAN is not in CMake (manually).

**Fragment from [`examples/c++/37-diagnostics.c++`](examples/c++/37-diagnostics.c++)** — reading `mdbx_txn_info` (via `txn.get_info(...)`: id/lag/used/dirty) and the reader list (`mdbx_reader_list`, here `env.enumerate_readers(...)`) — the very data for the "DB growth" and `MDBX_MAP_FULL` algorithms above:

```cpp
      // Information about the current writing transaction.
      const auto info = txn.get_info(true /* scan_rlt */);
      std::cout << "txn_info: id=" << info.txn_id << " lag=" << info.txn_reader_lag << " used=" << info.txn_space_used
                << " dirty=" << info.txn_space_dirty << "\n";
      txn.commit();
    }

    // List of readers (RLT).
    struct visitor {
      int operator()(const mdbx::env::reader_info &ri, int) {
        std::cout << "reader slot=" << ri.slot << " pid=" << ri.pid << " tid=" << ri.thread
                  << " txnid=" << ri.transaction_id << " lag=" << ri.transaction_lag << "\n";
        return mdbx::continue_loop;
      }
    } v;
    env.enumerate_readers(v);
```

Full code: [37-diagnostics.c++](examples/c++/37-diagnostics.c++).

### 30.2. Step-by-step algorithm: DB growth

1. `mdbx_env_info_ex()` — metas and geometry (`size_now` vs `size_upper`).
2. `mdbx_reader_list()` / `mdbx_reader_check()` — who holds snapshots.
3. `mdbx_stat -r` — retained.
4. PROFGC: `max_reader_lag`, `max_retained_pages`, `kicks`.
5. `mdbx_stat -p` — `newly` vs `cow` (new pages instead of reuse).
6. Decision: parking/HSR/steady-point/defragmentation; under `SAFE_NOSYNC` — auto-sync.

### 30.3. Step-by-step algorithm: commit slowdown

1. `mdbx_txn_commit_ex()` — decompose `whole` into stages.
2. `sync` dominates → durability mode; too frequent fsync → batch or `NOMETASYNC`/`SAFE_NOSYNC`.
3. `gc_wallclock` is large → PROFGC: `work_rsteps`/`work_xpages`/`work_majflt` (fragmentation/large
   values).
4. `write` is large with small data → early spill (dp_limit) or duplicated dirty pages.
5. WRITEMAP + page-faults → `prefault_write_enable`.
6. `mdbx_stat -p` — msync/fsync counters.

### 30.4. Step-by-step algorithm: MDBX_MAP_FULL

1. **Abort the current write-transaction** (continuation after resize → `BAD_TXN`).
2. `mdbx_txn_info()` — `txn_space_limit_hard` vs used.
3. `mdbx_reader_list()` — old snapshots pin space.
4. PROFGC — `kicks`, `max_reader_lag`, `max_retained_pages`.
5. Check HSR (`mdbx_env_set_hsr`), parking, geometry (`upper` lowered?).
6. Decision: HSR (wait/kill/grow), evict parked, new steady, increase `upper`
   before open, defragmentation.

### 30.5. Step-by-step algorithm: deadlock / MDBX_BUSY

1. Check thread discipline: codes `THREAD_MISMATCH`/`TXN_OVERLAPPING`/`BAD_RSLOT`.
2. Under `NOSTICKYTHREADS` — exclude synchronous write-functions (`set_option/set_flags/set_geometry/
sync/stat/defrag/close`) from a thread not owning the writing transaction.
3. Audit `fork()` (resurrect) and repeated `env_open`.
4. `mdbx_reader_list()` will show foreign owners.

### 30.6. Step-by-step algorithm: DB corruption

1. `mdbx_chk -w -vvv` — localize (meta? tree? GC? order?).
2. Check the environment: `boot_id` (LXC?), page cache incoherency (#269, `incoherence`).
3. One meta corrupted → `mdbx_chk -1/-2` + `-T` (switch to a valid one).
4. `MDBX_WANNA_RECOVERY` → read-write or `mdbx_env_open_for_recovery()` (since 0.12.7 it does not modify the database).
5. Recovery checks are safe; restore from backup (`mdbx_copy`).

### 30.7. Step-by-step algorithm: reader slot leak

1. `mdbx_reader_check(env, &dead)` — how many dead slots.
2. Cause: threads terminated without cleanup (TLS destructor; glibc #21031/#21032; DSO unload).
3. Solution: explicit `mdbx_thread_register/unregister`; the `reset+renew` pattern; `resurrect_after_fork`.

### 30.8. Checklists before production deployment

- [ ] Geometry set before open (`upper` adequate; the engine itself chooses the default ≈ golden ratio of RAM).
- [ ] HSR callback installed.
- [ ] Sync mode consciously chosen.
- [ ] Release build without asserts.
- [ ] `maxreaders` covers the number of threads.
- [ ] Long reads use parking/reset+renew.
- [ ] Post-crash test: kill -9 + open + `mdbx_chk`.

> **Examples for this chapter:** [`examples/c++/37-diagnostics.c++`](examples/c++/37-diagnostics.c++).

### 30.9. Summary of chapter 30

- Tools: chk, PROFGC, commit_latency, txn_info, reader_check, debug build.
- 6 step-by-step algorithms: growth, commit slowdown, MAP_FULL, deadlock, corruption, slot leak.
- Pre-deployment checklist is mandatory.

### 30.10. Chapter 30 checklist

- [ ] Master `mdbx_chk` (options `-d`, `-w`, `-0/-1/-2`/`-T`) as the main integrity tool.
- [ ] Diagnose DB growth per the algorithm in §30.2 (`mdbx_reader_list`, `mdbx_stat -r`/`-p`, PROFGC).
- [ ] Decompose commit slowdown into `MDBX_commit_latency` stages (`mdbx_txn_commit_ex`) — §30.3.
- [ ] On `MDBX_MAP_FULL` — abort the transaction, check readers, HSR and geometry — §30.4.
- [ ] Deadlock/`MDBX_BUSY` — audit thread discipline and `NOSTICKYTHREADS` — §30.5.
- [ ] DB corruption — `mdbx_chk -w -vvv`, restore from backup (`mdbx_copy`) — §30.6.
- [ ] Check reader slot leaks via `mdbx_reader_check` — §30.7.
- [ ] Pass the production-deployment checklist §30.8 (including kill -9 + open + `mdbx_chk`).

---

## Chapter 31. Migrating from LMDB

### 31.1. Why migrate

libmdbx is a reworked LMDB: more durability modes, GC instead of free-list, stricter
recovery guarantees, higher write performance with batching. **The data format is not
compatible with LMDB**: the triple of meta pages, two-phase commit, checksums and the GC tree are
own changes; LMDB files are not opened directly, migration is performed by transferring data.

### 31.2. Format compatibility

- **LMDB files are not opened** by libmdbx directly: the formats differ (triple of meta pages,
  two-phase commit, checksums, GC tree).
- **libmdbx 0.11.x ↔ 0.12.x** databases are mutually compatible (the format is frozen since v11.3).
- Migration from LMDB is a data transfer (`mdbx_dump`/`mdbx_load`, `mdbx_copy`), not
  a file rename.

### 31.3. Breaking changes

- `MDBX_NOLOCK` removed.
- `MDBX_NOTLS` → `MDBX_NOSTICKYTHREADS`.
- Other renames and new flag requirements — check against `mdbx.h`.

### 31.4. mdb_* → mdbx_* correspondence table

| LMDB              | libmdbx                |
| ----------------- | ---------------------- |
| `mdb_env_create`  | `mdbx_env_create`      |
| `mdb_env_open`    | `mdbx_env_open`        |
| `mdb_txn_begin`   | `mdbx_txn_begin`       |
| `mdb_dbi_open`    | `mdbx_dbi_open`        |
| `mdb_put/get/del` | `mdbx_put/get/del`     |
| `mdb_cursor_get`  | `mdbx_cursor_get`      |
| `MDB_NOTLS`       | `MDBX_NOSTICKYTHREADS` |

**Fragment from [`examples/c/38-migration.c`](examples/c/38-migration.c)** — the `mdb_*` → `mdbx_*` correspondence map from the example header:

```c
// Macro/name map for porting code from LMDB (the full table is in §31.4):
//   mdb_env_create      → mdbx_env_create
//   mdb_env_open        → mdbx_env_open
//   mdb_env_close       → mdbx_env_close
//   mdb_env_set_mapsize → mdbx_env_set_geometry (a different API!)
//   mdb_txn_begin       → mdbx_txn_begin
//   mdb_txn_commit      → mdbx_txn_commit
//   mdb_txn_abort       → mdbx_txn_abort
//   mdb_dbi_open        → mdbx_dbi_open
//   mdb_get/put/del     → mdbx_get/mdbx_put/mdbx_del
//   mdb_cursor_open/get → mdbx_cursor_open/mdbx_cursor_get
//   MDB_NOTLS           → MDBX_NOSTICKYTHREADS
//   MDB_NOSYNC          → MDBX_SAFE_NOSYNC or MDBX_UTTERLY_NOSYNC
//   MDB_APPEND          → MDBX_APPEND
//   MDB_INTEGERKEY      → MDBX_INTEGERKEY
//   mdb_strerror        → mdbx_strerror
```

Full code: [38-migration.c](examples/c/38-migration.c).

### 31.5. Behavioral differences

- Three metas vs two; two-phase commit.
- Page checksums.
- More diagnostic codes (`WANNA_RECOVERY`, `MVCC_RETARDED` and others).
- Empty keys/values are allowed.

### 31.6. Migration checklist

1. Make a backup (`mdbx_copy -c` or dump+load).
2. Open the database read-only — check with `mdbx_chk`.
3. Replace the calls per the correspondence table.
4. Run the tests under ASAN/UBSAN.
5. Tune geometry/modes for the new engine (do not blindly port LMDB settings).

### 31.7. Summary of chapter 31

- The libmdbx data format is not compatible with LMDB: the triple of meta pages, two-phase commit, checksums, GC tree; LMDB files are not opened directly.
- Migration is a data transfer (`mdbx_dump`/`mdbx_load`, `mdbx_copy`), not a file rename; libmdbx 0.11.x ↔ 0.12.x databases are mutually compatible (the format is frozen since v11.3).
- Breaking changes: `MDBX_NOLOCK` removed, `MDBX_NOTLS` → `MDBX_NOSTICKYTHREADS`; check other renames against `mdbx.h`.
- Function names change predictably (`mdb_env_create` → `mdbx_env_create`), but `mdb_env_set_mapsize` → `mdbx_env_set_geometry` is a different API.
- Behavioral differences: three metas vs two, page checksums, more diagnostic codes, empty keys/values allowed.
- Migration order: backup → read-only check with `mdbx_chk` → replace calls → tests under ASAN/UBSAN → re-tune geometry/modes.

### 31.8. Chapter 31 checklist

- [ ] Make a backup (`mdbx_copy -c` or dump+load) before any changes.
- [ ] Open the database read-only and verify it with `mdbx_chk`.
- [ ] Replace the calls per the correspondence table §31.4, including `mdb_env_set_mapsize` → `mdbx_env_set_geometry`.
- [ ] Replace the flags: `MDBX_NOLOCK` removed, `MDBX_NOTLS` → `MDBX_NOSTICKYTHREADS`.
- [ ] Run the tests under ASAN/UBSAN.
- [ ] Tune geometry and modes for libmdbx from scratch, do not blindly port LMDB settings.

---

> **Examples for this chapter:** [`examples/c/38-migration.c`](examples/c/38-migration.c) (mdb_* → mdbx_* correspondence map — in the file header).

## Chapter 32. Design patterns on libmdbx

### 32.1. Pattern 1: Key-value with auto-increment ID

There are two approaches to monotonic IDs: the built-in `mdbx_dbi_sequence()` (an atomic
per-table counter) and the classic portable approach — a counter in a service table. The example
below demonstrates the second variant: it works identically across all libmdbx versions and
doesn't depend on `MDBX_LIFORECLAIM` subtleties.

**Fragment from [`examples/c++/39-pattern-sequence-id.c++`](examples/c++/39-pattern-sequence-id.c++)** — auto-increment ID: the counter lives in the `meta` table and is incremented in the same writing transaction as the record insert:

```cpp
uint64_t next_id(mdbx::txn_managed &txn, const mdbx::map_handle &meta) {
  constexpr auto counter_key = mdbx::slice("seq");
  uint64_t current = 0;
  try {
    current = txn.get(meta, counter_key).as_uint64();
  } catch (const mdbx::not_found &) {
    current = 0;
  }
  ++current;
  txn.upsert(meta, counter_key, mdbx::slice::wrap(current));
  return current;
}
```

Full code: [39-pattern-sequence-id.c++](examples/c++/39-pattern-sequence-id.c++).

### 32.2. Pattern 2: Secondary index (DUPSORT)

Main table + "field → list of IDs" index (Volume II, chapter 8). Update in one transaction.

**Fragment from [`examples/c++/40-pattern-secondary-index.c++`](examples/c++/40-pattern-secondary-index.c++)** — secondary index "field → list of IDs": the main table `users` and the index `by_role` (DUPSORT, `value_mode::multi`) are updated in one transaction:

```cpp
    auto txn = env.start_write();
    // Main table: id → {name, role}.
    auto users = txn.create_map("users", mdbx::key_mode::ordinal, mdbx::value_mode::single);
    // Index: role → list of ids (multi-values).
    auto by_role = txn.create_map("by_role", mdbx::key_mode::usual, mdbx::value_mode::multi);

    struct rec {
      uint64_t id;
      const char *name;
      const char *role;
    };
    static const rec records[] = {{1, "alice", "admin"}, {2, "bob", "dev"}, {3, "carol", "admin"}};
    for (const auto &r : records) {
      txn.upsert(users, mdbx::slice::wrap(r.id), mdbx::slice(std::string(r.name) + "|" + r.role));
      txn.upsert(by_role, mdbx::slice(r.role), mdbx::slice::wrap(r.id));
    }
    txn.commit();
```

Full code: [40-pattern-secondary-index.c++](examples/c++/40-pattern-secondary-index.c++).

### 32.3. Pattern 3: Composite key

Concatenation of fields + comparator (or big-endian for numbers). Prefix search — `SET_RANGE`.

**Fragment from [`examples/c++/41-pattern-composite-key.c++`](examples/c++/41-pattern-composite-key.c++)** — composite key: fixed-width fields are packed into big-endian, for which the lexicographic order matches the numeric one (range search — `SET_RANGE`):

```cpp
uint64_t pack(uint16_t year, uint16_t month) {
  const uint64_t value = (uint64_t(year) << 48) | (uint64_t(month) << 32);
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return __builtin_bswap64(value);
#else
  return value;
#endif
}

uint16_t decode_year(const mdbx::slice &key) {
  const auto *p = static_cast<const uint8_t *>(key.data());
  return uint16_t((uint16_t(p[0]) << 8) | p[1]);
}

uint16_t decode_month(const mdbx::slice &key) {
  const auto *p = static_cast<const uint8_t *>(key.data());
  return uint16_t((uint16_t(p[2]) << 8) | p[3]);
}
```

Full code: [41-pattern-composite-key.c++](examples/c++/41-pattern-composite-key.c++).

### 32.4. Pattern 4: Task queue (table-as-queue)

`MDBX_DUPSORT` + sequence: key — priority/number, value — task. Natural retrieval order.

**Fragment from [`examples/c++/42-pattern-queue.c++`](examples/c++/42-pattern-queue.c++)** — task queue: key — monotonic number (ordinal), the consumer takes the first element via a cursor and deletes it:

```cpp
    // Consumer: takes the first element via a cursor and deletes it.
    auto txn = env.start_write();
    auto queue = txn.open_map("queue", mdbx::key_mode::ordinal, mdbx::value_mode::single);
    std::string dequeued;
    auto cur = txn.open_cursor(queue);
    while (true) {
      auto r = cur.to_first(false);
      if (!r)
        break;
      const auto id = r.key.as_uint64();
      const auto task = r.value.as_string();
      dequeued += std::to_string(id) + ":" + std::string(task) + " ";
      cur.erase(false);
    }
    txn.commit();
    std::cout << "dequeued: " << dequeued << "\n";
```

Full code: [42-pattern-queue.c++](examples/c++/42-pattern-queue.c++).

### 32.5. Pattern 5: Ring buffer

Bounded size: when exceeded, delete the oldest keys (cursor + `del`).

**Fragment from [`examples/c++/43-pattern-ring-buffer.c++`](examples/c++/43-pattern-ring-buffer.c++)** — ring buffer of the last N: fixed slots `0..N-1`, writing overwrites `counter % N`:

```cpp
    // Writing 12 elements into a buffer of 5 slots.
    {
      auto txn = env.start_write();
      auto ring = txn.create_map("ring", mdbx::key_mode::ordinal, mdbx::value_mode::single);
      for (unsigned i = 0; i < 12; ++i) {
        const unsigned slot = i % kSlots;
        txn.upsert(ring, buffer::key_from_u64(slot), mdbx::slice("s" + std::to_string(i)));
      }
      txn.commit();
    }
```

Full code: [43-pattern-ring-buffer.c++](examples/c++/43-pattern-ring-buffer.c++).

### 32.6. Pattern 6: Full-scan iterator

Cursor + `get_batch`/`bunch_delete` for bulk processing.

**Fragment from [`examples/c++/44-pattern-full-scan.c++`](examples/c++/44-pattern-full-scan.c++)** — full scan with the cursor `to_first()` → `to_next()` with a filter:

```cpp
    auto rtxn = env.start_read();
    auto table = rtxn.open_map(nullptr);
    auto cur = rtxn.open_cursor(table);

    // Full scan with the "even keys" filter.
    size_t scanned = 0, matched = 0;
    for (auto r = cur.to_first(); r; r = cur.to_next(false)) {
      ++scanned;
      if (r.key.as_string().size() % 2 == 0)
        ++matched;
    }
    std::cout << "scanned " << scanned << " entries\n";
```

Full code: [44-pattern-full-scan.c++](examples/c++/44-pattern-full-scan.c++).

### 32.7. Pattern 7: Replication via per-table txnid

Change markers: store the transaction number of the last change; a secondary process polls
and copies deltas.

**Fragment from [`examples/c++/45-pattern-replication.c++`](examples/c++/45-pattern-replication.c++)** — replication via per-table txnid: the value stores a version (`data|N`), the replica reads only records newer than its `last_seen`:

```cpp
    // The replica reads everything newer than its last txnid.
    auto replicate = [&](uint64_t last_seen) -> size_t {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      size_t n = 0;
      auto cur = rtxn.open_cursor(table);
      for (auto r = cur.to_first(); r; r = cur.to_next(false)) {
        if (version_of(r.value) > last_seen)
          ++n;
      }
      rtxn.abort();
      return n;
    };
```

Full code: [45-pattern-replication.c++](examples/c++/45-pattern-replication.c++).

### 32.8. Pattern 8: Read-your-writes via clone / embark_read

`mdbx_txn_clone`/`embark_read` — own writes are visible immediately in the same logical
operation without a separate transaction.

**Fragment from [`examples/c++/46-pattern-read-your-writes.c++`](examples/c++/46-pattern-read-your-writes.c++)** — read-your-writes: the same writing transaction immediately reads its own change; after `commit` it is also visible to a fresh reader:

```cpp
    // The writing transaction immediately reads its own change.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      std::cout << "within txn sees: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";
      txn.commit();
    }

    // After the commit the change is visible to a fresh reader too.
    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::cout << "after commit via fresh txn sees: " << rtxn.get(table, mdbx::slice("key")).as_string() << "\n";
      rtxn.abort();
    }
```

Full code: [46-pattern-read-your-writes.c++](examples/c++/46-pattern-read-your-writes.c++).

Each pattern: problem statement → solution with code → trade-offs → alternatives (in the full version
of the knowledge base — `knowledge-base/`).

### 32.9. Summary of chapter 32

- Auto-increment ID: a counter in a service table, incremented in the same write transaction as the insert; the alternative is the built-in `mdbx_dbi_sequence()`.
- A secondary (DUPSORT) index "field → list of IDs" is updated in one transaction with the main table.
- Composite key: fixed-width fields in big-endian (lexicographic order = numeric), range search — `SET_RANGE`.
- Task queue: an ordinal key (monotonic number), the consumer takes the first element via a cursor and deletes it.
- Ring buffer: fixed slots `0..N-1`, writing overwrites `counter % N`.
- Full scan — cursor `to_first()` → `to_next()`, bulk processing — `get_batch`/`bunch_delete`.
- Replication via per-table txnid (a version marker in the value) and read-your-writes via `mdbx_txn_clone`/`embark_read`.

### 32.10. Chapter 32 checklist

- [ ] For monotonic IDs choose `mdbx_dbi_sequence()` or a counter in a service table (the portable variant).
- [ ] Update the secondary index (DUPSORT, `value_mode::multi`) in the same transaction as the main record.
- [ ] For composite keys use big-endian/a comparator and range search via `SET_RANGE`.
- [ ] Implement the task queue via an ordinal key and deletion of the first element by a cursor.
- [ ] For bounded structures (ring buffer) delete the oldest records on overflow.
- [ ] Perform bulk scans/deletions via `get_batch`/`bunch_delete`.
- [ ] Build replication on txnid markers, the replica — on a read-only snapshot filtered by `last_seen`.

---

> **Examples for this chapter:**
> [`examples/c++/39-pattern-sequence-id.c++`](examples/c++/39-pattern-sequence-id.c++);
> [`examples/c++/40-pattern-secondary-index.c++`](examples/c++/40-pattern-secondary-index.c++);
> [`examples/c++/41-pattern-composite-key.c++`](examples/c++/41-pattern-composite-key.c++);
> [`examples/c++/42-pattern-queue.c++`](examples/c++/42-pattern-queue.c++);
> [`examples/c++/43-pattern-ring-buffer.c++`](examples/c++/43-pattern-ring-buffer.c++);
> [`examples/c++/44-pattern-full-scan.c++`](examples/c++/44-pattern-full-scan.c++);
> [`examples/c++/45-pattern-replication.c++`](examples/c++/45-pattern-replication.c++);
> [`examples/c++/46-pattern-read-your-writes.c++`](examples/c++/46-pattern-read-your-writes.c++).

## Chapter 33. Roadmap and the future: MithrilDB

### 33.1. Current state

libmdbx as of 2026 — a mature engine (0.15.x devel-canon), the DB format frozen since 2018.
Fundamental GC/freelist improvements are planned only in the next generation.

### 33.2. MithrilDB

A common API for several storage formats; amalgamation simplifies distribution.

### 33.3. Replication

Prerequisites: change subscription, early GC cleanup (2025), non-linear GC processing (end-to-end
tracking of page usage by readable snapshots).

### 33.4. Other roadmap items

- Change subscription (mailboxes, ring buffers).
- Encryption and compression at the engine level.
- A streaming read/write mode for long values ("Streaming BLOBs") — without manual chunking, see
  [Restrictions & gotchas](https://libmdbx.dqdkfa.ru/docs/en/overview/restrictions.html).
- SWIG and cross-language interop.

### 33.5. Summary of chapter 33

- libmdbx as of 2026 — a mature engine (0.15.x devel-canon); the DB format frozen since 2018.
- Fundamental GC/freelist improvements are planned only in the next generation — MithrilDB: a common API for several storage formats, amalgamation simplifies distribution.
- Replication prerequisites: change subscription, early GC cleanup (2025), non-linear GC processing (end-to-end tracking of page usage by readable snapshots).
- Other roadmap items: engine-level encryption and compression, a streaming read/write mode for long values ("Streaming BLOBs"), SWIG and cross-language interop.

### 33.6. Chapter 33 checklist

- [ ] In long-term planning account for the DB format being frozen since 2018 (0.11.x ↔ 0.12.x compatibility).
- [ ] Do not expect fundamental GC/freelist improvements in the current generation — watch MithrilDB.
- [ ] For replication rely on the available prerequisites (change subscription, early GC cleanup).
- [ ] Treat roadmap features (encryption, compression, the streaming mode for long values / Streaming BLOBs, SWIG) as future, not current, capabilities.

---

## Volume summary

You know the rules and tips, platform-specific notes, HSR, diagnostics via proven algorithms,
migration from LMDB and design patterns.

**What's next:** Volume VI — bindings: how to use libmdbx from Rust, Go, Python,
Node.js, .NET, C++, Dart, Nim, Java, Haskell, Ruby, Scala.