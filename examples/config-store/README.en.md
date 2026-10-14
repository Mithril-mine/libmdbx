# The `config-store` end-to-end project

A demonstration configuration storage built on libmdbx (C++17). The project
grows together with the textbook: each **slice** (`config-store-0N.c++`) is a
self-contained, compilable program that shows one new capability on top of the
previous ones.

## Evolution scheme

| Slice | Volume / chapter | What's new compared to the previous slice |
| ---- | ----------- | ------------------------------------------- |
| `config-store-02` | I / ch. 2 | Scaffolding: opening the DB, set/get/del commands |
| `config-store-04` | I / ch. 4 | add (insert_unique) vs set (upsert); KEYEXIST/NOTFOUND handling |
| `config-store-05` | I / ch. 5 | Batches in a single transaction; rollback (abort); read-only observer |
| `config-store-06` | II / ch. 6 | list: cursor walk over keys; prefix range (SET_RANGE) |
| `config-store-08` | II / ch. 8 | Secondary index "tag → list of keys"; by-tag query |
| `config-store-09` | II / ch. 9 | Geometry, runtime options, statistics output |
| `config-store-10` | II / ch. 10 | Selecting a durability mode; sync_to_disk |
| `config-store-11` | II / ch. 11 | Multithreaded access: a writer + reader clones |
| `config-store-12` | II / ch. 12 | Error resilience and guaranteed exit codes |

## Building and running

From the example directory:

```sh
g++ -std=c++17 -I<libmdbx-include> -I../common config-store-02.c++ -lmdbx -o config-store-02
./config-store-02
```

Or via the project's CMake (targets `mdbx_config_store_02` … `mdbx_config_store_12`,
registered as CTest tests):

```sh
cmake --build build --target mdbx_config_store_02
ctest --test-dir build -R config_store_02
```

## Exit-code policy

- `EXIT_SUCCESS` (0) — the run fully succeeded, the summary in stdout as `ok: ...`.
- `EXIT_FAILURE` (1) — any error: diagnostics to stderr.
- Expected `MDBX_KEYEXIST`/`MDBX_NOTFOUND` codes are handled explicitly.

The slices use the shared `common.h++` helpers.