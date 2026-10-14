// EN: config-store, slice 09: geometry, options and statistics
// EN: Textbook section: Volume II, chapter 9, "Environment configuration", §9.2/§9.4/§9.6.
// EN: What is added: setting the geometry at creation time, runtime options and printing
// EN:   the environment statistics (mdbx_env_stat/info).
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-09.c++ -lmdbx
// EN: Run: ./config-store-09
// EN: Expected output (numbers depend on the platform):
// RU: config-store, срез 09: геометрия, опции и статистика
// RU: Раздел учебника: Том II, глава 9, «Конфигурация окружения», §9.2/§9.4/§9.6.
// RU: Что добавляется: задание геометрии при создании, runtime-опций и вывод
// RU:   статистики окружения (mdbx_env_stat/info).
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-09.c++ -lmdbx
// RU: Запуск: ./config-store-09
// RU: Ожидаемый вывод (числа зависят от платформы):
//   stat: ps=<N> depth=<N> leaf=<N> branch=<N> overflow=<N> entries=<N>
//   ok: config-store-09 works
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
    const std::string path = std::string(example::tmpdir()) + "/config-store-09.mdbx";
    mdbx::env::remove(path);

    // EN: Geometry: a compact DB with explicit size bounds.
    // RU: Геометрия: компактная БД с явными границами размера.
    mdbx::env::geometry geo;
    geo.size_lower = 4 * mdbx::env::geometry::MB;
    geo.size_now = 8 * mdbx::env::geometry::MB;
    geo.size_upper = 32 * mdbx::env::geometry::MB;
    geo.growth_step = 2 * mdbx::env::geometry::MB;
    geo.shrink_threshold = 512 * 1024;

    mdbx::env_managed env(path, mdbx::env_managed::create_parameters().set_geometry(geo),
                          mdbx::env::operate_parameters());

    // EN: Runtime options.
    // RU: Runtime-опции.
    env.set_sync_threshold(64 * 1024);
    env.set_extra_option(mdbx::env::extra_runtime_option::prefault_write_enable, 1);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 100; ++i) {
        const auto key = "key." + std::to_string(i);
        const auto value = "value." + std::to_string(i);
        txn.insert(table, mdbx::slice(key), mdbx::slice(value));
      }
      txn.commit();
    }

    const auto stat = env.get_stat();
    std::cout << "stat: ps=" << stat.ms_psize << " depth=" << stat.ms_depth << " leaf=" << stat.ms_leaf_pages
              << " branch=" << stat.ms_branch_pages << " overflow=" << stat.ms_overflow_pages
              << " entries=" << stat.ms_entries << "\n";

    std::cout << "ok: config-store-09 works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}