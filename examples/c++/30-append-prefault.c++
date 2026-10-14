// EN: Bulk load: MDBX_APPEND + prefault_write_enable
// EN: Textbook section: Volume IV, chapter 23, "Micro-optimizations", §23.3/§23.4.
// EN: What it demonstrates: bulk loading of sorted keys via txn.append()
// EN:   (MDBX_APPEND) with prefault_write_enable; a time comparison with regular
// EN:   insertion (for information — numbers depend on the system).
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 30-append-prefault.c++ -lmdbx
// EN: Run: ./30-append-prefault
// EN: Expected output:
// EN:   append load: <T1> us
// EN:   regular insert load: <T2> us
// EN:   ok: APPEND bulk-load with prefault
// EN: On failure — message to stderr and exit 1.
// RU: Пакетная загрузка: MDBX_APPEND + prefault_write_enable
// RU: Раздел учебника: Том IV, глава 23, «Микрооптимизации», §23.3/§23.4.
// RU: Что демонстрирует: пакетную загрузку отсортированных ключей через
// RU:   txn.append() (MDBX_APPEND) с prefault_write_enable; сравнение времени
// RU:   с обычной вставкой (информационно — числа зависят от системы).
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 30-append-prefault.c++ -lmdbx
// RU: Запуск: ./30-append-prefault
// RU: Ожидаемый вывод:
// RU:   append load: <T1> us
// RU:   regular insert load: <T2> us
// RU:   ok: APPEND bulk-load with prefault
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

namespace {

// EN: MDBX_APPEND requires strictly increasing keys, so we use fixed-width keys
// EN: with leading zeros (the order matches the numeric one).
// RU: MDBX_APPEND требует строго возрастающих ключей, поэтому используем
// RU: ключи фиксированной ширины с ведущими нулями (порядок совпадает с числовым).
std::string key_of(int i) {
  char buf[16];
  snprintf(buf, sizeof(buf), "k%08d", i);
  return buf;
}

} // namespace

static uint64_t load_append(const std::string &path, int count) {
  mdbx::env::remove(path);
  auto env = example::env_open(path);
  env.set_extra_option(mdbx::env::extra_runtime_option::prefault_write_enable, 1);

  const auto t0 = std::chrono::steady_clock::now();
  auto txn = env.start_write();
  auto table = txn.open_map(nullptr);
  for (int i = 0; i < count; ++i)
    // EN: MDBX_APPEND
    // RU: MDBX_APPEND
    txn.append(table, mdbx::slice(key_of(i)), mdbx::slice("value"));
  txn.commit();
  const auto t1 = std::chrono::steady_clock::now();
  return uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count());
}

static uint64_t load_insert(const std::string &path, int count) {
  mdbx::env::remove(path);
  auto env = example::env_open(path);

  const auto t0 = std::chrono::steady_clock::now();
  auto txn = env.start_write();
  auto table = txn.open_map(nullptr);
  for (int i = 0; i < count; ++i)
    txn.insert(table, mdbx::slice(key_of(i)), mdbx::slice("value"));
  txn.commit();
  const auto t1 = std::chrono::steady_clock::now();
  return uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count());
}

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-30-append-prefault.mdbx";
    constexpr int count = 200000;

    const uint64_t append_us = load_append(path, count);
    const uint64_t insert_us = load_insert(path, count);
    std::cout << "append load: " << append_us << " us\n";
    std::cout << "regular insert load: " << insert_us << " us\n";

    std::cout << "ok: APPEND bulk-load with prefault\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}