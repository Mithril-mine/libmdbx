// EN: Bulk operations: get_batch, bunch_delete, estimate
// EN: Textbook section: Volume IV, chapter 25, "Bulk operations", §25.2–§25.4.
// EN: What it demonstrates: mdbx_cursor_get_batch (reading in batches), erase_bunch
// EN:   (bunch deletion — cutting out pages), cursor.estimate() (estimation of the
// EN:   number of records in a range).
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 32-bulk-ops.c++ -lmdbx
// EN: Run: ./32-bulk-ops
// EN: Expected output:
// EN:   batch reads: 1000 pairs in <N> chunks
// EN:   estimate: ~<M> keys
// EN:   bunch delete removed 500 items
// EN:   ok: get_batch, bunch_delete, estimate work
// EN: On failure — message to stderr and exit 1.
// RU: Массовые операции: get_batch, bunch_delete, estimate
// RU: Раздел учебника: Том IV, глава 25, «Массовые операции», §25.2–§25.4.
// RU: Что демонстрирует: mdbx_cursor_get_batch (чтение пачками), erase_bunch
// RU:   (удаление «гроздьями» — вырезание страниц), cursor.estimate() (оценка
// RU:   количества записей в диапазоне).
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 32-bulk-ops.c++ -lmdbx
// RU: Запуск: ./32-bulk-ops
// RU: Ожидаемый вывод:
// RU:   batch reads: 1000 pairs in <N> chunks
// RU:   estimate: ~<M> keys
// RU:   bunch delete removed 500 items
// RU:   ok: get_batch, bunch_delete, estimate work
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-32-bulk-ops.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 1000; ++i)
        txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("value"));
      txn.commit();
    }

    auto txn = env.start_write();
    auto table = txn.open_map(nullptr);
    auto cur = txn.open_cursor(table);

    // EN: Batch reading: up to 100 pairs per call.
    // RU: Пакетное чтение: до 100 пар за вызов.
    size_t pairs = 0, chunks = 0;
    cur.to_first();
    bool is_last = false;
    while (!is_last) {
      auto batch = cur.get_batch(100, mdbx::cursor::move_operation::next, &is_last);
      pairs += batch.size();
      ++chunks;
    }
    std::cout << "batch reads: " << pairs << " pairs in " << chunks << " chunks\n";

    // EN: Estimating the number of records (approximately).
    // RU: Оценка количества записей (приблизительно).
    cur.to_first();
    auto est = cur.estimate(mdbx::cursor::move_operation::last);
    std::cout << "estimate: ~" << est.approximate_quantity << " keys\n";

    // EN: Bunch deletion: everything from the current position to the end.
    // RU: Удаление «гроздьями»: всё от текущей позиции до конца.
    cur.to_key_exact(mdbx::slice("k500"));
    const size_t removed = cur.erase_bunch(mdbx::cursor::bunch_delete::delete_after_including);
    std::cout << "bunch delete removed " << removed << " items\n";

    txn.commit();

    if (pairs != 1000 || removed < 500) {
      std::cerr << "FAIL: unexpected bulk results\n";
      return EXIT_FAILURE;
    }

    std::cout << "ok: get_batch, bunch_delete, estimate work\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}