# libmdbx documentation

**libmdbx** is an embedded transactional key-value database: compact, extremely
fast (memory-mapped, B+tree, O(log N)), ACID, crash-safe by default, with no WAL
and no server process — safe for concurrent multi-process access.

Choose a section:

- **[Overview](overview/index.md)** — what libmdbx is, its characteristics,
  comparison with alternatives, and when to choose it (plus the honest list of
  [restrictions and gotchas](overview/restrictions.md)).
- **[Getting started](getting-started/install-build.md)** — forms of delivery, how to
  build, [first steps](getting-started/first-steps.md) with the API, and
  [durability modes](getting-started/durability-modes.md).
- **[Textbook](textbook/index.md)** — a practical guide in six volumes,
  from the first steps to expert topics and bindings.
- **[Deep dive](deep-dive/architecture.md)** — the architecture, internal
  mechanisms, and what libmdbx improves over LMDB.
- **[Reference](reference/index.md)** — the C/C++ API reference, command-line tools,
  and the change log.

The documentation is available in English and Russian — use the language pill
in the top-right corner of any page to switch.
