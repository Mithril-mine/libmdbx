// EN: Get-cached: transparent read-your-writes cache statuses
// EN: Textbook section: Volume IV, chapter 24, "Cache lookup (get-cached)", §24.2 "Statuses".
// EN: What it demonstrates: txn.get_cached() with the statuses REFRESHED/HIT/CONFIRMED/DIRTY;
// EN:   the statuses BEHIND/UNABLE/RACE arise in multithreaded and ABA scenarios.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 31-get-cached.c++ -lmdbx
// EN: Run: ./31-get-cached
// EN: Expected output:
// EN:   read1: REFRESHED value=v1
// EN:   read2: HIT value=v1
// EN:   after commit v2: CONFIRMED/REFRESHED value=v2
// EN:   dirty write: DIRTY value=v3
// EN:   ok: get-cached statuses observed
// EN: On failure — message to stderr and exit 1.
// RU: Get-cached: прозрачные статусы кэша read-your-writes
// RU: Раздел учебника: Том IV, глава 24, «Кэш-поиск (get-cached)», §24.2 «Статусы».
// RU: Что демонстрирует: txn.get_cached() со статусами REFRESHED/HIT/CONFIRMED/DIRTY;
// RU:   статусы BEHIND/UNABLE/RACE возникают в многопоточных и ABA-сценариях.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 31-get-cached.c++ -lmdbx
// RU: Запуск: ./31-get-cached
// RU: Ожидаемый вывод:
// RU:   read1: REFRESHED value=v1
// RU:   read2: HIT value=v1
// RU:   after commit v2: CONFIRMED/REFRESHED value=v2
// RU:   dirty write: DIRTY value=v3
// RU:   ok: get-cached statuses observed
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

namespace {

const char *status_name(int status) {
  switch (status) {
  case MDBX_CACHE_ERROR:
    return "ERROR";
  case MDBX_CACHE_BEHIND:
    return "BEHIND";
  case MDBX_CACHE_UNABLE:
    return "UNABLE";
  case MDBX_CACHE_RACE:
    return "RACE";
  case MDBX_CACHE_DIRTY:
    return "DIRTY";
  case MDBX_CACHE_HIT:
    return "HIT";
  case MDBX_CACHE_CONFIRMED:
    return "CONFIRMED";
  case MDBX_CACHE_REFRESHED:
    return "REFRESHED";
  default:
    return "?";
  }
}

} // namespace

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-31-get-cached.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      txn.commit();
    }

    // EN: A cache entry survives transactions; the constructor calls reset().
    // RU: Кэш-запись переживает транзакции; конструктор вызывает reset().
    mdbx::cache_entry entry;

    // EN: The first call reads the data from the DB and fills the cache.
    // RU: Первый вызов читает данные из БД и заполняет кэш.
    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      mdbx::txn::cache_status status;
      const auto value = rtxn.get_cached(table, mdbx::slice("key"), entry, &status);
      std::cout << "read1: " << status_name(int(status)) << " value=" << value.as_string() << "\n";
      // EN: A repeated call in the same snapshot — a cache hit.
      // RU: Повторный вызов в том же снапшоте — попадание в кэш.
      mdbx::txn::cache_status status2;
      const auto value2 = rtxn.get_cached(table, mdbx::slice("key"), entry, &status2);
      std::cout << "read2: " << status_name(int(status2)) << " value=" << value2.as_string() << "\n";
      rtxn.abort();
    }

    // EN: After committing a new value, the cache is refreshed to the current state.
    // RU: После коммита нового значения кэш обновляется до актуального состояния.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.upsert(table, mdbx::slice("key"), mdbx::slice("v2"));
      txn.commit();
    }
    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      mdbx::txn::cache_status status;
      const auto value = rtxn.get_cached(table, mdbx::slice("key"), entry, &status);
      std::cout << "after commit v2: " << status_name(int(status)) << " value=" << value.as_string() << "\n";
      rtxn.abort();
    }

    // EN: Uncommitted changes inside a write transaction — the DIRTY status.
    // RU: Неподтверждённые изменения внутри пишущей транзакции — статус DIRTY.
    {
      auto wtxn = env.start_write();
      auto table = wtxn.open_map(nullptr);
      wtxn.upsert(table, mdbx::slice("key"), mdbx::slice("v3"));
      mdbx::txn::cache_status status;
      const auto value = wtxn.get_cached(table, mdbx::slice("key"), entry, &status);
      std::cout << "dirty write: " << status_name(int(status)) << " value=" << value.as_string() << "\n";
      wtxn.abort();
    }

    std::cout << "ok: get-cached statuses observed\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}