# Volume IV. Performance and Optimization

> **Level:** for experienced developers.
> **Goal of the volume:** you know how to tune libmdbx for a specific scenario, understand the sources
> of write amplification and bottlenecks, and can measure and interpret metrics.

---

## Chapter 21. WAF — write amplification factor

### 21.1. What is WAF and why it matters

**WAF** (Write Amplification Factor) — the ratio of bytes actually written to disk to the amount of data
accepted from the application. WAF = 100 means: for every byte you write, the disk receives 100 bytes.
A high WAF means SSD wear, a shorter disk lifetime and worse throughput.

### 21.2. Sources of amplification

1. **Page granularity.** CoW does not modify an old page, it creates a new one. Changing
   one byte in a leaf = writing a whole page (e.g., 4 KB).
2. **Path to the root.** A new version of a leaf changes the pointer in its parent → the parent also
   becomes new → and so on up to the root. A point update = `(height+1) × pagesize` bytes written.
3. **GC writes and meta.** Freed pages are described in the GC-tree; two-phase meta adds another
   1–3 pages per commit.
4. **Spill.** A spilled page that is modified again is written once more.
5. **Merge/rebalance.** Merging half-empty pages can rewrite neighboring ones.

### 21.3. Batching — the main lever

Within a single transaction a page enters the dirty list **once**. N operations on M
unique pages write `M × pagesize` bytes (+GC+meta). Therefore:

- one operation per transaction: WAF = height+1 ≈ 4–5 (and higher);
- 10 000 operations in a single transaction: WAF approaches "1 page per many changes".

**Conclusion: batching (100–10 000 operations per commit) is the most powerful lever for reducing WAF.**

### 21.4. Influence of tree height

B+tree height ~ log_B(N), where B is the average number of children in a node (hundreds). Increasing the
data volume by 2^k times adds ~k levels. For point-update the specific WAF grows **logarithmically** with
volume — slowly.

### 21.5. Spill as a trade-off

When the dirty list overflows `dp_limit` (≈1/42 of RAM), some pages are flushed early.
If such a page is modified again — it is written once more (extra WAF). This is a
"memory vs WAF" trade-off.

### 21.6. prefer_waf_insteadof_balance and merge_threshold

The `prefer_waf_insteadof_balance` option intentionally sacrifices page fill uniformity for a
smaller number of written pages (reuse an already-modified neighbor page); it is enabled by
default. The merge threshold is `MDBX_opt_merge_threshold` (16.16 %; default 33%, allowed range
[12.5%..50%]). Both are direct levers of WAF.

### 21.7. WAF calculation for typical scenarios

For a point-update in a tree of height h on a 4 KB page: write ≈ `(h+1) × 4096` bytes per operation.
At h=4 and a 32-byte value: 20 KB / 32 B ≈ **WAF ≈ 640** (single operations!). With batching
of 10 000 records to the same 5 000 unique pages: `5 000 × 4096 / (10 000 × 32)` ≈ **WAF ≈ 64**.
As the batch grows further, WAF keeps decreasing.

> **Warning:** numbers without context are useless — always state the sync mode, page size,
> tree height and transaction size.

**Fragment from [`examples/c++/28-waf-batching.c++`](examples/c++/28-waf-batching.c++)** — measuring
written pages (`newly+cow` from `mi_pgop_stat`, the same counters as `mdbx_stat -p`) for different
write batch sizes:

```cpp
  constexpr int total = 1000;
  const auto initial = env.get_info().mi_pgop_stat;
  for (int off = 0; off < total; off += (int)batch_size) {
    auto txn = env.start_write();
    auto table = txn.open_map(nullptr);
    for (int i = off; i < off + (int)batch_size && i < total; ++i)
      txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("value"));
    txn.commit();
  }
  const auto final = env.get_info().mi_pgop_stat;
  const uint64_t pages = (final.newly - initial.newly) + (final.cow - initial.cow);
  std::cout << "batch=" << batch_size << ": pages=" << pages << " ops=" << total << " pages/op=" << (double)pages / total
            << "\n";
```

Full code: [28-waf-batching.c++](examples/c++/28-waf-batching.c++).

> **Examples for this chapter:** [`examples/c++/28-waf-batching.c++`](examples/c++/28-waf-batching.c++).

### 21.8. Chapter 21 summary

- WAF = written to disk / accepted from the application.
- Sources: page granularity, path to the root, GC/meta, spill, merge.
- Batching is the main lever: a page is written once per transaction.
- Growth with volume is logarithmic; large values give a linear contribution.

### 21.9. Exercises

1. Calculate WAF for a point-update at height 3 and a 4 KB page.
2. How will a batch of 1 000 updates on the same 500 pages affect it?

### 21.10. Chapter 21 checklist

- [ ] I can compute the write for a point-update as `(height+1) × pagesize` bytes and derive the WAF from it.
- [ ] I name all the sources of amplification: page granularity, path to the root, GC/meta, spill, merge/rebalance.
- [ ] I can explain why a page enters the dirty list once per transaction and why batching is the main lever for reducing WAF.
- [ ] I know that the specific WAF grows logarithmically with volume and can explain why (height ~ log_B(N)).
- [ ] I understand the spill trade-off: `dp_limit` ≈ 1/42 of RAM, and a spilled page being written again.
- [ ] I know the effect of `prefer_waf_insteadof_balance` and `MDBX_opt_merge_threshold`.
- [ ] I present WAF numbers only with context: sync mode, page size, tree height, transaction size.

---

## Chapter 22. Choosing a configuration for the scenario

### 22.1. Scenario 1: High write throughput (Ethereum-like)

- Mode: `MDBX_SAFE_NOSYNC` + auto-sync (`syncbytes`/`syncperiod`), or `NOMETASYNC`.
- Geometry: `upper` with headroom; large `growth_step` (fsync less often).
- Page: 4–8 KB.
- Batching: tens to thousands of operations per commit.
- Expected numbers: hundreds of thousands to millions of inserts/s (depends on hardware and key size).

### 22.2. Scenario 2: Read-heavy with rare writes (cache, index)

- Mode: default (`DURABLE`); writes are rare, reliability matters.
- Page: matched to the value size; short keys.
- `maxreaders` — according to the number of reader threads.
- Reads scale linearly across cores (wait-free).

### 22.3. Scenario 3: Embedded mobile application (Isar-like)

- Small database; 4 KB page.
- `MDBX_EXCLUSIVE` when exclusive access is needed.
- Careful read-only transactions; parking for long reads.
- Low WAF (little churn) — extends flash lifetime.

### 22.4. Scenario 4: Analytics, full DB scan

- Full-scan reading; important: a 64 KB page for sequential reads? (No — everything is read; what matters
  is locality and the absence of long writers.)
- Cursors + `get_batch`; range estimation via `estimate_*`.

### 22.5. Scenario 5: Multi-process reading (web server + workers)

- Several processes open one database read-only; one — write.
- `maxreaders` accounts for all processes; the LCK-file is shared.
- Be careful with `boot_id` in containers; page-cache coherence (#269).

### 22.6. Scenario 6: Low latency, small transactions

- Each transaction — one operation (minimal latency).
- Mode: `NOMETASYNC`/`SAFE_NOSYNC`, so as not to pay fsync for each one.
- On Windows remember LockFileEx (small transactions are expensive).
- Overhead guideline: ~0.5 µs per transaction on top of I/O.

**Fragment from [`examples/c++/29-scenario-configs.c++`](examples/c++/29-scenario-configs.c++)** —
typical configurations as functions returning `operate_parameters`: flags and options for each
scenario of the chapter:

```cpp
// Write-heavy: lazy sync, enough named tables.
params write_heavy() { return params().lazy_weak_tail().set_max_maps(64); }

// Read-heavy: maximum durability, many reader slots.
params read_heavy() { return params().robust_synchronous().set_max_readers(1024).set_max_maps(64); }

// Mobile device: small file size, gentle with flash memory.
params mobile() { return params().lazy_weak_tail().set_max_maps(64); }

// Analytics: batched writes, do not fuss with balancing.
params analytics() { return params().lazy_weak_tail().set_max_maps(64); }

// Multi-process read: many reader slots, stable geometry.
params multi_process_read() { return params().set_max_readers(4096).set_max_maps(64); }

// Low latency: NOMETASYNC (data on disk, metadata may lag).
params low_latency() { return params().half_synchronous_weak_last().set_max_maps(64); }
```

Full code: [29-scenario-configs.c++](examples/c++/29-scenario-configs.c++).

> **Examples for this chapter:** [`examples/c++/29-scenario-configs.c++`](examples/c++/29-scenario-configs.c++).

### 22.7. Chapter 22 summary

- The sync mode is chosen by durability requirements.
- Geometry is set with headroom; do not undersize `upper`.
- Batching scales to the scenario (latency vs throughput).
- Platform specifics (Windows LockFileEx, LXC boot_id) matter.

### 22.8. Exercises

1. Pick a configuration for the "analytics" scenario and justify the page size choice.
2. What changes for "low latency" on Windows compared to Linux?

### 22.9. Chapter 22 checklist

- [ ] For each of the six scenarios I can justify the choice of sync mode (from `DURABLE` to `SAFE_NOSYNC`/`NOMETASYNC`).
- [ ] I know that geometry is set with headroom and why `upper` should not be undersized.
- [ ] I can pick a batch size matching the latency/throughput balance of a scenario.
- [ ] I understand when `maxreaders` matters (read-heavy, multi-process reading).
- [ ] I remember the platform traps: LockFileEx on Windows, `boot_id` in containers, page-cache coherence (#269).
- [ ] I can explain why switching to a large page for full-scan is not a silver bullet.

---

## Chapter 23. Micro-optimizations

### 23.1. SIMD search for free pages

Searching for dense sequences in the GC uses vector kernels: SSE2/AVX2/AVX512/NEON
(speedup ×4/×8/×16; chosen at runtime). This is an internal optimization — you only influence it
via `rp_augment_limit`/`gc_time_limit`.

### 23.2. Branchless binary search (CMOV)

Binary search over the separators of branch pages uses unconditional moves (CMOV) — no
branching, predictability. The gain is noticeable in hot search paths.

### 23.3. Radix-sort and sorting networks

Used in internal procedures (sorting page lists, PNL merging).

### 23.4. C11 atomics for weak memory models

On ARM/AArch64/PPC/MIPS/RISC-V careful atomics are used (acquire/release, CAS, safe64).
Correctness is guaranteed by the build (atomicity checking).

### 23.5. Minimizing system calls

Hot paths avoid syscalls; I/O batching via bulk iovec; `mdbx_copy` uses bulk
writes.

### 23.6. Prefault write and mincore

`MDBX_opt_prefault_write_enable` — proactive page writing to eliminate page-faults and
disk reads on first access in `MDBX_WRITEMAP`. Residency is tracked via
`mincore`/`madvise`. The win is when the DB > RAM and pages are frequently evicted.

### 23.7. Auto-appending split and merging with a dirty neighbor

- Auto-appending split — efficient addition of keys at the end (append scenarios).
- Merge with an already-dirty neighbor — fewer CoW copies (related to `prefer_waf_insteadof_balance`).

**Fragment from [`examples/c++/30-append-prefault.c++`](examples/c++/30-append-prefault.c++)** —
batch loading of sorted keys: `prefault_write_enable` + `txn.append()` (`MDBX_APPEND`):

```cpp
static uint64_t load_append(const std::string &path, int count) {
  mdbx::env::remove(path);
  auto env = example::env_open(path);
  env.set_extra_option(mdbx::env::extra_runtime_option::prefault_write_enable, 1);

  const auto t0 = std::chrono::steady_clock::now();
  auto txn = env.start_write();
  auto table = txn.open_map(nullptr);
  for (int i = 0; i < count; ++i)
    txn.append(table, mdbx::slice(key_of(i)), mdbx::slice("value")); // MDBX_APPEND
  txn.commit();
  const auto t1 = std::chrono::steady_clock::now();
  return uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count());
}
```

Full code: [30-append-prefault.c++](examples/c++/30-append-prefault.c++).

### 23.8. When these optimizations matter, and when they do not

SIMD/CMOV give from a few percent to tens of percent on specific paths. For most applications what decides is
**batching and the sync mode**, not micro-optimizations. Do not optimize blindly — measure.

> **Examples for this chapter:** [`examples/c++/30-append-prefault.c++`](examples/c++/30-append-prefault.c++).

### 23.9. Chapter 23 summary

- Internal optimizations (SIMD, CMOV, radix, atomics) are already built in.
- Your levers: prefault, auto-append, merge policy.
- Micro-optimizations are secondary to batching and the sync mode.

### 23.10. Exercises

1. When is enabling prefault write justified? State the condition (DB size vs RAM).
2. Why is "optimizing without measuring" an anti-pattern?

### 23.11. Chapter 23 checklist

- [ ] I know which micro-optimizations are already built in (SIMD kernels, CMOV, radix-sort, C11 atomics), and that the SIMD search is influenced only via `rp_augment_limit`/`gc_time_limit`.
- [ ] I can state the condition under which prefault write is justified: DB > RAM, frequent page evictions, `MDBX_WRITEMAP`.
- [ ] I understand the benefit of the auto-appending split for append scenarios and of merging with an already-dirty neighbor (related to `prefer_waf_insteadof_balance`).
- [ ] I know that hot paths minimize syscalls (bulk iovec, bulk writes in `mdbx_copy`).
- [ ] I can explain why batching and the sync mode matter more than micro-optimizations, and why optimizing must follow measurements.

---

## Chapter 24. Cache lookup (get-cached)

### 24.1. What is get-cached

`mdbx_cache_get()` — a read service for "recurring" keys. For a key, the address of the value in
mmap + version tags are stored. On a repeated read, instead of a full descent of the B+tree a
**lazy (early) exit** is performed: we descend only while we encounter pages modified after the last
confirmation.

### 24.2. Mechanism

Cache entry: `{trunk_txnid, last_confirmed_txnid, offset, length}` (`offset == 0` = "key absent").

The first call is a full search + filling the entry. Subsequent ones:

1. the descent stops at the first page not modified after `last_confirmed_txnid`;
2. if nothing changed — `MDBX_CACHE_HIT` after a few cheap comparisons;
3. if there are changes — confirmation of freshness (`MDBX_CACHE_CONFIRMED`) or a full search
   (`MDBX_CACHE_REFRESHED`).

### 24.3. Statuses

| Status                            | Meaning                                                                                  |
| --------------------------------- | ----------------------------------------------------------------------------------------- |
| `HIT` / `CONFIRMED` / `REFRESHED` | The result is obtained                                                                     |
| `DIRTY`                           | The value is on a dirty (uncommitted) page; valid only in the current write transaction    |
| `BEHIND`                          | Reader is older than the version in the cache; bypass the cache                            |
| `UNABLE`                          | ABA situation; search bypassing the cache                                                  |
| `RACE`                            | Concurrent update of the entry; search bypassing                                          |
| `ERROR` (`MDBX_CACHE_ERROR`)      | An error unrelated to the key being absent                                                |

**Fragment from [`examples/c++/31-get-cached.c++`](examples/c++/31-get-cached.c++)** — displaying
all `get_cached` cache statuses: `HIT`/`CONFIRMED`/`REFRESHED`, `DIRTY`, `BEHIND`, `UNABLE`,
`RACE` and `ERROR`:

```cpp
const char *status_name(int status) {
  switch (status) {
  case MDBX_CACHE_ERROR:
    return "ERROR";
  case MDBX_CACHE_BEHIND:
    return "BEHIND";
  case MDBX_CACHE_UNABLE:
    return "UNABLE";
  case MDBX_CACHE_RACE:
    return "RACE";
  case MDBX_CACHE_DIRTY:
    return "DIRTY";
  case MDBX_CACHE_HIT:
    return "HIT";
  case MDBX_CACHE_CONFIRMED:
    return "CONFIRMED";
  case MDBX_CACHE_REFRESHED:
    return "REFRESHED";
  default:
    return "?";
  }
}
```

Full code: [31-get-cached.c++](examples/c++/31-get-cached.c++).

### 24.4. When it gives a speedup

The cache is effective for **recurring** keys with a small number of changes (configuration, counters,
markers, lookaside tables). For unique keys there is no benefit (the first search is full anyway).
The ratio of a full search to the HIT path easily reaches 10²–10³.

### 24.5. When it does not work

- Unique keys (no repeats).
- Active write transactions yield `DIRTY` (not cached).
- Multithreading requires `MDBX_NOSTICKYTHREADS` for the multithreaded variant;
  the single-threaded variant (`mdbx_cache_get_SingleThreaded`) is cheaper.

### 24.6. History

The request for a multithreaded lockfree get-cached — Gabriel RABHI ("Ghost Body Object"). The
multithreaded variant is implemented; the status on the current canon — to be clarified as the API evolves.

> **Examples for this chapter:** [`examples/c++/31-get-cached.c++`](examples/c++/31-get-cached.c++).

### 24.7. Chapter 24 summary

- get-cached is a lazy cache with an early exit and version tags.
- HIT/…/RACE are result statuses; `DIRTY` in write transactions.
- Effective for recurring "hot" keys; up to 1000× faster than a search.
- The multithreaded variant requires NOSTICKYTHREADS.

### 24.8. Exercises

1. Estimate: a loop of 1 000 000 reads of one key — how much will the cache save?
2. Why is `DIRTY` not cached?

### 24.9. Chapter 24 checklist

- [ ] I describe the cache entry `{trunk_txnid, last_confirmed_txnid, offset, length}` and the meaning of `offset == 0`.
- [ ] I can explain the lazy-exit mechanics: the descent stops at the first page not modified after `last_confirmed_txnid`.
- [ ] I distinguish the statuses `HIT`/`CONFIRMED`/`REFRESHED`/`DIRTY`/`BEHIND`/`UNABLE`/`RACE`/`ERROR` and know when the search bypasses the cache.
- [ ] I understand that `DIRTY` is valid only within the current write transaction and is not cached.
- [ ] I can estimate when the cache gives a speedup (recurring "hot" keys) and when it does not (unique keys).
- [ ] I know the `MDBX_NOSTICKYTHREADS` requirement for the multithreaded variant and the cheaper single-threaded variant (`mdbx_cache_get_SingleThreaded`).

---

## Chapter 25. Bulk operations

### 25.1. bunch_delete: deleting in bunches

`mdbx_cursor_bunch_delete()` with an action mode (`MDBX_bunch_action_t`) deletes the current value /
the whole multivalue / everything before or after the position / everything in a row. Instead of iterating
over elements the engine **cuts out whole pages and branches** — the cost is proportional to the number
of pages, not elements.

### 25.2. get_batch: batched reading

`mdbx_cursor_get_batch()` — symmetric bulk reading in batches (bulk iovec).

### 25.3. GET/PUT_MULTIPLE

Working with multivalues in batches: `MDBX_GET_MULTIPLE`/`MDBX_PUT_MULTIPLE` for DUPFIXED tables.

### 25.4. estimate_range/distance/move

Heuristic estimation without scanning: `mdbx_estimate_range()`, `mdbx_estimate_distance()`,
`mdbx_estimate_move()`. It is built from the common pages of the stacks of two positions. Accuracy: in the
worst case a deviation of up to **4× at each level** of the tree (except the first and last); in practice —
a few percent.

**Fragment from [`examples/c++/32-bulk-ops.c++`](examples/c++/32-bulk-ops.c++)** — bulk operations:
`get_batch` (reading in batches), `estimate` (estimation of the number of records) and `bunch_delete`
(deleting in bunches with page cutting-out):

```cpp
    // Batched reading: up to 100 pairs per call.
    size_t pairs = 0, chunks = 0;
    cur.to_first();
    bool is_last = false;
    while (!is_last) {
      auto batch = cur.get_batch(100, mdbx::cursor::move_operation::next, &is_last);
      pairs += batch.size();
      ++chunks;
    }
    std::cout << "batch reads: " << pairs << " pairs in " << chunks << " chunks\n";

    // Estimation of the number of records (approximately).
    cur.to_first();
    auto est = cur.estimate(mdbx::cursor::move_operation::last);
    std::cout << "estimate: ~" << est.approximate_quantity << " keys\n";

    // Deleting in bunches: everything from the current position to the end.
    cur.to_key_exact(mdbx::slice("k500"));
    const size_t removed = cur.erase_bunch(mdbx::cursor::bunch_delete::delete_after_including);
    std::cout << "bunch delete removed " << removed << " items\n";
```

Full code: [32-bulk-ops.c++](examples/c++/32-bulk-ops.c++).

### 25.5. txn_clone: parallel processing on a single snapshot

`mdbx_txn_clone()` multiplies a read-only transaction: several handlers on a single snapshot without
re-scanning the RLT.

### 25.6. Transaction batching

100–10 000 operations per commit is a typical range for write-heavy. More operations → lower WAF and
higher throughput.

### 25.7. When a "maximum-size transaction" is an anti-pattern

- A transaction with millions of operations can overflow the dirty-page limits (`TXN_FULL`), cause a
  spill and a latency spike.
- A long write transaction blocks other writers and widens the risk window.
- Split into reasonable batches (e.g., 10 000 operations each) — a balance between WAF and latency.

> **Examples for this chapter:** [`examples/c++/32-bulk-ops.c++`](examples/c++/32-bulk-ops.c++).

### 25.8. Chapter 25 summary

- bunch_delete/delete_range — bulk deletion by cutting out pages.
- get_batch, GET/PUT_MULTIPLE — batched read/write.
- estimate_* — cheap range estimation (up to 4×/level in the worst case).
- txn_clone — parallel processing on a single snapshot.
- A reasonable batch is better than a "maximum-size transaction".

### 25.9. Exercises

1. Compare row-by-row range deletion and `bunch_delete` on 100 000 keys.
2. When is `estimate_range` useful before running a query?

### 25.10. Chapter 25 checklist

- [ ] I can explain why `bunch_delete` is cheaper than iterating over elements: whole pages and branches are cut out, the cost is proportional to the number of pages.
- [ ] I know the symmetric bulk reads/writes: `get_batch`, `MDBX_GET_MULTIPLE`/`MDBX_PUT_MULTIPLE` (DUPFIXED tables).
- [ ] I understand how `estimate_*` works: estimation from the common pages of the stacks of two positions, up to 4× per tree level in the worst case.
- [ ] I can use `txn_clone` for parallel processing on a single snapshot.
- [ ] I can name the consequences of a "maximum-size transaction": `TXN_FULL`, spill, a latency spike, blocking other writers.
- [ ] I choose a reasonable batch size (e.g., ~10 000 operations) as a balance between WAF and latency.

---

## Chapter 26. Benchmarks and measurements

### 26.1. ioarena

`ioarena` — the project's standard benchmark pipeline. It runs on ready-made configurations;
it shows write/read TPS for different engines and modes.

### 26.2. What to measure

- **Write TPS** — commits/sec in the sync modes.
- **Read get/s** — operations/sec.
- **WAF** — via page counters (`mdbx_stat -p`).
- **Commit latency** — `mdbx_txn_commit_ex()` → `MDBX_commit_latency` by stages
  (`preparation/gc_wallclock/audit/write/sync/ending/whole`).

### 26.3. MDBX_ENABLE_PROFGC

GC profiling: collected in the LCK-file, returned in `commit_latency.gc_prof`. Key fields:
`work_rtime_monotonic`/`work_xtime_cpu` (GC time), `work_rsteps`/`work_xpages` (fragmentation),
`work_majflt` (page-faults), `max_reader_lag`/`max_retained_pages` (long readers), `kicks`
(HSR — Handle-Slow-Readers, eviction of stuck readers; see Volume V, ch. 29).

**Fragment from [`examples/c++/33-profiler.c++`](examples/c++/33-profiler.c++)** — reading PROFGC fields
from `commit_latency.gc_prof` (`wloops`, `coalescences`, `flushes`, `kicks`, `max_reader_lag`,
`max_retained_pages`); the fields are filled in builds with `MDBX_ENABLE_PROFGC`:

```cpp
    // Load that generates GC work: inserts and deletes.
    for (int round = 0; round < 5; ++round) {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 500; ++i) {
        const auto key = "k" + std::to_string(round * 1000 + i);
        txn.insert(table, mdbx::slice(key), mdbx::slice("payload"));
      }
      txn.commit();
      auto del = env.start_write();
      auto dtable = del.open_map(nullptr);
      for (int i = 0; i < 500; ++i)
        del.erase(dtable, mdbx::slice("k" + std::to_string(round * 1000 + i)));
      del.commit();
    }

    auto txn = env.start_write();
    auto table = txn.open_map(nullptr);
    txn.insert(table, mdbx::slice("last"), mdbx::slice("v"));
    const auto lat = txn.commit_get_latency();

    const auto &gc = lat.gc_prof;
    std::cout << "gc_prof: wloops=" << gc.wloops << " coalescences=" << gc.coalescences << " flushes=" << gc.flushes
              << " kicks=" << gc.kicks << " max_reader_lag=" << gc.max_reader_lag
              << " max_retained_pages=" << gc.max_retained_pages << "\n";
```

Full code: [33-profiler.c++](examples/c++/33-profiler.c++).

### 26.4. Typical benchmarking mistakes

- **Asserts**: a debug build (`MDBX_CHECKING>0`) slows things down manyfold.
- **Page-cache incoherence** (#269) on Linux with several processes.
- **Methodology**: "small transactions + DURABLE on Windows" is LockFileEx, not the engine.
- **Comparison without context**: mode, page, tree height, hardware.

### 26.5. Why libmdbx may be "6–7× slower than LMDB"

Usually it means: asserts are enabled, or an incorrect benchmark (small transactions on Windows with
LockFileEx), or a single-op mode is measured, where libmdbx deliberately pays for stricter
guarantees (two-phase meta, checks). With a correct methodology and the same modes the gap
disappears or turns in libmdbx's favor.

### 26.6. Guidelines

- Write: ~200 TPS of single durable transactions on an ordinary SSD; with batching and SAFE_NOSYNC — tens–
  hundreds of thousands and millions of operations/s.
- Read: 1–3 million gets/s (short keys, warm cache); linear scaling across cores.
- Inserts: 20K–10M/s depending on the mode and hardware.

> **Warning:** always provide the context of a number — sync mode, page size, transaction size,
> hardware.

### 26.7. Read scaling

Reading is wait-free and scales linearly across cores — until memory saturates (bandwidth/TLB). The bottleneck
is usually not the engine but the cache/memory.

> **Examples for this chapter:** [`examples/c++/33-profiler.c++`](examples/c++/33-profiler.c++).

### 26.8. Chapter 26 summary

- ioarena — the standard benchmark; TPS/get/s/WAF/latency.
- commit_latency + PROFGC — bottleneck diagnostics.
- Asserts and methodology are the main traps.
- "6–7× slower than LMDB" is almost always a measurement error.
- Guidelines: 200 TPS durable, 1–3M gets/s, 20K–10M inserts/s.

### 26.9. Exercises

1. Measure `commit_latency` on your configuration and identify the dominant stage.
2. Explain why the "one transaction per operation" comparison is incorrect for evaluating the engine.

### 26.10. Chapter 26 checklist

- [ ] I know what to measure: write TPS, read get/s, WAF (via page counters), commit latency by stage.
- [ ] I can read `MDBX_commit_latency` (`preparation/gc_wallclock/audit/write/sync/ending/whole`) and identify the dominant stage.
- [ ] I understand what the `gc_prof` fields (`max_reader_lag`, `max_retained_pages`, `kicks`) say about long readers and HSR kicks.
- [ ] I avoid the typical mistakes: measuring a build with asserts, "small transactions + DURABLE on Windows", comparison without context.
- [ ] I can explain why "6–7× slower than LMDB" is almost always a measurement error or the price of stricter guarantees.
- [ ] I know the guidelines (~200 TPS durable, 1–3M gets/s) and always provide the context of a number.
- [ ] I can explain why reads scale linearly across cores and where the real bottleneck is (memory/cache, not the engine).

---

## Volume summary

You know how to measure and tune libmdbx: you understand WAF, choose a configuration for the scenario,
use micro-optimizations, get-cached and bulk operations, and read the GC profile and latency.

**What's next:** Volume V — expert topics: rules and tips from the knowledge base, platform nuances, HSR,
diagnostics and debugging, migration from LMDB, design patterns, roadmap.