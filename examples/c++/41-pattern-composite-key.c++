// EN: Pattern: composite key (packed fields, ordered range scans)
// EN: Textbook section: Volume V, chapter 32, "Design patterns", §32.3.
// EN: Task: store records under a composite key (a, b) and run range
// EN:   queries on a (e.g., "all records for the year 2026").
// EN: Solution: packing fixed-width fields into a big-endian key, so that
// EN:   lexicographic order matches numeric order (based on S10).
// EN: Trade-offs: fields must be fixed-width and byte-comparable;
// EN:   variable length requires its own scheme or comparator.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 41-pattern-composite-key.c++ -lmdbx
// EN: Run: ./41-pattern-composite-key
// EN: Expected output:
// RU: Паттерн: составной ключ (упакованные поля, упорядоченные диапазонные запросы)
// RU: Раздел учебника: Том V, глава 32, «Паттерны проектирования», §32.3.
// RU: Задача: хранить записи по составному ключу (a, b) и делать диапазонные
// RU:   запросы по a (например, «все записи за 2026-год»).
// RU: Решение: упаковка полей фиксированной ширины в big-endian ключ, при котором
// RU:   лексикографический порядок совпадает с числовым (на основе S10).
// RU: Компромиссы: поля должны быть фиксированной ширины и байт-сравнимыми;
// RU:   переменной длины требует собственную схему или компаратор.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 41-pattern-composite-key.c++ -lmdbx
// RU: Запуск: ./41-pattern-composite-key
// RU: Ожидаемый вывод:
//   year=2026: (2026,1) (2026,7) (2026,12)
//   ok: composite-key pattern works
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <cstdint>
#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace {

// EN: Big-endian (network-order) representation of a 64-bit value: the byte order
// EN: used for fixed-width packed keys, so lexicographic order matches numeric.
// RU: Big-endian (сетевой порядок) представление 64-битного значения: порядок байтов,
// RU: используемый в упакованных ключах фиксированной ширины, поэтому лексикографический порядок совпадает с числовым.
uint64_t be64(uint64_t value) {
#if defined(_MSC_VER)
  return _byteswap_uint64(value);
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return __builtin_bswap64(value);
#else
  return value;
#endif
}

uint64_t pack(uint16_t year, uint16_t month) {
  const uint64_t value = (uint64_t(year) << 48) | (uint64_t(month) << 32);
  return be64(value);
}

uint16_t decode_year(const mdbx::slice &key) {
  const auto *p = static_cast<const uint8_t *>(key.data());
  return uint16_t((uint16_t(p[0]) << 8) | p[1]);
}

uint16_t decode_month(const mdbx::slice &key) {
  const auto *p = static_cast<const uint8_t *>(key.data());
  return uint16_t((uint16_t(p[2]) << 8) | p[3]);
}

} // namespace

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-41-pattern-composite-key.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      static const uint16_t years[] = {2025, 2026, 2026, 2026, 2027};
      static const uint16_t months[] = {11, 1, 7, 12, 2};
      for (size_t i = 0; i < sizeof(years) / sizeof(years[0]); ++i)
        txn.insert(table, mdbx::slice::wrap(pack(years[i], months[i])), mdbx::slice("event"));
      txn.commit();
    }

    auto rtxn = env.start_read();
    auto table = rtxn.open_map(nullptr);
    auto cur = rtxn.open_cursor(table);

    // EN: Range year=2026: keys are sorted by (year, month).
    // RU: Диапазон year=2026: ключи отсортированы по (year, month).
    std::string events;
    for (auto r = cur.to_key_greater_or_equal(mdbx::slice::wrap(pack(2026, 0)), false);
         r && decode_year(r.key) == 2026; r = cur.to_next(false))
      events += "(" + std::to_string(decode_year(r.key)) + "," + std::to_string(decode_month(r.key)) + ") ";
    std::cout << "year=2026: " << events << "\n";
    rtxn.abort();

    std::cout << "ok: composite-key pattern works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}