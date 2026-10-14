// EN: Defragmentation: mdbx_env_defrag
// EN: Textbook section: Volume III, chapter 17, "Growth, shrinking and defragmentation", §17.5 "mdbx_env_defrag".
// EN: What it demonstrates: accumulation of "tail" garbage on overwrites; calling
// EN:   mdbx_env_defrag() and the reduction of the file size.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 24-defrag.c++ -lmdbx
// EN: Run: ./24-defrag
// EN: Expected output (numbers depend on the system):
// EN:   size before = <S1>
// EN:   defrag: pages_shrinked=<N> moved=<M>
// EN:   size after  = <S2> (<= <S1>)
// EN:   ok: defrag shrunk the database file
// EN: On failure — message to stderr and exit 1.
// RU: Дефрагментация: mdbx_env_defrag
// RU: Раздел учебника: Том III, глава 17, «Рост, сжатие и дефрагментация», §17.5 «mdbx_env_defrag».
// RU: Что демонстрирует: накопление «хвостового» мусора при перезаписях; вызов
// RU:   mdbx_env_defrag() и уменьшение размера файла.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 24-defrag.c++ -lmdbx
// RU: Запуск: ./24-defrag
// RU: Ожидаемый вывод (числа зависят от системы):
// RU:   size before = <S1>
// RU:   defrag: pages_shrinked=<N> moved=<M>
// RU:   size after  = <S2> (<= <S1>)
// RU:   ok: defrag shrunk the database file
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-24-defrag.mdbx";
    mdbx::env::remove(path);

    // EN: Explicit geometry: the upper bound is set to be certainly sufficient for
    // EN: the workload (hundreds of KB of data), but not dependent on the system
    // EN: default size (on 32-bit platforms it may turn out to be too small).
    // RU: Явная геометрия: верхнюю границу задаём заведомо достаточной для
    // RU: нагрузки (сотни КБ данных), но не зависящей от системного размера по
    // RU: умолчанию (на 32-битных платформах он может оказаться малым).
    mdbx::env::geometry geo;
    geo.make_dynamic(1 * mdbx::env::geometry::MiB, 64 * mdbx::env::geometry::MiB);
    mdbx::env_managed env = example::env_open(path, geo);

    // EN: First pass: fill with large values.
    // RU: Первый проход: заполняем большими значениями.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 2000; ++i) {
        const auto key = "k" + std::to_string(i);
        const std::string value(4096, 'a');
        txn.insert(table, mdbx::slice(key), mdbx::slice(value));
      }
      txn.commit();
    }
    // EN: Second pass: overwrite with smaller values and delete half of the keys —
    // EN: "tail" pages are formed at the end of the file.
    // RU: Второй проход: перезаписываем меньшими значениями и удаляем половину —
    // RU: образуются «хвостовые» страницы в конце файла.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 1000; ++i) {
        const auto key = "k" + std::to_string(i);
        txn.upsert(table, mdbx::slice(key), mdbx::slice("small"));
      }
      for (int i = 1000; i < 2000; ++i)
        txn.erase(table, mdbx::slice("k" + std::to_string(i)));
      txn.commit();
    }

    const uint64_t size_before = env.get_info().mi_dxb_fsize;
    std::cout << "size before = " << size_before << "\n";

    // EN: Defragmentation: time_atleast in 1/65536 fractions of a second.
    // RU: Дефрагментация: time_atleast в 1/65536 долях секунды.
    MDBX_defrag_result result;
    const int rc = mdbx_env_defrag(env.handle(), /*defrag_atleast=*/0, /*time_atleast_dot16=*/65536,
                                   /*defrag_enough=*/0, /*time_limit_dot16=*/655360,
                                   /*acceptable_backlash=*/-1, /*preferred_batch=*/-1,
                                   /*progress_callback=*/nullptr, /*ctx=*/nullptr, &result);
    if (rc != MDBX_SUCCESS && rc != MDBX_RESULT_TRUE) {
      std::cerr << "mdbx_env_defrag: " << mdbx_strerror(rc) << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "defrag: pages_shrinked=" << result.pages_shrinked << " moved=" << result.pages_moved << "\n";

    const uint64_t size_after = env.get_info().mi_dxb_fsize;
    std::cout << "size after  = " << size_after << "\n";

    if (size_after > size_before) {
      std::cerr << "FAIL: file did not shrink\n";
      return EXIT_FAILURE;
    }

    std::cout << "ok: defrag shrunk the database file\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}