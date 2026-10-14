// EN: config-store, slice 05: transactional commands and rollback
// EN: Textbook section: Volume I, chapter 5, "Transactions — first steps", §5.2/§5.4.
// EN: What is added: a grouped operation in one transaction; rollback via abort;
// EN:   read-only transactions see only committed values.
// EN:   Note: a reader and a writer of the same thread cannot overlap
// EN:   (MDBX_TXN_OVERLAPPING); a stable snapshot "on top" of the writer is held by
// EN:   a separate thread — see examples/c++/04-transactions.c++.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-05.c++ -lmdbx
// EN: Run: ./config-store-05
// EN: Expected output:
// RU: config-store, срез 05: транзакционные команды и откат
// RU: Раздел учебника: Том I, глава 5, «Транзакции — первые шаги», §5.2/§5.4.
// RU: Что добавляется: групповая операция в одной транзакции; откат через abort;
// RU:   read-only транзакции видят только закоммиченные значения.
// RU:   Примечание: читатель и писатель одного потока не могут пересекаться
// RU:   (MDBX_TXN_OVERLAPPING); стабильный снапшот «поверх» писателя держит
// RU:   отдельный поток — см. examples/c++/04-transactions.c++.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-05.c++ -lmdbx
// RU: Запуск: ./config-store-05
// RU: Ожидаемый вывод:
//   after first commit: theme = dark
//   committed batch: host = localhost, port = 8080
//   after rollback of second batch: port still = 8080, debug absent
//   ok: config-store-05 works
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
    const std::string path = std::string(example::tmpdir()) + "/config-store-05.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    // EN: A batch of changes committed by one transaction.
    // RU: Пакет изменений, фиксируемый одной транзакцией.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.upsert(table, mdbx::slice("theme"), mdbx::slice("dark"));
      txn.upsert(table, mdbx::slice("host"), mdbx::slice("localhost"));
      txn.upsert(table, mdbx::slice("port"), mdbx::slice("8080"));
      txn.commit();
    }

    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::cout << "after first commit: theme = " << rtxn.get(table, mdbx::slice("theme")).as_string() << "\n";
      rtxn.abort();
    }

    // EN: A batch that will be rolled back (abort): the changes are not saved.
    // RU: Пакет, который будет откачен (abort): изменения не сохраняются.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.upsert(table, mdbx::slice("port"), mdbx::slice("9999"));
      txn.upsert(table, mdbx::slice("debug"), mdbx::slice("on"));
      // EN: rollback
      // RU: откат
      txn.abort();
    }

    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::cout << "committed batch: host = " << rtxn.get(table, mdbx::slice("host")).as_string()
                << ", port = " << rtxn.get(table, mdbx::slice("port")).as_string() << "\n";
      std::cout << "after rollback of second batch: port still = " << rtxn.get(table, mdbx::slice("port")).as_string()
                << ", debug absent\n";
      try {
        (void)rtxn.get(table, mdbx::slice("debug"));
        std::cerr << "FAIL: rolled-back key appeared\n";
        return EXIT_FAILURE;
      } catch (const mdbx::not_found &) {
        // EN: expected
        // RU: ожидаемо
      }
      rtxn.abort();
    }

    std::cout << "ok: config-store-05 works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}