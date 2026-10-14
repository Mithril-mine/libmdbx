# Сквозной проект `config-store`

Демонстрационное конфигурационное хранилище на libmdbx (C++17). Проект растёт
вместе с учебником: каждый **срез** (`config-store-0N.c++`) — самодостаточная,
компилируемая программа, показывающая одну новую возможность поверх предыдущих.

## Схема эволюции

| Срез | Том / глава | Что нового по сравнению с предыдущим срезом |
| ---- | ----------- | ------------------------------------------- |
| `config-store-02` | I / гл. 2 | Каркас: открытие БД, команды set/get/del |
| `config-store-04` | I / гл. 4 | add (insert_unique) vs set (upsert); обработка KEYEXIST/NOTFOUND |
| `config-store-05` | I / гл. 5 | Пакеты в одной транзакции; откат (abort); read-only наблюдатель |
| `config-store-06` | II / гл. 6 | list: обход ключей курсором; диапазон по префиксу (SET_RANGE) |
| `config-store-08` | II / гл. 8 | Вторичный индекс «тег → список ключей», запрос by-tag |
| `config-store-09` | II / гл. 9 | Геометрия, runtime-опции, вывод статистики |
| `config-store-10` | II / гл. 10 | Выбор режима долговечности; sync_to_disk |
| `config-store-11` | II / гл. 11 | Многопоточный доступ: писатель + клоны читателей |
| `config-store-12` | II / гл. 12 | Устойчивость к ошибкам и гарантированные exit-коды |

## Сборка и запуск

Из каталога примера:

```sh
g++ -std=c++17 -I<libmdbx-include> -I../common config-store-02.c++ -lmdbx -o config-store-02
./config-store-02
```

Либо через CMake проекта (цели `mdbx_config_store_02` … `mdbx_config_store_12`,
зарегистрированы как CTest-тесты):

```sh
cmake --build build --target mdbx_config_store_02
ctest --test-dir build -R config_store_02
```

## Политика exit-кодов

- `EXIT_SUCCESS` (0) — прогон полностью успешен, итог в stdout вида `ok: ...`.
- `EXIT_FAILURE` (1) — любая ошибка: диагностика в stderr.
- Ожидаемые коды `MDBX_KEYEXIST`/`MDBX_NOTFOUND` обрабатываются явно.

Срезы используют общие хелперы `common.h++`.