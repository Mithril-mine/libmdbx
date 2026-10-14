# Tooling: command-line utilities

> Related: [First steps](../getting-started/first-steps.md) ·
> [Durability modes](../getting-started/durability-modes.md) ·
> [API reference](../reference/index.md)

The amalgamated package includes utilities for database administration and
diagnostics (build with `MDBX_BUILD_TOOLS=ON` or `make mdbx_chk` etc.).

Every utility prints a version banner with `-V`, a usage message with
`-h`/`--help`, and exits with status `0` on success and a non-zero status on
failure. Most of them interrupt gracefully on `SIGINT`/`SIGTERM` (Ctrl-C).

## mdbx_chk — integrity check

Validates the database structure: the B+tree, GC consistency, key ordering,
page accounting. Recommended:

- after crashes while debugging your own integration;
- periodically — as part of operational monitoring;
- after restoring from a backup.

By default the environment is opened read-only in exclusive/monopolistic
mode; if it is busy, the tool falls back to cooperative mode.

```sh
mdbx_chk database.mdb          # basic read-only check
mdbx_chk -vv database.mdb      # verbose
mdbx_chk -w database.mdb       # read-write mode: reconciles fixable issues
```

| Option | Description |
| --- | --- |
| `-V` | Print the library version and build information, exit. |
| `-v` | More verbose output; repeatable up to 9 times (`-vv`, `-vvv`, ...). |
| `-q` | Quiet mode. |
| `-c` | Force cooperative mode; do not try to open exclusively. Only exclusive mode allows a complete check of all meta-pages and the actual database file size. |
| `-w` | Open read-write and lock for writing. Allows rollback to the last steady commit and checking meta-page transaction IDs. |
| `-d` | Disable page-by-page B-tree traversal (lost/double-used pages then are not detected). |
| `-i` | Ignore wrong order errors (may false-positive with custom comparators). |
| `-s table` | Check and show info only for a specific table. |
| `-0`/`-1`/`-2` | Use a specific meta-page for checking. |
| `-t` | Turn to the specified meta-page on a successful check. |
| `-T` | Turn to the specified meta-page even on an unsuccessful check. |
| `-u` | Warm up the database before checking. |
| `-U` | Warm up and force lock database pages in memory before checking. |
| `-n` | Legacy no-op (single-file environments are detected automatically). |
| `-h`/`--help` | Print a usage message and exit. |

`mdbx_chk` distinguishes problem classes via the exit status:

| Code | Meaning |
| --- | --- |
| `0` | no errors detected |
| `1` | minor problems (neither meta, nor KV/GC related) |
| `2` | major problems (meta-page, key-value or GC issues) |
| `3` | libmdbx error |
| `4` | system error |
| `5` | interrupted by the user or invalid usage |

## mdbx_copy — hot copying

A consistent copy of the database **without stopping writers** — an ordered
page walk over the snapshot. A plain file copy (`cp`) during active writes
gives no such guarantee. The destination may be a file/directory path or
standard output. Note: copying in parallel with active writers may grow the
source file meanwhile.

```sh
mdbx_copy source.mdb backup.mdb
mdbx_copy -c source.mdb backup.mdb  # compact: skip unused pages
mdbx_copy source.mdb > backup.mdb   # dump to stdout
```

| Option | Description |
| --- | --- |
| `-V` | Print the library version and build information, exit. |
| `-q` | Quiet mode. |
| `-c` | Compact while copying: copy only current data pages, omit freed/unused ones. Slower and more CPU-intensive; currently fails if the environment suffered a page leak. |
| `-f` | Silently overwrite the target if it exists instead of erroring. |
| `-d` | Enforce the copy to be a dynamic-size database (grow/shrink by reasonable steps on the fly). |
| `-p` | Use read-transaction parking/ousting while copying the MVCC-snapshot to avoid stopping recycling and overflowing the database. |
| `-u` | Warm up the database before copying. |
| `-U` | Warm up and force lock database pages in memory before copying. |
| `-n` | Legacy option; single-file environments are detected automatically. |
| `-h`/`--help` | Print a usage message and exit. |

The programmatic counterpart — `mdbx_env_copy2...()` (the C API).

## mdbx_dump / mdbx_load — dump and load

A text (or unicode-escaped) dump of the database content → migration between
platforms, content diagnostics, recovery:

```sh
mdbx_dump database.mdb > dump.txt
mdbx_load new.mdb < dump.txt
```

Important: databases dumped/loaded with custom comparison functions lose them
on reload — the resulting database uses the default comparison, which most
likely corrupts the data. Such databases must be loaded with `-a`.

### mdbx_dump

| Option | Description |
| --- | --- |
| `-V` | Print the library version and build information, exit. |
| `-q` | Quiet mode. |
| `-c` | Concise mode without repeating keys; incompatible with Berkeley DB and LMDB. |
| `-f file` | Write to the specified file instead of stdout. |
| `-l` | List the tables in the environment and exit (no data output). |
| `-p` | Output printable characters directly (portable-text subset; less portable across systems). |
| `-a` | Dump all tables in the environment. |
| `-s table` | Dump only the specified table (usually the main table). |
| `-r` | Rescue mode: ignore some errors to dump a corrupted database. |
| `-u` | Warm up the database before dumping. |
| `-U` | Warm up and force lock database pages in memory before dumping. |
| `-n` | Legacy option; single-file environments are detected automatically. |
| `-h`/`--help` | Print a usage message and exit. |

### mdbx_load

| Option | Description |
| --- | --- |
| `-V` | Print the library version and build information, exit. |
| `-q` | Quiet mode. |
| `-a` | Append all records in input order; no sorting or duplicate checking. Required for reloading data dumped from databases with custom comparators. |
| `-b number` | Insertion batch size in items (100K by default). |
| `-L megabytes` | Limit a single transaction to the given size in megabytes. |
| `-d percent` | Desired pages filling density, 50..100 (100 by default). |
| `-G L:U:G:S:P` | Override database geometry: lower size, upper size, growth step, shrink threshold, page size (all in bytes). |
| `-f file` | Read from the specified file instead of stdin. |
| `-s table` | Load into the specified table (main table by default). |
| `-N` | Do not overwrite existing records, skip them. |
| `-p` | Purge the target table(s) before loading. |
| `-T` | Load plain text: paired lines (key line, then data line). |
| `-r` | Rescue mode: ignore errors to load a corrupted dump. |
| `-n` | Legacy option; single-file environments are detected automatically. |
| `-h`/`--help` | Print a usage message and exit. |

## mdbx_stat — statistics

Statistics of the environment, tables, transactions and readers: sizes,
page/item counters, the enumeration of active readers (useful when
diagnosing "long-lived readers" — see
[restrictions](../overview/restrictions.md)).

```sh
mdbx_stat database.mdb
mdbx_stat -a -e database.mdb     # env info + all tables
```

| Option | Description |
| --- | --- |
| `-V` | Print the library version and build information, exit. |
| `-q` | Quiet mode. |
| `-p` | Page-operation statistics of the current multi-process session. |
| `-e` | Display environment info. |
| `-a` | Display the status of all tables. |
| `-s table` | Display the status of a specific table. |
| `-f` | Display GC/freelist info. `-ff` — summarize each GC entry, `-fff` — full list of page IDs. |
| `-r` | Display the reader table (pid, thread, txn id). `-rr` — additionally check and clear stale reader entries. |
| `-n` | Legacy option; single-file environments are detected automatically. |
| `-h`/`--help` | Print a usage message and exit. |

## mdbx_drop — drop a database/table

Empties or deletes a database (table) in the environment:

```sh
mdbx_drop -d database.mdb            # delete the main database
mdbx_drop -s named_table database.mdb
```

| Option | Description |
| --- | --- |
| `-V` | Print the library version and build information, exit. |
| `-q` | Quiet mode. |
| `-d` | Delete the specified database instead of just emptying it. |
| `-s table` | Operate on a specific table (main table by default). |
| `-n` | Legacy option; single-file environments are detected automatically. |
| `-h`/`--help` | Print a usage message and exit. |

## mdbx_defrag — defragmentation

Moves live pages towards the file start and truncates the tail. Usually not
required (the continuous zero-overhead compactification works on every
commit), but useful after massive deletions in older database versions.
The database remains fully usable during defragmentation; the tool may be
interrupted with Ctrl-C without corrupting the database.

```sh
mdbx_defrag database.mdb
mdbx_defrag -t 600 database.mdb    # limit duration to 10 minutes
```

| Option | Description |
| --- | --- |
| `-V` | Print the library version and build information, exit. |
| `-v` | More verbose output; repeatable up to 9 times. |
| `-q` | Quiet mode. |
| `-1` | Single quick defragmentation cycle without two-stage moves. |
| `-2`..`-9` | Limit the number of defragmentation cycles. |
| `-t seconds` | Limit the duration of defragmentation. |
| `-f percent` | Defragment until the given percentage of unused space is freed (1..100, default 100). |
| `-r percent` | Acceptable undefragmented residue (percent of used space, default 0). |
| `-s megabytes` | Preferred defragmentation step/transaction size. |
| `-c` | Force cooperative mode; do not try to open exclusively. |
| `-u` | Warm up the database before defragmentation. |
| `-U` | Warm up and force lock database pages in memory before defragmentation. |
| `-h`/`--help` | Print a usage message and exit. |

## General recommendations

- All the utilities are safe to run against a **copy** for diagnostics.
- `mdbx_chk -V` shows the build options (`options:` string) — with
  container/cross-host usage the string must match (see
  [Installation and building](../getting-started/install-build.md#containers)).
- For automation — the exit codes reflect the operation status; the verbose
  output is controlled by the verbosity options.
