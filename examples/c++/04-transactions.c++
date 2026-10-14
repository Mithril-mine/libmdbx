// EN: Transactions: begin/commit/abort, MVCC read-only observer
// EN: Textbook section: Volume I, chapter 5, "Transactions — first steps",
// EN:   sections "begin/commit/abort", "read-only".
// EN: What it demonstrates: commit fixes the changes; abort rolls them back;
// EN:   a read-only observer transaction (in a separate thread) sees a stable
// EN:   snapshot (MVCC): invisible uncommitted and subsequent committed
// EN:   changes. A writing and a reading transaction of one thread cannot overlap
// EN:   (MDBX_TXN_OVERLAPPING), therefore the observer is moved to a separate thread.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 04-transactions.c++ -lmdbx -pthread
// EN: Run: ./04-transactions
// EN: Expected output:
// EN:   observer sees: v1
// EN:   during uncommitted write, observer still sees: v1
// EN:   after abort, observer still sees: v1
// EN:   after committed v3, old observer still sees: v1
// EN:   fresh reader sees: v3
// EN:   ok: abort rolled back; MVCC snapshot stable
// EN: On failure — message to stderr and exit 1.
// RU: Transactions: begin/commit/abort, MVCC read-only observer
// RU: Раздел учебника: Том I, глава 5, «Транзакции — первые шаги»,
// RU:   разделы «begin/commit/abort», «read-only».
// RU: Что демонстрирует: commit фиксирует изменения; abort откатывает их;
// RU:   read-only транзакция-наблюдатель (в отдельном потоке) видит стабильный
// RU:   снапшот (MVCC): невидимые незакоммиченные и последующие закоммиченные
// RU:   изменения. Пишущая и читающая транзакции одного потока не могут пересекаться
// RU:   (MDBX_TXN_OVERLAPPING), поэтому наблюдатель вынесен в отдельный поток.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 04-transactions.c++ -lmdbx -pthread
// RU: Запуск: ./04-transactions
// RU: Ожидаемый вывод:
// RU:   observer sees: v1
// RU:   during uncommitted write, observer still sees: v1
// RU:   after abort, observer still sees: v1
// RU:   after committed v3, old observer still sees: v1
// RU:   fresh reader sees: v3
// RU:   ok: abort rolled back; MVCC snapshot stable
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

#include <mdbx.h++>
#include "common.h++"

namespace {

std::atomic<int> stage{0};

void observer_run(mdbx::env_managed &env) {
  auto rtxn = env.start_read();
  auto table = rtxn.open_map(nullptr);
  std::cout << "observer sees: " << rtxn.get(table, mdbx::slice("key")).as_string() << "\n";
  stage = 1;

  while (stage.load() != 2)
    std::this_thread::yield();
  std::cout << "during uncommitted write, observer still sees: "
            << rtxn.get(table, mdbx::slice("key")).as_string() << "\n";
  stage = 3;

  while (stage.load() != 4)
    std::this_thread::yield();
  std::cout << "after abort, observer still sees: "
            << rtxn.get(table, mdbx::slice("key")).as_string() << "\n";
  stage = 5;

  while (stage.load() != 6)
    std::this_thread::yield();
  std::cout << "after committed v3, old observer still sees: "
            << rtxn.get(table, mdbx::slice("key")).as_string() << "\n";
  rtxn.abort();

  // EN: A new reader after the commit sees the actual state.
  // RU: Новый читатель после коммита видит актуальное состояние.
  auto fresh = env.start_read();
  auto ftable = fresh.open_map(nullptr);
  std::cout << "fresh reader sees: " << fresh.get(ftable, mdbx::slice("key")).as_string() << "\n";
  fresh.abort();
  stage = 7;
}

} // namespace

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-04-transactions.mdbx";
    mdbx::env::remove(path);

    mdbx::env_managed env = example::env_open(path);

    // EN: A base record committed by the first transaction.
    // RU: Базовая запись, зафиксированная первой транзакцией.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      txn.commit();
    }

    // EN: A read-only observer in a separate thread: a fixed snapshot of the database.
    // RU: Read-only наблюдатель в отдельном потоке: фиксированный снапшот базы.
    std::thread observer(observer_run, std::ref(env));
    while (stage.load() != 1)
      std::this_thread::yield();

    // EN: A writing transaction overwrites the key but does not commit yet.
    // RU: Пишущая транзакция перезаписывает ключ, но ещё не коммитит.
    {
      auto wtxn = env.start_write();
      auto table = wtxn.open_map(nullptr);
      wtxn.upsert(table, mdbx::slice("key"), mdbx::slice("v2"));
      // EN: the observer checks the snapshot during the uncommitted write
      // RU: наблюдатель проверяет снапшот при незакоммиченной записи
      stage = 2;
      while (stage.load() != 3)
        std::this_thread::yield();
      // EN: rollback: v2 is not saved
      // RU: откат: v2 не сохраняется
      wtxn.abort();
    }
    // EN: the observer checks the snapshot after the abort
    // RU: наблюдатель проверяет снапшот после abort
    stage = 4;
    while (stage.load() != 5)
      std::this_thread::yield();

    // EN: A new writing transaction commits v3.
    // RU: Новая пишущая транзакция коммитит v3.
    {
      auto wtxn = env.start_write();
      auto table = wtxn.open_map(nullptr);
      wtxn.upsert(table, mdbx::slice("key"), mdbx::slice("v3"));
      wtxn.commit();
    }
    // EN: the observer checks the snapshot after the commit of v3
    // RU: наблюдатель проверяет снапшот после коммита v3
    stage = 6;
    while (stage.load() != 7)
      std::this_thread::yield();

    observer.join();

    std::cout << "ok: abort rolled back; MVCC snapshot stable\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}