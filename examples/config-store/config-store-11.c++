// EN: config-store, slice 11: multithreaded access
// EN: Textbook section: Volume II, chapter 11, "Multithreading", §11.2 "Sticky threads".
// EN: What is added: several reader threads and one writer on the same
// EN:   environment; parallel readers see a stable snapshot.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-11.c++ -lmdbx -pthread
// EN: Run: ./config-store-11
// EN: Expected output:
// RU: config-store, срез 11: многопоточный доступ
// RU: Раздел учебника: Том II, глава 11, «Многопоточность», §11.2 «Sticky threads».
// RU: Что добавляется: несколько потоков читателей и один писатель на одном
// RU:   окружении; параллельные читатели видят стабильный снапшот.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-11.c++ -lmdbx -pthread
// RU: Запуск: ./config-store-11
// RU: Ожидаемый вывод:
//   readers saw the same snapshot (10 keys)
//   after writer finished, total keys = 20
//   ok: config-store-11 works
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/config-store-11.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 10; ++i) {
        const auto key = "key." + std::to_string(i);
        txn.insert(table, mdbx::slice(key), mdbx::slice("v"));
      }
      txn.commit();
    }

    auto base = env.start_read();
    auto base_table = base.open_map(nullptr);
    const uint64_t snapshot_size = base.get_map_stat(base_table).ms_entries;

    // EN: The writer adds 10 more keys.
    // RU: Писатель добавляет ещё 10 ключей.
    std::atomic<bool> writer_done{false};
    std::thread writer([&] {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 10; i < 20; ++i) {
        const auto key = "key." + std::to_string(i);
        txn.insert(table, mdbx::slice(key), mdbx::slice("w"));
      }
      txn.commit();
      writer_done = true;
    });

    // EN: Two readers work on clones of the shared snapshot.
    // RU: Два читателя работают по клонам общего снапшота.
    std::atomic<uint64_t> counts[2];
    std::vector<std::thread> readers;
    for (int r = 0; r < 2; ++r) {
      readers.emplace_back([&, r] {
        auto clone = base.clone();
        auto table = clone.open_map(nullptr);
        counts[r] = clone.get_map_stat(table).ms_entries;
        clone.abort();
      });
    }
    for (auto &t : readers)
      t.join();
    writer.join();
    base.abort();

    if (counts[0] != snapshot_size || counts[1] != snapshot_size) {
      std::cerr << "FAIL: readers saw different snapshots\n";
      return EXIT_FAILURE;
    }
    std::cout << "readers saw the same snapshot (" << snapshot_size << " keys)\n";

    {
      auto txn = env.start_read();
      auto table = txn.open_map(nullptr);
      std::cout << "after writer finished, total keys = " << txn.get_map_stat(table).ms_entries << "\n";
      txn.abort();
    }

    std::cout << "ok: config-store-11 works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}