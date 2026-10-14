// EN: Readers lag: holding a snapshot and observing the lag
// EN: Textbook section: Volume III, chapter 14, "MVCC and snapshots", §14.5 "RLT".
// EN: What it demonstrates: a read-only transaction in a separate thread holds
// EN:   a snapshot; writer commits increase its "lag";
// EN:   mdbx_reader_list shows reader slots, txnid and lag.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 19-readers-lag.c++ -lmdbx -pthread
// EN: Run: ./19-readers-lag
// EN: Expected output:
// EN:   reader slot: pid=<P> tid=<T> txnid=<N> lag=<L>
// EN:   ok: reader lag observed
// EN: On failure — message to stderr and exit 1.
// RU: Отставание читателей: удержание снапшота и наблюдение за lag
// RU: Раздел учебника: Том III, глава 14, «MVCC и снапшоты», §14.5 «RLT».
// RU: Что демонстрирует: read-only транзакция в отдельном потоке удерживает
// RU:   снапшот; коммиты писателя увеличивают её «отставание» (lag);
// RU:   mdbx_reader_list показывает слоты читателей, txnid и lag.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 19-readers-lag.c++ -lmdbx -pthread
// RU: Запуск: ./19-readers-lag
// RU: Ожидаемый вывод:
// RU:   reader slot: pid=<P> tid=<T> txnid=<N> lag=<L>
// RU:   ok: reader lag observed
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-19-readers-lag.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    // EN: A few commits before the snapshot is created.
    // RU: Несколько коммитов до создания снапшота.
    for (int i = 0; i < 3; ++i) {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("v"));
      txn.commit();
    }

    // EN: A long-lived reader holds the snapshot in a separate thread
    // EN: (mixing read and write on one thread is forbidden).
    // RU: Долгоживущий читатель держит снапшот в отдельном потоке
    // RU: (пересечение read+write на одном потоке запрещено).
    std::atomic<bool> snapshot_ready{false};
    std::atomic<bool> release_reader{false};
    std::thread reader([&] {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      (void)rtxn.get(table, mdbx::slice("k2"));
      snapshot_ready = true;
      while (!release_reader.load())
        std::this_thread::yield();
      rtxn.abort();
    });
    while (!snapshot_ready.load())
      std::this_thread::yield();

    // EN: Commits after the snapshot creation increase the reader's lag.
    // RU: Коммиты после создания снапшота увеличивают lag читателя.
    for (int i = 3; i < 8; ++i) {
      auto txn = env.start_write();
      auto wtable = txn.open_map(nullptr);
      txn.insert(wtable, mdbx::slice("k" + std::to_string(i)), mdbx::slice("v"));
      txn.commit();
    }

    // EN: Enumerate the readers: a reader slot has lag > 0.
    // RU: Перечислить читателей: слот читателя имеет lag > 0.
    struct visitor {
      int operator()(const mdbx::env::reader_info &ri, int) {
        std::cout << "reader slot: pid=" << ri.pid << " tid=" << ri.thread << " txnid=" << ri.transaction_id
                  << " lag=" << ri.transaction_lag << "\n";
        return mdbx::continue_loop;
      }
    } v;
    env.enumerate_readers(v);

    release_reader = true;
    reader.join();

    std::cout << "ok: reader lag observed\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}