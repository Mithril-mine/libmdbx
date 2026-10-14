## Examples

This set of examples will be updated, and most of the examples are also publicly available tests.

### C API
The [`example-mdbx.c`](example-mdbx.c) is the example of using the C API.
However, it is strongly recommended to use the modern [C++ API](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__cxx__api.html), which requires less effort
and ensures against many bugs related to resource leaks.

### Berkeley DB
If you have already used Berkeley DB, then it will be useful to make a line-by-line comparison of [`example-mdbx.c`](example-mdbx.c) and the [`sample-bdb.txt`](sample-bdb.txt) file.

---

## Textbook examples (docs/{ru,en}/textbook)

Scenarios S01–S47 from the libmdbx textbook (`docs/ru/textbook/`), one
self-contained program per scenario, plus the end-to-end `config-store` project
(growing slices 02–12, one per key chapter). The C++17 versions are primary;
a C11 version exists where a C port makes sense. Each program:

- exits `EXIT_SUCCESS` (0) only on a fully successful run and prints a final
  `ok: ...` line to stdout;
- prints any diagnostic to stderr and exits `EXIT_FAILURE` (1) on failure;
- handles expected error codes (`MDBX_KEYEXIST`, `MDBX_NOTFOUND`, `MDBX_BUSY`,
  ...) explicitly instead of aborting;
- uses the shared helpers only: `common/common.h++` and `common/common.h`
  (`common/common.c`).

### Scenario → textbook section

| # | File | Volume / chapter / section |
| - | ---- | -------------------------- |
| S01 | `c++/01-hello.c++` (+ C) | I / ch. 2 «Установка и первый запуск», §2.4 «Минимальный пример» |
| S02 | `c++/02-data-model.c++` (+ C) | I / ch. 3 «Базовая модель данных», §3.2–§3.5 |
| S03 | `c++/03-crud.c++` (+ C) | I / ch. 4 «Базовые операции CRUD», §4.2–§4.3 |
| S04 | `c++/04-transactions.c++` (+ C) | I / ch. 5 «Транзакции — первые шаги», §5.2, §5.4 |
| S05 | `c++/05-cursors.c++` (+ C) | II / ch. 6 «Курсоры», §6.2, §6.4 |
| S06 | `c++/06-dupsort-delete.c++` (+ C) | II / ch. 6 «Курсоры», §6.6 |
| S07 | `c++/07-dupsort.c++` (+ C) | II / ch. 7 «Мультизначения и DUPSORT», §7.2, §7.4 |
| S08 | `c++/08-inverted-index.c++` (+ C) | II / ch. 7 «Мультизначения и DUPSORT», §7.5 |
| S09 | `c++/09-secondary-index.c++` | II / ch. 8 «Вторичные индексы», §8.2 |
| S10 | `c++/10-composite-key.c++` | II / ch. 8 «Вторичные индексы», §8.5 |
| S11 | `c++/11-geometry-options.c++` (+ C) | II / ch. 9 «Конфигурация окружения», §9.2, §9.4 |
| S12 | `c++/12-env-stat.c++` (+ C) | II / ch. 9 «Конфигурация окружения», §9.6 |
| S13 | `c++/13-sync-modes.c++` (+ C) | II / ch. 10 «Режимы долговечности», §10.2, §10.4 |
| S14 | `c++/14-multithreading.c++` | II / ch. 11 «Многопоточность», §11.2, §11.4 |
| S15 | `c++/15-fork-resurrect.c++` | II / ch. 11 «Многопоточность», §11.5 |
| S16 | `c++/16-parking.c++` | II / ch. 11 «Многопоточность», §11.6 |
| S17 | `c++/17-error-handling.c++` (+ C) | II / ch. 12 «Обработка ошибок», §12.2, §12.4 |
| S18 | `c++/18-tree-height.c++` (+ C) | III / ch. 13 «Архитектура хранения», §13.4 |
| S19 | `c++/19-readers-lag.c++` | III / ch. 14 «MVCC и снапшоты», §14.5–§14.6 |
| S20 | `c++/20-commit-latency.c++` | III / ch. 15 «CoW и конвейер коммита», §15.5 |
| S21 | `c++/21-gc-observe.c++` | III / ch. 16 «GC», §16.5 |
| S22 | `c++/22-gc-limits.c++` | III / ch. 16 «GC», §16.6 |
| S23 | `c++/23-map-full.c++` | III / ch. 17 «Рост, сжатие и дефрагментация», §17.3 |
| S24 | `c++/24-defrag.c++` | III / ch. 17 «Рост, сжатие и дефрагментация», §17.5 |
| S25 | `c++/25-nested-txn.c++` | III / ch. 18 «Вложенные транзакции», §18.2–§18.3 |
| S26 | `c++/26-two-processes.c++` | III / ch. 19 «Файл блокировок», §19.2–§19.3 |
| S27 | `c++/27-recovery.c++` | III / ch. 20 «Долговечность и восстановление», §20.4–§20.5 |
| S28 | `c++/28-waf-batching.c++` | IV / ch. 21 «WAF», §21.5–§21.6 |
| S29 | `c++/29-scenario-configs.c++` | IV / ch. 22 «Выбор конфигурации под сценарий» |
| S30 | `c++/30-append-prefault.c++` | IV / ch. 23 «Микрооптимизации», §23.3–§23.4 |
| S31 | `c++/31-get-cached.c++` | IV / ch. 24 «Кэш-поиск (get-cached)», §24.2, §24.4 |
| S32 | `c++/32-bulk-ops.c++` | IV / ch. 25 «Массовые операции», §25.2–§25.4 |
| S33 | `c++/33-profiler.c++` | IV / ch. 26 «Бенчмарки и измерения», §26.3–§26.4 |
| S34 | `c++/34-rules-counter.c++` | V / ch. 27 «Правила и советы», §27.1–§27.4 |
| S35 | `c++/35-platform-notes.c++` | V / ch. 28 «Платформенные нюансы», §28.2–§28.3 |
| S36 | `c++/36-hsr.c++` | V / ch. 29 «HSR», §29.3–§29.4 |
| S37 | `c++/37-diagnostics.c++` | V / ch. 30 «Диагностика», §30.2, §30.4 |
| S38 | `c/38-migration.c` | V / ch. 31 «Миграция с LMDB», §31.4 |
| S39 | `c++/39-pattern-sequence-id.c++` | V / ch. 32 «Паттерны проектирования», §32.1 |
| S40 | `c++/40-pattern-secondary-index.c++` | V / ch. 32 «Паттерны проектирования», §32.2 |
| S41 | `c++/41-pattern-composite-key.c++` | V / ch. 32 «Паттерны проектирования», §32.3 |
| S42 | `c++/42-pattern-queue.c++` | V / ch. 32 «Паттерны проектирования», §32.4 |
| S43 | `c++/43-pattern-ring-buffer.c++` | V / ch. 32 «Паттерны проектирования», §32.5 |
| S44 | `c++/44-pattern-full-scan.c++` | V / ch. 32 «Паттерны проектирования», §32.6 |
| S45 | `c++/45-pattern-replication.c++` | V / ch. 32 «Паттерны проектирования», §32.7 |
| S46 | `c++/46-pattern-read-your-writes.c++` | V / ch. 32 «Паттерны проектирования», §32.8 |
| S47 | `c++/47-cpp-api.c++` | VI / ch. 40 «C++ API» |

"(+ C)" marks scenarios that also ship a C11 version in `c/`. The `config-store`
project is described in [`config-store/README.md`](config-store/README.md).

### Building & running

Via CMake (all examples are registered as CTest tests; run them with):

```sh
cmake -S . -B build -DMDBX_BUILD_CXX=ON -DMDBX_ENABLE_TESTS=ON
cmake --build build --target mdbx_ex_01-hello            # one example
ctest --test-dir build -R '^mdbx_(ex_|config_store_)'    # all textbook examples
```

Or compile a single example by hand (the exact command is in the title comment
of each file):

```sh
# C++ (helpers are header-only for C++)
g++ -std=c++17 -I<libmdbx-include> -Iexamples/common examples/c++/01-hello.c++ -lmdbx -o 01-hello
./01-hello

# C (requires common/common.c)
gcc -std=c11 -I<libmdbx-include> -Iexamples/common \
    examples/c/01-hello.c examples/common/common.c -lmdbx -o 01-hello
./01-hello
```

Platform notes:
- Windows builds via MinGW are covered by the cross-compilation CI job
  (compile-only: libmdbx refuses to mmap under Wine; run on native Windows/WSL).
- Examples that involve `fork()`/process spawning and recovery (`S15`, `S26`,
  `S27`) degrade gracefully on Windows: the process-related parts are skipped
  with a message, the rest still runs.

### Localization for the docsite & textbooks

The per-locale copies in `docs/{ru,en}/textbook/examples` (see `make docsite` /
`make books`) are generated from this canonical tree by
[`docs/docsite/localize_examples.py`](../docs/docsite/localize_examples.py):

- comments marked `// EN:`/`// RU:` are filtered per locale and the marker is
  stripped (`// RU: текст` → `// текст` in the `ru` copy and vice versa);
- prose files are taken from the per-locale translations tracked next to the
  canonical ones: `README.<locale>.md` for the READMEs (shipped as
  `README.txt`) and `sample-bdb.ru.txt` for the Berkeley DB sample in the `ru`
  copy.

The canonical bilingual tree is the single source of truth — CI builds and
tests it as-is.