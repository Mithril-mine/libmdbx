// EN: Commit pipeline: latency by stages
// EN: Textbook section: Volume III, chapter 15, "CoW and commit pipeline", §15.5 "Pipeline stages".
// EN: What it demonstrates: breakdown of MDBX_commit_latency by stages (preparation,
// EN:   gc_wallclock, audit, write, sync, ending, whole) for a series of commits.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 20-commit-latency.c++ -lmdbx
// EN: Run: ./20-commit-latency
// EN: Expected output (numbers are microseconds, system-dependent):
// EN:   avg latency (us): preparation=<N> gc=<N> audit=<N> write=<N> sync=<N> ending=<N> whole=<N>
// EN:   ok: commit latency stages collected
// EN: On failure — message to stderr and exit 1.
// RU: Конвейер коммита: задержка по стадиям
// RU: Раздел учебника: Том III, глава 15, «CoW и конвейер коммита», §15.5 «Стадии конвейера».
// RU: Что демонстрирует: разбор MDBX_commit_latency по стадиям (preparation,
// RU:   gc_wallclock, audit, write, sync, ending, whole) для серии коммитов.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 20-commit-latency.c++ -lmdbx
// RU: Запуск: ./20-commit-latency
// RU: Ожидаемый вывод (числа — микросекунды, зависят от системы):
// RU:   avg latency (us): preparation=<N> gc=<N> audit=<N> write=<N> sync=<N> ending=<N> whole=<N>
// RU:   ok: commit latency stages collected
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <cstdint>
#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-20-commit-latency.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    constexpr int commits = 100;
    uint64_t prep = 0, gc = 0, audit = 0, write = 0, sync = 0, ending = 0, whole = 0;

    for (int i = 0; i < commits; ++i) {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("v"));
      // EN: returns MDBX_commit_latency
      // RU: возвращает MDBX_commit_latency
      const auto lat = txn.commit_get_latency();
      prep += lat.preparation;
      gc += lat.gc_wallclock;
      audit += lat.audit;
      write += lat.write;
      sync += lat.sync;
      ending += lat.ending;
      whole += lat.whole;
    }

    std::cout << "avg latency (us): preparation=" << (prep / commits) << " gc=" << (gc / commits)
              << " audit=" << (audit / commits) << " write=" << (write / commits) << " sync=" << (sync / commits)
              << " ending=" << (ending / commits) << " whole=" << (whole / commits) << "\n";

    std::cout << "ok: commit latency stages collected\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}