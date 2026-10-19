// EN: Error handling: parsing return codes, retry strategy
// EN: Textbook section: Volume II, chapter 12, "Error handling",
// EN:   sections "Codes", "Retry strategies".
// EN: What it demonstrates: handling of expected codes (MDBX_KEYEXIST, MDBX_NOTFOUND)
// EN:   as exceptions mdbx::key_exists/mdbx::not_found; reading the numeric code from
// EN:   the exception; a retry pattern on MDBX_BUSY (environment busy).
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 17-error-handling.c++ -lmdbx
// EN: Run: ./17-error-handling
// EN: Expected output:
// EN:   key_exists: code=<N> (MDBX_KEYEXIST: Key/data pair already exists)
// EN:   not_found: code=<N> (MDBX_NOTFOUND: No matching key/data pair found)
// EN:   busy attempt 1: failed (<reason>)
// EN:   busy: persists after 3 attempts (expected while env is open)
// EN:   ok: expected codes parsed, retry pattern shown
// EN: On failure — message to stderr and exit 1.
// RU: Обработка ошибок: разбор кодов возврата, retry-стратегия
// RU: Раздел учебника: Том II, глава 12, «Обработка ошибок»,
// RU:   разделы «Коды», «Retry-стратегии».
// RU: Что демонстрирует: обработку ожидаемых кодов (MDBX_KEYEXIST, MDBX_NOTFOUND)
// RU:   как исключений mdbx::key_exists/mdbx::not_found; чтение числового кода из
// RU:   исключения; шаблон retry на MDBX_BUSY (занятость окружения).
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 17-error-handling.c++ -lmdbx
// RU: Запуск: ./17-error-handling
// RU: Ожидаемый вывод:
// RU:   key_exists: code=<N> (MDBX_KEYEXIST: Key/data pair already exists)
// RU:   not_found: code=<N> (MDBX_NOTFOUND: No matching key/data pair found)
// RU:   busy attempt 1: failed (<причина>)
// RU:   busy: persists after 3 attempts (expected while env is open)
// RU:   ok: expected codes parsed, retry pattern shown
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include "common.h++"
#include <mdbx.h++>

namespace {

// EN: Prints the code and the name of the exception.
// RU: Печатает код и имя исключения.
void report(const char *label, const mdbx::exception &ex) {
  std::cout << label << ": code=" << ex.error().code() << " (" << ex.what() << ")\n";
}

} // namespace

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-17-error-handling.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    auto txn = env.start_write();
    auto table = txn.open_map(nullptr);

    // EN: MDBX_KEYEXIST — the expected code when inserting an existing key.
    // RU: MDBX_KEYEXIST — ожидаемый код при вставке существующего ключа.
    txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
    try {
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v2"));
      std::cerr << "FAIL: duplicate insert did not throw\n";
      return EXIT_FAILURE;
    } catch (const mdbx::key_exists &ex) {
      report("key_exists", ex);
    }

    // EN: MDBX_NOTFOUND — the expected code when reading a missing key.
    // RU: MDBX_NOTFOUND — ожидаемый код при чтении отсутствующего ключа.
    try {
      (void)txn.get(table, mdbx::slice("absent"));
      std::cerr << "FAIL: get of absent key did not throw\n";
      return EXIT_FAILURE;
    } catch (const mdbx::not_found &ex) {
      report("not_found", ex);
    }
    txn.commit();

    // EN: Retry strategy: an attempt to open the environment exclusively while
    // EN: it is already open by the current process yields "busy" (MDBX_BUSY on
    // EN: process contention, or the system error EAGAIN within a single process).
    // EN: Such a failure is caught and handled by retrying with a limit.
    // RU: Retry-стратегия: попытка открыть окружение в исключительном режиме, пока
    // RU: оно уже открыто текущим процессом, даёт «занятость» (MDBX_BUSY при
    // RU: конкуренции процессов или системную ошибку EAGAIN внутри процесса).
    // RU: Такой отказ перехватывается и обрабатывается повтором с ограничением.
    const int max_attempts = 3;
    int attempt = 0;
    for (; attempt < max_attempts; ++attempt) {
      try {
        mdbx::env_managed busy(path, mdbx::env::operate_parameters().exclusive());
        std::cerr << "FAIL: exclusive open unexpectedly succeeded\n";
        return EXIT_FAILURE;
      } catch (const std::exception &ex) {
        std::cout << "busy attempt " << (attempt + 1) << ": failed (" << ex.what() << ")\n";
      }
    }
    std::cout << "busy: persists after " << attempt << " attempts (expected while env is open)\n";

    std::cout << "ok: expected codes parsed, retry pattern shown\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}