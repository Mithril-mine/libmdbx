// EN: WAF: written pages per batch size
// EN: Textbook section: Volume IV, chapter 21, "WAF", §21.5 "WAF calculation", §21.6 "prefer_waf/merge_threshold".
// EN: What it demonstrates: measuring written pages (mi_pgop_stat: newly+cow) for
// EN:   different write batch sizes and calculating "pages per operation".
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 28-waf-batching.c++ -lmdbx
// EN: Run: ./28-waf-batching
// EN: Expected output (numbers depend on the system):
// EN:   batch=1: pages=... ops=1000 pages/op=...
// EN:   batch=10: pages=... ops=1000 pages/op=...
// EN:   batch=100: pages=... ops=1000 pages/op=...
// EN:   ok: WAF measured per batch size
// EN: On failure — message to stderr and exit 1.
// RU: WAF: записанные страницы в зависимости от размера батча
// RU: Раздел учебника: Том IV, глава 21, «WAF», §21.5 «Расчёт WAF», §21.6 «prefer_waf/merge_threshold».
// RU: Что демонстрирует: замер записанных страниц (mi_pgop_stat: newly+cow) для
// RU:   разных размеров батчей записи и расчёт «страниц на операцию».
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 28-waf-batching.c++ -lmdbx
// RU: Запуск: ./28-waf-batching
// RU: Ожидаемый вывод (числа зависят от системы):
// RU:   batch=1: pages=... ops=1000 pages/op=...
// RU:   batch=10: pages=... ops=1000 pages/op=...
// RU:   batch=100: pages=... ops=1000 pages/op=...
// RU:   ok: WAF measured per batch size
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <cstdint>
#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

static void measure(const std::string &path, size_t batch_size) {
  mdbx::env::remove(path);
  mdbx::env_managed env = example::env_open(path);

  constexpr int total = 1000;
  const auto initial = env.get_info().mi_pgop_stat;
  for (int off = 0; off < total; off += (int)batch_size) {
    auto txn = env.start_write();
    auto table = txn.open_map(nullptr);
    for (int i = off; i < off + (int)batch_size && i < total; ++i)
      txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("value"));
    txn.commit();
  }
  const auto final = env.get_info().mi_pgop_stat;
  const uint64_t pages = (final.newly - initial.newly) + (final.cow - initial.cow);
  std::cout << "batch=" << batch_size << ": pages=" << pages << " ops=" << total << " pages/op=" << (double)pages / total
            << "\n";
}

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-28-waf-batching.mdbx";
    for (size_t batch : {size_t(1), size_t(10), size_t(100)})
      measure(path, batch);

    std::cout << "ok: WAF measured per batch size\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}