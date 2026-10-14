# Том I. Основы

> **Уровень:** для новичков — тех, кто никогда не работал с embedded key-value базами данных.
> **Цель тома:** вы понимаете, что такое libmdbx, умеете собрать её, открыть базу и выполнить
> первые операции чтения и записи в транзакциях.
> **Сквозной проект:** на протяжении томов I–II мы строим одно приложение — **конфигуратор
> приложений** (простое key-value хранилище параметров, которое постепенно обрастает индексами,
> многопоточностью и оптимизациями).
>
> Прогрессия тома: концепция → практика → механизм → нюанс. Код примеров — полный и компилируемый;
> вспомогательные функции (`env_open` в C++ / `ex_env_open` в C, а также `check_rc`, `die`) едины
> для всего учебника (см. `examples/common/`).

---

## Глава 1. Что такое libmdbx

### 1.1. Встраиваемая база данных

Базы данных бывают двух принципиально разных видов: **клиент-серверные** и **встраиваемые**.

Клиент-серверная база (PostgreSQL, MySQL) — это отдельный процесс-сервер. Ваше приложение общается
с ним по сети или сокету: отправляет SQL, получает ответы. Сервер владеет файлами данных, управляет
кэшем, конкурентным доступом и журналом. Вы платите за это сложностью развёртывания, отдельным
процессом и сетевыми накладными.

Встраиваемая база (SQLite, libmdbx) — это **библиотека**, которую вы линкуете прямо в свой процесс.
Нет сервера, нет сети: ваше приложение вызывает функции, а те напрямую читают и пишут файл.
Преимущества — простота (одна библиотека), скорость (нет сетевого слоя) и лёгкость распространения
(приложение само носит свою базу). Плата — вы сами отвечаете за конкурентный доступ и резервное
копирование.

**libmdbx** — встраиваемая транзакционная **key-value** база данных, глубоко переработанный потомок
известной библиотеки LMDB. «Транзакционная» означает, что группа операций выполняется атомарно:
либо все изменения применяются, либо ни одно.

> **Аналогия.** SQLite — это «встраиваемый PostgreSQL». libmdbx — это «встраиваемый
> высокопроизводительный кэш-сервер»: она не знает SQL и таблиц в реляционном смысле, но зато
> умеет отдавать данные со скоростью обращения к памяти.

### 1.2. Key-value модель данных

Key-value база хранит простые пары: **ключ** → **значение**. Обе части — просто последовательности
байтов, о смысле которых знает только ваше приложение.

```
"user:1001"  →  {"name": "Анна", "role": "admin"}
"user:1002"  →  {"name": "Борис", "role": "user"}
"theme"      →  "dark"
```

Отличие libmdbx от «обычного» словаря (std::map, HashMap):

- **Ключи всегда упорядочены.** Данные хранятся в сбалансированном дереве (B+tree), поэтому ключи
  можно обходить по порядку, искать диапазоны, находить «ближайший больший ключ». Это как массив с
  отсортированными ключами, а не как хэш-таблица.
- **Значения могут быть большими** — вплоть до ~2 ГБ (уходят на отдельные «overflow»-страницы).
- **Данные лежат в файле, отображённом в память.** Чтение ключа — это фактически разыменование
  указателя, без копирования и без сериализации.

### 1.3. Почему mmap: память как интерфейс к данным

Большинство баз данных держат собственный буферный кэш: при чтении — копируют страницу с диска в
память библиотеки; при записи — накапливают изменения в этом кэше и периодически сбрасывают на диск.

libmdbx использует другой подход: **memory-mapped файл** (mmap). Операционная система отображает
файл базы прямо в адресное пространство процесса. Данные выглядят как обычный массив байтов.
Чтение — это обращение по адресу; страницу при необходимости подтянет ядро (page cache).
Отдельного буферного кэша в библиотеке нет — его роль выполняет страничный кэш ОС.

Из этого следуют три важных свойства:

1. **Чтение не копирует данные.** Вы получаете указатель прямо в mmap-области — никакой
   десериализации. Это источник рекордной скорости чтения (обычно миллионы операций в секунду).
2. **Запись идёт через Copy-on-Write (CoW).** Меняя страницу, библиотека не трогает старую версию
   (её могут читать другие транзакции), а создаёт новую копию. Подробности — в Томе III.
3. **Операционная система сама решает, какие страницы держать в памяти.** Библиотека не выбирает
   стратегию вытеснения — это делает ядро.

### 1.4. Место libmdbx в ландшафте

| База                | Модель                    | Архитектурные свойства                                                                  |
| ------------------- | ------------------------- | --------------------------------------------------------------------------------------- |
| **LMDB**            | KV, mmap, MVCC            | Родоначальник подхода; проще, меньше возможностей                                       |
| **libmdbx**         | KV, mmap, MVCC            | Переработанный LMDB: больше режимов долговечности, GC вместо free-list, жёстче гарантии |
| **BerkeleyDB**      | KV, классическая          | Старшее поколение; блокировки уровня страниц, нет mmap                                  |
| **LevelDB/RocksDB** | KV, LSM                   | Оптимизированы на запись, фоновое сжатие, плохая предсказуемость чтения                 |
| **SQLite**          | Реляционная, встраиваемая | SQL, транзакции; медленнее на KV-сценариях                                              |

Здесь **MVCC** (multi-version concurrency control, управление многоверсионностью) — способ
одновременного чтения и записи без блокировок читателей: каждая транзакция видит неизменный
снимок данных (подробно — глава 5 и том III). **LSM** (log-structured merge-tree) — альтернативная
архитектура, оптимизированная под запись ценой предсказуемости чтения.

Сравнение на уровне свойств (не бенчмарков): если вам нужен упорядоченный key-value с
транзакциями, молниеносным чтением и предсказуемой записью — выбирайте libmdbx. Если нужен SQL —
SQLite. Если запись доминирует над чтением и можно терпеть фоновое сжатие — RocksDB.

### 1.5. Лицензия, экосистема, кто использует

libmdbx распространяется по разрешительной лицензии: исторически — OpenLDAP Public License (OLPL),
на master — **Apache-2.0 целиком** (ре-лицензирование в 0.13.x). Позволяет коммерческое использование
без открытия собственного кода.

Библиотеку используют в высоконагруженных проектах:

- **Ethereum-экосистема**: Erigon, Akula, Silkworm (все быстрые реализации Ethereum работают на
  libmdbx);
- **Isar** — локальная база Flutter-приложений;
- мессенджеры, аналитика, кэш-слои.

Экосистема биндингов покрывает Rust, Go, Python, Node.js, .NET, C++, Zig, Dart, Nim, Java,
Haskell, Ruby, Scala (подробно — Том VI).

### 1.6. Резюме главы 1

- libmdbx — встраиваемая транзакционная key-value БД (библиотека, не сервер).
- Ключи упорядочены (B+tree); значения — произвольные байты до ~2 ГБ.
- Данные отображены в память (mmap): чтение без копирования, запись через Copy-on-Write.
- Свойства: wait-free читатели, один писатель, транзакции, отсутствие WAL (восстановление по мете).
- Apache-2.0 (master)/OLPL (исторически); используется в Ethereum-клиентах, Isar и других проектах.

### 1.7. Упражнения

1. Чем клиент-серверная БД отличается от встраиваемой? Приведите пример, когда встраиваемая
   предпочтительнее.
2. Что даёт упорядоченность ключей сверх обычного словаря?
3. Почему чтение через mmap может быть быстрее, чем через собственный кэш БД?

### 1.8. Чек-лист главы 1

- [ ] я могу объяснить разницу между клиент-серверной и встраиваемой БД и сказать, когда какая предпочтительнее;
- [ ] понимаю, что libmdbx — упорядоченный key-value движок (B+tree), а не хэш-таблица и не SQL;
- [ ] могу описать роль mmap: чтение без копирования, запись через Copy-on-Write, вытеснение страниц делает ядро;
- [ ] знаю ключевые свойства: MVCC, wait-free читатели, один писатель, отсутствие WAL;
- [ ] помню лицензию (Apache-2.0 на master) и примеры проектов на libmdbx (Erigon, Isar).

### 1.9. Что дальше

Понимание «что такое libmdbx» пора превратить в работающий код. В главе 2 мы соберём библиотеку
двумя способами (амальгама и CMake), подключим её к проекту и запустим первый полный пример
«hello, libmdbx». Для сквозного проекта-конфигуратора это обязательный шаг: без собранной
библиотеки и проверенного цикла open → put → get двигаться дальше нельзя.

---

## Глава 2. Установка и первый запуск

### 2.1. Сборка из исходников

libmdbx можно собирать двумя способами:

- **Амальгамированный** (амальгама): один файл `mdbx.c` + заголовок `mdbx.h`, которые можно просто
  положить в ваш проект. Минимум зависимостей, идеально для встраивания.
- **Полный исходник**: каталог с `CMakeLists.txt`, тестами, примерами и утилитами (`mdbx_stat`,
  `mdbx_chk`, `mdbx_copy`, `mdbx_dump`, `mdbx_load`, `mdbx_defrag`).

Амальгама: скачайте `mdbx.h` и `mdbx.c` и добавьте `mdbx.c` в сборку вашего проекта. Больше ничего
не требуется — это настоящий «single-file» движок.

### 2.2. Подключение к проекту

**CMake** (полный исходник):

```cmake
add_subdirectory(libmdbx)
target_link_libraries(yourapp PRIVATE mdbx)
```

или **pkg-config**:

```sh
pkg-config --cflags --libs libmdbx
```

Собираете сами:

```sh
make -j
make install  # ставит libmdbx.a / libmdbx.so, mdbx.h, .pc
```

**Статика vs динамика.** Статическая линковка проще распространять (нет проблем с версиями .so) и
даёт больше шансов оптимизации. Динамическая позволяет обновлять движок без пересборки приложения.

### 2.3. Ключевые опции сборки

| Опция                   | Смысл                                                                                                                      |
| ----------------------- | -------------------------------------------------------------------------------------------------------------------------- |
| `MDBX_LOCKING`          | Реализация блокировок: `POSIX2008` (Linux, по умолчанию), `POSIX2001`, `SYSV` (macOS по умолчанию), `WIN32FILES` (Windows) |
| `MDBX_CHECKING`         | Уровень внутренних проверок/ассертов (−1…3). `2`/`3` сильно замедляют — только для отладки                                 |
| `MDBX_FORCE_ASSERTIONS` | Устаревшая опция (эквивалент `MDBX_CHECKING=2`); лучше использовать `MDBX_CHECKING`                                        |
| `MDBX_ENABLE_BIGFOOT`   | Цепочки GC-записей для очень больших освобождений; включена по умолчанию на 64-битных сборках                              |
| `MDBX_DEBUG`            | Логирование/ассерты/аудит (0 по умолчанию)                                                                                 |
| `MDBX_VALIDATION`       | Дополнительные структурные проверки при каждой операции                                                                    |
| `MDBX_ENABLE_PGOP_STAT` | Счётчики операций со страницами (для диагностики)                                                                          |

> **Совет:** для production собирайте с `MDBX_CHECKING=0` (или не указывайте вовсе) и
> `NDEBUG`. Отладочная сборка с ассертами может быть в разы медленнее и маскировать реальные
> тайминги.

### 2.4. Минимальный пример

Первый полный листинг учебника. Он открывает базу, записывает ключ и читает его.

```c
#include <stdio.h>
#include <string.h>
#include <mdbx.h>

static void die(const char *what, int rc) {
    fprintf(stderr, "%s: %s (%d)\n", what, mdbx_strerror(rc), rc);
    exit(1);
}

int main(void) {
    MDBX_env *env = NULL;
    MDBX_txn *txn = NULL;
    MDBX_dbi dbi;
    MDBX_val key, data;
    int rc;

    /* 1. Создаём окружение */
    rc = mdbx_env_create(&env);
    if (rc != MDBX_SUCCESS) die("env_create", rc);

    /* 2. Открываем (или создаём) файл базы; геометрия по умолчанию */
    rc = mdbx_env_open(env, "./demo.mdbx",
                       MDBX_NOSUBDIR, 0664);
    if (rc != MDBX_SUCCESS) die("env_open", rc);

    /* 3. Транзакция чтения-записи */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc != MDBX_SUCCESS) die("txn_begin", rc);

    /* 4. Открываем главную таблицу (имя NULL = главная) */
    rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
    if (rc != MDBX_SUCCESS) die("dbi_open", rc);

    /* 5. Записываем "greeting" -> "hello, libmdbx" */
    const char *k = "greeting";
    const char *v = "hello, libmdbx";
    key.iov_len = strlen(k);
    key.iov_base = (void *)k;
    data.iov_len = strlen(v);
    data.iov_base = (void *)v;
    rc = mdbx_put(txn, dbi, &key, &data, 0);
    if (rc != MDBX_SUCCESS) die("put", rc);

    /* 6. Коммитим */
    rc = mdbx_txn_commit(txn);
    if (rc != MDBX_SUCCESS) die("txn_commit", rc);

    /* 7. Читаем в новой read-only транзакции */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) die("txn_begin-ro", rc);
    rc = mdbx_get(txn, dbi, &key, &data);
    if (rc != MDBX_SUCCESS) die("get", rc);
    printf("Значение: %.*s\n", (int)data.iov_len, (char *)data.iov_base);
    mdbx_txn_abort(txn);

    mdbx_env_close(env);
    return 0;
}
```

Разберём по шагам.

- `mdbx_env_create` создаёт пустой объект окружения — «соединение» с будущей базой.
- `mdbx_env_open` открывает файл базы; создание управляется параметром `mode`: ненулевой `0664`
  означает «создать, если базы ещё нет». Флаг `MDBX_CREATE` сюда не передавайте — он для
  `mdbx_dbi_open()`, а его бит совпадает с `MDBX_NOMETASYNC`. `MDBX_NOSUBDIR` означает, что имя
  файла дано напрямую, а не как префикс для пары `данные+блокировки`.

> **Важно:** бит `MDBX_CREATE` = `0x40000` — это тот же бит, что и `MDBX_NOMETASYNC`. Если
> передать `MDBX_CREATE` в `flags` у `mdbx_env_open()`, БД создастся (из-за ненулевого `mode`),
> но приложение **молча** получит ослабленную долговечность (`NOMETASYNC`). Создание включается
> только через `mode != 0`.

- Все изменения происходят **внутри транзакции** — до коммита они не видны другим читателям.
- `mdbx_dbi_open(txn, NULL, 0, &dbi)` открывает **главную таблицу** (подробно — глава 3).
- `mdbx_put` кладёт пару; `mdbx_get` читает её.
- `MDBX_val` — структура `{ iov_base, iov_len }`: указатель и длина байтового окна.

**Фрагмент из [`examples/c++/01-hello.c++`](examples/c++/01-hello.c++)** — тот же цикл open → put → get → close на C++ API: RAII-объекты (`env_managed`, транзакции) сами закрывают окружение, а ключи и значения передаются как `mdbx::slice`:

```cpp
#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common/common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-01-hello.mdbx";
    mdbx::env::remove(path); // стереть остатки предыдущего запуска

    // Создание: конструктор с create_parameters создаёт БД, если её нет.
    mdbx::env_managed env(path, mdbx::env_managed::create_parameters(), mdbx::env::operate_parameters());
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr); // главная таблица
      txn.insert(table, mdbx::slice("key"), mdbx::slice("hello, libmdbx"));
      txn.commit();
    }
    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::cout << "got: " << rtxn.get(table, mdbx::slice("key")).as_string() << "\n";
    }
```

Полный код: [01-hello.c++](examples/c++/01-hello.c++) · [C-версия](examples/c/01-hello.c)

### 2.5. Сборка и запуск тестов

В полном исходнике:

```sh
cmake -S . -B build -DMDBX_ENABLE_WERROR=ON
cmake --build build -j
ctest --test-dir build
```

Полезен стохастический стресс-тест `mdbx_test` с фиксированным `--prng-seed` — он воспроизводим и
помогает отлаживать редкие ошибки (подробно в Томе V, глава 30).

### 2.6. Нюанс: файлы данных и блокировок

Одна «база» физически состоит из **двух файлов**: данных (`demo.mdbx`) и блокировок (`demo.mdbx-lck`).
Файл блокировок содержит мьютекс писателя и таблицу читателей — это межпроцессная инфраструктура.
Не удаляйте `.lck` при открытой базе; при копировании базы помните про пару файлов (подробно —
Том III, глава 19).

> **Примеры к главе:** [`examples/c++/01-hello.c++`](examples/c++/01-hello.c++) · [C-версия](examples/c/01-hello.c);
> сквозной проект: [`config-store-02.c++`](examples/config-store/config-store-02.c++).

### 2.7. Резюме главы 2

- Амальгама (`mdbx.c`+`mdbx.h`) или полный исходник с CMake.
- Ключевые опции сборки: `MDBX_LOCKING`, `MDBX_CHECKING`, `MDBX_ENABLE_BIGFOOT`.
- Минимальный цикл: create → open → txn_begin → dbi_open → put/get → commit → close.
- База — пара файлов: данные + блокировки.
- Отладочные проверки включайте осознанно — они замедляют.

### 2.8. Упражнения

1. Соберите минимальный пример и запустите дважды — убедитесь, что значение переживает перезапуск.
2. Добавьте второй ключ и прочитайте оба в одной транзакции.
3. Что произойдёт, если при первом запуске (базы ещё нет) передать `mode = 0`?

### 2.9. Чек-лист главы 2

- [ ] я умею собирать библиотеку двумя способами: амальгамой (`mdbx.c` + `mdbx.h`) и полным исходником с CMake;
- [ ] знаю ключевые опции сборки (`MDBX_LOCKING`, `MDBX_CHECKING`, `MDBX_ENABLE_BIGFOOT`) и почему для production нужны `MDBX_CHECKING=0` и `NDEBUG`;
- [ ] могу собрать и запустить минимальный пример, убедившись, что значение переживает перезапуск;
- [ ] знаю минимальный цикл работы: create → open → txn_begin → dbi_open → put/get → commit → close;
- [ ] помню, что база — это пара файлов (данные + `.lck`), и не удаляю файл блокировок при открытой базе.

### 2.10. Что дальше

Следующая глава — о том, что именно вы записываете и читаете: базовая модель данных libmdbx.
Мы разберём `MDBX_val`, главную и именованные таблицы (`MDBX_dbi`), лимиты, integer-ключи и
пустые ключи/значения. Эти понятия определяют схему сквозного проекта-конфигуратора: какие
таблицы создавать и что в них хранить.

---

## Глава 3. Базовая модель данных

### 3.1. MDBX_val: ключ и значение как байтовые строки

Все данные в libmdbx — последовательности байтов. Ни тип, ни кодировка, ни структура не
интересуют движок. Окно на такие данные — `MDBX_val`:

```c
typedef struct MDBX_val {
    size_t iov_len;   /* длина в байтах */
    void  *iov_base;  /* указатель на данные */
} MDBX_val;
```

Важно: `iov_base` обычно указывает **прямо в mmap-область** базы. Поэтому:

- читать значение можно без копирования;
- но после завершения транзакции указатель может стать недействительным — скопируйте данные, если
  собираетесь их хранить дольше транзакции.

> **Нюанс (C++): `mdbx::slice` и `mdbx::buffer`.** В C++ API роль `MDBX_val` играют два типа.
> `mdbx::slice` — невладеющее представление (аналог `std::string_view` над `MDBX_val`): он лишь
> ссылается на чужие байты и ничего не копирует. Конструктор из `std::string` сделан `explicit`
> неслучайно: `mdbx::slice(std::to_string(k))` прямо в аргументе вызова — безопасно, потому что
> временная строка живёт до конца полного выражения (то есть пока не завершится весь вызов
> `insert()`/`get()`, который копирует байты внутрь базы), а вот сохранить такой срез в переменную
> и использовать позже — висячая ссылка. `mdbx::buffer` — наоборот, владеющий контейнер: по
> умолчанию он **копирует** содержимое (перегрузки с `make_reference=true` возвращаются к
> поведению `slice`), поэтому `mdbx::buffer(std::to_string(k))` не накладывает требований на
> время жизни источника.

### 3.2. Главная таблица и именованные таблицы (dbi)

Внутри одного файла базы живут **несколько деревьев**. Каждое дерево — «таблица» (в терминах API —
`MDBX_dbi`, map / sub-DB). Есть:

- **главная таблица** — дескрипторы всех остальных таблиц (имя → корень дерева, флаги, счётчики);
- **именованные таблицы** — ваши данные;
- служебные (например, GC-дерево для свободных страниц — подробно в Томе III).

Открытие таблицы в транзакции:

```c
/* главная таблица */
mdbx_dbi_open(txn, NULL, 0, &dbi_main);
/* именованная таблица (создаётся при первом открытии с MDBX_CREATE) */
mdbx_dbi_open(txn, "users", MDBX_CREATE, &dbi_users);
```

`MDBX_dbi` — это не глобальный объект, а **привязка к таблице внутри конкретной транзакции**:
хендл валиден там, где его открыли, и наследуется вложенными транзакциями. Использование хендла из
другой транзакции/потока — ошибка (`MDBX_BAD_DBI`).

> **Нюанс: повторное открытие таблицы.** Для уже существующей таблицы `mdbx_dbi_open()` требует
> передачи её постоянных флагов (`MDBX_INTEGERKEY`, `MDBX_DUPSORT`, `MDBX_DUPFIXED`, …); расхождение
> даёт `MDBX_INCOMPATIBLE`. Если флаги заранее неизвестны (таблица могла быть создана другим
> кодом), откройте её с флагом `MDBX_DB_ACCEDE`: таблица откроется с фактическими флагами, а узнать
> их можно через `mdbx_dbi_flags()`. Аналогичный флаг `MDBX_ACCEDE` есть у `mdbx_env_open()` — он
> открывает базу, уже используемую другим процессом в неизвестном режиме, без ошибки
> `MDBX_INCOMPATIBLE` (Том II, глава 9).

### 3.3. Ограничения

| Параметр  | Значение                                                                                     |
| --------- | -------------------------------------------------------------------------------------------- |
| Ключ      | до ~половины страницы (зависит от размера страницы и режима таблицы); integer-ключи — 8 байт |
| Значение  | до `MDBX_MAXDATASIZE` ≈ 2 ГБ                                                                 |
| Таблиц    | `MDBX_MAX_DBI` = 32765                                                                       |
| Читателей | настраивается (`mdbx_env_set_maxreaders`), до `MDBX_READERS_LIMIT` = 32767                   |

Размер страницы выбирается при создании базы: 256…65536 байт, по умолчанию 4096, и далее **не
изменяется**. Выбор влияет на производительность (Том IV).

Актуальные пределы движок отдаёт через семейство `mdbx_limits_*` — они зависят от размера страницы
и сборки, поэтому вместо «зашитых» чисел используйте функции: `mdbx_limits_dbsize_min()`,
`mdbx_limits_dbsize_max()`, `mdbx_limits_keysize_min()`, `mdbx_limits_keysize_max()`,
`mdbx_limits_valsize_min()`, `mdbx_limits_valsize_max()`, `mdbx_limits_pgsize_min()`,
`mdbx_limits_pgsize_max()`, `mdbx_limits_txnsize_max()`, `mdbx_limits_pairsize4page_max()`,
`mdbx_limits_valsize4page_max()`. Удобные алиасы для отдельных значений —
`mdbx_env_get_maxkeysize(_ex)` и `mdbx_env_get_maxvalsize_ex`.

### 3.4. Integer-ключи и нативный порядок байт

Для числовых ключей используйте специальные типы: `MDBX_INTEGERKEY` (таблица с целочисленными
ключами `uint32_t`/`uint64_t` **нативного порядка**) и `MDBX_INTEGERDUP` (целочисленные
мультизначения). Нативный порядок байт означает: ключ `42` в памяти как `uint64_t` — без
перестановок байт. Это критично для сравнений и курсоров.

Нестандартный порядок сортировки задаётся **компаратором** — колбэком типа `MDBX_cmp_func`.
Кастомные компараторы ключей/значений передаются при открытии таблицы в расширенных вариантах
`mdbx_dbi_open_ex()`/`mdbx_dbi_open_ex2()` (аргументы `keycmp`/`datacmp`). Стандартные компараторы
бинарных ключей/значений — `mdbx_cmp()`/`mdbx_dcmp()`; выбрать нужный под конкретный набор флагов
таблицы можно через `mdbx_get_keycmp(flags)`/`mdbx_get_datacmp(flags)`.

### 3.5. Нулевые ключи и значения (отличие от LMDB)

В отличие от LMDB, libmdbx позволяет **пустые ключи** (длина 0) и **пустые значения** (длина 0).
Это удобно для маркеров и флагов-наличия. Пустое значение — это `iov_len == 0` (указатель может
быть NULL). Различайте «ключа нет» (`MDBX_NOTFOUND`) и «ключ есть с пустым значением»
(`MDBX_SUCCESS` при `iov_len == 0`).

**Фрагмент из [`examples/c++/02-data-model.c++`](examples/c++/02-data-model.c++)** — integer-ключи (`key_mode::ordinal`) и нулевое значение (`mdbx::slice()`):

```cpp
    // Главная таблица (name == nullptr): обычные ключи и значения.
    auto main = txn.open_map(nullptr);
    txn.insert(main, mdbx::slice("key"), mdbx::slice("value"));
    std::cout << "main[key] = " << txn.get(main, mdbx::slice("key")).as_string() << "\n";

    // Именованная таблица: отдельное пространство ключей внутри той же БД.
    auto named = txn.create_map("named", mdbx::key_mode::usual, mdbx::value_mode::single);
    txn.insert(named, mdbx::slice("config"), mdbx::slice("42"));
    std::cout << "named[config] = " << txn.get(named, mdbx::slice("config")).as_string() << "\n";

    // Integer-ключи: таблица с key_mode::ordinal (MDBX_INTEGERKEY),
    // ключи — uint64_t в нативном порядке байт.
    auto ord = txn.create_map("ordinal", mdbx::key_mode::ordinal, mdbx::value_mode::single);
    const uint64_t k1 = 1, k2 = 1000;
    txn.insert(ord, buffer::key_from_u64(k1), mdbx::slice("one"));
    txn.insert(ord, buffer::key_from_u64(k2), mdbx::slice("thousand"));
    std::cout << "ordinal[1000] = " << txn.get(ord, buffer::key_from_u64(k2)).as_string() << "\n";

    // Пустой ключ (нулевой длины) и пустое значение.
    txn.insert(main, mdbx::slice(), mdbx::slice("empty-key-value"));
    txn.insert(named, mdbx::slice("empty"), mdbx::slice());
    std::cout << "empty value length = " << txn.get(named, mdbx::slice("empty")).size() << "\n";
```

Полный код: [02-data-model.c++](examples/c++/02-data-model.c++) · [C-версия](examples/c/02-data-model.c)

> **Примеры к главе:** [`examples/c++/02-data-model.c++`](examples/c++/02-data-model.c++) · [C-версия](examples/c/02-data-model.c).

### 3.6. Резюме главы 3

- `MDBX_val` = байтовое окно `{len, ptr}`; указатель может жить в mmap.
- Один файл — много таблиц (деревьев); главная таблица хранит дескрипторы.
- `MDBX_dbi` — привязка к таблице в конкретной транзакции.
- Лимиты: ключ ~½ страницы, значение ~2 ГБ, таблиц 32765.
- Integer-ключи — нативный порядок; пустые ключи и значения разрешены.

### 3.7. Упражнения

1. Создайте две именованные таблицы («settings», «counters») и положите по ключу в каждую.
2. Попробуйте прочитать ключ из хендла, открытого в другой транзакции — что вернёт API?
3. Проверьте поведение с пустым значением: положите `iov_len=0`, прочитайте — как отличить
   «пустое значение» от «ключа нет»?

### 3.8. Чек-лист главы 3

- [ ] я понимаю, что `MDBX_val` — это байтовое окно `{iov_len, iov_base}` без типа и кодировки;
- [ ] умею открывать главную и именованные таблицы через `mdbx_dbi_open()` и знаю роль флага `MDBX_CREATE`;
- [ ] понимаю, что `MDBX_dbi` валиден только в своей транзакции, и помню про `MDBX_BAD_DBI` и `MDBX_DB_ACCEDE`;
- [ ] проверяю лимиты через функции `mdbx_limits_*`, а не через зашитые числа;
- [ ] знаю, зачем `MDBX_INTEGERKEY` (нативный порядок байт) и как отличить пустое значение (`iov_len == 0`) от отсутствия ключа (`MDBX_NOTFOUND`).

### 3.9. Что дальше

Теперь, когда модель данных ясна, можно перейти к операциям над ней. В главе 4 мы освоим базовый
набор CRUD — `mdbx_put`, `mdbx_get`, `mdbx_del`, `mdbx_replace` — с флагами записи, обработкой
ожидаемых кодов и полным примером. Именно эти операции лягут в основу `cfg_set`/`cfg_get`
сквозного конфигуратора.

---

## Глава 4. Базовые операции CRUD

### 4.1. mdbx_put — вставка и обновление

```c
int mdbx_put(MDBX_txn *txn, MDBX_dbi dbi,
             const MDBX_val *key, const MDBX_val *data, unsigned flags);
```

Флаги:

| Флаг                | Смысл                                                 |
| ------------------- | ----------------------------------------------------- |
| `0` / `MDBX_UPSERT` | Вставить или заменить значение                        |
| `MDBX_NOOVERWRITE`  | Вставить только если ключа нет; иначе `MDBX_KEYEXIST` |
| `MDBX_NODUPDATA`    | (DUPSORT) не добавлять дубликат значения              |
| `MDBX_CURRENT`      | Обновить текущее значение курсора                     |

### 4.2. mdbx_get — чтение по ключу

```c
int mdbx_get(MDBX_txn *txn, MDBX_dbi dbi,
             const MDBX_val *key, MDBX_val *data);
```

Возвращает `MDBX_SUCCESS` (нашли) или `MDBX_NOTFOUND` (ключа нет). Для DUPSORT-таблиц нужен
`MDBX_GET_BOTH` (искать конкретное значение) — подробно в Томе II, глава 7.

Расширенный вариант `mdbx_get_ex(txn, dbi, &key, &data, &values_count)` дополнительно возвращает
**число значений ключа** (для обычной таблицы — 1; для DUPSORT — сколько значений связано с ключом;
удобно без отдельного курсора и `mdbx_cursor_count()`).

### 4.3. mdbx_del — удаление

```c
int mdbx_del(MDBX_txn *txn, MDBX_dbi dbi,
             const MDBX_val *key, const MDBX_val *data);
```

Если `data == NULL` — удаляется весь ключ. Если задан и таблица DUPSORT — удаляется конкретное
значение.

**Фрагмент из [`examples/c++/03-crud.c++`](examples/c++/03-crud.c++)** — флаги записи на C++ API: `insert` соответствует `MDBX_NOOVERWRITE` (при повторе — исключение `mdbx::key_exists`), `upsert` — `MDBX_UPSERT`, `update` — `MDBX_CURRENT`. Флаги `MDBX_NODUPDATA`/`MDBX_ALLDUPS` для мультизначений — в Томе II, главы 6–7:

```cpp
    // NOOVERWRITE: вставка уникального ключа.
    txn.insert(table, mdbx::slice("key"), mdbx::slice("value1"));

    // Повторная вставка того же ключа -> key_exists (MDBX_KEYEXIST).
    try {
      txn.insert(table, mdbx::slice("key"), mdbx::slice("value2"));
      std::cerr << "FAIL: duplicate insert did not throw\n";
      return EXIT_FAILURE;
    } catch (const mdbx::key_exists &) {
      std::cout << "insert duplicate -> MDBX_KEYEXIST as expected\n";
    }

    // try_insert не бросает исключение, а сообщает результат флагом done.
    auto ins = txn.try_insert(table, mdbx::slice("key"), mdbx::slice("value2"));
    std::cout << "try_insert duplicate -> done=" << (ins.done ? 1 : 0) << "\n";

    // UPSERT: вставить или перезаписать.
    txn.upsert(table, mdbx::slice("key"), mdbx::slice("value2"));
    std::cout << "after upsert: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";

    // CURRENT (update): обновить только существующий ключ.
    txn.update(table, mdbx::slice("key"), mdbx::slice("value3"));
    std::cout << "update existing: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";
```

Полный код: [03-crud.c++](examples/c++/03-crud.c++) · [C-версия](examples/c/03-crud.c)

### 4.4. mdbx_replace — условная замена

`mdbx_replace` объединяет put+get в одну операцию: заменить значение, узнать старое, вставить
только если совпадает текущее. Используется для атомарных «сравни и замени» без лишних поисков.

```c
int mdbx_replace(MDBX_txn *txn, MDBX_dbi dbi,
                 const MDBX_val *key, const MDBX_val *new_data,
                 const MDBX_val *old_data /* может быть NULL */,
                 MDBX_val *old_value /* может быть NULL */, unsigned flags);
```

### 4.5. Чтение: mdbx_get vs курсор

`mdbx_get` — точечный поиск по точному ключу. Если нужно обойти данные, найти диапазон, «ближайший
больший ключ» — используйте курсор (Том II, глава 6). Для точного чтения `mdbx_get` достаточно.

### 4.6. Полный пример: CRUD

```c
#include <stdio.h>
#include <string.h>
#include <mdbx.h>

static void die(const char *what, int rc) {
    fprintf(stderr, "%s: %s (%d)\n", what, mdbx_strerror(rc), rc);
    exit(1);
}

int main(void) {
    MDBX_env *env = NULL; MDBX_txn *txn = NULL; MDBX_dbi dbi;
    MDBX_val key, data, out;
    int rc;

    rc = mdbx_env_create(&env);
    if (rc) die("create", rc);
    rc = mdbx_env_open(env, "./crud.mdbx", MDBX_NOSUBDIR, 0664);
    if (rc) die("open", rc);
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc) die("txn", rc);
    rc = mdbx_dbi_open(txn, "kv", MDBX_CREATE, &dbi);
    if (rc) die("dbi", rc);

    /* insert */
    const char *k1 = "alpha", *v1 = "1";
    key.iov_base = (void *)k1; key.iov_len = strlen(k1);
    data.iov_base = (void *)v1; data.iov_len = strlen(v1);
    rc = mdbx_put(txn, dbi, &key, &data, MDBX_NOOVERWRITE);
    if (rc != MDBX_SUCCESS && rc != MDBX_KEYEXIST) die("put", rc);

    /* update */
    const char *v2 = "2";
    data.iov_base = (void *)v2; data.iov_len = strlen(v2);
    rc = mdbx_put(txn, dbi, &key, &data, 0); /* UPSERT */
    if (rc) die("put2", rc);

    /* get */
    out.iov_base = NULL; out.iov_len = 0;
    rc = mdbx_get(txn, dbi, &key, &out);
    if (rc) die("get", rc);
    printf("alpha = %.*s\n", (int)out.iov_len, (char *)out.iov_base);

    /* replace: прочитать старое значение и записать новое */
    MDBX_val old = {0, NULL};
    const char *v3 = "3";
    data.iov_base = (void *)v3; data.iov_len = strlen(v3);
    rc = mdbx_replace(txn, dbi, &key, &data, NULL, &old, 0);
    if (rc) die("replace", rc);
    printf("было: %.*s\n", (int)old.iov_len, (char *)old.iov_base);

    /* del */
    rc = mdbx_del(txn, dbi, &key, NULL);
    if (rc) die("del", rc);

    /* проверим удаление */
    rc = mdbx_get(txn, dbi, &key, &out);
    printf("get после удаления: %s (%d)\n",
           rc == MDBX_NOTFOUND ? "NOTFOUND (ожидаемо)" : "неожиданно", rc);

    mdbx_txn_commit(txn);
    mdbx_env_close(env);
    return 0;
}
```

> **Внимание:** `old_value` из `mdbx_replace` и `out` из `mdbx_get` указывают в mmap. Не храните
> эти указатели после `commit`/`abort` — данные могут быть переиспользованы.

### 4.7. Обработка ошибок

Коды возврата: `MDBX_SUCCESS` (0) — успех; `MDBX_RESULT_TRUE`/`MDBX_RESULT_FALSE` — специальные
результаты некоторых операций; отрицательные `MDBX_*` — ошибки.

Частые «ожидаемые» коды (это не сбои, а штатные ответы):

- `MDBX_KEYEXIST` — ключ уже есть (при `MDBX_NOOVERWRITE`);
- `MDBX_NOTFOUND` — ключ (или значение) не найден;
- `MDBX_BAD_DBI` — невалидный/закрытый хендл таблицы.

Строковое описание: `mdbx_strerror(rc)`. Есть и потокобезопасный вариант `mdbx_strerror_r()` —
используйте его в многопоточном коде (Том II, глава 12).

> **Примеры к главе:** [`examples/c++/03-crud.c++`](examples/c++/03-crud.c++) · [C-версия](examples/c/03-crud.c);
> сквозной проект: [`config-store-04.c++`](examples/config-store/config-store-04.c++).

### 4.8. Резюме главы 4

- put/get/del/replace — базовые операции; флаги управляют поведением при конфликтах.
- `mdbx_get` — точечное чтение; для обхода/диапазонов — курсоры.
- `MDBX_KEYEXIST`/`MDBX_NOTFOUND` — ожидаемые состояния, а не ошибки.
- `mdbx_replace` — атомарная замена «чтение старого + запись нового» одним вызовом.
- Указатели из mmap валидны только внутри транзакции.

### 4.9. Упражнения

1. Добавьте обработку «expected error»: вставьте ключ повторно с `NOOVERWRITE` и обработайте
   `MDBX_KEYEXIST` без выхода из программы.
2. Напишите цикл, который вставляет 10 000 ключей в **одной** транзакции и измеряет время.
3. Что делает `mdbx_del` с `data`-аргументом в обычной (не DUPSORT) таблице?

### 4.10. Чек-лист главы 4

- [ ] я умею выполнять все базовые операции: `mdbx_put`, `mdbx_get`, `mdbx_del` и `mdbx_replace`;
- [ ] понимаю различие флагов записи `MDBX_NOOVERWRITE`, `MDBX_UPSERT`, `MDBX_CURRENT` и выбираю их осознанно;
- [ ] обрабатываю `MDBX_KEYEXIST` и `MDBX_NOTFOUND` как штатные состояния, а не сбои;
- [ ] понимаю, что `mdbx_replace` — атомарное «сравнить и заменить» одним вызовом;
- [ ] помню, что указатели из mmap (`out`, `old_value`) нельзя использовать после commit/abort.

### 4.11. Что дальше

Одиночные операции мы уже умеем, но настоящая сила libmdbx — в транзакциях. Глава 5 вводит
begin/commit/abort, различие read-only и read-write транзакций и интуицию MVCC — почему читатели
не блокируют писателя. Для конфигуратора это фундамент: атомарная запись группы настроек и
безопасная параллельная работа с базой.

---

## Глава 5. Транзакции — первые шаги

### 5.1. Что такое транзакция и зачем она нужна

Транзакция — это группа операций, которая выполняется **атомарно** и **изолированно**:

- либо все изменения применяются, либо ни одно (атомарность);
- пока транзакция не закоммичена, её изменения не видны другим (изоляция).

Пример: перевод денег — списать со счёта A и зачислить на счёт B. Если это сделать двумя
отдельными операциями, между ними можно потерять данные при сбое. В транзакции — безопасно.

### 5.2. begin / commit / abort

```c
int mdbx_txn_begin(MDBX_env *env, MDBX_txn *parent,
                   unsigned flags, MDBX_txn **txn);
int mdbx_txn_commit(MDBX_txn *txn);
int mdbx_txn_abort(MDBX_txn *txn);
```

- `mdbx_txn_begin(env, NULL, flags, &txn)` — начать транзакцию. `flags == 0` (то же, что
  `MDBX_TXN_READWRITE`) — читающая-пишущая; для транзакции только на чтение передавайте
  `MDBX_TXN_RDONLY`.
- `mdbx_txn_commit(txn)` — применить изменения (для write) или просто закрыть (для read).
- `mdbx_txn_abort(txn)` — отбросить изменения и закрыть.

> **Совет:** всегда закрывайте транзакцию — либо commit, либо abort. Незакрытая read-транзакция
> может «заморозить» переработку страниц (Том III, глава 16).

### 5.3. Read-only vs read-write

**Read-only транзакции** не блокируют друг друга и не требуют мьютекса писателя. Их можно иметь
много одновременно, в любых потоках.

**Read-write транзакция** в любой момент времени в базе **одна** (глобальный мьютекс писателя).
Если два потока попытаются начать пишущую транзакцию одновременно — один дождётся другого.

### 5.4. Почему читатели не блокируют друг друга (MVCC интуитивно)

libmdbx использует **MVCC** (multiversion concurrency control). Упрощённо:

1. Каждая транзакция видит «снапшот» базы — состояние на момент её начала.
2. Пишущая транзакция не изменяет существующие страницы, а создаёт **новые версии** (Copy-on-Write).
3. Читатель, начавшийся раньше, продолжает видеть старые версии, пока жив его снапшот.

Поэтому читатели никогда не конфликтуют ни друг с другом, ни с писателем. Расплата — старые версии
страниц занимают место, пока живы старые снапшоты (подробно — Том III, главы 14 и 16).

**Фрагмент из [`examples/c++/04-transactions.c++`](examples/c++/04-transactions.c++)** — read-only наблюдатель в отдельном потоке видит стабильный MVCC-снапшот, пока пишущая транзакция перезаписывает ключ и затем делает `abort` (откат):

```cpp
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      txn.commit();
    }

    // Read-only наблюдатель в отдельном потоке: фиксированный снапшот базы.
    std::thread observer(observer_run, std::ref(env));
    while (stage.load() != 1)
      std::this_thread::yield();

    // Пишущая транзакция перезаписывает ключ, но ещё не коммитит.
    {
      auto wtxn = env.start_write();
      auto table = wtxn.open_map(nullptr);
      wtxn.upsert(table, mdbx::slice("key"), mdbx::slice("v2"));
      stage = 2; // наблюдатель проверяет снапшот при незакоммиченной записи
      while (stage.load() != 3)
        std::this_thread::yield();
      wtxn.abort(); // откат: v2 не сохраняется
    }
```

Полный код: [04-transactions.c++](examples/c++/04-transactions.c++) · [C-версия](examples/c/04-transactions.c)

### 5.5. Базовое правило: одна транзакция — один поток

Транзакция привязана к потоку, который её создал («sticky thread»). Использовать объект транзакции
из другого потока нельзя — это нарушение дисциплины (`MDBX_THREAD_MISMATCH`). Впрочем, есть режим
`MDBX_NOSTICKYTHREADS` для пулов потоков и корутин (Том II, глава 11).

### 5.6. Полный пример с транзакциями

```c
#include <stdio.h>
#include <string.h>
#include <mdbx.h>

static void die(const char *what, int rc) {
    fprintf(stderr, "%s: %s (%d)\n", what, mdbx_strerror(rc), rc);
    exit(1);
}

int main(void) {
    MDBX_env *env = NULL; MDBX_txn *txn = NULL; MDBX_dbi dbi;
    MDBX_val key, data, out;
    int rc;

    rc = mdbx_env_create(&env);
    if (rc) die("create", rc);
    rc = mdbx_env_open(env, "./txn.mdbx", MDBX_NOSUBDIR, 0664);
    if (rc) die("open", rc);

    /* ---- Write-транзакция: два ключа атомарно ---- */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc) die("txn_w", rc);
    rc = mdbx_dbi_open(txn, "kv", MDBX_CREATE, &dbi);
    if (rc) die("dbi", rc);

    const char *ka = "a", *va = "1";
    const char *kb = "b", *vb = "2";
    key.iov_base = (void *)ka; key.iov_len = strlen(ka);
    data.iov_base = (void *)va; data.iov_len = strlen(va);
    mdbx_put(txn, dbi, &key, &data, 0);
    key.iov_base = (void *)kb; key.iov_len = strlen(kb);
    data.iov_base = (void *)vb; data.iov_len = strlen(vb);
    mdbx_put(txn, dbi, &key, &data, 0);
    rc = mdbx_txn_commit(txn);
    if (rc) die("commit", rc);

    /* ---- Read-only транзакция ---- */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc) die("txn_r", rc);
    out.iov_base = NULL; out.iov_len = 0;
    key.iov_base = (void *)ka; key.iov_len = strlen(ka);
    rc = mdbx_get(txn, dbi, &key, &out);
    if (rc) die("get", rc);
    printf("a = %.*s\n", (int)out.iov_len, (char *)out.iov_base);
    mdbx_txn_abort(txn); /* read-only: просто закрываем */

    /* ---- Abort: изменения отбрасываются ---- */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc) die("txn_w2", rc);
    const char *kc = "c", *vc = "3";
    key.iov_base = (void *)kc; key.iov_len = strlen(kc);
    data.iov_base = (void *)vc; data.iov_len = strlen(vc);
    mdbx_put(txn, dbi, &key, &data, 0);
    mdbx_txn_abort(txn); /* c не появится в базе */

    /* проверяем, что "c" нет */
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc) die("txn_r2", rc);
    rc = mdbx_get(txn, dbi, &key, &out);
    printf("после abort: %s\n", rc == MDBX_NOTFOUND ? "ключа нет (верно)" : "неожиданно");
    mdbx_txn_abort(txn);

    mdbx_env_close(env);
    return 0;
}
```

> **Продолжить чтение после коммита — `mdbx_txn_commit_embark_read()`.** Коммит в libmdbx
> «посаживает» транзакцию на свежий снапшот (детент — удержание читателя на снимке состояния,
> механизм раскрыт в томе III); специальная функция `mdbx_txn_commit_embark_read()`
> делает то же и сразу возвращает **новую read-only транзакцию на уже закоммиченном состоянии** —
> без разрыва «коммит → новый begin». Полезно для паттернов read-your-writes и обработки очередей
> (Том V, глава 32). Идентификатор транзакции (порядковый номер в базе) можно узнать через
> `mdbx_txn_id()`.

### 5.7. Сквозной проект: начало

Наш конфигуратор приложений. Идея: хранить настройки в виде таблиц «секция → ключ → значение».

```c
/* config_store.h — каркас сквозного проекта (главы 2–5) */
#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H
#include <mdbx.h>

typedef struct {
    MDBX_env *env;
    MDBX_dbi  main;
} config_store_t;

int cfg_open(config_store_t *cs, const char *path);
int cfg_set(config_store_t *cs, const char *key, const char *value);
int cfg_get(config_store_t *cs, const char *key, char *buf, size_t buflen);
int cfg_close(config_store_t *cs);

#endif
```

Реализация — следующими главами; к концу Тома II это будет полноценное приложение с индексами и
многопоточностью.

> **Примеры к главе:** [`examples/c++/04-transactions.c++`](examples/c++/04-transactions.c++) · [C-версия](examples/c/04-transactions.c);
> сквозной проект: [`config-store-05.c++`](examples/config-store/config-store-05.c++).

### 5.8. Резюме главы 5

- Транзакция = атомарная, изолированная группа операций.
- begin (0/`MDBX_TXN_READWRITE` — пишущая; `MDBX_TXN_RDONLY` — только чтение), commit, abort.
- MVCC: читатели видят снапшот и не блокируют писателя.
- Одна транзакция — один поток (по умолчанию).
- Незакрытые read-транзакции «морозят» переработку страниц.

### 5.9. Упражнения

1. Напишите функцию `cfg_set`, которая использует `MDBX_NOOVERWRITE` для создания и `0` для
   обновления — как их объединить в одну транзакцию?
2. Измерьте: сколько write-транзакций в секунду выдерживает ваш диск (без батчинга)?
3. Почему начинать read-write транзакцию «на всякий случай» — плохая идея (подсказка: мьютекс
   писателя)?

### 5.10. Чек-лист главы 5

- [ ] я понимаю свойства транзакции: атомарность (всё или ничего) и изоляцию;
- [ ] умею начинать и корректно закрывать транзакции всех видов: `MDBX_TXN_READWRITE`, `MDBX_TXN_RDONLY`, `mdbx_txn_abort()`;
- [ ] понимаю, почему пишущая транзакция всегда одна, и как MVCC позволяет читателям не блокировать писателя;
- [ ] знаю правило «одна транзакция — один поток» и код `MDBX_THREAD_MISMATCH`;
- [ ] умею показать на примере, что abort отбрасывает изменения, а незакрытая read-транзакция мешает переработке страниц.

### 5.11. Что дальше

На этом Том I завершён: у вас есть работающий каркас конфигуратора. Том II доводит его до
полноценного приложения: курсоры для обхода и диапазонов (глава 6), DUPSORT и вторичные индексы
(главы 7–8), конфигурация окружения (глава 9), режимы долговечности (глава 10), многопоточность
(глава 11) и обработка ошибок (глава 12). Начинаем с курсоров — без них не сделать ни обхода
таблицы, ни поиска по диапазону.

---

## Итог тома

Вы познакомились с базовой моделью libmdbx: что это, как собрать, как открыть базу, выполнить
CRUD в транзакциях. У вас работает сквозной проект-«конфигуратор».

**Что дальше:** Том II — курсоры, мультизначения и DUPSORT, вторичные индексы, конфигурация
окружения, режимы долговечности, многопоточность и обработка ошибок.
