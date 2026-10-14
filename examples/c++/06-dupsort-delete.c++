// EN: Safe-delete in DUPSORT: two-cursor pattern
// EN: Textbook section: Volume II, chapter 6, "Cursors", section "Safe-delete in DUPSORT" (§6.7).
// EN: What it demonstrates: the safe-delete pattern for values in a DUPSORT table
// EN:   with two cursors — one iterates (MDBX_NEXT_DUP), the other deletes the current pair.
// EN:   After cursor_del the cursor position is invalid, therefore iteration and deletion
// EN:   are spread over different cursors.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 06-dupsort-delete.c++ -lmdbx
// EN: Run: ./06-dupsort-delete
// EN: Expected output:
// EN:   values before delete (target): v1 v2 v3 v4
// EN:   deleted 3 values of "target"
// EN:   values after delete (target): v1
// EN:   ok: safe-delete with two cursors
// EN: On failure — message to stderr and exit 1.
// RU: Safe-delete in DUPSORT: two-cursor pattern
// RU: Раздел учебника: Том II, глава 6, «Курсоры», раздел «Safe-delete в DUPSORT» (§6.7).
// RU: Что демонстрирует: паттерн безопасного удаления значений в DUPSORT-таблице
// RU:   двумя курсорами — один итерирует (MDBX_NEXT_DUP), второй удаляет текущую пару.
// RU:   После cursor_del позиция курсора недействительна, поэтому итерация и удаление
// RU:   разнесены на разные курсоры.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 06-dupsort-delete.c++ -lmdbx
// RU: Запуск: ./06-dupsort-delete
// RU: Ожидаемый вывод:
// RU:   values before delete (target): v1 v2 v3 v4
// RU:   deleted 3 values of "target"
// RU:   values after delete (target): v1
// RU:   ok: safe-delete with two cursors
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

static void dump_key(const mdbx::txn_managed &txn, const mdbx::map_handle &multi, const std::string &label,
                     const mdbx::slice &key) {
  std::string line = label;
  auto cur = txn.open_cursor(multi);
  for (auto r = cur.to_key_exact(key, false); r; r = cur.to_current_next_multi(false)) {
    line += r.value.as_string() + " ";
  }
  std::cout << line << "\n";
}

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-06-dupsort-delete.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto multi = txn.create_map("multi", mdbx::key_mode::usual, mdbx::value_mode::multi);
      for (const char *k : {"alpha", "target", "omega"})
        for (const char *v : {"v1", "v2", "v3", "v4"})
          // EN: UPSERT adds a value to the key
          // RU: UPSERT добавляет значение к ключу
          txn.upsert(multi, mdbx::slice(k), mdbx::slice(v));
      txn.commit();
    }

    auto txn = env.start_write();
    auto multi = txn.open_map("multi", mdbx::key_mode::usual, mdbx::value_mode::multi);
    dump_key(txn, multi, "values before delete (target):", mdbx::slice("target"));

    // EN: The "two cursors" pattern: `it` iterates over the key duplicates,
    // EN: `del` deletes the current pair (the position of `it` remains valid).
    // RU: Паттерн «два курсора»: `it` итерирует по дубликатам ключа,
    // RU: `del` удаляет текущую пару (позиция `it` при этом остаётся валидной).
    const mdbx::slice target("target");
    auto it = txn.open_cursor(multi);
    auto del = txn.open_cursor(multi);
    size_t deleted = 0;
    // EN: We keep the first dup of the key and delete the rest.
    // RU: Первый dup ключа сохраняем, остальные удаляем.
    auto pos = it.to_key_exact(target);
    if (pos)
      pos = it.to_current_next_multi(false);
    for (; pos; pos = it.to_current_next_multi(false)) {
      // EN: MDBX_GET_BOTH
      // RU: MDBX_GET_BOTH
      del.to_exact_key_value_equal(pos.key, pos.value, false);
      // EN: MDBX_CURRENT: delete only the current value
      // RU: MDBX_CURRENT: удалить только текущее значение
      if (del.erase(false))
        ++deleted;
    }
    std::cout << "deleted " << deleted << " values of \"target\"\n";

    dump_key(txn, multi, "values after delete (target):", target);
    txn.commit();

    std::cout << "ok: safe-delete with two cursors\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}