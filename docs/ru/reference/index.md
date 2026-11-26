# Справочник по API

> **Примечание:** справочник API пока не переведён на русский язык — все
> страницы справочника доступны только в английской локализации
> ([`/docs/en/reference/`](https://libmdbx.dqdkfa.ru/docs/en/reference/)).
> Перевод появится, когда будет решён вопрос
> хранения и синхронизации перевода между локализациями.

Справочник по C и C++ API генерируется из аннотаций в исходниках (`mdbx.h`,
`mdbx.h++`) инструментом Doxygen; аннотации API являются единственным
источником истины.

## C API по модулям

C API организован в группы документации («модули»); каждая страница модуля
несёт полную документацию своих функций, макросов и типов. Обзорная страница
[C API](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__api.html)
содержит базовые дескрипторы (`MDBX_env`, `MDBX_val`, ...) и общие
определения.

- [C API](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__api.html) — базовые дескрипторы и общие определения
- [Открытие и закрытие](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__opening.html) — создание/открытие/закрытие окружения: `mdbx_env_create()`, `mdbx_env_open()`, ...
- [Транзакции](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__transactions.html) — `mdbx_txn_begin()`, `mdbx_txn_commit()`, ...
- [Таблицы](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__dbi.html) — дескрипторы таблиц (DBI): `mdbx_dbi_open()`, `mdbx_map_handle()`, ...
- [Create/Read/Update/Delete](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__crud.html) — `mdbx_put()`, `mdbx_get()`, `mdbx_replace()`, `mdbx_del()`, ...
- [Курсоры](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__cursors.html) — работа с курсорами и операции: `mdbx_cursor_open()`, `mdbx_cursor_get()`, ...
- [Статистика и информация](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__statinfo.html) — `mdbx_env_info()`, `mdbx_env_stat()`, `mdbx_dbi_stat()`, ...
- [Настройки](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__settings.html) — геометрия и опции окружения: `mdbx_env_set_geometry()`, ...
- [SYNC MODES](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__sync__modes.html) — флаги durability и заметки о миграции с LMDB
- [Оценка диапазонных запросов](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__rqest.html) — подсказки стоимости/размера `mdbx_estimate_*()`
- [Дополнительные операции](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__extra.html) — копирование, пути, проверки читателей и прочее
- [Проверка и восстановление](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__chk.html) — внутренний API уровня `mdbx_chk`
- [Журналирование и отладка](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__debug.html) — `mdbx_debug_flags()`, `mdbx_logger()`, ...
- [Общие макросы](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__api__macros.html) — макросы проверки возможностей и вспомогательные
- [Value-to-Key](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__value2key.html) / [Key-to-Value](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__key2value.html) — `mdbx_to_key()` / `mdbx_from_key()` для отказа от пользовательских компараторов
- [Обработка ошибок](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__err.html) — коды ошибок и сообщения: `mdbx_strerror()`, `mdbx_liberr2str()`, ...

## C++ API по модулям

- [C++ API](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__cxx__api.html) — зонтичный модуль пространства имён `mdbx`
- [Ядро](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__cxx__core.html) — окружение, транзакции, курсоры, таблицы и map-дескрипторы
- [Срезы и буферы](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__cxx__data.html) — `mdbx::slice`, `mdbx::buffer`, ... и кодеки value-to-key/key-to-value
- [Исключения и ошибки](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__cxx__exceptions.html) — иерархия `mdbx::exception` и отображение ошибок в исключения

## Ключевые типы одним взглядом

C: непрозрачные дескрипторы `MDBX_env`, `MDBX_txn`, `MDBX_cursor`, `MDBX_dbi`
и `MDBX_val` описаны на странице
[C API](https://libmdbx.dqdkfa.ru/docs/en/reference/api/group__c__api.html);
функции для них — в [модулях выше](#c-api-по-модулям).

C++:

- Окружения: [`mdbx::env`](https://libmdbx.dqdkfa.ru/docs/en/reference/api/classmdbx_1_1env.html) /
  [`mdbx::env_managed`](https://libmdbx.dqdkfa.ru/docs/en/reference/api/classmdbx_1_1env__managed.html)
- Транзакции: [`mdbx::txn`](https://libmdbx.dqdkfa.ru/docs/en/reference/api/classmdbx_1_1txn.html) /
  [`mdbx::txn_managed`](https://libmdbx.dqdkfa.ru/docs/en/reference/api/classmdbx_1_1txn__managed.html)
- Курсоры: [`mdbx::cursor`](https://libmdbx.dqdkfa.ru/docs/en/reference/api/classmdbx_1_1cursor.html) /
  [`mdbx::cursor_managed`](https://libmdbx.dqdkfa.ru/docs/en/reference/api/classmdbx_1_1cursor__managed.html)

## Точки входа (английский)

- [Аннотированный индекс](https://libmdbx.dqdkfa.ru/docs/en/reference/api/annotated.html) — все классы и структуры
- [Классы](https://libmdbx.dqdkfa.ru/docs/en/reference/api/classes.html) · [Иерархия классов](https://libmdbx.dqdkfa.ru/docs/en/reference/api/hierarchy.html)
- [Пространства имён](https://libmdbx.dqdkfa.ru/docs/en/reference/api/namespaces.html)
- [Модули](https://libmdbx.dqdkfa.ru/docs/en/reference/api/modules.html) — полный список групп документации (те же модули, что и выше)
- [Члены классов](https://libmdbx.dqdkfa.ru/docs/en/reference/api/class_members.html) · [Макросы](https://libmdbx.dqdkfa.ru/docs/en/reference/api/macros.html)
- [Список файлов](https://libmdbx.dqdkfa.ru/docs/en/reference/api/files.html) — три заголовка со своими структурами и макросами; документация функций живёт на страницах модулей выше
- [Список устаревшего](https://libmdbx.dqdkfa.ru/docs/en/reference/api/deprecated.html)

Сами заголовки с полной встроенной документацией:

- [Заголовок C API `mdbx.h`](https://libmdbx.dqdkfa.ru/docs/en/reference/api/mdbx_8h.html)
- [Заголовок C++ API `mdbx.h++`](https://libmdbx.dqdkfa.ru/docs/en/reference/api/mdbx_8h_09_09.html)

На этом же разделе: [утилиты командной строки](tooling.md).

Вводное руководство — в [гайдах](../index.md); страница
[первых шагов](../getting-started/first-steps.md) показывает канонический поток
env → транзакция → DBI → get/put с готовыми примерами.
