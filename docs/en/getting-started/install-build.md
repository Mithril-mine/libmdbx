# Installation and building

> Related: [First steps](first-steps.md) · [Tooling](../reference/tooling.md) ·
> [Textbook: Volume I "Fundamentals"](../textbook/01-tom-i-osnovy.md)

## Distribution form

Since December 2025 _libmdbx_ is available **only as amalgamated source code**
(the [SQLite](https://www.sqlite.org/amalgamation.html) model) — without
external dependencies and internal resources needed only for libmdbx's own
development. Packages for common Linux distributions are planned after the
`1.0` release.

Sources: [SourceCraft](https://sourcecraft.dev/dqdkfa/libmdbx) and the mirror
[GitHub](https://github.com/Mithril-mine/libmdbx). For production use the
`stable` branch or the latest release (through staging); use `master` for
developing derivative projects.

The amalgamated package is a few flat files:

| File | Purpose |
| --- | --- |
| `mdbx.h`, `mdbx.c` | C API and implementation |
| `mdbx.h++`, `mdbx.c++` | C++ API and implementation |
| `mdbx-internals.h` | internal definitions needed to build `mdbx.c` |
| `mdbx_chk.c`, `mdbx_copy.c`, `mdbx_dump.c`, `mdbx_load.c`, `mdbx_stat.c`, `mdbx_defrag.c` | command-line tools |
| `CMakeLists.txt`, `GNUmakefile`, `Makefile` | build scripts |

## Examples yes, tests no

The amalgamated package **includes compilable examples** (`examples/` — C11/C++
17 programs exercised by CTest) which also serve as a smoke test. **Tests are
not part of the package**: the full test suite belongs to the development tree
and is distributed under a separate proprietary license, different from the
Apache-2.0 license of the library itself. This separation keeps integration
simple and the licensing clean.

## CMake (recommended)

```sh
cmake -S . -B build        # add -DMDBX_BUILD_CXX=ON for the C++ API
cmake --build build
ctest --test-dir build     # smoke tests (if MDBX_ENABLE_TESTS)
```

Useful options: `MDBX_BUILD_CXX` (the C++ API), `MDBX_BUILD_TOOLS` (the tools),
`MDBX_BUILD_SHARED_LIBRARY` (shared vs static), `MDBX_ENABLE_TESTS`,
`MDBX_WITHOUT_MSVC_CRT` (no runtime dependency on the MSVC CRT).
`make help` and `make options` list the available targets and build options.

## GNU Make

```sh
make all      # build the library
make check    # basic checks
```

If the system `make` is not GNU Make you will see many errors: use `gmake`
(FreeBSD/BSD) or install GNU Make (macOS: `brew install make`).

## Platforms

- **Windows**: the original CMake + Visual Studio 2019/2022 (recent CMake/SDK
  versions for C11 and `alignas()`); for MinGW — 10.2+ with a modern CMake
  (conveniently via [chocolatey](https://chocolatey.org/)); with other build
  methods do not forget to add `ntdll.lib` to the link.
- **macOS**: `brew install bash make cmake ninja gnu-sed gnu-tar
  --with-default-names`, then `make all`.
- **FreeBSD and BSDs**: install GNU Make, bash, compilers; then
  `gmake all && gmake check`.
- **Android**: CMake per the
  [official guide](https://developer.android.com/studio/projects/add-native-code).
- **iOS**: CMake with the
  [toolchain file](https://github.com/leetal/ios-cmake).
- **HarmonyOS**: CMake with the toolchain file from the HarmonyOS SDK.
- **WSL2** — supported; **WSL1 — not** (a fundamental limitation; libmdbx
  returns `ENOLCK` when opening a database to avoid data loss).

## Reproducible builds

By default the build time is tracked (`MDBX_BUILD_TIMESTAMP`). For
[reproducible builds](https://reproducible-builds.org/) predefine a fixed
value: `make MDBX_BUILD_TIMESTAMP=unknown ...` or
`cmake -DMDBX_BUILD_TIMESTAMP:STRING=unknown ...` (and the reproducibility of
your toolchain is on you).

## Containers

Inside a single container — no special traits. For host↔container
interoperability three things **must** be guaranteed:

1. **Coherence of the memory mapping content** and the unified page cache of
   the OS kernel for the host and every container working with the database
   (a single physical copy of each memory-mapped DB page in system memory).
2. **PID uniqueness** (POSIX) and/or a common PID space: for Docker —
   `--pid=host`, or a shared `--pid=container:<name|id>` for all DB-aware
   processes. For Windows — process handle inter-visibility
   (`OpenProcess(SYNCHRONIZE, ..., PID)` must return a reasonable error, not
   `ERROR_INVALID_PARAMETER`).
3. **Compatible versions** of libmdbx and `libc`/`pthreads`: the `options:`
   string of `mdbx_chk -V` must be the same for the host and the containers;
   do not mix `glibc` with `musl` — if in doubt, use the same LTS version or
   full virtualization.

## DSO/DLL unloading and TLS destructors

When building libmdbx as a shared library (or using static libmdbx inside
another dynamic library) make sure your system correctly calls
Thread-Local-Storage object destructors when unloading dynamic libraries —
otherwise unloading a DSO/DLL with libmdbx inside, after using it, may leak
resources or crash a multithreaded application. This works correctly on:
Windows 7+; systems with `__cxa_thread_atexit_impl()` in libc
(GNU libc ≥ 2.18); systems with the glibc bugs
[#21031](https://sourceware.org/bugzilla/show_bug.cgi?id=21031)/
[#21032](https://sourceware.org/bugzilla/show_bug.cgi?id=21032) fixed.

## API and the textbook

- The online [API reference](../reference/index.md) and
  the [mdbx.h](https://sourcecraft.dev/dqdkfa/libmdbx/blob?file=mdbx.h) /
  [mdbx.h++](https://sourcecraft.dev/dqdkfa/libmdbx/blob?file=mdbx.h%2B%2B)
  headers.
- The [Textbook](../textbook/01-tom-i-osnovy.md) — six volumes from the
  first steps to expert topics (RU and EN, each with a downloadable PDF).
