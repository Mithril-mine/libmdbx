# API reference

The C and C++ API reference is generated from the source annotations
(`mdbx.h`, `mdbx.h++`) by Doxygen and is available in English only: a Russian
translation does not exist yet. The API annotations are the single source of
truth.

## C API by modules

The C API is organized into documentation groups ("modules"); each module
page below carries the full documentation of its functions, macros and
types. The [C API overview](api/group__c__api.md) holds the core handles
(`MDBX_env`, `MDBX_val`, ...) and common definitions.

- [C API](api/group__c__api.md) — core handles and common definitions
- [Opening & Closing](api/group__c__opening.md) — environment
  create/open/close: `mdbx_env_create()`, `mdbx_env_open()`, ...
- [Transactions](api/group__c__transactions.md) — `mdbx_txn_begin()`,
  `mdbx_txn_commit()`, ...
- [Tables](api/group__c__dbi.md) — table (DBI) handles:
  `mdbx_dbi_open()`, `mdbx_map_handle()`, ...
- [Create/Read/Update/Delete](api/group__c__crud.md) — `mdbx_put()`,
  `mdbx_get()`, `mdbx_replace()`, `mdbx_del()`, ...
- [Cursors](api/group__c__cursors.md) — cursor handling and operations:
  `mdbx_cursor_open()`, `mdbx_cursor_get()`, ...
- [Statistics & Information](api/group__c__statinfo.md) —
  `mdbx_env_info()`, `mdbx_env_stat()`, `mdbx_dbi_stat()`, ...
- [Settings](api/group__c__settings.md) — geometry and environment
  options: `mdbx_env_set_geometry()`, ...
- [SYNC MODES](api/group__sync__modes.md) — durability flags and their
  LMDB-migration notes
- [Range query estimation](api/group__c__rqest.md) — `mdbx_estimate_*()`
  cost/size hints
- [Extra operations](api/group__c__extra.md) — copying, paths, reader
  checks, and the rest of the odds and ends
- [Checking and Recovery](api/group__chk.md) — `mdbx_chk`-style internal
  API for low-level inspection
- [Logging and runtime debug](api/group__c__debug.md) —
  `mdbx_debug_flags()`, `mdbx_logger()`, ...
- [Common Macros](api/group__api__macros.md) — feature test and helper
  macros
- [Value-to-Key](api/group__value2key.md) /
  [Key-to-Value](api/group__key2value.md) — `mdbx_to_key()` /
  `mdbx_from_key()` helpers to avoid custom comparators
- [Error handling](api/group__c__err.md) — error codes and messages:
  `mdbx_strerror()`, `mdbx_liberr2str()`, ...

## C++ API by modules

- [C++ API](api/group__cxx__api.md) — the umbrella module of the
  `mdbx` namespace
- [Core](api/group__cxx__core.md) — environment, transactions, cursors,
  key-value tables and map handles
- [Slices and Buffers](api/group__cxx__data.md) — `mdbx::slice`,
  `mdbx::buffer`, ... and the value-to-key/key-to-value codecs
- [Exceptions and Errors](api/group__cxx__exceptions.md) —
  the `mdbx::exception` hierarchy and error-to-exception mapping

## Core types at a glance

C: the opaque handles `MDBX_env`, `MDBX_txn`, `MDBX_cursor`, `MDBX_dbi`
and `MDBX_val` are described on the [C API page](api/group__c__api.md);
the functions manipulating them live in the modules above.

C++:

- Environments: [`mdbx::env`](api/classmdbx_1_1env.md) /
  [`mdbx::env_managed`](api/classmdbx_1_1env__managed.md)
- Transactions: [`mdbx::txn`](api/classmdbx_1_1txn.md) /
  [`mdbx::txn_managed`](api/classmdbx_1_1txn__managed.md)
- Cursors: [`mdbx::cursor`](api/classmdbx_1_1cursor.md) /
  [`mdbx::cursor_managed`](api/classmdbx_1_1cursor__managed.md)

## Entry points

- [Annotated index](api/annotated.md) — all classes and structures
- [Classes](api/classes.md) · [Class hierarchy](api/hierarchy.md)
- [Namespaces](api/namespaces.md)
- [Modules](api/modules.md) — the complete list of documentation groups
  (the same modules as listed above)
- [Class members](api/class_members.md) · [Macros](api/macros.md)
- [File list](api/files.md) — the three headers with their structures and
  macros; the function documentation lives in the module pages above
- [Deprecated list](api/deprecated.md)

The headers themselves, with the complete inline documentation:

- [The `mdbx.h` C API header](api/mdbx_8h.md)
- [The `mdbx.h++` C++ API header](api/mdbx_8h_09_09.md)

Also in this section: [command-line tools](tooling.md).

Getting-started guidance lives in the [guides](../index.md);
the [first steps](../getting-started/first-steps.md) page shows the canonical
env → transaction → DBI → get/put flow with runnable examples.
