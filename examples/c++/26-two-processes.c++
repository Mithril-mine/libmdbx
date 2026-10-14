// EN: Two processes on one database
// EN: Textbook section: Volume III, chapter 19, "Lock file", §19.2/§19.3.
// EN: What it demonstrates: inter-process access — a writer process commits data,
// EN:   a reader process (a child via fork) reads it; mdbx_reader_check() clears
// EN:   dead reader slots.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 26-two-processes.c++ -lmdbx
// EN: Run: ./26-two-processes
// EN: Expected output (POSIX):
// EN:   parent wrote: k=v1
// EN:   child reads: v1
// EN:   check_readers cleared slots: 0
// EN:   ok: two processes share the database
// EN: On Windows the example does not run and reports a skip.
// RU: Два процесса на одной базе данных
// RU: Раздел учебника: Том III, глава 19, «Файл блокировок», §19.2/§19.3.
// RU: Что демонстрирует: межпроцессный доступ — процесс-писатель коммитит данные,
// RU:   процесс-читатель (потомок через fork) их читает; mdbx_reader_check()
// RU:   очищает мёртвые слоты читателей.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 26-two-processes.c++ -lmdbx
// RU: Запуск: ./26-two-processes
// RU: Ожидаемый вывод (POSIX):
// RU:   parent wrote: k=v1
// RU:   child reads: v1
// RU:   check_readers cleared slots: 0
// RU:   ok: two processes share the database
// RU: На Windows пример не выполняется и сообщает о пропуске.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS)
int main() {
  std::cout << "skipped: fork() is not available on Windows\n";
  return EXIT_SUCCESS;
}
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-26-two-processes.mdbx";
    mdbx::env::remove(path);

    // EN: Writer: the parent process commits the data and closes the environment.
    // RU: Писатель: родительский процесс коммитит данные и закрывает окружение.
    {
      auto env = example::env_open(path);
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("k"), mdbx::slice("v1"));
      txn.commit();
      std::cout << "parent wrote: k=v1" << std::endl;
    }

    const pid_t pid = fork();
    if (pid < 0) {
      std::cerr << "fork failed\n";
      return EXIT_FAILURE;
    }

    if (pid == 0) {
      // EN: Reader: the child opens the same DB and reads the data.
      // RU: Читатель: потомок открывает ту же БД и читает данные.
      auto env = example::env_open(path);
      auto txn = env.start_read();
      auto table = txn.open_map(nullptr);
      std::cout << "child reads: " << txn.get(table, mdbx::slice("k")).as_string() << std::endl;
      txn.abort();
      _exit(EXIT_SUCCESS);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
      std::cerr << "child failed\n";
      return EXIT_FAILURE;
    }

    // EN: Checking the readers table: dead slots after the child exits.
    // RU: Проверка таблицы читателей: мёртвые слоты после выхода потомка.
    auto env = example::env_open(path);
    const unsigned dead = env.check_readers();
    std::cout << "check_readers cleared slots: " << dead << "\n";

    std::cout << "ok: two processes share the database\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}
#endif // Windows vs POSIX