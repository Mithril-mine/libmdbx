// EN: Data model: MDBX_val, integer keys, empty key/value, named tables
// EN: Textbook section: Volume I, chapter 3, "Basic data model",
// EN:   sections "MDBX_val", "Main table and dbi", "Integer keys", "Zero-length keys/values".
// EN: What it demonstrates: representation of keys and values as "pointer+length";
// EN:   the main table and named tables; integer keys (key_mode::ordinal);
// EN:   an empty key and an empty value.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 02-data-model.c++ -lmdbx
// EN: Run: ./02-data-model
// EN: Expected output:
// EN:   main[key] = value
// EN:   named[config] = 42
// EN:   ordinal[1000] = thousand
// EN:   empty value length = 0
// EN:   ok: integer keys, named table, empty key/value
// EN: On failure — message to stderr and exit 1.
// RU: Data model: MDBX_val, integer keys, empty key/value, named tables
// RU: Раздел учебника: Том I, глава 3, «Базовая модель данных»,
// RU:   разделы «MDBX_val», «Главная таблица и dbi», «Integer-ключи», «Нулевые ключи/значения».
// RU: Что демонстрирует: представление ключей и значений как «указатель+длина»;
// RU:   главную таблицу и именованные таблицы; integer-ключи (key_mode::ordinal);
// RU:   пустой ключ и пустое значение.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 02-data-model.c++ -lmdbx
// RU: Запуск: ./02-data-model
// RU: Ожидаемый вывод:
// RU:   main[key] = value
// RU:   named[config] = 42
// RU:   ordinal[1000] = thousand
// RU:   empty value length = 0
// RU:   ok: integer keys, named table, empty key/value
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <cstdint>
#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

using buffer = mdbx::buffer<mdbx::default_allocator, mdbx::default_capacity_policy>;

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-02-data-model.mdbx";
    mdbx::env::remove(path);

    mdbx::env_managed env = example::env_open(path);
    auto txn = env.start_write();

    // EN: The main table (name == nullptr): ordinary keys and values.
    // RU: Главная таблица (name == nullptr): обычные ключи и значения.
    auto main = txn.open_map(nullptr);
    txn.insert(main, mdbx::slice("key"), mdbx::slice("value"));
    std::cout << "main[key] = " << txn.get(main, mdbx::slice("key")).as_string() << "\n";

    // EN: A named table: a separate key space inside the same database.
    // RU: Именованная таблица: отдельное пространство ключей внутри той же БД.
    auto named = txn.create_map("named", mdbx::key_mode::usual, mdbx::value_mode::single);
    txn.insert(named, mdbx::slice("config"), mdbx::slice("42"));
    std::cout << "named[config] = " << txn.get(named, mdbx::slice("config")).as_string() << "\n";

    // EN: Integer keys: a table with key_mode::ordinal (MDBX_INTEGERKEY),
    // EN: keys are uint64_t in native byte order.
    // RU: Integer-ключи: таблица с key_mode::ordinal (MDBX_INTEGERKEY),
    // RU: ключи — uint64_t в нативном порядке байт.
    auto ord = txn.create_map("ordinal", mdbx::key_mode::ordinal, mdbx::value_mode::single);
    const uint64_t k1 = 1, k2 = 1000;
    txn.insert(ord, buffer::key_from_u64(k1), mdbx::slice("one"));
    txn.insert(ord, buffer::key_from_u64(k2), mdbx::slice("thousand"));
    std::cout << "ordinal[1000] = " << txn.get(ord, buffer::key_from_u64(k2)).as_string() << "\n";

    // EN: An empty key (zero length) and an empty value.
    // RU: Пустой ключ (нулевой длины) и пустое значение.
    txn.insert(main, mdbx::slice(), mdbx::slice("empty-key-value"));
    txn.insert(named, mdbx::slice("empty"), mdbx::slice());
    std::cout << "empty value length = " << txn.get(named, mdbx::slice("empty")).size() << "\n";

    txn.commit();

    std::cout << "ok: integer keys, named table, empty key/value\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}