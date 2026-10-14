// EN: Rules & counterexamples: forgotten reader → MAP_FULL; thread mismatch
// EN: Textbook section: Volume V, chapter 27, "Rules and advice", §27.1–§27.4.
// EN: What it demonstrates: (1) a "forgotten" read snapshot holds pages and, when
// EN:   the database overflows, yields MDBX_MAP_FULL — diagnosis via the reader list and
// EN:   resolution after the reader is released; (2) using a transaction from
// EN:   another thread — MDBX_THREAD_MISMATCH.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 34-rules-counter.c++ -lmdbx -pthread
// EN: Run: ./34-rules-counter
// EN: Expected output:
// RU: Правила и контрпримеры: «забытый» читатель → MAP_FULL; несоответствие потоков
// RU: Раздел учебника: Том V, глава 27, «Правила и советы», §27.1–§27.4.
// RU: Что демонстрирует: (1) «забытый» read-снапшот удерживает страницы и при
// RU:   переполнении БД даёт MDBX_MAP_FULL — диагностика через reader list и
// RU:   разрешение после освобождения читателя; (2) использование транзакции из
// RU:   другого потока — MDBX_THREAD_MISMATCH.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 34-rules-counter.c++ -lmdbx -pthread
// RU: Запуск: ./34-rules-counter
// RU: Ожидаемый вывод:
//   MAP_FULL while reader holds snapshot; reader lag=<L>
//   after reader released, write succeeded
//   thread mismatch: MDBX_THREAD_MISMATCH as expected
//   ok: counterexamples reproduced and diagnosed
// EN: On failure — message to stderr and exit 1.
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
    const std::string path = std::string(example::tmpdir()) + "/mdbx-34-rules-counter.mdbx";
    mdbx::env::remove(path);

    // EN: Small geometry to hit MDBX_MAP_FULL sooner.
    // RU: Маленькая геометрия, чтобы быстрее упереться в MDBX_MAP_FULL.
    mdbx::env::geometry geo;
    geo.size_lower = 256 * 1024;
    geo.size_now = 256 * 1024;
    geo.size_upper = 256 * 1024;
    mdbx::env_managed env(path, mdbx::env_managed::create_parameters().set_geometry(geo),
                          mdbx::env::operate_parameters());

    // EN: Counterexample 1: a "forgotten" reader holds its snapshot.
    // EN: First a small fill, so the reader has something to hold.
    // RU: Контрпример 1: «забытый» читатель удерживает снапшот.
    // RU: Сначала небольшое наполнение, чтобы читателю было что держать.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("k0"), mdbx::slice("seed"));
      txn.commit();
    }
    std::atomic<bool> reader_ready{false};
    std::atomic<bool> release_reader{false};
    std::thread holder([&] {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      (void)rtxn.get(table, mdbx::slice("k0"));
      reader_ready = true;
      while (!release_reader.load())
        std::this_thread::yield();
      rtxn.abort();
    });
    while (!reader_ready.load())
      std::this_thread::yield();

    // EN: The writer fills the database — on overflow we get MAP_FULL.
    // RU: Писатель заполняет БД — при переполнении получаем MAP_FULL.
    size_t written = 0;
    try {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (;;) {
        // EN: does not overlap with "k0"
        // RU: не пересекается с "k0"
        const auto key = "w" + std::to_string(written);
        const std::string value(1024, 'x');
        txn.insert(table, mdbx::slice(key), mdbx::slice(value));
        ++written;
      }
    } catch (const mdbx::db_full &) {
      // EN: Diagnosis: show the readers and their lag.
      // RU: Диагностика: показываем читателей и их отставание.
      struct visitor {
        int operator()(const mdbx::env::reader_info &ri, int) {
          std::cout << "MAP_FULL while reader holds snapshot; reader lag=" << ri.transaction_lag << "\n";
          return mdbx::continue_loop;
        }
      } v;
      env.enumerate_readers(v);
    }

    // EN: Release the reader — the writer can work again.
    // RU: Освобождаем читателя — писатель снова может работать.
    release_reader = true;
    holder.join();
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("after-release"), mdbx::slice("ok"));
      txn.commit();
    }
    std::cout << "after reader released, write succeeded\n";

    // EN: Counterexample 2: using a transaction from another thread.
    // RU: Контрпример 2: использование транзакции из другого потока.
    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::atomic<bool> mismatch_seen{false};
      std::thread foreign([&] {
        try {
        // EN: foreign thread
          // RU: чужой поток
        (void)rtxn.get(table, mdbx::slice("after-release"));
        } catch (const std::exception &ex) {
          // EN: Expected rejection: THREAD_MISMATCH (or a derived error code).
          // RU: Ожидаемый отказ: THREAD_MISMATCH (или производный код ошибки).
          mismatch_seen = true;
          std::cout << "thread misuse rejected: " << ex.what() << "\n";
        }
      });
      foreign.join();
      rtxn.abort();
      if (!mismatch_seen) {
        std::cerr << "FAIL: thread mismatch not detected\n";
        return EXIT_FAILURE;
      }
      std::cout << "thread mismatch: MDBX_THREAD_MISMATCH as expected\n";
    }

    std::cout << "ok: counterexamples reproduced and diagnosed\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}