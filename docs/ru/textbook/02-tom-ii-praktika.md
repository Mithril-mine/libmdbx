# Том II. Практическое использование

> **Уровень:** для разработчиков, уже освоивших Том I.
> **Цель тома:** вы умеете строить приложения на libmdbx — курсоры, мультизначения и DUPSORT,
> вторичные индексы, конфигурацию окружения, режимы долговечности, многопоточность и устойчивую
> обработку ошибок.
> **Сквозной проект:** конфигуратор приложений доводим до полноценного приложения с индексами и
> многопоточностью.
>
> Схема глав: концепция → механизм → практика → нюанс.

---

## Глава 6. Курсоры

### 6.1. Что такое курсор и зачем он нужен

`mdbx_get` умеет только точечный поиск по точному ключу. Для всего остального — обхода таблицы,
поиска диапазона, «ближайшего большего ключа», итерации с удалением — нужен **курсор**.

Курсор — это «указатель» на позицию внутри дерева (точнее, стек позиций от корня к листу). Он
позволяет двигаться по упорядоченным ключам в обе стороны и выполнять операции относительно
текущей позиции.

### 6.2. Открытие и закрытие

```c
int mdbx_cursor_open(MDBX_txn *txn, MDBX_dbi dbi, MDBX_cursor **cursor);
int mdbx_cursor_close(MDBX_cursor *cursor);
```

Курсор принадлежит транзакции: создаётся внутри неё и живёт не дольше её. После `commit`/`abort`
курсор закрывать нельзя — используйте его только до завершения транзакции.

### 6.3. Позиционирование

```c
int mdbx_cursor_get(MDBX_cursor *cur, MDBX_val *key, MDBX_val *data, MDBX_cursor_op op);
```

Ключевые операции:

| Операция                           | Смысл                                                            |
| ---------------------------------- | ---------------------------------------------------------------- |
| `MDBX_FIRST` / `MDBX_LAST`         | Перейти к первому / последнему ключу                             |
| `MDBX_NEXT` / `MDBX_PREV`          | Следующий / предыдущий ключ                                      |
| `MDBX_SET`                         | Найти точный ключ (позиция на него)                              |
| `MDBX_SET_RANGE`                   | Найти первый ключ ≥ заданного («ближайший больший или равный»)   |
| `MDBX_SET_LOWERBOUND`              | Как `SET_RANGE`, но требует допустимый data-аргумент для DUPSORT |
| `MDBX_SET_UPPERBOUND`              | Найти первый ключ > заданного                                    |
| `MDBX_GET_BOTH` / `GET_BOTH_RANGE` | (DUPSORT) найти конкретное значение / первое ≥ значения          |

После `MDBX_LAST`, если данные закончились, операция возвращает `MDBX_NOTFOUND`, а курсор
оказывается «на конце» — состояние `eof`. Это штатная ситуация для завершения обхода.

Для частых сценариев есть отдельные удобные функции вместо пары «`cursor_get(op)` +
проверка кода»: `mdbx_cursor_on_first()`, `mdbx_cursor_on_last()` и их dup-варианты
`mdbx_cursor_on_first_dup()`/`mdbx_cursor_on_last_dup()`; проверить «конец обхода» — логический
`mdbx_cursor_eof()`. Дистанцию между двумя позициями курсора можно узнать через
`mdbx_cursor_distance()` — полезно для оценки объёма диапазона перед обработкой.

**Фрагмент из [`examples/c++/05-cursors.c++`](examples/c++/05-cursors.c++)** — прямая итерация (FIRST/NEXT) и `SET_RANGE` («ближайший больший или равный»):

```cpp
    auto cur = rtxn.open_cursor(table);

    // Прямая итерация: FIRST затем NEXT до конца.
    size_t forward_count = 0;
    std::string forward;
    for (auto r = cur.to_first(); r; r = cur.to_next(false)) {
      forward += r.key.as_string() + " ";
      ++forward_count;
    }
    std::cout << "forward (" << forward_count << "): " << forward << "\n";

    // SET_RANGE: первый ключ, не меньший заданного.
    auto r = cur.to_key_greater_or_equal(mdbx::slice("k5"), false);
    std::cout << "set_range(k5) -> ";
    if (r)
      std::cout << r.key.as_string() << "\n";
    else
      std::cout << "<none>\n";
    r = cur.to_key_greater_or_equal(mdbx::slice("k5x"), false);
    std::cout << "set_range(k5x) -> ";
    if (r)
      std::cout << r.key.as_string() << "\n";
    else
      std::cout << "<none>\n";
```

Полный код: [05-cursors.c++](examples/c++/05-cursors.c++) · [C-версия](examples/c/05-cursors.c)

### 6.4. Пример: обход таблицы и поиск диапазона

```c
#include <stdio.h>
#include <string.h>
#include <mdbx.h>

static void die(const char *w, int rc) {
    fprintf(stderr, "%s: %s (%d)\n", w, mdbx_strerror(rc), rc);
    exit(1);
}

int main(void) {
    MDBX_env *env = NULL; MDBX_txn *txn = NULL; MDBX_dbi dbi;
    MDBX_cursor *cur = NULL;
    MDBX_val k, d;
    int rc;

    rc = mdbx_env_create(&env); if (rc) die("create", rc);
    rc = mdbx_env_open(env, "./cur.mdbx", MDBX_NOSUBDIR, 0664);
    if (rc) die("open", rc);
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc) die("txn", rc);
    rc = mdbx_dbi_open(txn, "kv", MDBX_CREATE, &dbi);
    if (rc) die("dbi", rc);

    /* наполним таблицу */
    const char *words[] = {"alpha","bravo","charlie","delta","echo"};
    for (int i = 0; i < 5; i++) {
        k.iov_base = (void *)words[i]; k.iov_len = strlen(words[i]);
        d.iov_base = (void *)"x"; d.iov_len = 1;
        rc = mdbx_put(txn, dbi, &k, &d, 0); if (rc) die("put", rc);
    }

    /* полный обход */
    rc = mdbx_cursor_open(txn, dbi, &cur); if (rc) die("cursor", rc);
    printf("Все ключи:\n");
    while ((rc = mdbx_cursor_get(cur, &k, &d, MDBX_NEXT)) == MDBX_SUCCESS)
        printf("  %.*s\n", (int)k.iov_len, (char *)k.iov_base);
    /* на конце — MDBX_NOTFOUND, это нормально */
    if (rc != MDBX_NOTFOUND) die("next", rc);

    /* диапазон: первый ключ >= "c" и все следующие */
    printf("Диапазон >= c:\n");
    const char *from = "c";
    k.iov_base = (void *)from; k.iov_len = strlen(from);
    rc = mdbx_cursor_get(cur, &k, &d, MDBX_SET_RANGE);
    if (rc == MDBX_SUCCESS) {
        do {
            printf("  %.*s\n", (int)k.iov_len, (char *)k.iov_base);
        } while ((rc = mdbx_cursor_get(cur, &k, &d, MDBX_NEXT)) == MDBX_SUCCESS);
    }
    if (rc != MDBX_NOTFOUND) die("range", rc);

    mdbx_cursor_close(cur);
    mdbx_txn_commit(txn);
    mdbx_env_close(env);
    return 0;
}
```

### 6.5. Курсор после удаления — UB (критическое предупреждение)

> **Внимание:** после `mdbx_cursor_del()` позиция курсора не определена. Использование курсора
> без повторного позиционирования — неопределённое поведение. Всегда после удаления делайте
> `NEXT` (или `PREV`) и проверяйте `MDBX_NOTFOUND`.

### 6.6. Клонирование курсоров

`mdbx_cursor_clone()` размножает позицию курсора (для параллельной обработки на одном снапшоте),
`mdbx_cursor_bind()` — привязывает существующий курсор к другой транзакции/таблице.

### 6.7. Паттерн safe-delete в DUPSORT

Удаление значений в DUPSORT-таблице одним курсором во время итерации — опасно (известны баги в
0.12.x, подробно в Томе V). Безопасный паттерн — **два курсора**: один позиционируется, второй
удаляет.

**Фрагмент (C, иллюстрация):** полная компилируемая версия — ниже и в
[`examples/c++/06-dupsort-delete.c++`](examples/c++/06-dupsort-delete.c++).

```c
/* удалить все значения ключа "target" в DUPSORT-таблице */
MDBX_cursor *it, *del;
mdbx_cursor_open(txn, dbi, &it);
mdbx_cursor_open(txn, dbi, &del);
const char *tgt = "target";
k.iov_base = (void *)tgt; k.iov_len = strlen(tgt);
rc = mdbx_cursor_get(it, &k, &d, MDBX_SET);          /* первый dup */
while (rc == MDBX_SUCCESS) {
    MDBX_val dk = k, dv = d;
    /* второй курсор на ту же позицию */
    rc = mdbx_cursor_get(del, &dk, &dv, MDBX_GET_BOTH);
    if (rc != MDBX_SUCCESS) break;
    rc = mdbx_cursor_del(del, MDBX_NODUPDATA);
    if (rc) break;
    rc = mdbx_cursor_get(it, &k, &d, MDBX_NEXT_DUP);
}
mdbx_cursor_close(it);
mdbx_cursor_close(del);
```

Для массового удаления целых диапазонов лучше использовать `mdbx_cursor_bunch_delete()`
(«удаление гроздьями») — оно вырезает целые страницы и ветви, а не перебирает элементы.

**Фрагмент из [`examples/c++/06-dupsort-delete.c++`](examples/c++/06-dupsort-delete.c++)** — паттерн safe-delete двумя курсорами: `it` итерирует по дубликатам (`NEXT_DUP`), `del` позиционируется через `GET_BOTH` и удаляет текущее значение:

```cpp
    auto txn = env.start_write();
    auto multi = txn.open_map("multi", mdbx::key_mode::usual, mdbx::value_mode::multi);
    dump_key(txn, multi, "values before delete (target):", mdbx::slice("target"));

    // Паттерн «два курсора»: `it` итерирует по дубликатам ключа,
    // `del` удаляет текущую пару (позиция `it` при этом остаётся валидной).
    const mdbx::slice target("target");
    auto it = txn.open_cursor(multi);
    auto del = txn.open_cursor(multi);
    size_t deleted = 0;
    // Первый dup ключа сохраняем, остальные удаляем.
    auto pos = it.to_key_exact(target);
    if (pos)
      pos = it.to_current_next_multi(false);
    for (; pos; pos = it.to_current_next_multi(false)) {
      del.to_exact_key_value_equal(pos.key, pos.value, false); // MDBX_GET_BOTH
      if (del.erase(false)) // MDBX_CURRENT: удалить только текущее значение
        ++deleted;
    }
```

Полный код: [06-dupsort-delete.c++](examples/c++/06-dupsort-delete.c++) · [C-версия](examples/c/06-dupsort-delete.c)

> **Примеры к главе:** [`examples/c++/05-cursors.c++`](examples/c++/05-cursors.c++) · [C-версия](examples/c/05-cursors.c);
> [`examples/c++/06-dupsort-delete.c++`](examples/c++/06-dupsort-delete.c++) · [C-версия](examples/c/06-dupsort-delete.c);
> сквозной проект: [`config-store-06.c++`](examples/config-store/config-store-06.c++).

### 6.8. Резюме главы 6

- Курсор — позиция в упорядоченном дереве; `mdbx_cursor_get(op)` управляет перемещением.
- `MDBX_SET_RANGE` — основа диапазонных запросов.
- После `cursor_del` позиция недействительна — перепозиционируйтесь.
- Для удаления в DUPSORT — два курсора или `bunch_delete`.

### 6.9. Упражнения

1. Напишите функцию обхода всех ключей от конца к началу (`MDBX_LAST` + `MDBX_PREV`).
2. Найдите «ключ, следующий за X» двумя способами: `SET_RANGE` и `SET_UPPERBOUND`.
3. Удалите каждый второй ключ в обходе и объясните, почему позиционирование обязано обновляться.

### 6.10. Чек-лист главы 6

- [ ] я умею открывать и закрывать курсор и понимаю, что он живёт не дольше своей транзакции;
- [ ] свободно пользуюсь `MDBX_FIRST`/`MDBX_LAST`/`MDBX_NEXT`/`MDBX_PREV` и `MDBX_SET_RANGE` для диапазонных запросов;
- [ ] помню, что после `mdbx_cursor_del()` позиция не определена, и всегда перепозиционируюсь;
- [ ] умею применять паттерн «два курсора» для безопасного удаления в DUPSORT;
- [ ] могу пройти таблицу «от конца к началу» (`MDBX_LAST` + `MDBX_PREV`) и отличить конец обхода (`MDBX_NOTFOUND`, `mdbx_cursor_eof()`) от ошибки.

### 6.11. Что дальше

Следующая глава расширяет модель данных: флаг `MDBX_DUPSORT` превращает таблицу в мультикарту
«ключ → упорядоченное множество значений». Мы разберём формы хранения, флаги `MDBX_DUPFIXED`,
`MDBX_INTEGERDUP`, `MDBX_REVERSEDUP`, навигацию по дубликатам и главный приём — инвертированный
индекс «поле → список ID». Именно он станет основой вторичных индексов конфигуратора в главе 8.

---

## Глава 7. Мультизначения и DUPSORT

### 7.1. Что такое DUPSORT

Обычная таблица хранит один ключ → одно значение. Флаг `MDBX_DUPSORT` превращает таблицу в
**мультикарту**: один ключ → **упорядоченное множество значений**. Значение играет роль «второго
ключа» со своим порядком сортировки.

Пример: теги пользователя `user:1001 → {"admin", "staff", "vip"}`.

### 7.2. Формы хранения

Внутри (подробно в Томе III) движение значений выбирается по количеству:

- немного значений на ключ — плотная **суб-страница** (вложенная страница в листе);
- много значений — отдельное **вложенное B+tree**;
- фиксированный размер значений — специализированные **dupfix-страницы** плотной упаковки.

Важное следствие: значение-«мультизначение» не уходит на overflow-страницы, поэтому ограничено
рамками ключа (порядка половины страницы).

### 7.3. Флаги DUPSORT

| Флаг              | Смысл                                                                                 |
| ----------------- | ------------------------------------------------------------------------------------- |
| `MDBX_DUPSORT`    | Таблица — мультикарта                                                                 |
| `MDBX_DUPFIXED`   | Все значения фиксированной длины (требует одинаковой длины! иначе `MDBX_BAD_VALSIZE`) |
| `MDBX_INTEGERDUP` | Значения — `uint32_t`/`uint64_t` нативного порядка (требует `DUPFIXED`+`DUPSORT`)     |
| `MDBX_REVERSEDUP` | Обратный порядок сортировки значений                                                  |

### 7.4. Операции с мультизначениями

```c
/* добавить значение к ключу (MDBX_NODUPDATA защищает от дублей) */
mdbx_put(txn, dbi, &key, &data, MDBX_NODUPDATA);

/* найти конкретное значение */
mdbx_cursor_get(cur, &key, &data, MDBX_GET_BOTH);       /* точно */
mdbx_cursor_get(cur, &key, &data, MDBX_GET_BOTH_RANGE); /* первое >= data */

/* навигация по значениям ключа */
mdbx_cursor_get(cur, &key, &data, MDBX_FIRST_DUP);
mdbx_cursor_get(cur, &key, &data, MDBX_LAST_DUP);
mdbx_cursor_get(cur, &key, &data, MDBX_NEXT_DUP);
mdbx_cursor_get(cur, &key, &data, MDBX_PREV_DUP);

/* число значений ключа */
size_t count;
mdbx_cursor_count(cur, &count);
```

### 7.5. Инвертированные индексы: паттерн «ключ → список ID»

Классическое применение DUPSORT — **инвертированный индекс**: значение поля → список идентификаторов
записей. Это «дешёвый вторичный индекс» без отдельной таблицы-связки.

**Фрагмент (C, иллюстрация):** полная компилируемая версия — ниже и в
[`examples/c++/08-inverted-index.c++`](examples/c++/08-inverted-index.c++).

```c
/* индекс "role" -> список user_id */
MDBX_dbi idx_role;
mdbx_dbi_open(txn, "idx_role", MDBX_CREATE | MDBX_DUPSORT, &idx_role);

/* при создании пользователя: role = "admin", id = 1001 */
MDBX_val k = { (void *)"admin", 5 };
MDBX_val v = { &user_id_u64, sizeof(user_id_u64) };
mdbx_put(txn, idx_role, &k, &v, MDBX_NODUPDATA);
```

Получение списка админов — это курсор по `idx_role` с ключом `"admin"` и `NEXT_DUP`.

**Фрагмент из [`examples/c++/08-inverted-index.c++`](examples/c++/08-inverted-index.c++)** — построение инвертированного индекса «слово → список ID документов» поверх DUPSORT-таблицы (`value_mode::multi`):

```cpp
    {
      auto txn = env.start_write();
      auto docs = txn.create_map("docs", mdbx::key_mode::usual, mdbx::value_mode::single);
      // Индекс: ключ — слово, значения — ID документов (мультизначения).
      auto words = txn.create_map("words", mdbx::key_mode::usual, mdbx::value_mode::multi);

      static const struct {
        const char *id;
        const char *text;
      } entries[] = {{"doc0", "the quick brown fox"}, {"doc1", "quick red fox"}, {"doc2", "lazy brown dog"}};

      for (const auto &e : entries) {
        txn.insert(docs, mdbx::slice(e.id), mdbx::slice(e.text));
        std::istringstream stream(e.text);
        std::string word;
        while (stream >> word)
          txn.upsert(words, mdbx::slice(word), mdbx::slice(e.id)); // UPSERT добавляет ID к слову
      }
      txn.commit();
    }
```

Полный код: [08-inverted-index.c++](examples/c++/08-inverted-index.c++) · [C-версия](examples/c/08-inverted-index.c)

### 7.6. Полный пример: вторичный индекс на DUPSORT

```c
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <mdbx.h>

static void die(const char *w, int rc) {
    fprintf(stderr, "%s: %s (%d)\n", w, mdbx_strerror(rc), rc);
    exit(1);
}

int main(void) {
    MDBX_env *env = NULL; MDBX_txn *txn = NULL;
    MDBX_dbi users, by_role;
    MDBX_val k, v, d;
    MDBX_cursor *cur = NULL;
    int rc;

    rc = mdbx_env_create(&env); if (rc) die("create", rc);
    rc = mdbx_env_open(env, "./idx.mdbx", MDBX_NOSUBDIR, 0664);
    if (rc) die("open", rc);
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc) die("txn", rc);

    mdbx_dbi_open(txn, "users", MDBX_CREATE | MDBX_INTEGERKEY, &users);
    mdbx_dbi_open(txn, "by_role", MDBX_CREATE | MDBX_DUPSORT, &by_role);

    /* user_id -> name;  by_role["admin"] -> {id,...} */
    struct { uint64_t id; const char *name; const char *role; } rows[] = {
        {1001, "Анна",  "admin"}, {1002, "Борис", "user"},
        {1003, "Виктор","admin"}, {1004, "Галина", "user"},
    };
    for (int i = 0; i < 4; i++) {
        k.iov_base = &rows[i].id; k.iov_len = sizeof(uint64_t);
        v.iov_base = (void *)rows[i].name; v.iov_len = strlen(rows[i].name);
        mdbx_put(txn, users, &k, &v, 0);

        k.iov_base = (void *)rows[i].role; k.iov_len = strlen(rows[i].role);
        v.iov_base = &rows[i].id; v.iov_len = sizeof(uint64_t);
        mdbx_put(txn, by_role, &k, &v, MDBX_NODUPDATA);
    }

    /* вывести всех админов через индекс */
    const char *role = "admin";
    k.iov_base = (void *)role; k.iov_len = strlen(role);
    mdbx_cursor_open(txn, by_role, &cur);
    rc = mdbx_cursor_get(cur, &k, &d, MDBX_SET);
    if (rc == MDBX_SUCCESS) {
        printf("Админы:\n");
        do {
            printf("  user id = %llu\n",
                   (unsigned long long)*(uint64_t *)d.iov_base);
        } while ((rc = mdbx_cursor_get(cur, &k, &d, MDBX_NEXT_DUP)) == MDBX_SUCCESS);
    }
    mdbx_cursor_close(cur);

    mdbx_txn_commit(txn);
    mdbx_env_close(env);
    return 0;
}
```

> **Примеры к главе:** [`examples/c++/07-dupsort.c++`](examples/c++/07-dupsort.c++) · [C-версия](examples/c/07-dupsort.c);
> [`examples/c++/08-inverted-index.c++`](examples/c++/08-inverted-index.c++) · [C-версия](examples/c/08-inverted-index.c).

### 7.7. Резюме главы 7

- `MDBX_DUPSORT` = ключ → упорядоченное множество значений.
- `DUPFIXED` требует одинаковых длин; `INTEGERDUP` — целочисленные значения нативного порядка.
- Навигация по значениям: `FIRST_DUP/LAST_DUP/NEXT_DUP/PREV_DUP/GET_BOTH`.
- Инвертированный индекс на DUPSORT — дешёвый вторичный индекс.

### 7.8. Упражнения

1. Постройте индекс «по возрасту» и выведите всех пользователей старше 30.
2. Объясните, зачем `MDBX_NODUPDATA` при добавлении в DUPSORT-индекс.
3. Что произойдёт при попытке `DUPFIXED`-вставки значения другой длины?

### 7.9. Чек-лист главы 7

- [ ] я понимаю, что `MDBX_DUPSORT` — это «ключ → упорядоченное множество значений», и значение играет роль второго ключа;
- [ ] знаю, когда требуются `MDBX_DUPFIXED` (одинаковая длина значений) и `MDBX_INTEGERDUP`;
- [ ] умею находить и перебирать значения ключа: `MDBX_GET_BOTH`, `MDBX_GET_BOTH_RANGE`, `MDBX_FIRST_DUP`/`NEXT_DUP`, `mdbx_cursor_count()`;
- [ ] добавляю значения с `MDBX_NODUPDATA`, чтобы не плодить дубликаты;
- [ ] умею построить инвертированный индекс «слово → список ID» поверх DUPSORT-таблицы.

### 7.10. Что дальше

Мультизначения — полуфабрикат; в главе 8 из него собирается законченный приём: вторичный индекс.
Мы построим схему «основная таблица + индексные», научимся поддерживать их консистентность в одной
транзакции, сравним DUPSORT-индекс с отдельной таблицей и познакомимся с составными ключами и
удалением по индексу. Для сквозного проекта это шаг к «мини-ORM» с сущностью «пользователи».

---

## Глава 8. Вторичные индексы

### 8.1. Зачем нужны вторичные индексы в KV-базе

В key-value базе нет SQL-индексов: данные ищутся только по первичному ключу. Чтобы искать «по
другому полю», мы строим индекс **сами** — отдельную таблицу, где ключом становится искомое поле,
а значением — первичный ключ исходной записи.

### 8.2. Паттерн: основная таблица + индексные

```
users:        user_id -> {name, email, ...}        (первичная)
idx_email:    email   -> user_id                    (индекс)
idx_by_role:  role    -> user_id  (DUPSORT)         (индекс)
```

Поддержание консистентности — **на вашей стороне**: при каждом изменении `users` обновите все
индексы в **той же транзакции** (иначе будет рассинхрон).

**Фрагмент из [`examples/c++/09-secondary-index.c++`](examples/c++/09-secondary-index.c++)** — основная таблица и индекс обновляются в одной транзакции: при вставке пользователей — и при удалении (запись + её индексные ссылки):

```cpp
    {
      auto txn = env.start_write();
      auto users = txn.create_map("users", mdbx::key_mode::usual, mdbx::value_mode::single);
      // Индекс: ключ — роль, значения — ID пользователей (мультизначения).
      auto by_role = txn.create_map("by_role", mdbx::key_mode::usual, mdbx::value_mode::multi);

      // Вставка пользователей и поддержание индекса — в одной транзакции.
      txn.insert(users, mdbx::slice("user1"), mdbx::slice("Alice|admin"));
      txn.upsert(by_role, mdbx::slice("admin"), mdbx::slice("user1"));
      txn.insert(users, mdbx::slice("user2"), mdbx::slice("Bob|dev"));
      txn.upsert(by_role, mdbx::slice("dev"), mdbx::slice("user2"));
      txn.insert(users, mdbx::slice("user3"), mdbx::slice("Carol|admin"));
      txn.upsert(by_role, mdbx::slice("admin"), mdbx::slice("user3"));
      txn.commit();
    }

    // Удаление пользователя вместе с его записью в индексе.
    {
      auto txn = env.start_write();
      auto users = txn.open_map("users", mdbx::key_mode::usual, mdbx::value_mode::single);
      auto by_role = txn.open_map("by_role", mdbx::key_mode::usual, mdbx::value_mode::multi);
      txn.erase(users, mdbx::slice("user2"));
      txn.erase(by_role, mdbx::slice("dev"), mdbx::slice("user2")); // значение конкретного ключа
      txn.commit();
    }
```

Полный код: [09-secondary-index.c++](examples/c++/09-secondary-index.c++)

> **Нюанс: открытие таблицы с неизвестными флагами.** Если таблица могла быть создана другим
> кодом (и её постоянные флаги — `MDBX_DUPSORT`, `MDBX_INTEGERKEY` и т.п. — заранее неизвестны),
> передайте при открытии `MDBX_DB_ACCEDE`: вместо `MDBX_INCOMPATIBLE` таблица откроется со своими
> фактическими флагами. Узнать их можно через `mdbx_dbi_flags()` (точная версия —
> `mdbx_dbi_flags_ex()` — дополнительно возвращает state-биты таблицы:
> `MDBX_DBI_CREAT`/`MDBX_DBI_DIRTY`/`MDBX_DBI_FRESH`/`MDBX_DBI_STALE`). Это же касается повторного
> открытия таблиц в новой транзакции: хендл `MDBX_dbi` из старой транзакции не переносится в новую,
> а `mdbx_dbi_open()` в новой транзакции без тех же флагов вернёт `MDBX_INCOMPATIBLE`.

### 8.3. DUPSORT-индекс vs отдельная таблица

- **DUPSORT-индекс** («поле → список ID») идеален для «один ко многим»: список адресатов — это
  значения одного ключа. Быстрое добавление/удаление одного ID.
- **Отдельная таблица** («составной ключ → маркер») — когда нужно больше данных об отношении или
  составные условия.

### 8.4. Составные индексы

Составной ключ — конкатенация полей в одном ключе. Порядок байт критичен: чтобы числовое поле в
составном ключе сравнивалось корректно, его нужно приводить к **big-endian** порядку (или
использовать целочисленные типы нативного порядка с отдельным компаратором).

**Фрагмент из [`examples/c++/10-composite-key.c++`](examples/c++/10-composite-key.c++)** — упаковка двух полей в составной ключ с big-endian порядком байт: лексикографический порядок ключа совпадает с числовым порядком пары:

```cpp
// Упаковка (x, y) в uint64_t так, чтобы побайтовый (лексикографический) порядок
// ключа совпадал с числовым порядком пары. Для этого используется big-endian
// представление беззнаковых полей фиксированной ширины.
uint64_t pack(uint32_t x, uint32_t y) {
  const uint64_t value = (uint64_t(x) << 32) | uint64_t(y);
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return __builtin_bswap64(value);
#else
  return value;
#endif
}
```

Полный код: [10-composite-key.c++](examples/c++/10-composite-key.c++)

### 8.5. Удаление по индексу

Удалять через индекс нельзя напрямую — найдите первичный ключ по индексу, затем удалите запись и
обновите индексы. Используйте безопасный паттерн итерации (глава 6).

### 8.6. Мини-ORM поверх libmdbx (сквозной проект)

Расширяем конфигуратор: добавляем сущность «пользователи» с индексом по имени.

```c
/* schema: users(id -> name), idx_name(name -> id) */
int user_add(config_store_t *cs, uint64_t id, const char *name) {
    MDBX_txn *txn; MDBX_dbi users, idx;
    MDBX_val k, v;
    int rc;

    rc = mdbx_txn_begin(cs->env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc) return rc;
    mdbx_dbi_open(txn, "users", MDBX_CREATE | MDBX_INTEGERKEY, &users);
    mdbx_dbi_open(txn, "idx_name", MDBX_CREATE | MDBX_DUPSORT, &idx);

    k.iov_base = &id; k.iov_len = sizeof(id);
    v.iov_base = (void *)name; v.iov_len = strlen(name);
    rc = mdbx_put(txn, users, &k, &v, MDBX_NOOVERWRITE);
    if (rc == MDBX_KEYEXIST) { mdbx_txn_abort(txn); return rc; }

    /* индекс: name -> id */
    k.iov_base = (void *)name; k.iov_len = strlen(name);
    v.iov_base = &id; v.iov_len = sizeof(id);
    rc = mdbx_put(txn, idx, &k, &v, MDBX_NODUPDATA);

    rc = mdbx_txn_commit(txn);
    return rc;
}
```

> **Нюанс:** атомарность гарантирует именно «всё или ничего»: если падение случится между
> операциями, транзакция откатится целиком. Это и есть цена правильных индексов — обновляйте их
> только в одной транзакции с основной записью.

> **Примеры к главе:** [`examples/c++/09-secondary-index.c++`](examples/c++/09-secondary-index.c++);
> [`examples/c++/10-composite-key.c++`](examples/c++/10-composite-key.c++);
> сквозной проект: [`config-store-08.c++`](examples/config-store/config-store-08.c++).

### 8.7. Резюме главы 8

- Вторичный индекс — таблица «поле → первичный ключ», которую вы ведёте сами.
- Все изменения (данные + индексы) — в одной транзакции.
- DUPSORT-индекс для «один ко многим»; составные ключи для нескольких полей.
- Удаление записи — вместе с её индексными ссылками; иначе индекс «протухнет».
- Инвертированный индекс поверх DUPSORT — дешёвая альтернатива отдельной таблице-связке.

### 8.8. Упражнения

1. Добавьте индекс «по городу» и напишите функцию поиска.
2. Напишите `user_rename`, которая атомарно меняет имя и перестраивает `idx_name`.
3. Почему составные ключи требуют аккуратности с порядком байт?

### 8.9. Чек-лист главы 8

- [ ] я понимаю, что вторичный индекс — это таблица «поле → первичный ключ», которую веду сам;
- [ ] обновляю данные и все индексы в одной транзакции, помня о цене нарушения этого правила;
- [ ] осознанно выбираю между DUPSORT-индексом («поле → список ID») и отдельной таблицей;
- [ ] знаю, зачем в составных ключах числовые поля приводятся к big-endian;
- [ ] удаляю запись вместе с её индексными ссылками; иначе индекс «протухнет»;
- [ ] помню про `MDBX_DB_ACCEDE` на случай, когда флаги таблицы заранее неизвестны.

### 8.10. Что дальше

Индексы и данные упираются в следующий вопрос: как настроить само окружение. Глава 9 — о геометрии
файла (`mdbx_env_set_geometry()`), лимитах `maxreaders`/`maxdbs`, флагах окружения, runtime-опциях
`MDBX_opt_*` и статистике. Конфигуратору, который растёт от тестовой базы до рабочей, нужны
управляемый размер файла и предсказуемые лимиты — иначе рано или поздно придёт `MDBX_MAP_FULL`.

---

## Глава 9. Конфигурация окружения

### 9.1. Геометрия файла

Геометрия определяет, как растёт и сжимается файл базы:

```c
int mdbx_env_set_geometry(MDBX_env *env,
    intptr_t size_lower, intptr_t size_now, intptr_t size_upper,
    intptr_t growth_step, intptr_t shrink_threshold, unsigned pagesize);
```

Параметры (значение `-1` = «по умолчанию/не менять»):

| Параметр           | Смысл                                                                 |
| ------------------ | --------------------------------------------------------------------- |
| `size_lower`       | Нижняя граница размера файла (не сжиматься ниже)                      |
| `size_now`         | Начальный размер при создании                                         |
| `size_upper`       | **Жёсткий предел**; при достижении и нехватке места — `MDBX_MAP_FULL` |
| `growth_step`      | Шаг роста файла                                                       |
| `shrink_threshold` | Порог, при котором файл может усечься                                 |
| `pagesize`         | Размер страницы (256…65536; задаётся до первого open)                 |

> **Внимание:** геометрию задавайте **один раз, до `env_open`** (или при создании). Менять `upper`
> «на ходу» надёжно нельзя. Значение `upper` для новой БД движок выбирает сам (~золотое сечение ОЗУ,
> ограничено mmap-лимитом ≈140 ТБ на 64-бит); `TOO_LARGE`/`ENOMEM` возможны при явно чрезмерном
> `upper` (например, под ASAN/Valgrind) — всегда задавайте адекватный `upper` явно.

> **Историческая справка.** `mdbx_env_set_mapsize(env, size)` — устаревшая обёртка над
> `mdbx_env_set_geometry()`, эквивалентная вызову `mdbx_env_set_geometry(env, size, size, size, -1, -1, -1)`.
> Как и геометрия, она меняет размер БД «на лету» (в том числе после `mdbx_env_open()`), но
> приравнивает нижнюю/текущую/верхнюю границы к одному значению — база теряет возможность
> авто-роста выше `size`. Предпочитайте `mdbx_env_set_geometry()` с раздельными параметрами.
> Аналогично `mdbx_env_sync()` — устаревший синоним `mdbx_env_sync_ex(env, force=true, nonblock=false)`,
> т.е. блокирующая полная синхронизация (глава 10).

### 9.2. maxreaders / maxdbs / pagesize

```c
mdbx_env_set_maxreaders(env, readers);  /* слоты таблицы читателей, до open */
mdbx_env_set_maxdbs(env, dbs);          /* лимит именованных таблиц */
/* размер страницы задаётся аргументом pagesize в mdbx_env_set_geometry() — отдельной функции нет */
```

Текущие значения читаются парами геттеров: `mdbx_env_get_maxreaders()`/`mdbx_env_get_maxdbs()`.

`maxreaders` — сколько потоков одновременно могут держать read-транзакции. Не занижайте его в
приложениях с пулами потоков.

### 9.3. Флаги окружения

```c
mdbx_env_set_flags(env, flags, onoff);
mdbx_env_get_flags(env, &flags);
```

Ключевые флаги: `MDBX_RDONLY`, `MDBX_WRITEMAP`, `MDBX_NOMETASYNC`, `MDBX_SAFE_NOSYNC`,
`MDBX_UTTERLY_NOSYNC`, `MDBX_NOSTICKYTHREADS`, `MDBX_EXCLUSIVE`.

Ещё два важных флага передаются **при `mdbx_env_open()`**, а не через `set_flags`:

- `MDBX_ACCEDE` — открыть базу, **уже используемую другим процессом в неизвестном режиме**: вместо
  ошибки `MDBX_INCOMPATIBLE` окружение откроется в совместимом с текущим использованием режиме
  (применяется к флагам долговечности, `MDBX_LIFORECLAIM` и `MDBX_NORDAHEAD`). Не влияет, если
  текущий процесс — единственный или все открытия read-only.
- `MDBX_EXCLUSIVE` — монопольное открытие: успех только если база не открыта никем другим.

Пара полезных функций окружения: `mdbx_env_warmup()` — «прогреть» файл БД в оперативной памяти
(кэш страниц) до начала работы; `mdbx_env_get_fd()` — файловый дескриптор данных (для своих
fsync/резервного копирования).

### 9.4. Runtime options (MDBX_opt_*)

Тонкая настройка через `mdbx_env_set_option`/`mdbx_env_get_option`:

| Опция                                          | Смысл                                                                                      |
| ---------------------------------------------- | ------------------------------------------------------------------------------------------ |
| `MDBX_opt_rp_augment_limit`                    | Предел накопления списков при поиске последовательностей в GC                              |
| `MDBX_opt_gc_time_limit`                       | Тайм-лимит на поиск в GC в пишущей транзакции (1/65536 с)                                  |
| `MDBX_opt_txn_dp_limit`                        | Лимит грязных страниц транзакции (по умолчанию ≈1/42 ОЗУ)                                  |
| `MDBX_opt_loose_limit`                         | Кэш loose-страниц (по умолчанию 64)                                                        |
| `MDBX_opt_writethrough_threshold`              | Порог выбора `O_DSYNC` vs `fdatasync` (дефолт 2 грязных страницы; на Windows игнорируется) |
| `MDBX_opt_prefault_write_enable`               | Упреждающая запись для WRITEMAP                                                            |
| `MDBX_opt_sync_bytes` / `MDBX_opt_sync_period` | Авто-синхронизация (дефолты: ~1.05 ГБ / 42.42 с; явный 0 = отключено)                      |
| `MDBX_opt_merge_threshold`                     | Порог слияния страниц (16.16 %; дефолт 33%, диапазон [12.5%..50%])                         |
| `MDBX_opt_prefer_waf_insteadof_balance`        | Предпочтение грязного соседа при слиянии (дефолт true с 2026-01-04)                        |

**Фрагмент из [`examples/c++/11-geometry-options.c++`](examples/c++/11-geometry-options.c++)** — задание геометрии до `open` (через `create_parameters().set_geometry(geo)`) и runtime-опций после:

```cpp
    // Геометрия: нижняя/текущая/верхняя границы размера БД, шаг роста,
    // порог сжатия. Здесь — компактная БД для примера.
    mdbx::env::geometry geo;
    geo.size_lower = 8 * mdbx::env::geometry::MB;
    geo.size_now = 16 * mdbx::env::geometry::MB;
    geo.size_upper = 64 * mdbx::env::geometry::MB;
    geo.growth_step = 8 * mdbx::env::geometry::MB;
    geo.shrink_threshold = 2 * mdbx::env::geometry::MB;

    mdbx::env_managed env(path, mdbx::env_managed::create_parameters().set_geometry(geo),
                          mdbx::env::operate_parameters());

    // Runtime-опции: устанавливаем и читаем обратно.
    env.set_extra_option(mdbx::env::extra_runtime_option::writethrough_threshold, 1 << 20);
    env.set_extra_option(mdbx::env::extra_runtime_option::merge_threshold_dot16, 65536 / 3);
    env.set_sync_threshold(256 * 1024); // sync_bytes = 256 KiB
    env.set_extra_option(mdbx::env::extra_runtime_option::prefault_write_enable, 1);
```

Полный код: [11-geometry-options.c++](examples/c++/11-geometry-options.c++) · [C-версия](examples/c/11-geometry-options.c)

### 9.5. Выбор размера страницы

| Страница | Когда                                                             |
| -------- | ----------------------------------------------------------------- |
| 4 КБ     | По умолчанию; универсально                                        |
| 8 КБ     | Страховка от большой фрагментированной GC                         |
| 64 КБ    | Большие значения (меньше overflow-прогонов); дороже мелкие записи |

### 9.6. Статистика

```c
mdbx_env_info_ex(env, &info, sizeof(info));  /* геометрия, меты, счётчики */
mdbx_env_stat_ex(env, txn, &stat, sizeof(stat)); /* размеры, страницы */
mdbx_dbi_stat_ex(txn, dbi, &dst, sizeof(dst), 0); /* статистика таблицы */
```

**Фрагмент из [`examples/c++/12-env-stat.c++`](examples/c++/12-env-stat.c++)** — чтение статистики окружения (аналоги `mdbx_env_stat_ex()`/`mdbx_env_info_ex()`):

```cpp
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 100; ++i) {
        const auto key = std::to_string(i);
        const auto value = "value-" + std::to_string(i);
        txn.insert(table, mdbx::slice(key), mdbx::slice(value));
      }
      txn.commit();
    }

    const auto stat = env.get_stat(); // аналог mdbx_env_stat_ex()
    std::cout << "stat: ps=" << stat.ms_psize << " depth=" << stat.ms_depth
              << " leaf=" << stat.ms_leaf_pages << " branch=" << stat.ms_branch_pages
              << " overflow=" << stat.ms_overflow_pages << " entries=" << stat.ms_entries << "\n";

    const auto info = env.get_info(); // аналог mdbx_env_info_ex()
    std::cout << "info: recent_txnid=" << info.mi_recent_txnid
              << " latter_reader_txnid=" << info.mi_latter_reader_txnid << " geo.current=" << info.mi_geo.current
              << "\n";
```

Полный код: [12-env-stat.c++](examples/c++/12-env-stat.c++) · [C-версия](examples/c/12-env-stat.c)

> **Примеры к главе:** [`examples/c++/11-geometry-options.c++`](examples/c++/11-geometry-options.c++) · [C-версия](examples/c/11-geometry-options.c);
> [`examples/c++/12-env-stat.c++`](examples/c++/12-env-stat.c++) · [C-версия](examples/c/12-env-stat.c);
> сквозной проект: [`config-store-09.c++`](examples/config-store/config-store-09.c++).

### 9.7. Резюме главы 9

- Геометрия (lower/now/upper/growth/shrink/pagesize) задаётся до open.
- `upper` — жёсткий предел; заниженный → `MDBX_MAP_FULL`, явно завышенный (например, под
  ASAN/Valgrind) → `TOO_LARGE`/`ENOMEM`. Дефолт для новой БД движок выбирает сам (≈ золотое сечение ОЗУ).
- `maxreaders`/`maxdbs`/`pagesize` — до первого open.
- Опции `MDBX_opt_*` — тонкая настройка GC/спилла/синхронизации.

### 9.8. Упражнения

1. Задайте геометрию «от 1 МБ до 4 ГБ, шаг 64 МБ» до open и проверьте `env_info`.
2. Что вернёт `env_open` при явно завышенном `upper` (например, 140 ТБ на 64-бит)? Зафиксируйте код ошибки.
3. Создайте базу с страницей 64 КБ и сравните с 4 КБ на чтении больших значений.

### 9.9. Чек-лист главы 9

- [ ] я задаю геометрию (`size_lower`/`size_now`/`size_upper`/`growth_step`/`shrink_threshold`) до `env_open`;
- [ ] понимаю, что `size_upper` — жёсткий предел: при исчерпании приходит `MDBX_MAP_FULL`, а чрезмерный `upper` даёт `TOO_LARGE`/`ENOMEM`;
- [ ] знаю, где задаются `maxreaders`, `maxdbs` и размер страницы;
- [ ] различаю ключевые флаги окружения (`MDBX_WRITEMAP`, `MDBX_SAFE_NOSYNC`, `MDBX_EXCLUSIVE`, `MDBX_ACCEDE`) и основные `MDBX_opt_*`-опции;
- [ ] умею снимать статистику через `mdbx_env_info_ex()`/`mdbx_env_stat_ex()` и использовать её для диагностики.

### 9.10. Что дальше

Настроенное окружение — ещё не всё: нужно решить, что происходит при коммите. Глава 10 разбирает
режимы долговечности: weak/steady мета, чем рискуют `MDBX_NOMETASYNC` и `MDBX_SAFE_NOSYNC`, зачем
нужна авто-синхронизация и как режимы различаются на macOS, Linux и Windows. Выбор режима напрямую
определяет скорость и надёжность конфигуратора.

---

## Глава 10. Режимы долговечности

### 10.1. Что значит «закоммитить»

Строгий коммит должен: (1) записать на диск данные (новые версии страниц) и (2) записать мету —
сделать снапшот «видимым» для будущих открытий. Между этими событиями различают:

- **weak (слабая) мета** — данные в файле, но не гарантированно на постоянном носителе;
- **steady (устойчивая) мета** — данные сброшены на диск; снапшот переживает сбой системы.

### 10.2. Режимы

| Режим                              | Поведение                                               | Риск                                                                      |
| ---------------------------------- | ------------------------------------------------------- | ------------------------------------------------------------------------- |
| `MDBX_SYNC_DURABLE` (по умолчанию) | Данные → flush → мета → flush                           | Нет: полный ACID                                                          |
| `MDBX_NOMETASYNC`                  | Данные флашатся, мета — отложенно                       | Потеря последних коммитов при сбое                                        |
| `MDBX_SAFE_NOSYNC`                 | Ничего не флашится сразу, сохраняется предыдущий steady | Откат к последнему steady; **рост файла** (эффект квази-долгого читателя) |
| `MDBX_UTTERLY_NOSYNC`              | Никаких флашей, никаких steady-гарантий                 | База может не пережить сбой                                               |
| `MDBX_WRITEMAP`                    | Запись через mmap (+msync)                              | Сочетается с режимами выше                                                |

**Фрагмент из [`examples/c++/13-sync-modes.c++`](examples/c++/13-sync-modes.c++)** — смена sync-режима между открытиями одной и той же БД: полная долговечность (`robust_synchronous`) → `NOMETASYNC` (`half_synchronous_weak_last`):

```cpp
    // Прогон 1: полная долговечность (sync перед ответом коммита).
    {
      auto env = example::env_open(path, mdbx::env::operate_parameters().robust_synchronous());
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      txn.commit();
      env.sync_to_disk(true); // явный сброс на диск
      std::cout << "durable: ok\n";
    }

    // Прогон 2: NOMETASYNC (метаданные не синхронизируются при каждом коммите).
    {
      auto env = example::env_open(path, mdbx::env::operate_parameters().half_synchronous_weak_last());
      auto txn = env.start_read();
      auto table = txn.open_map(nullptr);
      std::cout << "nometasync: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";
      txn.abort();
    }
```

Полный код: [13-sync-modes.c++](examples/c++/13-sync-modes.c++) · [C-версия](examples/c/13-sync-modes.c)

### 10.3. Когда какой режим

- **По умолчанию** — максимум надёжности (ACID). Для баз, где потеря коммита неприемлема.
- **`MDBX_NOMETASYNC`** — высоконагруженная запись, где можно потерять «хвост» последних коммитов.
- **`MDBX_SAFE_NOSYNC`** — запись может ускориться в разы (до 10×), но: файл растёт (страницы
  новее steady не переиспользуются до нового steady-point), при сбое — откат к последнему steady.
- **`MDBX_UTTERLY_NOSYNC`** — только для некритичных данных/кэшей.
- **`MDBX_WRITEMAP`** — для больших транзакций и тонкой настройки записи; сочетается с флагами
  выше (например, `SAFE_NOSYNC | WRITEMAP`).

### 10.4. Авто-синхронизация

`MDBX_SAFE_NOSYNC` без управления ростом — источник бесконтрольного роста файла. Управляйте им:

```c
mdbx_env_set_syncbytes(env, 512ULL * 1024 * 1024); /* flush после 512 МБ записи */
mdbx_env_set_syncperiod(env, 3 << 16);             /* 3 секунды в формате 16.16: 3<<16 = 3·65536 единиц (сырое значение 3 ≈ 46 мкс) */
```

И вручную из отдельного потока: `mdbx_env_sync_ex(env, force, nonblock)` — при `nonblock=true` вызов
возвращается сразу (асинхронно), завершение проверяется дешёвым `mdbx_env_sync_poll(env)`.

### 10.5. Платформенные нюансы

- **macOS**: по умолчанию `fcntl(F_FULLFSYNC)` — максимум долговечности, но медленно;
  `MDBX_APPLE_SPEED_INSTEADOF_DURABILITY` меняет приоритет на скорость.
- **Linux**: `boot_id` используется для контроля отката слабых мет (важно в LXC).
- **Windows**: файловые блокировки `LockFileEx` медленнее именованных мьютексов (влияет на мелкие
  транзакции); `MDBX_WRITEMAP` помогает. Сквозная запись используется всегда, `MDBX_NOMETASYNC`
  переключает на «ленивая + flush» (порог writethrough игнорируется).

### 10.6. Пример: переключение режима

**Фрагмент (C, иллюстрация):**

```c
MDBX_env *env;
mdbx_env_create(&env);
mdbx_env_set_geometry(env, -1, -1, 8LL * 1024 * 1024 * 1024, -1, -1, -1);
mdbx_env_set_flags(env, MDBX_SAFE_NOSYNC, 1);   /* включаем */
mdbx_env_set_syncbytes(env, 256ULL * 1024 * 1024);
mdbx_env_open(env, "./safe.mdbx", MDBX_NOSUBDIR, 0664);
```

> **Примеры к главе:** [`examples/c++/13-sync-modes.c++`](examples/c++/13-sync-modes.c++) · [C-версия](examples/c/13-sync-modes.c);
> сквозной проект: [`config-store-10.c++`](examples/config-store/config-store-10.c++).

### 10.7. Резюме главы 10

- Режимы долговечности различаются тем, что флашится: данные, мета, ничего.
- `SAFE_NOSYNC` — скорость ценой роста файла и отката к steady при сбое.
- Авто-синхронизация (`syncbytes`/`syncperiod`) обязательна для `SAFE_NOSYNC`.
- Платформенные различия (F_FULLFSYNC, boot_id, LockFileEx) влияют на выбор.

### 10.8. Упражнения

1. Сравните скорость коммитов в `DURABLE` vs `SAFE_NOSYNC` на 10 000 мелких транзакциях.
2. Наблюдайте рост файла в `SAFE_NOSYNC` без авто-sync — и после настройки `syncbytes`.
3. Объясните, почему `UTTERLY_NOSYNC` — «максимальный риск»: что именно может не пережить сбой.

### 10.9. Чек-лист главы 10

- [ ] я понимаю, что строгий коммит — это «данные на диск + мета на диск», и различаю weak/steady мета;
- [ ] знаю риски каждого режима: `MDBX_SYNC_DURABLE`, `MDBX_NOMETASYNC`, `MDBX_SAFE_NOSYNC`, `MDBX_UTTERLY_NOSYNC` и сочетания с `MDBX_WRITEMAP`;
- [ ] для `MDBX_SAFE_NOSYNC` настраиваю авто-синхронизацию (`syncbytes`/`syncperiod`), чтобы контролировать рост файла;
- [ ] умею переключать режимы и вызывать `mdbx_env_sync_ex()` вручную;
- [ ] помню платформенные нюансы: `F_FULLFSYNC` на macOS, `boot_id` на Linux, `LockFileEx` на Windows.

### 10.10. Что дальше

Режимы долговечности задали правила записи; следующая глава — о работе с базой из нескольких
потоков. Глава 11 разбирает sticky threads, флаг `MDBX_NOSTICKYTHREADS` и его ловушки, TLS-слоты
читателей, `fork()`, клонирование транзакций и парковку долгоживущих читателей. Конфигуратору с
пулом потоков или корутинами без этих знаний не обойтись.

---

## Глава 11. Многопоточность

### 11.1. Модель потоков: sticky threads по умолчанию

По умолчанию транзакция «прилипает» к потоку, который её создал. Другие потоки не могут
использовать этот объект — получите `MDBX_THREAD_MISMATCH`. Читатели (разные транзакции в разных
потоках) работают параллельно и без блокировок.

**Фрагмент из [`examples/c++/14-multithreading.c++`](examples/c++/14-multithreading.c++)** — писатель коммитит новые записи, а читатели на клонах одного снапшота (`base.clone()`) продолжают видеть стабильную картину:

```cpp
    std::atomic<bool> writer_done{false};
    std::thread writer([&] {
      for (int batch = 0; batch < 2; ++batch) {
        auto txn = env.start_write();
        auto table = txn.open_map(nullptr);
        for (int i = 0; i < 10; ++i) {
          const int k = 10 + batch * 10 + i;
          txn.insert(table, mdbx::slice(std::to_string(k)), mdbx::slice("w"));
        }
        txn.commit();
        std::this_thread::yield();
      }
      writer_done = true;
    });

    // Два читателя: каждый работает со своим клоном базового снапшота.
    std::atomic<size_t> snapshot_count[2];
    std::atomic<size_t> fresh_count[2];
    std::vector<std::thread> readers;
    for (int r = 0; r < 2; ++r) {
      readers.emplace_back([&, r] {
        auto clone = base.clone(); // клон снапшота для этого потока
        auto table = clone.open_map(nullptr);
        snapshot_count[r] = clone.get_map_stat(table).ms_entries;
        clone.abort();
```

Полный код: [14-multithreading.c++](examples/c++/14-multithreading.c++)

### 11.2. MDBX_NOSTICKYTHREADS

Флаг `MDBX_NOSTICKYTHREADS` разрешает передавать транзакцию между потоками — нужно для пулов
потоков, корутин, async-runtime (tokio и т.п.), где операция может продолжиться в другом потоке.

> **Внимание:** с `NOSTICKYTHREADS` функции, требующие write-блокировку среды
> (`mdbx_env_set_option`, `set_flags`, `set_geometry`, `sync`, `stat`, `defrag`, `close`),
> могут deadlock'нуться, если пишущая транзакция «переехала» в другой поток, который ждёт
> эту блокировку. Сериализуйте такие вызовы (channel/mutex), либо держите их в потоке-владельце
> транзакции.

### 11.3. Регистрация потоков и TLS

Слот читателя привязывается к потоку через TLS. При корректном завершении потока деструктор
очищает слот. Если поток завершается «нештатно» (или TLS-деструктор не сработал — известные баги
glibc #21031/#21032), слот может «утечь». Для потоков без TLS — явная регистрация:

```c
mdbx_thread_register(env);
/* ... */
mdbx_thread_unregister(env);
```

Диагностика утечек — `mdbx_reader_check(env, &dead)` (Том V, глава 30).

### 11.4. fork()

После `fork()` наследник **не наследует** mmap- и record-блокировки. Использовать окружение в
дочернем процессе можно только после:

```c
mdbx_env_resurrect_after_fork(env);
```

Вызывается один раз в наследнике, не в родителе.

### 11.5. Клонирование транзакций

`mdbx_txn_clone()` размножает read-only транзакцию — несколько обработчиков на одном снапшоте
без повторного сканирования RLT (RLT — таблица зарегистрированных читателей, внутренняя
структура libmdbx; подробно в томе III, гл. 14) — что полезно для параллельной обработки.

### 11.6. Парковка и вытеснение

Долгоживущий читатель «пинит» **детент** (старейший активный снапшот; подробно — Том III,
глава 14) и мешает переработке страниц. Решения:

```c
mdbx_txn_park(txn);   /* освободить слот читателя, сохранив хендл */
mdbx_txn_unpark(txn); /* возобновить (при вытеснении — с restart_if_ousted=true) */
```

Если писателю не хватает пространства, припаркованные читатели **выселяются** (ousted): слот
переводится из `PARKED` в `OUSTED`; при следующем обращении читатель либо перезапустится на свежем
снапшоте, либо получит `MDBX_OUSTED`.

> **Внимание:** после парковки снапшот не удерживается — разыменовывать указатели, полученные до
> парковки, запрещено (страницы могут быть переиспользованы).

### 11.7. Несколько сред; MDBX_EXCLUSIVE

- Нельзя открывать одну и ту же БД дважды в одном процессе (защита от гонок; legacy-режим —
  `MDBX_DBG_LEGACY_MULTIOPEN`).
- Несколько разных баз (разные файлы) в одном процессе — можно.
- `MDBX_EXCLUSIVE` — монопольное открытие (только этот процесс).

> **Примеры к главе:** [`examples/c++/14-multithreading.c++`](examples/c++/14-multithreading.c++);
> [`examples/c++/15-fork-resurrect.c++`](examples/c++/15-fork-resurrect.c++);
> [`examples/c++/16-parking.c++`](examples/c++/16-parking.c++);
> сквозной проект: [`config-store-11.c++`](examples/config-store/config-store-11.c++).

### 11.8. Резюме главы 11

- Sticky threads по умолчанию; `NOSTICKYTHREADS` для пулов/корутин, но с риском deadlock в
  write-функциях.
- TLS-деструкторы чистят слоты читателей; при нештатных завершениях — `reader_check`/регистрация.
- `resurrect_after_fork` обязателен в наследнике.
- Парковка/вытеснение — решение для долгих чтений.

### 11.9. Упражнения

1. Напишите пример с двумя потоками: один пишет, второй читает. Проверьте отсутствие блокировок
   читателя.
2. Воспроизведите сценарий: write-транзакция начата в потоке A, поток B вызывает `env_sync` с
   `NOSTICKYTHREADS` — зафиксируйте deadlock.
3. Придумайте, когда `txn_clone` экономит ресурсы по сравнению с несколькими `txn_begin`.

### 11.10. Чек-лист главы 11

- [ ] я понимаю, что транзакция по умолчанию привязана к потоку, и узнаю `MDBX_THREAD_MISMATCH`;
- [ ] знаю, когда оправдан `MDBX_NOSTICKYTHREADS` и какие write-функции при нём могут deadlock'нуться;
- [ ] понимаю, как очищаются TLS-слоты читателей и чем помогают `mdbx_thread_register()`/`mdbx_thread_unregister()`;
- [ ] после `fork()` вызываю `mdbx_env_resurrect_after_fork()` в наследнике, а не в родителе;
- [ ] умею клонировать read-only транзакции (`mdbx_txn_clone()`) для параллельной обработки одного снапшота;
- [ ] использую `mdbx_txn_park()`/`mdbx_txn_unpark()` для долгоживущих читателей и помню, что припаркованный снапшот не удерживается.

### 11.11. Что дальше

Остался последний практический навык — реакция на ошибки. Глава 12 систематизирует коды возврата:
ожидаемые состояния (`MDBX_KEYEXIST`, `MDBX_NOTFOUND`), проблемы пространства (`MDBX_MAP_FULL` vs
`MDBX_TXN_FULL`), ошибки повреждения и нарушения дисциплины, а также retry-циклы для конкурентных
сценариев. После неё конфигуратор будет устойчив и к сбоям, и к конкуренции.

---

## Глава 12. Обработка ошибок

### 12.1. Коды возврата

- `MDBX_SUCCESS` (0) — успех.
- Отрицательные `MDBX_*` — ошибки (`MDBX_NOTFOUND`, `MDBX_KEYEXIST`, `MDBX_BAD_DBI`, ...).
- Специальные результаты: `MDBX_RESULT_TRUE` (операция выполнена), `MDBX_RESULT_FALSE` (нет
  данных/не требуется).
- Системные коды (errno-подобные) могут приходить из ОС — они отрицательные и выводятся как есть.

### 12.2. mdbx_strerror_r vs mdbx_strerror

`mdbx_strerror(rc)` использует общий буфер — **не потокобезопасен**. В многопоточном коде
используйте:

```c
char buf[128];
mdbx_strerror_r(rc, buf, sizeof(buf));
```

### 12.3. MDBX_MAP_FULL: что делать

`MDBX_MAP_FULL` — база упёрлась в `upper`, а переиспользовать нечего (GC пуст/заморожен). Порядок:

1. **Abort текущей write-транзакции** (продолжение после изменения геометрии → `MDBX_BAD_TXN`).
2. Проверить долгих читателей (`mdbx_stat -r` retained; `mdbx_env_info_ex`).
3. Задать/поднять `upper` **до создания/при открытии**.
4. Установить HSR-колбэк (HSR — Handle-Slow-Readers, механизм вытеснения застрявших читателей;
   Том V, глава 29).

### 12.4. MAP_FULL vs TXN_FULL

- `MDBX_MAP_FULL` — исчерпано пространство файла (геометрия).
- `MDBX_TXN_FULL` — исчерпан внутренний лимит грязных/retired-страниц транзакции. Лечение —
  коммитить раньше, дробить транзакции, проверить `mdbx_txn_info()`.

### 12.5. Ожидаемые коды

- `MDBX_KEYEXIST` — ключ уже есть (при `NOOVERWRITE`).
- `MDBX_NOTFOUND` — ключа/значения нет.
  Это штатные состояния, а не ошибки: обрабатывайте их ветвлением, а не паникой.

### 12.6. Ошибки повреждения и восстановления

- `MDBX_BAD_TXN`, `MDBX_BAD_DBI` — невалидный хендл (нарушение дисциплины/закрытая транзакция).
- `MDBX_EBADSIGN` — повреждена сигнатура.
- `MDBX_WANNA_RECOVERY` — БД требует восстановления при read-only открытии; откройте read-write
  или `mdbx_env_open_for_recovery()`.
- `MDBX_MVCC_RETARDED` — читатель старше актуального снапшота.
- `MDBX_LAGGARD_READER` — читатель отстал (см. HSR).

### 12.7. Нарушение дисциплины

- `MDBX_THREAD_MISMATCH` — транзакцию используют из чужого потока.
- `MDBX_TXN_OVERLAPPING` — пересечение read/write транзакций в одном потоке.
- `MDBX_BUSY` — конфликт владения.
- `MDBX_BAD_RSLOT` — неверный слот читателя.
  Это сигналы архитектурной ошибки, а не «случайных глюков».

### 12.8. Стратегии повторных попыток (retry loops)

Для конкурентных сценариев («сравни-и-замени»):

**Фрагмент (C, иллюстрация):**

```c
for (;;) {
    mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    /* читаем условие */
    rc = mdbx_get(txn, dbi, &key, &val);
    if (rc) { mdbx_txn_abort(txn); return rc; }
if (!condition_met(val)) { mdbx_txn_abort(txn); return MDBX_RESULT_FALSE; }
    /* пишем новое значение */
    rc = mdbx_put(txn, dbi, &key, &newval, 0);
    if (rc) { mdbx_txn_abort(txn); return rc; }
    rc = mdbx_txn_commit(txn);
    if (rc == MDBX_MAP_FULL) { /* увеличить upper/подождать читателей */ continue; }
    return rc;
}
```

**Фрагмент из [`examples/c++/17-error-handling.c++`](examples/c++/17-error-handling.c++)** — retry-цикл на «занятость» окружения (`MDBX_BUSY` при конкуренции процессов / `EAGAIN` внутри процесса): открытие в исключительном режиме, пока БД уже открыта, повторяется ограниченное число раз:

```cpp
    // Retry-стратегия: попытка открыть окружение в исключительном режиме, пока
    // оно уже открыто текущим процессом, даёт «занятость» (MDBX_BUSY при
    // конкуренции процессов или системную ошибку EAGAIN внутри процесса).
    // Такой отказ перехватывается и обрабатывается повтором с ограничением.
    const int max_attempts = 3;
    int attempt = 0;
    for (; attempt < max_attempts; ++attempt) {
      try {
        mdbx::env_managed busy(path, mdbx::env::operate_parameters().exclusive());
        std::cerr << "FAIL: exclusive open unexpectedly succeeded\n";
        return EXIT_FAILURE;
      } catch (const std::exception &ex) {
        std::cout << "busy attempt " << (attempt + 1) << ": failed (" << ex.what() << ")\n";
      }
    }
    std::cout << "busy: persists after " << attempt << " attempts (expected while env is open)\n";
```

Полный код: [17-error-handling.c++](examples/c++/17-error-handling.c++) · [C-версия](examples/c/17-error-handling.c)

> **Примеры к главе:** [`examples/c++/17-error-handling.c++`](examples/c++/17-error-handling.c++) · [C-версия](examples/c/17-error-handling.c);
> сквозной проект: [`config-store-12.c++`](examples/config-store/config-store-12.c++).

### 12.9. Резюме главы 12

- Успех = 0; ошибки отрицательные; `RESULT_TRUE/FALSE` — специальные результаты.
- `mdbx_strerror_r` — потокобезопасный вариант.
- `MAP_FULL` лечится геометрией+HSR; `TXN_FULL` — ранними коммитами.
- `KEYEXIST`/`NOTFOUND` — ожидаемые состояния.
- Коды дисциплины (`THREAD_MISMATCH` и др.) указывают на архитектурные ошибки.

### 12.10. Упражнения

1. Напишите обработчик `MDBX_KEYEXIST` в сквозном проекте: при конфликте — читать и решать.
2. Опишите, чем `MAP_FULL` отличается от `TXN_FULL`, на примерах.
3. Сделайте retry-цикл вокруг `cfg_set` со стратегией «читать-проверить-записать».

### 12.11. Чек-лист главы 12

- [ ] я правильно читаю коды возврата: 0 — успех, отрицательные — ошибки, `MDBX_RESULT_TRUE`/`MDBX_RESULT_FALSE` — специальные результаты;
- [ ] в многопоточном коде использую `mdbx_strerror_r()`, а не `mdbx_strerror()`;
- [ ] знаю порядок действий при `MDBX_MAP_FULL` (abort, проверка читателей, поднять `upper` до открытия, HSR) и отличаю его от `MDBX_TXN_FULL`;
- [ ] обрабатываю `MDBX_KEYEXIST` и `MDBX_NOTFOUND` ветвлением, а не паникой;
- [ ] воспринимаю коды дисциплины (`MDBX_THREAD_MISMATCH`, `MDBX_TXN_OVERLAPPING`) как сигнал архитектурной ошибки;
- [ ] умею писать retry-цикл «читать-проверить-записать» для конкурентных сценариев.

### 12.12. Что дальше

Том II дал полный практический инструментарий, но многие его правила («закрывайте read-транзакции»,
«геометрия — до open», «`SAFE_NOSYNC` растит файл») выглядели как требования без объяснений.
Том III открывает внутренние механизмы libmdbx: B+tree и mmap, MVCC, конвейер коммита, GC свободных
страниц, геометрию и восстановление после сбоев. С ними правила превращаются в понимание «почему».

---

## Итог тома

Том II дал вам практический инструментарий: курсоры, DUPSORT и индексы, конфигурацию окружения,
режимы долговечности, многопоточность и обработку ошибок. Сквозной проект-конфигуратор теперь —
полноценное приложение с индексами.

**Что дальше:** Том III — внутренние механизмы: B+tree и mmap, MVCC, конвейер коммита, GC,
геометрия, вложенные транзакции, файл блокировок, долговечность и восстановление.
