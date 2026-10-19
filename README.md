
libmdbx
=======

<!-- section-begin overview -->

_libmdbx_ is an extremely fast, compact, powerful, embedded, transactional [key-value database](https://en.wikipedia.org/wiki/Key-value_database), with [Apache 2.0 license](https://sourcecraft.dev/dqdkfa/libmdbx/blob?file=LICENSE). _libmdbx_ has a specific set of properties and capabilities, focused on creating unique lightweight solutions.

1. Allows **a swarm of multi-threaded processes to [ACID](https://en.wikipedia.org/wiki/ACID)ly read and update** several key-value [maps](https://en.wikipedia.org/wiki/Associative_array) and [multimaps](https://en.wikipedia.org/wiki/Multimap) in a locally-shared database.

2. Provides **extraordinary performance**, minimal overhead through [Memory-Mapping](https://en.wikipedia.org/wiki/Memory-mapped_file) and `O(log N)` operation costs by virtue of [B+tree](https://en.wikipedia.org/wiki/B%2B_tree).

3. Requires **no maintenance and no crash recovery** since it doesn't use [WAL](https://en.wikipedia.org/wiki/Write-ahead_logging), but that might be a caveat for write-intensive workloads with durability requirements.

4. Enforces [serializability](https://en.wikipedia.org/wiki/Serializability) for writers through a single [mutex](https://en.wikipedia.org/wiki/Mutual_exclusion) and affords [wait-free](https://en.wikipedia.org/wiki/Non-blocking_algorithm#Wait-freedom) for parallel readers without atomic/interlocked operations, while **writing and reading transactions do not block each other**.

5. **Guarantees data integrity** after crash unless this was explicitly neglected in favour of write performance.

6. Supports Linux, Windows, MacOS, Harmony, Android, iOS, FreeBSD, DragonFly BSD, Solaris, OpenSolaris, OpenIndiana, NetBSD, OpenBSD and other systems compliant with **POSIX.1-2008**.

7. **Compact and friendly for full embedding**. Just a few flat source code files. Here no internal threads nor server process(es), but implements the core of the [Berkeley DB](https://en.wikipedia.org/wiki/Berkeley_DB) API with many powerful extensions.

<!-- section-end -->

Historically, _libmdbx_ is a deeply revised and extended descendant of the legendary [Lightning Memory-Mapped Database](https://en.wikipedia.org/wiki/Lightning_Memory-Mapped_Database). _libmdbx_ inherits all benefits from _LMDB_, but resolves some issues and adds [a large set of improvements](#improvements-beyond-lmdb).

[![Telegram: Support | Discussions | News](https://img.shields.io/endpoint?color=scarlet&logo=telegram&label=Support%20%7C%20Discussions%20%7C%20News&url=https%3A%2F%2Ftg.sumanjay.workers.dev%2Flibmdbx)](https://t.me/libmdbx)

> Please rely on  the [libmdbx.dqdkfa.ru](https://libmdbx.dqdkfa.ru) site with [FAQ](https://libmdbx.dqdkfa.ru/#sec-faq), [Tips](https://libmdbx.dqdkfa.ru/#sec-tips), [Rules/Checklists](https://libmdbx.dqdkfa.ru/#sec-rules), [Knowledge base](https://libmdbx.dqdkfa.ru/#sec-knowledge), references for [`C++`](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__cxx__api.html) and [`C`](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__api.html) API, etc.

-----

# Characteristics

<!-- section-begin characteristics -->

## Features

- Key-value data model, keys are always sorted.

- Fully [ACID](https://en.wikipedia.org/wiki/ACID)-compliant, through to [MVCC](https://en.wikipedia.org/wiki/Multiversion_concurrency_control) and [CoW](https://en.wikipedia.org/wiki/Copy-on-write).

- Multiple key-value tables/sub-databases within a single datafile.

- Range lookups, including range query estimation.

- Efficient support for short fixed-length keys, including native 32/64-bit integers.

- Ultra-efficient support for [multimaps](https://en.wikipedia.org/wiki/Multimap). Multi-values sorted, searchable and iterable. Keys stored without duplication.

- Data is [memory-mapped](https://en.wikipedia.org/wiki/Memory-mapped_file) and accessible directly/zero-copy. In-memory full-scan of database records is extremely fast.

- Transactions for readers and writers, ones do not block others.

- No transaction conflicts nor deadlocks since writes are strongly serialized.

- Readers are [non-blocking](https://en.wikipedia.org/wiki/Non-blocking_algorithm), notwithstanding [snapshot isolation](https://en.wikipedia.org/wiki/Snapshot_isolation).

- Reads scale linearly across CPUs.

- Both explicit defragmentation and continuous zero-overhead database compactification.

- Automatic on-the-fly database size adjustment.

- `O(log N)` cost of lookup, insert, update, and delete operations by virtue of [B+ tree characteristics](https://en.wikipedia.org/wiki/B%2B_tree#Characteristics).

- Improbably fast and robust get(key) operations [accelerated by a shareable lock-free cache](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__crud.html#function-mdbx_cache_get).

- Online hot backup.

- Append operation for efficient bulk insertion of pre-sorted data.

- No [WAL](https://en.wikipedia.org/wiki/Write-ahead_logging) nor any transaction journal. No crash recovery needed. No maintenance is required.

- Flexible transaction API with support for nested transactions.

- Customizable database page size.

## Limitations

- **Page size**: a power of 2, minimum `256` (mostly for testing), maximum `65536` bytes, default `4096` bytes.
- **Key size**: minimum `0`, maximum ≈½ pagesize (`2022` bytes for default 4K pagesize, `32742` bytes for 64K pagesize).
- **Value size**: minimum `0`, maximum `2146435072` (`0x7FF00000`) bytes for maps, ≈½ pagesize for multimaps (`2022` bytes for default 4K pagesize, `32742` bytes for 64K pagesize).
- **Write transaction size**: up to `1327217884` pages (`4.944272` TiB for default 4K pagesize, `79.108351` TiB for 64K pagesize).
- **Database size**: up to `2147483648` pages (≈`8.0` TiB for default 4K pagesize, ≈`128.0` TiB for 64K pagesize).
- **Maximum tables/sub-databases**: `32765`.

## Gotchas

* There cannot be more than one writer at a time, i.e. no more than one write transaction at a time.

* _libmdbx_ is based on [B+ tree](https://en.wikipedia.org/wiki/B%2B_tree), so access to database pages is mostly random.
Thus SSDs provide a significant performance boost over spinning disks for large databases.

* _libmdbx_ uses [shadow paging](https://en.wikipedia.org/wiki/Shadow_paging) instead of [WAL](https://en.wikipedia.org/wiki/Write-ahead_logging).
Thus syncing data to disk might be a bottleneck for write-intensive workloads.

* _libmdbx_ uses [copy-on-write](https://en.wikipedia.org/wiki/Copy-on-write) for [snapshot isolation](https://en.wikipedia.org/wiki/Snapshot_isolation) during updates, but a read transaction prevents recycling of retired/freed pages while it is still reading them.
Thus altering of data during a parallel long-lived read operation will increase the process work set, may exhaust entire free database space, the database can grow quickly, and result in performance degradation.
Try to avoid long running read transactions, otherwise use [transaction parking](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__transactions.html#function-mdbx_txn_park)
and/or [Handle-Slow-Readers callback](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__err.html#function-mdbx_env_set_hsr).

* _libmdbx_ is extraordinarily fast and provides minimal overhead for data access, so you should reconsider using brute force techniques and double check your code.
On the one hand, in the case of _libmdbx_, a simple linear search may be more profitable than complex indexes.
On the other hand, if you make something suboptimally, you can notice its detrimental effect only on sufficiently large data.

## Durability and synchronization modes

_libmdbx_ provides a set of trade-offs between durability and write performance, selected per environment via `mdbx_env_set_option()`:

| Mode | DB data synced to disk | Meta-page synced | Survives OS crash | Survives power loss |
| --- | --- | --- | --- | --- |
| `MDBX_DURABLE` (default) | each commit | each commit | yes | yes |
| `MDBX_NOMETASYNC` | each commit | no | yes | last commits may roll back |
| `MDBX_SAFE_NOSYNC` | lazy (thresholds/timeout) | lazy | yes | recent commits may be lost, integrity guaranteed |
| `MDBX_UTTERLY_NOSYNC` | no explicit sync | no | no | possible corruption (matches LMDB `MDB_NOSYNC`) |

In addition, `MDBX_WRITEMAP` maps data pages and writes through `mmap`, which can be combined with the modes above (typically with `MDBX_NOMETASYNC`) to reduce data copying at the cost of more memory-mapped I/O. See the [API reference](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__opening.html) for the exact semantics.

## Comparison with other databases
For now please refer to [chapter of "BoltDB comparison with other databases"](https://github.com/coreos/bbolt#comparison-with-other-databases) which is also (mostly) applicable to _libmdbx_ with minor clarification:
 - a database could be shared by multiple processes, i.e. no multi-process issues;
 - no issues with moving a cursor(s) after the deletion;
 - _libmdbx_ provides zero-overhead database compactification, so a database file could be shrunk/truncated in particular cases;
 - excluding disk I/O time _libmdbx_ could be ≈3 times faster than BoltDB and up to 10–100,000× faster than both BoltDB and LMDB in specific edge cases;
 - _libmdbx_ provides even more features compared to BoltDB and/or LMDB.

<!-- section-end -->

<!-- section-begin bindings -->

Bindings
========

The full list of all bindings (several dozen) is available in the [Bindings and Projects](https://libmdbx.dqdkfa.ru/#sec-projects) section of the libmdbx homesite; here are only a few of the highest demand.

| Runtime |  Repo  | Author |
| ------- | ------ | ------ |
| Rust    | [libmdbx-rs](https://github.com/vorot93/libmdbx-rs)   | [Artem Vorotnikov](https://github.com/vorot93) |
| Go      | [mdbx-go](https://github.com/torquem-ch/mdbx-go)      | [Alex Sharov](https://github.com/AskAlexSharov) |
| .NET    | [libmdbx-dotnet](https://public.git.amsoft.spb.ru/libmdbx/libmdbx-dotnet) | [Anton Maisak](mailto:anton@maisak.ru) |
| CPython | [PyPi/clibmdbx](https://pypi.org/project/clibmdbx/)   | [`@jyj117`](https://github.com/jyj117) |
| Python  | [PyPi/libmdbx](https://pypi.org/project/libmdbx/)     | [Lazymio](https://github.com/wtdcode) |
| NodeJS  | [mdbxmou](https://github.com/ikonopistsev/mdbxmou)    | [Igor Ikonopistsev](https://github.com/ikonopistsev) |
| Zig     | [mdbx-zig](https://github.com/theseyan/lmdbx-zig)     | [Sayan J. Das](https://github.com/theseyan) |

<!-- section-end -->

Usage
=====

<!-- section-begin usage -->

Since December 2025 _libmdbx_ is available only in an amalgamated source code form like [SQLite](https://www.sqlite.org/amalgamation.html), without additional dependencies and internal resources needed only for development of _libmdbx_ itself. Package support for common Linux distributions is planned in the future, after the `1.0` release.

The source code is available on [SourceCraft](https://sourcecraft.dev/dqdkfa/libmdbx) and mirror on [GitHub](https://github.com/Mithril-mine/libmdbx).
Please use the `stable` branch or the latest release for production environments through staging, and the `master` branch for development of derivative projects.

## Building and Testing

[Source code](https://en.wikipedia.org/wiki/Source_code) provides build through the use of [CMake](https://cmake.org/) or [GNU Make](https://www.gnu.org/software/make/) with [bash](https://en.wikipedia.org/wiki/Bash_(Unix_shell)).

All build ways are completely traditional and have minimal prerequisites like `build-essential`, i.e. the non-obsolete C/C++ compiler and a [SDK](https://en.wikipedia.org/wiki/Software_development_kit) for the target platform. Obviously you need building tools itself, i.e. `git`, `cmake` or GNU `make` with `bash`. For your convenience, `make help` and `make options` are also available for listing existing targets and build options respectively.

So just use CMake or GNU Make in your habitual manner, and feel free to file an issue in case something is unexpected or breaks down.

### Testing
Amalgamated source code does not contain most of the tests and other internal components for several reasons. You can find explanations of the reasons in the comments to the presentation of [_libmdbx_ roadmap](https://libmdbx.dqdkfa.ru/release/libmdbx-roadmap-HNY2026-english.pdf) on the eve of 2026. However, an extended example of using the C++ API will be added soon, which can also be used as a simple smoke-test.

### Common important details

#### Build reproducibility
By default _libmdbx_ track build time via `MDBX_BUILD_TIMESTAMP` build option and macro. So for a [reproducible builds](https://en.wikipedia.org/wiki/Reproducible_builds) you should predefine/override it to known fixed string value. For instance:

 - for reproducible build with make: `make MDBX_BUILD_TIMESTAMP=unknown ` ...
 - or during configure by CMake: `cmake -DMDBX_BUILD_TIMESTAMP:STRING=unknown ` ...

Of course, in addition to this, your toolchain must ensure the reproducibility of builds. For more information please refer to [reproducible-builds.org](https://reproducible-builds.org/).

#### Containers
There are no special traits nor quirks if you use _libmdbx_ ONLY inside the single container. But in a cross-container(s) or with a host-container(s) interoperability cases the three major things MUST be guaranteed:

1. Coherence of memory mapping content and unified page cache inside OS kernel for host and all container(s) operated with a DB. Basically this means there must be only a single physical copy of each memory mapped DB' page in the system memory.

2. Uniqueness of [PID](https://en.wikipedia.org/wiki/Process_identifier) values and/or a common space for them:
    - for POSIX systems: PID uniqueness for all processes operated with a DB. I.e. the `--pid=host` is required to run DB-aware processes inside Docker, or, without host interaction, a `--pid=container:<name|id>` with the same name/id.
    - for non-POSIX (i.e. Windows) systems: inter-visibility of processes handles. I.e. the `OpenProcess(SYNCHRONIZE, ..., PID)` must return reasonable error, including `ERROR_ACCESS_DENIED`, but not the `ERROR_INVALID_PARAMETER` as for an invalid/non-existent PID.

3. The versions/builds of _libmdbx_ and `libc`/`pthreads` (`glibc`, `musl`, etc) must be compatible.
   - Basically, the `options:` string in the output of `mdbx_chk -V` must be the same for host and container(s). See `MDBX_LOCKING`, `MDBX_USE_OFDLOCKS` and other build options for details.
   - Avoid using different versions of `libc`, especially mixing different implementations, i.e. `glibc` with `musl`, etc. Prefer to use the same LTS version, or switch to full virtualization/isolation if in doubt.

#### DSO/DLL unloading and destructors of Thread-Local-Storage objects
When building _libmdbx_ as a shared library or using static _libmdbx_ as a part of another dynamic library, it is advisable to make sure that your system ensures the correctness of calling the destructors of Thread-Local-Storage objects when unloading dynamic libraries.

If this is not the case, then unloading a dynamic-link library with _libmdbx_ code inside, can result in either a resource leak or a crash due to calling destructors from an already unloaded DSO/DLL object. The problem can only manifest in a multithreaded application, which makes the unloading of shared dynamic libraries with _libmdbx_ code inside, after using _libmdbx_. It is known that TLS-destructors are properly maintained in the following cases:

- On all modern versions of Windows (Windows 7 and later).

- On systems with the [`__cxa_thread_atexit_impl()`](https://sourceware.org/glibc/wiki/Destructor%20support%20for%20thread_local%20variables) function in the standard C library, including systems with GNU libc version 2.18 and later.

- On systems with libpthread/ntpl from GNU libc with bug fixes [#21031](https://sourceware.org/bugzilla/show_bug.cgi?id=21031) and [#21032](https://sourceware.org/bugzilla/show_bug.cgi?id=21032), or where there are no similar bugs in the pthreads implementation.

### Linux and other platforms with GNU Make
To build the library it is enough to execute `make all` in the directory of source code, and `make check` to execute the basic tests.

If the `make` installed on the system is not GNU Make, there will be a lot of errors from make when trying to build. In this case, perhaps you should use `gmake` instead of `make`, or even `gnu-make`, etc.

### FreeBSD and related platforms
As a rule on BSD and its derivatives the default is to use Berkeley Make and [Bash](https://en.wikipedia.org/wiki/Bash_(Unix_shell)) is not installed.

So you need to install the required components: GNU Make, Bash, C and C++ compilers compatible with GCC or CLANG. After that, to build the library, it is enough to execute `gmake all` (or `make all`) in the directory with source code, and `gmake check` (or `make check`) to run the basic tests.

### Windows
To build _libmdbx_ on Windows the _original_ CMake and [Microsoft Visual Studio 2019 or 2022](https://en.wikipedia.org/wiki/Microsoft_Visual_Studio) are recommended. Please use the recent versions of CMake, Visual Studio and Windows SDK to avoid troubles with C11 support and `alignas()` feature.

To build with MinGW the 10.2 or recent version coupled with a modern CMake are required. So it is recommended to use [chocolatey](https://chocolatey.org/) to install and/or update the ones.

Other ways to build are potentially possible but are not supported and will not be. The `CMakeLists.txt` or `GNUMakefile` scripts will probably need to be modified accordingly. Using other methods do not forget to add the `ntdll.lib` to linking.

It should be noted that in _libmdbx_ there were efforts to avoid runtime dependencies from CRT and other MSVC libraries. For this is enough to pass the `-DMDBX_WITHOUT_MSVC_CRT:BOOL=ON` option during configure by CMake.

### Windows Subsystem for Linux
_libmdbx_ could be used in [WSL2](https://en.wikipedia.org/wiki/Windows_Subsystem_for_Linux#WSL_2) but NOT in [WSL1](https://en.wikipedia.org/wiki/Windows_Subsystem_for_Linux#WSL_1) environment. This is a consequence of the fundamental shortcomings of _WSL1_ and cannot be fixed. To avoid data loss, _libmdbx_ returns the `ENOLCK` (37, "No record locks available") error when opening the database in a _WSL1_ environment.

### MacOS
Current [native build tools](https://en.wikipedia.org/wiki/Xcode) for MacOS include GNU Make, CLANG and an outdated version of Bash. However, the build script uses GNU-kind of `sed` and `tar`. So the easiest way to install all prerequisites is to use [Homebrew](https://brew.sh/), just by `brew install bash make cmake ninja gnu-sed gnu-tar --with-default-names`.

Next, to build the library, it is enough to run `make all` in the directory with source code, and run `make check` to execute the base tests. If something goes wrong, it is recommended to install [Homebrew](https://brew.sh/) and try again.

### Harmony OS
Please use CMake with the ["toolchain file"](https://cmake.org/cmake/help/latest/variable/CMAKE_TOOLCHAIN_FILE.html) provided by HarmonyOS SDK.

### Android
Please use CMake to build _libmdbx_ for Android. Please refer to the [official guide](https://developer.android.com/studio/projects/add-native-code).

### iOS
To build _libmdbx_ for iOS, please use CMake with the ["toolchain file"](https://cmake.org/cmake/help/latest/variable/CMAKE_TOOLCHAIN_FILE.html) from the [ios-cmake](https://github.com/leetal/ios-cmake) project.

<!-- section-end -->

## Packaging / Conan

**The recommended way to consume libmdbx is the source-level route** — the
amalgamated sources (`mdbx.h`, `mdbx.c`, `mdbx.h++`, `mdbx.c++`) for direct
embedding, or CMake subproject / FetchContent. Then the library and the
application are compiled by the same toolchain with the same flags in the
same build, which structurally rules out ABI drift.

A Conan recipe is also shipped (`conanfile.py`) as a convenience for
prebuilt consumption. However, Conan resolves, builds and caches binaries
according to the *active profile*, and nothing ties that profile to the
compiler the consumer's project is actually built with. On any
multi-compiler machine the profile and the consumer's real toolchain can
silently diverge: `find_package(mdbx)` succeeds and the mismatch surfaces
only at runtime.

This is especially dangerous for the C++ binding: `mdbx.h++` is compiled by
the *consumer's* compiler while the out-of-line definitions and extern
template instantiations (e.g. `mdbx::default_buffer`) live in the package's
binary, compiled by the *profile's* compiler. For a consistent C++ ABI the
active Conan profile must exactly match the consumer's
compiler/version, stdlib (`libstdc++` vs `libc++`), `_GLIBCXX_USE_CXX11_ABI`
setting, build type and C++ standard. The recipe warns about this both when
building and when consuming the package, and the `mdbx.cxx_standard` option
lets you pin the C++ standard instead of relying on compiler auto-picking.

For the pure-C core the risk is modest, since the platform C ABI is stable
across compilers; a C++ consumer that is not diligent about toolchain match
may prefer to embed the sources instead of using the prebuilt package.

### Package binary identity

Conan's binary identifier (`package_id`) is the hash of the *active profile*
and **every** recipe option: the recipe does not opt out of any setting nor
option. So for a given library version the following shape the identity — and
equally important, what does *not*:

| Part of the binary identity | NOT part of the identity (can silently diverge) |
| --- | --- |
| profile `settings`: os, arch, compiler & its version, build_type | compiler warnings and warning machinery |
| every `mdbx.*` option: ABI knobs (`64bit_atomic`, `64bit_cas`, `unaligned_ok`, `cacheline_size`, `without_msvc_crt`, `build_cxx`, `cxx_standard`, `locking`), feature toggles (`build_tools`, `enable_*`, `force_assertions`, ...) | the exact `-O`-level details *inside* a given build_type |
| `mdbx.lto`: `Default`/`True`/`False`/`Fat` produce distinct ids | the consumer's own CMake toolchain-file and its `CMAKE_*_FLAGS` |
| `mdbx.sanitize`: `none`/`address`/`undefined`/`address,undefined` | libstdc++/glibc versions of the machine that built the cached binary |
| `shared` | whether the consumer's own build engages LTO (only `mdbx.lto=Fat` keeps the packaged objects usable regardless) |

The "NOT part" rows are exactly the silent ABI traps: Conan serves a cached
binary with a matching hash while the consumer's real environment differs.
Nothing but the profile ties the two together, so keep the profile in sync
with what actually builds the consumer project.

### mdbx.lto and mdbx.sanitize

- `mdbx.lto=Default` leaves LTO to CMake's own default; `True`/`False` force
  `INTERPROCEDURAL_OPTIMIZATION` on/off; `Fat` additionally sets
  `MDBX_LTO_FAT_OBJECTS`, so GCC/Clang produce dual LTO+native object code
  (`-ffat-lto-objects`) and the packaged library links into a consumer build
  regardless of whether the consumer engages LTO. On MSVC `Fat` behaves as
  `True`.
- `mdbx.sanitize=none|address|undefined|address,undefined` adds
  `-fsanitize=...` to the compile and link flags. A sanitized package must be
  consumed by a sanitized build (the sanitizer runtime participates in the
  ABI), hence both options participate in the package identity.

On the roadmap the C++ API will become the native one, with the C API
provided as a secondary interface (an emulation facade) over it. That
transition is expected to be long and is *not* the case yet: currently the C
core and the C ABI are first-class, and the toolchain-match requirement above
applies mostly to consumers of the C++ binding. Once the transition lands,
the requirement will apply to every consumer of a prebuilt package.

## API description

Please refer to the online [_libmdbx_ API reference](https://libmdbx.dqdkfa.ru/docs) and/or see the [mdbx.h++](mdbx.h%2B%2B) and [mdbx.h](mdbx.h) headers.

## Textbook

A complete practical guide to libmdbx — six volumes from the first steps to expert topics and bindings — is available online in [Russian](https://libmdbx.dqdkfa.ru/docs/ru/textbook/) and [English](https://libmdbx.dqdkfa.ru/docs/en/textbook/); each online version links a downloadable PDF.

The textbook's source (markdown, in `docs/textbook/ru` and `docs/textbook/en`) and all code examples (`examples/` — compilable C11/C++17 programs, exercised by CTest) are part of this repository. Both localizations are built into HTML + PDF with `make books` (requires `pip install mkdocs mkdocs-material mkdocs-with-pdf`).

## Contributing

Contributions are welcome. The project uses [CMake](https://cmake.org/)+[CTest](https://cmake.org/cmake/help/latest/module/CTest.html) for building and testing and [LLVM](https://llvm.org/) code style; build support for at least Linux and Windows is expected for any change. Please keep the amalgamated build green (`make dist`), avoid enabling link-time optimization in test builds, and add/update tests together with your change. For the project structure, build details and workflows, see `docs/` and the [home site](https://libmdbx.dqdkfa.ru).

--------------------------------------------------------------------------------

<!-- section-begin history -->

# History

Historically, _libmdbx_ is a deeply revised and extended descendant of the [Lightning Memory-Mapped Database](https://en.wikipedia.org/wiki/Lightning_Memory-Mapped_Database). At first the development was carried out within the [ReOpenLDAP](https://sourcecraft.dev/dqdkfa/reopenldap) project. About a year later _libmdbx_ was separated into a standalone project, which was [presented at Highload++ 2015 conference](http://www.highload.ru/2015/abstracts/1831.html).

Since 2017 _libmdbx_ is used in [Fast Positive Tables](https://sourcecraft.dev/dqdkfa/libfpta), and until 2025 development was funded by [Positive Technologies](https://www.ptsecurity.com). Since 2020 _libmdbx_ is used in Ethereum: [Erigon](https://github.com/erigontech/erigon), [Akula](https://github.com/akula-bft/akula), [Silkworm](https://github.com/erigontech/silkworm), [Reth](https://github.com/paradigmxyz/reth), etc.

On 2022-04-15 the GitHub administration, without any warning nor explanation, deleted _libmdbx_ along with a lot of other projects, simultaneously blocking access for many developers. Therefore on 2022-04-21 I have migrated to a reliable trusted infrastructure.
The origin for now is at [SourceCraft](https://sourcecraft.dev/dqdkfa/libmdbx) and ~~GitHub~~ is blacklisted forever to play this role.

Since May 2024 and version 0.13 _libmdbx_ was re-licensed under Apache-2.0 license. Please refer to the [`COPYRIGHT` file](https://sourcecraft.dev/dqdkfa/libmdbx/blob/raw?file=COPYRIGHT) for license change explanation.

## Acknowledgments
Howard Chu <hyc@openldap.org> and Hallvard Furuseth <hallvard@openldap.org> are the authors of _LMDB_, from which _libmdbx_ was forked in 2015.

Martin Hedenfalk <martin@bzero.se> is the author of `btree.c` code, which was used to begin development of _LMDB_.

<!-- section-end -->

<!-- section-begin improvements -->

Improvements beyond LMDB
========================

_libmdbx_ is superior to legendary _[LMDB](https://symas.com/lmdb/)_ in terms of features and reliability, not inferior in performance. In comparison to _LMDB_, _libmdbx_ makes things "just work" perfectly and out-of-the-box, not silently and catastrophically break down. The list below is reduced to the improvements most notable and obvious from the user's point of view.

## Some Added Features

* Keys could be more than 2 times longer than _LMDB_, support of zero-length for keys and values.
   > For DB with default page size _libmdbx_ supports keys up to 2022 bytes and up to 32742 bytes for 64K page size. _LMDB_ allows key size up to 511 bytes and may silently lose data with large values.

* Up to 30% faster than _LMDB_ in [CRUD](https://en.wikipedia.org/wiki/Create,_read,_update_and_delete) benchmarks.
   > Benchmarks of the in-[tmpfs](https://en.wikipedia.org/wiki/Tmpfs) scenarios, that tests the speed of the engine itself, showed that _libmdbx_ 10-20% faster than _LMDB_, and up to 30% faster when _libmdbx_ compiled with specific build options which downgrades several runtime checks to match LMDB behaviour.
   >
   > However, libmdbx may be slower than LMDB on Windows, since it uses the native file-locking API. These locks are really slow, but they prevent an inconsistent backup from being obtained by copying the DB file during an ongoing write transaction. So I think this is the right decision, and for speed, it's better to use Linux, or ask Microsoft to fix up file locks.
   >
   > Noted above and other results could be easily reproduced with [ioArena](https://sourcecraft.dev/dqdkfa/ioarena) by running the `make bench-quartet` command, including comparisons with [RocksDB](https://en.wikipedia.org/wiki/RocksDB) and [WiredTiger](https://en.wikipedia.org/wiki/WiredTiger).

* Automatic on-the-fly database size adjustment, both growth and truncation.
   > _libmdbx_ manages the database size according to parameters specified by `mdbx_env_set_geometry()` function, which include the growth step and the truncation threshold.
   >
   > Unfortunately, on-the-fly database size adjustment doesn't work under [Wine](https://en.wikipedia.org/wiki/Wine_(software)) due to its internal limitations and unimplemented functions, i.e. the `MDBX_UNABLE_EXTEND_MAPSIZE` error will be returned.

* Automatic continuous zero-overhead database compactification.
   > During each commit _libmdbx_ merges freed pages which are adjacent to the unallocated area at the end of file, and then truncates unused space when enough of it has accumulated.

* The same database format for 32- and 64-bit builds.
   > _libmdbx_ database format depends only on the [endianness](https://en.wikipedia.org/wiki/Endianness) but not on the [bitness](https://en.wiktionary.org/wiki/bitness).

* The "Big Foot" feature that solves specific performance issues with huge transactions and extra-large page-number-lists.

* LIFO policy for Garbage Collection recycling. Thanks to the write-back disk cache, this can multiply write performance several times in the best case.
   > LIFO means that the most recently unused pages will be taken for reuse. Therefore the loop of database pages circulation becomes as short as possible. In other words, the set of pages, that are (over)written in memory and on disk during a series of write transactions, will be as small as possible. This creates ideal conditions for the battery-backed or flash-backed disk cache efficiency.

* Parking of read transactions with ousting and auto-restart, [Handle-Slow-Readers callback](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__err.html#function-mdbx_env_set_hsr) to resolve issues due to long-lived read transactions.

* Fast estimation of range query result volume, i.e. how many items can be found between a `KEY1` and a `KEY2`. This is a prerequisite for building and/or optimizing query execution plans.
   > _libmdbx_ performs a rough estimate based on the common B-tree pages along the paths from root to corresponding keys.

* Database integrity check API both with standalone `mdbx_chk` utility.

* Support for opening databases in the exclusive mode, including on a network share.

* Extended information about the whole database, tables/sub-databases, transactions, readers enumeration.
    > _libmdbx_ provides a lot of information, including dirty and leftover pages for a write transaction, reading lag and holdover space for read transactions.

* The ["get-cached" feature](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__crud.html#function-mdbx_cache_get) with lightweight transparent cache that can provide dramatic acceleration in many cases.

* [Cloning a read transaction](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__transactions.html#function-mdbx_txn_clone) and [resurrection after fork](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__extra.html#function-mdbx_env_resurrect_after_fork) features.

* Automated steady sync-to-disk upon several thresholds and/or timeout via cheap polling.

* Extended update and quick delete operations.
    > _libmdbx_ allows [getting the previous value](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__crud.html#function-mdbx_replace) _at once_ when updating, and addressing the particular item of a multi-value with the same key.
    > _libmdbx_ supports [massive deletion by bunches](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__crud.html#function-mdbx_cursor_bunch_delete) of adjacent elements much faster by cutting out entire pages and branches from a B-tree.

* Ability to determine whether the particular data is on a dirty page or not, which allows avoiding copy-out before updates.

* Sequence generation and three persistent 64-bit vector-clock-like markers.

* Useful runtime options for tuning the engine to the application's requirements and use cases.

## Other fixes and specifics

* Fixed more than a dozen bugs, many of which are still present in LMDB, in particular: page leaks, wrong table/sub-database statistics, segfaults in several conditions, non-optimal page merge strategy, updating an existing item with resizing data (including for a multimap), etc.

* All cursors can be reused and should be closed explicitly, regardless of whether they were opened within a write or read transaction.

* Opening database handles is spared from race conditions and pre-opening is not needed.

* Returning `MDBX_EMULTIVAL` error in case of ambiguous update or delete.

* Guarantee of database integrity even in asynchronous unordered write mode.
   > _libmdbx_ proposes an additional trade-off by `MDBX_SAFE_NOSYNC` in an append-like manner for updates, that avoids database corruption after a system crash contrary to LMDB.
   > Nevertheless, the `MDBX_UTTERLY_NOSYNC` mode is available to match LMDB's behaviour for `MDB_NOSYNC`.

* On **MacOS & iOS** the `fcntl(F_FULLFSYNC)` syscall is used _by default_ to synchronize data with the disk, as this is [the only way to guarantee data durability](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/fsync.2.html) in case of power failure. Unfortunately, in scenarios with high write intensity, the use of `F_FULLFSYNC` significantly degrades performance compared to LMDB, where the `fsync()` syscall is used. Therefore, _libmdbx_ allows you to override this behavior by defining the `MDBX_OSX_SPEED_INSTEADOF_DURABILITY=1` option while building the library.

* On **Windows** the `LockFileEx()` syscall is used for locking, since it allows placing the database on network drives, and provides protection against misuse (aka [poka-yoke](https://en.wikipedia.org/wiki/Poka-yoke)). Therefore _libmdbx_ may be a little lag in performance tests from LMDB where the named mutexes are used.

<!-- section-end -->

--------------------------------------------------------------------------------

## MithrilDB and Future

<!-- section-begin mithril -->

The next version is under non-public development and will be released as **MithrilDB** and `libmithrildb` for libraries & packages. Admittedly mythical [Mithril](https://en.wikipedia.org/wiki/Mithril) resembles silver, but is stronger and lighter than steel. Therefore _MithrilDB_ is an aptly relevant name.

_MithrilDB_ differs radically from _libmdbx_ in the new database format and API based on C++20. The goal of this revolution is to provide a clearer and robust API, add more features and new valuable properties of the database. All fundamental architectural problems of libmdbx/LMDB have been solved there, but now the active development has been suspended for top-three reasons:

1. For now _libmdbx_ is mostly enough and I am busy with scalability.
2. Waiting for fresh [Elbrus CPU](https://wiki.elbrus.ru/) of [e2k architecture](https://en.wikipedia.org/wiki/Elbrus_2000), especially with hardware acceleration of [Streebog](https://en.wikipedia.org/wiki/Streebog) and [Kuznyechik](https://en.wikipedia.org/wiki/Kuznyechik), which are required for Merkle tree, etc.
3. The expectation of needs and opportunities due to the wide use of NVDIMM (aka persistent memory), modern NVMe and [Ангара](https://ru.wikipedia.org/wiki/Ангара_(интерконнект)).

However, _MithrilDB_ may not be available in jurisdictions that have imposed sanctions on Russia: I do not want my work to serve, even indirectly, purposes that I cannot accept. Whether this restriction will be implemented through licensing and support terms, or by keeping the source closed, is not yet decided. My position here is simple and firm — I will not allow my work to contribute to weapons that are used against people I care about.

Nonetheless, I try not to make any promises regarding _MithrilDB_ until release.

Contrary to _MithrilDB_, _libmdbx_ will forever remain free and open source. Moreover with high-quality support whenever possible. Tu deviens responsable pour toujours de ce que tu as apprivoisé. Regardless of the earlier GitHub incident and the current sanctions, _libmdbx_ will continue to comply with the original open license and the principles of constructive cooperation. I will also keep supporting Windows, even though it is no longer a technology we use ourselves.

<!-- section-end -->
