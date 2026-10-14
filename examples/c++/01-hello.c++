// EN: Hello, libmdbx — minimal example
// EN: Textbook section: Volume I, chapter 2, "Installation and first launch", section "Minimal example".
// EN: What it demonstrates: the full cycle open (create) → put → get → del → close;
// EN:   the semantics of the two-argument constructor env_managed(path, operate_parameters),
// EN:   which opens only an existing database (analogous to mode == 0 in the C API mdbx_env_open()).
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 01-hello.c++ -lmdbx
// EN: Run: ./01-hello
// EN: Expected output:
// EN:   got: hello, libmdbx
// EN:   reopen(mode=0) on existing db: ok
// EN:   reopen(mode=0) on missing db: refused (ENOFILE)
// EN:   ok: value = "hello, libmdbx"; deleted
// EN: On failure — message to stderr and exit 1.
// RU: Hello, libmdbx — минимальный пример
// RU: Раздел учебника: Том I, глава 2, «Установка и первый запуск», раздел «Минимальный пример».
// RU: Что демонстрирует: полный цикл open (создание) → put → get → del → close;
// RU:   семантику двухаргументного конструктора env_managed(path, operate_parameters),
// RU:   открывающего только существующую БД (аналог mode == 0 в C API mdbx_env_open()).
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 01-hello.c++ -lmdbx
// RU: Запуск: ./01-hello
// RU: Ожидаемый вывод:
// RU:   got: hello, libmdbx
// RU:   reopen(mode=0) on existing db: ok
// RU:   reopen(mode=0) on missing db: refused (ENOFILE)
// RU:   ok: value = "hello, libmdbx"; deleted
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-01-hello.mdbx";
    // EN: erase leftovers from the previous run
    // RU: стереть остатки предыдущего запуска
    mdbx::env::remove(path);

    // EN: Creation: the constructor with create_parameters creates the DB if it does not exist.
    // RU: Создание: конструктор с create_parameters создаёт БД, если её нет.
    mdbx::env_managed env(path, mdbx::env_managed::create_parameters(), mdbx::env::operate_parameters());
    {
      auto txn = env.start_write();
      // EN: main table
      // RU: главная таблица
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("hello, libmdbx"));
      txn.commit();
    }
    {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      std::cout << "got: " << rtxn.get(table, mdbx::slice("key")).as_string() << "\n";
    }

    // EN: Opening an existing database: the two-argument constructor does not create a new one.
    // RU: Открытие существующей БД: двухаргументный конструктор не создаёт новую.
    env.close();
    {
      mdbx::env_managed existing(path, mdbx::env::operate_parameters());
      std::cout << "reopen(mode=0) on existing db: ok\n";
      auto wtxn = existing.start_write();
      auto table = wtxn.open_map(nullptr);
      wtxn.erase(table, mdbx::slice("key"));
      wtxn.commit();
    }

    // EN: Opening a missing database must fail with an error.
    // RU: Открытие отсутствующей БД обязано завершиться ошибкой.
    try {
      mdbx::env_managed missing(path + ".absent", mdbx::env::operate_parameters());
      std::cerr << "FAIL: unexpectedly opened a non-existent database\n";
      return EXIT_FAILURE;
    } catch (const std::exception &ex) {
      // EN: missing-file codes (MDBX_ENOFILE) arrive as std::system_error
      // RU: коды отсутствия файла (MDBX_ENOFILE) приходят как std::system_error
      std::cout << "reopen(mode=0) on missing db: refused (" << ex.what() << ")\n";
    }

    std::cout << "ok: value = \"hello, libmdbx\"; deleted\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}