ChangeLog
=========

The source code is available on [SourceCraft](https://sourcecraft.dev/dqdkfa/libmdbx) and mirror on [GitHub](https://github.com/Mithril-mine/libmdbx).
Please use the `stable` branch or the latest release for production environments through staging, and the `master` branch for development of derivative projects.
Всё будет хорошо!

## v0.15.1 at a deep development stage

### Improvements:

 - Minor clarification/refining README and doxygen API descriptons.

### Fixes:

 - Fixed wrong `env->incore` assertion in `gc_alloc_ex()`: the asserted state (steady point not advanced by `dxb_sync_locked()`) is legitimate on any filesystem, and on Windows `osal_check_fs_incore()` always reports not-in-core, so the assertion could never hold. It is removed and allocation falls through to the unallocated part of the file (issue #52).
 - Fixed the missed propagation of a mid-commit flush error in `iov_page()` on all platforms — a failed `iov_write()` previously reported `MDBX_SUCCESS` while the dirty page was never queued for writing, with a risk of silent data loss.
 - Fixed Windows ioring corruption when committing large write-mapped durable transactions: gather segments were appended to a `WriteFileEx` single item and outstanding `WriteFileEx` writes in a mixed batch were not awaited before reading `STATUS_PENDING` as an error, which could reset the ring under a live APC and crash in `ior_wocr()` with `hEvent == NULL`.
 - Fixed typos in the ChangeLog.
 - Fixed `MDBX_TXN_TRY` leakage into a transaction state flags.

--------------------------------------------------------------------------------

## v0.15.0 at 2026-09-22

Not a release, but a technical tag marking the start of development for a new version branch.

### Important:

The development of subsequent releases in the v0.14.x version line will continue in the `stable` branch.
In the `master` branch, development of version v0.15 will begin, with the technical version tag set to v0.15.0.

********************************************************************************

For early releases and changes see the ChangeLog-NN the git commit history.
