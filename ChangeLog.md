ChangeLog
=========

The source code is available on [SourceCraft](https://sourcecraft.dev/dqdkfa/libmdbx) and mirror on [GitHub](https://github.com/Mithril-mine/libmdbx).
Please use the `stable` branch or the latest release for production environments through staging, and the `master` branch for development of derivative projects.
Всё будет хорошо!

## v0.15.1 at a deep development stage

Cutting-edge release with new core features and internal refactoring.

### Important:

 - The [project website](https://libmdbx.dqdkfa.ru) is back online (it was lost in an attack by ukrainian/european bandera terrorists; no source code was damaged) and was significantly expanded:
    - a new textbook with about 50 examples, "deep dive" articles on the architecture and internals, and "first steps" guides;
    - a more readable API reference layout, with the public C/C++ API documentation completed and refined to pass a strict doxygen audit;
    - man pages for the tools (including the new `mdbx_defrag`) and a full option reference of every command-line utility.
 - The `master` branch develops v0.15; engineering of the v0.14.x line continues in the `stable` branch.

### Appreciations:

 - [Andrea Lanfranchi](https://github.com/AndreaLanfranchi) for extensive bug reporting and assistance with fixing most of the issues below.
 - [Omar Gylyjov](https://github.com/dahiAbi) for reporting and fixing the Android database-open failure.
 - [wangkaiyi](https://github.com/wangkaiyi) for reporting the compacting-copy hang.
 - [Mohsin Ismail](https://github.com/mibaloch) for reporting the preopen-inspection defect.

### Backward compatibility breaks:

 - The `MDBX_ENABLE_REFUND` build option was removed; the zero-cost online auto-compactification it controlled is now always enabled ([#193](https://github.com/Mithril-mine/libmdbx-devel/issues/193)).
 - The C++ `mdbx::env::get_preopen_snapinfo()` overloads without an explicit mode argument were removed; the remaining overloads take `mode = MDBX_PREOPEN_PROPERLY` by default, so ordinary calls keep working, but code relying on the old overload set no longer compiles ([#182](https://github.com/Mithril-mine/libmdbx-devel/issues/182)).
 - The C++ `mdbx::env::mode`/`mdbx::env::durability` enums are now scoped (`enum class`); the former `env::`-level enumerator names are kept as deprecated `[[deprecated(...)]]` aliases, so existing code keeps compiling while new code should use the scoped form (e.g. `mdbx::env::mode::nested_transactions` instead of `mdbx::env::nested_transactions`).

### Improvements:

 - Introduced a unified bilingual documentation site, six bilingual guides and consolidated engineering documentation.
 - Added a new bilingual textbook and integrated it into the documentation site, with the accompanying example programs ported and restructured.
 - Added `mdbx_preopen_snapinfo_ex()` with an explicit mode (`MDBX_PREOPEN_LEGACY`, `MDBX_PREOPEN_PROPERLY`, `MDBX_PREOPEN_PESSIMISTIC`, `MDBX_PREOPEN_OPTIMISTIC`) of processing weak checkpoints during a preopen inspection; `mdbx_preopen_snapinfo()` now corresponds to `PROPERLY` ([#182](https://github.com/Mithril-mine/libmdbx-devel/issues/182)).
 - Added the man page for the `mdbx_defrag` utility and updated the other man pages.
 - Added the `MDBX_AVOID_FTOK` build option for the System V IPC locking: it replaces the system `ftok()` with a key derived from the full file identity, drastically lowering the chance of a key collision ([#80](https://github.com/Mithril-mine/libmdbx-devel/issues/80)).
 - Added the `-h`/`--help` options to all command-line utilities.
 - Clarification/refining README and doxygen API descriptions.
 - On Windows the attempts to shrink the database file are no longer repeated on every commit when they are doomed to be deferred (while other processes use the database): they are skipped outright and rate-limited by the new `MDBX_opt_shrink_retry_period` option, which reduces the commit overhead for workloads mixing long-lived readers with the automatic file shrinking ([#170](https://github.com/Mithril-mine/libmdbx-devel/issues/170)).
 - Added the `MDBX_opt_shrink_retry_period` option: datafile shrink attempts that are doomed to be deferred are now skipped outright and rate-limited, which reduces the commit overhead for workloads mixing long-lived readers with automatic file shrinking ([#170](https://github.com/Mithril-mine/libmdbx-devel/issues/170)).

### Fixes:

 - Data loss and durability:
    - A failed disk write could be silently reported as success, and a large write-mapped durable commit could crash on Windows ([#47](https://github.com/Mithril-mine/libmdbx-devel/issues/47)).
    - Auxiliary page-write errors were silently ignored, masking a failing disk; on Windows they also clashed with the transaction file lock, so page pre-warming never worked ([#203](https://github.com/Mithril-mine/libmdbx-devel/issues/203)).
    - The writer trusted a forged snapshot-age hint from the shared lock file and could reuse pages still referenced by live readers; such a hint is now rejected ([#81](https://github.com/Mithril-mine/libmdbx-devel/issues/81)).
    - `mdbx_drop()` accepted the internal free-list handle and destroyed the garbage-collection tree, permanently leaking pages and corrupting accounting; such a call is now rejected ([#69](https://github.com/Mithril-mine/libmdbx-devel/issues/69)).
    - A failed second in-process open released the first handle's file locks, letting another process take exclusive access ([#59](https://github.com/Mithril-mine/libmdbx-devel/issues/59)).
    - A copy (`mdbx_env_copy()`, `mdbx_txn_copy2pathname()`) could delete a pre-existing destination database when its file lock failed, leave an orphan file blocking later copies, or keep the old file's tail after an overwrite ([#58](https://github.com/Mithril-mine/libmdbx-devel/issues/58), [#112](https://github.com/Mithril-mine/libmdbx-devel/issues/112)).

 - Resistance to corrupted or crafted "poisoned" data:
    - A crafted garbage-collection record listing one physical page twice got that page handed out to two b-tree nodes in a single write transaction, silently corrupting data; such records are now rejected ([#83](https://github.com/Mithril-mine/libmdbx-devel/issues/83)).
    - A crafted database could drive the engine out of bounds (crash or data corruption) when deleting records, dropping a table, estimating a range, or checking the file; such databases are now detected and rejected ([#57](https://github.com/Mithril-mine/libmdbx-devel/issues/57), [#63](https://github.com/Mithril-mine/libmdbx-devel/issues/63), [#74](https://github.com/Mithril-mine/libmdbx-devel/issues/74), [#75](https://github.com/Mithril-mine/libmdbx-devel/issues/75)).
    - Fixed a rare durability bug: a failed disk write during a commit could be silently reported as success, and on Windows a large write-mapped durable transaction could crash the engine while committing (issue #47).
    - The defragmentation walk became exponential on a crafted tree with shared sub-trees (a denial of service), and a crafted set of duplicate values could crash or corrupt memory; both are now rejected ([#82](https://github.com/Mithril-mine/libmdbx-devel/issues/82), [#176](https://github.com/Mithril-mine/libmdbx-devel/issues/176)).
    - The shared reader-table occupancy counter from a corrupt lock file drove out-of-bounds scans over the fixed-size reader table; the count is now clamped everywhere ([#79](https://github.com/Mithril-mine/libmdbx-devel/issues/79), [#113](https://github.com/Mithril-mine/libmdbx-devel/issues/113)).
    - `mdbx_gc_info()` looped forever holding the writer lock on a transaction that had already reclaimed a record; the documented visitor stop is now honored as well ([#88](https://github.com/Mithril-mine/libmdbx-devel/issues/88)).
    - `mdbx_cursor_scroll()` and `mdbx_cursor_distance()` indexed the cursor stack out of bounds when `deepness` was above `INT_MAX` ([#85](https://github.com/Mithril-mine/libmdbx-devel/issues/85)).

 - Crashes, hangs and undefined behavior in the core engine:
    - `mdbx_replace()`/`mdbx_replace_ex()` and the C++ `txn::replace_reserve()` used with `MDBX_RESERVE` could crash the process or silently lose the new value; `MDBX_RESERVE` on a table with duplicates is now rejected upfront, as `mdbx_put()` already did ([#166](https://github.com/Mithril-mine/libmdbx-devel/issues/166)).
    - `mdbx_replace_ex()` with overlapping buffers could hang or crash the next operation on the same table ([#56](https://github.com/Mithril-mine/libmdbx-devel/issues/56)).
    - `mdbx_env_copy()` with `MDBX_CP_COMPACT` hung forever when the dedicated writer thread hit a write error (e.g. a closed destination pipe or a full disk); the error is now reported instead ([#171](https://github.com/Mithril-mine/libmdbx-devel/issues/171)).
    - `mdbx_txn_rollback()` on a nested write transaction could leave the parent unusable or corrupt it when the restart failed; nested transactions are now finalized cleanly ([#103](https://github.com/Mithril-mine/libmdbx-devel/issues/103), [#124](https://github.com/Mithril-mine/libmdbx-devel/issues/124)).
    - Cloning a read transaction (`mdbx_txn_clone()`) could crash the process on cleanup paths, or yield a clone reading already-recycled pages when a parked origin had been ousted; such clones now fail cleanly with `MDBX_OUSTED`/`MDBX_EINVAL` ([#128](https://github.com/Mithril-mine/libmdbx-devel/issues/128), [#148](https://github.com/Mithril-mine/libmdbx-devel/issues/148)).
    - Parking and unparking read transactions misbehaved: parking a failed transaction crashed on the later abort, cloning a parked transaction produced an unusable clone, and unparking a broken or ousted transaction reported success; all three now behave as documented ([#104](https://github.com/Mithril-mine/libmdbx-devel/issues/104)).
    - A nested-transaction begin failing out-of-memory left the parent transaction in an inconsistent state ([#67](https://github.com/Mithril-mine/libmdbx-devel/issues/67)).
    - `mdbx_cursor_count_ex()` wrote past the caller's buffer when given the legacy structure size ([#68](https://github.com/Mithril-mine/libmdbx-devel/issues/68)).
    - A hardware divide-by-zero in `mdbx_cursor_put(MDBX_MULTIPLE)` with a zero element size and a huge count; such a put is now rejected ([#73](https://github.com/Mithril-mine/libmdbx-devel/issues/73)).
    - Undefined behavior in the JSON-integer key encoding (a negative shift for magnitudes above 2^52, negation of `INT64_MIN`, an empty key passed to `memcpy`).
    - An arithmetic overflow miscomputed the maximum database size on 32-bit builds with large RAM.
    - The `MDBX_TXN_TRY` flag leaked into the transaction state flags.
    - A forked child treated a failed unlock of an inherited inter-process lock as fatal only at certain log levels; the failure is now always tolerated ([#111](https://github.com/Mithril-mine/libmdbx-devel/issues/111)).

 - Wrong results and API behavior:
    - `mdbx_cursor_get_batch()` with `MDBX_FIRST` on an already positioned cursor silently returned only the tail of the table instead of rewinding; `MDBX_FIRST` now always starts from the first pair ([#151](https://github.com/Mithril-mine/libmdbx-devel/issues/151)).
    - `mdbx_cursor_scroll()` and `mdbx_cursor_distribute()` returned wrong positions or counts on tables with duplicates near the tree height, and distribute corrupted one of the caller's cursors; distribution now uses a private iterator ([#84](https://github.com/Mithril-mine/libmdbx-devel/issues/84), [#123](https://github.com/Mithril-mine/libmdbx-devel/issues/123)).
    - `mdbx_env_stat_ex()` under-counted the totals once a named table was used in the transaction ([#108](https://github.com/Mithril-mine/libmdbx-devel/issues/108)).
    - `mdbx_estimate_distance()` returned a negated result when both cursors were on different duplicates of one key ([#86](https://github.com/Mithril-mine/libmdbx-devel/issues/86)).
    - `mdbx_jsonInteger_from_key()` decoded the stored integer `0` as `INT64_MAX` ([#87](https://github.com/Mithril-mine/libmdbx-devel/issues/87)).
    - `mdbx_dbi_rename()` stored a stale table record when the handle was in the stale state (e.g. after an aborted transaction), silently corrupting the database ([#106](https://github.com/Mithril-mine/libmdbx-devel/issues/106)).
    - Table open/rename mishandled the documented pseudo-names and cross-process drops: a crash on `MDBX_CHK_GC`/`MDBX_CHK_META`, success with the old flags instead of `MDBX_INCOMPATIBLE`, and success instead of `MDBX_NOTFOUND` for a table dropped by another process ([#107](https://github.com/Mithril-mine/libmdbx-devel/issues/107)).
    - `mdbx_env_chk()` left `result.tables` dangling after the check, and the C++ `env::chk()` threw an internal error for every database with a found problem instead of reporting it ([#93](https://github.com/Mithril-mine/libmdbx-devel/issues/93)).
    - `mdbx_strerror()` had no message for `MDBX_BACKLOG_DEPLETED`, so the C++ `error::what()` returned null and crashed typical error handlers ([#96](https://github.com/Mithril-mine/libmdbx-devel/issues/96)).
    - The non-printf logger passed the full formatted length to the callback even when the message did not fit the application's buffer; the truncated message is now properly terminated ([#116](https://github.com/Mithril-mine/libmdbx-devel/issues/116)).
    - `mdbx_env_warmup()` crashed on a created-but-not-opened environment (it now returns `MDBX_EPERM`), and setting a sensible `MDBX_opt_gc_time_limit` no longer drops the automatic `rp_augment_limit` to its minimum ([#114](https://github.com/Mithril-mine/libmdbx-devel/issues/114)).
    - With `MDBX_WRITEMAP`, `mdbx_txn_info()` reported doubled dirty space and lost the spilled pages (the counters shared one memory cell), and `mdbx_ratio2digits()` returned a string with an uninitialized trailing byte ([#117](https://github.com/Mithril-mine/libmdbx-devel/issues/117)).

 - Windows:
    - A long-lived read transaction could deadlock a writer that grew the database file; the file is now grown in place, and a shrink or remap blocked by an active reader is deferred and retried instead of hanging or poisoning the environment ([#154](https://github.com/Mithril-mine/libmdbx-devel/issues/154)).
    - Opening a database in exclusive mode switched to the slow flush-based commit path; exclusive-mode commits are now as fast as usual, roughly doubling the exclusive-mode speed ([#179](https://github.com/Mithril-mine/libmdbx-devel/issues/179)).
    - `MDBX_NOMETASYNC`/`MDBX_SAFE_NOSYNC` degraded to full durable syncs on every commit while a datafile shrink stayed deferred ([#154](https://github.com/Mithril-mine/libmdbx-devel/issues/154)).
    - The datafile shrink decision depended on discard bookkeeping instead of the real file size (all platforms), so a shrink could be skipped indefinitely ([#154](https://github.com/Mithril-mine/libmdbx-devel/issues/154)).
    - A file-lock request that ended after a wait left the granted lock held (blocking later opens) and could corrupt a stack variable; a failed overlapped result also reported success ([#110](https://github.com/Mithril-mine/libmdbx-devel/issues/110)).
    - `MDBX_DBG_*` switches given as an empty string were ignored ([#109](https://github.com/Mithril-mine/libmdbx-devel/issues/109)).
    - A spurious internal assertion failure during page allocation ([#52](https://github.com/Mithril-mine/libmdbx-devel/issues/52)).
    - The I/O ring merged only two contiguous write chunks instead of all of them ([#111](https://github.com/Mithril-mine/libmdbx-devel/issues/111)).

 - Tools:
    - `mdbx_chk` crashed on databases with named sub-tables when using the `-i`/`-s` options, and read out of bounds on a corrupted GC record ([#49](https://github.com/Mithril-mine/libmdbx-devel/issues/49), [#76](https://github.com/Mithril-mine/libmdbx-devel/issues/76)).
    - `mdbx_dump`/`mdbx_load` silently lost data on round-trip when a key or value started with a space, `mdbx_dump` reported success even when writing the dump failed, and `mdbx_load` accepted a truncated dump as a complete database ([#61](https://github.com/Mithril-mine/libmdbx-devel/issues/61), [#70](https://github.com/Mithril-mine/libmdbx-devel/issues/70)).
    - `mdbx_dump -f` truncated the output file before the command line was validated ([#71](https://github.com/Mithril-mine/libmdbx-devel/issues/71)).
    - `mdbx_load` failed with a missing-file error when reading a dump from standard input ([#50](https://github.com/Mithril-mine/libmdbx-devel/issues/50)).
    - Table names containing control characters: dumping was refused, listing and stat output are now sanitized, and such a name in a `database=` header is rejected ([#72](https://github.com/Mithril-mine/libmdbx-devel/issues/72)).

 - C++ API:
    - Initializing a non-owning `txn`/`cursor`/`env` handle from an owning `*_managed` temporary silently dropped ownership — the writer lock, cursor or environment leaked; such moves are now compile-time errors ([#77](https://github.com/Mithril-mine/libmdbx-devel/issues/77), [#121](https://github.com/Mithril-mine/libmdbx-devel/issues/121)).
    - `txn_managed::checkpoint()` dropped the handle when the core rejected the transaction, leaking the writer lock until the environment was closed ([#89](https://github.com/Mithril-mine/libmdbx-devel/issues/89)).
    - The `*_managed` wrappers could terminate the process on a failed `close()`/`abort()`, and `env_managed::close()` had a use-after-free ([#60](https://github.com/Mithril-mine/libmdbx-devel/issues/60)).
    - Committing or aborting a parent transaction while its nested child was alive tore the child down and later double-freed its handle; it now fails with `MDBX_BAD_TXN` ([#92](https://github.com/Mithril-mine/libmdbx-devel/issues/92)).
    - Assigning a `std::string_view` to `mdbx::buffer` was declared `noexcept` although it allocates, so an out-of-memory terminated the process instead of throwing a catchable exception ([#78](https://github.com/Mithril-mine/libmdbx-devel/issues/78)).
    - `cursor::erase("key")` bound string literals to the boolean overloads and erased every value at the cursor position instead of the named key ([#90](https://github.com/Mithril-mine/libmdbx-devel/issues/90)).
    - `buffer` append/assign/add_header wrote wrong bytes or read freed storage when the source overlapped the buffer's own storage and it had to grow; such a source is now copied out first ([#94](https://github.com/Mithril-mine/libmdbx-devel/issues/94), [#95](https://github.com/Mithril-mine/libmdbx-devel/issues/95), [#125](https://github.com/Mithril-mine/libmdbx-devel/issues/125), [#152](https://github.com/Mithril-mine/libmdbx-devel/issues/152)).
    - `buffer::reserve()` shrank the capacity against STL expectations, `buffer(txn, slice)` rejected user memory and an empty slice, and `txn::extract()`/`replace()` lost the caller's allocator ([#99](https://github.com/Mithril-mine/libmdbx-devel/issues/99)).
    - A literal `0`/`NULL` passed as a pointer with a length built a reference to address zero or crashed; it is a compile-time error again, and a null pointer with a non-zero length is rejected with `bad_value_size` in every `slice`/`buffer` form, including the reserve methods ([#137](https://github.com/Mithril-mine/libmdbx-devel/issues/137), [#142](https://github.com/Mithril-mine/libmdbx-devel/issues/142), [#162](https://github.com/Mithril-mine/libmdbx-devel/issues/162)).
    - The lookup/scan wrappers threw or returned wrong results in normal cases: an exact match on a multimap, an empty table, the last chunk, and `get_batch()` with a small limit ([#98](https://github.com/Mithril-mine/libmdbx-devel/issues/98), [#127](https://github.com/Mithril-mine/libmdbx-devel/issues/127)).
    - Several wrappers threw on documented non-error results or returned wrong values: `thread_register()`/`thread_unregister()`, `sequence()` overflow, `commit()` of an errored transaction, `txn::flags()`, `env::get_pagesize()` under an open reader, `amend(true)`, `try_start_write()`, `setup_debug()`, the sync-period setter ([#97](https://github.com/Mithril-mine/libmdbx-devel/issues/97), [#102](https://github.com/Mithril-mine/libmdbx-devel/issues/102)).
    - The API did not compile in documented configurations: the experimental-filesystem selector, `buffer(std::to_string(..))`, `append(basic_string_view)`, `slice(std::span)` ([#100](https://github.com/Mithril-mine/libmdbx-devel/issues/100)).
    - Overload-resolution traps silently picked the wrong function: `if (slice)` was always true, a three-argument `cursor::move()` misbound its arguments, `buffer(p, 3)` ran `strlen` instead of honoring the length, and `hex()`/`base58()`/`base64()` of a view encoded the pointer bytes instead of the data ([#101](https://github.com/Mithril-mine/libmdbx-devel/issues/101)).
    - Added the non-throwing `slice::try_as_pod<T>()` returning an empty `std::optional` on a size mismatch ([#144](https://github.com/Mithril-mine/libmdbx-devel/issues/144)).
    - Documentation refined: the `release_all_cursors(false)` and `unbind_all_cursors()` hazards, the `env::chk()` tables-pointer lifetime, the single-thread requirement of `env_managed` close, the throwing behavior of the environment configuration setters, and the `append_producer()` precondition ([#91](https://github.com/Mithril-mine/libmdbx-devel/issues/91), [#143](https://github.com/Mithril-mine/libmdbx-devel/issues/143), [#145](https://github.com/Mithril-mine/libmdbx-devel/issues/145), [#146](https://github.com/Mithril-mine/libmdbx-devel/issues/146)).

 - Performance:
    - Empty durable-mode commits still flushed the device, adding several milliseconds to every no-op commit ([#64](https://github.com/Mithril-mine/libmdbx-devel/issues/64)).
    - Every no-sync commit in write-map mode on Windows flushed the whole mapping ([#65](https://github.com/Mithril-mine/libmdbx-devel/issues/65)).

 - Platform support:
    - `mdbx_env_open()` failed with `EINVAL` on Android (f2fs) due to an invalid internal file position ([#197](https://github.com/Mithril-mine/libmdbx-devel/issues/197)).
    - OpenBSD was excluded from the POSIX-semaphore locking auto-selection, since its `sem_init()` always fails for process-shared semaphores and a default build could not open any environment ([#115](https://github.com/Mithril-mine/libmdbx-devel/issues/115)).
    - The System V IPC locking could destroy another live database's semaphores on a key collision; every semaphore set now carries a signature derived from the database file identity, and the open refuses with an error instead of removing a foreign set ([#80](https://github.com/Mithril-mine/libmdbx-devel/issues/80), [#159](https://github.com/Mithril-mine/libmdbx-devel/issues/159)).
    - `mdbx_preopen_snapinfo()` reported corruption for a database whose newest checkpoint is a weak one of the current boot (e.g. after an unclean shutdown with `MDBX_SAFE_NOSYNC`); preopen now falls back to the usable steady checkpoint, while read-only opens return `MDBX_WANNA_RECOVERY` ([#182](https://github.com/Mithril-mine/libmdbx-devel/issues/182)).

 - Build:
    - Fixed MSVC+AddressSanitizer builds failing to compile ([#62](https://github.com/Mithril-mine/libmdbx-devel/issues/62)).
    - Fixed linking with compiler-rt for `__builtin_cpu_supports()` by Clang.
    - Conan recipe: added the `mdbx.lto` and `mdbx.sanitize` options and documented the package binary identity rules.

 - Leaks:
    - Four transaction error paths leaked table handles or promoted wrong geometry values: a failed checkpoint or commit-as-read, a write start failing after the snapshot was taken, a nested resize that could not be undone, and a discarded meta write error that hid an I/O failure ([#105](https://github.com/Mithril-mine/libmdbx-devel/issues/105)).
    - Fixed the memory leak caused by a table name with zero length.

--------------------------------------------------------------------------------

## v0.15.0 at 2026-09-22

Not a release, but a technical tag marking the start of development for a new version branch.

The engineering of subsequent releases of the v0.14.x version line will continue in the `stable` branch.
In the `master` branch, development of version v0.15 will begin, with the technical version tag set to v0.15.0.

********************************************************************************

For early releases and changes see the ChangeLog-NN the git commit history.
