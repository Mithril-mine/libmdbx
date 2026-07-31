ChangeLog
=========

The source code is available on [SourceCraft](https://sourcecraft.dev/dqdkfa/libmdbx) and mirror on [GitHub](https://github.com/Mithril-mine/libmdbx).
Please use the `stable` branch or the latest release for production environment through staging, but the `master` branch for development a derivative projects.
Donations are welcome to ETH `0xD104d8f8B2dC312aaD74899F83EBf3EEBDC1EA3A`,
BTC `bc1qzvl9uegf2ea6cwlytnanrscyv8snwsvrc0xfsu`, SOL `FTCTgbHajoLVZGr8aEFWMzx3NDMyS5wXJgfeMTmJznRi`.
Всё будет хорошо!

## v0.14.3 is re-scheduled for 2026-08-08.

The supporting release of a stable branch with bug fixes.

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

 - Now API functions that do not receive a transaction in arguments, but require a write lock, always checks that the current thread owns (launched) the writing transaction.
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

 - The `mdbx_replace_ex()` now returns only previous data from a DB, but never new ones, even it has the same value.

### Improvements:

 - Deferred invalidation of the dbi-handles of dropped tables has been implemented until the corresponding transactions are committed.

   Previously, libmdbx implemented the behavior historically inherited from LMDB, when handles of a dropped tables were immediately closed, regardless of the possible subsequent abortion of such transactions.
   Now, when tables are dropped, both ones associated handles and data remain available for other transactions running in parallel within the current process.

   This improvement has been asking for a long time, but it required a lot of preparation and refactoring which are done step-by-step during a few last releases.

 - Embarcadero C++ Builder now could be used to build libmdbx on Windows.

 - Allowed to use cursors bonded to the same table/DBI, but to different read-only transaction, in an API with multiple cursors in the parameters.

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

### Fixes:

 - Fixed the [issue](https://github.com/Mithril-mine/libmdbx/issues/361) of losing a table content after abortion the nested transaction where such table was dropped.

 - Fixed `ERROR_LOCK_VIOLATION` during defrag on Windows in operation modes using overlapped I/O.

 - Fixed `env_owned_wrtxn()` to avoid by-pass locking in the `MDBX_NOSTICKYTHREADS` mode.

 - Fixed unreasonably high memory 2GB consumption in `mdbx_load` utility due to leftover debug changes.

 - Fixed loss of `mincore()` cache due erase/overwrite on insert.

 - Fixed a lot of typos and a few bugs detected by CodeQL.

 - Rare or specific conditions:
    - Fixed major typo in condition inside `latch_maindb_locked()`.
      However, despite the severity of the error, the scenario of its manifestation could not be found due to a combination of other checks in the code.
    - Fixed possibility of infinite loop inside `mdbx_txn_abort()` because of `memcmp()`/`memcpy()` typo.
    - Fixed potential buffer over-read by `fgets()` in `mdbx_load` utility.
    - Fixed missing `return` statement in one of the error paths inside `mdbx_cursor_bind()`.
    - Fixed extra rdt-unlock in the failure path of `dxb_resize()`.
    - Fixed NULL deference in `walk_pgno()` when operating on a corrupted DB, which also affects `mdbx_chk` utility.

 - Resource leaks:
    - Fixed `mach_port_t` leak inside `mdbx_get_sysraminfo()`.
    - Fixed Windows section handle leak inside `osal_mresize()` in unsuccessful case.
    - Fixed a leak of the table name in the failure path of `dbi_open_locked()` in a specific cases.
    - Fixed minor leaks/non-cleanup when `defrag_init()` failed.
    - Fixed `cond_pair` leak in the failure path of `copy_with_compacting()`.

 - Spilling:
    - Fixed spurious assertion inside `spill_cursor_keep()`.
    - Fixed committing a pure nested transaction has a spilled pages.
    - Fixed a case when a prepared GC-slots are spilled-out during `gc_update()`.
    - Fixed a leak of spilled pages list on a nested transaction abort.
    - Reworked internal cursors cloning to be compatible with subsequent pages tracking before spilling.
    - Introduced the internal `TXN_NIPPED` flag to suspend spilling during GC processing.
    - Fixed tracking and invalidation of the inner part of the sibling cursors inside `cursor_del()`.
    - Fixed sibling cursors tracking/invalidation in `cutoff_zikkurat()`.
    - Fixed cursors tracking in `node_move()`.

 - C++ API:
    - Fixed ODR violations warnings from modern GCC while both LTO and UBSAN are enabled.
    - Fixed UTF-8 U+100000..U+10FFFF range checking/decoding inside `mdbx::slice::is_printable()`.
    - Fixed missing headroom reservation in several `mdbx::buffer<>` methods.
    - Fixed missing `enable_validation(flags & MDBX_VALIDATION)` inside `mdbx::env::operate_options::operate_options()`.
    - Fixed off-by-one bugs in the `mdbx::from_base64` and `mdbx::slice::is_printable()`.
    - Fixed possibility of overflow in `mdbx::slice::safe_middle()`.
    - Fixed UB in case empty array passed to `mdbx::cursor::distribute()`.
    - Fixed `enable_validation` in the `std::ostream &operator<<(::std::ostream &, const env::operate_options &)`.
    - Added missing `mdbx::cursor::estimate(move_operation operation, const slice &key)`, `mdbx::env::extra_runtime_option::presync_threshold` and `mdbx::env::geometry::&operator=()`.
    - Fixed using wide characher count instead of bytes in `mdbx::slice` and `mdbx::buffer<>` methods.
    - Fixed `mdbx::buffer::reserve()` and `mdbx::buffer::assign()` for cases when buffer hold a reference to external data.
    - Fixed `mdbx::txn::extract()`, `mdbx::txn::replace()` and `mdbx::txn::replace_reserve()`.
    - Re-enabled `mdbx::buffer(std::basic_string<>, ...)` constructors.

 - Minors:
    - Fixed assertions triggering in specific scenarios of creating and renaming tables within nested transactions.
    - Fixed handling a returned intermediate error codes in `meta_wipe_steady()`.
    - Fixed UBSAN issue inside thread-local storage destructor callback when DB opened in a without-lck (exclusive read-only) mode.
    - Fixed the exit status of `mdbx_load` for specific error cases.
    - Fixed running `ctest -T memcheck` by adding workaround of CTest/CMake bugs for Valgrind parameters.
    - Fixed/removed leftover usage of float point in `mdbx_stat` utility.
    - Fixed `mdbx_defrag` for `-f` option handling.
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

 - Almost all of these fixes were included in previous releases of the stable branch version 0.13.x. Therefore, should not assume that these fixes are related specifically to release 0.14.2.

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
   also for bug reporting (put-`MDBX_MULTIPLE` regression). Big thank for assistance with debugging and testing.
 - [Alex Sharov](https://github.com/AskAlexSharov) за сообщение об ошибках и тестирование.
 - [Виктору Логунову](https://t.me/vl_username) за сообщение об опечатки в имени переменной в Conan-рецепте.
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

   Тем не менее, при намеренном отключении «Big Foot», либо при работы с БД
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
   прерывания/откате вложенной транзакции и в соответствующей таблицы были изменения в рамках прерванной вложенной транзакции.
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
   использоваться курсоры для DBI=0 и поэтому эта итераций пропускалась.

   В результате, при завершении читающих транзакциях, курсоры связанные с
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
    - максимальный (ограничивающий) объём дефрагментации и максимальной время, которое допустимо потратить.

    Упрощенно, алгоритмически явная дефрагментация сводится к сканированию b-tree с формированием списка страниц расположенных близко к концу БД, а затем копирование этих страниц в не-используемые, но расположенные ближе к началу БД.
    В результате, после фиксации дефрагментирующей транзакции оригиналы скопированных страниц становятся не-используемыми, а размер БД может быть уменьшен за счет отсечения ни-используемых страниц в конце используемого пространства.
    Будет реализовано в 0.14.2.

3. Нелинейная переработка GC, без остановки переработки мусора на старом MVCC-снимке используемом долгой транзакцией чтения.

    После реализации запланированного, любая длительная читающая транзакция по-прежнему будет удерживать от переработки используемый/читаемый MVCC-снимок данных (все образующие его страницы БД), но позволит перерабатывать все неиспользуемые MVCC-снимки, как до читаемого, так и после.
    Это позволит устранить [один из основных архитектурных недостатков](https://libmdbx.dqdkfa.ru/intro.html#long-lived-read) унаследованных от LMDB и связанных с ростом размера БД пропорционально объёму производимых изменений данных на фоне долго работающей транзакции чтения.

    Будет реализовано предположительно в 0.14.3, 0.14.4 или даже в 0.15.x.
    Перенос в 0.15.x оправдан возможностью переноса функционала дефрагментации в stable-ветку, но посмотри как пойдут дела.

********************************************************************************

For early releases and changes see the ChangeLog-NN the git commit history.
