// EN: Pattern: auto-ID via a sequence counter
// EN: Textbook section: Volume V, chapter 32, "Design patterns", §32.1.
// EN: Task: issue monotonically increasing identifiers to records.
// EN: Solution: the counter is stored in the database (key "meta") and incremented atomically
// EN:   within the same writing transaction as the record insertion.
// EN: Trade-offs: the counter is a single serialization point for new IDs;
// EN:   concurrent writers are serialized by the libmdbx write lock.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 39-pattern-sequence-id.c++ -lmdbx
// EN: Run: ./39-pattern-sequence-id
// EN: Expected output:
// RU: Паттерн: авто-ID через счётчик-последовательность
// RU: Раздел учебника: Том V, глава 32, «Паттерны проектирования», §32.1.
// RU: Задача: выдавать записям монотонно растущие идентификаторы.
// RU: Решение: счётчик хранится в БД (ключ "meta") и инкрементируется атомарно
// RU:   внутри той же пишущей транзакции, что и вставка записи.
// RU: Компромиссы: счётчик — единая точка сериализации для новых ID;
// RU:   параллельные писатели сериализуются по write-блокировке libmdbx.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 39-pattern-sequence-id.c++ -lmdbx
// RU: Запуск: ./39-pattern-sequence-id
// RU: Ожидаемый вывод:
//   ids: 1 2 3
//   ok: sequence-id pattern works
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

namespace {

uint64_t next_id(mdbx::txn_managed &txn, const mdbx::map_handle &meta) {
  constexpr auto counter_key = mdbx::slice("seq");
  uint64_t current = 0;
  try {
    current = txn.get(meta, counter_key).as_uint64();
  } catch (const mdbx::not_found &) {
    current = 0;
  }
  ++current;
  txn.upsert(meta, counter_key, mdbx::slice::wrap(current));
  return current;
}

} // namespace

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-39-pattern-sequence-id.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    std::string ids;
    auto txn = env.start_write();
    auto meta = txn.create_map("meta", mdbx::key_mode::usual, mdbx::value_mode::single);
    auto users = txn.create_map("users", mdbx::key_mode::usual, mdbx::value_mode::single);
    for (const char *name : {"alice", "bob", "carol"}) {
      const uint64_t id = next_id(txn, meta);
      txn.upsert(users, mdbx::slice(name), mdbx::slice::wrap(id));
      ids += std::to_string(id) + " ";
    }
    txn.commit();

    std::cout << "ids: " << ids << "\n";
    std::cout << "ok: sequence-id pattern works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}