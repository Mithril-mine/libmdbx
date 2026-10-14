# libmdbx Textbook

> A complete practical guide to the embedded transactional database
> **libmdbx** — from the first steps to expert topics and bindings.
> All code examples are compilable C++17/C11 programs in the
> [`examples/`](examples/README.txt) directory; fragments in the text are marked as
> "Fragment" and supplemented with links to the full code.
> A [PDF edition of the textbook](libmdbx-textbook-en.pdf) is available for offline reading.

---

## Who this textbook is for

The textbook is intended for developers working with C/C++ (and, in Volume VI,
with other languages through bindings). The minimum level is confident command
of C and a basic understanding of what databases are. Experienced engineers may
start with the second or third volume: internal mechanisms, performance, and
expert topics are self-contained provided the reader is familiar with the API.

## Structure and how to read it

The textbook consists of six volumes linked by a single logic of progression:

| Volume | Topic | Level |
|---|---|---|
| [Volume I. Fundamentals](01-tom-i-osnovy.md) | What libmdbx is, installation, data model, CRUD, transactions | beginner |
| [Volume II. Practice](02-tom-ii-praktika.md) | Cursors, DUPSORT, indexes, configuration, durability, multithreading, errors | developer |
| [Volume III. Mechanisms](03-tom-iii-mehanizmy.md) | B+tree, MVCC, CoW, GC, geometry, locking, recovery | knowledgeable |
| [Volume IV. Performance](04-tom-iv-proizvoditelnost.md) | WAF, configurations, micro-optimizations, get-cached, bulk operations, benchmarks | experienced |
| [Volume V. Expert topics](05-tom-v-ekspertnye-temy.md) | Rules, platforms, HSR, diagnostics, migration, patterns, roadmap | advanced |
| [Volume VI. Bindings](06-tom-vi-bindings.md) | Rust, Go, Python, Node.js, .NET, C++, and others | integrator |

- **Beginners**: we recommend reading Volumes I–II in sequence; there you will
  also follow the end-to-end configurator project, whose exercises reinforce the
  material.
- **Experienced**: Volumes III–V as needed; each volume is self-contained.
- **Integrators**: Volume VI: binding cards with the specifics of each language.

Each chapter follows the pattern "concept → mechanism → practice → nuance", and
ends with a summary, a checklist, and (in Volumes I–II) exercises.

## Code examples

All examples live in [`examples/`](examples/README.txt) and are built via the
project's CMake (or pkg-config):

```sh
cmake -S . -B build -DMDBX_ENABLE_TESTS=ON
cmake --build build
ctest --test-dir build        # also runs the examples
```

Links of the form [`examples/c++/01-hello.c++`](examples/c++/01-hello.c++) in
the text point to a full compilable example (the project's `examples/`
directory, also reachable from this directory via a symbolic link); short
illustrations are marked "Fragment" and are not required to compile on their
own.

## Versions and compatibility

The material corresponds to the current `master` branch (at the time of
publication — the v0.15.x series). The APIs shown are verified against
`mdbx.h` / `mdbx.h++`; some fine-grained parameters (option defaults,
signatures of service functions) should be checked against the official
documentation and the header files of your version.

## About the textbook

The textbook was created by the AI agent **skynet** based on the libmdbx
source code (the `master` branch), the official header files
`mdbx.h`/`mdbx.h++`, and the project documentation. The material has been
cross-checked against the code and factually verified. The goal is to provide
a structured, accurate, and practical guide to libmdbx.

Feedback, remarks, and suggestions — in the project's issues or in the
[Telegram group](https://t.me/libmdbx).