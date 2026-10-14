// EN: config-store, slice 04: CRUD commands and write flags
// EN: Textbook section: Volume I, chapter 4, "Basic CRUD operations", §4.2/§4.3.
// EN: What is added: the add (insert_unique), set (upsert), get, del commands;
// EN:   explicit handling of MDBX_KEYEXIST/MDBX_NOTFOUND as expected codes.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-04.c++ -lmdbx
// EN: Run: ./config-store-04 [add KEY VALUE | set KEY VALUE | get KEY | del KEY]
// EN: Expected output (demo):
// RU: config-store, срез 04: CRUD-команды и флаги записи
// RU: Раздел учебника: Том I, глава 4, «Базовые операции CRUD», §4.2/§4.3.
// RU: Что добавляется: команды add (insert_unique), set (upsert), get, del;
// RU:   явная обработка MDBX_KEYEXIST/MDBX_NOTFOUND как ожидаемых кодов.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-04.c++ -lmdbx
// RU: Запуск: ./config-store-04 [add KEY VALUE | set KEY VALUE | get KEY | del KEY]
// RU: Ожидаемый вывод (демо):
//   add host = localhost
//   add host again -> MDBX_KEYEXIST as expected
//   set host = 127.0.0.1
//   get host = 127.0.0.1
//   get absent -> MDBX_NOTFOUND as expected
//   ok: config-store-04 works
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

namespace {

const std::string db_path() { return std::string(example::tmpdir()) + "/config-store-04.mdbx"; }

void usage() {
  std::cerr << "usage: config-store-04 [add|set KEY VALUE | get KEY | del KEY]\n";
}

} // namespace

int main(int argc, char **argv) {
  try {
    mdbx::env::remove(db_path());
    mdbx::env_managed env = example::env_open(db_path());

    if (argc >= 4 && std::string(argv[1]) == "add") {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      try {
        // EN: NOOVERWRITE
        // RU: NOOVERWRITE
        txn.insert(table, mdbx::slice(argv[2]), mdbx::slice(argv[3]));
        txn.commit();
        std::cout << "ok: add " << argv[2] << " = " << argv[3] << "\n";
      } catch (const mdbx::key_exists &) {
        std::cerr << "already exists: " << argv[2] << "\n";
        return EXIT_FAILURE;
      }
      return EXIT_SUCCESS;
    }
    if (argc >= 4 && std::string(argv[1]) == "set") {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      // EN: UPSERT
      // RU: UPSERT
      txn.upsert(table, mdbx::slice(argv[2]), mdbx::slice(argv[3]));
      txn.commit();
      std::cout << "ok: set " << argv[2] << " = " << argv[3] << "\n";
      return EXIT_SUCCESS;
    }
    if (argc >= 3 && std::string(argv[1]) == "get") {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      try {
        std::cout << rtxn.get(table, mdbx::slice(argv[2])).as_string() << "\n";
      } catch (const mdbx::not_found &) {
        std::cerr << "no such key: " << argv[2] << "\n";
        return EXIT_FAILURE;
      }
      rtxn.abort();
      return EXIT_SUCCESS;
    }
    if (argc >= 3 && std::string(argv[1]) == "del") {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      if (!txn.erase(table, mdbx::slice(argv[2]))) {
        std::cerr << "no such key: " << argv[2] << "\n";
        return EXIT_FAILURE;
      }
      txn.commit();
      std::cout << "ok: del " << argv[2] << "\n";
      return EXIT_SUCCESS;
    }
    if (argc > 1) {
      usage();
      return EXIT_FAILURE;
    }

    // EN: Demo: add does not overwrite, set does overwrite.
    // RU: Демо: add не перезаписывает, set перезаписывает.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("host"), mdbx::slice("localhost"));
      txn.commit();
    }
    std::cout << "add host = localhost\n";
    try {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("host"), mdbx::slice("other"));
      txn.commit();
    } catch (const mdbx::key_exists &) {
      std::cout << "add host again -> MDBX_KEYEXIST as expected\n";
    }
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.upsert(table, mdbx::slice("host"), mdbx::slice("127.0.0.1"));
      txn.commit();
    }
    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::cout << "set host = 127.0.0.1\n";
      std::cout << "get host = " << rtxn.get(table, mdbx::slice("host")).as_string() << "\n";
      try {
        (void)rtxn.get(table, mdbx::slice("absent"));
      } catch (const mdbx::not_found &) {
        std::cout << "get absent -> MDBX_NOTFOUND as expected\n";
      }
      rtxn.abort();
    }

    std::cout << "ok: config-store-04 works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}