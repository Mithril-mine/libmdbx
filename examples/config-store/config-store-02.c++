// EN: config-store, slice 02: application skeleton
// EN: Textbook section: Volume I, chapter 2, "Installation and first launch", §2.4 "Minimal example".
// EN: What is added compared to the previous slice: the configuration store
// EN:   skeleton — opening the DB, set/get/del commands over the main table.
// EN:   End-to-end project: as the volumes progress, the slice becomes a full application.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-02.c++ -lmdbx
// EN: Run: ./config-store-02 [set KEY VALUE | get KEY | del KEY]
// EN:   without arguments a demo scenario is executed.
// EN: Expected output (demo):
// RU: config-store, срез 02: каркас приложения
// RU: Раздел учебника: Том I, глава 2, «Установка и первый запуск», §2.4 «Минимальный пример».
// RU: Что добавляется по сравнению с предыдущим срезом: каркас конфигурационного
// RU:   хранилища — открытие БД, команды set/get/del поверх главной таблицы.
// RU:   Сквозной проект: по мере томов срез превращается в полноценное приложение.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-02.c++ -lmdbx
// RU: Запуск: ./config-store-02 [set KEY VALUE | get KEY | del KEY]
// RU:   без аргументов выполняется демонстрационный сценарий.
// RU: Ожидаемый вывод (демо):
//   ok: set theme = dark
//   ok: set lang = ru
//   ok: theme = dark
//   ok: del lang
//   ok: config-store-02 works
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

namespace {

const std::string db_path() { return std::string(example::tmpdir()) + "/config-store-02.mdbx"; }

void usage() {
  std::cerr << "usage: config-store-02 [set KEY VALUE | get KEY | del KEY]\n";
}

} // namespace

int main(int argc, char **argv) {
  try {
    // EN: the demo always starts with a clean DB
    // RU: демо всегда стартует с чистой БД
    mdbx::env::remove(db_path());
    mdbx::env_managed env = example::env_open(db_path());

    if (argc >= 4 && std::string(argv[1]) == "set") {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.upsert(table, mdbx::slice(argv[2]), mdbx::slice(argv[3]));
      txn.commit();
      std::cout << "ok: set " << argv[2] << " = " << argv[3] << "\n";
      return EXIT_SUCCESS;
    }
    if (argc >= 3 && std::string(argv[1]) == "get") {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::cout << rtxn.get(table, mdbx::slice(argv[2])).as_string() << "\n";
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

    // EN: Demo scenario: a mini-CLI over the same commands.
    // RU: Демонстрационный сценарий: мини-CLI над теми же командами.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.upsert(table, mdbx::slice("theme"), mdbx::slice("dark"));
      txn.upsert(table, mdbx::slice("lang"), mdbx::slice("ru"));
      txn.commit();
    }
    std::cout << "ok: set theme = dark\n";
    std::cout << "ok: set lang = ru\n";
    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::cout << "ok: theme = " << rtxn.get(table, mdbx::slice("theme")).as_string() << "\n";
      rtxn.abort();
    }
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.erase(table, mdbx::slice("lang"));
      txn.commit();
    }
    std::cout << "ok: del lang\n";

    std::cout << "ok: config-store-02 works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}