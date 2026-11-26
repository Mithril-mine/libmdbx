// EN: Common helpers for the C++ examples of the libmdbx textbook.
// EN: Textbook section: used by all C++ examples.
// EN: Exit-code policy: on error — diagnostic message to stderr and exit with EXIT_FAILURE.
// RU: Общие хелперы для C++-примеров учебника libmdbx.
// RU: Раздел учебника: используется всеми примерами на C++.
// RU: Политика exit-кодов: при ошибке — диагностика в stderr и завершение с EXIT_FAILURE.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MDBX_EXAMPLES_COMMON_CPP_HPP_
#define MDBX_EXAMPLES_COMMON_CPP_HPP_

#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

#include <mdbx.h++>

// EN: The runtime Valgrind detection, with the include-order caveat: the
// EN: library's preface.h provides its own fallback stubs, so valgrind.h
// EN: must be included only when its client-request macros are not there
// EN: yet. Used to cap the default geometry (see env_open below).
// RU: Детектирование запуска под Valgrind во время исполнения с оговоркой
// RU: о порядке включений: preface.h библиотеки определяет собственные
// RU: заглушки-fallback, поэтому valgrind.h включается только если его
// RU: клиентские макросы ещё не определены. Используется для ограничения
// RU: геометрии по умолчанию (см. env_open ниже).
#if !defined(RUNNING_ON_VALGRIND) && defined(__has_include)
#if __has_include(<valgrind/valgrind.h>)
#include <valgrind/valgrind.h>
#endif
#endif
#if !defined(RUNNING_ON_VALGRIND)
#define RUNNING_ON_VALGRIND (0)
#endif

namespace example {

// EN: Directory for temporary DBs: $TMPDIR (on Windows — %TEMP%) or /tmp.
// RU: Каталог для временных БД: $TMPDIR (на Windows — %TEMP%) или /tmp.
inline const char *tmpdir() {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996) /* getenv: This function or variable may be unsafe */
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
  const char *dir = std::getenv("TMPDIR");
#if defined(_WIN32) || defined(_WIN64)
  if (!dir || !*dir)
    dir = std::getenv("TEMP");
  if (!dir || !*dir)
    dir = ".";
#else
  if (!dir || !*dir)
    dir = "/tmp";
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
  return dir;
}

// EN: Opens (creating if needed) the environment at path
// EN: with parameters params (default geometry).
// EN: By default only the main table is available in libmdbx; the number of named
// EN: tables is set via max_maps. Here 0 (default) is replaced with a value
// EN: sufficient for all textbook examples.
// EN: Under Valgrind the engine's default upper bound (a fraction of the
// EN: system RAM, i.e. terabytes) must be mapped inside the Valgrind's own
// EN: address space, which is unreliable and may fail/restrain (the textbook
// EN: warns "TOO_LARGE/ENOMEM are possible under ASAN/Valgrind - always set
// EN: a reasonable upper explicitly"), so a modest cap is applied here.
// RU: Открывает (создавая при необходимости) окружение по пути path
// RU: с параметрами params (геометрия по умолчанию).
// RU: По умолчанию в libmdbx доступна только главная таблица; число именованных
// RU: таблиц задаётся через max_maps. Здесь 0 (default) заменяется на достаточное
// RU: для всех примеров учебника значение.
// RU: Под Valgrind верхнюю границу движка по умолчанию (доля системной ОЗУ,
// RU: т.е. терабайты) приходится отображать внутри адресного пространства
// RU: самого Valgrind — это ненадёжно и может отказывать/притормаживать
// RU: (учебник предупреждает: «TOO_LARGE/ENOMEM возможны под ASAN/Valgrind —
// RU: всегда задавайте разумный upper явно»), поэтому здесь применяется
// RU: скромное ограничение.
inline mdbx::env_managed env_open(std::string_view path, mdbx::env::operate_parameters params = {}) {
  if (params.max_maps == 0)
    params.max_maps = 64;
#if !defined(_WIN32) && !defined(_WIN64)
  if (RUNNING_ON_VALGRIND) {
    mdbx::env::geometry geo;
    geo.size_upper = intptr_t(64) << 20 /* 64 MiB is ample for the textbook examples */;
    return mdbx::env_managed(std::string(path), mdbx::env_managed::create_parameters().set_geometry(geo), params);
  }
#endif
  return mdbx::env_managed(std::string(path), mdbx::env_managed::create_parameters(), params);
}

// EN: Same, but with an explicit geometry: it does not rely on the system-dependent
// EN: default size (the default upper bound is derived from the amount of RAM) and
// EN: instead sets fixed bounds that are mappable even on 32-bit platforms.
// RU: То же, но с явной геометрией: не полагается на зависящий от системы размер
// RU: по умолчанию (upper по умолчанию рассчитывается из объёма RAM), а задаёт
// RU: фиксированные границы, доступные для отображения и на 32-битных платформах.
inline mdbx::env_managed env_open(std::string_view path, const mdbx::env::geometry &geo,
                                  mdbx::env::operate_parameters params = {}) {
  if (params.max_maps == 0)
    params.max_maps = 64;
  return mdbx::env_managed(std::string(path), mdbx::env_managed::create_parameters().set_geometry(geo), params);
}

// EN: Opens a write transaction.
// RU: Открывает транзакцию на запись.
inline mdbx::txn_managed rw_txn(mdbx::env_managed &env) { return env.start_write(); }

// EN: Runs an action, converting exceptions (mdbx::exception, std::system_error, etc.)
// EN: into a diagnostic message in stderr and an exit with EXIT_FAILURE.
// RU: Запускает действие, переводя исключения (mdbx::exception, std::system_error и пр.)
// RU: в диагностику в stderr и выход с EXIT_FAILURE.
template <class F> auto check(F &&f) -> decltype(f()) {
  try {
    return f();
  } catch (const std::exception &ex) {
    std::fprintf(stderr, "%s\n", ex.what());
    std::exit(EXIT_FAILURE);
  }
}

} // namespace example

#endif /* MDBX_EXAMPLES_COMMON_CPP_HPP_ */