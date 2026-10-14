// EN: Diagnostics: txn_info, reader list, commit latency; mdbx_chk
// EN: Textbook section: Volume V, chapter 30, "Diagnostics", §30.2 "Tools", §30.4 "Scenarios".
// EN: What it demonstrates: a helper dumper — mdbx_txn_info (id/lag/space),
// EN:   mdbx_reader_list (reader slots), parsing of commit_latency; instructions
// EN:   for running mdbx_chk to verify integrity.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 37-diagnostics.c++ -lmdbx
// EN: Run: ./37-diagnostics
// EN: Expected output:
// RU: Диагностика: txn_info, список читателей, латенция коммита; mdbx_chk
// RU: Раздел учебника: Том V, глава 30, «Диагностика», §30.2 «Инструменты», §30.4 «Сценарии».
// RU: Что демонстрирует: хелпер-даммер — mdbx_txn_info (id/lag/space),
// RU:   mdbx_reader_list (слоты читателей), разбор commit_latency; инструкция
// RU:   вызова mdbx_chk для проверки целостности.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 37-diagnostics.c++ -lmdbx
// RU: Запуск: ./37-diagnostics
// RU: Ожидаемый вывод:
//   txn_info: id=<N> lag=<L> used=<U> dirty=<D>
//   ok: diagnostics dumpers work
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
    const std::string path = std::string(example::tmpdir()) + "/mdbx-37-diagnostics.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 100; ++i)
        txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("v"));

      // EN: Information about the current writing transaction.
      // RU: Информация о текущей пишущей транзакции.
      const auto info = txn.get_info(true /* scan_rlt */);
      std::cout << "txn_info: id=" << info.txn_id << " lag=" << info.txn_reader_lag << " used=" << info.txn_space_used
                << " dirty=" << info.txn_space_dirty << "\n";
      txn.commit();
    }

    // EN: The reader list (RLT).
    // RU: Список читателей (RLT).
    struct visitor {
      int operator()(const mdbx::env::reader_info &ri, int) {
        std::cout << "reader slot=" << ri.slot << " pid=" << ri.pid << " tid=" << ri.thread
                  << " txnid=" << ri.transaction_id << " lag=" << ri.transaction_lag << "\n";
        return mdbx::continue_loop;
      }
    } v;
    env.enumerate_readers(v);

    // EN: Latency of the last commit (see S20).
    // RU: Латенция последнего коммита (см. S20).
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("last"), mdbx::slice("v"));
      const auto lat = txn.commit_get_latency();
      std::cout << "commit latency: whole=" << lat.whole << " us\n";
    }

    // EN: Integrity check (CLI): mdbx_chk -w <path> [-vvv]
    // RU: Проверка целостности (CLI): mdbx_chk -w <path> [-vvv]
    std::cout << "hint: run 'mdbx_chk -w -vvv " << path << "' to verify integrity\n";

    std::cout << "ok: diagnostics dumpers work\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}