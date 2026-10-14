// EN: Profiler: GC profiling counters (PROFGC) and ioarena
// EN: Textbook section: Volume IV, chapter 26, "Benchmarks and measurements", §26.3 "PROFGC".
// EN: What it demonstrates: reading gc_prof fields from commit_latency (wloops,
// EN:   coalescences, flushes, kicks, max_reader_lag, max_retained_pages, etc.).
// EN:   Fields are populated only in builds with MDBX_ENABLE_PROFGC (otherwise zeros).
// EN: Benchmark CLI: ioarena (github.com/erthink/ioarena), command of the form
// EN:   `ioarena -e db -E mdbx://<path> --tasks=...`.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 33-profiler.c++ -lmdbx
// EN: Run: ./33-profiler
// EN: Expected output:
// RU: Профилировщик: счётчики профилирования GC (PROFGC) и ioarena
// RU: Раздел учебника: Том IV, глава 26, «Бенчмарки и измерения», §26.3 «PROFGC».
// RU: Что демонстрирует: чтение gc_prof-полей из commit_latency (wloops,
// RU:   coalescences, flushes, kicks, max_reader_lag, max_retained_pages и др.).
// RU:   Поля заполняются только в сборках с MDBX_ENABLE_PROFGC (иначе нули).
// RU: CLI для бенчмарков: ioarena (github.com/erthink/ioarena), команда вида
// RU:   `ioarena -e db -E mdbx://<path> --tasks=...`.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 33-profiler.c++ -lmdbx
// RU: Запуск: ./33-profiler
// RU: Ожидаемый вывод:
//   gc_prof: wloops=<N> coalescences=<M> flushes=<K> kicks=<J> max_reader_lag=<L> max_retained_pages=<P>
//   ok: PROFGC counters read
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
    const std::string path = std::string(example::tmpdir()) + "/mdbx-33-profiler.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    // EN: Load that generates GC work: insertions and deletions.
    // RU: Нагрузка, порождающая работу GC: вставки и удаления.
    for (int round = 0; round < 5; ++round) {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 500; ++i) {
        const auto key = "k" + std::to_string(round * 1000 + i);
        txn.insert(table, mdbx::slice(key), mdbx::slice("payload"));
      }
      txn.commit();
      auto del = env.start_write();
      auto dtable = del.open_map(nullptr);
      for (int i = 0; i < 500; ++i)
        del.erase(dtable, mdbx::slice("k" + std::to_string(round * 1000 + i)));
      del.commit();
    }

    auto txn = env.start_write();
    auto table = txn.open_map(nullptr);
    txn.insert(table, mdbx::slice("last"), mdbx::slice("v"));
    const auto lat = txn.commit_get_latency();

    const auto &gc = lat.gc_prof;
    std::cout << "gc_prof: wloops=" << gc.wloops << " coalescences=" << gc.coalescences << " flushes=" << gc.flushes
              << " kicks=" << gc.kicks << " max_reader_lag=" << gc.max_reader_lag
              << " max_retained_pages=" << gc.max_retained_pages << "\n";

    std::cout << "ok: PROFGC counters read\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}