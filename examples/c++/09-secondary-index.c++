// EN: Secondary index: index + main table in one transaction
// EN: Textbook section: Volume II, chapter 8, "Secondary indexes", section "Maintaining consistency".
// EN: What it demonstrates: the main "users" table and a secondary index
// EN:   "role → list of IDs"; updating both tables in one transaction;
// EN:   deleting a record together with its index references; a query via the index.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 09-secondary-index.c++ -lmdbx
// EN: Run: ./09-secondary-index
// EN: Expected output:
// EN:   admins via index: user1 user3
// EN:   ok: secondary index maintained in one txn
// EN: On failure — message to stderr and exit 1.
// RU: Secondary index: index + main table in one transaction
// RU: Раздел учебника: Том II, глава 8, «Вторичные индексы», раздел «Поддержание консистентности».
// RU: Что демонстрирует: основную таблицу «пользователи» и вторичный индекс
// RU:   «роль → список ID»; обновление обеих таблиц в одной транзакции;
// RU:   удаление записи вместе с её индексными ссылками; запрос через индекс.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 09-secondary-index.c++ -lmdbx
// RU: Запуск: ./09-secondary-index
// RU: Ожидаемый вывод:
// RU:   admins via index: user1 user3
// RU:   ok: secondary index maintained in one txn
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-09-secondary-index.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto users = txn.create_map("users", mdbx::key_mode::usual, mdbx::value_mode::single);
      // EN: The index: key is a role, values are user IDs (multi-values).
      // RU: Индекс: ключ — роль, значения — ID пользователей (мультизначения).
      auto by_role = txn.create_map("by_role", mdbx::key_mode::usual, mdbx::value_mode::multi);

      // EN: Inserting users and maintaining the index — in one transaction.
      // RU: Вставка пользователей и поддержание индекса — в одной транзакции.
      txn.insert(users, mdbx::slice("user1"), mdbx::slice("Alice|admin"));
      txn.upsert(by_role, mdbx::slice("admin"), mdbx::slice("user1"));
      txn.insert(users, mdbx::slice("user2"), mdbx::slice("Bob|dev"));
      txn.upsert(by_role, mdbx::slice("dev"), mdbx::slice("user2"));
      txn.insert(users, mdbx::slice("user3"), mdbx::slice("Carol|admin"));
      txn.upsert(by_role, mdbx::slice("admin"), mdbx::slice("user3"));
      txn.commit();
    }

    // EN: Deleting a user together with his record in the index.
    // RU: Удаление пользователя вместе с его записью в индексе.
    {
      auto txn = env.start_write();
      auto users = txn.open_map("users", mdbx::key_mode::usual, mdbx::value_mode::single);
      auto by_role = txn.open_map("by_role", mdbx::key_mode::usual, mdbx::value_mode::multi);
      txn.erase(users, mdbx::slice("user2"));
      // EN: the value of a specific key
      // RU: значение конкретного ключа
      txn.erase(by_role, mdbx::slice("dev"), mdbx::slice("user2"));
      txn.commit();
    }

    // EN: The "all admins" query via the index.
    // RU: Запрос «все админы» через индекс.
    auto rtxn = env.start_read();
    auto by_role = rtxn.open_map("by_role", mdbx::key_mode::usual, mdbx::value_mode::multi);
    auto cur = rtxn.open_cursor(by_role);
    std::string admins;
    for (auto r = cur.to_key_exact(mdbx::slice("admin")); r; r = cur.to_current_next_multi(false))
      admins += r.value.as_string() + " ";
    std::cout << "admins via index: " << admins << "\n";
    rtxn.abort();

    std::cout << "ok: secondary index maintained in one txn\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}