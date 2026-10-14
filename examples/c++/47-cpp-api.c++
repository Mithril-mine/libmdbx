// EN: C++ API: typed operations, exceptions, move semantics
// EN: Textbook section: Volume VI, chapter 40, "C++ API".
// EN: What it demonstrates: mdbx::env_managed/mdbx::txn_managed with RAII;
// EN:   typed map-style operations (insert/upsert/get/erase);
// EN:   typed exceptions (mdbx::key_exists, mdbx::not_found);
// EN:   move semantics of transactions/cursors; key_mode/value_mode.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 47-cpp-api.c++ -lmdbx
// EN: Run: ./47-cpp-api
// EN: Expected output:
// RU: C++ API: типизированные операции, исключения, move-семантика
// RU: Раздел учебника: Том VI, глава 40, «C++ API».
// RU: Что демонстрирует: mdbx::env_managed/mdbx::txn_managed с RAII;
// RU:   типизированные операции map-стиля (insert/upsert/get/erase);
// RU:   типизированные исключения (mdbx::key_exists, mdbx::not_found);
// RU:   move-семантику транзакций/курсоров; key_mode/value_mode.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 47-cpp-api.c++ -lmdbx
// RU: Запуск: ./47-cpp-api
// RU: Ожидаемый вывод:
//   ordinal[42] = answer
//   multi[tag]: x y
//   duplicate -> mdbx::key_exists as expected
//   missing -> mdbx::not_found as expected
//   ok: C++ API typed operations work
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

using buffer = mdbx::buffer<mdbx::default_allocator, mdbx::default_capacity_policy>;

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-47-cpp-api.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      // EN: RAII: will close/roll back by itself on exception
      // RU: RAII: сам закроется/откатится при исключении
      auto txn = env.start_write();
      auto ordinal = txn.create_map("ordinal", mdbx::key_mode::ordinal, mdbx::value_mode::single);
      auto multi = txn.create_map("multi", mdbx::key_mode::usual, mdbx::value_mode::multi);

      txn.insert(ordinal, buffer::key_from_u64(42), "answer");
      txn.upsert(multi, mdbx::slice("tag"), mdbx::slice("x"));
      txn.upsert(multi, mdbx::slice("tag"), mdbx::slice("y"));

      // EN: Typed exceptions by error code.
      // RU: Типизированные исключения по коду ошибки.
      try {
        txn.insert(ordinal, buffer::key_from_u64(42), "again");
        std::cerr << "FAIL: duplicate insert did not throw\n";
        return EXIT_FAILURE;
      } catch (const mdbx::key_exists &) {
        std::cout << "duplicate -> mdbx::key_exists as expected\n";
      }

      txn.commit();
    }

    auto rtxn = env.start_read();
    auto ordinal = rtxn.open_map("ordinal", mdbx::key_mode::ordinal, mdbx::value_mode::single);
    auto multi = rtxn.open_map("multi", mdbx::key_mode::usual, mdbx::value_mode::multi);
    std::cout << "ordinal[42] = " << rtxn.get(ordinal, buffer::key_from_u64(42)).as_string() << "\n";

    // EN: Cursor move semantics: the variable is moved, RAII is preserved.
    // RU: Move-семантика курсора: переменная перемещается, RAII сохраняется.
    mdbx::cursor_managed cur = rtxn.open_cursor(multi);
    std::string values;
    for (auto r = cur.to_key_exact(mdbx::slice("tag")); r; r = cur.to_current_next_multi(false))
      values += r.value.as_string() + " ";
    std::cout << "multi[tag]: " << values << "\n";

    try {
      (void)rtxn.get(ordinal, buffer::key_from_u64(999));
      std::cerr << "FAIL: get of absent key did not throw\n";
      return EXIT_FAILURE;
    } catch (const mdbx::not_found &) {
      std::cout << "missing -> mdbx::not_found as expected\n";
    }
    rtxn.abort();

    std::cout << "ok: C++ API typed operations work\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}