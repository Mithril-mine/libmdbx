# Шпаргалка по CRUD

> Смежные: [Первые шаги](first-steps.md) ·
> [Установка и сборка](install-build.md) ·
> [Учебник: Том I «Основы»](../textbook/01-tom-i-osnovy.md) ·
> [Справочник C API](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__api.html)

Компактная шпаргалка по наиболее частым операциям. Авторитетный
Quick Reference с полными таблицами решений генерируется из аннотаций
`mdbx.h` и расположен в справочнике C API:

- [Quick Reference для операций Insert/Update/Delete](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__crud.html) —
  случаи для таблиц с UNIQUE-ключами и с NON-UNIQUE (DUPSORT) ключами, а также
  соответствующие флаги (`MDBX_NOOVERWRITE`, `MDBX_UPSERT`, `MDBX_CURRENT`,
  `MDBX_APPEND`, `MDBX_APPENDDUP`, ...).

## C++ API: кратко

C++-обёртка отображает C-операции на методы `mdbx::txn` и `mdbx::cursor`.
Наиболее употребимые:

| Операция | `mdbx::txn` | `mdbx::cursor` | Аналог в C |
|----------|-------------|----------------|------------|
| Вставить или вернуть ошибку | `insert(map, key, value)` | `insert(key, value)` | `mdbx_put(MDBX_NOOVERWRITE)` |
| Вставить или заменить | `upsert(map, key, value)` | `upsert(key, value)` | `mdbx_put(MDBX_UPSERT)` |
| Обновить существующее | `update(map, key, value)` | `update(key, value)` | `mdbx_put(MDBX_CURRENT)` |
| Прочитать | `get(map, key)` | `to_first()/to_find()/...` | `mdbx_get()` |
| Прочитать и удалить | `extract(map, key)` | — | `mdbx_replace()` |
| Удалить по ключу | `erase(map, key)` | `erase(key)` | `mdbx_del()` |
| Удалить пару | `erase(map, key, value)` | `erase(key, value)` | `mdbx_del()` |
| Добавить в конец | `put(map, key, value, MDBX_APPEND)` | `put(key, value, MDBX_APPEND)` | `mdbx_put(MDBX_APPEND)` |

> Подробнее — в [справочнике C++ API](https://libmdbx.dqdkfa.ru/docs/en/reference/api/classmdbx_1_1txn.html)
> и [классе `mdbx::cursor`](https://libmdbx.dqdkfa.ru/docs/en/reference/api/classmdbx_1_1cursor.html).

## Примеры

- [`examples/c++/03-crud.c++`](https://sourcecraft.dev/dqdkfa/libmdbx/blob?file=examples/c%2B%2B/03-crud.c%2B%2B) ·
  [C версия](https://sourcecraft.dev/dqdkfa/libmdbx/blob?file=examples/c/03-crud.c)
- Учебник: [Глава 4. Базовые операции CRUD](../textbook/01-tom-i-osnovy.md)

## Читайте также

- [Первые шаги](first-steps.md) — минимальный цикл env → txn → dbi → get/put.
- [Режимы durability](durability-modes.md) — компромиссы между надёжностью и скоростью.