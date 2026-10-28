# Quick CRUD reference

> Related: [First steps](first-steps.md) ·
> [Installation and building](install-build.md) ·
> [Textbook: Volume I "Fundamentals"](../textbook/01-tom-i-osnovy.md) ·
> [C API reference](../reference/api/group__c__api.md)

This is a compact cheat-sheet for the most common operations. The authoritative
Quick Reference with the full decision tables is generated from the `mdbx.h`
annotations and lives in the C API reference:

- [Quick Reference for Insert/Update/Delete operations](../reference/api/group__c__crud.md) —
  cases for tables with UNIQUE keys and with NON-UNIQUE (DUPSORT) keys, plus the
  corresponding flags (`MDBX_NOOVERWRITE`, `MDBX_UPSERT`, `MDBX_CURRENT`,
  `MDBX_APPEND`, `MDBX_APPENDDUP`, ...).

## C++ API at a glance

The C++ binding maps the C operations onto `mdbx::txn` and `mdbx::cursor`
methods. The most common ones:

| Operation | `mdbx::txn` | `mdbx::cursor` | C equivalent |
|-----------|-------------|----------------|--------------|
| Insert or fail | `insert(map, key, value)` | `insert(key, value)` | `mdbx_put(MDBX_NOOVERWRITE)` |
| Insert or replace | `upsert(map, key, value)` | `upsert(key, value)` | `mdbx_put(MDBX_UPSERT)` |
| Update existing | `update(map, key, value)` | `update(key, value)` | `mdbx_put(MDBX_CURRENT)` |
| Read | `get(map, key)` | `to_first()/to_find()/...` | `mdbx_get()` |
| Read & remove | `extract(map, key)` | — | `mdbx_replace()` |
| Delete by key | `erase(map, key)` | `erase(key)` | `mdbx_del()` |
| Delete pair | `erase(map, key, value)` | `erase(key, value)` | `mdbx_del()` |
| Append (tail) | `put(map, key, value, MDBX_APPEND)` | `put(key, value, MDBX_APPEND)` | `mdbx_put(MDBX_APPEND)` |

> See the full reference in the [C++ API](../reference/api/classmdbx_1_1txn.md)
> and [cursor](../reference/api/classmdbx_1_1cursor.md) pages.

## Examples

- [`examples/c++/03-crud.c++`](https://sourcecraft.dev/dqdkfa/libmdbx/blob?file=examples/c%2B%2B/03-crud.c%2B%2B) ·
  [C version](https://sourcecraft.dev/dqdkfa/libmdbx/blob?file=examples/c/03-crud.c)
- Textbook: [Chapter 4. Basic CRUD operations](../textbook/01-tom-i-osnovy.md)

## Further reading

- [First steps](first-steps.md) — the minimal env → txn → dbi → get/put flow.
- [Durability modes](durability-modes.md) — trade-offs between durability and speed.
