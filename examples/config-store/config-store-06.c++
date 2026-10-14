// EN: config-store, slice 06: iteration and the key list
// EN: Textbook section: Volume II, chapter 6, "Cursors", §6.2 "Positioning".
// EN: What is added: the list command — traversing all keys via a cursor; range
// EN:   scanning by prefix (SET_RANGE).
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-06.c++ -lmdbx
// EN: Run: ./config-store-06
// EN: Expected output:
// RU: config-store, срез 06: итерация и список ключей
// RU: Раздел учебника: Том II, глава 6, «Курсоры», §6.2 «Позиционирование».
// RU: Что добавляется: команда list — обход всех ключей через курсор; диапазонный
// RU:   просмотр по префиксу (SET_RANGE).
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-06.c++ -lmdbx
// RU: Запуск: ./config-store-06
// RU: Ожидаемый вывод:
//   keys: app.debug app.host app.port system.theme
//   prefix 'app.': app.debug app.host app.port
//   ok: config-store-06 works
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/config-store-06.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.upsert(table, mdbx::slice("system.theme"), mdbx::slice("dark"));
      txn.upsert(table, mdbx::slice("app.host"), mdbx::slice("localhost"));
      txn.upsert(table, mdbx::slice("app.port"), mdbx::slice("8080"));
      txn.upsert(table, mdbx::slice("app.debug"), mdbx::slice("off"));
      txn.commit();
    }

    auto rtxn = env.start_read();
    auto table = rtxn.open_map(nullptr);
    auto cur = rtxn.open_cursor(table);

    // EN: Full traversal of the keys.
    // RU: Полный обход ключей.
    std::string all;
    for (auto r = cur.to_first(); r; r = cur.to_next(false))
      all += r.key.as_string() + " ";
    std::cout << "keys: " << all << "\n";

    // EN: Range by the prefix 'app.'.
    // RU: Диапазон по префиксу 'app.'.
    std::string prefix;
    for (auto r = cur.to_key_greater_or_equal(mdbx::slice("app."), false);
         r && r.key.as_string().compare(0, 4, "app.") == 0; r = cur.to_next(false))
      prefix += r.key.as_string() + " ";
    std::cout << "prefix 'app.': " << prefix << "\n";

    rtxn.abort();

    std::cout << "ok: config-store-06 works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}