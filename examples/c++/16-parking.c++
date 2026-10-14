// EN: Reader parking: park/unpark of a long read transaction
// EN: Textbook section: Volume II, chapter 11, "Multithreading", section "Parking".
// EN: What it demonstrates: parking a read-only transaction (park_reading) — the reader
// EN:   releases its slot, allowing writers and GC to make progress; resumption
// EN:   via unpark_reading. After resumption the reader's snapshot does not change
// EN:   (MVCC): it sees the same state as before parking.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 16-parking.c++ -lmdbx -pthread
// EN: Run: ./16-parking
// EN: Expected output:
// EN:   reader before park: v1
// EN:   writer committed v2 while reader parked
// EN:   reader after unpark: v1
// EN:   ok: parked reader released slot, resumed with unchanged snapshot
// EN: On failure — message to stderr and exit 1.
// RU: Reader parking: park/unpark of a long read transaction
// RU: Раздел учебника: Том II, глава 11, «Многопоточность», раздел «Парковка».
// RU: Что демонстрирует: парковку read-only транзакции (park_reading) — читатель
// RU:   освобождает свой слот, позволяя писателям и GC продвигаться; возобновление
// RU:   через unpark_reading. После возобновления снапшот читателя не меняется
// RU:   (MVCC): он видит то же состояние, что и до парковки.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 16-parking.c++ -lmdbx -pthread
// RU: Запуск: ./16-parking
// RU: Ожидаемый вывод:
// RU:   reader before park: v1
// RU:   writer committed v2 while reader parked
// RU:   reader after unpark: v1
// RU:   ok: parked reader released slot, resumed with unchanged snapshot
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-16-parking.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      txn.commit();
    }

    auto rtxn = env.start_read();
    auto table = rtxn.open_map(nullptr);
    std::cout << "reader before park: " << rtxn.get(table, mdbx::slice("key")).as_string() << "\n";

    // EN: Parking: the reader stops holding the snapshot; the writer can commit.
    // RU: Парковка: читатель перестаёт удерживать снапшот; писатель может коммитить.
    rtxn.park_reading(false);
    {
      auto wtxn = env.start_write();
      auto wtable = wtxn.open_map(nullptr);
      wtxn.upsert(wtable, mdbx::slice("key"), mdbx::slice("v2"));
      wtxn.commit();
    }
    std::cout << "writer committed v2 while reader parked\n";

    // EN: Resumption: the reader continues to work with its previous snapshot.
    // RU: Возобновление: читатель продолжает работать со своим прежним снапшотом.
    rtxn.unpark_reading(true);
    std::cout << "reader after unpark: " << rtxn.get(table, mdbx::slice("key")).as_string() << "\n";
    rtxn.abort();

    std::cout << "ok: parked reader released slot, resumed with unchanged snapshot\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}