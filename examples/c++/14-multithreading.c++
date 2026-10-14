// EN: Multithreading: writer + cloned readers
// EN: Textbook section: Volume II, chapter 11, "Multithreading",
// EN:   sections "Sticky threads", "NOSTICKYTHREADS".
// EN: What it demonstrates: a writer and parallel readers on one environment;
// EN:   cloning a read-only transaction via txn_managed::clone() — all clones
// EN:   see one snapshot, independent of the writer's commits.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 14-multithreading.c++ -lmdbx -pthread
// EN: Run: ./14-multithreading
// EN: Expected output:
// EN:   snapshot(reader0) = 10
// EN:   snapshot(reader1) = 10
// EN:   fresh(reader0) = 30
// EN:   fresh(reader1) = 30
// EN:   ok: readers see stable snapshot while writer commits
// EN: On failure — message to stderr and exit 1.
// RU: Multithreading: writer + cloned readers
// RU: Раздел учебника: Том II, глава 11, «Многопоточность»,
// RU:   разделы «Sticky threads», «NOSTICKYTHREADS».
// RU: Что демонстрирует: писатель и параллельные читатели на одном окружении;
// RU:   клонирование read-only транзакции через txn_managed::clone() — все клоны
// RU:   видят один снапшот, не зависящий от коммитов писателя.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 14-multithreading.c++ -lmdbx -pthread
// RU: Запуск: ./14-multithreading
// RU: Ожидаемый вывод:
// RU:   snapshot(reader0) = 10
// RU:   snapshot(reader1) = 10
// RU:   fresh(reader0) = 30
// RU:   fresh(reader1) = 30
// RU:   ok: readers see stable snapshot while writer commits
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
    const std::string path = std::string(example::tmpdir()) + "/mdbx-14-multithreading.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    // EN: The initial 10 records.
    // RU: Начальные 10 записей.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 10; ++i)
        txn.insert(table, mdbx::slice(std::to_string(i)), mdbx::slice("x"));
      txn.commit();
    }

    // EN: The base snapshot for the clones.
    // RU: Базовый снапшот для клонов.
    auto base = env.start_read();
    (void)base.open_map(nullptr);

    // EN: The writer adds 20 more records in parallel with reading.
    // RU: Писатель добавляет ещё 20 записей параллельно чтению.
    std::atomic<bool> writer_done{false};
    std::thread writer([&] {
      for (int batch = 0; batch < 2; ++batch) {
        auto txn = env.start_write();
        auto table = txn.open_map(nullptr);
        for (int i = 0; i < 10; ++i) {
          const int k = 10 + batch * 10 + i;
          txn.insert(table, mdbx::slice(std::to_string(k)), mdbx::slice("w"));
        }
        txn.commit();
        std::this_thread::yield();
      }
      writer_done = true;
    });

    // EN: Two readers: each works with its own clone of the base snapshot.
    // RU: Два читателя: каждый работает со своим клоном базового снапшота.
    std::atomic<uint64_t> snapshot_count[2];
    std::atomic<uint64_t> fresh_count[2];
    std::vector<std::thread> readers;
    for (int r = 0; r < 2; ++r) {
      readers.emplace_back([&, r] {
        // EN: a snapshot clone for this thread
        // RU: клон снапшота для этого потока
        auto clone = base.clone();
        auto table = clone.open_map(nullptr);
        snapshot_count[r] = clone.get_map_stat(table).ms_entries;
        clone.abort();

        while (!writer_done.load())
          std::this_thread::yield();

        // EN: After the writer finishes, a fresh transaction sees all the data.
        // RU: После завершения писателя — свежая транзакция видит все данные.
        auto fresh = env.start_read();
        auto ftable = fresh.open_map(nullptr);
        fresh_count[r] = fresh.get_map_stat(ftable).ms_entries;
        fresh.abort();
      });
    }
    for (auto &t : readers)
      t.join();
    writer.join();
    base.abort();

    std::cout << "snapshot(reader0) = " << snapshot_count[0].load() << "\n";
    std::cout << "snapshot(reader1) = " << snapshot_count[1].load() << "\n";
    std::cout << "fresh(reader0) = " << fresh_count[0].load() << "\n";
    std::cout << "fresh(reader1) = " << fresh_count[1].load() << "\n";

    if (snapshot_count[0] != 10 || snapshot_count[1] != 10 || fresh_count[0] != 30 || fresh_count[1] != 30) {
      std::cerr << "FAIL: unexpected counts\n";
      return EXIT_FAILURE;
    }

    std::cout << "ok: readers see stable snapshot while writer commits\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}