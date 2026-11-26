ChangeLog
=========

The source code is available on [SourceCraft](https://sourcecraft.dev/dqdkfa/libmdbx) and mirror on [GitHub](https://github.com/Mithril-mine/libmdbx).
Please use the `stable` branch or the latest release for production environments through staging, and the `master` branch for development of derivative projects.
Всё будет хорошо!

## v0.15.1 at a deep development stage

A hardening and refinement release: data-safety and corruption-resistance fixes throughout the engine, faster and safer commits on Windows, a cleaner C++ API, and a new documentation site with a textbook.

### Added:

 - Added a bilingual [documentation site](https://libmdbx.dqdkfa.ru) with a ~50-example textbook and a refined API reference.
 - Added the man page for `mdbx_defrag`, updated the other man pages and every `-h` option.
 - Added `mdbx_preopen_snapinfo_ex()` with an explicit mode of processing weak checkpoints ([#182](https://github.com/Mithril-mine/libmdbx-devel/issues/182)).
 - Added the `MDBX_AVOID_FTOK` build option to avoid System V IPC key collisions ([#80](https://github.com/Mithril-mine/libmdbx-devel/issues/80)).
 - Added the `MDBX_opt_shrink_retry_period` option to rate-limit futile datafile-shrink attempts ([#170](https://github.com/Mithril-mine/libmdbx-devel/issues/170)).
 - Added the non-throwing `slice::try_as_pod<T>()`, empty on a size mismatch ([#144](https://github.com/Mithril-mine/libmdbx-devel/issues/144)).
 - Added the `mdbx.lto` and `mdbx.sanitize` options to the Conan recipe.

### Changed:

 - **Breaking:** Changed the C++ `env::mode`/`env::durability` enums to scoped; old names stay deprecated aliases.
 - Changed the locking auto-selection to fallback to System V semaphores instead of the POSIX-2001
   mutexes, which have no recovery from a dead lock owner
   ([#565](https://github.com/Mithril-mine/libmdbx-devel/issues/565),
   [#402](https://github.com/Mithril-mine/libmdbx-devel/issues/402)).
 - Changed `mdbx_defrag` to name every stopping reason and suggest `mdbx_copy -c` for full compaction.
 - Changed Windows commit handling: doomed datafile-shrink attempts are skipped and rate-limited ([#170](https://github.com/Mithril-mine/libmdbx-devel/issues/170)).
 - Changed the oldest-reader scan to bound retries under heavy reader churn ([#325](https://github.com/Mithril-mine/libmdbx-devel/issues/325)).
 - Changed `mdbx_env_defrag()` to adapt the movement batch to the time limit and to report full completion as success ([#55](https://github.com/Mithril-mine/libmdbx-devel/issues/55)).
 - Changed the `MDBX_GET_MULTIPLE` docs: a batch is a whole page of the current key's duplicates ([12/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
 - Changed the `mdbx_cursor_get_batch()` docs: which end-of-data code to expect in each case ([13/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
 - Changed the `mdbx_cursor_on_first()`/`on_last()` docs: whether moving in the direction makes sense ([14/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
 - Changed the C++ docs for cursor unbinding, `chk()` tables lifetime and `append_producer()` ([#91](https://github.com/Mithril-mine/libmdbx-devel/issues/91), [#143](https://github.com/Mithril-mine/libmdbx-devel/issues/143), [#145](https://github.com/Mithril-mine/libmdbx-devel/issues/145), [#146](https://github.com/Mithril-mine/libmdbx-devel/issues/146)).
 - Changed the per-fork internal cleanup logging from NOTICE to VERBOSE ([9/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
 - Changed the routine open-time and database-creation logging from NOTICE to VERBOSE ([#535](https://github.com/Mithril-mine/libmdbx-devel/issues/535)).
 - Changed the `MDBX_ENABLE_BUNCHES_REMOVAL=0` build to compile again; a CI compile check added ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).

### Removed:

 - **Breaking:** Removed the `MDBX_CPU_WRITEBACK_INCOHERENT` build option; weakly-ordered CPUs are always assumed ([#361](https://github.com/Mithril-mine/libmdbx-devel/issues/361)).
 - **Breaking:** Removed the `MDBX_ENABLE_REFUND` build option; online auto-compactification is always enabled ([#193](https://github.com/Mithril-mine/libmdbx-devel/issues/193)).
 - **Breaking:** Removed the `MDBX_PNL_ASCENDING` build option; descending page order is always used ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).
 - **Breaking:** Removed the C++ `env::get_preopen_snapinfo()` overloads without an explicit mode argument ([#182](https://github.com/Mithril-mine/libmdbx-devel/issues/182)).

### Fixed:

 - Data safety and durability:
    - Fixed a bunch-delete on an unpositioned cursor aborting assert-enabled builds ([#503](https://github.com/Mithril-mine/libmdbx-devel/issues/503)).
    - Fixed a GC-record corruption when the retired-pages list grows during a GC write ([#494](https://github.com/Mithril-mine/libmdbx-devel/issues/494)).
    - Fixed unbounded dense-solving work stalling commits on boundary GC shapes, with fewer GC slots consumed by the solver ([#497](https://github.com/Mithril-mine/libmdbx-devel/issues/497)).
    - Fixed a failed begin of a nested transaction corrupting the parent's geometry with a spurious resize-undo ([#448](https://github.com/Mithril-mine/libmdbx-devel/issues/448)).
    - Fixed `mdbx_env_copy()` destroying a live database when the destination is the database's own file ([#450](https://github.com/Mithril-mine/libmdbx-devel/issues/450)).
    - Fixed `mdbx_dump -a` omitting the main table's records when named tables exist ([#385](https://github.com/Mithril-mine/libmdbx-devel/issues/385)).
    - Fixed a failed flush in the pre-sync and meta-sync paths leaving the environment usable ([#322](https://github.com/Mithril-mine/libmdbx-devel/issues/322)).
    - Fixed a failed range delete leaving a committable half-cut tree ([#311](https://github.com/Mithril-mine/libmdbx-devel/issues/311)).
    - Fixed `mdbx_drop()` for MAIN_DBI with named tables leaking their pages; now refused ([#311](https://github.com/Mithril-mine/libmdbx-devel/issues/311)).
    - Fixed a table rename staying applied after its transaction aborts, corrupting data ([#307](https://github.com/Mithril-mine/libmdbx-devel/issues/307)).
    - Fixed `mdbx_env_defrag()` inside a write transaction losing committed data ([#306](https://github.com/Mithril-mine/libmdbx-devel/issues/306)).
    - Fixed failed disk writes silently passing as success, and a Windows crash on a large durable commit ([#47](https://github.com/Mithril-mine/libmdbx-devel/issues/47)).
    - Fixed silently ignored auxiliary page-write errors that masked a failing disk ([#203](https://github.com/Mithril-mine/libmdbx-devel/issues/203)).
    - Fixed a forged snapshot-age hint letting the writer reuse pages still read by live transactions ([#81](https://github.com/Mithril-mine/libmdbx-devel/issues/81)).
    - Fixed `mdbx_drop()` on the internal free-list handle corrupting the page accounting; now rejected ([#69](https://github.com/Mithril-mine/libmdbx-devel/issues/69)).
    - Fixed a failed second in-process open releasing the first handle's file locks ([#59](https://github.com/Mithril-mine/libmdbx-devel/issues/59)).
    - Fixed a compacting copy to a pipe reporting success but writing a wrong root page, losing records ([#309](https://github.com/Mithril-mine/libmdbx-devel/issues/309)).
    - Fixed a non-compacting copy on Linux that could be corrupt on some network or FUSE filesystems ([#327](https://github.com/Mithril-mine/libmdbx-devel/issues/327)).
    - Fixed a copy deleting a pre-existing destination on a lock failure or keeping its old tail ([#58](https://github.com/Mithril-mine/libmdbx-devel/issues/58), [#112](https://github.com/Mithril-mine/libmdbx-devel/issues/112)).
    - Fixed `mdbx_env_copy()` deleting an existing destination on an unrelated open failure ([#310](https://github.com/Mithril-mine/libmdbx-devel/issues/310)).
    - Fixed a copy of a private database being readable by its group on POSIX ([#331](https://github.com/Mithril-mine/libmdbx-devel/issues/331)).
    - Fixed `mdbx_put(MDBX_CURRENT)` leaving a dangling cursor after a size error, crashing later operations ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).
    - Fixed a 32-bit wrap distorting the geometry read from a database made on another-bitness host ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).

 - Resistance to corrupted or crafted data:
    - Fixed a crafted database aborting the assert-enabled builds on a DUPSORT cursor positioning; now rejected with `MDBX_CORRUPTED` ([#454](https://github.com/Mithril-mine/libmdbx-devel/issues/454)).
    - Fixed a crafted garbage-collection record handing one page to two b-tree nodes; now rejected ([#83](https://github.com/Mithril-mine/libmdbx-devel/issues/83)).
    - Fixed crafted databases crashing or corrupting on delete, drop, estimate or check; now rejected ([#57](https://github.com/Mithril-mine/libmdbx-devel/issues/57), [#63](https://github.com/Mithril-mine/libmdbx-devel/issues/63), [#74](https://github.com/Mithril-mine/libmdbx-devel/issues/74), [#75](https://github.com/Mithril-mine/libmdbx-devel/issues/75)).
    - Fixed the same-size in-place replacement of a zero-length duplicate value passing a null pointer to `memcpy()` (undefined behavior, trapped by UBSan) ([#444](https://github.com/Mithril-mine/libmdbx-devel/issues/444)).
    - Fixed a crafted tree making the defragmentation walk exponential, a denial of service ([#82](https://github.com/Mithril-mine/libmdbx-devel/issues/82)).
    - Fixed a crafted set of duplicate values crashing or corrupting memory; now rejected ([#176](https://github.com/Mithril-mine/libmdbx-devel/issues/176)).
    - Fixed a corrupt reader-table counter driving out-of-bounds scans; the count is clamped now ([#79](https://github.com/Mithril-mine/libmdbx-devel/issues/79), [#113](https://github.com/Mithril-mine/libmdbx-devel/issues/113)).
    - Fixed `mdbx_gc_info()` looping forever while holding the writer lock ([#88](https://github.com/Mithril-mine/libmdbx-devel/issues/88)).
    - Fixed an out-of-bounds read in `mdbx_cursor_scroll()`/`distance()` on a huge `deepness` ([#85](https://github.com/Mithril-mine/libmdbx-devel/issues/85)).
    - Fixed a damaged first meta-page failing the open with two intact ones at another page size ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).
    - Fixed a crafted maximal-height tree writing one slot past the cursor stack in a root split ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).
    - Fixed crafted huge item counts overflowing the estimate arithmetic; the result is clamped now ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).

 - Crashes, hangs and undefined behavior:
    - Fixed a writer crashing with `SIGBUS` while another thread of its process starts a read transaction ([#592](https://github.com/Mithril-mine/libmdbx-devel/issues/592)).
    - Fixed `mdbx_env_open()` hangs, wrong mode inheritance and a missed duplicate-open refusal ([#317](https://github.com/Mithril-mine/libmdbx-devel/issues/317)).
    - Fixed the parent's cursors left on freed pages after a failed nested commit or checkpoint ([#326](https://github.com/Mithril-mine/libmdbx-devel/issues/326)).
    - Fixed a double free and crash on a write error during a large commit; now reported ([#321](https://github.com/Mithril-mine/libmdbx-devel/issues/321)).
    - Fixed `mdbx_enumerate_tables()` crashing or reading freed memory when racing with table operations ([#341](https://github.com/Mithril-mine/libmdbx-devel/issues/341)).
    - Fixed repeating a delete after a key's last duplicate corrupting data or crashing; now a no-op ([#301](https://github.com/Mithril-mine/libmdbx-devel/issues/301), [#305](https://github.com/Mithril-mine/libmdbx-devel/issues/305)).
    - Fixed a delete after a failed seek among duplicates corrupting memory; now safely refused ([#305](https://github.com/Mithril-mine/libmdbx-devel/issues/305)).
    - Fixed a write through one cursor leaving others stale, so a later delete lost committed values ([#304](https://github.com/Mithril-mine/libmdbx-devel/issues/304)).
    - Fixed a racing writer tripping a checking-build assertion for every writer sharing the lock file ([#347](https://github.com/Mithril-mine/libmdbx-devel/issues/347)).
    - Fixed lost load/store ordering on ARM-like CPUs in builds without C11 atomics ([#340](https://github.com/Mithril-mine/libmdbx-devel/issues/340), [#361](https://github.com/Mithril-mine/libmdbx-devel/issues/361)).
    - Fixed `mdbx_replace()` with `MDBX_RESERVE` crashing; now rejected on tables with duplicates ([#166](https://github.com/Mithril-mine/libmdbx-devel/issues/166)).
    - Fixed `mdbx_replace_ex()` with overlapping buffers hanging or crashing the next operation ([#56](https://github.com/Mithril-mine/libmdbx-devel/issues/56)).
    - Fixed a compacting copy hanging forever when its writer thread hit a write error; now reported ([#171](https://github.com/Mithril-mine/libmdbx-devel/issues/171)).
    - Fixed `mdbx_txn_rollback()` of a nested transaction corrupting the parent ([#103](https://github.com/Mithril-mine/libmdbx-devel/issues/103), [#124](https://github.com/Mithril-mine/libmdbx-devel/issues/124)).
    - Fixed cloning a read transaction crashing on cleanup or reading recycled pages; fails cleanly now ([#128](https://github.com/Mithril-mine/libmdbx-devel/issues/128), [#148](https://github.com/Mithril-mine/libmdbx-devel/issues/148)).
    - Fixed parking a failed transaction crashing on abort and cloning a parked one being unusable ([#104](https://github.com/Mithril-mine/libmdbx-devel/issues/104)).
    - Fixed a nested-transaction begin failing out-of-memory leaving the parent inconsistent ([#67](https://github.com/Mithril-mine/libmdbx-devel/issues/67)).
    - Fixed `mdbx_cursor_count_ex()` writing past the caller's buffer with the legacy structure size ([#68](https://github.com/Mithril-mine/libmdbx-devel/issues/68)).
    - Fixed `mdbx_cursor_put(MDBX_MULTIPLE)` dividing by zero on a zero element size; now rejected ([#73](https://github.com/Mithril-mine/libmdbx-devel/issues/73)).
    - Fixed undefined behavior in the JSON-integer key encoding for huge magnitudes.
    - Fixed an arithmetic overflow miscomputing the database size limit on 32-bit builds with large RAM.
    - Fixed the `MDBX_TXN_TRY` flag leaking into the transaction state flags.
    - Fixed a forked child treating a failed unlock of an inherited lock as fatal at some log levels ([#111](https://github.com/Mithril-mine/libmdbx-devel/issues/111)).
    - Fixed a child forked during another thread's database call hanging at exit; `fork()` now waits ([#324](https://github.com/Mithril-mine/libmdbx-devel/issues/324), [#329](https://github.com/Mithril-mine/libmdbx-devel/issues/329)).
    - Fixed `mdbx_env_chk()` heap corruption or a crash when a callback stopped the check early ([#314](https://github.com/Mithril-mine/libmdbx-devel/issues/314)).
    - Fixed a crash or memory corruption after a resize failed with `MDBX_UNABLE_EXTEND_MAPSIZE` ([#334](https://github.com/Mithril-mine/libmdbx-devel/issues/334)).
    - Fixed `mdbx_txn_park()`/`unpark()` crashing without a reader slot; now `MDBX_EPERM` ([#316](https://github.com/Mithril-mine/libmdbx-devel/issues/316)).
    - Fixed `mdbx_cursor_renew()` crashing on a wrong cursor or transaction handle ([1/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
    - Fixed crashes in open transactions or `mdbx_env_warmup()` after the database size limit was raised ([#315](https://github.com/Mithril-mine/libmdbx-devel/issues/315)).
    - Fixed a use of uninitialized cursor memory reported by MSan/Valgrind for non-dupsort tables.
    - Fixed negating `INT64_MIN` (formal undefined behavior) in the scroll and readahead heuristics ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).

 - Wrong results and API behavior:
    - Fixed `mdbx_env_defrag()` reporting success and no obstacles when stuck at an unmovable page ([#550](https://github.com/Mithril-mine/libmdbx-devel/issues/550)).
    - Fixed `mdbx_env_defrag()` ignoring the time limit when the shrink goal is unreachable ([#550](https://github.com/Mithril-mine/libmdbx-devel/issues/550)).
    - Fixed `mdbx_env_defrag()` stopping with an error while data pages could still move to free pages ([#600](https://github.com/Mithril-mine/libmdbx-devel/issues/600)).
    - Fixed `mdbx_env_defrag()` cycling without end when free pages remain below the data ([#600](https://github.com/Mithril-mine/libmdbx-devel/issues/600)).
    - Fixed `mdbx_env_defrag()` returning with free pages at the end of the datafile not cut off ([#600](https://github.com/Mithril-mine/libmdbx-devel/issues/600)).
    - Fixed `mdbx_env_defrag()` reporting a limit of the structure of the trees as an error ([#600](https://github.com/Mithril-mine/libmdbx-devel/issues/600)).
    - Fixed large transactions growing the datafile with `MDBX_ENABLE_BIGFOOT` disabled ([#605](https://github.com/Mithril-mine/libmdbx-devel/issues/605)).
    - Fixed `mdbx_env_defrag()` never returning when the pages it moves come back to their places ([#614](https://github.com/Mithril-mine/libmdbx-devel/issues/614)).
    - Fixed `mdbx_env_defrag()` moving nothing and reporting an error when the GC keeps a long list of free pages ([#619](https://github.com/Mithril-mine/libmdbx-devel/issues/619)).
    - Fixed `mdbx_env_defrag()` flushing the data pages of every cycle in the `MDBX_NOSYNC` and `MDBX_UTTERLY_NOSYNC` modes, where the GC makes do with wiping the steady checkpoint; the datafile is truncated at the end of the call now, after a steady checkpoint, and a call that found a steady checkpoint leaves one behind.
    - Fixed a rare SIGPIPE process-death on copying to a pipe closed by the reader; the `EPIPE` error is returned now ([#529](https://github.com/Mithril-mine/libmdbx-devel/pull/529)).
    - Fixed the default geometry upper reaching the mappable limit exactly, breaking tool pipelines with `MDBX_TOO_LARGE`.
    - Fixed `mdbx_chk -s` silently succeeding when the named table is absent; now an error ([#386](https://github.com/Mithril-mine/libmdbx-devel/issues/386)).
    - Fixed a whole-table range delete resetting the table's sequence ([#311](https://github.com/Mithril-mine/libmdbx-devel/issues/311)).
    - Fixed `mdbx_env_info_ex()` leaving the output unset on a not-open environment; zeroed now ([2/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
    - Fixed `MDBX_NEXT/PREV_MULTIPLE` without `MDBX_DUPFIXED` giving empty data and moving past the end ([3/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
    - Fixed `mdbx_estimate_range()` ignoring `begin_data`/`end_data`; `estimate_move()` erred on success ([4/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
    - Fixed `mdbx_cursor_scan()` returning an error instead of `MDBX_RESULT_FALSE` on single-value keys ([5/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
    - Fixed `MDBX_opt_txn_dp_initial` set before open being truncated to 32 bits ([6/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
    - Fixed a spurious `MDBX_READERS_FULL` right after dead reader slots were freed; now retried ([7/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
    - Fixed internal `N_TREE`/`N_DUP` flags in `mdbx_cursor_put()`/`del()` corrupting; now rejected ([8/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
    - Fixed a huge `mdbx_put(MDBX_MULTIPLE)` batch failing with `MDBX_TXN_FULL` instead of spilling ([10/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
    - Fixed `mdbx_replace()` accepting `MDBX_ALLDUPS` and aliased data buffers; now `MDBX_EINVAL` ([11/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
    - Fixed `mdbx_dbi_rename()` to the same name returning `MDBX_KEYEXIST` in debug builds; a no-op now ([15/15 of #319](https://github.com/Mithril-mine/libmdbx-devel/issues/319)).
    - Fixed `MDBX_TO_PAIR_*` returning the key's first multi-value instead of the positioned one ([#313](https://github.com/Mithril-mine/libmdbx-devel/issues/313)).
    - Fixed `mdbx_put(MDBX_CURRENT)` deleting all multi-values before checking the new value's size ([#313](https://github.com/Mithril-mine/libmdbx-devel/issues/313)).
    - Fixed a 32-bit Linux process getting a far too small total RAM on hosts with 4 GiB or more ([#339](https://github.com/Mithril-mine/libmdbx-devel/issues/339)).
    - Fixed `mdbx_env_delete()` leaving an orphan lock file and reporting success while still in use ([#345](https://github.com/Mithril-mine/libmdbx-devel/issues/345)).
    - Fixed `mdbx_env_info_ex()` reporting the first writer's open-time mode instead of the current flags ([#338](https://github.com/Mithril-mine/libmdbx-devel/issues/338)).
    - Fixed `mdbx_env_set_flags()` creating a forbidden `MDBX_WRITEMAP` and lazy-mode mix; now refused ([#338](https://github.com/Mithril-mine/libmdbx-devel/issues/338)).
    - Fixed `mdbx_cursor_get_batch(MDBX_FIRST)` returning only the tail of an already positioned cursor ([#151](https://github.com/Mithril-mine/libmdbx-devel/issues/151)).
    - Fixed `mdbx_cursor_scroll()`/`distribute()` returning wrong results near the tree height ([#84](https://github.com/Mithril-mine/libmdbx-devel/issues/84), [#123](https://github.com/Mithril-mine/libmdbx-devel/issues/123)).
    - Fixed `mdbx_env_stat_ex()` under-counting the totals once a named table was used ([#108](https://github.com/Mithril-mine/libmdbx-devel/issues/108)).
    - Fixed `mdbx_estimate_distance()` returning a negated result for cursors on one key's duplicates ([#86](https://github.com/Mithril-mine/libmdbx-devel/issues/86)).
    - Fixed `mdbx_jsonInteger_from_key()` decoding the stored integer `0` as `INT64_MAX` ([#87](https://github.com/Mithril-mine/libmdbx-devel/issues/87)).
    - Fixed `mdbx_dbi_rename()` on a stale handle storing a stale record, corrupting the database ([#106](https://github.com/Mithril-mine/libmdbx-devel/issues/106)).
    - Fixed table open/rename mishandling pseudo-names and cross-process drops, up to a crash ([#107](https://github.com/Mithril-mine/libmdbx-devel/issues/107), [#308](https://github.com/Mithril-mine/libmdbx-devel/issues/308)).
    - Fixed `mdbx_env_chk()` leaving `result.tables` dangling; C++ `env::chk()` threw on found problems ([#93](https://github.com/Mithril-mine/libmdbx-devel/issues/93)).
    - Fixed `mdbx_env_warmup()` crashing on a created-but-not-opened environment; now `MDBX_EPERM` ([#114](https://github.com/Mithril-mine/libmdbx-devel/issues/114)).
    - Fixed `mdbx_txn_info()` reporting doubled dirty space and losing spilled pages with `MDBX_WRITEMAP` ([#117](https://github.com/Mithril-mine/libmdbx-devel/issues/117)).
    - Fixed `MDBX_CP_RENEW_TXN` having no effect since v0.14.2, so a throttled copy failed spuriously ([#337](https://github.com/Mithril-mine/libmdbx-devel/issues/337)).
    - Fixed a doubly-nested transaction failing with `MDBX_PROBLEM` with a custom spill option set ([#342](https://github.com/Mithril-mine/libmdbx-devel/issues/342)).
    - Fixed `mdbx_drop()` on an already empty table not resetting its sequence, as for a non-empty one ([#312](https://github.com/Mithril-mine/libmdbx-devel/issues/312)).
    - Fixed `MDBX_opt_sync_bytes` silently truncating an over-large threshold; now `MDBX_EINVAL` ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).
    - Fixed `mdbx_txn_checkpoint()` leaving the latency report zeroed on the nothing-to-do path ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).
    - Fixed the legacy `mdbx_txn_release_all_cursors()` symbol missing from the shared library ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).
    - Fixed the C++ `slice::assign()` leaving a partially updated slice on a rejected length; the raw
      pointer/length variant is no longer public ([#551](https://github.com/Mithril-mine/libmdbx-devel/issues/551)).
    - Fixed the C++ `map_handle::info` mode accessors producing undeclared `key_mode`/`value_mode` values ([#551](https://github.com/Mithril-mine/libmdbx-devel/issues/551)).
    - Fixed 32-bit overflow of the C++ hex and Base58 encoders' length estimation ([#551](https://github.com/Mithril-mine/libmdbx-devel/issues/551)).
    - Fixed the C++ `buffer_pair_spec` allocator constructor being unconditionally `noexcept` ([#551](https://github.com/Mithril-mine/libmdbx-devel/issues/551)).

 - Platform-specific:
    - Fixed the batched writes on Windows after a mid-batch write failure: the cancelled requests are now drained before the ring's resources are reused ([#449](https://github.com/Mithril-mine/libmdbx-devel/issues/449)).
    - Fixed a long-lived read transaction deadlocking writers growing the file on Windows; now in place ([#154](https://github.com/Mithril-mine/libmdbx-devel/issues/154)).
    - Fixed exclusive-mode commits using the slow flush path; they are as fast as usual now ([#179](https://github.com/Mithril-mine/libmdbx-devel/issues/179)).
    - Fixed `MDBX_NOMETASYNC`/`SAFE_NOSYNC` degrading to full durable syncs on every commit on Windows ([#154](https://github.com/Mithril-mine/libmdbx-devel/issues/154)).
    - Fixed the Windows shrink decision using bookkeeping instead of the real file size, skipping shrinks ([#154](https://github.com/Mithril-mine/libmdbx-devel/issues/154)).
    - Fixed a file-lock wait leaving the lock held or reporting false success, blocking later opens ([#110](https://github.com/Mithril-mine/libmdbx-devel/issues/110)).
    - Fixed empty `MDBX_DBG_*` switches being ignored on Windows ([#109](https://github.com/Mithril-mine/libmdbx-devel/issues/109)).
    - Fixed a spurious internal assertion failure during page allocation on Windows ([#52](https://github.com/Mithril-mine/libmdbx-devel/issues/52)).
    - Fixed the Windows I/O ring merging only two contiguous write chunks instead of all of them ([#111](https://github.com/Mithril-mine/libmdbx-devel/issues/111)).
    - Fixed threads hanging forever starting a read transaction after a failed lock-file request ([#332](https://github.com/Mithril-mine/libmdbx-devel/issues/332)).
    - Fixed a rare Windows resize failure closing an unrelated application handle ([#333](https://github.com/Mithril-mine/libmdbx-devel/issues/333)).
    - Fixed `mdbx_env_delete()` with `ENSURE_UNUSED`/`WAIT_FOR_UNUSED` never deleting anything ([#336](https://github.com/Mithril-mine/libmdbx-devel/issues/336)).
    - Fixed a signal during a POSIX/SysV lock wait making the environment fatal with `MDBX_PANIC` ([#323](https://github.com/Mithril-mine/libmdbx-devel/issues/323)).
    - Fixed `mdbx_env_open()` failing with `EINVAL` on Android f2fs ([#197](https://github.com/Mithril-mine/libmdbx-devel/issues/197)).
    - Fixed OpenBSD defaulting to POSIX-semaphore locking, where `sem_init()` always fails ([#115](https://github.com/Mithril-mine/libmdbx-devel/issues/115)).
    - Fixed a System V IPC key collision destroying another database's semaphores; now refused ([#80](https://github.com/Mithril-mine/libmdbx-devel/issues/80), [#159](https://github.com/Mithril-mine/libmdbx-devel/issues/159)).
    - Fixed `mdbx_preopen_snapinfo()` falsely reporting corruption for a weak checkpoint after a crash ([#182](https://github.com/Mithril-mine/libmdbx-devel/issues/182)).
    - Fixed a failed file preallocation being ignored until write time with recent Linux headers ([#320](https://github.com/Mithril-mine/libmdbx-devel/issues/320)).
    - Fixed a copy failing with `EBADF` without `fallocate()` on older Linux headers ([#330](https://github.com/Mithril-mine/libmdbx-devel/issues/330)).
    - Fixed `mdbx_env_copy()` failing with `EEXIST` on a filesystem without direct I/O ([#331](https://github.com/Mithril-mine/libmdbx-devel/issues/331)).
    - Fixed MSVC+AddressSanitizer builds failing to compile ([#62](https://github.com/Mithril-mine/libmdbx-devel/issues/62)).
    - Fixed linking with compiler-rt for `__builtin_cpu_supports()` by Clang.
    - Fixed Android NDK builds failing to compile with the auto-selected System V locking below API 26.
    - Fixed C++ API compilation where `<compare>` is not pulled transitively (Android NDK).
    - Fixed a spurious opening assertion on the platforms packing local buffers loosely (macOS/arm64).

 - Tools:
    - Fixed `mdbx_load` silently loading a dump truncated inside a table header; now reported ([#384](https://github.com/Mithril-mine/libmdbx-devel/issues/384)).
    - Fixed `mdbx_chk` crashing with `-i`/`-s` on named tables and over-reading a corrupt GC record ([#49](https://github.com/Mithril-mine/libmdbx-devel/issues/49), [#76](https://github.com/Mithril-mine/libmdbx-devel/issues/76)).
    - Fixed `mdbx_dump`/`mdbx_load` losing data on round-trip; a truncated dump passed as complete ([#61](https://github.com/Mithril-mine/libmdbx-devel/issues/61), [#70](https://github.com/Mithril-mine/libmdbx-devel/issues/70)).
    - Fixed `mdbx_dump -f` truncating the output file before the command line was validated ([#71](https://github.com/Mithril-mine/libmdbx-devel/issues/71)).
    - Fixed `mdbx_load` failing when reading a dump from standard input ([#50](https://github.com/Mithril-mine/libmdbx-devel/issues/50)).
    - Fixed `mdbx_load -p -s` deleting every named table; `-s` after a header and `-N`/`-p` misbehaved ([#312](https://github.com/Mithril-mine/libmdbx-devel/issues/312)).
    - Fixed `mdbx_load -b` failing on an unchanged batch; empty and long table names restored wrongly ([#312](https://github.com/Mithril-mine/libmdbx-devel/issues/312)).
    - Fixed `mdbx_stat -f` failing on an empty GC and `mdbx_dump` closing a failed table as complete ([#312](https://github.com/Mithril-mine/libmdbx-devel/issues/312)).
    - Fixed handling of table names with control characters: rejected in dumps, sanitized in listings ([#72](https://github.com/Mithril-mine/libmdbx-devel/issues/72)).
    - Fixed `mdbx_load` silently dropping the rest of a dump line at an embedded NUL byte; now an error ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).
    - Fixed `mdbx_load -p` failing on the main-table section of every multi-table dump ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).
    - Fixed `mdbx_stat -r`/`-rr` exiting with failure for informational reader-table outcomes ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).
    - Fixed tool loggers crashing on a null format and `mdbx_load` ctype calls on high-bit bytes ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).
    - Fixed `mdbx_copy` options not clustering and SIGINT/SIGTERM being silently swallowed ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).
    - Fixed `mdbx_chk` exiting with 3 instead of the documented interrupted code on a signal ([#485](https://github.com/Mithril-mine/libmdbx-devel/issues/485)).

 - C++ API:
    - Fixed binding a non-owning handle from an owning temporary leaking it; now a compile error ([#77](https://github.com/Mithril-mine/libmdbx-devel/issues/77), [#121](https://github.com/Mithril-mine/libmdbx-devel/issues/121)).
    - Fixed `txn_managed::checkpoint()` dropping the handle on a core rejection, leaking the writer lock ([#89](https://github.com/Mithril-mine/libmdbx-devel/issues/89)).
    - Fixed the `*_managed` wrappers terminating the process on a failed close, with a use-after-free ([#60](https://github.com/Mithril-mine/libmdbx-devel/issues/60)).
    - Fixed committing a parent with a live nested child double-freeing; now fails with `MDBX_BAD_TXN` ([#92](https://github.com/Mithril-mine/libmdbx-devel/issues/92)).
    - Fixed `mdbx::buffer` assignment from `string_view` allocating despite `noexcept`; could terminate ([#78](https://github.com/Mithril-mine/libmdbx-devel/issues/78)).
    - Fixed `cursor::erase("key")` erasing every value at the cursor instead of the named key ([#90](https://github.com/Mithril-mine/libmdbx-devel/issues/90)).
    - Fixed `buffer` writing wrong bytes on overlapping sources that forced a capacity grow ([#94](https://github.com/Mithril-mine/libmdbx-devel/issues/94), [#95](https://github.com/Mithril-mine/libmdbx-devel/issues/95), [#125](https://github.com/Mithril-mine/libmdbx-devel/issues/125), [#152](https://github.com/Mithril-mine/libmdbx-devel/issues/152)).
    - Fixed `buffer::reserve()` shrinking capacity, user memory rejected and the allocator lost ([#99](https://github.com/Mithril-mine/libmdbx-devel/issues/99)).
    - Fixed a `0`/`NULL` pointer with a length building a reference to address zero; now a compile error ([#137](https://github.com/Mithril-mine/libmdbx-devel/issues/137), [#142](https://github.com/Mithril-mine/libmdbx-devel/issues/142), [#162](https://github.com/Mithril-mine/libmdbx-devel/issues/162)).
    - Fixed the lookup/scan wrappers throwing or giving wrong results on multimaps and empty tables ([#98](https://github.com/Mithril-mine/libmdbx-devel/issues/98), [#127](https://github.com/Mithril-mine/libmdbx-devel/issues/127)).
    - Fixed several wrappers throwing on documented non-error results or returning wrong values ([#97](https://github.com/Mithril-mine/libmdbx-devel/issues/97), [#102](https://github.com/Mithril-mine/libmdbx-devel/issues/102)).
    - Fixed the C++ API not compiling in several documented configurations ([#100](https://github.com/Mithril-mine/libmdbx-devel/issues/100)).
    - Fixed overload resolution silently picking the wrong function (`if (slice)`, `move()`, `hex()`) ([#101](https://github.com/Mithril-mine/libmdbx-devel/issues/101)).
    - Fixed the C++ dumps API treating an empty source as a valid dump: the dump predicates now uniformly reject it ([#551](https://github.com/Mithril-mine/libmdbx-devel/issues/551)).
    - Fixed the C++ `slice::is_printable(true)` treating UTF-8 lead bytes as printable extended-ASCII ([#551](https://github.com/Mithril-mine/libmdbx-devel/issues/551)).
    - Fixed flags conversion dropping `MDBX_NOSTICKYTHREADS` used with `MDBX_EXCLUSIVE` ([#318](https://github.com/Mithril-mine/libmdbx-devel/issues/318)).

 - Performance:
    - Fixed the ~8x slowdown of short reads on Linux: ending a read-only transaction called `getpid()` ([#348](https://github.com/Mithril-mine/libmdbx-devel/issues/348)).
    - Fixed empty durable-mode commits still flushing the device, adding milliseconds to every no-op ([#64](https://github.com/Mithril-mine/libmdbx-devel/issues/64)).
    - Fixed every no-sync write-map commit on Windows flushing the whole mapping ([#65](https://github.com/Mithril-mine/libmdbx-devel/issues/65)).

 - Resource leaks:
    - Fixed four transaction error paths leaking table handles or promoting wrong geometry values ([#105](https://github.com/Mithril-mine/libmdbx-devel/issues/105)).
    - Fixed a memory leak on a zero-length table name.

 - Minor:
    - Fixed `mdbx_strerror()` having no message for `MDBX_BACKLOG_DEPLETED`, crashing error handlers ([#96](https://github.com/Mithril-mine/libmdbx-devel/issues/96)).
    - Fixed the non-printf logger passing the full length on truncation; the message is terminated now ([#116](https://github.com/Mithril-mine/libmdbx-devel/issues/116)).

### Appreciations:

 - [Andrea Lanfranchi](https://github.com/AndreaLanfranchi) for extensive bug reporting and assistance with fixing most of the issues below.
 - [Omar Gylyjov](https://github.com/dahiAbi) for reporting and fixing the Android database-open failure.
 - [wangkaiyi](https://github.com/wangkaiyi) for reporting the compacting-copy hang.
 - [Mohsin Ismail](https://github.com/mibaloch) for reporting the preopen-inspection defect.
 - [SSE4](https://github.com/SSE4) for reporting and fixing the writer crash on a concurrent start of a read transaction.

--------------------------------------------------------------------------------

## v0.15.0 at 2026-09-22

Not a release, but a technical tag marking the start of development for a new version branch.

The engineering of subsequent releases of the v0.14.x version line will continue in the `stable` branch.
In the `master` branch, development of version v0.15 will begin, with the technical version tag set to v0.15.0.

********************************************************************************

For early releases and changes see the ChangeLog-NN the git commit history.
