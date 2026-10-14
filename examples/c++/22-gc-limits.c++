// EN: GC limits: rp_augment_limit and gc_time_limit
// EN: Textbook section: Volume III, chapter 16, "GC", §16.6 "rp_augment_limit/gc_time_limit".
// EN: What it demonstrates: setting and reading the GC options — rp_augment_limit and
// EN:   gc_time_limit — via extra_runtime_option; the effect on working with long
// EN:   values (fragmentation/growth).
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 22-gc-limits.c++ -lmdbx
// EN: Run: ./22-gc-limits
// EN: Expected output (numbers depend on the system):
// EN:   rp_augment_limit = <N>
// EN:   gc_time_limit = <M>
// EN:   ok: GC limits set and read
// EN: On failure — message to stderr and exit 1.
// RU: Лимиты GC: rp_augment_limit и gc_time_limit
// RU: Раздел учебника: Том III, глава 16, «GC», §16.6 «rp_augment_limit/gc_time_limit».
// RU: Что демонстрирует: установку и чтение опций GC — rp_augment_limit и
// RU:   gc_time_limit — через extra_runtime_option; влияние на работу с длинными
// RU:   значениями (фрагментация/рост).
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 22-gc-limits.c++ -lmdbx
// RU: Запуск: ./22-gc-limits
// RU: Ожидаемый вывод (числа зависят от системы):
// RU:   rp_augment_limit = <N>
// RU:   gc_time_limit = <M>
// RU:   ok: GC limits set and read
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
    const std::string path = std::string(example::tmpdir()) + "/mdbx-22-gc-limits.mdbx";
    mdbx::env::remove(path);

    // EN: A workload with long values takes tens of MB, so the upper bound is
    // EN: set explicitly instead of relying on the system default size (which may
    // EN: be insufficient on 32-bit platforms).
    // RU: Нагрузка с длинными значениями занимает десятки МБ, поэтому верхнюю
    // RU: границу задаём явно, а не полагаемся на системный размер по умолчанию
    // RU: (на 32-битных платформах он может оказаться недостаточным).
    mdbx::env::geometry geo;
    geo.make_dynamic(1 * mdbx::env::geometry::MiB, 64 * mdbx::env::geometry::MiB);
    mdbx::env_managed env = example::env_open(path, geo);

    using opt = mdbx::env::extra_runtime_option;

    // EN: GC limits: 0 for gc_time_limit means "allocate the entire available
    // EN: transaction time budget"; rp_augment_limit limits the growth of the page
    // EN: reserve for a reclaim operation.
    // RU: Лимиты GC: 0 для gc_time_limit означает «выделять весь доступный бюджет
    // RU: времени транзакции»; rp_augment_limit ограничивает рост запаса страниц
    // RU: для операции рекламации.
    env.set_extra_option(opt::rp_augment_limit, 128 * 1024);
    // EN: gc_time_limit is specified in 1/65536 fractions of a second (16.16 fixed point):
    // EN: 2500 ≈ 38 ms for searching page runs in the GC inside a transaction.
    // RU: gc_time_limit задаётся в 1/65536 долях секунды (16.16 fixed point):
    // RU: 2500 ≈ 38 мс на поиск последовательностей страниц в GC внутри транзакции.
    env.set_extra_option(opt::gc_time_limit, 2500);

    // EN: A workload with long values: forces the GC to work harder.
    // RU: Нагрузка с длинными значениями: заставляет GC работать активнее.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 2000; ++i) {
        const auto key = "k" + std::to_string(i);
        const std::string value(4096 + (i % 7) * 512, 'v');
        txn.insert(table, mdbx::slice(key), mdbx::slice(value));
      }
      txn.commit();
      auto del = env.start_write();
      auto dtable = del.open_map(nullptr);
      for (int i = 0; i < 2000; i += 2)
        del.erase(dtable, mdbx::slice("k" + std::to_string(i)));
      del.commit();
    }

    std::cout << "rp_augment_limit = " << env.extra_option(opt::rp_augment_limit) << "\n";
    std::cout << "gc_time_limit = " << env.extra_option(opt::gc_time_limit) << "\n";

    std::cout << "ok: GC limits set and read\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}