# Справочник по API

> **Примечание:** справочник API пока не переведён на русский язык — все
> страницы справочника доступны только в английской локализации
> (`/docs/en/reference/api/`). Перевод появится, когда будет решён вопрос
> хранения и синхронизации перевода между локализациями.

Справочник по C и C++ API генерируется из аннотаций в исходниках (`mdbx.h`,
`mdbx.h++`) инструментом Doxygen; аннотации API являются единственным
источником истины.

Точки входа (английский):

- [Аннотированный индекс](https://libmdbx.dqdkfa.ru/docs/en/reference/api/annotated.html) — все классы и структуры
- [Классы](https://libmdbx.dqdkfa.ru/docs/en/reference/api/classes.html) · [Иерархия классов](https://libmdbx.dqdkfa.ru/docs/en/reference/api/hierarchy.html)
- [Пространства имён](https://libmdbx.dqdkfa.ru/docs/en/reference/api/namespaces.html)
- [Файлы](https://libmdbx.dqdkfa.ru/docs/en/reference/api/files.html) — C API (`mdbx.h`) со всеми функциями и макросами,
  а также C++ API (`mdbx.h++`)
- [Члены классов](https://libmdbx.dqdkfa.ru/docs/en/reference/api/class_members.html) · [Макросы](https://libmdbx.dqdkfa.ru/docs/en/reference/api/macros.html)

На этом же разделе: [утилиты командной строки](tooling.md);
[журнал изменений](https://libmdbx.dqdkfa.ru/docs/en/reference/changelog.html) (английский).

Вводное руководство — в [гайдах](../index.md); страница
[первых шагов](../getting-started/first-steps.md) показывает канонический поток
env → транзакция → DBI → get/put с готовыми примерами.
