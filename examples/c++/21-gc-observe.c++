// EN: GC observation: newly/cow pages, FIFO vs LIFO reclaim
// EN: Textbook section: Volume III, chapter 16, "GC", §16.5 "FIFO vs LIFO".
// EN: What it demonstrates: an insert+delete+commit cycle and page statistics
// EN:   (newly/cow via mi_pgop_stat); comparison of file growth under FIFO and LIFO
// EN:   reclaim (MDBX_LIFORECLAIM).
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 21-gc-observe.c++ -lmdbx
// EN: Run: ./21-gc-observe
// EN: Expected output (numbers depend on the system):
// EN:   pgop after churn: newly=<N> cow=<M>
// EN:   fifo size = <S>, lifo size = <S2>
// EN:   ok: GC pages observed; LIFO reclaim toggled
// EN: On failure — message to stderr and exit 1.
// RU: Наблюдение за GC: страницы newly/cow, рекламация FIFO vs LIFO
// RU: Раздел учебника: Том III, глава 16, «GC», §16.5 «FIFO vs LIFO».
// RU: Что демонстрирует: цикл «вставка+удаление+коммит» и статистику страниц
// RU:   (newly/cow через mi_pgop_stat); сравнение роста файла при FIFO и LIFO
// RU:   рекламации (MDBX_LIFORECLAIM).
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 21-gc-observe.c++ -lmdbx
// RU: Запуск: ./21-gc-observe
// RU: Ожидаемый вывод (числа зависят от системы):
// RU:   pgop after churn: newly=<N> cow=<M>
// RU:   fifo size = <S>, lifo size = <S2>
// RU:   ok: GC pages observed; LIFO reclaim toggled
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <cstdint>
#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

static uint64_t file_size(const mdbx::env_managed &env) { return env.get_info().mi_dxb_fsize; }

static void churn(mdbx::env_managed &env) {
  for (int round = 0; round < 10; ++round) {
    auto txn = env.start_write();
    auto table = txn.open_map(nullptr);
    for (int i = 0; i < 500; ++i) {
      const auto key = "k" + std::to_string(round * 1000 + i);
      txn.insert(table, mdbx::slice(key), mdbx::slice("payload"));
    }
    txn.commit();
    auto del = env.start_write();
    auto dtable = del.open_map(nullptr);
    for (int i = 0; i < 500; ++i) {
      const auto key = "k" + std::to_string(round * 1000 + i);
      del.erase(dtable, mdbx::slice(key));
    }
    del.commit();
  }
}

static uint64_t run_churn(const std::string &path, mdbx::env::operate_parameters params) {
  mdbx::env_managed env(path, mdbx::env_managed::create_parameters(), params);
  churn(env);
  return file_size(env);
}

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-21-gc-observe.mdbx";
    mdbx::env::remove(path);

    // EN: FIFO (default): GC pages are reused in the order of release.
    // RU: FIFO (по умолчанию): страницы GC переиспользуются в порядке освобождения.
    {
      mdbx::env_managed env = example::env_open(path);
      churn(env);
      const auto info = env.get_info();
      std::cout << "pgop after churn: newly=" << info.mi_pgop_stat.newly << " cow=" << info.mi_pgop_stat.cow << "\n";
    }
    const uint64_t fifo_size = run_churn(path, mdbx::env::operate_parameters());

    // EN: LIFO: the most recently released pages are used first.
    // RU: LIFO: свежеосвобождённые страницы используются в первую очередь.
    mdbx::env::remove(path);
    mdbx::env::operate_parameters params;
    // EN: MDBX_LIFORECLAIM
    // RU: MDBX_LIFORECLAIM
    params.set_reclaiming(mdbx::env::reclaiming_options().set_lifo(true));
    const uint64_t lifo_size = run_churn(path, params);
    std::cout << "fifo size = " << fifo_size << ", lifo size = " << lifo_size << "\n";

    std::cout << "ok: GC pages observed; LIFO reclaim toggled\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}