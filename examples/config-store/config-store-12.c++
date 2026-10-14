// EN: config-store, slice 12: error resilience and exit codes
// EN: Textbook section: Volume II, chapter 12, "Error handling", §12.4 "Retry strategies".
// EN: What is added: handling of expected errors (missing key, duplicate) and
// EN:   guaranteed exit codes: 0 — success, 1 — failure; messages to stderr.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-12.c++ -lmdbx
// EN: Run: ./config-store-12
// EN: Expected output:
// RU: config-store, срез 12: устойчивость к ошибкам и exit-коды
// RU: Раздел учебника: Том II, глава 12, «Обработка ошибок», §12.4 «Retry-стратегии».
// RU: Что добавляется: обработка ожидаемых ошибок (нет ключа, дубликат) и
// RU:   гарантированные exit-коды: 0 — успех, 1 — сбой; сообщения в stderr.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-12.c++ -lmdbx
// RU: Запуск: ./config-store-12
// RU: Ожидаемый вывод:
//   ok: add host = localhost
//   expected failure: already exists (exit code 1, handled)
//   ok: get host = localhost
//   expected failure: no such key (exit code 1, handled)
//   ok: config-store-12 works
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
    const std::string path = std::string(example::tmpdir()) + "/config-store-12.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    // EN: A successful operation: exit code 0.
    // RU: Успешная операция: exit-код 0.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("host"), mdbx::slice("localhost"));
      txn.commit();
    }
    std::cout << "ok: add host = localhost\n";

    // EN: Expected error: a repeated add -> MDBX_KEYEXIST.
    // RU: Ожидаемая ошибка: повторный add -> MDBX_KEYEXIST.
    try {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("host"), mdbx::slice("other"));
      txn.commit();
      std::cerr << "FAIL: duplicate add did not throw\n";
      return EXIT_FAILURE;
    } catch (const mdbx::key_exists &) {
      std::cout << "expected failure: already exists (exit code 1, handled)\n";
    }

    // EN: Reading an existing key.
    // RU: Чтение существующего ключа.
    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::cout << "ok: get host = " << rtxn.get(table, mdbx::slice("host")).as_string() << "\n";
      rtxn.abort();
    }

    // EN: Expected error: reading an absent key -> MDBX_NOTFOUND.
    // RU: Ожидаемая ошибка: чтение отсутствующего ключа -> MDBX_NOTFOUND.
    try {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      (void)rtxn.get(table, mdbx::slice("absent"));
      rtxn.abort();
      std::cerr << "FAIL: get of absent key did not throw\n";
      return EXIT_FAILURE;
    } catch (const mdbx::not_found &) {
      std::cout << "expected failure: no such key (exit code 1, handled)\n";
    }

    std::cout << "ok: config-store-12 works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}