// EN: config-store, slice 10: choosing the synchronization mode
// EN: Textbook section: Volume II, chapter 10, "Durability modes", §10.2 "Synchronization flags".
// EN: What is added: choosing the durability mode when opening the DB; an explicit flush
// EN:   to disk (sync_to_disk) before exiting.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-10.c++ -lmdbx
// EN: Run: ./config-store-10 [durable|nometasync|safe_nosync]
// EN: Expected output (demo, safe_nosync by default):
// RU: config-store, срез 10: выбор режима синхронизации
// RU: Раздел учебника: Том II, глава 10, «Режимы долговечности», §10.2 «Флаги синхронизации».
// RU: Что добавляется: выбор режима долговечности при открытии БД; явный сброс
// RU:   на диск (sync_to_disk) перед завершением.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-10.c++ -lmdbx
// RU: Запуск: ./config-store-10 [durable|nometasync|safe_nosync]
// RU: Ожидаемый вывод (демо, по умолчанию safe_nosync):
//   durability: safe_nosync
//   ok: set theme = dark
//   ok: sync done
//   ok: config-store-10 works
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main(int argc, char **argv) {
  try {
    const std::string path = std::string(example::tmpdir()) + "/config-store-10.mdbx";
    mdbx::env::remove(path);

    // EN: The durability mode is chosen from the argument (safe_nosync by default).
    // RU: Режим долговечности выбирается из аргумента (по умолчанию — safe_nosync).
    mdbx::env::operate_parameters params;
    std::string mode_name = "safe_nosync";
    const std::string arg = argc > 1 ? argv[1] : "";
    if (arg == "durable") {
      params.robust_synchronous();
      mode_name = "durable";
    } else if (arg == "nometasync") {
      params.half_synchronous_weak_last();
      mode_name = "nometasync";
    } else if (arg == "safe_nosync") {
      params.lazy_weak_tail();
      mode_name = "safe_nosync";
    } else if (!arg.empty()) {
      std::cerr << "usage: config-store-10 [durable|nometasync|safe_nosync]\n";
      return EXIT_FAILURE;
    }
    std::cout << "durability: " << mode_name << "\n";

    mdbx::env_managed env = example::env_open(path, params);
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.upsert(table, mdbx::slice("theme"), mdbx::slice("dark"));
      txn.commit();
    }
    std::cout << "ok: set theme = dark\n";

    env.sync_to_disk(true);
    std::cout << "ok: sync done\n";

    std::cout << "ok: config-store-10 works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}