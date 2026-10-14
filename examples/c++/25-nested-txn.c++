// EN: Nested transactions: join/undo, fate of tables
// EN: Textbook section: Volume III, chapter 18, "Nested transactions", §18.2/§18.3.
// EN: What it demonstrates: a nested transaction (start_nested): abort rolls back its
// EN:   changes (undo); commit folds them into the parent (join); a table created in a
// EN:   nested transaction and then rolled back disappears.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 25-nested-txn.c++ -lmdbx
// EN: Run: ./25-nested-txn
// EN: Expected output:
// EN:   parent sees k1 after nested abort: v1
// EN:   k2 rolled back by nested abort: absent
// EN:   tmp table dropped with nested abort: absent
// EN:   k3 joined by nested commit: v3
// EN:   ok: nested transactions join/abort; table fate honored
// EN: On failure — message to stderr and exit 1.
// RU: Вложенные транзакции: join/undo, судьба таблиц
// RU: Раздел учебника: Том III, глава 18, «Вложенные транзакции», §18.2/§18.3.
// RU: Что демонстрирует: вложенную транзакцию (start_nested): abort откатывает её
// RU:   изменения (undo); commit вкладывает их в родителя (join); таблица,
// RU:   созданная во вложенной транзакции и откаченная, исчезает.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 25-nested-txn.c++ -lmdbx
// RU: Запуск: ./25-nested-txn
// RU: Ожидаемый вывод:
// RU:   parent sees k1 after nested abort: v1
// RU:   k2 rolled back by nested abort: absent
// RU:   tmp table dropped with nested abort: absent
// RU:   k3 joined by nested commit: v3
// RU:   ok: nested transactions join/abort; table fate honored
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-25-nested-txn.mdbx";
    mdbx::env::remove(path);

    // EN: Nested transactions are enabled by an environment option.
    // RU: Вложенные транзакции включаются опцией окружения.
    mdbx::env::operate_parameters params;
    params.options.nested_transactions = true;
    mdbx::env_managed env = example::env_open(path, params);

    auto txn = env.start_write();
    auto table = txn.open_map(nullptr);
    txn.insert(table, mdbx::slice("k1"), mdbx::slice("v1"));

    // EN: A nested transaction with a rollback (undo).
    // RU: Вложенная транзакция с откатом (undo).
    {
      auto nested = txn.start_nested();
      auto ntable = nested.open_map(nullptr);
      nested.insert(ntable, mdbx::slice("k2"), mdbx::slice("v2"));
      // EN: rollback of the nested transaction
      // RU: откат вложенной
      nested.abort();
    }
    std::cout << "parent sees k1 after nested abort: " << txn.get(table, mdbx::slice("k1")).as_string() << "\n";
    try {
      (void)txn.get(table, mdbx::slice("k2"));
      std::cerr << "FAIL: k2 survived nested abort\n";
      return EXIT_FAILURE;
    } catch (const mdbx::not_found &) {
      std::cout << "k2 rolled back by nested abort: absent\n";
    }

    // EN: A table created in a nested transaction that was then rolled back disappears.
    // RU: Таблица, созданная во вложенной транзакции и откаченной, исчезает.
    {
      auto nested = txn.start_nested();
      auto tmp = nested.create_map("tmp-table", mdbx::key_mode::usual, mdbx::value_mode::single);
      nested.insert(tmp, mdbx::slice("x"), mdbx::slice("y"));
      nested.abort();
    }
    try {
      (void)txn.open_map("tmp-table");
      std::cerr << "FAIL: tmp table survived nested abort\n";
      return EXIT_FAILURE;
    } catch (const mdbx::not_found &) {
      std::cout << "tmp table dropped with nested abort: absent\n";
    }

    // EN: A nested transaction with commit — the changes go into the parent (join).
    // RU: Вложенная транзакция с commit — изменения входят в родителя (join).
    {
      auto nested = txn.start_nested();
      auto ntable = nested.open_map(nullptr);
      nested.insert(ntable, mdbx::slice("k3"), mdbx::slice("v3"));
      nested.commit();
    }
    std::cout << "k3 joined by nested commit: " << txn.get(table, mdbx::slice("k3")).as_string() << "\n";

    txn.commit();

    std::cout << "ok: nested transactions join/abort; table fate honored\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}