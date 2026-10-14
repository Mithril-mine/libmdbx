// EN: Durability modes: sync flags, sync_period/sync_bytes
// EN: Textbook section: Volume II, chapter 10, "Durability modes",
// EN:   sections "Sync flags", "syncbytes/syncperiod".
// EN: What it demonstrates: opening the same database with different durability
// EN:   modes (robust_synchronous / half_synchronous / lazy); an explicit flush
// EN:   to disk via sync_to_disk(); setting sync_bytes/sync_period.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 13-sync-modes.c++ -lmdbx
// EN: Run: ./13-sync-modes
// EN: Expected output:
// EN:   durable: ok
// EN:   nometasync: ok
// EN:   safe_nosync: ok
// EN:   ok: sync modes switched between opens; sync_to_disk works
// EN: On failure — message to stderr and exit 1.
// RU: Durability modes: sync flags, sync_period/sync_bytes
// RU: Раздел учебника: Том II, глава 10, «Режимы долговечности»,
// RU:   разделы «Флаги синхронизации», «syncbytes/syncperiod».
// RU: Что демонстрирует: открытие одной и той же БД с разными режимами
// RU:   долговечности (robust_synchronous / half_synchronous / lazy); явный сброс
// RU:   на диск через sync_to_disk(); установку sync_bytes/sync_period.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 13-sync-modes.c++ -lmdbx
// RU: Запуск: ./13-sync-modes
// RU: Ожидаемый вывод:
// RU:   durable: ok
// RU:   nometasync: ok
// RU:   safe_nosync: ok
// RU:   ok: sync modes switched between opens; sync_to_disk works
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-13-sync-modes.mdbx";
    mdbx::env::remove(path);

    // EN: Run 1: full durability (sync before the commit returns).
    // RU: Прогон 1: полная долговечность (sync перед ответом коммита).
    {
      auto env = example::env_open(path, mdbx::env::operate_parameters().robust_synchronous());
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      txn.commit();
      // EN: an explicit flush to disk
      // RU: явный сброс на диск
      env.sync_to_disk(true);
      std::cout << "durable: ok\n";
    }

    // EN: Run 2: NOMETASYNC (metadata is not synced on every commit).
    // RU: Прогон 2: NOMETASYNC (метаданные не синхронизируются при каждом коммите).
    {
      auto env = example::env_open(path, mdbx::env::operate_parameters().half_synchronous_weak_last());
      auto txn = env.start_read();
      auto table = txn.open_map(nullptr);
      std::cout << "nometasync: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";
      txn.abort();
    }

    // EN: Run 3: SAFE_NOSYNC (data is on disk, metadata may lag behind).
    // RU: Прогон 3: SAFE_NOSYNC (данные на диске, метаданные могут отставать).
    {
      auto env = example::env_open(path, mdbx::env::operate_parameters().lazy_weak_tail());
      // EN: sync_bytes = 64 KiB
      // RU: sync_bytes = 64 KiB
      env.set_sync_threshold(64 * 1024);
      // EN: sync_period = 1 s
      // RU: sync_period = 1 s
      env.set_sync_period__seconds_double(1.0);
      auto txn = env.start_read();
      auto table = txn.open_map(nullptr);
      std::cout << "safe_nosync: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";
      txn.abort();
    }

    std::cout << "ok: sync modes switched between opens; sync_to_disk works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}