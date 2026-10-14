ChangeLog
=========

The source code is available on [SourceCraft](https://sourcecraft.dev/dqdkfa/libmdbx) and mirror on [GitHub](https://github.com/Mithril-mine/libmdbx).
Please use the `stable` branch or the latest release for production environments through staging, and the `master` branch for development of derivative projects.
Всё будет хорошо!

## v0.15.1 at a deep development stage

Cutting-edge release with new core features and internal refactoring.

### Important:

  - The operation of the [project website](https://libmdbx.dqdkfa.ru) has been restored after an attack by ukrainian/european bandera terrorists.
    Along with the website’s revival, the documentation has been significantly expanded, and the layout of the API reference guide is now more readable. The documentation now includes:
       - an extensive [textbook](https://libmdbx.dqdkfa.ru/docs/en/textbook/index.html) on libmdbx with about 50 examples;
       - the “deep dive” section with a description of the [architecture](https://libmdbx.dqdkfa.ru/docs/en/deep-dive/architecture.html) and [internal mechanisms](https://libmdbx.dqdkfa.ru/docs/en/deep-dive/internals.html);
       - the [“first steps”](https://libmdbx.dqdkfa.ru/docs/en/getting-started/first-steps.html) sections and the description of [improvements](https://libmdbx.dqdkfa.ru/docs/en/deep-dive/improvements.html) in comparison with LMDB.

    As a result of the attack, no source code was damaged, but the working project site, configured work environments and the infrastructure necessary for their operation were lost.
    Therefore, we had to deploy a new website from working drafts without completing the proofreading of the textbook and other new content. So work on updating the website and proofreading the documentation will continue.

  - In the `master` branch, development of version v0.15 will begin. The engineering of subsequent releases of the v0.14.x version line will continue in the `stable` branch.

### Improvements:

 - Minor clarification/refining README and doxygen API descriptions.
 - Added the `-h`/`--help` options to all the command-line utilities (`mdbx_chk`, `mdbx_copy`, `mdbx_drop`, `mdbx_dump`, `mdbx_load`, `mdbx_stat`, `mdbx_defrag`) and deduplicated their common helpers into a shared header-only module.
 - Added the man page for the `mdbx_defrag` utility and updated the other man pages.
 - Rewrote the «Tooling: command-line utilities» document with full option reference of every utility.
 - Added a new bilingual (RU+EN) textbook and integrated it into the documentation site, with the accompanying example programs ported and restructured.
 - Introduced a unified bilingual documentation site (MkDocs-based docsite), six bilingual guides and consolidated engineering documentation (architecture/deep-dive/improvements), replacing the legacy doxygen-based pipeline.

### Fixes:

 - Fixed missed propagation of a mid-commit flush error in `iov_page()` — a failed write could be silently reported as success (issue #47).
 - Fixed Windows ioring corruption when committing large write-mapped durable transactions — could crash in `ior_wocr()` under a live APC (issue #47).
 - Fixed `mdbx_chk` crash when `-i`/`-s` filtering out named sub-tables (issue #49).
 - Fixed `mdbx_load` reading a dump from standard input by default — `-f` no longer consumes the database path argument (issue #50).
 - Fixed a wrong `env->incore` assertion in `gc_alloc_ex()` (issue #52).
 - Fixed an arithmetic overflow in `reasonable_db_maxsize()` on 32-bit builds on 64-bit machines with large RAM.
 - Fixed an off-by-one in `rkl_push()` while rolling back a duplicate insertion: the tail of the internal list was lost, making the structure invalid. The defect was unreachable in current usage scenarios, but could be triggered by specific crafted internal sequences.
 - Fixed `MDBX_TXN_TRY` leakage into transaction state flags.
 - Fixed typos in the ChangeLog.

--------------------------------------------------------------------------------

## v0.15.0 at 2026-09-22

Not a release, but a technical tag marking the start of development for a new version branch.

The engineering of subsequent releases of the v0.14.x version line will continue in the `stable` branch.
In the `master` branch, development of version v0.15 will begin, with the technical version tag set to v0.15.0.

********************************************************************************

For early releases and changes see the ChangeLog-NN the git commit history.
