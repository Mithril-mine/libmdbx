// EN: fork() and resurrect_after_fork
// EN: Textbook section: Volume II, chapter 11, "Multithreading", section "fork".
// EN: What it demonstrates: after fork() the child process continues to work with the same
// EN:   environment; mdbx_env_resurrect_after_fork() restores the state of
// EN:   locks/readers in the child.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 15-fork-resurrect.c++ -lmdbx
// EN: Run: ./15-fork-resurrect
// EN: Expected output (POSIX):
// EN:   parent: wrote key=v1
// EN:   child: resurrected, reads key=v1, wrote key=child
// EN:   parent: after child, key=child
// EN:   ok: fork + resurrect_after_fork
// EN: On Windows the example is not executed and reports a skip.
// RU: fork() and resurrect_after_fork
// RU: Раздел учебника: Том II, глава 11, «Многопоточность», раздел «fork».
// RU: Что демонстрирует: после fork() дочерний процесс продолжает работать с тем же
// RU:   окружением; mdbx_env_resurrect_after_fork() восстанавливает состояние
// RU:   блокировок/читателей в потомке.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 15-fork-resurrect.c++ -lmdbx
// RU: Запуск: ./15-fork-resurrect
// RU: Ожидаемый вывод (POSIX):
// RU:   parent: wrote key=v1
// RU:   child: resurrected, reads key=v1, wrote key=child
// RU:   parent: after child, key=child
// RU:   ok: fork + resurrect_after_fork
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
    const std::string path = std::string(example::tmpdir()) + "/mdbx-15-fork-resurrect.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      txn.commit();
    }
    std::cout << "parent: wrote key=v1" << std::endl;
    // EN: so that the stdout buffer is not duplicated in the child
    // RU: чтобы буфер stdout не дублировался в потомке
    std::cout.flush();

    const pid_t pid = fork();
    if (pid < 0) {
      std::cerr << "fork failed\n";
      return EXIT_FAILURE;
    }

    if (pid == 0) {
      // EN: The child: restore the environment after fork() and read the data.
      // RU: Потомок: восстановить окружение после fork() и прочитать данные.
      env.resurrect_after_fork();
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      std::cout << "child: resurrected, reads key=" << txn.get(table, mdbx::slice("key")).as_string() << std::endl;
      txn.upsert(table, mdbx::slice("key"), mdbx::slice("child"));
      txn.commit();
      env.close();
      _exit(EXIT_SUCCESS);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
      std::cerr << "child failed\n";
      return EXIT_FAILURE;
    }

    {
      auto txn = env.start_read();
      auto table = txn.open_map(nullptr);
      std::cout << "parent: after child, key=" << txn.get(table, mdbx::slice("key")).as_string() << "\n";
      txn.abort();
    }

    std::cout << "ok: fork + resurrect_after_fork\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}
#endif // Windows vs POSIX