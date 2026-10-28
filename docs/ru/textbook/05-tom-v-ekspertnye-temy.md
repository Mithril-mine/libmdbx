# Том V. Экспертные темы и edge cases

> **Уровень:** для продвинутых пользователей.
> **Цель тома:** вы знаете тонкости и грабли, умеете отлаживать проблемы, принимать архитектурные
> решения, мигрировать с LMDB и строить устойчивые паттерны.

---

## Глава 27. Правила и советы из базы знаний

Полный перечень правил и советов — в базе знаний (`knowledge-base/rules-and-checklist/`,
`knowledge-base/tips/`). Здесь — группировка по темам с акцентами критичности.

### 27.1. Транзакции и потоки (критично)

- **Одна транзакция — один поток.** Передача объекта транзакции другому потоку — UB/ошибка
  (`MDBX_THREAD_MISMATCH`).
- **Не передавайте транзакцию между потоками** даже с `NOSTICKYTHREADS`, если другой поток может
  синхронно вызвать write-функцию среды — deadlock.
- **Не используйте объекты транзакций из двух потоков одновременно.**
- После `commit`/`abort` указатели на данные в mmap недействительны.

### 27.2. Жизненный цикл и целостность

- Не открывайте БД дважды в одном процессе; `fork()` — только с `resurrect_after_fork` в наследнике.
- Незакрытая read-транзакция «морозит» переработку страниц — закрывайте/паркуйте.
- Никогда не копируйте файл базы «на лету» — используйте `mdbx_copy`.
- `MDBX_UTTERLY_NOSYNC` — только для некритичных данных.

### 27.3. Пространство и рост

- Задавайте геометрию один раз до open; `upper` не занижать.
- Уменьшать `rp_augment_limit` — только вместе с `gc_time_limit`.
- Длинные значения: увеличивайте страницу (до 64 КБ), дробите записи.
- Значение, не помещающееся в одну страницу, требует прогона **подряд идущих** страниц: при
  частой перезаписи больших значений следите за фрагментацией (гистограммы `mdbx_chk`, число
  overflow-страниц в `mdbx_env_stat`), при хранении «тяжёлых» данных рассмотрите вынос из БД.
- Если длинное значение нужно хранить в БД — разбивайте его на чанки, помещающиеся в одну
  страницу (`mdbx_limits_valsize4page_max()`): в обычной таблице — суффикс номера чанка в ключе,
  в DUPSORT-таблице — префикс номера мультизначения и номера чанка. Ручное разбиение требуется
  только до появления планируемого **потокового API для длинных значений** («Streaming BLOBs»).
  Подробности — [Ограничения и подводные камни](https://libmdbx.dqdkfa.ru/docs/ru/overview/restrictions.html).

### 27.4. Операции

- Удаления делайте до вставок в той же транзакции; массовые удаления — `bunch_delete`/`delete_range`.
- Не стройте `std::map` поверх БД ради упорядоченности — выигрыш обычно меньше затрат.
- Для DUPSORT-удаления при итерации — два курсора.

### 27.5. Типичные контрпримеры

- Нарушение «одна транзакция — один поток» → случайные `BAD_RSLOT`/пересечения.
- Открытие БД дважды → гонки и повреждение регистраций.
- Забытый читатель → рост файла до `MDBX_MAP_FULL`.
- Отладочная сборка в проде → кратные замедления.

**Фрагмент из [`examples/c++/34-rules-counter.c++`](examples/c++/34-rules-counter.c++)** — контрпример «забытого» read-снапшота: поток открыл снапшот (`env.start_read()`) и держит его (`abort()` только по внешнему флагу); пока читатель жив, писатель упирается в `MDBX_MAP_FULL` (GC заморожен, файл растёт):

```cpp
    std::atomic<bool> reader_ready{false};
    std::atomic<bool> release_reader{false};
    std::thread holder([&] {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      (void)rtxn.get(table, mdbx::slice("k0"));
      reader_ready = true;
      while (!release_reader.load())
        std::this_thread::yield();
      rtxn.abort();
    });
    while (!reader_ready.load())
      std::this_thread::yield();
```

Полный код: [34-rules-counter.c++](examples/c++/34-rules-counter.c++).

---

> **Примеры к главе:** [`examples/c++/34-rules-counter.c++`](examples/c++/34-rules-counter.c++).

### 27.6. Резюме главы 27

- Одна транзакция — один поток: передача транзакции между потоками — UB/ошибка (`MDBX_THREAD_MISMATCH`), даже с `NOSTICKYTHREADS` возможен deadlock.
- Незакрытая read-транзакция «морозит» переработку страниц — файл растёт до `MDBX_MAP_FULL`; закрывайте/паркуйте.
- Не открывайте БД дважды в одном процессе; `fork()` — только с `resurrect_after_fork` в наследнике; копирование — только `mdbx_copy`.
- `MDBX_UTTERLY_NOSYNC` в production не использовать.
- Геометрию задавайте один раз до open, `upper` не занижать; длинные значения — крупнее страница (до 64 КБ), дробите на чанки, помещающиеся в одну страницу, или выносите из БД; значения, не помещающиеся в одну страницу, требуют контигуального прогона и чувствительны к фрагментации — до появления потокового API («Streaming BLOBs»).
- Удаления — до вставок; массовые удаления — `bunch_delete`/`delete_range`; DUPSORT-удаление при итерации — два курсора.
- Типичные контрпримеры: нарушение потоковой дисциплины (`BAD_RSLOT`), двойное открытие, забытый читатель, отладочная сборка в проде.

### 27.7. Чек-лист главы 27

- [ ] Убедиться, что транзакции не передаются между потоками и не используются из двух потоков одновременно.
- [ ] Закрывать/парковать read-транзакции сразу после использования.
- [ ] Копировать файл базы только через `mdbx_copy`.
- [ ] Задать геометрию до open и не занижать `upper`.
- [ ] Исключить `MDBX_UTTERLY_NOSYNC` из production-конфигурации.
- [ ] Выполнять удаления до вставок; массовые удаления — через `bunch_delete`/`delete_range`.
- [ ] Для DUPSORT-удаления при итерации использовать два курсора.

## Глава 28. Платформенные нюансы

### 28.1. Linux

- `boot_id` учитывается при откате слабых мет; в LXC — общий/отсутствует.
- `/dev/shm` (tmpfs): создание с запасом; `ENOSPC` от `fallocate()` можно игнорировать;
  с 0.13.8 fallocate защищает от SIGBUS.
- Page cache: некогерентность unified page cache (#269) — `MDBX_FORCE_CHECK_MMAP_COHERENCY`.
- `mincore`/`madvise(MADV_NOHUGEPAGE)` — отказ от THP.
- Linux < 4.x: `mdbx_env_create()` → `MDBX_INCOMPATIBLE`.

### 28.2. Windows

- `LockFileEx` медленнее именованных мьютексов; мелкие транзакции дороги.
- 32-бит: лимиты адресного пространства; `/LARGEADDRESSAWARE` (+1 ГБ к `MAX_MAPSIZE32`).
- Сжатие файла — только в однопроцессном сценарии; расширение через Native API;
  изменение геометрии приостанавливает потоки (SRWL — Windows Slim Reader/Writer Lock,
  тонкозернистая блокировка Windows).
- WSL1: `ENOLCK` — работа принципиально невозможна.

### 28.3. macOS / iOS

- `fcntl(F_FULLFSYNC)` по умолчанию (максимум долговечности);
  `MDBX_APPLE_SPEED_INSTEADOF_DURABILITY` — жертва durability ради скорости.
- SysV-семафоры по умолчанию (flavour `SYSV`).
- `fcntl(F_PREALLOCATE)` резервирует в конец — БД растёт вдвое быстрее штатного.

### 28.4. Android / bionic

- TLS и атомики отличаются от glibc; 32-бит Android — возможное зависание при откате коммита
  (исторически; с 0.11.7 проблем нет).

### 28.5. Контейнеры

- LXC: boot_id; Docker — PID uniqueness.
- tmpfs: ENOSPC от fallocate (игнорировать).
- NFS/CIFS/SMB: только эксклюзивный режим или кооперативный read-only; копирование работает.

### 28.6. Wine

- Не реализованы механизмы динамического изменения размера БД — задавайте фиксированный
  достаточный размер; `mdbx_module_handler(...)` при запуске для статической линковки.

**Фрагмент из [`examples/c++/35-platform-notes.c++`](examples/c++/35-platform-notes.c++)** — платформенная оговорка в коде: «абсурдный» запрос геометрии обрабатывается через try/catch, потому что на tmpfs/`WSL1`/32-битной Windows `ENOSPC`/`ENOLCK` — штатная реакция, а не фатальный сбой:

```cpp
    // Проверка границ геометрии не обязана падать: запрашиваем «абсурдную»
    // верхнюю границу и аккуратно обрабатываем результат.
    mdbx::env::geometry probe;
    probe.size_upper = intptr_t(8) * mdbx::env::geometry::TB;
    try {
      env.set_geometry(probe);
      std::cout << "geometry probe: accepted\n";
    } catch (const std::exception &ex) {
      // На tmpfs/WSL1/32-битной Windows это может завершиться ошибкой —
      // для примера важно, что она перехвачена, а не уронила процесс.
      std::cout << "geometry probe: rejected (" << ex.what() << ")\n";
    }
```

Полный код: [35-platform-notes.c++](examples/c++/35-platform-notes.c++).

> **Примеры к главе:** [`examples/c++/35-platform-notes.c++`](examples/c++/35-platform-notes.c++).

### 28.7. Резюме главы 28

- Платформенные различия касаются блокировок, синхронизации и размеров адресного пространства.
- LXC/boot_id и page-cache — специфика контейнеров/Linux.
- Windows LockFileEx и F_FULLFSYNC на macOS — главные платформенные «грабли».

### 28.8. Чек-лист главы 28

- [ ] Linux: учесть `boot_id` (LXC), tmpfs `ENOSPC` от fallocate, при необходимости отказаться от THP.
- [ ] Windows: помнить о дорогих мелких транзакциях и лимитах 32-бит (`/LARGEADDRESSAWARE`).
- [ ] macOS: осознанно выбрать между `fcntl(F_FULLFSYNC)` и `MDBX_APPLE_SPEED_INSTEADOF_DURABILITY`.
- [ ] Android/bionic: проверить поведение на 32-бит (историческое зависание при откате коммита — до 0.11.7).
- [ ] Контейнеры: LXC — boot_id, Docker — PID uniqueness; NFS/CIFS/SMB — только эксклюзивный или кооперативный read-only.
- [ ] Wine: задать фиксированный достаточный размер БД (динамическое изменение размера не работает).
- [ ] Обрабатывать `ENOSPC`/`ENOLCK` на tmpfs/WSL1/32-бит Windows как штатные ошибки, а не фатальные сбои.

---

## Глава 29. Handle-Slow-Readers (HSR)

### 29.1. Что такое HSR и зачем он нужен

Когда база заполнена **из-за долгих читателей** (GC заморожен), библиотека вызывает колбэк HSR —
единственный штатный способ приложения повлиять на исход конфликта «долгий читатель против
писателя».

### 29.2. Сигнатура и параметры

```c
typedef int (*MDBX_hsr_func)(const MDBX_env *env, const MDBX_txn *txn,
                             mdbx_pid_t pid, mdbx_tid_t tid,
                             uint64_t laggard, unsigned gap,
                             size_t space, int retry);
```

- `laggard` — отставание проблемного читателя;
- `gap` — число попыток;
- `space` — объём, который освободится после завершения читателя;
- `retry` — счётчик повторных вызовов.

### 29.3. Возвращаемые значения

| Значение | Действие                                                              |
| -------- | --------------------------------------------------------------------- |
| `0`      | Колбэк подождал/решил; libmdbx пересканирует RLT и повторяет          |
| `1`      | Читательская транзакция прервана асинхронно; слот очистить немедленно |
| `2+`     | Процесс-читатель убит; libmdbx сбрасывает его регистрацию             |

Вызывается только когда база заполнена из-за читателей.

### 29.4. Типовые реализации

- **Логирование** + возврат 0 (ждать).
- **Мягкое завершение**: пометить читателю флаг, вернуть 1.
- **Принудительное**: убить процесс читателя, вернуть 2.
- **Согласие на рост**: если позволяет геометрия — увеличить `upper` и вернуть 0.

### 29.5. HSR и SAFE_NOSYNC

В `SAFE_NOSYNC` «квази-долгий читатель» — это steady-коммит, а не живой слот: HSR на него не
влияет. Управлять ростом нужно авто-sync (`syncbytes`/`syncperiod`).

**Фрагмент из [`examples/c++/36-hsr.c++`](examples/c++/36-hsr.c++)** — HSR-колбэк: сообщает о проблемном читателе (pid/lag/space), просит его освободить снапшот и ждёт фактического освобождения, после чего возвращает `MDBX_RESULT_TRUE`. Регистрация — `env.set_HandleSlowReaders(hsr_callback)`; вместе с `SAFE_NOSYNC` колбэк остаётся главным средством против долгих читателей:

```cpp
// Колбэк вызывается, когда БД «упирается» в предел из-за читателей,
// удерживающих старые снапшоты. Здесь: сообщаем о событии, просим читателя
// освободить снапшот и ждём его, после чего возвращаем MDBX_RESULT_TRUE
// («проблема устранена» — писатель продолжит).
int hsr_callback(const MDBX_env *, const MDBX_txn *, mdbx_pid_t pid, mdbx_tid_t tid, uint64_t laggard,
                 unsigned gap, size_t space, int retry) noexcept {
  std::cout << "hsr invoked: laggard pid=" << pid << " tid=" << tid << " lag=" << laggard << " gap=" << gap
            << " space=" << space << " retry=" << retry << "\n";
  hsr_called = true;
  hsr_release_reader = true;
  while (!reader_released.load())
    std::this_thread::yield(); // дождаться фактического освобождения
  return MDBX_RESULT_TRUE;
}
```

Полный код: [36-hsr.c++](examples/c++/36-hsr.c++).

### 29.6. Пример реализации

**Фрагмент (C, иллюстрация):** полная компилируемая версия — в
[`examples/c++/36-hsr.c++`](examples/c++/36-hsr.c++).

```c
static int my_hsr(const MDBX_env *env, const MDBX_txn *txn,
                  mdbx_pid_t pid, mdbx_tid_t tid,
                  uint64_t laggard, unsigned gap,
                  size_t space, int retry) {
    fprintf(stderr, "HSR: reader pid=%d lag=%llu space=%zu retry=%d\n",
            (int)pid, (unsigned long long)laggard, space, retry);
    if (retry > 3)
        return 2; /* убить проблемного читателя */
    return 0;     /* ждём ещё */
}
/* установка */
mdbx_env_set_hsr(env, my_hsr);
```

> **Примеры к главе:** [`examples/c++/36-hsr.c++`](examples/c++/36-hsr.c++).

### 29.7. Резюме главы 29

- HSR — колбэк разрешения конфликта с долгими читателями.
- Параметры: laggard/gap/space/retry; возвраты 0/1/2+.
- Типовые стратегии: ждать/убить/расти.
- С SAFE_NOSYNC управляйте ростом через авто-sync.

### 29.8. Чек-лист главы 29

- [ ] Установить HSR-колбэк (`mdbx_env_set_hsr` / `set_HandleSlowReaders`).
- [ ] Определить политику по `retry`: логирование, мягкое завершение, принудительное убийство, согласие на рост.
- [ ] Использовать параметры `laggard`/`gap`/`space`/`retry` для принятия решения в колбэке.
- [ ] Помнить семантику возвратов: 0 — ждать, 1 — прервать читателя, 2+ — убить процесс-читатель.
- [ ] При `SAFE_NOSYNC` настроить авто-sync (`syncbytes`/`syncperiod`) — HSR на steady-коммит не влияет.

---

## Глава 30. Диагностика и отладка

### 30.1. Инструменты

**`mdbx_chk`** — проверка целостности. Ключевые опции:

| Опция       | Что делает                                                                   |
| ----------- | ---------------------------------------------------------------------------- |
| `-v…-vvvvv` | Подробность вывода                                                           |
| `-q`        | Полная тишина                                                                |
| `-c`        | Кооперативный (не эксклюзивный) режим; полная проверка — только эксклюзивный |
| `-w`        | Read-write: откат к steady, проверка txnid мет                               |
| `-d`        | Постраничный обход B+tree; без него нельзя найти lost/double-used страницы   |
| `-i`        | Игнор ложных ошибок порядка (кастомные компараторы)                          |
| `-s table`  | Проверить только таблицу                                                     |
| `-0/-1/-2`  | Конкретная мета; `-t`/`-T` — переключиться на неё                            |

Exit-код: `0` = ошибок нет.

**`MDBX_ENABLE_PROFGC`** — профиль GC в `commit_latency.gc_prof`. Ключевые поля:
`max_reader_lag`, `max_retained_pages`, `work_rtime_monotonic`/`work_xtime_cpu`,
`work_rsteps`/`work_xpages`, `work_majflt`, `self_*`, `wloops`, `flushes`, `kicks`.

**`MDBX_commit_latency`** — стадии коммита: `preparation`, `gc_wallclock`, `audit`, `write`, `sync`,
`ending`, `whole`, `gc_cputime`.

**`mdbx_txn_info()`** — `txn_reader_lag`, `txn_space_used/limit_soft/limit_hard/retired/leftover/dirty`.
Для долгих читателей важны `txn_space_retired` и `txn_space_leftover`.

**Отладочная сборка** — `MDBX_DEBUG`/`MDBX_CHECKING` (−1..3); `MDBX_FORCE_ASSERTIONS` deprecated.
Санитайзеры: `ENABLE_ASAN`/`ENABLE_UBSAN`/`ENABLE_MEMCHECK`; TSAN в CMake нет (вручную).

**Фрагмент из [`examples/c++/37-diagnostics.c++`](examples/c++/37-diagnostics.c++)** — чтение `mdbx_txn_info` (через `txn.get_info(...)`: id/лаг/использованное/dirty) и списка читателей (`mdbx_reader_list`, здесь `env.enumerate_readers(...)`) — те самые данные для алгоритмов «рост БД» и `MDBX_MAP_FULL` выше:

```cpp
      // Информация о текущей пишущей транзакции.
      const auto info = txn.get_info(true /* scan_rlt */);
      std::cout << "txn_info: id=" << info.txn_id << " lag=" << info.txn_reader_lag << " used=" << info.txn_space_used
                << " dirty=" << info.txn_space_dirty << "\n";
      txn.commit();
    }

    // Список читателей (RLT).
    struct visitor {
      int operator()(const mdbx::env::reader_info &ri, int) {
        std::cout << "reader slot=" << ri.slot << " pid=" << ri.pid << " tid=" << ri.thread
                  << " txnid=" << ri.transaction_id << " lag=" << ri.transaction_lag << "\n";
        return mdbx::continue_loop;
      }
    } v;
    env.enumerate_readers(v);
```

Полный код: [37-diagnostics.c++](examples/c++/37-diagnostics.c++).

### 30.2. Пошаговый алгоритм: рост БД

1. `mdbx_env_info_ex()` — меты и геометрия (`size_now` vs `size_upper`).
2. `mdbx_reader_list()` / `mdbx_reader_check()` — кто держит снапшоты.
3. `mdbx_stat -r` — retained.
4. PROFGC: `max_reader_lag`, `max_retained_pages`, `kicks`.
5. `mdbx_stat -p` — `newly` vs `cow` (новые страницы вместо переиспользования).
6. Решение: парковка/HSR/steady-point/дефрагментация; при `SAFE_NOSYNC` — авто-sync.

### 30.3. Пошаговый алгоритм: тормоз коммита

1. `mdbx_txn_commit_ex()` — разложить `whole` на стадии.
2. `sync` доминирует → режим долговечности; слишком частые fsync → батч или `NOMETASYNC`/`SAFE_NOSYNC`.
3. `gc_wallclock` велик → PROFGC: `work_rsteps`/`work_xpages`/`work_majflt` (фрагментация/большие
   значения).
4. `write` велик при малых данных → спилл рано (dp_limit) или дублирование грязных страниц.
5. WRITEMAP + page-fault'ы → `prefault_write_enable`.
6. `mdbx_stat -p` — msync/fsync счётчики.

### 30.4. Пошаговый алгоритм: MDBX_MAP_FULL

1. **Abort текущей write-транзакции** (продолжение после resize → `BAD_TXN`).
2. `mdbx_txn_info()` — `txn_space_limit_hard` vs used.
3. `mdbx_reader_list()` — старые снапшоты пинят пространство.
4. PROFGC — `kicks`, `max_reader_lag`, `max_retained_pages`.
5. Проверить HSR (`mdbx_env_set_hsr`), парковку, геометрию (`upper` занижен?).
6. Решение: HSR (ждать/убить/расти), выселение припаркованных, новый steady, увеличение `upper`
   до open, дефрагментация.

### 30.5. Пошаговый алгоритм: deadlock / MDBX_BUSY

1. Проверить дисциплину потоков: коды `THREAD_MISMATCH`/`TXN_OVERLAPPING`/`BAD_RSLOT`.
2. При `NOSTICKYTHREADS` — исключить синхронные write-функции (`set_option/set_flags/set_geometry/
sync/stat/defrag/close`) из потока, не владеющего пишущей транзакцией.
3. Аудит `fork()` (resurrect) и повторных `env_open`.
4. `mdbx_reader_list()` покажет чужих владельцев.

### 30.6. Пошаговый алгоритм: повреждение БД

1. `mdbx_chk -w -vvv` — локализовать (мета? дерево? GC? порядок?).
2. Проверить среду: `boot_id` (LXC?), некогерентность page cache (#269, `incoherence`).
3. Повреждена одна мета → `mdbx_chk -1/-2` + `-T` (переключиться на валидную).
4. `MDBX_WANNA_RECOVERY` → read-write или `mdbx_env_open_for_recovery()` (с 0.12.7 не изменяет базу).
5. Recovery-проверки безопасны; восстановление из бэкапа (`mdbx_copy`).

### 30.7. Пошаговый алгоритм: утечка слотов читателей

1. `mdbx_reader_check(env, &dead)` — сколько мёртвых слотов.
2. Причина: потоки завершились без очистки (TLS-деструктор; glibc #21031/#21032; DSO-выгрузка).
3. Решение: явная `mdbx_thread_register/unregister`; паттерн `reset+renew`; `resurrect_after_fork`.

### 30.8. Чек-листы перед production-деплоем

- [ ] Геометрия задана до open (`upper` адекватен; движок сам выбирает дефолт ≈ золотое сечение ОЗУ).
- [ ] HSR-колбэк установлен.
- [ ] Режим синхронизации осознанно выбран.
- [ ] Release-сборка без ассертов.
- [ ] `maxreaders` покрывает число потоков.
- [ ] Долгие чтения используют парковку/reset+renew.
- [ ] Тест после сбоя: kill -9 + открытие + `mdbx_chk`.

> **Примеры к главе:** [`examples/c++/37-diagnostics.c++`](examples/c++/37-diagnostics.c++).

### 30.9. Резюме главы 30

- Инструменты: chk, PROFGC, commit_latency, txn_info, reader_check, отладочная сборка.
- 6 пошаговых алгоритмов: рост, тормоз коммита, MAP_FULL, deadlock, повреждение, утечка слотов.
- Чек-лист перед деплоем обязателен.

### 30.10. Чек-лист главы 30

- [ ] Освоить `mdbx_chk` (опции `-d`, `-w`, `-0/-1/-2`/`-T`) как основной инструмент целостности.
- [ ] Диагностировать рост БД по алгоритму §30.2 (`mdbx_reader_list`, `mdbx_stat -r`/`-p`, PROFGC).
- [ ] Тормоз коммита раскладывать по стадиям `MDBX_commit_latency` (`mdbx_txn_commit_ex`) — §30.3.
- [ ] При `MDBX_MAP_FULL` — abort транзакции, проверка читателей, HSR и геометрии — §30.4.
- [ ] Deadlock/`MDBX_BUSY` — аудит потоковой дисциплины и `NOSTICKYTHREADS` — §30.5.
- [ ] Повреждение БД — `mdbx_chk -w -vvv`, восстановление из бэкапа (`mdbx_copy`) — §30.6.
- [ ] Утечку слотов читателей проверять через `mdbx_reader_check` — §30.7.
- [ ] Пройти чек-лист production-деплоя §30.8 (включая kill -9 + открытие + `mdbx_chk`).

---

## Глава 31. Миграция с LMDB

### 31.1. Зачем мигрировать

libmdbx — переработанный LMDB: больше режимов долговечности, GC вместо free-list, жёстче
гарантии восстановления, выше производительность записи при батчинге. **Формат данных не
совместим с LMDB**: тройка мета-страниц, двухфазный коммит, контрольные суммы и GC-дерево —
собственные изменения; файлы LMDB напрямую не открываются, миграция выполняется переносом данных.

### 31.2. Совместимость форматов

- Файлы **LMDB не открываются** libmdbx напрямую: форматы разные (тройка мета-страниц,
  двухфазный коммит, контрольные суммы, GC-дерево).
- Между собой совместимы БД **libmdbx 0.11.x ↔ 0.12.x** (формат заморожен с v11.3).
- Миграция с LMDB — перенос данных (`mdbx_dump`/`mdbx_load`, `mdbx_copy`), а не
  переименование файла.

### 31.3. Breaking changes

- `MDBX_NOLOCK` убран.
- `MDBX_NOTLS` → `MDBX_NOSTICKYTHREADS`.
- Другие переименования и новые требования к флагам — сверяйте с `mdbx.h`.

### 31.4. Таблица соответствия mdb_* → mdbx_*

| LMDB              | libmdbx                |
| ----------------- | ---------------------- |
| `mdb_env_create`  | `mdbx_env_create`      |
| `mdb_env_open`    | `mdbx_env_open`        |
| `mdb_txn_begin`   | `mdbx_txn_begin`       |
| `mdb_dbi_open`    | `mdbx_dbi_open`        |
| `mdb_put/get/del` | `mdbx_put/get/del`     |
| `mdb_cursor_get`  | `mdbx_cursor_get`      |
| `MDB_NOTLS`       | `MDBX_NOSTICKYTHREADS` |

**Фрагмент из [`examples/c/38-migration.c`](examples/c/38-migration.c)** — карта соответствия `mdb_*` → `mdbx_*` из заголовка примера:

```c
// Карта макросов/имён при переносе кода с LMDB (полная таблица — в §31.4):
//   mdb_env_create      → mdbx_env_create
//   mdb_env_open        → mdbx_env_open
//   mdb_env_close       → mdbx_env_close
//   mdb_env_set_mapsize → mdbx_env_set_geometry (другой API!)
//   mdb_txn_begin       → mdbx_txn_begin
//   mdb_txn_commit      → mdbx_txn_commit
//   mdb_txn_abort       → mdbx_txn_abort
//   mdb_dbi_open        → mdbx_dbi_open
//   mdb_get/put/del     → mdbx_get/mdbx_put/mdbx_del
//   mdb_cursor_open/get → mdbx_cursor_open/mdbx_cursor_get
//   MDB_NOTLS           → MDBX_NOSTICKYTHREADS
//   MDB_NOSYNC          → MDBX_SAFE_NOSYNC или MDBX_UTTERLY_NOSYNC
//   MDB_APPEND          → MDBX_APPEND
//   MDB_INTEGERKEY      → MDBX_INTEGERKEY
//   mdb_strerror        → mdbx_strerror
```

Полный код: [38-migration.c](examples/c/38-migration.c).

### 31.5. Поведенческие отличия

- Три меты vs две; двухфазный коммит.
- Контрольные суммы страниц.
- Больше диагностических кодов (`WANNA_RECOVERY`, `MVCC_RETARDED` и др.).
- Пустые ключи/значения разрешены.

### 31.6. Чек-лист миграции

1. Сделайте бэкап (`mdbx_copy -c` или dump+load).
2. Откройте базу в read-only — проверьте `mdbx_chk`.
3. Замените вызовы по таблице соответствия.
4. Прогоните тесты под ASAN/UBSAN.
5. Настройте геометрию/режимы под новый движок (не переносите вслепую LMDB-настройки).

### 31.7. Резюме главы 31

- Формат данных libmdbx не совместим с LMDB: тройка мета-страниц, двухфазный коммит, контрольные суммы, GC-дерево; файлы LMDB напрямую не открываются.
- Миграция — перенос данных (`mdbx_dump`/`mdbx_load`, `mdbx_copy`), а не переименование файла; между собой совместимы БД 0.11.x ↔ 0.12.x (формат заморожен с v11.3).
- Breaking changes: `MDBX_NOLOCK` убран, `MDBX_NOTLS` → `MDBX_NOSTICKYTHREADS`; остальные переименования сверяйте с `mdbx.h`.
- Имена функций меняются предсказуемо (`mdb_env_create` → `mdbx_env_create`), но `mdb_env_set_mapsize` → `mdbx_env_set_geometry` — другой API.
- Поведенческие отличия: три меты vs две, контрольные суммы страниц, больше диагностических кодов, разрешены пустые ключи/значения.
- Порядок миграции: бэкап → read-only проверка `mdbx_chk` → замена вызовов → тесты под ASAN/UBSAN → настройка геометрии/режимов заново.

### 31.8. Чек-лист главы 31

- [ ] Сделать бэкап (`mdbx_copy -c` или dump+load) до любых изменений.
- [ ] Открыть базу в read-only и проверить её `mdbx_chk`.
- [ ] Заменить вызовы по таблице соответствия §31.4, включая `mdb_env_set_mapsize` → `mdbx_env_set_geometry`.
- [ ] Заменить флаги: `MDBX_NOLOCK` убран, `MDBX_NOTLS` → `MDBX_NOSTICKYTHREADS`.
- [ ] Прогнать тесты под ASAN/UBSAN.
- [ ] Настроить геометрию и режимы под libmdbx заново, не перенося LMDB-настройки вслепую.

---

> **Примеры к главе:** [`examples/c/38-migration.c`](examples/c/38-migration.c) (карта соответствия mdb_* → mdbx_* — в заголовке файла).

## Глава 32. Паттерны проектирования на libmdbx

### 32.1. Паттерн 1: Key-value с автоинкрементным ID

Для монотонных ID есть и встроенный механизм — `mdbx_dbi_sequence()` (атомарный счётчик таблицы),
и классический переносимый подход — счётчик в служебной таблице. Пример ниже демонстрирует второй
вариант: он одинаково работает во всех версиях libmdbx и не зависит от `MDBX_LIFORECLAIM`-тонкостей.

**Фрагмент из [`examples/c++/39-pattern-sequence-id.c++`](examples/c++/39-pattern-sequence-id.c++)** — автоинкрементный ID: счётчик лежит в таблице `meta` и инкрементируется в той же пишущей транзакции, что и вставка записи:

```cpp
uint64_t next_id(mdbx::txn_managed &txn, const mdbx::map_handle &meta) {
  constexpr auto counter_key = mdbx::slice("seq");
  uint64_t current = 0;
  try {
    current = txn.get(meta, counter_key).as_uint64();
  } catch (const mdbx::not_found &) {
    current = 0;
  }
  ++current;
  txn.upsert(meta, counter_key, mdbx::slice::wrap(current));
  return current;
}
```

Полный код: [39-pattern-sequence-id.c++](examples/c++/39-pattern-sequence-id.c++).

### 32.2. Паттерн 2: Вторичный индекс (DUPSORT)

Основная таблица + индекс «поле → список ID» (Том II, глава 8). Обновлять в одной транзакции.

**Фрагмент из [`examples/c++/40-pattern-secondary-index.c++`](examples/c++/40-pattern-secondary-index.c++)** — вторичный индекс «поле → список ID»: основная таблица `users` и индекс `by_role` (DUPSORT, `value_mode::multi`) обновляются в одной транзакции:

```cpp
    auto txn = env.start_write();
    // Основная таблица: id → {name, role}.
    auto users = txn.create_map("users", mdbx::key_mode::ordinal, mdbx::value_mode::single);
    // Индекс: role → список id (мультизначения).
    auto by_role = txn.create_map("by_role", mdbx::key_mode::usual, mdbx::value_mode::multi);

    struct rec {
      uint64_t id;
      const char *name;
      const char *role;
    };
    static const rec records[] = {{1, "alice", "admin"}, {2, "bob", "dev"}, {3, "carol", "admin"}};
    for (const auto &r : records) {
      txn.upsert(users, mdbx::slice::wrap(r.id), mdbx::slice(std::string(r.name) + "|" + r.role));
      txn.upsert(by_role, mdbx::slice(r.role), mdbx::slice::wrap(r.id));
    }
    txn.commit();
```

Полный код: [40-pattern-secondary-index.c++](examples/c++/40-pattern-secondary-index.c++).

### 32.3. Паттерн 3: Составной ключ

Конкатенация полей + компаратор (или big-endian для чисел). Поиск по префиксу — `SET_RANGE`.

**Фрагмент из [`examples/c++/41-pattern-composite-key.c++`](examples/c++/41-pattern-composite-key.c++)** — составной ключ: поля фиксированной ширины упаковываются в big-endian, при котором лексикографический порядок совпадает с числовым (диапазонный поиск — `SET_RANGE`):

```cpp
uint64_t pack(uint16_t year, uint16_t month) {
  const uint64_t value = (uint64_t(year) << 48) | (uint64_t(month) << 32);
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return __builtin_bswap64(value);
#else
  return value;
#endif
}

uint16_t decode_year(const mdbx::slice &key) {
  const auto *p = static_cast<const uint8_t *>(key.data());
  return uint16_t((uint16_t(p[0]) << 8) | p[1]);
}

uint16_t decode_month(const mdbx::slice &key) {
  const auto *p = static_cast<const uint8_t *>(key.data());
  return uint16_t((uint16_t(p[2]) << 8) | p[3]);
}
```

Полный код: [41-pattern-composite-key.c++](examples/c++/41-pattern-composite-key.c++).

### 32.4. Паттерн 4: Очередь задач (table-as-queue)

`MDBX_DUPSORT` + sequence: ключ — приоритет/номер, значение — задача. Естественный порядок выдачи.

**Фрагмент из [`examples/c++/42-pattern-queue.c++`](examples/c++/42-pattern-queue.c++)** — очередь задач: ключ — монотонный номер (ordinal), потребитель забирает первый элемент курсором и удаляет его:

```cpp
    // Потребитель: забирает первый элемент через курсор и удаляет.
    auto txn = env.start_write();
    auto queue = txn.open_map("queue", mdbx::key_mode::ordinal, mdbx::value_mode::single);
    std::string dequeued;
    auto cur = txn.open_cursor(queue);
    while (true) {
      auto r = cur.to_first(false);
      if (!r)
        break;
      const auto id = r.key.as_uint64();
      const auto task = r.value.as_string();
      dequeued += std::to_string(id) + ":" + std::string(task) + " ";
      cur.erase(false);
    }
    txn.commit();
    std::cout << "dequeued: " << dequeued << "\n";
```

Полный код: [42-pattern-queue.c++](examples/c++/42-pattern-queue.c++).

### 32.5. Паттерн 5: Кольцевой буфер

Ограниченный размер: при превышении удаляем самые старые ключи (курсор + `del`).

**Фрагмент из [`examples/c++/43-pattern-ring-buffer.c++`](examples/c++/43-pattern-ring-buffer.c++)** — кольцевой буфер последних N: фиксированные слоты `0..N-1`, запись перезаписывает `counter % N`:

```cpp
    // Пишем 12 элементов в буфер на 5 слотов.
    {
      auto txn = env.start_write();
      auto ring = txn.create_map("ring", mdbx::key_mode::ordinal, mdbx::value_mode::single);
      for (unsigned i = 0; i < 12; ++i) {
        const unsigned slot = i % kSlots;
        txn.upsert(ring, buffer::key_from_u64(slot), mdbx::slice("s" + std::to_string(i)));
      }
      txn.commit();
    }
```

Полный код: [43-pattern-ring-buffer.c++](examples/c++/43-pattern-ring-buffer.c++).

### 32.6. Паттерн 6: Полносканирующий итератор

Курсор + `get_batch`/`bunch_delete` для массовой обработки.

**Фрагмент из [`examples/c++/44-pattern-full-scan.c++`](examples/c++/44-pattern-full-scan.c++)** — полное сканирование курсором `to_first()` → `to_next()` с фильтром:

```cpp
    auto rtxn = env.start_read();
    auto table = rtxn.open_map(nullptr);
    auto cur = rtxn.open_cursor(table);

    // Полное сканирование с фильтром «чётные ключи».
    size_t scanned = 0, matched = 0;
    for (auto r = cur.to_first(); r; r = cur.to_next(false)) {
      ++scanned;
      if (r.key.as_string().size() % 2 == 0)
        ++matched;
    }
    std::cout << "scanned " << scanned << " entries\n";
```

Полный код: [44-pattern-full-scan.c++](examples/c++/44-pattern-full-scan.c++).

### 32.7. Паттерн 7: Репликация через per-table txnid

Маркеры изменений: хранить номер транзакции последнего изменения; вторичный процесс опрашивает
и копирует дельты.

**Фрагмент из [`examples/c++/45-pattern-replication.c++`](examples/c++/45-pattern-replication.c++)** — репликация через per-table txnid: значение хранит версию (`data|N`), реплика читает только записи новее своего `last_seen`:

```cpp
    // Реплика читает всё новее своего последнего txnid.
    auto replicate = [&](uint64_t last_seen) -> size_t {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      size_t n = 0;
      auto cur = rtxn.open_cursor(table);
      for (auto r = cur.to_first(); r; r = cur.to_next(false)) {
        if (version_of(r.value) > last_seen)
          ++n;
      }
      rtxn.abort();
      return n;
    };
```

Полный код: [45-pattern-replication.c++](examples/c++/45-pattern-replication.c++).

### 32.8. Паттерн 8: Read-your-writes через clone / embark_read

`mdbx_txn_clone`/`embark_read` — собственные записи видны немедленно в той же логической
операции без отдельной транзакции.

**Фрагмент из [`examples/c++/46-pattern-read-your-writes.c++`](examples/c++/46-pattern-read-your-writes.c++)** — read-your-writes: та же пишущая транзакция немедленно читает собственное изменение; после `commit` оно видно и свежему читателю:

```cpp
    // Пишущая транзакция сразу читает собственное изменение.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      std::cout << "within txn sees: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";
      txn.commit();
    }

    // После коммита изменение видно и свежему читателю.
    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::cout << "after commit via fresh txn sees: " << rtxn.get(table, mdbx::slice("key")).as_string() << "\n";
      rtxn.abort();
    }
```

Полный код: [46-pattern-read-your-writes.c++](examples/c++/46-pattern-read-your-writes.c++).

Каждый паттерн: постановка задачи → решение с кодом → компромиссы → альтернативы (в полной версии
базы знаний — `knowledge-base/`).

### 32.9. Резюме главы 32

- Автоинкрементный ID: счётчик в служебной таблице, инкрементируется в той же write-транзакции, что и вставка; альтернатива — встроенный `mdbx_dbi_sequence()`.
- Вторичный индекс (DUPSORT) «поле → список ID» обновляется в одной транзакции с основной таблицей.
- Составной ключ: поля фиксированной ширины в big-endian (лексикографический порядок = числовой), диапазонный поиск — `SET_RANGE`.
- Очередь задач: ordinal-ключ (монотонный номер), потребитель забирает первый элемент курсором и удаляет.
- Кольцевой буфер: фиксированные слоты `0..N-1`, запись перезаписывает `counter % N`.
- Полное сканирование — курсор `to_first()` → `to_next()`, массовая обработка — `get_batch`/`bunch_delete`.
- Репликация через per-table txnid (маркер версии в значении) и read-your-writes через `mdbx_txn_clone`/`embark_read`.

### 32.10. Чек-лист главы 32

- [ ] Для монотонных ID выбрать `mdbx_dbi_sequence()` или счётчик в служебной таблице (переносимый вариант).
- [ ] Вторичный индекс (DUPSORT, `value_mode::multi`) обновлять в той же транзакции, что и основную запись.
- [ ] Для составных ключей использовать big-endian/компаратор и диапазонный поиск `SET_RANGE`.
- [ ] Очередь задач реализовать через ordinal-ключ и удаление первого элемента курсором.
- [ ] Для ограниченных структур (кольцевой буфер) удалять самые старые записи при переполнении.
- [ ] Массовые выборки/удаления выполнять через `get_batch`/`bunch_delete`.
- [ ] Репликацию строить на маркерах txnid, реплику — на read-only снапшоте с фильтром по `last_seen`.

---

> **Примеры к главе:**
> [`examples/c++/39-pattern-sequence-id.c++`](examples/c++/39-pattern-sequence-id.c++);
> [`examples/c++/40-pattern-secondary-index.c++`](examples/c++/40-pattern-secondary-index.c++);
> [`examples/c++/41-pattern-composite-key.c++`](examples/c++/41-pattern-composite-key.c++);
> [`examples/c++/42-pattern-queue.c++`](examples/c++/42-pattern-queue.c++);
> [`examples/c++/43-pattern-ring-buffer.c++`](examples/c++/43-pattern-ring-buffer.c++);
> [`examples/c++/44-pattern-full-scan.c++`](examples/c++/44-pattern-full-scan.c++);
> [`examples/c++/45-pattern-replication.c++`](examples/c++/45-pattern-replication.c++);
> [`examples/c++/46-pattern-read-your-writes.c++`](examples/c++/46-pattern-read-your-writes.c++).

## Глава 33. Roadmap и будущее: MithrilDB

### 33.1. Текущее состояние

libmdbx на 2026 год — зрелый движок (0.15.x devel-канон), формат БД заморожен с 2018 года.
Фундаментальные улучшения GC/freelist планируются только в следующем поколении.

### 33.2. MithrilDB

Общий API для нескольких форматов хранения; амальгамация упрощает распространение.

### 33.3. Репликация

Предпосылки: подписка на изменения, ранняя очистка GC (2025), нелинейная обработка GC (сквозное
отслеживание использования страниц читаемыми снапшотами).

### 33.4. Прочее в roadmap

- Подписка на изменения (почтовые ящики, кольцевые буферы).
- Шифрование и сжатие на уровне движка.
- Потоковый режим чтения/записи длинных значений («Streaming BLOBs») — без ручного
  фрагментирования на чанки, см. [Ограничения и подводные камни](https://libmdbx.dqdkfa.ru/docs/ru/overview/restrictions.html).
- SWIG и кроссязыковое взаимодействие.

### 33.5. Резюме главы 33

- libmdbx на 2026 год — зрелый движок (0.15.x devel-канон); формат БД заморожен с 2018 года.
- Фундаментальные улучшения GC/freelist планируются только в следующем поколении — MithrilDB: общий API для нескольких форматов хранения, амальгамация упрощает распространение.
- Предпосылки репликации: подписка на изменения, ранняя очистка GC (2025), нелинейная обработка GC (сквозное отслеживание использования страниц читаемыми снапшотами).
- Прочее в roadmap: шифрование и сжатие на уровне движка, потоковый режим чтения/записи длинных значений («Streaming BLOBs»), SWIG и кроссязыковое взаимодействие.

### 33.6. Чек-лист главы 33

- [ ] При долгосрочном планировании учитывать, что формат БД заморожен с 2018 года (совместимость 0.11.x ↔ 0.12.x).
- [ ] Не ждать фундаментальных улучшений GC/freelist в текущем поколении — следить за MithrilDB.
- [ ] Для репликации опираться на доступные предпосылки (подписка на изменения, ранняя очистка GC).
- [ ] Оценивать roadmap-фичи (шифрование, сжатие, потоковый режим длинных значений / Streaming BLOBs, SWIG) как будущие, а не текущие возможности.

---

## Итог тома

Вы знаете правила и советы, платформенные нюансы, HSR, диагностику по проверенным алгоритмам,
миграцию с LMDB и паттерны проектирования.

**Что дальше:** Том VI — привязки (bindings): как использовать libmdbx из Rust, Go, Python,
Node.js, .NET, C++, Dart, Nim, Java, Haskell, Ruby, Scala.
