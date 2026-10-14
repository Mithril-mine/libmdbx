// EN: CRUD: put/get/del with write flags
// EN: Textbook section: Volume I, chapter 4, "Basic CRUD operations",
// EN:   sections "put/get/del/replace", "Write flags".
// EN: What it demonstrates: NOOVERWRITE (insert_unique) and MDBX_KEYEXIST as an expected code;
// EN:   UPSERT; CURRENT (update); deletion; MDBX_NOTFOUND as an expected code.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 03-crud.c++ -lmdbx
// EN: Run: ./03-crud
// EN: Expected output:
// EN:   insert duplicate -> MDBX_KEYEXIST as expected
// EN:   try_insert duplicate -> done=0
// EN:   after upsert: value2
// EN:   update existing: value3
// EN:   update absent -> MDBX_NOTFOUND as expected
// EN:   erase(key) -> removed=true
// EN:   get(after erase) -> MDBX_NOTFOUND as expected
// EN:   ok: NOOVERWRITE/UPSERT/CURRENT; KEYEXIST/NOTFOUND handled explicitly
// EN: On failure — message to stderr and exit 1.
// RU: CRUD: put/get/del with write flags
// RU: Раздел учебника: Том I, глава 4, «Базовые операции CRUD»,
// RU:   разделы «put/get/del/replace», «Флаги записи».
// RU: Что демонстрирует: NOOVERWRITE (insert_unique) и MDBX_KEYEXIST как ожидаемый код;
// RU:   UPSERT; CURRENT (update); удаление; MDBX_NOTFOUND как ожидаемый код.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 03-crud.c++ -lmdbx
// RU: Запуск: ./03-crud
// RU: Ожидаемый вывод:
// RU:   insert duplicate -> MDBX_KEYEXIST as expected
// RU:   try_insert duplicate -> done=0
// RU:   after upsert: value2
// RU:   update existing: value3
// RU:   update absent -> MDBX_NOTFOUND as expected
// RU:   erase(key) -> removed=true
// RU:   get(after erase) -> MDBX_NOTFOUND as expected
// RU:   ok: NOOVERWRITE/UPSERT/CURRENT; KEYEXIST/NOTFOUND handled explicitly
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-03-crud.mdbx";
    mdbx::env::remove(path);

    mdbx::env_managed env(path, mdbx::env_managed::create_parameters(), mdbx::env::operate_parameters());
    auto txn = env.start_write();
    auto table = txn.open_map(nullptr);

    // EN: NOOVERWRITE: inserting a unique key.
    // RU: NOOVERWRITE: вставка уникального ключа.
    txn.insert(table, mdbx::slice("key"), mdbx::slice("value1"));

    // EN: Re-inserting the same key -> key_exists (MDBX_KEYEXIST).
    // RU: Повторная вставка того же ключа -> key_exists (MDBX_KEYEXIST).
    try {
      txn.insert(table, mdbx::slice("key"), mdbx::slice("value2"));
      std::cerr << "FAIL: duplicate insert did not throw\n";
      return EXIT_FAILURE;
    } catch (const mdbx::key_exists &) {
      std::cout << "insert duplicate -> MDBX_KEYEXIST as expected\n";
    }

    // EN: try_insert does not throw; it reports the result via the done flag.
    // RU: try_insert не бросает исключение, а сообщает результат флагом done.
    auto ins = txn.try_insert(table, mdbx::slice("key"), mdbx::slice("value2"));
    std::cout << "try_insert duplicate -> done=" << (ins.done ? 1 : 0) << "\n";

    // EN: UPSERT: insert or overwrite.
    // RU: UPSERT: вставить или перезаписать.
    txn.upsert(table, mdbx::slice("key"), mdbx::slice("value2"));
    std::cout << "after upsert: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";

    // EN: CURRENT (update): update only an existing key.
    // RU: CURRENT (update): обновить только существующий ключ.
    txn.update(table, mdbx::slice("key"), mdbx::slice("value3"));
    std::cout << "update existing: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";
    try {
      txn.update(table, mdbx::slice("absent"), mdbx::slice("x"));
      std::cerr << "FAIL: update of absent key did not throw\n";
      return EXIT_FAILURE;
    } catch (const mdbx::not_found &) {
      std::cout << "update absent -> MDBX_NOTFOUND as expected\n";
    }

    // EN: Deletion by key.
    // RU: Удаление по ключу.
    const bool removed = txn.erase(table, mdbx::slice("key"));
    std::cout << "erase(key) -> removed=" << (removed ? "true" : "false") << "\n";
    try {
      (void)txn.get(table, mdbx::slice("key"));
      std::cerr << "FAIL: get after erase did not throw\n";
      return EXIT_FAILURE;
    } catch (const mdbx::not_found &) {
      std::cout << "get(after erase) -> MDBX_NOTFOUND as expected\n";
    }

    txn.commit();

    std::cout << "ok: NOOVERWRITE/UPSERT/CURRENT; KEYEXIST/NOTFOUND handled explicitly\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}