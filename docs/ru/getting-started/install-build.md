# Установка и сборка

> Смежные: [Первые шаги](first-steps.md) · [Тулинг](../reference/tooling.md) ·
> [Учебник: Том I «Основы»](../textbook/01-tom-i-osnovy.md)

## Форм поставки

С декабря 2025 _libmdbx_ доступна **только в виде амальгамированных исходников**
(по модели [SQLite](https://www.sqlite.org/amalgamation.html)) — без внешних
зависимостей и внутренних ресурсов, нужных только для разработки самой libmdbx.
Пакеты для Linux-дистрибутивов планируются после релиза `1.0`.

Исходники: [SourceCraft](https://sourcecraft.dev/dqdkfa/libmdbx) и зеркало
[GitHub](https://github.com/Mithril-mine/libmdbx). Для production — ветка `stable`
или последний релиз (через staging); для разработки производных проектов — `master`.

Амальгамированная поставка — это несколько плоских файлов:

| Файл | Назначение |
| --- | --- |
| `mdbx.h`, `mdbx.c` | C API и реализация |
| `mdbx.h++`, `mdbx.c++` | C++ API и реализация |
| `mdbx-internals.h` | внутренние определения для сборки `mdbx.c` |
| `mdbx_chk.c`, `mdbx_copy.c`, `mdbx_dump.c`, `mdbx_load.c`, `mdbx_stat.c`, `mdbx_defrag.c` | утилиты командной строки |
| `CMakeLists.txt`, `GNUmakefile`, `Makefile` | сценарии сборки |

## Примеры есть, тестов — нет

Амальгамированный пакет **содержит компилируемые примеры** (`examples/` —
программы на C11/C++17, проверяемые CTest), которые можно использовать и как
smoke-test. **Тесты в пакет не входят**: полный тестовый сьют — часть дерева
разработки и распространяется под отдельной проприетарной лицензией, отличной
от Apache-2.0 самой библиотеки. Такое разделение упрощает интеграцию и
соблюдает лицензионную чистоту.

## CMake (рекомендуется)

```sh
cmake -S . -B build        # добавьте -DMDBX_BUILD_CXX=ON для C++ API
cmake --build build
ctest --test-dir build     # smoke-тесты (если MDBX_ENABLE_TESTS)
```

Полезные опции: `MDBX_BUILD_CXX` (C++ API), `MDBX_BUILD_TOOLS` (утилиты),
`MDBX_BUILD_SHARED_LIBRARY` (динамическая/статическая), `MDBX_ENABLE_TESTS`,
`MDBX_WITHOUT_MSVC_CRT` (без рантайм-зависимостей от MSVC CRT).
`make help` и `make options` перечисляют цели и опции сборки.

## GNU Make

```sh
make all      # собрать библиотеку
make check    # базовые проверки
```

Если системный `make` — не GNU Make, будет множество ошибок: используйте
`gmake` (FreeBSD/BSD) или установите GNU Make (macOS: `brew install make`).

## Платформы

- **Windows**: оригинальный CMake + Visual Studio 2019/2022 (свежие версии
  CMake/SDK ради C11 и `alignas()`); для MinGW — 10.2+ и современный CMake
  (удобно через [chocolatey](https://chocolatey.org/)); при иных способах
  сборки не забудьте `ntdll.lib` в линковку.
- **macOS**: `brew install bash make cmake ninja gnu-sed gnu-tar
  --with-default-names`, затем `make all`.
- **FreeBSD и BSD**: установите GNU Make, bash, компиляторы; затем
  `gmake all && gmake check`.
- **Android**: CMake по [официальному гайду](https://developer.android.com/studio/projects/add-native-code).
- **iOS**: CMake с [toolchain-файлом](https://github.com/leetal/ios-cmake).
- **HarmonyOS**: CMake с toolchain-файлом из HarmonyOS SDK.
- **WSL2** — поддерживается; **WSL1 — нет** (фундаментальное ограничение,
  libmdbx вернёт `ENOLCK` при открытии БД, чтобы избежать потери данных).

## Воспроизводимость сборки

По умолчанию фиксируется время сборки (`MDBX_BUILD_TIMESTAMP`). Для
[воспроизводимых сборок](https://reproducible-builds.org/) задайте фиксиро-
ванное значение: `make MDBX_BUILD_TIMESTAMP=unknown ...` или
`cmake -DMDBX_BUILD_TIMESTAMP:STRING=unknown ...` (и воспроизводимость самого
тулчейна — на вашей стороне).

## Контейнеры { #контейнеры }

Внутри одного контейнера — без особенностей. При интероперабельности
хост ↔ контейнеры **обязательно**:

1. **Когерентность memory mapping** и unified page cache ядра для хоста и всех
   контейнеров, работающих с БД (единственная физическая копия каждой
   memory-mapped страницы в системной памяти).
2. **Уникальность PID** (POSIX) и/или общее пространство PID: для Docker —
   `--pid=host`, либо `--pid=container:<name|id>` общий для всех работающих с БД.
   Для Windows — видимостью process handles (`OpenProcess(SYNCHRONIZE, ...,
   PID)` должен возвращать разумную ошибку, но не `ERROR_INVALID_PARAMETER`).
3. **Совместимость версий** libmdbx и `libc`/`pthreads`: строка `options:` в
   выводе `mdbx_chk -V` должна совпадать для хоста и контейнеров; не смешивайте
   `glibc` с `musl` — при сомнениях используйте одну LTS-версию или полную
   виртуализацию.

## DSO/DLL и деструкторы TLS

При сборке libmdbx как разделяемой библиотеки (или использовании статической
libmdbx внутри другой динамической библиотеки) убедитесь, что система
корректно вызывает деструкторы Thread-Local-Storage объектов при выгрузке
динамических библиотек — иначе выгрузка DLL/DSO с libmdbx после использования
может привести к утечкам или краху в многопоточном приложении. Корректно это
работает: на Windows 7+; при наличии `__cxa_thread_atexit_impl()` в libc
(GNU libc ≥ 2.18); при исправленных bug'ах glibc
[#21031](https://sourceware.org/bugzilla/show_bug.cgi?id=21031)/
[#21032](https://sourceware.org/bugzilla/show_bug.cgi?id=21032).

## API и учебник

- Онлайн-[справочник API](../reference/index.md) и
  заголовки [mdbx.h](https://sourcecraft.dev/dqdkfa/libmdbx/blob?file=mdbx.h) /
  [mdbx.h++](https://sourcecraft.dev/dqdkfa/libmdbx/blob?file=mdbx.h%2B%2B).
- [Учебник](../textbook/01-tom-i-osnovy.md) — шесть томов от первых шагов до
  экспертных тем (RU и EN, с PDF).
