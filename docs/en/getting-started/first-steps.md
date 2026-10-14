# First steps

> Related: [Installation and building](install-build.md) ·
> [Tooling](../reference/tooling.md) · [Restrictions](../overview/restrictions.md) ·
> [Textbook: Volume I "Fundamentals"](../textbook/01-tom-i-osnovy.md) ·
> [C API reference](../reference/api/group__c__api.md)

Everything starts with an **environment** (`MDBX_env`): `mdbx_env_create()` →
`mdbx_env_open()` → work → `mdbx_env_close()`. A non-zero `mode` argument of
`mdbx_env_open()` allows creating the database and the directory when they do
not exist and specifies the file mode bits for the new files.

Two files appear in the directory: the **lock file** (LCK) and the **storage
file** (DXB). If you do not want a directory — the `MDBX_NOSUBDIR` option: the
given path is used directly as the data file, with a `-lck`-suffixed file next
to it for locking.

## A transaction

Within the environment a **transaction** is created (`mdbx_txn_begin()`):
read-write or read-only; write transactions may be nested. A transaction must
be used by one thread at a time. Transactions are always required, even for
read-only access — they provide the consistent view of the data.

## A table

Within the transaction a **table** is opened (`mdbx_dbi_open()`) — a key-value
space inside the environment. If only one table will ever be used, a `NULL`
name may be passed. For named databases the `MDBX_CREATE` flag creates the
table when it does not exist, and `mdbx_env_set_maxdbs()` must be called after
`mdbx_env_create()` and before `mdbx_env_open()` to set the budget of named
tables.

> A single transaction can open multiple tables. Generally tables should be
> opened once, by the first transaction of the process.

## Reading and writing

Inside the transaction `mdbx_get()` and `mdbx_put()` work with key-value
pairs. A pair is two `MDBX_val` structures (similar to POSIX `struct iovec`):
`iov_base` (the data pointer) and `iov_len` (the length). Unlike LMDB, libmdbx
supports zero-length keys and values.

Since libmdbx is very efficient (usually zero-copy), the data returned in an
`MDBX_val` may be memory-mapped straight from disk: **look but do not touch**
(and do not `free()`). Once the transaction is closed the values can no longer
be used — copy them if you need to keep them.

## Cursors

For more powerful things — a **cursor** (`mdbx_cursor_open()` within the
transaction): `mdbx_cursor_get()`, `mdbx_cursor_put()`, `mdbx_cursor_del()`.

`mdbx_cursor_get()` positions itself by the requested operation (and, for some
of them, by the key): to list all pairs — `MDBX_FIRST`, then `MDBX_NEXT` until
the end; to retrieve all keys starting from a given one — `MDBX_SET`. The full
operation list — the
[reference](../reference/api/group__c__crud.md).

`mdbx_cursor_put()` either positions the cursor by the key itself, or you use
the `MDBX_CURRENT` operation for the current position (the key must then match
the current position's key).

## Commit and rollback

A transaction is committed by `mdbx_txn_commit()` or entirely discarded by
`mdbx_txn_abort()`.

> Important (a difference from LMDB): opened cursors **can be reused and must
> be closed explicitly** — regardless of whether they were opened within a
> read-only or a write transaction. This removes ambiguity and the whole class
> of use-after-free/double-free errors.

> Important (a difference from LMDB): handles of opened tables become
> immediately available to other transactions — regardless of whether the
> transaction will be aborted or reset.

Multiple read-only transactions may be active simultaneously, but only one can
write: further attempts to begin a write transaction block until the current
one is committed or aborted. Reads are unaffected.

## Threads and processes

- **Do not open a database twice in the same process** — libmdbx tracks and
  prevents this. Share the opened environment across all threads. The LMDB
  compatible escape hatch (multi-open) is the `MDBX_DBG_LEGACY_MULTIOPEN`
  option via `mdbx_setup_debug()` before calling other functions; lock
  recovery may cause pauses.
- **Do not use the environment in a child after `fork()`** — libmdbx checks
  this at critical points. If needed —
  `mdbx_env_resurrect_after_fork()`.
- **No more than one transaction per thread.** Violations are reported with
  `MDBX_TXN_OVERLAPPING`, `MDBX_BAD_RSLOT`, `MDBX_BUSY` — unless the
  `MDBX_NOSTICKYTHREADS` option is set (transaction hand-off between threads;
  with it you must know exactly what you are doing). A write transaction must
  be committed/aborted in the thread that started it — otherwise
  `MDBX_THREAD_MISMATCH`.

## Duplicate keys (multimaps)

`mdbx_get()` returns only the first value of a key. For multiple values open
the table with `MDBX_DUPSORT`: then `mdbx_put()` adds the value to the key
(instead of replacing), `mdbx_del()` honors the value field (point-wise
deletion), and cursors gain extra operations for traversing duplicates.

## Read optimization

If read-only transactions are frequently begun and aborted — use
`mdbx_txn_reset()` (releases the old data copies) + `mdbx_txn_renew()`
(resumption); cursors — `mdbx_cursor_renew()`/`mdbx_cursor_close()`. To
permanently finish a transaction — `mdbx_txn_abort()`.

## Cleaning up

All created cursors are closed with `mdbx_cursor_close()` (see the important
note above). Table handles are usually left open: closing makes them
unavailable to all transactions of the environment — do not close a handle
while at least one transaction is using it.

## Where to go next

- Database size management: `mdbx_env_set_geometry()`.
- Bulk loading: `MDBX_MULTIPLE`, `MDBX_APPEND`.
- LIFO reclaiming speedup on write-back storage: `MDBX_LIFORECLAIM`.
- Range query result estimation: `mdbx_estimate_*`.
- Resolving a "database full" caused by slow readers: `mdbx_env_set_hsr()`.
- Sequences and canary markers: `mdbx_dbi_sequence()`, `MDBX_canary`.
- [Durability modes](durability-modes.md) — the reliability/performance
  trade-offs.
- [Textbook: Volume I](../textbook/01-tom-i-osnovy.md) — the systematic
  exposition.
