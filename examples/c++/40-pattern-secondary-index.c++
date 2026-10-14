// EN: Pattern: secondary index «field → list of IDs»
// EN: Textbook section: Volume V, chapter 32, "Design patterns", §32.2.
// EN: Task: fast lookup of records by a field value (not the primary key).
// EN: Solution: a separate index table "field value → list of IDs" on top of
// EN:   DUPSORT (based on the inverted index from S08), updated in the same transaction.
// EN: Trade-offs: a doubled write per insert/delete; the index must be
// EN:   maintained manually and consistently with the main table.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 40-pattern-secondary-index.c++ -lmdbx
// EN: Run: ./40-pattern-secondary-index
// EN: Expected output:
// RU: Паттерн: вторичный индекс «поле → список ID»
// RU: Раздел учебника: Том V, глава 32, «Паттерны проектирования», §32.2.
// RU: Задача: быстрый поиск записей по значению поля (не первичному ключу).
// RU: Решение: отдельная таблица-индекс «значение поля → список ID» поверх
// RU:   DUPSORT (на основе inverted-index из S08), обновляемая в той же транзакции.
// RU: Компромиссы: удвоение записи на каждую вставку/удаление; индекс нужно
// RU:   поддерживать вручную и согласованно с основной таблицей.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 40-pattern-secondary-index.c++ -lmdbx
// RU: Запуск: ./40-pattern-secondary-index
// RU: Ожидаемый вывод:
//   users with role=admin: 1 3
//   ok: secondary-index pattern works
// EN: On failure — message to stderr and exit 1.
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
    const std::string path = std::string(example::tmpdir()) + "/mdbx-40-pattern-secondary-index.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    auto txn = env.start_write();
    // EN: Main table: id → {name, role}.
    // RU: Основная таблица: id → {name, role}.
    auto users = txn.create_map("users", mdbx::key_mode::ordinal, mdbx::value_mode::single);
    // EN: Index: role → list of ids (multi-values).
    // RU: Индекс: role → список id (мультизначения).
    auto by_role = txn.create_map("by_role", mdbx::key_mode::usual, mdbx::value_mode::multi);

    struct rec {
      uint64_t id;
      const char *name;
      const char *role;
    };
    static const rec records[] = {{1, "alice", "admin"}, {2, "bob", "dev"}, {3, "carol", "admin"}};
    for (const auto &r : records) {
      txn.upsert(users, mdbx::slice::wrap(r.id), mdbx::slice(std::string(r.name) + "|" + r.role));
      txn.upsert(by_role, mdbx::slice(r.role), mdbx::slice::wrap(r.id));
    }
    txn.commit();

    auto rtxn = env.start_read();
    auto index = rtxn.open_map("by_role", mdbx::key_mode::usual, mdbx::value_mode::multi);
    std::string admins;
    auto cur = rtxn.open_cursor(index);
    for (auto r = cur.to_key_exact(mdbx::slice("admin")); r; r = cur.to_current_next_multi(false))
      admins += std::to_string(r.value.as_uint64()) + " ";
    std::cout << "users with role=admin: " << admins << "\n";
    rtxn.abort();

    std::cout << "ok: secondary-index pattern works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}