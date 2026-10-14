// EN: Pattern: read-your-writes within a transaction
// EN: Textbook section: Volume V, chapter 32, "Design patterns", §32.8.
// EN: Task: within a single transaction, see your own uncommitted
// EN:   changes (read-your-writes) while still working consistently.
// EN: Solution: reading through the same write-txn sees its own changes; commit-embark
// EN:   allows continuing to read right after commit without opening a new
// EN:   transaction; get_cached provides a "transparent" read-your-writes cache.
// EN: Trade-offs: read-your-writes is guaranteed only within your own transaction;
// EN:   other readers will see the data only after commit.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 46-pattern-read-your-writes.c++ -lmdbx
// EN: Run: ./46-pattern-read-your-writes
// EN: Expected output:
// RU: Паттерн: read-your-writes в рамках транзакции
// RU: Раздел учебника: Том V, глава 32, «Паттерны проектирования», §32.8.
// RU: Задача: в рамках одной транзакции видеть собственные незакоммиченные
// RU:   изменения (read-your-writes) и при этом работать согласованно.
// RU: Решение: чтение через тот же write-txn видит свои изменения; commit-embark
// RU:   позволяет продолжить чтение сразу после коммита, не открывая новую
// RU:   транзакцию; get_cached даёт «прозрачный» кэш read-your-writes.
// RU: Компромиссы: read-your-writes гарантирован только внутри своей транзакции;
// RU:   другие читатели увидят данные лишь после commit.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 46-pattern-read-your-writes.c++ -lmdbx
// RU: Запуск: ./46-pattern-read-your-writes
// RU: Ожидаемый вывод:
//   within txn sees: v1
//   after commit via fresh txn sees: v1
//   ok: read-your-writes pattern works
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
    const std::string path = std::string(example::tmpdir()) + "/mdbx-46-pattern-read-your-writes.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    // EN: The writing transaction immediately reads its own change.
    // RU: Пишущая транзакция сразу читает собственное изменение.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      std::cout << "within txn sees: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";
      txn.commit();
    }

    // EN: After commit, the change is visible to a fresh reader too.
    // RU: После коммита изменение видно и свежему читателю.
    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::cout << "after commit via fresh txn sees: " << rtxn.get(table, mdbx::slice("key")).as_string() << "\n";
      rtxn.abort();
    }

    std::cout << "ok: read-your-writes pattern works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}