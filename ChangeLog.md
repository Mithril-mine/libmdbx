ChangeLog
=========

The source code is available on [SourceCraft](https://sourcecraft.dev/dqdkfa/libmdbx) and mirror on [GitHub](https://github.com/Mithril-mine/libmdbx).
Please use the `stable` branch or the latest release for production environments through staging, and the `master` branch for development of derivative projects.
Всё будет хорошо!

## v0.14.5 at a release engineering stage

The supporting release of a stable branch with bug fixes.

### Important:

The development of subsequent releases in the v0.14.x version line will continue in the `stable` branch.
In the `master` branch, development of version v0.15 will begin, with the technical version tag set to v0.15.0.

### Appreciations:

 - [Andrea Lanfranchi](https://github.com/AndreaLanfranchi) for bug reporting and assistance with fixing ones.

### Fixes:

 - Core:
    - Fixed undefined behavior in the JSON-integer key encoding: a negative shift when encoding magnitudes above 2^52 and the negation of `INT64_MIN` were UB caught by UBSAN, and a null-but-empty key could be passed to `memcpy` while inserting a node (backport).
    - Fixed the `MDBX_TXN_TRY` flag leaking into transaction state flags (backport).
    - Fixed `mdbx_env_copy()`/`mdbx_copy -f` possibly deleting a pre-existing destination database when it could not be locked ([#58](https://github.com/Mithril-mine/libmdbx-devel/issues/58)) (backport).
    - Fixed a failed second in-process open releasing the first handle's file locks, letting another process take exclusive access ([#59](https://github.com/Mithril-mine/libmdbx-devel/issues/59)) (backport).
    - Fixed a checked-build crash when a nested-transaction begin failed partway through the loose-page move ([#67](https://github.com/Mithril-mine/libmdbx-devel/issues/67)) (backport).
    - Fixed a nested-transaction begin failing out-of-memory leaving the parent transaction in an inconsistent state ([#67](https://github.com/Mithril-mine/libmdbx-devel/issues/67)) (backport).
    - Fixed `mdbx_cursor_count_ex()` writing past the caller's buffer when called with the legacy structure size ([#68](https://github.com/Mithril-mine/libmdbx-devel/issues/68)) (backport).
    - Fixed `mdbx_drop()` accepting the internal free-list (GC) handle and destroying the garbage-collection tree, permanently leaking every tracked page and corrupting page accounting; the call is now rejected ([#69](https://github.com/Mithril-mine/libmdbx-devel/issues/69)) (backport).
    - Fixed `mdbx_cursor_scroll()` and `mdbx_cursor_distribute()` returning wrong positions or counts on DUPSORT tables when `deepness` was at or above the tree height: scroll was off by one step for each duplicate set it entered or left, and distribute used (and corrupted) one of the caller's cursors as its working iterator. Distances measured from inside a single-level duplicate set are now also discounted by the start position; the functions account the duplicates consistently and distribute clones a private iterator ([#84](https://github.com/Mithril-mine/libmdbx-devel/issues/84)) (backport).
    - Fixed `mdbx_dbi_rename()` storing a stale table record under the new name when the handle begins a write transaction in the `DBI_STALE` state (e.g. after an aborted transaction), silently corrupting the database; the handle is now refreshed before the rename, as in `mdbx_dbi_sequence()` ([#106](https://github.com/Mithril-mine/libmdbx-devel/issues/106)) (backport).
    - Fixed table open/rename mishandling the documented pseudo-names: `MDBX_CHK_GC`/`MDBX_CHK_META` were dereferenced before the pointer comparison and crashed the process, re-opening a non-empty table through an open handle with `MDBX_CREATE` and different flags returned success with the old flags instead of `MDBX_INCOMPATIBLE`, and opening a table that another process dropped returned success instead of `MDBX_NOTFOUND` ([#107](https://github.com/Mithril-mine/libmdbx-devel/issues/107)) (backport).
    - Fixed `mdbx_env_stat_ex()` under-counting the totals once a named table was used in the transaction, because the MainDB table-record scan skipped the rest of the leaf page; `gc_reserve4return()` now checks the result of `gc_prepare_stockpile4update()`, and the multi-page prefault in `gc_alloc_ex()` writes each page run at its real file offset ([#108](https://github.com/Mithril-mine/libmdbx-devel/issues/108)) (backport).
    - Fixed `mdbx_env_warmup()` accepting an environment that was created but not opened and dereferencing its unmapped database with a crash; it now returns `MDBX_EPERM` like the other map-using calls. On Windows the `MDBX_opt_writethrough_threshold` getter now returns the value the setter accepts, so save/restore round-trips work. And `default_rp_augment_limit()` now converts the stored GC time limit from monotonic units before comparing it with the 16-second scale, so setting a sensible `MDBX_opt_gc_time_limit` no longer drops the automatic `rp_augment_limit` to its minimum ([#114](https://github.com/Mithril-mine/libmdbx-devel/issues/114)) (backport).
    - Fixed `mdbx_env_copy()`/`mdbx_txn_copy2pathname()` mishandling the destination file: with `MDBX_CP_OVERWRITE` a copy whose file lock failed could unlink a destination owned by a concurrent copy (which then reported success with no file left); an exclusive-create failure could leave an orphan file blocking later copies; and an overwritten destination kept the old file's tail. The destination is now truncated once locked, survives lock conflicts and un-owned create failures, and is marked damaged before the stub meta write ([#112](https://github.com/Mithril-mine/libmdbx-devel/issues/112)) (backport).
    - Fixed `mdbx_env_copy()`/`mdbx_txn_copy2pathname()` with `MDBX_CP_OVERWRITE` emptying a destination file before validating the transaction and the flags, so a rejected copy deleted the existing file; the old content is now dropped only right before the copy, after all checks ([#119](https://github.com/Mithril-mine/libmdbx-devel/issues/119)) (backport).
    - Fixed the non-printf logger passing the full formatted length to the callback even when the message did not fit the application's buffer: on POSIX the callback was told to read past the end of the caller's memory, and on Windows the over-long message was silently dropped. The truncated message is now terminated and delivered with the length that actually fits ([#116](https://github.com/Mithril-mine/libmdbx-devel/issues/116)) (backport).
    - Fixed four defects: with `MDBX_WRITEMAP` the dirty and spilled page counters used one memory cell, so `mdbx_txn_info()` reported twice the dirty space before a spill and lost the spilled pages after it (the counters are now separate); `mdbx_ratio2digits()` with zero or negative precision returned a string with an uninitialized trailing byte; the build-time range check for `MDBX_CHECKING` let out-of-range checking levels compile; and a debug probe for a dangling page read the write-part of a read-only transaction, which is not allocated ([#117](https://github.com/Mithril-mine/libmdbx-devel/issues/117)) (backport).
    - Fixed the read-transaction lifecycle: parking a read transaction that had an error cleared its reader slot but kept it live so the later abort double-released the slot and could terminate the process, cloning a parked transaction registered its snapshot wrongly (the clone was unusable after unpark), and an unpark that had to finish the transaction reported success instead of `MDBX_OUSTED`/`MDBX_BAD_TXN` ([#104](https://github.com/Mithril-mine/libmdbx-devel/issues/104)) (backport).
    - Fixed `mdbx_txn_rollback()` of a nested write transaction leaving it unusable: the transaction was restarted without clearing its finished flags and without releasing the lists the nested begin had allocated, so every later call and the parent's commit failed with `MDBX_BAD_TXN` (and debug builds aborted on an assertion); the nested transaction now keeps running as documented and the parent can commit ([#103](https://github.com/Mithril-mine/libmdbx-devel/issues/103)) (backport).
    - Fixed `mdbx_env_chk()` leaving a dangling `result.tables` pointer after the check: it pointed into the checker's internal state which was freed before returning ([#93](https://github.com/Mithril-mine/libmdbx-devel/issues/93)) (backport).
    - Fixed `mdbx_estimate_distance()` returning a negated result when both cursors were on different duplicates of one key ([#86](https://github.com/Mithril-mine/libmdbx-devel/issues/86)) (backport).
    - Fixed `mdbx_jsonInteger_from_key()` decoding the stored integer `0` as `INT64_MAX`; zero now round-trips correctly ([#87](https://github.com/Mithril-mine/libmdbx-devel/issues/87)) (backport).
    - Excluded OpenBSD from the POSIX-semaphore locking auto-selection, since its `sem_init()` always fails with `EPERM` for process-shared semaphores and thus a default build could not open any environment; also corrected the misleading comment naming macOS, which uses the POSIX-2001 path anyway ([#115](https://github.com/Mithril-mine/libmdbx-devel/issues/115)) (backport).
    - Made the datafile shrink decision depend on the real file size instead of the discard bookkeeping, so a shrink is no longer skipped indefinitely and is retried until it happens ([#154](https://github.com/Mithril-mine/libmdbx-devel/issues/154)) (backport).
    - Stopped `MDBX_NOMETASYNC`/`MDBX_SAFE_NOSYNC` degrading to full durable syncs on every commit while a datafile shrink stays deferred; the extra sync is now performed only while the steady checkpoint is still beyond the new end of the file ([#154](https://github.com/Mithril-mine/libmdbx-devel/issues/154)) (backport).
    - Added the `MDBX_opt_shrink_retry_period` option: datafile shrink attempts that are doomed to be deferred are now skipped outright and rate-limited, which reduces the commit overhead for workloads mixing long-lived readers with automatic file shrinking ([#170](https://github.com/Mithril-mine/libmdbx-devel/issues/170)) (backport).
    - Auxiliary page-write errors were silently ignored, masking a failing disk; on Windows they also clashed with the transaction file lock, so page pre-warming never worked ([#203](https://github.com/Mithril-mine/libmdbx-devel/issues/203)) (backport).
    - Fixed `mdbx_preopen_snapinfo()` reporting a database as corrupted when its newest checkpoint was a weak one of the current boot, even though a usable steady checkpoint existed; the inspection now accepts an explicit mode and returns the steady-checkpoint geometry instead ([#182](https://github.com/Mithril-mine/libmdbx-devel/issues/182)) (backport).
    - Fixed `osal_fseek_shut()` leaving a failing offset in the out-parameter when the binary search ended on a failing probe ([#198](https://github.com/Mithril-mine/libmdbx-devel/issues/198)) (backport).

 - Crafted or rare specific conditions:
    - Fixed a rare durability bug: a failed disk write during a commit could be silently reported as success (backport).
    - Fixed an arithmetic overflow that miscomputed the maximum database size on 32-bit builds with large RAM (backport).
    - Fixed a rare internal structure corruption during rollback of a duplicate insertion (backport).
    - Fixed `mdbx_replace_ex()` possibly hanging or crashing the next operation on the same sub-table when called with overlapping buffers ([#56](https://github.com/Mithril-mine/libmdbx-devel/issues/56)) (backport).
    - Fixed a crafted database file that passed validation and could later corrupt data or crash when deleted ([#57](https://github.com/Mithril-mine/libmdbx-devel/issues/57)) (backport).
    - Fixed a crafted database file driving the engine out of bounds (crash or data corruption) when deleting records ([#63](https://github.com/Mithril-mine/libmdbx-devel/issues/63)) (backport).
    - Fixed a hardware divide-by-zero in `mdbx_cursor_put(MDBX_MULTIPLE)` when a `MDBX_DUPFIXED` element size of zero was supplied together with a huge element count; the put is now rejected with `MDBX_BAD_VALSIZE` ([#73](https://github.com/Mithril-mine/libmdbx-devel/issues/73)) (backport).
    - Fixed `tree_cutoff_twig()` recursing without bound on a crafted database whose branch page forms a cycle, overflowing the C stack when a table is dropped under `MDBX_VALIDATION`; the recursion depth is now bounded ([#74](https://github.com/Mithril-mine/libmdbx-devel/issues/74)) (backport).
    - Fixed a hardware divide-by-zero in the range-estimation API (`mdbx_estimate_*`) when a crafted database had a valid tree root but zero leaf/branch page counts; such a tree is now rejected on open with `MDBX_VALIDATION` ([#75](https://github.com/Mithril-mine/libmdbx-devel/issues/75)) (backport).
    - Fixed the shared reader-table occupancy counter being used unclamped as a loop/allocation bound over the fixed-size reader table, so a too-large value from a corrupt lock file drove out-of-bounds reads (and a heap overflow on 32-bit); it is now clamped to `max_readers` ([#79](https://github.com/Mithril-mine/libmdbx-devel/issues/79)) (backport).
    - Fixed the writer trusting a forged `cached_oldest_txnid` from the external lock mapping, reusing pages still referenced by live readers; it is now rejected (with a forced reader-table rescan) whenever it exceeds the steady snapshot, and the hung-reader recovery no longer passes a non-positive reader pid to the possibly kill-capable HSR callback ([#81](https://github.com/Mithril-mine/libmdbx-devel/issues/81)) (backport).
    - Fixed the defragmentation walk becoming exponential on a crafted/corrupt tree whose branch pages reference shared sub-trees (a reconverging DAG): the walk was bounded only by the depth of a single path, so it re-descended shared pages once per root-to-leaf path (a denial of service); the walk now rejects any page referenced more than once (a valid copy-on-write tree never shares a page) and is bounded by the file's page count ([#82](https://github.com/Mithril-mine/libmdbx-devel/issues/82)) (backport).
    - Fixed `pnl_merge()` creating a page-list with one page listed twice when two retiring lists overlapped on a crafted/corrupt GC: a merged list could hand the same physical page out twice within one transaction, aliasing two b-tree nodes; the manual merge path now squeezes duplicates out of the result ([#83](https://github.com/Mithril-mine/libmdbx-devel/issues/83)) (backport).
    - Fixed `osal_ipclock_unlock()` in a forked child treating a failed unlock as fatal only when NOTICE logging was enabled, so whether the child panicked depended on the log level; the failure is now always tolerated in a child. Also fixed a Windows I/O ring missing the `last_bytes` update when extending a `WriteFile` item, so only two contiguous chunks were merged instead of all of them ([#111](https://github.com/Mithril-mine/libmdbx-devel/issues/111)) (backport).
    - Fixed `mdbx_env_info_ex()` (and `mdbx_env_chk()`) reading the shared reader-table occupancy counter without clamping it to `max_readers`, so a corrupt or hostile lock file drove an out-of-bounds scan over the fixed-size reader table and crashed the process, as the other reader-table scans already did after the #79 fix; the count is now clamped like everywhere else ([#113](https://github.com/Mithril-mine/libmdbx-devel/issues/113)) (backport).
    - Fixed `mdbx_cursor_scroll()` and `mdbx_cursor_distance()` indexing the cursor stack out of bounds when `deepness` was above `INT_MAX` (e.g. `UINT_MAX`); the value is now clamped like in `mdbx_cursor_distribute()` ([#85](https://github.com/Mithril-mine/libmdbx-devel/issues/85)) (backport).
    - Fixed crafted databases with invalid offsets inside a set of duplicate values crashing or corrupting memory; nested DUPSORT sub-page offsets/geometry are now validated on every path and such databases are rejected ([#176](https://github.com/Mithril-mine/libmdbx-devel/issues/176)) (backport).
    - Fixed `mdbx_env_copy()` with `MDBX_CP_COMPACT` hanging forever when the dedicated writer thread hit a write error (e.g. a closed destination pipe or a full disk); the error is now reported instead ([#171](https://github.com/Mithril-mine/libmdbx-devel/issues/171)) (backport).

 - Windows:
    - Fixed a spurious internal assertion failure during page allocation on Windows ([#52](https://github.com/Mithril-mine/libmdbx-devel/issues/52)) (backport).
    - Fixed Windows crashes when committing large write-mapped durable transactions (backport).
    - Fixed Windows rejecting long environment values (e.g. long build paths) (backport).
    - Fixed Windows `osal_getenv_singlethreaded()` reporting an environment variable set to an empty value as not set, so `MDBX_DBG_*` switches given as an empty string were ignored; an empty value is now returned as-is again. Also, a failed growth of the static value buffer no longer reports success or loses the previous buffer ([#109](https://github.com/Mithril-mine/libmdbx-devel/issues/109)) (backport).
    - Fixed Windows `flock_ex()` handling of a lock request that ends after a wait: on timeout the previously granted lock stayed held (untracked, blocking later opens) and the stack `OVERLAPPED` could be written by the kernel after return; a failed `GetOverlappedResult()` also reported success because the byte count was stored into the error slot. The cancelled request is now awaited and its lock released if it was granted meanwhile, and the real error is returned ([#110](https://github.com/Mithril-mine/libmdbx-devel/issues/110)) (backport).
    - Fixed opening a database in exclusive mode switching to the slow flush-based commit path; exclusive-mode commits are now as fast as usual, roughly doubling the exclusive-mode speed ([#179](https://github.com/Mithril-mine/libmdbx-devel/issues/179)) (backport).
    - Fixed a Windows hang: a long-lived read transaction could deadlock a writer that grew the database file; the file is now grown in place, and a shrink or remap blocked by an active reader is deferred and retried instead of hanging or poisoning the environment ([#154](https://github.com/Mithril-mine/libmdbx-devel/issues/154)) (backport).

 - Tools:
    - Added the man page for the `mdbx_defrag` utility (backport).
    - Fixed `mdbx_load` failing with a missing-file error when reading a dump from standard input ([#50](https://github.com/Mithril-mine/libmdbx-devel/issues/50)) (backport).
    - Fixed `mdbx_chk` crashing on databases with named sub-tables when using the `-i`/`-s` options ([#49](https://github.com/Mithril-mine/libmdbx-devel/issues/49), [#51](https://github.com/Mithril-mine/libmdbx-devel/issues/51)) (backport).
    - Fixed `mdbx_dump -p`/`mdbx_load` silently losing data on round-trip when a key or value starts with a space ([#61](https://github.com/Mithril-mine/libmdbx-devel/issues/61)) (backport).
    - Fixed `mdbx_dump -f` truncating the output file on a mistyped command line before the arguments were validated ([#71](https://github.com/Mithril-mine/libmdbx-devel/issues/71)) (backport).
    - Fixed `mdbx_dump` reporting success even when writing the dump to a file failed, and `mdbx_load` accepting a truncated dump as a complete database ([#70](https://github.com/Mithril-mine/libmdbx-devel/issues/70)) (backport).
    - Fixed `mdbx_dump`/`mdbx_load`/`mdbx_stat` mishandling table names containing control characters: dumping was refused, listing and stat output are sanitized, and such a name in a `database=` header is rejected ([#72](https://github.com/Mithril-mine/libmdbx-devel/issues/72)) (backport).
    - Fixed `mdbx_chk` reading out of bounds and crashing on a corrupted GC record whose page count exceeded the page-list limit; the count is now clamped to what the record can hold ([#76](https://github.com/Mithril-mine/libmdbx-devel/issues/76)) (backport).

 - Performance:
    - Fixed a performance regression: empty durable-mode commits still flushed the device, adding several milliseconds to every no-op commit ([#64](https://github.com/Mithril-mine/libmdbx-devel/issues/64)) (backport).
    - Fixed a Windows performance regression: every no-sync commit in write-map mode flushed the whole mapping ([#65](https://github.com/Mithril-mine/libmdbx-devel/issues/65)) (backport).
    - Optimized `MDBX_APPEND` for a persistent cursor: each appended key used to re-descend the tree from the root for the last key, making ordered bulk load slower than an ordinary ascending insert; the last key is now read from the current leaf, dropping the per-key descents to zero with an unchanged error/exact-match contract ([#66](https://github.com/Mithril-mine/libmdbx-devel/issues/66)) (backport).

 - C++ API:
    - Fixed the C++ `*_managed` wrappers possibly terminating the process on a failed `close()`/`abort()`, and an `env_managed::close()` use-after-free ([#60](https://github.com/Mithril-mine/libmdbx-devel/issues/60)) (backport).
    - Fixed `mdbx::buffer` assigning `std::string_view` being declared `noexcept` although it allocates owned storage, so an out-of-memory terminated the process via `std::terminate()` instead of propagating a catchable exception; the operator now throws normally. Also fixed `is_freestanding()` comparing the payload offset with `<` instead of `<=`, misclassifying an owned buffer whose data sits at the very end of the silo as a non-owning reference ([#78](https://github.com/Mithril-mine/libmdbx-devel/issues/78)) (backport).
    - Fixed `mdbx::txn` being silently constructed from an owning `mdbx::txn_managed`: `mdbx::txn t = env.start_write();` sliced the managed temporary, dropping ownership so the write transaction (and the environment's writer lock) was never released — every later writer blocked or failed with `MDBX_BUSY`; the derived-to-base value conversions are now deleted, turning the slice into a compile error while the base handle stays a copyable non-owning view ([#77](https://github.com/Mithril-mine/libmdbx-devel/issues/77)) (backport).
    - Fixed C++ `cursor::erase()` binding string literals to the `bool` overloads: a literal converts to `bool` by a standard conversion, so `c.erase("key")` deleted all values at the cursor position and `c.erase(key, "value")` deleted every value of the key, silently losing data; string-literal overloads now win and forward to the `slice` forms, erasing exactly the named key or pair ([#90](https://github.com/Mithril-mine/libmdbx-devel/issues/90), [#120](https://github.com/Mithril-mine/libmdbx-devel/issues/120)) (backport).
    - Fixed C++ `buffer::append_hex()`/`append_base58()`/`append_base64()` reading freed storage when the source was the buffer itself and the storage had to grow, and the move-assignment reading a freed self-referencing source; the producers now run into a temporary before the storage is relocated, and a self-referencing source is copied through `assign_freestanding()` before the release. Follow-up for #122: that copy no longer reallocates inside a `noexcept` move-assign (the bytes are moved within the existing storage instead), and `append()`/`add_header()` also copy a source lying in the own headroom or tailroom into a temporary before growing ([#94](https://github.com/Mithril-mine/libmdbx-devel/issues/94), [#95](https://github.com/Mithril-mine/libmdbx-devel/issues/95), [#122](https://github.com/Mithril-mine/libmdbx-devel/issues/122)) (backport).
    - Fixed the same non-owning slicing as `txn` in the C++ `cursor` and `env`: initializing the base handle from an owning `cursor_managed`/`env_managed` temporary silently leaked the cursor or kept the environment open; such moves are now compile errors while a non-owning view of a living lvalue is still allowed. Also, `exception(const mdbx::error &)` is no longer `noexcept` (it allocates the message), and an inplace `buffer` emptied to its very storage end stays inplace instead of degrading to a dangling reference ([#121](https://github.com/Mithril-mine/libmdbx-devel/issues/121)) (backport).
    - Fixed C++ wrappers reporting non-error results as a `std::logic_error`: committing a transaction that had an error (e.g. after `make_broken()`) now throws `MDBX_BAD_TXN` instead of an unrelated logic-error, `txn::get_tree_deepmask()` of a plain table reports mask 0 instead of throwing, and `txn::sequence()` on overflow throws `std::overflow_error` with a message ([#102](https://github.com/Mithril-mine/libmdbx-devel/issues/102)) (backport).
    - Fixed C++ overload-resolution traps that compiled silently and did the wrong thing: a non-const `slice` in a boolean context always looked truthy, `cursor::move(op, key, value)` bound a plain value to the throw flag, an out-parameter move accepted an owning buffer and scribbled a map view into it, `buffer(p, n)`/`assign(p, n)` with an integer length ignored the length and ran `strlen` via the `const char*, bool` overload, and `buffer::hex/base58/base64()` on a `std::string_view`/`MDBX_val` encoded the object bytes instead of the data ([#101](https://github.com/Mithril-mine/libmdbx-devel/issues/101)) (backport).
    - Fixed C++ API build failures in documented configurations: the experimental-filesystem selector tested a misspelled macro name (so choosing it broke the build or silently fell back), a `buffer` "stealing" constructor from an rvalue string never compiled and shadowed the copying one, and `buffer::append(std::string_view)` and `slice(std::span)` called nonexistent iterator-pair overloads ([#100](https://github.com/Mithril-mine/libmdbx-devel/issues/100)) (backport).
    - Fixed several C++ `buffer` defects: `reserve(size)` could shrink the capacity and move the data when spare room existed, the inplace reshape tripped a checked-build assertion on an ordinary reserve by miscomputing the old headroom, constructing a buffer from a transaction and a slice rejected user-allocated memory and empty slices (the copy/reference decision now uses `mdbx_is_dirty()` directly), and `txn::extract()`/`replace()`/`replace_reserve()` dropped the caller's allocator and double-copied the result ([#99](https://github.com/Mithril-mine/libmdbx-devel/issues/99)) (backport).
    - Fixed the C++ `get_equal_or_great()` reporting an existing key as "not exact" on a multimap when an empty value was requested, and `fullscan()`/`scan_until()`/`scan_until_from()` throwing `not_found` on an empty table instead of returning false; end-of-data is now reported as a falsy result. Also fixed `mdbx_cursor_get_batch()` rejecting a limit of 2 or 3 although the copy loop needs only two slots ([#98](https://github.com/Mithril-mine/libmdbx-devel/issues/98)) (backport).
    - Fixed the C++ API committing or aborting a parent write transaction while a nested child transaction was still alive, which freed the child so its destructor later aborted an already-reused (unrelated) transaction, losing its changes; parent `commit()`/`abort()` now fail with `MDBX_BAD_TXN` while a child is live, keeping the parent usable ([#92](https://github.com/Mithril-mine/libmdbx-devel/issues/92)) (backport).
    - The C++ `mdbx::env::mode`/`mdbx::env::durability` enums became scoped (`enum class`); the former `env::`-level enumerator names are kept as backward-compatible static constants (e.g. `mdbx::env::nested_transactions`), so existing code keeps compiling and migrates smoothly to the scoped form (backport).

 - Build:
    - Fixed MSVC+AddressSanitizer builds failing to compile ([#62](https://github.com/Mithril-mine/libmdbx-devel/issues/62)) (backport).
    - Fixed linking with compiler-rt for `__builtin_cpu_supports()` by Clang (backport).
    - Conan recipe: added the `mdbx.lto` and `mdbx.sanitize` options and documented the package binary identity rules (backport).

 - Leaks:
    - Fixed the memory leak caused by a table name with zero length (backport).

 - Minors:
    - Clarification of the README and doxygen API descriptions (backport).
    - Added the `MDBX_PATHNAME_T()` macro expanding a path literal to a wide string on Windows like the `_T()` idiom, without changing the API ([#200](https://github.com/Mithril-mine/libmdbx-devel/issues/200)) (backport).
    - Added the dedicated `MDBX_DBG_ERR` logging level for `mdbx_setup_debug()`, so error messages can be shown even when debug output is quieted (backport).
    - etc...

--------------------------------------------------------------------------------

## v0.14.4 "Skynet Eve" (Накануне Skynet) at 2026-09-18

The supporting release of a stable branch with bug fixes.
Recommended for use in production.

### Important:

 The project website [libmdbx.dqdkfa.ru](https://libmdbx.dqdkfa.ru/) has been re-created:
   - The advice, explanations, questions and answers, from correspondence, forums and other sources have been compiled.
   - All information has been aggregated into a knowledge base, and the corresponding sections have been compiled on the website.
   - The documentation generated via Doxygen has been moved to /doxygen, with old links redirection.
   - A lot of work has been done using AI.

### Appreciations:

 - [Andrea Lanfranchi](https://github.com/AndreaLanfranchi) for bug reporting and assistance with fixing ones.

### Backward compatibility breaks:

 - On Windows now stricter database placement checks on network, shared, and layered volumes to prevent database corruption and data loss.
   This can lead to a `MDBX_EREMOTE` error (`ERROR_REMOTE_STORAGE_MEDIA_ERROR`) return in dangerous situations when WOF is active (on compressed volumes).
   Just don't use Windows, let alone WOF, to avoid data loss, or open databases exclusively for reading only (`MDBX_EXCLUSIVE | MDBX_RDONLY`).

### Improvements:

 - The "crash" meaning clarified in the documentation.
 - Prefer to use non-portable `fallocate()` to avoid emulation in glibc.
 - More CI workflows: CLANG-ARM64 on Windows, MinGW both 32- and 64-bit bitness, etc.
 - Refined `mdbx_get_sysraminfo()` internals for better `avail_pages` estimation.
 - Clarified `mdbx_env_defrag()` to eliminate doubts about the non‑impact of `result=nullptr` on the returned code.

### Fixes:

 - Fixed `entry point _except_handler4 not found in mdbx.dll` in 32-bit build with `MDBX_WITHOUT_MSVC_CRT=ON`.
 - Fixed building by CLANG/LLVM on Windows, including Microsoft Clang from Visual Studio.
 - Fixed `scan4seq_neon()` for LLP64/ARM64-Windows.
 - Fixed building for Win32 with `MDBX_NATIVE_SEH=OFF` without MASM, including cross-compilation cases.
 - Fixed division truncation/underflow in the `default_rp_augment_limit()` which led to too low and "stepwise" default `MDBX_opt_rp_augment_limit`.
 - Fixed regression leading to returning `MDBX_BAD_TXN` instead of `MDBX_OUSTED` in particular cases.
 - Fixed doxygen mis-reference to the `mdbx_set_panic()`.
 - Fixed missing sub-page header check inside `page_check()` which could cause `SIGSEGV` when checking a corrupted database.

### Misc:
 - The title of the "MAC chat" link has been changed and the icon has been removed.

--------------------------------------------------------------------------------

## v0.14.3 "Китов" (Kitov) at 2026-08-09

The supporting release with a lot of bug fixes,
in memory of [Anatoly Kitov](https://en.wikipedia.org/wiki/Anatoly_Kitov), a pioneer of cybernetics and the one of inventors of M-100, which was a most powerful computer of the World in 1958.

### Important:

 - Starting from v0.14.3, the 0.14.x branch gets stable status and will only receive bug fixes, but other improvements only in exceptional cases.
   Further development will be continued under the 0.15.x branch.

### Appreciations:

 - [Andrea Lanfranchi](https://github.com/AndreaLanfranchi) for bugs reporting.
 - [Cosmin Apreutesei](https://github.com/capr) for bugs reporting.
 - [stslam](https://github.com/stslam) for Embarcadero C++ Builder support.
 - [Yi Chen](https://github.com/94xhn) for bugs fixing.
 - [cui](https://github.com/cuiweixie) for discovery and fixing bugs.

### Backward compatibility breaks:

 - Now API functions that do not receive a transaction in arguments, but require a write lock, always check that the current thread owns (launched) the writing transaction.
   If the current thread does NOT own the writing transaction (did not start it), then the write-transaction lock will be acquired.
   This can lead to deadlock in `MDBX_NOSTICKYTHREADS` mode!

   These functions include:
    - `mdbx_env_set_flags()`, `mdbx_env_set_option()`;
    - `mdbx_env_set_geometry()`;
    - `mdbx_env_sync_ex()`, `mdbx_env_sync()`, `mdbx_env_sync_poll()`;
    - `mdbx_env_stat()`, `mdbx_env_stat_ex()` when called with `txn=nullptr`;
    - `mdbx_env_defrag()`, `mdbx_env_close_ex()`, `mdbx_env_close()`;

   Deadlock in `MDBX_NOSTICKYTHREADS` mode can only occur when a writing transaction is started in one thread, and a synchronous call to one of the listed above functions is performed in the other, in a context that prevents the completion of a running writing transaction:
    - a writing transaction has been started, and an application logic of its completion is waiting for some work to be done by other threads;
    - one of these threads, which must be done before completing the writing transaction, calls one of the specified functions;
    - deadlock: such a thread will wait for the completion of the transaction, and the thread will wait on the lock, which will be released upon completion of the transaction.

 - Some deprecated enums and defines were removed from API.

 - On Windows platform the Windows-10 API is now used by default.
   Previous versions are still supported, but now they should be explicitly requested during library build by defining `_WIN32_WINNT`.

 - Restructured project directories, renamed `ut_and_examples` into `examples`, etc.

 - The `mdbx_replace_ex()` now returns only previous data from a DB, but never new ones, even if it has the same value.

### Improvements:

 - Deferred invalidation of the dbi-handles of dropped tables has been implemented until the corresponding transactions are committed.

   Previously, libmdbx implemented the behavior historically inherited from LMDB, when handles of dropped tables were immediately closed, regardless of the possible subsequent abortion of such transactions.
   Now, when tables are dropped, both their associated handles and data remain available for other transactions running in parallel within the current process.

   This improvement has been requested for a long time, but it required a lot of preparation and refactoring which are done step-by-step during a few last releases.

 - Embarcadero C++ Builder now could be used to build libmdbx on Windows.

 - Allowed to use cursors bonded to the same table/DBI, but to different read-only transactions, in an API with multiple cursors in the parameters.

 - Added the missing recipe for Conan to an amalgamated source code.

 - Added check-and-retry of presync-to-disk condition to avoid latency spikes in commit path during asynchronous calls of `mdbx_env_sync()`, `mdbx_env_sync_ex()`, `mdbx_env_sync_poll()`.

 - Added the `MDBX_opt_presync_threshold` option.

 - In the C++ API the move assignment operator of the `mdbx::buffer<>` template now supports the case of unequal allocators by copying the contents of a source.

 - Added adjustment of the maximum size of the database and memory mapping in the modes of using Valgrind or AddressSanitizer, which greatly simplifies the use of these tools.

 - For Windows, the `MDBX_WITHOUT_MSVC_CRT=ON` build mode has been significantly improved using ntdll functions to eliminate dependence on MSVC CRT.
   Among other things, now there is a replacement for the __try/__except/__finally operators, support for Structural Exception Handling in the `SAFESEH` mode, simple substitution of _load_config_used, etc.

 - Provided CI on SourceCraft and GitHub.

 - Refined handling `MDBX_BUILD_OPTIONS` in the `GNUmakefile` to avoid redefinition/overriding.

 - On Windows provided `mdbx_env_deleteA()` and define `mdbx_env_deleteT()` depend on the `UNICODE`.

 - Added `/experimental:c11atomics` workaround for MSVC compiler weakness to enable C11 atomics.

### Fixes:

 - Fixed the [issue](https://github.com/Mithril-mine/libmdbx/issues/361) of losing a table content after abortion the nested transaction where such table was dropped.

 - Fixed `ERROR_LOCK_VIOLATION` during defrag on Windows in operation modes using overlapped I/O.

 - Fixed the regression/bug in the copy-without-compaction code, which led to producing a copy without a payload.

 - Fixed `env_owned_wrtxn()` to avoid by-pass locking in the `MDBX_NOSTICKYTHREADS` mode.

 - Fixed unreasonably high 2GB memory consumption in `mdbx_load` utility due to leftover debug changes.

 - Fixed loss of `mincore()` cache due erase/overwrite on insert.

 - Fixed a lot of typos and a few bugs detected by CodeQL.

 - Fixed losing of global init and thread-local-storage destructors in static library build by MinGW toolchain in particular cases.

 - Rare or specific conditions:
    - Fixed major typo in condition inside `latch_maindb_locked()`.
      However, despite the severity of the error, the scenario of its manifestation could not be found due to a combination of other checks in the code.
    - Fixed possibility of infinite loop inside `mdbx_txn_abort()` because of `memcmp()`/`memcpy()` typo.
    - Fixed potential buffer over-read by `fgets()` in `mdbx_load` utility.
    - Fixed missing `return` statement in one of the error paths inside `mdbx_cursor_bind()`.
    - Fixed extra rdt-unlock in the failure path of `dxb_resize()`.
    - Fixed NULL dereference in `walk_pgno()` when operating on a corrupted DB, which also affects `mdbx_chk` utility.

 - Resource leaks:
    - Fixed `mach_port_t` leak inside `mdbx_get_sysraminfo()`.
    - Fixed Windows section handle leak inside `osal_mresize()` in unsuccessful case.
    - Fixed a leak of the table name in the failure path of `dbi_open_locked()` in specific cases.
    - Fixed minor leaks/non-cleanup when `defrag_init()` failed.
    - Fixed `cond_pair` leak in the failure path of `copy_with_compacting()`.

 - Spilling:
    - Fixed spurious assertion inside `spill_cursor_keep()`.
    - Fixed committing a pure nested transaction that has spilled pages.
    - Fixed a case when a prepared GC-slots are spilled-out during `gc_update()`.
    - Fixed a leak of spilled pages list on a nested transaction abort.
    - Reworked internal cursors cloning to be compatible with subsequent pages tracking before spilling.
    - Introduced the internal `TXN_NIPPED` flag to suspend spilling during GC processing.
    - Fixed tracking and invalidation of the inner part of the sibling cursors.
    - Reworked cursor's stack with introducing the `stash` of pages.
    - Fixed spilling/accounting for `MDBX_AVOID_MSYNC=ON`.

 - C++ API:
    - Fixed ODR violation warnings from modern GCC while both LTO and UBSAN are enabled.
    - Fixed UTF-8 U+100000..U+10FFFF range checking/decoding inside `mdbx::slice::is_printable()`.
    - Fixed missing headroom reservation in several `mdbx::buffer<>` methods.
    - Fixed missing `enable_validation(flags & MDBX_VALIDATION)` inside `mdbx::env::operate_options::operate_options()`.
    - Fixed off-by-one bugs in the `mdbx::from_base64` and `mdbx::slice::is_printable()`.
    - Fixed possibility of overflow in `mdbx::slice::safe_middle()`.
    - Fixed UB in case empty array passed to `mdbx::cursor::distribute()`.
    - Fixed `enable_validation` in the `std::ostream &operator<<(::std::ostream &, const env::operate_options &)`.
    - Added missing `mdbx::cursor::estimate(move_operation operation, const slice &key)`, `mdbx::env::extra_runtime_option::presync_threshold` and `mdbx::env::geometry::&operator=()`.
    - Fixed using wide character count instead of bytes in `mdbx::slice` and `mdbx::buffer<>` methods.
    - Fixed `mdbx::buffer::reserve()` and `mdbx::buffer::assign()` for cases when buffer hold a reference to external data.
    - Fixed `mdbx::txn::extract()`, `mdbx::txn::replace()` and `mdbx::txn::replace_reserve()`.
    - Re-enabled `mdbx::buffer(std::basic_string<>, ...)` constructors.

 - Minors:
    - Fixed assertions triggering in specific scenarios of creating and renaming tables within nested transactions.
    - Fixed handling returned intermediate error codes in `meta_wipe_steady()`.
    - Fixed UBSAN issue inside thread-local storage destructor callback when DB opened in a without-lck (exclusive read-only) mode.
    - Fixed the exit status of `mdbx_load` for specific error cases.
    - Fixed running `ctest -T memcheck` by adding workaround of CTest/CMake bugs for Valgrind parameters.
    - Fixed/removed leftover usage of floating point in `mdbx_stat` utility.
    - Fixed `mdbx_defrag` for `-f` option handling.
    - Fixed MSVC warnings of implicit narrow type-casting.
    - Added compile-time guard for C11 atomics to avoid wrong code generation on non-x86 platforms by MSVC.
    - etc...

--------------------------------------------------------------------------------

## v0.14.2 "Буревестник" (stormy petrel, aka Bourevestnik) at 2026-05-14

The forward-looking release with new major features and internal refactoring.

### Important:

 - Due to numerous user requests, this ChangeLog will be kept in English.
   However, it should be noted that it was originally provided in Russian and then translated by AI during the formation of the release, because of this, there may be flaws in the text.

 - Since 2026 _libmdbx_ project has changed its code development and distribution model.
   **To get acquainted with important changes and plans, we recommend reading the compact [presentation "libmdbx: successes, obstacles, goals and roadmap"](https://libmdbx.dqdkfa.ru/release/libmdbx-roadmap-HNY2026-english.pdf), which contains important explanations in the form of embedded comments.**

 - The upstream of _libmdbx_ project has been relocated to the jurisdiction of the Russian Federation.
   We are confident that this will protect the project from any sanctions and ensure its accessibility to all users around the world.
   Please use https://libmdbx.dqdkfa.ru for documentation and https://sourcecraft.dev/dqdkfa/libmdbx for the source code.
   Nonetheless _libmdbx_ is still open source and provided with first-class free support.

### Appreciations:

 - [Erigon](https://erigon.tech/) for sponsorship.
 - The "AntiPublic" project for sponsorship.
 - [Artyom Vorotnikov](https://github.com/vorot93) for [Rust bindings](https://github.com/vorot93/libmdbx-rs), reporting bugs and testing.
 - [Stefan de Konink](https://github.com/skinkie) for [Python bindings](https://github.com/wtdcode/mdbx-py) and documentation improvement.
 - [Cosmin Apreutesei](https://github.com/capr) for error reporting and testing.
 - [Chloe Cano](https://github.com/Segwaz) for fuzzing, bug reporting and fixes.
 - [Weixie Cui](https://github.com/cuiweixie) for bug fixing through many pull-requests.
 - [Alexander Kelchin](https://serebrium.ru) (the "Serebrium" Company) for error messages and prototypes of exploits.
 - [Anton Maisak](https://public.git.amsoft.spb.ru/libmdbx/libmdbx-dotnet) for new .NET bindings.

### Backward compatibility breaks:

 - The typedefs of the various callbacks are now unified and includes an asterisk of indirection in `C` syntax notation.
   Perhaps this is the most annoying change that breaks the builds and requires changing your code. However, it is necessary to restore order.
   In most cases, the required changes are limited to removing the `*` chars after a callback type(s).

 - The size and composition of the `MDBX_envinfo` structure has been changed, and the `mdbx_env_info_ex()` function no longer supports the old versions. This breaks the compatibility of the ABI with older versions of the library, but preserves API compatibility at the source code level.

 - The template `mdbx::buffer<ALLOCATOR, POLICY>` is now inherited from `mdbx::slice` and `mdbx::buffer_tag`, which simplified the C++ API and the use of the meta-programming approach.

 - When building using GNU Make and CMake now, instead of a single `config.h`, different `config-gnumake.h` and `config-cmake.h` files are generated.

 - It is forbidden to open existing tables with different flags, unless the `MDBX_DB_ACCEDE` option is explicitly set.

 - The dumps generated by the `mdbx_dump` utility no longer output the current size of the database and `maxreaders`, so that the contents of the dump depend only on the contents of the database.

 - The build option `MDBX_FORCE_ASSERTIONS` has been deprecated, and `MDBX_CHECKING` (within range of `-1`..`3`) should be used instead.

 - The `NDEBUG` macro, which is generally accepted in C, no longer affects assert checks inside the library, but retains its traditional influence on assert checks related to argument control in the inline methods of the `C++` API.

### New features:

 - Implemented "Early GC Cleanup".

   Now the recycled GC records are deleted not when the writing transaction is committed, but as soon as possible. This opens the way to the implementation of explicit defragmentation (without copying the database) and further to non-sequential GC processing (which will eliminate the problem of DB swelling/overflow due to GC processing stopping during long-term reading transactions).

   The amount of overhead is now proportional to the volume of operations performed. Therefore, in most scenarios, the overhead is slightly less, but on the contrary, a little more when canceling nested transactions.

 - Database defragmentation/compaction support has been implemented and the `mdbx_defrag` utility has been added with a set of command-line options that allow you to define key parameters and defragmentation limits.

 - API Extension:

    - the `MDBX_CP_OVERWRITE` option has been added to the database copy function (overwriting the target file), and the `mdbx_copy` utility has a similar command-line option `-f`.
    - added the functions `mdbx_cursor_bunch_delete()` and `mdbx_cursor_delete_range()`, which perform massive deletion of adjacent elements much faster by excluding pages and branches with deleted items from a B-tree entirely.
    - added data retrieval functions with "caching" the `mdbx_cache_get()` and `mdbx_cache_get_SingleThreaded()`.
    - added the `mdbx_txn_refresh()` function to quickly refresh the reading transaction.
    - added the `mdbx_txn_checkpoint()` function to commit write transaction without releasing locks.
    - added the `mdbx_txn_commit_embark_read()` function to commit a writing transaction and start a reading one without interfering with other changes.
    - added the `mdbx_txn_amend()` function to change data starting from a snapshot of the data used in a given read transaction.
    - added the `mdbx_txn_rollback()` function to abort and restart a transaction with the cancellation of all changes, but without releasing locks.
    - added support for cloning reading transactions using `mdbx_txn_clone()`.
    - added support for nested read-only transactions.
    - added the `mdbx_gc_info()` function to get information about GC, page usage, and the ability to iterate GC content.
    - added the `mdbx_env_defrag()` function for explicit DB defragmentation, as well as the `mdbx_defrag` utility.
    - added the `MDBX_opt_split_reserve` option to control the fullness of tree pages when splitting them.
    - added the functions `mdbx_cursor_distance()`, `mdbx_cursor_scroll()` and `mdbx_cursor_distribute()` to simplify multithreaded parallel scanning.

 - Support for Harmony OS (OHOS) and Haiku OS.

 - Floating-point operations are no longer used both inside the library and in utilities, and linking to `libm` has been removed from build scripts.

 - It is possible to set debugging options `MDBX_DBG_ASSERT`, `MDBX_DBG_AUDIT` and others through environment variables. However, the corresponding debugging capabilities still need to be activated during the build.

 - Expanded and redesigned the composition of information generated by the function `mdbx_chk_env()` and the output utility `mdbx_chk`.

 - The main libmdbx repository has been migrated from GitFlic to SourceCraft.

   For my part, I am saddened by the need to perform such manipulations, because they create significant inconvenience to users, but (unfortunately) there are sufficient reasons for this:
    - Instead of the promised internationalization, GitFlic has only a Russian-language localization with a lot of technical features that make it difficult to use machine translation systems. This made it impossible for many users to use the service and generated a number of legitimate complaints/reproaches, including from developers from China, Brazil, Korea, Iran, etc.
    - In the three years since migrating to GitFlic, several outrageous errors in the markdown editor have not been fixed, which turned the design of releases into an annoying struggle. In addition, the development roadmap has disappeared from the public space. In total, this forced me to abandon GitFlic.

 - The number of page receiving/loading operations has been added to the transaction statistics, which makes it possible to quantify the amount of work with cursors and the effectiveness of various indexing and data retrieval approaches.
   The collection of relevant statistics is controlled by the additional build option `MDBX_ENABLE_PGET_STAT`.

 - The command-line options `-b number`, `-L megabytes`, `-d percent` and `-G geometry` have been added to the `mdbx_load` utility, allowing you to set the size of batch inserts, limit the volume of transactions, set the desired page filling density and redefine the geometry of the database when loading data from a dump.

 - Search was accelerated by using a branchless algorithm and embedding code of built-in/default comparators.

 - Redesigned internal verification statements and related build options.
   At the same time, `NDEBUG` no longer affects checks in the main engine code, which eliminates the causes of unexpected performance drops due to the lack of a definition of `NDEBUG` in non-debugging builds of users.

   The checks are divided into three categories (cheap, medium, expensive), controlled by the build option `MDBX_CHECKING`, which takes values from `-1` to `3` inclusive.
   The value of `3` corresponds to the maximum number of checks, and `-1` disables both all `assert()` and `ENSURE()` checks.
   By default, `MDBX_CHECKING` is assumed to be equal to the `MDBX_DEBUG` option, which in turn defaults to `0`, which corresponds to a regular (non-debugging) library build.
   This way, compatibility with the previous behavior is maintained and at the same time precise control of debugging checks is ensured.

### Behavior change:

 - Re-enabled/enabled on older Linux kernels, starting with version 3.16, since now there is no reason to stop working on 3.16 while supporting 4.x kernels, and there are still projects (Isar, Isar-Community, Hive) that require such support.

 - The default value of the page merge threshold has been changed from 25% to 33%.

 - To reduce the likelihood of unexpected errors due to transients and delayed processing in the OS kernel during competitive closing and opening of databases by different processes, the number of repeated attempts to capture locks has been tripled. Presumably, this will also solve the problem of unexpected `EAGAIN` (11) errors on Android when restarting applications and opening the database immediately after closing.

 - By default, Windows builds are now performed using the Windows 10 SDK, rather than Windows 7.

 - The error `MDBX_WANNA_RECOVERY` when opening the database in read-only mode is now returned if the database size is not a multiple of the system page size, but not a multiple of the size of the virtual memory allocation block is ignored. This eliminates the regression that occurred due to a change in behavior after using the system call `fallocate()` to prevent `SIGBUS` after incrementing the database file in a populated file system.

### Other improvements:

 - The logic of not using OFD locks on POSIX platforms has been finalized.
   Now, in addition to `EINVAL`, additional error codes are taken into account (`ENOSYS`, `ENOIMPL`, `ENOTSUP`, `ENOSUPP`, `EOPNOTSUPP`), which will allow the compiled library to work in some cases when the current kernel/container/emulator does not support the required system calls.

 - The tests are supplemented with scenarios to check the added features, identified regressions and errors.

 - Support for the `--numa#` option has been added to the test framework to link a stochastic test to a NUMA node, and explicit distribution across NUMA nodes has been added to the battery/tmux script, which significantly increased efficiency when testing on NUMA machines.

 - The stochastic script implements a random order of running individual tests.

 - The output of a histogram of filling pages forming the tree structure and participating in split/merge/rebalance operations has been added to the database integrity verification functionality and the mdbx_chk utility.

 - A workaround has been added for Android to reduce the likelihood of an `EAGAIN` system error due to a lack of system resources and transients when closing and quickly re-opening the database.

 - For Linux, added error prevention in the fast_commit implementation of the Ext4 file system.

 - Support for the "Skip" and "Repeat" options has been added to debugging builds on Windows when assert checks are triggered.

 - File locks used on the Windows platform involve waiting with timeouts, which theoretically should reduce the likelihood of `ERROR_LOCK_VIOLATION` (`33`) errors when opening a database in competitive scenarios.

 - Redesign of the buffer implementation and other improvements in the C++ API.

 - The tests fixed several minor memory leaks and the UBSAN warning about the `memcmp(, length = 0)` call.

### Fixes:

 - Almost all of these fixes were included in previous releases of the stable branch version 0.13.x. Therefore, one should not assume that these fixes are related specifically to release 0.14.2.

 - Fixed a critical error in the `mdbx_env_resurrect_after_fork()` functionality when using SysV semaphores.

   The error appeared only after the child process was generated by `fork()` against the background of an ongoing writing transaction, which led to incorrect operation of semaphores and further to a variety of errors, up to database corruption.
   The problem has existed since the appearance of `mdbx_env_resurrect_after_fork()` and affected OSX as well as POSIX platforms when building with the option `MDBX_LOCKING=5`.

 - Fixed a problem in the database copying API on POSIX systems other than Linux, as well as in some cases when the target file is located on a non-local file system.
   The problem manifested itself mainly on OSX, with the return of the error `EWOULDBLOCK`/`EAGAIN` (35), due to a flaw/conflict between the locks `fcntl(F_SETLK)` and `flock()` in the OS kernel.
   The error handling of file lock capture in the copy API on POSIX systems has been redesigned.

 - Fixed an error that led to an unexpected return of `MDBX_BAD_DBI` when multiple transactions were started simultaneously within the same process after opening the database.

 - Fixed an error that caused the unexpected return of `MDBX_DBS_FULL` when reopening already open tables and the limit of open DBI descriptors has already been reached.

 - Fixed an assembly error for the Android platform when explicitly defining `_FILE_OFFSET_BITS`.

 - Fixed or deleted several incorrect assert checks, which caused debugging builds to crash in specific situations.
   Mainly in the code of the functions `txn_end()`, `txn_lock()` and `txn_unlock()` on both Windows and POSIX.

 - Minor MSVC warnings have been eliminated. Warnings `C5286` and `C5287` are disabled.

 - Fixed getting unexpected `SIGBUS` due to delayed/lazy allocation of space in a populated file system after incrementing a DB file.
   A more detailed explanation is provided in the commit comment. [`2a7f460345edbeb26a51782cbe6af3c55254ae77`](https://gitflic.ru/project/erthink/libmdbx/commit/2a7f460345edbeb26a51782cbe6af3c55254ae77).

 - Fixed an assert check in the scan path of the DBI-descriptor bitmap, which led to rare crashes of 32-bit debug builds.

 - Redesigned the search for utilities `lib.exe` and `dlltool.exe` when building using CMake on Windows.

 - Fixed crashes when executing Thread-Local-Storage constructors when unloading the library and when there are env instances whose initialization has not been completed.

 - On Windows, the reason for the return of the unexpected error `ERROR_IO_PENDING` in scenarios of multiple opening of one database in one process has been fixed.

 - Fixed an issue with auto-tuning parameters when setting geometry with a specified minimum page size, which could cause the page size to increase on machines with a large amount of RAM.

 - In CMake scripts, regression was eliminated, due to which the `ctest` infrastructure did not use the set Valgrind parameters, including `MEMORYCHECK_SUPPRESSIONS_FILE`.
   Now using `ctest -D ExperimentalMemCheck` does not result in multiple false-positive diagnoses. However, to use Valgrind, you still need to build a library with the predefined macro `ENABLE_MEMCHECK`.

 - Fixed the use of the identifier `ERROR_UNHANDLED_ERROR`, which is not detected in new versions of the Windows SDK.

 - Fixed non-closing of DBI descriptors for tables created in nested transactions when such transactions were interrupted.

 - Eliminated unnecessary data synchronization operations with disk when allocating pages when the database is almost full.

 - The `mdbx_load` utility has fixed errors in loading zero-length values and exchanging shrink/growth parameters in the database geometry.

 - Fixed a `SIGSEGV` crash when all meta pages are not fully usable.

 - Fixed a typo in the condition for determining a change in the size of the database when rolling back a nested transaction.

 - Fixed information collection via `kstat()` for `bootid` on Solaris and related platforms.

 - Fixed a typo in the `ST_EXPORTED` processing path that broke the build on platforms where the mentioned flag is defined for `fstatvfs()`.

 - Fixed a `SIGSEGV` crash due to an attempt to clean/overwrite a corrupted meta page when opening the database in read-only mode.

--------------------------------------------------------------------------------

English version [by liar Google](https://libmdbx-dqdkfa-ru.translate.goog/md__change_log.html?_x_tr_sl=ru&_x_tr_tl=en) and [by Yandex](https://translated.turbopages.org/proxy_u/ru-en.en/https/libmdbx.dqdkfa.ru/md__change_log.html).

## v0.14.1 выпуск "Горналь" от 2025-05-05

Первый выпуск в новом кусте/линейке версий с добавлением функционала, расширением API и внутренними переработками.

Благодарности:

 - [Erigon](https://erigon.tech/) за спонсорство.
 - [Alain Picard](https://github.com/castortech) for support [Java bindings](https://github.com/castortech/mdbxjni) and MacOS universal binaries patch for CMake build scenario,
   also for bug reporting (put-`MDBX_MULTIPLE` regression). Big thanks for assistance with debugging and testing.
 - [Alex Sharov](https://github.com/AskAlexSharov) за сообщение об ошибках и тестирование.
 - [Виктору Логунову](https://t.me/vl_username) за сообщение об опечатке в имени переменной в Conan-рецепте.
 - [Илье Михееву](https://t.me/IlyaMkhv) за сообщение о лишнем/ненужном предупреждении несоответствия файла БД новому размеру.
 - [maxc0d3r](https://gitflic.ru/user/maxc0d3r) for bug reporting and testing.
 - [Алексею Костюку (aka Keller)](https://t.me/keller18306) за сообщения о проблеме копирования на NFS.

Новое:

 - Переработан код обновления GC и возврата страниц при фиксации транзакций.

   Возникающая при этом задача алгоритмически сложна, так как список
   возвращаемых страниц находится в рекурсивной зависимости от самой
   процедуры возврата и связанных с этим операций, а прямые решения во
   многих случаях приводят к многократному росту накладных расходов.
   Поэтому исторически эта часть кода была запутанным наслоением «сдержек и
   противовесов», что создавало препятствие для развития. В ходе этой
   доработки, унаследованный из LMDB код связанный с обновлением GC, был
   полностью заменен вместе со всеми базирующимися на нём заплатками.

   Новая реализация использует контейнеры идентификаторов (aka RKL),
   комбинирующие внутри списки элементов и непрерывные интервалы, что
   позволяет предельно сократить накладные расходы и упросить реализацию
   остальных алгоритмов. Основывается новая реализация на простом
   прагматичном подходе «резервирования со взвешенным запасом». Для
   подавляющего подмножества сценариев этого достаточно для однопроходного
   обновления GC, с общей сложностью от `O(1)` для мелких транзакций, до
   `O(log(N))` для огромных. При этом реализованный еще в 0.12.1 подход «Big
   Foot» (дробление больших списков retired-страниц) полностью избавляет GC
   от потребности в последовательностях смежных/соседствующих страниц и
   одновременно позволяет работать новому коду обновления GC только по
   самому простому и быстрому пути.

   Тем не менее, при намеренном отключении «Big Foot», либо при работе с БД
   от старых версий движка без «Big Foot», возможны сложные ситуации, когда
   в GC могут огромные списки страниц, которые желательно дробить при
   возвращении неиспользованных переработанных остатков. В таких сценариях
   для возврата в GC требуется создавать больше записей чем было исходно
   переработано, что может приводить к нехватке имеющихся/переработанных
   идентификаторов. Тогда в игру вступает следующая часть нового кода,
   поиск в GC «дыр» (неиспользуемых промежутков/интервалов в пространстве
   ключей GC). Далее, если свободных идентификаторов (неиспользуемого
   пространства ключей GC) будет недостаточно, что весьма вероятно в
   некоторых сценариях, будет решаться задача родственная «укладке
   рюкзака». В конечном итоге, неиспользованные переработанные страницы
   будут возвращены в GC, с максимально равномерным
   распределением/дроблением и использованием имеющихся последовательностей
   смежных/соседствующих страниц, что гарантирует близость к теоретическому
   минимуму суммарной стоимости текущих действий и последующих операций.

   На данный момент нет известных практических сценариев ведущих к
   отказу/неуспеху новой реализации обновления GC. Но гипотетически такие
   случаи возможны, как из-за ошибок/недочетов в реализации, так и из-за
   использования катастрофически неудачных режимов работы и значений опций
   (например `MDBX_opt_rp_augment_limit`). В текущем понимании, в том числе
   основываясь на объем тестирования, вероятность проявления
   ошибок/недочетов оценивается как крайне низкая, а устраняться замеченные
   проблемы будут по мере обнаружения. Однако, полностью автоматическое
   решение самых кошмарных и запутанных ситуаций с GC следует ожидать
   только при реализации дефрагментации — просто потому что нет иного
   рационального способа решения, за вычетом копирования БД с
   дефрагментацией.

 - Добавлена опция сборки `MDBX_NOSUCCESS_PURE_COMMIT` предназначенная для отладки кода пользователя.
   По-умолчанию опция выключена и при фиксации пустых транзакции возвращается `MDBX_SUCCESS`.
   При включении опции, фиксация пишущих транзакций без каких-либо изменений считается нештатным поведением, с возвратом из `mdbx_txn_commit()` кода `MDBX_RESULT_TRUE` вместо `MDBX_SUCCESS`.
   Таким образом, у пользователя появляется возможность легко диагностировать лишние/ненужные транзакции записи.

 - Добавлена опция сборки `MDBX_ENABLE_NON_READONLY_EXPORT` позволяющая использовать в режиме чтения-записи БД расположенных в файловых системах экспортированных через NFS.
   По-умолчанию опция выключена и при открытии в неэксклюзивном режиме чтения-записи БД расположенных в файловых системах доступных извне по NFS будет возвращаться ошибка `MDBX_EREMOTE`.
   Включение опции позволяет открывать БД в описанных выше ситуациях, но риск чтения неверных данных на удалённой стороне ложится на пользователя.

 - Поддержка MacOS universal binaries при сборке посредством CMake.

 - Для закрытия или отсоединения всех курсоров с получением их количества в API добавлена функция `mdbx_txn_release_all_cursors_ex()`.

 - Добавлена операция `MDBX_SEEK_AND_GET_MULTIPLE` в API курсора, позволяющая за одну операцию выполнить позиционирование
   курсора на конкретное значение и начать чтение multi-значений в пакетном режиме.

 - Добавлены методы `mdbx::cursor::put_multiple_samelength()`, `mdbx::cursor::seek_multiple_samelength()`, `mdbx::cursor_managed::withdraw_handle()`.

 - В политику управления выделением для `mdbx::buffer<ALLOCATOR, CAPACITY_POLICY>` добавлен параметр `inplace_storage_size_rounding`.
   Одновременно с этим переработан внутренний union-тип `mdbx::buffer<ALLOCATOR, CAPACITY_POLICY>::silo::bin` для возможности увеличения без пенальти встроенного в экземпляр буфера места под данные.

 - Добавлена опция `-c` (concise) для включения компактного режима в `mdbx_dump`, также поддержка таких дампов в `mdbx_load`.
   В таких дампах значение ключей сохраняются однократно (не повторяются), что может существенно уменьшать результирующий объём для таблиц с multi-значениями (aka dupsort).
   Однако, компактные дампы не совместимы с форматом ожидаемым/поддерживаемым в Berkeley Database и LMDB.

 - В API добавлена функция `mdbx_cursor_close2()` возвращающая код ошибки.

 - В chk-функционал добавлена гистограмма количества multi-значений/дубликатов.
   При использовании утилиты `mdbx_chk`, для получения соответствующей (и массы другой) информации, достаточно увеличить детализацию несколько раз использовав опцию `-v`.

Изменение поведения:

 - Теперь при вставке данных в dupsort-таблицу CoW копирование целевых страниц выполняется после проверки отсутствия добавляемого значения среди уже присутствующих multi-значений (aka дубликатов).
   В результате вставка уже присутствующих "дубликатов" не приводит к каким-либо изменениям в БД и принципиально увеличивает производительность в таких сценариях.
   В текущем понимании, добавленная проверка не приводит к заметному увеличению накладных расходов и, как следствие, не приводит к снижению производительности в сценариях с обычным/регулярным обновлением и/или вставкой данных.

 - Использование системного кода ошибки `EREMOTEIO` ("Remote I/O error") вместо `ENOTBLK` ("Block device required") в качестве `MDBX_EREMOTE` для индикации ошибочной ситуации открытия БД расположенной на сетевом носителе.

 - Функция `mdbx_txn_release_all_cursors()` возвращает только код ошибки, не смешивая его с количеством обработанных/закрытых курсоров.
   Для аналогичных действий с получением количества закрытых курсоров в API добавлена функция `mdbx_txn_release_all_cursors_ex()`.

 - Поддержка пустого набора данных в put-операции `MDBX_MULTIPLE` ради упрощения пользовательского кода, какой-либо модификации данных в БД при этом не происходит.

 - Для основных вариантов использования шаблона `mdbx::buffer<>` теперь явно инстанцируются внутри библиотеки,
   одновременно соответствующие специализации шаблона помечены как `external` для предотвращения повторного инстанцирования в пользовательском коде.

 - Запрещена отвязка/открепление курсоров во вложенных транзакциях, т.е. вызовы `mdbx_cursor_unbind()` и
   `mdbx_txn_release_all_cursors(unbind=true)` для курсоров открытых в одной из родительских транзакций.
   Причина в том, что в случае отмены вложенной транзакции возникает неконструктивная неопределенность  — следует ли
   восстанавливать состояние курсоров. Если не восстанавливать, то получается что вложенная транзакция может поломать родительскую,
   сделав её продолжение невозможным. Если восстанавливать, то также следует «воскрешать» закрытые курсоры,
   что неизбежно приведет к путанице, утечкам памяти и использованию после освобождения.

 - В C++ API отменён вброс исключения при запросе транзакции у отсоединённого курсора посредством вывоза `mdbx::cursor::txn()`.

 - При невозможности отвязки курсора от его текущей транзакции функция `mdbx_cursor_bind()`
   теперь возвращает `MDBX_EINVAL` вместо `MDBX_BAD_TXN`.

Исправления:

 - Для совместимости с GCC 15.x в режиме C23 изменен порядок указания атрибутов функций.

 - Устранён регресс допускающий SIGSEGV в операциях обновления после вытеснения/spilling страниц в больших транзакциях.
   Ошибка присутствует в выпусках v0.13.1, v0.13.2, v0.13.3 и оставалась незамеченной из-за специфических условий и низкой вероятности проявления.
   Более подробная информация в описании коммита `cb8eec6d11cdab4f7d3cf87913e8009149dcf60b`.

 - Устранено лишнее/ненужное предупреждение в сценарии изменения размера БД посредством вызова `mdbx_env_set_geometry()` до её открытия.
   API предусматривает возможность запросить изменение геометрии/размера БД перед её открытием, чтобы избежать как лишних накладных расходов,
   так и потенциальных ошибок из-за нехватки адресного пространства. В этом сценарии ранее могло выдаваться лишнее/ненужное предупреждение
   о несоответствии файла БД новому размеру. Теперь этот недостаток исправлен.

 - Восстановлена доступность дескрипторов таблиц, открытых в дочерней транзакции, после её фиксации, в случае отсутствия изменений в данных.
   Проблема не была замечена ранее из-за специфического сценария проявления.
   Ошибка присутствует в версиях 0.13.x и последующих, начиная с коммита `e6af7d7c53428ca2892bcbf7eec1c2acee06fd44` от 2023-11-05.

 - Устранён сбой аудита таблиц при инвалидации дескрипторов таблиц вследствие отмены вложенной транзакции.
   Проблема не была замечена ранее из-за специфического сценария проявления.
   Ошибка присутствует в версиях 0.13.x и последующих, начиная с коммита `e6af7d7c53428ca2892bcbf7eec1c2acee06fd44` от 2023-11-05.

 - Устранена причина потенциальных сбоев и/и деградации производительности в сценарии закрытия курсора до завершения вложенной транзакции,
   с последующим изменением данных той-же таблицы в текущей вложенной транзакции, либо её дочерних транзакциях.
   Проблема обнаружена при ручном анализе кода, сценарии воспроизведения/проявления проблемы пока не известны.
   Ошибка присутствует в версиях 0.13.x и последующих, начиная с коммита `3de3d425a128a3c6f7866503f5f93b80c09dbe41` от 2024-05-19.

 - Устранена причина ложных ошибок при работе `mdbx_chk` с высоким уровнем логирования.
   Проблема возникала из-за неверной трактовки `MDBX_NOTFOUND` при штатном окончании итерируемых данных.

 - Устранена причина попыток рекурсивного захвата мьютекса при работе `mdbx_chk -w` в сборах с поддержкой Valring/ASAN и под управлением этих инструментов.

 - Устранена вероятность ситуации гонки в `tbl_setup(MDBX_DUPFIXED | MDBX_INTEGERDUP)` при работе в разных потоках.
   В реальных сценариях вероятность проявления проблемы была близка к нулю.
   Для подробностей смотрите комментарий коммита `3e91500fac475947f5b58268d5edd3c9cc4f77f6`.

 - Устранён регресс затенения курсоров во вложенных транзакциях.
   При реализации отложенной/ленивой инициализации dbi-дескрипторов также было реализовано отложенное затенение курсоров (создание копии состояния для отката при прерывании транзакции),
   что существенно уменьшало накладные расходы при старте и завершении вложенных транзакций в сценариях с большим количеством курсоров.
   Однако, была допущена логическая ошибка, вследствие которой отложенная инициализация и затенение выполнялись при использовании dbi-дескрипторов, но не курсора открытого в родительской транзакции.
   В результате, родительские курсоры во вложенных транзакциях могли не затеняться, что приводило к неконсистентному состоянию в случае
   прерывания/откате вложенной транзакции и в соответствующей таблице были изменения в рамках прерванной вложенной транзакции.
   Проблема не реализовывалась в тестовых сценариях и не была замечена при эксплуатации, но была обнаружена при расширении тестов.
   Ошибка присутствует в версиях 0.13.x и последующих, начиная с коммита `e6af7d7c53428ca2892bcbf7eec1c2acee06fd44` от 2023-11-05.

 - Устранён регресс в пути обработки операции `MDBX_MULTIPLE`.
   Пакетная вставка значений посредством `MDBX_MULTIPLE` могла приводить к падениям и повреждению структуры БД. Ошибка оставалось не
   замеченной из-за специфических условий проявления, которые не реализовались в тестах.
   Проблема присутствовала во всех выпусках начиная с v0.13.1, но соответствующая ошибка не связана с конкретным коммита в истории, а
   является следствием нескольких доработок (шагов рефакторинга), которые суммарно привели к регрессу.
   Технически ошибка обусловлена не-обнулением переменной, чего не происходило в некотором пути выполнения, так как исходно не требовалось.
   Однако, такое обнуление потребовалось после ряда этапов оптимизации и рефакторинга смежных участков кода.
   Для подробностей смотрите комментарий коммита `23a417fe19614481c6546845995d6dc845baf797`.

 - Скорректировано описание ошибки `MDBX_MVCC_RETARDED` и текста соответствующего сообщения.

 - В C++ API добавлена упущенная проверка `__cpp_concepts >= 202002` для использования концептов C++.

 - Устранён регресс при использовании курсоров для DBI=0 в читающих транзакциях.

   После рефакторинга и ряда оптимизаций для завершения/гашения
   курсоров в читающих и пишущих транзакций стал использоваться общий код.
   Причем за основу, был взят соответствующий фрагмент относящийся к
   пишущим транзакциям, в которых пользователю не позволяется
   использоваться курсоры для DBI=0 и поэтому эта итерация пропускалась.

   В результате, при завершении читающих транзакций, курсоры связанные с
   DBI=0 не завершались должным образом, а при их повторном использовании
   или явном закрытии после завершения читающей транзакции происходило
   обращение к уже освобожденной памяти. Если же такие курсоры
   отсоединялись или закрывались до завершения читающей транзакции, то
   ошибка не имела шансов на проявление.

 - Устранён регресс в виде ошибки `EAGAIN` при копировании БД на NFS и CIFS/SMB.

   При доработках/развитии API в функции копирования был добавлен захват
   файловой блокировки посредством как `fcntl()`, так и `flock()`. Однако,
   в зависимости от версии локального ядра, версии удалённого сервера NFS и
   опций монтирования, это могло приводить к возврату POSIX-ошибки `EAGAIN`
   (`11` на большинстве платформ, включая Linux).

 - Устранена ошибка merge/rebase внутри `mdbx_txn_release_all_cursors_ex()`,
   что могло приводить к последующим неожиданным ошибкам `MDBX_EBADSIGN` и утечкам памяти.
   Для проверки сценария дополнен соответствующий тест.

 - Исправлена assert-проверка в пути завершения вложенных транзакций.
   Для проверки сценария дополнен соответствующий тест.

 - Устранена возможность возврата неожиданной ошибки `MDBX_BUSY` из `mdbx_txn_lock(dont_wait=false)`.

Прочие доработки:

 - Существенный рефакторинг с реструктуризацией кода, переименованием внутренних структур, их полей и внутренних функций.

 - Доработка использования LTO в CMake-сценариях: использование `-flto=auto` для GCC >= 11.4,
   расслабление условий для включения LTO для CLANG на Linux, расширение поиска `LLVMgold.so` в относительных lib-директориях.

 - Добавлены дополнительные проверки сигнатур курсоров при итерации связанных списков.

 - Кратное сокращение итераций тестов в зависимости от конфигурации Valgrind/Debug/CI.

 - Устранены предупреждения UBSAN о невыравненном доступе в тесте extra/close-dbi.

 - Добавлен перехват и логирование исключений в extra-тестах на C++.

 - Расширены тесты extra/dupfix-multiple, extra/cursor-closing и extra/txn.

 - В утилиту тестирования добавлена поддержка режима/опции `MDBX_VALIDATION` и поддержка значений `on`/`off` для опций командной строки.

 - Добавлены doxygen-описания для doubtless-positioning констант.

 - Переработана проверка курсоров на входе в API-функций с добавлением `cursor_check()`, `cursor_reset()` и `cursor_drown()`.

 - Отключено использование C23 `[[атрибутов]]` для версий CLANG меньше 20.

 - Во избежание потенциальных проблем отключено использование `copy_file_range()` на ядрах Linux 5.3 - 5.18.

 - Вброс `std::invalid_argument` теперь производится явным сообщением `MDBX_EINVAL`.

--------------------------------------------------------------------------------

## v0.14.0 от 2025-01-13

Технический тэг, отмечающий начало ветки `0.14`
с новым функционалом и изменением API.

Запланированные новые возможности 0.14:

1. Ранняя (не-отложенная) очистка GC и рефакторинг обновления GC. Самостоятельной видимой для пользователя ценности не имеет, но требуется для последующих пунктов.
   Будет реализовано в 0.14.1.

2. Явная дефрагментация БД. В API будет добавлена функция с двумя парами параметров:
    - минимальный (требуемый) объём дефрагментации (уменьшения БД) и минимальное время, которое следует потратить;
    - максимальный (ограничивающий) объём дефрагментации и максимальное время, которое допустимо потратить.

    Упрощенно, алгоритмически явная дефрагментация сводится к сканированию b-tree с формированием списка страниц расположенных близко к концу БД, а затем копирование этих страниц в не-используемые, но расположенные ближе к началу БД.
    В результате, после фиксации дефрагментирующей транзакции оригиналы скопированных страниц становятся не-используемыми, а размер БД может быть уменьшен за счет отсечения не-используемых страниц в конце используемого пространства.
    Будет реализовано в 0.14.2.

3. Нелинейная переработка GC, без остановки переработки мусора на старом MVCC-снимке используемом долгой транзакцией чтения.

    После реализации запланированного, любая длительная читающая транзакция по-прежнему будет удерживать от переработки используемый/читаемый MVCC-снимок данных (все образующие его страницы БД), но позволит перерабатывать все неиспользуемые MVCC-снимки, как до читаемого, так и после.
    Это позволит устранить [один из основных архитектурных недостатков](https://libmdbx.dqdkfa.ru/docs/en/deep-dive/architecture.html) унаследованных от LMDB и связанных с ростом размера БД пропорционально объёму производимых изменений данных на фоне долго работающей транзакции чтения.

    Будет реализовано предположительно в 0.14.3, 0.14.4 или даже в 0.15.x.
    Перенос в 0.15.x оправдан возможностью переноса функционала дефрагментации в stable-ветку, но посмотри как пойдут дела.

********************************************************************************

For early releases and changes see the ChangeLog-NN the git commit history.
