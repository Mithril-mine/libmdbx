// EN: Tree height and page types via env_stat
// EN: Textbook section: Volume III, chapter 13, "Storage architecture", §13.4 "Tree assessment".
// EN: What it demonstrates: mdbx_env_stat_ex(): leaf/branch/overflow pages, the height
// EN:   of the B-tree; writing a value larger than the page size creates overflow pages.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 18-tree-height.c++ -lmdbx
// EN: Run: ./18-tree-height
// EN: Expected output (numbers depend on the data):
// EN:   stat: depth=<N> leaf=<N> branch=<N> overflow=<N> entries=5001
// EN:   ok: tree height and page types observed; overflow value detected
// EN: On failure — message to stderr and exit 1.
// RU: Высота дерева и типы страниц через env_stat
// RU: Раздел учебника: Том III, глава 13, «Архитектура хранения», §13.4 «Оценка дерева».
// RU: Что демонстрирует: mdbx_env_stat_ex(): страницы leaf/branch/overflow, высоту
// RU:   B-дерева; запись значения больше размера страницы создаёт overflow-страницы.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 18-tree-height.c++ -lmdbx
// RU: Запуск: ./18-tree-height
// RU: Ожидаемый вывод (числа зависят от данных):
// RU:   stat: depth=<N> leaf=<N> branch=<N> overflow=<N> entries=5001
// RU:   ok: tree height and page types observed; overflow value detected
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-18-tree-height.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 5000; ++i)
        txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("v"));
      // EN: A value larger than the page size (4096) goes to overflow pages.
      // RU: Значение больше размера страницы (4096) уходит на overflow-страницы.
      const std::string big(64 * 1024, 'x');
      txn.insert(table, mdbx::slice("big"), mdbx::slice(big));
      txn.commit();
    }

    // EN: mdbx_env_stat_ex()
    // RU: mdbx_env_stat_ex()
    const auto stat = env.get_stat();
    std::cout << "stat: depth=" << stat.ms_depth << " leaf=" << stat.ms_leaf_pages
              << " branch=" << stat.ms_branch_pages << " overflow=" << stat.ms_overflow_pages
              << " entries=" << stat.ms_entries << "\n";

    if (stat.ms_overflow_pages == 0) {
      std::cerr << "FAIL: expected overflow pages for a large value\n";
      return EXIT_FAILURE;
    }
    if (stat.ms_entries < 5001) {
      std::cerr << "FAIL: unexpected entry count\n";
      return EXIT_FAILURE;
    }

    std::cout << "ok: tree height and page types observed; overflow value detected\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}