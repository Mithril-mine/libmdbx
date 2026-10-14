// EN: Environment statistics: env_stat / env_info
// EN: Textbook section: Volume II, chapter 9, "Environment configuration", section "env_stat".
// EN: What it demonstrates: mdbx_env_stat_ex()/mdbx_env_info_ex(): pages by type
// EN:   (leaf/branch/overflow), the B-tree height, the number of entries, the geometry size,
// EN:   txnid and the "tail" reader.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 12-env-stat.c++ -lmdbx
// EN: Run: ./12-env-stat
// EN: Expected output (values depend on the amount of data):
// EN:   stat: ps=4096 depth=2 leaf=1 branch=1 overflow=0 entries=100
// EN:   info: recent_txnid=<N> latter_reader_txnid=<M>
// EN:   ok: env_stat/env_info read
// EN: On failure — message to stderr and exit 1.
// RU: Environment statistics: env_stat / env_info
// RU: Раздел учебника: Том II, глава 9, «Конфигурация окружения», раздел «env_stat».
// RU: Что демонстрирует: mdbx_env_stat_ex()/mdbx_env_info_ex(): страницы по типам
// RU:   (leaf/branch/overflow), высоту B-дерева, число записей, размер геометрии,
// RU:   txnid и «хвостового» читателя.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 12-env-stat.c++ -lmdbx
// RU: Запуск: ./12-env-stat
// RU: Ожидаемый вывод (значения зависят от объёма данных):
// RU:   stat: ps=4096 depth=2 leaf=1 branch=1 overflow=0 entries=100
// RU:   info: recent_txnid=<N> latter_reader_txnid=<M>
// RU:   ok: env_stat/env_info read
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-12-env-stat.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 100; ++i) {
        const auto key = std::to_string(i);
        const auto value = "value-" + std::to_string(i);
        txn.insert(table, mdbx::slice(key), mdbx::slice(value));
      }
      txn.commit();
    }

    // EN: analogous to mdbx_env_stat_ex()
    // RU: аналог mdbx_env_stat_ex()
    const auto stat = env.get_stat();
    std::cout << "stat: ps=" << stat.ms_psize << " depth=" << stat.ms_depth
              << " leaf=" << stat.ms_leaf_pages << " branch=" << stat.ms_branch_pages
              << " overflow=" << stat.ms_overflow_pages << " entries=" << stat.ms_entries << "\n";

    // EN: analogous to mdbx_env_info_ex()
    // RU: аналог mdbx_env_info_ex()
    const auto info = env.get_info();
    std::cout << "info: recent_txnid=" << info.mi_recent_txnid
              << " latter_reader_txnid=" << info.mi_latter_reader_txnid << " geo.current=" << info.mi_geo.current
              << "\n";

    std::cout << "ok: env_stat/env_info read\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}