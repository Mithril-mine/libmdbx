// EN: Multi-values and DUPSORT: DUPFIXED, GET_BOTH/GET_BOTH_RANGE
// EN: Textbook section: Volume II, chapter 7, "Multi-values and DUPSORT",
// EN:   sections "DUPFIXED/INTEGERDUP", "GET_BOTH", "Inverted indexes".
// EN: What it demonstrates: a table with multiple values (value_mode::multi
// EN:   with DUPFIXED); insertion of fixed-length values (uint32_t); reading
// EN:   all values of a key; exact pair search (GET_BOTH) and range search over
// EN:   values (GET_BOTH_RANGE).
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 07-dupsort.c++ -lmdbx
// EN: Run: ./07-dupsort
// EN: Expected output:
// EN:   values(sensor): 1 3 5 7 9
// EN:   get_both(9) -> 9
// EN:   get_both_range(>=6) -> 7
// EN:   ok: DUPSORT|DUPFIXED, GET_BOTH/GET_BOTH_RANGE
// EN: On failure — message to stderr and exit 1.
// RU: Multi-values and DUPSORT: DUPFIXED, GET_BOTH/GET_BOTH_RANGE
// RU: Раздел учебника: Том II, глава 7, «Мультизначения и DUPSORT»,
// RU:   разделы «DUPFIXED/INTEGERDUP», «GET_BOTH», «Инвертированные индексы».
// RU: Что демонстрирует: таблицу со множественными значениями (value_mode::multi
// RU:   с DUPFIXED); вставку фиксированных по длине значений (uint32_t); чтение
// RU:   всех значений ключа; точный поиск пары (GET_BOTH) и поиск по диапазону
// RU:   значений (GET_BOTH_RANGE).
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 07-dupsort.c++ -lmdbx
// RU: Запуск: ./07-dupsort
// RU: Ожидаемый вывод:
// RU:   values(sensor): 1 3 5 7 9
// RU:   get_both(9) -> 9
// RU:   get_both_range(>=6) -> 7
// RU:   ok: DUPSORT|DUPFIXED, GET_BOTH/GET_BOTH_RANGE
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <cstdint>
#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-07-dupsort.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      // EN: value_mode::multi_samelength = MDBX_DUPSORT | MDBX_DUPFIXED:
      // EN: all values of a key must be of the same length (here — 4 bytes of uint32_t).
      // RU: value_mode::multi_samelength = MDBX_DUPSORT | MDBX_DUPFIXED:
      // RU: все значения ключа должны быть одной длины (здесь — 4 байта uint32_t).
      auto multi = txn.create_map("multi", mdbx::key_mode::usual, mdbx::value_mode::multi_samelength);
      const auto key = mdbx::slice("sensor");
      const uint32_t values[] = {5, 1, 9, 3, 7};
      for (uint32_t v : values)
        // EN: UPSERT adds a value to the key
        // RU: UPSERT добавляет значение к ключу
        txn.upsert(multi, key, mdbx::slice::wrap(v));
      txn.commit();
    }

    auto rtxn = env.start_read();
    auto multi = rtxn.open_map("multi", mdbx::key_mode::usual, mdbx::value_mode::multi_samelength);
    const auto key = mdbx::slice("sensor");

    // EN: Reading all values of a key (they are stored sorted).
    // RU: Чтение всех значений ключа (хранятся отсортированными).
    std::string all;
    auto cur = rtxn.open_cursor(multi);
    for (auto r = cur.to_key_exact(key); r; r = cur.to_current_next_multi(false))
      all += std::to_string(r.value.as_uint32()) + " ";
    std::cout << "values(sensor): " << all << "\n";

    // EN: GET_BOTH: exact search of a "key + value" pair.
    // RU: GET_BOTH: точный поиск пары «ключ + значение».
    const uint32_t needle = 9;
    auto found = cur.find_multivalue(key, mdbx::slice::wrap(needle), false);
    std::cout << "get_both(9) -> "
              << (found ? std::to_string(found.value.as_uint32()) : std::string("<none>")) << "\n";

    // EN: GET_BOTH_RANGE: the first value not less than the given one.
    // RU: GET_BOTH_RANGE: первое значение, не меньшее заданного.
    const uint32_t low = 6;
    auto range = cur.lower_bound_multivalue(key, mdbx::slice::wrap(low), false);
    std::cout << "get_both_range(>=6) -> "
              << (range ? std::to_string(range.value.as_uint32()) : std::string("<none>")) << "\n";

    rtxn.abort();

    std::cout << "ok: DUPSORT|DUPFIXED, GET_BOTH/GET_BOTH_RANGE\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}