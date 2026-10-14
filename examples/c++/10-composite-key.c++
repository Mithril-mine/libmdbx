// EN: Composite key: pack two fields into one ordered key
// EN: Textbook section: Volume II, chapter 8, "Secondary indexes", section "Composite keys".
// EN: What it demonstrates: packing two unsigned fields into a composite key
// EN:   of fixed length (8 bytes, big-endian) so that the lexicographic order
// EN:   of keys matches the numeric one; a range query "all keys with the given first
// EN:   field" via SET_RANGE. No custom comparator is required: byte-order encoding
// EN:   is sufficient.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 10-composite-key.c++ -lmdbx
// EN: Run: ./10-composite-key
// EN: Expected output:
// EN:   entries: (0,1) (0,2) (1,0) (1,1) (7,3) (7,7) (7,42) (9,0)
// EN:   range x=7: (7,3) (7,7) (7,42)
// EN:   ok: composite key packed and range-scanned
// EN: On failure — message to stderr and exit 1.
// RU: Composite key: pack two fields into one ordered key
// RU: Раздел учебника: Том II, глава 8, «Вторичные индексы», раздел «Составные ключи».
// RU: Что демонстрирует: упаковку двух беззнаковых полей в составной ключ
// RU:   фиксированной длины (8 байт, big-endian) так, что лексикографический порядок
// RU:   ключей совпадает с числовым; диапазонный запрос «все ключи с заданным первым
// RU:   полем» через SET_RANGE. Собственный компаратор не требуется: достаточно
// RU:   байт-порядкового кодирования.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 10-composite-key.c++ -lmdbx
// RU: Запуск: ./10-composite-key
// RU: Ожидаемый вывод:
// RU:   entries: (0,1) (0,2) (1,0) (1,1) (7,3) (7,7) (7,42) (9,0)
// RU:   range x=7: (7,3) (7,7) (7,42)
// RU:   ok: composite key packed and range-scanned
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
// RU: Big-endian (network-order) representation of a 64-bit value: the byte order
// RU: used for fixed-width packed keys, so lexicographic order matches numeric.
uint64_t be64(uint64_t value) {
#if defined(_MSC_VER)
  return _byteswap_uint64(value);
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return __builtin_bswap64(value);
#else
  return value;
#endif
}

// EN: Packing (x, y) into uint64_t so that the per-byte (lexicographic) order
// EN: of the key matches the numeric order of the pair. For this, a big-endian
// EN: representation of fixed-width unsigned fields is used.
// RU: Упаковка (x, y) в uint64_t так, чтобы побайтовый (лексикографический) порядок
// RU: ключа совпадал с числовым порядком пары. Для этого используется big-endian
// RU: представление беззнаковых полей фиксированной ширины.
uint64_t pack(uint32_t x, uint32_t y) {
  const uint64_t value = (uint64_t(x) << 32) | uint64_t(y);
  return be64(value);
}

// EN: Decoding the composite key back into (x, y).
// RU: Декодирование составного ключа обратно в (x, y).
std::string key_to_string(const mdbx::slice &key) {
  const auto *p = static_cast<const uint8_t *>(key.data());
  const uint32_t hi = (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
  const uint32_t lo = (uint32_t(p[4]) << 24) | (uint32_t(p[5]) << 16) | (uint32_t(p[6]) << 8) | uint32_t(p[7]);
  return "(" + std::to_string(hi) + "," + std::to_string(lo) + ")";
}

// EN: The first field of the composite key (x), decoded from big-endian.
// RU: Первое поле составного ключа (x), декодированное из big-endian.
uint32_t key_first_field(const mdbx::slice &key) {
  const auto *p = static_cast<const uint8_t *>(key.data());
  return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}

} // namespace

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-10-composite-key.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      static const uint32_t xs[] = {0, 0, 1, 1, 7, 7, 7, 9};
      static const uint32_t ys[] = {1, 2, 0, 1, 3, 42, 7, 0};
      for (size_t i = 0; i < sizeof(xs) / sizeof(xs[0]); ++i) {
        const uint64_t packed = pack(xs[i], ys[i]);
        txn.insert(table, mdbx::slice::wrap(packed), mdbx::slice("v"));
      }
      txn.commit();
    }

    auto rtxn = env.start_read();
    auto table = rtxn.open_map(nullptr);

    // EN: Full traversal: keys are sorted by the value of the composite key.
    // RU: Полный обход: ключи отсортированы по значению составного ключа.
    std::string all;
    auto cur = rtxn.open_cursor(table);
    for (auto r = cur.to_first(); r; r = cur.to_next(false))
      all += key_to_string(r.key) + " ";
    std::cout << "entries: " << all << "\n";

    // EN: The range x=7: from (7,0); we stop when the first field != 7.
    // RU: Диапазон x=7: от (7,0); останавливаемся, когда первое поле != 7.
    std::string range;
    const uint64_t lo = pack(7, 0);
    for (auto r = cur.to_key_greater_or_equal(mdbx::slice::wrap(lo), false); r && key_first_field(r.key) == 7;
         r = cur.to_next(false))
      range += key_to_string(r.key) + " ";
    std::cout << "range x=7: " << range << "\n";

    rtxn.abort();

    std::cout << "ok: composite key packed and range-scanned\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}