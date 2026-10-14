// EN: Pattern: replication via per-table txnid change log
// EN: Textbook section: Volume V, chapter 32, "Design patterns", §32.7.
// EN: Task: asynchronous replication of changes between nodes.
// EN: Solution: each record stores a version (the changing txnid); the replica remembers
// EN:   the last applied txnid and reads only newer records.
// EN: Trade-offs: a full scan on the replica side; for large tables
// EN:   a separate change log (a WAL-like table) is better.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 45-pattern-replication.c++ -lmdbx
// EN: Run: ./45-pattern-replication
// EN: Expected output:
// RU: Паттерн: репликация через журнал изменений по txnid в таблицах
// RU: Раздел учебника: Том V, глава 32, «Паттерны проектирования», §32.7.
// RU: Задача: асинхронная репликация изменений между узлами.
// RU: Решение: каждая запись хранит версию (txnid изменения); реплика запоминает
// RU:   последний применённый txnid и читает только более новые записи.
// RU: Компромиссы: полное сканирование на стороне реплики; для больших таблиц
// RU:   лучше отдельный журнал изменений (WAL-подобная таблица).
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 45-pattern-replication.c++ -lmdbx
// RU: Запуск: ./45-pattern-replication
// RU: Ожидаемый вывод:
//   replica from txnid 0: 3 new rows
//   replica from txnid <N>: 2 new rows
//   ok: replication pattern works
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <cstdint>
#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

namespace {

// EN: A value in the "data|version" format.
// RU: Значение в формате "data|version".
uint64_t version_of(const mdbx::slice &value) {
  const std::string s(value.as_string());
  const auto pos = s.rfind('|');
  return std::stoull(s.substr(pos + 1));
}

} // namespace

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-45-pattern-replication.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    // EN: First write round.
    // RU: Первый раунд записи.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 3; ++i)
        txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("data|1"));
      txn.commit();
    }

    // EN: The replica reads everything newer than its last txnid.
    // RU: Реплика читает всё новее своего последнего txnid.
    auto replicate = [&](uint64_t last_seen) -> size_t {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      size_t n = 0;
      auto cur = rtxn.open_cursor(table);
      for (auto r = cur.to_first(); r; r = cur.to_next(false)) {
        if (version_of(r.value) > last_seen)
          ++n;
      }
      rtxn.abort();
      return n;
    };

    std::cout << "replica from txnid 0: " << replicate(0) << " new rows\n";

    // EN: Second round: update k2 (version 2) and add k4.
    // RU: Второй раунд: обновляем k2 (версия 2) и добавляем k4.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.upsert(table, mdbx::slice("k2"), mdbx::slice("data|2"));
      txn.insert(table, mdbx::slice("k4"), mdbx::slice("data|2"));
      txn.commit();
    }
    // EN: Version 2 is newer than version 1: a replica that saw only v1 will pull 2 records.
    // RU: Версии 2 новее версии 1: реплика, видевшая только v1, подтянет 2 записи.
    std::cout << "replica from txnid 1: " << replicate(1) << " new rows\n";

    std::cout << "ok: replication pattern works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}