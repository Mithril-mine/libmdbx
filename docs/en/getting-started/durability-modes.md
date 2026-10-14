# Durability and synchronization modes

> Related: [Installation and building](install-build.md) ·
> [Restrictions](../overview/restrictions.md) ·
> [Textbook: Volume IV "Performance"](../textbook/04-tom-iv-proizvoditelnost.md)

_libmdbx_ offers a set of trade-offs between durability and write performance,
selected per environment. The default mode is the most reliable one.

## The modes table

| Mode | DB data synced | Meta page synced | Survives OS crash | Survives power loss |
| --- | --- | --- | --- | --- |
| `MDBX_DURABLE` (default) | each commit | each commit | yes | yes |
| `MDBX_NOMETASYNC` | each commit | no | yes | last commits may roll back |
| `MDBX_SAFE_NOSYNC` | lazy (thresholds/timeout) | lazy | yes | recent commits may be lost, integrity guaranteed |
| `MDBX_UTTERLY_NOSYNC` | no explicit sync | no | no | possible corruption (matches LMDB `MDB_NOSYNC`) |

The exact semantics — the
[API reference](../reference/api/group__c__opening.md).

## The key differences

- **`MDBX_DURABLE`** — full durability: every commit syncs both the data and
  the meta pages. The price is an fsync per commit; on slow storage this is
  the bottleneck of write-intensive workloads (shadow paging instead of WAL:
  the write volume per commit is proportional to the changed pages).
- **`MDBX_NOMETASYNC`** — data is synced, the meta is not: after a power loss
  the system rolls back to the last fully committed steady state; integrity
  is preserved.
- **`MDBX_SAFE_NOSYNC`** — libmdbx syncs lazily by itself (thresholds and
  timeouts). Integrity is guaranteed even after an OS crash and a power loss
  (unlike LMDB `MDB_NOSYNC`), but a few recent commits may be lost. Ideal for
  append-like workloads.
- **`MDBX_UTTERLY_NOSYNC`** — maximum speed, minimum guarantees: only for
  non-critical data (caches, temporary sets).

## Automatic synchronization

For `MDBX_SAFE_NOSYNC`/`MDBX_NOMETASYNC` the thresholds of the automatic
steady flush are configurable: `mdbx_env_set_syncbytes()` (by the written
volume) and `mdbx_env_set_syncperiod()` (by timeout); the current thresholds
are visible in `mdbx_env_info_ex`. The asynchronous `mdbx_env_sync_ex()` /
`mdbx_env_sync_poll()` let the application initiate a flush at a convenient
moment.

## WRITEMAP

`MDBX_WRITEMAP` maps the data pages and writes through the mmap (`msync`
instead of `pwrite`) — it reduces data copying at the cost of more
memory-mapped I/O; it combines with the modes above (typically with
`MDBX_NOMETASYNC`). Related options: `MDBX_opt_prefault_write_enable`
(prefault writing), `MDBX_opt_writethrough_threshold` (synchronous writes vs
write-then-fdatasync).

## Platform notes

- **macOS/iOS**: `fcntl(F_FULLFSYNC)` is used by default — the only way to
  guarantee durability across a power failure; in write-heavy scenarios this
  noticeably degrades performance versus LMDB (a plain `fsync()`). Override —
  the `MDBX_OSX_SPEED_INSTEADOF_DURABILITY=1` build option.
- **Windows**: `LockFileEx()` is used for locking — it works on network drives
  and protects against misuse (poka-yoke); speed benchmarks may lag behind
  LMDB with its named mutexes.

## Recommendations

- Production by default — `MDBX_DURABLE`.
- Logs, telemetry, caches tolerating the loss of the last records —
  `MDBX_SAFE_NOSYNC` (+ the auto-sync thresholds).
- Never use `MDBX_UTTERLY_NOSYNC` for valuable data.
- Do not mix modes on the same environment across processes.
