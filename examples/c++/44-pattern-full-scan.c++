// EN: Pattern: full-scan iterator
// EN: Textbook section: Volume V, chapter 32, "Design patterns", §32.6.
// EN: Task: traverse all records of a table (full scan) with counting
// EN:   and/or filtering.
// EN: Solution: a cursor from FIRST to the end; for large tables — estimation via
// EN:   cursor.estimate() before scanning.
// EN: Trade-offs: a full scan costs O(N) and is unsuitable for large
// EN:   data without indexes; the reader holds a snapshot — do not drag it out.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 44-pattern-full-scan.c++ -lmdbx
// EN: Run: ./44-pattern-full-scan
// EN: Expected output:
// RU: Паттерн: итератор полного сканирования
// RU: Раздел учебника: Том V, глава 32, «Паттерны проектирования», §32.6.
// RU: Задача: обойти все записи таблицы (полное сканирование) с подсчётом
// RU:   и/или фильтрацией.
// RU: Решение: курсор от FIRST до конца; для больших таблиц — оценка через
// RU:   cursor.estimate() перед сканированием.
// RU: Компромиссы: полное сканирование стоит O(N) и не подходит для больших
// RU:   данных без индексов; читатель держит снапшот — не затягивать.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 44-pattern-full-scan.c++ -lmdbx
// RU: Запуск: ./44-pattern-full-scan
// RU: Ожидаемый вывод:
//   scanned 1000 entries
//   ok: full-scan pattern works
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <cstddef>
#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-44-pattern-full-scan.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 1000; ++i)
        txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("v"));
      txn.commit();
    }

    auto rtxn = env.start_read();
    auto table = rtxn.open_map(nullptr);
    auto cur = rtxn.open_cursor(table);

    // EN: Full scan with an "even keys" filter.
    // RU: Полное сканирование с фильтром «чётные ключи».
    size_t scanned = 0, matched = 0;
    for (auto r = cur.to_first(); r; r = cur.to_next(false)) {
      ++scanned;
      if (r.key.as_string().size() % 2 == 0)
        ++matched;
    }
    std::cout << "scanned " << scanned << " entries\n";
    (void)matched;

    if (scanned != 1000) {
      std::cerr << "FAIL: unexpected scan count\n";
      return EXIT_FAILURE;
    }
    rtxn.abort();

    std::cout << "ok: full-scan pattern works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}